//=================================================================================================
//===
//=== variable_class.cpp
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

#include "variable_class.h"
#include "locker.h"
#include "variable_item.h"
#include "variable_list.h"
#include "data_snapshot_item.h"
#include "data_snapshot.h"
#include <cstring>


rVariableClass::rVariableClass(pthread_mutex_t& mutex)
{
	m_mutex = &mutex;
}


//-------------------------------------------------------------------------------------------------
// Конструктор удаляет все дерево переменных, включая дочерние и соседние узлы
rVariableClass::~rVariableClass()
{
}



UDINT rVariableClass::processing()
{
	if (!m_linkClass) {
		return TRITONN_RESULT_OK;
	}

	m_linkClass->writeExt(m_varList);
	m_linkClass->readExt(m_varList);

	return TRITONN_RESULT_OK;
}


const rVariable* rVariableClass::findVar(const std::string& name)
{
	return m_varList.find(name);
}

//-------------------------------------------------------------------------------------------------
// Получение данных от менеджера данных
UDINT rVariableClass::get(rSnapshot& snapshot)
{
	rLocker locker(*m_mutex); UNUSED(locker);

	for (auto item : snapshot) {
		const rVariable* var = item->getVariable();

		if (!item->isToAssign() || !var) {
			continue;
		}

		// Переменная невидимая, а уровень доступа не админ и не СА
		if (var->isHide() && (0 == (snapshot.getAccess() & ACCESS_MASK_VIEWHIDE))) {
			item->setNotFound();
			continue;
		}

		const void *ptr = var->isExternal() ? var->m_extRead : var->m_pointer;
		memcpy(item->m_data, ptr, item->getSizeVar());
		item->m_status = rSnapshotItem::Status::ASSIGNED;
	}

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
// Запись данных в менеджер данных
UDINT rVariableClass::set(rSnapshot& snapshot)
{
	char buffer[8] = {0};

	rLocker locker(*m_mutex); UNUSED(locker);

	for (auto ssitem : snapshot) {
		const rVariable* var = ssitem->getVariable();

		if (!ssitem->isToWrite() || !var) {
			continue;
		}

		// Переменная невидимая, а уровень доступа не админ и не СА
		if (var->isHide() && (0 == (snapshot.getAccess() & ACCESS_MASK_VIEWHIDE))) {
			ssitem->setNotFound();
			continue;
		}

		// Переменная только для чтения
		if (var->isReadonly()) {
			if (var->isSUWrite()) {
				if (0 == (snapshot.getAccess() & ACCESS_SA)) {
					ssitem->setReadonly();
					continue;
				}
			} else {
				ssitem->setReadonly();
				continue;
			}
		}

		if ((var->getAccess() & snapshot.getAccess()) != var->getAccess()) {
			ssitem->setAccessDenied();
			//TODO Выдать сообщение
			continue;
		}

		var->setBuffer(ssitem->m_data);
//		void *ptr = var->isExternal() ? var->m_extWrite : var->m_pointer;
//		std::memcpy(ptr, , ssitem->getSizeVar());
		ssitem->m_status = rSnapshotItem::Status::WRITED;

//		if(var->isExternal()) {
//			var->m_flags |= rVariable::Flags::EXTWRITED;
//		}
	}

	return TRITONN_RESULT_OK;
}


UDINT rVariableClass::getAllVariables(rSnapshot& snapshot)
{
	snapshot.clear();

	for (auto var : m_varList) {
		if (var->isHide()) {
			continue;
		}
		snapshot.add(var);
	}

	return TRITONN_RESULT_OK;
}

UDINT rVariableClass::writeExt(rVariableList& varlist)
{
	rLocker locker(m_mutex); UNUSED(locker);

	for (auto var : varlist) {
		if (!var) {
			continue;
		}

		if (!var->isExternal() || !var->m_extVar) {
			continue;
		}

		std::memcpy(static_cast<void *>(var->m_extRead), var->m_extVar->m_pointer, EPT_SIZE[var->getType()]);
	}
	return TRITONN_RESULT_OK;
}

UDINT rVariableClass::readExt(rVariableList& varlist)
{
	rLocker locker(m_mutex); UNUSED(locker);

	for (auto var : varlist) {
		if (!var) {
			continue;
		}

		if (!var->isExternal() || !var->m_extVar) {
			continue;
		}

		if (!(var->m_extVar->m_flags & rVariable::Flags::EXTWRITED)) {
			continue;
		}

		memcpy(var->m_extVar->m_pointer, var->m_extWrite, EPT_SIZE[var->getType()]);
		var->m_extVar->m_flags &= ~rVariable::Flags::EXTWRITED;
	}
	return TRITONN_RESULT_OK;
}

UDINT rVariableClass::addExternal(rVariableList& varlist)
{
	for (auto var : varlist.m_list) {
		if (varlist.find(var->m_name)) {
			continue;
		}
		m_varList.m_list.push_back(new rVariable(var));
	}
	return TRITONN_RESULT_OK;
}

UDINT rVariableClass::lintToExternal(rVariableClass* varclass)
{
	m_linkClass = varclass;

	if (varclass) {
		varclass->addExternal(m_varList);
	}

	return TRITONN_RESULT_OK;
}
