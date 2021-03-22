//=================================================================================================
//===
//=== compared_values.cpp
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

#include <math.h>
#include "compared_values.h"
#include "event_manager.h"



template <typename T>
rCompared<T>::rCompared() : Value(), Saved() {}

template <typename T>
rCompared<T>::rCompared(T v) : Value(v), Saved(v) {}

template <typename T>
rCompared<T>::~rCompared() { ; }

template <typename T>
void rCompared<T>::Init(const T &val) { Value = Saved = val; }

template <typename T>
UDINT rCompared<T>::Compare(rEvent &event)
{
	if (isDiff()) {
		event << Value << Saved;
		rEventManager::instance().Add(event);

		Saved = Value;

		return 1;
	}

	return 0;
}

template <typename T>
bool rCompared<T>::isDiff() const
{
	return Value != Saved;
}


rCmpUSINT::rCmpUSINT()        : rCompared()  { ; }
rCmpUSINT::rCmpUSINT(USINT v) : rCompared(v) { ; }
rCmpUSINT::~rCmpUSINT()                      { ; }



rCmpUINT::rCmpUINT()       : rCompared()  { ; }
rCmpUINT::rCmpUINT(UINT v) : rCompared(v) { ; }
rCmpUINT::~rCmpUINT()                     { ; }


rCmpINT::rCmpINT()      : rCompared()  { ; }
rCmpINT::rCmpINT(INT v) : rCompared(v) { ; }
rCmpINT::~rCmpINT()                    { ; }

rCmpUDINT::rCmpUDINT()        : rCompared()  { ; }
rCmpUDINT::rCmpUDINT(UDINT v) : rCompared(v) { ; }
rCmpUDINT::~rCmpUDINT()                      { ; }


//
rCmpDINT::rCmpDINT()       : rCompared()  { ; }
rCmpDINT::rCmpDINT(DINT v) : rCompared(v) { ; }
rCmpDINT::~rCmpDINT()                     { ; }


//
rCmpREAL::rCmpREAL()       : rCompared()  { ; }
rCmpREAL::rCmpREAL(REAL v) : rCompared(v) { ; }
rCmpREAL::~rCmpREAL()                     { ; }

UDINT rCmpREAL::Compare(rEvent &event)
{
	return Compare(0.0f, event);
}

UDINT rCmpREAL::Compare(REAL prec, rEvent &event)
{
	if(fabsf(Value - Saved) < prec)
	{
		event << Value << Saved;
		rEventManager::instance().Add(event);
		
		Saved = Value;
		
		return 1;
	}
	
	return 0;
}



rCmpLREAL::rCmpLREAL()        : rCompared()  { ; }
rCmpLREAL::rCmpLREAL(LREAL v) : rCompared(v) { ; }
rCmpLREAL::~rCmpLREAL()                      { ; }

UDINT rCmpLREAL::Compare(rEvent &event)
{
	return Compare(0.0, event);
}

UDINT rCmpLREAL::Compare(LREAL prec, rEvent &event)
{
	if(fabs(Value - Saved) > prec)
	{
		event << Value << Saved;
		rEventManager::instance().Add(event);
		
		Saved = Value;
		
		return 1;
	}
	
	return 0;
}


