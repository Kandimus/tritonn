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
	virtual ~rTickCount();

	UDINT Timer();
	UDINT GetCount();
	UDINT Reset();

	static UDINT SysTick(void); // msec
	static UDINT SysTickUs(void); // usec
	static UDINT Tick(timeval &tv);  // msec
	static UDINT TickUs(timeval &tv);
	static UDINT UnixTime();

protected:
	timeval tv;

	UDINT GetCount(UDINT reset);
};

