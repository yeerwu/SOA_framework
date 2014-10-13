#include "ESBFramework.h"

#pragma comment(lib,"libevent_core.lib")
#pragma comment(lib,"libevent_extras.lib")
#pragma comment(lib,"libevent.lib")

int main(int argc, char* argv[])
{
	WSADATA wsa_data;
    WSAStartup(0x0201, &wsa_data);

	ESB_Singleton<ESBFramework>::instance()->init();  //initialize the esb framework
	printf("Framework stopped running...\n");

	ESB_Singleton<ESBFramework>::instance()->close(); //close the esb framework

	return 0;
}