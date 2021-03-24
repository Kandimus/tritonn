//=================================================================================================
//===
//=== units.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Работа с единицами измерения
//===
//=================================================================================================

#include <cmath>
#include <string.h>
#include "units.h"

#define UNIT_INIT(i, a, b)             { UnitsTable[i].A = a; UnitsTable[i].B = b; }


rUnits::rUnitsAB rUnits::UnitsTable[U_DIMLESS];


rUnits::Error rUnits::ConvertValue(LREAL srcVal, UDINT srcUnit, LREAL &dstVal, UDINT dstUnit)
{
	LREAL baseVal;

	// Если ед.измерения равны, то ничего не меняем
	if (srcUnit == dstUnit || srcUnit == U_any || dstUnit == U_any) {
		dstVal = srcVal;
		return Error::NONE;
	}

	if(std::isnan(UnitsTable[srcUnit].A) || std::isnan(UnitsTable[srcUnit].B) || std::isnan(UnitsTable[dstUnit].A) || std::isnan(UnitsTable[dstUnit].B))
	{
		dstVal = srcVal;
		return Error::ISNAN;
	}


	// Если одна из единиц измерения не состоит в группе, или ед.измерения из разных групп,
	// то выходим с ошибкой без конвертации
	if((srcUnit >= U_DIMLESS) || (dstUnit >= U_DIMLESS) || ((srcUnit >> 4) != (dstUnit >> 4)))
	{
		dstVal = srcVal;
		return Error::INCOMPATIBILITY;
	}

	// Если нужно, то приводим исходное значение к стандартному типу
	if(srcUnit & 0x0F)
	{
		baseVal = (srcVal - UnitsTable[srcUnit].A) / UnitsTable[srcUnit].B;
	}

	// Если нужно приводим исходное значение от базового типа к требуемому
	if(dstUnit & 0x0F)
	{
		baseVal = UnitsTable[dstUnit].A + srcVal * UnitsTable[dstUnit].B;
	}

	dstVal = baseVal;
	return Error::NONE;
}



void rUnits::Init()
{
	// Очистим таблицу, теперь в ней лежат nan-числа, это и есть маркеры того, что
	// ковертация этого типа не реализована или запрещена
	memset((char *)UnitsTable, 0xFF, sizeof(UnitsTable));


	// Константы ниже описывают формулу Y = A + B * X, для преведения из стандартной величины, в исходную
	// Для обратного (из исходной в стандартную) формула будет X = (Y - A) / B

	UNIT_INIT(U_any, 0, 1.0);

	// Температура
	UNIT_INIT(U_C, 0.0   , 1.0);
	UNIT_INIT(U_K, 273.15, 1.0);
	UNIT_INIT(U_F, 32.0  , 1.8);

	// Давление
	UNIT_INIT(U_MPa    , 0, 1.0);
	UNIT_INIT(U_kPa    , 0, 1000.0);
	UNIT_INIT(U_kgs_sm2, 0, 10.197162);
	UNIT_INIT(U_bar    , 0, 10.0);
	UNIT_INIT(U_mbar   , 0, 10000);
	UNIT_INIT(U_atm    , 0, 9.869232667160128);
	UNIT_INIT(U_mmptst , 0, 7500.637554192);
	UNIT_INIT(U_psi    , 0, 145.03773773022);

	// Плотность
	UNIT_INIT(U_kg_m3, 0, 1.0);
	UNIT_INIT(U_g_sm3, 0, 0.001);

	// Вязкость динамическая
	UNIT_INIT(U_sP, 0, 1.0);
	UNIT_INIT(U_P , 0, 0.01);
	UNIT_INIT(U_kgss_m2, 0, 0.0001019716212978);
	UNIT_INIT(U_Pas, 0, 0.001);

	// Вязкость кинематическая
	UNIT_INIT(U_sSt, 0, 1.0);
	UNIT_INIT(U_St , 0, 0.01);
	UNIT_INIT(U_m2_s, 0, 0.000001);

	// Масса
	UNIT_INIT(U_t , 0, 1.0);
	UNIT_INIT(U_kg, 0, 1000.0);
	UNIT_INIT(U_g , 0, 1000000.0);

	// Объем
	UNIT_INIT(U_m3   , 0, 1.0);
	UNIT_INIT(U_liter, 0, 1000.0);
	UNIT_INIT(U_ml   , 0, 1000000.0)

	// Массовый расход
	UNIT_INIT(U_t_h , 0, 1.0);
	UNIT_INIT(U_kg_h, 0, 1000.0);

	// Объемный расход
	UNIT_INIT(U_m3_h , 0, 1.0);
	UNIT_INIT(U_ltr_h, 0, 1000.0);

	// К-фактор массовый
	UNIT_INIT(U_imp_t , 0, 1.0);
	UNIT_INIT(U_imp_kg, 0, 0.001);

	// К-фактор объемный
	UNIT_INIT(U_imp_m3 , 0, 1.0);
	UNIT_INIT(U_imp_ltr, 0, 0.001);

	// Частота
	UNIT_INIT(U_Hz , 0, 1.0);
	UNIT_INIT(U_kHz, 0, 0.001);
	UNIT_INIT(U_MHz, 0, 0.000001);

	// Время
	UNIT_INIT(U_usec , 0, 1.0);
	UNIT_INIT(U_mksec, 0, 1e-3);
	UNIT_INIT(U_msec , 0, 1e-6);
	UNIT_INIT(U_sec  , 0, 1e-9);

	//
	UNIT_INIT(U_bytes , 0, 1.0);
	UNIT_INIT(U_KBytes, 0, 1024.0);
	UNIT_INIT(U_MBytes, 0, 1024.0 * 1024.0);
	UNIT_INIT(U_MBytes, 0, 1024.0 * 1024.0 * 1024.0);
	UNIT_INIT(U_GBytes, 0, 1024.0 * 1024.0 * 1024.0 * 1024.0);
}
