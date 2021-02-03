//=================================================================================================
//===
//=== data_denssol.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс расчета плотности с плотномера, по коэф-там Солортрона
//===
//=================================================================================================

#include <math.h>
#include <vector>
#include <limits>
#include <cmath>
#include "tinyxml2.h"
#include "density.h"
#include "event_eid.h"
#include "text_id.h"
#include "text_manager.h"
#include "event_manager.h"
#include "data_manager.h"
#include "data_link.h"
#include "data_config.h"
#include "variable_item.h"
#include "variable_list.h"
#include "data_station.h"
#include "data_denssol.h"
#include "xml_util.h"
#include "generator_md.h"


const UDINT DENSSOL_LE_PERIOD    = 0x00000001;
const UDINT DENSSOL_LE_INPUTS    = 0x00000002;
const UDINT DENSSOL_LE_VALUE     = 0x00000004;
const UDINT DENSSOL_LE_STATION   = 0x00000008;
const UDINT DENSSOL_LE_ITERATION = 0x00000010;


///////////////////////////////////////////////////////////////////////////////////////////////////
//
rDensSol::rDensSol(const rStation* owner) : rSource(owner), Setup(0)
{
	LockErr     = 0;
	Calibr      = 20.0;
//	Setup       = DNSSOL_SETUP_OFF;

	// Настройка линков (входов)
	InitLink(rLink::Setup::OUTPUT  , Dens  , U_kg_m3  , SID::DENSITY    , XmlName::DENSITY  , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INOUTPUT, Temp  , U_C      , SID::TEMPERATURE, XmlName::TEMP     , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INOUTPUT, Pres  , U_bar    , SID::PRESSURE   , XmlName::PRES     , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INPUT   , Period, U_mksec  , SID::PERIOD     , XmlName::PERIOD   , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT  , Dens15, U_kg_m3  , SID::DENSITY15  , XmlName::DENSITY15, rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT  , Dens20, U_kg_m3  , SID::DENSITY20  , XmlName::DENSITY20, rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT  , B     , U_1_C    , SID::B          , XmlName::B        , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT  , B15   , U_1_C    , SID::B15        , XmlName::B15      , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT  , Y     , U_1_MPa  , SID::Y          , XmlName::Y        , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT  , Y15   , U_1_MPa  , SID::Y15        , XmlName::Y15      , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT  , CTL   , U_DIMLESS, SID::CTL        , XmlName::CTL      , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT  , CPL   , U_DIMLESS, SID::CPL        , XmlName::CPL      , rLink::SHADOW_NONE);
}


rDensSol::~rDensSol()
{
	;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDensSol::InitLimitEvent(rLink &link)
{
	link.Limit.EventChangeAMin  = ReinitEvent(EID_DENSSOL_NEW_AMIN)  << link.Descr << link.Unit;
	link.Limit.EventChangeWMin  = ReinitEvent(EID_DENSSOL_NEW_WMIN)  << link.Descr << link.Unit;
	link.Limit.EventChangeWMax  = ReinitEvent(EID_DENSSOL_NEW_WMAX)  << link.Descr << link.Unit;
	link.Limit.EventChangeAMax  = ReinitEvent(EID_DENSSOL_NEW_AMAX)  << link.Descr << link.Unit;
	link.Limit.EventChangeHyst  = ReinitEvent(EID_DENSSOL_NEW_HYST)  << link.Descr << link.Unit;
	link.Limit.EventChangeSetup = ReinitEvent(EID_DENSSOL_NEW_SETUP) << link.Descr << link.Unit;
	link.Limit.EventAMin        = ReinitEvent(EID_DENSSOL_AMIN)      << link.Descr << link.Unit;
	link.Limit.EventWMin        = ReinitEvent(EID_DENSSOL_WMIN)      << link.Descr << link.Unit;
	link.Limit.EventWMax        = ReinitEvent(EID_DENSSOL_WMAX)      << link.Descr << link.Unit;
	link.Limit.EventAMax        = ReinitEvent(EID_DENSSOL_AMAX)      << link.Descr << link.Unit;
	link.Limit.EventNan         = ReinitEvent(EID_DENSSOL_NAN)       << link.Descr << link.Unit;
	link.Limit.EventNormal      = ReinitEvent(EID_DENSSOL_NORMAL)    << link.Descr << link.Unit;

	return 0;
}


//-------------------------------------------------------------------------------------------------
//


//-------------------------------------------------------------------------------------------------
//
UDINT rDensSol::Calculate()
{
	rEvent event_f;
	rEvent event_s;
	UDINT  err    = 0;

	if (rSource::Calculate()) {
		return 0;
	}

	//-------------------------------------------------------------------------------------------
	// Обработка ввода пользователя
	Calibr.Compare   (COMPARE_LREAL_PREC, ReinitEvent(EID_DENSSOL_CALIBR));
	Coef.K0.Compare  (COMPARE_LREAL_PREC, ReinitEvent(EID_DENSSOL_K0    ));
	Coef.K1.Compare  (COMPARE_LREAL_PREC, ReinitEvent(EID_DENSSOL_K1    ));
	Coef.K2.Compare  (COMPARE_LREAL_PREC, ReinitEvent(EID_DENSSOL_K2    ));
	Coef.K18.Compare (COMPARE_LREAL_PREC, ReinitEvent(EID_DENSSOL_K18   ));
	Coef.K19.Compare (COMPARE_LREAL_PREC, ReinitEvent(EID_DENSSOL_K19   ));
	Coef.K20A.Compare(COMPARE_LREAL_PREC, ReinitEvent(EID_DENSSOL_K20A  ));
	Coef.K20B.Compare(COMPARE_LREAL_PREC, ReinitEvent(EID_DENSSOL_K20B  ));
	Coef.K21A.Compare(COMPARE_LREAL_PREC, ReinitEvent(EID_DENSSOL_K21A  ));
	Coef.K21B.Compare(COMPARE_LREAL_PREC, ReinitEvent(EID_DENSSOL_K21B  ));

	// Если введены новые коэффициенты
	if (m_accept) {
		UsedCoef = Coef;
		m_accept = 0;

		rEventManager::instance().Add(ReinitEvent(EID_DENSSOL_ACCEPT));
	}

	if (CheckExpr(err, DENSSOL_LE_INPUTS,
				  event_f.Reinit(EID_DENSSOL_FAULT_INPUTS) << ID << Descr,
				  event_s.Reinit(EID_DENSSOL_GOOD_INPUTS ) << ID << Descr)) {
		return SetFault();
	}

	//
	if (CheckExpr(Period.Value < 1, DENSSOL_LE_PERIOD,
				  event_f.Reinit(EID_DENSSOL_FAULT_PERIOD) << ID << Descr,
				  event_s.Reinit(EID_DENSSOL_GOOD_PERIOD ) << ID << Descr)) {
		return SetFault();
	}

	// Расчет плотности
	rDensity::Product product = m_station->m_product;
	LREAL dTemp = Temp.Value - Calibr.Value;
	LREAL K20   = UsedCoef.K20A.Value + UsedCoef.K20B.Value * Pres.Value;
	LREAL K21   = UsedCoef.K21A.Value + UsedCoef.K21B.Value * Pres.Value;
	UDINT limit = 0;

	Dens.Value = UsedCoef.K0.Value + UsedCoef.K1.Value * Period.Value + UsedCoef.K2.Value * Period.Value * Period.Value;
	Dens.Value = Dens.Value * (1.0 + UsedCoef.K18.Value * dTemp      ) + UsedCoef.K19.Value * dTemp;
	Dens.Value = Dens.Value * (1.0 + K20                * Pres.Value ) + K21                * Pres.Value;

	// Приведение плотности к 15 градусам
	// по ГОСТ Р 50.2.076-2010
	LREAL OldD15 = 0.0;
	LREAL D15_2  = 0.0;
	UDINT count_iteration = 0;
	UDINT count_product   = 0;
	USINT  oil_id         = rDensity::getTypeProduct(Dens.Value, product);

	while(count_product < 3)
	{
		dTemp        = Temp.Value - 15.0;
		OldD15       = -100000.0;
		Dens15.Value = Dens.Value;
		K0           = rDensity::K0[oil_id];
		K1           = rDensity::K1[oil_id];
		K2           = rDensity::K2[oil_id];

		while(std::abs(OldD15 - Dens15.Value) > DENSITY15_CONVERGENCE)
		{
			D15_2        = Dens15.Value * Dens15.Value;
			OldD15       = Dens15.Value;
			B15.Value    = (K0 + K1 * Dens15.Value) / (D15_2) + K2;
			B.Value      = B15.Value + 1.6 * B15.Value * B15.Value * dTemp;
			Y15.Value    = rDensity::getY15(Dens15.Value);
			Y.Value      = rDensity::getY  (Dens15.Value, Temp.Value);
			CPL.Value    = 1.0 / (1.0 - Y.Value * Pres.Value);
			CTL.Value    = exp(-B15.Value * dTemp * (1 + 0.8 * B15.Value * dTemp));
			Dens15.Value = (Dens.Value / CPL.Value) / CTL.Value;

			if(++count_iteration >= 20) break;
		}

		// Проверка полученной плотности
		if(CheckExpr(count_iteration >= 20, DENSSOL_LE_ITERATION, event_f.Reinit(EID_DENSSOL_FAULT_ITERATION) << ID << Descr,
																					 event_s.Reinit(EID_DENSSOL_GOOD_ITERATION ) << ID << Descr))
		{
			return SetFault();
		}

		// Для бензинов, определяем куда попали по плотности, если
		// тип изменился, то считаем еще раз.
		if(oil_id == rDensity::getTypeProduct(Dens15.Value, product)) break;

		oil_id = rDensity::getTypeProduct(Dens15.Value, product);
		++count_product;
	}

	//  Проверка полученной плотности
	USINT product_id = static_cast<USINT>(product);
	limit = rDensity::Limit[0][product_id] <= Dens15.Value && Dens15.Value < rDensity::Limit[1][product_id];
	if(CheckExpr(!limit, DENSSOL_LE_VALUE, ReinitEvent(event_f, EID_DENSSOL_FAULT_VALUE) << Dens15.Value, ReinitEvent(event_s, EID_DENSSOL_GOOD_VALUE ) << Dens15.Value))
	{
		Fault = 1;
	}

	// Расчитаем плотность при 20 *С
	Dens20.Value = rDensity::getDens20(Dens15.Value, B15.Value);

	PostCalculate();

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDensSol::SetFault()
{
	Dens.Value   = std::numeric_limits<LREAL>::quiet_NaN();;
	Dens15.Value = std::numeric_limits<LREAL>::quiet_NaN();;
	Dens20.Value = std::numeric_limits<LREAL>::quiet_NaN();;
	B.Value      = std::numeric_limits<LREAL>::quiet_NaN();;
	Y.Value      = std::numeric_limits<LREAL>::quiet_NaN();;
	CTL.Value    = std::numeric_limits<LREAL>::quiet_NaN();;
	CPL.Value    = std::numeric_limits<LREAL>::quiet_NaN();;
	B15.Value    = std::numeric_limits<LREAL>::quiet_NaN();;
	Y15.Value    = std::numeric_limits<LREAL>::quiet_NaN();;
	K0           = std::numeric_limits<LREAL>::quiet_NaN();;
	K1           = std::numeric_limits<LREAL>::quiet_NaN();;
	K2           = std::numeric_limits<LREAL>::quiet_NaN();;
	Fault        = 1;

	return Fault;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
//



///////////////////////////////////////////////////////////////////////////////////////////////////
//

///////////////////////////////////////////////////////////////////////////////////////////////////
//
UDINT rDensSol::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	// Variables
	list.add(Alias + ".k0"               , TYPE_LREAL, rVariable::Flags::R___, &K0                 , U_DIMLESS, 0);
	list.add(Alias + ".k1"               , TYPE_LREAL, rVariable::Flags::R___, &K1                 , U_DIMLESS, 0);
	list.add(Alias + ".k2"               , TYPE_LREAL, rVariable::Flags::R___, &K2                 , U_DIMLESS, 0);
	list.add(Alias + ".factor.k0"        , TYPE_LREAL, rVariable::Flags::RS__, &UsedCoef.K0.Value  , U_COEFSOL, ACCESS_SA);
	list.add(Alias + ".factor.k1"        , TYPE_LREAL, rVariable::Flags::RS__, &UsedCoef.K1.Value  , U_COEFSOL, ACCESS_SA);
	list.add(Alias + ".factor.k2"        , TYPE_LREAL, rVariable::Flags::RS__, &UsedCoef.K2.Value  , U_COEFSOL, ACCESS_SA);
	list.add(Alias + ".factor.k18"       , TYPE_LREAL, rVariable::Flags::RS__, &UsedCoef.K18.Value , U_COEFSOL, ACCESS_SA);
	list.add(Alias + ".factor.k19"       , TYPE_LREAL, rVariable::Flags::RS__, &UsedCoef.K19.Value , U_COEFSOL, ACCESS_SA);
	list.add(Alias + ".factor.k20a"      , TYPE_LREAL, rVariable::Flags::RS__, &UsedCoef.K20A.Value, U_COEFSOL, ACCESS_SA);
	list.add(Alias + ".factor.k20b"      , TYPE_LREAL, rVariable::Flags::RS__, &UsedCoef.K20B.Value, U_COEFSOL, ACCESS_SA);
	list.add(Alias + ".factor.k21a"      , TYPE_LREAL, rVariable::Flags::RS__, &UsedCoef.K21A.Value, U_COEFSOL, ACCESS_SA);
	list.add(Alias + ".factor.k21b"      , TYPE_LREAL, rVariable::Flags::RS__, &UsedCoef.K21B.Value, U_COEFSOL, ACCESS_SA);
	list.add(Alias + ".factor.set.k0"    , TYPE_LREAL, rVariable::Flags::___L, &Coef.K0.Value      , U_COEFSOL, ACCESS_FACTORS);
	list.add(Alias + ".factor.set.k1"    , TYPE_LREAL, rVariable::Flags::___L, &Coef.K1.Value      , U_COEFSOL, ACCESS_FACTORS);
	list.add(Alias + ".factor.set.k2"    , TYPE_LREAL, rVariable::Flags::___L, &Coef.K2.Value      , U_COEFSOL, ACCESS_FACTORS);
	list.add(Alias + ".factor.set.k18"   , TYPE_LREAL, rVariable::Flags::___L, &Coef.K18.Value     , U_COEFSOL, ACCESS_FACTORS);
	list.add(Alias + ".factor.set.k19"   , TYPE_LREAL, rVariable::Flags::___L, &Coef.K19.Value     , U_COEFSOL, ACCESS_FACTORS);
	list.add(Alias + ".factor.set.k20a"  , TYPE_LREAL, rVariable::Flags::___L, &Coef.K20A.Value    , U_COEFSOL, ACCESS_FACTORS);
	list.add(Alias + ".factor.set.k20b"  , TYPE_LREAL, rVariable::Flags::___L, &Coef.K20B.Value    , U_COEFSOL, ACCESS_FACTORS);
	list.add(Alias + ".factor.set.k21a"  , TYPE_LREAL, rVariable::Flags::___L, &Coef.K21A.Value    , U_COEFSOL, ACCESS_FACTORS);
	list.add(Alias + ".factor.set.k21b"  , TYPE_LREAL, rVariable::Flags::___L, &Coef.K21B.Value    , U_COEFSOL, ACCESS_FACTORS);
	list.add(Alias + ".factor.set.accept", TYPE_USINT, rVariable::Flags::___L, &m_accept           , U_DIMLESS, ACCESS_FACTORS);
	list.add(Alias + ".Calibration"      , TYPE_LREAL, rVariable::Flags::___L, &Calibr.Value       , U_C      , ACCESS_FACTORS);
	list.add(Alias + ".Setup"            , TYPE_UINT , rVariable::Flags::RS__, &Setup.Value        , U_DIMLESS, ACCESS_FACTORS);

	list.add(Alias + ".fault"            , TYPE_UDINT, rVariable::Flags::R___, &Fault              , U_DIMLESS, 0);

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDensSol::LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	if (TRITONN_RESULT_OK != rSource::LoadFromXML(element, err, prefix)) {
		return err.getError();
	}

	tinyxml2::XMLElement *xml_koef   = element->FirstChildElement(XmlName::FACTORS);
	tinyxml2::XMLElement *xml_temp   = element->FirstChildElement(XmlName::TEMP);
	tinyxml2::XMLElement *xml_pres   = element->FirstChildElement(XmlName::PRES);
	tinyxml2::XMLElement *xml_period = element->FirstChildElement(XmlName::PERIOD);

	if (!xml_koef || !xml_temp || !xml_pres || !xml_period) {
		return err.set(DATACFGERR_DENSSOL, element->GetLineNum(), "fault inputs");
	}

	// Обязательные линки и параметры, без которых работа не возможна
	if(TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_temp->FirstChildElement  (XmlName::LINK), Temp  )) return err.getError();
	if(TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_pres->FirstChildElement  (XmlName::LINK), Pres  )) return err.getError();
	if(TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_period->FirstChildElement(XmlName::LINK), Period)) return err.getError();

	UDINT fault = 0;
	Coef.K0.Init  (XmlUtils::getTextLREAL(xml_koef->FirstChildElement("k0")  , 0.0, fault));
	Coef.K1.Init  (XmlUtils::getTextLREAL(xml_koef->FirstChildElement("k1")  , 0.0, fault));
	Coef.K2.Init  (XmlUtils::getTextLREAL(xml_koef->FirstChildElement("k2")  , 0.0, fault));
	Coef.K18.Init (XmlUtils::getTextLREAL(xml_koef->FirstChildElement("k18") , 0.0, fault));
	Coef.K19.Init (XmlUtils::getTextLREAL(xml_koef->FirstChildElement("k19") , 0.0, fault));
	Coef.K20A.Init(XmlUtils::getTextLREAL(xml_koef->FirstChildElement("k20a"), 0.0, fault));
	Coef.K20B.Init(XmlUtils::getTextLREAL(xml_koef->FirstChildElement("k20b"), 0.0, fault));
	Coef.K21A.Init(XmlUtils::getTextLREAL(xml_koef->FirstChildElement("k21a"), 0.0, fault));
	Coef.K21B.Init(XmlUtils::getTextLREAL(xml_koef->FirstChildElement("k21b"), 0.0, fault));

	if (fault) {
		return err.set(DATACFGERR_DENSSOL, xml_koef->GetLineNum(), "");
	}

	// Не обязательный параметр
	Calibr.Init(XmlUtils::getTextLREAL(element->FirstChildElement(XmlName::CALIBR), 20.0, fault));

	fault = 0;

	// Так как мы еще не загрузили данные из EEPROM, то принимает текущие коэф-ты за рабочие.
	UsedCoef = Coef;

	// Проверки
	if (!m_station) {
		rEventManager::instance().Add(ReinitEvent(EID_DENSSOL_FAULT_STATION));
		return err.set(DATACFGERR_DENSSOL_NOSTN, element->GetLineNum(), "station is empty");
	}

	ReinitLimitEvents();

	return TRITONN_RESULT_OK;
}


std::string rDensSol::saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal)
{
	Period.Limit.m_setup.Init(rLimit::Setup::NONE);
	Temp.Limit.m_setup.Init(rLimit::Setup::NONE);
	Pres.Limit.m_setup.Init(rLimit::Setup::NONE);
	Dens.Limit.m_setup.Init(rLimit::Setup::NONE);
	Dens15.Limit.m_setup.Init(rLimit::Setup::NONE);
	Dens20.Limit.m_setup.Init(rLimit::Setup::NONE);
	B.Limit.m_setup.Init(rLimit::Setup::NONE);
	Y.Limit.m_setup.Init(rLimit::Setup::NONE);
	CTL.Limit.m_setup.Init(rLimit::Setup::NONE);
	CPL.Limit.m_setup.Init(rLimit::Setup::NONE);
	B15.Limit.m_setup.Init(rLimit::Setup::NONE);
	Y15.Limit.m_setup.Init(rLimit::Setup::NONE);

	return rSource::saveKernel(isio, objname, comment, isglobal);
}


UDINT rDensSol::generateMarkDown(rGeneratorMD& md)
{
	Dens.Limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	Temp.Limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	Pres.Limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	Period.Limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	Dens15.Limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	Dens20.Limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	B.Limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	B15.Limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	Y.Limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	Y15.Limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	CTL.Limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);
	CPL.Limit.m_setup.Init(rLimit::Setup::HIHI | rLimit::Setup::HI | rLimit::Setup::LO | rLimit::Setup::LOLO);

	md.add(this, true);

	return TRITONN_RESULT_OK;
}


