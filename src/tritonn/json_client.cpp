//=================================================================================================
//===
//=== json_client.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Клиент для класса-потока json
//===
//=================================================================================================

#include <string.h>
#include "log_manager.h"
#include "json_client.h"


rJSONClient::rJSONClient() : rClientTCP()
{
	Buff   = nullptr;
	Size   = 0;
	Packet = nullptr;
}


rJSONClient::rJSONClient(SOCKET socket, sockaddr_in *addr) : rClientTCP(socket, addr)
{
	Buff   = nullptr;
	Size   = 0;
	Packet = nullptr;
}


rJSONClient::~rJSONClient()
{
	if(Buff)   delete[] Buff;
	if(Packet) delete[] Packet;

	Packet = nullptr;
	Buff   = nullptr;
	Size   = 0;
}


//
USINT *rJSONClient::Recv(USINT *read_buff, UDINT read_size)
{
	// Если буффера еще нет, то это начало пакета
	if(!Buff)
	{
		Buff   = new USINT[read_size];
		Size   = read_size;

		memcpy(Buff, read_buff, Size);

		TRACEERROR((const char *)Buff);
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

	// Считаем скобки, что бы понять приняли всю посылку или нет
	UDINT pos = CheckBrace();
	if(pos)
	{
		Packet      = new char[pos + 2];
		memset(Packet, 0, pos + 2);
		memcpy(Packet, Buff, pos + 1);
		Packet[pos + 1] = 0;

		delete[] Buff;

		Size = 0;
		Buff = nullptr;

		return JSONCLNT_RECV_SUCCESS;
	}

	return JSONCLNT_RECV_PROCESS;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rJSONClient::CheckBrace()
{
	char *str        = (char *)Buff;
	UDINT ii         = 0;
	UDINT openbrace  = 0;
	UDINT closebrace = 0;

	for(ii = 0; ii < Size; ++ii)
	{
		if(str[ii] == '{') ++openbrace;
		if(str[ii] == '}') ++closebrace;

		if(openbrace && closebrace && (openbrace == closebrace))
		{
			return ii;
		}
	}

	return 0;
}



