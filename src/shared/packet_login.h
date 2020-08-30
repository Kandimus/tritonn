//=================================================================================================
//===
//=== packet_login.h
//===
//=== Copyright (c) 2003-2013 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс описывающий сообщение входа пользователя в терминал.
//===
//=================================================================================================

#pragma once

#include <string>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include "def.h"

using std::string;


struct rPacketLoginData
{
	UDINT    Marker;                      // [ 4] Маркер пакета
	UINT     Size;                        // [ 2] Длина пакета
	USINT    UserName[SHA_DIGEST_LENGTH]; // [20] Хеш имени пользователя
	USINT    UserPwd[SHA_DIGEST_LENGTH];  // [20] Хеш пароля пользователя
};

class rPacketLogin
{
public:
	rPacketLoginData Data;

	// Конструкторы и деструкторы
	rPacketLogin();
	virtual ~rPacketLogin();

	UDINT SetUserName(const string &name);
	UDINT SetUserPwd (const string &pwd);

protected:
	UDINT GetHash(const string &str, USINT *buff);
};

const UDINT LENGTH_PACKET_LOGIN = sizeof(rPacketLoginData);


