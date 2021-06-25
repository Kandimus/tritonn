#include <iostream>

#include "tritonn_version.h"
#include "def.h"
#include "hash.h"
#include "simpleargs.h"
#include "xmlfile.h"
#include "bashcolor.h"

const char* SIGNATURE = "signature";
const char* SALT      = "salt";
const char* REPORT    = "report";
const char* CONFIG    = "config";
const char* NOCOLOR   = "no-color";

int main(int argc, const char* argv[])
{
	rSimpleArgs::instance()
			.addOption(SIGNATURE, 's', "signature")
			.addOption(SALT     , 'S', "0123456789abcdefghijklmnoABCDEFGHIJKLMNO")
			.addSwitch(REPORT   , 'r')
			.addOption(CONFIG   , 'c', XMLCONFIG_HASH_SALT)
			.addSwitch(NOCOLOR  , 'n');

	rSimpleArgs::instance().parse(argc, argv);

	printf("----------------------------------------------------------------------------------------------\n");
	printf("checkxml %i.%i.%i.%x (C) VeduN, 2021 RSoft, OZNA\n\n", TRITONN_VERSION_MAJOR, TRITONN_VERSION_MINOR, TRITONN_VERSION_BUILD, TRITONN_VERSION_HASH);

	tinyxml2::XMLDocument doc;
	std::string           salt = rSimpleArgs::instance().getOption(SALT);

	bool c = !rSimpleArgs::instance().isSet(NOCOLOR);

	if (rSimpleArgs::instance().isSet(REPORT) && rSimpleArgs::instance().isSet(CONFIG)) {
		printf("%sError: --config and --report cannot be applied together%s\n\n",
			   rBashColor::red(c).c_str(), rBashColor::reset(c).c_str());
		return 1;
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
				   rBashColor::green(c).c_str(), rBashColor::reset(c).c_str());
			break;

		case XMLFILE_RESULT_NFHASH:
			printf("%sError %u: not find signature%s\n\n",
				   rBashColor::red(c).c_str(), result, rBashColor::reset(c).c_str());
			break;

		case XMLFILE_RESULT_BADHASH:
			printf("%sError %u: bad hash%s\n\n",
				   rBashColor::red(c).c_str(), result, rBashColor::reset(c).c_str());
			break;

		case XMLFILE_RESULT_NOTEQUAL:
			printf("%sError %u: hash is not equal%s\n\n",
				   rBashColor::red(c).c_str(), result, rBashColor::reset(c).c_str());
			break;

		case FILE_RESULT_NOTFOUND:
			printf("Error %u: file not found\n\n", result);
			break;

		default: printf("Error %u\n", result); break;
	}

	return 0;
}
