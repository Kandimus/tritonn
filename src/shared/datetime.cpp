//=================================================================================================
//===
//=== datetime.cpp
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#include "datetime.h"
#include "stringex.h"

rDateTime::rDateTime()
{
	setCurTime();
}

rDateTime::rDateTime(const Time64_T& time64)
{
	m_datetime.tv_sec  = time64;
	m_datetime.tv_usec = 0;
}

rDateTime::rDateTime(const UDT& udt)
{
	m_datetime = udt;
}

rDateTime::rDateTime(const STM& stm)
{
	STM _stm = stm;

	if (_stm.tm_year > 1900) {
		_stm.tm_mon  = _stm.tm_mon - 1;
		_stm.tm_year = _stm.tm_year - 1900;
	}

	m_datetime.tv_sec  = mktime(&_stm);
	m_datetime.tv_usec = 0;
}

std::string rDateTime::toString() const
{
	tm dt;

	localtime_r(&m_datetime.tv_sec, &dt);

	return String_format("%02i.%02i.%04i %02i:%02i:%02i.%03li",
						 dt.tm_mday, dt.tm_mon + 1, dt.tm_year + 1900,
						 dt.tm_hour, dt.tm_min, dt.tm_sec, m_datetime.tv_usec / 1000);
}

void rDateTime::setCurTime()
{
	gettimeofday(&m_datetime, NULL);
}

void rDateTime::clear()
{
	m_datetime.tv_sec  = 0;
	m_datetime.tv_usec = 0;
}

Container& operator << (Container& cnt, const rDateTime& event)
{
	return event.toContainer(cnt);
}

Container& rDateTime::toContainer(Container& cnt) const
{
	cnt << m_datetime.tv_sec << m_datetime.tv_usec;

	return cnt;
}

Container& operator >> (Container& cnt, rDateTime& event)
{
	return event.fromContainer(cnt);
}

Container& rDateTime::fromContainer(Container& cnt)
{
	cnt >> m_datetime.tv_sec >> m_datetime.tv_usec;
	return cnt;
}
