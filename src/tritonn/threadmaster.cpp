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
#include "locker.h"
#include "tickcount.h"
//#include "data_manager.h"
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

	m_list.push_back(info);

	generateVars(info, alias);

	if(flags & TMF_NOTRUN) return TRITONN_RESULT_OK;

	do
	{
		mSleep(50);

		info->m_status = info->m_class->GetStatus();

		if(info->m_status == rThreadStatus::FINISHED)
		{
			TRACEERROR("Can't run thread.");
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
	for (DINT ii = m_list.size() - 1; ii >= 0; --ii) {
		std::string name = m_list[ii]->m_class->GetRTTI();

		m_list[ii]->m_class->Finish();
		pthread_join(*m_list[ii]->m_thread, NULL);

		if((m_list[ii]->m_flags & TMF_DELETE) && m_list[ii]->m_class)
		{
			delete m_list[ii]->m_class;
		}

		delete m_list[ii];

		TRACEERROR("--------- Поток %s закрыт!", name.c_str());
	}

	m_list.clear();

	Finish();
}



//-------------------------------------------------------------------------------------------------
//
rThreadStatus rThreadMaster::Proccesing()
{
	rCPUState  cpu_start;
	rTickCount savetimer;
	rTickCount *notruntimer = new rTickCount(); // Таймер, для нитей, которые запускаются через TMF_NOTRUN

	savetimer.Reset();
	notruntimer->Reset();

	while(true)
	{
		GetCPUState(cpu_start);

		// Обработка команд нити
		rThreadStatus thread_status = rThreadClass::Proccesing();
		if(!THREAD_IS_WORK(thread_status))
		{
			CloseAll();
			Closed();
			return rThreadStatus::CLOSED;
		}

		// Следим за всеми потоками
		for(UDINT ii = 0; ii < List.size(); ++ii)
		{
			rThreadInfo *thread_info = List[ii];

			thread_info->Status = thread_info->Class->GetStatus();

			if(THREAD_IS_WORK(thread_info->Status))
			{
				thread_info->Flags &= ~TMF_NOTRUN;
			}
			else if(!(thread_info->Flags & TMF_NOTRUN))
			{
				TRACEERROR("--------- Аварийное закрытие потока %s!", thread_info->Class->GetRTTI());
				break;
			}

			// Снятие флага TMF_NOTRUN
			if(notruntimer)
			{
				if(notruntimer->GetCount() >= 10 * 1000)
				{
					thread_info->Flags &= ~TMF_NOTRUN;
					delete notruntimer;
					notruntimer = nullptr;
				}
			}

			CalcThreadTimeInfo(thread_info);
		}

		if(savetimer.GetCount() >= 5 * 60 * 1000)
		{
			SaveAllTimerInfo();
			savetimer.Reset();
		}

		CalcSysInfo(cpu_start);
		printf("CPU %.2f%% (%+.2f%%)\n", CurSysInfo.CPUUsage, CurSysInfo.ModifyCPU);


		//TRACEERROR("Менеджер данных работает!");
	} // while
}




UDINT rThreadMaster::CalcSysInfo(rCPUState &cpu_start)
{
	struct sysinfo sys_info;
	rCPUState  cpu;

	if(sysinfo(&sys_info) != -1)
	{
		UDINT freemem = (sys_info.freeram * sys_info.mem_unit) / 1024;

		CurSysInfo.ModifyMem = freemem - CurSysInfo.FreeMem;
		CurSysInfo.FreeMem   = freemem;
//		printf("Free memory %ikb (%+ikb)\n", FreeMem, ModifyMem);
	}

	GetCPUState(cpu);

	float active = cpu.GetActive() - cpu_start.GetActive();
	float idle   = cpu.GetIdle()   - cpu_start.GetIdle();
	float total  = active + idle;
	float usage  = (100.f * active / total);

	CurSysInfo.ModifyCPU = usage - CurSysInfo.CPUUsage;
	CurSysInfo.CPUUsage  = usage;

	SysInfo.push_back(CurSysInfo);

    return 0;
}


//
UDINT rThreadMaster::GetCPUState(rCPUState &cpu)
{
	FILE *fstat = fopen("/proc/stat", "r");

	if(nullptr == fstat)
	{
		return -1;
	}

	//read values from /proc/pid/stat
	bzero(&cpu, sizeof(rCPUState));

	if (fscanf(fstat, "%*s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
						 &cpu.User, &cpu.Nice, &cpu.System, &cpu.Idle, &cpu.IOWait, &cpu.IRQ, &cpu.SoftIRQ, &cpu.Steal, &cpu.Guest, &cpu.GuestNice) == EOF)
	{
		fclose(fstat);
		return -1;
	}

	fclose(fstat);

	//cpu.TotalUsage = cpu.User + cpu.Nice + cpu.System + cpu.Idle + cpu.IOWait + cpu.IRQ + cpu.SoftIRQ + cpu.Steal + cpu.Guest + cpu.GuestNice;

	return TRITONN_RESULT_OK;
}



UDINT rThreadMaster::CalcThreadTimeInfo(rThreadInfo *ti)
{
	UDINT worktime = 0;
	UDINT idletime = 0;
	vector<rThreadTimeInfo> vti;

	ti->Class->GetTimeInfo(vti);

	if(vti.size())
	{
		for(UDINT ii = 0; ii < vti.size(); ++ii)
		{
			worktime += vti[ii].Work;
			idletime += vti[ii].Idle;
		}
		worktime /= vti.size();
		idletime /= vti.size();
	}

	if(worktime < ti->TimeAvr.WorkMin) ti->TimeAvr.WorkMin = worktime;
	if(worktime > ti->TimeAvr.WorkMax) ti->TimeAvr.WorkMax = worktime;
	if(idletime < ti->TimeAvr.IdleMin) ti->TimeAvr.IdleMin = idletime;
	if(idletime > ti->TimeAvr.IdleMax) ti->TimeAvr.IdleMax = idletime;

	ti->TimeAvr.WorkAverage = UDINT((LREAL(ti->TimeAvr.WorkAverage) * LREAL(ti->Counter) + LREAL(worktime)) / LREAL(ti->Counter + 1));
	ti->TimeAvr.IdleAverage = UDINT((LREAL(ti->TimeAvr.IdleAverage) * LREAL(ti->Counter) + LREAL(idletime)) / LREAL(ti->Counter + 1));
	++ti->Counter;

	if(worktime > ti->TimeAvr.WorkAverage || 0 == ti->CntAvrMax)
	{
		ti->TimeAvr.WorkAvgMax = UDINT((LREAL(ti->TimeAvr.WorkAvgMax) * LREAL(ti->CntAvrMax) + LREAL(worktime)) / LREAL(ti->CntAvrMax + 1));
		++ti->CntAvrMax;
	}

	ti->TimeInfo.push_back(new rThreadTimeInfo(worktime, idletime));

	//printf("%s : work avr %u us, avr max %u us, max %u us\n", ti->Class->GetRTTI(), ti->TimeAvr.WorkAverage, ti->TimeAvr.WorkAvgMax, ti->TimeAvr.WorkMax);
    return 0;
}


UDINT rThreadMaster::SaveAllTimerInfo()
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

	for(UDINT ii = 0; ii < SysInfo.size(); ++ii)
	{
		cputext  += String_format("%.2f;", SysInfo[ii].CPUUsage);
		cpumtext += String_format("%.2f;", SysInfo[ii].ModifyCPU);
		memtext  += String_format("%u;"  , SysInfo[ii].FreeMem);
		memmtext += String_format("%i;"  , SysInfo[ii].ModifyMem);
	}
	SysInfo.clear();

	SimpleFileSave(filename, cputext + "\n" + cpumtext + "\n" + memtext + "\n" + memmtext);


	for(UDINT ii = 0; ii < List.size(); ++ii)
	{
		rThreadInfo *thread_info = List[ii];

		filename = String_format("%s%s_%u.csv", DIR_TIMEINFO.c_str(), thread_info->Class->GetRTTI(), utime);
		worktext = "work;";
		idletext = "idle;";

		for(UDINT jj = 0; jj < thread_info->TimeInfo.size(); ++jj)
		{
			worktext += String_format("%u;", thread_info->TimeInfo[jj]->Work);
			idletext += String_format("%u;", thread_info->TimeInfo[jj]->Idle);

			delete thread_info->TimeInfo[jj];
			thread_info->TimeInfo[jj] = nullptr;
		}

		SimpleFileSave(filename, worktext + "\n" + idletext);

		thread_info->TimeInfo.clear();
	}

	return TRITONN_RESULT_OK;
}


UDINT rThreadMaster::GenerateVars(rThreadInfo* ti, const string& alias)
{
	if (!ti || alias.empty()) {
		return TRITONN_RESULT_OK;
	}

	rVariable::ListVar.push_back(new rVariable("system.diag." + alias + ".Work.Max"    , TYPE_UDINT , VARF_R___, &ti->TimeAvr.WorkMax    , U_DIMLESS, 0));
	rVariable::ListVar.push_back(new rVariable("system.diag." + alias + ".Work.Min"    , TYPE_UDINT , VARF_R___, &ti->TimeAvr.WorkMin    , U_DIMLESS, 0));
	rVariable::ListVar.push_back(new rVariable("system.diag." + alias + ".Work.AvgMax" , TYPE_UDINT , VARF_R___, &ti->TimeAvr.WorkAvgMax , U_DIMLESS, 0));
	rVariable::ListVar.push_back(new rVariable("system.diag." + alias + ".Work.Average", TYPE_UDINT , VARF_R___, &ti->TimeAvr.WorkAverage, U_DIMLESS, 0));
	rVariable::ListVar.push_back(new rVariable("system.diag." + alias + ".Idle.Max"    , TYPE_UDINT , VARF_R___, &ti->TimeAvr.IdleMax    , U_DIMLESS, 0));
	rVariable::ListVar.push_back(new rVariable("system.diag." + alias + ".Idle.Min"    , TYPE_UDINT , VARF_R___, &ti->TimeAvr.IdleMin    , U_DIMLESS, 0));
	rVariable::ListVar.push_back(new rVariable("system.diag." + alias + ".Idle.Average", TYPE_UDINT , VARF_R___, &ti->TimeAvr.IdleAverage, U_DIMLESS, 0));

	return TRITONN_RESULT_OK;
}


