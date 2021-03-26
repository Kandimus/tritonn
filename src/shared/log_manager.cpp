//=================================================================================================
//===
//=== log_manager.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс-поток для выдачи log-сообщений по TCP
//===
//=================================================================================================

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <syslog.h>
#include "log_manager.h"
#include "simplefile.h"
#include "time64.h"

std::string rLogManager::m_dir = "";

rLogManager::rLogManager()
{
	RTTI     = "rLogManager";

	fnAddCalback = nullptr;

	pthread_mutex_init(&m_mutexCallback, NULL);
	
	m_level.Set(LOG::ALL);  //TODO Нужно изменить на нормальный уровень
	m_terminal.Set(false);
	m_enable.Set(false);    //TODO В штатном режиме можно после полной инициализации выключать
}


rLogManager::~rLogManager()
{
	pthread_mutex_destroy(&m_mutexCallback);
}

UDINT rLogManager::add(UDINT mask, const char* filesource, UDINT lineno, const char *format, ...)
{
	UDINT terminal = false;
	UDINT level    = 0;
	
	if (!m_enable.Get()) {
		return TRITONN_RESULT_OK;
	}

	// Принудительно выставляем в маску логирования сообщения от менеджера сообщений
	level = m_level.Get();
	
	// Если маска сообщения не совпадает с глобальным уровнем, то пропускаем это сообщение
	if (!(level & mask)) {
		return TRITONN_RESULT_OK;
	}
	
	// Если установлен флаг LOG:LOGMGR, то это логирование от менеджера логирования xD
	// Сообщения PANIC тоже печатаем всегда.
	terminal = m_terminal.Get() || (mask & (LOG::LOGMGR | LOG::P));
	
	char* buff = new char[MAX_TEXT_BUFF];
	va_list(args);
	va_start(args, format);
	vsnprintf(buff, MAX_TEXT_BUFF, format, args);
	va_end(args);

	auto fulltext = saveLogText(mask, nullptr, filesource, lineno, buff);
	delete buff;

	if(terminal) {
		fprintf(stderr, "%s", fulltext.c_str());
	}

	// Запускаем callback функцию, если она есть
	lockCallback();
	{
		if(fnAddCalback)
		{
			fnAddCalback(fulltext);
		}
	}
	unlockCallback();

	return TRITONN_RESULT_OK;
}


void rLogManager::outErr(const char *filename, UDINT lineno, const char *format, ...)
{
	char* buff = new char[MAX_TEXT_BUFF];
	va_list(args);
	va_start(args, format);
	vsnprintf(buff, MAX_TEXT_BUFF, format, args);
	va_end(args);

	auto fulltext = saveLogText(LOG::P, nullptr, filename, lineno, buff);

	delete buff;

	fprintf(stderr, "%s", fulltext.c_str());
}

//-------------------------------------------------------------------------------------------------
//
rThreadStatus rLogManager::Proccesing()
{
	rThreadStatus thread_status = rThreadStatus::UNDEF;

	while(1)
	{
		// Обработка команд нити
		thread_status = rThreadClass::Proccesing();
		if(!THREAD_IS_WORK(thread_status))
		{
			return thread_status;
		}

		//TODO Удаление старых файлов. или архивация.
		
		rThreadClass::EndProccesing();
	}

	return rThreadStatus::UNDEF;
}


UDINT rLogManager::addLogMask(UDINT lm)
{
	UDINT oldlvl = m_level.Get();

	m_level.Set(oldlvl | lm);

	return oldlvl;
}


UDINT rLogManager::removeLogMask(UDINT lm)
{
	UDINT oldlvl = m_level.Get();

	m_level.Set(oldlvl & (~lm));

	return oldlvl;
}


UDINT rLogManager::setLogMask(UDINT lm)
{
	UDINT oldlvl = m_level.Get();

	m_level.Set(lm);

	return oldlvl;

}


void rLogManager::setAddCalback(Fn_LogAddCallback fn)
{
	lockCallback();
	fnAddCalback = fn;
	unlockCallback();
}

std::string rLogManager::saveLogText(UDINT mask, const UDT* time, const std::string& source, UDINT lineno, const std::string& text)
{
	std::string filename = m_dir;
	char logt[5] = "----";
	tm   dt;
	UDT  curtime;

	// Тип сообщения
	if(mask & LOG::P) logt[0] = 'P';
	if(mask & LOG::A) logt[1] = 'A';
	if(mask & LOG::W) logt[2] = 'W';
	if(mask & LOG::I) logt[3] = 'I';

	if (!time) {
		gettimeofday(&curtime, NULL);
	} else {
		curtime = *time;
	}

	localtime_r(&curtime.tv_sec, &dt);
	filename += String_format("%u.txt", curtime.tv_sec / 86400);

	std::string fulltext = String_format("%02i.%02i.%04i %02i:%02i:%02i.%03li [%s",
										 dt.tm_mday, dt.tm_mon + 1, dt.tm_year + 1900,
										 dt.tm_hour, dt.tm_min, dt.tm_sec, curtime.tv_usec / 1000,
										 logt);

	if (source.size()) {
		fulltext += ":" + source + String_format(":%u", lineno);
	}

	fulltext += String_format("] %08x %s\n", mask, text.c_str());

	SimpleFileAppend(filename, fulltext);

	return fulltext;
}



