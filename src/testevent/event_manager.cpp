//=================================================================================================
//===
//=== event_manager.cpp
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

//#include <string.h>
#include "event_manager.h"
#include "container.h"
#include "../tritonn/event/event.h"

rEventManager::rEventManager() :
	rTCPClientClass(m_client)
{
	RTTI     = "rEventManager";
	LogMask |= 0;

	ReconnetTime.Set(2000);
//	Setup.Set(rTCPClientClass::Setup::NORECONNECT);
}


rEventManager::~rEventManager()
{
	Disconnect();
}


//-------------------------------------------------------------------------------------------------
//
UDINT rEventManager::Connect(const string &ip, UINT port)
{
	UDINT result = rTCPClientClass::Connect(ip, port);

	if(!result)
	{
		printf("connected to %s:%u\n", ip.c_str(), port);
	}

	return result;
}

//-------------------------------------------------------------------------------------------------
//
rThreadStatus rEventManager::Proccesing()
{
	rThreadStatus thread_status = rThreadStatus::UNDEF;

	while (1) {
		// Обработка команд нити
		thread_status = rTCPClientClass::Proccesing();
		if(!THREAD_IS_WORK(thread_status))
		{
			return thread_status;
		}
	}

	return rThreadStatus::CLOSED;
}

UDINT rEventManager::RecvFromServer(USINT *buff, UDINT size)
{
	USINT* data = m_client.Recv(buff, size);

	if (!m_client.getSize()) {
		return TRITONN_RESULT_OK;
	}

	Container cnt(m_client.getData(), m_client.getSize());
	rEvent    event;

	while (!cnt.isEOF()) {
		UDINT pos = cnt.getReadPos();

		cnt >> event;

		if (cnt.isEOF()) {
			cnt.setReadPos(pos);
			break;
		}

		if (event.getMagic() != rEvent::Packet::MAGIC) {
			printf("recv bad paket\n");

			m_client.dropBuffer();
			return TRITONN_RESULT_OK;

		} else {
			auto text = event.toString();
			printf("%s\n", text.c_str());
		}
	}

	m_client.popFrontBuffer(cnt.getReadPos());

	return TRITONN_RESULT_OK;
}
