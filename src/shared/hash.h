//=================================================================================================
//===
//=== hash.cpp
//===
//=== Copyright (c) 2019-2020 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Описание типовых функций и утилит для работ с хешами SHA-1
//===
//=================================================================================================

#pragma once

#include <string>
#include "types.h"

using std::string;

//namespace rHash {


extern UDINT GetStrHash(const string &str, USINT *buff);
extern UDINT GetBufHash(const void *data, UDINT size, USINT *buff);
extern UDINT CmpHash(const USINT *buff1, const USINT *buff2);

extern int EncryptEAS(const string &text, const unsigned char key[16], const unsigned char iv[16], string &result);
extern int DecryptEAS(const string &text, const unsigned char key[16], const unsigned char iv[16], string &result);

//}
