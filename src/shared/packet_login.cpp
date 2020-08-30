//=================================================================================================
//===
//=== packet_login.cpp
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

#include <string.h>
#include "packet_login.h"

rPacketLogin::rPacketLogin()
{
	Data.Marker = MARKER_PACKET_LOGIN;
	Data.Size   = LENGTH_PACKET_LOGIN;

	memset(Data.UserName, 0, sizeof(Data.UserName));
	memset(Data.UserPwd , 0, sizeof(Data.UserPwd));
}


rPacketLogin::~rPacketLogin()
{
	
}


UDINT rPacketLogin::SetUserName(const string &name)
{
	return GetHash(name, Data.UserName);
}

UDINT rPacketLogin::SetUserPwd (const string &pwd)
{
	return GetHash(pwd, Data.UserPwd);
}


UDINT rPacketLogin::GetHash(const string &str, USINT *buff)
{
	UDINT       result = -1;
	EVP_MD_CTX *context = EVP_MD_CTX_create();

	if(!context) return result;

	if(EVP_DigestInit_ex(context, EVP_sha1(), NULL))
	{
		if(EVP_DigestUpdate(context, str.c_str(), str.length()))
		{
			unsigned int lengthOfHash = 0;

			if(EVP_DigestFinal_ex(context, buff, &lengthOfHash))
			{
				result = 0;
			}
		}
	}

	EVP_MD_CTX_destroy(context);

	return result;
}
