//=================================================================================================
//===
//=== modbus_datablocks.cpp
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#include "modbus_datablocks.h"
#include "xml_util.h"


//-------------------------------------------------------------------------------------------------
// Поиск требуемого dataset в дереве конфигурации
tinyxml2::XMLElement* rDataBlock::Find(tinyxml2::XMLElement* element, const std::string& name)
{
	auto xml_modbus = element->Parent();
	if (!xml_modbus) {
		return nullptr;
	}

	auto xml_comms = xml_modbus->Parent();
	if (!xml_comms) {
		return nullptr;
	}

	auto xml_blocks = xml_comms->FirstChildElement(XmlName::DATABLOCKS);
	if (!xml_blocks) {
		return nullptr;
	}

	XML_FOR(xml_item, xml_blocks, XmlName::DATABLOCK) {
		const char *item_name = xml_item->Attribute(XmlName::NAME);

		if (!item_name) {
			continue;
		}

		if (String_equali(name, item_name)) {
			return xml_item;
		}
	}

	return nullptr;
}


std::string rDataBlock::getXml()
{
	std::string result = "## DataBlocks\n````xml\n";

	result += String_format("<%s>\n", XmlName::DATABLOCKS);
	result += String_format("\t<%s name=\"datablock name\">\n", XmlName::DATABLOCK);
	result += String_format("\t\t<%s>variable alias</%s>\n", XmlName::VARIABLE, XmlName::VARIABLE);
	result += String_format("\t\t<%s convert=\"datatype\">variable alias</%s> <!-- convert is optional -->\n", XmlName::VARIABLE, XmlName::VARIABLE);
	result += "\t\t...\n";
	result += String_format("\t\t<%s>variable alias</%s>\n", XmlName::VARIABLE, XmlName::VARIABLE);
	result += String_format("\t</%s>\n", XmlName::DATABLOCK);
	result += "\t...\n";
	result += String_format("\t<%s name=\"datablock name\">\n", XmlName::DATABLOCK);
	result += String_format("\t\t<%s>variable alias</%s>\n", XmlName::VARIABLE, XmlName::VARIABLE);
	result += String_format("\t\t<%s convert=\"datatype\">variable alias</%s> <!-- convert is optional -->\n", XmlName::VARIABLE, XmlName::VARIABLE);
	result += "\t\t...\n";
	result += String_format("\t\t<%s>variable alias</%s>\n", XmlName::VARIABLE, XmlName::VARIABLE);
	result += String_format("\t</%s>\n", XmlName::DATABLOCK);
	result += String_format("</%s>\n", XmlName::DATABLOCKS);

	result += "````\n";

	return result;
}
