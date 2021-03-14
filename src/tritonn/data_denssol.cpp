//=================================================================================================
//===
//=== data_denssol.cpp
//===
//=== Copyright (c) 2019-2021 by RangeSoft.
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
#include "comment_defines.h"


const UDINT DENSSOL_LE_PERIOD    = 0x00000001;
const UDINT DENSSOL_LE_INPUTS    = 0x00000002;
const UDINT DENSSOL_LE_VALUE     = 0x00000004;
const UDINT DENSSOL_LE_ITERATION = 0x00000010;


///////////////////////////////////////////////////////////////////////////////////////////////////
//
rDensSol::rDensSol(const rStation* owner) : rSource(owner), m_setup(0)
{
	m_lockErr = 0;
	m_calibrT = 20.0;
//	Setup     = DNSSOL_SETUP_OFF;

	// Настройка линков (входов)
	initLink(rLink::Setup::OUTPUT  , m_dens  , U_kg_m3  , SID::DENSITY    , XmlName::DENSITY  , rLink::SHADOW_NONE);//, "Вычисленная плотность в текущих условиях");
	initLink(rLink::Setup::INOUTPUT, m_temp  , U_C      , SID::TEMPERATURE, XmlName::TEMP     , rLink::SHADOW_NONE);//, "Температура измрения плотности");
	initLink(rLink::Setup::INOUTPUT, m_pres  , U_bar    , SID::PRESSURE   , XmlName::PRES     , rLink::SHADOW_NONE);//, "Давление измерения плотности");
	initLink(rLink::Setup::INPUT   , m_period, U_mksec  , SID::PERIOD     , XmlName::PERIOD   , rLink::SHADOW_NONE);//, "Период плотномера");
	initLink(rLink::Setup::OUTPUT  , m_dens15, U_kg_m3  , SID::DENSITY15  , XmlName::DENSITY15, rLink::SHADOW_NONE);//, "Вычисленная плотность при 15 °C");
	initLink(rLink::Setup::OUTPUT  , m_dens20, U_kg_m3  , SID::DENSITY20  , XmlName::DENSITY20, rLink::SHADOW_NONE);//, "Вычисленная плотность при 20 °C");
	initLink(rLink::Setup::OUTPUT  , m_b     , U_1_C    , SID::B          , XmlName::B        , rLink::SHADOW_NONE);//, "Вычисленный коффициент объемного расширения в текущих условиях");
	initLink(rLink::Setup::OUTPUT  , m_b15   , U_1_C    , SID::B15        , XmlName::B15      , rLink::SHADOW_NONE);//, "Вычисленный коффициент объемного расширения при 15 °C");
	initLink(rLink::Setup::OUTPUT  , m_y     , U_1_MPa  , SID::Y          , XmlName::Y        , rLink::SHADOW_NONE);//, "Вычисленный коэффициент сжимаемости в текущих условиях");
	initLink(rLink::Setup::OUTPUT  , m_y15   , U_1_MPa  , SID::Y15        , XmlName::Y15      , rLink::SHADOW_NONE);//, "Вычисленный коэффициент сжимаемости при 15 °C");
	initLink(rLink::Setup::OUTPUT  , m_ctl   , U_DIMLESS, SID::CTL        , XmlName::CTL      , rLink::SHADOW_NONE);//, "Вычисленный коффицинт влияния температуры");
	initLink(rLink::Setup::OUTPUT  , m_cpl   , U_DIMLESS, SID::CPL        , XmlName::CPL      , rLink::SHADOW_NONE);//, "Вычисленный коффицинт влияния давления");
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDensSol::initLimitEvent(rLink& link)
{
	link.m_limit.EventChangeAMin  = reinitEvent(EID_DENSSOL_NEW_AMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMin  = reinitEvent(EID_DENSSOL_NEW_WMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMax  = reinitEvent(EID_DENSSOL_NEW_WMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeAMax  = reinitEvent(EID_DENSSOL_NEW_AMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeHyst  = reinitEvent(EID_DENSSOL_NEW_HYST)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeSetup = reinitEvent(EID_DENSSOL_NEW_SETUP) << link.m_descr << link.m_unit;
	link.m_limit.EventAMin        = reinitEvent(EID_DENSSOL_AMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMin        = reinitEvent(EID_DENSSOL_WMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMax        = reinitEvent(EID_DENSSOL_WMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventAMax        = reinitEvent(EID_DENSSOL_AMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventNan         = reinitEvent(EID_DENSSOL_NAN)       << link.m_descr << link.m_unit;
	link.m_limit.EventNormal      = reinitEvent(EID_DENSSOL_NORMAL)    << link.m_descr << link.m_unit;

	return 0;
}


//-------------------------------------------------------------------------------------------------
//


//-------------------------------------------------------------------------------------------------
//
UDINT rDensSol::calculate()
{
	rEvent event_f;
	rEvent event_s;
	UDINT  err    = 0;

	if (rSource::calculate()) {
		return 0;
	}

	//-------------------------------------------------------------------------------------------
	// Обработка ввода пользователя
	m_calibrT.Compare(COMPARE_LREAL_PREC, reinitEvent(EID_DENSSOL_CALIBR));
	m_setCoef.K0.Compare  (COMPARE_LREAL_PREC, reinitEvent(EID_DENSSOL_K0    ));
	m_setCoef.K1.Compare  (COMPARE_LREAL_PREC, reinitEvent(EID_DENSSOL_K1    ));
	m_setCoef.K2.Compare  (COMPARE_LREAL_PREC, reinitEvent(EID_DENSSOL_K2    ));
	m_setCoef.K18.Compare (COMPARE_LREAL_PREC, reinitEvent(EID_DENSSOL_K18   ));
	m_setCoef.K19.Compare (COMPARE_LREAL_PREC, reinitEvent(EID_DENSSOL_K19   ));
	m_setCoef.K20A.Compare(COMPARE_LREAL_PREC, reinitEvent(EID_DENSSOL_K20A  ));
	m_setCoef.K20B.Compare(COMPARE_LREAL_PREC, reinitEvent(EID_DENSSOL_K20B  ));
	m_setCoef.K21A.Compare(COMPARE_LREAL_PREC, reinitEvent(EID_DENSSOL_K21A  ));
	m_setCoef.K21B.Compare(COMPARE_LREAL_PREC, reinitEvent(EID_DENSSOL_K21B  ));

	// Если введены новые коэффициенты
	if (m_accept) {
		m_curCoef = m_setCoef;
		m_accept  = 0;

		rEventManager::instance().Add(reinitEvent(EID_DENSSOL_ACCEPT));
	}

	if (checkExpr(err, DENSSOL_LE_INPUTS,
				  event_f.Reinit(EID_DENSSOL_FAULT_INPUTS) << m_ID << m_descr,
				  event_s.Reinit(EID_DENSSOL_GOOD_INPUTS ) << m_ID << m_descr)) {
		return setFault();
	}

	//
	if (checkExpr(m_period.m_value < 1, DENSSOL_LE_PERIOD,
				  event_f.Reinit(EID_DENSSOL_FAULT_PERIOD) << m_ID << m_descr,
				  event_s.Reinit(EID_DENSSOL_GOOD_PERIOD ) << m_ID << m_descr)) {
		return setFault();
	}

	// Расчет плотности
	rDensity::Product product = m_station->m_product;
	LREAL dTemp = m_temp.m_value - m_calibrT.Value;
	LREAL K20   = m_curCoef.K20A.Value + m_curCoef.K20B.Value * m_pres.m_value;
	LREAL K21   = m_curCoef.K21A.Value + m_curCoef.K21B.Value * m_pres.m_value;
	UDINT limit = 0;

	m_dens.m_value = m_curCoef.K0.Value + m_curCoef.K1.Value * m_period.m_value + m_curCoef.K2.Value * m_period.m_value * m_period.m_value;
	m_dens.m_value = m_dens.m_value * (1.0 + m_curCoef.K18.Value * dTemp         ) + m_curCoef.K19.Value * dTemp;
	m_dens.m_value = m_dens.m_value * (1.0 + K20                 * m_pres.m_value) + K21                 * m_pres.m_value;

	// Приведение плотности к 15 градусам
	// по ГОСТ Р 50.2.076-2010
	LREAL OldD15 = 0.0;
	LREAL D15_2  = 0.0;
	UDINT count_iteration = 0;
	UDINT count_product   = 0;
	USINT oil_id          = rDensity::getTypeProduct(m_dens.m_value, product);

	while (count_product < 3) {
		dTemp            = m_temp.m_value - 15.0;
		OldD15           = -100000.0;
		m_dens15.m_value = m_dens.m_value;
		m_k0             = rDensity::K0[oil_id];
		m_k1             = rDensity::K1[oil_id];
		m_k2             = rDensity::K2[oil_id];

		while (std::abs(OldD15 - m_dens15.m_value) > rDensity::CONVERGENCE) {
			D15_2            = m_dens15.m_value * m_dens15.m_value;
			OldD15           = m_dens15.m_value;
			m_b15.m_value    = (m_k0 + m_k1 * m_dens15.m_value) / (D15_2) + m_k2;
			m_b.m_value      = m_b15.m_value + 1.6 * m_b15.m_value * m_b15.m_value * dTemp;
			m_y15.m_value    = rDensity::getY15(m_dens15.m_value);
			m_y.m_value      = rDensity::getY  (m_dens15.m_value, m_temp.m_value);
			m_cpl.m_value    = 1.0 / (1.0 - m_y.m_value * m_pres.m_value);
			m_ctl.m_value    = exp(-m_b15.m_value * dTemp * (1 + 0.8 * m_b15.m_value * dTemp));
			m_dens15.m_value = (m_dens.m_value / m_cpl.m_value) / m_ctl.m_value;

			if(++count_iteration >= 20) break;
		}

		// Проверка полученной плотности
		if(checkExpr(count_iteration >= 20, DENSSOL_LE_ITERATION,
					 event_f.Reinit(EID_DENSSOL_FAULT_ITERATION) << m_ID << m_descr,
					 event_s.Reinit(EID_DENSSOL_GOOD_ITERATION ) << m_ID << m_descr)) {

			return setFault();
		}

		// Для бензинов, определяем куда попали по плотности, если
		// тип изменился, то считаем еще раз.
		if (oil_id == rDensity::getTypeProduct(m_dens15.m_value, product)) {
			break;
		}

		oil_id = rDensity::getTypeProduct(m_dens15.m_value, product);
		++count_product;
	}

	//  Проверка полученной плотности
	USINT product_id = static_cast<USINT>(product);
	limit = rDensity::Limit[0][product_id] <= m_dens15.m_value && m_dens15.m_value < rDensity::Limit[1][product_id];

	if(checkExpr(!limit, DENSSOL_LE_VALUE,
				 reinitEvent(event_f, EID_DENSSOL_FAULT_VALUE) << m_dens15.m_value,
				 reinitEvent(event_s, EID_DENSSOL_GOOD_VALUE ) << m_dens15.m_value)) {

		m_fault = 1;
	}

	// Расчитаем плотность при 20 *С
	m_dens20.m_value = rDensity::getDens20(m_dens15.m_value, m_b15.m_value);

	postCalculate();

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDensSol::setFault()
{
	m_dens.m_value   = std::numeric_limits<LREAL>::quiet_NaN();;
	m_dens15.m_value = std::numeric_limits<LREAL>::quiet_NaN();;
	m_dens20.m_value = std::numeric_limits<LREAL>::quiet_NaN();;
	m_b.m_value      = std::numeric_limits<LREAL>::quiet_NaN();;
	m_y.m_value      = std::numeric_limits<LREAL>::quiet_NaN();;
	m_ctl.m_value    = std::numeric_limits<LREAL>::quiet_NaN();;
	m_cpl.m_value    = std::numeric_limits<LREAL>::quiet_NaN();;
	m_b15.m_value    = std::numeric_limits<LREAL>::quiet_NaN();;
	m_y15.m_value    = std::numeric_limits<LREAL>::quiet_NaN();;
	m_k0             = std::numeric_limits<LREAL>::quiet_NaN();;
	m_k1             = std::numeric_limits<LREAL>::quiet_NaN();;
	m_k2             = std::numeric_limits<LREAL>::quiet_NaN();;
	m_fault          = 1;

	return m_fault;
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
	list.add(m_alias + ".k0"               , TYPE_LREAL, rVariable::Flags::R__, &m_k0                , U_DIMLESS, 0             , COMMENT::CALC_COEF + " K0");
	list.add(m_alias + ".k1"               , TYPE_LREAL, rVariable::Flags::R__, &m_k1                , U_DIMLESS, 0             , COMMENT::CALC_COEF + " K1");
	list.add(m_alias + ".k2"               , TYPE_LREAL, rVariable::Flags::R__, &m_k2                , U_DIMLESS, 0             , COMMENT::CALC_COEF + " K2");
	list.add(m_alias + ".factor.k0"        , TYPE_LREAL, rVariable::Flags::RS_, &m_curCoef.K0.Value  , U_COEFSOL, ACCESS_SA     , COMMENT::FACTOR + " K0");
	list.add(m_alias + ".factor.k1"        , TYPE_LREAL, rVariable::Flags::RS_, &m_curCoef.K1.Value  , U_COEFSOL, ACCESS_SA     , COMMENT::FACTOR + " K1");
	list.add(m_alias + ".factor.k2"        , TYPE_LREAL, rVariable::Flags::RS_, &m_curCoef.K2.Value  , U_COEFSOL, ACCESS_SA     , COMMENT::FACTOR + " K2");
	list.add(m_alias + ".factor.k18"       , TYPE_LREAL, rVariable::Flags::RS_, &m_curCoef.K18.Value , U_COEFSOL, ACCESS_SA     , COMMENT::FACTOR + " K18");
	list.add(m_alias + ".factor.k19"       , TYPE_LREAL, rVariable::Flags::RS_, &m_curCoef.K19.Value , U_COEFSOL, ACCESS_SA     , COMMENT::FACTOR + " K19");
	list.add(m_alias + ".factor.k20a"      , TYPE_LREAL, rVariable::Flags::RS_, &m_curCoef.K20A.Value, U_COEFSOL, ACCESS_SA     , COMMENT::FACTOR + " K20A");
	list.add(m_alias + ".factor.k20b"      , TYPE_LREAL, rVariable::Flags::RS_, &m_curCoef.K20B.Value, U_COEFSOL, ACCESS_SA     , COMMENT::FACTOR + " K20B");
	list.add(m_alias + ".factor.k21a"      , TYPE_LREAL, rVariable::Flags::RS_, &m_curCoef.K21A.Value, U_COEFSOL, ACCESS_SA     , COMMENT::FACTOR + " K21A");
	list.add(m_alias + ".factor.k21b"      , TYPE_LREAL, rVariable::Flags::RS_, &m_curCoef.K21B.Value, U_COEFSOL, ACCESS_SA     , COMMENT::FACTOR + " K21B");
	list.add(m_alias + ".factor.set.k0"    , TYPE_LREAL, rVariable::Flags::___, &m_setCoef.K0.Value  , U_COEFSOL, ACCESS_FACTORS, COMMENT::FACTOR_SET + " K0");
	list.add(m_alias + ".factor.set.k1"    , TYPE_LREAL, rVariable::Flags::___, &m_setCoef.K1.Value  , U_COEFSOL, ACCESS_FACTORS, COMMENT::FACTOR_SET + " K1");
	list.add(m_alias + ".factor.set.k2"    , TYPE_LREAL, rVariable::Flags::___, &m_setCoef.K2.Value  , U_COEFSOL, ACCESS_FACTORS, COMMENT::FACTOR_SET + " K2");
	list.add(m_alias + ".factor.set.k18"   , TYPE_LREAL, rVariable::Flags::___, &m_setCoef.K18.Value , U_COEFSOL, ACCESS_FACTORS, COMMENT::FACTOR_SET + " K18");
	list.add(m_alias + ".factor.set.k19"   , TYPE_LREAL, rVariable::Flags::___, &m_setCoef.K19.Value , U_COEFSOL, ACCESS_FACTORS, COMMENT::FACTOR_SET + " K19");
	list.add(m_alias + ".factor.set.k20a"  , TYPE_LREAL, rVariable::Flags::___, &m_setCoef.K20A.Value, U_COEFSOL, ACCESS_FACTORS, COMMENT::FACTOR_SET + " K20A");
	list.add(m_alias + ".factor.set.k20b"  , TYPE_LREAL, rVariable::Flags::___, &m_setCoef.K20B.Value, U_COEFSOL, ACCESS_FACTORS, COMMENT::FACTOR_SET + " K20B");
	list.add(m_alias + ".factor.set.k21a"  , TYPE_LREAL, rVariable::Flags::___, &m_setCoef.K21A.Value, U_COEFSOL, ACCESS_FACTORS, COMMENT::FACTOR_SET + " K21A");
	list.add(m_alias + ".factor.set.k21b"  , TYPE_LREAL, rVariable::Flags::___, &m_setCoef.K21B.Value, U_COEFSOL, ACCESS_FACTORS, COMMENT::FACTOR_SET + " K21B");
	list.add(m_alias + ".factor.set.accept", TYPE_USINT, rVariable::Flags::___, &m_accept            , U_DIMLESS, ACCESS_FACTORS, COMMENT::FACTOR_ACC);
	list.add(m_alias + ".Calibration"      , TYPE_LREAL, rVariable::Flags::___, &m_calibrT.Value     , U_C      , ACCESS_FACTORS, "Значение температуры калибровки");
//	list.add(m_alias + ".Setup"            , TYPE_UINT , rVariable::Flags::RS_, &m_setup.Value       , U_DIMLESS, ACCESS_FACTORS);

	list.add(m_alias + ".fault"            , TYPE_UDINT, rVariable::Flags::R__, &m_fault             , U_DIMLESS, 0             , COMMENT::FAULT);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDensSol::loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	if (TRITONN_RESULT_OK != rSource::loadFromXML(element, err, prefix)) {
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
	if(TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_temp->FirstChildElement  (XmlName::LINK), m_temp  )) return err.getError();
	if(TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_pres->FirstChildElement  (XmlName::LINK), m_pres  )) return err.getError();
	if(TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_period->FirstChildElement(XmlName::LINK), m_period)) return err.getError();

	UDINT fault = 0;
	m_setCoef.K0.Init  (XmlUtils::getTextLREAL(xml_koef->FirstChildElement(XmlName::K0)  , 0.0, fault));
	m_setCoef.K1.Init  (XmlUtils::getTextLREAL(xml_koef->FirstChildElement(XmlName::K1)  , 0.0, fault));
	m_setCoef.K2.Init  (XmlUtils::getTextLREAL(xml_koef->FirstChildElement(XmlName::K2)  , 0.0, fault));
	m_setCoef.K18.Init (XmlUtils::getTextLREAL(xml_koef->FirstChildElement(XmlName::K18) , 0.0, fault));
	m_setCoef.K19.Init (XmlUtils::getTextLREAL(xml_koef->FirstChildElement(XmlName::K19) , 0.0, fault));
	m_setCoef.K20A.Init(XmlUtils::getTextLREAL(xml_koef->FirstChildElement(XmlName::K20A), 0.0, fault));
	m_setCoef.K20B.Init(XmlUtils::getTextLREAL(xml_koef->FirstChildElement(XmlName::K20B), 0.0, fault));
	m_setCoef.K21A.Init(XmlUtils::getTextLREAL(xml_koef->FirstChildElement(XmlName::K21A), 0.0, fault));
	m_setCoef.K21B.Init(XmlUtils::getTextLREAL(xml_koef->FirstChildElement(XmlName::K21B), 0.0, fault));

	if (fault) {
		return err.set(DATACFGERR_DENSSOL, xml_koef->GetLineNum(), "");
	}

	// Не обязательный параметр
	m_calibrT.Init(XmlUtils::getTextLREAL(element->FirstChildElement(XmlName::CALIBR), 20.0, fault));

	fault = 0;

	// Так как мы еще не загрузили данные из EEPROM, то принимает текущие коэф-ты за рабочие.
	m_curCoef = m_setCoef;

	// Проверки
	if (!m_station) {
		rEventManager::instance().Add(reinitEvent(EID_DENSSOL_FAULT_STATION));
		return err.set(DATACFGERR_DENSSOL_NOSTN, element->GetLineNum(), "station is empty");
	}

	reinitLimitEvents();

	return TRITONN_RESULT_OK;
}


UDINT rDensSol::generateMarkDown(rGeneratorMD& md)
{
	m_dens.m_limit.m_setup.Init  (LIMIT_SETUP_ALL);
	m_temp.m_limit.m_setup.Init  (LIMIT_SETUP_ALL);
	m_pres.m_limit.m_setup.Init  (LIMIT_SETUP_ALL);
	m_period.m_limit.m_setup.Init(LIMIT_SETUP_ALL);
	m_dens15.m_limit.m_setup.Init(LIMIT_SETUP_ALL);
	m_dens20.m_limit.m_setup.Init(LIMIT_SETUP_ALL);
	m_b.m_limit.m_setup.Init     (LIMIT_SETUP_ALL);
	m_b15.m_limit.m_setup.Init   (LIMIT_SETUP_ALL);
	m_y.m_limit.m_setup.Init     (LIMIT_SETUP_ALL);
	m_y15.m_limit.m_setup.Init   (LIMIT_SETUP_ALL);
	m_ctl.m_limit.m_setup.Init   (LIMIT_SETUP_ALL);
	m_cpl.m_limit.m_setup.Init   (LIMIT_SETUP_ALL);

	md.add(this, true, rGeneratorMD::Type::CALCULATE)
			.addXml(XmlName::CALIBR, m_calibrT.Value)
			.addXml(String_format("<%s>", XmlName::FACTORS))
			.addXml(XmlName::K0  , m_setCoef.K0.Value, false, "\t")
			.addXml(XmlName::K1  , m_setCoef.K1.Value, false, "\t")
			.addXml(XmlName::K2  , m_setCoef.K2.Value, false, "\t")
			.addXml(XmlName::K18 , m_setCoef.K18.Value, false, "\t")
			.addXml(XmlName::K19 , m_setCoef.K19.Value, false, "\t")
			.addXml(XmlName::K20A, m_setCoef.K20A.Value, false, "\t")
			.addXml(XmlName::K20B, m_setCoef.K20B.Value, false, "\t")
			.addXml(XmlName::K21A, m_setCoef.K21A.Value, false, "\t")
			.addXml(XmlName::K21B, m_setCoef.K21B.Value, false, "\t")
			.addXml(String_format("</%s>", XmlName::FACTORS));

	return TRITONN_RESULT_OK;
}


