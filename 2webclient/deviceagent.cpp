// MNDBServer.cpp : 定义控制台应用程序的入口点。
//

#ifdef WIN32
#include "stdafx.h"
#endif

#include "MNDBServer.h"
#include "sys/epoll.h"
#include "signal.h"
#include "string.h"
#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <assert.h>
#include <iomanip>

//#include "TestExam.h"

AutoLock mGlobalLock;

//////////////////////////////////////////////////////////////////////////

#ifdef WIN32
#else
#define strcpy_s strcpy
#endif
#define MAX_CONN_SEND_RETRY	10

int send_flg = 0;
int reinit_flg = 0;
int coninit_flg = 0;
int closecon_flg = 0;
int msg_flg = 0;
int send_thread_flg = 0;

int server_not_send_flag = 0;
char szMsgName[100];

long nLocalDeviceID = 0;
// AutoLock mapConnectionsLock;
// map<long,stConnectionInfo> mapConnections;

class core_transfer;
AutoLock vecConnectionsLock;
vector<stConnectionInfo> vecConnections;
int epfd = 0;
int file_handle[1024] = {};
struct OutStream{
	int fd;
	ssize_t len;
};
OutStream out_stream[1024] = {};
//typedef std::map<long, int[1024]> FileMap;
//FileMap out_stream;

#define invalid_socket (-1)

class core_transfer {
public:
	explicit core_transfer(std::string& remote, int meta_id, short handle_id)
		: sock_(invalid_socket), remote_end_(remote)
		, meta_id_(meta_id), handle_id_(handle_id)
	{
		bzero(&remote_, sizeof (remote_));
		remote_.sin_family = AF_INET;
		remote_.sin_port = htons(8080);

		if (inet_aton(remote.c_str(), &remote_.sin_addr) == 0) {
			std::cerr << "invalid remote address" << std::endl;
		} else {
			std::cout << "init remote ok" << std::endl;
		}
	}

	~core_transfer() {
		if (sock_ != invalid_socket) {
			close(sock_);
		}
		sock_ = invalid_socket;
	}

	int create_connection() {
		if (is_open(sock_)) {
			std::cerr << __func__ << " -- already open" << std::endl;
			return -1;
		}
		sock_ = socket(AF_INET, SOCK_STREAM, 0);
		std::cout << __func__ << " -- socket: " << sock_ << std::endl;

		int recv_cache_size = 128 * 1024;
		int send_cache_size = 128 * 1024;

		setsockopt(sock_, SOL_SOCKET, SO_RCVBUF, (const char*) &recv_cache_size, sizeof(int));
		setsockopt(sock_, SOL_SOCKET, SO_SNDBUF, (const char*) &send_cache_size, sizeof(int));

		if (sock_ < 0) {
			return -1;
		} else {
			return 0;
		}
	}

	int connect_blk() {
		create_connection();
		if (!is_open(sock_)) {
			std::cerr << __func__ << " -- invalid socket" << std::endl;
			return -1;
		}
		if (connect(sock_, (struct sockaddr*) &remote_, sizeof (remote_)) != 0) {
			std::cerr << __func__ << " -- connect error" << std::endl;
			return -1;
		} else {
			return 0;
		}
	}

	void disconnect_blk() {
		if (!is_open(sock_)) {
			std::cerr << __func__ << " -- invalid socket" << std::endl;
			return ;
		}

		if (close(sock_) != 0) {
			std::cerr << __func__ << " -- terminate failed" << std::endl;
			sock_ = invalid_socket;
		} else {
			sock_ = invalid_socket;
		}
	}

	void reconnect_blk() {
		if (is_open(sock_)) {
			disconnect_blk();
			connect_blk();
		} else {
			connect_blk();
		}
	}

	int send_blk(const char* data, ::size_t len) {
		if (!is_open(sock_)) {
			std::cerr << __func__ << " -- invalid socket" << std::endl;
			return -1;
		}
		::size_t bytes_transfer = 0;
		int once_tranfer = 0;
		try {
			signal(SIGPIPE, SIG_IGN);
			while (len > bytes_transfer) {
				once_tranfer = send(sock_, (char*)data + bytes_transfer,
					len - bytes_transfer, 0);
				if (once_tranfer < 0) {
					std::cerr << __func__ << " -- send error, bytes_transfer: "
						<< bytes_transfer << std::endl;
					return bytes_transfer;
				} else if (once_tranfer == 0) {
					std::cerr << __func__ << " -- send null" << std::endl;
					return bytes_transfer;
				} else {
					bytes_transfer += once_tranfer;
					if (bytes_transfer >= len) {
						std::cout << __func__ << " -- send ok, bytes_transfer: "
							<< bytes_transfer << ", len: " << len << std::endl;
						return bytes_transfer;
					}
				}
			}
		} catch (std::exception& e) {
			std::cerr << e.what() << std::endl;
		}

		// shouldn't be here
		std::cout << __func__ << " exception quit" << std::endl;
		return bytes_transfer;
	}

	// 阻塞读,读一次返回相应字节数
	int recv_blk(char* buf, ::size_t len) {
		if (!is_open(sock_)) {
			std::cerr << __func__ << " -- invalid socket" << std::endl;
			return -1;
		}
		int once_read = read(sock_, buf, len);
		return once_read;
	}

	int get_metaid() {
		return meta_id_;
	}

	std::string get_remote_end() {
		return remote_end_;
	}

	int get_raw_socket() {
		return sock_;
	}

	short get_handle_id() {
		return handle_id_;
	}

protected:
	bool is_open(int sock) {
		return (sock > 0);
	}

private:
	int sock_;
	struct sockaddr_in remote_;
	std::string remote_end_;
	int meta_id_;
	short handle_id_;
};

vector<core_transfer*> vecEndpoints;
//////////////////////////////////////////////////////////////////////////
// 
// #ifndef WIN32	 //Linux   platform
// #include <termio.h>
// #define getch() get_char(0)	 //without stdout
// #define getche() get_char(1)	//with stdout
// #else	 //WIN32   platform
// #include "conio.h"
// #endif
// 
// #ifndef WIN32	 //Linux platform
// 
// /************************************************************************
//  * method name: get_char
//  * input:       0--getch()  1--getche()
//  * return:      char
//  * remark:      Linux simulative gech() , getche()
// Instead of "conio.h" using in VC6.0 environment
//  ************************************************************************/
// char get_char(int flag)
// {
// char ch;
// 
// struct termios new_settings;
// struct termios stored_settings;
// 
// tcgetattr(0, &stored_settings);
// new_settings = stored_settings;
// new_settings.c_lflag &= (~ICANON);
// new_settings.c_cc[VTIME] = 0;
// 
// tcgetattr(0, &stored_settings);
// new_settings.c_cc[VMIN] = 1;
// tcsetattr(0, TCSANOW, &new_settings);
// 
// if (flag == 1)	 //getche()
// {
// ch = getchar();
// }
// else	 //getch()
// {
// system("stty   -echo");
// ch = getchar();
// system("stty   echo");
// 
// tcsetattr   (0,   TCSANOW,   &stored_settings);
// }
// 
// return ch;
// }
// #endif
//////////////////////////////////////////////////////////////////////////
// 
// #ifndef WIN32
// 
// #include <stdio.h>
// #include <termios.h>
// char getche()
// {
// 	char s;
// 	//        FILE *in;
// 	//        FILE *out;
// 	struct termios initial_settings,new_settings;
// 	//        in=fopen("/dev/tty","r");
// 	//        out=fopen("/dev/tty","w");
// 
// 	tcgetattr(fileno(stdin),&initial_settings);//保存原来的设置
// 	new_settings = initial_settings;//开始新终端控制设置
// 	new_settings.c_lflag &= ~ICANON;
// 	new_settings.c_lflag &= ~ECHO;
// 	new_settings.c_cc[VMIN] = 1;
// 	new_settings.c_cc[VTIME] = 0;
// 	if(tcsetattr(fileno(stdin),TCSANOW,&new_settings)!=0)//应用新的设置
// 		fprintf(stderr,"coutld not set attributes\n");
// 
// 	s=fgetc(stdin);
// 	fprintf(stdout,"\nyou input %c\n",s);
// 
// 	tcsetattr(fileno(stdin),TCSANOW,&initial_settings);//返回原来的设置
// 	return s;
// }
// #else	 //WIN32   platform
// #include "conio.h"
// #endif
//////////////////////////////////////////////////////////////////////////

// const int nBufLen = 4*1024*1024;
// char szBuf[nBufLen];
// static int send_count=0;

//////////////////////////////////////////////////////////////////////////
char szName[64]={0};
int nType=0;
char szIp[20]={0};
int nPort=0;

char szTarget[64]={0};
int nConnectionType=0;//0 p2p , 1 transport

int timer_enable = 0;

int frame_mode_enable = 0;//0 data mode , 1 frame mode 
//0 client , 1 server
//at first , client and server connect to IDM.
//then , client p2p(or transfer) connect to server , if success , send msg to server .
//server send data to all connections .
int mode = 0;

//send big buffer to buffing udx's sending 
int buffing=0;
int buffing_count=10;

//////////////////////////////////////////////////////////////////////////
int parse(int argc, char* argv[])
{
	for (int i=0;i<argc;i++)
	{
		//		printf("input arg : %d --> %s \n",i,argv[i]);

		char * arg = argv[i];

		if (arg[0] != '-')
			continue;

		arg++;
		if (strcmp(arg,"timer") == 0)//timer 定时器是否打开
		{
			i++;
			if (i >= argc)
				return -1;
			arg = argv[i];

			timer_enable = atoi(arg);
			if (timer_enable!=0)
				timer_enable = 1;
		}
		else if (strcmp(arg,"name") == 0)//name 名称
		{
			i++;
			if (i >= argc)
				return -1;
			arg = argv[i];

			if (strlen(arg) > 64)
				arg[63] = 0;

			strcpy_s(szName,arg);
		}
		else if (strcmp(arg,"type") == 0)//type类型
		{
			i++;
			if (i >= argc)
				return -1;
			arg = argv[i];

			nType = atoi(arg);
			if (nType > 2)
				nType = 0;
		}
		else if (strcmp(arg,"ip") == 0)//IP地址
		{
			i++;
			if (i >= argc)
				return -1;
			arg = argv[i];

			strcpy_s(szIp,arg);
		}
		else if (strcmp(arg,"port") == 0)//port端口
		{
			i++;
			if (i >= argc)
				return -1;
			arg = argv[i];

			nPort = atoi(arg);
		}
		else if (strcmp(arg,"target") == 0)//target目标名称
		{
			i++;
			if (i >= argc)
				return -1;
			arg = argv[i];

			if (strlen(arg) > 64)
				arg[63] = 0;

			strcpy_s(szTarget,arg);
		}
		else if (strcmp(arg,"contype") == 0)//contype连接类型
		{
			i++;
			if (i >= argc)
				return -1;
			arg = argv[i];

			nConnectionType = atoi(arg);
			if (nConnectionType > 1)
				nConnectionType = 1;
		}
		else if (strcmp(arg,"mode") == 0)//mode 模式，0 客户端，1 服务端
		{
			i++;
			if (i >= argc)
				return -1;
			arg = argv[i];

			mode = atoi(arg);
			if (mode != 0)
				mode = 1;
		}
		else if (strcmp(arg,"frame_mode_enable") == 0)//frame_mode_enable 帧模式，0 关闭，1 打开
		{
			i++;
			if (i >= argc)
				return -1;
			arg = argv[i];

			frame_mode_enable = atoi(arg);
			if (frame_mode_enable != 0)
				frame_mode_enable = 1;
		}
		else if (strcmp(arg,"buffing") == 0)//buffing 发送大数据量包，使udx发送进入缓存模式，0 关闭，1 打开
		{
			i++;
			if (i >= argc)
				return -1;
			arg = argv[i];

			buffing = atoi(arg);
			if (buffing != 0)
				buffing = 1;
		}
		else if (strcmp(arg,"buffing_count") == 0)//buffing多少次断开连接
		{
			i++;
			if (i >= argc)
				return -1;
			arg = argv[i];

			buffing_count = atoi(arg);
			if (buffing_count > 100)
				buffing_count = 100;
		}
		
		

	}

	return 0;
}

void printarg()
{
	printf("arguments : \n");

	printf("local name : %s\n",szName);
	printf("type : %s\n",SDK_DEVICE_TYPE_STR[nType]);
	printf("IDM Server : %s:%d\n",szIp,nPort);

	printf("target name : %s\n",szTarget);
	printf("connection type : %s\n",SDK_CONNECTION_TYPE_STR[nConnectionType]);

	printf("mode : %d\n",mode);
	printf("frame_mode_enable : %d\n",frame_mode_enable);	
}
//////////////////////////////////////////////////////////////////////////
static void OnSDKInit(stDeviceInfo * pInfo,int nErrno)									//初始化回调
{
	if (!epfd) {
		epfd = epoll_create(20000);
	}
//	AutoLockHelper locker1(&mGlobalLock);
	nLocalDeviceID = pInfo->nDeviceId;
	printf("OnSDKInit %d , DeviceId : %d  error: %d \r\n",(int)pInfo->nDeviceId,(int)nLocalDeviceID,nErrno);

	if (mode == 0)
	if (nErrno == 0)
	{
		coninit_flg = 1;
//		msg_flg = 1;

	}

	if (nErrno == 0)
		reinit_flg = 0;

// 	if (nErrno == 0)
// 	if (mode == 1)//server
// 		StartSendThread();
}

static void OnSDKMessage(stDeviceInfo * pInfo,char *pMessageSender,char *pMessageType,unsigned char * pData,unsigned int nLen)	//接收消息回调
{
//	AutoLockHelper locker1(&mGlobalLock);
	char * pMsg = new char[nLen+1];
	pMsg[nLen]=0;
	memcpy(pMsg,pData,nLen);	
	printf("OnSDKMessage %d , sender : %s type : %s len: %d string:%s\r\n",(int)pInfo->nDeviceId,pMessageSender,pMessageType,nLen,pMsg);

	if (mode == 1)
	{
		if (strcmp(pMsg,"start send")==0)
		{
			send_flg = 1;
		}
		if (strcmp(pMsg,"stop send")==0)
		{
			send_flg = 0;
		}
	}

	delete []pMsg;
}

static void OnSDKClose(stDeviceInfo * pInfo)												//关闭回调
{
//	AutoLockHelper locker1(&mGlobalLock);
	printf("OnSDKClose %p \r\n",(void*)pInfo->nDeviceId);

	if (pInfo->nDeviceId == nLocalDeviceID)
	{
		AutoLockHelper locker(&vecConnectionsLock);
		vector<stConnectionInfo>::iterator itor;
		for (int i=0;i<vecConnections.size();i++)
		{
			ConnectionClose(pInfo->nDeviceId,vecConnections[i].nConnectionId);
		}
		vecConnections.clear();
		if (1 == mode) {
			for (int i = 0; i < vecEndpoints.size(); i++) {
				epoll_event ev = {0, {0}};
				epoll_ctl(epfd, EPOLL_CTL_DEL, vecEndpoints[i]->get_raw_socket(), &ev);
				vecEndpoints[i]->disconnect_blk();
				delete vecEndpoints[i];
			}
			vecEndpoints.clear();
		}
// 		AutoLockHelper locker(&mapConnectionsLock);
// 		map<long,stConnectionInfo>::iterator itor;
// 		for (itor=mapConnections.begin();itor!=mapConnections.end();itor++)
// 		{
// 			ConnectionClose(pInfo->nDeviceId,itor->second.nConnectionId);
// 		}
// 		mapConnections.clear();
	}
	/*
	错误，此处依然处于回调中，不能close设备
	DeviceClose(nLocalDeviceID);
	nLocalDeviceID = 0;
	*/
	reinit_flg = 1;

//	StopSendThread();
}

static void OnSDKTimer(stDeviceInfo * pInfo)												//定时回调50ms
{
//	AutoLockHelper locker1(&mGlobalLock);
	time_t t;
	time(&t);
	tm t1;

	char szTime[50];

#ifdef WIN32
	localtime_s(&t1, &t);
	sprintf(szTime,"[%.2d:%.2d:%.2d]",t1.tm_hour,t1.tm_min,t1.tm_sec);
#else
	localtime_r(&t, &t1);
	snprintf(szTime,50,"[%.2d:%.2d:%.2d]",t1.tm_hour,t1.tm_min,t1.tm_sec);
#endif	

	static int device_counter=0;
	if (pInfo)
	if (pInfo->nDeviceId!=0)
	if (pInfo->nDeviceId == nLocalDeviceID)
		device_counter++;

	if (timer_enable)
	if ((device_counter%20)==0)
		printf("OnSDKTimer %s %p %d \r\n",szTime,(void*)pInfo->nDeviceId,device_counter);
}

int nErrorCount = 0;
static void OnSDKConnectionInit(stConnectionInfo * pInfo,int nErrno)				//连接初始化回调
{
//	AutoLockHelper locker1(&mGlobalLock);
// 	send_count=0;
	printf("++++++++++++++++++++++++++++++++++[%p]OnSDKConnectionInit[%d] %p->%p , error: %d %s\r\n",pInfo->nDeviceContext,pInfo->nContext,(void*)pInfo->nDeviceId,(void*)pInfo->nConnectionId,nErrno,pInfo->szType);
	if (nErrno!=0)
	{
		nErrorCount++;
		return;
	}


	{
		AutoLockHelper locker(&vecConnectionsLock);
//		pInfo->nContext = 1;
		vecConnections.push_back(*pInfo);

// 		AutoLockHelper locker(&mapConnectionsLock);
// 		pInfo->nContext = 1;
// 		mapConnections[pInfo->nConnectionId] = *pInfo;
		if (1 == mode) {
			/*
			std::string remote = "10.12.2.125";
			core_transfer* cf = new core_transfer(remote, pInfo->nConnectionId);
			cf->connect_blk();
			cf->set_index_is_load(false);
			int sockfd = cf->get_raw_socket();
			assert(sockfd > 0);
			epoll_event ev;
			ev.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET;
			ev.data.fd = sockfd;
			epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
			vecEndpoints.push_back(cf);
			*/
		}
	}

	stConnectionConfig cfg;
	cfg.nBufferSize = 200*1024;
	ConnectionSetConfig(pInfo->nConnectionId,&cfg);

	if (frame_mode_enable)
	{
		ConnectionEnableFrameMode(pInfo->nConnectionId,true);
	}

	if (mode == 0)//client
	{
		if (pInfo->blPromoter)
		{
			strcpy(szMsgName,pInfo->szName);
			msg_flg = 1;
		}

// 		ConnectionClose(pInfo->nDeviceId,pInfo->nConnectionId);
// 		ConnectionInit(pInfo->nDeviceId,pInfo);
	}

	if (mode == 1)//server
	{
		send_thread_flg = 1;
//		StartSendThread();
	}
}

static void OnSDKConnectionData(stConnectionInfo * pInfo,unsigned char * pConnData,unsigned int nLen)		//接收数据回调
{
//	AutoLockHelper locker1(&mGlobalLock);
#ifdef WIN32
	SYSTEMTIME now_time;
	GetLocalTime(&now_time);
	printf("[%.2d:%.2d:%.2d.%.3d] OnSDKConnectionData %d->%d , len: %d string : %s \r\n",now_time.wHour,now_time.wMinute,now_time.wSecond,now_time.wMilliseconds,(void*)pInfo->nDeviceId,(void*)pInfo->nConnectionId,nLen,pData);
#else

	time_t   now;         //实例化time_t结构   
	struct   tm     *timenow;         //实例化tm结构指针   
	time(&now);   
	timenow   =   localtime(&now);  

// 	time_t t;
// 	time(&t);
// 	tm t1;
// 
// 	localtime_r(&t, &t1);

	unsigned char* pHead = pConnData;
	unsigned char* pData = pConnData + 32;
	//printf("[%s] OnSDKConnectionData %d->%d , len: %d string : %s \r\n",asctime(timenow),(void*)pInfo->nDeviceId,(void*)pInfo->nConnectionId,nLen,pData);
	if (1 == mode) {
		printf("[%s] OnSDKConnectionData [%ld] -> [%ld] , len: %d\n",asctime(timenow),pInfo->nDeviceId,pInfo->nConnectionId,nLen);
		std::cout << pHead[8] << " " << pHead[9] << std::endl;
		short handle = *((short*) (pHead + 8));
		unsigned char is_disconnect = *(pHead + 10);
		int cmd = (int) pHead[0];
		bool has_handle = FALSE;
		if (1 == cmd) {
			printf("request: %s\n", pData);
			const char* jsonstr = "{\"id\":0,\"params\":[{\"id\":0,\"params\":{\"definition\":\"ZL_7108E\"},\"result\":true},{\"id\":0,\"params\":{\"definition\":16},\"result\":true},{\"id\":0,\"params\":{\"netInterface\":[{\"PhysicalAddress\":\"01:11:B0:EF:1C:A4\"}]},\"result\":true}],\"result\":true}\n";
			char response[1025 * 32] = {};
			response[0] = 1;
			unsigned int len = strlen(jsonstr) + 1;
			memcpy(response + 4, (char*) &len, 4);
			memcpy(response + 32, (char*) jsonstr, len);
			printf("response: %s\n", jsonstr);
			ConnectionSendData(pInfo->nConnectionId, (unsigned char*) response, len + 32);
			return ;
		}
		AutoLockHelper lock(&vecConnectionsLock);
		vector<core_transfer*>::iterator it;

		for (it = vecEndpoints.begin(); it != vecEndpoints.end(); it++) {
			if (pInfo->nConnectionId == (*it)->get_metaid() && handle == (*it)->get_handle_id()) {
				has_handle = true;
				break ;
			}
		}
		if (is_disconnect && has_handle) {
			int& out_handle = out_stream[(*it)->get_handle_id()].fd;
			if (out_handle > 0) {
				close(out_handle);
			}
			out_handle = -1;
			std::cout << __func__ << " --  delete handle: " << (*it)->get_handle_id() << std::endl;
			(*it)->disconnect_blk();
			delete (*it);
			vecEndpoints.erase(it);
		} else {
			if (!has_handle) {
				std::string remote = "10.12.2.121";
				core_transfer* cf = new core_transfer(remote, pInfo->nConnectionId, handle);
				cf->connect_blk();
				int sockfd = cf->get_raw_socket();
				assert(sockfd > 0);
				epoll_event ev;
				ev.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET;
				ev.data.fd = sockfd;
				epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
				it = vecEndpoints.insert(it, cf);
				std::cout << __func__ << " -- new handle: " << handle << ", sockfd: " << cf->get_raw_socket() << std::endl;
			}
			std::cout << __func__ << " -- handle: " << handle << ", recv data: " << std::endl;
			printf("%.*s\n", nLen - 32, pData);
			std::string send_data = "GET / HTTP/1.1\r\n";
			bool load = false;
			unsigned char* request = (unsigned char*) strstr((char*) pData, "GET /verification");
			if (request != NULL)
				load = true;
			if (load) {
				// TODO 手动修改,以后修改成httpparser解析
				// 先进行粗糙解析,假定都是HTTP/1.1
				//unsigned char* http_protocol = strstr(pData, "HTTP");
				unsigned char* description = (unsigned char*) strstr((char*) pData, "Connection");
				if (description >= pData && description < pData + nLen) {
					std::cout << "load index" << std::endl;
					std::cout << "host ip: " << (*it)->get_remote_end() << std::endl;
					send_data += "Host: ";
					send_data += (*it)->get_remote_end();
					send_data += "\r\n";
					send_data += (char*) description;
				} else {
					send_data = (char*) pData;
				}
			} else {
				unsigned char* description = (unsigned char*) strstr((char*) pData, "Host");
				if (description >= pData && description < pData + nLen) {
					send_data.clear();
					send_data.append((char*) pData, description - pData);
					send_data += "Host: ";
					send_data += (*it)->get_remote_end();
					send_data += "\r\n";
					description = (unsigned char*) strstr((char*) pData, "Connection");
					send_data += (char*) description;
				} else {
					send_data = (char*) pData;
				}
			}
			//std::cout << __func__ << " -- send data: " << send_data << std::endl;
			int ret = (*it)->send_blk(send_data.c_str(), nLen);
			if (ret <= 0) {
				epoll_event ev = {0, {0}};
				epoll_ctl(epfd, EPOLL_CTL_DEL, (*it)->get_raw_socket(), &ev);
				(*it)->reconnect_blk();
				ev.events = EPOLLET | EPOLLIN | EPOLLERR | EPOLLHUP;
				ev.data.fd = (*it)->get_raw_socket();
				assert(ev.data.fd > 0);
				epoll_ctl(epfd, EPOLL_CTL_ADD, ev.data.fd, &ev);
				// try again
				ret = (*it)->send_blk(send_data.c_str(), nLen);
				std::cout << __func__ << " 1 send_bytes: " << ret << std::endl;
			} else {
				std::cout << __func__ << " 2 send_bytes: " << ret << std::endl;
			}
		}
	} else {
		printf("[%s] OnSDKConnectionData [%ld] -> [%ld] , len: %d\n",asctime(timenow),pInfo->nDeviceId,pInfo->nConnectionId,nLen);
		short handle = *((short*) (pHead + 8));
		char filename[128] = {};
		sprintf(filename, "file-content-%d", handle);

		if (file_handle[handle] < 0) {
			file_handle[handle] = open(filename, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
		}

		std::cout << "handle: " << handle << std::endl;
		assert(file_handle[handle] > 0);
		if (file_handle[handle] > 0) {
			int ret = write(file_handle[handle], pData, nLen - 32);
			assert(ret == (nLen - 32));
			syncfs(file_handle[handle]);
		}
	}

#endif
}

#ifdef WIN32
static DWORD last_frame_ticket = GetTickCount();
static void OnSDKConnectionFrame( stConnectionInfo * pInfo, bool bVideo,bool bKeyFrame,unsigned char * pData,unsigned int nLen,int nType2 )		//接收帧回调
{
//	AutoLockHelper locker1(&mGlobalLock);
	SYSTEMTIME now_time;
	GetLocalTime(&now_time);

	DWORD tick1 = GetTickCount();
	printf("[%.2d:%.2d:%.2d.%.3d] OnSDKConnectionFrame len: %d frame type : %s-%s      %d - %d = %d ms\r\n",now_time.wHour,now_time.wMinute,now_time.wSecond,now_time.wMilliseconds,nLen,bVideo?"V":"A",bKeyFrame?"I":"P",tick1,last_frame_ticket,tick1-last_frame_ticket);
	last_frame_ticket = tick1;
}
#else
#include   <stdio.h>   
#include   <stdlib.h>                                                 /*   包含标准库头文件   */   
#include   <sys/time.h>   

/**   
      *   计算两个时间的间隔，得到时间差   
      *   @param   struct   timeval*   resule   返回计算出来的时间   
      *   @param   struct   timeval*   x             需要计算的前一个时间   
      *   @param   struct   timeval*   y             需要计算的后一个时间   
      *   return   -1   failure   ,0   success   
	  **/   
int   timeval_subtract(struct   timeval*   result,   struct   timeval*   x,   struct   timeval*   y)   
{   
	int   nsec;   

	if   (   x->tv_sec>y->tv_sec   )   
		return   -1;   

	if   (   (x->tv_sec==y->tv_sec)   &&   (x->tv_usec>y->tv_usec)   )   
		return   -1;   

	result->tv_sec   =   (   y->tv_sec-x->tv_sec   );   
	result->tv_usec   =   (   y->tv_usec-x->tv_usec   );   

	if   (result->tv_usec<0)   
	{   
		result->tv_sec--;   
		result->tv_usec+=1000000;   
	}   

	return   0;   
}   

static struct timeval last_frame_ticket;
static void OnSDKConnectionFrame( stConnectionInfo * pInfo, bool bVideo,bool bKeyFrame,unsigned char * pData,unsigned int nLen,int nType2 )		//接收帧回调
{
//	AutoLockHelper locker1(&mGlobalLock);
	time_t   now;         //实例化time_t结构   
	struct   tm     *timenow;         //实例化tm结构指针   
	time(&now);   
	timenow   =   localtime(&now);  


	struct timeval time_now,diff;
	gettimeofday(&time_now,0);
	timeval_subtract(&diff,&last_frame_ticket,&time_now);
	printf("[%s] OnSDKConnectionFrame len: %d frame type : %s-%s      %d ms\r\n",asctime(timenow),nLen,bVideo?"V":"A",bKeyFrame?"I":"P",diff.tv_usec/1000);
	last_frame_ticket = time_now;
}
#endif

static void OnSDKConnectionClose(stConnectionInfo * pInfo)
{
//	AutoLockHelper locker1(&mGlobalLock);
	printf("OnSDKConnectionClose %p->%p \r\n",(void*)pInfo->nDeviceId,(void*)pInfo->nConnectionId);

	{
		AutoLockHelper locker(&vecConnectionsLock);
		vector<stConnectionInfo>::iterator itor;

		for (itor=vecConnections.begin();itor!=vecConnections.end();)
		{
			stConnectionInfo mInfo = *itor;

			if (mInfo.nConnectionId == pInfo->nConnectionId)
			{
				itor = vecConnections.erase(itor);
			}
			else
				itor++;
		}

		if (1 == mode) {
			vector<core_transfer*>::iterator it;
			for (it = vecEndpoints.begin(); it != vecEndpoints.end();) {
				core_transfer* cf = *it;
				if (cf->get_metaid() == pInfo->nConnectionId) {
					epoll_event ev = {0, {0}};
					epoll_ctl(epfd, EPOLL_CTL_DEL, cf->get_raw_socket(), &ev);
					it = vecEndpoints.erase(it);
					cf->disconnect_blk();
					delete (cf);
				} else {
					it ++;
				}
			}
		}

// 		AutoLockHelper locker(&mapConnectionsLock);
// 		map<long,stConnectionInfo>::iterator itor;
// 
// 		for (itor=mapConnections.begin();itor!=mapConnections.end();)
// 		{
// 			if (itor->second.nConnectionId == pInfo->nConnectionId)
// 			{
// 				mapConnections.erase(itor++);
// 			}
// 			else
// 				itor++;
// 		}
	}

	ConnectionClose(pInfo->nDeviceId,pInfo->nConnectionId);

}

// int exit_flg=0;
static void OnSDKConnectionTimer(stConnectionInfo * pInfo)												//定时回调50ms
{
//	AutoLockHelper locker1(&mGlobalLock);
	static int connection_counter=0;
	connection_counter++;

	if (timer_enable)
	if ((connection_counter%20)==0)
		printf("OnSDKConnectionTimer %p %p %d\r\n",(void*)pInfo->nDeviceId,(void*)pInfo->nConnectionId,connection_counter);
// 
// #ifdef WIN32
// 	if (connection_counter > 20*5)//3s
// 	{
// 		char * pCmd = "stop send";
// 		DeviceSendMsg(pInfo->nDeviceId,pInfo->szName,SDK_MESSAGE_TYPE_STR[0],(unsigned char *)pCmd,strlen(pCmd));
// 
// 		C_Channel_T chanel;
// 		memset(&chanel, 0, sizeof(C_Channel_T));
// 
// 		chanel.channel = 0;
// 		chanel.stream = 0;
// 		chanel.rate = 25;
// 		chanel.quality = 5;
// 		chanel.encode = 2;
// 		chanel.pixel = 0;
// 
// 
// 		IDM_TRANS_MSG stopChannel;
// 		stopChannel.len = sizeof(C_Channel_T) + 1;
// 		stopChannel.data = new char[stopChannel.len];
// 		stopChannel.data[0] = 11;
// 		memcpy(&stopChannel.data[1], &chanel, sizeof(C_Channel_T));
// 
// 		DeviceSendMsg(pInfo->nDeviceId,pInfo->szName,SDK_MESSAGE_TYPE_STR[0],(unsigned char *)stopChannel.data, stopChannel.len);
// 
// 		if(NULL != stopChannel.data)
// 		{
// 			delete [] stopChannel.data;
// 			stopChannel.data = NULL;
// 		}
// 		connection_counter = 0;
// 		exit_flg = 1;
// 	}
// #endif

}


void InitDevice()
{
//	AutoLockHelper locker1(&mGlobalLock);
	stDeviceInfo xDeviceInfo;

	strcpy(xDeviceInfo.szId,szName);
	strcpy(xDeviceInfo.szType,SDK_DEVICE_TYPE_STR[nType]);

	strcpy(xDeviceInfo.szServerIp,szIp);
	xDeviceInfo.nServerPort = nPort;

	strcpy(xDeviceInfo.szUser,"root");
	strcpy(xDeviceInfo.szPwd,"123456");

	xDeviceInfo.bIsSync = TRUE;

 	xDeviceInfo.OnInit = OnSDKInit;
	xDeviceInfo.OnMessage = OnSDKMessage;
 	xDeviceInfo.OnClose = OnSDKClose;
	if (timer_enable)
		xDeviceInfo.OnTimer = OnSDKTimer;

	xDeviceInfo.OnConnectionInit = OnSDKConnectionInit;
	xDeviceInfo.OnConnectionData = OnSDKConnectionData;
	xDeviceInfo.OnConnectionFrame = OnSDKConnectionFrame;
	xDeviceInfo.OnConnectionClose = OnSDKConnectionClose;
	if (timer_enable)
		xDeviceInfo.OnConnectionTimer = OnSDKConnectionTimer;

	//初始化，注册
	int rt = DeviceInit(&xDeviceInfo);
	if (rt != 0)
		printf("DeviceInit failed!\r\n");
	else
		printf("DeviceInit success!\r\n");
}

int ConnectionIndex = 0;
void InitConnection()
{
//	AutoLockHelper locker1(&mGlobalLock);
	stConnectionInfo xConnectionInfo;

	xConnectionInfo.blPromoter = TRUE;
	xConnectionInfo.nContext = ConnectionIndex++;

	strcpy(xConnectionInfo.szName,szTarget);

	xConnectionInfo.bIsSync = FALSE;

	strcpy(xConnectionInfo.szMode,SDK_CONNECTION_MODE_STR[0]);
	strcpy(xConnectionInfo.szType,SDK_CONNECTION_TYPE_STR[nConnectionType]);	
//	strcpy(xConnectionInfo.szType,SDK_CONNECTION_MODE_STR_ADV[1]);	

// 	strcpy(xConnectionInfo.szTargetServerIp,"192.168.0.107");
// 	xConnectionInfo.nTargetServerPort = 9201;

	int rt = ConnectionInit(nLocalDeviceID,&xConnectionInfo);
	if (rt != 0)
		printf("ConnectionInit failed! %d\r\n",rt);
	else
		printf("ConnectionInit success!\r\n");
}

void CloseConnection()
{
//	AutoLockHelper locker1(&mGlobalLock);

	AutoLockHelper locker(&vecConnectionsLock);
	vector<stConnectionInfo>::iterator itor;
	for (int i=0;i<vecConnections.size();i++)
	{
		ConnectionClose(nLocalDeviceID,vecConnections[i].nConnectionId);
	}
	vecConnections.clear();
	if (1 == mode) {
		vector<core_transfer*>::iterator it;
		for (int i = 0; i < vecEndpoints.size(); i++) {
			epoll_event ev = {0, {0}};
			epoll_ctl(epfd, EPOLL_CTL_DEL, vecEndpoints[i]->get_raw_socket(), &ev);
			vecEndpoints[i]->disconnect_blk();
			delete vecEndpoints[i];
		}
		vecEndpoints.clear();
	}

// 	AutoLockHelper locker(&mapConnectionsLock);
// 	map<long,stConnectionInfo>::iterator itor;
// 
// 	for (itor=mapConnections.begin();itor!=mapConnections.end();itor++)
// 	{
// 		ConnectionClose(nLocalDeviceID,itor->second.nConnectionId);
// 		printf("ConnectionClose %p %p \n",nLocalDeviceID,itor->second.nConnectionId);
// 	}
// 
// 	mapConnections.clear();
}
#if 0
static int buffing_count1=0;

bool bRunning = TRUE;
#ifdef WIN32

HANDLE m_hExitSendThread = NULL;
HANDLE m_hSendThread = NULL;
static UINT WINAPI SendThread( LPVOID pParam )

#else

pthread_t thread_id = 0;
void *tret = NULL;
void * SendThread( void * pParam )
#endif // WIN32
{
#ifndef WIN32
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	bRunning = TRUE;
#endif

	int send_count = 0;
	int error_count = 0;

	while(bRunning)
	{
#ifdef WIN32
		DWORD thread_waite = WaitForSingleObject(m_hExitSendThread, 0);
		if(thread_waite == WAIT_OBJECT_0)
		{
			return -10000;
		}
#else

#endif
		if (send_count>1000)
		{
			AutoLockHelper locker(&vecConnectionsLock);
			for (int i=0;i<vecConnections.size();i++)
			{
				ConnectionClose(vecConnections[i].nDeviceId,vecConnections[i].nConnectionId);
				ConnectionInit(vecConnections[i].nDeviceId,&vecConnections[i]);
			}
			vecConnections.clear();
			send_count = 0;
		}

		if (!server_not_send_flag)
 		{
//			AutoLockHelper locker1(&mGlobalLock);

			long * pConIds = NULL;
			int nIdCount=0;
			{
				AutoLockHelper locker(&vecConnectionsLock);
	// 			AutoLockHelper locker(&mapConnectionsLock);
	// 			map<long,stConnectionInfo>::iterator itor;

				nIdCount = vecConnections.size();
				if (nIdCount>0)
					pConIds=new long[nIdCount];
				if (pConIds)
				for (int i=0;i<vecConnections.size();i++)
				{
					pConIds[i]=vecConnections[i].nConnectionId;
				}
			}

			int rt = 0;
			int len = 0;
			
			for (int i=0;i<nIdCount;i++)
//			for (itor=mapConnections.begin();itor!=mapConnections.end();itor++)
			{
// 				if (itor->second.nContext == 1)
// 				{	
// 					(&itor->second)->nContext = 0;
// 				if (vecConnections[i].nContext == 1)
// 				{	
// 					vecConnections[i].nContext = 0;
// 
// 					send_count = 0;

// 					if ((send_count%25)!=0)
// 					{
// 						//不是I帧跳过
// 						printf("ConnectionSendData wait for I Frame ! %d [%d] \n",(int)itor->second.nConnectionId,send_count);
// 						continue;
// 					}
// 					else
// 					{
// 						(&itor->second)->nContext = 0;
// 						printf("ConnectionSendData I Frame , Start Send! %d [%d] \n",(int)itor->second.nConnectionId,send_count);
// 					}

//				}

				if (buffing)
				{
					len = (rand() % (1000))*1024 + 2*1024*1000;
					sprintf(szBuf,"[%d]-[%s]-[%d]",send_count,"Buffing",len);
				}
				else
				{
					sprintf(szBuf,"[%d]-[%s]",send_count,((send_count%25)==0)?"I":"P");
					if ((send_count%25)==0)
					{
						len = (rand() % (100))*1000 + 100000;
					}
					else
					{
						len = (rand() % (5000)) + 1000;
					}
				}

				rt = ConnectionSendData(pConIds[i],(unsigned char *)szBuf,len);
				printf("ConnectionSendData len %d rt %ld %p [%d]! \n",len,rt,pConIds[i],send_count);

				if (rt==0)
					buffing_count1++;
				else
					buffing_count1=0;

				if (buffing)
				if (buffing_count1>buffing_count)
				{
					printf("close buffing_count1 %d \n",buffing_count1);
					{
						AutoLockHelper locker(&vecConnectionsLock);
						ConnectionClose(vecConnections[i].nDeviceId,vecConnections[i].nConnectionId);
						vector<stConnectionInfo>::iterator itor=vecConnections.begin();
						itor += i;
						vecConnections.erase(itor);
					}
					buffing_count1=0;
				}


// 				rt = ConnectionSendData(itor->second.nConnectionId,(unsigned char *)szBuf,len);
// 				printf("ConnectionSendData len %d rt %d %p [%d]! \n",len,rt,(int)itor->second.nConnectionId,send_count);
/*
				int re_send_count=0;
				while (rt==0)
				{
					rt = ConnectionSendData(vecConnections[i].nConnectionId,(unsigned char *)szBuf,len);
// 					rt = ConnectionSendData(itor->second.nConnectionId,(unsigned char *)szBuf,len);
					if (re_send_count>10)
						break;
					printf("ConnectionSendData resend len %d error %d %p [%d] %d! \n",len,rt,(int)vecConnections[i].nConnectionId,send_count,re_send_count);
//					printf("ConnectionSendData resend len %d error %d %p [%d] %d! \n",len,rt,(int)itor->second.nConnectionId,send_count,re_send_count);
					re_send_count++;

					if (!bRunning)
						break;
#ifdef WIN32
					Sleep(100);
#else
					usleep(100000);
#endif
				}
				*/
// 				if (rt <=0)
// 				if ((error_count % 20)==0)
				{
// 					error_count++;
				}
				// 			if ((send_count%25)==0)
				// 				ConnectionSendFrame(itor->second,true,true,(unsigned char *)szBuf,(rand() % (300000)) + 400000,0);
				// 			else
				// 				ConnectionSendFrame(itor->second,true,false,(unsigned char *)szBuf,(rand() % (5000)) + 1000,0);
			}

			send_count++;

			if (pConIds)
				delete []pConIds;
		}
#ifdef WIN32
		Sleep(100);
#else
		usleep(100000);
#endif
	}
#ifdef WIN32
	return 10000;
#else
	return (void*)10000;
#endif
}



void StartSendThread()
{
#ifdef WIN32
	if (!m_hExitSendThread)
	{
		m_hExitSendThread = CreateEvent(NULL,FALSE,TRUE,NULL);
		ResetEvent(m_hExitSendThread);
	}
	if (!m_hSendThread)
		m_hSendThread = (HANDLE)_beginthreadex(NULL, 64*1024, SendThread, 0, STACK_SIZE_PARAM_IS_A_RESERVATION , NULL);
#else
	if (!thread_id)
	{
		if (pthread_create(&thread_id,NULL,SendThread,NULL)!=0)
		{
			printf("Create thread error!\n");
			thread_id = 0;
		}
	}
#endif // WIN32

}

void StopSendThread()
{
	bRunning = FALSE;

#ifdef WIN32
	if (m_hSendThread)
	{
		SetEvent(m_hExitSendThread);
		WaitForSingleObject(m_hSendThread,INFINITE);
		CloseHandle(m_hExitSendThread);
	}
	m_hSendThread = NULL;
	m_hExitSendThread = NULL;
#else
	if (thread_id)
	{
		pthread_cancel(thread_id);
		pthread_join(thread_id,NULL);
		thread_id=0;
	}
#endif // WIN32
}

#endif


int ExportLog(unsigned char * pData,unsigned int nLen)
{

#ifdef WIN32

#endif

	return 0;
}

CSendThread send_thread1;
CSendThread send_thread2;
CSendThread send_thread3;
CSendThread send_thread4;
CSendThread send_thread5;
CRecvThread recv_thread1;


int nRunCounter = 0;

#ifdef WIN32
int _tmain(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif // WIN32
{
	for (int i = 0; i < 1024; i++) {
		file_handle[i] = -1;
	}
	for (int j = 0; j < 1024; j++) {
		out_stream[j].fd = -1;
		out_stream[j].len = 0;
	}
	parse(argc,argv);
	printarg();

 	srand(time(NULL));

	//使用前，先调用INIT
	{
//		AutoLockHelper locker1(&mGlobalLock);
		SDKInit();
		printf("SDKInit %d! \n",mode);
	}

#ifdef WIN32
	char * pIniPath = "c:\\sdk\\";
	int rt = SDKSetConfig(pIniPath);
	printf("SDKSetConfig %d! %s\n",rt,pIniPath);
	
#endif

	const char* jsonstr = "{\"id\":0,\"params\":[{\"id\":0,\"params\":{\"definition\":16},\"result\":true},{\"id\":0,\"params\":{\"definition\":\"ZL_7108E\"},\"result\":true},{\"id\":0,\"params\":{\"netInterface\":[{\"PhysicalAddress\":\"00:10:5C:F2:1C:B4\"}]},\"result\":true}],\"result\":true}\n";
	printf("json: %s", jsonstr);
	//创建设备
	InitDevice();
	
	if (mode == 1)//服务端
	{
		//send_thread1.start();
		//send_thread2.start();
		//send_thread3.start();
		//send_thread4.start();
		//send_thread5.start();
		recv_thread1.start();

		while (1)
		{
// 			if (send_flg == 1)
			{
//				AutoLockHelper locker1(&mGlobalLock);
				AutoLockHelper locker(&vecConnectionsLock);
				for (int i=0;i<vecConnections.size();i++)
				{
// 					int len = (rand() % (nBufLen/2)) + (nBufLen/2);
// 					int rt = ConnectionSendData(vecConnections[i].nConnectionId,(unsigned char *)szBuf,len);
// 					printf("send_flg ConnectionSendData len %d %d %p mode %d! \n",len,rt,(int)vecConnections[i].nConnectionId,mode);

					printf("%d connection[%d]  %p ! \n",i,vecConnections[i].nContext,vecConnections[i].nConnectionId);
				}
			}

			char ch = getchar();//win下可使用getche()，linux下暂未找到替代，只能以回车继续循环的运行
			if (ch=='q')
				break;

			if (ch == 'i')
			{
				reinit_flg  =1;
			}

			if (ch == 's')
			{
				closecon_flg = 1;
			}

			if (ch == 'x')
			{
				server_not_send_flag = 1;
			}
			if (ch == 'z')
			{
				server_not_send_flag = 0;
			}

			printf("mode %d running %d %d %d  %d\n",mode,send_flg,nRunCounter++,vecConnections.size(),ConnectionIndex);
			printf("endpoints size: %lu\n", vecEndpoints.size());

			if (closecon_flg)
			{
				closecon_flg = 0;
				printf("close connection \n");
				CloseConnection();
			}

			if (reinit_flg)
			{
				printf("reinit device \n");
				InitDevice();
			}

#ifdef WIN32
			Sleep(100);
#else
			usleep(100000);
#endif		
		}

		printf("mode %d exit \n",mode);
	}

	//客户端
	if (mode == 0)
	{
		while (1)
		{
			printf("mode %d press 'q' to exit ! %d %d\n",mode,vecConnections.size(),ConnectionIndex);

			int ch = getchar();

			{
//				AutoLockHelper locker1(&mGlobalLock);
				AutoLockHelper locker(&vecConnectionsLock);
				for (int i=0;i<vecConnections.size();i++)
				{
					printf("%d connection[%ld][%s]  %ld ! \n",i,vecConnections[i].nContext,vecConnections[i].szName,vecConnections[i].nConnectionId);
				}
			}

			static vector<short> id_request;
			if (ch == 'q')
				break;

			if (ch == 'i')
			{
				reinit_flg  =1;
			}

			if (ch == 'c')
			{
				coninit_flg = 1;
			}

			if (ch == 's')
			{
				closecon_flg = 1;
			}
			if (ch == 't') {
				AutoLockHelper locker(&vecConnectionsLock);
				for (int i = 0; i < vecConnections.size(); i++) {
					char buf[1024] = {};
					static short conn_id = 0;
					//const char* request = "GET /css/main.css HTTP/1.1\r\nHost: http://www.163.com\r\nConnection: keep-alive\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2272.101 Safari/537.36\r\nDNT: 1\r\nAccept-Encoding: gzip, deflate, sdch\r\nAccept-Language: zh-CN,zh;q=0.8\r\n\r\n";
					const char* request = "GET jsCore/rpcCore.js HTTP/1.1\r\nHost: http://www.163.com\r\nConnection: keep-alive\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2272.101 Safari/537.36\r\nDNT: 1\r\nAccept-Encoding: gzip, deflate, sdch\r\nAccept-Language: zh-CN,zh;q=0.8\r\n\r\n";
					char* w8 = buf + 8;
					char* data = buf + 32;
					conn_id++;
					memcpy(w8, &conn_id, 2);
					memcpy(data, request, strlen(request) + 1);
					id_request.push_back(conn_id);
					ConnectionSendData(vecConnections[i].nConnectionId, (unsigned char*) buf, strlen (request) + 33);
					std::cout << "id: " << vecConnections[i].nConnectionId << " test send over" << std::endl;
				}
			}
			if (ch == 'y') {
				AutoLockHelper locker(&vecConnectionsLock);
				for (int i = 0; i < vecConnections.size(); i++) {
					for (int j = 0; j < id_request.size(); j++) {
						char buf[32] = {};
						memcpy(buf + 8, &(id_request[j]), 2);
						buf[10] = 1;
						ConnectionSendData(vecConnections[i].nConnectionId, (unsigned char*) buf, 32);
						std::cout << "id: " << vecConnections[i].nConnectionId << " test send over" << std::endl;
					}
				}
				id_request.clear();
			}

			if (coninit_flg)
			{
				coninit_flg = 0;
				printf("init connection \n");
				InitConnection();
			}

			if (closecon_flg)
			{
				closecon_flg = 0;
				printf("close connection \n");
				CloseConnection();
			}

			if (msg_flg)
			{
				msg_flg = 0;
				printf("send msg to server \n");
				SendMsgToServer();
			}

			if (reinit_flg)
			{
				printf("reinit device \n");
				InitDevice();
			}

		}
		printf("mode %d exit \n",mode);
	}

//	StopSendThread();

	//关闭设备
	{
//		AutoLockHelper locker1(&mGlobalLock);
		DeviceClose(nLocalDeviceID);
		printf("DeviceClose ! \n");
	}

	//使用后，调用UNINIT

	{
//		AutoLockHelper locker1(&mGlobalLock);
		SDKUnInit();
		printf("SDKUnInit ! will exit ! \n");
	}

	for (int i = 0; i < 1024; i++) {
		if (file_handle[i] > 0) {
			close(file_handle[i]);
		}
	}
	for (int j = 0; j < 1024; j++) {
		if (out_stream[j].fd < 0) {
			close(out_stream[j].fd);
		}
		out_stream[j].len = 0;
	}
	
 	return 0;
}

void SendMsgToServer()
{
// 	char * pCmd = "start send";
// 	DeviceSendMsg(nLocalDeviceID,szMsgName,SDK_MESSAGE_TYPE_STR[0],(unsigned char *)pCmd,strlen(pCmd));

	C_Channel_T chanel;
	memset(&chanel, 0, sizeof(C_Channel_T));

	chanel.channel = 0;
	chanel.stream = 0;
	chanel.rate = 25;
	chanel.quality = 5;
	chanel.encode = 2;
	chanel.pixel = 0;

	IDM_TRANS_MSG startChannel;
	startChannel.len = sizeof(C_Channel_T) + 1;
	startChannel.data = new char[startChannel.len];
	startChannel.data[0] = 10;
	memcpy(&startChannel.data[1], &chanel, sizeof(C_Channel_T));

	DeviceSendMsg(nLocalDeviceID,szMsgName,SDK_MESSAGE_TYPE_STR[0],(unsigned char *)startChannel.data, startChannel.len);

	if(NULL != startChannel.data)
	{
		delete [] startChannel.data;
		startChannel.data = NULL;
	}

}


static int nSendThreadIndex=0;
CSendThread::CSendThread()
{
	index = nSendThreadIndex++;
	
	send_count = 0;

	buffing_count1=0;

	bRunning=TRUE;
#ifdef WIN32
	m_hExitSendThread=NULL;
	m_hSendThread=NULL;
#else
	thread_id = 0;
	tret = NULL;
#endif // WIN32

}

CRecvThread::CRecvThread() {
	bRunning = true;
	thread_id = 0;
}

CRecvThread::~CRecvThread() {
	stop();
}

CSendThread::~CSendThread()
{
	stop();
}

int CSendThread::start()
{
#ifdef WIN32
	if (!m_hExitSendThread)
	{
		m_hExitSendThread = CreateEvent(NULL,FALSE,TRUE,NULL);
		ResetEvent(m_hExitSendThread);
	}
	if (!m_hSendThread)
	{
		m_hSendThread = (HANDLE)_beginthreadex(NULL, 64*1024, SendThread, this, STACK_SIZE_PARAM_IS_A_RESERVATION , NULL);
		if (!m_hSendThread)
		{
			printf("Create thread error!\n");
			return -1;
		}
	}
#else
	if (!thread_id)
	{
		if (pthread_create(&thread_id,NULL,SendThread,this)!=0)
		{
			printf("Create thread error!\n");
			thread_id = 0;
			return -1;
		}
	}
#endif // WIN32

	return 0;
}

int CRecvThread::start() {
	if (!thread_id) {
		if (pthread_create(&thread_id, NULL, RecvThread, this) != 0) {
			std::cout << "create thread error" << std::endl;
			thread_id = 0;
			return -1;
		}
	}
}

void CSendThread::stop()
{
	bRunning = FALSE;

#ifdef WIN32
	if (m_hSendThread)
	{
		SetEvent(m_hExitSendThread);
		WaitForSingleObject(m_hSendThread,INFINITE);
		CloseHandle(m_hExitSendThread);
	}
	m_hSendThread = NULL;
	m_hExitSendThread = NULL;
#else
	if (thread_id)
	{
		pthread_cancel(thread_id);
		pthread_join(thread_id,NULL);
		thread_id=0;
	}
#endif // WIN32
}

void CRecvThread::stop() {
	bRunning = FALSE;

	if (thread_id) {
		pthread_cancel(thread_id);
		pthread_join(thread_id, NULL);
		thread_id = 0;
	}
}

void* CRecvThread::RecvThread(void* pParam) {
	CRecvThread* pTrd = (CRecvThread*) pParam;
	//static short resp_id = 1;

	while (pTrd->bRunning) {
		epoll_event ev[128];
		int ev_num = epoll_wait(epfd, ev, 128, -1);
		for (int i = 0; i < ev_num; i++) {
			if (ev[i].events & EPOLLIN) {
				std::cout << __func__ << " -- got message from server" << std::endl;
				AutoLockHelper locker(&vecConnectionsLock);
				vector<core_transfer*>::iterator it;
				for (it = vecEndpoints.begin(); it != vecEndpoints.end();) {
					if ((*it)->get_raw_socket() == ev[i].data.fd) {
						const ::size_t __Recv_len_per = 1024 * 1024 * 2;
						char buf[__Recv_len_per + 32] = "";
						int len = (*it)->recv_blk(buf + 32, __Recv_len_per);
						int handle_id = (*it)->get_handle_id();
						//int& out_handle = out_stream[handle_id].fd;
						//ssize_t& tlen = out_stream[handle_id].len;
						//char filename[128] = {};
						//sprintf(filename, "out-stream-%d-%d", (*it)->get_metaid(), handle_id);

						memcpy(&buf[8], &handle_id, 2);
						//memcpy(&buf[12], &resp_id, 2);
						//std::cout << "resp_id: " << resp_id << std::endl;
						//resp_id++;
						if (len > 0) {
							//if (out_handle < 0) {
							//	out_handle = open(filename, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
							//	tlen = 0;
							//}
							//assert(out_handle > 0);
							//int ret = write(out_handle, buf + 32, len);
							//assert(ret == (len));
							//tlen += ret;
							int retry_count = 0;
							// TODO if ConnectionSendData return a size which is less than len + 32, application
							// should run a loop to call it.
							while (ConnectionSendData((*it)->get_metaid(), (unsigned char*) buf, len + 32) < 0) {
								if (retry_count > MAX_CONN_SEND_RETRY) {
									assert(0);
								}
								std::cout << "send error, retry count: " << retry_count << std::endl;
								retry_count++;
							}
							retry_count = 0;
							std::cout << __func__ << " -- sdk send, meta_id: " << (*it)->get_metaid() << ", len: " << len << std::endl;
							while (len >= __Recv_len_per) {
								len = (*it)->recv_blk(buf + 32, __Recv_len_per);
								//int ret = write(out_handle, buf + 32, len);
								//tlen += ret;
								//assert(ret == (len));
								//memcpy(&buf[12], &resp_id, 2);
								while (ConnectionSendData((*it)->get_metaid(), (unsigned char*) buf, len + 32) < 0) {
									if (retry_count > MAX_CONN_SEND_RETRY) {
										assert(0);
									}
									std::cout << "send error, retry count: " << retry_count << std::endl;
									retry_count++;
								}
								//std::cout << "resp_id: " << resp_id << std::endl;
								std::cout << __func__ << " -- sdk send, meta_id(connection_id): "
									<< (*it)->get_metaid() << ", len: " << len << std::endl;
								//resp_id++;
							}
							std::cout << "handle_id: " << handle_id << ", sockfd: " << (*it)->get_raw_socket() << std::endl;
							//std::cout << "tlen: " << tlen << std::endl;
							it ++;
						} else {
							it ++;
							std::cout << "len=0, handle_id: " << handle_id << std::endl;
						}
						//syncfs(out_handle);
					} else {
						it++;
					}
					std::cout << "hit" << std::endl;
				}
			}
			if ((ev[i].events & EPOLLHUP) || (ev[i].events & EPOLLERR)) {
				std::cout << __func__ << " -- events: " << ev[i].events << std::endl;
				AutoLockHelper locker(&vecConnectionsLock);
				vector<core_transfer*>::iterator it;
				for (it = vecEndpoints.begin(); it != vecEndpoints.end();) {
					if ((*it)->get_raw_socket() == ev[i].data.fd) {
						std::cout << __func__ << " -- disconnect handle: " << (*it)->get_handle_id()
							<< ", sock: " << (*it)->get_raw_socket() << std::endl;
						(*it)->disconnect_blk();
						unsigned char header[32] = "";
						short handle = (*it)->get_handle_id();
						memcpy(&header[8], &handle, 2);
						header[10] = 1;
						ConnectionSendData((*it)->get_metaid(), header, 32);
						delete (*it);
						it = vecEndpoints.erase(it);
						break ;
					}
					it++;
				}
			}
		}
	}
}


#ifdef WIN32
UINT WINAPI CSendThread::SendThread( LPVOID pParam )
#else
void * CSendThread::SendThread( void * pParam )
#endif // WIN32
{

	CSendThread * pTrd = (CSendThread *)pParam;

#ifndef WIN32
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	pTrd->bRunning = TRUE;
#endif

	int send_count = 0;
	int error_count = 0;

	while(pTrd->bRunning)
	{
#ifdef WIN32
		DWORD thread_waite = WaitForSingleObject(pTrd->m_hExitSendThread, 0);
		if(thread_waite == WAIT_OBJECT_0)
		{
			return -10000;
		}
#else

#endif
// 		if (send_count>1000)
// 		{
// 			AutoLockHelper locker(&vecConnectionsLock);
// 			for (int i=0;i<vecConnections.size();i++)
// 			{
// 				ConnectionClose(vecConnections[i].nDeviceId,vecConnections[i].nConnectionId);
// 				ConnectionInit(vecConnections[i].nDeviceId,&vecConnections[i]);
// 			}
// 			vecConnections.clear();
// 			send_count = 0;
// 		}

		//printf("thread idx: %d, server_not_send_flag: %d\n",
		//	pTrd->thread_id, server_not_send_flag);
		if (!server_not_send_flag)
 		{
//			AutoLockHelper locker1(&mGlobalLock);

			long * pConIds = NULL;
			int nIdCount=0;
			{
				AutoLockHelper locker(&vecConnectionsLock);

				nIdCount = vecConnections.size();
				if (nIdCount>0)
					pConIds=new long[nIdCount];
				if (pConIds)
				for (int i=0;i<vecConnections.size();i++)
				{
					pConIds[i]=vecConnections[i].nConnectionId;
				}
			}

			int rt = 0;
			int len = 0;
			
			for (int i=0;i<nIdCount;i++)
			{
				if (buffing)
				{
					len = (rand() % (200))*1024 + 1024*500;
					sprintf(pTrd->szBuf,"[%d]-[%s]-[%d]",send_count,"Buffing",len);
				}
				else
				{
					sprintf(pTrd->szBuf,"[%d]-[%s]",send_count,((send_count%25)==0)?"I":"P");
					if ((send_count%25)==0)
					{
						len = (rand() % (100))*1000 + 100000;
					}
					else
					{
						len = (rand() % (5000)) + 1000;
					}
				}

				rt = ConnectionSendData(pConIds[i],(unsigned char *)pTrd->szBuf,len);
				printf("trd[%d]->[%ld] ConnectionSendData len %d rt %ld [%d]! \n",pTrd->index,pConIds[i],len,rt,send_count);

				if (rt==0)
					pTrd->buffing_count1++;
				else
					pTrd->buffing_count1=0;

				if (buffing)
				if (pTrd->buffing_count1>buffing_count)
				{
					printf("trd[%d]->[%ld] close buffing_count1 %d \n",pTrd->index,pConIds[i],pTrd->buffing_count1);
					{
						AutoLockHelper locker(&vecConnectionsLock);
						ConnectionClose(vecConnections[i].nDeviceId,vecConnections[i].nConnectionId);
						vector<stConnectionInfo>::iterator itor=vecConnections.begin();
						itor += i;
						vecConnections.erase(itor);
					}
					pTrd->buffing_count1=0;
				}

			}

			send_count++;

			if (pConIds)
				delete []pConIds;
		}
#ifdef WIN32
		Sleep(100);
#else
		usleep(100000);
#endif
	}
#ifdef WIN32
	return 10000;
#else
	return (void*)10000;
#endif
}

