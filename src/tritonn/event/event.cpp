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
	m_magic   = Packet::MAGIC;
	m_version = 1;

	clear();
}


rEvent::rEvent(DINT eid)
{
	m_magic   = Packet::MAGIC;
	m_version = 1;

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
void* rEvent::getParamByID(UDINT ID, TYPE& type) const
{
	UDINT pos   = 0;
	UDINT curid = 0;
	
	while(pos < m_size)
	{
		if(ID == curid)
		{
			type = static_cast<TYPE>(m_data[pos]);
			return (void *)(m_data + pos + 1);
		}
		
		pos += 1 + getTypeSize(static_cast<TYPE>(m_data[pos]));
		++curid;
	}
	
	type = TYPE::UNDEF;
	return nullptr;
}

std::string rEvent::toString() const
{
	std::string result = m_timestamp.toString();

	result += String_format("\t%08x\t%02x\t", m_EID, m_size);
	result += String_FromBuffer(m_data, DATA_SIZE);

	return result;
}

bool rEvent::addSTR(STRID val)
{
	if (m_size + getTypeSize(TYPE::STRID) >= DATA_SIZE) {
		return false;
	}

	m_data[m_size++] = static_cast<unsigned char>(TYPE::STRID);
	UDINT vv = val.toUDINT();
	memcpy(m_data + m_size, &vv, getTypeSize(TYPE::STRID));
	m_size += getTypeSize(TYPE::STRID);
	return true;
}

Container& operator << (Container& cnt, const rEvent& event)
{
	return event.toContainer(cnt);
}

Container& rEvent::toContainer(Container& cnt) const
{
	cnt << m_magic << m_version << m_size << m_timestamp << m_EID;

	return cnt.add(m_data, DATA_SIZE);
}

Container& operator >> (Container& cnt, rEvent& event)
{
	return event.fromContainer(cnt);
}

Container& rEvent::fromContainer(Container& cnt)
{
	cnt >> m_magic >> m_version >> m_size >> m_timestamp >> m_EID;
	return cnt.get(m_data, DATA_SIZE);
}
