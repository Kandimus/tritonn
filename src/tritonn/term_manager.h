//=================================================================================================
//===
//=== term_manager.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс-поток для общения с терминалом ttterm по TCP
//===
//=================================================================================================

#pragma once

#include "tcp_class.h"
#include "variable_class.h"
#include "singlenton.h"

class  rTermClient;
struct rPacketLoginData;
struct rPacketSetData;
struct rPacketGetData;

namespace TT {
	class DataMsg;
}

class rTermManager : public rTCPClass//, public rVariableClass
{
	SINGLETON(rTermManager)

protected:
	virtual rThreadStatus Proccesing(void);
	virtual rClientTCP*   NewClient (SOCKET socket, sockaddr_in *addr);
	virtual UDINT         ClientRecv(rClientTCP *client, USINT *buff, UDINT size);

	bool PacketLogin(rTermClient* client);
	bool PacketData (rTermClient* client);
	void sendDefaultMessage(rTermClient* client);

	void addConfInfo(TT::DataMsg& msg);
	void addVersion(TT::DataMsg& msg);
};




