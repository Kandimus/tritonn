//=================================================================================================
//===
//=== xml_util.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Вспомогательные функции для работы с XML
//===
//=================================================================================================

#include "xml_util.h"

namespace XmlUtils
{


//-------------------------------------------------------------------------------------------------
//
UDINT getAttributeUDINT(tinyxml2::XMLElement *element, const std::string &name, UDINT def)
{
	return (element->Attribute(name.c_str()) ) ? strtoul(element->Attribute(name.c_str()), NULL, 0)  : def;
}

DINT getAttributeDINT(tinyxml2::XMLElement *element, const std::string &name, const DINT def)
{
	return (element->Attribute(name.c_str()) ) ? strtol(element->Attribute(name.c_str()), NULL, 0) : def;
}

INT getAttributeINT(tinyxml2::XMLElement *element, const std::string &name, const INT def)
{
	return static_cast<INT>(getAttributeDINT(element, name, def));
}

USINT getAttributeUSINT (tinyxml2::XMLElement *element, const std::string &name, const USINT def)
{
	return static_cast<USINT>(getAttributeUDINT(element, name, def));
}


//-------------------------------------------------------------------------------------------------
//
std::string getAttributeString(tinyxml2::XMLElement *element, const std::string &name, const string &def)
{
	return (element->Attribute(name.c_str()) ) ? element->Attribute(name.c_str())  : def;
}


//-------------------------------------------------------------------------------------------------
//
std::string getTextString(tinyxml2::XMLElement *element, const std::string &def, UDINT &err)
{
	std::string result = def;

	if(nullptr == element)
	{
		err = 1;
		return result;
	}

	if(nullptr == element->GetText())
	{
		err = 1;
		return result;
	}

	result = element->GetText();
	return result;
}


//-------------------------------------------------------------------------------------------------
//
LREAL getTextLREAL(tinyxml2::XMLElement *element, LREAL def, UDINT &err)
{
	LREAL result = def;

	if(nullptr == element)
	{
		err = 1;
		return result;
	}

	if(tinyxml2::XML_SUCCESS != element->QueryDoubleText(&result))
	{
		err = 1;
		return result;
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
//
UDINT getTextUDINT(tinyxml2::XMLElement *element, UDINT def, UDINT &err)
{
	UDINT result = def;

	if(nullptr == element)
	{
		err = 1;
		return result;
	}

	if(tinyxml2::XML_SUCCESS != element->QueryIntText((int *)&result))
	{
		err = 1;
		return result;
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
//
UINT getTextUINT(tinyxml2::XMLElement *element, UINT def, UDINT &err)
{
	USINT result = def;
	UDINT value  = getTextUDINT(element, (UDINT)def, err);

	if(err) return result;

	if(value > 0xFFFF)
	{
		err = 1;
		return result;
	}

	result = value;

	return result;
}


//-------------------------------------------------------------------------------------------------
//
USINT getTextUSINT(tinyxml2::XMLElement *element, USINT def, UDINT &err)
{
	USINT result = def;
	UDINT value  = getTextUDINT(element, (UDINT)def, err);

	if(err) return result;

	if(value > 0xFF)
	{
		err = 1;
		return result;
	}

	result = value;

	return result;
}


USINT getTextBOOL(tinyxml2::XMLElement *element, bool def, UDINT &err)
{
	bool  result = def;
	UDINT value  = getTextUDINT(element, (UDINT)def, err);

	if(err) return result;

	if(value > 1)
	{
		err = 1;
		return result;
	}

	result = value == true;

	return result;
}


}

