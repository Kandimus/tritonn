//=================================================================================================
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
#include "simplefile.h"


unsigned int simpleFileErrno(unsigned int deferrno)
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


unsigned int simpleFileCreateDir(const std::string& filename)
{
	std::string path = "";

	for (auto ch : filename) {
		path += ch;

		if(ch == '/')
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


unsigned int simpleFileDelete(const std::string& filename)
{
	if (unlink(filename.c_str())) {
		return simpleFileErrno(FILE_RESULT_CANTDELETE);
	}

	return TRITONN_RESULT_OK;
}


//
unsigned int simpleFileLoad(const std::string& filename, std::string& text)
{
	FILE* file = fopen(filename.c_str(), "rt");

	if (!file) {
		return simpleFileErrno(FILE_RESULT_CANTOPEN);
	}

	fseek(file, 0, SEEK_END);
	unsigned int size = ftell(file);

	if (!size) {
		fclose(file);
		return FILE_RESULT_ISEMPTY;
	}

	char* buff = new char[size + 1];
	fseek(file, 0, SEEK_SET);

	unsigned int fr = fread(buff, 1, size, file);
	buff[size] = 0;
	fclose(file);

	if (fr != size) {
		delete[] buff;
		text = "";
		return FILE_RESULT_IOERROR;
	}

	text = buff;
	delete[] buff;

	return TRITONN_RESULT_OK;
}


unsigned int simpleFileSaveExt(const std::string& filename, const std::string& text, const std::string& mode)
{
	// Проверка на доступность файла
	if(access(filename.c_str(), F_OK))
	{
		// Попытка создать требуемые директории
		unsigned int result = simpleFileCreateDir(filename);

		if (result) {
			return result;
		}
	}

	FILE* file = fopen(filename.c_str(), mode.c_str());
	if (!file) {
		return simpleFileErrno(FILE_RESULT_CANTOPEN);
	}

	unsigned int fw = fwrite(text.c_str(), 1, text.size(), file);
	fclose(file);

	if (fw != text.size()) {
		return FILE_RESULT_IOERROR;
	}

	return TRITONN_RESULT_OK;
}


unsigned int simpleFileSave(const std::string& filename, const std::string& text)
{
	return simpleFileSaveExt(filename, text, "wt");
}

unsigned int simpleFileAppend(const std::string& filename, const std::string& text)
{
	return simpleFileSaveExt(filename, text, "at");
}

unsigned int simpleFileGuaranteedSave(const std::string& filename, const std::string& text)
{
	std::string  nametmp = filename + ".temp";
	unsigned int result  = simpleFileSave(nametmp, text);

	if (result != TRITONN_RESULT_OK) {
		return result;
	}

	::rename(nametmp.c_str(), filename.c_str());
	return simpleFileErrno(FILE_RESULT_CANTOPEN);
}
