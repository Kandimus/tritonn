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

//-------------------------------------------------------------------------------------------------
//
class rGeneratorMD
{
public:
	enum ItemType{
		BITSFLAG,
		UDINT_VAL,
		LREAL_VAL,
	};

	class rItem
	{
		struct rProperty
		{
			std::string       m_name;
			const rBitsArray* m_bits;
		};

	public:
		rItem(const rSource* source, const std::string& name);
		virtual ~rItem();

		rItem* addProperty(const std::string& name, const rBitsArray* bits);
		rItem* addProperty(const std::string& name, UDINT defval);
		rItem* addProperty(const std::string& name, LREAL defval);
		rItem* addXml(const std::string& xml);

	protected:
		UDINT save(const std::string& filename);

	protected:
		const rSource* m_source;
		std::string    m_name;
		std::vector<rProperty>   m_properties;
		std::vector<std::string> m_xml;
	};

public:
	rGeneratorMD(std::string path);
	virtual ~rGeneratorMD();

	rGeneratorMD::rItem* add(const rSource* source, const std::string& name);


	UDINT save();

protected:
	std::vector<rItem> m_items;
};

