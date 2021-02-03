//=================================================================================================
//===
//=== event_class.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс для регистрации событий, как системных, так и пользователя.
//===
//=================================================================================================

#include <string.h>
#include "event_class.h"


rEvent::rEvent(void)
{
	Clear();
}


rEvent::rEvent(DINT eid)
{
	Reinit(eid);
}


rEvent::~rEvent()
{
	EID  = EID_UNDEF;
	Size = 0;
}

rEvent& rEvent::operator = (const rEvent& event)
{
	if (&event == this) {
		return *this;
	}

	EID  = event.EID;
	Size = event.Size;

	gettimeofday(&Timestamp, NULL);

	memcpy(Data, event.Data, MAX_EVENT_DATA);

	return *this;
}

//-------------------------------------------------------------------------------------------------
//
void rEvent::Clear()
{
	Timestamp.tv_sec  = 0;
	Timestamp.tv_usec = 0;
	Size              = 0;
	EID               = EID_UNDEF;

	memset(Data, 0, MAX_EVENT_DATA);
}


//
rEvent &rEvent::Reinit(UDINT eid)
{
	EID  = eid;
	Size = 0;

	gettimeofday(&Timestamp, NULL);
	memset(Data, 0, MAX_EVENT_DATA);

	return *this;
}


//-------------------------------------------------------------------------------------------------
void *rEvent::GetParamByID(UDINT ID, UDINT &type)
{
	UDINT pos   = 0;
	UDINT curid = 0;
	
	while(pos < Size)
	{
		if(ID == curid)
		{
			type = Data[pos];
			return (void *)(Data + pos + 1);
		}
		
		pos += 1 + EPT_SIZE[Data[pos]];
		++curid;
	}
	
	type = TYPE_UNDEF;
	return nullptr;
}



