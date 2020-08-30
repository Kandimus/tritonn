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


struct rPacketLoginAnsweData;
struct rPacketGetData;
struct rPacketGetAnsweData;
struct rPacketSetData;
struct rPacketSetAnsweData;


class rTritonnManager : public rTCPClientClass
{
public:
	rTritonnManager(rClientTCP &client);
	virtual ~rTritonnManager();

	virtual UDINT Connect(const string &ip, UINT port);

	UDINT SendPacketSet(rPacketSetData &data);
	UDINT SendPacketGet(rPacketGetData &data);

protected:
	virtual UDINT Proccesing(void);
	virtual UDINT RecvFromServer(USINT *buff, UDINT size);

	string GetPSGARError(USINT err);

	UDINT PacketLoginAnswe(rPacketLoginAnsweData *data);
	UDINT PacketSetAnswe  (rPacketSetAnsweData   *data);
	UDINT PacketGetAnswe  (rPacketGetAnsweData   *data);
	
public:

protected:
	UDINT Access;
};




