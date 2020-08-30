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
	gettimeofday(&tv, NULL); 
}


rTickCount::~rTickCount()
{
	;
}


UDINT rTickCount::Tick(timeval &tv)
{
	return tv.tv_sec * 1000LL + tv.tv_usec / 1000;
}


UDINT rTickCount::TickUs(timeval &tv)
{
	return tv.tv_sec * 1000000LL + tv.tv_usec;
}


UDINT rTickCount::Timer()
{
	return GetCount(true);
}


//
UDINT rTickCount::GetCount()
{

	return GetCount(false);
}


UDINT rTickCount::Reset()
{
	gettimeofday(&tv, NULL);

	return 0;
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


UDINT rTickCount::GetCount(UDINT reset)
{
	UDINT   oldtick = 0;
	UDINT   curtick = 0;
	timeval curtv;

	gettimeofday(&curtv, NULL);

	oldtick = Tick(tv);
	curtick = Tick(curtv);

	if(reset)
	{
		tv = curtv;
	}

	return curtick - oldtick;
}

