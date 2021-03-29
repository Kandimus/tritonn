//=================================================================================================
//===
//=== system_manager.cpp
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#include "system_manager.h"
#include "log_manager.h"

rSystemManager::rSystemManager()
{
	RTTI = "rSystemManager";
	m_timer.start(COMMAND_TIMER);
}


rSystemManager::~rSystemManager()
{
	m_list.clear();
}

rThreadStatus rSystemManager::Proccesing()
{
	rThreadStatus thread_status = rThreadStatus::UNDEF;

	while(1) {
		// Обработка команд нити
		thread_status = rThreadClass::Proccesing();
		if(!THREAD_IS_WORK(thread_status))
		{
			return thread_status;
		}

		if (m_timer.isFinished()) {
			Lock();
			if (m_list.size()) {
				int result = system(m_list.front().c_str());

				TRACEI(LOG::SYSTEM, "%i <- [%s]", result, m_list.front().c_str());

				m_list.pop_front();
			}
			Unlock();

			m_timer.restart();
		}

		rThreadClass::EndProccesing();
	}

	return rThreadStatus::UNDEF;
}


void rSystemManager::add(const std::string& text)
{
	Lock();
	m_list.push_back(text);
	Unlock();
}

void rSystemManager::addTarByTime(const std::string& path, const std::string& mask, UDINT days, const std::string& tarname)
{
	std::string text = getTarByTime(path, mask,  days, tarname);
	add(text);
}

void rSystemManager::addDelByTime(const std::string& path, const std::string& mask, UDINT days)
{
	std::string text = getDelByTime(path, mask,  days);
	add(text);
}

std::string rSystemManager::getTarByTime(const std::string& path, const std::string& mask, UDINT days, const std::string& tarname)
{
	return "find " + path + "/" + mask + " -type f -mtime +" + String_format("%u", days) +
			" | xargs tar -zcf " + path + "/" + tarname + ".tar.gz";
}

std::string rSystemManager::getDelByTime(const std::string& path, const std::string& mask, UDINT days)
{
	return "find " + path + "/" + mask + " -type f -mtime +" + String_format("%u", days) +
			" | xargs rm -rf";
}
