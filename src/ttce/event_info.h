#pragma once

#include <vector>
#include <string>
#include "tinyxml2.h"
#include "event_eid.h"


const UDINT PROP_USED = 1;


struct rEventInfo
{
	rEventInfo(const string &n, UDINT eid) : Name(n), EID(eid), Property(0) {}

	std::string Name;
	UDINT  EID;
	UDINT  Property;
};



class rTextItem
{
public:
	rTextItem(UDINT id, const string &text, tinyxml2::XMLElement *element, UDINT property)
	{
		ID       = id;
		Text     = text;
		Element  = element;
		Property = property;
	}

	UDINT  ID;
	std::string Text;
	UDINT  Property;

	tinyxml2::XMLElement *Element;
};


class rTextLang
{
public:
	string Name;
	UDINT  Setup;


	tinyxml2::XMLElement *Element;
	std::vector<rTextItem> Texts;

	rTextItem *Get(UDINT sid);
};


//extern vector<rTextLang *>  gLangs;
extern std::vector<std::string>  gLangs;


extern UDINT MakeEID(const string &name, USINT type, USINT obj, UINT id, std::vector<rEventInfo> *list);
