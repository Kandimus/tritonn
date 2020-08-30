//=================================================================================================
//===
//=== log_manager.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс-поток для выдачи log-сообщений по TCP
//===
//=================================================================================================

//#include <string.h>
#include <ncursesw/ncurses.h>
#include "log_manager.h"
#include "packet_client.h"
#include "display_manager.h"
#include "tritonn_manager.h"
#include "packet_login.h"
#include "packet_loginanswe.h"
#include "packet_get.h"
#include "packet_getanswe.h"
#include "packet_set.h"
#include "packet_setanswe.h"

extern rDisplayManager gDisplayManager;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// КОНСТРУКТОРЫ И ДЕСТРУКТОР
rTritonnManager::rTritonnManager(rClientTCP &client) : rTCPClientClass(client)
{
	RTName   += "::rTritonnManager";
	LogMask  |= 0;

	Setup.Set(TCPCLIENT_SETUP_NORECONNECT);
}


rTritonnManager::~rTritonnManager()
{
	;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rTritonnManager::Connect(const string &ip, UINT port)
{
	UDINT result = rTCPClientClass::Connect(ip, port);

	if(!result)
	{
		gDisplayManager.ShowLogin();
	}

	return result;
}

//-------------------------------------------------------------------------------------------------
//
UDINT rTritonnManager::Proccesing()
{
	UDINT thread_status = 0;

	while(1)
	{
		//printf("+\n");

		// Обработка команд нити
		thread_status = rTCPClientClass::Proccesing();
		if(!THREAD_IS_WORK(thread_status))
		{
			return thread_status;
		}
	}

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rTritonnManager::SendPacketSet(rPacketSetData &data)
{
	rPacketSet packet(data);
	UDINT      result = Send(&packet.Data, packet.Data.Size);

//	printf("Send SET ok (%i)\n", result);

	return result;
}



UDINT rTritonnManager::SendPacketGet(rPacketGetData &data)
{
	rPacketGet packet(data);

	return Send(&packet.Data, packet.Data.Size);
}



//-------------------------------------------------------------------------------------------------
//
UDINT rTritonnManager::RecvFromServer(USINT *buff, UDINT size)
{
	rPacketClient *client = (rPacketClient *)Client;
	USINT         *data   = client->Recv(buff, size);
	UDINT          result = 0;

//	printf("recv size %i\n", size);

	if(TERMCLNT_RECV_ERROR == data)
	{
		return 0;
	}

	// Посылку считали не полностью
	if(nullptr == data)
	{
		return 1;
	}

	// Проверка на то, что принимаемая посылка из перечня допустимых
	if((client->Marker == MARKER_PACKET_LOGINANSWE && client->Length == LENGTH_PACKET_LOGINANSWE) ||
		(client->Marker == MARKER_PACKET_SETANSWE   && client->Length == LENGTH_PACKET_SETANSWE  ) ||
		(client->Marker == MARKER_PACKET_GETANSWE   && client->Length == LENGTH_PACKET_GETANSWE  ))
	{
		//printf("recv packet %i, lenght %i\n", client->Marker, client->Length);
		;
	}
	else
	{
		TRACEW(LogMask, "Server send unknow packet. Marker %#X, length %u", client->Marker, client->Length);

		return 0; // Все плохо, пришла не понятная нам посылка, нужно отключение клиента
	}

	// Получаем посылку
	switch(client->Marker)
	{
		case MARKER_PACKET_LOGINANSWE: result = PacketLoginAnswe((rPacketLoginAnsweData *)data); break;
		case MARKER_PACKET_SETANSWE  : result = PacketSetAnswe  ((rPacketSetAnsweData   *)data); break;
		case MARKER_PACKET_GETANSWE  : result = PacketGetAnswe  ((rPacketGetAnsweData   *)data); break;

		default:	result = 0; // Как мы тут оказались не понятно ))) Но клиента отключим
	}

	client->PopBuff(client->Length);

	return result;
}


UDINT rTritonnManager::PacketLoginAnswe(rPacketLoginAnsweData *data)
{
	if(!data->Access)
	{
		TRACEA(LogMask, "Unknow user name or password.");
		return 0;
	}

	Access = data->Access;
	gDisplayManager.CallbackLoginAnswe(data);

	TRACEI(LogMask, "Login is OK.");

	return 1;
}


//-------------------------------------------------------------------------------------------------
UDINT rTritonnManager::PacketSetAnswe(rPacketSetAnsweData *data)
{
	if(!Access)
	{
		TRACEA(LogMask, "Receive packet, but not authorized.");
		return 0;
	}

//	printf("Set answe\n");
	gDisplayManager.CallbackSetAnswe(data);

	return 1;
}


//-------------------------------------------------------------------------------------------------
UDINT rTritonnManager::PacketGetAnswe(rPacketGetAnsweData *data)
{
	if(!Access)
	{
		TRACEA(LogMask, "Receive packet 'get', but not authorized.");
		return 0;
	}

	gDisplayManager.CallbackGetAnswe(data);

	return 1;
}

