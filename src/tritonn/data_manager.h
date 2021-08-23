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
#include "structures.h"
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
	void  setLiveStatus(Live status/*, UDINT haltreason*/);
	Live  getLiveStatus();

	// Перезагрузка
	void  DoHalt(HaltReason hr, UDINT reason);
	UDINT Restart(USINT restart, const string &filename);

	// Конфигурация
	UDINT LoadConfig();
	UDINT saveMarkDown();

	// Работа с языками
	void  setLang(const std::string& lang);

	void  doSaveVars();
	void  forceLoadDumpVars(bool forceload);
	void  forceLoadDumpTotals(bool forceload);

	UDINT startInterfaces();
	void  startReports();

protected:
	virtual rThreadStatus Proccesing();

	UDINT DeleteWebFiles();
	UDINT CreateWebLang();

	UDINT CreateHaltEvent(rError& err);

	UDINT getConfFile(std::string& conf);

	UDINT saveDataVariables();
	UDINT saveDataTotals();
	void  loadDumps();
	UDINT checkDataVariables();
	UDINT checkDataTotals();
	void  loadDataVariables();
	void  loadDataTotals();

	void  generateMarkDown(rGeneratorMD& md);
	void  generateTypes(rGeneratorMD& md);
	void  generateSettings(rGeneratorMD& md);

private:
	rSafityValue<Live>  m_live;     // Текущий статус жизни процесса
	rSafityValue<USINT> Halt;       // Флаг, перехода в HALT режим
	rSafityValue<USINT> m_doSaveVars;

	std::vector<rSource*>    m_listSource; // Список всех объектов (линии, станции, ввод-вывод и объекты)
	std::vector<rInterface*> ListInterface;
	std::vector<rReport*>    m_listReport; // Список отчетов
	std::vector<string>      ListLang;

	std::string m_hashCfg; //TODO Убрать в rConfigInfo

	rTickCount m_timerTotal;
	rDumpFile  m_dumpVars;
	rDumpFile  m_dumpTotals;

	int LoadEEPROM();
	int SaveEEPROM();
};



