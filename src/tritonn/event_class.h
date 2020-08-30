//=================================================================================================
//===
//=== event_class.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс для регистрации событий, как системных, так и пользователя.
//===
//=================================================================================================

#pragma once


#include "def.h"
#include "event_eid.h"

//#define EVENT_ADDDATA(x, y)       {if(Size + EPT_SIZE[x] + 1 >= MAX_EVENT_DATA) return 1; Data[Size++] = EPT_SIZE[x]; *(y *)(Data + Size) = val; Size += EPT_SIZE[x]; return 0; }
#define EVENT_ADDDATA(x, y)       {if(Size + EPT_SIZE[x] + 1 >= MAX_EVENT_DATA) return 1; Data[Size++] = x; *(y *)(Data + Size) = val; Size += EPT_SIZE[x]; return 0; }

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Структура заполения параметров события
//
// | TYPE_1 | VALUE_1 | TYPE_2 | VALUE_2 | ... | TYPE_n | VALUE_n |
// +--------+---------+--------+---------+-...-+--------+---------+
// | 1 байт | X байт  |
//
///////////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////////////
// Структура сообщения
class rEvent
{
public:
	rEvent();
	rEvent(DINT eid);
	virtual ~rEvent();

	void  Clear();

	rEvent &Reinit(UDINT eid);
	DINT    GetType()   { return (EID >> 30) & 0x0003; } // Получить тип сообщения (см EMT_*)
	DINT    GetSource() { return (EID >> 29) & 0x0001; } // Получить источник сообщения (0 - 3Тонн, 1 - пользователь)
	DINT    GetObject() { return (EID >> 16) & 0x03FF; } //
	DINT    GetID()     { return  EID        & 0xFFFF; } //
	UDINT   GetEID()    { return EID; }
	
	void *GetParamByID(UDINT ID, UDINT &type);

	// Функции для добавления параметров события, в случае переполнения буффера возвращают 1, в случае успеха - 0
	UDINT AddSINT (SINT  val) { EVENT_ADDDATA(TYPE_SINT ,  SINT); }
	UDINT AddUSINT(USINT val) { EVENT_ADDDATA(TYPE_USINT, USINT); }
	UDINT AddINT  (INT   val) { EVENT_ADDDATA(TYPE_INT  ,   INT); }
	UDINT AddUINT (UINT  val) { EVENT_ADDDATA(TYPE_UINT ,  UINT); }
	UDINT AddDINT (DINT  val) { EVENT_ADDDATA(TYPE_DINT ,  DINT); }
	UDINT AddUDINT(UDINT val) { EVENT_ADDDATA(TYPE_UDINT, UDINT); }
	UDINT AddREAL (REAL  val) { EVENT_ADDDATA(TYPE_REAL ,  REAL); }
	UDINT AddLREAL(LREAL val) { EVENT_ADDDATA(TYPE_LREAL, LREAL); }
	UDINT AddSTR  (STRID val) { EVENT_ADDDATA(TYPE_STRID, UDINT); }

	rEvent & operator << (const SINT  &val) { AddSINT (val); return *this; }
	rEvent & operator << (const USINT &val) { AddUSINT(val); return *this; }
	rEvent & operator << (const INT   &val) { AddINT  (val); return *this; }
	rEvent & operator << (const UINT  &val) { AddUINT (val); return *this; }
	rEvent & operator << (const DINT  &val) { AddDINT (val); return *this; }
	rEvent & operator << (const UDINT &val) { AddUDINT(val); return *this; }
	rEvent & operator << (const REAL  &val) { AddREAL (val); return *this; }
	rEvent & operator << (const LREAL &val) { AddLREAL(val); return *this; }
	rEvent & operator << (const STRID &val) { AddSTR  (val); return *this; }

private:
	UDT         Timestamp; // 8 байт
	DINT        EID;       // 4 байта
	USINT       Size;      // 1 байт
	USINT       Reserv[3]; // 3 байта для выравнивания по границе 4 байт
	USINT       Data[MAX_EVENT_DATA];  // 64 байта
};

#undef EVENT_ADDDATA

