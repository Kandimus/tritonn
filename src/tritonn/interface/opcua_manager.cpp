///=================================================================================================
//===
//=== opcua_manager.cpp
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


#include "opcua_manager.h"
#include <string.h>
#include "tritonn_version.h"
#include "locker.h"
#include "tickcount.h"
#include "stringex.h"
#include "error.h"
#include "log_manager.h"
#include "xml_util.h"
#include "../data_snapshot_item.h"
#include "../data_manager.h"
#include "../data_config.h"
#include "../variable_item.h"
#include "../variable_list.h"
#include "../generator_md.h"

#define ANSI_COLOR_RESET   "\x1b[0m"

extern const char *logCategoryNames[7];
extern const char *logLevelNames[6];
extern void OPCServer_log(void *context, UA_LogLevel level, UA_LogCategory category, const char *msg, va_list args);

//const char *logLevelNames[6] = {"trace", "debug", "info", "warn", "error", "fatal"};
UDINT       log2Mask[6]      = {   LOG::I,    LOG::I,   LOG::I,   LOG::W,    LOG::A,    LOG::P};

const UA_Logger OPCServer_Logger = {OPCServer_log, NULL, UA_Log_Stdout_clear};


void OPCServer_log(void *context, UA_LogLevel level, UA_LogCategory category, const char *msg, va_list args)
{
	/* Assume that context is casted to UA_LogLevel */
	/* TODO we may later change this to a struct with bitfields to filter on category */
	if(context != NULL && (UA_LogLevel)(uintptr_t)context > level)
	{
		return;
	}

	string logmsg  = String_vaformat(msg, args);
	string logtext = String_format("OPC UA [%s/%s]" ANSI_COLOR_RESET " %s", logLevelNames[level], logCategoryNames[category], logmsg.c_str());

	rLogManager::instance().add(log2Mask[level], __FILENAME__, __LINE__, logtext.c_str());
}


//TODO Нужно переделать на SimpleFile
UA_ByteString UA_loadFile(const char *const path)
{
	UA_ByteString fileContents = UA_STRING_NULL;

	/* Open the file */
	FILE *fp = fopen(path, "rb");
	if(!fp)
	{
		errno = 0; /* We read errno also from the tcp layer... */
		return fileContents;
	}

	/* Get the file length, allocate the data and read */
	fseek(fp, 0, SEEK_END);
	fileContents.length = (size_t)ftell(fp);
	fileContents.data   = (UA_Byte *)UA_malloc(fileContents.length * sizeof(UA_Byte));
	if(fileContents.data)
	{
		fseek(fp, 0, SEEK_SET);
		size_t read = fread(fileContents.data, sizeof(UA_Byte), fileContents.length, fp);
		if(read != fileContents.length)
		{
			UA_ByteString_clear(&fileContents);
		}
	}
	else
	{
		fileContents.length = 0;
	}
	fclose(fp);

	return fileContents;
}


UA_StatusCode readCurrentTime(UA_Server */*server*/, const UA_NodeId */*sessionId*/, void */*sessionContext*/, const UA_NodeId *nodeId, void *nodeContext,
										UA_Boolean /*sourceTimeStamp*/, const UA_NumericRange */*range*/, UA_DataValue *dataValue)
{
	rOPCUAManager *opcua = (rOPCUAManager *)nodeContext;

	if(nullptr == opcua) return UA_STATUSCODE_BADINTERNALERROR;

	return opcua->GetNodeValue(nodeId, dataValue);
}



UA_StatusCode writeCurrentTime(UA_Server */*server*/, const UA_NodeId */*sessionId*/, void */*sessionContext*/, const UA_NodeId *nodeId, void *nodeContext,
										 const UA_NumericRange *range, const UA_DataValue *dataValue)
{
	UNUSED(range);

	rOPCUAManager* opcua = (rOPCUAManager *)nodeContext;

	if (!opcua) {
		return UA_STATUSCODE_BADINTERNALERROR;
	}

	 return opcua->SetNodeValue(nodeId, dataValue);
}




rOPCVarLink::rOPCVarLink(const rSnapshotItem *item, UA_NodeId *node, const UA_DataType *type)
{
	Item   = item;
	UAType = type;

	UA_NodeId_copy(node, &Node);
	memset(Value, 0, sizeof(Value));
}



const string rOPCUAManager::Lang     = "en-EN";
const string rOPCUAManager::RootName = "tritonn";



//TODO Вынести создания Сервера в отдельную процедуру!!!
rOPCUAManager::rOPCUAManager(bool createopcua)
	: rInterface(Mutex),
	  m_snapshot(rDataManager::instance().getVariableClass())
{
	RTTI      = "rOPCUAManager";
	OPCServer = nullptr;

	memset(&Logins, 0, sizeof(Logins));
	LoginsCount    = 0;
	LoginAnonymous = 0;

	if(!createopcua) {
		return;
	}

	OPCCertificate = UA_loadFile("./cert/tritonn_cert.der");
	OPCPrivateKey  = UA_loadFile("./cert/tritonn_key.der");

	UA_ServerConfig_setDefaultWithSecurityPolicies(&OPCServerConf, 4840, &OPCCertificate, &OPCPrivateKey, nullptr, 0, nullptr, 0, nullptr, 0);

	OPCServerConf.logger = OPCServer_Logger;
	UA_ServerConfig_setCustomHostname(&OPCServerConf, UA_STRING_STATIC("0.0.0.0"));
	//TODO Нужно указать корректный адрес "/tritonn", через Endpoint или discovery?

	// Исправляем UA_BuildInfo
	UA_BuildInfo_clear(&OPCServerConf.buildInfo);
	OPCServerConf.buildInfo.productUri       = UA_STRING_ALLOC("http://tritonn.ozna.ru");
	OPCServerConf.buildInfo.manufacturerName = UA_STRING_ALLOC("tritonn");
	OPCServerConf.buildInfo.productName      = UA_STRING_ALLOC("Tritonn OPC UA Server");
	OPCServerConf.buildInfo.softwareVersion  = UA_STRING_ALLOC(TRITONN_VERSION);
	OPCServerConf.buildInfo.buildNumber      = UA_STRING_ALLOC(TRITONN_COMPILE_DATE " " TRITONN_COMPILE_TIME);
	OPCServerConf.buildInfo.buildDate        = (uint64_t(TRITONN_COMPILE_UNIX) * UA_DATETIME_SEC) + UA_DATETIME_UNIX_EPOCH;

	UA_ApplicationDescription_clear(&OPCServerConf.applicationDescription);
	OPCServerConf.applicationDescription.applicationUri  = UA_STRING_ALLOC("urn:tritonn.server.application");
	OPCServerConf.applicationDescription.productUri      = UA_STRING_ALLOC("urn:tritonn.application");
	OPCServerConf.applicationDescription.applicationName = UA_LOCALIZEDTEXT_ALLOC("en", "Tritonn Application");
	OPCServerConf.applicationDescription.applicationType = UA_APPLICATIONTYPE_SERVER;
}

//-------------------------------------------------------------------------------------------------
//
rThreadStatus rOPCUAManager::Proccesing()
{
	UA_Boolean waitInternal  = false;
	rTickCount tick;

	tick.start(500);

	while(true)
	{
		// Обработка команд нити
		rThreadStatus thread_status = rThreadClass::Proccesing();
		if(!THREAD_IS_WORK(thread_status))
		{
			return thread_status;
		}


		if(tick.isFinished())
		{
			rLocker lock(Mutex); lock.Nop();

			m_snapshot.resetAssign();

			m_snapshot.get();
			tick.restart();

			rVariableClass::processing();
			rThreadClass::EndProccesing();
		}

		UA_UInt16 timeout = UA_Server_run_iterate(OPCServer, waitInternal);
		Delay.Set(timeout);
	} // while
}




//-------------------------------------------------------------------------------------------------
UDINT rOPCUAManager::startServer()
{
	UA_StatusCode result = TRITONN_RESULT_OK;
//	rLocker lock(Mutex); lock.Nop();

	if(OPCServer != nullptr)
	{
		UA_Server_run_shutdown(OPCServer);
		UA_Server_delete(OPCServer);
		OPCServer = nullptr;
	}

	// Настраиваем уровни доступа
	OPCServerConf.accessControl.clear(&OPCServerConf.accessControl);
	UA_StatusCode retval = UA_AccessControl_default(&OPCServerConf, LoginAnonymous, &OPCServerConf.securityPolicies[OPCServerConf.securityPoliciesSize - 1].policyUri, LoginsCount, Logins);
	if(retval != UA_STATUSCODE_GOOD)
	{
		//TODO Event
		rDataManager::instance().DoHalt(HaltReason::OPC, retval);
		return retval;
	}

	//
	OPCServer = UA_Server_newWithConfig(&OPCServerConf);


	AddFolder(RootName);

	m_snapshot.getAllVariables();
	//m_snapshot.add("hardware.ai6_1.ch_01.simulate.value");
	result = AddAllVariables();

	if (result != TRITONN_RESULT_OK) {
		rDataManager::instance().DoHalt(HaltReason::OPC, result);
		return result;
	}

	// Запускаем OPC сервер
	result = UA_Server_run_startup(OPCServer);
	if (result != UA_STATUSCODE_GOOD) {
		UA_Server_run_shutdown(OPCServer);
		UA_Server_delete(OPCServer);
		OPCServer = nullptr;

		rDataManager::instance().DoHalt(HaltReason::OPC, result);

		return result;
	}

	rThreadClass::Run(300);

	return TRITONN_RESULT_OK;
}


rThreadClass *rOPCUAManager::getThreadClass()
{
	return (rThreadClass *)this;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rOPCUAManager::AddAllVariables()
{
	UDINT result = TRITONN_RESULT_OK;

	for(auto item : m_snapshot) {
		result = AddVariable(item);

		if(TRITONN_RESULT_OK != result) return result;
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rOPCUAManager::AddVariable(const rSnapshotItem *ssitem)
{
	string path       = "";
	string parentpath = "";
	string name       = "";
	DINT   pos        = -1;
	UDINT  result     = TRITONN_RESULT_OK;

	if(nullptr == ssitem)
	{
		return OPCUA_ERROR_VARNF;
	}

	const rVariable*   var      = ssitem->getVariable();
	const UA_DataType *datatype = nullptr;

	if(nullptr == var)
	{
		return OPCUA_ERROR_VARNF;
	}

	path = RootName + "." + var->getName();
	name = var->getName();
	pos  = path.rfind('.');

	if(pos > 0)
	{
		parentpath = path.substr(0, pos);
		name       = path.substr(pos + 1);

		result = AddFolder(parentpath);

		if(result != TRITONN_RESULT_OK) return result;
	}

	datatype = GetTypeUA(var);
	if(nullptr == datatype)
	{
		return OPCUA_ERROR_BADVARTYPE;
	}

	UA_VariableAttributes attr     = UA_VariableAttributes_default;
	UA_NodeId             NodeId   = UA_NODEID_STRING(1, (char *)path.c_str());
	UA_NodeId             ParentId = UA_NODEID_STRING(1, (char *)parentpath.c_str());
	UA_QualifiedName      QualName = UA_QUALIFIEDNAME(1, (char *)name.c_str());
	UA_NodeId             TypeId   = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
	UA_NodeId             ReferId  = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
	UA_DataSource         datasrc;

	OPCVarLink.push_back(rOPCVarLink(ssitem, &NodeId, datatype));

//	UA_Variant_setScalar(&attr.value, OPCVarLink.back().Value, datatype);
	attr.description = UA_LOCALIZEDTEXT((char *)Lang.c_str(), (char *)name.c_str());
	attr.displayName = UA_LOCALIZEDTEXT((char *)Lang.c_str(), (char *)name.c_str());
//	attr.dataType    = datatype->typeId;
	attr.accessLevel = UA_ACCESSLEVELMASK_READ;

	if(!var->isReadonly())
	{
		attr.accessLevel |= UA_ACCESSLEVELMASK_WRITE;
	}

	// Add the variable node to the information model
	/*
	UA_Server_addVariableNode(OPCServer,
		NodeId,                                           // Уникальный Id тега/папки
		ParentId,                                         // Родительский узел
		UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
		QualName,
		UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
		attr,
		NULL, NULL);
	*/
	datasrc.read  = readCurrentTime;
	datasrc.write = writeCurrentTime;
	result        = UA_Server_addDataSourceVariableNode(OPCServer, NodeId, ParentId, ReferId, QualName, TypeId, attr, datasrc, this, NULL);

	return result;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rOPCUAManager::AddFolder(const string &path)
{
	string    parentpath  = "";
	string    name        = path;
	DINT      pos         = -1;
	UDINT     result      = TRITONN_RESULT_OK;
	UA_NodeId ParentId    = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
	void     *NodeContext = nullptr;

	// Проверяем, существует ли уже такой узел
	result = UA_Server_getNodeContext(OPCServer, UA_NODEID_STRING(1, (char *)path.c_str()), &NodeContext);
	if(TRITONN_RESULT_OK == result)
	{
		return result;
	}

	pos = path.rfind('.');

	if(pos > 0)
	{
		parentpath = path.substr(0, pos);
		name       = path.substr(pos + 1);
		ParentId   = UA_NODEID_STRING(1, (char *)parentpath.c_str());
		result     = AddFolder(parentpath);

		if(result != TRITONN_RESULT_OK) return result;
	}


	UA_ObjectAttributes attr = UA_ObjectAttributes_default;
	attr.description = UA_LOCALIZEDTEXT((char *)Lang.c_str(), (char *)name.c_str());
	attr.displayName = UA_LOCALIZEDTEXT((char *)Lang.c_str(), (char *)name.c_str());
	result = UA_Server_addObjectNode(OPCServer,
		UA_NODEID_STRING(1, (char *)path.c_str()),           // Уникальный Id тега/папки
		ParentId,                                            // Родительский узел
		UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),            // Указатель на тип узла?
		UA_QUALIFIEDNAME(1, (char *)name.c_str()),           // Имя узла (NodeId)
		UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),       // Тип узла
		attr,                                                // Атрибуты узла
		this,                                                // UserData
		NULL);

	return result;
}


const UA_DataType *rOPCUAManager::GetTypeUA(const rVariable *var)
{
	if(nullptr == var) return nullptr;

	switch(var->getType())
	{
		case TYPE::SINT:  return &UA_TYPES[UA_TYPES_SBYTE];
		case TYPE::USINT: return &UA_TYPES[UA_TYPES_BYTE];
		case TYPE::INT:   return &UA_TYPES[UA_TYPES_INT16];
		case TYPE::UINT:  return &UA_TYPES[UA_TYPES_UINT16];
		case TYPE::DINT:  return &UA_TYPES[UA_TYPES_INT32];
		case TYPE::UDINT: return &UA_TYPES[UA_TYPES_UINT32];
		case TYPE::REAL:  return &UA_TYPES[UA_TYPES_FLOAT];
		case TYPE::LREAL: return &UA_TYPES[UA_TYPES_DOUBLE];
		case TYPE::STRID: return &UA_TYPES[UA_TYPES_UINT32];

		default: return nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
UDINT rOPCUAManager::loadFromXML(tinyxml2::XMLElement* xml_root, rError& err)
{
	tinyxml2::XMLElement* xml_properties = nullptr;

	rInterface::loadFromXML(xml_root, err);

	m_alias = "comms.opcua";

	if (!xml_root) {
		return TRITONN_RESULT_OK;
	}

	xml_properties = xml_root->FirstChildElement(XmlName::PROPERTIES);

	if (xml_properties) {
		UDINT fault = 0;
		LoginAnonymous = XmlUtils::getTextUDINT(xml_properties->FirstChildElement(XmlName::ANONYMOUS), LoginAnonymous, fault);
	}

	// Считываем пользователей
	tinyxml2::XMLElement *xml_security_root = rDataConfig::instance().GetRootSecurity();
	tinyxml2::XMLElement *xml_opcua         = nullptr;

	if (!xml_security_root) {
		return err.set(DATACFGERR_OPCUA_USER_NF, 0, "");
	}

	xml_opcua = xml_security_root->FirstChildElement(XmlName::OPCUA);

	if (!xml_opcua) {
		return err.set(DATACFGERR_OPCUA_USER_NF, xml_security_root->GetLineNum(), "");
	}

	LoginsCount = 0;
	XML_FOR(xml_user, xml_opcua, XmlName::USER) {
		UDINT       fault    = 0;
		std::string login    = XmlUtils::getTextString(xml_user->FirstChildElement(XmlName::LOGIN)   , "", fault);
		std::string password = XmlUtils::getTextString(xml_user->FirstChildElement(XmlName::PASSWORD), "", fault);

		if("" == login || "" == password) {
			return err.set(DATACFGERR_OPCUA_BAD_USER, xml_security_root->GetLineNum(), "");
		}

		Logins[LoginsCount].username = UA_STRING_ALLOC(login.c_str());
		Logins[LoginsCount].password = UA_STRING_ALLOC(password.c_str());
		++LoginsCount;

		if(LoginsCount >= 4)
		{
			break;
		}
	}

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rOPCUAManager::GetNodeValue(const UA_NodeId *node, UA_DataValue *dataValue)
{
	for(UDINT ii = 0; ii < OPCVarLink.size(); ++ii)
	{
		rOPCVarLink *link = &OPCVarLink[ii];

		if(UA_NodeId_equal(&link->Node, node))
		{
			rLocker lock(Mutex); lock.Nop();

			link->Item->getBuffer(link->Value);
			UA_Variant_setScalarCopy(&dataValue->value, link->Value, link->UAType);
			dataValue->hasValue = true;

			return UA_STATUSCODE_GOOD;
		}
	}

	return UA_STATUSCODE_BADINTERNALERROR;
}



UDINT rOPCUAManager::SetNodeValue(const UA_NodeId *node, const UA_DataValue *dataValue)
{
	for(UDINT ii = 0; ii < OPCVarLink.size(); ++ii)
	{
		rOPCVarLink *link = &OPCVarLink[ii];

		if(UA_NodeId_equal(&link->Node, node))
		{
			if(dataValue->hasValue && UA_Variant_isScalar(&dataValue->value) && dataValue->value.type == link->UAType && dataValue->value.data)
			{
				rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);
				rLocker lock(Mutex); lock.Nop();

				ss.add(link->Item->getVariable()->getName(), dataValue->value.data);

				ss.set();

				if(!ss[0]->isWrited()) {
					return UA_STATUSCODE_BADNOTWRITABLE;
				}

				return UA_STATUSCODE_GOOD;
			}
		}
	}

	return UA_STATUSCODE_BADINTERNALERROR;
}

UDINT rOPCUAManager::generateVars(rVariableClass* parent)
{
	UNUSED(parent);

	return TRITONN_RESULT_OK;
}


UDINT rOPCUAManager::checkVars(rError& err)
{
	UNUSED(err);
	return TRITONN_RESULT_OK;
}

UDINT rOPCUAManager::generateMarkDown(rGeneratorMD& md)
{
	md.add(this)
			.addXml("<" + std::string(XmlName::PROPERTIES) + "> " + rGeneratorMD::rItem::XML_OPTIONAL)
			.addXml(XmlName::ANONYMOUS, LoginAnonymous)
			.addXml("</" + std::string(XmlName::PROPERTIES) + ">");

	return TRITONN_RESULT_OK;
}


