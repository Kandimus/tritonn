//=================================================================================================
//===
//=== packet_answer.h
//===
//=== Copyright (c) 2003-2013 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс описывающий сообщение ответа в терминал.
//===
//=================================================================================================

#pragma once

#include <string>
#include "def.h"

const UDINT MAX_ANSWE_VAR_LEN   = 112;
const UDINT MAX_ANSWE_DATA      = 64;

const UDINT AVS_EMPTY     = 0;
const UDINT AVS_OK        = 1; // Удачная запись или чтение
const UDINT AVN_NOTFOUND  = 2;
const UDINT AVN_READONLY  = 3;


struct rAnsweData   // [128]
{
	UINT    Status;
	TYPE    Type;
	char    Name[MAX_ANSWE_VAR_LEN];  // [ 64]
	LREAL   Value;      // [  8]
};


class rPacketAnswe
{
public:
	UDINT      Marker;                // [ 4] Маркер пакета
	UINT       Size;                  // [ 2] Длина пакета
	USINT      Count;                 // [20] Хеш имени пользователя
	rAnsweData Data[MAX_ANSWE_DATA];  // [20] Хеш пароля пользователя

	// Конструкторы и деструкторы
	rPacketAnswe();
	virtual ~rPacketAnswe();
};


const UDINT LENGTH_PACKET_ANSWE = sizeof(rPacketAnswe);
const UDINT MARKER_PACKET_ANSWE = 0x56CD0E2B; // Маркер пакета с ответом на команду


