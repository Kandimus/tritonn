﻿//=================================================================================================
//===
//=== simplefile.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Загрузка и сохранение простого текстового файла
//===
//=================================================================================================

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "hash.h"
#include "stringex.h"
#include "simplefile.h"


UDINT SimpleFileErrno(UDINT deferrno)
{
	switch(errno)
	{
		case ENODEV:
		case ENOENT:
		case ENXIO:
			return FILE_RESULT_NOTFOUND;

		case EPERM:
		case EACCES:
			return FILE_RESULT_EACCESS;

		case EISDIR:
			return FILE_RESULT_ISDIR;

		case ETXTBSY:
		case EWOULDBLOCK:
			return FILE_RESULT_BLOCKED;

		case EROFS:
			return FILE_RESULT_READONLY;

		default:
			return deferrno;
	}
}


UDINT SimpleFileCreateDir(const std::string& filename)
{
	std::string path = "";

	for(UDINT ii = 0; ii < filename.size(); ++ii)
	{
		path += filename[ii];

		if(filename[ii] == '/')
		{
			if(access(path.c_str(), F_OK)) // даной директории нет, или нет доступа
			{
				if(mkdir(path.c_str(), 0744))
				{
					return FILE_RESULT_EDIR;
				}
			}
		}
	}

	return TRITONN_RESULT_OK;
}


UDINT SimpleFileDelete(const std::string& filename)
{
	if (unlink(filename.c_str())) {
		return SimpleFileErrno(FILE_RESULT_CANTDELETE);
	}

	return TRITONN_RESULT_OK;
}


//
UDINT SimpleFileLoad(const std::string& filename, std::string& text)
{
	FILE  *file = fopen(filename.c_str(), "rt");
	char  *buff = nullptr;
	UDINT  size = 0;
	UDINT  fr   = 0;

	if (!file) {
		return SimpleFileErrno(FILE_RESULT_CANTOPEN);
	}

	fseek(file, 0, SEEK_END);
	size = ftell(file);

	if(!size)
	{
		fclose(file);
		return FILE_RESULT_ISEMPTY;
	}

	buff = new char[size + 1];
	fseek(file, 0, SEEK_SET);
	fr = fread(buff, 1, size, file);
	buff[size] = 0;
	fclose(file);

	if(fr != size)
	{
		delete[] buff;
		text = "";
		return FILE_RESULT_IOERROR;
	}

	text = buff;
	delete[] buff;

	return TRITONN_RESULT_OK;
}


UDINT SimpleFileSaveExt(const std::string& filename, const std::string& text, const std::string& mode)
{
	FILE  *file = nullptr;
	UDINT  fw   = 0;

	// Проверка на доступность файла
	if(access(filename.c_str(), F_OK))
	{
		// Попытка создать требуемые директории
		UDINT result = SimpleFileCreateDir(filename);

		if(result) return result;
	}

	file = fopen(filename.c_str(), mode.c_str());
	if (!file) {
		return SimpleFileErrno(FILE_RESULT_CANTOPEN);
	}

	fw = fwrite(text.c_str(), 1, text.size(), file);
	fclose(file);

	if(fw != text.size())
	{
		return FILE_RESULT_IOERROR;
	}

	return TRITONN_RESULT_OK;
}


UDINT SimpleFileSave(const std::string& filename, const std::string& text)
{
	return SimpleFileSaveExt(filename, text, "wt");
}

UDINT simpleFileSaveSignature(const std::string& filename, const std::string& text, const std::string& marker)
{
	std::string sig_text = text;
	DINT pos = sig_text.find(marker);

	if (pos != -1) {
		USINT hash[MAX_HASH_SIZE];

		GetStrHash(text, hash);

		std::string str_hash = String_FromBuffer(hash, MAX_HASH_SIZE);

printf("~~~~~~~~~~~~~~ %s\n", str_hash.c_str());

		pos += marker.size();
		while (sig_text[pos] != '=' && pos < sig_text.size()) {
			++pos;
		}

		if (pos >= sig_text.size()) {
			return FILE_RESULT_ENCRYPT_ERROR;
		}

		while (sig_text[pos] != '\"' && pos < sig_text.size()) {
			++pos;
		}

		DINT begin = ++pos;

		if (pos >= sig_text.size()) {
			return FILE_RESULT_ENCRYPT_ERROR;
		}

		while (sig_text[pos] != '\"' && pos < sig_text.size()) {
			++pos;
		}

		DINT end = pos;

		if (pos >= sig_text.size()) {
			return FILE_RESULT_ENCRYPT_ERROR;
		}

printf("~~~~~~~~~~~~~~ %i ... %i\n", begin, end);

	}

	return SimpleFileSaveExt(filename, sig_text, "wt");
}

UDINT SimpleFileAppend(const std::string& filename, const std::string& text)
{
	return SimpleFileSaveExt(filename, text, "at");
}

UDINT SimpleFileGaranteedSave(const std::string& filename, const std::string& text)
{
	std::string nametmp = filename + ".temp";
	UDINT       result  = SimpleFileSave(nametmp, text);

	if (result != TRITONN_RESULT_OK) {
		return result;
	}

	::rename(nametmp.c_str(), filename.c_str());
	return SimpleFileErrno(FILE_RESULT_CANTOPEN);
}
