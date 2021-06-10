//=================================================================================================
//===
//=== modbustcpslave_manager.h
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

#pragma once

#include <vector>
#include "tcp_class.h"
#include "interface.h"
#include "../data_snapshot.h"


class rSnapshotItem;
class rModbusTCPSlaveClient;

struct rModbusLink
{
	rSnapshotItem* m_item    = nullptr;
	UINT           m_address = 0;
	TYPE           m_convert = TYPE::UNDEF;
};


struct rTempLink
{
	std::string VarName;
	UDINT       Address;
	UDINT       LineNum;
	TYPE        m_type;
};


struct rModbusSwap
{
	USINT Byte  = 1;
	USINT Word  = 0;
	USINT DWord = 0;
	USINT Reserv;
};


class rModbusTCPSlaveManager: public rTCPClass, public rInterface
{
public:
	rModbusTCPSlaveManager();
	virtual ~rModbusTCPSlaveManager();


// Наследование от rTCPClass
protected:
	virtual rThreadStatus Proccesing(void) override;
	virtual rClientTCP*   NewClient (SOCKET socket, sockaddr_in *addr) override;
	virtual UDINT         ClientRecv(rClientTCP *client, USINT *buff, UDINT size) override;

// Наследование от rInterface
public:
	virtual const char*   getRTTI() override { return "tcpslave"; }
	virtual UDINT         loadFromXML(tinyxml2::XMLElement* xml_root, rError& err) override;
	virtual UDINT         generateMarkDown(rGeneratorMD& md) override;
	virtual std::string   getAdditionalXml() const override;
	virtual UDINT         generateVars(rVariableClass* parent) override;
	virtual UDINT         checkVars(rError& err) override;
	virtual UDINT         startServer() override;
	virtual rThreadClass* getThreadClass() override;

protected:
	void Func_0x03 (rModbusTCPSlaveClient *client);
	void Func_0x06 (rModbusTCPSlaveClient *client);
	void Func_0x10 (rModbusTCPSlaveClient *client);
	void Func_Error(rModbusTCPSlaveClient *client, USINT err);

	void SwapBuffer(void *value, UDINT size);

	rSnapshotItem *FindSnapshotItem(UINT addr);

	UDINT TypeCountReg(TYPE type);

	UDINT LoadStandartModbus(rError& err);

	void  clearTempList();

protected:
	rSnapshot   m_snapshot;
	UINT*       Modbus = nullptr;
	std::string Name;            // Имя объекта, для переменных
	USINT       SlaveID = 0;
	UDINT       Security;        // Флаг использования ScurityModbus
	USINT       Live;            // Текущий статус менеджера. Выделена в отдельную переменную, что бы можно было засунуть ее в rDataManager::lock
	USINT       MaxError = 3;    // Максимальное кол-во ошибочных пакетов, при достижении этой цифры клиент будет отключен
	USINT       RxError;         // Текущее кол-во принятых ошибочных пакетов
	UDINT       Tx;              // Кол-во отосланных пакетов
	UDINT       Rx;              // Кол-во принятых пакетов
	UDINT       ClientCount;     // Кол-во текущих клиентов. Выделена в отдельную переменную, что бы можно было засунуть ее в rDataManager::lock

	rModbusSwap m_swap;

	vector<rModbusLink> ModbusLink;
	vector<rTempLink*>  m_tempLink;

public:
};




