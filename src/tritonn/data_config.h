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
#include "tinyxml2.h"
#include "cJSON.h"
#include "def.h"

using std::vector;

namespace tinyxml2
{
	class XMLElement;
}

class  rSource;
class  rLink;
class  rInterface;
class  rStation;
class  rReport;
struct rSystemVariable;


struct rBitFlag
{
	string Name;
	UDINT  Value;

	rBitFlag(const string &name, UDINT value) : Name(name), Value(value) {}
};


class rDataConfig
{
public:
	rDataConfig();
	virtual ~rDataConfig();

	string FileName;
	string ErrorStr;
	UDINT  ErrorID;
	UDINT  ErrorLine;
	string Prefix;

	UDINT LoadFile(const string &filename, rSystemVariable &sysvar, vector<rSource *> &listsrc, vector<rInterface *> &listiface, vector<rReport *> &listrpt);


public:
	static vector<rBitFlag> AIModeFlags;
	static vector<rBitFlag> AISetupFlags;
	static vector<rBitFlag> FISetupFlags;
	static vector<rBitFlag> STNProductValues;
	static vector<rBitFlag> STRFMeterFlags;
	static vector<rBitFlag> SelectorSetupFlags;
	static vector<rBitFlag> SelectorModeFlags;
	static vector<rBitFlag> LimitSetupFlags;
	static vector<rBitFlag> ReportTypeFlags;
	static vector<rBitFlag> ReportPeriodFlags;
	static vector<rBitFlag> VarSetupFlags;

	static void   InitBitFlags();
	static string GetFlagNameByBit  (vector<rBitFlag> &arr, UDINT value);
	static string GetFlagNameByValue(vector<rBitFlag> &arr, UDINT value);
	static UDINT  GetFlagBitByStr   (vector<rBitFlag> &arr, const string &name, UDINT &err);
	static UDINT  GetFlagFromStr    (vector<rBitFlag> &arr, const string &str, UDINT &err);

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
	vector<rSource *>    *ListSource;
	vector<rInterface *> *ListInterface;
	vector<rReport *>    *ListReport;
	vector<rLink   *>     ListLink; //TODO Нужно ли это оставлять тут, или перенести в rDataManager?

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





const char CFGNAME_ACCESS[]        = "access";
//const char CFGNAME_ADDRESS[]       = "address";
const char CFGNAME_ADDRESSBLOCK[]  = "addressblock";
const char CFGNAME_ADDRESSMAP[]    = "addressmap";
const char CFGNAME_AI[]            = "ai";
const char CFGNAME_ALIAS[]         = "alias";
const char CFGNAME_ANONYMOUS[]     = "anonymous";
const char CFGNAME_B[]             = "b";
const char CFGNAME_B15[]           = "b15";
//const char CFGNAME_BLOCK[]         = "block";
//const char CFGNAME_BLOCKS[]        = "blocks";
const char CFGNAME_BEGIN[]         = "begin";
const char CFGNAME_BYTE[]          = "byte";
const char CFGNAME_CALC[]          = "calculate";
const char CFGNAME_CALIBR[]        = "calibration";
const char CFGNAME_CONFIG[]        = "config";
const char CFGNAME_COUNTER[]       = "counter";
const char CFGNAME_FI[]            = "counter";
const char CFGNAME_COMMS[]         = "comms";
const char CFGNAME_COUNTERR[]      = "counterror";
const char CFGNAME_CPL[]           = "cpl";
const char CFGNAME_CTL[]           = "ctl";
const char CFGNAME_CURRENT[]       = "current";
const char CFGNAME_CUSTOM[]        = "custom";
const char CFGNAME_DATABLOCK[]     = "datablock";
const char CFGNAME_DATAMAP[]       = "datamap";
const char CFGNAME_DATASET[]       = "dataset";
const char CFGNAME_DATASETS[]      = "datasets";
const char CFGNAME_DEFAULT[]       = "default";
const char CFGNAME_DENSITY[]       = "density";
const char CFGNAME_DENSITY15[]     = "density15";
const char CFGNAME_DENSITY20[]     = "density20";
const char CFGNAME_DENSSOL[]       = "densitometer";
const char CFGNAME_DESC[]          = "description";
const char CFGNAME_DWORD[]         = "dword";
const char CFGNAME_EVENTS[]        = "events";
const char CFGNAME_EXTERNAL[]      = "external";
const char CFGNAME_FACTORS[]       = "factors";
const char CFGNAME_FAULT[]         = "fault";
const char CFGNAME_FAULTS[]        = "faults";
const char CFGNAME_FAULTVAL[]      = "faultvalue";
const char CFGNAME_FAULTVALS[]     = "faultvalues";
const char CFGNAME_FLOWRATEMASS[]  = "flowrate.mass";
const char CFGNAME_FLOWRATEVOL[]   = "flowrate.volume";
const char CFGNAME_FLOWRATEVOL15[] = "flowrate.volume15";
const char CFGNAME_FLOWRATEVOL20[] = "flowrate.volume20";
const char CFGNAME_FREQ[]          = "frequency";
const char CFGNAME_HARDWARE[]      = "hardware";
const char CFGNAME_HI[]            = "hi";
const char CFGNAME_HIHI[]          = "hihi";
const char CFGNAME_HYSTER[]        = "hysteresis";
const char CFGNAME_HZ[]            = "hz";
const char CFGNAME_GROUP[]         = "group";
const char CFGNAME_ID[]            = "id";
const char CFGNAME_IMPULSE[]       = "impulse";
const char CFGNAME_INPUTS[]        = "inputs";
//const char CFGNAME_INTERFACE[]     = "interface";
//const char CFGNAME_INTERFACES[]    = "interfaces";
const char CFGNAME_INTERNAL[]      = "internal";
const char CFGNAME_IO[]            = "io";
const char CFGNAME_IP[]            = "ip";
const char CFGNAME_ITEM[]          = "item";
const char CFGNAME_KEYPAD[]        = "keypad";
const char CFGNAME_KEYPADS[]       = "keypads";
const char CFGNAME_KEYPAD_KF[]     = "keypad_kf";
const char CFGNAME_KEYPAD_MF[]     = "keypad_mf";
const char CFGNAME_KF[]            = "kf";
const char CFGNAME_LANG[]          = "lang";
const char CFGNAME_LIMIT[]         = "limit";
const char CFGNAME_LIMITS[]        = "limits";
const char CFGNAME_LINEARIZATION[] = "linearization";
const char CFGNAME_LINK[]          = "link";
const char CFGNAME_LO[]            = "lo";
const char CFGNAME_LOGIN[]         = "login";
const char CFGNAME_LOLO[]          = "lolo";
const char CFGNAME_MAINTENANCE[]   = "maintenance";
const char CFGNAME_MAX[]           = "max";
const char CFGNAME_MIN[]           = "min";
const char CFGNAME_MODBUS[]        = "modbus";
const char CFGNAME_MODE[]          = "mode";
const char CFGNAME_MODULE[]        = "module";
const char CFGNAME_MSELECTOR[]     = "multiselector";
const char CFGNAME_NAME[]          = "name";
const char CFGNAME_NAMES[]         = "names";
const char CFGNAME_OPCUA[]         = "opcua";
const char CFGNAME_PASSWORD[]      = "password";
const char CFGNAME_PERIOD[]        = "period";
const char CFGNAME_PHYSICAL[]      = "physical";
const char CFGNAME_POINT[]         = "point";
const char CFGNAME_POINTS[]        = "points";
const char CFGNAME_PORT[]          = "port";
const char CFGNAME_PRECISION[]     = "precision";
const char CFGNAME_PRESENT[]       = "present";
const char CFGNAME_PRES[]          = "pressure";
const char CFGNAME_PRODUCT[]       = "product";
const char CFGNAME_PROPERTIES[]    = "properties";
const char CFGNAME_REDUCEDDENS[]   = "reduceddens";
const char CFGNAME_REPORT[]        = "report";
const char CFGNAME_REPORTS[]       = "reports";
const char CFGNAME_REPORTSYSTEM[]  = "reportsystem";
const char CFGNAME_RIGHTS[]        = "rights";
const char CFGNAME_SAMPLER[]       = "sampler";
const char CFGNAME_SCALE[]         = "scale";
const char CFGNAME_SECURITY[]      = "security";
const char CFGNAME_SELECTOR[]      = "selector";
const char CFGNAME_SETUP[]         = "setup";
const char CFGNAME_SLAVETCP[]      = "slavetcp";
const char CFGNAME_SNAPSHOTS[]     = "snapshots";
const char CFGNAME_STATIONS[]      = "stations";
const char CFGNAME_STATION[]       = "station";
const char CFGNAME_STORAGE[]       = "storage";
const char CFGNAME_STR[]           = "str";
const char CFGNAME_STREAMS[]       = "streams";
const char CFGNAME_STREAM[]        = "stream";
const char CFGNAME_STRINGS[]       = "strings";
const char CFGNAME_SWAP[]          = "swap";
const char CFGNAME_TOTAL[]         = "total";
const char CFGNAME_TRITONN[]       = "tritonn";
const char CFGNAME_TEMP[]          = "temperature";
const char CFGNAME_TYPE[]          = "type";
const char CFGNAME_UNIT[]          = "unit";
const char CFGNAME_USER[]          = "user";
const char CFGNAME_USERS[]         = "users";
const char CFGNAME_VALUE[]         = "value";
const char CFGNAME_VARIABLE[]      = "variable";
const char CFGNAME_VARIABLES[]     = "variables";
const char CFGNAME_VARS[]          = "vars";
const char CFGNAME_WHITELIST[]     = "whitelist";
const char CFGNAME_WORD[]          = "word";
const char CFGNAME_Y[]             = "y";
const char CFGNAME_Y15[]           = "y15";




