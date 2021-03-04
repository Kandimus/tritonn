//=================================================================================================
//===
//=== text_id.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Идентификаторы стандартных строк
//===
//=================================================================================================

#pragma once

#include "def.h"


enum SID : UDINT
{
	// Строки до 1000 являются единицами измерения, и представлены в UNITID (units.h)

	PRESENT = 1000,
	PHYSICAL,
	CURRENT,
	TEMPERATURE,
	PRESSURE,
	PERIOD,
	DENSITY,
	DENSITY15,
	DENSITY20,
	B,
	B15,
	Y,
	Y15,
	CTL,
	CPL,
	IMPULSE,
	FREQUENCY,
	FLOWRATE_MASS,
	FLOWRATE_VOLUME,
	FLOWRATE_VOLUME15,
	FLOWRATE_VOLUME20,
	VALUE,
	FAULT,
	CANFILLED0,
	CANFILLED1,
	CANFAULT0,
	CANFAULT1,
	CANMASS0,
	CANMASS1,
	CANIOSTART,
	CANIOSTOP,
	CANSELECT,
	GRAB,
	OPEN,
	CLOSE,
	OPENED,
	CLOSED,


	SEL_OUT = 1500,
	SEL_IN1,
	SEL_IN2,
	SEL_IN3,
	SEL_IN4,
	SEL_FLT1,
	SEL_FLT2,
	SEL_FLT3,
	SEL_FLT4,
	SEL_GRP1_IN1,
	SEL_GRP1_IN2,
	SEL_GRP1_IN3,
	SEL_GRP1_IN4,
	SEL_GRP2_IN1,
	SEL_GRP2_IN2,
	SEL_GRP2_IN3,
	SEL_GRP2_IN4,
	SEL_GRP3_IN1,
	SEL_GRP3_IN2,
	SEL_GRP3_IN3,
	SEL_GRP3_IN4,
	SEL_GRP4_IN1,
	SEL_GRP4_IN2,
	SEL_GRP4_IN3,
	SEL_GRP4_IN4,
	SEL_GRP5_IN1,
	SEL_GRP5_IN2,
	SEL_GRP5_IN3,
	SEL_GRP5_IN4,
	SEL_GRP6_IN1,
	SEL_GRP6_IN2,
	SEL_GRP6_IN3,
	SEL_GRP6_IN4,
	SEL_GRP7_IN1,
	SEL_GRP7_IN2,
	SEL_GRP7_IN3,
	SEL_GRP7_IN4,
	SEL_GRP8_IN1,
	SEL_GRP8_IN2,
	SEL_GRP8_IN3,
	SEL_GRP8_IN4,
	SEL_GRP1_FLT1,
	SEL_GRP1_FLT2,
	SEL_GRP1_FLT3,
	SEL_GRP1_FLT4,
	SEL_GRP2_FLT1,
	SEL_GRP2_FLT2,
	SEL_GRP2_FLT3,
	SEL_GRP2_FLT4,
	SEL_GRP3_FLT1,
	SEL_GRP3_FLT2,
	SEL_GRP3_FLT3,
	SEL_GRP3_FLT4,
	SEL_GRP4_FLT1,
	SEL_GRP4_FLT2,
	SEL_GRP4_FLT3,
	SEL_GRP4_FLT4,
	SEL_GRP5_FLT1,
	SEL_GRP5_FLT2,
	SEL_GRP5_FLT3,
	SEL_GRP5_FLT4,
	SEL_GRP6_FLT1,
	SEL_GRP6_FLT2,
	SEL_GRP6_FLT3,
	SEL_GRP6_FLT4,
	SEL_GRP7_FLT1,
	SEL_GRP7_FLT2,
	SEL_GRP7_FLT3,
	SEL_GRP7_FLT4,
	SEL_GRP8_FLT1,
	SEL_GRP8_FLT2,
	SEL_GRP8_FLT3,
	SEL_GRP8_FLT4,
	SEL_GRP1_OUT,
	SEL_GRP2_OUT,
	SEL_GRP3_OUT,
	SEL_GRP4_OUT,
	SEL_GRP5_OUT,
	SEL_GRP6_OUT,
	SEL_GRP7_OUT,
	SEL_GRP8_OUT,

	UNKNOW = 0xFFFFFFFF
};
