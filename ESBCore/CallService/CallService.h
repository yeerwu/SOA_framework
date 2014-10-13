#ifndef _CALL_SERVICE_H_
#define _CALL_SERVICE_H_

#include "Service.h"

using namespace std;

class CCallService: public CService
{
public:
	CCallService()
	{
		svrname_ = "CallService";
		uuid_ = "127.0.0.1_";
		uuid_ += svrname_;
		uuid_ += "_1";
	}

	virtual ~CCallService()
	{
	}

	virtual int dealevent();

//service interface
public:
	int invite(const string& _person); //test interface, make a call out

};

#endif