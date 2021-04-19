//=================================================================================================
//===
//=== data_stream.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс измерительной линии
//===
//=================================================================================================

#include <vector>
#include <limits>
#include <string.h>
#include "event/eid.h"
#include "event/manager.h"
#include "text_id.h"
#include "xml_util.h"
#include "precision.h"
#include "data_manager.h"
#include "data_config.h"
#include "variable_list.h"
#include "data_station.h"
#include "data_stream.h"
#include "data_snapshot.h"
#include "io/defines.h"
#include "generator_md.h"
#include "comment_defines.h"


const UDINT STREAM_LE_ACCOUNTING = 0x00000002;
const UDINT STREAM_LE_KEYPADKF   = 0x00000004;


rBitsArray rStream::m_flagsFlowmeter;
rBitsArray rStream::m_flagsSetup;

std::string rFlowFactor::FOOTNOTE = "[^maxpoint]";

///////////////////////////////////////////////////////////////////////////////////////////////////
//
rStream::rStream(const rStation* owner) :
	rSource(owner),
	m_setup(Setup::OFF),
	m_total(reinitEvent(EID_STREAM_TOTAL_MASS),
			reinitEvent(EID_STREAM_TOTAL_VOLUME),
			reinitEvent(EID_STREAM_TOTAL_VOLUME15),
			reinitEvent(EID_STREAM_TOTAL_VOLUME20))
{
	if (m_flagsFlowmeter.empty()) {
		m_flagsFlowmeter
				.add("TURBINE"   , static_cast<UINT>(Type::TURBINE)   , "Турбинный ПР")
				.add("CORIOLIS"  , static_cast<UINT>(Type::CORIOLIS)  , "Кориолисовый ПР")
				.add("ULTRASONIC", static_cast<UINT>(Type::ULTRASONIC), "Ультрозвуковой ПР");
	}

	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("", static_cast<UINT>(Setup::OFF), COMMENT::SETUP_OFF);
	}

	m_maintenance   = true;
	m_linearization = false;
//	rTotal   Total;

	initLink(rLink::Setup::INPUT   , m_counter     , U_imp   , SID::IMPULSE          , XmlName::IMPULSE      , rLink::SHADOW_NONE);
	initLink(rLink::Setup::INPUT   , m_freq        , U_Hz    , SID::FREQUENCY        , XmlName::FREQ         , XmlName::IMPULSE);
	initLink(rLink::Setup::INOUTPUT, m_temp        , U_C     , SID::TEMPERATURE      , XmlName::TEMP         , rLink::SHADOW_NONE);
	initLink(rLink::Setup::INOUTPUT, m_pres        , U_MPa   , SID::PRESSURE         , XmlName::PRES         , rLink::SHADOW_NONE);
	initLink(rLink::Setup::INOUTPUT, m_dens        , U_kg_m3 , SID::DENSITY          , XmlName::DENSITY      , rLink::SHADOW_NONE);
	initLink(rLink::Setup::INOUTPUT, m_dens15      , U_kg_m3 , SID::DENSITY15        , XmlName::DENSITY15    , XmlName::DENSITY);
	initLink(rLink::Setup::INOUTPUT, m_dens20      , U_kg_m3 , SID::DENSITY20        , XmlName::DENSITY20    , XmlName::DENSITY);
	initLink(rLink::Setup::INOUTPUT, m_b15         , U_1_C   , SID::B15              , XmlName::B15          , XmlName::DENSITY);
	initLink(rLink::Setup::INOUTPUT, m_y15         , U_1_MPa , SID::Y15              , XmlName::Y15          , XmlName::DENSITY);
	initLink(rLink::Setup::OUTPUT  , m_flowMass    , U_t_h   , SID::FLOWRATE_MASS    , XmlName::FLOWRATEMASS , rLink::SHADOW_NONE);
	initLink(rLink::Setup::OUTPUT  , m_flowVolume  , U_m3_h  , SID::FLOWRATE_VOLUME  , XmlName::FLOWRATEVOL  , rLink::SHADOW_NONE);
	initLink(rLink::Setup::OUTPUT  , m_flowVolume15, U_m3_h  , SID::FLOWRATE_VOLUME15, XmlName::FLOWRATEVOL15, rLink::SHADOW_NONE);
	initLink(rLink::Setup::OUTPUT  , m_flowVolume20, U_m3_h  , SID::FLOWRATE_VOLUME20, XmlName::FLOWRATEVOL20, rLink::SHADOW_NONE);
}


//-------------------------------------------------------------------------------------------------
//
UDINT rStream::initLimitEvent(rLink& link)
{
	link.m_limit.EventChangeAMin  = reinitEvent(EID_STREAM_NEW_AMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMin  = reinitEvent(EID_STREAM_NEW_WMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMax  = reinitEvent(EID_STREAM_NEW_WMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeAMax  = reinitEvent(EID_STREAM_NEW_AMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeHyst  = reinitEvent(EID_STREAM_NEW_HYST)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeSetup = reinitEvent(EID_STREAM_NEW_SETUP) << link.m_descr << link.m_unit;
	link.m_limit.EventAMin        = reinitEvent(EID_STREAM_AMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMin        = reinitEvent(EID_STREAM_WMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMax        = reinitEvent(EID_STREAM_WMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventAMax        = reinitEvent(EID_STREAM_AMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventNan         = reinitEvent(EID_STREAM_NAN)       << link.m_descr << link.m_unit;
	link.m_limit.EventNormal      = reinitEvent(EID_STREAM_NORMAL)    << link.m_descr << link.m_unit;

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rStream::calculate()
{
	if (rSource::calculate()) {
		return TRITONN_RESULT_OK;
	}

	//
	m_setFactor.KeypadKF.Compare(reinitEvent(EID_STREAM_KEYPADKF));
	m_setFactor.KeypadMF.Compare(reinitEvent(EID_STREAM_KEYPADMF));

	for (auto& point : m_setFactor.m_point) {
		point.Hz.Compare(reinitEvent(EID_STREAM_POINT_HZ) << rStringID(U_Hz) << (point.m_id + 1));
		point.Kf.Compare(reinitEvent(EID_STREAM_POINT_KF) << getUnitKF()     << (point.m_id + 1));
	}

	rEvent event_s;
	rEvent event_f;

	// Вывод линии из учета
	checkExpr(m_maintenance, STREAM_LE_ACCOUNTING,
			  event_f.reinit(EID_STREAM_ACCOUNTING_ON ) << m_ID << m_descr,
			  event_s.reinit(EID_STREAM_ACCOUNTING_OFF) << m_ID << m_descr);

	// Использование ручного ввода KF
	checkExpr(m_linearization, STREAM_LE_KEYPADKF,
			  event_f.reinit(EID_STREAM_KEYPADKF_OFF) << m_ID << m_descr,
			  event_s.reinit(EID_STREAM_KEYPADKF_ON ) << m_ID << m_descr);

	// Ввод новых коэф-тов
	if (m_acceptKF) {
		m_curFactor = m_setFactor;
		m_acceptKF  = 0;
		rEventManager::instance().add(reinitEvent(EID_STREAM_ACCEPT));
	}

	//-------------------------------------------------------------------------------------------
	// Расчет текущего коэф-та
	m_curKF = m_linearization ? calcualateKF() : m_curFactor.KeypadKF.Value;

	// Нарастающие и расход
	// Увеличим глобальный счетчик
	calcTotal();

	// Расчитаем расход исходя из частоты
	if (m_flowmeter == Type::CORIOLIS) {
		m_flowMass.m_value   = m_freq.m_value     / m_curKF        * 3600.0 * m_curFactor.KeypadMF.Value;
		m_flowVolume.m_value = m_flowMass.m_value / m_dens.m_value * 1000.0 * m_curFactor.KeypadMF.Value;
	} else {
		m_flowVolume.m_value = m_freq.m_value       / m_curKF        * 3600.0 * m_curFactor.KeypadMF.Value;
		m_flowMass.m_value   = m_flowVolume.m_value * m_dens.m_value / 1000.0 * m_curFactor.KeypadMF.Value;
	}

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
// Получение единиц измерения К-фактора (исходя из типа расходомера)
UDINT rStream::getUnitKF() const
{
	if (!m_station) {
		return U_UNDEF;
	}

	return m_flowmeter == Type::CORIOLIS ? m_station->getUnit().getMassKF() : m_station->getUnit().getVolumeKF();
}


UDINT rStream::enableFreqOut() const
{
	if (m_counter.isValid()) {
		std::string moduleAlias = m_counter.getModuleAlias();

		if (moduleAlias.size()) {
			rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_SYSTEM);

			ss.add(IO::HARWARE_PREFIX + moduleAlias + "." + IO::VARNAME_OUTTYPE, 0); //TODO Подставить нужное число
			ss.set();

			return TRITONN_RESULT_OK;
		}
	}

	return DATACFGERR_STREAM_NOFREQCHANNEL;
}

UDINT rStream::disableFreqOut() const
{
	if (m_counter.isValid()) {
		std::string moduleAlias = m_counter.getModuleAlias();

		if (moduleAlias.size()) {
			rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_SYSTEM);

			ss.add(IO::HARWARE_PREFIX + moduleAlias + "." + IO::VARNAME_OUTTYPE, m_counter.getChannelNumber() + 1); //TODO Подставить нужное число
			ss.set();

			return TRITONN_RESULT_OK;
		}
	}

	return DATACFGERR_STREAM_NOFREQCHANNEL;
}

//-------------------------------------------------------------------------------------------------
//
UDINT rStream::generateVars(rVariableList& list)
{
	auto unit = m_station->getUnit();
	std::string strtot = m_alias + ".total.";
	std::string strfac = m_alias + ".factors.";

	rSource::generateVars(list);

	// Внутренние переменные
	list.add(m_alias + ".setup"          ,             rVariable::Flags::RS__, &m_setup.Value             , U_DIMLESS       , ACCESS_SA     , COMMENT::SETUP + m_flagsSetup.getInfo());
	list.add(m_alias + ".flowmeter"      , TYPE_USINT, rVariable::Flags::R___, &m_flowmeter               , U_DIMLESS       , 0             , "Тип расходомера:<br/>" + m_flagsFlowmeter.getInfo(true));
	list.add(m_alias + ".maintenance"    ,             rVariable::Flags::___D, &m_maintenance             , U_DIMLESS       , ACCESS_MAINTENANCE, "Статус учета:<br/>0 - в учете<br/>1 - не в учете");
	list.add(m_alias + ".linearization"  ,             rVariable::Flags::___D, &m_linearization           , U_DIMLESS       , ACCESS_FACTORS, "Тип коэффициентов:<br/>0 - единый К-фактор<br/>1 - кусочно-линейная интерполяция");
	list.add(strtot  + "present.Volume"  ,             rVariable::Flags::R___, &m_total.Present.Volume    , unit.getVolume(), 0             , COMMENT::TOTAL_PRESENT + COMMENT::VOLUME);
	list.add(strtot  + "present.Volume15",             rVariable::Flags::R___, &m_total.Present.Volume15  , unit.getVolume(), 0             , COMMENT::TOTAL_PRESENT + COMMENT::VOLUME15);
	list.add(strtot  + "present.Volume20",             rVariable::Flags::R___, &m_total.Present.Volume20  , unit.getVolume(), 0             , COMMENT::TOTAL_PRESENT + COMMENT::VOLUME20);
	list.add(strtot  + "present.Mass"    ,             rVariable::Flags::R___, &m_total.Present.Mass      , unit.getMass()  , 0             , COMMENT::TOTAL_PRESENT + COMMENT::MASS);
	list.add(strtot  + "present.impulse" ,             rVariable::Flags::R___, &m_total.Present.Count     , U_imp           , 0             , COMMENT::TOTAL_PRESENT + COMMENT::IMP_COUNT);
	list.add(strtot  + "Inc.Volume"      ,             rVariable::Flags::RSH_, &m_total.Inc.Volume        , unit.getVolume(), ACCESS_SA     , COMMENT::TOTAL_INC     + COMMENT::VOLUME);
	list.add(strtot  + "Inc.Volume15"    ,             rVariable::Flags::RSH_, &m_total.Inc.Volume15      , unit.getVolume(), ACCESS_SA     , COMMENT::TOTAL_INC     + COMMENT::VOLUME15);
	list.add(strtot  + "Inc.Volume20"    ,             rVariable::Flags::RSH_, &m_total.Inc.Volume20      , unit.getVolume(), ACCESS_SA     , COMMENT::TOTAL_INC     + COMMENT::VOLUME20);
	list.add(strtot  + "Inc.Mass"        ,             rVariable::Flags::RSH_, &m_total.Inc.Mass          , unit.getMass()  , ACCESS_SA     , COMMENT::TOTAL_INC     + COMMENT::MASS);
	list.add(strtot  + "Inc.inpulse"     ,             rVariable::Flags::RSH_, &m_total.Inc.Count         , U_imp           , ACCESS_SA     , COMMENT::TOTAL_INC     + COMMENT::IMP_COUNT);
	list.add(strtot  + "raw.Volume"      ,             rVariable::Flags::RSH_, &m_total.Raw.Volume        , unit.getVolume(), 0             , COMMENT::TOTAL_RAW     + COMMENT::VOLUME);
	list.add(strtot  + "raw.Volume15"    ,             rVariable::Flags::RSH_, &m_total.Raw.Volume15      , unit.getVolume(), 0             , COMMENT::TOTAL_RAW     + COMMENT::VOLUME15);
	list.add(strtot  + "raw.Volume20"    ,             rVariable::Flags::RSH_, &m_total.Raw.Volume20      , unit.getVolume(), 0             , COMMENT::TOTAL_RAW     + COMMENT::VOLUME20);
	list.add(strtot  + "raw.Mass"        ,             rVariable::Flags::RSH_, &m_total.Raw.Mass          , unit.getMass()  , 0             , COMMENT::TOTAL_RAW     + COMMENT::MASS);
	list.add(strtot  + "raw.impulse"     ,             rVariable::Flags::RSH_, &m_total.Raw.Count         , U_imp           , 0             , COMMENT::TOTAL_RAW     + COMMENT::IMP_COUNT);
	list.add(strtot  + "past.Volume"     ,             rVariable::Flags::RSH_, &m_total.Past.Volume       , unit.getVolume(), 0             , COMMENT::TOTAL_PAST    + COMMENT::VOLUME);
	list.add(strtot  + "past.Volume15"   ,             rVariable::Flags::RSH_, &m_total.Past.Volume15     , unit.getVolume(), 0             , COMMENT::TOTAL_PAST    + COMMENT::VOLUME15);
	list.add(strtot  + "past.Volume20"   ,             rVariable::Flags::RSH_, &m_total.Past.Volume20     , unit.getVolume(), 0             , COMMENT::TOTAL_PAST    + COMMENT::VOLUME20);
	list.add(strtot  + "past.Mass"       ,             rVariable::Flags::RSH_, &m_total.Past.Mass         , unit.getMass()  , 0             , COMMENT::TOTAL_PAST    + COMMENT::MASS);
	list.add(strtot  + "past.impulse"    ,             rVariable::Flags::RSH_, &m_total.Past.Count        , U_imp           , 0             , COMMENT::TOTAL_PAST    + COMMENT::IMP_COUNT);
	list.add(m_alias + ".presentkf"      ,             rVariable::Flags::R___, &m_curKF                   , getUnitKF()     , 0             , "Вычисленный К-фактор");
	list.add(strfac  + "kf"              ,             rVariable::Flags::R__D, &m_curFactor.KeypadKF.Value, getUnitKF()     , 0             , COMMENT::FACTORS     + COMMENT::KFACTOR);
	list.add(strfac  + "mf"              ,             rVariable::Flags::R__D, &m_curFactor.KeypadMF.Value, U_DIMLESS       , 0             , COMMENT::FACTORS     + COMMENT::MFACTOR);
	list.add(strfac  + "set.kf"          ,             rVariable::Flags::____, &m_setFactor.KeypadKF.Value, getUnitKF()     , ACCESS_FACTORS, COMMENT::FACTORS_SET + COMMENT::KFACTOR);
	list.add(strfac  + "set.mf"          ,             rVariable::Flags::____, &m_setFactor.KeypadMF.Value, U_DIMLESS       , ACCESS_FACTORS, COMMENT::FACTORS_SET + COMMENT::MFACTOR);
	list.add(strfac  + "set.accept"      ,             rVariable::Flags::____, &m_acceptKF                , U_DIMLESS       , ACCESS_FACTORS, COMMENT::FACTOR_ACC);


	for (UDINT ii = 0; ii < m_curFactor.m_point.size(); ++ii) {
		std::string name_kf     = String_format("%s.factors.point_%i.kf"    , m_alias.c_str(), ii + 1);
		std::string name_hz     = String_format("%s.factors.point_%i.hz"    , m_alias.c_str(), ii + 1);
		std::string name_set_kf = String_format("%s.factors.set.point_%i.kf", m_alias.c_str(), ii + 1);
		std::string name_set_hz = String_format("%s.factors.set.point_%i.hz", m_alias.c_str(), ii + 1);
		std::string comment     = COMMENT::FACTORS     + COMMENT::POINT + String_format(" %u. ", ii + 1);
		std::string comment_set = COMMENT::FACTORS_SET + COMMENT::POINT + String_format(" %u. ", ii + 1);

		list.add(name_kf    , rVariable::Flags::R___, &m_curFactor.m_point[ii].Kf.Value, getUnitKF(), 0             , comment     + COMMENT::KFACTOR + rFlowFactor::FOOTNOTE);
		list.add(name_set_kf, rVariable::Flags::___D, &m_setFactor.m_point[ii].Kf.Value, getUnitKF(), ACCESS_FACTORS, comment_set + COMMENT::KFACTOR + rFlowFactor::FOOTNOTE);
		list.add(name_hz    , rVariable::Flags::R___, &m_curFactor.m_point[ii].Hz.Value, U_Hz       , 0             , comment     + COMMENT::FREQ + rFlowFactor::FOOTNOTE);
		list.add(name_set_hz, rVariable::Flags::___D, &m_setFactor.m_point[ii].Hz.Value, U_Hz       , ACCESS_FACTORS, comment_set + COMMENT::FREQ + rFlowFactor::FOOTNOTE);
	}

	list.add(m_alias + ".fault", rVariable::Flags::R___, &m_fault, U_DIMLESS, 0, COMMENT::FAULT);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rStream::loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strFlowMeter = XmlUtils::getAttributeString(element, XmlName::FLOWMETER, m_flagsFlowmeter.getNameByBits(static_cast<USINT>(Type::CORIOLIS)));

	if (!m_station) {
		return err.set(DATACFGERR_STREAM_NOSTN, element ? element->GetLineNum() : -1, "");
	}

	if (TRITONN_RESULT_OK != rSource::loadFromXML(element, err, prefix)) {
		return err.getError();
	}

	UDINT fault = 0;
	m_linearization = XmlUtils::getAttributeUSINT(element, XmlName::LINEARIZATION, 0);
	m_maintenance   = XmlUtils::getAttributeUSINT(element, XmlName::MAINTENANCE  , 0);
	m_flowmeter     = static_cast<Type>(m_flagsFlowmeter.getValue(strFlowMeter, fault));

	if (fault) {
		return err.set(DATACFGERR_STREAM, element->GetLineNum(), "undefened paramters");
	}

	auto xml_impulse = element->FirstChildElement(XmlName::IMPULSE);
	auto xml_freq    = element->FirstChildElement(XmlName::FREQ);
	auto xml_temp    = element->FirstChildElement(XmlName::TEMP);
	auto xml_pres    = element->FirstChildElement(XmlName::PRES);
	auto xml_dens    = element->FirstChildElement(XmlName::DENSITY);
	auto xml_dens15  = element->FirstChildElement(XmlName::DENSITY15);
	auto xml_dens20  = element->FirstChildElement(XmlName::DENSITY20);
	auto xml_b15     = element->FirstChildElement(XmlName::B15);
	auto xml_y15     = element->FirstChildElement(XmlName::Y15);
	auto factors_xml = element->FirstChildElement(XmlName::FACTORS);

	if (!xml_impulse || !xml_temp || !xml_pres || !xml_dens || !factors_xml) {
		return err.set(DATACFGERR_STREAM, element->GetLineNum(), "undefened links");
	}

	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_impulse->FirstChildElement(XmlName::LINK), m_counter)) return err.getError();
	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_pres->FirstChildElement   (XmlName::LINK), m_pres))    return err.getError();
	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_dens->FirstChildElement   (XmlName::LINK), m_dens))    return err.getError();
	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_temp->FirstChildElement   (XmlName::LINK), m_temp))    return err.getError();

	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadShadowLink(xml_freq  , m_freq  , m_counter, XmlName::FREQ)     ) return err.getError();
	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadShadowLink(xml_dens15, m_dens15, m_dens   , XmlName::DENSITY15)) return err.getError();
	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadShadowLink(xml_dens20, m_dens20, m_dens   , XmlName::DENSITY20)) return err.getError();
	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadShadowLink(xml_b15   , m_b15   , m_dens   , XmlName::B15)      ) return err.getError();
	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadShadowLink(xml_y15   , m_y15   , m_dens   , XmlName::Y15)      ) return err.getError();

	// Загрузка факторов
	m_setFactor.KeypadKF.Init(XmlUtils::getTextLREAL(factors_xml->FirstChildElement(XmlName::KEYPAD_KF), 0.0, fault));
	m_setFactor.KeypadMF.Init(XmlUtils::getTextLREAL(factors_xml->FirstChildElement(XmlName::KEYPAD_MF), 0.0, fault));

	if (fault) {
		return err.set(DATACFGERR_STREAM_FACTORS, factors_xml->GetLineNum(), "fault load keypad factors");
	}

	tinyxml2::XMLElement* points_xml = factors_xml->FirstChildElement(XmlName::POINTS);
	if (points_xml) {
		XML_FOR(point_xml, points_xml, XmlName::POINT) {
			rFactorPoint point;

			point.Hz.Init(XmlUtils::getTextLREAL(point_xml->FirstChildElement(XmlName::HERTZ), 0.0, fault));
			point.Kf.Init(XmlUtils::getTextLREAL(point_xml->FirstChildElement(XmlName::KF   ), 0.0, fault));
			point.m_id = m_setFactor.m_point.size();

			if (fault) {
				return err.set(DATACFGERR_STREAM_FACTORS, point_xml->GetLineNum(), "fault load point");
			}

			m_setFactor.m_point.push_back(point);
		}

		if (m_setFactor.m_point.size() > rFlowFactor::MAXPOINTS) {
			return err.set(DATACFGERR_STREAM_TOMANYPOINTS, points_xml->GetLineNum(), "");
		}
	}
	m_curFactor = m_setFactor;

	// Необходимо переустановить единицы измерения
	auto unit = m_station->getUnit();
	m_temp.m_unit         = unit.getTemperature();
	m_pres.m_unit         = unit.getPressure();
	m_dens.m_unit         = unit.getDensity();
	m_dens15.m_unit       = unit.getDensity();
	m_dens20.m_unit       = unit.getDensity();
	m_flowMass.m_unit     = unit.getMass();
	m_flowVolume.m_unit   = unit.getVolume();
	m_flowVolume15.m_unit = unit.getVolume();
	m_flowVolume20.m_unit = unit.getVolume();

	reinitLimitEvents();

	return TRITONN_RESULT_OK;
}

LREAL rStream::calcualateKF()
{
	double minKF = 0;
	double maxKF = 0;
	double minHz = 0;
	double maxHz = 0;

	if (m_freq.m_value <= 0) {
		return 0;
	}

	if (!m_linearization) {
		return m_curFactor.KeypadKF.Value;
	}

	// Проверка на уход частоты за минимум
	if (m_freq.m_value < m_curFactor.m_point[0].Hz.Value) {
		return m_curFactor.m_point[0].Kf.Value;
	}

	//TODO можно переписать в цикл for(auto), через переменные prev и present
	for (UDINT ii = 1; ii < m_curFactor.m_point.size(); ++ii) {
		// Проверка ухода частоты за максимум
		if (m_curFactor.m_point[ii].Hz.Value <= 0) {
			return m_curFactor.m_point[ii - 1].Kf.Value;
		}

		if (m_freq.m_value < m_curFactor.m_point[ii].Hz.Value) {
			minKF = m_curFactor.m_point[ii - 1].Kf.Value;
			maxKF = m_curFactor.m_point[ii    ].Kf.Value;
			minHz = m_curFactor.m_point[ii - 1].Hz.Value;
			maxHz = m_curFactor.m_point[ii    ].Hz.Value;

			return minKF + ((maxKF - minKF) / (maxHz - minHz)) * (m_freq.m_value - minHz);
		}
	}

	// Ушли за максимум, при полностью заполенной таблице
	return m_curFactor.m_point.back().Kf.Value;
}



void rStream::calcTotal()
{
	if (m_maintenance) {
		m_total.Inc.Count    = 0;
		m_total.Inc.Mass     = 0;
		m_total.Inc.Volume   = 0;
		m_total.Inc.Volume15 = 0;
		m_total.Inc.Volume20 = 0;
		return;
	}

	if (m_flowmeter == Type::CORIOLIS) {
		m_total.Inc.Mass     = m_total.Inc.Count  / m_curKF          * m_curFactor.KeypadMF.Value;
		m_total.Inc.Volume   = m_total.Inc.Mass   / m_dens.m_value   * 1000.0;
		m_total.Inc.Volume15 = m_total.Inc.Mass   / m_dens15.m_value * 1000.0;
		m_total.Inc.Volume20 = m_total.Inc.Mass   / m_dens20.m_value * 1000.0;
	} else {
		m_total.Inc.Volume   = m_total.Inc.Count  / m_curKF        * m_curFactor.KeypadMF.Value;
		m_total.Inc.Mass     = m_total.Inc.Volume * m_dens.m_value                    / 1000.0;
		m_total.Inc.Volume15 = m_total.Inc.Volume * m_dens.m_value / m_dens15.m_value / 1000.0;
		m_total.Inc.Volume20 = m_total.Inc.Volume * m_dens.m_value / m_dens20.m_value / 1000.0;
	}

	m_total.Inc.Mass     = Round(m_total.Inc.Mass    , 5);
	m_total.Inc.Volume   = Round(m_total.Inc.Volume  , 5);
	m_total.Inc.Volume15 = Round(m_total.Inc.Volume15, 5);
	m_total.Inc.Volume20 = Round(m_total.Inc.Volume20, 5);

	m_total.Calculate(m_station->getUnit());
}

UDINT rStream::generateMarkDown(rGeneratorMD& md)
{
	m_counter.m_limit.m_setup.Init(LIMIT_SETUP_ALL);
	m_freq.m_limit.m_setup.Init   (LIMIT_SETUP_ALL);
	m_temp.m_limit.m_setup.Init   (LIMIT_SETUP_ALL);
	m_pres.m_limit.m_setup.Init   (LIMIT_SETUP_ALL);
	m_dens.m_limit.m_setup.Init   (LIMIT_SETUP_ALL);
	m_dens15.m_limit.m_setup.Init (LIMIT_SETUP_ALL);
	m_dens20.m_limit.m_setup.Init (LIMIT_SETUP_ALL);
	m_b15.m_limit.m_setup.Init    (LIMIT_SETUP_ALL);
	m_y15.m_limit.m_setup.Init    (LIMIT_SETUP_ALL);

	m_flowMass.m_limit.m_setup.Init    (LIMIT_SETUP_ALL);
	m_flowVolume.m_limit.m_setup.Init  (LIMIT_SETUP_ALL);
	m_flowVolume15.m_limit.m_setup.Init(LIMIT_SETUP_ALL);
	m_flowVolume20.m_limit.m_setup.Init(LIMIT_SETUP_ALL);

	for (auto ii = 0; ii < rFlowFactor::MAXPOINTS; ++ii) {
		m_curFactor.m_point.push_back(rFactorPoint(ii, 0, 0));
	}

	md.add(this, true, rGeneratorMD::Type::CALCULATE)
			.addProperty(XmlName::SETUP        , &m_flagsSetup, false)
			.addProperty(XmlName::FLOWMETER    , &m_flagsFlowmeter, true)
			.addProperty(XmlName::LINEARIZATION, static_cast<UDINT>(m_linearization))
			.addProperty(XmlName::MAINTENANCE  , static_cast<UDINT>(m_maintenance))
			.addXml("<" + std::string(XmlName::IO) + "> " + rGeneratorMD::rItem::XML_OPTIONAL)
			.addXml("\t<!-- list of io objects -->")
			.addXml("</" + std::string(XmlName::IO) + ">")
			.addXml("<" + std::string(XmlName::CALC) + "> " + rGeneratorMD::rItem::XML_OPTIONAL)
			.addXml("\t<!-- list of calculate objects -->")
			.addXml("</" + std::string(XmlName::CALC) + ">")
			.addXml("<" + std::string(XmlName::FACTORS) + "> " + rGeneratorMD::rItem::XML_OPTIONAL)
			.addXml(std::string(XmlName::KEYPAD_MF), m_curFactor.KeypadMF.Value, true, "\t")
			.addXml(std::string(XmlName::KEYPAD_KF), m_curFactor.KeypadKF.Value, true, "\t")
			.addXml("\t<" + std::string(XmlName::POINTS) + "> " + rGeneratorMD::rItem::XML_OPTIONAL)
			.addXml("\t\t<" + std::string(XmlName::POINT) + "> " + rGeneratorMD::rItem::XML_OPTIONAL)
			.addXml(std::string(XmlName::HERTZ), 0.0, false, "\t\t\t")
			.addXml(std::string(XmlName::KF   ), 0.0, false, "\t\t\t")
			.addXml("\t\t</" + std::string(XmlName::POINT) + ">")
			.addXml("\t\t...")
			.addXml("\t\t<" + std::string(XmlName::POINT) + "> " + rGeneratorMD::rItem::XML_OPTIONAL)
			.addXml(std::string(XmlName::HERTZ), 0.0, false, "\t\t\t")
			.addXml(std::string(XmlName::KF   ), 0.0, false, "\t\t\t")
			.addXml("\t\t</" + std::string(XmlName::POINT) + ">")
			.addXml("\t</" + std::string(XmlName::POINTS) + ">")
			.addXml("</" + std::string(XmlName::FACTORS) + ">")
			.addRemark(rFlowFactor::FOOTNOTE + ": Количество точек зависит от количества точек в конфигурации и не может быть изменено во время работы. Максимальное количество точек равно 12.");

	return TRITONN_RESULT_OK;
}
