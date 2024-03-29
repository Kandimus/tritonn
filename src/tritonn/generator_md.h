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

	enum class Type
	{
		HARWARE = 0,
		INTERFACE,
		REPORT,
		CALCULATE,
		IOCHANNEL,
		IOMDULE,
		IOCHANNEL_OPT,
		CUSTOM,
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
			bool        m_isNumber;
		};

	public:
		rItem(const std::string& name);
		rItem(rSource* source, bool isstdinput, Type type);
		rItem(rIOBaseModule* module);
		rItem(rInterface* interface);
		virtual ~rItem() = default;

		rItem& addProperty(const std::string& name, const rBitsArray* bits, bool isnumber = false);
		rItem& addProperty(const std::string& name, UDINT defval);
		rItem& addProperty(const std::string& name, LREAL defval);
		rItem& addXml(const std::string& xmlstring, bool isoptional = false);
		rItem& addXml(const std::string& xmlname, const std::string& defval, bool isoptional = false, const std::string& prefix = "");
		rItem& addXml(const std::string& xmlname, UDINT defval, bool isoptional = false, const std::string& prefix = "");
		rItem& addXml(const std::string& xmlname, LREAL defval, bool isoptional = false, const std::string& prefix = "");
		rItem& addLink(const std::string& xmlname, bool isoptional = false, const std::string& prefix = "");
		rItem& addRemark(const std::string& remark);
		rItem& setFilename(const std::string& filename);

		Type        getType() const { return m_type; }
		bool        isCustom() const { return m_type == Type::CUSTOM; }
		bool        isReport() const { return m_type == Type::REPORT; }
		bool        isInterface() const { return m_type == Type::INTERFACE; }
		bool        isCalculate() const { return m_type == Type::CALCULATE; }
		bool        isSource() const { return isIO() || isCalculate() || isReport(); }
		bool        isIO() const { return m_type == Type::IOCHANNEL || m_type == Type::IOMDULE || m_type == Type::IOCHANNEL_OPT; }
		bool        isHarware() const { return m_type == Type::HARWARE; }
		std::string getName() const { return m_name; }
		std::string getFilename() const { return m_filename; }
		std::string save();

	public:
		static const std::string XML_OPTIONAL;
		static const std::string XML_LINK;
		static const std::string CONTENTS;
		static const std::string TEXT_BITS;
		static const std::string TEXT_NUMBER;

	protected:
		rSource*       m_source     = nullptr;
		rIOBaseModule* m_module     = nullptr;
		rInterface*    m_interface  = nullptr;
		std::string    m_name       = "";
		std::string    m_filename   = "";
		bool           m_isStdInput = true;
		Type           m_type       = Type::CALCULATE;
		std::string    m_remark;
		std::vector<rProperty>   m_properties;
		std::vector<std::string> m_xml;
	};

public:
	rGeneratorMD();
	virtual ~rGeneratorMD();

	rItem& add(const std::string& name);
	rItem& add(rSource* source, bool isstdinput, Type type);
	rItem& add(rIOBaseModule* module);
	rItem& add(rInterface* interface);

	UDINT save(const std::string& path);

protected:
	std::vector<rItem> m_items;
	std::string m_path;

protected:
	UDINT save_index(const std::string& path);
};

