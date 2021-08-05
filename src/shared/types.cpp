//=================================================================================================
//===
//=== types.cpp
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#include "types.h"

const std::string TYPENAME_UNDEF = "undef";
const std::string TYPENAME_USINT = "usint";
const std::string TYPENAME_SINT  = "sint";
const std::string TYPENAME_UINT  = "uint";
const std::string TYPENAME_INT   = "int";
const std::string TYPENAME_UDINT = "udint";
const std::string TYPENAME_DINT  = "dint";
const std::string TYPENAME_REAL  = "real";
const std::string TYPENAME_LREAL = "lreal";
const std::string TYPENAME_STRID = "strid";


UDINT getTypeSize(TYPE type)
{
	switch(type) {
		case TYPE::SINT:  return sizeof(SINT);
		case TYPE::USINT: return sizeof(USINT);
		case TYPE::INT:   return sizeof(INT);
		case TYPE::UINT:  return sizeof(UINT);
		case TYPE::DINT:  return sizeof(DINT);
		case TYPE::UDINT: return sizeof(UDINT);
		case TYPE::REAL:  return sizeof(REAL);
		case TYPE::LREAL: return sizeof(LREAL);
		case TYPE::STRID: return sizeof(UDINT);
		default: return 255;
	}
}

std::string getTypeName(TYPE type)
{
	switch(type) {
		case TYPE::SINT:  return TYPENAME_SINT;
		case TYPE::USINT: return TYPENAME_USINT;
		case TYPE::INT:   return TYPENAME_INT;
		case TYPE::UINT:  return TYPENAME_UINT;
		case TYPE::DINT:  return TYPENAME_DINT;
		case TYPE::UDINT: return TYPENAME_UDINT;
		case TYPE::REAL:  return TYPENAME_REAL;
		case TYPE::LREAL: return TYPENAME_LREAL;
		case TYPE::STRID: return TYPENAME_STRID;
		default: return TYPENAME_UNDEF;
	}
}

TYPE getTypeByName(const std::string& str)
{
	if (str == TYPENAME_USINT) return TYPE::USINT;
	if (str == TYPENAME_SINT)  return TYPE::SINT;
	if (str == TYPENAME_UINT)  return TYPE::UINT;
	if (str == TYPENAME_INT)   return TYPE::INT;
	if (str == TYPENAME_UDINT) return TYPE::UDINT;
	if (str == TYPENAME_DINT)  return TYPE::DINT;
	if (str == TYPENAME_REAL)  return TYPE::REAL;
	if (str == TYPENAME_LREAL) return TYPE::LREAL;
	if (str == TYPENAME_STRID) return TYPE::STRID;

	return TYPE::UNDEF;
}
