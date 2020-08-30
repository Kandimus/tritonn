//=================================================================================================
//===
//=== safity.cpp
//===
//=== Copyright (c) 2019 by VeduN.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Шаблоны для создания потокозащищенных переменных и классов
//===
//=================================================================================================

#include "thread_class.h"
/*
template <typename T>
rSafityValue<T>::rSafityValue(T val) : Value(val)
{
	mutex = PTHREAD_MUTEX_INITIALIZER;
}


template <typename T>
rSafityValue<T>::rSafityValue() : Value()
{
	mutex = PTHREAD_MUTEX_INITIALIZER;
}

template <typename T>
rSafityValue<T>::~rSafityValue()
{
	;
}

template <typename T>
T rSafityValue<T>::Get(void)
{
	T result;

	int retVal = pthread_mutex_lock(&mutex);
	if(!retVal)
	{
		result = Value;
		pthread_mutex_unlock(&mutex);
	}
	else
	{
		;
	}

	return result;
}


template <typename T>
int rSafityValue<T>::Set(T val)
{
	int retVal = pthread_mutex_lock(&mutex);
	if(!retVal)
	{
		Value = val;
		pthread_mutex_unlock(&mutex);
	}
	return retVal;
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////


template <class T>
rSafityClass<T>::rSafityClass() : Value()
{
	mutex = PTHREAD_MUTEX_INITIALIZER;
}

template <class T>
rSafityClass<T>::~rSafityClass()
{
	;
}

template <class T>
T rSafityClass<T>::Get(void)
{
	T result;

	int retVal = pthread_mutex_lock(&mutex);
	if(!retVal)
	{
		result = Value;
		pthread_mutex_unlock(&mutex);
	}
	else
	{
		;
	}

	return result;
}


template <class T>
int rSafityClass<T>::Set(T &val)
{
	int retVal = pthread_mutex_lock(&mutex);
	if(!retVal)
	{
		Value = val;
		pthread_mutex_unlock(&mutex);
	}
	return retVal;
}

