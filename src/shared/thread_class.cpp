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

rThreadClass::rThreadClass() : Status(rThreadStatus::UNDEF), Command(rThreadCommand::NONE)
{
	LogMask = 0;

	pthread_mutex_init(&Mutex    , nullptr);
	pthread_mutex_init(&MutexTime, nullptr);
}


rThreadClass::rThreadClass(int isrecursive) : Status(rThreadStatus::UNDEF), Command(rThreadCommand::NONE)
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
	rThreadStatus status = Status.Get();

	if (status == rThreadStatus::PAUSED || status == rThreadStatus::RUNNING) {
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


rThreadStatus rThreadClass::GetStatus()
{
	return Status.Get();
}


void rThreadClass::Finish()
{
	Command.Set(rThreadCommand::FINISH);
}


void rThreadClass::Closed()
{
	if (Status.Get() == rThreadStatus::FINISHED) {
		Status.Set(rThreadStatus::CLOSED);
	}
}


//
void rThreadClass::Fault()
{
	Status.Set(rThreadStatus::ABORTED);
	pthread_exit(NULL);
}


int rThreadClass::Restore()
{
	return Command.Set(rThreadCommand::RESTORE);
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


rThreadStatus rThreadClass::Proccesing()
{
	rThreadCommand command = rThreadCommand::NONE;
	int   pause   = 0;
	UDINT delay   = 0;

	while(1)
	{
		// Считываем параметры
		command = Command.Get();
		pause   = Pause.Get();
		delay   = Delay.Get();

		// Сбрасываем команду и паузу
		Command.Set(rThreadCommand::NONE);
		Pause.Set(0);

		// Разбираем команды
		switch(command)
		{
			case rThreadCommand::FINISH:  Status.Set(rThreadStatus::FINISHED); return rThreadStatus::FINISHED;
			case rThreadCommand::RESTORE: Status.Set(rThreadStatus::RUNNING);  return rThreadStatus::RUNNING;
			default: ;
		}

		// Если была команда паузы
		if (pause) {
			Status.Set(rThreadStatus::PAUSED);

			// Если это бесконечная пауза, то ждем команды RESTORE или FINISH
			if (pause == -1) {
				continue;
			}

			// Ставим поток на паузу
			mSleep(pause);
		}

		Status.Set(rThreadStatus::RUNNING);

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

		return rThreadStatus::RUNNING;
	}

	return rThreadStatus::UNDEF; // Сюда никогда не попадем
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
	rThreadStatus result = ((rThreadClass *)attr)->Proccesing();

	return (void *)result;
}

