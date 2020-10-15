//=================================================================================================
//===
//=== io_basechannel.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Базовый класс канала
//===
//=================================================================================================

#pragma once


class rIOBaseChannel
{
public:

	rIOBaseChannel() {}
	virtual ~rIOBaseChannel() {}

protected:
	virtual UDINT simulate();

protected:
	USINT m_simType = 0;
};



