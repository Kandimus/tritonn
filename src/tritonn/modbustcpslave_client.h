//=================================================================================================
//===
//=== modbustcp_client.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Клиент для класса-потока modbustcp
//===
//=================================================================================================

#pragma once

#include <string>
#include "tcp_client.h"

#define MODBUSTCP_RECV_ERROR   ((USINT *)0xFFFFFFFF)
#define MODBUSTCP_RECV_SUCCESS ((USINT *)0x00000001)
#define MODBUSTCP_RECV_PROCESS ((USINT *)0x00000000)


using std::string;

struct rModbusTCPHeader
{
	UINT  TransactionID;
	UINT  ProtocolID;
	UINT  Length;
};


class rModbusTCPSlaveClient : public rClientTCP
{
public:
	rModbusTCPSlaveClient();
	rModbusTCPSlaveClient(SOCKET socket, sockaddr_in *addr);
	virtual ~rModbusTCPSlaveClient();

	// Наследование от rClientTCP
	virtual USINT *Recv(USINT *read_buff, UDINT read_size);
	virtual UDINT Send(void *packet, UDINT size);

public:
	static UINT SwapUINT(UINT val);
	static void SwapUINTPtr(void *val, int countbyte);

protected:

public:
	rModbusTCPHeader Header;
	UDINT            HeaderRead;
	USINT           *Buff;
	UDINT            Size;
	char            *Packet;
};




