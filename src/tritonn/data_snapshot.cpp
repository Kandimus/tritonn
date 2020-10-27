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


const rVariable *rSnapshot::add(const rSnapshotItem &snapshot        ) { m_list.push_back(new rSnapshotItem(snapshot )); return m_list.back()->GetVariable(); }
const rVariable *rSnapshot::Add(const string &name                   ) { m_list.push_back(new rSnapshotItem(name     )); return m_list.back()->GetVariable(); }
const rVariable *rSnapshot::Add(const string &name, SINT          val)
{
	const rVariable* var = findVar(name);
	m_list.push_back(new rSnapshotItem(name, val)); return m_list.back()->GetVariable();
}
const rVariable *rSnapshot::Add(const string &name, USINT         val) { m_list.push_back(new rSnapshotItem(name, val)); return m_list.back()->GetVariable(); }
const rVariable *rSnapshot::Add(const string &name, INT           val) { m_list.push_back(new rSnapshotItem(name, val)); return m_list.back()->GetVariable(); }
const rVariable *rSnapshot::Add(const string &name, UINT          val) { m_list.push_back(new rSnapshotItem(name, val)); return m_list.back()->GetVariable(); }
const rVariable *rSnapshot::Add(const string &name, DINT          val) { m_list.push_back(new rSnapshotItem(name, val)); return m_list.back()->GetVariable(); }
const rVariable *rSnapshot::Add(const string &name, UDINT         val) { m_list.push_back(new rSnapshotItem(name, val)); return m_list.back()->GetVariable(); }
const rVariable *rSnapshot::Add(const string &name, REAL          val) { m_list.push_back(new rSnapshotItem(name, val)); return m_list.back()->GetVariable(); }
const rVariable *rSnapshot::Add(const string &name, LREAL         val) { m_list.push_back(new rSnapshotItem(name, val)); return m_list.back()->GetVariable(); }
const rVariable *rSnapshot::Add(const string &name, STRID         val) { m_list.push_back(new rSnapshotItem(name, val)); return m_list.back()->GetVariable(); }
const rVariable *rSnapshot::Add(const string &name, const string &val) { m_list.push_back(new rSnapshotItem(name, val)); return m_list.back()->GetVariable(); }
const rVariable *rSnapshot::Add(const string &name, void         *buf) { m_list.push_back(new rSnapshotItem(name, buf)); return m_list.back()->GetVariable(); }


rSnapshotItem *rSnapshot::operator[](const UDINT index)
{
	if(index >= m_list.size()) return nullptr;

	return m_list[index];
}

rSnapshotItem* rSnapshot::operator()(const std::string& name)
{
	for (auto& item : m_list) {
		if (item->GetVariable()) {
			if (item->GetVariable()->Name == name) {
				return item;
			}
		}
	}

	return nullptr;
}

rSnapshotItem* rSnapshot::Back() const
{
	 return m_list.back();
}

UDINT rSnapshot::GetAccess() const
{
	return Access;
}

void rSnapshot::SetAccess(UDINT access)
{
	Access = access;
}


void rSnapshot::ResetAssign()
{
	 for(auto &list : m_list)
	{
		list->ResetAssign();
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
