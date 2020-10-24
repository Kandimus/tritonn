//=================================================================================================
//===
//=== data_config.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Сборный класс описывающий связи между элементами.
//=== Так же осуществляет загрузку конфигурации с диска.
//===
//=================================================================================================

#pragma once

#include <vector>
#include "def.h"

namespace tinyxml2
{
	class XMLElement;
	class XMLDocument;
}

struct cJSON;
class  rSource;
class  rLink;
class  rInterface;
class  rStation;
class  rReport;
struct rSystemVariable;


struct rBitFlag
{
	std::string Name;
	UDINT  Value;

	rBitFlag(const std::string &name, UDINT value) : Name(name), Value(value) {}
};


class rDataConfig
{
public:
	rDataConfig();
	virtual ~rDataConfig();

	std::string FileName;
	std::string ErrorStr;
	UDINT       ErrorID;
	UDINT       ErrorLine;
	std::string Prefix;

	UDINT LoadFile(const std::string &filename, rSystemVariable &sysvar, std::vector<rSource *> &listsrc, std::vector<rInterface *> &listiface, std::vector<rReport *> &listrpt);


public:
	static std::vector<rBitFlag> STNProductValues;
	static std::vector<rBitFlag> STRFMeterFlags;
	static std::vector<rBitFlag> SelectorSetupFlags;
	static std::vector<rBitFlag> SelectorModeFlags;
	static std::vector<rBitFlag> LimitSetupFlags;
	static std::vector<rBitFlag> ReportTypeFlags;
	static std::vector<rBitFlag> ReportPeriodFlags;

	static void        InitBitFlags();
	static std::string GetFlagNameByBit  (std::vector<rBitFlag> &arr, UDINT value);
	static std::string GetFlagNameByValue(std::vector<rBitFlag> &arr, UDINT value);
	static UDINT       GetFlagBitByStr   (std::vector<rBitFlag> &arr, const string &name, UDINT &err);
	static UDINT       GetFlagFromStr    (std::vector<rBitFlag> &arr, const string &str, UDINT &err);

	static USINT  GetAttributeUSINT (tinyxml2::XMLElement *element, const string &name, const USINT def);
	static UDINT  GetAttributeUDINT (tinyxml2::XMLElement *element, const string &name, const UDINT def);
	static string GetAttributeString(tinyxml2::XMLElement *element, const string &name, const string &def);

	static string GetTextString(tinyxml2::XMLElement *element, const string &def, UDINT &err);
	static LREAL  GetTextLREAL (tinyxml2::XMLElement *element, LREAL         def, UDINT &err);
	static UDINT  GetTextUDINT (tinyxml2::XMLElement *element, UDINT         def, UDINT &err);
	static UINT   GetTextUINT  (tinyxml2::XMLElement *element, UINT          def, UDINT &err);
	static USINT  GetTextUSINT (tinyxml2::XMLElement *element, USINT         def, UDINT &err);

protected:
	cJSON                *CfgJSON;
	cJSON                *CfgJSON_IO;
	cJSON                *CfgJSON_OBJ;
	cJSON                *CfgJSON_VAR;
	cJSON                *CfgJSON_USR;
	tinyxml2::XMLElement *XMLRootSecurity;
	rSystemVariable      *SysVar;
	std::vector<rSource* >    *ListSource;
	std::vector<rInterface* > *ListInterface;
	std::vector<rReport* >    *ListReport;
	std::vector<rLink* >       ListLink; //TODO Нужно ли это оставлять тут, или перенести в rDataManager?

	UDINT LoadSecurity  (tinyxml2::XMLElement *root, tinyxml2::XMLDocument &doc_security);
	UDINT LoadHardware  (tinyxml2::XMLElement *root);
	UDINT LoadConfig    (tinyxml2::XMLElement *root);
	UDINT LoadIO        (tinyxml2::XMLElement *root, cJSON *jroot, rStation *owner);
	UDINT LoadCalc      (tinyxml2::XMLElement *root, cJSON *jroot, rStation *owner);
	UDINT LoadStation   (tinyxml2::XMLElement *root, cJSON *jroot);
	UDINT LoadStream    (tinyxml2::XMLElement *root, cJSON *jroot, rStation *owner);
	UDINT LoadReport    (tinyxml2::XMLElement *root);
	UDINT LoadCustom    (tinyxml2::XMLElement *root);
	UDINT LoadVariable  (tinyxml2::XMLElement *root);
	UDINT LoadUsers     (tinyxml2::XMLElement *root, cJSON *jroot);
	UDINT LoadComms     (tinyxml2::XMLElement *root);
	UDINT LoadModbus    (tinyxml2::XMLElement *root);
	UDINT LoadModbusTCP (tinyxml2::XMLElement *root);
	UDINT LoadOPCUA     (tinyxml2::XMLElement *root);

	UDINT ResolveLinks(void);
	UDINT ResolveReports(void);

	void  SaveWeb();

public:
	UDINT LoadLink(tinyxml2::XMLElement *element, rLink &link);
	UDINT LoadShadowLink(tinyxml2::XMLElement *element, rLink &link, rLink &mainlink, const string &name);


	tinyxml2::XMLElement *GetRootSecurity();
};

