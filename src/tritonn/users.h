//=================================================================================================
//===
//=== users.h
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

#pragma once

#include <vector>
#include "string.h"
#include "def.h"

class rGeneratorMD;

//-------------------------------------------------------------------------------------------------
//
class rUser
{
public:
	enum class LoginResult : UDINT
	{
		SUCCESS = 0, // Логин успешен
		FAULT, // Логин или пароль не верен
		BLOCKED,  // Пользователь заблокирован
		CHANGEPWD,  // Пользователю нужно сменить пароль
	};

	enum UserFlags : USINT
	{
		CHANGEPWD = 0x01,       // Пользователь должен сменить пароль
		MASK_BLOCKED = 0xF0,
		BLOCKEDMANUAL = 0x10,   // Пользователя заблокировал администратор
		BLOCKEDAUTOMAT = 0x20,  // Пользователь заблокирован после 3-х попыток ввода пароля

	};

//	friend class rDataManager;
private:
	struct rInterface
	{
		UDINT Login;
		USINT Pwd[MAX_HASH_SIZE];

		rInterface()
		{
			Login = 0;
			memset(Pwd, 0, MAX_HASH_SIZE);
		}
	};

public:
	virtual ~rUser() = default;

	UDINT  GetAccess  (UDINT external = false) const;
	UDINT  CheckAccess(UDINT access, UDINT external = false) const;
	UDINT  GetIntAccess() const;
	UDINT  GetExtAccess() const;

	USINT  GetStatus()    const;
	string GetName()      const;

	static void generateMarkDown(rGeneratorMD& md);

	// Добавление пользователя
	static UDINT        Add      (const string &name, const string &pwd, UDINT intaccess, UDINT extaccess, UDINT login, USINT *loginpwd_hash);
	static UDINT        Add      (const string &name, USINT   *pwd_hash, UDINT intaccess, UDINT extaccess, UDINT login, USINT *loginpwd_hash);
	// Поиск пользователя
	static const rUser *Find     (const string &name);
	static const rUser *Find     (UDINT login);
	// Проверка имени/логина и пароля
	static const rUser *Login    (const string &name, USINT *pwd, LoginResult& result);
	static const rUser *LoginHash(USINT *name_hash, USINT *pwd_hash, LoginResult& result);
	static const rUser *Login    (UDINT login, UDINT pwd, LoginResult& result);
	//
	static UDINT        NewPwd   (const string &name, const USINT *newpwd);
	static UDINT        SetAccess(const string &name, UDINT access, UDINT webaccess);
	static UDINT        SetStatus(const string &name, USINT status);
	static void         deleteAll();

protected:
	USINT      Hash[MAX_HASH_SIZE]; // Hash имени пользователя
	USINT      Pwd [MAX_HASH_SIZE]; // Hash пароля
	string     Name;                // Имя пользователя
	UDINT      IntAccess;           // Уровень доступа Дисплей
	UDINT      ExtAccess;           // Уровень доступа для внешних подключений
	USINT      Status;
	rInterface Interface;

	static const rUser *FindHash(USINT *name_hash);

	rUser(const string &name, USINT *pwd_hash  , UDINT intaccess, UDINT extaccess, UDINT login, USINT *loginpwd_hash);

	static std::vector<rUser*> ListUser;
};



