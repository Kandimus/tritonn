//=================================================================================================
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

#include "rpmsg_connector.h"
#include "module_cpu.h"
#include "module_ai6a.h"
#include "module_ai6p.h"
#include "module_ao4.h"
#include "module_di16.h"
#include "module_do16.h"
#include "module_di8do8.h"
#include "module_fi4.h"
#include "module_crm.h"


rIOManager::rIOManager() : rVariableClass(Mutex)
{
	RTTI = "rIOManager";

#ifdef TRITONN_YOCTO
	candrv_init();
#endif
}


rIOManager::~rIOManager()
{
#ifdef TRITONN_YOCTO
	candrv_deinit();
#endif

	for(auto item : m_modules) {
		if (item) {
			delete item;
		}
	}
	m_modules.clear();
}


rIOBaseChannel* rIOManager::getChannel(USINT module, rIOBaseChannel::Type type, USINT channel)
{
	if (module >= m_modules.size()) {
		return nullptr;
	}

	return m_modules[module]->getChannel(channel, type);
}

rIOBaseModule* rIOManager::getModule(USINT module, rIOBaseModule::Type type)
{
	if (module >= m_modules.size()) {
		return nullptr;
	}

	if (m_modules[module]->getType() != type && type != rIOBaseModule::Type::UNDEF) {
		return nullptr;
	}

	return m_modules[module]->getModulePtr();
}

rIOBaseInterface* rIOManager::getModuleInterface(USINT module, rIOBaseModule::Type type)
{
	if (module >= m_modules.size()) {
		return nullptr;
	}

	if (m_modules[module]->getType() != type && type != rIOBaseModule::Type::UNDEF) {
		return nullptr;
	}

	return m_modules[module]->getModuleInterface();
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
	bool isSim = rSimpleArgs::instance().isSet(rArg::Simulate);

	while (true) {
		// Обработка команд нити
		thread_status = rThreadClass::Proccesing();
		if (!THREAD_IS_WORK(thread_status)) {
			return thread_status;
		}

		{
			rLocker lock(Mutex); UNUSED(lock);

			for(auto& item : m_modules) {
				UDINT result = item->processing(isSim);
				if (result != TRITONN_RESULT_OK) {
					rDataManager::instance().DoHalt(HaltReason::HARDWARE, result);
					break;
				}
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

rIOBaseModule* rIOManager::addModule(const std::string& type, rError& err, UDINT lineno, std::map<std::string, UDINT>& maxmap)
{
	rIOBaseModule* module = nullptr;

	if (type == rModuleCPU::getRTTI()) {
		if (m_modules.size()) {
			err.set(DATACFGERR_HARDWARE_CPU_FAULT, lineno, "cpu must be the first module");
			return nullptr;
		}

		module = dynamic_cast<rIOBaseModule*>(new rModuleCPU(maxmap[rModuleCPU::getRTTI()]));
		++maxmap[rModuleCPU::getRTTI()];

	} else {
		if (!m_modules.size()) {
			err.set(DATACFGERR_HARDWARE_CPU_NF, lineno, "cpu must be the first module");
			return nullptr;
		}

		if (type == rModuleAI6a::getRTTI()) {
			module = dynamic_cast<rIOBaseModule*>(new rModuleAI6a(maxmap[type]));

		} else if (type == rModuleAI6p::getRTTI()) {
			module = dynamic_cast<rIOBaseModule*>(new rModuleAI6p(maxmap[type]));

		} else if (type == rModuleAO4::getRTTI()) {
			module = dynamic_cast<rIOBaseModule*>(new rModuleAO4(maxmap[type]));

		} else if (type == rModuleDI16::getRTTI()) {
			module = dynamic_cast<rIOBaseModule*>(new rModuleDI16(maxmap[type]));

		} else if (type == rModuleDO16::getRTTI()) {
			module = dynamic_cast<rIOBaseModule*>(new rModuleDO16(maxmap[type]));

		} else if (type == rModuleDI8DO8::getRTTI()) {
			module = dynamic_cast<rIOBaseModule*>(new rModuleDI8DO8(maxmap[type]));

		} else if (type == rModuleFI4::getRTTI()) {
			module = dynamic_cast<rIOBaseModule*>(new rModuleFI4(maxmap[type]));

		} else if (type == rModuleCRM::getRTTI()) {
			module = dynamic_cast<rIOBaseModule*>(new rModuleCRM(maxmap[type]));

		} else {
			err.set(DATACFGERR_HARDWARE_UNKNOWNMODULE, lineno, "");
			return nullptr;
		}

		++maxmap[type];
	}

	m_modules.push_back(module);

	return module;
}

bool rIOManager::checkListOfModules() const
{
	//TODO
	return true;
}
