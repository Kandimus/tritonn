//=================================================================================================
//===
//=== log_client.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Клиент для класса-потока логов
//===
//=================================================================================================

#include "log_client.h"


rLogClient::rLogClient(SOCKET socket, sockaddr_in *addr) : rClientTCP(socket, addr)
{
	Login = 0;
}


rLogClient::~rLogClient()
{
	Login = 0;
}


USINT *rLogClient::Recv(USINT */*read_buff*/, UDINT /*read_size*/)
{
	return nullptr;
}
