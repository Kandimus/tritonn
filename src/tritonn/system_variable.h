//=================================================================================================
//===
//=== system_variables.h
//===
//=== Copyright (c) 2019-2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс для системных переменных rDataManager
//===
//=================================================================================================

#pragma once

#include "def.h"
#include "structures.h"
#include "bits_array.h"
#include "singlenton.h"

class rVariableList;
class rGeneratorMD;

namespace tinyxml2 {
	class XMLElement;
}

// Системные переменные
class rSystemVariable
{
	SINGLETON(rSystemVariable)

public:
	void  generateMarkDown(rGeneratorMD& md);
	UDINT initVariables(rVariableList& list);
	void  processing();

	void  clearConfigInfo1();
	void  getConfigInfo(rConfigInfo& conf);
	UDINT loadConfigInfo(const std::string& filename, tinyxml2::XMLElement* root);

	void  getState(rState& st);
	void  getTime(struct tm& sdt);

	rSystemVariable*       getPointer() const;
	const rSystemVariable* getConstPointer() const;
//	const rConfigInfo*     getConfName() const { return &m_configInfo; }

	void        getVersion(rVersion& ver) const { ver = m_version; }
	std::string getLiveName(Live live)    const { return m_flagsLive.getNameByValue(static_cast<UDINT>(live)); }

	void setEventAlarm(UDINT count);
	void setLive(Live live);
	void setSimulate(USINT sim);

private:
	pthread_rwlock_t m_rwlock;

	rVersion      m_version;
	rMetrologyVer m_metrologyVer;
	rState        m_state;
	Time64_T      m_unixTime;
	STM           m_dateTime;
	STM           m_setDateTime;
	USINT         m_setDateTimeAccept;
	rConfigInfo   m_configInfo;

	rBitsArray    m_flagsLive;

	//TODO IP


};





