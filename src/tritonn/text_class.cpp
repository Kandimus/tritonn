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
//#include "data_config.h"
#include "text_id.h"
#include "xml_util.h"



const UDINT TEXTLANG_SETUP_USED = 0x00000001;


rTextClass::rTextClass()
{
	CurLang = nullptr;

	Langs.clear();
}



rTextClass::~rTextClass()
{
	for(auto lang: Langs) {
		delete lang;
	}
	Langs.clear();
}


//-------------------------------------------------------------------------------------------------
// Загрузка из основного файла
UDINT rTextClass::LoadSystem(const string& filename, rError& err)
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLElement *root = nullptr;

	if (tinyxml2::XML_SUCCESS != doc.LoadFile(filename.c_str())) {
		return err.set(doc.ErrorID(), doc.ErrorLineNum(), doc.ErrorStr());
	}

	root = doc.FirstChildElement(XmlName::STRINGS);
	if (!root) {
		return err.set(DATACFGERR_NOTSYSTEXTFILE, 0, "");
	}

	XML_FOR(lang, root, XmlName::LANG) {
		if (TRITONN_RESULT_OK != LoadLang(lang, true, err)) {
			return err.getError();
		}
	}

	return TRITONN_RESULT_OK;
}



// Загрузка дополнительных строк
UDINT rTextClass::Load(tinyxml2::XMLElement* root, rError& err)
{
	if (!root) {
		return err.set(DATACFGERR_LANG_STRUCT, 0, "");
	}

	XML_FOR(lang, root, XmlName::LANG) {
		if (TRITONN_RESULT_OK != LoadLang(lang, false, err)) {
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
	DeleteUnused();

	return TRITONN_RESULT_OK;
}



UDINT rTextClass::LoadLang(tinyxml2::XMLElement *root, UDINT create, rError& err)
{
	std::string langID = "";
	rTextLang*  lang   = nullptr;

	if (!root) {
		return err.set(DATACFGERR_LANG_STRUCT, 0, "");
	}

	langID = String_tolower(root->Attribute(XmlName::VALUE));
	lang   = GetLangPtr(langID);

	if (!lang) {
		if (create) {
			lang        = new rTextLang();
			lang->Name  = langID;
			lang->Setup = 0;

			Langs.push_back(lang);
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
		if (GetPtr(id, lang)) {
			return err.set(DATACFGERR_LANG_DUPID, xml_item->GetLineNum(), "");
		}

		// Добавляем элемент
		lang->Texts.push_back(rTextItem(id, text));
	}

	return TRITONN_RESULT_OK;
}



UDINT rTextClass::DeleteUnused()
{
	UDINT result = 0;

	for(UDINT ii = 0; ii < Langs.size(); ++ii)
	{
		if(!(Langs[ii]->Setup & TEXTLANG_SETUP_USED))
		{
			TRACEI(LM_TEXT, "Delete unused lang: '%s'", Langs[ii]->Name.c_str());
			delete Langs[ii];
			Langs.erase(Langs.begin() + ii);
			--ii;
			++result;
		}
	}

	return result;
}


//
rTextLang *rTextClass::GetLangPtr(const string &name)
{
	string lowname = String_tolower(name);

	for(UDINT ii = 0; ii < Langs.size(); ++ii)
	{
		if(lowname == Langs[ii]->Name) return Langs[ii];
	}

	return nullptr;
}


//
string rTextClass::GetCurLang()
{
	if(nullptr == CurLang) return "";
	return CurLang->Name;
}


UDINT rTextClass::SetCurLang(const string &lang)
{
	rTextLang *plang = GetLangPtr(lang);

	if(nullptr == plang) return 0;

	CurLang = plang;

	return 1;
}


// Получение ссылки на строку в текущем языке
const string *rTextClass::GetPtr(STRID id)
{
	return GetPtr(id, GetCurLang());
}


// Получение ссылки на строку, с указанием имени языка
const string *rTextClass::GetPtr(STRID id, const string &lang)
{
	return GetPtr(id, GetLangPtr(lang));
}


//
const string *rTextClass::GetPtr(STRID id, rTextLang *lang)
{
	if(nullptr == lang) return nullptr;

	for(UDINT ii = 0;  ii < lang->Texts.size(); ++ii)
	{
		if(lang->Texts[ii].ID == id) return &lang->Texts[ii].Text;
	}

	return nullptr;
}


//
UDINT rTextClass::Get(STRID id, string &text)
{
	return Get(id, GetCurLang(), text);
}



//
UDINT rTextClass::Get(STRID id, const string &lang, string &text)
{
	const std::string *result = GetPtr(id, lang);

	if(nullptr == result) return 1;

	text = *result;

	return 0;
}


UDINT rTextClass::GetListLang(vector<string> &list)
{
	for(UDINT ii = 0; ii < Langs.size(); ++ii)
	{
		list.push_back(Langs[ii]->Name);
	}
	return 0;
}


UDINT rTextClass::GetListSID (const string &lang, vector<rTextItem> &list)
{
	rTextLang *langptr = GetLangPtr(lang);

	if(nullptr == langptr) return 1;

	for(UDINT ii = 0; ii < langptr->Texts.size(); ++ii)
	{
		list.push_back(langptr->Texts[ii]);
	}
	return 0;
}
