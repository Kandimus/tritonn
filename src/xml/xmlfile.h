//=================================================================================================
//===
//=== xmlfile.h
//===
//=== Copyright (c) 2020 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Проверка XML файла на соответствие HASH
//===
//=================================================================================================

#pragma once

#include <string>
#include "types.h"
#include "tinyxml2.h"

extern UDINT xmlDumpFile(const std::string&filename, tinyxml2::XMLDocument &doc, std::string& strhash);
extern UDINT xmlFileCheck(const std::string&filename, tinyxml2::XMLDocument &doc, const std::string& signature, const std::string& salt);
extern UDINT xmlFileSave(const std::string& filename, const std::string& text, const std::string& marker);

