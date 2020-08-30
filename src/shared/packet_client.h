//=================================================================================================
//===
//=== packet_client.h
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

#pragma once

#include "tcp_client.h"

#define TERMCLNT_RECV_ERROR   ((USINT *)0xFFFFFFFF)


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

public:
	USINT *Buff;
	UDINT  Size;
	UDINT  Marker;
	UINT   Length;
};


