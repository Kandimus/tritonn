//=================================================================================================
//===
//=== event_manager.h
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#pragma once

#include "tcpclient_class.h"
#include "tcpbuff_client.h"
#include "singlenton.h"

class rEventManager : public rTCPClientClass
{
	SINGLETON(rEventManager)

public:
	virtual UDINT Connect(const string &ip, UINT port) override;

protected:
	virtual rThreadStatus Proccesing(void) override;
	virtual UDINT RecvFromServer(USINT *buff, UDINT size) override;

private:
	rTCPBufferClient m_client;
};




