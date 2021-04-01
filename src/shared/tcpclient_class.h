//=================================================================================================
//===
//=== tcpclient_class.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Базовый класс TCP-клиента для класса-потока TCP
//===
//=================================================================================================

#pragma once

//#include <netinet/in.h>
#include "../shared/def.h"
#include "../shared/thread_class.h"
#include "../shared/tcp_client.h"
#include "tickcount.h"


class rTCPClientClass : public rThreadClass
{
public:
	enum Setup : UINT
	{
		NORECONNECT = 0x0001,
	};

	rTCPClientClass(rClientTCP &client);
	virtual ~rTCPClientClass() = default;

	UDINT IsConnected();
	UDINT Disconnect();
	UDINT Send(void *packet, UDINT size); // Отправка данных серверу

public:
	virtual UDINT Connect(const string &ip, UINT port);
	virtual UDINT RecvFromServer(USINT *buff, UDINT size) = 0; // Call-back на получение данных от клиента

public:
	 rSafityValue<UDINT> ReconnetTime;
	 rSafityValue<UINT>  m_setup;

protected:
	virtual rThreadStatus Proccesing();

	UDINT ReadFromServer(void);

	rClientTCP *Client;
	std::string m_IP;
	UINT        m_port;
	rSafityValue<UDINT> Connected;
	rTickCount  m_timerKeepAlive;

private:
   UDINT Destroy();
};


