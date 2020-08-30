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


const UDINT TCPCLIENT_SETUP_NORECONNECT = 0x00000001;


class rTCPClientClass : public rThreadClass
{
public:
	rTCPClientClass(rClientTCP &client);
	virtual ~rTCPClientClass();

	virtual UDINT Connect(const string &ip, UINT port);
	UDINT IsConnected();
	UDINT Disconnect();

	// Call-back на получение данных от клиента
	virtual UDINT RecvFromServer(USINT *buff, UDINT size) = 0;
	
	// Отправка данных серверу
	UDINT Send(void *packet, UDINT size);

public:
	 rSafityValue<UDINT> ReconnetTime;
	 rSafityValue<UDINT> Setup;


protected:
	virtual UDINT Proccesing();

	UDINT ReadFromServer(void);

	rClientTCP *Client;
	string      IP;
	UINT        Port;
	rSafityValue<UDINT> Connected;

private:
   UDINT Destroy();
};


