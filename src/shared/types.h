//=================================================================================================
//===
//=== threadclass.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Описание базовых типов
//===
//=================================================================================================

#pragma once

class rStringID;

typedef unsigned char        USINT;
typedef unsigned short       UINT;
//typedef uint32_t             UDINT;
typedef unsigned int         UDINT;
typedef          char        SINT;
typedef          short       INT;
typedef          int         DINT;
typedef rStringID            STRID;
typedef float                REAL;
typedef double               LREAL;
typedef const char *         CCHPTR;
typedef struct tm            STM;


///////////////////////////////////////////////////////////////////////////////////////////////////
// Типы времени
#if defined(_WIN32) || defined(WIN32)

typedef uint64_t                  UDT;                       

// UNIX and other
#else

#include <sys/time.h>

typedef struct timeval            UDT;

#endif


///////////////////////////////////////////////////////////////////////////////////////////////////
// Сокеты (попытка унификации между Linux и Windows)
#if defined(_WIN32) || defined(WIN32)

	#define SOCKET_SET_NONBLOCK(x)    {unsigned long Val = 1; ioctlsocket((x), FIONBIO, (unsigned long *)&Val); }
	#define SHUT_RDWR                 SD_BOTH

// UNIX and other
#else

	typedef int                       SOCKET;

	#define SOCKET_ERROR              (-1)
	#define SOCKET_SET_NONBLOCK(x)    {fcntl((x), F_SETFL, O_NONBLOCK);}
	#define SOKET_SET_REUSEPORT(x)    {int optval = 1; setsockopt((x), SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));}

#endif

#define UNUSED(x)                    (void)(x);

