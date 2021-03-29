//=================================================================================================
//===
//=== text_class.cpp
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

#include "text_class.h"
#include "log_manager.h"
#include "error.h"
#include "tinyxml2.h"
#include "text_id.h"
#include "xml_util.h"



const UDINT TEXTLANG_SETUP_USED = 0x00000001;


rTextClass::rTextClass()
{
	CurLang = nullptr;

	m_langs.clear();
}



rTextClass::~rTextClass()
{
	for (auto lang : m_langs) {
		delete lang;
	}

	m_langs.clear();
}


//-------------------------------------------------------------------------------------------------
// Загрузка из основного файла
UDINT rTextClass::loadSystem(const string& filename, rError& err)
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLElement* root = nullptr;

	if (tinyxml2::XML_SUCCESS != doc.LoadFile(filename.c_str())) {
		return err.set(doc.ErrorID(), doc.ErrorLineNum(), doc.ErrorStr());
	}

	root = doc.FirstChildElement(XmlName::STRINGS);
	if (!root) {
		return err.set(DATACFGERR_NOTSYSTEXTFILE, 0, "");
	}

	XML_FOR(lang, root, XmlName::LANG) {
		if (TRITONN_RESULT_OK != loadLang(lang, true, err)) {
			return err.getError();
		}
	}

	return TRITONN_RESULT_OK;
}



// Загрузка дополнительных строк
UDINT rTextClass::load(tinyxml2::XMLElement* root, rError& err)
{
	if (!root) {
		return err.set(DATACFGERR_LANG_STRUCT, 0, "");
	}

	XML_FOR(lang, root, XmlName::LANG) {
		if (TRITONN_RESULT_OK != loadLang(lang, false, err)) {
			return err.getError();
		}
	}

/*
	// Устанавливаем язык по умолчанию
	string deflang = root->Attribute(CFGNAME_DEFAULT);

	if(!SetCurLang(deflang))
	{
		ErrorID   = DATACFGERR_LANG_DEFAULT;
		ErrorLine = root->GetLineNum();

		return ErrorID;
	}
*/
	deleteUnused();

	return TRITONN_RESULT_OK;
}



UDINT rTextClass::loadLang(tinyxml2::XMLElement* root, UDINT create, rError& err)
{
	std::string langID = "";
	rTextLang*  lang   = nullptr;

	if (!root) {
		return err.set(DATACFGERR_LANG_STRUCT, 0, "");
	}

	langID = String_tolower(root->Attribute(XmlName::VALUE));
	lang   = getLangPtr(langID);

	if (!lang) {
		if (create) {
			lang        = new rTextLang();
			lang->Name  = langID;
			lang->Setup = 0;

			m_langs.push_back(lang);
		} else {
			return err.set(DATACFGERR_LANG_UNKNOW, root->GetLineNum(), "");
		}
	} else {
		lang->Setup |= TEXTLANG_SETUP_USED;
	}

	XML_FOR(xml_item, root, XmlName::STR) {
		UDINT  id    = XmlUtils::getAttributeUDINT(xml_item, XmlName::ID, SID::UNKNOW);
		CCHPTR ptext = xml_item->GetText();
		string text  = (nullptr == ptext) ? "" : ptext;

		if (SID::UNKNOW == id) {
			return err.set(DATACFGERR_LANG_ID, xml_item->GetLineNum(), "");
		}

		// Проверка на сопадение ID
		if (getPtr(id, lang)) {
			return err.set(DATACFGERR_LANG_DUPID, xml_item->GetLineNum(), "");
		}

		// Добавляем элемент
		lang->Texts.push_back(rTextItem(id, text));
	}

	return TRITONN_RESULT_OK;
}



UDINT rTextClass::deleteUnused()
{
	UDINT result = 0;

	for(UDINT ii = 0; ii < m_langs.size(); ++ii) {
		if(!(m_langs[ii]->Setup & TEXTLANG_SETUP_USED))
		{
			TRACEI(LOG::TEXT, "Delete unused lang: '%s'", m_langs[ii]->Name.c_str());
			delete m_langs[ii];
			m_langs.erase(m_langs.begin() + ii);
			--ii;
			++result;
		}
	}

	return result;
}


//
rTextLang *rTextClass::getLangPtr(const string &name) const
{
	std::string lowname = String_tolower(name);

	for(UDINT ii = 0; ii < m_langs.size(); ++ii) {
		if(lowname == m_langs[ii]->Name) {
			return m_langs[ii];
		}
	}

	return nullptr;
}


//
std::string rTextClass::getCurLang() const
{
	if (nullptr == CurLang) {
		return "";
	}

	return CurLang->Name;
}


bool rTextClass::setCurLang(const std::string& lang)
{
	rTextLang *plang = getLangPtr(lang);

	if (nullptr == plang) {
		return false;
	}

	CurLang = plang;

	return true;
}


// Получение ссылки на строку в текущем языке
const std::string* rTextClass::getPtr(STRID id) const
{
	return getPtr(id, getCurLang());
}


// Получение ссылки на строку, с указанием имени языка
const std::string* rTextClass::getPtr(STRID id, const string &lang) const
{
	return getPtr(id, getLangPtr(lang));
}


//
const std::string* rTextClass::getPtr(STRID id, rTextLang* lang) const
{
	if (!lang) {
		return nullptr;
	}

	for (auto& item : lang->Texts) {
		if (item.ID == id) {
			return &item.Text;
		}
	}

	return nullptr;
}


//
bool rTextClass::get(STRID id, std::string& text) const
{
	return get(id, getCurLang(), text);
}



//
bool rTextClass::get(STRID id, const std::string& lang, std::string& text) const
{
	const std::string* result = getPtr(id, lang);

	if (!result) {
		return false;
	}

	text = *result;

	return true;
}


void rTextClass::getListLang(std::vector<string>& list) const
{
	for (auto item : m_langs) {
		list.push_back(item->Name);
	}
}


void rTextClass::getListSID(const std::string& lang, std::vector<rTextItem>& list) const
{
	rTextLang *langptr = getLangPtr(lang);

	if (!langptr) {
		return;
	}

	for (auto& item : langptr->Texts) {
		list.push_back(item);
	}
}
