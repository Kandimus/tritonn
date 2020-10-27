//=================================================================================================
//===
//=== data_shapshot.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Глобальная структура текущих данных
//===
//=================================================================================================

#pragma once

#include "def.h"
#include <vector>
#include <string>

class rSnapshotItem;
class rVariable;
class rVariableClass;

class rSnapshot
{
public:
	rSnapshot(rVariableClass* varclass);
	rSnapshot(rVariableClass* varclass, UDINT access);
	virtual ~rSnapshot();

	const rVariable *add(const rSnapshotItem& snapshot);
	const rVariable *add(const std::string& name           );
	const rVariable *add(const std::string& name, SINT  val);
	const rVariable *add(const std::string& name, USINT val);
	const rVariable *add(const std::string& name, INT   val);
	const rVariable *add(const std::string& name, UINT  val);
	const rVariable *add(const std::string& name, DINT  val);
	const rVariable *add(const std::string& name, UDINT val);
	const rVariable *add(const std::string& name, REAL  val);
	const rVariable *add(const std::string& name, LREAL val);
	const rVariable *add(const std::string& name, STRID val);
	const rVariable *add(const std::string& name, const std::string &val);
	const rVariable *add(const std::string& name, void* buf);

	rSnapshotItem* operator[](const UDINT index);
	rSnapshotItem* operator()(const std::string& name);

	UDINT getAccess() const;
	void  setAccess(UDINT access);
	void  resetAssign();
	void  clear();

	rSnapshotItem* last() const;
	std::vector<rSnapshotItem*>::iterator begin() { return m_list.begin(); }
	std::vector<rSnapshotItem*>::iterator end()   { return m_list.end();   }
	size_t size() const { return m_list.size(); }

protected:
	std::vector<rSnapshotItem *> m_list;
	UDINT           m_access;
	rVariableClass* m_varClass;

protected:
	const rVariable* findVar(const std::string& name);
};

