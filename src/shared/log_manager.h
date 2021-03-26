//=================================================================================================
//===
//=== log_class.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Универсальный класс-поток для выдачи log-сообщений по TCP
//===
//=================================================================================================

#pragma once

//#include <list>
#include "thread_class.h"
#include "singlenton.h"
#include "log_defines.h"

class rLogManager: public rThreadClass
{
	enum
	{
		MAX_TEXT_BUFF = 4096,
	};

	SINGLETON(rLogManager)

public:
	UDINT add(UDINT mask, const char* filesource, UDINT lineno, const char* format, ...);

	// Управление логированием
	UDINT addLogMask(UDINT lm);     // Добавление маски к уже существующей
	UDINT removeLogMask(UDINT lm);  // Удаление маски из существующей
	UDINT setLogMask(UDINT lm);     // Установка новой маски
	
	static void setDir(const std::string& dir) { m_dir = dir; }
	static void outErr(const char *filename, UDINT lineno, const char *format, ...);

	void setAddCalback(Fn_LogAddCallback fn);

protected:
	virtual rThreadStatus Proccesing(void);

private:
	DINT lockCallback()   { return pthread_mutex_lock  (&m_mutexCallback); }
	DINT unlockCallback() { return pthread_mutex_unlock(&m_mutexCallback); }

	static std::string saveLogText(UDINT mask, const UDT* time, const std::string& source, UDINT lineno, const std::string& text);

public:
	rSafityValue<UDINT> m_terminal;  // Включение/выключение дублирования сообщений на консоль
	rSafityValue<UDINT> m_enable;    // Включение/выключение логирования

private:
	rSafityValue<UDINT> m_level;         // Текущий уровень логирования
	pthread_mutex_t     m_mutexCallback; //

	Fn_LogAddCallback   fnAddCalback;

	static std::string  m_dir;
};




