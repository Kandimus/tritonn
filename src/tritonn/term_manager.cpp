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
#include "stringex.h"



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

	if (tclient->getHeader().m_magic != TT::DataMagic &&
		tclient->getHeader().m_magic != TT::LoginMagic) {

		unsigned char *ip = (unsigned char *)&client->Addr.sin_addr.s_addr;

		TRACEW(LogMask, "Client [%i.%i.%i.%i] send unknow packet (magic 0x%X, length %u).",
			   ip[0], ip[1], ip[2], ip[3], tclient->getHeader().m_magic, tclient->getHeader().m_dataSize);

		return 0; // Все плохо, пришла не понятная нам посылка, нужно отключение клиента
	}

	switch(tclient->getHeader().m_magic) {
		case TT::LoginMagic: result = PacketLogin(tclient); break;
		case TT::DataMagic:  result = PacketData (tclient); break;
		default:             result = 0; break;
	}

	tclient->clearPacket();

	return result;
}


//-------------------------------------------------------------------------------------------------
//
bool rTermManager::PacketLogin(rTermClient* client)
{
	if (!client) {
		TRACEW(LogMask, "Client is NULL!");
		return false;
	}

	//NOTE может не отключать пользователя?
	if(nullptr != client->User) {
		TRACEW(LogMask, "Re-authorization. Client disconnected.");
		return false;
	}

	TT::LoginMsg msg = deserialize_LoginMsg(client->getBuff());

	if (!isCorrectLoginMsg(msg)) {
		TRACEW(LogMask, "Login message deserialize is fault.");
		return false;
	}

	USINT user_hash[MAX_HASH_SIZE] = {0};
	USINT pwd_hash [MAX_HASH_SIZE] = {0};

	String_ToBuffer(msg.user().c_str(), user_hash, MAX_HASH_SIZE);
	String_ToBuffer(msg.pwd().c_str() , pwd_hash , MAX_HASH_SIZE);

	rUser::LoginResult result;
	client->User = rUser::LoginHash(user_hash, pwd_hash, result);
	if (!client->User) {
		TRACEW(LogMask, "Unknow authorization. Cilent disconnected.");
		return false;
	}

	if (result != rUser::LoginResult::SUCCESS && result != rUser::LoginResult::CHANGEPWD ) {
		TRACEW(LogMask, "Fault authorization. Cilent disconnected.");
		return false;
	} else if(client->User->GetAccess() & ACCESS_SA) {
		TRACEW(LogMask, "SA is authorized.");
	}
	else if(client->User->GetAccess() & ACCESS_ADMIN) {
		TRACEW(LogMask, "Administrator is authorized.");
	} else {
		TRACEW(LogMask, "Authorization failed. Cilent disconnected.");
		return false;
	}

	TT::LoginMsg answe;

	answe.set_result(static_cast<UDINT>(result));
	answe.set_access(client->User->GetAccess());

	client->send(answe);

	sendDefaultMessage(client);

//	rSystemVariable::instance().getVersion   (answe.Data.Version);
//	rSystemVariable::instance().getState     (answe.Data.State);
//	rSystemVariable::instance().getConfigInfo(answe.Data.Config);
	return true;
}


//-------------------------------------------------------------------------------------------------
//
bool rTermManager::PacketData(rTermClient* client)
{
	if (!client) {
		TRACEW(LogMask, "Client is NULL!");
		return false;
	}

	if (!client->User) {
		TRACEW(LogMask, "Client is not authorized. Client disconnected.");
		return false;
	}

	TT::DataMsg msg = deserialize_DataMsg(client->getBuff());

	if (!isCorrectDataMsg(msg)) {
		TRACEW(LogMask, "Data message deserialize is fault.");
		return false;
	}

	TT::DataMsg answe;

	if (msg.has_userdata()) {
		answe.set_userdata(msg.userdata());
	}

	if (isWriteDataMsg(msg)) {
		rSnapshot ss(rDataManager::instance().getVariableClass(), client->User->GetAccess());

		for (auto ii = 0; ii < msg.write_size(); ++ii) {
			auto item       = msg.write(ii);
			auto answe_item = answe.add_write();

			answe_item->set_name(item.name());

			ss.add(item.name(), item.has_value() ? item.value() : "");
		}

		ss.set();

		for (auto ii = 0; ii < msg.write_size(); ++ii) {
			auto answe_item = answe.mutable_write(ii);

			answe_item->set_result(ss[ii]->getStatus());

			answe_item->set_value(ss[ii]->getValueString());
		}
	}

	if (isReadDataMsg(msg)) {
		rSnapshot ss(rDataManager::instance().getVariableClass(), client->User->GetAccess());

		for (auto ii = 0; ii < msg.read_size(); ++ii) {
			auto item       = msg.read(ii);
			auto answe_item = answe.add_read();

			answe_item->set_name(item.name());

			ss.add(item.name());
		}

		ss.get();

		for (auto ii = 0; ii < msg.read_size(); ++ii) {
			auto answe_item = answe.mutable_read(ii);

			answe_item->set_result(ss[ii]->getStatus());
			answe_item->set_value(ss[ii]->getValueString());
		}
	}

	if (isNeedConfigInfoDataMsg(msg)) {
		addConfInfo(answe);
	}

	if (isNeedVersionDataMsg(msg)) {
		addVersion(answe);
	}

	rState state;
	rSystemVariable::instance().getState(state);

	answe.mutable_state()->set_eventalarm(state.m_eventAlarm);
	answe.mutable_state()->set_issimulate(state.m_isSimulate);
	answe.mutable_state()->set_live(state.m_live);
	answe.mutable_state()->set_startreason(state.m_startReason);

	client->send(answe);

	return 1;
}

void rTermManager::sendDefaultMessage(rTermClient* client)
{
	TT::DataMsg msg;

	addConfInfo(msg);
	addVersion(msg);

	client->send(msg);
}

void rTermManager::addConfInfo(TT::DataMsg& msg)
{
	rConfigInfo ci;

	rSystemVariable::instance().getConfigInfo(ci);

	msg.mutable_confinfo()->set_developer(ci.m_developer);
	msg.mutable_confinfo()->set_filename(ci.m_filename);
	msg.mutable_confinfo()->set_hash(ci.m_hash);
	msg.mutable_confinfo()->set_name(ci.m_name);
	msg.mutable_confinfo()->set_version(ci.m_version);
}

void rTermManager::addVersion(TT::DataMsg& msg)
{
	rVersion ver;

	rSystemVariable::instance().getVersion(ver);

	msg.mutable_version()->set_build(ver.m_build);
	msg.mutable_version()->set_hash(ver.m_hash);
	msg.mutable_version()->set_major(ver.m_major);
	msg.mutable_version()->set_minor(ver.m_minor);
}

