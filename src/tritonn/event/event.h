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
#include "container.h"

#define EVENT_ADDDATA(x, y)       {if(m_size + getTypeSize(x) + 1 >= DATA_SIZE) return 1; m_data[m_size++] = static_cast<unsigned char>(x); *(y *)(m_data + m_size) = val; m_size += getTypeSize(x); return 0; }

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Структура заполения параметров события
//
// | TYPE_1 | VALUE_1 | TYPE_2 | VALUE_2 | ... | TYPE_n | VALUE_n |
// +--------+---------+--------+---------+-...-+--------+---------+
// | 1 байт | X байт  |
//
///////////////////////////////////////////////////////////////////////////////////////////////////

struct rEvent
{
	enum Packet : UDINT
	{
		MAGIC = 0xFFEEDDCC,
	};

	enum
	{
		DATA_SIZE = 64,
	};

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
	UDINT   getMagic()  const { return  m_magic; }
	const rDateTime& getTime() const { return  m_timestamp; }
	
	void* getParamByID(UDINT ID, TYPE& type) const;
	std::string toString() const;
	const void* getRaw() const { return &m_magic; }

	// Функции для добавления параметров события, в случае переполнения буффера возвращают 1, в случае успеха - 0
	UDINT addSINT (SINT  val) { EVENT_ADDDATA(TYPE::SINT ,  SINT); }
	UDINT addUSINT(USINT val) { EVENT_ADDDATA(TYPE::USINT, USINT); }
	UDINT addINT  (INT   val) { EVENT_ADDDATA(TYPE::INT  ,   INT); }
	UDINT addUINT (UINT  val) { EVENT_ADDDATA(TYPE::UINT ,  UINT); }
	UDINT addDINT (DINT  val) { EVENT_ADDDATA(TYPE::DINT ,  DINT); }
	UDINT addUDINT(UDINT val) { EVENT_ADDDATA(TYPE::UDINT, UDINT); }
	UDINT addREAL (REAL  val) { EVENT_ADDDATA(TYPE::REAL ,  REAL); }
	UDINT addLREAL(LREAL val) { EVENT_ADDDATA(TYPE::LREAL, LREAL); }
	UDINT addSTR  (STRID val) { EVENT_ADDDATA(TYPE::STRID, UDINT); }

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

	Container& toContainer(Container& cnt) const;
	Container& fromContainer(Container& cnt);

private:
	UDINT     m_magic;
	USINT     m_version;   // 1 byte
	USINT     m_size;      // 1 byte
	USINT     m_reserv[2]; // 2 байта для выравнивания по границе 4 байт
	rDateTime m_timestamp; // 8 байт
	DINT      m_EID;       // 4 байта
	USINT     m_data[DATA_SIZE];  // 64 байта
};

#undef EVENT_ADDDATA

Container& operator << (Container& cnt, const rEvent& event);
Container& operator >> (Container& cnt, rEvent& event);
