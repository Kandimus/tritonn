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

#include "tritonn_version.h"
#include "generator_md.h"
#include "data_source.h"
#include "bits_array.h"
#include "simplefile.h"
#include "io/basemodule.h"
#include "interface/interface.h"
#include "xml_util.h"

const std::string rGeneratorMD::rItem::XML_OPTIONAL = "<!-- Optional -->";
const std::string rGeneratorMD::rItem::XML_LINK     = "<link alias=\"object's output\"/>";
const std::string rGeneratorMD::rItem::CONTENTS     = "<p align='right'><a href='index.html'>[Оглавление]</a></p>\n\n";
const std::string rGeneratorMD::rItem::TEXT_BITS    = "text value | text value | ... | text value";
const std::string rGeneratorMD::rItem::TEXT_NUMBER  = "text value";

rGeneratorMD::rGeneratorMD()
{
}

rGeneratorMD::~rGeneratorMD()
{
}

rGeneratorMD::rItem& rGeneratorMD::add(const std::string& name)
{
	m_items.push_back(rItem(name));

	return m_items.back();
}

rGeneratorMD::rItem& rGeneratorMD::add(rSource* source, bool isstdinput, Type type)
{
	m_items.push_back(rItem(source, isstdinput, type));

	return m_items.back();
}

rGeneratorMD::rItem& rGeneratorMD::add(rIOBaseModule* module)
{
	m_items.push_back(rItem(module));

	return m_items.back();
}

rGeneratorMD::rItem& rGeneratorMD::add(rInterface* interface)
{
	m_items.push_back(rItem(interface));

	return m_items.back();
}

UDINT rGeneratorMD::save(const std::string& path)
{
	UDINT result = TRITONN_RESULT_OK;

	result = save_index(path);
	if (result != TRITONN_RESULT_OK) {
		return result;
	}

	for (auto& item : m_items) {
		result = SimpleFileSave(path + "/" + item.getFilename() + ".md", item.save());

		if (result != TRITONN_RESULT_OK) {
			return result;
		}
	}

	return TRITONN_RESULT_OK;
}

UDINT rGeneratorMD::save_index(const std::string& path)
{
	std::string text = "<!DOCTYPE html>\n<html><head><meta charset=\"UTF-8\"/>"
					   "<title>Tritonn help</title></head><body>"
					   "<h1>Tritonn help</h1><bsp/><h4><i>ver " + std::string(TRITONN_VERSION) + "</i></h4>\n";

	text += "<hr align=\"left\">";
	text += "<style type='text/css'> TABLE { border-collapse : collapse;} TD { vertical-align: top; }</style>";
	text += "<table width=75% cellspacing=3px padding=5px><tr>\n";
	text += "<td><h3>Hardware</h3>\n";
	for (auto& item : m_items) {
		if (item.isHarware()) {
			text += "<a href=\"" + item.getFilename() + ".md\">" + item.getFilename() + "</a><br>\n";
		}
	}
	text += "</td>\n";

	text += "<td><h3>IO objects</h3>\n";
	for (auto& item : m_items) {
		if (item.isIO()) {
			text += "<a href=\"" + item.getFilename() + ".md\">" + item.getFilename() + "</a><br>\n";
		}
	}
	text += "</td>\n";

	text += "<td><h3>Calculate objects</h3>\n";
	for (auto& item : m_items) {
		if (item.isCalculate()) {
			text += "<a href=\"" + item.getFilename() + ".md\">" + item.getFilename() + "</a><br>\n";
		}
	}
	text += "</td>\n";

	text += "<td><h3>Interfaces</h3>\n";
	for (auto& item : m_items) {
		if (item.isInterface()) {
			text += "<a href=\"" + item.getFilename() + ".md\">" + item.getFilename() + "</a><br>\n";
		}
	}
	text += "</td>\n";

	text += "<td><h3>Other</h3>\n";
	for (auto& item : m_items) {
		if (item.isCustom() || item.isReport()) {
			text += "<a href=\"" + item.getFilename() + ".md\">" + item.getFilename() + "</a><br>\n";
		}
	}
	text += "</td></tr></table>\n";

	text += "</body></html>";

	return SimpleFileSave(path + "/index.html", text);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////

rGeneratorMD::rItem::rItem(const std::string& name)
{
	m_name     = name;
	m_filename = m_name;
	m_type     = Type::CUSTOM;
}

rGeneratorMD::rItem::rItem(rSource* source, bool isstdinput, Type type)
{
	m_source     = source;
	m_name       = source->RTTI();
	m_filename   = m_name;
	m_isStdInput = isstdinput;
	m_type       = type;

	if (isHarware() || isInterface()) {
		m_type = Type::CALCULATE;
	}
}

rGeneratorMD::rItem::rItem(rIOBaseModule* module)
{
	m_module   = module;
	m_name     = module->getName();
	m_filename = m_name;
	m_type     = Type::HARWARE;
}

rGeneratorMD::rItem::rItem(rInterface* interface)
{
	m_interface = interface;
	m_name      = interface->getRTTI();
	m_filename  = m_name;
	m_type      = Type::INTERFACE;
}

rGeneratorMD::rItem& rGeneratorMD::rItem::addProperty(const std::string& name, const rBitsArray* bits, bool isnumber)
{
	m_properties.push_back(rProperty());
	m_properties.back().m_name     = name;
	m_properties.back().m_bits     = bits;
	m_properties.back().m_type     = ItemType::BITSFLAG;
	m_properties.back().m_isNumber = isnumber;

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
	m_xml.push_back(xmlstring + (isoptional ? " " + XML_OPTIONAL : ""));

	return *this;
}

rGeneratorMD::rItem& rGeneratorMD::rItem::addXml(const std::string& xmlname, const std::string& defval, bool isoptional, const std::string& prefix)
{
	return addXml(prefix + "<" + xmlname + ">" + defval + "<" + xmlname + "/>", isoptional);
}

rGeneratorMD::rItem& rGeneratorMD::rItem::addXml(const std::string& xmlname, UDINT defval, bool isoptional, const std::string& prefix)
{
	return addXml(String_format("%s<%s>%u<%s/>", prefix.c_str(), xmlname.c_str(), defval, xmlname.c_str()), isoptional);
}

rGeneratorMD::rItem& rGeneratorMD::rItem::addXml(const std::string& xmlname, LREAL defval, bool isoptional, const std::string& prefix)
{
	return addXml(String_format("%s<%s>%g</%s>", prefix.c_str(), xmlname.c_str(), defval, xmlname.c_str()), isoptional);
}

rGeneratorMD::rItem& rGeneratorMD::rItem::addLink(const std::string& xmlname, bool isoptional, const std::string& prefix)
{
	return addXml(prefix + "<" + xmlname + ">" + XML_LINK + "<" + xmlname + "/>", isoptional);
}

rGeneratorMD::rItem& rGeneratorMD::rItem::addRemark(const std::string& remark)
{
	if (m_remark.size()) {
		m_remark += "<br/>";
	}

	m_remark += remark;

	return *this;
}

rGeneratorMD::rItem& rGeneratorMD::rItem::setFilename(const std::string& filename)
{
	m_filename = filename;

	return *this;
}

std::string rGeneratorMD::rItem::save()
{
	std::string result = "";

	result += CONTENTS;
	result += "# " + m_name + "\n";
	result += "> " + std::string(TRITONN_VERSION) + "\n";

	if (isCustom()) {
		result += m_remark;
		result += "\n" + CONTENTS;
		return result;
	}

	result += "## XML\n````xml\n";

	if (isHarware()) {
		result += "<" + std::string(XmlName::MODULE) + " name=\"" + m_name + "\" ";
	} else {
		result += "<" + m_name + " name=\"valid object name\" ";
	}
	result += std::string(XmlName::DESC) + "=\"string index\" ";

	for (auto& prop : m_properties) {
		switch (prop.m_type) {
			case ItemType::BITSFLAG:  result += prop.m_name + "=\"" + (prop.m_isNumber ? TEXT_NUMBER : TEXT_BITS) + "\" "; break;
			case ItemType::UDINT_VAL: result += String_format("%s=\"%u\" ", prop.m_name.c_str(), prop.m_intVal); break;
			case ItemType::LREAL_VAL: result += String_format("%s=\"%g\" ", prop.m_name.c_str(), prop.m_realVal); break;
			default: result += String_format("error_property "); break;
		}
	}
	result += ">\n";

	if (isHarware()) {
		result += m_module->getXmlChannels();
	} else if (isInterface()) {
		;
	} else {
		if (isIO()) {
			result += "\t<io_link module=\"module index\"";
			result += m_type == Type::IOMDULE ? "" : " channel=\"channel index\"";
			result += "/>";

			if (m_type == Type::IOCHANNEL_OPT) {
				result += " " + XML_OPTIONAL;
			}

			result += "\n";
		}

		if (m_isStdInput) {
			result += m_source->getXmlInput();
		}
	}

	for (auto& item : m_xml) {
		result += "\t" + item + "\n";
	}

	if (isHarware()) {
		result += "</" + std::string(XmlName::MODULE) + ">\n````\n";
	} else {
		result += "</" + m_name + ">\n````\n";
	}

	for (auto& item : m_properties) {
		if (item.m_type == ItemType::BITSFLAG && item.m_bits) {
			result += item.m_bits->getMarkDown(item.m_name);
		}
	}

	if (isHarware()) {
		result += m_module->getMarkDown();
	} else if (isInterface()){
		result += m_interface->getMarkDown();
	} else {
		result += m_source->getMarkDown();
	}

	int pos = result.find("[^mutable]");
	if (pos >= 0) {
		m_remark += "\n[^mutable]: Если объект не привязан к модулю ввода-вывода, то данная переменная будет записываемой.\n";
	}


	if (m_remark.size()) {
		result += /*"> "*/"\n" + m_remark + "\n";
	}

	result += "\n" + CONTENTS;

	return result;
}
