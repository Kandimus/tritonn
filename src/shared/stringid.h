//=================================================================================================
//===
//=== threadclass.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс обертка, для отделения STRID от типа UDINT
//===
//=================================================================================================

#pragma once

#include "types.h"

class rStringID
{
public:
	rStringID() { ID = 0; }
	rStringID(UDINT id) { ID = id; }
	virtual ~rStringID() { ; }

	rStringID & operator = (const UDINT &val) { ID = val; return *this; }
	operator UDINT() { return ID; }
	operator UDINT() const { return ID; }
	operator UDINT*() { return &ID; }
	operator std::string() const { return "function not implemented"; }

	UDINT *GetPtr() { return &ID; }
	UDINT  toUDINT() const { return ID; }

protected:
	UDINT ID;
};

