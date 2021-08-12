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
#include "data_proto.h"
#include "login_proto.h"
#include "crc32.h"

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
	virtual ~rPacketClient() = default;

	virtual USINT* Recv(USINT *read_buff, UDINT read_size) override;
	virtual UDINT send(const TT::DataMsg& message);
	virtual UDINT send(const TT::LoginMsg& message);
	virtual USINT* checkBuffer();

	void clearPacket();
	const std::vector<USINT>& getBuff()   { return m_buff;   }
	const rPacketHeader&      getHeader() { return m_header; }

protected:
	void clearHeader();

protected:
	std::vector<USINT> m_buff;
	rPacketHeader      m_header;
	rCRC32             m_crc;
};


