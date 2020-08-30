//=================================================================================================
//===
//=== tcp_client.cpp
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

#include <unistd.h>
#include "tcp_client.h"


rClientTCP::rClientTCP()
{
	Socket  = SOCKET_ERROR;
}


rClientTCP::rClientTCP(SOCKET socket, sockaddr_in *addr)
{
	Socket  = socket;
	Addr    = *addr;
}


rClientTCP::~rClientTCP()
{
	if(SOCKET_ERROR != Socket)
	{
		shutdown(Socket, SHUT_RDWR);
		close(Socket);
		Socket = SOCKET_ERROR;
	}
}


//
UDINT rClientTCP::Send(void *packet, UDINT size)
{
	UDINT  sendbytes = 0;
	UDINT  result    = 0;
	USINT *buff      = (USINT *)packet;

	do
	{
		result = send(Socket, buff + sendbytes, size - sendbytes, 0);

		if(result <= 0)
		{
			return result;
		}
		
		sendbytes += result;
	}
	while(sendbytes < size);

	return result;
}


void rClientTCP::Disconnect()
{
	shutdown(Socket, SHUT_RDWR);
	close(Socket);

	Socket = SOCKET_ERROR;
}
