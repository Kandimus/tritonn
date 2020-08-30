//=================================================================================================
//===
//=== packet_getanswe.cpp
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

#include <string.h>
#include "packet_getanswe.h"

rPacketGetAnswe::rPacketGetAnswe()
{
	Data.Marker = MARKER_PACKET_GETANSWE;
	Data.Size   = LENGTH_PACKET_GETANSWE;

	Data.Count = 0;
	memset(Data.Name  , 0, sizeof(Data.Name));
	memset(Data.Value , 0, sizeof(Data.Value));
	memset(Data.Result, 0, sizeof(Data.Result));
}


rPacketGetAnswe::rPacketGetAnswe(rPacketGetAnsweData &data)
{
	Data = data;

	Data.Marker = MARKER_PACKET_GETANSWE;
	Data.Size   = LENGTH_PACKET_GETANSWE;
}

rPacketGetAnswe::~rPacketGetAnswe()
{
	
}


