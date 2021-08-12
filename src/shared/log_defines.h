//=================================================================================================
//===
//=== log_defines.h
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#pragma once

#include <string>

enum LOG : UDINT
{
	I        = 0x00000001,
	W        = 0x00000002,
	A        = 0x00000004,
	P        = 0x00000008,
	SYSTEM   = 0x00000010,
	TCPSERV  = 0x00000020,
	TCPCLNT  = 0x00000040,
	EVENT    = 0x00000080,
	TERMINAL = 0x00000100,
	TEXT     = 0x00000200,
	OPCUA    = 0x00000400,
	LINK     = 0x00000800,
	JSON     = 0x00001000,
	THREAD   = 0x00002000,
	SYSVAR   = 0x00004000,
	PACKET   = 0x00008000,

	MAIN     = 0x01000000,
	CONFIG   = 0x02000000,
	DATAMGR  = 0x04000000,
	EVENTMGR = 0x08000000,

	LOGMGR   = 0x80000000,
	ALL      = 0xFFFFFFFF,
};

#define TRACE(mask, format, ...)              { rLogManager::instance().add(         (mask), __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define TRACEI(mask, format, ...)             { rLogManager::instance().add(LOG::I | (mask), __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define TRACEW(mask, format, ...)             { rLogManager::instance().add(LOG::W | (mask), __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define TRACEA(mask, format, ...)             { rLogManager::instance().add(LOG::A | (mask), __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define TRACEP(mask, format, ...)             { rLogManager::instance().add(LOG::P | (mask), __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }



