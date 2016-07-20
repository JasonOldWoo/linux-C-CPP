#ifndef __MNDBSERVER_H_
#define __MNDBSERVER_H_



#include <iostream>  
#include <string> 
using namespace std;

#include "udxos.h"
#include "DeviceSDK.h"


#ifdef WIN32

#ifdef _DEBUG
#pragma comment(lib,"../../DeviceSDK/fastudx_win32/DeviceSDK.lib")
#pragma comment(lib,"../../DeviceSDK/fastudx_win32/FastUdx.lib")
#else
#pragma comment(lib,"../../DeviceSDK/fastudx_win32/DeviceSDK.lib")
#pragma comment(lib,"../../DeviceSDK/fastudx_win32/FastUdx.lib")
#endif

#else //linux


#endif // WIN32



#pragma pack( push, 1 )


#define GW_CHANNEL_DATA_LEN					(256)				// 通道数据长度
typedef struct C_Channel_T_STR
{
	C_Channel_T_STR()
	{
		channel = 1;
		stream = 0;
		iFrame = 0;
		rate = 25;
		quality = 5;
		bitrate = 0;
		encode = 2;
		pixel = 0;
		memset(data, 0, GW_CHANNEL_DATA_LEN);
		value = 0;
	};
	unsigned int channel;		// 从0开始
	unsigned int stream;		// 0 主码流，1 辅码流
	unsigned int iFrame;		// I帧间隔
	unsigned int rate;			// 帧率 1~25
	unsigned int quality;		// 画质（1~6，取值5）
	unsigned int bitrate;		// 码流，单位kbps
	unsigned int encode;		// 编码。1.MPEG4, 2.H.264, 3. AVC, 4. H.265
	unsigned int pixel;			// 分辨率
	char	data[GW_CHANNEL_DATA_LEN];	// 数据,可以传文件名一类的数据.
	unsigned int value;			// 传送自定义数据使用
} C_Channel_T;

typedef struct
{
	char* data;
	int len;
}IDM_TRANS_MSG;

#pragma pack( pop)



static void OnSDKInit(stDeviceInfo * pInfo,int nErrno);
static void OnSDKMessage(stDeviceInfo * pInfo,char *pMessageSender,char *pMessageType,unsigned char * pData,unsigned int nLen);
static void OnSDKClose(stDeviceInfo * pInfo);
static void OnSDKTimer(stDeviceInfo * pInfo);

static void OnSDKConnectionInit(stConnectionInfo * pInfo,int nErrno);
static void OnSDKConnectionData(stConnectionInfo * pInfo,unsigned char * pData,unsigned int nLen);
static void OnSDKConnectionClose(stConnectionInfo * pInfo);
static void OnSDKConnectionTimer(stConnectionInfo * pInfo);


void InitDevice();
void InitConnection();

void SendMsgToServer();


void StartSendThread();
void StopSendThread();

#ifdef WIN32
#include <Windows.h>
#include <process.h>
#else
#include<pthread.h>
#endif


//////////////////////////////////////////////////////////////////////////
class AutoLock
{
#ifdef WIN32
public:
	AutoLock()
	{
		InitializeCriticalSection(&g_cs);
	};
	~AutoLock()
	{
		DeleteCriticalSection(&g_cs);
	};

	void Lock()
	{
		EnterCriticalSection(&g_cs);
	};
	void Unlock()
	{
		LeaveCriticalSection(&g_cs);
	};

	CRITICAL_SECTION g_cs;
#else
public:
	AutoLock()
	{
		int rt = pthread_mutex_init(&mutex_lock, NULL);
		printf("pthread_mutex_init %d\n",rt);
	};
	~AutoLock()
	{
		pthread_mutex_destroy(&mutex_lock); 
	};

	void Lock()
	{
		pthread_mutex_lock(&mutex_lock);
	};
	void Unlock()
	{
		pthread_mutex_unlock(&mutex_lock);
	};

	pthread_mutex_t mutex_lock;
#endif
};
//////////////////////////////////////////////////////////////////////////
class AutoLockHelper
{
public:
	AutoLockHelper(AutoLock * lock)
	{
		m_lock=lock;
		if (m_lock)
			m_lock->Lock();
	};
	~AutoLockHelper(void)
	{
		if (m_lock)
			m_lock->Unlock();
	};
private:
	AutoLock * m_lock;
};
//////////////////////////////////////////////////////////////////////////

// 
// #define UDP_LONG unsigned int 
// 
// inline UDP_LONG DEKHash(string str)  
// {  
// 	UDP_LONG hash = str.size();  
// 
// 	unsigned char * p = (unsigned char *)str.c_str();
// 	int len = str.size();
// 
// 	printf("%d %d\r\n",len,strlen((char*)p));
// 	
// 	for(int i = 0; i < str.size(); i++)  
// 	{  
// 		UDP_LONG t1 = 0;
// 		UDP_LONG t2 = 0;
// 		UDP_LONG t3 = 0;
// 		t1 |= ((UDP_LONG) hash << 5);
// 		t2 |= ((UDP_LONG) hash >> 27);
// 		t3 |= ((UDP_LONG) t1 ^ t2 ^ p[i]);
// 
// 
// 		hash = t3;  
// 	}  
// 	return hash;  
// }  
// 

const int nBufLen = 1024*1024;

class CSendThread
{
public:
	CSendThread();
	~CSendThread();

	int start();
	void stop();


	char szBuf[nBufLen];
	int send_count;

	int index;

	int buffing_count1;

	bool bRunning;
#ifdef WIN32

	HANDLE m_hExitSendThread;
	HANDLE m_hSendThread;
	static UINT WINAPI SendThread( LPVOID pParam );

#else

	pthread_t thread_id;
	void *tret;
	static void * SendThread( void * pParam );
#endif // WIN32

};

class CRecvThread {
public:
	CRecvThread();
	~CRecvThread();

	int start();
	void stop();

	bool bRunning;

	pthread_t thread_id;
	static void* RecvThread(void* pParam);
};

#endif
