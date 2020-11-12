//=================================================================================================
//===
//=== test_thread.cpp
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

#include "test_thread.h"
#define CATCH_CONFIG_RUNNER
#include "../catchtest/catch.hpp"

rTestThread::rTestThread()
{
	RTTI = "rTestThread";
}

rTestThread::~rTestThread()
{
}

void rTestThread::setArgs(int argc, char* argv[])
{
	m_argc = argc;
	m_argv = argv;
}

//-------------------------------------------------------------------------------------------------
//
rThreadStatus rTestThread::Proccesing()
{
	rThreadClass::Proccesing();

	Catch::Session().run(m_argc, m_argv);

	Finish();
	rThreadClass::Proccesing();

	return rThreadStatus::FINISHED;
}
