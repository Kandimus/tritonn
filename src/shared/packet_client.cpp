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
#include "log_manager.h"
#include "stringex.h"

rPacketClient::rPacketClient() : rClientTCP()
{
	clearPacket();
}


rPacketClient::rPacketClient(SOCKET socket, sockaddr_in *addr) : rClientTCP(socket, addr)
{
	clearPacket();
}

UDINT rPacketClient::send(const TT::DataMsg& message)
{
	rPacketHeader hdr;
	std::vector<USINT> arr = serialize_DataMsg(message);

	hdr.m_magic    = TT::DataMagic;
	hdr.m_reserved = 0;
	hdr.m_flags    = 0;
	hdr.m_version  = 0x0100;
	hdr.m_dataSize = arr.size();
	hdr.m_crc32    = m_crc.get(&hdr, sizeof(hdr) - 4);

	Send(&hdr, sizeof(rPacketHeader));
	Send(arr.data(), arr.size());

	//TRACEI(LOG::PACKET, "DataMsg -> [%s %s]", String_FromBuffer((const unsigned char*)&hdr, sizeof(hdr)).c_str(), String_FromBuffer(arr.data(), arr.size()).c_str());
	return 0;
}

UDINT rPacketClient::send(const TT::LoginMsg& message)
{
	rPacketHeader hdr;
	std::vector<USINT> arr = serialize_LoginMsg(message);

	hdr.m_magic    = TT::LoginMagic;
	hdr.m_reserved = 0;
	hdr.m_flags    = 0;
	hdr.m_version  = 0x0100;
	hdr.m_dataSize = arr.size();
	hdr.m_crc32    = m_crc.get(&hdr, sizeof(hdr) - 4);

	Send(&hdr, sizeof(rPacketHeader));
	Send(arr.data(), arr.size());

//TRACEI(LOG::PACKET, "LoginMsg -> [%s %s]", String_FromBuffer((const unsigned char*)&hdr, sizeof(hdr)).c_str(), String_FromBuffer(arr.data(), arr.size()).c_str());
	return 0;
}


USINT *rPacketClient::Recv(USINT *read_buff, UDINT read_size)
{
	if (m_buff.empty()) {
		m_buff.reserve(4 * 1024 * 1024);
		m_buff.resize(read_size);

		clearHeader();
		memcpy(m_buff.data(), read_buff, read_size);
	} else {
		UDINT pos = m_buff.size();

		m_buff.resize(m_buff.size() + read_size);

		memcpy(m_buff.data() + pos, read_buff, read_size);
	}

	return checkBuffer();
}

USINT* rPacketClient::checkBuffer()
{
	if (!m_header.m_magic || !m_header.m_dataSize) {
		if (m_buff.size() < sizeof(rPacketHeader)) {
			return nullptr;
		}

		m_header = *(rPacketHeader*)m_buff.data();
		m_buff.erase(m_buff.begin(), m_buff.begin() + sizeof(rPacketHeader));

		UDINT crc32 = m_crc.get(&m_header, sizeof(m_header) - 4);

		if (crc32 != m_header.m_crc32) {
			TRACEA(LOG::PACKET, "Bad CRC in packet %08X", m_header.m_magic);

			clearHeader();
			m_buff.clear();
			return TERMCLNT_RECV_ERROR;
		}
	}

	return m_buff.size() < m_header.m_dataSize ? nullptr : m_buff.data();
}

//
void rPacketClient::clearPacket()
{
	if (m_header.m_dataSize >= m_buff.size()) {
		m_buff.clear();
	} else {
		m_buff.erase(m_buff.begin(), m_buff.begin() + m_header.m_dataSize - 1);
	}
	clearHeader();
//TRACEI(LOG::PACKET, "clear. Buffer [%s]", String_FromBuffer(m_buff.data(), m_buff.size()).c_str());
}

void rPacketClient::clearHeader()
{
	m_header.m_magic    = 0;
	m_header.m_dataSize = 0;
}
