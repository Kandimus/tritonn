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
	hdr.m_crc32    = m_crc.get(&hdr, sizeof(hdr) - sizeof(hdr.m_crc32));

//	arr2.resize(sizeof(hdr));
//	memcpy(arr2.data(), &hdr, sizeof(hdr));
//	arr2 += arr;

	Send(&hdr, sizeof(rPacketHeader));
	Send(arr.data(), arr.size());
//	Send(arr2.data(), arr2.size());

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
	hdr.m_crc32    = m_crc.get(&hdr, sizeof(hdr) - sizeof(hdr.m_crc32));

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

		UDINT crc32 = m_crc.get(&m_header, sizeof(m_header) - 4);

		if (crc32 != m_header.m_crc32) {
			TRACEA(LOG::PACKET, "Bad CRC in packet %08X, size %i, [%s]", m_header.m_magic, m_buff.size(), String_FromBuffer(m_buff.data(), m_buff.size()).c_str());

			clearHeader();
			m_buff.clear();
			return TERMCLNT_RECV_ERROR;
		}

		m_buff.erase(m_buff.begin(), m_buff.begin() + sizeof(rPacketHeader));
	}

	return m_buff.size() < m_header.m_dataSize ? nullptr : m_buff.data();
}

//
void rPacketClient::clearPacket()
{
	if (m_header.m_dataSize >= m_buff.size()) {
		m_buff.clear();
	} else {
		m_buff.erase(m_buff.begin(), m_buff.begin() + m_header.m_dataSize);
	}
	clearHeader();
//TRACEI(LOG::PACKET, "clear. Buffer [%s]", String_FromBuffer(m_buff.data(), m_buff.size()).c_str());
}

void rPacketClient::clearHeader()
{
	m_header.m_magic    = 0;
	m_header.m_dataSize = 0;
}

bool rPacketClient::serialize_Header(rPacketHeader& hdr, std::vector<USINT>& arr)
{
	if (!hdr.m_dataSize || !hdr.m_magic) {
		return false;
	}

	int pos = arr.size();
	arr.resize(arr.size() + sizeof(hdr));
	memcpy(arr.data() + pos, &hdr, sizeof(hdr));

	return true;
}

std::vector<USINT> rPacketClient::getPacket()
{
	std::vector<USINT> result;

	if (!m_header.m_magic || !m_header.m_dataSize || m_buff.size() < m_header.m_dataSize) {
		return result;
	}

	result.resize(m_header.m_dataSize);
	memcpy(result.data(), m_buff.data(), m_header.m_dataSize);

	return result;
}
