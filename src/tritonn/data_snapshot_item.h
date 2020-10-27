﻿//=================================================================================================
//===
//=== data_shapshot_item.h
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

class rVariable;
class rSnapshot;

class rSnapshotItem
{
public:
	enum Status : UDINT
	{
		UNDEF = 0,           // Статус не определен
		NOTFOUND,            // Переменая не найдена
		READONLY,            // Переменная только для чтения (результат записи)
		ACCESSDENIED,        // Доступ запрещен (результат записи)
		TOASSIGN,            // Значение переменной не присвоено (результат чтения)
		ASSIGNED,            // Значение переменной присвоено (результат чтения)
		TOWRITE,             // Значение переменной готово для записи
		WRITED,              // Значение записанно (результат записи)
		ERROR,               // Ошибка записи или чтения
	};

	rSnapshotItem(const rSnapshotItem& snapshot);
	rSnapshotItem(const rVariable* var);
	rSnapshotItem(const rVariable* var, SINT  val);
	rSnapshotItem(const rVariable* var, USINT val);
	rSnapshotItem(const rVariable* var, INT   val);
	rSnapshotItem(const rVariable* var, UINT  val);
	rSnapshotItem(const rVariable* var, DINT  val);
	rSnapshotItem(const rVariable* var, UDINT val);
	rSnapshotItem(const rVariable* var, REAL  val);
	rSnapshotItem(const rVariable* var, LREAL val);
	rSnapshotItem(const rVariable* var, STRID val);
	rSnapshotItem(const rVariable* var, const std::string &val);
	rSnapshotItem(const rVariable* var, void *buf);
	virtual ~rSnapshotItem();

	SINT        getValueSINT();
	USINT       getValueUSINT();
	INT         getValueINT();
	UINT        getValueUINT();
	DINT        getValueDINT();
	UDINT       getValueUDINT();
	REAL        getValueREAL();
	LREAL       getValueLREAL();
	STRID       getValueSTRID();
	std::string getValueString();

	bool isAssigned() const { return m_status == Status::ASSIGNED; }
	bool isToAssign() const { return m_status == Status::TOASSIGN; }
	bool isToWrite()  const { return m_status == Status::TOWRITE; }
	void setNotFound() { m_status = Status::NOTFOUND; }
	void setReadonly() { m_status = Status::READONLY; }
	void setAccessDenied() { m_status = Status::ACCESSDENIED; }

	bool             setBuffer(void *buffer);
	bool             getBuffer(void *buffer);
	const rVariable *getVariable() const { return m_var; }
	Status           getStatus()   const { return m_status; }
	UDINT            getSizeVar()  const;
	void             resetAssign();

public:
	Status           m_status;

private:
	const rVariable *m_var;
	char             m_data[8];

protected:
	rSnapshotItem();
	void clearData();
};
