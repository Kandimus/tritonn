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
#include "average.h"
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
	rAverage     avr;
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

	avr.generateMarkDown(md);
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
