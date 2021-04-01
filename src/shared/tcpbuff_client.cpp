//=================================================================================================
//===
//=== tcpbuff_client.cpp
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#include <string.h>
#include "tcpbuff_client.h"

rTCPBufferClient::rTCPBufferClient() :
	rClientTCP()
{
	m_data = nullptr;
	m_data = 0;
}


rTCPBufferClient::rTCPBufferClient(SOCKET socket, sockaddr_in *addr) :
	rClientTCP(socket, addr)
{
	m_data = nullptr;
	m_len  = 0;
}


rTCPBufferClient::~rTCPBufferClient()
{
	dropBuffer();
}


USINT *rTCPBufferClient::Recv(USINT *read_buff, UDINT read_size)
{
	if (read_size) {
		resizeBuffer(read_buff, read_size);
	} else {
		return CLIENT_RECV_ERROR;
	}

	return m_data;
}


void rTCPBufferClient::resizeBuffer(USINT* buff, UDINT size)
{
	USINT *tmp = new USINT[m_len + size];

	if (m_data) {
		memcpy(tmp, m_data, m_len);
	}

	memcpy(tmp + m_len, buff, size);

	if (m_data) {
		delete[] m_data;
	}

	m_data = tmp;
	m_len += size;
}


void rTCPBufferClient::dropBuffer()
{
	if (m_data) {
		delete[] m_data;
		m_data = nullptr;
	}

	m_len = 0;
}


void rTCPBufferClient::popFrontBuffer(UDINT size)
{
	if (!m_data) {
		m_len = 0;
		return;
	}

	if (size > m_len) {
		size = m_len;
	}

	if (m_len == size) {
		dropBuffer();
		return;
	}

	USINT* tmp = new USINT[m_len - size];

	memcpy(tmp, m_data + size, m_len - size);

	delete[] m_data;

	m_data = tmp;
	m_len -= size;
}

