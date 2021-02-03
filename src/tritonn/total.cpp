//=================================================================================================
//===
//=== total.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Нарастающие
//===
//=================================================================================================


#include "total.h"
#include "precision.h"
#include "data_objunits.h"
#include "event_manager.h"

rTotal::rTotal(rEvent& mass, rEvent& volume, rEvent& volume15, rEvent& volume20)
{
	m_eventMass     = mass;
	m_eventVolume   = volume;
	m_eventVolume15 = volume15;
	m_eventVolume20 = volume20;

	m_eventMass     << MAX_TOTAL_LIMIT;
	m_eventVolume   << MAX_TOTAL_LIMIT;
	m_eventVolume15 << MAX_TOTAL_LIMIT;
	m_eventVolume20 << MAX_TOTAL_LIMIT;

}

void rTotal::checkMax()
{
	if(Raw.Mass >= MAX_TOTAL_LIMIT)
	{
		if (m_eventMass.GetEID()) {
			rEvent event = m_eventMass;
			rEventManager::instance().Add(event);
		}
		Raw.Mass -= MAX_TOTAL_LIMIT;
	}

	if(Raw.Volume >= MAX_TOTAL_LIMIT)
	{
		if (m_eventVolume.GetEID()) {
			rEvent event = m_eventVolume;
			rEventManager::instance().Add(event);
		}
		Raw.Volume -= MAX_TOTAL_LIMIT;
	}

	if(Raw.Volume15 >= MAX_TOTAL_LIMIT)
	{
		if (m_eventVolume15.GetEID()) {
			rEvent event = m_eventVolume15;
			rEventManager::instance().Add(event);
		}
		Raw.Volume15 -= MAX_TOTAL_LIMIT;
	}

	if(Raw.Volume20 >= MAX_TOTAL_LIMIT)
	{
		if (m_eventVolume20.GetEID()) {
			rEvent event = m_eventVolume20;
			rEventManager::instance().Add(event);
		}
		Raw.Volume20 -= MAX_TOTAL_LIMIT;
	}
}



void rTotal::Calculate(const rObjUnit& unit)
{
	Past             = Present;

	Raw.Mass         = Round(Raw.Mass     + Inc.Mass    , 5);
	Raw.Volume       = Round(Raw.Volume   + Inc.Volume  , 5);
	Raw.Volume15     = Round(Raw.Volume15 + Inc.Volume15, 5);
	Raw.Volume20     = Round(Raw.Volume20 + Inc.Volume20, 5);
	Raw.Count       += Inc.Count;

	checkMax();

	Present.Mass     = Round(Raw.Mass    , rPrecision::Instance().Get(unit.getMass()));
	Present.Volume   = Round(Raw.Volume  , rPrecision::Instance().Get(unit.getVolume()));
	Present.Volume15 = Round(Raw.Volume15, rPrecision::Instance().Get(unit.getVolume()));
	Present.Volume20 = Round(Raw.Volume20, rPrecision::Instance().Get(unit.getVolume()));

	Inc.Mass         = 0.0;
	Inc.Volume       = 0.0;
	Inc.Volume15     = 0.0;
	Inc.Volume20     = 0.0;
	Inc.Count        = 0;
}


// Корректная разница нарастающих, с учетом сброса по пределу MAX_TOTAL_LIMIT
LREAL rTotal::Sub(LREAL sub1, LREAL sub2)
{
	LREAL result = sub1 - sub2;

	if(result >= 0.0) return result;

	return (-result) + (MAX_TOTAL_LIMIT - sub2);
}


void rTotal::Clear(rBaseTotal &total)
{
	total.Count    = 0;
	total.Mass     = 0.0;
	total.Volume   = 0.0;
	total.Volume15 = 0.0;
	total.Volume20 = 0.0;
}
