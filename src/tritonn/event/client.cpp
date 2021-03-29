//=================================================================================================
//===
//=== event_client.cpp
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#include "client.h"


rEventClient::rEventClient(SOCKET socket, sockaddr_in* addr) : rClientTCP(socket, addr)
{
	Login = 0;
}


rEventClient::~rEventClient()
{
	Login = 0;
}


USINT *rEventClient::Recv(USINT */*read_buff*/, UDINT /*read_size*/)
{
	return nullptr;
}
