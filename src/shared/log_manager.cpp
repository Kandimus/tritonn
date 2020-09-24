//=================================================================================================
//===
//=== log_manager.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс-поток для выдачи log-сообщений по TCP
//===
//=================================================================================================

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <syslog.h>
#include "log_client.h"
#include "log_manager.h"

std::string rLogManager::m_logAppName = "logapp";

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// КОНСТРУКТОРЫ И ДЕСТРУКТОР
rLogManager::rLogManager()
	: rTCPClass("0.0.0.0", TCP_PORT_LOG, MAX_LOG_CLIENT), fnAddCalback(nullptr)
{
	RTTI     = "rLogManager";
	IncCount = 0;

	pthread_mutex_init(&MutexList, NULL);
	pthread_mutex_init(&MutexCallback, NULL);
	
	Level.Set(LM_ALL); //TODO Нужно изменить на нормальный уровень
	Terminal.Set(false);
	Enable.Set(false);     //TODO В штатном режиме можно после полной инициализации выключать

    openlog(m_logAppName.c_str(), LOG_NDELAY/* | LOG_PERROR*/, LOG_LOCAL0);
}


rLogManager::~rLogManager()
{
	List.clear();
	
	closelog();

	pthread_mutex_destroy(&MutexList);
	pthread_mutex_destroy(&MutexCallback);
}



//-------------------------------------------------------------------------------------------------
//
//! В данной функции нельзя вызывать обычный Lock(), т.к. сообщение может добавлять класс TCPClass
//! от которого и унаследованн данный класс логов, и произойдет блокирование мьютексов
UDINT rLogManager::Add(UDINT mask, const char *filename, UDINT lineno, const char *format, ...)
{
	UDINT terminal = false;
	UDINT level    = 0;
	int   priority = LOG_INFO;
	
	if(!Enable.Get()) return 1;

	// Принудительно выставляем в маску логирования сообщения от менеджера сообщений
	level  = Level.Get();
	
	// Если маска сообщения не совпадает с глобальным уровнем, то пропускаем это сообщение
	if(!(level & mask)) return 0;
	
	// Если установлен флаг LM_LOG, то это логирование от менеджера логирования xD
	// в этом случае писать в порт безсмысленно, т.к. ошибка именно в порту, по этому выдаем на экран принудительно
	// Сообщения PANIC тоже печатаем всегда.
	terminal = Terminal.Get() | (mask & LM_LOG) | (level & LM_P);
	
	// Создаем сообщение
	rPacketLog  packet(mask, lineno, filename);

	// Формируем строку сообщения	сразу в поле packet.Text, без промежуточных буфферов
	va_list(args);
	va_start(args, format);
	vsnprintf(packet.Text, MAX_LOG_TEXT, format, args);
	va_end(args);
	
	//
	if(mask & LM_I) priority = LOG_INFO;
	if(mask & LM_W) priority = LOG_WARNING;
	if(mask & LM_A) priority = LOG_CRIT;
	if(mask & LM_P) priority = LOG_ALERT;
	
	// Отправляем сообщение в системный rsyslog
	syslog(priority, "%s", packet.Text);

	if(terminal)
	{
		PrintToTerminal(&packet);
	}

	// Запускаем callback функцию, если она есть
	LockCallback();
	{
		if(fnAddCalback)
		{
			fnAddCalback(packet.Text);
		}
	}
	UnlockCallback();

	if(rTCPClass::IsStarted())
	{
		LockList();
		packet.MIC = IncCount++;
		List.push_back(packet);
		UnlockList();
	}

	return 0;
}



void rLogManager::OutErr(const char *filename, UDINT lineno, const char *format, ...)
{
	rPacketLog packet(LM_A, lineno, filename);
	
	va_list(args);
	va_start(args, format);
	vsnprintf(packet.Text, MAX_LOG_TEXT, format, args);
	va_end(args);

	syslog(LOG_ALERT, "%s", packet.Text);

	if(rLogManager::Instance().Terminal.Get())
	{
		PrintToTerminal(&packet);
	}
}


rLogManager &rLogManager::Instance()
{
	static rLogManager Singleton;

	return Singleton;
}

//-------------------------------------------------------------------------------------------------
// Вывод сообщения на терминал
void rLogManager::PrintToTerminal(rPacketLog *packet)
{
	char logt[5] = "----";
	tm   dt;

	// Тип сообщения
	if(packet->Mask & LM_P) logt[0] = 'P';
	if(packet->Mask & LM_A) logt[1] = 'A';
	if(packet->Mask & LM_W) logt[2] = 'W';
	if(packet->Mask & LM_I) logt[3] = 'I';

	// Получаем дату и время в структуре
	localtime_r(&packet->Date.tv_sec, &dt);

	fprintf(stderr, "%02i.%02i.%04i %02i:%02i:%02i.%03li [%s %s:%i] %s\n", dt.tm_mday, dt.tm_mon, dt.tm_year, dt.tm_hour, dt.tm_min, dt.tm_sec, packet->Date.tv_usec / 1000, logt, packet->FileName, packet->LineNo, packet->Text);
}


//-------------------------------------------------------------------------------------------------
//
rThreadStatus rLogManager::Proccesing()
{
	UDINT packetsize    = LENGTH_PACKET_LOG;
	rThreadStatus thread_status = rThreadStatus::UNDEF;
	UDINT clientCount   = 0;

	list<rPacketLog> sendlist;

	while(1)
	{
		// Обработка команд нити
		thread_status = rTCPClass::Proccesing();
		if(!THREAD_IS_WORK(thread_status))
		{
			return thread_status;
		}

		Lock();
		{
			clientCount = Client.size();
		}
		Unlock();

      // Если нет подключенных клиентов, то созданные сообщения удаляем
		if(clientCount)
		{
			LockList();
			{
				List.clear();
			}
			UnlockList();
		}

		//TODO За скан одно сообщение без паузы, или все текущие?
		//TODO Нужно еще проверять на переполнение списка логов, установить предельное значение
		LockList();
		{
			if(List.size())
			{
				sendlist = List;
				List.clear();
			}
		}
		UnlockList();

		// Отсылаем сообщение всем клиентам
		for(list<rPacketLog>::iterator ipaket = sendlist.begin(); ipaket != sendlist.end(); ++ipaket)
		{
			Send(NULL, &(*ipaket), packetsize);
		}
		
		rThreadClass::EndProccesing();
	}

	return rThreadStatus::UNDEF;
}


rClientTCP *rLogManager::NewClient(SOCKET socket, sockaddr_in *addr)
{
	return (rClientTCP *)new rLogClient(socket, addr);
}


UDINT rLogManager::ClientRecv(rClientTCP *client, USINT *buff, UDINT size)
{
	UNUSED(client);
	UNUSED(buff);
	UNUSED(size);
	//TODO доделать обработку принятия по каналу логов сообщения от удаленного клиента. Или же рубить его тут, бо нефиг слать нам мусор
	return 0;
}



UDINT rLogManager::StartServer()
{
	return rTCPClass::StartServer("0.0.0.0", TCP_PORT_LOG);
}


UDINT rLogManager::AddLogMask(UDINT lm)
{
	UDINT oldlvl = Level.Get();

	Level.Set(oldlvl | lm);

	return oldlvl;
}


UDINT rLogManager::RemoveLogMask(UDINT lm)
{
	UDINT oldlvl = Level.Get();

	Level.Set(oldlvl & (~lm));

	return oldlvl;
}


UDINT rLogManager::SetLogMask(UDINT lm)
{
	UDINT oldlvl = Level.Get();

	Level.Set(lm);

	return oldlvl;

}


DINT rLogManager::LockList()
{
	return pthread_mutex_lock(&MutexList);
}


DINT rLogManager::UnlockList()
{
	return pthread_mutex_unlock(&MutexList);
}


DINT rLogManager::LockCallback()
{
	return pthread_mutex_lock(&MutexCallback);
}


DINT rLogManager::UnlockCallback()
{
	return pthread_mutex_unlock(&MutexCallback);
}


UDINT rLogManager::SetAddCalback(Fn_LogAddCallback fn)
{
	LockCallback();
	fnAddCalback = fn;
	UnlockCallback();

	return 0;
}




