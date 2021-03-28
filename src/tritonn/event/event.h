//=================================================================================================
//===
//=== event/event.h
//===
//=== Copyright (c) 2019-2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#pragma once

#include "def.h"
#include "eid.h"
#include "datetime.h"

#define EVENT_ADDDATA(x, y)       {if(m_size + EPT_SIZE[x] + 1 >= DATA_SIZE) return 1; m_data[m_size++] = x; *(y *)(m_data + m_size) = val; m_size += EPT_SIZE[x]; return 0; }

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Структура заполения параметров события
//
// | TYPE_1 | VALUE_1 | TYPE_2 | VALUE_2 | ... | TYPE_n | VALUE_n |
// +--------+---------+--------+---------+-...-+--------+---------+
// | 1 байт | X байт  |
//
///////////////////////////////////////////////////////////////////////////////////////////////////

class rEvent
{
	enum
	{
		DATA_SIZE = 64,
	};

public:
	rEvent();
	rEvent(DINT eid);
	virtual ~rEvent();

	void  clear();

	rEvent& reinit(UDINT eid);
	DINT    getType()   const { return (m_EID >> 30) & 0x0003; } // Получить тип сообщения (см EMT_*)
	DINT    getSource() const { return (m_EID >> 29) & 0x0001; } // Получить источник сообщения (0 - 3Тонн, 1 - пользователь)
	DINT    getObject() const { return (m_EID >> 16) & 0x03FF; } //
	DINT    getID()     const { return  m_EID        & 0xFFFF; } //
	UDINT   getEID()    const { return  m_EID; }
	const rDateTime& getTime() const { return  m_timestamp; }
	
	void* getParamByID(UDINT ID, UDINT &type) const;
	std::string toString() const;

	// Функции для добавления параметров события, в случае переполнения буффера возвращают 1, в случае успеха - 0
	UDINT addSINT (SINT  val) { EVENT_ADDDATA(TYPE_SINT ,  SINT); }
	UDINT addUSINT(USINT val) { EVENT_ADDDATA(TYPE_USINT, USINT); }
	UDINT addINT  (INT   val) { EVENT_ADDDATA(TYPE_INT  ,   INT); }
	UDINT addUINT (UINT  val) { EVENT_ADDDATA(TYPE_UINT ,  UINT); }
	UDINT addDINT (DINT  val) { EVENT_ADDDATA(TYPE_DINT ,  DINT); }
	UDINT addUDINT(UDINT val) { EVENT_ADDDATA(TYPE_UDINT, UDINT); }
	UDINT addREAL (REAL  val) { EVENT_ADDDATA(TYPE_REAL ,  REAL); }
	UDINT addLREAL(LREAL val) { EVENT_ADDDATA(TYPE_LREAL, LREAL); }
	UDINT addSTR  (STRID val) { EVENT_ADDDATA(TYPE_STRID, UDINT); }

	rEvent& operator << (const SINT  &val) { addSINT (val); return *this; }
	rEvent& operator << (const USINT &val) { addUSINT(val); return *this; }
	rEvent& operator << (const INT   &val) { addINT  (val); return *this; }
	rEvent& operator << (const UINT  &val) { addUINT (val); return *this; }
	rEvent& operator << (const DINT  &val) { addDINT (val); return *this; }
	rEvent& operator << (const UDINT &val) { addUDINT(val); return *this; }
	rEvent& operator << (const REAL  &val) { addREAL (val); return *this; }
	rEvent& operator << (const LREAL &val) { addLREAL(val); return *this; }
	rEvent& operator << (const STRID &val) { addSTR  (val); return *this; }

	rEvent& operator = (const rEvent &event);

private:
	rDateTime m_timestamp; // 8 байт
	DINT      m_EID;       // 4 байта
	USINT     m_size;      // 1 байт
	USINT     m_reserv[3]; // 3 байта для выравнивания по границе 4 байт
	USINT     m_data[DATA_SIZE];  // 64 байта
};

#undef EVENT_ADDDATA

