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

rGeneratorMD::rGeneratorMD()
{
}

rGeneratorMD::~rGeneratorMD()
{
}

rGeneratorMD::rItem* rGeneratorMD::add(const rSource* source, const std::string& name)
{
	m_items.push_back(rItem(source, name));

	return &m_items.back();
}

UDINT save(std::string path)
{
	return TRITONN_RESULT_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////

rGeneratorMD::rItem::rItem(const rSource* source, const std::string& name)
{
	m_source = source;
	m_name   = name;
}

rGeneratorMD::rItem::~rItem()
{
	;
}

rGeneratorMD::rItem* rGeneratorMD::rItem::addProperty(const std::string& name, const rBitsArray* bits)
{
	m_properties.push_back(rProperty());
	m_properties.back().m_name = name;
	m_properties.back().m_bits = bits;
	m_properties.back().m_type = ItemType::BITSFLAG;

	return this;
}

rGeneratorMD::rItem* rGeneratorMD::rItem::addProperty(const std::string& name, UDINT defval)
{
	m_properties.push_back(rProperty());
	m_properties.back().m_name   = name;
	m_properties.back().m_intVal = defval;
	m_properties.back().m_type   = ItemType::UDINT_VAL;

	return this;
}

rGeneratorMD::rItem* rGeneratorMD::rItem::addProperty(const std::string& name, LREAL defval)
{
	m_properties.push_back(rProperty());
	m_properties.back().m_name    = name;
	m_properties.back().m_realVal = defval;
	m_properties.back().m_type    = ItemType::LREAL_VAL;

	return this;
}

rGeneratorMD::rItem* rGeneratorMD::rItem::addXml(const std::string& xmlstring, bool isoptional = false)
{
	m_xml.push_back(xmlstring + (isoptional ? XML_OPTIONAL : ""));

	return this;
}

rGeneratorMD::rItem* rGeneratorMD::rItem::addXml(const std::string& xmlname, const std::string& defval, bool isoptional = false)
{
	return addXml(String_format("<%s>%s<%s/> %s", xmlname.c_str(), defval.c_str(), xmlname.c_str()), isoptional);
}

rGeneratorMD::rItem* rGeneratorMD::rItem::addXml(const std::string& xmlname, UDINT defval, bool isoptional = false)
{
	return addXml(String_format("<%s>%u<%s/>", xmlname.c_str(), defval, xmlname.c_str()), isoptional);
}

rGeneratorMD::rItem* rGeneratorMD::rItem::addXml(const std::string& xmlname, LREAL defval, bool isoptional = false)
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

	for (auto& item : m_xml) {
		result += item + "\n";
	}
	result += String_format("</%s>\n````\n", m_source->RTTI());

	for (auto& item : m_properties) {
		if (item.m_type == ItemType::BITSFLAG) {

		}
	}

	for (auto var : list) {
		if (var->isHide()) {
			continue;
		}

		result += var->saveKernel(Alias.size() + 1, "\t\t");
	}
	result += "\t</values>\n";

	// Входа
	if (m_inputs.size()) {

		result += "\t<inputs>\n";

		for (auto link : m_inputs) {
			UDINT shadow_count = 0;

			result += String_format("\t\t<input name=\"%s\" unit=\"%i\"", link->IO_Name.c_str(), (UDINT)link->Unit);

			for (auto sublink : m_inputs) {
				if (link == sublink) {
					continue;
				}

				if (sublink->Shadow == link->IO_Name) {
					if (0 == shadow_count) {
						result += ">\n";
					}

					result += String_format("\t\t\t<shadow name=\"%s\"/>\n", sublink->IO_Name.c_str());
					++shadow_count;
				}
			}

			if (0 == shadow_count) {
				if (link->Shadow.size()) {
					result += String_format(" shadow=\"%s\"", link->Shadow.c_str());
				}
				result += "/>\n";
			}
			else
			{
				result += "\t\t</input>\n";
			}
		}
		result += "\t</inputs>\n";
	}

	// Outputs
	if (m_outputs.size()) {
		result += "\t<outputs>\n";

		for (auto link : m_outputs) {
			result += String_format("\t\t<output name=\"%s\" unit=\"%i\"%s/>\n",
					link->IO_Name.c_str(), (UDINT)link->Unit, (link == m_outputs[0]) ? " default=\"1\"" : "");
		}

		result += "\t\t<output name=\"fault\" unit=\"512\"/>\n"
				  "\t</outputs>\n";
	}

	result += String_format("</%s>\n", Tag[isio ? 1 : 0].c_str());

	return result;
}
