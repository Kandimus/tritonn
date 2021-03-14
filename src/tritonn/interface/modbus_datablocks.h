//=================================================================================================
//===
//=== modbus_datablock.h
//===
//=== Copyright (c) 2019-2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#pragma once

#include <string>

namespace tinyxml2 {
	class XMLElement;
}

namespace rDataBlock {

std::string getXml();
tinyxml2::XMLElement* Find(tinyxml2::XMLElement* xml_blocks, const std::string& name);

}




