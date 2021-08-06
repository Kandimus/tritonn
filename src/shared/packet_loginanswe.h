//=================================================================================================
//===
//=== packet_loginanswe.h
//===
//=== Copyright (c) 2003-2013 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс описывающий ответ на сообщение входа пользователя в терминал.
//===
//=================================================================================================

#pragma once

#include <string>
#include "def.h"
#include "structures.h"


using std::string;

struct rPacketLoginAnsweData
{
	UDINT    Marker;                      // [ 4] Маркер пакета
	UINT     Size;                        // [ 2] Длина пакета

	USINT       Access;      //
	rVersion    Version;     //
	rState      State;
//	rConfigInfo Config;
};

class rPacketLoginAnswe
{
public:
	rPacketLoginAnsweData Data;

	// Конструкторы и деструкторы
	rPacketLoginAnswe();
	virtual ~rPacketLoginAnswe();
};

const UDINT LENGTH_PACKET_LOGINANSWE = sizeof(rPacketLoginAnsweData);

