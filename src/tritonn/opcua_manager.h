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



using std::vector;


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
	virtual UDINT LoadFromXML(tinyxml2::XMLElement *xml_root, rDataConfig &cfg);
	//	virtual UDINT SaveKernel(FILE *file, const string &objname, const string &comment);
	virtual UDINT GenerateVars(vector<rVariable *> &list);
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

	vector<rOPCVarLink>      OPCVarLink;
	rSnapshot                Snapshot;

protected:
	const UA_DataType *GetTypeUA(const rVariable *var);
	UDINT AddAllVariables();
	UDINT AddVariable(const rSnapshotItem *ssitem);
	UDINT AddFolder(const string &path);
};





