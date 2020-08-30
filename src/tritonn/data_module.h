//=================================================================================================
//===
//=== data_module.h
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

#pragma once

//#include "tinyxml2.h"
#include "def.h"

namespace tinyxml2
{
	class XMLElement;
};

class rSource;
class rSnapshot;

class rConfigModule
{
public:
	rConfigModule();
	virtual ~rConfigModule();
	
	rSource *Channel[MAX_MODULE_CHANNEL];

//	virtual UDINT LoadChannel(tinyxml2::XMLElement *ch, rSnapshot &snapshot) = 0;

protected:
	UDINT MaxChannel;
};


class rModuleAI4 : public rConfigModule
{
	rModuleAI4();
	virtual ~rModuleAI4();

	//virtual UDINT LoadChannel(tinyxml2::XMLElement *ch, rSnapshot &snapshot);
};


