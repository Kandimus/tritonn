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

bool findSignature(DINT pos, const std::string& text, DINT& pos_begin, DINT& pos_end)
{
	pos_begin = -1;
	pos_end   = -1;

	while (text[pos] != '=' && pos < text.size()) {
		++pos;
	}

	if (pos >= text.size()) {
		return false;
	}

	while (text[pos] != '\"' && pos < text.size()) {
		++pos;
	}

	pos_begin = ++pos;

	if (pos >= text.size()) {
		return false;
	}

	while (text[pos] != '\"' && pos < text.size()) {
		++pos;
	}

	pos_end = pos;

	if (pos >= text.size()) {
		return false;
	}

	return true;
}


UDINT xmlDumpFile(const std::string&filename, tinyxml2::XMLDocument &doc, std::string& strhash)
{
	std::string text;
	UDINT       result = simpleFileLoad(filename, text);

	strhash.clear();

	if (TRITONN_RESULT_OK != result) {
		return result;
	}

	doc.Clear();

	if (tinyxml2::XML_SUCCESS != doc.Parse(text.c_str())) {
		return doc.ErrorID();
	}

	auto root = doc.RootElement();

	if (!root->Attribute("hash")) {
		return XMLFILE_RESULT_NFHASH;
	}

	strhash = root->Attribute("hash");

	if (MAX_STRHASH_SIZE != strhash.size()) {
		return XMLFILE_RESULT_NFHASH;
	}

	return TRITONN_RESULT_OK;
}

UDINT xmlFileCheck(const std::string &filename, tinyxml2::XMLDocument &doc, const std::string& signature, const std::string& salt)
{
	std::string text;
	UDINT       result = simpleFileLoad(filename, text);

	if (TRITONN_RESULT_OK != result) {
		return result;
	}

	doc.Clear();

	if (tinyxml2::XML_SUCCESS != doc.Parse(text.c_str())) {
		return doc.ErrorID();
	}

	auto root = doc.RootElement();

	if (!root->Attribute(signature.c_str())) {
		return XMLFILE_RESULT_NFHASH;
	}

	std::string strhash = root->Attribute(signature.c_str());

	if (MAX_STRHASH_SIZE != strhash.size()) {
		return XMLFILE_RESULT_NFHASH;
	}

	int pos   = text.find(signature);
	int begin = -1;
	int end   = -1;
	if(pos <= 0) {
		return XMLFILE_RESULT_BADHASH;
	}

	pos += signature.size();
	if (!findSignature(pos, text, begin, end)) {
		return XMLFILE_RESULT_ENCRYPT_ERROR;
	}

	// Заменяем hash на соль
	text = text.replace(begin, end - begin, salt);

	USINT fhash[MAX_HASH_SIZE] = {0};
	USINT shash[MAX_HASH_SIZE] = {0};

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



UDINT xmlFileSave(const std::string& filename, const std::string& text, const std::string& marker)
{
	DINT        pos      = text.find(marker);
	std::string sig_text = text;

	simpleFileSave(filename + "_", text);

	if (pos != -1) {
		DINT begin = -1;
		DINT end   = -1;

		pos += marker.size();
		if (!findSignature(pos, text, begin, end)) {
			return XMLFILE_RESULT_ENCRYPT_ERROR;
		}

		USINT hash[MAX_HASH_SIZE];

		GetStrHash(text, hash);

		std::string str_hash = String_FromBuffer(hash, MAX_HASH_SIZE);

		simpleFileSave(filename + "_sha", str_hash);

		sig_text = sig_text.replace(begin, end - begin, str_hash);
	}

	return simpleFileSave(filename, sig_text);
}
