//=================================================================================================
//===
//=== data_limit.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс пределов сигнала
//===
//=================================================================================================

#pragma once

#include "data_source.h"
#include "bits_array.h"
#include "compared_values.h"

class rVariableList;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rLimit //: public rSource
{
public:
	enum Setup
	{
		NONE = 0x0000,
		OFF  = 0x0001,     // Сигнализация по пределам отключена
		LOLO = 0x0002,     // Выдавать сообщение AMIN
		LO   = 0x0004,     // Выдавать сообщение WMIN
		HI   = 0x0008,     // Выдавать сообщение WMAX
		HIHI = 0x0010,     // Выдавать сообщение AMAX
	};

	enum class Status : UINT
	{
		UNDEF  = 0, // Статус неопределен
		ISNAN,      // Не действительное значение числа
		LOLO,       // Значение ниже аварийного минимума
		LO,         // Значение ниже предаварийного минимума
		NORMAL,     // Значение в нормальном диапазоне
		HI,         // Значение выше предаварийного максимума
		HIHI,       // Значение выше аварийного максимума
	};

	const UINT   LIMIT_STATUS_MASK         = 0x00FF;     // Маска для извлечения статуса пределов

	rLimit();
	virtual ~rLimit();

public:
	virtual UDINT LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg);
	virtual UDINT generateVars(rVariableList& list, const string &owner_name, STRID owner_unit);

public:
	virtual UINT Calculate(LREAL val, UDINT check);

protected:
	void SendEvent(rEvent &e, LREAL *val, LREAL *lim, UDINT dontsend);

public:
	rCmpLREAL m_lolo;
	rCmpLREAL m_lo;
	rCmpLREAL m_hi;
	rCmpLREAL m_hihi;
	rCmpLREAL Hysteresis;              //

	rEvent    EventAMin;
	rEvent    EventWMin;
	rEvent    EventWMax;
	rEvent    EventAMax;
	rEvent    EventNormal;
	rEvent    EventNan;

	rEvent    EventChangeAMin;
	rEvent    EventChangeWMin;
	rEvent    EventChangeWMax;
	rEvent    EventChangeAMax;
	rEvent    EventChangeHyst;
	rEvent    EventChangeSetup;

	Status    m_status = Status::UNDEF;
	rCmpUINT  m_setup  = Setup::OFF;

	static rBitsArray m_flagsSetup;
};


