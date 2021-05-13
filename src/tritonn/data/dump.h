﻿//=================================================================================================
//===
//=== data/dump.h
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#include <string>
#include "def.h"
#include "tinyxml2.h"

class rDumpFile
{
public:
	rDumpFile() = default;
	virtual ~rDumpFile() = default;

	std::string m_prefix = "";
	std::string m_suffix = "";

	tinyxml2::XMLDocument m_xmlDoc;

protected:
	std::string m_filename;
	UDINT       m_result = TRITONN_RESULT_OK;

public:
	tinyxml2::XMLElement* xmlRoot() { return m_xmlDoc.RootElement(); }
	void  xmlClear() { m_xmlDoc.Clear(); }
	UDINT checkFile(const std::string& filename, const std::string& hash);
	UDINT getResult() const { return m_result; }
	std::string getStrFilename() const { return m_filename; }
	const char* getFilename() const { return m_filename.c_str(); }
};