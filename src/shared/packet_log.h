//=================================================================================================
//===
//=== log_packet.h
//===
//=== Copyright (c) 2003-2013 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс описывающий log-сообщение. Оставлен для возможного переноса в Proto
//===
//=================================================================================================

#pragma once

#include <string>
#include "def.h"

using std::string;


class rPacketLog
{
public:
	UDINT    Marker;                     // [ 4] Маркер пакета
	UINT     Size;                       // [ 2] Длина пакета
	UINT     LineNo;                     // [ 2] Номер линии
	SINT     FileName[MAX_LOG_FILENAME]; // [64] Имя файла, вызвавший лог
	UDT      Date;                       // [ 8] Дата возникновения сообщения, с микросекундами
	UDINT    MIC;                        // [ 4] Инкрементный идентификатор сообщения (Message Increment Count)
	UDINT    Mask;                       // [ 4] Маска сообщения
	SINT     Text[MAX_LOG_TEXT];         // Текст сообщения

	// Конструкторы и деструкторы
	rPacketLog();
	rPacketLog(UDINT mask, UINT lineno, const string &filename);
	virtual ~rPacketLog();
};

const UDINT LENGTH_PACKET_LOG = sizeof(rPacketLog);
const UDINT MARKER_PACKET_LOG = 0x99AF680D; // Маркер пакета с лог-сообщением

