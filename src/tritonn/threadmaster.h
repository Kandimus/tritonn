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
//#include "safity.h"
#include "thread_class.h"
#include "structures.h"

using std::vector;


//
struct rThreadInfo
{
	UDINT          Flags;
	rThreadStatus  Status;
	pthread_t*     Thread;
	rThreadClass*  Class;
	rThreadTimeAvr TimeAvr;
	UDINT          Counter;
	UDINT          CntAvrMax;

	vector<rThreadTimeInfo *> TimeInfo;
};


struct rSysInfo
{
	UDINT FreeMem;
	DINT  ModifyMem;
	REAL  CPUUsage;
	REAL  ModifyCPU;
};


struct rCPUState
{
	 uint64_t User;
	 uint64_t Nice;
	 uint64_t System;
	 uint64_t Idle;
	 uint64_t IOWait;
	 uint64_t IRQ;
	 uint64_t SoftIRQ;
	 uint64_t Steal;
	 uint64_t Guest;
	 uint64_t GuestNice;

	 inline uint64_t GetActive() { return User + Nice + System + IRQ + SoftIRQ + Steal + Guest + GuestNice; }
	 inline uint64_t GetIdle()   { return Idle + IOWait; }
	 inline uint64_t GetTotal()  { return GetActive() + GetIdle(); }
};


//-------------------------------------------------------------------------------------------------
//
class rThreadMaster : public rThreadClass
{
	SINGLETON(rThreadMaster)

// Методы
public:
	// Работа с нитями
	UDINT Add(rThreadClass* thread, UDINT flags, const string& alias);

	// Командная строка
	UDINT       ParseArgs(int argc, const char** argv);
	rArguments *GetArg();

protected:
	virtual rThreadStatus Proccesing();

	void  CloseAll();

	UDINT GetCPUState(rCPUState& cpu);
	UDINT CalcSysInfo(rCPUState& cpu_prev);
	UDINT CalcThreadTimeInfo(rThreadInfo* ti);
	UDINT SaveAllTimerInfo();
	UDINT GenerateVars(rThreadInfo* ti, const string& alias);

private:
	vector<rThreadInfo *> List; // Список всех нитей
	vector<rSysInfo>      SysInfo;  //
	rArguments            Arg;
	rSysInfo              CurSysInfo;
	UDINT                 TimerNotRun = 5000;

};



