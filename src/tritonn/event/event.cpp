//=================================================================================================
//===
//=== event/event.cpp
//===
//=== Copyright (c) 2019-2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс для регистрации событий, как системных, так и пользователя.
//===
//=================================================================================================

#include <string.h>
#include "event.h"


rEvent::rEvent(void)
{
	clear();
}


rEvent::rEvent(DINT eid)
{
	reinit(eid);
}

rEvent::~rEvent()
{
	m_EID  = EID_UNDEF;
	m_size = 0;
}

rEvent &rEvent::operator = (const rEvent &event)
{
	if (&event == this) {
		return *this;
	}

	m_EID  = event.m_EID;
	m_size = event.m_size;

	m_timeStamp = event.m_timeStamp;
//	gettimeofday(&m_timestamp, NULL);

	memcpy(m_data, event.m_data, DATA_SIZE);

	return *this;
}

//-------------------------------------------------------------------------------------------------
//
void rEvent::clear()
{
	m_timestamp.tv_sec  = 0;
	m_timestamp.tv_usec = 0;
	m_size              = 0;
	m_EID               = EID_UNDEF;

	memset(m_data, 0, DATA_SIZE);
}

rEvent& rEvent::reinit(UDINT eid)
{
	m_EID  = eid;
	m_size = 0;

	gettimeofday(&m_timestamp, NULL);
	memset(Data, 0, DATA_SIZE);

	return *this;
}


//-------------------------------------------------------------------------------------------------
void* rEvent::getParamByID(UDINT ID, UDINT &type)
{
	UDINT pos   = 0;
	UDINT curid = 0;
	
	while(pos < m_size)
	{
		if(ID == curid)
		{
			type = Data[pos];
			return (void *)(Data + pos + 1);
		}
		
		pos += 1 + EPT_SIZE[Data[pos]];
		++curid;
	}
	
	type = TYPE_UNDEF;
	return nullptr;
}
