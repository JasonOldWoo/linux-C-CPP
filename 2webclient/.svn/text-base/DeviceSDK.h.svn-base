#ifndef __DEVICESDK_H_
#define __DEVICESDK_H_

/************************************************************************/
/* DeviceSDK库                                                          */
/* 版本 1_9_2                                                           */
/* changelog：                                                          */
/*																		*/
/* 1_9_2																*/
/* 重新规划版本号规则，第一个版本从1.9.2开始。2016-2-19					*/
/* 版本号描述：															*/
/*	x：主版本号，重大升级，主版本号更新，不同主版本号的库之间不兼容，		*/
/*	   所以库更新时，如果主版本号变动则不能删除旧的版本库					*/
/*	y：次版本号，增量升级，增加一些新的接口，但保留原有接口，				*/
/*	   高版本的库向低版本库兼容											*/
/*	z：发布版本号，库的一些诸如错误修改，性能改善等，不添加新接口，也不更改	*/
/*	   接口。主版本号与次版本号相同的前提下，不同发布版本之间完全兼容		*/
/*																		*/
/*
2016-2-22
根据最新兼容性规则要求：
1.SDKVersion不允许变动，程序更新时版本更新到最新即可；DeviceSendMsg接口不允许变动
2.SDKInit、SDKSetConfig、SDKSetMode、SDKUnInit、DeviceInit、DeviceClose、DeviceGetUserList等接口在更新时，无需更新接口名称，gwSDK始终用最新的接口。
3.ConnectionInit、ConnectionSetConfig、ConnectionEnableFrameMode、ConnectionClose、ConnectionSendData、ConnectionSendFrame接口，在更新版本时，需要增填对应版本的接口

4.回调函数OnMessage接口不允许变动
5.OnInit、OnClose等接口在更新时，无需更新接口名称，gwSDK始终用最新的接口。
6.OnConnectionInit、OnConnectionData、OnConnectionClose接口，在更新版本时，需要增填对应版本的接口
*/
/************************************************************************/


#ifdef WIN32

#include <windows.h>

#ifdef  DEVICEDLL
#define DEVICE_EXPORT  extern "C" __declspec(dllexport)
#else
#define DEVICE_EXPORT  extern "C" __declspec(dllimport)
#endif

#else
#define DEVICE_EXPORT
#endif



#ifdef WIN32

#pragma pack( push, 1 )
#define SDKPACKED 

#else
#define SDKPACKED	__attribute__((packed, aligned(1)))
#endif


DEVICE_EXPORT const char * SDKVersion
	(
	);


//////////////////////////////////////////////////////////////////////////
//初始化SDK，在使用sdk之前调用
//
//参数：
//
//返回值：无
//
DEVICE_EXPORT void SDKInit
	(	
	);

	
//////////////////////////////////////////////////////////////////////////
//设置SDK配置文件，必须在SDKInit之后调用。
//
//参数：
//pIniFilePath ini配置文件路径。
//
//如：pIniFilePath为c:\(win下)  或  /usr/(linux下)
//ini文件为c:\config.ini(win下)  或  /usr/config.ini(linux下)
//日志文件保存路径为c:\log\(win下)  或  /usr/log/(linux下)
//
//ini配置文件内容示例：
//[main]
//	name=SDKTest   //日志文件名前缀
//	level=all      //全部日志
//	display=1      //打印开关，0关闭，1打开
//	outfile=1      //输出到文件开团，0关闭，1打开
//	monthdir=0     //月目录开关，0关闭，1打开
//	limitsize=50   //单个文件大小限制，单位MB
//	fileline=0     //行号开关，0关闭，1打开
//	enable=1       //日志开关，0关闭，1打开
//
//返回值：
//	SDK_RV_SUCCEEDED                    = 0,		//成功
//	SDK_RV_NULL_POINTER					= -4,		//空指针
//	SDK_RV_FILE_NOT_EXIST				= -17		//文件不存在
//
DEVICE_EXPORT int SDKSetConfig
	(	
	char * pIniFilePath
	);


//////////////////////////////////////////////////////////////////////////
//SDK模式结构，记录全局模式设置
//
//导出日志函数指针
typedef void (*fSDKExportLog)(char * pLog,unsigned int nLen);
struct stSDKMode
{
public:
	stSDKMode()
	{
		pExportLog = NULL;
	};
	fSDKExportLog pExportLog;//导出日志函数指针。
}SDKPACKED;
//////////////////////////////////////////////////////////////////////////
//设置SDK模式，必须在SDKInit之后调用。
//
//参数：
DEVICE_EXPORT void SDKSetMode
	(
		stSDKMode * pSDKMode
	);

//////////////////////////////////////////////////////////////////////////
//反初始化SDK，在使用sdk完之后调用
//
//参数：无
//
//返回值：无
//
DEVICE_EXPORT void SDKUnInit
	(
	);






//////////////////////////////////////////////////////////////////////////
/*字符串均以/0作为结尾，且均未加密*/
#define MAX_ID_LENGTH 64                      //设备id字符串最大长度
#define MAX_IP_ADDR_LENGTH 64                  //ip地址最大长度

#define MAX_NAME_LENGTH 32                    //用户名字符串最大长度
#define MAX_PASSWORD_LENGTH 32                 //用户密码字符串最大长度
//////////////////////////////////////////////////////////////////////////
//设备类型字符串
#define MAX_TYPE_STR_LENGTH 50                      //类型字符串最大长度
static char * SDK_DEVICE_TYPE_STR[]=
{
	"SDK_DT_IPC" ,          //IPC设备
	"SDK_DT_PC" ,			//PC设备
	"SDK_DT_Mobile"      //移动端设备
};
//////////////////////////////////////////////////////////////////////////
//连接类型字符串
static char * SDK_CONNECTION_TYPE_STR[]=
{
	"SDK_CT_P2P"      ,         //P2P
	"SDK_CT_TRANSFER"           //中转
};
//////////////////////////////////////////////////////////////////////////
//连接模式字符串
#define MAX_MODE_STR_LENGTH 50                      //模式字符串最大长度
static char * SDK_CONNECTION_MODE_STR[]=
{
	"SDK_CM_AUTO"    ,         //自动连接，由服务器告知服务器信息
	"SDK_CM_SPECIAL"           //指定连接
};
//////////////////////////////////////////////////////////////////////////
//错误码，对应于回调中nError
enum SDK_ERROR_NUM
{
	SDK_EN_SUCCEEDED                    = 0,			//成功
	SDK_EN_SVR_CONNECT_FAILED           = -10000,		//连接服务器失败
	SDK_EN_SVR_REGISTER_REFUSED         = -10001,		//注册拒绝（冲突、超时等）
	SDK_EN_DEV_CONNECTREQ_FAILED        = -10002,		//连接请求失败
	SDK_EN_DEV_P2P_CONNECT_FAILED		= -10003,		//P2P连接失败
	SDK_EN_DEV_TRANSFER_CONNECT_FAILED  = -10004,		//中转连接失败
	SDK_EN_CONNECT_FAILED				= -10005		//连接失败
};

//返回值
//如果发送数据，返回值大于0，一般为数据发送的实际长度。
enum SDK_RETURN_VALUE
{
	SDK_RV_SUCCEEDED                    = 0,		//成功
	SDK_RV_BUFFERING                    = 0,		//成功，缓存中，只针对发送数据
	SDK_RV_UNKOWN_TYPE					= -1,		//类型错误
	SDK_RV_UNKOWN_MODE					= -2,		//模式错误
	SDK_RV_NAME_ERROR					= -3,		//名称错误
	SDK_RV_NULL_POINTER					= -4,		//空指针
	SDK_RV_ERROR_LENGTH					= -5,		//长度错误，为0，或超长，或不够指定长度
	SDK_RV_STRING_OVERLENGTH			= -6,		//字符串超过最大长度
	SDK_RV_DEVICE_NOTEXIST				= -7,		//设备不存在
	SDK_RV_CONNECTION_NOTEXIST			= -8,		//连接不存在
	SDK_RV_UDX_CREATE_FAILED			= -9,		//UDX创建失败
	SDK_RV_UDX_NOTEXIST					= -10,		//UDX连接不存在
	SDK_RV_UDX_DISCONNECT				= -11,		//UDX连接断开
	SDK_RV_UDX_CONNECT_FAILED			= -12,		//UDX连接失败
	SDK_RV_SVR_NOTEXIST					= -13,		//服务器连接不存在
	SDK_RV_SVR_DISCONNECT				= -14,		//服务器连接断开
	SDK_RV_SVR_NOTREGISTERED			= -15,		//未在服务器注册
	SDK_RV_SEND_MODE_ERROR				= -16,		//发送模式错误
	SDK_RV_FILE_NOT_EXIST				= -17,		//文件不存在
	SDK_RV_MODE_ERROR					= -18		//模式错误
};

/* 数据结构开始 */


//////////////////////////////////////////////////////////////////////////
struct stConnectionInfo;
//////////////////////////////////////////////////////////////////////////
//设备信息结构，记录设备相关信息
//
struct stDeviceInfo
{
	stDeviceInfo()
	{
		memset(szId,0,MAX_ID_LENGTH);
		memset(szType,0,MAX_TYPE_STR_LENGTH);
		nPort = 0;
		memset(szServerIp,0,MAX_IP_ADDR_LENGTH);
		nServerPort = 0;
		memset(szUser,0,MAX_NAME_LENGTH);
		memset(szPwd,0,MAX_PASSWORD_LENGTH);
		bIsSync = false;
		nDeviceContext = 0;
		nDeviceId = 0;
		OnInit = NULL;
		OnMessage = NULL;
		OnClose = NULL;
		OnTimer = NULL;
		OnConnectionInit = NULL;
		OnConnectionData = NULL;
		OnConnectionFrame = NULL;
		OnConnectionClose = NULL;
		OnConnectionTimer = NULL;
	};
	char szId[MAX_ID_LENGTH];                        //设备id、名称
	char szType[MAX_TYPE_STR_LENGTH];                //设备类型字符串
	unsigned short nPort;							//本地端口号，比如6000

	char szServerIp[MAX_IP_ADDR_LENGTH];			//服务器ip，比如"192.168.1.1"
	unsigned short nServerPort;                      //服务器端口号，比如8000

	char szUser[MAX_NAME_LENGTH];                     //用户名字符串，比如"root"
	char szPwd[MAX_PASSWORD_LENGTH];                  //密码字符串，比如"123456"

	bool bIsSync;									//同步方式：true同步方式，false异步方式

	long nDeviceContext;									//用户输入的上下文，在回调函数中返回给用户，比如，可以为窗口指针
	long nDeviceId;									//设备ID，用于表示本地设备，成功后在回调中赋值

	/************************************************************************************/
	/* 设备初始化回调函数																	*/
	/* 参数：																			*/
	/* pInfo 设备信息结构指针，为DeviceInit函数传入的信息，nDeviceId保存以标识本地设备		*/
	/* nErrno 错误码，																	*/
	/*			SDK_EN_SUCCEEDED注册成功，												*/
	/*			SDK_EN_SVR_CONNECT_FAILED连接服务器失败，将会重连							*/
	/*			SDK_EN_SVR_REGISTER_REFUSED注册冲突										*/
	/************************************************************************************/
	void (*OnInit)(stDeviceInfo * pInfo,int nErrno);									//初始化回调
	/************************************************************************************/
	/* 设备接收消息回调函数																*/
	/* 参数：																			*/
	/* pInfo 设备信息结构指针，为DeviceInit函数传入的信息									*/
	/* pMessageSender 消息发送者名称字符串												*/
	/* pMessageType 消息类型字符串														*/
	/* pData 接收到的消息数据的指针														*/
	/* nLen 接收到的消息数据长度															*/
	/************************************************************************************/
	void (*OnMessage)(stDeviceInfo * pInfo,char *pMessageSender,char *pMessageType,unsigned char * pData,unsigned int nLen);	//接收消息回调
	/************************************************************************************/
	/* 设备关闭回调函数																	*/
	/* 参数：																			*/
	/* pInfo 设备信息结构指针，为DeviceInit函数传入的信息									*/
	/************************************************************************************/
	void (*OnClose)(stDeviceInfo * pInfo);												//关闭回调
	/************************************************************************************/
	/* 设备定时器回调函数																	*/
	/* 参数：																			*/
	/* pInfo 设备信息结构指针，为DeviceInit函数传入的信息									*/
	/************************************************************************************/
	void (*OnTimer)(stDeviceInfo * pInfo);												//定时器回调，50ms


	/************************************************************************************/
	/* 连接初始化回调函数																	*/
	/* 参数：																			*/
	/* pInfo 为连接信息，																*/
	/* nErrno 错误码，																	*/
	/*       SDK_EN_SUCCEEDED成功，nConnectionId保存以标识产生的连接						*/
	/*		 SDK_EN_DEV_CONNECTREQ_FAILED连接请求失败,									*/
	/*		 SDK_EN_DEV_P2P_CONNECT_FAILED直接连接失败,									*/
	/*		 SDK_EN_DEV_TRANSFER_CONNECT_FAILED中转连接失败,								*/
	/************************************************************************************/
	void (*OnConnectionInit)(stConnectionInfo * pInfo,int nErrno);						//连接初始化回调
	/************************************************************************************/
	/* 设备接收数据回调函数																*/
	/* 参数：																			*/
	/* pInfo 设备信息结构指针，为ConnectionInit函数传入的信息								*/
	/* pData 接收到的消息数据的指针														*/
	/* nLen 接收到的消息数据长度															*/
	/************************************************************************************/
	void (*OnConnectionData)(stConnectionInfo * pInfo,unsigned char * pData,unsigned int nLen);		//接收数据回调
	/************************************************************************************/
	/* 设备接收帧回调函数																	*/
	/* 参数：																			*/
	/* pInfo 设备信息结构指针，为ConnectionInit函数传入的信息								*/
	/* bVideo 是否视频																	*/
	/* bKeyFrame 是否关键帧																*/
	/* pData 接收到的消息数据的指针														*/
	/* nLen 接收到的消息数据长度															*/
	/* nType2 类型2																		*/
	/************************************************************************************/
	void (*OnConnectionFrame)(stConnectionInfo * pInfo,	bool bVideo,bool bKeyFrame,unsigned char * pData,unsigned int nLen,int nType2);		//接收帧回调
	/************************************************************************************/
	/* 连接关闭回调函数																	*/
	/* 参数：																			*/
	/* pInfo 设备信息结构指针，为ConnectionInit函数传入的信息								*/
	/************************************************************************************/
	void (*OnConnectionClose)(stConnectionInfo * pInfo);								//连接关闭回调
	/************************************************************************************/
	/* 连接定时器回调函数																	*/
	/* 参数：																			*/
	/* pInfo 设备信息结构指针，为ConnectionInit函数传入的信息								*/
	/************************************************************************************/
	void (*OnConnectionTimer)(stConnectionInfo * pInfo);												//连接定时器回调，50ms

}SDKPACKED;
//////////////////////////////////////////////////////////////////////////
//连接信息结构，记录连接设备信息
//
struct stConnectionInfo
{
	stConnectionInfo()
	{
		blPromoter = true;
		memset(szName,0,MAX_ID_LENGTH);
		bIsSync = false;

		nContext = 0;
		nConnectionId = 0;
		nDeviceId = 0;

		memset(szMode,0,MAX_MODE_STR_LENGTH);
		memset(szType,0,MAX_TYPE_STR_LENGTH);

		memset(szTargetServerIp,0,MAX_IP_ADDR_LENGTH);
		nTargetServerPort = 0;

		memset(szRemoteIp,0,MAX_IP_ADDR_LENGTH);
		nRemotePort = 0;

		nDeviceContext = 0;
	};
	bool blPromoter;						//是否连接发起者
	char szName[MAX_ID_LENGTH];				//目标设备名称
	bool bIsSync;							//同步方式：true同步方式，false异步方式

 	long nContext;									//用户输入的上下文，在回调函数中返回给用户，比如，可以为窗口指针
	long nDeviceId;
	long nConnectionId;								//连接ID，成功后，在回调中赋值

	char szMode[MAX_MODE_STR_LENGTH];			//连接模式字符串
	char szType[MAX_TYPE_STR_LENGTH];			//连接类型字符串

	char szTargetServerIp[MAX_IP_ADDR_LENGTH];	//目标服务器IP
	unsigned short nTargetServerPort;			//目标服务器端口

	char szRemoteIp[MAX_IP_ADDR_LENGTH];	//远端IP地址，连接建立成功后为有效的ip地址
	unsigned short nRemotePort;			//远端端口，连接建立成功后为有效的ip端口

	long nDeviceContext;							//DeviceInit时，从数据结构stDeviceInfo中输入的nContext值
}SDKPACKED;
//////////////////////////////////////////////////////////////////////////
//连接配置数据结构，保存连接的配置信息
//
struct stConnectionConfig
{
public:
	stConnectionConfig()
	{
		nConstRate = 0;
		nMaxRate = 0;
		nMinRate = 0;
		nBufferSize = 0;
		nMaxLostRate = 0;
	};

	unsigned int nConstRate;					//恒定速率
	unsigned int nMaxRate;						//最大速率
	unsigned int nMinRate;						//最小速率
	unsigned int nBufferSize;					//缓存大小
	unsigned int nMaxLostRate;					//最大丢包率
}SDKPACKED;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//消息类型字符串
static char * SDK_MESSAGE_TYPE_STR[]=
{
	"SDK_MT_DEVICE_MSG"             //
};


#ifdef WIN32
#pragma pack(pop)
#endif



/* 函数接口开始 */

//////////////////////////////////////////////////////////////////////////
//初始化设备
//
//参数：
//pDeviceInfo 设备信息结构指针，不能为空
//
//返回值：
//	SDK_RV_SUCCEEDED                    = 0,		//成功
//	SDK_RV_UNKOWN_TYPE					= -1,		//类型错误
//	SDK_RV_NULL_POINTER					= -4,		//空指针
//	SDK_RV_UDX_CREATE_FAILED			= -9,		//UDX创建失败
//	SDK_RV_UDX_CONNECT_FAILED			= -12,		//UDX连接失败
//
//在回调OnInit中确认设备是否建立，是否在服务器注册成功
//
DEVICE_EXPORT int DeviceInit
	(
	stDeviceInfo * pDeviceInfo
	);
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//关闭本地设备，同时将关闭所有与其他设备连接
//
//参数：
//nDeviceId 为设备id，为OnInit函数返回的设备id
//
//返回值：
//无
//
DEVICE_EXPORT void DeviceClose
	(
	long nDeviceId
	);
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//连接初始化，用于本地设备和其他设备建立连接
//
//参数：
//nDeviceId 为设备id，为OnInit函数返回的设备id
//pConnectionInfo  连接信息指针，不能为空
//
//返回值：
//
//	SDK_RV_SUCCEEDED                    = 0,		//成功
//	SDK_RV_UNKOWN_TYPE					= -1,		//类型错误
//	SDK_RV_UNKOWN_MODE					= -2,		//模式错误
//	SDK_RV_NAME_ERROR					= -3,		//名称错误
//	SDK_RV_NULL_POINTER					= -4,		//空指针
//	SDK_RV_STRING_OVERLENGTH			= -6,		//字符串超过最大长度
//	SDK_RV_DEVICE_NOTEXIST				= -7,		//设备不存在
//	SDK_RV_SVR_NOTEXIST					= -13,		//服务器连接不存在
//	SDK_RV_SVR_NOTREGISTERED			= -15		//未在服务器注册
//
//在回调OnConnectionInit中确认连接是否建立
//
DEVICE_EXPORT int ConnectionInit
	(
	long nDeviceId,
	stConnectionInfo * pConnectionInfo
	);
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//连接设置，如未调用过，使用默认值
//
//参数：
//nConnectionId 连接ID，为OnConnectionInit函数返回的值
//pCfg 为连接配置数据结构指针，不能为空
//
//返回值：无
//
DEVICE_EXPORT void ConnectionSetConfig
	(
	long nConnectionId,
	stConnectionConfig * pCfg
	);
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//帧模式设置，默认帧模式为关闭
//
//参数：
//nConnectionId 连接ID，为OnConnectionInit函数返回的值
//bEnable 是否打开帧模式，true打开，false关闭
//
//返回值：
//	SDK_RV_SUCCEEDED                    = 0,		//成功
//	SDK_RV_UNKOWN_TYPE					= -1,		//类型错误
//	SDK_RV_CONNECTION_NOTEXIST			= -8,		//连接不存在
//
DEVICE_EXPORT int ConnectionEnableFrameMode
	(
	long nConnectionId,
	bool bEnable
	);
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//关闭连接
//
//参数：
//nDeviceId 为设备id，为OnInit函数返回的设备id
//nConnectionId	连接id，为OnInit函数返回的值
//
//返回值：
//无
//
DEVICE_EXPORT void ConnectionClose
	(
	long nDeviceId,
	long nConnectionId
	);
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
//发送消息，用于通过服务器转发控制消息到目标设备
//
//参数：
//nDeviceId 为设备id，为OnInit函数返回的设备id
//pName         目标设备名称，不能为空。如果为字符串“”，则表示发送给服务器，否则为具体的目标设备。
//pType			消息类型字符串，只能使用SDK_MESSAGE_TYPE_STR中指定的类型
//pData         消息数据地址
//nLen          消息数据长度
//
//返回值：
//大于0 发送成功，已发送长度
//	SDK_RV_BUFFERING                    = 0,		//成功，缓存中，只针对发送数据
//	SDK_RV_UNKOWN_TYPE					= -1,		//类型错误
//	SDK_RV_NULL_POINTER					= -4,		//空指针
//	SDK_RV_ERROR_LENGTH					= -5,		//长度错误，为0，或超长，或不够指定长度
//	SDK_RV_STRING_OVERLENGTH			= -6,		//字符串超过最大长度
//	SDK_RV_DEVICE_NOTEXIST				= -7,		//设备不存在
//	SDK_RV_UDX_NOTEXIST					= -10,		//UDX连接不存在
//	SDK_RV_UDX_DISCONNECT				= -11,		//UDX连接断开
DEVICE_EXPORT int DeviceSendMsg
	(
	long nDeviceId,
	char * pName,
	char * pType,
	unsigned char * pData,
	unsigned int nLen
	);
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//发送数据
//
//参数：
//nConnectionId	连接id，为OnConnectionInit函数返回的值
//pData          数据地址
//nLen           数据长度
//
//返回值：
//大于0 发送成功，已发送长度
//	SDK_RV_BUFFERING                    = 0,		//成功，缓存中，只针对发送数据
//	SDK_RV_UNKOWN_TYPE					= -1,		//类型错误
//	SDK_RV_NULL_POINTER					= -4,		//空指针
//	SDK_RV_ERROR_LENGTH					= -5,		//长度错误，为0，或超长，或不够指定长度
//	SDK_RV_CONNECTION_NOTEXIST			= -8,		//连接不存在
//	SDK_RV_UDX_NOTEXIST					= -10,		//UDX连接不存在
//	SDK_RV_UDX_DISCONNECT				= -11,		//UDX连接断开
//  SDK_RV_SEND_MODE_ERROR				= -16		//发送模式错误
//
DEVICE_EXPORT int ConnectionSendData
	(
	long nConnectionId,
	unsigned char *pData,
	unsigned int nLen
	);
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
//通过连接发送音视频帧数据，连接类型为P2P或TRANSFER
//
//参数：
//nConnectionId	连接id，为OnConnectionInit函数返回的值
//bVideo 是否视频
//bKeyFrame 是否关键帧
//pData 数据指针
//nLen 数据长度
//nType2 类型2
//
//返回值：
//	SDK_RV_SUCCEEDED                    = 0,		//成功
// 	SDK_RV_UNKOWN_TYPE					= -1,		//类型错误
// 	SDK_RV_NULL_POINTER					= -4,		//空指针
// 	SDK_RV_ERROR_LENGTH					= -5,		//长度错误，为0，或超长，或不够指定长度
// 	SDK_RV_CONNECTION_NOTEXIST			= -8,		//连接不存在
// 	SDK_RV_UDX_NOTEXIST					= -10,		//UDX连接不存在
// 	SDK_RV_UDX_DISCONNECT				= -11,		//UDX连接断开
//  SDK_RV_SEND_MODE_ERROR				= -16		//发送模式错误

DEVICE_EXPORT int ConnectionSendFrame
	(
	long nConnectionId,
	bool bVideo,
	bool bKeyFrame,
	unsigned char *pData,
	unsigned int nLen,
	int nType2
	);
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
//获取在服务器上注册的用户列表
//
//参数：
//nDeviceId 为设备id，为OnInit函数返回的设备id
//
//返回值：
//	SDK_RV_SUCCEEDED                    = 0,		//成功
//	SDK_RV_NULL_POINTER					= -4,		//空指针
//	SDK_RV_CONNECTION_NOTEXIST			= -8,		//连接不存在
//	SDK_RV_UDX_NOTEXIST					= -10,		//UDX连接不存在
//	SDK_RV_UDX_DISCONNECT				= -11,		//UDX连接断开
//	SDK_RV_SVR_NOTEXIST					= -13,		//服务器连接不存在
//	SDK_RV_SVR_NOTREGISTERED			= -15		//未在服务器注册
//
DEVICE_EXPORT int DeviceGetUserList
	(
	long nDeviceId 
	);
//////////////////////////////////////////////////////////////////////////



#endif