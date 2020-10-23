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
	USINT Major;
	USINT Minor;
	USINT Patch;
	USINT Status; // Стату сборки 0 - betta, 1 - nonstable, 2 - stable
	UDINT Build;
	UDINT CRC;    // Контрольная сумма
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
	USINT AI;
	USINT FI;
	USINT Station;
	USINT Stream;
	USINT DensSol;
	USINT ReducedDens;
	USINT Selector;
};





