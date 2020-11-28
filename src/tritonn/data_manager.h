//=================================================================================================
//===
//=== data_manager.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Основной класс-нить для обработки данных
//===
//=================================================================================================

#pragma once

#include <vector>
#include "safity.h"
#include "thread_class.h"
#include "variable_class.h"
#include "data_sysvar.h"


class rSnapshot;
class rInterface;
class rSource;
class rReport;
class rError;


//-------------------------------------------------------------------------------------------------
//
class rDataManager : public rThreadClass, public rVariableClass
{
	SINGLETON(rDataManager)

// Методы
public:
	// Текущий статус и работа со статусами
	void     SetLiveStatus(USINT status/*, UDINT haltreason*/);
	USINT    GetLiveStatus();

	void     GetVersion(rVersion &ver) const;
	void     GetConfigInfo(rConfigInfo &conf) const;
	void     GetState(rState &st);
	void     GetTime(struct tm &sdt);

	// Перезагрузка
	void     DoHalt(UDINT reason);
	UDINT    Restart(USINT restart, const string &filename);

	// Конфигурация
	UDINT    LoadConfig();
	UDINT    SaveKernel();
	const rConfigInfo *GetConfName() const;

	// Работа с языками
	UDINT    SetLang(const string &lang);

	UDINT    StartInterfaces();

protected:
	virtual rThreadStatus Proccesing();

	UDINT DeleteWebFiles();
	UDINT CreateWebLang();

	UDINT CreateHaltEvent(rError& err);

	UDINT getConfFile(std::string& conf);


private:
	rSafityValue<USINT> Live;     // Текущий статус жизни процесса
	rSafityValue<USINT> Halt;     // Флаг, перехода в HALT режим

	rSystemVariable          m_sysVar;     // Системные переменные
	std::vector<rSource*>    ListSource; // Список всех объектов (линии, станции, ввод-вывод и объекты)
	std::vector<rInterface*> ListInterface;
	std::vector<rReport*>    ListReport; // Список отчетов
	std::vector<string>      ListLang;

	int LoadEEPROM();
	int SaveEEPROM();

};



