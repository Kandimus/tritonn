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


#include "../data_manager.h"
#include <string.h>
#include "../generator_md.h"
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
#include "../interface/modbustcpslave_manager.h"
#include "../interface/opcua_manager.h"



UDINT rDataManager::saveMarkDown()
{
	rGeneratorMD md;

	// io
	rAI          ai;
	rCounter     fi;
	rDI          di;
	rDO          do_;
	rProve       prove;

	ai.generateMarkDown(md);
	fi.generateMarkDown(md);
	di.generateMarkDown(md);
	do_.generateMarkDown(md);
	prove.generateMarkDown(md);

	// calc
	rSampler     smp;
	rReducedDens rd;
	rDensSol     ds;
	rRVar        rvar;
	rSelector    sel;
	rSelector    msel;
	rStation     stn;
	rStream      str(&stn);

	sel.generateIO();

	msel.m_setup.Value |= rSelector::Setup::MULTI;
	msel.generateIO();

	rvar.generateMarkDown(md);
	rd.generateMarkDown(md);
	smp.generateMarkDown(md);
	ds.generateMarkDown(md);
	sel.generateMarkDown(md);
	msel.generateMarkDown(md);
	str.generateMarkDown(md);
	stn.generateMarkDown(md);

	// Hardware
	rModuleAI6 ai6;
	rModuleCRM crm;
	rModuleDI8DO8 di8do8;
	rModuleFI4 fi4;

	ai6.generateMarkDown(md);
	crm.generateMarkDown(md);
	di8do8.generateMarkDown(md);
	fi4.generateMarkDown(md);

	// Interfaces
	rModbusTCPSlaveManager mtcps;
	rOPCUAManager opcua;

	mtcps.generateMarkDown(md);
	opcua.generateMarkDown(md);

	// Other
	rSystemVariable sysvar;
	rReport         prpt;
	rReport         brpt;

	prpt.m_type = rReport::Type::PERIODIC;
	brpt.m_type = rReport::Type::BATCH;

	prpt.generateMarkDown(md);
	brpt.generateMarkDown(md);
	sysvar.generateMarkDown(md);

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
	rSampler smp;
	auto rep     = new rReport();

	ssel->generateIO();

	msel->m_setup.Value |= rSelector::Setup::MULTI;
	msel->generateIO();

	text += "\n<!-- \n\tStation/stream objects list \n-->\n<objects>\n";
	text += smp.saveKernel(false, "sampler", "Пробоотборник", false);
	text += denssol->saveKernel(false, "densitometer", "Плотномер (Солартрон)", false);
	text += rdcdens->saveKernel(false, "reduceddens", "Приведение плотности", true);
	text += ssel->saveKernel   (false, "selector", "Селектор", true);
	text += msel->saveKernel   (false, "multiselector", "Мультиселектор", true);
	text += stn->saveKernel    (false, "station", "Станция", true);
	text += str->saveKernel    (false, "stream", "Линия", false);
//	rep->Type = REPORT_PERIODIC;
//	text += rep->saveKernel    (false, "report", "Отчет (периодический)", true);
//	rep->Type = REPORT_BATCH;
//	text += rep->saveKernel    (false, "report", "Отчет (по партии)", true);
	text += "</objects>\n";
	text += "\n</kernel>";

	delete rep;
	delete rdcdens;
	delete denssol;
	delete msel;
	delete ssel;
	delete str;
	delete stn;

	return 0;
}

