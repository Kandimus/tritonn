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
#include <ncurses.h>
#include "log_manager.h"
#include "packet_client.h"
#include "display_manager.h"
#include "tritonn_manager.h"
#include "tritonn_manager.h"
#include "login_proto.h"
#include "data_proto.h"
#include "../tritonn/users.h"
#include "stringex.h"

extern rDisplayManager gDisplayManager;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// КОНСТРУКТОРЫ И ДЕСТРУКТОР
rTritonnManager::rTritonnManager(rClientTCP &client) : rTCPClientClass(client)
{
	RTTI    += "::rTritonnManager";
	LogMask |= 0;

	m_setup.Set(rTCPClientClass::Setup::NORECONNECT);
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
rThreadStatus rTritonnManager::Proccesing()
{
	rThreadStatus thread_status = rThreadStatus::UNDEF;

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

	return rThreadStatus::CLOSED;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rTritonnManager::sendDataMsg(TT::DataMsg& msg)
{
	rPacketClient* client = dynamic_cast<rPacketClient*>(Client);
	return client->send(msg);
}

UDINT rTritonnManager::sendLoginMsg(TT::LoginMsg& msg)
{
	rPacketClient* client = dynamic_cast<rPacketClient*>(Client);
	return client->send(msg);
}



//-------------------------------------------------------------------------------------------------
//
UDINT rTritonnManager::RecvFromServer(USINT *buff, UDINT size)
{
	auto client = dynamic_cast<rPacketClient*>(Client);
	auto data   = client->Recv(buff, size);

	do {
		if (TERMCLNT_RECV_ERROR == data) {
			return 1;
		}

		// Посылку считали не полностью
		if(!data) {
			return 0;
		}

		if (client->getHeader().m_magic != TT::DataMagic &&
			client->getHeader().m_magic != TT::LoginMagic) {

			TRACEW(LogMask, "Server send unknow packet (magic 0x%X, length %u).", client->getHeader().m_magic, client->getHeader().m_dataSize);

			return 2;
		}

		bool result  = 0;
		switch (client->getHeader().m_magic) {
			case TT::LoginMagic: result = PacketLogin(client); break;
			case TT::DataMagic:  result = PacketData(client); break;
			default: return true;
		}

		if (!result) {
			return result;
		}

		client->clearPacket();

		data = client->checkBuffer();
	} while (true);

	return false;
}


bool rTritonnManager::PacketLogin(rPacketClient* client)
{
	if (!client) {
		TRACEW(LogMask, "Client is NULL!");
		return false;
	}

	TT::LoginMsg msg = deserialize_LoginMsg(client->getBuff(), client->getHeader().m_dataSize);

	if (!isCorrectLoginMsg(msg) || !msg.has_result() || !msg.has_access()) {
		TRACEW(LogMask, "Login message deserialize is fault. reason %i%i%i", !isCorrectLoginMsg(msg), !msg.has_result(), !msg.has_access());
		return false;
	}

	UDINT result = msg.result();
	if (static_cast<rUser::LoginResult>(result) != rUser::LoginResult::SUCCESS &&
		static_cast<rUser::LoginResult>(result) != rUser::LoginResult::CHANGEPWD) {
		TRACEA(LogMask, "Unknow user name or password. Result %i", result);
		return false;
	}

//TRACEI(LogMask, "LoginMsg <- [%s]", String_FromBuffer(client->getBuff().data(), client->getHeader().m_dataSize).c_str());
	Access = msg.access();
	gDisplayManager.CallbackLogin(&msg);

	TRACEI(LogMask, "Login is OK. Access %08X", Access);

	return true;
}


//-------------------------------------------------------------------------------------------------
bool rTritonnManager::PacketData(rPacketClient* client)
{
	if (!client) {
		TRACEW(LogMask, "Client is NULL!");
		return false;
	}

	if(!Access) {
		TRACEA(LogMask, "Receive packet, but not authorized.");
		return false;
	}

	TT::DataMsg msg = deserialize_DataMsg(client->getBuff(), client->getHeader().m_dataSize);

	if (!isCorrectDataMsg(msg)) {
		TRACEW(LogMask, "Data message deserialize is fault.");
		return false;
	}

	gDisplayManager.CallbackData(&msg);

	return true;
}
