#ifndef _ESB_FRAMEWORK_H_
#define _ESB_FRAMEWORK_H_

#include <zookeeper.h>
#include <vector>
#include <string>
#include <Config.h>
#include "event.h"

#include <WinSock2.h>

using namespace std;

#define SVR_REQ_EVENT 1
#define SVR_RSP_EVENT 2
#define SVR_REG_EVENT 3
#define SVR_BIND_EVENT 4
#define SVR_ROUTE_ERROR 5

class PhyNode;
struct event_base;

template <class TYPE>
class ESB_Singleton
{
public:
  /// Global access point to the Singleton.
  static TYPE *instance (void);

  /// Explicitly delete the Singleton instance.
  static void destroy (void);

  /// Dump the state of the object.
  static void dump (void);

protected:
  /// Default constructor.
	ESB_Singleton (void) {}
	~ESB_Singleton() {}

  /// Contained instance.
  TYPE instance_;

  static ESB_Singleton<TYPE> *singleton_;
};

class Mutex
{
public:
	Mutex();
	~Mutex();

	void lock();
	void unlock();

protected:
	HANDLE mtx_;
};

class ESBService
{
public:
	ESBService()
		:name_(""), uuid_("")
	{
		burden_ = 0;
		fd_ = -1;
	}

	ESBService(string _name, string _uuid)
		: name_(_name), uuid_(_uuid)
	{
		burden_ = 0;
		fd_ = -1;
	}

	void updateburden(int _burden)
	{
		burden_ = _burden;
	}

	string name_;  //service name
	string uuid_;  //unique id
	int burden_;    //service CPU burden
	int fd_; //socket connected with esbframework
};

class ESBFramework
{
public:
	ESBFramework();
	~ESBFramework();

	void init();
	void close();

	void initeventbase();

	//zookeeper callback, watch event notification
	static void esb_watcher_g(zhandle_t* zh, int type, int state,
        const char* path, void* watcherCtx);

	static void accept_svrhandle(const int sfd, const short event, void *arg);
	static void accept_esbhandle(const int sfd, const short event, void *arg);

	static void svrbuff_on_read(struct bufferevent *bev, void * arg);
	static void esbbuff_on_read(struct bufferevent *bev, void * arg);

	static void svrbuff_on_error(struct bufferevent *bev, short what, void *ctx);
	static void esbbuff_on_error(struct bufferevent *bev, short what, void *ctx);

	void dealsvrreq(char* buffer, int fd);
	void dealsvrsp(char* buffer, int fd);
	void dealsvrreg(char* buffer, int fd);
	void dealsvrbind(char* buffer, int fd);
	void dealsvrerror(int fd);

	static DWORD run( LPVOID _param );
protected:

	void initsvrlisten();
	void initesblisten();

	string getservicename(const string& _uuid);

	int send_n(int _fd, char* buff, long size);
	int read_n(int _fd, char* buff, long size);

public:
	map<string, ESBService> services_;  //services connceted to current esbframework
	map<string, PhyNode> phynodes_;     //all other nodes which start esbframework

	Mutex mtx_;  //used to ensure data security

	IniConfig config_;  //store config items
	zhandle_t* zookeeper_;  //zeokeeper handle

	HANDLE hThread_;
	event_base* main_base;

	int listen_svrfd;
	int listen_esbfd;
};


#endif
