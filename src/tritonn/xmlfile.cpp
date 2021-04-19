//=================================================================================================
//===
//=== xmlfile.cpp
//===
//=== Copyright (c) 2020 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Загрузка и сохранение простого текстового файла
//===
//=================================================================================================

#include "tinyxml2.h"
#include "stringex.h"
#include "hash.h"
#include "simplefile.h"


using tinyxml2::XMLElement;


UDINT XMLFileCheck(const std::string &filename, tinyxml2::XMLDocument &doc, std::string& strhash)
{
	USINT       fhash[MAX_HASH_SIZE] = {0};
	USINT       shash[MAX_HASH_SIZE] = {0};
	std::string text;
	XMLElement* root   = nullptr;
	UDINT       result = SimpleFileLoad(filename, text);

	strhash.clear();

	if (TRITONN_RESULT_OK != result) {
		return result;
	}

	doc.Clear();

	if (tinyxml2::XML_SUCCESS != doc.Parse(text.c_str())) {
		return doc.ErrorID();
	}

	root = doc.RootElement();

	if (!root->Attribute("hash")) {
		return XMLFILE_RESULT_NFHASH;
	}

	strhash = root->Attribute("hash");

	if (MAX_HASH_SIZE != strhash.size()) {
		return XMLFILE_RESULT_NFHASH;
	}

	int pos = text.find(strhash);
	if(pos <= 0) {
		return XMLFILE_RESULT_BADHASH;
	}

	// Заменяем hash на соль
	for (UDINT ii = 0; ii < MAX_HASH_SIZE; ++ii) {
		text[pos + ii] = XMLHASH_SALT[ii];
	}

	// получаем оба HASH
	String_ToBuffer(strhash.c_str(), fhash, MAX_HASH_SIZE);
	GetStrHash(text, shash);

	//
	if(!CmpHash(fhash, shash))
	{
		return XMLFILE_RESULT_NOTEQUAL;
	}

	return TRITONN_RESULT_OK;
}

