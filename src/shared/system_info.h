//=================================================================================================
//===
//=== system_info.h
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

#pragma once

#include "def.h"
#include <vector>

class rCPUState
{
public:
	uint64_t m_user      = 0;
	uint64_t m_nice      = 0;
	uint64_t m_system    = 0;
	uint64_t m_idle      = 0;
	uint64_t m_IOWait    = 0;
	uint64_t m_IRQ       = 0;
	uint64_t m_softIRQ   = 0;
	uint64_t m_steal     = 0;
	uint64_t m_guest     = 0;
	uint64_t m_guestNice = 0;

	bool     calculate();
	uint64_t getActive() { return m_user + m_nice + m_system + m_IRQ + m_softIRQ + m_steal + m_guest + m_guestNice; }
	uint64_t getIdle()   { return m_idle + m_IOWait; }
	uint64_t getTotal()  { return getActive() + getIdle(); }
	float    getUsage()  { return 100.f * static_cast<float>(getActive()) / static_cast<float>(getTotal()); }
	bool     isInit()    { return m_user || m_nice || m_system || m_idle; }
};


class rSystemInfo
{
public:
	unsigned int m_freeMem    = 0;
//	unsigned int m_availableMem = 0;
	int          m_modifyMem  = 0;
	float        m_usageCPU   = 0.0f;
	float        m_modifyCPU  = 0.0f;
	rCPUState    m_oldCPU;

	bool calculate();
};

