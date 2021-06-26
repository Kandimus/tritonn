#include <iostream>

#include "tritonn_version.h"
#include "def.h"
#include "hash.h"
#include "simpleargs.h"
#include "simplefile.h"
#include "stringex.h"
#include "xml_util.h"
#include "xmlfile.h"
#include "bashcolor.h"

const char* SIGNATURE          = "signature";
const char  SIGNATURE_SHORT    = 's';
const char* SALT               = "salt";
const char  SALT_SHORT         = 'S';
const char* REPORT             = "report";
const char  REPORT_SHORT       = 'r';
const char* CONFIG             = "config";
const char  CONFIG_SHORT       = 'c';
const char* EXTRACTUSERS       = "extract-users";
const char  EXTRACTUSERS_SHORT = 'u';
const char* NOCOLOR            = "no-color";
const char  NOCOLOR_SHORT      = 'n';
const char* OUTPUT             = "output";
const char  OUTPUT_SHORT       = 'o';

bool color = true;

void checkXml()
{
	tinyxml2::XMLDocument doc;
	std::string           salt = rSimpleArgs::instance().getOption(SALT);

	if (rSimpleArgs::instance().isSet(REPORT) && rSimpleArgs::instance().isSet(CONFIG)) {
		printf("%sError: --config and --report cannot be applied together%s\n\n",
			   rBashColor::red(color).c_str(), rBashColor::reset(color).c_str());
		return;
	}

	if (rSimpleArgs::instance().isSet(REPORT)) {
		salt = XMLREPORT_HASH_SALT;
	}

	if (rSimpleArgs::instance().isSet(CONFIG)) {
		salt = XMLCONFIG_HASH_SALT;
	}

	UDINT result = xmlFileCheck(rSimpleArgs::instance().getArgument(0), doc, rSimpleArgs::instance().getOption(SIGNATURE), salt);

	switch(result) {
		case TRITONN_RESULT_OK:
			printf("%sSuccess: hash is correct%s\n\n",
				   rBashColor::green(color).c_str(), rBashColor::reset(color).c_str());
			break;

		case XMLFILE_RESULT_NFHASH:
			printf("%sError %u: not find signature%s\n\n",
				   rBashColor::red(color).c_str(), result, rBashColor::reset(color).c_str());
			break;

		case XMLFILE_RESULT_BADHASH:
			printf("%sError %u: bad hash%s\n\n",
				   rBashColor::red(color).c_str(), result, rBashColor::reset(color).c_str());
			break;

		case XMLFILE_RESULT_NOTEQUAL:
			printf("%sError %u: hash is not equal%s\n\n",
				   rBashColor::red(color).c_str(), result, rBashColor::reset(color).c_str());
			break;

		case FILE_RESULT_NOTFOUND:
			printf("Error %u: file not found\n\n", result);
			break;

		default: printf("Error %u\n", result); break;
	}
}

void extractUsers()
{
	tinyxml2::XMLDocument doc;

	if (tinyxml2::XML_SUCCESS != doc.LoadFile(rSimpleArgs::instance().getArgument(0).c_str())) {
		printf("Error %u, line %u: %s\n", doc.ErrorID(), doc.ErrorLineNum(), doc.ErrorStr());
		return;
	}

	auto xml_root = doc.FirstChildElement(XmlName::TRITONN);
	if (!xml_root) {
		printf("%sError %u: Cant found TRITONN section%s\n",
			   rBashColor::red(color).c_str(), DATACFGERR_STRUCT, rBashColor::reset(color).c_str());
		return;
	}

	auto xml_security = xml_root->FirstChildElement(XmlName::SECURITY);
	if (!xml_security) {
		printf("%sError %u: Can't found SECURITY section%s\n",
			   rBashColor::red(color).c_str(), DATACFGERR_SECURITY_NF, rBashColor::reset(color).c_str());
		return;
	}

	UDINT       fault    = false;
	std::string aes_text = String_deletewhite(XmlUtils::getTextString(xml_security, "", fault));
	std::string xml_src  = "";

	if (fault || aes_text.empty()) {
		printf("%sError %u: Security is empty%s\n",
			   rBashColor::red(color).c_str(), DATACFGERR_SECURITY_DESCRYPT, rBashColor::reset(color).c_str());
		return;
	}

	if (DecryptEAS(aes_text, AES_KEY, AES_IV, xml_src)) {
		printf("%sError %u: Can't decrypt SECURITY section%s\n",
			   rBashColor::red(color).c_str(), DATACFGERR_SECURITY_DESCRYPT, rBashColor::reset(color).c_str());
		return;
	}

	simpleFileSave(rSimpleArgs::instance().getOption(OUTPUT), xml_src);
}

int main(int argc, const char* argv[])
{
	rSimpleArgs::instance()
			.addOption(SIGNATURE   , SIGNATURE_SHORT, "signature")
			.addOption(SALT        , SALT_SHORT     , "0123456789abcdefghijklmnoABCDEFGHIJKLMNO")
			.addOption(OUTPUT      , OUTPUT_SHORT   , "ttutil.output")
			.addSwitch(EXTRACTUSERS, EXTRACTUSERS_SHORT)
			.addSwitch(REPORT      , REPORT_SHORT)
			.addSwitch(CONFIG      , CONFIG_SHORT)
			.addSwitch(NOCOLOR     , NOCOLOR_SHORT);

	rSimpleArgs::instance().parse(argc, argv);

	printf("----------------------------------------------------------------------------------------------\n");
	printf("ttutil %i.%i.%i.%x (C) VeduN, 2021 RSoft, OZNA\n\n", TRITONN_VERSION_MAJOR, TRITONN_VERSION_MINOR, TRITONN_VERSION_BUILD, TRITONN_VERSION_HASH);

	color = !rSimpleArgs::instance().isSet(NOCOLOR);

	if (rSimpleArgs::instance().isSet(REPORT) || rSimpleArgs::instance().isSet(CONFIG)) {
		checkXml();
	} else if (rSimpleArgs::instance().isSet(EXTRACTUSERS)) {
		extractUsers();
	} else {
		printf("Usage:\n");
		printf("\t--%s -%c\t- validate report file\n", REPORT, REPORT_SHORT);
		printf("\t--%s -%c\t- validate config file\n", CONFIG, CONFIG_SHORT);
		printf("\t--%s -%c\t- extract users from config without check hash\n", EXTRACTUSERS, EXTRACTUSERS_SHORT);
	}

	return 0;
}
