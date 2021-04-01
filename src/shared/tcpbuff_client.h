//=================================================================================================
//===
//=== tcpbuff_client.h
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#pragma once

#include "tcp_client.h"

#define CLIENT_RECV_ERROR   ((USINT*)0xFFFFFFFF)

class rTCPBufferClient : public rClientTCP
{
public:
	rTCPBufferClient();
	rTCPBufferClient(SOCKET socket, sockaddr_in *addr);
	virtual ~rTCPBufferClient();

	virtual USINT *Recv(USINT *read_buff, UDINT read_size);

	USINT* getData() const { return m_data; }
	UDINT  getSize() const { return m_len;  }

	void resizeBuffer(USINT* buff, UDINT size);
	void popFrontBuffer(UDINT len);
	void dropBuffer();

protected:
	USINT* m_data;
	UDINT  m_len;
};
