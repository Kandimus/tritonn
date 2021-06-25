//=================================================================================================
//===
//=== modbustcpslave_manager.cpp
//===
//=== Copyright (c) 2019-2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс-поток Modbus TCP Slave
//===
//=================================================================================================

#include "modbustcpslave_manager.h"
#include <string.h>
#include "locker.h"
#include "stringex.h"
#include "log_manager.h"
#include "xml_util.h"
#include "../error.h"
#include "../data_manager.h"
#include "../variable_list.h"
#include "../data_snapshot_item.h"
#include "../users.h"
#include "../units.h"
#include "simplefile.h"
#include "modbustcpslave_client.h"
#include "modbus_datablocks.h"
#include "../generator_md.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// КОНСТРУКТОРЫ И ДЕСТРУКТОР
rModbusTCPSlaveManager::rModbusTCPSlaveManager()
	: rTCPClass("0.0.0.0", LanPort::PORT_MODBUS, MAX_MBTCP_CLIENT),
	  rInterface(Mutex),
	  m_snapshot(rDataManager::instance().getVariableClass())
{
	RTTI     = "rModbusTCPSlaveManager";
	LogMask |= LOG::TERMINAL;
	Modbus   = new UINT[65536];

	memset(Modbus, 0, sizeof(Modbus[0]) * 65536);
}


rModbusTCPSlaveManager::~rModbusTCPSlaveManager()
{
	clearTempList();

	if (Modbus) {
		delete[] Modbus;
		Modbus = nullptr;
	}
}

//-------------------------------------------------------------------------------------------------
//
rThreadStatus rModbusTCPSlaveManager::Proccesing()
{
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

			for(auto& link : ModbusLink) {

				if(!link.m_item->isAssigned()) {
					continue;
				}

				TYPE type = link.m_item->getVariable()->getType();

				Modbus[link.m_address] = 0;

				if (link.m_convert == TYPE::UNDEF) {
					link.m_item->getBuffer(&Modbus[link.m_address]);

				} else {
					switch(link.m_convert) {
						case TYPE::USINT: {
							auto val = link.m_item->getValueUSINT();
							memcpy(&Modbus[link.m_address], &val, sizeof(val));
							break;
						}

						case TYPE::SINT: {
							auto val = link.m_item->getValueSINT();
							memcpy(&Modbus[link.m_address], &val, sizeof(val));
							break;
						}

						case TYPE::UINT: {
							auto val = link.m_item->getValueUINT();
							memcpy(&Modbus[link.m_address], &val, sizeof(val));
							break;
						}

						case TYPE::INT: {
							auto val = link.m_item->getValueINT();
							memcpy(&Modbus[link.m_address], &val, sizeof(val));
							break;
						}

						case TYPE::STRID:
						case TYPE::UDINT: {
							auto val = link.m_item->getValueUDINT();
							memcpy(&Modbus[link.m_address], &val, sizeof(val));
							break;
						}

						case TYPE::DINT: {
							auto val = link.m_item->getValueDINT();
							memcpy(&Modbus[link.m_address], &val, sizeof(val));
							break;
						}

						case TYPE::REAL: {
							auto val = link.m_item->getValueREAL();
							memcpy(&Modbus[link.m_address], &val, sizeof(val));
							break;
						}

						case TYPE::LREAL: {
							auto val = link.m_item->getValueLREAL();
							memcpy(&Modbus[link.m_address], &val, sizeof(val));
							break;
						}

						default:
							break;
					}

					type = link.m_convert;
				}

				SwapBuffer(&Modbus[link.m_address], getTypeSize(type));
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
	auto   mbclient = dynamic_cast<rModbusTCPSlaveClient*>(client);
	USINT* data    = mbclient->Recv(buff, size);
	UDINT  result  = 0;

	while (data == MODBUSTCP_RECV_SUCCESS) {
		// Проверка на корректный UnitID
		if (mbclient->Buff[0] == SlaveID || SlaveID == 0) {
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
	if(m_swap.DWord && !(size & 0x07) && size >= 8)
	{
		UDINT *buff = (UDINT *)value;
		UDINT  temp = buff[0];

		buff[0] = buff[1];
		buff[1] = temp;
	}

	if(m_swap.Word && !(size & 0x03) && size >= 4)
	{
		for(UDINT ii = 0; ii < size - 2; ii += 2)
		{
			UINT *buff = (UINT *)value;
			UINT  temp = buff[ii];

			buff[ii]     = buff[ii + 1];
			buff[ii + 1] = temp;
		}
	}

	if(m_swap.Byte && !(size & 0x01) && size >= 2)
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
		if (item.m_address == addr) {
			return item.m_item;
		}
	}

	return nullptr;
}


UDINT rModbusTCPSlaveManager::TypeCountReg(TYPE type)
{
	switch(type)
	{
		case TYPE::SINT : return 1;
		case TYPE::USINT: return 1;
		case TYPE::INT  : return 1;
		case TYPE::UINT : return 1;
		case TYPE::DINT : return 2;
		case TYPE::UDINT: return 2;
		case TYPE::REAL : return 2;
		case TYPE::LREAL: return 4;
		case TYPE::STRID: return 2;

		default: return 0xFFFF;
	}
}


rThreadClass *rModbusTCPSlaveManager::getThreadClass()
{
	return (rThreadClass *)this;
}


UDINT rModbusTCPSlaveManager::startServer()
{
	UDINT result = rTCPClass::startServer("", 0);

	rTCPClass::Run(300);

	return result;
}



//-------------------------------------------------------------------------------------------------
// Проверка переменных, создание Snapshoot
UDINT rModbusTCPSlaveManager::checkVars(rError& err)
{
	UDINT address = 0;

	if (LoadStandartModbus(err)) {
		return err.getError();
	}

	for(auto tlink : m_tempLink) {
		rModbusLink link;
		TYPE type = TYPE::UNDEF;

		if (tlink->VarName.size()) {

			if (!m_snapshot.add(tlink->VarName)) {
				clearTempList();
				return err.set(DATACFGERR_INTERFACES_NF_VAR, tlink->LineNum, tlink->VarName);
			}

			if (tlink->Address) {
				address = tlink->Address;
			}

			link.m_address = address;
			link.m_item    = m_snapshot.last();
			link.m_convert = tlink->m_type;
			type           = link.m_item->getVariable()->getType();

			ModbusLink.push_back(link);

		} else {
			type = tlink->m_type;
		}

		address += TypeCountReg(type);

		// check block start + count > 65535
		if (address > 0x0000FFFF) {
			clearTempList();
			return err.set(DATACFGERR_INTERFACES_ADDR_OVERFLOW, tlink->LineNum, tlink->VarName);
		}
	}

	clearTempList();

	return TRITONN_RESULT_OK;
}



//-------------------------------------------------------------------------------------------------
// Загрузка СТАНДАРТНОЙ области данных модбас (системной)
UDINT rModbusTCPSlaveManager::LoadStandartModbus(rError& err)
{
	string filetext = "";
	UDINT  result   = TRITONN_RESULT_OK;

	if (TRITONN_RESULT_OK != (result = simpleFileLoad(FILE_MODBUS, filetext))) {
		return err.set(result, 0, "");
	}

	tinyxml2::XMLDocument doc;

	if (tinyxml2::XML_SUCCESS != doc.Parse(filetext.c_str())) {
		return err.set(doc.ErrorID(), doc.ErrorLineNum(), doc.ErrorStr());
	}

	auto xml_modbus = doc.FirstChildElement(XmlName::MODBUS);
	if (!xml_modbus) {
		return TRITONN_RESULT_OK;
	}

	XML_FOR(xml_item, xml_modbus, XmlName::ITEM) {
		UDINT fault = 0;
		rModbusLink link;
		std::string alias = XmlUtils::getTextString(xml_item, "", fault);

		m_snapshot.add(alias);

		link.m_address = XmlUtils::getAttributeUDINT(xml_item, XmlName::ADDR, 0xFFFF); //TODO заменить на константу
		link.m_item    = m_snapshot.last();

		if (link.m_address > 100) {
			return err.set(DATACFGERR_INTERFACES_NF_STD_VAR, xml_item->GetLineNum(), String_format("incorrect address %u", link.m_address));
		}

		if (!link.m_item->getVariable()) {
			return err.set(DATACFGERR_INTERFACES_NF_STD_VAR, xml_item->GetLineNum(), alias);
		}

		ModbusLink.push_back(link);
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
UDINT rModbusTCPSlaveManager::loadFromXML(tinyxml2::XMLElement* xml_root, rError& err)
{
	if (rInterface::loadFromXML(xml_root, err) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	auto xml_host = xml_root->FirstChildElement(XmlName::HOST);
	if (xml_host) {
		UDINT       fault = false;
		UDINT       port  = XmlUtils::getTextUINT  (xml_host->FirstChildElement(XmlName::PORT), m_hostPort, fault);
		std::string ip    = XmlUtils::getTextString(xml_host->FirstChildElement(XmlName::IP)  , "0.0.0.0" , fault);

		setServerIP(ip, port);
	}

	m_alias   = "comms.modbus." + m_alias;
	Name      = XmlUtils::getAttributeString(xml_root, XmlName::NAME     , "");
	SlaveID   = XmlUtils::getAttributeUDINT (xml_root, XmlName::ID       , SlaveID);
	Security  = XmlUtils::getAttributeUDINT (xml_root, XmlName::SECURITY , 0);
	MaxError  = XmlUtils::getAttributeUDINT (xml_root, XmlName::COUNT_ERR, MaxError);
	MaxClient = XmlUtils::getAttributeUDINT (xml_root, XmlName::MAXCLIENT, MaxClient);

	auto xml_adrmap = xml_root->FirstChildElement(XmlName::ADDRESSMAP);
	auto xml_swap   = xml_root->FirstChildElement(XmlName::SWAP);
	auto xml_wlist  = xml_root->FirstChildElement(XmlName::WHITELIST);

	if (!xml_adrmap) {
		return err.set(DATACFGERR_INTERFACES_NF_BLOCKS, xml_root->GetLineNum(), "");
	}

	// Загружаем информацию по Swap
	if(xml_swap) {
		UDINT fault = 0;

		m_swap.Byte  = XmlUtils::getTextUSINT(xml_swap->FirstChildElement(XmlName::BYTE ), m_swap.Byte , fault);
		m_swap.Word  = XmlUtils::getTextUSINT(xml_swap->FirstChildElement(XmlName::WORD ), m_swap.Word , fault);
		m_swap.DWord = XmlUtils::getTextUSINT(xml_swap->FirstChildElement(XmlName::DWORD), m_swap.DWord, fault);
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
		UDINT fault     = 0;
		UDINT address   = XmlUtils::getAttributeUDINT(xml_item, XmlName::BEGIN, 0xFFFFFFFF);
		auto  blockname = XmlUtils::getTextString    (xml_item, "", fault);

		if (fault) {
			return err.set(DATACFGERR_INTERFACES_BAD_BLOCK, xml_item->GetLineNum(), "fault blockname");
		}

		if (address < 400000 || address > 465535) {
			return err.set(DATACFGERR_INTERFACES_BAD_ADDR, xml_item->GetLineNum(), "Invalide address " + String_format("%u", address));
		}
		address -= 400000;

		auto xml_block = rDataBlock::Find(xml_root, blockname);

		if (!xml_block) {
			return err.set(DATACFGERR_INTERFACES_BAD_BLOCK, xml_adrmap->GetLineNum(), blockname);
		}

		//
		XML_FOR_ALL(xml_var, xml_block) {
			auto name  = String_tolower(xml_var->Name());
			auto tlink = new rTempLink();

			fault = 0;
			tlink->Address = address;
			tlink->LineNum = xml_var->GetLineNum();
			tlink->m_type  = TYPE::UNDEF;
			address       = 0;

			if (name == XmlName::VARIABLE) {
				tlink->VarName = XmlUtils::getTextString(xml_var, "", fault);

				if (fault) {
					clearTempList();
					return err.set(DATACFGERR_INTERFACES_BAD_VAR, xml_var->GetLineNum(), "");
				}

				std::string convert_type = XmlUtils::getAttributeString(xml_var, XmlName::CONVERT, "", XmlUtils::Flags::TOLOWER);

				if (convert_type.size()) {
					tlink->m_type = getTypeByName(convert_type);

					if (tlink->m_type == TYPE::UNDEF) {
						clearTempList();
						return err.set(DATACFGERR_INTERFACES_BAD_COVERT, xml_var->GetLineNum(), "incorrect convert type '" + convert_type + "'");
					}
				}

			} else if (name == XmlName::WHITESPACE) {
				std::string strtype = XmlUtils::getAttributeString(xml_var, XmlName::NATIVE, "", XmlUtils::Flags::TOLOWER);
				tlink->VarName = "";
				tlink->m_type  = getTypeByName(strtype);

				if (tlink->m_type == TYPE::UNDEF) {
					clearTempList();
					return err.set(DATACFGERR_INTERFACES_BAD_WS, xml_var->GetLineNum(), "incorrect native type '" + strtype + "'");
				}
			} else {
				clearTempList();
				return err.set(DATACFGERR_INTERFACES_UNKNOW_VAR, xml_var->GetLineNum(), name);
			}

			m_tempLink.push_back(tlink);
		}
	}

	return TRITONN_RESULT_OK;
}


UDINT rModbusTCPSlaveManager::generateVars(rVariableClass* parent)
{
	m_varList.add(m_alias + ".status" , rVariable::Flags::R___, &Live       , U_DIMLESS, 0, "Статус");
	m_varList.add(m_alias + ".tx"     , rVariable::Flags::R___, &Tx         , U_DIMLESS, 0, "Байт передано");
	m_varList.add(m_alias + ".rx"     , rVariable::Flags::R___, &Rx         , U_DIMLESS, 0, "Байт считано");
	m_varList.add(m_alias + ".errorrx", rVariable::Flags::R___, &RxError    , U_DIMLESS, 0, "Количество ошибок");
	m_varList.add(m_alias + ".clients", rVariable::Flags::R___, &ClientCount, U_DIMLESS, 0, "Количество подключенных клиентов");

	if (parent) {
		rVariableClass::linkToExternal(parent);
	}

	return TRITONN_RESULT_OK;
}

UDINT rModbusTCPSlaveManager::generateMarkDown(rGeneratorMD& md)
{
	md.add(this)
			.addProperty(XmlName::ID       , static_cast<UDINT>(SlaveID))
			.addProperty(XmlName::COUNT_ERR, static_cast<UDINT>(MaxError))
			.addProperty(XmlName::SECURITY , Security)
			.addProperty(XmlName::MAXCLIENT, MaxClient)
			.addXml("<" + std::string(XmlName::HOST) + "> " + rGeneratorMD::rItem::XML_OPTIONAL)
			.addXml(XmlName::IP  , "ip address xx.xx.xx.xx", true, "\t")
			.addXml(XmlName::PORT, "port ", true, "\t")
			.addXml("</" + std::string(XmlName::HOST) + ">")
			.addXml("<" + std::string(XmlName::WHITELIST) + "> " + rGeneratorMD::rItem::XML_OPTIONAL)
			.addXml(XmlName::IP, "ip address xx.xx.xx.xx", false, "\t")
			.addXml("\t...")
			.addXml(XmlName::IP, "ip address xx.xx.xx.xx", false, "\t")
			.addXml("</" + std::string(XmlName::WHITELIST) + ">")
			.addXml("<" + std::string(XmlName::SWAP) + "> " + rGeneratorMD::rItem::XML_OPTIONAL)
			.addXml(XmlName::BYTE , static_cast<UDINT>(m_swap.Byte) , false, "\t")
			.addXml(XmlName::WORD , static_cast<UDINT>(m_swap.Word) , false, "\t")
			.addXml(XmlName::DWORD, static_cast<UDINT>(m_swap.DWord), false, "\t")
			.addXml("</" + std::string(XmlName::SWAP) + ">")
			.addXml("<" + std::string(XmlName::ADDRESSMAP) + ">")
			.addXml(String_format("\t<%s begin=\"start address\" lock=\"0\">datablock name</%s>", XmlName::ADDRESSBLOCK, XmlName::ADDRESSBLOCK))
			.addXml("\t...")
			.addXml(String_format("\t<%s begin=\"start address\" lock=\"0\">datablock name</%s>", XmlName::ADDRESSBLOCK, XmlName::ADDRESSBLOCK))
			.addXml("</" + std::string(XmlName::ADDRESSMAP) + ">");

	return TRITONN_RESULT_OK;
}

std::string rModbusTCPSlaveManager::getAdditionalXml() const
{
	return rDataBlock::getXml();
}

void rModbusTCPSlaveManager::clearTempList()
{
	for (auto item : m_tempLink) {
		delete item;
	}

	m_tempLink.clear();
}
