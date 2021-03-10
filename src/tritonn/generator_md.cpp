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

const char* rGeneratorMD::rItem::XML_OPTIONAL = "<!-- Optional -->";
const char* rGeneratorMD::rItem::XML_LINK     = "<link alias=\"object's output\"/>";
const char* rGeneratorMD::rItem::CONTENTS     = "<p align='right'><a href='index.html'>[Оглавление]</a></p>\n\n";

rGeneratorMD::rGeneratorMD()
{
}

rGeneratorMD::~rGeneratorMD()
{
}

rGeneratorMD::rItem& rGeneratorMD::add(rSource* source, bool isstdinput)
{
	m_items.push_back(rItem(source, isstdinput));


	return m_items.back();
}

rGeneratorMD::rItem& rGeneratorMD::add(rIOBaseModule* module)
{
	m_items.push_back(rItem(module));

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
		result = SimpleFileSave(path + "/" + item.getName() + ".md", item.save());
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
					   "<h1>Tritonn help</h1><bsp/><h2>ver " + std::string(TRITONN_VERSION) + "</h2>\n";

	text += "<hr align=\"left\">";
	text += "<br/><h3>Hardware</h3>\n";
	for (auto& item : m_items) {
		if (item.isModule()) {
			text += "<a href=\"" + item.getName() + ".md\">" + item.getName() + "</a><br>\n";
		}
	}

	text += "<br/><h3>Source</h3>\n";
	for (auto& item : m_items) {
		if (!item.isModule()) {
			text += "<a href=\"" + item.getName() + ".md\">" + item.getName() + "</a><br>\n";
		}
	}

	text += "</body></html>";

	return SimpleFileSave(path + "/index.html", text);
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

rGeneratorMD::rItem::rItem(rIOBaseModule* module)
{
	m_module = module;
	m_name   = module->getName();
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
	m_xml.push_back(xmlstring + (isoptional ? string(" ") + XML_OPTIONAL : ""));

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
	return addXml(String_format("%s<%s>%g<%s/>", prefix.c_str(), xmlname.c_str(), defval, xmlname.c_str()), isoptional);
}

rGeneratorMD::rItem& rGeneratorMD::rItem::addLink(const std::string& xmlname, bool isoptional, const std::string& prefix)
{
	return addXml(prefix + "<" + xmlname + ">" + XML_LINK + "<" + xmlname + "/>", isoptional);
}

rGeneratorMD::rItem& rGeneratorMD::rItem::addIOLink(bool onlymodule, bool isoptional, const std::string& prefix)
{
	return addXml("<io_link module=\"module index\"" + std::string(onlymodule ? "" : " channel=\"channel index\"") + "/>", isoptional);
}

rGeneratorMD::rItem& rGeneratorMD::rItem::addRemark(const std::string& remark)
{
	if (m_remark.size()) {
		m_remark += "<br/>";
	}

	m_remark += remark;

	return *this;
}

std::string rGeneratorMD::rItem::save()
{
	std::string result = "";

	result += CONTENTS;
	result += "# " + m_name + "\n";
	result += ">" + std::string(TRITONN_VERSION) + "\n";
	result += "## XML\n````xml\n";
	result += "<" + m_name + " name=\"valid object name\" descr=\"string index\" ";

	for (auto& prop : m_properties) {
		switch (prop.m_type) {
			case ItemType::BITSFLAG:  result += String_format("%s=\"text's bits\" ", prop.m_name.c_str()); break;
			case ItemType::UDINT_VAL: result += String_format("%s=\"%u\" ", prop.m_name.c_str(), prop.m_intVal); break;
			case ItemType::LREAL_VAL: result += String_format("%s=\"%g\" ", prop.m_name.c_str(), prop.m_realVal); break;
			default: result += String_format("error_property "); break;
		}
	}
	result += ">\n";

	if (!isModule()) {
		if (m_isStdInput) {
			result += m_source->getXmlInput();
		}
	} else {
		result += m_module->getXmlChannels();
	}

	for (auto& item : m_xml) {
		result += "\t" + item + "\n";
	}
	result += "</" + m_name + ">\n````\n";

	for (auto& item : m_properties) {
		if (item.m_type == ItemType::BITSFLAG && item.m_bits) {
			result += item.m_bits->getMarkDown(item.m_name);
		}
	}

	result += isModule() ? m_module->getMarkDown() : m_source->getMarkDown();

	if (result.find("[^mutable]") >= 0) {
		m_remark += "\n[^mutable]: Если объект не привязан к модулю ввода-вывода, то данная переменная будет записываемой.\n";
	}


	if (m_remark.size()) {
		result += /*"> "*/"\n" + m_remark + "\n";
	}

	result += "\n" + std::string(CONTENTS);

	return result;
}
