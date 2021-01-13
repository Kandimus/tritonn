//=================================================================================================
//===
//=== generator_md.h
//===
//=== Copyright (c) 2020 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Генератор файлов помощи MD
//===
//=================================================================================================

#include "generator_md.h"
#include "data_source.h"
#include "bits_array.h"
#include "simplefile.h"

const char* rGeneratorMD::rItem::XML_OPTIONAL = "<!-- Optional -->";

rGeneratorMD::rGeneratorMD()
{
}

rGeneratorMD::~rGeneratorMD()
{
}

rGeneratorMD::rItem& rGeneratorMD::add(rSource* source, bool isstdinput)
{
//	if (!source) {
//		return nullptr;
//	}

	m_items.push_back(rItem(source, isstdinput));


	return m_items.back();
}

UDINT rGeneratorMD::save(std::string path)
{
	for (auto& item : m_items) {
		SimpleFileSave(path + "/" + item.getName() + ".md", item.save());
	}

	return TRITONN_RESULT_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////

rGeneratorMD::rItem::rItem(rSource* source, bool isstdinput)
{
	m_source     = source;
	m_name       = source->RTTI();
	m_isStdInput = isstdinput;
}

rGeneratorMD::rItem::~rItem()
{
	;
}

rGeneratorMD::rItem& rGeneratorMD::rItem::addProperty(const std::string& name, const rBitsArray* bits)
{
	m_properties.push_back(rProperty());
	m_properties.back().m_name = name;
	m_properties.back().m_bits = bits;
	m_properties.back().m_type = ItemType::BITSFLAG;

	return *this;
}

rGeneratorMD::rItem& rGeneratorMD::rItem::addProperty(const std::string& name, UDINT defval)
{
	m_properties.push_back(rProperty());
	m_properties.back().m_name   = name;
	m_properties.back().m_intVal = defval;
	m_properties.back().m_type   = ItemType::UDINT_VAL;

	return *this;
}

rGeneratorMD::rItem& rGeneratorMD::rItem::addProperty(const std::string& name, LREAL defval)
{
	m_properties.push_back(rProperty());
	m_properties.back().m_name    = name;
	m_properties.back().m_realVal = defval;
	m_properties.back().m_type    = ItemType::LREAL_VAL;

	return *this;
}

rGeneratorMD::rItem& rGeneratorMD::rItem::addXml(const std::string& xmlstring, bool isoptional)
{
	m_xml.push_back(xmlstring + (isoptional ? XML_OPTIONAL : ""));

	return *this;
}

rGeneratorMD::rItem& rGeneratorMD::rItem::addXml(const std::string& xmlname, const std::string& defval, bool isoptional)
{
	return addXml(String_format("<%s>%s<%s/> %s", xmlname.c_str(), defval.c_str(), xmlname.c_str()), isoptional);
}

rGeneratorMD::rItem& rGeneratorMD::rItem::addXml(const std::string& xmlname, UDINT defval, bool isoptional)
{
	return addXml(String_format("<%s>%u<%s/>", xmlname.c_str(), defval, xmlname.c_str()), isoptional);
}

rGeneratorMD::rItem& rGeneratorMD::rItem::addXml(const std::string& xmlname, LREAL defval, bool isoptional)
{
	return addXml(String_format("<%s>%g<%s/>", xmlname.c_str(), defval, xmlname.c_str()), isoptional);
}

std::string rGeneratorMD::rItem::save()
{
	std::string result = "";

	result += String_format("# %s\n## XML\n````xml\n", m_source->RTTI());
	result += String_format("<%s name=\"valid object name\" ", m_source->RTTI());

	for (auto& prop : m_properties) {
		switch (prop.m_type) {
			case ItemType::BITSFLAG:  result += String_format("%s=\"text's bits\" ", prop.m_name.c_str()); break;
			case ItemType::UDINT_VAL: result += String_format("%s=\"%u\" ", prop.m_name.c_str(), prop.m_intVal); break;
			case ItemType::LREAL_VAL: result += String_format("%s=\"%g\" ", prop.m_name.c_str(), prop.m_realVal); break;
			default: result += String_format("error_property "); break;
		}
	}
	result += ">\n";

	if (m_isStdInput) {
		result += m_source->getXmlInput();
	}

	for (auto& item : m_xml) {
		result += "\t" + item + "\n";
	}
	result += String_format("</%s>\n````\n", m_source->RTTI());

	for (auto& item : m_properties) {
		if (item.m_type == ItemType::BITSFLAG && item.m_bits) {
			result += item.m_bits->getMarkDown(item.m_name);
		}
	}

	result += m_source->getMarkDown();

	return result;
}
