//=================================================================================================
//===
//=== tcp_client.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Базовый класс клиента для класса-потока TCP
//===
//=================================================================================================

#pragma once

#include <netinet/in.h>
#include "def.h"


class rClientTCP
{
public:
   rClientTCP();
	rClientTCP(SOCKET socket, sockaddr_in *addr);
	virtual ~rClientTCP();

	virtual USINT *Recv(USINT *read_buff, UDINT read_size) = 0;
	virtual UDINT Send(void *packet, UDINT size);

	void  Disconnect();

public:
	SOCKET      Socket;   //
	sockaddr_in Addr;     //
};


