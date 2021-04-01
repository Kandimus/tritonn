//=================================================================================================
//===
//=== stringid.h
//===
//=== Copyright (c) 2019-2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#pragma once

#include <string>
#include "types.h"
#include "container.h"

class rStringID
{
public:
	rStringID() : m_ID(0) { }
	rStringID(UDINT id) : m_ID(id) { }
	virtual ~rStringID() = default;

	rStringID& operator =  (const UDINT&     val) { m_ID = val; return *this; }

	bool       operator != (const UDINT&     val) { return m_ID != val;      }
	bool       operator != (const rStringID& val) { return m_ID != val.m_ID; }
	bool       operator == (const UDINT&     val) { return m_ID == val;      }
	bool       operator == (const rStringID& val) { return m_ID == val.m_ID; }

	operator UDINT() { return m_ID; }
	operator UDINT() const { return m_ID; }
	operator UDINT*() { return &m_ID; }
	operator std::string() const { return "function not implemented"; }

	UDINT *getPtr() { return &m_ID; }
	UDINT  toUDINT() const { return m_ID; }

	Container& toContainer(Container& cnt) const;
	Container& fromContainer(Container& cnt);

protected:
	UDINT m_ID;
};

Container& operator << (Container& cnt, const rStringID& event);
Container& operator >> (Container& cnt, rStringID& event);
