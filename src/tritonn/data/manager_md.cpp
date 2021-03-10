//=================================================================================================
//===
//=== data/manager_md.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Сохранение help-файлов
//===
//=================================================================================================


#include "tritonn_version.h" //TODO после удаления saveKernel удалить и это
#include "simplefile.h"  //TODO после удаления saveKernel удалить и это
#include "../data_manager.h"
#include <string.h>
#include "../generator_md.h"
#include "../modbustcpslave_manager.h"
#include "../opcua_manager.h"
#include "../data_station.h"
#include "../data_stream.h"
#include "../data_selector.h"
#include "../data_denssol.h"
#include "../data_reduceddensity.h"
#include "../data_ai.h"
#include "../data_di.h"
#include "../data_do.h"
#include "../data_counter.h"
#include "../data_report.h"
#include "../data_rvar.h"
#include "../data_sampler.h"
#include "prove.h"
#include "../io/manager.h"
#include "../io/module_ai6.h"
#include "../io/module_crm.h"
#include "../io/module_di8do8.h"
#include "../io/module_fi4.h"


UDINT rDataManager::saveMarkDown()
{
	rGeneratorMD md;

	rAI          ai;
	rProve       prove;
	rSampler     smp;
	rReducedDens rd;
	rDensSol     ds;
	rRVar        rvar;

	// io
	ai.generateMarkDown(md);
	prove.generateMarkDown(md);

	// calc
	rvar.generateMarkDown(md);
	rd.generateMarkDown(md);
	smp.generateMarkDown(md);
	ds.generateMarkDown(md);

	rModuleAI6 ai6;
	rModuleCRM crm;
	rModuleDI8DO8 di8do8;
	rModuleFI4 fi4;

	ai6.generateMarkDown(md);
	crm.generateMarkDown(md);
	di8do8.generateMarkDown(md);
	fi4.generateMarkDown(md);

	md.save(DIR_MARKDOWN);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//TODO Переделать на SimpleFile
UDINT rDataManager::SaveKernel()
{
	std::string text = "";
	auto stn     = new rStation();
	auto str     = new rStream(stn);
	auto ssel    = new rSelector();
	auto msel    = new rSelector();
	auto denssol = new rDensSol();
	auto rdcdens = new rReducedDens();
	rDI di;
	rDO do_;
	rCounter fi;
	rSampler smp;
	auto rep     = new rReport();
	auto rvar    = new rRVar();
	auto mbSlTCP = new rModbusTCPSlaveManager();
//	auto opcua   = new rOPCUAManager();

	ssel->generateIO();

	msel->m_setup.Value |= rSelector::Setup::MULTI;
	msel->generateIO();

	text += String_format("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
						  "<kernel ver=\"%i.%i\" xmlns=\"http://tritonn.ru\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://tritonn.ru ./kernel.xsd\">\n",
						  TRITONN_VERSION_MAJOR, TRITONN_VERSION_MINOR);

	text += m_sysVar.saveKernel();

	text += rvar->saveKernel(false, "var", "Переменная", true);

	text += "\n<!-- \n\tIO objects list \n-->\n<io_list>\n";
	text += fi.saveKernel (true, "counter", "Частотно-импульсный сигнал", true);
	text += di.saveKernel (true, "di", "Дискретный входной сигнал", true);
	text += do_.saveKernel(true, "do", "Дискретный выходной сигнал", true);
	text += "</io_list>\n";

	text += "\n<!-- \n\tStation/stream objects list \n-->\n<objects>\n";
	text += smp.saveKernel(false, "sampler", "Пробоотборник", false);
	text += denssol->saveKernel(false, "densitometer", "Плотномер (Солартрон)", false);
	text += rdcdens->saveKernel(false, "reduceddens", "Приведение плотности", true);
	text += ssel->saveKernel   (false, "selector", "Селектор", true);
	text += msel->saveKernel   (false, "multiselector", "Мультиселектор", true);
	text += stn->saveKernel    (false, "station", "Станция", true);
	text += str->saveKernel    (false, "stream", "Линия", false);
	rep->Type = REPORT_PERIODIC;
	text += rep->saveKernel    (false, "report", "Отчет (периодический)", true);
	rep->Type = REPORT_BATCH;
	text += rep->saveKernel    (false, "report", "Отчет (по партии)", true);
	text += "</objects>\n";

	text += "\n<!-- \nExternal interfaces \n-->\n<interfaces>\n";
	text += mbSlTCP->saveKernel("ModbusSlaveTCP", "Модбас слейв TCP");
//	opcua->SaveKernel(file, "OPCUA", "OPC UA server");
	text += "</interfaces>\n";

	text += "\n</kernel>";

	UDINT result = SimpleFileSave(DIR_FTP + "kernel.xml", text);

//	delete opcua;
	delete mbSlTCP;
	delete rvar;
	delete rep;
	delete rdcdens;
	delete denssol;
	delete msel;
	delete ssel;
	delete str;
	delete stn;

	return result;
}

