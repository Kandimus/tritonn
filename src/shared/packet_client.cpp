//=================================================================================================
//===
//=== packet_client.cpp
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

#include <string.h>
#include "packet_client.h"


rPacketClient::rPacketClient() : rClientTCP()
{
	clearPacket();
}


rPacketClient::rPacketClient(SOCKET socket, sockaddr_in *addr) : rClientTCP(socket, addr)
{
	clearPacket();
}

UDINT rPacketClient::send(const TTT::DataMsg& message)
{
	rPacketHeader hdr;
	std::vector<USINT> arr = serialize_DataMsg(message);

	hdr.m_magic    = TTT::DataMagic;
	hdr.m_reserved = 0;
	hdr.m_flags    = 0;
	hdr.m_version  = 0x0100;
	hdr.m_dataSize = arr.size();
	hdr.m_crc32    = 0;
//	hdr.m_crc32    =

	Send(&hdr, sizeof(rPacketHeader));
	Send(arr.data(), arr.size());

	return 0;
}

UDINT rPacketClient::send(const TTT::LoginMsg& message)
{
	rPacketHeader hdr;
	std::vector<USINT> arr = serialize_LoginMsg(message);

	hdr.m_magic    = TTT::LoginMagic;
	hdr.m_reserved = 0;
	hdr.m_flags    = 0;
	hdr.m_version  = 0x0100;
	hdr.m_dataSize = arr.size();
	hdr.m_crc32    = 0;
//	hdr.m_crc32    =

	Send(&hdr, sizeof(rPacketHeader));
	Send(arr.data(), arr.size());

	return 0;
}


USINT *rPacketClient::Recv(USINT *read_buff, UDINT read_size)
{
	// Если буффера еще нет, то это начало пакета
	if(m_buff.empty())
	{
		m_buff.reserve(4 * 1024 * 1024);
		m_buff.resize(read_size);

		clearHeader();
		memcpy(m_buff.data(), read_buff, read_size);
	}
	else
	{
		UDINT pos = m_buff.size();

		m_buff.resize(m_buff.size() + read_size);

		memcpy(m_buff.data() + pos, read_buff, read_size);
	}

	if (!m_header.m_magic || !m_header.m_dataSize) {
		if (m_buff.size() < sizeof(rPacketHeader)) {
			return nullptr;
		}

		m_header = *(rPacketHeader*)m_buff.data();
		m_buff.erase(m_buff.begin(), m_buff.begin() + sizeof(rPacketHeader));
	}

	return m_buff.size() < m_header.m_dataSize ? nullptr : m_buff.data();
}


//
void rPacketClient::clearPacket()
{
	// Удаляем посылку из буффера
	if (m_header.m_dataSize >= m_buff.size()) {
		m_buff.clear();
	} else {
		m_buff.erase(m_buff.begin(), m_buff.begin() + m_header.m_dataSize);
	}
	clearHeader();
}

void rPacketClient::clearHeader()
{
	m_header.m_magic    = 0;
	m_header.m_dataSize = 0;
}
