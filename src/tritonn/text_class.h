//=================================================================================================
//===
//=== text_class.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Базовый класс обработки строк
//===
//=================================================================================================

#pragma once

#include <string>
#include <vector>
#include "tinyxml2.h"
#include "def.h"


using std::string;
using std::vector;



///////////////////////////////////////////////////////////////////////////////////////////////////
class rTextItem
{
public:
	rTextItem(UDINT id, const string &text)
	{
		ID   = id;
		Text = text;
	}


	UDINT ID;
	string Text;
};


class rTextLang
{
public:
	string Name;
	UDINT  Setup;

	vector<rTextItem> Texts;
};



class rTextClass
{
public:
	rTextClass();
	virtual ~rTextClass();

public:
	UDINT ErrorID;
	UDINT ErrorLine;

private:
	vector<rTextLang *>  Langs;
	rTextLang           *CurLang;


public:
	UDINT         LoadSystem(const string& filename);
	UDINT         Load(tinyxml2::XMLElement *root);
	UDINT         SetCurLang(const string &lang);
	string        GetCurLang();
	UDINT         Get(UDINT id, const string &lang, string &text);
	UDINT         Get(UDINT id, string &text);
	const string *GetPtr(UDINT id, const string &lang);
	const string *GetPtr(UDINT id);

	UDINT         GetListLang(vector<string> &list);
	UDINT         GetListSID (const string &lang, vector<rTextItem> &list);

private:
	UDINT         LoadLang(tinyxml2::XMLElement *root, UDINT create);
	UDINT         DeleteUnused();
	rTextLang    *GetLangPtr(const string &name);
	const string *GetPtr(UDINT id, rTextLang *lang);
};



