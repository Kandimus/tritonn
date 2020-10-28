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
#include "variable_item.h"
#include "variable_list.h"
#include "data_snapshot_item.h"
#include "data_snapshot.h"


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
	char buffer[8] = {0};


	rLocker locker(*m_mutex); UNUSED(locker);

	for (auto item : snapshot) {
		if (!item->isToAssign() || !item->getVariable()) {
			continue;
		}

		// Переменная невидимая, а уровень доступа не админ и не СА
		if (item->getVariable()->isHide() && (0 == (snapshot.getAccess() & ACCESS_MASK_VIEWHIDE))) {
			item->setNotFound();
			continue;
		}

		void *ptr = item->getVariable()->isExternal() ? item->getVariable()->m_extRead : item->getVariable()->m_pointer;
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

	for (auto item : snapshot) {
		if (!item->isToWrite() || !item->getVariable()) {
			continue;
		}

		// Переменная невидимая, а уровень доступа не админ и не СА
		if (item->getVariable()->isHide() && (0 == (snapshot.getAccess() & ACCESS_MASK_VIEWHIDE))) {
			item->setNotFound();
			continue;
		}

		// Переменная только для чтения
		if (item->getVariable()->isReadonly()) {
			if (item->getVariable()->isSUWrite()) {
				if (0 == (snapshot.getAccess() & ACCESS_SA)) {
					item->setReadonly();
					continue;
				}
			} else {
				item->setReadonly();
				continue;
			}
		}

		if ((item->getVariable()->getAccess() & snapshot.getAccess()) != item->getVariable()->getAccess()) {
			item->setAccessDenied();
			//TODO Выдать сообщение
			continue;
		}

		if (item->)
		memcpy(item->getVariable()->m_pointer, item->m_data, item->getSizeVar());
		item->m_status = rSnapshotItem::Status::WRITED;
	}

	return TRITONN_RESULT_OK;
}


UDINT rVariableClass::getAllVariables(rSnapshot& snapshot)
{
	rLocker locker(Mutex); UNUSED(locker);

	snapshot.clear();

	for (auto var : m_listVariables) {
		if (var->isHide()) {
			continue;
		}
		snapshot.add(var);
	}

	return TRITONN_RESULT_OK;
}

bool rVariableClass::addExternal(rVariableList& varlist)
{
	return m_varList.addExternal(varlist);
}
