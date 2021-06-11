//=================================================================================================
//===
//=== common.cpp
//===
//=== Copyright (c) 2019 by VeduN.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Описание типовых функций и утилит
//===
//=================================================================================================

#include <stdio.h>
#include <cmath>
#include <stdarg.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <algorithm>
#include "time64.h"
#include "def.h"

using std::string;


///////////////////////////////////////////////////////////////////////////////////////////////////
// Реализация задержки в милисекундах, через задержки в микросекунды и секунды
void mSleep(UDINT msec)
{
  UDINT usec = (msec % 1000) * 1000;
  UDINT sec  =  msec / 1000;

  if( sec)  sleep( sec);
  if(usec) usleep(usec);
}



void getCurrentTime(Time64_T &t64, struct tm *stm)
{
	UDT udt;

	gettimeofday(&udt, NULL);

	t64 = *((UDINT *)&udt.tv_sec);

	if(nullptr != stm)
	{
		localtime64_r(&t64, stm);
		stm->tm_year += 1900;
	}
}

void setCurrentTime(struct tm& stm)
{
	timeval tmv;

	tmv.tv_sec  = mktime(&stm);
	tmv.tv_usec = 0;
	settimeofday(&tmv, NULL);
}


LREAL Round(LREAL val,  USINT prec)
{
	static LREAL mul[6] = {1.0, 10.0, 100.0, 1000.0, 10000.0, 100000.0};

	if(prec > 6) prec = 6;

	if(val == 0.0) return val;

	return ((val < 0.0) ? std::ceil(val * mul[prec] - 0.5) : std::floor(val * mul[prec] + 0.5)) / mul[prec];
}



string String_ConvertByType(TYPE type, void *val)
{
	switch(type)
	{
		case TYPE::USINT: return String_format("%hhu", *(USINT *)val); break;
		case TYPE::SINT : return String_format("%hhi", *(SINT  *)val); break;
		case TYPE::UINT : return String_format("%hu" , *(UINT  *)val); break;
		case TYPE::INT  : return String_format("%hi" , *(INT   *)val); break;
		case TYPE::UDINT: return String_format("%u"  , *(UDINT *)val); break;
		case TYPE::DINT : return String_format("%i"  , *(DINT  *)val); break;
		case TYPE::REAL : return String_format("%#g" , *(REAL  *)val); break;
		case TYPE::LREAL: return String_format("%#g" , *(LREAL *)val); break;
		case TYPE::STRID: return String_format("%u"  , *(UDINT *)(STRID *)val); break;
		default: return "";
	}
}




UINT IsLeapYear(UINT year)
{
	return ((year % 4 == 0) && (year % 100 != 0) && (year % 400 == 0));
}

UINT DayInMonthShift(UINT year, UINT month)
{
	UINT days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	if(!month) return 0;

	while(month > 12)
	{
		month -= 12;
		++year;
	}

	if(month == 2) return days[1] + IsLeapYear(year);

	return days[month - 1];
}

// Функция возвращает номер недели, согласно ISO 8601
UINT WeekNumber(tm &curtm)
{
	USINT     iso8601[]     = { 6, 7, 8, 9, 10, 4, 5 };
	Time64_T  curtime       = mktime64(&curtm);
	Time64_T  startOfYear   = curtime - curtm.tm_yday;
	Time64_T  endOfYear     = curtime + (365 + IsLeapYear(curtm.tm_year) - curtm.tm_yday);
	struct tm startOfYearTM;

	gmtime64_r(&startOfYear, &startOfYearTM);

	// Согласно ISO 8601 четверг считается четвёртым днём недели, а также днём,
	// который определяет нумерацию недель: первая неделя года определяется как неделя,
	// содержащая первый четверг года, и так далее.
	int nds = curtm.tm_yday + iso8601[startOfYearTM.tm_wday];
	int wk  = nds / 7;

	switch (wk)
	{
		// Возвращаем номер недели от 31 декабря предыдущего года
		case 0:
		{
			struct tm lastyear;

			--startOfYear;
			gmtime64_r(&startOfYear, &lastyear);
			return WeekNumber(lastyear);
		}

		// Если 31 декабря выпадает до четверга 1 недели следующего года
		case 53:
		{
			struct tm endOfYearTM;

			gmtime64_r(&endOfYear, &endOfYearTM);
			if(endOfYearTM.tm_wday < 4)
			{
				return 1;
			}
		}
	}

	return wk;
}


