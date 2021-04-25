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
#include "data/dump.h"
#include "data_sysvar.h"
#include "tickcount.h"

namespace tinyxml2 {
	class XMLDocument;
}

class rSnapshot;
class rInterface;
class rSource;
class rReport;
class rError;
class rGeneratorMD;

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
	UDINT    saveMarkDown();
	void     generateMarkDown(rGeneratorMD& md);
	const rConfigInfo *GetConfName() const;

	// Работа с языками
	UDINT    SetLang(const string &lang);

	void     doSaveVars();

	UDINT    StartInterfaces();

protected:
	virtual rThreadStatus Proccesing();

	UDINT DeleteWebFiles();
	UDINT CreateWebLang();

	UDINT CreateHaltEvent(rError& err);

	UDINT getConfFile(std::string& conf);

	void  generateDumpPrefixes();
	UDINT saveDataVariables();
	UDINT saveDataTotals();
	UDINT checkDataVariables();
	UDINT checkDataTotals();
	void  loadDataVariables();
	void  loadDataTotals();
	viod  loadDumps();

private:
	rSafityValue<USINT> m_live;     // Текущий статус жизни процесса
	rSafityValue<USINT> Halt;       // Флаг, перехода в HALT режим
	rSafityValue<USINT> m_doSaveVars;

	rSystemVariable          m_sysVar;     // Системные переменные
	std::vector<rSource*>    m_listSource; // Список всех объектов (линии, станции, ввод-вывод и объекты)
	std::vector<rInterface*> ListInterface;
	std::vector<rReport*>    ListReport; // Список отчетов
	std::vector<string>      ListLang;

	std::string m_hashCfg;

	rTickCount m_timerTotal;
	rDumpFile  m_dumpVars;
	rDumpFile  m_dumpTotals;

	int LoadEEPROM();
	int SaveEEPROM();
};



