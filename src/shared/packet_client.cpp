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
	Buff   = nullptr;
	Size   = 0;
	Marker = 0;
	Length = 0;
}


rPacketClient::rPacketClient(SOCKET socket, sockaddr_in *addr) : rClientTCP(socket, addr)
{
	Buff   = nullptr;
	Size   = 0;
	Marker = 0;
	Length = 0;
}


rPacketClient::~rPacketClient()
{
	ResetRecvBuff(nullptr, 0);
}


void rPacketClient::ResetRecvBuff(USINT *buff, UDINT size)
{
	if(Buff) delete[] Buff;

	Buff   = buff;
	Size   = size;
	Marker = 0;
	Length = 0;
}


USINT *rPacketClient::Recv(USINT *read_buff, UDINT read_size)
{
	// Если буффера еще нет, то это начало пакета
	if(m_buff.empty())
	{
		m_buff.reserve(read_size];
		m_size = read_size;

		clearHeader();
		memcpy(Buff, read_buff, Size);
	}
	else
	{
		USINT *tmp_buff = new USINT[Size + read_size];

		// Получаем общий буффер, состоящий из старого буффера и нового
		memcpy(tmp_buff       , Buff     , Size);
		memcpy(tmp_buff + Size, read_buff, read_size);

		delete[] Buff;
		Buff  = tmp_buff;
		Size += read_size;
	}

	// Заголовок еще не получен, ждем...
	if (Size < sizeof(rPacketHeader)) {
		return nullptr;
	}

	// Если получили заголовок, то заполняем поля пакета
	Marker = *(UDINT *)&Buff[0];
	Length = *(UINT  *)&Buff[4];

	// Проверка, что в буффере содержится одна целая посылка
	if(Size < Length)
	{
		// Все хорошо, ждем когда прийдут остальные данные
		return nullptr;
	}

	return Buff;
}


//
void rPacketClient::PopBuff(UDINT pop_size)
{
	// Удаляем посылку из буффера
	if(pop_size >= Size)
	{
		ResetRecvBuff(nullptr, 0);
	}
	else
	{
		USINT *tmp_buff = new USINT[Size - pop_size];

		memcpy(tmp_buff, Buff + pop_size, Size - pop_size);

		ResetRecvBuff(tmp_buff, Size - pop_size);
	}
}

void rPacketClient::clearHeader()
{
	m_header.m_magic    = 0;
	m_header.m_dataSize = 0;
}
