//=================================================================================================
//===
//=== event.h
//===
//=== Copyright (c) 2019 by RangeSoft.
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

#include "def.h"
#include "thread_class.h"
#include "event_class.h"
#include "text_class.h"


// Объявление типа массива событий, для использования в функциях
typedef rEvent    rEventArray[MAX_EVENT];

//-------------------------------------------------------------------------------------------------
//
class rEventManager : public rThreadClass
{
public:
	virtual ~rEventManager();

	UDINT LoadText(const string& filename);
	UDINT SetCurLang(const string &lang);

	UDINT Add          (rEvent &event);
	UDINT AddEvent     (DINT eid);
	UDINT AddEventUDINT(DINT eid, UDINT val);

	UDINT GetAlarm() { return Alarm.Get();  }
	UDINT Confirm()  { return Alarm.Set(0); } //TODO Будем ли реализовывать подтверждение каждого события?
	
	UDINT Get(rEventArray &arr);

// Singleton
private:
	rEventManager();
	rEventManager(const rEventManager &);
	rEventManager& operator=(rEventManager &);

public:
	static rEventManager &Instance();


protected:
	virtual UDINT Proccesing();


private:
	rSafityValue<UDINT> Alarm; // 
	UINT                Pos;   // Текущая позиция для записи
	rEventArray         Event; // Кольцевой массив, размер зависит от реальной EEPROM памяти
	rTextClass          Texts; //

	UDINT  LoadEEPROM();
	UDINT  SaveEEPROM(int pos, rEvent &event);
	string GetDescr(rEvent &event);
	UDINT  ParseNumber(const char *str, UDINT &num, UDINT &prec, UDINT &exp);
	string ParseParameter(rEvent &event, const char *str, UDINT &offset);
};





extern rEventManager gEventManager;

