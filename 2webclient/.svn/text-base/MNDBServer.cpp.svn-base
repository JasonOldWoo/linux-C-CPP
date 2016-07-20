// MNDBServer.cpp : 定义控制台应用程序的入口点。
//

#ifdef WIN32
#include "stdafx.h"
#endif

#include "MNDBServer.h"

//#include "TestExam.h"

AutoLock mGlobalLock;

//////////////////////////////////////////////////////////////////////////

#ifdef WIN32
#else
#define strcpy_s strcpy
#endif

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

AutoLock vecConnectionsLock;
vector<stConnectionInfo> vecConnections;



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

static void OnSDKConnectionData(stConnectionInfo * pInfo,unsigned char * pData,unsigned int nLen)		//接收数据回调
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

	printf("[%s] OnSDKConnectionData %d->%d , len: %d string : %s \r\n",asctime(timenow),(void*)pInfo->nDeviceId,(void*)pInfo->nConnectionId,nLen,pData);

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


int nRunCounter = 0;

#ifdef WIN32
int _tmain(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif // WIN32
{
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

	//创建设备
	InitDevice();
	
	if (mode == 1)//服务端
	{
		//send_thread1.start();
		//send_thread2.start();
		//send_thread3.start();
		//send_thread4.start();
		//send_thread5.start();

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
