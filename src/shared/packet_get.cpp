//=================================================================================================
//===
//=== packet_get.cpp
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

#include <string.h>
#include "packet_get.h"

rPacketGet::rPacketGet()
{
	Data.Marker = MARKER_PACKET_GET;
	Data.Size   = LENGTH_PACKET_GET;

	Data.Count = 0;
	memset(Data.Name , 0, sizeof(Data.Name));
}


rPacketGet::rPacketGet(rPacketGetData &data)
{
	Data = data;

	Data.Marker = MARKER_PACKET_GET;
	Data.Size   = LENGTH_PACKET_GET;
}

rPacketGet::~rPacketGet()
{
	
}


