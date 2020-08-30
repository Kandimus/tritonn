//=================================================================================================
//===
//=== log_packet.cpp
//===
//=== Copyright (c) 2003-2013 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс описывающий log-сообщение.
//===
//=================================================================================================


#include <string.h> 
#include "packet_log.h"

rPacketLog::rPacketLog()
{
	Marker      = MARKER_PACKET_LOG;
	Size        = LENGTH_PACKET_LOG;
	Text[0]     = 0;
	MIC         = 0;
	FileName[0] = 0;
	LineNo      = -1;
	Mask        = 0;
}


rPacketLog::rPacketLog(UDINT mask, UINT lineno, const string &filename)
{
#if defined(_WIN32) || defined(WIN32)
	//TODO Что тут писать-то? Есть в винде аналог этой функции и нужна ли она тут?
#else
	gettimeofday(&Date, NULL);
#endif

	Mask    = mask;
	MIC     = -1;
	Size    = 0;
	Text[0] = 0;
	LineNo  = lineno;
	strncpy(FileName, filename.c_str(), MAX_LOG_FILENAME - 1);
	FileName[filename.size()] = 0;
}

rPacketLog::~rPacketLog()
{
	
}
