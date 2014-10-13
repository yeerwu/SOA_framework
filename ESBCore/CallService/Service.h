#ifndef __SERVICE_H__
#define __SERVICE_H__

#include <string>
#include <Winsock2.h>
using namespace std;

#define SVR_REQ_EVENT 1
#define SVR_RSP_EVENT 2
#define SVR_REG_EVENT 3
#define SVR_BIND_EVENT 4
#define SVR_ROUTE_ERROR 5

class CService
{
public:
	CService()
		: fd_(-1), buff_pos(0)
	{
	}

	virtual ~CService();

	int start();
	int reg();
	string bind(const string& _svrname);

	int readevent();
	virtual int dealevent();
	int sendevent();

protected:
	int writestream(const string& _param);
	int writestream(int _param);
	int writestream(char _param);
	int writestream(const char* _buff, long size);

	int readstream(string& _param);
	int readstream(int& _param);
	int readstream(char& _param);
	int readstream(char* _buff, long size);

	int send_n(int _fd, char* buff, long size);
	int read_n(int _fd, char* buff, long size);

	int fd_; //service socket connected to esbframework

	char buff[4096]; // event for test use
	int buff_pos; 

	string svrname_; //service name
	string uuid_; // service uuid
};

#endif
