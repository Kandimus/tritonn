//=================================================================================================
//===
//=== tritonn_manager.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс-поток для общения с Tritonn
//===
//=================================================================================================

#pragma once

#include <list>
#include "tcpclient_class.h"

class rPacketClient;

namespace TT {
	class LoginMsg;
	class DataMsg;
}

class rTritonnManager : public rTCPClientClass
{
public:
	rTritonnManager(rClientTCP &client);
	virtual ~rTritonnManager();

	virtual UDINT Connect(const string &ip, UINT port);

	UDINT sendDataMsg(TT::DataMsg& msg);
	UDINT sendLoginMsg(TT::LoginMsg& msg);

protected:
	virtual rThreadStatus Proccesing(void);
	virtual UDINT RecvFromServer(USINT *buff, UDINT size);

	string GetPSGARError(USINT err);

	bool PacketLogin(rPacketClient* client);
	bool PacketData (rPacketClient* client);

protected:
	UDINT Access;
};




