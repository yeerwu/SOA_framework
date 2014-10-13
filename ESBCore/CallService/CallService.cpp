// CallService.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "CallService.h"

int CCallService::dealevent()
{
	return 0;
}

int CCallService::invite(const string& _person)
{
	printf("Start a call. First check the license...\n");
	//first, find license serviceto ask for validating service
	string lic_uuid = bind("LicenseService");
	if (lic_uuid != "")
	{
		printf("Find suitable service: %s\n", lic_uuid.c_str());
		//construct the req format
		//|type|4 byte size|src service uuid|4 byte size|dest service uuid
		//|4 byte size|interface name|4 byte size|param1|4 byte size|param2|...
		writestream((char)SVR_REQ_EVENT);
		writestream(uuid_);
		writestream(lic_uuid);
		writestream(string("validate")); //interface
		writestream(svrname_);  //param 1

		if (sendevent() == 0)
		{
			//receive response
			readevent();

			//rsp format
			//|type|4 byte size|src service uuid|4 byte size|dest service uuid
			//|4 byte size|interface name|4 byte result|4 byte size|param1|4 byte size|param2|...
			char evt_type = 0;
			int validres = 0;
			string srcuuid_, dstuuid_, strint;
			readstream(evt_type);
			if (evt_type == SVR_ROUTE_ERROR)
			{
				printf("validate request route failed\n");
				return -1;
			}
			readstream(srcuuid_);
			readstream(dstuuid_);
			readstream(strint);  //interface
			readstream(validres);

			printf("=========\nInvoke %s result: \nSrc: %s\nDst: %s\nInterface: %s\nResult:%d\n=========\n", 
				lic_uuid.c_str(),
				srcuuid_.c_str(),
				dstuuid_.c_str(),
				strint.c_str(),
				validres);
		}

	}
	else
	{
		printf("Find LicenseService failed\n");
	}

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsa_data;
    WSAStartup(0x0201, &wsa_data);

	CCallService service;
	service.start();
	service.reg();

	while (1)
	{
		service.invite("Bob"); //make a call out
		Sleep(1000);
	}

	return 0;
}

