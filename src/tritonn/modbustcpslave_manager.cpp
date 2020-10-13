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
#include "stringex.h"
#include "hash.h"
#include "log_manager.h"
#include "data_manager.h"
#include "data_variable.h"
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
	: rTCPClass("0.0.0.0", TCP_PORT_MODBUS, MAX_MBTCP_CLIENT)
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
	if(LoadStandartModbus())
	{
		CloseServer();
		Finish();
		//TODO Нужно вызвать HALT
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

		Snapshot.ResetAssign();
		rDataManager::Instance().Get(Snapshot);

		{
			rLocker lock(Mutex); lock.Nop();

			// TODO Нужно разобрать Snapshot и выложить его в Modbus
			for(UDINT ii = 0; ii < ModbusLink.size(); ++ii)
			{
				rModbusLink *link = &ModbusLink[ii];

				if(link->Item->GetStatus() != SS_STATUS_ASSIGN) continue;

				Modbus[link->Address] = 0;
				link->Item->GetBuffer(&Modbus[link->Address]);
				SwapBuffer(&Modbus[link->Address], link->Item->GetSizeVar());
			}
		}

		rThreadClass::EndProccesing();
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

	rSnapshot snapshot;

	snapshot.SetAccess(ACCESS_MASK_SA); //TODO тут нужен анализ на SecurityModbus

	rSnapshotItem *ss = FindSnapshotItem(adr);

	if(nullptr == ss)
	{
		memcpy(&Modbus[adr], &val, 2);
	}
	else
	{
		UDINT countregs = TypeCountReg(ss->GetVariable()->Type);
		UDINT sizeregs  = countregs << 1;

		if(countregs == 1)
		{
			SwapBuffer(&val, sizeregs);

			snapshot.Add(ss->GetVariable()->Name, val);

			rDataManager::Instance().Set(snapshot);
		}
	}

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

	rSnapshot snapshot;

	snapshot.SetAccess(ACCESS_MASK_SA); //TODO тут нужен анализ на SecurityModbus

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

		countregs = TypeCountReg(ss->GetVariable()->Type);
		sizeregs  = countregs << 1;

		if(countregs + ii >= count)
		{
			continue;
		}

		SwapBuffer(values + ii * 2, sizeregs);

		snapshot.Add(ss->GetVariable()->Name, values + ii * 2);

		ii += countregs;
	}

	rDataManager::Instance().Set(snapshot);

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
	for(UDINT ii = 0; ii < ModbusLink.size(); ++ii)
	{
		if(ModbusLink[ii].Address == addr) return ModbusLink[ii].Item;
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
UDINT rModbusTCPSlaveManager::CheckVars(rDataConfig &cfg)
{
	UDINT address = 0;

	for(UDINT ii = 0; ii < TempLink.size(); ++ii)
	{
		rTempLink  *tlink = &TempLink[ii];
		rModbusLink link;

		Snapshot.Add(tlink->VarName);

		if(nullptr == Snapshot.Back()->GetVariable())
		{
			cfg.ErrorLine = tlink->LineNum;
			cfg.ErrorStr  = tlink->VarName;
			cfg.ErrorID   = DATACFGERR_INTERFACES_NF_VAR;

			return cfg.ErrorID;
		}

		if(tlink->Address)
		{
			address = tlink->Address;
		}

		link.Address = address;
		link.Item    = Snapshot.Back();
		address     += TypeCountReg(link.Item->GetVariable()->Type);

		if(address > 0x0000FFFF)
		{
			cfg.ErrorLine = tlink->LineNum;
			cfg.ErrorStr  = tlink->VarName;
			cfg.ErrorID   = DATACFGERR_INTERFACES_ADDROVERFLOW;

			return cfg.ErrorID;
		}

		ModbusLink.push_back(link);
	}

	return TRITONN_RESULT_OK;
}



//-------------------------------------------------------------------------------------------------
// Загрузка СТАНДАРТНОЙ области данных модбас (системной)
UDINT rModbusTCPSlaveManager::LoadStandartModbus()
{
	string filetext = "";
	UDINT  result   = TRITONN_RESULT_OK;

	if(TRITONN_RESULT_OK != (result = SimpleFileLoad(FILE_MODBUS, filetext)))
	{
		return result;
	}

	tinyxml2::XMLDocument  doc;
	tinyxml2::XMLElement  *xml_modbus;

	if(tinyxml2::XML_SUCCESS != doc.Parse(filetext.c_str()))
	{
		return (UDINT)doc.ErrorID();
	}

	xml_modbus = doc.FirstChildElement("modbus");
	if(nullptr == xml_modbus)
	{
		return -1;
	}

	for(tinyxml2::XMLElement *xml_item = xml_modbus->FirstChildElement("item"); xml_item != nullptr; xml_item = xml_item->NextSiblingElement("item"))
	{
		UDINT       err = 0;
		rModbusLink link;

		Snapshot.Add(rDataConfig::GetTextString(xml_item, "", err));

		link.Address = rDataConfig::GetAttributeUDINT(xml_item, "addr", 0xFFFF);
		link.Item    = Snapshot.Back();

		if(link.Address == 0xFFFF || nullptr == link.Item->GetVariable())
		{
			return -2;
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

	xml_blocks = xml_modbus->FirstChildElement(CFGNAME_DATAMAP);

	if(nullptr == xml_blocks) return nullptr;

	for(tinyxml2::XMLElement *xml_item = xml_blocks->FirstChildElement(CFGNAME_DATABLOCK); xml_item != nullptr; xml_item = xml_item->NextSiblingElement(CFGNAME_DATABLOCK))
	{
		const char *item_name = xml_item->Attribute(CFGNAME_NAME);

		if(nullptr == item_name) continue;

		if(String_equali(name, item_name)) return xml_item;
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
UDINT rModbusTCPSlaveManager::LoadFromXML(tinyxml2::XMLElement *xml_root, rDataConfig &cfg)
{
	UDINT  port = 0;
	string ip   = "";

	rInterface::LoadFromXML(xml_root, cfg);

	Alias    = "comms.modbus." + Alias;
	port     = rDataConfig::GetAttributeUDINT (xml_root, CFGNAME_PORT    , TCP_PORT_MODBUS);
	ip       = "0.0.0.0";
	Name     = rDataConfig::GetAttributeString(xml_root, CFGNAME_NAME    , "");
	SlaveID  = rDataConfig::GetAttributeUDINT (xml_root, CFGNAME_ID      , 0);
	Security = rDataConfig::GetAttributeUDINT (xml_root, CFGNAME_SECURITY, 0);
	MaxError = rDataConfig::GetAttributeUDINT (xml_root, CFGNAME_COUNTERR, 3);

	tinyxml2::XMLElement *xml_adrmap = xml_root->FirstChildElement(CFGNAME_ADDRESSMAP);
	tinyxml2::XMLElement *xml_swap   = xml_root->FirstChildElement(CFGNAME_SWAP);
	tinyxml2::XMLElement *xml_wlist  = xml_root->FirstChildElement(CFGNAME_WHITELIST);

	if(nullptr == xml_adrmap)
	{
		cfg.ErrorLine = xml_root->GetLineNum();

		return DATACFGERR_INTERFACES_NF_BLOCKS;
	}

	// Загружаем информацию по Swap
	if(nullptr != xml_swap)
	{
		UDINT err = 0;

		Swap.Byte  = rDataConfig::GetTextUSINT(xml_swap->FirstChildElement(CFGNAME_BYTE ), Swap.Byte , err);
		Swap.Word  = rDataConfig::GetTextUSINT(xml_swap->FirstChildElement(CFGNAME_WORD ), Swap.Word , err);
		Swap.DWord = rDataConfig::GetTextUSINT(xml_swap->FirstChildElement(CFGNAME_DWORD), Swap.DWord, err);
	}

	// Загружаем список "белых" адрессов
	if(nullptr != xml_wlist)
	{
		for(tinyxml2::XMLElement *xml_item = xml_wlist->FirstChildElement(CFGNAME_IP); nullptr != xml_item; xml_item = xml_item->NextSiblingElement(CFGNAME_IP))
		{
			if(xml_item->GetText())
			{
				if(!AddWhiteIP(xml_item->GetText()))
				{
					cfg.ErrorLine = xml_item->GetLineNum();

					return DATACFGERR_INCORRECT_IP;
				}
			}
		}
	}


	// Перебираем указанные блоки
	for(tinyxml2::XMLElement *xml_item = xml_adrmap->FirstChildElement(CFGNAME_ADDRESSBLOCK); nullptr != xml_item; xml_item = xml_item->NextSiblingElement(CFGNAME_ADDRESSBLOCK))
	{
		// Считываем блок модбаса
		UDINT  err       = 0;
		UDINT  address   = rDataConfig::GetAttributeUDINT(xml_item, CFGNAME_BEGIN, 0xFFFFFFFF);
		string blockname = rDataConfig::GetTextString    (xml_item, "", err);

		if(address > 0x0000FFFF)
		{
			cfg.ErrorLine = xml_item->GetLineNum();

			return DATACFGERR_INTERFACES_BADADDR;
		}

		if(err)
		{
			cfg.ErrorLine = xml_item->GetLineNum();

			return DATACFGERR_INTERFACES_BADBLOCK;
		}

		tinyxml2::XMLElement *xml_block = FindBlock(xml_root, blockname);

		if(nullptr == xml_block)
		{
			cfg.ErrorLine = xml_root->GetLineNum();

			return DATACFGERR_INTERFACES_BADBLOCK;
		}

		//
		for(tinyxml2::XMLElement *xml_var = xml_block->FirstChildElement(CFGNAME_VARIABLE); xml_var != nullptr; xml_var = xml_var->NextSiblingElement(CFGNAME_VARIABLE))
		{
			rTempLink tlink;

			err           = 0;
			tlink.Address = address;
			tlink.VarName = rDataConfig::GetTextString(xml_var, "", err);
			tlink.LineNum = xml_var->GetLineNum();
			address       = 0;

			if(err)
			{
				cfg.ErrorLine = tlink.LineNum;

				return DATACFGERR_INTERFACES_BADVAR;
			}

			TempLink.push_back(tlink);
		}
	}

	SetServerIP(ip, port);

	return TRITONN_RESULT_OK;
}


UDINT rModbusTCPSlaveManager::GenerateVars(vector<rVariable *> &list)
{
	list.push_back(new rVariable(Alias + ".status" , TYPE_UINT , VARF_R___, &Live       , U_DIMLESS, 0));
	list.push_back(new rVariable(Alias + ".tx"     , TYPE_UDINT, VARF_R___, &Tx         , U_DIMLESS, 0));
	list.push_back(new rVariable(Alias + ".rx"     , TYPE_UDINT, VARF_R___, &Rx         , U_DIMLESS, 0));
	list.push_back(new rVariable(Alias + ".errorrx", TYPE_USINT, VARF_R___, &RxError    , U_DIMLESS, 0));
	list.push_back(new rVariable(Alias + ".clients", TYPE_USINT, VARF_R___, &ClientCount, U_DIMLESS, 0));

	return TRITONN_RESULT_OK;
}


/*
UDINT rModbusTCPSlaveManager::SaveKernel(FILE *file, const string &objname, const string &comment)
{
	return TRITONN_RESULT_OK;
}
*/
