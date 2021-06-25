//=================================================================================================
//===
//=== log_manager.cpp
//===
//=== Copyright (c) 2019-2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#include "log_manager.h"
#include <stdarg.h>
#include "system_manager.h"
#include "simplefile.h"
#include "datetime.h"

rLogManager::rLogManager()
{
	RTTI = "rLogManager";

	m_systimer.start(SYSTEM_TIMER);

	m_save.Set(true);
	m_level.Set(LOG::ALL);  //TODO Нужно изменить на нормальный уровень
	m_terminal.Set(false);
	m_enable.Set(false);    //TODO В штатном режиме можно после полной инициализации выключать
}


rLogManager::~rLogManager()
{
}

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

		if (m_systimer.isFinished()) {
			rDateTime dt;

			rSystemManager::instance().addTarByTime(DIR_LOG, "*.log"   , COMPRESS_DAYS, String_format("%u", dt.getSec() / rDateTime::SEC_IN_DAY));
			rSystemManager::instance().addDelByTime(DIR_LOG, "*.log"   , COMPRESS_DAYS);
			rSystemManager::instance().addDelByTime(DIR_LOG, "*.tar.gz", DELETE_DAYS);

			m_systimer.restart();
		}

		rThreadClass::EndProccesing();
	}

	return rThreadStatus::UNDEF;
}


void  rLogManager::add(UDINT mask, const rDateTime& timestamp, const std::string& text)
{
	if (!check(mask)) {
		return;
	}

	auto fulltext = saveLogText(mask, timestamp, "", 0, text);

	outTerminal(mask, fulltext);
}

void rLogManager::add(UDINT mask, const char* filesource, UDINT lineno, const char *format, ...)
{	
	if (!check(mask)) {
		return;
	}

	char* buff = new char[MAX_TEXT_BUFF];
	va_list(args);
	va_start(args, format);
	vsnprintf(buff, MAX_TEXT_BUFF, format, args);
	va_end(args);

	auto fulltext = saveLogText(mask, rDateTime(), filesource, lineno, buff);
	delete[] buff;

	outTerminal(mask, fulltext);
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


bool rLogManager::check(UDINT mask)
{
	if (!m_enable.Get()) {
		return false;
	}

	// Принудительно выставляем в маску логирования сообщения от менеджера сообщений
	UDINT level = m_level.Get();

	// Если маска сообщения не совпадает с глобальным уровнем, то пропускаем это сообщение
	if (!(level & mask)) {
		return false;
	}

	return true;
}


void rLogManager::outTerminal(UDINT mask, const std::string& text)
{
	if (m_terminal.Get() || (mask & (LOG::LOGMGR | LOG::P))) {
		fprintf(stderr, "%s", text.c_str());
	}
}


std::string rLogManager::saveLogText(UDINT mask, const rDateTime& timestamp, const std::string& source, UDINT lineno, const std::string& text)
{
	std::string filename = m_dir + String_format("%u.log", timestamp.getSec() / rDateTime::SEC_IN_DAY);
	char logt[5] = "----";

	// Тип сообщения
	if(mask & LOG::P) logt[0] = 'P';
	if(mask & LOG::A) logt[1] = 'A';
	if(mask & LOG::W) logt[2] = 'W';
	if(mask & LOG::I) logt[3] = 'I';

	std::string fulltext = timestamp.toString() + "[" + logt;

	if (source.size()) {
		fulltext += ":" + source + String_format(":%u", lineno);
	}

	fulltext += String_format("] %08x %s\n", mask, text.c_str());

	if (m_save.Get()) {
		simpleFileAppend(filename, fulltext);
	}

	return fulltext;
}
