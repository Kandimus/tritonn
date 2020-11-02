//=================================================================================================
//===
//=== term_client.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Клиент для класса-потока терминала
//===
//=================================================================================================

#include <string.h>
#include "packet_login.h"
#include "term_client.h"



rTermClient::rTermClient(SOCKET socket, sockaddr_in *addr) : rPacketClient(socket, addr)
{
	User = nullptr;
}


rTermClient::~rTermClient()
{
	User = nullptr;
}


