//=================================================================================================
//===
//=== modbustcpslave_client.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Клиент для класса-потока modbustcpslave
//===
//=================================================================================================

#include <string.h>
#include "log_manager.h"
#include "modbustcpslave_client.h"


rModbusTCPSlaveClient::rModbusTCPSlaveClient() : rClientTCP()
{
	Buff       = nullptr;
	Size       = 0;
	Packet     = nullptr;
	HeaderRead = 0;
}


rModbusTCPSlaveClient::rModbusTCPSlaveClient(SOCKET socket, sockaddr_in *addr) : rClientTCP(socket, addr)
{
	Buff       = nullptr;
	Size       = 0;
	Packet     = nullptr;
	HeaderRead = 0;
}


rModbusTCPSlaveClient::~rModbusTCPSlaveClient()
{
	if(Buff)   delete[] Buff;
	if(Packet) delete[] Packet;

	Packet     = nullptr;
	Buff       = nullptr;
	Size       = 0;
	HeaderRead = 0;
}


//
USINT *rModbusTCPSlaveClient::Recv(USINT *read_buff, UDINT read_size)
{
	if(HeaderRead >= 6 && Size == Header.Length)
	{
		if(Buff) delete[] Buff;

		HeaderRead = 0;
		Size       = 0;
		Buff       = nullptr;
	}

	// Если заголовок считан не полностью, то считываем заголовок полностью
	if(HeaderRead < 6)
	{
		UDINT need_read = (read_size <= 6) ? read_size : (6 - HeaderRead);

		memcpy((USINT *)&Header + HeaderRead, read_buff, need_read);
		HeaderRead += need_read;
		read_buff  += need_read;
		read_size  -= need_read;

		SwapUINTPtr(&Header.Length, sizeof(Header.Length));

		Buff = new USINT[Header.Length]; // TODO Может сделать статическим
		Size = 0;
	}

	// Если заголовок считан успешно, то считываем данные
	if(HeaderRead >= 6 && read_size)
	{
		UDINT readlen = (read_size >= Header.Length - Size) ? Header.Length - Size : read_size;

		memcpy(Buff + Size, read_buff, readlen);
		Size += readlen;
	}

	return (HeaderRead >= 6 && Size == Header.Length) ? MODBUSTCP_RECV_SUCCESS : MODBUSTCP_RECV_PROCESS;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rModbusTCPSlaveClient::Send(void *packet, UDINT size)
{
	USINT buf[262];

	((UINT *)buf)[0] = Header.TransactionID;
	((UINT *)buf)[1] = Header.ProtocolID;
	((UINT *)buf)[2] = SwapUINT(size);

	memcpy(buf + 6, packet, size);

	return rClientTCP::Send(buf, size + 6);
}


UINT rModbusTCPSlaveClient::SwapUINT(UINT val)
{
	USINT *arr = (USINT *)&val;

	return arr[1] + (arr[0] << 8);
}


// Функция переворачивает байты в модбасе.
// Реализована такая упоротая функция в связи с тем, что Risk-процессоры не могут работать с адресом не кратным 4 байтам.
void rModbusTCPSlaveClient::SwapUINTPtr(void *val, int countbyte)
{
	USINT *arr = (USINT *)val;
	USINT  tmp;

	while(countbyte)
	{
		tmp        = arr[0];
		arr[0]     = arr[1];
		arr[1]     = tmp;
		arr       += 2;
		countbyte -= 2;
	}
}


