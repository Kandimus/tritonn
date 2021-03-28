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

#include "event.h"
#include <string.h>
#include "stringex.h"


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

	m_timestamp = event.m_timestamp;
//	gettimeofday(&m_timestamp, NULL);

	memcpy(m_data, event.m_data, DATA_SIZE);

	return *this;
}

//-------------------------------------------------------------------------------------------------
//
void rEvent::clear()
{
	m_size = 0;
	m_EID  = EID_UNDEF;

	memset(m_data, 0, DATA_SIZE);
	m_timestamp.clear();
}

rEvent& rEvent::reinit(UDINT eid)
{
	m_EID  = eid;
	m_size = 0;

	m_timestamp.setCurTime();
	memset(m_data, 0, DATA_SIZE);

	return *this;
}


//-------------------------------------------------------------------------------------------------
void* rEvent::getParamByID(UDINT ID, UDINT& type) const
{
	UDINT pos   = 0;
	UDINT curid = 0;
	
	while(pos < m_size)
	{
		if(ID == curid)
		{
			type = m_data[pos];
			return (void *)(m_data + pos + 1);
		}
		
		pos += 1 + EPT_SIZE[m_data[pos]];
		++curid;
	}
	
	type = TYPE_UNDEF;
	return nullptr;
}

std::string rEvent::toString() const
{
	std::string result = m_timestamp.toString();

	result += String_format("\t%08x\t%02x\t", m_EID, m_size);
	result += String_FromBuffer(m_data, DATA_SIZE);

	return result;
}
