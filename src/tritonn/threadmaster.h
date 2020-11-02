//=================================================================================================
//===
//=== threadmaster.h
//===
//=== Copyright (c) 2020 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Основной класс-нить слежением за остальными нитями
//===
//=================================================================================================

#pragma once

#include <vector>
#include "system_info.h"
#include "thread_class.h"
#include "variable_class.h"

//

//-------------------------------------------------------------------------------------------------
//
class rThreadMaster : public rThreadClass, public rVariableClass
{
	SINGLETON(rThreadMaster)

protected:
	struct rInfo
	{
		UDINT          m_flags;
		rThreadStatus  m_status;
		pthread_t*     m_thread;
		rThreadClass*  m_class;
		rThreadTimeAvr m_timeAvr;
		UDINT          m_counter;
		UDINT          m_cntAvrMax;

		std::vector<rThreadTimeInfo*> m_timeInfo;
	};

public:
	// Работа с нитями
	UDINT add(rThreadClass* thread, UDINT flags, const string& alias);

protected:
	virtual rThreadStatus Proccesing();

	void  closeAll();

//	UDINT getCPUState(rCPUState& cpu);
//	UDINT calcSysInfo(rCPUState& cpu_prev);
	UDINT calcThreadTimeInfo(rInfo* ti);
	UDINT saveAllTimerInfo();
	UDINT generateVars(rInfo* ti, const std::string& alias);

private:
	std::vector<rInfo*>       m_list; // Список всех нитей
	std::vector<rSystemInfo>  m_sysInfo;  //
	rSystemInfo               m_curSysInfo;
	UDINT                     m_timerNotRun = 5000;
};



