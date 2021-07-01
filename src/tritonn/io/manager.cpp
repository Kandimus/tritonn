﻿//=================================================================================================
//===
//=== io/manager.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Основной класс-нить для получения данных с модулей ввода-вывода
//===
//=================================================================================================

#include "manager.h"
#include "locker.h"
#include "xml_util.h"
#include "basechannel.h"
#include "../data_config.h"
#include "../data_manager.h"
#include "../system_variable.h"
#include "../units.h"
#include "simpleargs.h"
#include "../def_arguments.h"
#include "../variable_item.h"
#include "../threadmaster.h"
#include "../error.h"
#include "defines.h"

#include "module_cpu.h"
#include "module_ai6.h"
#include "module_di8do8.h"
#include "module_fi4.h"
#include "module_crm.h"


rIOManager::rIOManager() : rVariableClass(Mutex)
{
	RTTI = "rIOManager";
}


rIOManager::~rIOManager()
{
	for(auto item : m_modules) {
		if (item) {
			delete item;
		}
	}
	m_modules.clear();
}


rIOBaseChannel* rIOManager::getChannel(USINT module, USINT channel)
{
	if (module >= m_modules.size()) {
		return nullptr;
	}

	return m_modules[module]->getChannel(channel);
}

rIOBaseModule* rIOManager::getModule(USINT module)
{
	if (module >= m_modules.size()) {
		return nullptr;
	}

	return m_modules[module]->getModulePtr();
}

std::string rIOManager::getModuleAlias(USINT module) const
{
	if (module >= m_modules.size()) {
		return "";
	}

	return m_modules[module]->getAlias();
}

rThreadStatus rIOManager::Proccesing()
{
	rThreadStatus thread_status = rThreadStatus::UNDEF;

	while (true) {
		// Обработка команд нити
		thread_status = rThreadClass::Proccesing();
		if (!THREAD_IS_WORK(thread_status)) {
			return thread_status;
		}

		{
			rLocker lock(Mutex); UNUSED(lock);

			for(auto& item : m_modules) {
				item->processing(rSimpleArgs::instance().isSet(rArg::Simulate));
			}

			rVariableClass::processing();
			rThreadClass::EndProccesing();
		}
	}
}


UDINT rIOManager::generateVars(rVariableClass* parent)
{
	for (auto module : m_modules) {
		module->generateVars(IO::HARWARE_PREFIX, m_varList, rSimpleArgs::instance().isSet(rArg::Simulate));
	}

	if (parent) {
		rVariableClass::linkToExternal(parent);
	}

	return TRITONN_RESULT_OK;
}

rIOBaseModule* rIOManager::addModule(const std::string& type, rError& err, UDINT lineno)
{
	auto sysvar = rSystemVariable::instance().getPointer();
	rIOBaseModule* module = nullptr;

	if (type == rModuleCPU::getRTTI()) {
		if (m_modules.size()) {
			err.set(DATACFGERR_HARDWARE_CPU_FAULT, lineno, "cpu must be the first module");
			return nullptr;
		}

		module = dynamic_cast<rIOBaseModule*>(new rModuleCPU(sysvar->m_max[rModuleCPU::getRTTI()]));
		++sysvar->m_max[rModuleCPU::getRTTI()];

	} else {
		if (!m_modules.size()) {
			err.set(DATACFGERR_HARDWARE_CPU_NF, lineno, "cpu must be the first module");
			return nullptr;
		}

		if (type == rModuleAI6::getRTTI()) {
			module = dynamic_cast<rIOBaseModule*>(new rModuleAI6(sysvar->m_max[rModuleAI6::getRTTI()]));
			++sysvar->m_max[rModuleAI6::getRTTI()];

		} else if (type == rModuleDI8DO8::getRTTI()) {
			module = dynamic_cast<rIOBaseModule*>(new rModuleDI8DO8(sysvar->m_max[rModuleDI8DO8::getRTTI()]));
			++sysvar->m_max[rModuleDI8DO8::getRTTI()];

		} else if (type == rModuleFI4::getRTTI()) {
			module = dynamic_cast<rIOBaseModule*>(new rModuleFI4(sysvar->m_max[rModuleFI4::getRTTI()]));
			++sysvar->m_max[rModuleFI4::getRTTI()];

		} else if (type == rModuleCRM::getRTTI()) {
			module = dynamic_cast<rIOBaseModule*>(new rModuleCRM(sysvar->m_max[rModuleCRM::getRTTI()]));
			++sysvar->m_max[rModuleCRM::getRTTI()];

		} else {
			err.set(DATACFGERR_UNKNOWN_MODULE, lineno, "");
			return nullptr;
		}
	}

	m_modules.push_back(module);

	return module;
}

bool rIOManager::checkListOfModules() const
{
	//TODO
	return true;
}
