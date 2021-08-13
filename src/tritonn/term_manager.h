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

class rTermClient;

namespace TT {
	class DataMsg;
}

class rTermManager : public rTCPClass
{
	SINGLETON(rTermManager)

// rTCPClass
protected:
	virtual rThreadStatus Proccesing(void);
	virtual rClientTCP*   NewClient (SOCKET socket, sockaddr_in *addr);
	virtual UDINT         ClientRecv(rClientTCP *client, USINT *buff, UDINT size);

protected:
	bool packetLogin(rTermClient* client);
	bool packetData (rTermClient* client);
	void sendDefaultMessage(rTermClient* client);

	void addState(TT::DataMsg& msg);
	void addConfInfo(TT::DataMsg& msg);
	void addVersion(TT::DataMsg& msg);
	void addDateTime(TT::DataMsg& msg);
};




