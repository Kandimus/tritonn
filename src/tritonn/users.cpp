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

#include "users.h"
#include <vector>
#include <string.h>
#include "hash.h"
#include "xml_util.h"
#include "generator_md.h"

std::vector<rUser*> rUser::ListUser;


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
const rUser *rUser::Login(const string &name, USINT *pwd, LoginResult& result)
{
	USINT  hash[MAX_HASH_SIZE];

	GetStrHash(name, hash);

	return LoginHash(hash, pwd, result);
}


const rUser *rUser::LoginHash(USINT *name_hash, USINT *pwd_hash, LoginResult& result)
{
	const rUser *user = FindHash(name_hash);

	// Проверка что такой пользователь существует
	if(nullptr == user)
	{
		result = LoginResult::FAULT;
		return nullptr;
	}

	// Проверка на блокировку пользователя
	if(user->GetStatus() & UserFlags::MASK_BLOCKED)
	{
		result = LoginResult::BLOCKED;
		return nullptr;
	}

	// Проверка пароля
	if(!CmpHash(user->Pwd, pwd_hash))
	{
		result = LoginResult::FAULT;
		return nullptr;
	}

	// Пароли совпали
	result = (user->GetStatus() & UserFlags::CHANGEPWD) ? LoginResult::CHANGEPWD : LoginResult::SUCCESS;
	return user;
}


//
const rUser *rUser::Login(UDINT login, UDINT pwd, LoginResult& result)
{
	const rUser *user = Find(login);

	if (nullptr == user) {
		result = LoginResult::FAULT;
		return nullptr;
	}

	// Проверка на блокировку пользователя
	if(user->GetStatus() & UserFlags::MASK_BLOCKED)
	{
		result = LoginResult::BLOCKED;
		return nullptr;
	}

	// Проверка пароля
	USINT  hash[MAX_HASH_SIZE];

	GetBufHash(&pwd, sizeof(pwd), hash);
	if(!CmpHash(user->Interface.Pwd, hash))
	{
		result = LoginResult::FAULT;
		return nullptr;
	}

	// Пароли совпали
	result = LoginResult::SUCCESS;
	return user;
}

//-------------------------------------------------------------------------------------------------
//
void rUser::deleteAll()
{
	for (auto item : ListUser) {
		if (item) {
			delete item;
		}
	}

	ListUser.clear();
}


void rUser::generateMarkDown(rGeneratorMD& md)
{
	std::string text = "## XML\n````xml\n";
	std::string key  = "";
	std::string iv   = "";

	for (UDINT ii = 0; ii < 16; ++ii) {
		key += static_cast<char>(AES_KEY[ii]);
		iv  += static_cast<char>(AES_IV[ii]);
	}

	text += "<" + std::string(XmlName::SECURITY) + ">\n";
	text += "\t<!-- binary encode data to text string -->\n";
	text += "</" + std::string(XmlName::SECURITY) + ">\n";
	text += "````\n\n";

	text += "> Данные кодируются и записываются в текством виде (пример: A0B134FE)\n\n";
	text += "> Формат кодирования: SHA-1</br>";
	text += "Режим: CBC</br>";
	text += "Размер ключа: 128 bits</br>";
	text += "Вектор IV: " + iv + "</br>";
	text += "Ключ: " + key + "</br>\n\n";
	text += "## Формат исходных данных\n````xml\n";

	text += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	text += "<" + std::string(XmlName::SECURITY) + ">\n";
	text += "\t<" + std::string(XmlName::CONFIG) + ">\n";
	text += "\t\t<" + std::string(XmlName::PASSWORD) + ">SHA-1 hash password</" + std::string(XmlName::PASSWORD) + ">\n";
	text += "\t</" + std::string(XmlName::CONFIG) + ">\n";
	text += "\t<" + std::string(XmlName::USERS) + ">\n";
	text += "\t\t<" + std::string(XmlName::USER) + " name=\"user name\">\n";
	text += "\t\t\t<" + std::string(XmlName::PASSWORD) + ">SHA-1 hash password</" + std::string(XmlName::PASSWORD) + ">\n";
	text += "\t\t\t<" + std::string(XmlName::COMMS) + ">\n";
	text += "\t\t\t\t<" + std::string(XmlName::LOGIN) + ">user number name</" + std::string(XmlName::LOGIN) + ">\n";
	text += "\t\t\t\t<" + std::string(XmlName::PASSWORD) + ">SHA-1 hash password</" + std::string(XmlName::PASSWORD) + ">\n";
	text += "\t\t\t</" + std::string(XmlName::COMMS) + ">\n";
	text += "\t\t\t<" + std::string(XmlName::RIGHTS) + ">\n";
	text += "\t\t\t\t<" + std::string(XmlName::INTERNAL) + ">internal hex access</" + std::string(XmlName::INTERNAL) + ">\n";
	text += "\t\t\t\t<" + std::string(XmlName::EXTERNAL) + ">external hex access</" + std::string(XmlName::EXTERNAL) + ">\n";
	text += "\t\t\t</" + std::string(XmlName::RIGHTS) + ">\n";
	text += "\t\t</" + std::string(XmlName::USER) + ">\n";
	text += "\t\t<" + std::string(XmlName::USER) + ">\n";
	text += "\t\t\t<!-- ... -->\n";
	text += "\t\t</" + std::string(XmlName::USER) + ">\n";
	text += "\t</" + std::string(XmlName::USERS) + ">\n";
	text += "\t<" + std::string(XmlName::OPCUA) + ">\n";
	text += "\t\t<" + std::string(XmlName::USER) + ">\n";
	text += "\t\t\t<" + std::string(XmlName::LOGIN) + ">user name</" + std::string(XmlName::LOGIN) + ">\n";
	text += "\t\t\t<" + std::string(XmlName::PASSWORD) + ">uncrypted password</" + std::string(XmlName::PASSWORD) + ">\n";
	text += "\t\t</" + std::string(XmlName::USER) + ">\n";
	text += "\t</" + std::string(XmlName::OPCUA) + ">\n";
	text += "</" + std::string(XmlName::SECURITY) + ">\n";
	text += "````\n\n";

	md.add("users").addRemark(text);
}

