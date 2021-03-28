//=================================================================================================
//===
//=== tickcount.cpp
//===
//=== Copyright (c) 2003-2013 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс аналогичный GetTickCount из Win.
//===
//=================================================================================================

#include "tickcount.h"


rTickCount::rTickCount()
{
	gettimeofday(&m_tv, NULL);
}

bool rTickCount::isStarted() const
{
	return m_isStart;
}

void rTickCount::start(UDINT setting)
{
	restart();
	m_setting = setting;
}

void rTickCount::restart()
{
	m_isStart = true;
	gettimeofday(&m_tv, NULL);
}

void rTickCount::stop()
{
	m_isStart = false;
}

bool rTickCount::isFinished()
{
	return m_isStart && count() >= m_setting;
}

UDINT rTickCount::count()
{
	return getCount(false);
}

UDINT rTickCount::getCount(UDINT reset)
{
	UDINT   oldtick = 0;
	UDINT   curtick = 0;
	timeval curtv;

	gettimeofday(&curtv, NULL);

	oldtick = Tick(m_tv);
	curtick = Tick(curtv);

	if(reset)
	{
		m_tv = curtv;
	}

	return curtick - oldtick;
}


///////////////////////////////////////////////////////////////////////////////////////////////////

UDINT rTickCount::Tick(timeval &tv)
{
	return tv.tv_sec * 1000LL + tv.tv_usec / 1000;
}


UDINT rTickCount::TickUs(timeval &tv)
{
	return tv.tv_sec * 1000000LL + tv.tv_usec;
}

UDINT rTickCount::SysTick(void)
{
	timeval systv;
	gettimeofday(&systv, NULL);

	return Tick(systv);
}


UDINT rTickCount::SysTickUs(void)
{
	timeval systv;
	gettimeofday(&systv, NULL);

	return TickUs(systv);
}


UDINT rTickCount::UnixTime()
{
	timeval systv;
	gettimeofday(&systv, NULL);

	return systv.tv_sec;
}

