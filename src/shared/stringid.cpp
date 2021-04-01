//=================================================================================================
//===
//=== stringid.cpp
//===
//=== Copyright (c) 2019-2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#pragma once

#include "stringid.h"

Container& rStringID::toContainer(Container& cnt) const
{
	cnt << m_ID;
	return cnt;
}

Container& rStringID::fromContainer(Container& cnt)
{
	cnt >> m_ID;
	return cnt;
}

Container& operator << (Container& cnt, const rStringID& strid)
{
	strid.toContainer(cnt);
	return cnt;
}

Container& operator >> (Container& cnt, rStringID& strid)
{
	strid.fromContainer(cnt);
	return cnt;
}
