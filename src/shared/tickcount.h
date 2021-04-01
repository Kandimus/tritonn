//=================================================================================================
//===
//=== tickcount.h
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

#pragma once

#include "def.h"


class rTickCount
{
public:
	rTickCount();
	virtual ~rTickCount() = default;

	UDINT count();
	void  start(UDINT msec);
	bool  isFinished();
	void  stop();
	bool  isStarted() const;
	void  restart();



	static UDINT SysTick(void); // msec
	static UDINT SysTickUs(void); // usec
	static UDINT Tick(timeval &tv);  // msec
	static UDINT TickUs(timeval &tv);
	static UDINT UnixTime();

protected:
	timeval m_tv;
	UDINT   m_setting = 0;
	bool    m_isStart = false;

	UDINT getCount(UDINT reset);
};

