//=================================================================================================
//===
//=== packet_set.cpp
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

#include <string.h>
#include "packet_set.h"

rPacketSet::rPacketSet()
{
	Data.Marker = MARKER_PACKET_SET;
	Data.Size   = LENGTH_PACKET_SET;

	Data.Count = 0;
	memset(Data.Name , 0, sizeof(Data.Name));
	memset(Data.Value, 0, sizeof(Data.Value));
}


rPacketSet::rPacketSet(rPacketSetData &data)
{
	Data = data;

	Data.Marker = MARKER_PACKET_SET;
	Data.Size   = LENGTH_PACKET_SET;
}

rPacketSet::~rPacketSet()
{
	
}


