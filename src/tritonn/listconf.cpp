//=================================================================================================
//===
//=== data_listconf.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс для получения информации об доступных конфигурациях
//===
//=================================================================================================

#include <sys/stat.h>
#include <dirent.h>
#include "xmlfile.h"
#include "listconf.h"

vector<rItemConfig> rListConfig::List;


rListConfig::~rListConfig()
{
	List.clear();
}


UDINT rListConfig::Size()
{
	return List.size();
}


const rItemConfig *rListConfig::Get(const UDINT index)
{
	if(index >= List.size()) return nullptr;

	return &List[index];
}


//
UDINT rListConfig::Load()
{
	UDINT result = TRITONN_RESULT_OK;
	DIR  *dir;
	struct dirent *curdir;

	List.clear();

	if(nullptr == (dir = opendir(DIR_CONF.c_str())))
	{
		return FILE_RESULT_EACCESS;
	}

	while(nullptr != (curdir = readdir(dir)))
	{
		rItemConfig item;
		string      path;
		struct stat buf;   //TODO Проблема 2038 года

		item.Filename = curdir->d_name;
		path          = DIR_CONF + item.Filename;

		if("." == item.Filename || ".." == item.Filename) continue;

		// проверка на расширение
		int pos = item.Filename.rfind('.');
		if(pos <= 0) continue;
		if("xml" != item.Filename.substr(pos + 1))
		{
			continue;
		}

		if(stat(path.c_str(), &buf))
		{
			break;
		}

		// Проверка на "обычный" файл
		if(!S_ISREG(buf.st_mode))
		{
			continue;
		}

		item.Filetime = buf.st_mtim.tv_sec;

		tinyxml2::XMLDocument  doc;
		tinyxml2::XMLElement  *root;

		item.Status = XMLFileCheck(path, doc);

		//NOTE пока временно оставим, в рабочем режиме нужна проверка только на OK
		if(TRITONN_RESULT_OK != item.Status && XMLFILE_RESULT_NOTEQUAL != item.Status && XMLFILE_RESULT_BADHASH != item.Status && XMLFILE_RESULT_NFHASH != item.Status)
		{
			continue;
		}

		root = doc.RootElement();

		item.Description = (root->Attribute("development")) ? root->Attribute("development") : "";
		item.StrHash     = (root->Attribute("hash"))        ? root->Attribute("hash")        : "";

		List.push_back(item);
	}

	closedir(dir);

	return result;
}

