//=================================================================================================
//===
//=== data_snapshot_item.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//===
//===
//=================================================================================================

#include "data_snapshot_item.h"
#include <string.h>
#include "stringex.h"
#include "variable_item.h"


#define SNAPSHOT_ASSIGN		{ \
								if (!var) \
								{ \
									m_var    = nullptr; \
									m_status = Status::NOTFOUND; \
									return; \
								} \
								m_var = var; \
								m_status = Status::TOWRITE; \
								try { \
									switch (var->getType()) \
									{ \
										case TYPE_USINT: *(USINT *)m_data = static_cast<USINT>(val); return; \
										case TYPE_SINT : *(SINT  *)m_data = static_cast< SINT>(val); return; \
										case TYPE_UINT : *(UINT  *)m_data = static_cast< UINT>(val); return; \
										case TYPE_INT  : *(INT   *)m_data = static_cast<  INT>(val); return; \
										case TYPE_UDINT: *(UDINT *)m_data = static_cast<UDINT>(val); return; \
										case TYPE_DINT : *(DINT  *)m_data = static_cast< DINT>(val); return; \
										case TYPE_REAL : *(REAL  *)m_data = static_cast< REAL>(val); return; \
										case TYPE_LREAL: *(LREAL *)m_data = static_cast<LREAL>(val); return; \
										case TYPE_STRID: *(STRID *)m_data = STRID(static_cast<UDINT>(val)); return; \
										default: m_status = Status::ERROR; clearData(); return; \
									} \
								} catch(...) { \
									m_status = Status::ERROR; \
									clearData(); \
								} \
							}

#define SNAPSHOT_GET(x)		{ \
								if ((m_status != Status::ASSIGNED && m_status != Status::WRITED) || nullptr == m_var) return x(0); \
								try { \
									switch (m_var->getType()) { \
										case TYPE_USINT: return x(*(USINT *)m_data); \
										case TYPE_SINT : return x(*(SINT  *)m_data); \
										case TYPE_UINT : return x(*(UINT  *)m_data); \
										case TYPE_INT  : return x(*(INT   *)m_data); \
										case TYPE_UDINT: return x(*(UDINT *)m_data); \
										case TYPE_DINT : return x(*(DINT  *)m_data); \
										case TYPE_REAL : return static_cast<x>(*(REAL  *)m_data); \
										case TYPE_LREAL: return static_cast<x>(*(LREAL *)m_data); \
										case TYPE_STRID: return x(*(STRID *)m_data); \
										default: m_status = Status::ERROR; return static_cast<x>(0); \
									} \
								} catch(...) { \
									m_status = Status::ERROR; return static_cast<x>(0); \
								} \
							}




rSnapshotItem::rSnapshotItem(const rSnapshotItem& snapshot)
{
	*this = snapshot;
}

rSnapshotItem::rSnapshotItem(const rVariable* var)
{
	m_var    = var;
	m_status = Status::TOASSIGN;
}

rSnapshotItem::rSnapshotItem(const rVariable* var, SINT  val) { SNAPSHOT_ASSIGN }
rSnapshotItem::rSnapshotItem(const rVariable* var, USINT val) { SNAPSHOT_ASSIGN }
rSnapshotItem::rSnapshotItem(const rVariable* var, INT   val) { SNAPSHOT_ASSIGN }
rSnapshotItem::rSnapshotItem(const rVariable* var, UINT  val) { SNAPSHOT_ASSIGN }
rSnapshotItem::rSnapshotItem(const rVariable* var, DINT  val) { SNAPSHOT_ASSIGN }
rSnapshotItem::rSnapshotItem(const rVariable* var, UDINT val) { SNAPSHOT_ASSIGN }
rSnapshotItem::rSnapshotItem(const rVariable* var, REAL  val) { SNAPSHOT_ASSIGN }
rSnapshotItem::rSnapshotItem(const rVariable* var, LREAL val) { SNAPSHOT_ASSIGN }
rSnapshotItem::rSnapshotItem(const rVariable* var, STRID val) { SNAPSHOT_ASSIGN }

rSnapshotItem::rSnapshotItem(const rVariable* var, const string &val)
{
	if(!var) return;

	m_var    = var;
	m_status = Status::TOWRITE;

	try {
		// Проверка на 16-тиричное число
		if (val.size() >= 3) {

			if (val[0] == '0' && (val[1] == 'x' || val[1] == 'X')) {
				UDINT number = 0;
				DINT *ptrnum = reinterpret_cast<DINT*>(&number);

				if (String_IsValidHex(val.c_str() + 2, number)) {
					switch (m_var->getType())
					{
						//TODO наверно нужно сделать это через memcpy
						case TYPE_USINT: *(USINT *)m_data = static_cast<USINT>( number); break;
						case TYPE_SINT : *(SINT  *)m_data = static_cast< SINT>(*ptrnum); break;
						case TYPE_UINT : *(UINT  *)m_data = static_cast< UINT>( number); break;
						case TYPE_INT  : *(INT   *)m_data = static_cast<  INT>(*ptrnum); break;
						case TYPE_UDINT: *(UDINT *)m_data =                     number ; break;
						case TYPE_DINT : *(DINT  *)m_data =                    *ptrnum ; break;
						case TYPE_REAL : *(REAL  *)m_data = static_cast< REAL>( number); break;
						case TYPE_LREAL: *(LREAL *)m_data = static_cast<LREAL>( number); break;
						case TYPE_STRID: *(STRID *)m_data = static_cast<STRID>( number); break;
						default:
							m_status = Status::ERROR;
							clearData();
							break;
					}
				}
				return;
			}
		}

		switch (m_var->getType())
		{
			case TYPE_USINT: *(USINT *)m_data = static_cast<USINT>(std::stoul(val.c_str())); break;
			case TYPE_SINT : *(SINT  *)m_data = static_cast< SINT>(std::stol (val.c_str())); break;\
			case TYPE_UINT : *(UINT  *)m_data = static_cast< UINT>(std::stoul(val.c_str())); break;\
			case TYPE_INT  : *(INT   *)m_data = static_cast<  INT>(std::stol (val.c_str())); break;\
			case TYPE_UDINT: *(UDINT *)m_data = static_cast<UDINT>(std::stoul(val.c_str())); break;\
			case TYPE_DINT : *(DINT  *)m_data = static_cast< DINT>(std::stol (val.c_str())); break;\
			case TYPE_REAL : *(REAL  *)m_data = static_cast< REAL>(std::stof (val.c_str())); break;\
			case TYPE_LREAL: *(LREAL *)m_data = static_cast<LREAL>(std::stod (val.c_str())); break;\
			case TYPE_STRID: *(STRID *)m_data = static_cast<STRID>(std::stoul(val.c_str())); break;\
			default:
				m_status = Status::ERROR;
				clearData();
				break;
		}
	}
	catch(...)
	{
		m_status = Status::ERROR;
		clearData();
	}
}


rSnapshotItem::rSnapshotItem(const rVariable* var, void *buf)
{
	if (!var) {
		return;
	}

	m_var    = var;
	m_status = Status::TOWRITE;

	memcpy(m_data, buf, EPT_SIZE[m_var->getType()]);
}



rSnapshotItem::~rSnapshotItem()
{
	;
}


SINT  rSnapshotItem::getValueSINT()  {SNAPSHOT_GET( SINT);}
USINT rSnapshotItem::getValueUSINT() {SNAPSHOT_GET(USINT);}
INT   rSnapshotItem::getValueINT()   {SNAPSHOT_GET(  INT);}
UINT  rSnapshotItem::getValueUINT()  {SNAPSHOT_GET( UINT);}
DINT  rSnapshotItem::getValueDINT()  {SNAPSHOT_GET( DINT);}
UDINT rSnapshotItem::getValueUDINT() {SNAPSHOT_GET(UDINT);}
REAL  rSnapshotItem::getValueREAL()  {SNAPSHOT_GET( REAL);}
LREAL rSnapshotItem::getValueLREAL() {SNAPSHOT_GET(LREAL);}
STRID rSnapshotItem::getValueSTRID() {SNAPSHOT_GET(STRID);}

std::string rSnapshotItem::getValueString()
{
	if ((m_status != Status::ASSIGNED && m_status != Status::WRITED) || m_var == nullptr) {
		return "";
	}

	try
	{
		switch(m_var->getType())
		{
			case TYPE_USINT: return String_format("%hhu", *(USINT *)m_data); break;
			case TYPE_SINT : return String_format("%hhi", *(SINT  *)m_data); break;
			case TYPE_UINT : return String_format("%hu" , *(UINT  *)m_data); break;
			case TYPE_INT  : return String_format("%hi" , *(INT   *)m_data); break;
			case TYPE_UDINT: return String_format("%u"  , *(UDINT *)m_data); break;
			case TYPE_DINT : return String_format("%i"  , *(DINT  *)m_data); break;
			case TYPE_REAL : return String_format("%#g" , *(REAL  *)m_data); break;
			case TYPE_LREAL: return String_format("%#g" , *(LREAL *)m_data); break;
			case TYPE_STRID: return String_format("%u"  , *(UDINT *)m_data); break;
			default:
				m_status = Status::ERROR;
				return "";
		}
	}
	catch(...)
	{
		m_status = Status::ERROR;
		return "";
	}
}

bool rSnapshotItem::getBuffer(void *buffer) const
{
	if (!isAssigned()/* || !isWrited() || !isToWrite()*/) {
		return false;
	}

	memcpy(buffer, m_data, getSizeVar());
	return true;
}

//
void rSnapshotItem::clearData()
{
	memset(m_data, 0, 8);
}

UDINT rSnapshotItem::getSizeVar() const
{
	return (m_var) ? EPT_SIZE[m_var->getType()] : 0;
}


void rSnapshotItem::reset()
{
	m_status = Status::UNDEF;
}

void rSnapshotItem::toAssign()
{
	m_status = Status::TOASSIGN;
}
