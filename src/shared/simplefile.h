//=================================================================================================
//===
//=== simplefile.h
//===
//=== Copyright (c) 2019-2021 by RangeSoft.
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

extern unsigned int simpleFileCreateDir(const std::string& filename);
extern unsigned int simpleFileDelete(const std::string& filename);
extern unsigned int simpleFileLoad(const std::string& filename, std::string& text);
extern unsigned int simpleFileLoad(const std::string& filename, std::vector<char>& data);
extern unsigned int simpleFileSave(const std::string& filename, const std::string& text);
extern unsigned int simpleFileAppend(const std::string& filename, const std::string& text);
extern unsigned int simpleFileGaranteedSave(const std::string& filename, const std::string& text);
