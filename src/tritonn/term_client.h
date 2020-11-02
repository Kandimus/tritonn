//=================================================================================================
//===
//=== term_client.h
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

#pragma once

#include "packet_client.h"

class rUser;

class rTermClient : public rPacketClient
{
public:
	rTermClient(SOCKET socket, sockaddr_in *addr);
	virtual ~rTermClient();

public:
	const rUser *User; // Пользователь
};


