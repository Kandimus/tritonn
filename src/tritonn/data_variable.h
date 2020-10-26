//=================================================================================================
//===
//=== data_variable.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс доступа к объектам и переменным из внешних источников. Массив данных структур
//=== заполняется на старте приложения и остается не изменным.
//===
//=================================================================================================

#pragma once

#include <vector>
#include "def.h"

class rSource;
class rSystemVariable;
class rDataManager;
class rThreadMaster;

enum class rVarFlag : UINT
{
	NONE     = 0x0000,
	READONLY = 0x0001, // Запись запрещена
	SUWRITE  = 0x0002, // Запись разрешена суперпользователю
	HIDE     = 0x0004, // Переменная не будет отображатся в ОРС-сервере
	LOADABLE = 0x1000, // Переменная загружается из конфигурации
	EXTERNAL = 0x2000, //

	_____ = NONE,
	R____ = READONLY,
	_S___ = SUWRITE,
	RS___ = READONLY | SUWRITE,
	__H__ = HIDE,
	R_H__ = READONLY | HIDE,
	_SH__ = SUWRITE | HIDE,
	RSH__ = READONLY | SUWRITE | HIDE,
	___L_ = LOADABLE,
	R__L_ = READONLY | LOADABLE,
	_S_L_ = SUWRITE | LOADABLE,
	RS_L_ = READONLY | SUWRITE | LOADABLE,
	__HL_ = HIDE | LOADABLE,
	R_HL_ = READONLY | HIDE | LOADABLE,
	_SHL_ = SUWRITE | HIDE | LOADABLE,
	RSHL_ = READONLY | SUWRITE | HIDE | LOADABLE,
	____E = EXTERNAL,
	R___E = READONLY | EXTERNAL,
	_S__E = SUWRITE | EXTERNAL,
	RS__E = READONLY | SUWRITE | EXTERNAL,
	__H_E = HIDE | EXTERNAL,
	R_H_E = READONLY | HIDE | EXTERNAL,
	_SH_E = SUWRITE | HIDE | EXTERNAL,
	RSH_E = READONLY | SUWRITE | HIDE | EXTERNAL,
	___LE = LOADABLE | EXTERNAL,
	R__LE = READONLY | LOADABLE | EXTERNAL,
	_S_LE = SUWRITE | LOADABLE | EXTERNAL,
	RS_LE = READONLY | SUWRITE | LOADABLE | EXTERNAL,
	__HLE = HIDE | LOADABLE | EXTERNAL,
	R_HLE = READONLY | HIDE | LOADABLE | EXTERNAL,
	_SHLE = SUWRITE | HIDE | LOADABLE | EXTERNAL,
	RSHLE = READONLY | SUWRITE | HIDE | LOADABLE | EXTERNAL,
};

const rVarFlag VARF______ = 0;
const rVarFlag VARF_R____ = rVarFlag::READONLY;
const rVarFlag VARF__S___ = rVarFlag::SUWRITE;
const rVarFlag VARF_RS___ = rVarFlag::READONLY | rVarFlag::SUWRITE;
const rVarFlag VARF___H__ = rVarFlag::HIDE;
const rVarFlag VARF_R_H__ = rVarFlag::READONLY | rVarFlag::HIDE;
const rVarFlag VARF__SH__ = rVarFlag::SUWRITE | rVarFlag::HIDE;
const rVarFlag VARF_RSH__ = rVarFlag::READONLY | rVarFlag::SUWRITE | rVarFlag::HIDE;
const rVarFlag VARF____L_ = rVarFlag::LOADABLE;
const rVarFlag VARF_R__L_ = rVarFlag::READONLY | rVarFlag::LOADABLE;
const rVarFlag VARF__S_L_ = rVarFlag::SUWRITE | rVarFlag::LOADABLE;
const rVarFlag VARF_RS_L_ = rVarFlag::READONLY | rVarFlag::SUWRITE | rVarFlag::LOADABLE;
const rVarFlag VARF___HL_ = rVarFlag::HIDE | rVarFlag::LOADABLE;
const rVarFlag VARF_R_HL_ = rVarFlag::READONLY | rVarFlag::HIDE | rVarFlag::LOADABLE;
const rVarFlag VARF__SHL_ = rVarFlag::SUWRITE | rVarFlag::HIDE | rVarFlag::LOADABLE;
const rVarFlag VARF_RSHL_ = rVarFlag::READONLY | rVarFlag::SUWRITE | rVarFlag::HIDE | rVarFlag::LOADABLE;
const rVarFlag VARF_____E = rVarFlag::EXTERNAL;
const rVarFlag VARF_R___E = rVarFlag::READONLY | rVarFlag::EXTERNAL;
const rVarFlag VARF__S__E = rVarFlag::SUWRITE | rVarFlag::EXTERNAL;
const rVarFlag VARF_RS__E = rVarFlag::READONLY | rVarFlag::SUWRITE | rVarFlag::EXTERNAL;
const rVarFlag VARF___H_E = rVarFlag::HIDE | rVarFlag::EXTERNAL;
const rVarFlag VARF_R_H_E = rVarFlag::READONLY | rVarFlag::HIDE | rVarFlag::EXTERNAL;
const rVarFlag VARF__SH_E = rVarFlag::SUWRITE | rVarFlag::HIDE | rVarFlag::EXTERNAL;
const rVarFlag VARF_RSH_E = rVarFlag::READONLY | rVarFlag::SUWRITE | rVarFlag::HIDE | rVarFlag::EXTERNAL;
const rVarFlag VARF____LE = rVarFlag::LOADABLE | rVarFlag::EXTERNAL;
const rVarFlag VARF_R__LE = rVarFlag::READONLY | rVarFlag::LOADABLE | rVarFlag::EXTERNAL;
const rVarFlag VARF__S_LE = rVarFlag::SUWRITE | rVarFlag::LOADABLE | rVarFlag::EXTERNAL;
const rVarFlag VARF_RS_LE = rVarFlag::READONLY | rVarFlag::SUWRITE | rVarFlag::LOADABLE | rVarFlag::EXTERNAL;
const rVarFlag VARF___HLE = rVarFlag::HIDE | rVarFlag::LOADABLE | rVarFlag::EXTERNAL;
const rVarFlag VARF_R_HLE = rVarFlag::READONLY | rVarFlag::HIDE | rVarFlag::LOADABLE | rVarFlag::EXTERNAL;
const rVarFlag VARF__SHLE = rVarFlag::SUWRITE | rVarFlag::HIDE | rVarFlag::LOADABLE | rVarFlag::EXTERNAL;
const rVarFlag VARF_RSHLE = rVarFlag::READONLY | rVarFlag::SUWRITE | rVarFlag::HIDE | rVarFlag::LOADABLE | rVarFlag::EXTERNAL;



//-------------------------------------------------------------------------------------------------
//
class rVariable
{
	friend class rDataManager;
	friend class rThreadMaster;

public:
	rVariable(const std::string& name, TT_TYPE type, rVarFlag flags, void* pointer, STRID unit, UDINT access);
	virtual ~rVariable();

	string  Name;
	TT_TYPE Type;
	UINT    Flags;
	STRID   Unit;
	UDINT   Access;

	char    m_extWrite[8];
	char    m_extRead[8];

//	string GetSTR  (char *offset) const;
//	LREAL  GetLREAL(char *offset) const;

protected:
	UDINT  Hash;

private:
	void* Pointer; // Прямой указатель на данные. Использовать вне функций rDataManager::Get и rDataManager::Set запрещено!
};


class rVariableList
{
public:
	rVariableList();
	virtual ~rVariableList();

	void  add(const string& name, TT_TYPE type, rVarFlag flags, void* pointer, STRID unit, UDINT access);
	void  sort();
	void  deleteAll(void);
	UDINT saveToCSV(const string& path);
	const rVariable* find(const string& name);

private:
	vector<rVariable* > m_list;
};


