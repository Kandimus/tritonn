//=================================================================================================
//===
//=== tcp_class.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс для описания классов-нитей, работающих с TCP
//===
//=================================================================================================

#pragma once

#include <string>
#include <vector>
#include <netinet/in.h>
#include "tcp_client.h"
#include "../shared/thread_class.h"

using std::string;
using std::vector;


//-------------------------------------------------------------------------------------------------
//
class rTCPClass : public rThreadClass
{
public:
	rTCPClass(const std::string& ip, UINT port, UDINT maxclient);
	virtual ~rTCPClass();

	UDINT startServer(const std::string& ip, UINT port);
	void  setServerIP(const std::string& ip, UINT port);
	void  closeServer();

	virtual UDINT IsStarted() const;

	UDINT AddWhiteIP(UDINT  ip);
	UDINT AddWhiteIP(USINT *ip);
	UDINT AddWhiteIP(string ip);

protected:
	virtual rThreadStatus Proccesing();
	
	virtual UDINT CheckWhiteIP(UDINT ip);

	// Call-back на получение данных от клиента
	virtual UDINT ClientRecv(rClientTCP *client, USINT *buff, UDINT size) = 0;
	
	// Отправка данных клиентам
	virtual UDINT Send(rClientTCP *client, const void *packet, UDINT size);

	// Присоединение нового клиента
	virtual rClientTCP* NewClient(SOCKET socket, sockaddr_in *addr) = 0;

protected:
	vector<rClientTCP *>  Client;      // Массив клиентов
	vector<UDINT>         WhiteList;   // Список "белых" адресов, доступных для подключения
	vector<UDINT>         m_listHost;  // Список IP для открытия порта
	UDINT                 Select_sec;  // Время ожидания в функции SELECT
	UDINT                 Select_usec; // Время ожидания в функции SELECT
	std::string           m_hostIP; //
	UINT                  m_hostPort;
	UDINT                 MaxClient;  // Максимальное количество подключенных клиентов, если клиентов будет больше, то первый клиент будет отсоединен от сервера
	
private:
	UDINT                 Started;    // Флаг запуска сервера
	sockaddr_in           ServAddr;   // Адрес сервера
	SOCKET                Socket;     // Сокет на прослушку
	
	void  Destroy();
	UDINT ReadFromClient(rClientTCP *client); // Чтение данных от клиента
};



