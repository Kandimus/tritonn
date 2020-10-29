//=================================================================================================
//===
//=== data_snapshot.cpp
//===
//=== Copyright (c) 2020 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//===
//===
//=================================================================================================

#include "data_snapshot.h"
#include "data_snapshot_item.h"
#include "variable_item.h"
#include "variable_class.h"


rSnapshot::rSnapshot(rVariableClass* varclass)
{
	m_varClass = varclass;
	m_access   = 0;
}

rSnapshot::rSnapshot(rVariableClass* varclass, UDINT access)
{
	m_varClass = varclass;
	m_access   = access;
}


rSnapshot::~rSnapshot()
{
	for (auto item : m_list) {
		if (item) {
			delete item;
		}
	}

	m_list.clear();
	m_access = 0;
}


const rVariable* rSnapshot::add(const rSnapshotItem& snapshot)
{
	m_list.push_back(new rSnapshotItem(snapshot));
	m_list.back()->m_status = rSnapshotItem::Status::UNDEF;

	return m_list.back()->getVariable();
}

const rVariable *rSnapshot::add(const std::string& name                   ) { const rVariable* var = findVar(name); if (var) m_list.push_back(new rSnapshotItem(var)); return var; }
const rVariable *rSnapshot::add(const std::string& name, SINT          val) { const rVariable* var = findVar(name); if (var) m_list.push_back(new rSnapshotItem(var, val)); return var; }
const rVariable *rSnapshot::add(const std::string& name, USINT         val) { const rVariable* var = findVar(name); if (var) m_list.push_back(new rSnapshotItem(var, val)); return var; }
const rVariable *rSnapshot::add(const std::string& name, INT           val) { const rVariable* var = findVar(name); if (var) m_list.push_back(new rSnapshotItem(var, val)); return var; }
const rVariable *rSnapshot::add(const std::string& name, UINT          val) { const rVariable* var = findVar(name); if (var) m_list.push_back(new rSnapshotItem(var, val)); return var; }
const rVariable *rSnapshot::add(const std::string& name, DINT          val) { const rVariable* var = findVar(name); if (var) m_list.push_back(new rSnapshotItem(var, val)); return var; }
const rVariable *rSnapshot::add(const std::string& name, UDINT         val) { const rVariable* var = findVar(name); if (var) m_list.push_back(new rSnapshotItem(var, val)); return var; }
const rVariable *rSnapshot::add(const std::string& name, REAL          val) { const rVariable* var = findVar(name); if (var) m_list.push_back(new rSnapshotItem(var, val)); return var; }
const rVariable *rSnapshot::add(const std::string& name, LREAL         val) { const rVariable* var = findVar(name); if (var) m_list.push_back(new rSnapshotItem(var, val)); return var; }
const rVariable *rSnapshot::add(const std::string& name, STRID         val) { const rVariable* var = findVar(name); if (var) m_list.push_back(new rSnapshotItem(var, val)); return var; }
const rVariable *rSnapshot::add(const std::string& name, const std::string &val) { const rVariable* var = findVar(name); if (var) m_list.push_back(new rSnapshotItem(var, val)); return var; }
const rVariable *rSnapshot::add(const std::string& name, void         *buf) { const rVariable* var = findVar(name); if (var) m_list.push_back(new rSnapshotItem(var, buf)); return var; }


rSnapshotItem *rSnapshot::operator[](const UDINT index)
{
	if(index >= m_list.size()) return nullptr;

	return m_list[index];
}

rSnapshotItem* rSnapshot::operator()(const std::string& name)
{
	for (auto& item : m_list) {
		if (item->getVariable()) {
			if (item->getVariable()->getName() == name) {
				return item;
			}
		}
	}

	return nullptr;
}

rSnapshotItem* rSnapshot::last() const
{
	 return m_list.back();
}

UDINT rSnapshot::getAccess() const
{
	return m_access;
}

void rSnapshot::setAccess(UDINT access)
{
	m_access = access;
}


void rSnapshot::resetAssign()
{
	 for(auto &list : m_list)
	{
		list->reset();
	}
}


//-------------------------------------------------------------------------------------------------
//
void rSnapshot::clear()
{
	for (auto &item : m_list) {
		if (item) {
			delete item;
		}

		item = nullptr;
	}

	m_list.clear();
}


const rVariable* rSnapshot::findVar(const std::string& name)
{
	return m_varClass ? m_varClass->findVar(name) : nullptr;
}
