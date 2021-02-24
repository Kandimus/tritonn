//=================================================================================================
//===
//=== density.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Общие данные для расчета плотности
//===
//=================================================================================================

#pragma once


#include "def.h"

namespace rDensity
{

enum class Product : USINT
{
	PETROLEUM = 0,
	GAZOLENE,
	TRANSITION,
	JETFUEL,
	FUELOIL,
	SMARTBENZENE,
	LUBRICANT,
};

const LREAL K0[6] = {613.9723, 346.42280,  2690.7440, 594.5418, 186.9696, 0.0000};
const LREAL K1[6] = {  0.0000,   0.43884,     0.0000,   0.0000,   0.4862, 0.6278};
const LREAL K2[6] = {  0.0000,   0.00000, -0.0033762,   0.0000,   0.0000, 0.0000};

const LREAL CONVERGENCE = 0.0005;

const LREAL Limit[7][2] = {{611.2, 1163.8},  // PRODUCT_PETROLEUM
						   {611.2,  770.9},  // PRODUCT_GAZOLENE
						   {770.9,  788.0},  // PRODUCT_TRANSITION
						   {788.0,  838.7},  // PRODUCT_JETFUEL
						   {838.7, 1163.9},  // PRODUCT_FUELOIL
						   {611.2, 1163.9},  // PRODUCT_SMARTBENZINE
						   {801.3, 1163.9}}; // PRODUCT_LUBRICANT

// Определение типа бензина, по плотности
USINT getTypeProduct(LREAL dens, Product product);
LREAL getDens20(LREAL dens15, LREAL b15);
LREAL getY(LREAL dens15, LREAL temp);
LREAL getY15(LREAL dens15);
LREAL getMinLimit(Product product);
LREAL getMaxLimit(Product product);

};
