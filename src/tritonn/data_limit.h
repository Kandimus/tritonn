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

#define LIMIT_SETUP_ALL (rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO)

class rVariableList;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rLimit
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

	const UINT LIMIT_STATUS_MASK = 0x00FF;     // Маска для извлечения статуса пределов

	rLimit();
	virtual ~rLimit() = default;

	std::string getXML(const std::string& name, const std::string& prefix = "") const;

public:
	UDINT loadFromXML(tinyxml2::XMLElement *element, rError& err, const std::string& prefix);
	UDINT generateVars(rVariableList& list, const std::string& owner_name, STRID owner_unit, const std::string& owner_comment);
	UINT  calculate(LREAL val, UDINT check);

protected:
	void sendEvent(rEvent &e, LREAL *val, LREAL *lim, UDINT dontsend);

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

private:
	static rBitsArray m_flagsStatus;
};


