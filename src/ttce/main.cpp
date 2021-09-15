
#include <stdarg.h>
#include "stringex.h"
#include "simplefile.h"
#include "../tritonn/event/eid.h"
#include "../tritonn_version.h"
#include "event_info.h"
#include "tinyxml2.h"
#include "tritonn_version.h"

//vector<rTextLang *>  gLangs;
std::vector<std::string> gLangs;
std::vector<rEventInfo>  gList;


UDINT MakeEID(const string &name, USINT type, USINT obj, UINT id, std::vector<rEventInfo> *list)
{
	UDINT  result = MAKE_EID(type, obj, id);
	static std::vector<rEventInfo> local_list;

	if(list)
	{
		*list = local_list;
		return 0;
	}

	local_list.push_back(rEventInfo(name, result));

	return result;
}



UDINT FindEID(UDINT eid)
{
	for (auto& item : gList) {
		if(item.EID == eid)
		{
			item.Property |= PROP_USED;
			return 1;
		}
	}
	return 0;
}


void ClearUsed()
{
	for(UDINT ii = 0; ii < gList.size(); ++ii)
	{
		gList[ii].Property &= ~PROP_USED;
	}
}


UDINT FindLang(const string &lang)
{
	for(UDINT ii = 0; ii < gLangs.size(); ++ii)
	{
		if(gLangs[ii] == lang)
		{
			return 1;
		}
	}
	return 0;
}

//
//
UDINT LoadLang(tinyxml2::XMLElement *root, tinyxml2::XMLDocument *doc)
{
	std::vector<UDINT> EID;
	std::string langID = "";

	ClearUsed();

	if(nullptr == root)
	{
		printf("error LoadLang\n");
		exit(1);
	}

	if(root->Attribute("value"))
	{
		langID = root->Attribute("value");
		if(FindLang(langID))
		{
			printf("dublicate lang '%s'\n", langID.c_str());
			exit(1);
		}
	}
	else
	{
		printf("error in lang\n");
		exit(1);
	}


	// Загружаем строки
	for(tinyxml2::XMLElement *item = root->FirstChildElement("str"); item != nullptr; item = item->NextSiblingElement("str"))
	{
		CCHPTR peid   = item->Attribute("id");
		UDINT  eid    = (nullptr == peid  ) ? 0xFFFFFFFF : atoi(peid);

		if(0xFFFFFFFF == eid)
		{
			printf("error. LoadLang\n");
			exit(1);
		}

		// Если не нашли EID среди массива MAKE_EID, то помечаем в файле как неиспользуемый
		if(!FindEID(eid))
		{
			string unused = String_format(" %i UNUSED ### UNUSED ### UNUSED ### UNUSED ### UNUSED ### UNUSED ### UNUSED ### ", eid);
			tinyxml2::XMLComment *comment = doc->NewComment(unused.c_str());
			root->InsertAfterChild(item, comment);
			//root->DeleteChild(item);
			continue;
		}

		for(UDINT ii = 0; ii < EID.size(); ++ii)
		{
			if(EID[ii] == eid)
			{
				printf("error. dublicate id %i\n", eid);
				exit(1);
			}
		}

		// Добавляем элемент
		EID.push_back(eid);
	}

	for(UDINT ii = 0; ii < gList.size(); ++ii)
	{
		if(!(gList[ii].Property & PROP_USED))
		{
			tinyxml2::XMLElement *element = doc->NewElement("str");

			element->SetAttribute("id", *(DINT *)&gList[ii].EID);

			root->InsertEndChild(doc->NewComment("NEW"));
			root->InsertEndChild(doc->NewComment(gList[ii].Name.c_str()));
			root->InsertEndChild(element);

			printf("add not described (eid %i)\n", gList[ii].EID);
		}
	}

	return tinyxml2::XML_SUCCESS;
}


//-------------------------------------------------------------------------------------------------
// Загрузка из основного файла, автоматически добавляем те EID, которых нет в файле
UDINT LoadSystem(const char *filename)
{
	string newfilename = filename;
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLElement *root = nullptr;

	newfilename += ".new";

	if(tinyxml2::XML_SUCCESS != doc.LoadFile(filename))
	{
		printf("Can't load file '%s'\n", filename);
		exit(1);
	}

	root = doc.FirstChildElement("strings");
	if(nullptr == root)
	{
		printf("Can't found element 'strings'\n");
		exit(1);
	}


	// Загружаем языки
	for(tinyxml2::XMLElement *lang = root->FirstChildElement("lang"); lang != nullptr; lang = lang->NextSiblingElement("lang"))
	{
		if(tinyxml2::XML_SUCCESS != LoadLang(lang, &doc))
		{
			printf("Can't load lang\n");
			exit(1);
		}
	}


	doc.SaveFile(newfilename.c_str(), false);

	return tinyxml2::XML_SUCCESS;
}


int main(int argc, const char* argv[])
{
	UNUSED(argc);
	UNUSED(argv);

	printf("ttce ver %s\n", TRITONN_VERSION);

	MakeEID("", 0, 0, 0, &gList);

	LoadSystem("./systemevent.xml");
	//TODO Нужно сделать загрузку файла systemevent.xml и сравнение его и полученных EventID, с формированием одного файла

	std::string text = "";
	for (auto& item : gList) {
		text += String_format("<!-- %s -->\n<str id=\"%i\"></str>\n", item.Name.c_str(), static_cast<DINT>(item.EID));
	}

	UDINT result = simpleFileSave("./checkevent.txt", text);
	if(TRITONN_RESULT_OK != result) {
		printf("Cant create file 'checkevent.txt'. result %i!\n", result);
		return 1;
	}

	printf("\tfound %i events.\n", static_cast<DINT>(gList.size()));

	return 0;
}
