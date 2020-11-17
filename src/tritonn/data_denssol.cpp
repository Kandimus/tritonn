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


const UDINT DENSSOL_LE_PERIOD    = 0x00000001;
const UDINT DENSSOL_LE_INPUTS    = 0x00000002;
const UDINT DENSSOL_LE_VALUE     = 0x00000004;
const UDINT DENSSOL_LE_STATION   = 0x00000008;
const UDINT DENSSOL_LE_ITERATION = 0x00000010;


///////////////////////////////////////////////////////////////////////////////////////////////////
//
rDensSol::rDensSol() : Setup(0)
{
	LockErr     = 0;
	Calibr      = 20.0;
//	Setup       = DNSSOL_SETUP_OFF;

	// Настройка линков (входов)
	InitLink(LINK_SETUP_INOUTPUT, Temp  , U_C      , SID_TEMPERATURE, XmlName::TEMP     , LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_INOUTPUT, Pres  , U_bar    , SID_PRESSURE   , XmlName::PRES     , LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_INPUT   , Period, U_mksec  , SID_PERIOD     , XmlName::PERIOD   , LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_OUTPUT  , Dens  , U_kg_m3  , SID_DENSITY    , XmlName::DENSITY  , LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_OUTPUT  , Dens15, U_kg_m3  , SID_DENSITY15  , XmlName::DENSITY15, LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_OUTPUT  , Dens20, U_kg_m3  , SID_DENSITY20  , XmlName::DENSITY20, LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_OUTPUT  , B     , U_1_C    , SID_B          , XmlName::B        , LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_OUTPUT  , B15   , U_1_C    , SID_B15        , XmlName::B15      , LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_OUTPUT  , Y     , U_1_MPa  , SID_Y          , XmlName::Y        , LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_OUTPUT  , Y15   , U_1_MPa  , SID_Y15        , XmlName::Y15      , LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_OUTPUT  , CTL   , U_DIMLESS, SID_CTL        , XmlName::CTL      , LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_OUTPUT  , CPL   , U_DIMLESS, SID_CPL        , XmlName::CPL      , LINK_SHADOW_NONE);
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

	if(rSource::Calculate()) return 0;

	if(CheckExpr(err, DENSSOL_LE_INPUTS, event_f.Reinit(EID_DENSSOL_FAULT_INPUTS) << ID << Descr,
													 event_s.Reinit(EID_DENSSOL_GOOD_INPUTS ) << ID << Descr))
	{
		return SetFault();
	}

	//
	if(CheckExpr(Period.Value < 1, DENSSOL_LE_PERIOD, event_f.Reinit(EID_DENSSOL_FAULT_PERIOD) << ID << Descr,
																	  event_s.Reinit(EID_DENSSOL_GOOD_PERIOD ) << ID << Descr))
	{
		return SetFault();
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
	if(Accept)
	{
		UsedCoef = Coef;
		Accept   = 0;

		rEventManager::instance().Add(ReinitEvent(EID_DENSSOL_ACCEPT));
	}

	// Расчет плотности
	TYPE_PRODUCT product = Station->Product;
	LREAL        dTemp   = Temp.Value - Calibr.Value;
	LREAL        K20     = UsedCoef.K20A.Value + UsedCoef.K20B.Value * Pres.Value;
	LREAL        K21     = UsedCoef.K21A.Value + UsedCoef.K21B.Value * Pres.Value;
	UDINT        limit   = 0;

	Dens.Value = UsedCoef.K0.Value + UsedCoef.K1.Value * Period.Value + UsedCoef.K2.Value * Period.Value * Period.Value;
	Dens.Value = Dens.Value * (1.0 + UsedCoef.K18.Value * dTemp      ) + UsedCoef.K19.Value * dTemp;
	Dens.Value = Dens.Value * (1.0 + K20                * Pres.Value ) + K21                * Pres.Value;



	// Приведение плотности к 15 градусам
	// по ГОСТ Р 50.2.076-2010
	LREAL OldD15 = 0.0;
	LREAL D15_2  = 0.0;
	UDINT count_iteration = 0;
	UDINT count_product   = 0;
	USINT  oil_id         = GetTypeProduct(Dens.Value, product);

	while(count_product < 3)
	{
		dTemp        = Temp.Value - 15.0;
		OldD15       = -100000.0;
		Dens15.Value = Dens.Value;
		K0           = Dens_K0[oil_id];
		K1           = Dens_K1[oil_id];
		K2           = Dens_K2[oil_id];

		while(std::abs(OldD15 - Dens15.Value) > DENSITY15_CONVERGENCE)
		{
			D15_2        = Dens15.Value * Dens15.Value;
			OldD15       = Dens15.Value;
			B15.Value    = (K0 + K1 * Dens15.Value) / (D15_2) + K2;
			B.Value      = B15.Value + 1.6 * B15.Value * B15.Value * dTemp;
			Y15.Value    = 0.001 * exp(-1.62080 + 0.00021592 * 15.0       + (870960 + 4209.2 * 15.0      ) / D15_2);
			Y.Value      = 0.001 * exp(-1.62080 + 0.00021592 * Temp.Value + (870960 + 4209.2 * Temp.Value) / D15_2);
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
		if(oil_id == GetTypeProduct(Dens15.Value, product)) break;

		oil_id = GetTypeProduct(Dens15.Value, product);
		++count_product;
	}

	//  Проверка полученной плотности
	limit = Dens_Limit[0][product] <= Dens15.Value && Dens15.Value < Dens_Limit[1][product];
	if(CheckExpr(!limit, DENSSOL_LE_VALUE, ReinitEvent(event_f, EID_DENSSOL_FAULT_VALUE) << Dens15.Value, ReinitEvent(event_s, EID_DENSSOL_GOOD_VALUE ) << Dens15.Value))
	{
		Fault = 1;
	}

	// Расчитаем плотность при 20 *С
	Dens20.Value = GetDens20(Dens15.Value, B15.Value);

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
	list.add(Alias + ".K0"             , TYPE_LREAL, rVariable::Flags::R___, &K0                 , U_DIMLESS, 0);
	list.add(Alias + ".K1"             , TYPE_LREAL, rVariable::Flags::R___, &K1                 , U_DIMLESS, 0);
	list.add(Alias + ".K2"             , TYPE_LREAL, rVariable::Flags::R___, &K2                 , U_DIMLESS, 0);
	list.add(Alias + ".SetFactor.K0"   , TYPE_LREAL, rVariable::Flags::___L, &Coef.K0.Value      , U_COEFSOL, ACCESS_FACTORS);
	list.add(Alias + ".SetFactor.K1"   , TYPE_LREAL, rVariable::Flags::___L, &Coef.K1.Value      , U_COEFSOL, ACCESS_FACTORS);
	list.add(Alias + ".SetFactor.K2"   , TYPE_LREAL, rVariable::Flags::___L, &Coef.K2.Value      , U_COEFSOL, ACCESS_FACTORS);
	list.add(Alias + ".SetFactor.K18"  , TYPE_LREAL, rVariable::Flags::___L, &Coef.K18.Value     , U_COEFSOL, ACCESS_FACTORS);
	list.add(Alias + ".SetFactor.K19"  , TYPE_LREAL, rVariable::Flags::___L, &Coef.K19.Value     , U_COEFSOL, ACCESS_FACTORS);
	list.add(Alias + ".SetFactor.K20A" , TYPE_LREAL, rVariable::Flags::___L, &Coef.K20A.Value    , U_COEFSOL, ACCESS_FACTORS);
	list.add(Alias + ".SetFactor.K20B" , TYPE_LREAL, rVariable::Flags::___L, &Coef.K20B.Value    , U_COEFSOL, ACCESS_FACTORS);
	list.add(Alias + ".SetFactor.K21A" , TYPE_LREAL, rVariable::Flags::___L, &Coef.K21A.Value    , U_COEFSOL, ACCESS_FACTORS);
	list.add(Alias + ".SetFactor.K21B" , TYPE_LREAL, rVariable::Flags::___L, &Coef.K21B.Value    , U_COEFSOL, ACCESS_FACTORS);
	list.add(Alias + ".Factor.K0"      , TYPE_LREAL, rVariable::Flags::RS__, &UsedCoef.K0.Value  , U_COEFSOL, ACCESS_SA);
	list.add(Alias + ".Factor.K1"      , TYPE_LREAL, rVariable::Flags::RS__, &UsedCoef.K1.Value  , U_COEFSOL, ACCESS_SA);
	list.add(Alias + ".Factor.K2"      , TYPE_LREAL, rVariable::Flags::RS__, &UsedCoef.K2.Value  , U_COEFSOL, ACCESS_SA);
	list.add(Alias + ".Factor.K18"     , TYPE_LREAL, rVariable::Flags::RS__, &UsedCoef.K18.Value , U_COEFSOL, ACCESS_SA);
	list.add(Alias + ".Factor.K19"     , TYPE_LREAL, rVariable::Flags::RS__, &UsedCoef.K19.Value , U_COEFSOL, ACCESS_SA);
	list.add(Alias + ".Factor.K20A"    , TYPE_LREAL, rVariable::Flags::RS__, &UsedCoef.K20A.Value, U_COEFSOL, ACCESS_SA);
	list.add(Alias + ".Factor.K20B"    , TYPE_LREAL, rVariable::Flags::RS__, &UsedCoef.K20B.Value, U_COEFSOL, ACCESS_SA);
	list.add(Alias + ".Factor.K21A"    , TYPE_LREAL, rVariable::Flags::RS__, &UsedCoef.K21A.Value, U_COEFSOL, ACCESS_SA);
	list.add(Alias + ".Factor.K21B"    , TYPE_LREAL, rVariable::Flags::RS__, &UsedCoef.K21B.Value, U_COEFSOL, ACCESS_SA);
	list.add(Alias + ".Calibration"    , TYPE_LREAL, rVariable::Flags::___L, &Calibr.Value       , U_C      , ACCESS_FACTORS);
	list.add(Alias + ".AcceptFactor"   , TYPE_USINT, rVariable::Flags::___L, &Accept             , U_DIMLESS, ACCESS_FACTORS);
	list.add(Alias + ".Setup"          , TYPE_UINT , rVariable::Flags::RS__, &Setup.Value        , U_DIMLESS, ACCESS_FACTORS);

	list.add(Alias + ".fault"          , TYPE_UDINT, rVariable::Flags::R___, &Fault              , U_DIMLESS, 0);

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDensSol::LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg)
{
//	string defSetup = rDataConfig::GetFlagNameByBit  (rDataConfig::AISetupFlags, AISETUP_OFF);
//	string strSetup = (element->Attribute("setup")) ? element->Attribute("setup") : defSetup.c_str();
	UDINT  err = 0;

	if(tinyxml2::XML_SUCCESS != rSource::LoadFromXML(element, cfg)) return DATACFGERR_DENSSOL;

	tinyxml2::XMLElement *koef   = element->FirstChildElement(XmlName::FACTORS);
	tinyxml2::XMLElement *temp   = element->FirstChildElement(XmlName::TEMP);
	tinyxml2::XMLElement *pres   = element->FirstChildElement(XmlName::PRES);
	tinyxml2::XMLElement *period = element->FirstChildElement(XmlName::PERIOD);

	if(!koef || !temp || !pres || !period)
	{
		return DATACFGERR_DENSSOL;
	}

	// Обязательные линки и параметры, без которых работа не возможна
	if(tinyxml2::XML_SUCCESS != cfg.LoadLink(temp->FirstChildElement  (XmlName::LINK), Temp  )) return cfg.ErrorID;
	if(tinyxml2::XML_SUCCESS != cfg.LoadLink(pres->FirstChildElement  (XmlName::LINK), Pres  )) return cfg.ErrorID;
	if(tinyxml2::XML_SUCCESS != cfg.LoadLink(period->FirstChildElement(XmlName::LINK), Period)) return cfg.ErrorID;

	Coef.K0.Init  (rDataConfig::GetTextLREAL(koef->FirstChildElement("k0")  , 0.0, err));
	Coef.K1.Init  (rDataConfig::GetTextLREAL(koef->FirstChildElement("k1")  , 0.0, err));
	Coef.K2.Init  (rDataConfig::GetTextLREAL(koef->FirstChildElement("k2")  , 0.0, err));
	Coef.K18.Init (rDataConfig::GetTextLREAL(koef->FirstChildElement("k18") , 0.0, err));
	Coef.K19.Init (rDataConfig::GetTextLREAL(koef->FirstChildElement("k19") , 0.0, err));
	Coef.K20A.Init(rDataConfig::GetTextLREAL(koef->FirstChildElement("k20a"), 0.0, err));
	Coef.K20B.Init(rDataConfig::GetTextLREAL(koef->FirstChildElement("k20b"), 0.0, err));
	Coef.K21A.Init(rDataConfig::GetTextLREAL(koef->FirstChildElement("k21a"), 0.0, err));
	Coef.K21B.Init(rDataConfig::GetTextLREAL(koef->FirstChildElement("k21b"), 0.0, err));

	if(err)
	{
		return DATACFGERR_DENSSOL;
	}

	// Не обязательный параметр
	Calibr.Init(rDataConfig::GetTextLREAL(element->FirstChildElement(XmlName::CALIBR), 20.0, err));
	err = 0;

	// Так как мы еще не загрузили данные из EEPROM, то принимает текущие коэф-ты за рабочие.
	UsedCoef = Coef;

	// Проверки
	if(nullptr == Station)
	{
		rEventManager::instance().Add(ReinitEvent(EID_DENSSOL_FAULT_STATION));
		return DATACFGERR_DENSSOL_NOSTN;
	}

	ReinitLimitEvents();

	return tinyxml2::XML_SUCCESS;
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




