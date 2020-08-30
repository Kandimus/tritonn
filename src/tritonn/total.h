//=================================================================================================
//===
//=== total.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Нарастающие
//===
//=================================================================================================

#pragma once


#include "def.h"


const UDINT TOTAL_MAX_MASS     = 0x00000001;
const UDINT TOTAL_MAX_VOLUME   = 0x00000002;
const UDINT TOTAL_MAX_VOLUME15 = 0x00000002;
const UDINT TOTAL_MAX_VOLUME20 = 0x00000002;

struct rBaseTotal
{
	LREAL Mass;
	LREAL Volume;
	LREAL Volume15;
	LREAL Volume20;
	UDINT Count;
};


struct rTotal
{
	rBaseTotal Past;    // Нарастающие на прошлом скане
	rBaseTotal Raw;     // Сырые текущие нарастающие (обрезанные до 5 знаков)
	rBaseTotal Inc;     // Инкремент нарастающих на текущем скане (обрезанные до 5 знаков)
	rBaseTotal Present; // Текущие нарастающие

	UDINT Calculate(UDINT unitmass, UDINT unitvol);

	static LREAL Sub(LREAL sub1, LREAL sub2);
	static void  Clear(rBaseTotal &total);

protected:
	UDINT CheckMax();
};

