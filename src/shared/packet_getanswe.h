//=================================================================================================
//===
//=== packet_getanswe.h
//===
//=== Copyright (c) 2003-2013 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс описывающий ответ на сообщение чтения переменных из Tritonn.
//===
//=================================================================================================

#pragma once

#include "def.h"


struct rPacketGetAnsweData
{
	UDINT    Marker;                      // [ 4] Маркер пакета
	UINT     Size;                        // [ 2] Длина пакета

	SINT     Name  [MAX_PACKET_GET_COUNT][MAX_VARIABLE_LENGTH];
	SINT     Value [MAX_PACKET_GET_COUNT][MAX_VARVALUE_LENGTH];
	USINT    Result[MAX_PACKET_GET_COUNT];
	UDINT    Count;
	UDINT    UserData;
};


class rPacketGetAnswe
{
public:
	rPacketGetAnsweData Data;

	// Конструкторы и деструкторы
	rPacketGetAnswe();
	rPacketGetAnswe(rPacketGetAnsweData &data);
	virtual ~rPacketGetAnswe();
};

const UDINT LENGTH_PACKET_GETANSWE = sizeof(rPacketGetAnsweData);


