//=================================================================================================
//===
//=== json_manager.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс-поток для общения с json-клиентом
//===
//=================================================================================================

#pragma once

#include "tcp_class.h"


class  rUser;
class  rJSONClient;
struct cJSON;


class rActivity
{
public:
	rActivity();
	virtual ~rActivity() { ; }

	void UpdateIdle();

	DINT  Token;
	UDINT Session;
	UDINT Idle;
	const rUser *User;

};


class rJSONManager: public rTCPClass
{
public:
	virtual ~rJSONManager();

// Singleton
private:
	rJSONManager();
	rJSONManager(const rJSONManager &);
	rJSONManager& operator=(rJSONManager &);

public:
	static rJSONManager &Instance();


// Наследование от rTCPClass
protected:
	virtual rThreadStatus Proccesing(void);
	virtual rClientTCP*   NewClient (SOCKET socket, sockaddr_in *addr);
	virtual UDINT         ClientRecv(rClientTCP *client, USINT *buff, UDINT size);

protected:
	vector<rActivity> Activity;

	rActivity *GetActivity(DINT token);
	UDINT      DeleteActivity(DINT token);

	string GetErrorJSON   (JSON_ERROR err, const string &errmsg);
	void   CreateErrorJSON(cJSON *root, JSON_ERROR err, const string &errmsg);
	string JSONtoString   (cJSON *root, int deleteobj);
	string ParsingJSON    (const char *text);


	string Packet_REQ     (cJSON *root);
	string Packet_DataGet (cJSON *root);
	string Packet_DataSet (cJSON *root);
	string Packet_Login   (cJSON *root);
	string Packet_Logout  (cJSON *root);
	string Packet_Check   (cJSON *root);
	string Packet_Status  (cJSON *root);
	string Packet_Conf    (cJSON *root);
	string Packet_ListConf(cJSON *root);
	string Packet_Restart (cJSON *root);
};




