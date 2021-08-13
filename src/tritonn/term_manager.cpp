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
#include "term_client.h"
#include "term_manager.h"
#include "stringex.h"


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
	auto tclient = dynamic_cast<rTermClient*>(client);
	auto data    = tclient->Recv(buff, size);

	do {
		if (TERMCLNT_RECV_ERROR == data) {
			return 0;
		}

		if (!data) {
			return 1;
		}

		if (tclient->getHeader().m_magic != TT::DataMagic &&
			tclient->getHeader().m_magic != TT::LoginMagic) {

			unsigned char *ip = (unsigned char *)&client->Addr.sin_addr.s_addr;

			TRACEW(LogMask, "Client [%i.%i.%i.%i] send unknow packet (magic 0x%X, length %u).",
				   ip[0], ip[1], ip[2], ip[3], tclient->getHeader().m_magic, tclient->getHeader().m_dataSize);

			return 0; // Все плохо, пришла не понятная нам посылка, нужно отключение клиента
		}

		bool result  = 0;
		switch(tclient->getHeader().m_magic) {
			case TT::LoginMagic: result = packetLogin(tclient); break;
			case TT::DataMagic:  result = packetData (tclient); break;
			default:             return false;
		}

		if (!result) {
			return result;
		}

		tclient->clearPacket();

		data = tclient->checkBuffer();
	} while (true);

	return true;
}


//-------------------------------------------------------------------------------------------------
//
bool rTermManager::packetLogin(rTermClient* client)
{
	if (!client) {
		TRACEW(LogMask, "Client is NULL!");
		return false;
	}

	if(client->m_user) {
		TRACEW(LogMask, "Re-authorization. Client disconnected.");
		return false;
	}

	TT::LoginMsg msg = deserialize_LoginMsg(client->getBuff(), client->getHeader().m_dataSize);

	if (!isCorrectLoginMsg(msg)) {
		TRACEW(LogMask, "Login message deserialize is fault.");
		return false;
	}

	USINT user_hash[MAX_HASH_SIZE] = {0};
	USINT pwd_hash [MAX_HASH_SIZE] = {0};

	String_ToBuffer(msg.user().c_str(), user_hash, MAX_HASH_SIZE);
	String_ToBuffer(msg.pwd().c_str() , pwd_hash , MAX_HASH_SIZE);

	rUser::LoginResult result;
	client->m_user = rUser::LoginHash(user_hash, pwd_hash, result);
	if (!client->m_user) {
		TRACEW(LogMask, "Unknow authorization. Cilent disconnected.");
		return false;
	}

	if (result != rUser::LoginResult::SUCCESS && result != rUser::LoginResult::CHANGEPWD ) {
		TRACEW(LogMask, "Fault authorization. Cilent disconnected.");
		return false;

	} else if(client->m_user->GetAccess() & ACCESS_SA) {
		TRACEW(LogMask, "SA is authorized.");

	} else if(client->m_user->GetAccess() & ACCESS_ADMIN) {
		TRACEW(LogMask, "Administrator is authorized.");

	} else {
		TRACEW(LogMask, "Authorization failed. Cilent disconnected.");
		return false;
	}

	TT::LoginMsg answe;

	answe.set_result(static_cast<UDINT>(result));
	answe.set_access(client->m_user->GetAccess());

	client->send(answe);

	sendDefaultMessage(client);
	return true;
}


//-------------------------------------------------------------------------------------------------
//
bool rTermManager::packetData(rTermClient* client)
{
	if (!client) {
		TRACEW(LogMask, "Client is NULL!");
		return false;
	}

	if (!client->m_user) {
		TRACEW(LogMask, "Client is not authorized. Client disconnected.");
		return false;
	}

	TT::DataMsg msg = deserialize_DataMsg(client->getBuff(), client->getHeader().m_dataSize);

	if (!isCorrectDataMsg(msg)) {
		TRACEW(LogMask, "Data message deserialize is fault.");
		return false;
	}

	TT::DataMsg answe;

	if (msg.has_userdata()) {
		answe.set_userdata(msg.userdata());
	}

	if (isWriteDataMsg(msg)) {
		rSnapshot ss(rDataManager::instance().getVariableClass(), client->m_user->GetAccess());

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
		rSnapshot ss(rDataManager::instance().getVariableClass(), client->m_user->GetAccess());

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

	if (isNeedDateTimeDataMsg(msg)) {
		addDateTime(answe);
	}

	if (isUploadConfDataMsg(msg)) {
		TRACEI(LOG::PACKET, "Upload config is not implemented!");
	}

	addState(answe);

	client->send(answe);

	return 1;
}

void rTermManager::sendDefaultMessage(rTermClient* client)
{
	TT::DataMsg msg;

	addConfInfo(msg);
	addVersion(msg);
	addState(msg);

	client->send(msg);
}

void rTermManager::addState(TT::DataMsg& msg)
{
	rState state;
	rSystemVariable::instance().getState(state);

	msg.mutable_state()->set_eventalarm(state.m_eventAlarm);
	msg.mutable_state()->set_issimulate(state.m_isSimulate);
	msg.mutable_state()->set_live(state.m_live);
	msg.mutable_state()->set_startreason(state.m_startReason);
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

void rTermManager::addDateTime(TT::DataMsg& msg)
{
	tm sdt;

	rSystemVariable::instance().getTime(sdt);

	msg.mutable_datetime()->set_year (sdt.tm_year + 1900);
	msg.mutable_datetime()->set_month(sdt.tm_mon + 1);
	msg.mutable_datetime()->set_day  (sdt.tm_mday);
	msg.mutable_datetime()->set_hour (sdt.tm_hour);
	msg.mutable_datetime()->set_min  (sdt.tm_min);
	msg.mutable_datetime()->set_sec  (sdt.tm_sec);
}
