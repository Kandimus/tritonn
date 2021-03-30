//=================================================================================================
//===
//=== log_manager.h
//===
//=== Copyright (c) 2019-2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#pragma once

#include "thread_class.h"
#include "singlenton.h"
#include "tickcount.h"
#include "log_defines.h"

class rDateTime;

class rLogManager: public rThreadClass
{
	enum
	{
		MAX_TEXT_BUFF = 4096,
		SYSTEM_TIMER  = 24 * 60 * 60 * 100,
	};

	const UDINT COMPRESS_DAYS = 95;
	const UDINT DELETE_DAYS   = 366;

	SINGLETON(rLogManager)

public:
	void add(UDINT mask, const char* filesource, UDINT lineno, const char* format, ...);
	void add(UDINT mask, const rDateTime& timestamp, const std::string& text);

	// Управление логированием
	UDINT addLogMask(UDINT lm);     // Добавление маски к уже существующей
	UDINT removeLogMask(UDINT lm);  // Удаление маски из существующей
	UDINT setLogMask(UDINT lm);     // Установка новой маски
	
	void setDir(const std::string& dir) { m_dir = dir; }

	void setAddCalback(Fn_LogAddCallback fn);

protected:
	virtual rThreadStatus Proccesing(void);

private:
//	DINT lockCallback()   { return pthread_mutex_lock  (&m_mutexCallback); }
//	DINT unlockCallback() { return pthread_mutex_unlock(&m_mutexCallback); }

	bool        check(UDINT mask);
	std::string saveLogText(UDINT mask, const rDateTime& timestamp, const std::string& source, UDINT lineno, const std::string& text);
	void        outTerminal(UDINT mask, const std::string& text);

public:
	rSafityValue<UDINT> m_terminal;  // Включение/выключение дублирования сообщений на консоль
	rSafityValue<UDINT> m_enable;    // Включение/выключение логирования

private:
	std::string         m_dir = "";
	rTickCount          m_systimer;
	rSafityValue<UDINT> m_level;         // Текущий уровень логирования
	pthread_mutex_t     m_mutexCallback; //

	Fn_LogAddCallback   fnAddCalback;
};




