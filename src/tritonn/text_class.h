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
#include "def.h"

class rError;

namespace tinyxml2
{
	class XMLElement;
};

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
	std::vector<rTextLang*>  m_langs;
	rTextLang* CurLang;

public:
	UDINT       loadSystem(const std::string& filename, rError& err);
	UDINT       load(tinyxml2::XMLElement* root, rError& err);
	bool        setCurLang(const std::string& lang);
	std::string getCurLang() const;
	bool        get(STRID id, const std::string& lang, std::string& text) const;
	bool        get(STRID id, std::string& text) const;
	const std::string *getPtr(STRID id, const std::string &lang) const;
	const std::string *getPtr(STRID id) const;

	void        getListLang(std::vector<std::string>& list) const;
	void        getListSID (const std::string& lang, std::vector<rTextItem>& list) const;

private:
	UDINT         loadLang(tinyxml2::XMLElement *root, UDINT create, rError& err);
	UDINT         deleteUnused();
	rTextLang    *getLangPtr(const std::string& name) const;
	const std::string* getPtr(STRID id, rTextLang* lang) const;
};



