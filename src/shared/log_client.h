//=================================================================================================
//===
//=== log_client.h
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

#pragma once

#include "tcp_client.h"


class rLogClient : public rClientTCP
{
public:
	rLogClient(SOCKET socket, sockaddr_in *addr);
	virtual ~rLogClient();

	virtual USINT *Recv(USINT *read_buff, UDINT read_size);

public:
	UDINT Login;  // Флаг, что клиент прошел идентификацию, и от него можно принимать команды
};


