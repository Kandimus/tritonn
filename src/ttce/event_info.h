#pragma once

#include <vector>
#include <string>
#include "tinyxml2.h"
#include "event_eid.h"

using std::vector;
using std::string;


const UDINT PROP_USED = 1;


struct rEventInfo
{
	rEventInfo(const string &n, UDINT eid) : Name(n), EID(eid), Property(0) {}

	string Name;
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
	string Text;
	UDINT  Property;

	tinyxml2::XMLElement *Element;
};


class rTextLang
{
public:
	string Name;
	UDINT  Setup;


	tinyxml2::XMLElement *Element;
	vector<rTextItem> Texts;

	rTextItem *Get(UDINT sid);
};


//extern vector<rTextLang *>  gLangs;
extern vector<string>  gLangs;


extern UDINT MakeEID(const string &name, USINT type, USINT obj, UINT id, vector<rEventInfo> *list);
