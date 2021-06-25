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
#include "data_module.h"
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
class  rTotal;
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

	struct rLinkTotal
	{
		rTotal**    m_dest;
		std::string m_alias;
		std::string m_name;
		UDINT       m_lineno;
	};

public:
	UDINT LoadFile(const std::string& filename, rSystemVariable& sysvar, std::vector<rSource*>& listsrc, std::vector<rInterface*>& listiface, std::vector<rReport*>& listrpt);
	UDINT LoadLink(tinyxml2::XMLElement* element, rLink& link, bool required = true);
	UDINT LoadShadowLink(tinyxml2::XMLElement* element, rLink& link, rLink& mainlink, const string& name);

	const rSource* getSource(const std::string& alias);
	tinyxml2::XMLElement* GetRootSecurity();

protected:
	cJSON* m_json     = nullptr;
	cJSON* m_json_io  = nullptr;
	cJSON* m_json_obj = nullptr;
	cJSON* m_json_var = nullptr;
	cJSON* m_json_usr = nullptr;
	cJSON* m_json_hdw = nullptr;

	tinyxml2::XMLElement* m_xmlRootSecurity;
	rSystemVariable*      SysVar;
	std::vector<rSource*>    *ListSource;
	std::vector<rInterface*> *ListInterface;
	std::vector<rReport*>*    m_listReport;
	std::vector<rLink*>       ListLink; //TODO Нужно ли это оставлять тут, или перенести в rDataManager?
	std::vector<rLinkTotal>   m_listTotals;

	UDINT LoadSecurity  (tinyxml2::XMLElement* root, tinyxml2::XMLDocument& doc_security);
	UDINT loadHardware  (tinyxml2::XMLElement* root);
	UDINT LoadConfig    (tinyxml2::XMLElement* root);
	UDINT LoadStation   (tinyxml2::XMLElement* root, cJSON* jroot);
	UDINT loadStream    (tinyxml2::XMLElement* root, cJSON* jroot, rStation* owner, const std::string& prefix);
	UDINT loadIO        (tinyxml2::XMLElement* root, cJSON* jroot, rStation* owner, const std::string& prefix);
	UDINT loadCalc      (tinyxml2::XMLElement* root, cJSON* jroot, rStation* owner, const std::string& prefix);
	UDINT loadReport    (tinyxml2::XMLElement* root);
	UDINT LoadCustom    (tinyxml2::XMLElement* root);
	UDINT LoadVariable  (tinyxml2::XMLElement* root);
	UDINT LoadUsers     (tinyxml2::XMLElement* root, cJSON* jroot);
	UDINT LoadComms     (tinyxml2::XMLElement* root);
	UDINT LoadModbus    (tinyxml2::XMLElement* root);
	UDINT LoadModbusTCP (tinyxml2::XMLElement* root);
	UDINT LoadOPCUA     (tinyxml2::XMLElement* root);
	UDINT loadSettings  (tinyxml2::XMLElement* root);

	UDINT checkMaxCount();
	UDINT checkSource(void);
	UDINT ResolveLinks(void);
	UDINT resolveReports(void);

	void  saveWeb();

public:
	std::string FileName;
	rError      m_error;
};

