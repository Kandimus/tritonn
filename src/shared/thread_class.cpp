//=================================================================================================
//===
//=== thread_class.cpp
//===
//=== Copyright (c) 2019 by VeduN.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Базовый класс описывающий класс-нити
//===
//=================================================================================================

#include "tickcount.h"
#include "thread_class.h"




///////////////////////////////////////////////////////////////////////////////////////////////////
//
//

rThreadClass::rThreadClass() : Status(TCS_UNDEF)
{
	LogMask = 0;

	pthread_mutex_init(&Mutex    , nullptr);
	pthread_mutex_init(&MutexTime, nullptr);
}


rThreadClass::rThreadClass(int isrecursive) : Status(TCS_UNDEF)
{
	LogMask = 0;

	if(isrecursive)
	{
		pthread_mutexattr_t mutex_attr;

		pthread_mutexattr_init(&mutex_attr);
		pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&Mutex, &mutex_attr);
	}
	else
	{
		pthread_mutex_init(&Mutex, nullptr);
	}

	pthread_mutex_init(&MutexTime, nullptr);
}


rThreadClass::~rThreadClass()
{
	UDINT status = Status.Get();

	if (!(status == TCS_CLOSED || status == TCS_UNDEF)) {
		pthread_cancel(Thread);
	}

	pthread_mutex_destroy(&Mutex);
	pthread_mutex_destroy(&MutexTime);
}


//-------------------------------------------------------------------------------------------------
// Запуск класса-нити через статичную функцию
DINT rThreadClass::Run(UDINT delay)
{
	Delay.Set(delay);

	return pthread_create(&Thread, NULL, ThreadFunc, this);
}

int rThreadClass::GetStatus()
{
	return Status.Get();
}


int rThreadClass::Close()
{
	return Command.Set(TCC_CLOSE);
}


//
void rThreadClass::Fault()
{
	Status.Set(TCS_CLOSED);
	pthread_exit(NULL);
}

int rThreadClass::Restore()
{
	return Command.Set(TCC_RESTORE);
}



int rThreadClass::Lock()
{
	return pthread_mutex_lock(&Mutex);
}


int rThreadClass::Unlock()
{
	return pthread_mutex_unlock(&Mutex);
}


pthread_t *rThreadClass::GetThread()
{
	return &Thread;
}


const char *rThreadClass::GetRTTI()
{
	return RTTI.c_str();
}


UDINT rThreadClass::Proccesing()
{
	int   command = 0;
	int   pause   = 0;
	UDINT delay   = 0;

	while(1)
	{
		// Считываем параметры
		command = Command.Get();
		pause   = Pause.Get();
		delay   = Delay.Get();

		// Сбрасываем команду и паузу
		Command.Set(0);
		Pause.Set(0);

		// Разбираем команды
		switch(command)
		{
			case TCC_CLOSE:   Status.Set(TCS_CLOSED);  return TCS_CLOSED;
			case TCC_RESTORE: Status.Set(TCS_RUNNING); return TCS_RUNNING;
		}

		// Если была команда паузы
		if(pause)
		{
			Status.Set(TCS_PAUSED);

			// Если это бесконечная пауза, то ждем команды TCC_RESTORE или TCC_CLOSE
			if(pause == -1)
			{
				continue;
			}

			// Ставим поток на паузу
			mSleep(pause);
		}

		Status.Set(TCS_RUNNING);

		// Задержка потока
		if(delay) mSleep(delay);

		// Вычисляем время простоя
		{
			rLocker lock(MutexTime); lock.Nop();
			UDINT   curtime = rTickCount::SysTickUs();

			TimeInfo.push_back(new rThreadTimeInfo());
			TimeInfo.back()->Idle = curtime - LastTick;
			LastTick              = curtime;
		}

		return TCS_RUNNING;
	}

	return TCS_UNDEF; // Сюда никогда не попадем
}



void rThreadClass::EndProccesing()
{
	rLocker lock(MutexTime); lock.Nop();
	UDINT   curtime = rTickCount::SysTickUs();

	if(TimeInfo.size())
	{
		TimeInfo.back()->Work = curtime - LastTick;
	}

	LastTick = curtime;

	//printf("%s: work %i us, idle %i us\n", GetRTTI(), TimeInfo.back().Work, TimeInfo.back().Idle);
}


//-------------------------------------------------------------------------------------------------
//
UDINT rThreadClass::GetTimeInfo(std::vector<rThreadTimeInfo> &arr)
{
	rLocker lock(MutexTime); lock.Nop();

	arr.reserve(TimeInfo.size());

	for(UDINT ii = 0; ii < TimeInfo.size(); ++ii)
	{
		arr.push_back(new rThreadTimeInfo(TimeInfo[ii]));
		delete TimeInfo[ii];
		TimeInfo[ii] = nullptr;
	}
	TimeInfo.clear();

	return TRITONN_RESULT_OK;
}


// Функция обертка для запуска метода класса в отдельной нити
void *rThreadClass::ThreadFunc(void *attr)
{
	UDINT result = ((rThreadClass *)attr)->Proccesing();

	((rThreadClass *)attr)->Status.Set(TCS_CLOSED);

	return (void *)result;
}



