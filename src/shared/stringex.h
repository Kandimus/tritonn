//=================================================================================================
//===
//=== stringex.h
//===
//=== Copyright (c) 2019-2020 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Расширение функций стандартных строк С++
//===
//=================================================================================================

#pragma once

#include <string>

extern int         String_IsValidHex(const char *str, unsigned int &val);
extern int         String_BufferSize(const char *format, va_list arg_ptr);
extern std::string String_format(const char *format, ...);
extern std::string String_vaformat(const char *format, va_list arg_ptr);
extern std::string String_FileTime(time_t t);
extern std::string String_tolower(const std::string &str);
extern std::string String_toupper(const std::string &str);
extern bool        String_equali(const std::string &str1, const std::string &str2);
extern std::string String_deletewhite(const std::string &str);
extern int         String_ToBuffer(const char *str, unsigned char *buf, unsigned int bufsize);
extern std::string String_FromBuffer(const unsigned char *buf, unsigned int bufsize);
