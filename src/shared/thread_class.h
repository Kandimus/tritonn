//=================================================================================================
//===
//=== thread_class.h
//===
//=== Copyright (c) 2003-2013 by VeduN.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Базовый класс описывающий класс-нити
//===
//=================================================================================================

#pragma once

#include <pthread.h>
#include <vector>
#include "safity.h"
#include "thread_time.h"


class rLocker
{
public:
	rLocker(pthread_mutex_t &m)
	{
		mutex = &m;
		pthread_mutex_lock(mutex);
	}
	~rLocker()
	{
		pthread_mutex_unlock(mutex);
	}

	void Nop() {;}

private:
	pthread_mutex_t *mutex;
};





class rLogManager;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
class rThreadClass
{
public:
	rThreadClass();
	rThreadClass(int isrecursive);
	virtual ~rThreadClass();

	DINT Run(UDINT delay);              // Запуск класса-нити
	int  GetStatus();                   // Получение статуса
	int  Close();                       // Закрытие нити
	int  Restore();                     // Продолжение работы после команды pause
	pthread_t *GetThread();
	const char *GetRTTI();

	 UDINT GetTimeInfo(std::vector<rThreadTimeInfo> &arr);

	rSafityValue<INT>   Pause;          // Установка паузы
	rSafityValue<UDINT> Delay;          // Задержка после каждого цикла

protected:
	virtual UDINT Proccesing();         // Обработчик нити
	int  Lock();
	int  Unlock();
	void Fault();                       // Принудительное закрытие нити, в обход процедуры Proccesing

	void EndProccesing();               //

	string              RTTI    = "rThreadClass"; // Имя класса
	UDINT               LogMask = LM_SYSTEM;      // Маска для логирования. Для удобства
	pthread_mutex_t     Mutex;                    // Защитный мьютекс внутренних данных

	 std::vector<rThreadTimeInfo *> TimeInfo;

private:
	pthread_mutex_t     MutexTime;       //
	rSafityValue<UDINT> Status;          // Статус
	rSafityValue<UDINT> Command;	     // Команда
	pthread_t           Thread;          // Указатель на нить
	UDINT               LastTick;        // Временное хранения метки времени

	//TODO Нужно добавить статистику использования, скорость выполения и т.д. И организовать rThreadManager

	 static void *ThreadFunc(void *);    // Функция "обманка" для запуска нити
};

