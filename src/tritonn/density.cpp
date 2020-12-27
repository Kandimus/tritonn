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

#include <math.h>
#include "density.h"

namespace rDensity
{

// Определение типа бензина, по плотности
USINT getTypeProduct(LREAL dens, Product product)
{
	switch(product)
	{
		case Product::PETROLEUM   : return 0;
		case Product::GAZOLENE    : return 1;
		case Product::TRANSITION  : return 2;
		case Product::JETFUEL     : return 3;
		case Product::FUELOIL     : return 4;
		case Product::SMARTBENZENE:
			if(dens < 770.9) return 1;
			if(dens < 788.0) return 2;
			if(dens < 838.7) return 3;
			return 4;
		case Product::LUBRICANT   : return 5;
	}

	return 0xFF;
}


LREAL getDens20(LREAL dens15, LREAL b15)
{
	// Расчитаем плотность при 20 *С
	return dens15 * exp(-5.0 * b15 * (1.0 + 4.0 * b15));
}

LREAL getY(LREAL dens15, LREAL temp)
{
	return 0.001 * exp(-1.62080 + 0.00021592 * temp + (870960 + 4209.2 * temp) / (dens15 * dens15));
}


LREAL getY15(LREAL dens15)
{
	return getY(dens15, 15.0);
}


};
