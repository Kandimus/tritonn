//=================================================================================================
//===
//=== locker.h
//===
//=== Copyright (c) 2020 by VeduN.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс для автоматического unlock'а мьютекса
//===
//=================================================================================================

#pragma once

#include <pthread.h>

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
