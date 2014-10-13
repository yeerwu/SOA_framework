#ifndef _LICENSE_SERVICE_H_
#define _LICENSE_SERVICE_H_

#include "Service.h"

class CLicenseService: public CService
{
public:
	CLicenseService()
	{
		svrname_ = "LicenseService";
		uuid_ = "127.0.0.1_";
		uuid_ += svrname_;
		uuid_ += "_1";
	}

	virtual ~CLicenseService()
	{
	}

	virtual int dealevent();

//service interface
public:
	int validate(const string& service); 
	int getUsedValue(const string& service);
	int revoke(string service);

protected:
	int deal_req();
	int deal_rsp();
};

#endif