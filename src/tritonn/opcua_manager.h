//=================================================================================================
//===
//=== opcua_manager.h
//===
//=== Copyright (c) 2020 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Основной класс-нить для работы с OPC UA
//===
//=================================================================================================

#pragma once

#include <vector>
#include <open62541.h>
#include "thread_class.h"
#include "structures.h"
#include "data_interface.h"
#include "data_snapshot.h"

class rVariableList;
class rDataConfig;

struct rOPCVarLink
{
	rOPCVarLink(const rSnapshotItem *item, UA_NodeId *node, const UA_DataType *type);

	USINT                Value[8];
	UA_NodeId            Node;
	const rSnapshotItem *Item;
	const UA_DataType   *UAType;
};


//-------------------------------------------------------------------------------------------------
//
class rOPCUAManager : public rThreadClass, public rInterface
{
public:
	rOPCUAManager();
	virtual ~rOPCUAManager();


// Наследование от rInterface
public:
	virtual UDINT loadFromXML(tinyxml2::XMLElement *xml_root, rDataConfig &cfg);
	virtual UDINT generateVars(rVariableList &list);
	virtual UDINT CheckVars(rDataConfig &cfg);
	virtual UDINT StartServer();
	virtual rThreadClass *GetThreadClass();


// Методы
public:
	UDINT GetNodeValue(const UA_NodeId *node, UA_DataValue *dataValue);
	UDINT SetNodeValue(const UA_NodeId *node, const UA_DataValue *dataValue);

protected:
	virtual rThreadStatus Proccesing();

private:
	UA_Server               *OPCServer;
	UA_ServerConfig          OPCServerConf;
	UA_UsernamePasswordLogin Logins[4];
	UDINT                    LoginsCount;
	UDINT                    LoginAnonymous;
	UA_ByteString            OPCCertificate;
	UA_ByteString            OPCPrivateKey;

	static const string      Lang;
	static const string      RootName;

	std::vector<rOPCVarLink> OPCVarLink;
	rSnapshot                m_snapshot;

protected:
	const UA_DataType *GetTypeUA(const rVariable* var);
	UDINT AddAllVariables();
	UDINT AddVariable(const rSnapshotItem* ssitem);
	UDINT AddFolder(const std::string& path);
};





