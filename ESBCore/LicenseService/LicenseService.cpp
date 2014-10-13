// LicenseService.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "LicenseService.h"

int CLicenseService::deal_req()
{
	//|type|4 byte size|src service uuid|4 byte size|dest service uuid
	//|4 byte size|interface name|4 byte size|param1|4 byte size|param2|...
	string srcid, dstid, dstint, param1;
	readstream(srcid);
	readstream(dstid);
	readstream(dstint);
	readstream(param1);

	printf("===============\nDst: %s\nSrc: %s\nInterface: %s\nParam1: %s\n======================\n",
		dstid.c_str(),
		srcid.c_str(),
		dstint.c_str(),
		param1.c_str());

	if (dstint == "validate")
	{
		int res = validate(param1);
		printf("Validate %s result: %d\n", 
			param1.c_str(), res);

		//|type|4 byte size|src service uuid|4 byte size|dest service uuid
		//|4 byte size|interface name|4 byte result|4 byte size|param1|4 byte size|param2|...
		memset(buff, 0, 4096);
		buff_pos = 0;
		writestream((char)SVR_RSP_EVENT);
		writestream(dstid);
		writestream(srcid);
		writestream(dstint);
		writestream(res);

		send_n(fd_, buff, 4096);
	}

	return 0;
}

int CLicenseService::deal_rsp()
{
	return 0;
}

int CLicenseService::dealevent()
{
	char evt_type;
	readstream(evt_type);
	if (evt_type == SVR_REQ_EVENT)
	{
		deal_req();
	}
	else if (evt_type == SVR_RSP_EVENT)
	{
		deal_rsp();
	}
	else
	{
		printf("wrong event\n");
	}
	return 0;
}

int CLicenseService::validate(const string& service)
{
	printf("Service %s want to validate its license\n", service.c_str());

	//construct the response

	return 0;
}

int CLicenseService::getUsedValue(const string& service)
{
	return 0;
}

int CLicenseService::revoke(string service)
{

	return 0;
}


int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsa_data;
    WSAStartup(0x0201, &wsa_data);

	CLicenseService service;
	service.start();
	service.reg();

	while (1)
	{
		service.readevent(); // read request
		service.dealevent(); // deal request
	}
	return 0;
}

