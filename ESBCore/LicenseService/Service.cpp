#include "stdafx.h"
#include "Service.h"
#include <WinSock2.h>

CService::~CService()
{
	closesocket(fd_);
}

int CService::start()
{
	//create socket and connect to esbframework of current node
	struct sockaddr_in connect_addr;
	fd_ = socket(AF_INET, SOCK_STREAM, 0);

	memset(&connect_addr, 0, sizeof(connect_addr));

	connect_addr.sin_family = AF_INET;
	connect_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	connect_addr.sin_port = htons(10000);
	int res = connect(fd_, (SOCKADDR*)&connect_addr,sizeof(sockaddr_in));

	return res;
}

int CService::sendevent()
{
	if (fd_ >= 0)
	{
		send_n(fd_, buff, 4096);		
	}

	return 0;
}

int CService::readevent()
{
	if (fd_ >= 0)
	{
		memset(buff, 0, 4096);
		buff_pos = 0;

		read_n(fd_, buff, 4096);
	}

	return 0;
}

int CService::dealevent()
{
	return 0; //not implemented
}

int CService::reg()
{
	//reg format
	//|type|1 byte size|service name|...
	writestream((char)(SVR_REG_EVENT));
	writestream((char)svrname_.length());
	writestream(svrname_.c_str(), svrname_.length());

	sendevent();
	return 0;
}

string CService::bind(const string& _svrname)
{
	string uuid = "";
	memset(buff, 0, 4096);
	buff_pos = 0;
	//construct bind request
	//|type|4 byte size| dest service name|...
	writestream((char)SVR_BIND_EVENT);
	writestream(_svrname);

	if (sendevent() == 0)
	{
		readevent();
	}

	//|type|4 byte result|4 byte size| dest service uuid|...
	char evt_type = 0;
	int bind_res = 0;
	readstream((char)evt_type);
	readstream(bind_res);
	readstream(uuid);

	memset(buff, 0, 4096);
	buff_pos = 0;
	if (bind_res != 0)
	{
		printf("Service %s bind failed! Error: %d\n", 
			_svrname.c_str(), bind_res);
		return "";
	}

	return uuid;
}

int CService::writestream(const string& _param)
{
	writestream((int)_param.length());
	writestream(_param.c_str(), _param.length());

	return 0;
}

int CService::writestream(int _param)
{
	memcpy(buff+buff_pos, &_param, sizeof(int));
	buff_pos += sizeof(int);

	return 0;
}

int CService::writestream(char _param)
{
	memcpy(buff+buff_pos, &_param, sizeof(char));
	buff_pos += sizeof(char);

	return 0;
}

int CService::writestream(const char* _buff, long size)
{
	// test use, no consideration of outof bound
	memcpy(buff+buff_pos, _buff, size);
	buff_pos += size;

	return 0;
}

int CService::readstream(string& _param)
{
	char szStr[4096] = {0};
	int size = 0;
	readstream(size);
	readstream(szStr, size);
	_param = szStr;

	return 0;
}

int CService::readstream(int& _param)
{
	memcpy(&_param, buff+buff_pos, sizeof(int));
	buff_pos += sizeof(int);

	return 0;
}

int CService::readstream(char& _param)
{
	memcpy(&_param, buff+buff_pos, sizeof(char));
	buff_pos += sizeof(char);

	return 0;
}

int CService::readstream(char* _buff, long size)
{
	memcpy(_buff, buff+buff_pos, size);
	buff_pos += size;

	return 0;
}

int CService::send_n(int _fd, char* buff, long size)
{
	int n = 0;
	while (n < size)
	{
		n += send(_fd, buff+n, size-n, 0);
	}

	return 0;
}

int CService::read_n(int _fd, char* buff, long size)
{
	int n = 0;
	while (n < size)
	{
		n += recv(_fd, buff+n, size-n, 0);
	}

	return 0;
}