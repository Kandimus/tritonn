//=================================================================================================
//===
//=== packet_answe.cpp
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

#include <string.h>
#include "packet_answe.h"

rPacketAnswe::rPacketAnswe()
{
	Marker = MARKER_PACKET_ANSWE;
	Size   = LENGTH_PACKET_ANSWE;
	Count  = 0;
}


rPacketAnswe::~rPacketAnswe()
{
	
}
