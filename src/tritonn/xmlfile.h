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

using std::string;


extern UDINT XMLFileCheck(const string &filename, tinyxml2::XMLDocument &doc);

