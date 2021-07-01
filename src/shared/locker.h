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
	enum class TYPELOCK
	{
		READ,
		WRITE,
	};


	rLocker(pthread_mutex_t& m)
	{
		m_rwlock = nullptr;
		m_mutex  = &m;
		pthread_mutex_lock(m_mutex);
	}

	rLocker(pthread_mutex_t* m)
	{
		m_rwlock = nullptr;
		m_mutex = m;

		if (m_mutex) {
			pthread_mutex_lock(m_mutex);
		}
	}

	rLocker(pthread_rwlock_t& l, TYPELOCK type = TYPELOCK::READ)
	{
		m_type   = type;
		m_rwlock = &l;
		m_mutex  = nullptr;

		if (m_type == TYPELOCK::READ) {
			pthread_rwlock_rdlock(m_rwlock);
		} else {
			pthread_rwlock_wrlock(m_rwlock);
		}
	}

	rLocker(pthread_rwlock_t* l, TYPELOCK type = TYPELOCK::READ)
	{
		m_type   = type;
		m_rwlock = l;
		m_mutex  = nullptr;

		if (m_rwlock) {
			if (m_type == TYPELOCK::READ) {
				pthread_rwlock_rdlock(m_rwlock);
			} else {
				pthread_rwlock_wrlock(m_rwlock);
			}
		}
	}

	~rLocker()
	{
		if (m_mutex) {
			pthread_mutex_unlock(m_mutex);
		}

		if (m_rwlock) {
			pthread_rwlock_unlock(m_rwlock);
		}
	}

	void Nop() {;}

private:
	pthread_mutex_t*  m_mutex  = nullptr;
	pthread_rwlock_t* m_rwlock = nullptr;
	TYPELOCK          m_type   = TYPELOCK::READ;
};
