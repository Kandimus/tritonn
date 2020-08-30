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

using std::vector;

class rSource;
class rSystemVariable;
class rDataManager;
class rThreadMaster;


const UINT VARF_READONLY = 0x0001; // Запись запрещена
const UINT VARF_SUWRITE  = 0x0002; // Запись разрешена суперпользователю
const UINT VARF_HIDE     = 0x0004; // Переменная не будет отображатся в ОРС-сервере
const UINT VARF_LOADABLE = 0x1000; // Переменная загружается из конфигурации

const UINT VARF_____ = 0;
const UINT VARF_R___ = VARF_READONLY;
const UINT VARF__S__ = VARF_SUWRITE;
const UINT VARF_RS__ = VARF_READONLY | VARF_SUWRITE;
const UINT VARF___H_ = VARF_HIDE;
const UINT VARF_R_H_ = VARF_READONLY | VARF_HIDE;
const UINT VARF__SH_ = VARF_SUWRITE | VARF_HIDE;
const UINT VARF_RSH_ = VARF_READONLY | VARF_SUWRITE | VARF_HIDE;
const UINT VARF____L = VARF_LOADABLE;
const UINT VARF_R__L = VARF_READONLY | VARF_LOADABLE;
const UINT VARF__S_L = VARF_SUWRITE | VARF_LOADABLE;
const UINT VARF_RS_L = VARF_READONLY | VARF_SUWRITE | VARF_LOADABLE;
const UINT VARF___HL = VARF_HIDE | VARF_LOADABLE;
const UINT VARF_R_HL = VARF_READONLY | VARF_HIDE | VARF_LOADABLE;
const UINT VARF__SHL = VARF_SUWRITE | VARF_HIDE | VARF_LOADABLE;
const UINT VARF_RSHL = VARF_READONLY | VARF_SUWRITE | VARF_HIDE | VARF_LOADABLE;



//-------------------------------------------------------------------------------------------------
//
class rVariable
{
	friend class rDataManager;
	friend class rThreadMaster;

public:
	rVariable(string name, TT_TYPE type, UINT flags, void *pointer, STRID unit, UDINT access);
	virtual ~rVariable();

	string  Name;
	TT_TYPE Type;
	UINT    Flags;
	STRID   Unit;
	UDINT   Access;

	static void             Sort();
	static void             DeleteVariables(void);
	static const rVariable *Find(const string &name);
	static UDINT            SaveToCSV(const string &path);

//	string GetSTR  (char *offset) const;
//	LREAL  GetLREAL(char *offset) const;

protected:
	UDINT  Hash;

	static vector<rVariable *> ListVar;

private:
	void   *Pointer; // Прямой указатель на данные. Использовать вне функций rDataManager::Get и rDataManager::Set запрещено!
};



