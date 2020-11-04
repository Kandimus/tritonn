//=================================================================================================
//===
//=== system_info.cpp
//===
//=== Copyright (c) 2020 by VeduN, RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс для получения информации о процессоре и памяти
//===
//=================================================================================================


#include <string.h>
#include <sys/sysinfo.h>
#include "system_info.h"


bool rSystemInfo::calculate()
{
	struct sysinfo sys_info;
	rCPUState      cpu;

	if(sysinfo(&sys_info) != -1)
	{
		__kernel_ulong_t freemem      = (sys_info.freeram * sys_info.mem_unit) / 1024;

		m_modifyMem = freemem - m_freeMem;
		m_freeMem   = freemem;
//		m_availableMem =
	}

	cpu.calculate();

	float active = cpu.getActive() - m_CPU.getActive();
	float idle   = cpu.getIdle()   - m_CPU.getIdle();
	float total  = active + idle;
	float usage  = (100.f * active / total);

	m_CPU       = cpu;
	m_modifyCPU = usage - m_usageCPU;
	m_usageCPU  = usage;

	return true;
}


//
bool rCPUState::calculate()
{
	FILE *fstat = fopen("/proc/stat", "r");

	if (fstat == nullptr) {
		return false;
	}

	//read values from /proc/pid/stat
	if (fscanf(fstat, "%*s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu", &m_user, &m_nice, &m_system, &m_idle, &m_IOWait, &m_IRQ, &m_softIRQ, &m_steal, &m_guest, &m_guestNice) == EOF) {
		fclose(fstat);
		return false;
	}
	fclose(fstat);

	return true;
}
