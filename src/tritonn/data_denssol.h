//=================================================================================================
//===
//=== data_denssol.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс расчета плотности с плотномера, по коэф-там Солортрона и ГОСТ Р 50.2.076-2010 ГСИ
//===
//=================================================================================================

#pragma once

#include "data/link.h"
#include "compared_values.h"


struct rDensSolCoef
{
	rCmpLREAL K0;
	rCmpLREAL K1;
	rCmpLREAL K2;
	rCmpLREAL K18;
	rCmpLREAL K19;
	rCmpLREAL K20A;
	rCmpLREAL K20B;
	rCmpLREAL K21A;
	rCmpLREAL K21B;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rDensSol : public rSource
{
public:
	rDensSol(const rStation* owner = nullptr);
	virtual ~rDensSol() = default;
	
protected:
	UDINT setFault();

	// Виртуальные функции от rSource
public:
	virtual const char* getRTTI() const override { return "denssol"; }

	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix) override;
	virtual UDINT generateVars(rVariableList& list) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual UDINT calculate() override;
protected:
	virtual UDINT       initLimitEvent(rLink& link);

public:
	// Inputs, Inoutputs
	rLink        m_period;
	rLink        m_temp;
	rLink        m_pres;

	// Outputs
	rLink        m_dens;
	rLink        m_dens15;
	rLink        m_dens20;
	rLink        m_b;
	rLink        m_y;
	rLink        m_ctl;
	rLink        m_cpl;
	rLink        m_b15;
	rLink        m_y15;

	rDensSolCoef m_setCoef;               // Коэф-ты для установки пользователем
	rDensSolCoef m_curCoef;               // Используемые коэф-ты
	USINT        m_accept = 0;            // Команда на принятие коэф-тов
	rCmpLREAL    m_calibrT;               // Температура калибровки
	rCmpUINT     m_setup;                 // Настройка плотномера
	LREAL        m_k0;
	LREAL        m_k1;
	LREAL        m_k2;
};



