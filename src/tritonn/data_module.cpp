//=================================================================================================
//===
//=== data_module.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Классы для описания модулей ввода-вывода
//===
//=================================================================================================


#include "tinyxml2.h"
#include "data_snapshot.h"
#include "data_source.h"
#include "data_module.h"

using namespace tinyxml2;


rConfigModule::rConfigModule()
{
	for(UDINT ii = 0; ii < MAX_MODULE_CHANNEL; ++ii)
	{
		Channel[ii] = nullptr;
	}
}

rConfigModule::~rConfigModule()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////

rModuleAI4::rModuleAI4() : rConfigModule()
{
	MaxChannel = 4;
}

rModuleAI4::~rModuleAI4()
{

}


