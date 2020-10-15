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
#include "tickcount.h"
#include "data_variable.h"
#include "threadmaster.h"
#include "log_manager.h"
#include "stringex.h"
#include "simplefile.h"
#include "units.h"


rThreadMaster::rThreadMaster()
{
	RTTI          = "rThreadMaster";
	Arg.m_forceRun  = false;
	Arg.m_forceConf = "";
	Arg.m_forceRun  = false;
	Arg.m_logMask   = LM_ALL;
	Arg.m_simulateIO = false;
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
UDINT rThreadMaster::Add(rThreadClass *thread, UDINT flags, const string& alias)
{
	rLocker lock(Mutex); lock.Nop();

	rThreadInfo *info = new rThreadInfo();

	info->Class           = thread;
	info->Flags           = flags;
	info->Thread          = thread->GetThread();
	info->Counter         = 0;
	info->CntAvrMax       = 0;
	info->TimeAvr.IdleMin = 0xFFFFFFFF;
	info->TimeAvr.WorkMin = 0xFFFFFFFF;

	List.push_back(info);

	GenerateVars(info, alias);

	if(flags & TMF_NOTRUN) return TRITONN_RESULT_OK;

	do
	{
		mSleep(50);

		info->Status = info->Class->GetStatus();

		if(info->Status == rThreadStatus::FINISHED)
		{
			TRACEERROR("Can't run thread.");
			exit(0); //NOTE Нужно ли так жестко, может быть Halt?
			return 1;
		}
	}
	while(info->Status != rThreadStatus::RUNNING);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
// Запись данных в менеджер данных
UDINT rThreadMaster::ParseArgs(int argc, const char** argv)
{
	for(int ii = 1; ii < argc; ++ii)
	{
		std::string arg = std::string(argv[ii]);
		std::string arg2 = ii + 1 < argc ? argv[ii + 1] : "";

		if (arg == "--forcerun" || arg == "-f") {
			Arg.m_forceRun = 1;
			continue;
		}

		if (arg == "--terminal" || arg == "-t") {
			Arg.m_terminalOut = 1;
			continue;
		}

		if (arg == "--config" || arg == "-c") {
			Arg.m_forceConf = arg2;
			++ii;
			continue;
		}

		if (arg == "--log" || arg == "-l") {
			if(!String_IsValidHex(arg2.c_str(), Arg.m_logMask)) {
				Arg.m_logMask = LM_ALL;
			}
			++ii;
			continue;
		}
	}

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
rArguments *rThreadMaster::GetArg()
{
	return &Arg;
}


//-------------------------------------------------------------------------------------------------
//
void rThreadMaster::CloseAll()
{
	for (DINT ii = List.size() - 1; ii >= 0; --ii) {
		string name = List[ii]->Class->GetRTTI();

		List[ii]->Class->Finish();
		pthread_join(*List[ii]->Thread, NULL);

		if((List[ii]->Flags & TMF_DELETE) && List[ii]->Class)
		{
			delete List[ii]->Class;
		}

		delete List[ii];

		TRACEERROR("--------- Поток %s закрыт!", name.c_str());
	}

	List.clear();

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


