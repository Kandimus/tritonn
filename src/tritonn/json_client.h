//=================================================================================================
//===
//=== json_client.h
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

#pragma once

#include <string>
#include "tcp_client.h"

#define JSONCLNT_RECV_ERROR   ((USINT *)0xFFFFFFFF)
#define JSONCLNT_RECV_SUCCESS ((USINT *)0x00000001)
#define JSONCLNT_RECV_PROCESS ((USINT *)0x00000000)



using std::string;


class rJSONClient : public rClientTCP
{
public:
	rJSONClient();
	rJSONClient(SOCKET socket, sockaddr_in *addr);
	virtual ~rJSONClient();

	// Наследование от rClientTCP
	virtual USINT *Recv(USINT *read_buff, UDINT read_size);

protected:
	UDINT CheckBrace();

public:
	USINT *Buff;
	UDINT  Size;
	char  *Packet;
};


