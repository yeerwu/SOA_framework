#include "ESBFramework.h"
#include "PhyNode.h"

template <class TYPE> ESB_Singleton<TYPE> *
ESB_Singleton<TYPE>::singleton_ = 0;


template <class TYPE> TYPE *
ESB_Singleton<TYPE>::instance (void)
{
	if (singleton_ == NULL)
	{
		singleton_ = new ESB_Singleton<TYPE>;
	}

	return &(singleton_->instance_);
}

template <class TYPE> 
void ESB_Singleton<TYPE>::destroy (void)
{
	if (singleton_)
	{
		delete singleton_;
	}
}

template <class TYPE> 
void ESB_Singleton<TYPE>::dump (void)
{
	//dump the singleton object
}

Mutex::Mutex()
{
	mtx_ = CreateMutex( NULL , FALSE , NULL );
}

Mutex::~Mutex()
{
	CloseHandle(mtx_);
}

void Mutex::lock()
{
	WaitForSingleObject(mtx_, INFINITE);
}

void Mutex::unlock()
{
	ReleaseMutex(mtx_);
}

void ESBFramework::esb_watcher_g(zhandle_t* zh, int type, int state,
        const char* path, void* watcherCtx)
{
	printf("Something happened.\n");
    printf("type: %d\n", type);
    printf("state: %d\n", state);
    printf("path: %s\n", path);
}

DWORD ESBFramework::run( LPVOID _param )
{
	while (1)
	{
		Sleep(1000);
	}

	return 0;
}

ESBFramework::ESBFramework()
{
}

ESBFramework::~ESBFramework()
{
}

string ESBFramework::getservicename(const string& _uuid)
{
	char* p = NULL;
	p = strtok((char*)_uuid.c_str(), "_");
	p = strtok(NULL, "_"); 

	string svrname = p;
	return svrname;
}

void ESBFramework::init()
{
	//connect to zookeeper
	const char* host = "127.0.0.1:2181";
    int timeout = 30000;
    int res = ZOK;

    zoo_set_debug_level(ZOO_LOG_LEVEL_DEBUG);
    zookeeper_ = zookeeper_init(host,
            esb_watcher_g, timeout, 0, (void*)this, 0);
    if (zookeeper_ == NULL) {
        fprintf(stderr, "Error when connecting to zookeeper servers...\n");
        exit(EXIT_FAILURE);
    }

	//create current node to the zookeeper
	if (ZOK != zoo_exists(zookeeper_, "/Node", NULL, NULL))
	{
		res = zoo_create(zookeeper_, "/Node", "node", 5, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0);
		res = zoo_create(zookeeper_, "/Service", "service", 8, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0); 
	}
	res = zoo_create(zookeeper_, "/Node/esb_127.0.0.1", "127.0.0.1", (strlen("127.0.0.1")+1), 
		&ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, NULL, 0);

	initeventbase();

	hThread_ = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)ESBFramework::run ,
        NULL ,
        0 ,
        NULL );
}

void ESBFramework::close()
{
	WaitForSingleObject(hThread_, INFINITE);
	CloseHandle(hThread_);

	zookeeper_close(zookeeper_);
}

void ESBFramework::initeventbase()
{
	main_base = event_init();

	initsvrlisten();
	initesblisten();

	event_base_loop(main_base, 0);

}

void ESBFramework::initsvrlisten()
{
	struct sockaddr_in listen_addr;

	int port = 10000; //service socket listening port

	listen_svrfd = socket(AF_INET, SOCK_STREAM, 0);

	memset(&listen_addr, 0, sizeof(listen_addr));

	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = INADDR_ANY;
	listen_addr.sin_port = htons(port);

	int reuseaddr_on = 1;

	setsockopt(listen_svrfd, SOL_SOCKET, SO_REUSEADDR, (const char*)(&reuseaddr_on), sizeof(reuseaddr_on));

	bind(listen_svrfd, (struct sockaddr *) &listen_addr, sizeof(listen_addr));

	listen(listen_svrfd, 1024);

	/*将描述符设置为非阻塞*/
	unsigned long ul = 1; 
	ioctlsocket(listen_svrfd, FIONBIO, &ul);
	//int flags = fcntl(listen_svrfd, F_GETFL);

	//flags |= O_NONBLOCK;

	//fcntl(listen_svrfd, F_SETFL, flags);

	static struct event ev_svrlisten;
	event_set(&ev_svrlisten, listen_svrfd, EV_READ | EV_PERSIST, accept_svrhandle, (void *)&ev_svrlisten);
	event_add(&ev_svrlisten, NULL);
}

void ESBFramework::initesblisten()
{
	struct sockaddr_in listen_addr;
	int port = 12000; //service socket listening port

	listen_esbfd = socket(AF_INET, SOCK_STREAM, 0);

	memset(&listen_addr, 0, sizeof(listen_addr));

	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = INADDR_ANY;
	listen_addr.sin_port = htons(port);

	int reuseaddr_on = 1;

	setsockopt(listen_esbfd, SOL_SOCKET, SO_REUSEADDR, (const char*)(&reuseaddr_on), sizeof(reuseaddr_on));

	bind(listen_esbfd, (struct sockaddr *) &listen_addr, sizeof(listen_addr));

	listen(listen_esbfd, 1024);

	/*将描述符设置为非阻塞*/
	unsigned long ul = 1; 
	ioctlsocket(listen_svrfd, FIONBIO, &ul);

	static struct event ev_esblisten;
	event_set(&ev_esblisten, listen_esbfd, EV_READ | EV_PERSIST, accept_esbhandle, (void *)&ev_esblisten);
	event_add(&ev_esblisten, NULL);
}

void ESBFramework::accept_svrhandle(const int sfd, const short event, void *arg)
{
	struct sockaddr_in addr;

   socklen_t addrlen = sizeof(addr);

   int fd = accept(sfd, (struct sockaddr *) &addr, &addrlen); //处理连接

    bufferevent* buf_ev = bufferevent_new(fd, 
		ESBFramework::svrbuff_on_read, NULL, 
		ESBFramework::svrbuff_on_error, (void*)fd);

    buf_ev->wm_read.high = 4096;

    bufferevent_base_set(ESB_Singleton<ESBFramework>::instance()->main_base, buf_ev);

    bufferevent_enable(buf_ev, EV_READ|EV_PERSIST);
}

void ESBFramework::accept_esbhandle(const int sfd, const short event, void *arg)
{
    struct sockaddr_in addr;

   socklen_t addrlen = sizeof(addr);

   int fd = accept(sfd, (struct sockaddr *) &addr, &addrlen); //处理连接

    bufferevent* buf_ev = bufferevent_new(fd, 
		ESBFramework::esbbuff_on_read, NULL, 
		ESBFramework::esbbuff_on_error, (void*)fd);

    buf_ev->wm_read.high = 4096;

    bufferevent_base_set(ESB_Singleton<ESBFramework>::instance()->main_base, buf_ev);

    bufferevent_enable(buf_ev, EV_READ|EV_PERSIST);

	//record this phynode
	char esbip[128];
	sprintf(esbip, "%s", inet_ntoa(addr.sin_addr));
	printf("Another esbframework from %s connected...\n", esbip);

	ESB_Singleton<ESBFramework>::instance()->phynodes_[esbip].ip_ = esbip;
	ESB_Singleton<ESBFramework>::instance()->phynodes_[esbip].port_ = addr.sin_port;
}

void ESBFramework::svrbuff_on_read(struct bufferevent *bev, void * arg)
{
    char buffer[4096];

    int ret = bufferevent_read(bev, &buffer, 4096);

	switch(buffer[0])
	{
	case SVR_REQ_EVENT:
		ESB_Singleton<ESBFramework>::instance()->dealsvrreq(buffer, (int)arg);
		break;
	case SVR_RSP_EVENT:
		ESB_Singleton<ESBFramework>::instance()->dealsvrsp(buffer, (int)arg);
		break;
	case SVR_REG_EVENT:
		ESB_Singleton<ESBFramework>::instance()->dealsvrreg(buffer, (int)arg);
		break;
	case SVR_BIND_EVENT:
		ESB_Singleton<ESBFramework>::instance()->dealsvrbind(buffer, (int)arg);
		break;
	default:
		printf("Invalid message from service...\n");
		break;
	}
}

void ESBFramework::esbbuff_on_read(struct bufferevent *bev, void * arg)
{
    char buffer[4096];

    int ret = bufferevent_read(bev, &buffer, 4096);

}

void ESBFramework::svrbuff_on_error(struct bufferevent *bev, short what, void *ctx)
{
	//handle the situation that conncetion with services lost
	int fd = (int)ctx;
	ESB_Singleton<ESBFramework>::instance()->dealsvrerror(fd);
}

void ESBFramework::esbbuff_on_error(struct bufferevent *bev, short what, void *ctx)
{
	//handle the situation that connection with other phynode lost
}

void ESBFramework::dealsvrreq(char* buffer, int fd)
{
	//req format
	//|type|4 byte size|src service uuid|4 byte size|dest service uuid
	//|4 byte size|interface name|4 byte size|param1|4 byte size|param2|...
	char sztmp[256] = {0};
	string src, dst, svrname;
	int size, pos;
	memcpy(&size,  &buffer[1], sizeof(int));
	memcpy(sztmp, &buffer[5], size);
	src = sztmp;
	pos = 5+size;
	memset(sztmp, 0, 256);

	//read dest uuid and check if we need to forward the request to other nodes
	memcpy(&size, buffer+pos, sizeof(int));
	memcpy(sztmp, buffer+pos+4, size);
	dst = sztmp;

	svrname = getservicename(dst);
	mtx_.lock();
	if (services_.find(svrname) != services_.end())
	{
		send_n(services_[svrname].fd_, buffer, 4096);
	}
	else
	{
		//check the zookeeper from other nodes

		//|type|4 byte size|src service uuid|4 byte size|dest service uuid
		//|4 byte size|interface name|4 byte result|4 byte size|param1|4 byte size|param2|...
		char rspbuff[4096];
		rspbuff[0] = SVR_ROUTE_ERROR;
		send_n(fd, rspbuff, 4096);
	}

	mtx_.unlock();
}

void ESBFramework::dealsvrsp(char* buffer, int fd)
{
	//rsp format
	//|type|4 byte size|src service uuid|4 byte size|dest service uuid
	//|4 byte size|interface name|4 byte result|4 byte size|param1|4 byte size|param2|...
	char sztmp[256] = {0};
	string src, dst, svrname;
	int size, pos;
	memcpy(&size,  &buffer[1], sizeof(int));
	memcpy(sztmp, &buffer[5], size);
	src = sztmp;
	pos = 5+size;
	memset(sztmp, 0, 256);

	//read dest uuid and check if we need to forward the request to other nodes
	memcpy(&size, buffer+pos, sizeof(int));
	memcpy(sztmp, buffer+pos+4, size);
	dst = sztmp;

	svrname = getservicename(dst);
	mtx_.lock();
	if (services_.find(svrname) != services_.end())
	{
		send_n(services_[svrname].fd_, buffer, 4096);
	}
	else
	{
		char rspbuff[4096];
		rspbuff[0] = SVR_ROUTE_ERROR;
		send_n(fd, rspbuff, 4096);
	}
	mtx_.unlock();
}

void ESBFramework::dealsvrreg(char* buffer, int fd)
{
	//reg format
	//|type|1 byte size|service name|...
	int res = ZOK;
	int size = buffer[1];
	char szName[256] = {0};
	memcpy(szName, &buffer[2], size);

	mtx_.lock();
	services_[szName].name_ = szName;
	services_[szName].fd_ = fd;

	string uuid = "127.0.0.1_";
	uuid += szName;
	uuid += "_1";
	services_[szName].uuid_ = uuid;

	//create znode to the zookeeper
	char szPath[256] = {0};
	sprintf(szPath, "/Service/%s", szName);
	if (ZOK != zoo_exists(zookeeper_, szPath, NULL, NULL) )
	{
		res = zoo_create(zookeeper_, szPath, szName, (services_[szName].name_.length()+1), 
		&ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0);
	}

	sprintf(szPath, "/Service/%s/%s", services_[szName].name_.c_str(), services_[szName].uuid_.c_str());
	res = zoo_create(zookeeper_, szPath, uuid.c_str(), (uuid.length()+1), 
		&ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, NULL, 0);
	mtx_.unlock();
}

void ESBFramework::dealsvrerror(int fd)
{
	mtx_.lock();
	map<string, ESBService>::iterator itor = services_.begin();
	while (itor != services_.end())
	{
		if (itor->second.fd_ == fd)
			break;
		++itor;
	}

	if (itor != services_.end())
	{
		fprintf(stderr, "Service; %s has lost connection with framework...\n", itor->second.name_.c_str());

		//remove the node from zookeeper
		char szpath[512];
		sprintf(szpath, "/Service/%s/%s", itor->first.c_str(), itor->second.uuid_.c_str());
		int res = zoo_delete(zookeeper_, szpath, 0);

		services_.erase(itor);
	}
	else
	{
		fprintf(stderr, "Not find the related service\n");
	}

	mtx_.unlock();
}

void ESBFramework::dealsvrbind(char* buffer, int fd)
{
	//bind format
	//|type|4 byte size| dest service name|...

	int size = 0;
	memcpy(&size, &buffer[1], sizeof(int));

	char szbuff[4096] = {0};
	bool bmatach = false;
	char szName[256] = {0};
	memcpy(szName, &buffer[5], size);

	//check local node if we have this service
	//if (services_.find(szName) != services_.end())
	//{
	//	mtx_.lock();

	//	//forward the resp to src service
	//	//|type|4 byte result|4 byte size| dest service uuid|...
	//	szbuff[0] = SVR_BIND_EVENT;
	//	size = services_[szName].uuid_.length();
	//	memcpy(&szbuff[5], &size, sizeof(int));
	//	memcpy(&szbuff[9], services_[szName].uuid_.c_str(), services_[szName].uuid_.length());

	//	mtx_.unlock();
	//}
	//else
	{
		//check the zookeeper to see which node has the required service
		char szPath[256] = {0};
		sprintf(szPath, "/Service/%s", szName);
		String_vector tmpnodes;
		int res = zoo_get_children(zookeeper_, szPath, 0, &tmpnodes);
		if (tmpnodes.count > 0)
		{
			szbuff[0] = SVR_BIND_EVENT;
			size = strlen(tmpnodes.data[0]);
			memcpy(&szbuff[5], &size, sizeof(int));
			memcpy(&szbuff[9], tmpnodes.data[0], strlen(tmpnodes.data[0]) );
		}
		else
		{
			szbuff[0] = SVR_BIND_EVENT;
			res = -1;
			memcpy(&szbuff[1], &res, sizeof(int));
		}
	}

	send_n(fd, szbuff, 4096); //send bind response to the service
}

int ESBFramework::send_n(int _fd, char* buff, long size)
{
	int n = 0;
	while (n < size)
	{
		n += send(_fd, buff+n, size-n, 0);
	}

	return 0;
}

int ESBFramework::read_n(int _fd, char* buff, long size)
{
	int n = 0;
	while (n < size)
	{
		n += recv(_fd, buff+n, size-n, 0);
	}

	return 0;
}