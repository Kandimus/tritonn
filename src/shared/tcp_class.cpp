//=================================================================================================
//===
//=== tcp_class.cpp
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

#include <unistd.h>
#include <algorithm>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include "tcp_class.h"
#include "log_manager.h"



rTCPClass::rTCPClass(const string &ip, UINT port, UDINT maxclient) : rThreadClass()
{
	RTTI        = "rTCPClass";
	LogMask    |= LM_TCPSERV;
	Socket      = SOCKET_ERROR;
	MaxClient   = maxclient;
	strIP       = ip;
	Port        = port;
	Select_sec  = MAX_SELECT_SEC;
	Select_usec = MAX_SELECT_USEC;
}


rTCPClass::~rTCPClass()
{
	Destroy();
}


void  rTCPClass::SetServerIP(const string &ip, UINT port)
{
	strIP = ip;
	Port  = port;
}



UDINT rTCPClass::StartServer(const string &ip, UINT port)
{
	Lock();

	if(ip.size()) strIP = ip;
	if(port)      Port  = port;

	ServAddr.sin_family      = AF_INET;
	ServAddr.sin_port        = htons(Port);
	ServAddr.sin_addr.s_addr = inet_addr(strIP.c_str());

	Started = false;
	Socket  = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(Socket == SOCKET_ERROR)
	{
		// В конструкторах нельзя использовать события, так как класс еще может не создаться
		TRACEA(LogMask, "Ошибка при создании сокета на '%s:%i'. Ошибка %i", strIP.c_str(), Port, errno);

		Unlock();
		return 1;
	}

	if(setsockopt(Socket, SOL_SOCKET, SO_SNDBUF, (const char *)&MAX_TCP_SEND_BUFF, sizeof(MAX_TCP_SEND_BUFF)) == SOCKET_ERROR)
	{
		// В конструкторах нельзя использовать события, так как класс еще может не создаться
		TRACEA(LogMask, "Ошибка при установке опций сокета '%s:%i'. Ошибка %i", strIP.c_str(), Port, errno);

		close(Socket);
		Socket = SOCKET_ERROR;

		Unlock();
		return 2;
	}

	if(setsockopt(Socket, SOL_SOCKET, SO_RCVBUF, (const char *)&MAX_TCP_RECV_BUFF, sizeof(MAX_TCP_RECV_BUFF)) == SOCKET_ERROR)
	{
		// В конструкторах нельзя использовать события, так как класс еще может не создаться
		TRACEA(LogMask, "Ошибка при установке опций сокета '%s:%i'. Ошибка %i", strIP.c_str(), Port, errno);

		close(Socket);
		Socket = SOCKET_ERROR;

		Unlock();
		return 3;
	}

	// выключаем алгоритм Нэйгла
	int flag = 1;
	if(setsockopt(Socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag)) == SOCKET_ERROR)
	{
		TRACEA(LogMask, "Ошибка при установке опций сокета '%s:%i'. Ошибка %i", strIP.c_str(), Port, errno);

		close(Socket);
		Socket = SOCKET_ERROR;

		return 4;
	}

	// Устанавливаем асинхронный прием сообщений
	SOCKET_SET_NONBLOCK(Socket);

#ifdef QT_DEBUG
	SOKET_SET_REUSEPORT(Socket);
#endif

	// Захватываем сокет
	if(bind(Socket, (sockaddr *)&ServAddr, sizeof(ServAddr)))
	{
		// В конструкторах нельзя использовать события, так как класс еще может не создаться
		TRACEA(LogMask, "Ошибка при захвате сокета '%s:%i'. Ошибка %i", strIP.c_str(), Port, errno);

		close(Socket);
		Socket = SOCKET_ERROR;

		Unlock();
		return 5;
	}

	// get socket to listen
	if(SOCKET_ERROR == listen(Socket, SOMAXCONN))
	{
		// В конструкторах нельзя использовать события, так как класс еще может не создаться
		TRACEA(LogMask, "Ошибка при установке прослушки сокета '%s:%i'. Ошибка %i", strIP.c_str(), Port, errno);

		close(Socket);
		Socket = SOCKET_ERROR;

		Unlock();
		return 6;
	}

	Started = true;

	TRACEI(LogMask, "Сокет открыт на '%s:%i'.", strIP.c_str(), Port);

	Unlock();
	return 0;
}


//
UDINT rTCPClass::CloseServer()
{
	Lock();

	Destroy();

	Unlock();

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
void rTCPClass::Destroy()
{
	//
	for(unsigned int ii = 0; ii < Client.size(); ++ii)
	{
		delete Client[ii];
	}
	Client.clear();

	shutdown(Socket, SHUT_RDWR);
	close(Socket);
	Socket  = SOCKET_ERROR;
	Started = false;
}


//-------------------------------------------------------------------------------------------------
// Обработка сетевых подключений
// !!! Внимание !!!
// !!! Общее время работы нити будет складываться из времени задержки нити и времени задержки Select_*
rThreadStatus rTCPClass::Proccesing()
{
	SOCKET      SockAccept;
	sockaddr_in SinAccept;
	socklen_t   SinSize;  
	int         Maxfd;
	fd_set      readfds;
	fd_set      exfds;
	timeval     tv;
	
	SinSize = sizeof(SinAccept);
	
	// Обработка команд нити
	rThreadStatus thread_status = rThreadClass::Proccesing();
	if(!THREAD_IS_WORK(thread_status))
	{
		Destroy();
		return thread_status;
	}

	Lock();

	// Если сервер небыл запущен, то выходим
	if(!Started)
	{
		Unlock();
		return rThreadStatus::RUNNING;
	}


	if(Socket == SOCKET_ERROR)
	{
		Destroy();
		Unlock();
		Finish();
		return rThreadStatus::FINISHED;
	}
	
	// очищаем
	FD_ZERO(&readfds);
	FD_ZERO(&exfds);
	FD_SET(Socket, &exfds);
	tv.tv_sec  = Select_sec;
	tv.tv_usec = Select_usec;
	Maxfd      = Socket;

	// Добавляем сокеты для обработки
	for(unsigned int ii = 0; ii < Client.size(); ++ii)
	{
		FD_SET(Client[ii]->Socket, &readfds);
		FD_SET(Client[ii]->Socket, &exfds);

		Maxfd = std::max(Socket, Client[ii]->Socket);
	}

	if(select(Maxfd + 1, &readfds, NULL, &exfds, &tv) != SOCKET_ERROR)
	{
		// Проверка на подсоединение клиента
		if((SockAccept = accept(Socket, (sockaddr *)&SinAccept, &SinSize)) != SOCKET_ERROR)
		{
			USINT *ip  = (USINT *)&SinAccept.sin_addr.s_addr;
			UDINT  cid = Client.size();

			// Проверяем "белый" список
			if(!CheckWhiteIP(SinAccept.sin_addr.s_addr))
			{
				shutdown(SockAccept, SHUT_RDWR);
				close(SockAccept);

				TRACEI(LogMask, "Попытка подсоединения клиента [%i.%i.%i.%i] не входящего в разрешенный список '%s:%i'", ip[0], ip[1], ip[2], ip[3], strIP.c_str(), Port);
			}
			else
			{
				Client.push_back(NewClient(SockAccept, &SinAccept));


				if(Client.size() > MaxClient)
				{
					TRACEI(LogMask, "Достигнут предел количества клиентов на '%s:%i'. Первый клиент отсоединен", strIP.c_str(), Port);

					delete Client[0];
					Client.erase(Client.begin());
				}
				else
				{
					// выключаем алгоритм Нэйгла
					int flag = 1;
					if(setsockopt(Client.back()->Socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag)) == SOCKET_ERROR)
					{
						TRACEA(LogMask, "Ошибка при установке опций сокета клиента #%i [%i.%i.%i.%i]. Ошибка %i", cid, ip[0], ip[1], ip[2], ip[3], errno);

						delete Client[cid];
						Client.erase(Client.begin() + cid);
					}
					else
					{
						TRACEI(LogMask, "Подсоединение клиента #%i [%i.%i.%i.%i] к [%s:%i/%i]", cid, ip[0], ip[1], ip[2], ip[3], strIP.c_str(), Port, ServAddr.sin_port);
					}
				}
			}
		}
	}
	else
	{
		TRACEW(LogMask, "Ошибка обработки клиентов на '%s:%i/%i'.", strIP.c_str(), Port, ServAddr.sin_port);
	}

	if(FD_ISSET(Socket, &exfds))
	{
		TRACEW(LogMask, "Ошибка обработки клиентов на '%s:%i/%i'.", strIP.c_str(), Port, ServAddr.sin_port);
		//break; // ERROR
	}

	// Обработка клиентов
	for(unsigned int ii = 0; ii < Client.size(); ++ii)
	{
		// Отключение клиента
		if(FD_ISSET(Client[ii]->Socket, &exfds))
		{
			if(Client[ii]->Socket != SOCKET_ERROR)
			{
				TRACEI(LogMask, "Клиент #%i отключился от '%s:%i/%i'.", ii, strIP.c_str(), Port, ServAddr.sin_port);

				delete Client[ii];
				Client.erase(Client.begin() + ii);
				--ii;
			}
		}
      // Считывание данных от клиента
		else if(FD_ISSET(Client[ii]->Socket, &readfds))
		{
			if(!ReadFromClient(Client[ii]))
			{
				if(Client[ii]->Socket != SOCKET_ERROR)
				{
					TRACEW(LogMask, "Клиент #%i отсоединен от '%s:%i/%i'.", ii, strIP.c_str(), Port, ServAddr.sin_port);

					delete Client[ii];
					Client.erase(Client.begin() + ii);
					--ii;
				}
			}
		}
	} // for

	Unlock();
	return rThreadStatus::RUNNING;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
UDINT rTCPClass::ReadFromClient(rClientTCP* client)
{
	USINT *buff  = new USINT[MAX_TCPCLIENT_BUFF];
	DINT  rb     = 0;
	UDINT result = 0;
	
	rb = recv(client->Socket, buff, MAX_TCPCLIENT_BUFF, 0);
	
	if(rb > 0)
	{
		result = ClientRecv(client, buff, rb);
	}
	
	delete[] buff;
	
	if(rb == -1 && (errno != EAGAIN || errno != EWOULDBLOCK))
	{
		return 1; // Все хорошо, даже если мы ничего не считали, то сокет висит на приеме данных
	}
	
	return result;
}


UDINT rTCPClass::Send(rClientTCP *client, void *packet, UDINT size)
{
	UDINT  result = 0;

	if(!size) return 0;

	for(unsigned int ii = 0; ii < Client.size(); ++ii)
	{
		if(!client || client == Client[ii])
		{
			if(Client[ii]->Send(packet, size) != size)
			{
				//TODO А не удалить ли клиента?
				result += 1;
			}
		}
	}

	return result;
}


UDINT rTCPClass::IsStarted() const
{
	return Started;
}


UDINT rTCPClass::CheckWhiteIP(UDINT ip)
{
	if(WhiteList.empty()) return 1;

	for(UDINT ii = 0; ii < WhiteList.size(); ++ii)
	{
		if(WhiteList[ii] == ip) return 1;
	}

	return 0;
}


UDINT rTCPClass::AddWhiteIP(UDINT  ip)
{
	for(UDINT ii = 0; ii < WhiteList.size(); ++ii)
	{
		if(WhiteList[ii] == ip) return 2;
	}

	WhiteList.push_back(ip);
	return 1;
}


UDINT rTCPClass::AddWhiteIP(USINT *ip)
{
	return AddWhiteIP(*(UDINT *)ip);
}


UDINT rTCPClass::AddWhiteIP(string strip)
{
	UDINT ip = 0;
	// Парсим строку типа "XXX.XXX.XXX.XXX"
	if(!inet_pton(AF_INET, strip.c_str(), &ip)) return 0;

	return AddWhiteIP(ip);
}


/*
int rClientTCP::Recv()
{
	int  rs       = 0; // размер принятых байт
	int  count    = 0;

	// Если память под buff не выделенна, то это означает принятие начала новой посылки
	if(!Buff)
	{
		Pos  = 0;                                 // Сброс позиции
		Buff = new char[MAX_TCPCLIENT_BUFF];
		rs   = recv(Sock, Buff, MAX_TCPCLIENT_BUFF, 0);  // Получаем 1 int = длине посылке
		Pos += rs;

		if(rs <= 0)
		{
			SetError(ERR_SERVICE, _T("ServiceTCP: Receive undefined packet (rs %i, size %i)"), rs, c->size);
			return false;
		}
	}
	// Если m_buff не 0, то принимаем остаток посылки
	else
	{
		rs = recv(Sock, Buff + Pos, MAX_TCPCLIENT_BUFF - pos, 0); // Принимаем в m_buf[m_pos] данные, длиной m_size

		if(rs < c->size)
		{
			c->pos  += rs;
			c->size -= rs;
		}
		else
		{
			// Приняли данные.
			switch(c->buff[0])
			{
				// Внешнее закрытие
				case 0x01:
					SetError(ERR_SERVICE, _T("ServiceTCP: Receive command: CLOSE"));
					InterlockedIncrement(&gMoxaProp.RemoteClose);
					break;

				default:
					SetError(ERR_SERVICE, _T("ServiceTCP: Receive unknow command %i, packet size %i"), c->buff[0], c->ms);
					break;
			}
			
			delete c->buff;
			c->buff = NULL;
			c->size = 0;
			c->ms   = 0;

			return true;
		}
	}
	
	return true;
}
*/
