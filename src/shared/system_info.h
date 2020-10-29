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
	uint64_t m_user;
	uint64_t m_nice;
	uint64_t m_system;
	uint64_t m_idle;
	uint64_t m_IOWait;
	uint64_t m_IRQ;
	uint64_t m_softIRQ;
	uint64_t m_steal;
	uint64_t m_guest;
	uint64_t m_guestNice;

	bool     calculate();
	uint64_t getActive() { return m_user + m_nice + m_system + m_IRQ + m_softIRQ + m_steal + m_guest + m_guestNice; }
	uint64_t getIdle()   { return m_idle + m_IOWait; }
	uint64_t getTotal()  { return getActive() + getIdle(); }
};


class rSystemInfo
{
public:
	unsigned int m_freeMem;
	int          m_modifyMem;
	float        m_usageCPU;
	float        m_modifyCPU;
	rCPUState    m_CPU;

	bool calculate();
};

