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

#include "data_link.h"
#include "compared_values.h"


// Внутренние флаги
const LREAL DENSITY15_CONVERGENCE = 0.0005;


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
	rDensSol();
	virtual ~rDensSol();
	
	// Виртуальные функции от rSource
public:
	virtual const char *RTTI() const { return "denssol"; }

	virtual UDINT LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix);
	virtual UDINT generateVars(rVariableList& list);
	virtual std::string saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal);
	virtual UDINT Calculate();
protected:
	virtual UDINT InitLimitEvent(rLink &link);

public:
	// Inputs, Inoutputs
	rLink        Period;
	rLink        Temp;
	rLink        Pres;

	// Outputs
	rLink        Dens;
	rLink        Dens15;
	rLink        Dens20;
	rLink        B;
	rLink        Y;
	rLink        CTL;
	rLink        CPL;
	rLink        B15;
	rLink        Y15;

	rDensSolCoef Coef;                    // Коэф-ты для установки пользователем
	rDensSolCoef UsedCoef;                // Используемые коэф-ты
	USINT        Accept;                  // Команда на принятие коэф-тов
	rCmpLREAL    Calibr;                  // Температура калибровки
	rCmpUINT     Setup;                   // Настройка плотномера
	LREAL        K0;
	LREAL        K1;
	LREAL        K2;

protected:
	UDINT SetFault();

};



