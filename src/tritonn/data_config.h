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
#include "singlenton.h"
#include "error.h"

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
	SINGLETON(rDataConfig)

public:
	std::string FileName;
	rError      m_error;

	UDINT LoadFile(const std::string& filename, rSystemVariable& sysvar, std::vector<rSource*>& listsrc, std::vector<rInterface*>& listiface, std::vector<rReport*>& listrpt);

protected:
	cJSON                *CfgJSON;
	cJSON                *CfgJSON_IO;
	cJSON                *CfgJSON_OBJ;
	cJSON                *CfgJSON_VAR;
	cJSON                *CfgJSON_USR;
	tinyxml2::XMLElement* XMLRootSecurity;
	rSystemVariable*      SysVar;
	std::vector<rSource* >    *ListSource;
	std::vector<rInterface* > *ListInterface;
	std::vector<rReport* >    *ListReport;
	std::vector<rLink* >       ListLink; //TODO Нужно ли это оставлять тут, или перенести в rDataManager?

	UDINT LoadSecurity  (tinyxml2::XMLElement* root, tinyxml2::XMLDocument& doc_security);
	UDINT LoadHardware  (tinyxml2::XMLElement* root);
	UDINT LoadConfig    (tinyxml2::XMLElement* root);
	UDINT LoadStation   (tinyxml2::XMLElement* root, cJSON* jroot);
	UDINT LoadStream    (tinyxml2::XMLElement* root, cJSON* jroot, rStation* owner, const std::string& prefix);
	UDINT LoadIO        (tinyxml2::XMLElement* root, cJSON* jroot, rStation* owner, const std::string& prefix);
	UDINT LoadCalc      (tinyxml2::XMLElement* root, cJSON* jroot, rStation* owner, const std::string& prefix);
	UDINT LoadReport    (tinyxml2::XMLElement* root);
	UDINT LoadCustom    (tinyxml2::XMLElement* root);
	UDINT LoadVariable  (tinyxml2::XMLElement* root);
	UDINT LoadUsers     (tinyxml2::XMLElement* root, cJSON* jroot);
	UDINT LoadComms     (tinyxml2::XMLElement* root);
	UDINT LoadModbus    (tinyxml2::XMLElement* root);
	UDINT LoadModbusTCP (tinyxml2::XMLElement* root);
	UDINT LoadOPCUA     (tinyxml2::XMLElement* root);

	UDINT ResolveLinks(void);
	UDINT ResolveReports(void);

	void  SaveWeb();

public:
	UDINT LoadLink(tinyxml2::XMLElement* element, rLink& link, bool required = true);
	UDINT LoadShadowLink(tinyxml2::XMLElement* element, rLink& link, rLink& mainlink, const string& name);


	tinyxml2::XMLElement* GetRootSecurity();
};

