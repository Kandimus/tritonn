//=================================================================================================
//===
//=== packet_set.h
//===
//=== Copyright (c) 2003-2013 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс описывающий сообщение записи переменных в Tritonn.
//===
//=================================================================================================

#pragma once

#include <string>
#include "def.h"


struct rPacketSetData
{
	UDINT    Marker;                      // [ 4] Маркер пакета
	UINT     Size;                        // [ 2] Длина пакета

	SINT     Name[MAX_PACKET_SET_COUNT][MAX_VARIABLE_LENGTH];
	SINT     Value[MAX_PACKET_SET_COUNT][MAX_VARVALUE_LENGTH];
	UDINT    Count;
	UDINT    UserData;
};

class rPacketSet
{
public:
	rPacketSetData Data;

	// Конструкторы и деструкторы
	rPacketSet();
	rPacketSet(rPacketSetData &data);
	virtual ~rPacketSet();

};

const UDINT LENGTH_PACKET_SET = sizeof(rPacketSetData);


