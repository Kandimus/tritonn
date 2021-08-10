//=================================================================================================
//===
//=== packet_get.h
//===
//=== Copyright (c) 2003-2013 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс описывающий сообщение чтения переменных из Tritonn.
//===
//=================================================================================================

#pragma once

#include "def.h"


struct rPacketGetData
{
	UDINT    Marker;                      // [ 4] Маркер пакета
	UINT     Size;                        // [ 2] Длина пакета

//	SINT     Name[MAX_PACKET_GET_COUNT][MAX_VARIABLE_LENGTH];
	UDINT    Count;
	UDINT    UserData;
};

class rPacketGet
{
public:
	rPacketGetData Data;

	// Конструкторы и деструкторы
	rPacketGet();
	rPacketGet(rPacketGetData &data);
	virtual ~rPacketGet();

};

const UDINT LENGTH_PACKET_GET = sizeof(rPacketGetData);


