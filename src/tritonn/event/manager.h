//=================================================================================================
//===
//=== event/manager.h
//===
//=== Copyright (c) 2019-2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс-поток для регистрации событий, как системных, так и пользователя.
//===
//=== Все события сохраняются в энергонезависимую память
//===
//=================================================================================================

#pragma once

#include <list>
#include "def.h"
#include "tcp_class.h"
#include "event.h"
#include "../text_class.h"

//-------------------------------------------------------------------------------------------------
//
class rEventManager : public rTCPClass
{
	SINGLETON(rEventManager)

	enum
	{
		MAX_EVENT = 100,
		MAX_ALARM = 3,
	};

protected:
	virtual rThreadStatus Proccesing() override;

	virtual UDINT       ClientRecv(rClientTCP *client, USINT *buff, UDINT size) override;
	virtual rClientTCP* NewClient(SOCKET socket, sockaddr_in *addr) override;

public:
	UDINT startServer();
	UDINT loadText(const std::string& filename);
	UDINT setCurLang(const std::string& lang);
	const rTextClass& getTextClass() const { return m_texts; }

	void add          (const rEvent& event);
	void addEvent     (DINT eid);
	void addEventUDINT(DINT eid, UDINT val);

	UDINT getAlarm() { return m_alarm.Get();  }
	UDINT confirm()  { return m_alarm.Set(0); } //TODO Будем ли реализовывать подтверждение каждого события?

public:

	enum ClientCount
	{
		MAX = 2,
	};
	
private:
	rSafityValue<UDINT> m_alarm; //
	std::list<rEvent>   m_list; // Кольцевой массив, размер зависит от реальной EEPROM памяти
	rTextClass          m_texts; //
	pthread_mutex_t     m_mutexList;

	UDINT  LoadEEPROM();
	UDINT  SaveEEPROM(int pos, rEvent &event);

	std::string getDescr(const rEvent &event);
	UDINT       parseNumber(const char* str, UDINT& num, UDINT& prec, UDINT& exp);
	std::string parseParameter(const rEvent& event, const char* str, UDINT& offset);
	void        save(const rEvent& event);
};

