//=================================================================================================
//===
//=== comment_defines.h
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Вспомогательные константы
//===
//=================================================================================================

#pragma once

#include "def.h"

namespace COMMENT
{

const std::string CALC_COEF      = "Вычесленный коэффициент";
const std::string FACTOR         = "Коффициент";
const std::string FACTOR_ACC     = "Команда:\n 0 - нет действия\n 1 - применить введенные коффициенты";
const std::string FACTOR_SET     =  "Введенный коффициент";
const std::string FAULT          = "Флаг ошибки";
const std::string KEYPAD         = "Значение ручного ввода";
const std::string MODE           = "Режим:\n";
const std::string MODE_PHYS      = "Используется физическое значение";
const std::string MODE_KEYPAD    = "Переключение пользователем на ручной ввод";
const std::string SETUP          = "Настройка:\n";
const std::string SETUP_OFF      = "Выключен и не обрабатывается";
const std::string SETUP_ALARM0   = "При снятии сигнала выдавать аварийное сообщение";
const std::string SETUP_ALARM1   = "При установке сигнала выдавать аварийное сообщение";
const std::string SETUP_SUCCESS0 = "При снятии сигнала выдавать успешное сообщение";
const std::string SETUP_SUCCESS1 = "При установке сигнала выдавать успешное сообщение";
const std::string SETUP_WARNING0 = "При снятии сигнала выдавать предупредительне сообщение";
const std::string SETUP_WARNING1 = "При установке сигнала выдавать предупредительне сообщение";
const std::string STATUS         = "Статус:\n";
const std::string STATUS_FAULT   = "Выход из строя канала или модуля";
const std::string STATUS_NORMAL  = "Значение в норме";
const std::string STATUS_OFF     = "Сигнал выключен";
const std::string STATUS_UNDEF   = "Статус не определен";

const std::string FAULT_STRID   = "> НЕТ ОПИСАНИЯ! <";

};

