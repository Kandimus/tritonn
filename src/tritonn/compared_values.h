//=================================================================================================
//===
//=== compared_values.h
//===
//=== Copyright (c) 2003-2013 by VeduN.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Шаблон для сравниваемых заначений. Сознательно класс rEvent вынесен во вне, что бы не
//=== захламлять класс не нужными для сохранения в EEPROM данными.
//===
//=================================================================================================

#pragma once

#include "event/event.h"



template <typename T>
class rCompared
{
public:
	rCompared();
	rCompared(T v);
	virtual ~rCompared();

	virtual void  Init(const T &val);
	virtual UDINT Compare(rEvent &event);
	virtual bool  isDiff() const;

public:
	T Value;

//protected:
	T Saved;
};


class rCmpUSINT : public rCompared<USINT>
{
public:
	rCmpUSINT();
	rCmpUSINT(USINT v);
	virtual ~rCmpUSINT();
};


class rCmpSINT : public rCompared<SINT>
{
public:
	rCmpSINT()       : rCompared()  { ; }
	rCmpSINT(SINT v) : rCompared(v) { ; }
	virtual ~rCmpSINT()             { ; }
};


class rCmpUINT : public rCompared<UINT>
{
public:
	rCmpUINT();
	rCmpUINT(UINT v);
	virtual ~rCmpUINT();
};


class rCmpINT : public rCompared<INT>
{
public:
	rCmpINT();
	rCmpINT(INT v);
	virtual ~rCmpINT();

//	virtual UDINT Compare(rEvent &event) const;
};


class rCmpUDINT : public rCompared<UDINT>
{
public:
	rCmpUDINT();
	rCmpUDINT(UDINT v);
	virtual ~rCmpUDINT();

//	virtual UDINT Compare(rEvent &event) const;
};


class rCmpDINT : public rCompared<DINT>
{
public:
	rCmpDINT();
	rCmpDINT(DINT v);
	virtual ~rCmpDINT();

//	virtual UDINT Compare(rEvent &event) const;
};


//
class rCmpREAL : public rCompared<REAL>
{
public:
	rCmpREAL();
	rCmpREAL(REAL v);
	virtual ~rCmpREAL();

	virtual UDINT Compare(rEvent &event) override;
	virtual UDINT Compare(REAL prec, rEvent &event);
};



class rCmpLREAL : public rCompared<LREAL>
{
public:
	rCmpLREAL();
	rCmpLREAL(LREAL v);
	virtual ~rCmpLREAL();

	virtual UDINT Compare(rEvent &event) override;
	virtual UDINT Compare(LREAL prec, rEvent &event);
};


