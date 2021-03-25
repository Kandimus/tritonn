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

typedef void ( *Fn_LogAddCallback)(const std::string&);

enum LOG : UINT
{
	I        = 0x0001,
	W        = 0x0002,
	A        = 0x0004,
	P        = 0x0008,
	SYSTEM   = 0x0010,
	TCPSERV  = 0x0020,
	TCPCLNT  = 0x0040,
	EVENT    = 0x0080,
	TERMINAL = 0x0100,
	TEXT     = 0x0200,
	OPCUA    = 0x0400,
	LOGMGR   = 0x8000,
	ALL      = 0xFFFF,
};

#define TRACE(mask, format, ...)              { rLogManager::instance().add(         (mask), __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define TRACEI(mask, format, ...)             { rLogManager::instance().add(LOG::I | (mask), __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define TRACEW(mask, format, ...)             { rLogManager::instance().add(LOG::W | (mask), __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define TRACEA(mask, format, ...)             { rLogManager::instance().add(LOG::A | (mask), __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define TRACEP(mask, format, ...)             { rLogManager::instance().add(LOG::P | (mask), __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define TRACEERROR(format, ...)               { rLogManager::outErr(                         __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }



