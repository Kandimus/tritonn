﻿//=================================================================================================
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

#pragma once

#include <vector>
#include <string>
#include "def.h"

class rInterface;
class rSource;
class rBitsArray;
class rIOBaseModule;

//-------------------------------------------------------------------------------------------------
//
class rGeneratorMD
{
public:
	enum ItemType
	{
		BITSFLAG,
		UDINT_VAL,
		LREAL_VAL,
	};

	class rItem
	{
		struct rProperty
		{
			std::string m_name;
			ItemType    m_type;
			const rBitsArray* m_bits;
			UDINT       m_intVal;
			LREAL       m_realVal;
		};

	public:
		rItem(rSource* source, bool isstdinput);
		rItem(rIOBaseModule* module);
		virtual ~rItem() = default;

		rItem& addProperty(const std::string& name, const rBitsArray* bits);
		rItem& addProperty(const std::string& name, UDINT defval);
		rItem& addProperty(const std::string& name, LREAL defval);
		rItem& addXml(const std::string& xmlstring, bool isoptional = false);
		rItem& addXml(const std::string& xmlname, const std::string& defval, bool isoptional = false, const std::string& prefix = "");
		rItem& addXml(const std::string& xmlname, UDINT defval, bool isoptional = false, const std::string& prefix = "");
		rItem& addXml(const std::string& xmlname, LREAL defval, bool isoptional = false, const std::string& prefix = "");
		rItem& addLink(const std::string& xmlname, bool isoptional = false, const std::string& prefix = "");

		bool        isModule() const { return m_module != nullptr; }
		std::string getName() const { return m_name; }
		std::string save();

	public:
		static const char* XML_OPTIONAL;
		static const char* XML_LINK;

	protected:
		rSource*       m_source = nullptr;
		rIOBaseModule* m_module = nullptr;
		std::string    m_name;
		bool        m_isStdInput;
		std::vector<rProperty>   m_properties;
		std::vector<std::string> m_xml;
	};

public:
	rGeneratorMD();
	virtual ~rGeneratorMD();

	rGeneratorMD::rItem& add(rSource* source, bool isstdinput);
	rGeneratorMD::rItem& add(rIOBaseModule* module);

	UDINT save(const std::string& path);

protected:
	std::vector<rItem> m_items;
	std::string m_path;

protected:
	UDINT save_index(const std::string& path);
};

