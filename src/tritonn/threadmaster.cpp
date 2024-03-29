///=================================================================================================
//===
//=== threadmaster.cpp
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


#include <string.h>
#include <sys/sysinfo.h>
#include "reversed.h"
#include "locker.h"
#include "tickcount.h"
#include "variable_item.h"
#include "variable_list.h"
#include "variable_class.h"
#include "threadmaster.h"
#include "log_manager.h"
#include "stringex.h"
#include "simplefile.h"
#include "units.h"


rThreadMaster::rThreadMaster() : rVariableClass(Mutex)
{
	RTTI = "rThreadMaster";
}


rThreadMaster::~rThreadMaster()
{
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
// Получение данных от менеджера данных
UDINT rThreadMaster::add(rThreadClass *thread, UDINT flags, const std::string& alias)
{
	rLocker lock(Mutex); UNUSED(lock);

	rInfo *info = new rInfo();

	info->m_class           = thread;
	info->m_flags           = flags;
	info->m_thread          = thread->GetThread();
	info->m_counter         = 0;
	info->m_cntAvrMax       = 0;
	info->m_timeAvr.IdleMin = 0xFFFFFFFF;
	info->m_timeAvr.WorkMin = 0xFFFFFFFF;

	m_threadList.push_back(info);

	generateThreadVars(info, alias);

	if(flags & TMF_NOTRUN) return TRITONN_RESULT_OK;

	do {
		mSleep(50);

		info->m_status = info->m_class->GetStatus();

		if (info->m_status == rThreadStatus::FINISHED) {
			TRACEP(LOG::THREAD, "Can't run thread.");
			exit(0); //NOTE Нужно ли так жестко, может быть Halt?
			return 1;
		}
	}
	while(info->m_status != rThreadStatus::RUNNING);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//

//-------------------------------------------------------------------------------------------------
//
void rThreadMaster::closeAll()
{
	for (auto item : reversed(m_threadList)) {
		std::string name = item->m_class->GetRTTI();

		item->m_class->Finish();
		pthread_join(*item->m_thread, NULL);

		if ((item->m_flags & TMF_DELETE) && item->m_class) {
			delete item->m_class;
		}

		delete item;

		TRACEW(LOG::THREAD, "--------- Поток %s закрыт!", name.c_str());
	}

	m_threadList.clear();

	Finish();
}



//-------------------------------------------------------------------------------------------------
//
rThreadStatus rThreadMaster::Proccesing()
{
	rTickCount savetimer;
//	rTickCount *notruntimer = new rTickCount(); // Таймер, для нитей, которые запускаются через TMF_NOTRUN
	rTickCount notruntimer;

	savetimer.start(5 * 60 * 1000);
	notruntimer.start(10 * 1000);

	while(true)
	{
		// Обработка команд нити
		rThreadStatus thread_status = rThreadClass::Proccesing();
		if(!THREAD_IS_WORK(thread_status))
		{
			closeAll();
			Closed();
			return rThreadStatus::CLOSED;
		}

		// Следим за всеми потоками
		for (auto item : m_threadList) {
			item->m_status = item->m_class->GetStatus();

			if(THREAD_IS_WORK(item->m_status))
			{
				item->m_flags &= ~TMF_NOTRUN;
			}
			else if(!(item->m_flags & TMF_NOTRUN))
			{
				TRACEP(LOG::THREAD, "--------- Аварийное закрытие потока %s!", item->m_class->GetRTTI());
				break;
			}

			// Снятие флага TMF_NOTRUN
			if(notruntimer.isFinished())
			{
				item->m_flags &= ~TMF_NOTRUN;
				notruntimer.stop();
			}

			calcThreadTimeInfo(item);
		}

		if(savetimer.isFinished())
		{
			saveAllTimerInfo();
			savetimer.restart();
		}

		m_curSysInfo.calculate();
//		printf("CPU %.2f%% (%+.2f%%)\n", m_curSysInfo.m_usageCPU, m_curSysInfo.m_modifyCPU);

		{
			rLocker locker(Mutex); UNUSED(locker);

			rVariableClass::processing();
		}
	} // while
}


UDINT rThreadMaster::calcThreadTimeInfo(rInfo *ti)
{
	UDINT worktime = 0;
	UDINT idletime = 0;
	std::vector<rThreadTimeInfo> vti;

	ti->m_class->GetTimeInfo(vti);

	if(vti.size()) {
		for(auto& item : vti) {
			worktime += item.Work;
			idletime += item.Idle;
		}
		worktime /= vti.size();
		idletime /= vti.size();
	}

	if(worktime < ti->m_timeAvr.WorkMin) ti->m_timeAvr.WorkMin = worktime;
	if(worktime > ti->m_timeAvr.WorkMax) ti->m_timeAvr.WorkMax = worktime;
	if(idletime < ti->m_timeAvr.IdleMin) ti->m_timeAvr.IdleMin = idletime;
	if(idletime > ti->m_timeAvr.IdleMax) ti->m_timeAvr.IdleMax = idletime;

	ti->m_timeAvr.WorkAverage = UDINT((LREAL(ti->m_timeAvr.WorkAverage) * LREAL(ti->m_counter) + LREAL(worktime)) / LREAL(ti->m_counter + 1));
	ti->m_timeAvr.IdleAverage = UDINT((LREAL(ti->m_timeAvr.IdleAverage) * LREAL(ti->m_counter) + LREAL(idletime)) / LREAL(ti->m_counter + 1));
	++ti->m_counter;

	if(worktime > ti->m_timeAvr.WorkAverage || 0 == ti->m_cntAvrMax)
	{
		ti->m_timeAvr.WorkAvgMax = UDINT((LREAL(ti->m_timeAvr.WorkAvgMax) * LREAL(ti->m_cntAvrMax) + LREAL(worktime)) / LREAL(ti->m_cntAvrMax + 1));
		++ti->m_cntAvrMax;
	}

	ti->m_timeInfo.push_back(new rThreadTimeInfo(worktime, idletime));

	//printf("%s : work avr %u us, avr max %u us, max %u us\n", ti->Class->GetRTTI(), ti->TimeAvr.WorkAverage, ti->TimeAvr.WorkAvgMax, ti->TimeAvr.WorkMax);
    return 0;
}


UDINT rThreadMaster::saveAllTimerInfo()
{
	string filename = "";
	string worktext = "";
	string idletext = "";
	string cputext  = "";
	string memtext  = "";
	string cpumtext = "";
	string memmtext = "";
	UDINT  utime    = rTickCount::UnixTime();

	filename = String_format("%ssysinfo_%u.csv", DIR_TIMEINFO.c_str(), utime);
	cputext  = "cpu;";
	cpumtext = "cpu modify";
	memtext  = "memory;";
	memmtext = "memory modify;";

	for (auto& item : m_sysInfo) {
		cputext  += String_format("%.2f;", item.m_usageCPU);
		cpumtext += String_format("%.2f;", item.m_modifyCPU);
		memtext  += String_format("%u;"  , item.m_freeMem);
		memmtext += String_format("%i;"  , item.m_modifyMem);
	}
	m_sysInfo.clear();

	simpleFileSave(filename, cputext + "\n" + cpumtext + "\n" + memtext + "\n" + memmtext);


	for (auto thread : m_threadList) {
		filename = String_format("%s%s_%u.csv", DIR_TIMEINFO.c_str(), thread->m_class->GetRTTI(), utime);
		worktext = "work;";
		idletext = "idle;";

		for (auto ti : thread->m_timeInfo) {
			worktext += String_format("%u;", ti->Work);
			idletext += String_format("%u;", ti->Idle);

			delete ti;
		}

		simpleFileSave(filename, worktext + "\n" + idletext);

		thread->m_timeInfo.clear();
	}

	return TRITONN_RESULT_OK;
}


UDINT rThreadMaster::generateVars(rVariableClass* parent)
{
	m_varList.add("system.diag.processor.cpu.value"    , rVariable::Flags::R___, &m_curSysInfo.m_usageCPU , U_perc   , 0, "Текущая загрузка ЦПУ");
	m_varList.add("system.diag.processor.cpu.modify"   , rVariable::Flags::R___, &m_curSysInfo.m_modifyCPU, U_perc   , 0, "Изменение загрузки ЦПУ");
	m_varList.add("system.diag.processor.memory.free"  , rVariable::Flags::R___, &m_curSysInfo.m_freeMem  , U_DIMLESS, 0, "Свободная память");
	m_varList.add("system.diag.processor.memory.modify", rVariable::Flags::R___, &m_curSysInfo.m_modifyMem, U_DIMLESS, 0, "Изменение свободной памяти");

	if (parent) {
		rVariableClass::linkToExternal(parent);
	}

	return TRITONN_RESULT_OK;
}

UDINT rThreadMaster::generateThreadVars(rInfo* ti, const std::string& alias)
{
	if (!ti || alias.empty()) {
		return TRITONN_RESULT_OK;
	}

	std::string prefix = "system.diag." + alias + ".";

	m_varList.add(prefix + "work.max"    , rVariable::Flags::R___, &ti->m_timeAvr.WorkMax    , U_DIMLESS, 0, "Время выполения. Максимальное");
	m_varList.add(prefix + "work.min"    , rVariable::Flags::R___, &ti->m_timeAvr.WorkMin    , U_DIMLESS, 0, "Время выполения. Минимальное");
	m_varList.add(prefix + "work.avgmax" , rVariable::Flags::R___, &ti->m_timeAvr.WorkAvgMax , U_DIMLESS, 0, "Время выполения. Средне-максимальное");
	m_varList.add(prefix + "work.average", rVariable::Flags::R___, &ti->m_timeAvr.WorkAverage, U_DIMLESS, 0, "Время выполения. Среднее");
	m_varList.add(prefix + "idle.max"    , rVariable::Flags::R___, &ti->m_timeAvr.IdleMax    , U_DIMLESS, 0, "Время простоя. Максимальное");
	m_varList.add(prefix + "idle.min"    , rVariable::Flags::R___, &ti->m_timeAvr.IdleMin    , U_DIMLESS, 0, "Время простоя. Минимальное");
	m_varList.add(prefix + "idle.average", rVariable::Flags::R___, &ti->m_timeAvr.IdleAverage, U_DIMLESS, 0, "Время простоя. Среднее");

	return TRITONN_RESULT_OK;
}


