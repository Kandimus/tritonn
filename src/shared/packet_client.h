/*
 *
 * packet_client.h
 *
 * Copyright (c) 2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#pragma once

#include "tcp_client.h"

#define TERMCLNT_RECV_ERROR   ((USINT *)0xFFFFFFFF)

struct rPacketHeader
{
	UDINT m_magic;
	UINT  m_version;
	UINT  m_flags;
	UDINT m_reserved;
	UDINT m_dataSize;
	UDINT m_crc32;
};

class rPacketClient : public rClientTCP
{
public:
	rPacketClient();
	rPacketClient(SOCKET socket, sockaddr_in *addr);
	virtual ~rPacketClient();

	virtual USINT *Recv(USINT *read_buff, UDINT read_size);

	void PopBuff(UDINT size);

protected:
	void ResetRecvBuff(USINT *buff, UDINT pop_size);

	void clearHeader();

public:
	std::vector<USINT> m_buff;
	UDINT  m_size;

	rPacketHeader m_header;
};


