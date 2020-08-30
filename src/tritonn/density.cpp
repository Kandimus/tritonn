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



// Определение типа бензина, по плотности
USINT GetTypeProduct(LREAL dens, TYPE_PRODUCT product)
{
	switch(product)
	{
		case PRODUCT_PETROLEUM   : return 0;
		case PRODUCT_GAZOLENE    : return 1;
		case PRODUCT_TRANSITION  : return 2;
		case PRODUCT_JETFUEL     : return 3;
		case PRODUCT_FUELOIL     : return 4;
		case PRODUCT_SMARTBENZENE:
			if(dens < 770.9) return 1;
			if(dens < 788.0) return 2;
			if(dens < 838.7) return 3;
			return 4;
		case PRODUCT_LUBRICANT   : return 5;
	}

	return 0xFF;
}


LREAL GetDens20(LREAL dens15, LREAL b15)
{
	// Расчитаем плотность при 20 *С
	return dens15 * exp(-5.0 * b15 * (1.0 + 4.0 * b15));
}
