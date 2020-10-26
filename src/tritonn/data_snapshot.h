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
#include "data_variable.h"





class rSnapshotItem
{
	friend class rDataManager;
public:
	rSnapshotItem(const rSnapshotItem &snapshot);
	rSnapshotItem(const string &name);
	rSnapshotItem(const string &name, SINT  val);
	rSnapshotItem(const string &name, USINT val);
	rSnapshotItem(const string &name, INT   val);
	rSnapshotItem(const string &name, UINT  val);
	rSnapshotItem(const string &name, DINT  val);
	rSnapshotItem(const string &name, UDINT val);
	rSnapshotItem(const string &name, REAL  val);
	rSnapshotItem(const string &name, LREAL val);
	rSnapshotItem(const string &name, STRID val);
	rSnapshotItem(const string &name, const string &val);
	rSnapshotItem(const string &name, void *buf);
	virtual ~rSnapshotItem();

	SINT   GetValueSINT();
	USINT  GetValueUSINT();
	INT    GetValueINT();
	UINT   GetValueUINT();
	DINT   GetValueDINT();
	UDINT  GetValueUDINT();
	REAL   GetValueREAL();
	LREAL  GetValueLREAL();
	STRID  GetValueSTRID();
	string GetValueString();

	UDINT            GetBuffer(void *buffer) const;
	const rVariable *GetVariable() const;
	UDINT            GetStatus() const;
	UDINT            GetSizeVar() const;
	void             ResetAssign();

private:
	const rVariable *Variable;
	UDINT            Status;
	char             Data[8];


protected:
	rSnapshotItem();

	UDINT FindVariable(const string &name);
	void  ClearData();
	void  SetValue(void *val);

};


class rSnapshot
{
public:
	rSnapshot();
	rSnapshot(UDINT access);
	virtual ~rSnapshot();

	const rVariable *Add(/*rUser*/const rSnapshotItem &snapshot);
	const rVariable *Add(const string &name           );
	const rVariable *Add(const string &name, SINT  val);
	const rVariable *Add(const string &name, USINT val);
	const rVariable *Add(const string &name, INT   val);
	const rVariable *Add(const string &name, UINT  val);
	const rVariable *Add(const string &name, DINT  val);
	const rVariable *Add(const string &name, UDINT val);
	const rVariable *Add(const string &name, REAL  val);
	const rVariable *Add(const string &name, LREAL val);
	const rVariable *Add(const string &name, STRID val);
	const rVariable *Add(const string &name, const string &val);
	const rVariable *Add(const string &name, void *buf);

	rSnapshotItem *operator[](const UDINT index);
	rSnapshotItem *operator()(const std::string& name);
	rSnapshotItem *Back() const;



	UDINT GetAccess() const;
	void  SetAccess(UDINT access);
	void  ResetAssign();
	void  Clear();

	std::vector<rSnapshotItem*>::iterator begin() { return m_list.begin(); }
	std::vector<rSnapshotItem*>::iterator end()   { return m_list.end();   }
	size_t size() const { return m_list.size(); }

protected:
	 vector<rSnapshotItem *> m_list;
//	UDINT                   Result;
	UDINT                   Access;
};

