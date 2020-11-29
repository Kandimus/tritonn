//=================================================================================================
//===
//=== test_thread.h
//===
//=== Copyright (c) 2020 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс-нить для тестов
//===
//=================================================================================================

#pragma once

#include "thread_class.h"

class rTestThread : public rThreadClass
{
	SINGLETON(rTestThread)

public:
	virtual rThreadStatus Proccesing();

public:
	void setArgs(int argc, char* argv[]);

public:
	static const UDINT SleepValue;

private:
	int    m_argc = 0;
	char** m_argv = nullptr;
};
