//=================================================================================================
//===
//=== safity.h
//===
//=== Copyright (c) 2003-2013 by VeduN.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== БШаблоны для создания потокозащищенных переменных и классов
//===
//=================================================================================================

#pragma once

#include <pthread.h>
#include "def.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// Шаблон для описания потоково-защищенной переменной (целые и вещественные классы)
// ! Не рекомендуется использование данного шаблона для сложных классов, в связи с использованием
// ! в методе Set функции копированиЯ
template <typename T>
class rSafityValue
{
public:
	 rSafityValue()      : Value(   ) { mutex = PTHREAD_MUTEX_INITIALIZER; }
	 rSafityValue(T val) : Value(val) { mutex = PTHREAD_MUTEX_INITIALIZER; }
	 ~rSafityValue()                  { ; }

	 T   Get(void)
	 {
		 T result = T();

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

	 int Set(T val)
	 {
		 int retVal = pthread_mutex_lock(&mutex);
		 if(!retVal)
		 {
			 Value = val;
			 pthread_mutex_unlock(&mutex);
		 }
		 return retVal;
	 }

private:
    pthread_mutex_t mutex;
    T Value;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// Шаблон для потоко-защищенных классов
// ! Не допускается использовать для канонических типов, в связи с использованием в методе Set
// ! ссылки на класс
template <class T>
class rSafityClass
{
public:
    rSafityClass();
    ~rSafityClass();

    T   Get(void);
    int Set(T &val);

private:
    pthread_mutex_t mutex;
    T Value;
};


