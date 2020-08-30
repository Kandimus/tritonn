//=================================================================================================
//===
//=== packet_setanswe.cpp
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

#include <string.h>
#include "packet_setanswe.h"

rPacketSetAnswe::rPacketSetAnswe()
{
	Data.Marker = MARKER_PACKET_SETANSWE;
	Data.Size   = LENGTH_PACKET_SETANSWE;

	Data.Count = 0;
	memset(Data.Name  , 0, sizeof(Data.Name));
	memset(Data.Value , 0, sizeof(Data.Value));
	memset(Data.Result, 0, sizeof(Data.Result));
}


rPacketSetAnswe::rPacketSetAnswe(rPacketSetAnsweData &data)
{
	Data = data;

	Data.Marker = MARKER_PACKET_SETANSWE;
	Data.Size   = LENGTH_PACKET_SETANSWE;
}

rPacketSetAnswe::~rPacketSetAnswe()
{
	
}


