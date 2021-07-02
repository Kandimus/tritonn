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
#include "xml_util.h"
#include "../system_variable.h"
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
#include "masswater.h"
#include "volwater.h"
#include "kinematicviscosity.h"
#include "dynamicviscosity.h"
#include "../io/manager.h"
#include "../io/module_ai6.h"
#include "../io/module_crm.h"
#include "../io/module_di8do8.h"
#include "../io/module_fi4.h"
#include "../interface/modbustcpslave_manager.h"
#include "../interface/opcua_manager.h"
#include "../users.h"
#include "stringex.h"


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
	rMassWater   mw;
	rVolWater    vw;
	rKinematicViscosity kv;
	rDynamicViscosity   dv;
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
	mw.generateMarkDown(md);
	vw.generateMarkDown(md);
	kv.generateMarkDown(md);
	dv.generateMarkDown(md);
	sel.generateMarkDown(md);
	msel.generateMarkDown(md);
	str.generateMarkDown(md);
	stn.generateMarkDown(md);

	// Hardware
	UDINT moduleID = 0;
	rModuleAI6 ai6(moduleID);
	rModuleCRM crm(moduleID);
	rModuleDI8DO8 di8do8(moduleID);
	rModuleFI4 fi4(moduleID);

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
	rReport         prpt;
	rReport         brpt;

	prpt.m_type = rReport::Type::PERIODIC;
	brpt.m_type = rReport::Type::BATCH;

	generateTypes(md);
	generateMarkDown(md);
	generateSettings(md);
	prpt.generateMarkDown(md);
	brpt.generateMarkDown(md);

	rUser::generateMarkDown(md);
	rSystemVariable::instance().generateMarkDown(md);

	md.save(DIR_MARKDOWN);

	return TRITONN_RESULT_OK;
}


void rDataManager::generateMarkDown(rGeneratorMD& md)
{
	std::string text = "## XML\n````xml\n";

	text += "<" + std::string(XmlName::TRITONN) + " cfgver=\"configurator's version\" kernel=\"kernel's version\" development=\"text\" name=\"text\" hash=\"config's hash\">\n";
	text += "\t<" + std::string(XmlName::HARDWARE) + ">\n";
	text += "\t\t<!-- list of module -->\n";
	text += "\t</" + std::string(XmlName::HARDWARE) + ">\n";
	text += "\t<" + std::string(XmlName::CONFIG) + ">\n";
	text += "\t\t<" + std::string(XmlName::SETTINGS) + ">\n";
	text += "\t\t\t<!-- list of setting -->\n";
	text += "\t\t</" + std::string(XmlName::SETTINGS) + ">\n";
	text += "\t\t<" + std::string(XmlName::IO) + ">\n";
	text += "\t\t\t<!-- list of global io -->\n";
	text += "\t\t</" + std::string(XmlName::IO) + ">\n";
	text += "\t\t<" + std::string(XmlName::STATIONS) + ">\n";
	text += "\t\t\t<!-- list of stations -->\n";
	text += "\t\t</" + std::string(XmlName::STATIONS) + ">\n";
	text += "\t\t<" + std::string(XmlName::CALC) + ">\n";
	text += "\t\t\t<!-- list of global calculations -->\n";
	text += "\t\t</" + std::string(XmlName::CALC) + ">\n";
	text += "\t\t<" + std::string(XmlName::VARIABLES) + ">\n";
	text += "\t\t\t<!-- list of variables -->\n";
	text += "\t\t</" + std::string(XmlName::VARIABLES) + ">\n";
	text += "\t\t<" + std::string(XmlName::REPORTSYSTEM) + ">\n";
	text += "\t\t\t<!-- list of reports and datasets -->\n";
	text += "\t\t</" + std::string(XmlName::REPORTSYSTEM) + ">\n";
	text += "\t</" + std::string(XmlName::CONFIG) + ">\n";
	text += "\t<" + std::string(XmlName::CUSTOM) + " lang=\"short name of language\">\n";
	text += "\t\t<" + std::string(XmlName::PRECISION) + ">\n";
	text += "\t\t\t<!-- list of unit's precision -->\n";
	text += "\t\t</" + std::string(XmlName::PRECISION) + ">\n";
	text += "\t\t<" + std::string(XmlName::STRINGS) + ">\n";
	text += "\t\t\t<!-- list of user's string (descriptions) -->\n";
	text += "\t\t</" + std::string(XmlName::STRINGS) + ">\n";
	text += "\t</" + std::string(XmlName::CUSTOM) + ">\n";
	text += "\t<" + std::string(XmlName::SECURITY) + ">\n";
	text += "\t\t<!-- encoded user's datablock -->\n";
	text += "\t</" + std::string(XmlName::SECURITY) + ">\n";
	text += "\t<" + std::string(XmlName::COMMS) + ">\n";
	text += "\t\t<!-- list of interfaces and datablocks -->\n";
	text += "\t</" + std::string(XmlName::COMMS) + ">\n";
	text += "</" + std::string(XmlName::TRITONN) + ">\n";
	text += "````\n";

	md.add("config").addRemark(text);

}

void rDataManager::generateSettings(rGeneratorMD& md)
{
	std::string text = "## XML\n````xml\n";

	text += "<" + std::string(XmlName::SETTINGS) + ">\n";
	text += "\t<" + std::string(XmlName::CONTRACTHOUR) + ">hour</" + std::string(XmlName::CONTRACTHOUR) + ">\n";
	text += "\t<" + std::string(XmlName::EVENTSTORAGE) + ">days</" + std::string(XmlName::EVENTSTORAGE) + ">\n";
	text += "\t<" + std::string(XmlName::ETHERNET) + " " + std::string(XmlName::DEVICE) + "=\"device 1 name\">\n";
	text += "\t\t<" + std::string(XmlName::IP) + ">ip4 address xx.xx.xx.xx</" + std::string(XmlName::IP) + ">\n";
	text += "\t\t<" + std::string(XmlName::MASK) + ">ip4 mask xx.xx.xx.xx</" + std::string(XmlName::MASK) + ">\n";
	text += "\t\t<" + std::string(XmlName::GATEWAY) + ">ip4 mask xx.xx.xx.xx</" + std::string(XmlName::GATEWAY) + ">\n";
	text += "\t</" + std::string(XmlName::ETHERNET) + ">\n";
	text += "\t<" + std::string(XmlName::ETHERNET) + " " + std::string(XmlName::DEVICE) + "=\"device 2 name\">\n";
	text += "\t\t<" + std::string(XmlName::IP) + ">ip4 address xx.xx.xx.xx</" + std::string(XmlName::IP) + ">\n";
	text += "\t\t<" + std::string(XmlName::MASK) + ">ip4 mask xx.xx.xx.xx</" + std::string(XmlName::MASK) + ">\n";
	text += "\t\t<" + std::string(XmlName::GATEWAY) + ">ip4 mask xx.xx.xx.xx</" + std::string(XmlName::GATEWAY) + ">\n";
	text += "\t<" + std::string(XmlName::ETHERNET) + ">\n";
	text += "</" + std::string(XmlName::SETTINGS) + ">\n";
	text += "````\n";

	md.add("settings").addRemark(text);
}

void rDataManager::generateTypes(rGeneratorMD& md)
{
	std::string text = "";

	text += "Тип | Описание\n";
	text += ":---: | :---\n";
	text += String_format("%s | Беззнаковый целый тип размером 8 бит |\n"  , String_toupper(getTypeName(TYPE::USINT)).c_str());
	text += String_format("%s | Знаковый целый тип размером 8 бит |\n"     , String_toupper(getTypeName(TYPE::SINT )).c_str());
	text += String_format("%s | Беззнаковый целый тип размером 16 бит |\n" , String_toupper(getTypeName(TYPE::UINT )).c_str());
	text += String_format("%s | Знаковый целый тип размером 16 бит |\n"    , String_toupper(getTypeName(TYPE::INT  )).c_str());
	text += String_format("%s | Беззнаковый целый тип размером 32 бита |\n", String_toupper(getTypeName(TYPE::UDINT)).c_str());
	text += String_format("%s | Знаковый целый тип размером 32 бита |\n"   , String_toupper(getTypeName(TYPE::DINT )).c_str());
	text += String_format("%s | Вещественный тип размером 32 бита |\n"     , String_toupper(getTypeName(TYPE::REAL )).c_str());
	text += String_format("%s | Вещественный тип размером 64 бита |\n"     , String_toupper(getTypeName(TYPE::LREAL)).c_str());
	text += String_format("%s | Индекс строки размером 32 бита |\n"        , String_toupper(getTypeName(TYPE::STRID)).c_str());

	md.add("types").addRemark(text);
}
