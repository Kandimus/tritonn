//=================================================================================================
//===
//=== modbustcpslave_manager.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс-поток Modbus TCP Slave
//===
//=================================================================================================

#include <string.h>
#include "cJSON.h"
#include "time64.h"
#include "locker.h"
#include "stringex.h"
#include "hash.h"
#include "log_manager.h"
#include "xml_util.h"
#include "error.h"
#include "data_manager.h"
#include "variable_item.h"
#include "variable_list.h"
#include "data_snapshot_item.h"
#include "data_snapshot.h"
#include "users.h"
#include "listconf.h"
#include "simplefile.h"
#include "units.h"
#include "modbustcpslave_client.h"
#include "modbustcpslave_manager.h"


extern rVariable *gVariable;


///////////////////////////////////////////////////////////////////////////////////////////////////
//
// КОНСТРУКТОРЫ И ДЕСТРУКТОР
rModbusTCPSlaveManager::rModbusTCPSlaveManager()
	: rTCPClass("0.0.0.0", TCP_PORT_MODBUS, MAX_MBTCP_CLIENT),
	  rInterface(Mutex),
	  m_snapshot(rDataManager::instance().getVariableClass())
{
	RTTI        = "rModbusTCPSlaveManager";
	LogMask    |= LM_TERMINAL;
	Swap.Byte   = 1;
	Swap.Word   = 0;
	Swap.DWord  = 0;
	Modbus      = new UINT[65536];

	memset(Modbus, 0, sizeof(Modbus[0]) * 65536);
}


rModbusTCPSlaveManager::~rModbusTCPSlaveManager()
{
	if (Modbus) {
		delete[] Modbus;
		Modbus = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
//





//-------------------------------------------------------------------------------------------------
//
rThreadStatus rModbusTCPSlaveManager::Proccesing()
{
	rError err;

	if (LoadStandartModbus(err)) {
		CloseServer();
		Finish();

		rDataManager::instance().DoHalt(HALT_REASON_CONFIGFILE | err.getError());
		return rThreadStatus::FINISHED;
	}

	while(1)
	{
		// Обработка команд нити
		rThreadStatus thread_status = rTCPClass::Proccesing();
		if(!THREAD_IS_WORK(thread_status))
		{
			return thread_status;
		}

		m_snapshot.resetAssign();
		rDataManager::instance().set(m_snapshot);

		{
			rLocker lock(Mutex); lock.Nop();

			// TODO Нужно разобрать Snapshot и выложить его в Modbus
			for(UDINT ii = 0; ii < ModbusLink.size(); ++ii)
			{
				rModbusLink *link = &ModbusLink[ii];

				if(!link->m_item->isAssigned()) {
					continue;
				}

				Modbus[link->Address] = 0;
				link->m_item->getBuffer(&Modbus[link->Address]);
				SwapBuffer(&Modbus[link->Address], link->m_item->getSizeVar());
			}

			rVariableClass::processing();
			rThreadClass::EndProccesing();
		}
	}

	return rThreadStatus::UNDEF;
}


rClientTCP *rModbusTCPSlaveManager::NewClient(SOCKET socket, sockaddr_in *addr)
{
	return (rClientTCP *)new rModbusTCPSlaveClient(socket, addr);
}


UDINT rModbusTCPSlaveManager::ClientRecv(rClientTCP *client, USINT *buff, UDINT size)
{
	rModbusTCPSlaveClient *mbclient = (rModbusTCPSlaveClient *)client;
	USINT       *data    = mbclient->Recv(buff, size);
	UDINT        result  = 0;

	while(data == MODBUSTCP_RECV_SUCCESS)
	{
		// Проверка на корректный UnitID
		if(mbclient->Buff[0] == SlaveID || SlaveID == 0)
		{
			// Разбираем команду
			switch(mbclient->Buff[1])
			{
				// Чтение из области 4х
				case 0x03: Func_0x03(mbclient); break;

				// Запись одиночного регистра
				case 0x06: Func_0x06(mbclient); break;

				// Запись мульти-регистров
				case 0x10: Func_0x10(mbclient); break;

				default: Func_Error(mbclient, MODBUS_ERROR_FUNC);
			}
		}

		//
		UDINT packet_len = mbclient->Size + mbclient->HeaderRead;
		if(packet_len < size)
		{
			buff += packet_len;
			size -= packet_len;

			data = mbclient->Recv(buff, size);
		}
		else
		{
			result = 1;
			break;
		}
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
// Обработка функции
void rModbusTCPSlaveManager::Func_0x03 (rModbusTCPSlaveClient *client)
{
	USINT answe[256];
	UINT  adr   = client->Buff[3] + (client->Buff[2] << 8);
	USINT count = client->Buff[5] + (client->Buff[4] << 8);
	USINT size  = 3 + count * 2;

	if(count > 121)
	{
		Func_Error(client, MODBUS_ERROR_DATA);
		return;
	}

	answe[0] = client->Buff[0];
	answe[1] = client->Buff[1];
	answe[2] = size;

	memcpy(answe + 3, &Modbus[adr], count * sizeof(UINT));
	SwapBuffer(answe + 3, count * sizeof(UINT));

	client->Send(answe, size);
}



void rModbusTCPSlaveManager::Func_0x06 (rModbusTCPSlaveClient *client)
{
	USINT answe[6];
	USINT size = 6;
	UINT  adr  = client->Buff[3] + (client->Buff[2] << 8);
	UINT  val  = client->Buff[5] + (client->Buff[4] << 8);

	rSnapshot snapshot(rDataManager::instance().getVariableClass());

	//TODO тут нужен анализ на SecurityModbus
	snapshot.setAccess(ACCESS_MASK_SA);

	rSnapshotItem *ss = FindSnapshotItem(adr);

	if(nullptr == ss)
	{
		memcpy(&Modbus[adr], &val, 2);
	}
	else
	{
		UDINT countregs = TypeCountReg(ss->getVariable()->getType());
		UDINT sizeregs  = countregs << 1;

		if(countregs == 1)
		{
			SwapBuffer(&val, sizeregs);

			snapshot.add(ss->getVariable()->getName(), val);
		}
	}

	rDataManager::instance().set(snapshot);

	memcpy(answe, client->Buff, size);
	client->Send(answe, size);
}



void rModbusTCPSlaveManager::Func_0x10 (rModbusTCPSlaveClient *client)
{
	USINT answe[6];
	USINT size    = 6;
	UINT  adr     = client->Buff[3] + (client->Buff[2] << 8);
	UINT  count   = client->Buff[5] + (client->Buff[4] << 8);
	USINT bytes   = client->Buff[6];
	USINT *values = client->Buff + 7;

	if(count * 2 != bytes)
	{
		Func_Error(client, MODBUS_ERROR_DATA);
		return;
	}

	rSnapshot snapshot(rDataManager::instance().getVariableClass(), ACCESS_MASK_SA); //TODO тут нужен анализ на SecurityModbus

	for(UDINT ii = 0; ii < count;)
	{
		rSnapshotItem *ss = FindSnapshotItem(adr + ii);
		UDINT countregs = 0;
		UDINT sizeregs  = 0;

		if(nullptr == ss)
		{
			memcpy(&Modbus[adr + ii], values + ii * sizeof(INT), sizeof(INT));
			continue;
		}

		countregs = TypeCountReg(ss->getVariable()->getType());
		sizeregs  = countregs << 1;

		if(countregs + ii >= count)
		{
			continue;
		}

		SwapBuffer(values + ii * 2, sizeregs);

		snapshot.add(ss->getVariable()->getName(), values + ii * 2);

		ii += countregs;
	}

	snapshot.set();

	// Ответ
	memcpy(answe, client->Buff, 4);
	answe[4] = bytes;
	answe[5] = 0;

	client->Send(answe, size);
}


//-------------------------------------------------------------------------------------------------
//
void rModbusTCPSlaveManager::Func_Error(rModbusTCPSlaveClient *client, USINT err)
{
	USINT answe[3];

	answe[0] = client->Buff[0];
	answe[1] = client->Buff[1] | 0x80;
	answe[2] = err;

	client->Send(answe, 3);
}


void rModbusTCPSlaveManager::SwapBuffer(void *value, UDINT size)
{
	if(Swap.DWord && !(size & 0x07) && size >= 8)
	{
		UDINT *buff = (UDINT *)value;
		UDINT  temp = buff[0];

		buff[0] = buff[1];
		buff[1] = temp;
	}

	if(Swap.Word && !(size & 0x03) && size >= 4)
	{
		for(UDINT ii = 0; ii < size - 2; ii += 2)
		{
			UINT *buff = (UINT *)value;
			UINT  temp = buff[ii];

			buff[ii]     = buff[ii + 1];
			buff[ii + 1] = temp;
		}
	}

	if(Swap.Byte && !(size & 0x01) && size >= 2)
	{
		for(UDINT ii = 0; ii < size - 2; ii += 2)
		{
			USINT *buff = (USINT *)value;
			USINT temp  = buff[ii];

			buff[ii]     = buff[ii + 1];
			buff[ii + 1] = temp;
		}
	}
}


rSnapshotItem *rModbusTCPSlaveManager::FindSnapshotItem(UINT addr)
{
	for(auto& item : ModbusLink) {
		if(item.Address == addr) return item.m_item;
	}

	return nullptr;
}


UDINT rModbusTCPSlaveManager::TypeCountReg(TT_TYPE type)
{
	switch(type)
	{
		case TYPE_SINT : return 1;
		case TYPE_USINT: return 1;
		case TYPE_INT  : return 1;
		case TYPE_UINT : return 1;
		case TYPE_DINT : return 2;
		case TYPE_UDINT: return 2;
		case TYPE_REAL : return 2;
		case TYPE_LREAL: return 4;
		case TYPE_STRID: return 2;

		default: return 0xFFFF;
	}
}


rThreadClass *rModbusTCPSlaveManager::GetThreadClass()
{
	return (rThreadClass *)this;
}


UDINT rModbusTCPSlaveManager::StartServer()
{
	UDINT result = rTCPClass::StartServer("", 0);

	rTCPClass::Run(300);

	return result;
}



//-------------------------------------------------------------------------------------------------
// Проверка переменных, создание Snapshoot
UDINT rModbusTCPSlaveManager::CheckVars(rError& err)
{
	UDINT address = 0;

	for(auto& tlink : TempLink) {
		rModbusLink link;

		if (!m_snapshot.add(tlink.VarName)) {
			return err.set(DATACFGERR_INTERFACES_NF_VAR, tlink.LineNum, tlink.VarName);
		}

		if (tlink.Address) {
			address = tlink.Address;
		}

		link.Address = address;
		link.m_item  = m_snapshot.last();
		address     += TypeCountReg(link.m_item->getVariable()->getType());

		// check block start + count > 65535
		if (address > 0x0000FFFF) {
			return err.set(DATACFGERR_INTERFACES_ADDROVERFLOW, tlink.LineNum, tlink.VarName);
		}

		ModbusLink.push_back(link);
	}

	return TRITONN_RESULT_OK;
}



//-------------------------------------------------------------------------------------------------
// Загрузка СТАНДАРТНОЙ области данных модбас (системной)
UDINT rModbusTCPSlaveManager::LoadStandartModbus(rError& err)
{
	string filetext = "";
	UDINT  result   = TRITONN_RESULT_OK;

	if(TRITONN_RESULT_OK != (result = SimpleFileLoad(FILE_MODBUS, filetext)))
	{
		return err.set(result, 0, "");
	}

	tinyxml2::XMLDocument doc;
	tinyxml2::XMLElement* xml_modbus;

	if(tinyxml2::XML_SUCCESS != doc.Parse(filetext.c_str()))
	{
		return err.set(doc.ErrorID(), doc.ErrorLineNum(), doc.ErrorStr());
	}

	xml_modbus = doc.FirstChildElement("modbus");
	if(!xml_modbus)
	{
		return TRITONN_RESULT_OK;
	}

	XML_FOR(xml_item, xml_modbus, XmlName::ITEM) {
		UDINT fault = 0;
		rModbusLink link;

		m_snapshot.add(XmlUtils::getTextString(xml_item, "", fault));

		link.Address = XmlUtils::getAttributeUDINT(xml_item, "addr", 0xFFFF); //TODO заменить на константу
		link.m_item  = m_snapshot.last();

		if (link.Address == 0xFFFF || nullptr == link.m_item->getVariable()) {
			return err.set(DATACFGERR_INTERFACES_NF_VAR, xml_item->GetLineNum(), "not found variable or incorrect address");
		}

		ModbusLink.push_back(link);
	}

	return 0;
}




//-------------------------------------------------------------------------------------------------
// Поиск требуемого dataset в дереве конфигурации
tinyxml2::XMLElement *rModbusTCPSlaveManager::FindBlock(tinyxml2::XMLElement *element, const string &name)
{
	tinyxml2::XMLNode    *xml_modbus = element->Parent();
	tinyxml2::XMLElement *xml_blocks = nullptr;

	if(nullptr == xml_modbus) return nullptr;

	xml_blocks = xml_modbus->FirstChildElement(XmlName::DATAMAP);

	if(nullptr == xml_blocks) return nullptr;

	for(tinyxml2::XMLElement *xml_item = xml_blocks->FirstChildElement(XmlName::DATABLOCK); xml_item != nullptr; xml_item = xml_item->NextSiblingElement(XmlName::DATABLOCK))
	{
		const char *item_name = xml_item->Attribute(XmlName::NAME);

		if(nullptr == item_name) continue;

		if(String_equali(name, item_name)) return xml_item;
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
UDINT rModbusTCPSlaveManager::loadFromXML(tinyxml2::XMLElement* xml_root, rError& err)
{
	UDINT       port  = 0;
	std::string ip    = "";

	rInterface::loadFromXML(xml_root, err);

	Alias    = "comms.modbus." + Alias;
	port     = XmlUtils::getAttributeUDINT (xml_root, XmlName::PORT    , TCP_PORT_MODBUS);
	ip       = "0.0.0.0";
	Name     = XmlUtils::getAttributeString(xml_root, XmlName::NAME    , "");
	SlaveID  = XmlUtils::getAttributeUDINT (xml_root, XmlName::ID      , 0);
	Security = XmlUtils::getAttributeUDINT (xml_root, XmlName::SECURITY, 0);
	MaxError = XmlUtils::getAttributeUDINT (xml_root, XmlName::COUNTERR, 3);

	tinyxml2::XMLElement* xml_adrmap = xml_root->FirstChildElement(XmlName::ADDRESSMAP);
	tinyxml2::XMLElement* xml_swap   = xml_root->FirstChildElement(XmlName::SWAP);
	tinyxml2::XMLElement* xml_wlist  = xml_root->FirstChildElement(XmlName::WHITELIST);

	if (!xml_adrmap) {
		return err.set(DATACFGERR_INTERFACES_NF_BLOCKS, xml_root->GetLineNum(), "");
	}

	// Загружаем информацию по Swap
	if(xml_swap) {
		UDINT fault = 0;

		Swap.Byte  = XmlUtils::getTextUSINT(xml_swap->FirstChildElement(XmlName::BYTE ), Swap.Byte , fault);
		Swap.Word  = XmlUtils::getTextUSINT(xml_swap->FirstChildElement(XmlName::WORD ), Swap.Word , fault);
		Swap.DWord = XmlUtils::getTextUSINT(xml_swap->FirstChildElement(XmlName::DWORD), Swap.DWord, fault);
	}

	// Загружаем список "белых" адрессов
	if (xml_wlist) {
		XML_FOR(xml_item, xml_wlist, XmlName::IP) {
			if(xml_item->GetText()) {
				if (!AddWhiteIP(xml_item->GetText())) {
					return err.set(DATACFGERR_INCORRECT_IP, xml_wlist->GetLineNum(), "");
				}
			}
		}
	}


	// Перебираем указанные блоки
	XML_FOR(xml_item, xml_adrmap, XmlName::ADDRESSBLOCK) {
		// Считываем блок модбаса
		UDINT  fault     = 0;
		UDINT  address   = XmlUtils::getAttributeUDINT(xml_item, XmlName::BEGIN, 0xFFFFFFFF);
		string blockname = XmlUtils::getTextString    (xml_item, "", fault);

		if (address < 400000 || address > 465535) {
			return err.set(DATACFGERR_INTERFACES_BADADDR, xml_item->GetLineNum(), "");
		}
		address -= 400000;

		if (fault) {
			return err.set(DATACFGERR_INTERFACES_BADBLOCK, xml_item->GetLineNum(), "");
		}

		tinyxml2::XMLElement* xml_block = FindBlock(xml_root, blockname);

		if (!xml_block) {
			return err.set(DATACFGERR_INTERFACES_BADBLOCK, xml_root->GetLineNum(), "");
		}

		//
		XML_FOR (xml_var, xml_block, XmlName::VARIABLE) {
			rTempLink tlink;

			fault         = 0;
			tlink.Address = address;
			tlink.VarName = XmlUtils::getTextString(xml_var, "", fault);
			tlink.LineNum = xml_var->GetLineNum();
			address       = 0;

			if (fault) {
				return err.set(DATACFGERR_INTERFACES_BADVAR, xml_var->GetLineNum(), "");
			}

			TempLink.push_back(tlink);
		}
	}

	SetServerIP(ip, port);

	return TRITONN_RESULT_OK;
}


UDINT rModbusTCPSlaveManager::generateVars(rVariableClass* parent)
{
	m_varList.add(Alias + ".status" , TYPE_UINT , rVariable::Flags::R___, &Live       , U_DIMLESS, 0);
	m_varList.add(Alias + ".tx"     , TYPE_UDINT, rVariable::Flags::R___, &Tx         , U_DIMLESS, 0);
	m_varList.add(Alias + ".rx"     , TYPE_UDINT, rVariable::Flags::R___, &Rx         , U_DIMLESS, 0);
	m_varList.add(Alias + ".errorrx", TYPE_USINT, rVariable::Flags::R___, &RxError    , U_DIMLESS, 0);
	m_varList.add(Alias + ".clients", TYPE_USINT, rVariable::Flags::R___, &ClientCount, U_DIMLESS, 0);

	if (parent) {
		rVariableClass::linkToExternal(parent);
	}

	return TRITONN_RESULT_OK;
}

/*
UDINT rModbusTCPSlaveManager::SaveKernel(FILE *file, const string &objname, const string &comment)
{
	return TRITONN_RESULT_OK;
}
*/
