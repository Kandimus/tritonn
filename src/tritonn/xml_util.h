//=================================================================================================
//===
//=== xml_util.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Вспомогательные функции для работы с XML
//===
//=================================================================================================

#pragma once

#include <string>
#include "tinyxml2.h"
#include "def.h"

#define XML_FOR(var, root, name)       for (tinyxml2::XMLElement* var = root->FirstChildElement(name); var != nullptr; var = var->NextSiblingElement(name))
#define XML_FOR_ALL(var, root)         for (tinyxml2::XMLElement* var = root->FirstChildElement();     var != nullptr; var = var->NextSiblingElement())

namespace XmlUtils
{
USINT       getAttributeUSINT (tinyxml2::XMLElement *element, const std::string &name, const USINT def);
UDINT       getAttributeUDINT (tinyxml2::XMLElement *element, const std::string &name, const UDINT def);
std::string getAttributeString(tinyxml2::XMLElement *element, const std::string &name, const std::string &def);

std::string getTextString(tinyxml2::XMLElement *element, const std::string& def, UDINT &err);
LREAL       getTextLREAL (tinyxml2::XMLElement *element, LREAL def, UDINT &err);
UDINT       getTextUDINT (tinyxml2::XMLElement *element, UDINT def, UDINT &err);
UINT        getTextUINT  (tinyxml2::XMLElement *element, UINT  def, UDINT &err);
USINT       getTextUSINT (tinyxml2::XMLElement *element, USINT def, UDINT &err);
USINT       getTextBOOL  (tinyxml2::XMLElement *element, bool  def, UDINT &err);

}

namespace XmlName
{

const char ACCESS[]        = "access";
//const char ADDRESS[]       = "address";
const char ADDRESSBLOCK[]  = "addressblock";
const char ADDRESSMAP[]    = "addressmap";
const char AI[]            = "ai";
const char ALIAS[]         = "alias";
const char ANONYMOUS[]     = "anonymous";
const char B[]             = "b";
const char B15[]           = "b15";
//const char BLOCK[]         = "block";
//const char BLOCKS[]        = "blocks";
const char BEGIN[]         = "begin";
const char BYTE[]          = "byte";
const char CALC[]          = "calculate";
const char CALIBR[]        = "calibration";
const char CONFIG[]        = "config";
const char COUNTER[]       = "counter";
const char DI[]            = "di";
const char DO[]            = "do";
const char FI[]            = "counter";
const char CHANNEL[]       = "channel";
const char COMMS[]         = "comms";
const char COUNTERR[]      = "counterror";
const char CPL[]           = "cpl";
const char CTL[]           = "ctl";
const char CURRENT[]       = "current";
const char CUSTOM[]        = "custom";
const char DATABLOCK[]     = "datablock";
const char DATABLOCKS[]    = "datablocks";
const char DATAMAP[]       = "datamap";
const char DATASET[]       = "dataset";
const char DATASETS[]      = "datasets";
const char DEFAULT[]       = "default";
const char DENSITY[]       = "density";
const char DENSITY15[]     = "density15";
const char DENSITY20[]     = "density20";
const char DENSSOL[]       = "densitometer";
const char DESC[]          = "description";
const char DWORD[]         = "dword";
const char EVENTS[]        = "events";
const char EXTERNAL[]      = "external";
const char FACTORS[]       = "factors";
const char FAULT[]         = "fault";
const char FAULTS[]        = "faults";
const char FAULTVAL[]      = "faultvalue";
const char FAULTVALS[]     = "faultvalues";
const char FLOWMETER[]     = "flowmeter";
const char FLOWRATEMASS[]  = "flowrate.mass";
const char FLOWRATEVOL[]   = "flowrate.volume";
const char FLOWRATEVOL15[] = "flowrate.volume15";
const char FLOWRATEVOL20[] = "flowrate.volume20";
const char FREQ[]          = "frequency";
const char HARDWARE[]      = "hardware";
const char HI[]            = "hi";
const char HIHI[]          = "hihi";
const char HYSTER[]        = "hysteresis";
const char HERTZ[]         = "hz";
const char GROUP[]         = "group";
const char ID[]            = "id";
const char IMPULSE[]       = "impulse";
const char INPUTS[]        = "inputs";
//const char CFGNAME_INTERFACE[]     = "interface";
//const char CFGNAME_INTERFACES[]    = "interfaces";
const char INTERNAL[]      = "internal";
const char IO[]            = "io";
const char IOLINK[]        = "iolink";
const char IP[]            = "ip";
const char ITEM[]          = "item";
const char KEYPAD[]        = "keypad";
const char KEYPADS[]       = "keypads";
const char KEYPAD_KF[]     = "keypad_kf";
const char KEYPAD_MF[]     = "keypad_mf";
const char KF[]            = "kf";
const char LANG[]          = "lang";
const char LIMIT[]         = "limit";
const char LIMITS[]        = "limits";
const char LINEARIZATION[] = "linearization";
const char LINK[]          = "link";
const char LO[]            = "lo";
const char LOGIN[]         = "login";
const char LOLO[]          = "lolo";
const char MASTERRTU[]     = "masterrtu";
const char MASTERTCP[]     = "mastertcp";
const char MAINTENANCE[]   = "maintenance";
const char MAX[]           = "max";
const char MIN[]           = "min";
const char MODBUS[]        = "modbus";
const char MODE[]          = "mode";
const char MODULE[]        = "module";
const char MSELECTOR[]     = "multiselector";
const char NAME[]          = "name";
const char NAMES[]         = "names";
const char NUMBER[]        = "number";
const char OPCUA[]         = "opcua";
const char PASSWORD[]      = "password";
const char PERIOD[]        = "period";
const char PHYSICAL[]      = "physical";
const char POINT[]         = "point";
const char POINTS[]        = "points";
const char PORT[]          = "port";
const char PRECISION[]     = "precision";
const char PRESENT[]       = "present";
const char PRES[]          = "pressure";
const char PRODUCT[]       = "product";
const char PROPERTIES[]    = "properties";
const char REDUCEDDENS[]   = "reduceddens";
const char REPORT[]        = "report";
const char REPORTS[]       = "reports";
const char REPORTSYSTEM[]  = "reportsystem";
const char RIGHTS[]        = "rights";
const char SAMPLER[]       = "sampler";
const char SCALE[]         = "scale";
const char SECURITY[]      = "security";
const char SELECTOR[]      = "selector";
const char SETUP[]         = "setup";
const char SLAVERTU[]      = "slavertu";
const char SLAVETCP[]      = "slavetcp";
const char SNAPSHOTS[]     = "snapshots";
const char STATIONS[]      = "stations";
const char STATION[]       = "station";
const char STORAGE[]       = "storage";
const char STR[]           = "str";
const char STREAMS[]       = "streams";
const char STREAM[]        = "stream";
const char STRINGS[]       = "strings";
const char SWAP[]          = "swap";
const char TOTAL[]         = "total";
const char TRITONN[]       = "tritonn";
const char TEMP[]          = "temperature";
const char TYPE[]          = "type";
const char UNIT[]          = "unit";
const char USER[]          = "user";
const char USERS[]         = "users";
const char VALUE[]         = "value";
const char VARIABLE[]      = "variable";
const char VARIABLES[]     = "variables";
const char VARS[]          = "vars";
const char WHITELIST[]     = "whitelist";
const char WORD[]          = "word";
const char Y[]             = "y";
const char Y15[]           = "y15";

}

