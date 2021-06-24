#include <iostream>

#include "tritonn_version.h"
#include "hash.h"
#include "simpleargs.h"
#include "xmlfile.h"
#include "def.h"

int main(int argc, char* argv[])
{
	rSimpleArgs::instance()
			.addOption("signature", 's', "signature")
			.addOption("salt"     , 'S', "0123456789abcdefghijklmnoABCDEFGHIJKLMNO");

	rSimpleArgs::instance().parse(argc, (const char**)argv);

	printf("----------------------------------------------------------------------------------------------\n");
	printf("xmltest %i.%i.%i.%x (C) VeduN, 2019-2020 RSoft, OZNA\n\n", TRITONN_VERSION_MAJOR, TRITONN_VERSION_MINOR, TRITONN_VERSION_BUILD, TRITONN_VERSION_HASH);

	tinyxml2::XMLDocument doc;

	UDINT result = xmlFileCheck(rSimpleArgs::instance().getArgument(0), doc, rSimpleArgs::instance().getOption("signature"), rSimpleArgs::instance().getOption("salt"));
	
	switch(result) {
		case TRITONN_RESULT_OK:  printf("Success: hash is correct\n"); break;
		case XMLFILE_RESULT_NFHASH: printf("Error %u: not find signature\n", result); break;
		case XMLFILE_RESULT_BADHASH: printf("Error %u: bad hash\n", result); break;
		case XMLFILE_RESULT_NOTEQUAL: printf("Error %u: hash is not equal\n", result); break;
		default: printf("Error %u\n", result); break;
	}


	return 0;
}
