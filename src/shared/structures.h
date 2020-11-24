//=================================================================================================
//===
//=== threadclass.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Заголовочный файл, для описания структур
//===
//=================================================================================================

#pragma once

#include <string>
#include "types.h"

struct rVersion
{
	USINT m_major;
	USINT m_minor;
	UINT  m_build;
	UDINT m_hash;
	USINT m_status; // Стату сборки 0 - betta, 1 - nonstable, 2 - stable
	UDINT m_crc;    // Контрольная сумма
};






// Информация об загруженной конфигурации
// Структура на массивах для того что бы ее можно было спокойно положить в rPacket_*
struct rConfigInfo
{
	char File[MAX_CONFIG_NAME];      // Путь до конфигурации (файл)
	char Name[MAX_CONFIG_NAME];      // Имя конфигурации. Срока "Наименование"
	char Version[MAX_CFGVER_SIZE];   // Версия кофигурации //NOTE Может сделать версию просто числом?
	char Developer[MAX_CONFIG_NAME]; // Срока "Разработчик"
	char Hash[MAX_HASH_SIZE];        // Контрольная сумма конфигурации
};


// Состояние работы ядра
struct rState
{
	UDINT EventAlarm;     // Кол-во аварийных не квитированных сообщений
	USINT Live;           // Текущий статус
//	UDINT HaltReason;     // Код ошибки перехода в Halt //TODO Нужно ли это
//	USINT ColdRestart;    // Команда на "холодную" перезагрузку
//	USINT WarmRestart;    // Команда на "горячую" перезагрузку
	USINT StartReason;    // Причина последней перезагрузки
	USINT m_isSimulate;
};

// Учет времени работы потоков и функций
class rProfile
{
public:
	rProfile();
	virtual ~rProfile();

	UDINT Middle; // Среднее время
	UDINT Max;    // Максимальное пиковое
	UDINT Min;    // Минимальное пиковое
	UDINT MidMax; // Среднепиковое, в расчет идут значения выше 50% между midle и max

	UDINT CountMid;    // Счетчик проходов
	UDINT CountMidMax; //

	void Start();
	void Stop();
};

struct rMaxCount
{
	USINT m_ai          = 0;
	USINT m_fi          = 0;
	USINT m_di          = 0;
	USINT m_do          = 0;
	USINT m_station     = 0;
	USINT m_stream      = 0;
	USINT m_densSol     = 0;
	USINT m_reducedDens = 0;
	USINT m_selector    = 0;
};





