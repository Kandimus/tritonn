//=================================================================================================
//===
//=== log_class.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Универсальный класс-поток для выдачи log-сообщений по TCP
//===
//=================================================================================================

#pragma once

#include <list>
#include "tcp_class.h"
#include "packet_log.h"

using std::list;


typedef void ( *Fn_LogAddCallback)(const string &);

class rLogManager: public rTCPClass
{
public:
	virtual ~rLogManager();

	UDINT Add(UDINT mask, const char* filesource, UDINT lineno, const char *format, ...);

	// Управление логированием
	UDINT AddLogMask(UDINT lm);     // Добавление маски к уже существующей
	UDINT RemoveLogMask(UDINT lm);  // Удаление маски из существующей
	UDINT SetLogMask(UDINT lm);     // Установка новой маски
	
	// Функция для выдачи сообщений в поток stderr
	static void OutErr(const char *filename, UDINT lineno, const char *format, ...);

	// Singleton
	static rLogManager &Instance();

//	UDINT Init(const string &syslog_name);
	UDINT StartServer();
	UDINT SetAddCalback(Fn_LogAddCallback fn);

protected:
	virtual rThreadStatus Proccesing(void);
	virtual rClientTCP*   NewClient (SOCKET socket, sockaddr_in *addr);
	virtual UDINT         ClientRecv(rClientTCP *client, USINT *buff, UDINT size);
	
public:
	rSafityValue<UDINT>  Terminal;  // Включение/выключение дублирования сообщений на консоль
	rSafityValue<UDINT>  Enable;    // Включение/выключение логирования

    static std::string   m_logAppName;
//	rSafityValue<UDINT>  MaxLogs;   // Длина буффера сообщений //TODO Нужно ли?

private:
	rSafityValue<UDINT>  Level;     // Текущий уровень логирования
	UDINT                IncCount;  // Инкрементный инкремент сообщения
	list<rPacketLog>     List;      // Список сообщений
	pthread_mutex_t      MutexList; // Защитный мьютекс списка лог-сообщений
	pthread_mutex_t      MutexCallback; //

	Fn_LogAddCallback    fnAddCalback;

private:
	rLogManager();
	rLogManager( const rLogManager &);
	rLogManager& operator=( rLogManager &);

	DINT LockList();
	DINT UnlockList();
	DINT LockCallback();
	DINT UnlockCallback();
	static std::string saveLogText(UDINT mask, const UDT& time, const std::string& source, UDINT lineno, const std::string& text);
};




