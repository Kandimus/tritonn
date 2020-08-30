//=================================================================================================
//===
//=== modbustcpslave_manager.h
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

#pragma once

#include <vector>
#include "tinyxml2.h"
#include "tcp_class.h"
#include "data_interface.h"
#include "data_snapshot.h"

using std::vector;


class rVariable;
class rModbusTCPSlaveClient;

struct rModbusLink
{
	rSnapshotItem *Item;
	UINT           Address;
};


struct rTempLink
{
	string VarName;
	UDINT  Address;
	UDINT  LineNum;
};


struct rModbusSwap
{
	USINT Byte;
	USINT Word;
	USINT DWord;
	USINT Reserv;
};


class rModbusTCPSlaveManager: public rInterface, public rTCPClass
{
public:
	rModbusTCPSlaveManager();
	virtual ~rModbusTCPSlaveManager();


// Наследование от rTCPClass
protected:
	virtual UDINT       Proccesing(void);
	virtual rClientTCP *NewClient (SOCKET socket, sockaddr_in *addr);
	virtual UDINT       ClientRecv(rClientTCP *client, USINT *buff, UDINT size);

// Наследование от rInterface
public:
	virtual UDINT LoadFromXML(tinyxml2::XMLElement *xml_root, rDataConfig &cfg);
	virtual UDINT GenerateVars(vector<rVariable *> &list);
	virtual UDINT CheckVars(rDataConfig &cfg);
	virtual UDINT StartServer();
	virtual rThreadClass *GetThreadClass();

protected:
	rSnapshot   Snapshot;
	UINT*       Modbus = nullptr;
	string      Name;            // Имя объекта, для переменных
	USINT       SlaveID;
	UDINT       Security;        // Флаг использования ScurityModbus
	USINT       Live;            // Текущий статус менеджера. Выделена в отдельную переменную, что бы можно было засунуть ее в rDataManager::lock
	USINT       MaxError;        // Максимальное кол-во ошибочных пакетов, при достижении этой цифры клиент будет отключен
	USINT       RxError;         // Текущее кол-во принятых ошибочных пакетов
	UDINT       Tx;              // Кол-во отосланных пакетов
	UDINT       Rx;              // Кол-во принятых пакетов
	UDINT       ClientCount;     // Кол-во текущих клиентов. Выделена в отдельную переменную, что бы можно было засунуть ее в rDataManager::lock

	rModbusSwap Swap;

	vector<rModbusLink> ModbusLink;
	vector<rTempLink>   TempLink;

public:
	tinyxml2::XMLElement *FindBlock(tinyxml2::XMLElement *xml_blocks, const string &name);

protected:
	void Func_0x03 (rModbusTCPSlaveClient *client);
	void Func_0x06 (rModbusTCPSlaveClient *client);
	void Func_0x10 (rModbusTCPSlaveClient *client);
	void Func_Error(rModbusTCPSlaveClient *client, USINT err);

	void SwapBuffer(void *value, UDINT size);

	rSnapshotItem *FindSnapshotItem(UINT addr);

	UDINT TypeCountReg(TT_TYPE type);


	UDINT LoadStandartModbus();
};




