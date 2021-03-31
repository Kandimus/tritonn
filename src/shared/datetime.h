//=================================================================================================
//===
//=== datetime.h
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#pragma once

#include <string>
#include "types.h"
#include "container.h"

class rDateTime
{
public:

	enum : UDINT
	{
		SEC_IN_DAY = 24 * 60 * 60,
	};

	rDateTime();
	virtual ~rDateTime() = default;

	std::string toString() const;
	const UDT& getUDT() const { return m_datetime; }
	UDINT getSec() const { return m_datetime.tv_sec; }
	UDINT getUsec() const { return m_datetime.tv_usec; }

	void setCurTime();
	void clear();

	Container& toContainer(Container& cnt) const;
	Container& fromContainer(Container& cnt);

private:
	UDT m_datetime;
};

Container& operator << (Container& cnt, const rDateTime& event);
Container& operator >> (Container& cnt, rDateTime& event);
