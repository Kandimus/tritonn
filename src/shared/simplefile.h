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

extern UDINT SimpleFileCreateDir(const std::string& filename);
extern UDINT SimpleFileDelete(const std::string& filename);
extern UDINT SimpleFileLoad(const std::string& filename, std::string& text);
extern UDINT SimpleFileSave(const std::string& filename, const std::string& text);
extern UDINT SimpleFileAppend(const std::string& filename, const std::string& text);
extern UDINT SimpleFileGaranteedSave(const std::string& filename, const std::string& text);
