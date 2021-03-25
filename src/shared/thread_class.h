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
#include "singlenton.h"
#include "log_defines.h"

#define THREAD_IS_WORK(x)       (((x) == rThreadStatus::RUNNING) || ((x) == rThreadStatus::PAUSED))


// Статусы нити
enum class rThreadStatus : UDINT
{
	UNDEF = 0,
	FINISHED,
	RUNNING,
	PAUSED,
	ABORTED,
	CLOSED,
};


// Команды нити
enum class rThreadCommand : UDINT
{
	NONE = 0,
	FINISH,
	RESTORE,
	ABORT,
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

	DINT          Run(UDINT delay);              // Запуск класса-нити
	rThreadStatus GetStatus();                   // Получение статуса
	void          Finish();                      // Закрытие нити со статусом FINISH
	int           Restore();                     // Продолжение работы после команды pause
	pthread_t*    GetThread();
	const char*   GetRTTI();

	 UDINT GetTimeInfo(std::vector<rThreadTimeInfo> &arr);

	rSafityValue<INT>   Pause;          // Установка паузы
	rSafityValue<UDINT> Delay;          // Задержка после каждого цикла

protected:
	virtual rThreadStatus Proccesing();         // Обработчик нити

	void Closed();
	int  Lock();
	int  Unlock();
	void Fault();                       // Принудительное закрытие нити, в обход процедуры Proccesing, с выставлением флага ABORTED

	void EndProccesing();               //

	string              RTTI    = "rThreadClass"; // Имя класса
	UDINT               LogMask = LOG::SYSTEM;    // Маска для логирования. Для удобства
	pthread_mutex_t     Mutex;                    // Защитный мьютекс внутренних данных

	std::vector<rThreadTimeInfo* > TimeInfo;

private:
	pthread_mutex_t MutexTime;                    //
	pthread_t       Thread;                       // Указатель на нить
	UDINT           LastTick;                     // Временное хранения метки времени
	rSafityValue<rThreadStatus>  Status;          // Статус
	rSafityValue<rThreadCommand> Command;	       // Команда

	static void *ThreadFunc(void *);    // Функция "обманка" для запуска нити
};

