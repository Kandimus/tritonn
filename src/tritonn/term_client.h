/*
 *
 * term_client.h
 *
 * Copyright (c) 2019-2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#pragma once

#include "packet_client.h"

class rUser;

class rTermClient : public rPacketClient
{
public:
	rTermClient(SOCKET socket, sockaddr_in *addr);
	virtual ~rTermClient();

public:
	const rUser *m_user;
};


