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
		CALCULATE,
		IOCHANNEL,
		IOMDULE,
		IOCHANNEL_OPT,
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
		rItem(rSource* source, bool isstdinput, Type type);
		rItem(rIOBaseModule* module);
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

		Type        getType() const { return m_type; }
		bool        isCalculate() const { return m_type == Type::CALCULATE; }
		bool        isIO() const { return m_type == Type::IOCHANNEL || m_type == Type::IOMDULE || m_type == Type::IOCHANNEL_OPT; }
		bool        isHarware() const { return m_type == Type::HARWARE; }
		std::string getName() const { return m_name; }
		std::string save();

	public:
		static const char* XML_OPTIONAL;
		static const char* XML_LINK;
		static const char* CONTENTS;
		static const char* TEXT_BITS;
		static const char* TEXT_NUMBER;

	protected:
		rSource*       m_source = nullptr;
		rIOBaseModule* m_module = nullptr;
		std::string    m_name;
		bool           m_isStdInput = true;
		Type           m_type = Type::CALCULATE;
		std::string    m_remark;
		std::vector<rProperty>   m_properties;
		std::vector<std::string> m_xml;
	};

public:
	rGeneratorMD();
	virtual ~rGeneratorMD();

	rGeneratorMD::rItem& add(rSource* source, bool isstdinput, Type type);
	rGeneratorMD::rItem& add(rIOBaseModule* module);

	UDINT save(const std::string& path);

protected:
	std::vector<rItem> m_items;
	std::string m_path;

protected:
	UDINT save_index(const std::string& path);
};

