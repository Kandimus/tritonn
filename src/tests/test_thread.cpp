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
#include "simpletest.h"

rTestThread::rTestThread()
{
	RTTI = "rTestThread";
}

rTestThread::~rTestThread()
{
}

//-------------------------------------------------------------------------------------------------
//
rThreadStatus rTestThread::Proccesing()
{
	rSimpleTest::Instance().Run();

	Status.Set(rThreadStatus::FINISHED);
	return rThreadStatus::FINISHED;
}
