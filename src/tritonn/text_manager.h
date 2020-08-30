//=================================================================================================
//===
//=== text_class.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Синглтон обработки строк
//===
//=================================================================================================

#pragma once

#include "text_class.h"


class rTextManager : public rTextClass
{
public:
	virtual ~rTextManager();

// Singleton
private:
	rTextManager();
	rTextManager(const rTextManager &);
	rTextManager& operator=(rTextManager &);

public:
	static rTextManager &Instance();
};



