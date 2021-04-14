//=================================================================================================
//===
//=== event_eid.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Файл описания системных сообщений (rEvent), все вызовы функций создающих сообщения должны
//=== использовать данные макросы.
//===
//=================================================================================================

#pragma once

#include "def.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Структура EID
// | 31 30 | 29 | 28 - 26 | 25 - 16 | 15 - 0 |
//   --+--   |    ---+---   ---+---   ---+--
//     |     |       |         |         |
//     |     |       |         |         +----- Номер сообщения (0..65535)
//     |     |       |         +--------------- Объект (1..63 - 3Тонн, 64...1023 - Пользователь)
//     |     |       +------------------------- Резерв (0)
//     |     +--------------------------------- Источник (1 - 3Тонн, 0 - пользователь)
//     +--------------------------------------- Тип сообщения (EMT_*)
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#define MAKE_EID(t, o, i)              ((((t) & 0x3) << 30) | (0x20000000) | (((o) & 0x03FF) << 16) | ((i) & 0xFFFF))

#ifdef TRITONN_TTCE

	#include "../ttce/event_info.h"

	#define CREATE_EID(n, t, o, i)         const UDINT n = MakeEID(#n, t, o, i, nullptr);

#else

	#define CREATE_EID(n, t, o, i)         const UDINT n = ((((t) & 0x3) << 30) | (0x20000000) | (((o) & 0x03FF) << 16) | ((i) & 0xFFFF));

#endif


// Типы событий
const USINT EMT_SUCCESS       = 0;
const USINT EMT_INFO          = 1;
const USINT EMT_WARNING       = 2;
const USINT EMT_ERROR         = 3;


const UINT EVENT_OBJ_UNDEF    = 0x0000;
const UINT EVENT_OBJ_SYSTEM   = 0x0001;
const UINT EVENT_OBJ_TCP      = 0x0002;
const UINT EVENT_OBJ_LOG      = 0x0003;
const UINT EVENT_OBJ_EVENT    = 0x0004;
const UINT EVENT_OBJ_SELECTOR = 0x0005;
const UINT EVENT_OBJ_AI       = 0x0006;
const UINT EVENT_OBJ_FI       = 0x0007;
const UINT EVENT_OBJ_DENSSOL  = 0x0008;
const UINT EVENT_OBJ_RDCDENS  = 0x0009;
const UINT EVENT_OBJ_STREAM   = 0x000A;
const UINT EVENT_OBJ_STATION  = 0x000B;
const UINT EVENT_OBJ_TEST     = 0x000C;
const UINT EVENT_OBJ_REPORT   = 0x000D;
const UINT EVENT_OBJ_VAR      = 0x000E;
const UINT EVENT_OBJ_DI       = 0x000F;
const UINT EVENT_OBJ_DO       = 0x0010;
const UINT EVENT_OBJ_AO       = 0x0011;
const UINT EVENT_OBJ_SAMPLER  = 0x0012;
const UINT EVENT_OBJ_PROVE    = 0x0013;
const UINT EVENT_OBJ_MASWTR   = 0x0014;
const UINT EVENT_OBJ_VOLWTR   = 0x0015;
const UINT EVENT_OBJ_CINVISC  = 0x0016;
const UINT EVENT_OBJ_DYNVISC  = 0x0017;
const UINT EVENT_OBJ__END     = 0x0018;
const UINT EVENT_OBJ_MAX      = 0x003F;

const string EVENT_OBJ_DESC[EVENT_OBJ__END + 1] = {"UNDEF", "SYSTEM", "TCP", "LOG", "EVENT", "SELECTOR", "AI", "FI",
												   "DENSSOL", "REDUCEDDENS", "STREAM", "STATION", "TEST", "REPORT", "VARIABLES", "DI",
												   "DO", "AO", "SAMPLER", "PROVE", "MASSWATER", "VOLUMEWATER",
												   "CINEMATICVISCOSITY", "DYNAMICVISCOSITY",
												   "USER"};



//-------------------------------------------------------------------------------------------------
const UDINT EID_UNDEF                = 0;

//-------------------------------------------------------------------------------------------------
// System
CREATE_EID(EID_SYSTEM_START_WARM      , EMT_INFO   , EVENT_OBJ_SYSTEM  , 1)
CREATE_EID(EID_SYSTEM_START_COLD      , EMT_WARNING, EVENT_OBJ_SYSTEM  , 2)
CREATE_EID(EID_SYSTEM_RUNNING         , EMT_INFO   , EVENT_OBJ_SYSTEM  , 3) // Тритонн запущен
CREATE_EID(EID_SYSTEM_HALT            , EMT_ERROR  , EVENT_OBJ_SYSTEM  , 4) // Возникла критическая ошибка

CREATE_EID(EID_SYSTEM_ERRUNIT         , EMT_ERROR  , EVENT_OBJ_SYSTEM  , 10) // Ошибка конвертации ед. измерения
CREATE_EID(EID_SYSTEM_ERROUTPUT       , EMT_ERROR  , EVENT_OBJ_SYSTEM  , 11)
CREATE_EID(EID_SYSTEM_ERROUTVAL       , EMT_ERROR  , EVENT_OBJ_SYSTEM  , 12)
CREATE_EID(EID_SYSTEM_CFGERROR        , EMT_ERROR  , EVENT_OBJ_SYSTEM  , 20)  // Ошибка конфигурации
CREATE_EID(EID_SYSTEM_FILEIOERROR     , EMT_ERROR  , EVENT_OBJ_SYSTEM  , 21)  // Ошибки файлового в/в

CREATE_EID(EID_SYSTEM_RESTART_WARM    , EMT_WARNING, EVENT_OBJ_SYSTEM  , 30)  // Команда "Warm-restart"
CREATE_EID(EID_SYSTEM_RESTART_COLD    , EMT_WARNING, EVENT_OBJ_SYSTEM  , 31)  // Команда "Cold-restart"
CREATE_EID(EID_SYSTEM_RESTART_UNKNOW  , EMT_ERROR  , EVENT_OBJ_SYSTEM  , 32)  // Неизвестная команда перезагрузки


//-------------------------------------------------------------------------------------------------
// Selector
CREATE_EID(EID_SELECTOR_ERROR         , EMT_ERROR  , EVENT_OBJ_SELECTOR, 1)
CREATE_EID(EID_SELECTOR_NOCHANGE      , EMT_ERROR  , EVENT_OBJ_SELECTOR, 2)
CREATE_EID(EID_SELECTOR_TOFAULT       , EMT_ERROR  , EVENT_OBJ_SELECTOR, 3)
CREATE_EID(EID_SELECTOR_TONEXT        , EMT_INFO   , EVENT_OBJ_SELECTOR, 4)
CREATE_EID(EID_SELECTOR_NOTHINGNEXT   , EMT_ERROR  , EVENT_OBJ_SELECTOR, 5)
CREATE_EID(EID_SELECTOR_CLEARERROR    , EMT_SUCCESS, EVENT_OBJ_SELECTOR, 6)
CREATE_EID(EID_SELECTOR_SELECTED      , EMT_INFO   , EVENT_OBJ_SELECTOR, 7)
CREATE_EID(EID_SELECTOR_MODE          , EMT_INFO   , EVENT_OBJ_SELECTOR, 8)
// Selector.Limits
CREATE_EID(EID_SELECTOR_NEW_AMIN      , EMT_SUCCESS, EVENT_OBJ_SELECTOR, 100)
CREATE_EID(EID_SELECTOR_NEW_WMIN      , EMT_SUCCESS, EVENT_OBJ_SELECTOR, 101)
CREATE_EID(EID_SELECTOR_NEW_WMAX      , EMT_SUCCESS, EVENT_OBJ_SELECTOR, 102)
CREATE_EID(EID_SELECTOR_NEW_AMAX      , EMT_SUCCESS, EVENT_OBJ_SELECTOR, 103)
CREATE_EID(EID_SELECTOR_NEW_HYST      , EMT_SUCCESS, EVENT_OBJ_SELECTOR, 104)
CREATE_EID(EID_SELECTOR_NEW_SETUP     , EMT_SUCCESS, EVENT_OBJ_SELECTOR, 105)
CREATE_EID(EID_SELECTOR_AMIN          , EMT_ERROR  , EVENT_OBJ_SELECTOR, 110)
CREATE_EID(EID_SELECTOR_WMIN          , EMT_WARNING, EVENT_OBJ_SELECTOR, 111)
CREATE_EID(EID_SELECTOR_AMAX          , EMT_ERROR  , EVENT_OBJ_SELECTOR, 112)
CREATE_EID(EID_SELECTOR_WMAX          , EMT_WARNING, EVENT_OBJ_SELECTOR, 113)
CREATE_EID(EID_SELECTOR_NORMAL        , EMT_SUCCESS, EVENT_OBJ_SELECTOR, 114)
CREATE_EID(EID_SELECTOR_NAN           , EMT_ERROR  , EVENT_OBJ_SELECTOR, 115)


//-------------------------------------------------------------------------------------------------
// AI
CREATE_EID(EID_AI_NEW_MIN             , EMT_SUCCESS, EVENT_OBJ_AI      ,   1)
CREATE_EID(EID_AI_NEW_MAX             , EMT_SUCCESS, EVENT_OBJ_AI      ,   2)
CREATE_EID(EID_AI_NEW_SIMULATE        , EMT_SUCCESS, EVENT_OBJ_AI      ,   3)
CREATE_EID(EID_AI_CH_FAULT            , EMT_ERROR  , EVENT_OBJ_AI      ,   4)
CREATE_EID(EID_AI_CH_OK               , EMT_SUCCESS, EVENT_OBJ_AI      ,   5)
CREATE_EID(EID_AI_SIM_MANUAL          , EMT_WARNING, EVENT_OBJ_AI      ,   6)
CREATE_EID(EID_AI_SIM_AUTO            , EMT_WARNING, EVENT_OBJ_AI      ,   7)
CREATE_EID(EID_AI_SIM_LAST            , EMT_WARNING, EVENT_OBJ_AI      ,   8)
CREATE_EID(EID_AI_SIM_OFF             , EMT_WARNING, EVENT_OBJ_AI      ,   9)
CREATE_EID(EID_AI_MIN                 , EMT_ERROR  , EVENT_OBJ_AI      ,  10)
CREATE_EID(EID_AI_MAX                 , EMT_ERROR  , EVENT_OBJ_AI      ,  11)
CREATE_EID(EID_AI_MODULE              , EMT_ERROR  , EVENT_OBJ_AI      ,  12)
// AI.Limits
CREATE_EID(EID_AI_NEW_AMIN            , EMT_SUCCESS, EVENT_OBJ_AI      , 100)
CREATE_EID(EID_AI_NEW_WMIN            , EMT_SUCCESS, EVENT_OBJ_AI      , 101)
CREATE_EID(EID_AI_NEW_WMAX            , EMT_SUCCESS, EVENT_OBJ_AI      , 102)
CREATE_EID(EID_AI_NEW_AMAX            , EMT_SUCCESS, EVENT_OBJ_AI      , 103)
CREATE_EID(EID_AI_NEW_HYST            , EMT_SUCCESS, EVENT_OBJ_AI      , 104)
CREATE_EID(EID_AI_NEW_SETUP           , EMT_SUCCESS, EVENT_OBJ_AI      , 105)
CREATE_EID(EID_AI_AMIN                , EMT_ERROR  , EVENT_OBJ_AI      , 110)
CREATE_EID(EID_AI_WMIN                , EMT_WARNING, EVENT_OBJ_AI      , 111)
CREATE_EID(EID_AI_AMAX                , EMT_ERROR  , EVENT_OBJ_AI      , 112)
CREATE_EID(EID_AI_WMAX                , EMT_WARNING, EVENT_OBJ_AI      , 113)
CREATE_EID(EID_AI_NORMAL              , EMT_SUCCESS, EVENT_OBJ_AI      , 114)
CREATE_EID(EID_AI_NAN                 , EMT_ERROR  , EVENT_OBJ_AI      , 115)
//CREATE_EID(EID_AI_UNDEF               , EMT_ERROR  , EVENT_OBJ_AI      , 116)


//-------------------------------------------------------------------------------------------------
// Counter (FI)
CREATE_EID(EID_COUNTER_CH_FAULT       , EMT_ERROR  , EVENT_OBJ_FI      ,   1)
CREATE_EID(EID_COUNTER_CH_OK          , EMT_SUCCESS, EVENT_OBJ_FI      ,   2)
CREATE_EID(EID_COUNTER_MODULE         , EMT_ERROR  , EVENT_OBJ_FI      ,   3)
// FI.Limits
CREATE_EID(EID_COUNTER_NEW_AMIN       , EMT_SUCCESS, EVENT_OBJ_FI      , 100)
CREATE_EID(EID_COUNTER_NEW_WMIN       , EMT_SUCCESS, EVENT_OBJ_FI      , 101)
CREATE_EID(EID_COUNTER_NEW_WMAX       , EMT_SUCCESS, EVENT_OBJ_FI      , 102)
CREATE_EID(EID_COUNTER_NEW_AMAX       , EMT_SUCCESS, EVENT_OBJ_FI      , 103)
CREATE_EID(EID_COUNTER_NEW_HYST       , EMT_SUCCESS, EVENT_OBJ_FI      , 104)
CREATE_EID(EID_COUNTER_NEW_SETUP      , EMT_SUCCESS, EVENT_OBJ_FI      , 105)
CREATE_EID(EID_COUNTER_AMIN           , EMT_ERROR  , EVENT_OBJ_FI      , 110)
CREATE_EID(EID_COUNTER_WMIN           , EMT_WARNING, EVENT_OBJ_FI      , 111)
CREATE_EID(EID_COUNTER_AMAX           , EMT_ERROR  , EVENT_OBJ_FI      , 112)
CREATE_EID(EID_COUNTER_WMAX           , EMT_WARNING, EVENT_OBJ_FI      , 113)
CREATE_EID(EID_COUNTER_NORMAL         , EMT_SUCCESS, EVENT_OBJ_FI      , 114)
CREATE_EID(EID_COUNTER_NAN            , EMT_ERROR  , EVENT_OBJ_FI      , 115)

//-------------------------------------------------------------------------------------------------
// DensSol
CREATE_EID(EID_DENSSOL_CALIBR         , EMT_WARNING, EVENT_OBJ_DENSSOL ,   1)
CREATE_EID(EID_DENSSOL_K0             , EMT_WARNING, EVENT_OBJ_DENSSOL ,   2)
CREATE_EID(EID_DENSSOL_K1             , EMT_WARNING, EVENT_OBJ_DENSSOL ,   3)
CREATE_EID(EID_DENSSOL_K2             , EMT_WARNING, EVENT_OBJ_DENSSOL ,   4)
CREATE_EID(EID_DENSSOL_K18            , EMT_WARNING, EVENT_OBJ_DENSSOL ,   5)
CREATE_EID(EID_DENSSOL_K19            , EMT_WARNING, EVENT_OBJ_DENSSOL ,   6)
CREATE_EID(EID_DENSSOL_K20A           , EMT_WARNING, EVENT_OBJ_DENSSOL ,   7)
CREATE_EID(EID_DENSSOL_K20B           , EMT_WARNING, EVENT_OBJ_DENSSOL ,   8)
CREATE_EID(EID_DENSSOL_K21A           , EMT_WARNING, EVENT_OBJ_DENSSOL ,   9)
CREATE_EID(EID_DENSSOL_K21B           , EMT_WARNING, EVENT_OBJ_DENSSOL ,  10)
CREATE_EID(EID_DENSSOL_ACCEPT         , EMT_WARNING, EVENT_OBJ_DENSSOL ,  11)
CREATE_EID(EID_DENSSOL_FAULT_INPUTS   , EMT_ERROR  , EVENT_OBJ_DENSSOL ,  12)
CREATE_EID(EID_DENSSOL_GOOD_INPUTS    , EMT_SUCCESS, EVENT_OBJ_DENSSOL ,  13)
CREATE_EID(EID_DENSSOL_FAULT_PERIOD   , EMT_ERROR  , EVENT_OBJ_DENSSOL ,  14)
CREATE_EID(EID_DENSSOL_GOOD_PERIOD    , EMT_SUCCESS, EVENT_OBJ_DENSSOL ,  15)
CREATE_EID(EID_DENSSOL_FAULT_VALUE    , EMT_ERROR  , EVENT_OBJ_DENSSOL ,  16)
CREATE_EID(EID_DENSSOL_GOOD_VALUE     , EMT_SUCCESS, EVENT_OBJ_DENSSOL ,  17)
CREATE_EID(EID_DENSSOL_FAULT_ITERATION, EMT_ERROR  , EVENT_OBJ_DENSSOL ,  18)
CREATE_EID(EID_DENSSOL_GOOD_ITERATION , EMT_SUCCESS, EVENT_OBJ_DENSSOL ,  19)
CREATE_EID(EID_DENSSOL_FAULT_STATION  , EMT_SUCCESS, EVENT_OBJ_DENSSOL ,  20)
// DensSol.Limits
CREATE_EID(EID_DENSSOL_NEW_AMIN       , EMT_SUCCESS, EVENT_OBJ_DENSSOL , 100)
CREATE_EID(EID_DENSSOL_NEW_WMIN       , EMT_SUCCESS, EVENT_OBJ_DENSSOL , 101)
CREATE_EID(EID_DENSSOL_NEW_WMAX       , EMT_SUCCESS, EVENT_OBJ_DENSSOL , 102)
CREATE_EID(EID_DENSSOL_NEW_AMAX       , EMT_SUCCESS, EVENT_OBJ_DENSSOL , 103)
CREATE_EID(EID_DENSSOL_NEW_HYST       , EMT_SUCCESS, EVENT_OBJ_DENSSOL , 104)
CREATE_EID(EID_DENSSOL_NEW_SETUP      , EMT_SUCCESS, EVENT_OBJ_DENSSOL , 105)
CREATE_EID(EID_DENSSOL_AMIN           , EMT_ERROR  , EVENT_OBJ_DENSSOL , 110)
CREATE_EID(EID_DENSSOL_WMIN           , EMT_WARNING, EVENT_OBJ_DENSSOL , 111)
CREATE_EID(EID_DENSSOL_AMAX           , EMT_ERROR  , EVENT_OBJ_DENSSOL , 112)
CREATE_EID(EID_DENSSOL_WMAX           , EMT_WARNING, EVENT_OBJ_DENSSOL , 113)
CREATE_EID(EID_DENSSOL_NORMAL         , EMT_SUCCESS, EVENT_OBJ_DENSSOL , 114)
CREATE_EID(EID_DENSSOL_NAN            , EMT_ERROR  , EVENT_OBJ_DENSSOL , 115)


//-------------------------------------------------------------------------------------------------
// ReducedDensity
CREATE_EID(EID_RDCDDENS_FAULT_INPUTS  , EMT_ERROR  , EVENT_OBJ_RDCDENS ,   1)
CREATE_EID(EID_RDCDDENS_GOOD_INPUTS   , EMT_SUCCESS, EVENT_OBJ_RDCDENS ,   2)
CREATE_EID(EID_RDCDDENS_FAULT_DENSITY , EMT_ERROR  , EVENT_OBJ_RDCDENS ,   3)
CREATE_EID(EID_RDCDDENS_GOOD_DENSITY  , EMT_SUCCESS, EVENT_OBJ_RDCDENS ,   4)
// ReducedDensity.Limits
CREATE_EID(EID_RDCDDENS_NEW_AMIN      , EMT_SUCCESS, EVENT_OBJ_RDCDENS , 100)
CREATE_EID(EID_RDCDDENS_NEW_WMIN      , EMT_SUCCESS, EVENT_OBJ_RDCDENS , 101)
CREATE_EID(EID_RDCDDENS_NEW_WMAX      , EMT_SUCCESS, EVENT_OBJ_RDCDENS , 102)
CREATE_EID(EID_RDCDDENS_NEW_AMAX      , EMT_SUCCESS, EVENT_OBJ_RDCDENS , 103)
CREATE_EID(EID_RDCDDENS_NEW_HYST      , EMT_SUCCESS, EVENT_OBJ_RDCDENS , 104)
CREATE_EID(EID_RDCDDENS_NEW_SETUP     , EMT_SUCCESS, EVENT_OBJ_RDCDENS , 105)
CREATE_EID(EID_RDCDDENS_AMIN          , EMT_ERROR  , EVENT_OBJ_RDCDENS , 110)
CREATE_EID(EID_RDCDDENS_WMIN          , EMT_WARNING, EVENT_OBJ_RDCDENS , 111)
CREATE_EID(EID_RDCDDENS_AMAX          , EMT_ERROR  , EVENT_OBJ_RDCDENS , 112)
CREATE_EID(EID_RDCDDENS_WMAX          , EMT_WARNING, EVENT_OBJ_RDCDENS , 113)
CREATE_EID(EID_RDCDDENS_NORMAL        , EMT_SUCCESS, EVENT_OBJ_RDCDENS , 114)
CREATE_EID(EID_RDCDDENS_NAN           , EMT_ERROR  , EVENT_OBJ_RDCDENS , 115)


//-------------------------------------------------------------------------------------------------
// Stream
CREATE_EID(EID_STREAM_FAULT_INPUTS    , EMT_ERROR  , EVENT_OBJ_STREAM  ,   1)
CREATE_EID(EID_STREAM_GOOD_INPUTS     , EMT_SUCCESS, EVENT_OBJ_STREAM  ,   2)
CREATE_EID(EID_STREAM_ACCOUNTING_ON   , EMT_WARNING, EVENT_OBJ_STREAM  ,   3)
CREATE_EID(EID_STREAM_ACCOUNTING_OFF  , EMT_WARNING, EVENT_OBJ_STREAM  ,   4)
CREATE_EID(EID_STREAM_KEYPADKF_ON     , EMT_WARNING, EVENT_OBJ_STREAM  ,   5)
CREATE_EID(EID_STREAM_KEYPADKF_OFF    , EMT_WARNING, EVENT_OBJ_STREAM  ,   6)
CREATE_EID(EID_STREAM_ACCEPT          , EMT_WARNING, EVENT_OBJ_STREAM  ,   7)
CREATE_EID(EID_STREAM_KEYPADKF        , EMT_WARNING, EVENT_OBJ_STREAM  ,   8)
CREATE_EID(EID_STREAM_KEYPADMF        , EMT_WARNING, EVENT_OBJ_STREAM  ,   9)
CREATE_EID(EID_STREAM_POINT_HZ        , EMT_WARNING, EVENT_OBJ_STREAM  ,  10)
CREATE_EID(EID_STREAM_POINT_KF        , EMT_WARNING, EVENT_OBJ_STREAM  ,  11)
CREATE_EID(EID_STREAM_TOTAL_MASS      , EMT_WARNING, EVENT_OBJ_STREAM  ,  12)
CREATE_EID(EID_STREAM_TOTAL_VOLUME    , EMT_WARNING, EVENT_OBJ_STREAM  ,  13)
CREATE_EID(EID_STREAM_TOTAL_VOLUME15  , EMT_WARNING, EVENT_OBJ_STREAM  ,  14)
CREATE_EID(EID_STREAM_TOTAL_VOLUME20  , EMT_WARNING, EVENT_OBJ_STREAM  ,  15)
// Stream.Limits
CREATE_EID(EID_STREAM_NEW_AMIN        , EMT_SUCCESS, EVENT_OBJ_STREAM  , 100)
CREATE_EID(EID_STREAM_NEW_WMIN        , EMT_SUCCESS, EVENT_OBJ_STREAM  , 101)
CREATE_EID(EID_STREAM_NEW_WMAX        , EMT_SUCCESS, EVENT_OBJ_STREAM  , 102)
CREATE_EID(EID_STREAM_NEW_AMAX        , EMT_SUCCESS, EVENT_OBJ_STREAM  , 103)
CREATE_EID(EID_STREAM_NEW_HYST        , EMT_SUCCESS, EVENT_OBJ_STREAM  , 104)
CREATE_EID(EID_STREAM_NEW_SETUP       , EMT_SUCCESS, EVENT_OBJ_STREAM  , 105)
CREATE_EID(EID_STREAM_AMIN            , EMT_ERROR  , EVENT_OBJ_STREAM  , 110)
CREATE_EID(EID_STREAM_WMIN            , EMT_WARNING, EVENT_OBJ_STREAM  , 111)
CREATE_EID(EID_STREAM_AMAX            , EMT_ERROR  , EVENT_OBJ_STREAM  , 112)
CREATE_EID(EID_STREAM_WMAX            , EMT_WARNING, EVENT_OBJ_STREAM  , 113)
CREATE_EID(EID_STREAM_NORMAL          , EMT_SUCCESS, EVENT_OBJ_STREAM  , 114)
CREATE_EID(EID_STREAM_NAN             , EMT_ERROR  , EVENT_OBJ_STREAM  , 115)

//-------------------------------------------------------------------------------------------------
// Station
CREATE_EID(EID_STATION_TOTAL_MASS     , EMT_WARNING, EVENT_OBJ_STATION ,   1)
CREATE_EID(EID_STATION_TOTAL_VOLUME   , EMT_WARNING, EVENT_OBJ_STATION ,   2)
CREATE_EID(EID_STATION_TOTAL_VOLUME15 , EMT_WARNING, EVENT_OBJ_STATION ,   3)
CREATE_EID(EID_STATION_TOTAL_VOLUME20 , EMT_WARNING, EVENT_OBJ_STATION ,   4)
// Station.Limits
CREATE_EID(EID_STATION_NEW_AMIN       , EMT_SUCCESS, EVENT_OBJ_STATION , 100)
CREATE_EID(EID_STATION_NEW_WMIN       , EMT_SUCCESS, EVENT_OBJ_STATION , 101)
CREATE_EID(EID_STATION_NEW_WMAX       , EMT_SUCCESS, EVENT_OBJ_STATION , 102)
CREATE_EID(EID_STATION_NEW_AMAX       , EMT_SUCCESS, EVENT_OBJ_STATION , 103)
CREATE_EID(EID_STATION_NEW_HYST       , EMT_SUCCESS, EVENT_OBJ_STATION , 104)
CREATE_EID(EID_STATION_NEW_SETUP      , EMT_SUCCESS, EVENT_OBJ_STATION , 105)
CREATE_EID(EID_STATION_AMIN           , EMT_ERROR  , EVENT_OBJ_STATION , 110)
CREATE_EID(EID_STATION_WMIN           , EMT_WARNING, EVENT_OBJ_STATION , 111)
CREATE_EID(EID_STATION_AMAX           , EMT_ERROR  , EVENT_OBJ_STATION , 112)
CREATE_EID(EID_STATION_WMAX           , EMT_WARNING, EVENT_OBJ_STATION , 113)
CREATE_EID(EID_STATION_NORMAL         , EMT_SUCCESS, EVENT_OBJ_STATION , 114)
CREATE_EID(EID_STATION_NAN            , EMT_ERROR  , EVENT_OBJ_STATION , 115)


CREATE_EID(EID_TEST_SUCCESS           , EMT_SUCCESS, EVENT_OBJ_TEST    ,   1)
CREATE_EID(EID_TEST_INFO              , EMT_INFO   , EVENT_OBJ_TEST    ,   2)
CREATE_EID(EID_TEST_WARNING           , EMT_WARNING, EVENT_OBJ_TEST    ,   3)
CREATE_EID(EID_TEST_ERROR             , EMT_ERROR  , EVENT_OBJ_TEST    ,   4)

//-------------------------------------------------------------------------------------------------
// Report
CREATE_EID(EID_REPORT_GENERATED       , EMT_SUCCESS, EVENT_OBJ_REPORT  ,   1)
CREATE_EID(EID_REPORT_CANTSAVE        , EMT_ERROR  , EVENT_OBJ_REPORT  ,   2)


//-------------------------------------------------------------------------------------------------
// Var
CREATE_EID(EID_VAR_NEW_AMIN           , EMT_SUCCESS, EVENT_OBJ_VAR     , 100)
CREATE_EID(EID_VAR_NEW_WMIN           , EMT_SUCCESS, EVENT_OBJ_VAR     , 101)
CREATE_EID(EID_VAR_NEW_WMAX           , EMT_SUCCESS, EVENT_OBJ_VAR     , 102)
CREATE_EID(EID_VAR_NEW_AMAX           , EMT_SUCCESS, EVENT_OBJ_VAR     , 103)
CREATE_EID(EID_VAR_NEW_HYST           , EMT_SUCCESS, EVENT_OBJ_VAR     , 104)
CREATE_EID(EID_VAR_NEW_SETUP          , EMT_SUCCESS, EVENT_OBJ_VAR     , 105)
CREATE_EID(EID_VAR_AMIN               , EMT_ERROR  , EVENT_OBJ_VAR     , 110)
CREATE_EID(EID_VAR_WMIN               , EMT_WARNING, EVENT_OBJ_VAR     , 111)
CREATE_EID(EID_VAR_AMAX               , EMT_ERROR  , EVENT_OBJ_VAR     , 112)
CREATE_EID(EID_VAR_WMAX               , EMT_WARNING, EVENT_OBJ_VAR     , 113)
CREATE_EID(EID_VAR_NORMAL             , EMT_SUCCESS, EVENT_OBJ_VAR     , 114)
CREATE_EID(EID_VAR_NAN                , EMT_ERROR  , EVENT_OBJ_VAR     , 115)


//-------------------------------------------------------------------------------------------------
// DI
CREATE_EID(EID_DI_SUCCESS_ON          , EMT_SUCCESS, EVENT_OBJ_DI      ,   1)
CREATE_EID(EID_DI_SUCCESS_OFF         , EMT_SUCCESS, EVENT_OBJ_DI      ,   2)
CREATE_EID(EID_DI_WARNING_ON          , EMT_WARNING, EVENT_OBJ_DI      ,   3)
CREATE_EID(EID_DI_WARNING_OFF         , EMT_WARNING, EVENT_OBJ_DI      ,   4)
CREATE_EID(EID_DI_ALARM_ON            , EMT_ERROR  , EVENT_OBJ_DI      ,   5)
CREATE_EID(EID_DI_ALARM_OFF           , EMT_ERROR  , EVENT_OBJ_DI      ,   6)
CREATE_EID(EID_DI_NEW_SIMULATE        , EMT_SUCCESS, EVENT_OBJ_DI      ,  10)
CREATE_EID(EID_DI_CH_FAULT            , EMT_ERROR  , EVENT_OBJ_DI      ,  11)
CREATE_EID(EID_DI_CH_OK               , EMT_SUCCESS, EVENT_OBJ_DI      ,  12)
CREATE_EID(EID_DI_KEYPAD_ON           , EMT_WARNING, EVENT_OBJ_DI      ,  13)
CREATE_EID(EID_DI_KEYPAD_OFF          , EMT_WARNING, EVENT_OBJ_DI      ,  14)
CREATE_EID(EID_DI_MODULE              , EMT_ERROR  , EVENT_OBJ_DI      ,  15)
CREATE_EID(EID_DI_NEW_SETUP           , EMT_SUCCESS, EVENT_OBJ_DI      ,  16)


//-------------------------------------------------------------------------------------------------
// DO
CREATE_EID(EID_DO_SUCCESS_ON          , EMT_SUCCESS, EVENT_OBJ_DO      ,   1)
CREATE_EID(EID_DO_SUCCESS_OFF         , EMT_SUCCESS, EVENT_OBJ_DO      ,   2)
CREATE_EID(EID_DO_WARNING_ON          , EMT_WARNING, EVENT_OBJ_DO      ,   3)
CREATE_EID(EID_DO_WARNING_OFF         , EMT_WARNING, EVENT_OBJ_DO      ,   4)
CREATE_EID(EID_DO_ALARM_ON            , EMT_ERROR  , EVENT_OBJ_DO      ,   5)
CREATE_EID(EID_DO_ALARM_OFF           , EMT_ERROR  , EVENT_OBJ_DO      ,   6)
CREATE_EID(EID_DO_CH_FAULT            , EMT_ERROR  , EVENT_OBJ_DO      ,  10)
CREATE_EID(EID_DO_CH_OK               , EMT_SUCCESS, EVENT_OBJ_DO      ,  11)
CREATE_EID(EID_DO_KEYPAD_ON           , EMT_WARNING, EVENT_OBJ_DO      ,  12)
CREATE_EID(EID_DO_KEYPAD_OFF          , EMT_WARNING, EVENT_OBJ_DO      ,  13)
CREATE_EID(EID_DO_MODULE              , EMT_ERROR  , EVENT_OBJ_DO      ,  14)
CREATE_EID(EID_DO_NEW_SETUP           , EMT_SUCCESS, EVENT_OBJ_DO      ,  15)


//-------------------------------------------------------------------------------------------------
// SAMPLER
CREATE_EID(EID_SAMPLER_START_PERIOD   , EMT_SUCCESS, EVENT_OBJ_SAMPLER ,   1)
CREATE_EID(EID_SAMPLER_START_VOLUME   , EMT_SUCCESS, EVENT_OBJ_SAMPLER ,   2)
CREATE_EID(EID_SAMPLER_START_MASS     , EMT_SUCCESS, EVENT_OBJ_SAMPLER ,   3)
CREATE_EID(EID_SAMPLER_START_TEST     , EMT_SUCCESS, EVENT_OBJ_SAMPLER ,   4)
CREATE_EID(EID_SAMPLER_STOP           , EMT_SUCCESS, EVENT_OBJ_SAMPLER ,   5)
CREATE_EID(EID_SAMPLER_DONT_STOP      , EMT_ERROR  , EVENT_OBJ_SAMPLER ,   6)
CREATE_EID(EID_SAMPLER_FINISH         , EMT_WARNING, EVENT_OBJ_SAMPLER ,   7)
CREATE_EID(EID_SAMPLER_CONFIRM        , EMT_WARNING, EVENT_OBJ_SAMPLER ,   8)
CREATE_EID(EID_SAMPLER_MODE_FAULT     , EMT_ERROR  , EVENT_OBJ_SAMPLER ,  10)
CREATE_EID(EID_SAMPLER_COMMAND_FAULT  , EMT_ERROR  , EVENT_OBJ_SAMPLER ,  11)
CREATE_EID(EID_SAMPLER_SELECT_FAULT   , EMT_ERROR  , EVENT_OBJ_SAMPLER ,  12)
CREATE_EID(EID_SAMPLER_START_FAULT    , EMT_ERROR  , EVENT_OBJ_SAMPLER ,  13)


//-------------------------------------------------------------------------------------------------
// PROVE
CREATE_EID(EID_PROVE_BADCOMMAND       , EMT_ERROR  , EVENT_OBJ_PROVE   ,   1)
CREATE_EID(EID_PROVE_MODULE           , EMT_ERROR  , EVENT_OBJ_PROVE   ,   2)
CREATE_EID(EID_PROVE_ERRORSTABTEMP    , EMT_ERROR  , EVENT_OBJ_PROVE   ,   4)
CREATE_EID(EID_PROVE_ERRORSTABPRES    , EMT_ERROR  , EVENT_OBJ_PROVE   ,   5)
CREATE_EID(EID_PROVE_ERRORSTABDENS    , EMT_ERROR  , EVENT_OBJ_PROVE   ,   6)
CREATE_EID(EID_PROVE_ERRORSTABFREQ    , EMT_ERROR  , EVENT_OBJ_PROVE   ,   7)
CREATE_EID(EID_PROVE_BADSTREAMNUMBER  , EMT_ERROR  , EVENT_OBJ_PROVE   ,   8)
CREATE_EID(EID_PROVE_CANNOTMODIFYSETUP, EMT_ERROR  , EVENT_OBJ_PROVE   ,   9)
// Prove.Command
CREATE_EID(EID_PROVE_COMMANDSTART     , EMT_SUCCESS, EVENT_OBJ_PROVE   ,  21)
CREATE_EID(EID_PROVE_COMMANDABORT     , EMT_WARNING, EVENT_OBJ_PROVE   ,  22)
CREATE_EID(EID_PROVE_COMMANDRESET     , EMT_SUCCESS, EVENT_OBJ_PROVE   ,  23)
CREATE_EID(EID_PROVE_NOTSTARTED       , EMT_WARNING, EVENT_OBJ_PROVE   ,  24)
CREATE_EID(EID_PROVE_ALREADYSTARTED   , EMT_WARNING, EVENT_OBJ_PROVE   ,  25)
CREATE_EID(EID_PROVE_NOTERROR         , EMT_WARNING, EVENT_OBJ_PROVE   ,  26)
CREATE_EID(EID_PROVE_ISERROR          , EMT_WARNING, EVENT_OBJ_PROVE   ,  27)
// Prove.detectors
CREATE_EID(EID_PROVE_DETECTOR1        , EMT_SUCCESS, EVENT_OBJ_PROVE   ,  31)
CREATE_EID(EID_PROVE_DETECTOR2        , EMT_SUCCESS, EVENT_OBJ_PROVE   ,  32)
CREATE_EID(EID_PROVE_DETECTOR3        , EMT_SUCCESS, EVENT_OBJ_PROVE   ,  33)
CREATE_EID(EID_PROVE_DETECTOR4        , EMT_SUCCESS, EVENT_OBJ_PROVE   ,  34)
// Prove.State
CREATE_EID(EID_PROVE_STATE_IDLE       , EMT_SUCCESS, EVENT_OBJ_PROVE   ,  41)
CREATE_EID(EID_PROVE_STATE_START      , EMT_SUCCESS, EVENT_OBJ_PROVE   ,  42)
CREATE_EID(EID_PROVE_STATE_STAB       , EMT_SUCCESS, EVENT_OBJ_PROVE   ,  43)
CREATE_EID(EID_PROVE_STATE_RUNBALL    , EMT_SUCCESS, EVENT_OBJ_PROVE   ,  44)
CREATE_EID(EID_PROVE_STATE_VALVETOUP  , EMT_SUCCESS, EVENT_OBJ_PROVE   ,  45)
CREATE_EID(EID_PROVE_STATE_WAITTOUP   , EMT_SUCCESS, EVENT_OBJ_PROVE   ,  46)
CREATE_EID(EID_PROVE_STATE_VALVETODOWN, EMT_SUCCESS, EVENT_OBJ_PROVE   ,  47)
CREATE_EID(EID_PROVE_STATE_WAITD1     , EMT_SUCCESS, EVENT_OBJ_PROVE   ,  48)
CREATE_EID(EID_PROVE_STATE_WAITD2     , EMT_SUCCESS, EVENT_OBJ_PROVE   ,  49)
CREATE_EID(EID_PROVE_STATE_CALCULATE  , EMT_SUCCESS, EVENT_OBJ_PROVE   ,  50)
CREATE_EID(EID_PROVE_STATE_RETURNBALL , EMT_SUCCESS, EVENT_OBJ_PROVE   ,  51)
CREATE_EID(EID_PROVE_STATE_WAITED1_RVS, EMT_SUCCESS, EVENT_OBJ_PROVE   ,  52)
CREATE_EID(EID_PROVE_STATE_WAITED2_RVS, EMT_SUCCESS, EVENT_OBJ_PROVE   ,  53)
CREATE_EID(EID_PROVE_STATE_WAYCOMPLTED, EMT_SUCCESS, EVENT_OBJ_PROVE   ,  54)
CREATE_EID(EID_PROVE_STATE_FINISH     , EMT_SUCCESS, EVENT_OBJ_PROVE   ,  55)
CREATE_EID(EID_PROVE_STATE_ABORT      , EMT_WARNING, EVENT_OBJ_PROVE   ,  56)
CREATE_EID(EID_PROVE_STATE_ERRORFLOW  , EMT_ERROR  , EVENT_OBJ_PROVE   ,  57)
CREATE_EID(EID_PROVE_STATE_ERRORSTAB  , EMT_ERROR  , EVENT_OBJ_PROVE   ,  58)
CREATE_EID(EID_PROVE_STATE_ERRORTOUP  , EMT_ERROR  , EVENT_OBJ_PROVE   ,  59)
CREATE_EID(EID_PROVE_STATE_ERRORTDOWN , EMT_ERROR  , EVENT_OBJ_PROVE   ,  60)
CREATE_EID(EID_PROVE_STATE_ERRORD1    , EMT_ERROR  , EVENT_OBJ_PROVE   ,  61)
CREATE_EID(EID_PROVE_STATE_ERRORD2    , EMT_ERROR  , EVENT_OBJ_PROVE   ,  62)
CREATE_EID(EID_PROVE_STATE_ERRORDETS  , EMT_ERROR  , EVENT_OBJ_PROVE   ,  63)
CREATE_EID(EID_PROVE_STATE_ERRORRETURN, EMT_ERROR  , EVENT_OBJ_PROVE   ,  64)
CREATE_EID(EID_PROVE_STATE_ERRORRSTRID, EMT_ERROR  , EVENT_OBJ_PROVE   ,  65)
CREATE_EID(EID_PROVE_STATE_ERRORD1_RVS, EMT_ERROR  , EVENT_OBJ_PROVE   ,  66)
CREATE_EID(EID_PROVE_STATE_ERRORD2_RVS, EMT_ERROR  , EVENT_OBJ_PROVE   ,  67)
// Prove.Limits
CREATE_EID(EID_PROVE_NEW_AMIN         , EMT_SUCCESS, EVENT_OBJ_PROVE   , 100)
CREATE_EID(EID_PROVE_NEW_WMIN         , EMT_SUCCESS, EVENT_OBJ_PROVE   , 101)
CREATE_EID(EID_PROVE_NEW_WMAX         , EMT_SUCCESS, EVENT_OBJ_PROVE   , 102)
CREATE_EID(EID_PROVE_NEW_AMAX         , EMT_SUCCESS, EVENT_OBJ_PROVE   , 103)
CREATE_EID(EID_PROVE_NEW_HYST         , EMT_SUCCESS, EVENT_OBJ_PROVE   , 104)
CREATE_EID(EID_PROVE_NEW_SETUP        , EMT_SUCCESS, EVENT_OBJ_PROVE   , 105)
CREATE_EID(EID_PROVE_AMIN             , EMT_ERROR  , EVENT_OBJ_PROVE   , 110)
CREATE_EID(EID_PROVE_WMIN             , EMT_WARNING, EVENT_OBJ_PROVE   , 111)
CREATE_EID(EID_PROVE_AMAX             , EMT_ERROR  , EVENT_OBJ_PROVE   , 112)
CREATE_EID(EID_PROVE_WMAX             , EMT_WARNING, EVENT_OBJ_PROVE   , 113)
CREATE_EID(EID_PROVE_NORMAL           , EMT_SUCCESS, EVENT_OBJ_PROVE   , 114)
CREATE_EID(EID_PROVE_NAN              , EMT_ERROR  , EVENT_OBJ_PROVE   , 115)


//-------------------------------------------------------------------------------------------------
// MASSWATER
// Limits
CREATE_EID(EID_MASWTR_NEW_AMIN        , EMT_SUCCESS, EVENT_OBJ_MASWTR  , 100)
CREATE_EID(EID_MASWTR_NEW_WMIN        , EMT_SUCCESS, EVENT_OBJ_MASWTR  , 101)
CREATE_EID(EID_MASWTR_NEW_WMAX        , EMT_SUCCESS, EVENT_OBJ_MASWTR  , 102)
CREATE_EID(EID_MASWTR_NEW_AMAX        , EMT_SUCCESS, EVENT_OBJ_MASWTR  , 103)
CREATE_EID(EID_MASWTR_NEW_HYST        , EMT_SUCCESS, EVENT_OBJ_MASWTR  , 104)
CREATE_EID(EID_MASWTR_NEW_SETUP       , EMT_SUCCESS, EVENT_OBJ_MASWTR  , 105)
CREATE_EID(EID_MASWTR_AMIN            , EMT_ERROR  , EVENT_OBJ_MASWTR  , 110)
CREATE_EID(EID_MASWTR_WMIN            , EMT_WARNING, EVENT_OBJ_MASWTR  , 111)
CREATE_EID(EID_MASWTR_AMAX            , EMT_ERROR  , EVENT_OBJ_MASWTR  , 112)
CREATE_EID(EID_MASWTR_WMAX            , EMT_WARNING, EVENT_OBJ_MASWTR  , 113)
CREATE_EID(EID_MASWTR_NORMAL          , EMT_SUCCESS, EVENT_OBJ_MASWTR  , 114)
CREATE_EID(EID_MASWTR_NAN             , EMT_ERROR  , EVENT_OBJ_MASWTR  , 115)

//-------------------------------------------------------------------------------------------------
// VOLWATER
// Limits
CREATE_EID(EID_VOLWTR_NEW_AMIN        , EMT_SUCCESS, EVENT_OBJ_VOLWTR  , 100)
CREATE_EID(EID_VOLWTR_NEW_WMIN        , EMT_SUCCESS, EVENT_OBJ_VOLWTR  , 101)
CREATE_EID(EID_VOLWTR_NEW_WMAX        , EMT_SUCCESS, EVENT_OBJ_VOLWTR  , 102)
CREATE_EID(EID_VOLWTR_NEW_AMAX        , EMT_SUCCESS, EVENT_OBJ_VOLWTR  , 103)
CREATE_EID(EID_VOLWTR_NEW_HYST        , EMT_SUCCESS, EVENT_OBJ_VOLWTR  , 104)
CREATE_EID(EID_VOLWTR_NEW_SETUP       , EMT_SUCCESS, EVENT_OBJ_VOLWTR  , 105)
CREATE_EID(EID_VOLWTR_AMIN            , EMT_ERROR  , EVENT_OBJ_VOLWTR  , 110)
CREATE_EID(EID_VOLWTR_WMIN            , EMT_WARNING, EVENT_OBJ_VOLWTR  , 111)
CREATE_EID(EID_VOLWTR_AMAX            , EMT_ERROR  , EVENT_OBJ_VOLWTR  , 112)
CREATE_EID(EID_VOLWTR_WMAX            , EMT_WARNING, EVENT_OBJ_VOLWTR  , 113)
CREATE_EID(EID_VOLWTR_NORMAL          , EMT_SUCCESS, EVENT_OBJ_VOLWTR  , 114)
CREATE_EID(EID_VOLWTR_NAN             , EMT_ERROR  , EVENT_OBJ_VOLWTR  , 115)

//-------------------------------------------------------------------------------------------------
// CINVISC
// Limits
CREATE_EID(EID_CINVISC_NEW_AMIN        , EMT_SUCCESS, EVENT_OBJ_CINVISC, 100)
CREATE_EID(EID_CINVISC_NEW_WMIN        , EMT_SUCCESS, EVENT_OBJ_CINVISC, 101)
CREATE_EID(EID_CINVISC_NEW_WMAX        , EMT_SUCCESS, EVENT_OBJ_CINVISC, 102)
CREATE_EID(EID_CINVISC_NEW_AMAX        , EMT_SUCCESS, EVENT_OBJ_CINVISC, 103)
CREATE_EID(EID_CINVISC_NEW_HYST        , EMT_SUCCESS, EVENT_OBJ_CINVISC, 104)
CREATE_EID(EID_CINVISC_NEW_SETUP       , EMT_SUCCESS, EVENT_OBJ_CINVISC, 105)
CREATE_EID(EID_CINVISC_AMIN            , EMT_ERROR  , EVENT_OBJ_CINVISC, 110)
CREATE_EID(EID_CINVISC_WMIN            , EMT_WARNING, EVENT_OBJ_CINVISC, 111)
CREATE_EID(EID_CINVISC_AMAX            , EMT_ERROR  , EVENT_OBJ_CINVISC, 112)
CREATE_EID(EID_CINVISC_WMAX            , EMT_WARNING, EVENT_OBJ_CINVISC, 113)
CREATE_EID(EID_CINVISC_NORMAL          , EMT_SUCCESS, EVENT_OBJ_CINVISC, 114)
CREATE_EID(EID_CINVISC_NAN             , EMT_ERROR  , EVENT_OBJ_CINVISC, 115)

//-------------------------------------------------------------------------------------------------
// DYNVISC
// Limits
CREATE_EID(EID_DYNVISC_NEW_AMIN        , EMT_SUCCESS, EVENT_OBJ_DYNVISC, 100)
CREATE_EID(EID_DYNVISC_NEW_WMIN        , EMT_SUCCESS, EVENT_OBJ_DYNVISC, 101)
CREATE_EID(EID_DYNVISC_NEW_WMAX        , EMT_SUCCESS, EVENT_OBJ_DYNVISC, 102)
CREATE_EID(EID_DYNVISC_NEW_AMAX        , EMT_SUCCESS, EVENT_OBJ_DYNVISC, 103)
CREATE_EID(EID_DYNVISC_NEW_HYST        , EMT_SUCCESS, EVENT_OBJ_DYNVISC, 104)
CREATE_EID(EID_DYNVISC_NEW_SETUP       , EMT_SUCCESS, EVENT_OBJ_DYNVISC, 105)
CREATE_EID(EID_DYNVISC_AMIN            , EMT_ERROR  , EVENT_OBJ_DYNVISC, 110)
CREATE_EID(EID_DYNVISC_WMIN            , EMT_WARNING, EVENT_OBJ_DYNVISC, 111)
CREATE_EID(EID_DYNVISC_AMAX            , EMT_ERROR  , EVENT_OBJ_DYNVISC, 112)
CREATE_EID(EID_DYNVISC_WMAX            , EMT_WARNING, EVENT_OBJ_DYNVISC, 113)
CREATE_EID(EID_DYNVISC_NORMAL          , EMT_SUCCESS, EVENT_OBJ_DYNVISC, 114)
CREATE_EID(EID_DYNVISC_NAN             , EMT_ERROR  , EVENT_OBJ_DYNVISC, 115)
