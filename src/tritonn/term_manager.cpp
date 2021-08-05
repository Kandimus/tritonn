//=================================================================================================
//===
//=== term_manager.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс-поток для общения с терминалом ttterm по TCP
//===
//=================================================================================================

#include <string.h>
#include "log_manager.h"
#include "data_manager.h"
#include "system_variable.h"
#include "users.h"
#include "data_snapshot_item.h"
#include "data_snapshot.h"
#include "login_proto.h"
#include "data_proto.h"
#include "packet_get.h"
#include "packet_getanswe.h"
#include "packet_login.h"
#include "packet_loginanswe.h"
#include "packet_set.h"
#include "packet_setanswe.h"
#include "term_client.h"
#include "term_manager.h"



extern rVariable *gVariable;


///////////////////////////////////////////////////////////////////////////////////////////////////
//
// КОНСТРУКТОРЫ И ДЕСТРУКТОР
rTermManager::rTermManager()
	: rTCPClass("0.0.0.0", LanPort::PORT_TERM, 1)
{
	RTTI     = "rTermManager";
	LogMask |= LOG::TERMINAL;
}


rTermManager::~rTermManager()
{
	;
}


//-------------------------------------------------------------------------------------------------
//
rThreadStatus rTermManager::Proccesing()
{
	while(1)
	{
		// Обработка команд нити
		rThreadStatus thread_status = rTCPClass::Proccesing();
		if(!THREAD_IS_WORK(thread_status))
		{
			return thread_status;
		}

		rThreadClass::EndProccesing();
	}

	return rThreadStatus::UNDEF;
}


rClientTCP *rTermManager::NewClient(SOCKET socket, sockaddr_in *addr)
{
	return (rClientTCP *)new rTermClient(socket, addr);
}


UDINT rTermManager::ClientRecv(rClientTCP *client, USINT *buff, UDINT size)
{
	auto  tclient = dynamic_cast<rTermClient*>(client);
	auto  data    = tclient->Recv(buff, size);
	UDINT result  = 0;

	if (TERMCLNT_RECV_ERROR == data) {
		return 0;
	}

   // Посылку считали не полностью
	if (nullptr == data) {
		return 1;
	}

	if (tclient->m_header.m_magic != TTT::DataMagic &&
		tclient->m_header.m_magic != TTT::LoginMagic) {

		unsigned char *ip = (unsigned char *)&client->Addr.sin_addr.s_addr;

		TRACEW(LogMask, "Client [%i.%i.%i.%i] send unknow packet", ip[0], ip[1], ip[2], ip[3]);

		return 0; // Все плохо, пришла не понятная нам посылка, нужно отключение клиента
	}

	// Получаем посылку
//	switch(tclient->Marker)
//	{
//		case MARKER_PACKET_LOGIN: result = PacketLogin(tclient, (rPacketLoginData *)data); break;
//		case MARKER_PACKET_SET:   result = PacketSet  (tclient, (rPacketSetData   *)data); break;
//		case MARKER_PACKET_GET:   result = PacketGet  (tclient, (rPacketGetData   *)data); break;

//		default: result = 0; break; // Как мы тут оказались не понятно ))) Но клиента отключим
//	}

	// Удаляем из буффера приема клиента посылку
	tclient->clearPacket();

	return result;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rTermManager::PacketLogin(rTermClient *client, rPacketLoginData *packet)
{
	UDINT result = 0;

	if(nullptr != client->User)
	{
		TRACEW(LogMask, "Re-authorization. Client disconnected.");
		return 0;
	}

	// Проверка на соответствие логина и пароля уровню SA
	client->User = rUser::LoginHash(packet->UserName, packet->UserPwd, result);

	if(nullptr == client->User)
	{
		TRACEW(LogMask, "Unknow authorization. Cilent disconnected.");
		return 0;
	}

	if(LOGIN_OK != result)
	{
		TRACEW(LogMask, "Fault authorization. Cilent disconnected.");
		return 0;
	}
	else if(client->User->GetAccess() & ACCESS_SA)
	{
		TRACEW(LogMask, "SA is authorized.");
	}
	else if(client->User->GetAccess() & ACCESS_ADMIN)
	{
		TRACEW(LogMask, "Administrator is authorized.");
	}
	else
	{
		TRACEW(LogMask, "Authorization failed. Cilent disconnected.");
		return 0;
	}

	rPacketLoginAnswe answe;

	answe.Data.Access  = client->User->GetAccess();
	rSystemVariable::instance().getVersion   (answe.Data.Version);
	rSystemVariable::instance().getState     (answe.Data.State);
	rSystemVariable::instance().getConfigInfo(answe.Data.Config);

	Send(client, &answe.Data, LENGTH_PACKET_LOGINANSWE);

	return 1;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rTermManager::PacketSet(rTermClient *client, rPacketSetData *packet)
{
	if(nullptr == client->User)
	{
		TRACEW(LogMask, "Client is not authorized. Client disconnected.");
		return 0;
	}

//	TRACEW(LogMask, "Packet Set %s", packet->Name[0]);

	rPacketSetAnswe answe;
	rSnapshot       ss(rDataManager::instance().getVariableClass(), client->User->GetAccess());

	answe.Data.UserData = packet->UserData;
	answe.Data.Count    = packet->Count;

	for(UDINT ii = 0; ii < packet->Count; ++ii)
	{
		string      name   = packet->Name[ii];
		string      val    = packet->Value[ii];

		ss.add(name, val);

		// Копируем имя переменной из входящего пакета в исходящий
		memcpy(answe.Data.Name[ii], packet->Name[ii], MAX_VARIABLE_LENGTH);
	}

	ss.set();

	for(UDINT ii = 0; ii < packet->Count; ++ii)
	{

		answe.Data.Result[ii] = ss[ii]->getStatus();

		strncpy(answe.Data.Value[ii], ss[ii]->getValueString().c_str(), MAX_VARVALUE_LENGTH);
	}

	Send(client, &answe.Data, LENGTH_PACKET_SETANSWE);

	return 1;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rTermManager::PacketGet(rTermClient *client, rPacketGetData *packet)
{
	if(nullptr == client->User)
	{
		TRACEW(LogMask, "Client is not authorized. Client disconnected.");
		return 0;
	}

	//TRACEW(LogMask, "Packet Get.");

	rSnapshot       ss(rDataManager::instance().getVariableClass(), client->User->GetAccess());
	rPacketGetAnswe answe;

	answe.Data.UserData = packet->UserData;
	answe.Data.Count    = packet->Count;

	for(UDINT ii = 0; ii < packet->Count; ++ii)
	{
		memcpy(answe.Data.Name[ii], packet->Name[ii], MAX_VARIABLE_LENGTH);

		ss.add(packet->Name[ii]);
	}

	ss.get();

	for(UDINT ii = 0; ii < packet->Count; ++ii)
	{
		strncpy(answe.Data.Value[ii], ss[ii]->getValueString().c_str(), MAX_VARVALUE_LENGTH);
		answe.Data.Result[ii] = ss[ii]->getStatus();
	}

	Send(client, &answe.Data, LENGTH_PACKET_GETANSWE);

	return 1;
}




