//=================================================================================================
//===
//=== event_client.h
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#pragma once

#include "tcp_client.h"


class rEventClient : public rClientTCP
{
public:
	rEventClient(SOCKET socket, sockaddr_in* addr);
	virtual ~rEventClient();

	virtual USINT *Recv(USINT* read_buff, UDINT read_size);

public:
	UDINT Login;  // Флаг, что клиент прошел идентификацию, и от него можно принимать команды
};


