//=================================================================================================
//===
//=== simplefile.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Загрузка и сохранение простого текстового файла
//===
//=================================================================================================

#pragma once


#include "def.h"

extern UDINT SimpleFileCreateDir(const string &filename);
extern UDINT SimpleFileDelete(const string &filename);
extern UDINT SimpleFileLoad(const string &filename,       string &text);
extern UDINT SimpleFileSave(const string &filename, const string &text);

