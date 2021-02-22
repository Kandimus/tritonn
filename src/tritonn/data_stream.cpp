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
#include "event_eid.h"
#include "text_id.h"
#include "xml_util.h"
#include "event_manager.h"
#include "precision.h"
#include "data_manager.h"
#include "data_config.h"
#include "variable_item.h"
#include "variable_list.h"
#include "data_station.h"
#include "data_stream.h"
#include "data_snapshot.h"


const UDINT STREAM_LE_INPUTS     = 0x00000001;
const UDINT STREAM_LE_ACCOUNTING = 0x00000002;
const UDINT STREAM_LE_KEYPADKF   = 0x00000004;


rBitsArray rStream::m_flagsFlowmeter;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
rStream::rStream(const rStation* owner) :
	rSource(owner),
	Setup(STR_SETUP_OFF),
	m_total(ReinitEvent(EID_STREAM_TOTAL_MASS),
			ReinitEvent(EID_STREAM_TOTAL_VOLUME),
			ReinitEvent(EID_STREAM_TOTAL_VOLUME15),
			ReinitEvent(EID_STREAM_TOTAL_VOLUME20))
{
	if (m_flagsFlowmeter.empty()) {
		m_flagsFlowmeter
				.add("TURBINE"   , static_cast<UINT>(Type::TURBINE)   , "Турбинный ПР")
				.add("CORIOLIS"  , static_cast<UINT>(Type::CORIOLIS)  , "Кориолисовый ПР")
				.add("ULTRASONIC", static_cast<UINT>(Type::ULTRASONIC), "Ультрозвуковой ПР");
	}

	Maintenance   = true;
	Linearization = false;
//	rTotal   Total;

	InitLink(rLink::Setup::INPUT   , m_counter     , U_imp   , SID::IMPULSE          , XmlName::IMPULSE      , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INPUT   , m_freq        , U_Hz    , SID::FREQUENCY        , XmlName::FREQ         , XmlName::IMPULSE);
	InitLink(rLink::Setup::INOUTPUT, m_temp        , U_C     , SID::TEMPERATURE      , XmlName::TEMP         , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INOUTPUT, m_pres        , U_MPa   , SID::PRESSURE         , XmlName::PRES         , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INOUTPUT, m_dens        , U_kg_m3 , SID::DENSITY          , XmlName::DENSITY      , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INOUTPUT, m_dens15      , U_kg_m3 , SID::DENSITY15        , XmlName::DENSITY15    , XmlName::DENSITY);
	InitLink(rLink::Setup::INOUTPUT, m_dens20      , U_kg_m3 , SID::DENSITY20        , XmlName::DENSITY20    , XmlName::DENSITY);
	InitLink(rLink::Setup::INOUTPUT, m_b15         , U_1_C   , SID::B15              , XmlName::B15          , XmlName::DENSITY);
	InitLink(rLink::Setup::INOUTPUT, m_y15         , U_1_MPa , SID::Y15              , XmlName::Y15          , XmlName::DENSITY);
	InitLink(rLink::Setup::OUTPUT  , m_flowMass    , U_t_h   , SID::FLOWRATE_MASS    , XmlName::FLOWRATEMASS , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT  , m_flowVolume  , U_m3_h  , SID::FLOWRATE_VOLUME  , XmlName::FLOWRATEVOL  , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT  , m_flowVolume15, U_m3_h  , SID::FLOWRATE_VOLUME15, XmlName::FLOWRATEVOL15, rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT  , m_flowVolume20, U_m3_h  , SID::FLOWRATE_VOLUME20, XmlName::FLOWRATEVOL20, rLink::SHADOW_NONE);
}


rStream::~rStream()
{
	;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rStream::InitLimitEvent(rLink &link)
{
	link.Limit.EventChangeAMin  = ReinitEvent(EID_STREAM_NEW_AMIN)  << link.Descr << link.Unit;
	link.Limit.EventChangeWMin  = ReinitEvent(EID_STREAM_NEW_WMIN)  << link.Descr << link.Unit;
	link.Limit.EventChangeWMax  = ReinitEvent(EID_STREAM_NEW_WMAX)  << link.Descr << link.Unit;
	link.Limit.EventChangeAMax  = ReinitEvent(EID_STREAM_NEW_AMAX)  << link.Descr << link.Unit;
	link.Limit.EventChangeHyst  = ReinitEvent(EID_STREAM_NEW_HYST)  << link.Descr << link.Unit;
	link.Limit.EventChangeSetup = ReinitEvent(EID_STREAM_NEW_SETUP) << link.Descr << link.Unit;
	link.Limit.EventAMin        = ReinitEvent(EID_STREAM_AMIN)      << link.Descr << link.Unit;
	link.Limit.EventWMin        = ReinitEvent(EID_STREAM_WMIN)      << link.Descr << link.Unit;
	link.Limit.EventWMax        = ReinitEvent(EID_STREAM_WMAX)      << link.Descr << link.Unit;
	link.Limit.EventAMax        = ReinitEvent(EID_STREAM_AMAX)      << link.Descr << link.Unit;
	link.Limit.EventNan         = ReinitEvent(EID_STREAM_NAN)       << link.Descr << link.Unit;
	link.Limit.EventNormal      = ReinitEvent(EID_STREAM_NORMAL)    << link.Descr << link.Unit;

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rStream::GetFault(void)
{
	return 1;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rStream::Calculate()
{
	rEvent event_s;
	rEvent event_f;

	if(rSource::Calculate()) return 0;

	//
	m_setFactor.KeypadKF.Compare(ReinitEvent(EID_STREAM_KEYPADKF));
	m_setFactor.KeypadMF.Compare(ReinitEvent(EID_STREAM_KEYPADMF));

	for (auto& point : m_setFactor.m_point) {
		point.Hz.Compare(ReinitEvent(EID_STREAM_POINT_HZ) << rStringID(U_Hz) << (point.m_id + 1));
		point.Kf.Compare(ReinitEvent(EID_STREAM_POINT_KF) << getUnitKF()     << (point.m_id + 1));
	}

	// Вывод линии из учета
	CheckExpr(Maintenance, STREAM_LE_ACCOUNTING,
			  event_f.Reinit(EID_STREAM_ACCOUNTING_ON ) << ID << Descr,
			  event_s.Reinit(EID_STREAM_ACCOUNTING_OFF) << ID << Descr);

	// Использование ручного ввода KF
	CheckExpr(Linearization, STREAM_LE_KEYPADKF,
			  event_f.Reinit(EID_STREAM_KEYPADKF_OFF) << ID << Descr,
			  event_s.Reinit(EID_STREAM_KEYPADKF_ON ) << ID << Descr);

	// Ввод новых коэф-тов
	if(AcceptKF)
	{
		m_curFactor = m_setFactor;
		AcceptKF    = 0;
		rEventManager::instance().Add(ReinitEvent(EID_STREAM_ACCEPT));
	}

	//-------------------------------------------------------------------------------------------
	// Расчет текущего коэф-та
	CurKF = (Linearization) ? calcualateKF() : m_curFactor.KeypadKF.Value;

	// Нарастающие и расход
	// Увеличим глобальный счетчик
	calcTotal();

	// Расчитаем расход исходя из частоты
	if (m_flowmeter == Type::CORIOLIS) {
		m_flowMass.Value   = m_freq.Value     / CurKF        * 3600.0 * m_curFactor.KeypadMF.Value;
		m_flowVolume.Value = m_flowMass.Value / m_dens.Value * 1000.0 * m_curFactor.KeypadMF.Value;
	} else {
		m_flowVolume.Value = m_freq.Value       / CurKF        * 3600.0 * m_curFactor.KeypadMF.Value;
		m_flowMass.Value   = m_flowVolume.Value * m_dens.Value / 1000.0 * m_curFactor.KeypadMF.Value;
	}

	return 0;
}


//-------------------------------------------------------------------------------------------------
// Получение единиц измерения К-фактора (исходя из типа расходомера)
UDINT rStream::getUnitKF()
{
	if (!m_station) {
		return U_UNDEF;
	}

	return m_flowmeter == Type::CORIOLIS ? m_station->getUnit().getMassKF() : m_station->getUnit().getVolumeKF();
}



///////////////////////////////////////////////////////////////////////////////////////////////////
//
const rTotal *rStream::getTotal(void) const
{
	return &m_total;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
UDINT rStream::enableFreqOut() const
{
	if (m_freq.isValid()) {
		std::string moduleAlias = m_freq.getModuleAlias();

		if (moduleAlias.size()) {
			rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_SYSTEM);
			//TODO может заменить "hardware" константой?
			ss.add("hardware." + moduleAlias + ".outtype", 0); //TODO Подставить нужное число
			ss.set();

			return TRITONN_RESULT_OK;
		}
	}

	return DATACFGERR_STREAM_NOFREQCHANNEL;
}

UDINT rStream::disableFreqOut() const
{
	if (m_freq.isValid()) {
		std::string moduleAlias = m_freq.getModuleAlias();

		if (moduleAlias.size()) {
			rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_SYSTEM);

			ss.add("hardware." + moduleAlias + ".outtype", m_freq.getChannelNumber()); //TODO Подставить нужное число
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

	rSource::generateVars(list);

	// Внутренние переменные
	list.add(Alias + ".setup"                 , TYPE_UINT , rVariable::Flags::RS_L, &Setup.Value               , U_DIMLESS       , ACCESS_SA);
	list.add(Alias + ".flowmeter"             , TYPE_USINT, rVariable::Flags::R__L, &m_flowmeter               , U_DIMLESS       , 0);
	list.add(Alias + ".maintenance"           , TYPE_UDINT, rVariable::Flags::___L, &Maintenance               , U_DIMLESS       , ACCESS_MAINTENANCE);
	list.add(Alias + ".linearization"         , TYPE_UDINT, rVariable::Flags::___L, &Linearization             , U_DIMLESS       , ACCESS_FACTORS);
	list.add(Alias + ".total.present.Volume"  , TYPE_LREAL, rVariable::Flags::R___, &m_total.Present.Volume    , unit.getVolume(), 0);
	list.add(Alias + ".total.present.Volume15", TYPE_LREAL, rVariable::Flags::R___, &m_total.Present.Volume15  , unit.getVolume(), 0);
	list.add(Alias + ".total.present.Volume20", TYPE_LREAL, rVariable::Flags::R___, &m_total.Present.Volume20  , unit.getVolume(), 0);
	list.add(Alias + ".total.present.Mass"    , TYPE_LREAL, rVariable::Flags::R___, &m_total.Present.Mass      , unit.getMass()  , 0);
	list.add(Alias + ".total.present.impulse" , TYPE_UDINT, rVariable::Flags::R___, &m_total.Present.Count     , U_imp           , 0);
	list.add(Alias + ".total.Inc.Volume"      , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.Inc.Volume        , unit.getVolume(), ACCESS_SA);
	list.add(Alias + ".total.Inc.Volume15"    , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.Inc.Volume15      , unit.getVolume(), ACCESS_SA);
	list.add(Alias + ".total.Inc.Volume20"    , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.Inc.Volume20      , unit.getVolume(), ACCESS_SA);
	list.add(Alias + ".total.Inc.Mass"        , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.Inc.Mass          , unit.getMass()  , ACCESS_SA);
	list.add(Alias + ".total.Inc.inpulse"     , TYPE_UDINT, rVariable::Flags::RSH_, &m_total.Inc.Count         , U_imp           , ACCESS_SA);
	list.add(Alias + ".total.raw.Volume"      , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.Raw.Volume        , unit.getVolume(), 0);
	list.add(Alias + ".total.raw.Volume15"    , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.Raw.Volume15      , unit.getVolume(), 0);
	list.add(Alias + ".total.raw.Volume20"    , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.Raw.Volume20      , unit.getVolume(), 0);
	list.add(Alias + ".total.raw.Mass"        , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.Raw.Mass          , unit.getMass()  , 0);
	list.add(Alias + ".total.raw.impulse"     , TYPE_UDINT, rVariable::Flags::RSH_, &m_total.Raw.Count         , U_imp           , 0);
	list.add(Alias + ".total.past.Volume"     , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.Past.Volume       , unit.getVolume(), 0);
	list.add(Alias + ".total.past.Volume15"   , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.Past.Volume15     , unit.getVolume(), 0);
	list.add(Alias + ".total.past.Volume20"   , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.Past.Volume20     , unit.getVolume(), 0);
	list.add(Alias + ".total.past.Mass"       , TYPE_LREAL, rVariable::Flags::RSH_, &m_total.Past.Mass         , unit.getMass()  , 0);
	list.add(Alias + ".total.past.impulse"    , TYPE_UDINT, rVariable::Flags::RSH_, &m_total.Past.Count        , U_imp           , 0);
	list.add(Alias + ".presentkf"             , TYPE_LREAL, rVariable::Flags::R___, &CurKF                     , getUnitKF()     , 0);
	list.add(Alias + ".factors.kf"            , TYPE_LREAL, rVariable::Flags::R___, &m_curFactor.KeypadKF.Value, getUnitKF()     , 0);
	list.add(Alias + ".factors.mf"            , TYPE_LREAL, rVariable::Flags::R__L, &m_curFactor.KeypadMF.Value, U_DIMLESS       , 0);
	list.add(Alias + ".factors.set.kf"        , TYPE_LREAL, rVariable::Flags::___L, &m_setFactor.KeypadKF.Value, getUnitKF()     , ACCESS_FACTORS);
	list.add(Alias + ".factors.set.mf"        , TYPE_LREAL, rVariable::Flags::___L, &m_setFactor.KeypadMF.Value, U_DIMLESS       , ACCESS_FACTORS);
	list.add(Alias + ".factors.set.accept"    , TYPE_UDINT, rVariable::Flags::___L, &AcceptKF                  , U_DIMLESS       , ACCESS_FACTORS);


	for (UDINT ii = 0; ii < m_curFactor.m_point.size(); ++ii) {
		std::string name_kf     = String_format("%s.factors.point_%i.kf"    , Alias.c_str(), ii + 1);
		std::string name_hz     = String_format("%s.factors.point_%i.hz"    , Alias.c_str(), ii + 1);
		std::string name_set_kf = String_format("%s.factors.set.point_%i.kf", Alias.c_str(), ii + 1);
		std::string name_set_hz = String_format("%s.factors.set.point_%i.hz", Alias.c_str(), ii + 1);

		list.add(name_kf    , TYPE_LREAL, rVariable::Flags::R__L, &m_curFactor.m_point[ii].Kf, getUnitKF(), 0);
		list.add(name_set_kf, TYPE_LREAL, rVariable::Flags::____, &m_setFactor.m_point[ii].Kf, getUnitKF(), ACCESS_FACTORS);
		list.add(name_hz    , TYPE_LREAL, rVariable::Flags::R__L, &m_curFactor.m_point[ii].Hz, U_Hz       , 0);
		list.add(name_set_hz, TYPE_LREAL, rVariable::Flags::____, &m_setFactor.m_point[ii].Hz, U_Hz       , ACCESS_FACTORS);
	}

	list.add(Alias + ".fault", TYPE_UDINT, rVariable::Flags::R___, &Fault, U_DIMLESS, 0);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rStream::LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strFlowMeter = XmlUtils::getAttributeString(element, XmlName::FLOWMETER, m_flagsFlowmeter.getNameByBits(static_cast<USINT>(Type::CORIOLIS)));

	if (!m_station) {
		return err.set(DATACFGERR_STREAM_NOSTN, element ? element->GetLineNum() : -1, "");
	}

	if (TRITONN_RESULT_OK != rSource::LoadFromXML(element, err, prefix)) {
		return err.getError();
	}

	UDINT fault = 0;
	Linearization = XmlUtils::getAttributeUSINT(element, XmlName::LINEARIZATION, 0);
	Maintenance   = XmlUtils::getAttributeUSINT(element, XmlName::MAINTENANCE  , 0);
	m_flowmeter   = static_cast<Type>(m_flagsFlowmeter.getValue(strFlowMeter, fault));

	if (fault) {
		return err.set(DATACFGERR_STREAM, element->GetLineNum(), "undefened paramters");
	}

	tinyxml2::XMLElement* xml_impulse = element->FirstChildElement(XmlName::IMPULSE);
	tinyxml2::XMLElement* xml_freq    = element->FirstChildElement(XmlName::FREQ);
	tinyxml2::XMLElement* xml_temp    = element->FirstChildElement(XmlName::TEMP);
	tinyxml2::XMLElement* xml_pres    = element->FirstChildElement(XmlName::PRES);
	tinyxml2::XMLElement* xml_dens    = element->FirstChildElement(XmlName::DENSITY);
	tinyxml2::XMLElement* xml_dens15  = element->FirstChildElement(XmlName::DENSITY15);
	tinyxml2::XMLElement* xml_dens20  = element->FirstChildElement(XmlName::DENSITY20);
	tinyxml2::XMLElement* xml_b15     = element->FirstChildElement(XmlName::B15);
	tinyxml2::XMLElement* xml_y15     = element->FirstChildElement(XmlName::Y15);
	tinyxml2::XMLElement* factors_xml = element->FirstChildElement(XmlName::FACTORS);

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
	m_temp.Unit         = unit.getTemperature();
	m_pres.Unit         = unit.getPressure();
	m_dens.Unit         = unit.getDensity();
	m_dens15.Unit       = unit.getDensity();
	m_dens20.Unit       = unit.getDensity();
	m_flowMass.Unit     = unit.getMass();
	m_flowVolume.Unit   = unit.getVolume();
	m_flowVolume15.Unit = unit.getVolume();
	m_flowVolume20.Unit = unit.getVolume();

	ReinitLimitEvents();

	return TRITONN_RESULT_OK;
}


std::string rStream::saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal)
{
	m_counter.Limit.m_setup.Init(rLimit::Setup::NONE);
	m_freq.Limit.m_setup.Init(rLimit::Setup::NONE);
	m_temp.Limit.m_setup.Init(rLimit::Setup::NONE);
	m_pres.Limit.m_setup.Init(rLimit::Setup::NONE);
	m_dens.Limit.m_setup.Init(rLimit::Setup::NONE);
	m_dens15.Limit.m_setup.Init(rLimit::Setup::NONE);
	m_dens20.Limit.m_setup.Init(rLimit::Setup::NONE);
	m_b15.Limit.m_setup.Init(rLimit::Setup::NONE);
	m_y15.Limit.m_setup.Init(rLimit::Setup::NONE);

	m_flowMass.Limit.m_setup.Init(rLimit::Setup::NONE);
	m_flowVolume.Limit.m_setup.Init(rLimit::Setup::NONE);
	m_flowVolume15.Limit.m_setup.Init(rLimit::Setup::NONE);
	m_flowVolume20.Limit.m_setup.Init(rLimit::Setup::NONE);

	return rSource::saveKernel(isio, objname, comment, isglobal);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
LREAL rStream::calcualateKF()
{
	double minKF = 0;
	double maxKF = 0;
	double minHz = 0;
	double maxHz = 0;

	if (m_freq.Value <= 0) {
		return 0;
	}

	if (!Linearization) {
		return m_curFactor.KeypadKF.Value;
	}

	// Проверка на уход частоты за минимум
	if (m_freq.Value < m_curFactor.m_point[0].Hz.Value) {
		return m_curFactor.m_point[0].Kf.Value;
	}

	//TODO можно переписать в цикл for(auto), через переменные prev и present
	for (UDINT ii = 1; ii < m_curFactor.m_point.size(); ++ii) {
		// Проверка ухода частоты за максимум
		if (m_curFactor.m_point[ii].Hz.Value <= 0) {
			return m_curFactor.m_point[ii - 1].Kf.Value;
		}

		if (m_freq.Value < m_curFactor.m_point[ii].Hz.Value) {
			minKF = m_curFactor.m_point[ii - 1].Kf.Value;
			maxKF = m_curFactor.m_point[ii    ].Kf.Value;
			minHz = m_curFactor.m_point[ii - 1].Hz.Value;
			maxHz = m_curFactor.m_point[ii    ].Hz.Value;

			return minKF + ((maxKF - minKF) / (maxHz - minHz)) * (m_freq.Value - minHz);
		}
	}

	// Ушли за максимум, при полностью заполенной таблице
	return m_curFactor.m_point.back().Kf.Value;
}



void rStream::calcTotal()
{
	if (Maintenance) {
		m_total.Inc.Count    = 0;
		m_total.Inc.Mass     = 0;
		m_total.Inc.Volume   = 0;
		m_total.Inc.Volume15 = 0;
		m_total.Inc.Volume20 = 0;
		return;
	}

	if (m_flowmeter == Type::CORIOLIS) {
		m_total.Inc.Mass     = m_total.Inc.Count  / CurKF          * m_curFactor.KeypadMF.Value;
		m_total.Inc.Volume   = m_total.Inc.Mass   / m_dens.Value   * 1000.0;
		m_total.Inc.Volume15 = m_total.Inc.Mass   / m_dens15.Value * 1000.0;
		m_total.Inc.Volume20 = m_total.Inc.Mass   / m_dens20.Value * 1000.0;
	} else {
		m_total.Inc.Volume   = m_total.Inc.Count  / CurKF        * m_curFactor.KeypadMF.Value;
		m_total.Inc.Mass     = m_total.Inc.Volume * m_dens.Value                  / 1000.0;
		m_total.Inc.Volume15 = m_total.Inc.Volume * m_dens.Value / m_dens15.Value / 1000.0;
		m_total.Inc.Volume20 = m_total.Inc.Volume * m_dens.Value / m_dens20.Value / 1000.0;
	}

	m_total.Inc.Mass     = Round(m_total.Inc.Mass    , 5);
	m_total.Inc.Volume   = Round(m_total.Inc.Volume  , 5);
	m_total.Inc.Volume15 = Round(m_total.Inc.Volume15, 5);
	m_total.Inc.Volume20 = Round(m_total.Inc.Volume20, 5);

	m_total.Calculate(m_station->getUnit());
}

