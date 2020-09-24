//=================================================================================================
//===
//=== tcpclient_class.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Базовый класс TCP-клиента для класса-потока TCP
//===
//=================================================================================================

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include "../shared/log_manager.h"
#include "../shared/tcpclient_class.h"


rTCPClientClass::rTCPClientClass(rClientTCP &client) : ReconnetTime(1000), Setup(0), Connected(0)
{
	Client    = &client;
	LogMask   = LM_TCPCLNT;
}


rTCPClientClass::~rTCPClientClass()
{
	;
}


//------------------------------------------------------------------------------------------------
//
UDINT rTCPClientClass::Destroy()
{
	if(Client->Socket != SOCKET_ERROR)
	{
		Connected.Set(0);
		//shutdown(Client->Socket, SHUT_RDWR);
		close(Client->Socket);
		Client->Socket = SOCKET_ERROR;

		return 1;
	}

	return 0;
}


UDINT rTCPClientClass::IsConnected()
{
	return Connected.Get();
}

UDINT rTCPClientClass::Disconnect()
{
	rLocker locker(Mutex);

	TRACEW(LogMask, "Disconnect from %s:%i", IP.c_str(), Port);

	return Destroy();
}

//-------------------------
//
UDINT rTCPClientClass::Connect(const string &ip, UINT port)
{
	rLocker locker(Mutex);

	Destroy();

	IP   = ip;
	Port = port;

	TRACEI(LogMask, "Connecting to %s:%i ...", IP.c_str(), Port);

	Client->Socket = socket(AF_INET, SOCK_STREAM, 0);
	if(Client->Socket == SOCKET_ERROR)
	{
		Connected.Set(0);
		TRACEA(LogMask, "Can't create the socket. Error: %i", errno);

		return 1;
	}

	if(setsockopt(Client->Socket, SOL_SOCKET, SO_SNDBUF, (const char *)&MAX_TCP_SEND_BUFF, sizeof(MAX_TCP_SEND_BUFF)) == SOCKET_ERROR)
	{
		TRACEA(LogMask, "Function setsockopt(..., SO_SNDBUF, ...) fault. Error: %i", errno);

		Connected.Set(0);
		close(Client->Socket);
		Client->Socket = SOCKET_ERROR;

		return 2;
	}

	if(setsockopt(Client->Socket, SOL_SOCKET, SO_RCVBUF, (const char *)&MAX_TCP_RECV_BUFF, sizeof(MAX_TCP_RECV_BUFF)) == SOCKET_ERROR)
	{
		TRACEA(LogMask, "Function setsockopt(..., SO_RCVBUF, ...) fault. Error: %i", errno);

		Connected.Set(0);
		close(Client->Socket);
		Client->Socket = SOCKET_ERROR;

		return 3;
	}

	// выключаем алгоритм Нэйгла
	int flag = 1;
	if(setsockopt(Client->Socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag)) == SOCKET_ERROR)
	{
		TRACEA(LogMask, "Function setsockopt(..., TCP_NODELAY, ...) fault. Error: %i", errno);

		Connected.Set(0);
		close(Client->Socket);
		Client->Socket = SOCKET_ERROR;

		return 4;
	}

	Client->Addr.sin_family      = AF_INET;
	Client->Addr.sin_port        = htons(Port);
	Client->Addr.sin_addr.s_addr = inet_addr(IP.c_str());

	if(connect(Client->Socket, (struct sockaddr *)&Client->Addr, sizeof(Client->Addr)) == SOCKET_ERROR)
	{
		Connected.Set(0);
		TRACEA(LogMask, "Can't connect to %s:%i. Error %i", IP.c_str(), Port, errno);

		return 5;
	}

	// Устанавливаем асинхронный прием сообщений
	SOCKET_SET_NONBLOCK(Client->Socket);

	Connected.Set(1);
	TRACEI(LogMask, "Connected to %s:%i", IP.c_str(), Port);



	return 0;
}


//
rThreadStatus rTCPClientClass::Proccesing()
{
	rThreadStatus thread_status = rThreadStatus::UNDEF;
	UDINT   flagerr       = false;
	UDINT   setup         = Setup.Get();
	SOCKET  socket        = SOCKET_ERROR;
	int     Maxfd;
	fd_set  readfds;
	fd_set  exfds;
	timeval tv;

	// Обработка команд нити
	thread_status = rThreadClass::Proccesing();
	if(!THREAD_IS_WORK(thread_status))
	{
		return thread_status;
	}

	if(!Connected.Get())
	{
		return rThreadStatus::RUNNING;
	}

	{
		rLocker locker(Mutex);
		socket = Client->Socket;
	}

	// Если сокет не открыт, то проверяем флаг реконнекта
	if(socket == SOCKET_ERROR)
	{
		if(Connect(IP, Port))
		{
			// Команда на паузу, подождем перед реконнектом
			rThreadClass::Pause.Set(ReconnetTime.Get());
			return rThreadStatus::RUNNING;
		}
	}

	rLocker locker(Mutex);

	// очищаем
	FD_ZERO(&readfds);
	FD_ZERO(&exfds);
	FD_SET(Client->Socket, &exfds);
	FD_SET(Client->Socket, &readfds);
	tv.tv_sec  = MAX_SELECT_SEC;
	tv.tv_usec = MAX_SELECT_USEC;
	Maxfd      = Client->Socket;
	flagerr    = false;

	if(select(Maxfd + 1, &readfds, NULL, &exfds, &tv) == -1)
	{
		TRACEW(LM_TCPCLNT, "Function select fault. Error: %i", errno);
	}

	// Проверка на закрытие сервера
	if(FD_ISSET(Client->Socket, &exfds))
	{
		TRACEW(LM_TCPCLNT, "Server is shutdown.");

		flagerr = true;
	}
	// Считывание данных от сервера
	else if(FD_ISSET(Client->Socket, &readfds))
	{
		if(!ReadFromServer())
		{
			TRACEW(LM_TCPCLNT, "Can't recv data from server. Error: %i", errno);

			flagerr = true;
		}
	}


	// проверка на отключение от сервера
	if(flagerr)
	{
		Destroy();

		if(setup & TCPCLIENT_SETUP_NORECONNECT)
		{
			Connected.Set(0);
		}
		else
		{
			// Команда на паузу, подождем перед реконнектом
			rThreadClass::Pause.Set(ReconnetTime.Get());

			Connected.Set(1);
		}
	}

	Unlock();
	return rThreadStatus::RUNNING;
}


UDINT rTCPClientClass::Send(void *packet, UDINT size)
{
	return Client->Send(packet, size);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
UDINT rTCPClientClass::ReadFromServer()
{
	USINT *buff      = new USINT[MAX_TCPCLIENT_BUFF];
	DINT   readbytes = 0;
	UDINT  result    = 0;
	
	readbytes = recv(Client->Socket, buff, MAX_TCPCLIENT_BUFF, 0);
	
	if(readbytes > 0)
	{
		result = RecvFromServer(buff, readbytes);
	}
	
	delete[] buff;
	
	if(readbytes == -1 && (errno != EAGAIN || errno != EWOULDBLOCK))
	{
		return 1;
	}
	
	return result;
}




