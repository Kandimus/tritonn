//=================================================================================================
//===
//=== packet_setanswe.h
//===
//=== Copyright (c) 2003-2013 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс описывающий ответ на сообщение записи переменных в Tritonn.
//===
//=================================================================================================

#pragma once

#include <string>
#include "def.h"


struct rPacketSetAnsweData
{
	UDINT    Marker;                      // [ 4] Маркер пакета
	UINT     Size;                        // [ 2] Длина пакета

	SINT     Name  [MAX_PACKET_SET_COUNT][MAX_VARIABLE_LENGTH];
	SINT     Value [MAX_PACKET_SET_COUNT][MAX_VARVALUE_LENGTH];
	USINT    Result[MAX_PACKET_SET_COUNT];
	UDINT    Count;
	UDINT    UserData;
};


class rPacketSetAnswe
{
public:
	rPacketSetAnsweData Data;

	// Конструкторы и деструкторы
	rPacketSetAnswe();
	rPacketSetAnswe(rPacketSetAnsweData &data);
	virtual ~rPacketSetAnswe();
};

const UDINT LENGTH_PACKET_SETANSWE = sizeof(rPacketSetAnsweData);


