//=================================================================================================
//===
//=== data_listconf.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс для получения информации об доступных конфигурациях
//===
//=================================================================================================

#pragma once


#include <vector>
#include "def.h"


using std::vector;



struct rItemConfig
{
	string Filename;
	string Description;
	string StrHash;
	UDINT  Status;
	time_t Filetime;
};



class rListConfig
{
public:
	virtual ~rListConfig();

	static UDINT Load();
	static const rItemConfig *Get(const UDINT index);
	static UDINT Size();

// Static class
private:
	rListConfig();
	rListConfig(const rListConfig &);
	rListConfig& operator=(rListConfig &);

protected:
	static  vector<rItemConfig> List;

};
