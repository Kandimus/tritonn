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

class rError;

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
	std::string Text;
};


class rTextLang
{
public:
	std::string Name;
	UDINT       Setup;

	std::vector<rTextItem> Texts;
};



class rTextClass
{
public:
	rTextClass();
	virtual ~rTextClass();

private:
	std::vector<rTextLang*>  Langs;
	rTextLang* CurLang;

public:
	UDINT         LoadSystem(const std::string& filename, rError& err);
	UDINT         Load(tinyxml2::XMLElement* root, rError& err);
	UDINT         SetCurLang(const std::string& lang);
	std::string   GetCurLang();
	UDINT         Get(STRID id, const std::string& lang, std::string& text);
	UDINT         Get(STRID id, std::string& text);
	const std::string *GetPtr(STRID id, const std::string &lang);
	const std::string *GetPtr(STRID id);

	UDINT         GetListLang(std::vector<std::string> &list);
	UDINT         GetListSID (const std::string &lang, std::vector<rTextItem> &list);

private:
	UDINT         LoadLang(tinyxml2::XMLElement *root, UDINT create, rError& err);
	UDINT         DeleteUnused();
	rTextLang    *GetLangPtr(const std::string& name);
	const std::string* GetPtr(STRID id, rTextLang* lang);
};



