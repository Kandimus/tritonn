//=================================================================================================
//===
//=== data_module.cpp
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

#include <string.h>
#include "data_snapshot.h"
#include "stringex.h"


#define SNAPSHOT_ASSIGN      {if(FindVariable(name)) return; Status = SS_STATUS_ASSIGN; try{switch(Variable->Type){\
										case TYPE_USINT: *(USINT *)Data = USINT(val); return;\
										case TYPE_SINT : *(SINT  *)Data =  SINT(val); return;\
										case TYPE_UINT : *(UINT  *)Data =  UINT(val); return;\
										case TYPE_INT  : *(INT   *)Data =   INT(val); return;\
										case TYPE_UDINT: *(UDINT *)Data = UDINT(val); return;\
										case TYPE_DINT : *(DINT  *)Data =  DINT(val); return;\
										case TYPE_REAL : *(REAL  *)Data =  REAL(val); return;\
										case TYPE_LREAL: *(LREAL *)Data = LREAL(val); return;\
										case TYPE_STRID: *(STRID *)Data = STRID(val); return;\
										default: Status = SS_STATUS_NOTASSIGN; ClearData();return;}}catch(...){Status = SS_STATUS_NOTASSIGN;ClearData();}}

#define SNAPSHOT_GET(x)      {if((Status != SS_STATUS_ASSIGN && Status != SS_STATUS_WRITED) || nullptr == Variable) return x(0);\
										try{switch(Variable->Type){\
										case TYPE_USINT: return x(*(USINT *)Data);\
										case TYPE_SINT : return x(*(SINT  *)Data);\
										case TYPE_UINT : return x(*(UINT  *)Data);\
										case TYPE_INT  : return x(*(INT   *)Data);\
										case TYPE_UDINT: return x(*(UDINT *)Data);\
										case TYPE_DINT : return x(*(DINT  *)Data);\
										case TYPE_REAL : return x(*(REAL  *)Data);\
										case TYPE_LREAL: return x(*(LREAL *)Data);\
										case TYPE_STRID: return x(*(STRID *)Data);\
										default: Status = SS_STATUS_NOTASSIGN; return x(0);}}catch(...){Status = SS_STATUS_NOTASSIGN; return x(0);}}




rSnapshotItem::rSnapshotItem(const rSnapshotItem& snapshot)
{
	*this = snapshot;
}

rSnapshotItem::rSnapshotItem(const string &name)
{
	if (FindVariable(name)) {
		return;
	}

	Status = SS_STATUS_NOTASSIGN;
	ClearData();
}

rSnapshotItem::rSnapshotItem(const string &name, SINT  val        ) {SNAPSHOT_ASSIGN}
rSnapshotItem::rSnapshotItem(const string &name, USINT val        ) {SNAPSHOT_ASSIGN}
rSnapshotItem::rSnapshotItem(const string &name, INT   val        ) {SNAPSHOT_ASSIGN}
rSnapshotItem::rSnapshotItem(const string &name, UINT  val        ) {SNAPSHOT_ASSIGN}
rSnapshotItem::rSnapshotItem(const string &name, DINT  val        ) {SNAPSHOT_ASSIGN}
rSnapshotItem::rSnapshotItem(const string &name, UDINT val        ) {SNAPSHOT_ASSIGN}
rSnapshotItem::rSnapshotItem(const string &name, REAL  val        ) {SNAPSHOT_ASSIGN}
rSnapshotItem::rSnapshotItem(const string &name, LREAL val        ) {SNAPSHOT_ASSIGN}
rSnapshotItem::rSnapshotItem(const string &name, STRID val        ) {SNAPSHOT_ASSIGN}
rSnapshotItem::rSnapshotItem(const string &name, const string &val)
{
	if(FindVariable(name)) return;

	Status = SS_STATUS_ASSIGN;

	try {
		// Проверка на 16-тиричное число
		if(val.size() >= 3)
		{
			if(val[0] == '0' && (val[1] == 'x' || val[1] == 'X'))
			{
				UDINT number = 0;
				DINT *ptrnum = (DINT *)&number;

				if(String_IsValidHex(val.c_str() + 2, number))
				{
					switch(Variable->Type)
					{
						case TYPE_USINT: *(USINT *)Data = USINT( number); break;
						case TYPE_SINT : *(SINT  *)Data =  SINT(*ptrnum); break;\
						case TYPE_UINT : *(UINT  *)Data =  UINT( number); break;\
						case TYPE_INT  : *(INT   *)Data =   INT(*ptrnum); break;\
						case TYPE_UDINT: *(UDINT *)Data =        number; break;\
						case TYPE_DINT : *(DINT  *)Data =       *ptrnum; break;\
						case TYPE_REAL : *(REAL  *)Data =  REAL( number); break;\
						case TYPE_LREAL: *(LREAL *)Data = LREAL( number); break;\
						case TYPE_STRID: *(STRID *)Data = STRID( number); break;\
						default:
							Status = SS_STATUS_NOTASSIGN;
							ClearData();
							break;
					}
				}

				return;
			}
		}

		switch(Variable->Type)
		{
			case TYPE_USINT: *(USINT *)Data = USINT(std::stoul(val.c_str())); break;
			case TYPE_SINT : *(SINT  *)Data =  SINT(std::stol (val.c_str())); break;\
			case TYPE_UINT : *(UINT  *)Data =  UINT(std::stoul(val.c_str())); break;\
			case TYPE_INT  : *(INT   *)Data =   INT(std::stol (val.c_str())); break;\
			case TYPE_UDINT: *(UDINT *)Data = UDINT(std::stoul(val.c_str())); break;\
			case TYPE_DINT : *(DINT  *)Data =  DINT(std::stol (val.c_str())); break;\
			case TYPE_REAL : *(REAL  *)Data =  REAL(std::stof (val.c_str())); break;\
			case TYPE_LREAL: *(LREAL *)Data = LREAL(std::stod (val.c_str())); break;\
			case TYPE_STRID: *(STRID *)Data = STRID(std::stoul(val.c_str())); break;\
			default:
				Status = SS_STATUS_NOTASSIGN;
				ClearData();
				break;
		}
	}
	catch(...)
	{
		Status = SS_STATUS_NOTASSIGN;
		ClearData();
	}
}


rSnapshotItem::rSnapshotItem(const string &name, void *buf)
{
	if(FindVariable(name)) return;

	Status = SS_STATUS_ASSIGN;

	memcpy(Data, buf, EPT_SIZE[Variable->Type]);
}



rSnapshotItem::~rSnapshotItem()
{
	;
}


SINT   rSnapshotItem::GetValueSINT()  {SNAPSHOT_GET( SINT);}
USINT  rSnapshotItem::GetValueUSINT() {SNAPSHOT_GET(USINT);}
INT    rSnapshotItem::GetValueINT()   {SNAPSHOT_GET(  INT);}
UINT   rSnapshotItem::GetValueUINT()  {SNAPSHOT_GET( UINT);}
DINT   rSnapshotItem::GetValueDINT()  {SNAPSHOT_GET( DINT);}
UDINT  rSnapshotItem::GetValueUDINT() {SNAPSHOT_GET(UDINT);}
REAL   rSnapshotItem::GetValueREAL()  {SNAPSHOT_GET( REAL);}
LREAL  rSnapshotItem::GetValueLREAL() {SNAPSHOT_GET(LREAL);}
STRID  rSnapshotItem::GetValueSTRID() {SNAPSHOT_GET(STRID);}
string rSnapshotItem::GetValueString()
{
	if((Status != SS_STATUS_ASSIGN && Status != SS_STATUS_WRITED) || nullptr == Variable) return "";

	try
	{
		switch(Variable->Type)
		{
			case TYPE_USINT: return String_format("%hhu", *(USINT *)Data); break;
			case TYPE_SINT : return String_format("%hhi", *(SINT  *)Data); break;
			case TYPE_UINT : return String_format("%hu" , *(UINT  *)Data); break;
			case TYPE_INT  : return String_format("%hi" , *(INT   *)Data); break;
			case TYPE_UDINT: return String_format("%u"  , *(UDINT *)Data); break;
			case TYPE_DINT : return String_format("%i"  , *(DINT  *)Data); break;
			case TYPE_REAL : return String_format("%#g" , *(REAL  *)Data); break;
			case TYPE_LREAL: return String_format("%#g" , *(LREAL *)Data); break;
			case TYPE_STRID: return String_format("%u"  , *(UDINT *)Data); break;
			default:
				Status = SS_STATUS_NOTASSIGN;
				return "";
		}
	}
	catch(...)
	{
		Status = SS_STATUS_NOTASSIGN;
		return "";
	}
}


const rVariable *rSnapshotItem::GetVariable() const
{
	return Variable;
}

UDINT rSnapshotItem::GetStatus() const
{
	return Status;
}


UDINT rSnapshotItem::GetBuffer(void *buffer) const
{
	if(Status != SS_STATUS_ASSIGN) return 1;

	memcpy(buffer, Data, EPT_SIZE[Variable->Type]);
	return 0;
}



UDINT rSnapshotItem::GetSizeVar() const
{
	return (Variable) ? EPT_SIZE[Variable->Type] : 0;
}



UDINT rSnapshotItem::FindVariable(const string &name)
{
	Variable = rVariable::Find(name);

	if(nullptr == Variable)
	{
		ClearData();
		Status = SS_STATUS_NOTFOUND;
		return 1;
	}

	return 0;
}


//
void rSnapshotItem::ClearData()
{
	memset(Data, 0, 8);
}


void rSnapshotItem::ResetAssign()
{
	Status = SS_STATUS_NOTASSIGN;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////

rSnapshot::rSnapshot()
{
	Access = 0;
}

rSnapshot::rSnapshot(UDINT access)
{
	Access = access;
}


rSnapshot::~rSnapshot()
{
	for (auto& item : m_list) {
		delete item;
	}

	m_list.clear();
	Access = 0;
}


const rVariable *rSnapshot::Add(const rSnapshotItem &snapshot        ) { m_list.push_back(new rSnapshotItem(snapshot )); return m_list.back()->GetVariable(); }
const rVariable *rSnapshot::Add(const string &name                   ) { m_list.push_back(new rSnapshotItem(name     )); return m_list.back()->GetVariable(); }
const rVariable *rSnapshot::Add(const string &name, SINT          val) { m_list.push_back(new rSnapshotItem(name, val)); return m_list.back()->GetVariable(); }
const rVariable *rSnapshot::Add(const string &name, USINT         val) { m_list.push_back(new rSnapshotItem(name, val)); return m_list.back()->GetVariable(); }
const rVariable *rSnapshot::Add(const string &name, INT           val) { m_list.push_back(new rSnapshotItem(name, val)); return m_list.back()->GetVariable(); }
const rVariable *rSnapshot::Add(const string &name, UINT          val) { m_list.push_back(new rSnapshotItem(name, val)); return m_list.back()->GetVariable(); }
const rVariable *rSnapshot::Add(const string &name, DINT          val) { m_list.push_back(new rSnapshotItem(name, val)); return m_list.back()->GetVariable(); }
const rVariable *rSnapshot::Add(const string &name, UDINT         val) { m_list.push_back(new rSnapshotItem(name, val)); return m_list.back()->GetVariable(); }
const rVariable *rSnapshot::Add(const string &name, REAL          val) { m_list.push_back(new rSnapshotItem(name, val)); return m_list.back()->GetVariable(); }
const rVariable *rSnapshot::Add(const string &name, LREAL         val) { m_list.push_back(new rSnapshotItem(name, val)); return m_list.back()->GetVariable(); }
const rVariable *rSnapshot::Add(const string &name, STRID         val) { m_list.push_back(new rSnapshotItem(name, val)); return m_list.back()->GetVariable(); }
const rVariable *rSnapshot::Add(const string &name, const string &val) { m_list.push_back(new rSnapshotItem(name, val)); return m_list.back()->GetVariable(); }
const rVariable *rSnapshot::Add(const string &name, void         *buf) { m_list.push_back(new rSnapshotItem(name, buf)); return m_list.back()->GetVariable(); }


rSnapshotItem *rSnapshot::operator[](const UDINT index)
{
	if(index >= m_list.size()) return nullptr;

	return m_list[index];
}

rSnapshotItem* rSnapshot::operator()(const std::string& name)
{
	for (auto& item : m_list) {
		if (item->GetVariable()) {
			if (item->GetVariable()->Name == name) {
				return item;
			}
		}
	}

	return nullptr;
}

rSnapshotItem* rSnapshot::Back() const
{
	 return m_list.back();
}

UDINT rSnapshot::Size() const
{
	 return m_list.size();
}

UDINT rSnapshot::GetAccess() const
{
	return Access;
}

void rSnapshot::SetAccess(UDINT access)
{
	Access = access;
}


void rSnapshot::ResetAssign()
{
	 for(auto &list : m_list)
	{
		list->ResetAssign();
	}
}


//-------------------------------------------------------------------------------------------------
//
void rSnapshot::Clear()
{
	for (auto &item : m_list) {
		if (item) {
			delete item;
		}

		item = nullptr;
	}

	m_list.clear();
}
