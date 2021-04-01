//=================================================================================================
//===
//=== system_manager.h
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#pragma once

#include <list>
#include "thread_class.h"
#include "singlenton.h"
#include "tickcount.h"

class rSystemManager: public rThreadClass
{
	SINGLETON(rSystemManager)

public:
	void add(const std::string& command);
	void addTarByTime(const std::string& path, const std::string& mask, UDINT days, const std::string& tarname);
	void addDelByTime(const std::string& path, const std::string& mask, UDINT days);

	std::string getTarByTime(const std::string& path, const std::string& mask, UDINT days, const std::string& tarname);
	std::string getDelByTime(const std::string& path, const std::string& mask, UDINT days);

protected:
	virtual rThreadStatus Proccesing(void);

private:

	enum
	{
		COMMAND_TIMER = 1000,
	};

	std::list<std::string> m_list;
	rTickCount m_timer;
};




