//=================================================================================================
//===
//=== users.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс пользователя
//===
//=================================================================================================


#include <string.h>
#include "hash.h"
#include "users.h"


vector<rUser *> rUser::ListUser;



rUser::rUser(const string &name, USINT *pwd_hash, UDINT intaccess, UDINT extaccess, UDINT login, USINT *loginpwd_hash)
{
	memset(Hash, 0, MAX_HASH_SIZE);

	if(pwd_hash     ) memcpy(Pwd          , pwd_hash     , MAX_HASH_SIZE);
	if(loginpwd_hash) memcpy(Interface.Pwd, loginpwd_hash, MAX_HASH_SIZE);

	GetStrHash(name, Hash);

	Name            = name;
	IntAccess       = intaccess;
	ExtAccess       = extaccess;
	Status          = 0;
	Interface.Login = login;
}


//-------------------------------------------------------------------------------------------------
//
rUser::~rUser()
{
	;
}


UDINT rUser::GetAccess(UDINT external) const { return (external) ? ExtAccess : IntAccess; }

UDINT rUser::CheckAccess(UDINT access, UDINT external) const { return (external) ? (ExtAccess & access) : (IntAccess & access); }


UDINT rUser::GetIntAccess() const
{
	return IntAccess;
}


//
UDINT rUser::GetExtAccess() const
{
	return ExtAccess;
}


//
USINT rUser::GetStatus() const
{
	return Status;
}

string rUser::GetName() const
{
	return Name;
}


//
const rUser *rUser::Find(const string &name)
{
	USINT  hash[MAX_HASH_SIZE];

	GetStrHash(name, hash);

	return FindHash(hash);
}


//
const rUser *rUser::FindHash(USINT *name_hash)
{
	for(UDINT ii = 0; ii < ListUser.size(); ++ii)
	{
		if(CmpHash(ListUser[ii]->Hash, name_hash))
		{
			return ListUser[ii];
		}
	}

	return nullptr;
}


//
const rUser *rUser::Find(UDINT login)
{
	if(!login) return nullptr;

	for(UDINT ii = 0; ii < ListUser.size(); ++ii)
	{
		if(ListUser[ii]->Interface.Login == login)
		{
			return ListUser[ii];
		}
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rUser::Add(const string &name, const string &pwd, UDINT intaccess, UDINT extaccess, UDINT login, USINT *loginpwd_hash)
{
	const rUser *user = Find(name);

	if(nullptr != user) return 1;

	USINT pwd_hash[MAX_HASH_SIZE];

	GetStrHash(pwd, pwd_hash);

	ListUser.push_back(new rUser(name, pwd_hash, intaccess, extaccess, login, loginpwd_hash));

	return 0;
}



UDINT rUser::Add(const string &name, USINT *pwd_hash, UDINT intaccess, UDINT extaccess, UDINT login, USINT *loginpwd_hash)
{
	const rUser *user = Find(name);

	if(nullptr != user) return 1;

	ListUser.push_back(new rUser(name, pwd_hash, intaccess, extaccess, login, loginpwd_hash));

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
const rUser *rUser::Login(const string &name, USINT *pwd, UDINT &result)
{
	USINT  hash[MAX_HASH_SIZE];

	GetStrHash(name, hash);

	return LoginHash(hash, pwd, result);
}


const rUser *rUser::LoginHash(USINT *name_hash, USINT *pwd_hash, UDINT &result)
{
	const rUser *user = FindHash(name_hash);

	// Проверка что такой пользователь существует
	if(nullptr == user)
	{
		result = LOGIN_FAULT;
		return nullptr;
	}

	// Проверка на блокировку пользователя
	if(user->GetStatus() & USER_MASK_BLOCKED)
	{
		result = LOGIN_BLOCKED;
		return nullptr;
	}

	// Проверка пароля
	if(!CmpHash(user->Pwd, pwd_hash))
	{
		result = LOGIN_FAULT;
		return nullptr;
	}

	// Пароли совпали
	result = (user->GetStatus() & USER_CHANGEPWD) ? LOGIN_CHANGEPWD : LOGIN_OK;
	return user;
}


//
const rUser *rUser::Login(UDINT login, UDINT pwd, UDINT &result)
{
	const rUser *user = Find(login);

	if(nullptr == user)
	{
		result = LOGIN_FAULT;
		return nullptr;
	}

	// Проверка на блокировку пользователя
	if(user->GetStatus() & USER_MASK_BLOCKED)
	{
		result = LOGIN_BLOCKED;
		return nullptr;
	}

	// Проверка пароля
	USINT  hash[MAX_HASH_SIZE];

	GetBufHash(&pwd, sizeof(pwd), hash);
	if(!CmpHash(user->Interface.Pwd, hash))
	{
		result = LOGIN_FAULT;
		return nullptr;
	}

	// Пароли совпали
	result = LOGIN_OK;
	return user;
}

//-------------------------------------------------------------------------------------------------
//
void rUser::DeleteAll()
{
	for(UDINT ii = 0; ii < ListUser.size(); ++ii)
	{
		delete ListUser[ii];
	}
	ListUser.clear();
}



