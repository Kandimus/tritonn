/*
 *
 * term_client.cpp
 *
 * Copyright (c) 2019-2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#include <string.h>
#include "term_client.h"

rTermClient::rTermClient(SOCKET socket, sockaddr_in *addr) : rPacketClient(socket, addr)
{
	m_user = nullptr;
}

rTermClient::~rTermClient()
{
	m_user = nullptr;
}


