#ifndef __DEVICESDK_H_
#define __DEVICESDK_H_

/************************************************************************/
/* DeviceSDK��                                                          */
/* �汾 1_9_2                                                           */
/* changelog��                                                          */
/*																		*/
/* 1_9_2																*/
/* ���¹滮�汾�Ź��򣬵�һ���汾��1.9.2��ʼ��2016-2-19					*/
/* �汾��������															*/
/*	x�����汾�ţ��ش����������汾�Ÿ��£���ͬ���汾�ŵĿ�֮�䲻���ݣ�		*/
/*	   ���Կ����ʱ��������汾�ű䶯����ɾ���ɵİ汾��					*/
/*	y���ΰ汾�ţ���������������һЩ�µĽӿڣ�������ԭ�нӿڣ�				*/
/*	   �߰汾�Ŀ���Ͱ汾�����											*/
/*	z�������汾�ţ����һЩ��������޸ģ����ܸ��Ƶȣ��������½ӿڣ�Ҳ������	*/
/*	   �ӿڡ����汾����ΰ汾����ͬ��ǰ���£���ͬ�����汾֮����ȫ����		*/
/*																		*/
/*
2016-2-22
�������¼����Թ���Ҫ��
1.SDKVersion�������䶯���������ʱ�汾���µ����¼��ɣ�DeviceSendMsg�ӿڲ������䶯
2.SDKInit��SDKSetConfig��SDKSetMode��SDKUnInit��DeviceInit��DeviceClose��DeviceGetUserList�Ƚӿ��ڸ���ʱ��������½ӿ����ƣ�gwSDKʼ�������µĽӿڡ�
3.ConnectionInit��ConnectionSetConfig��ConnectionEnableFrameMode��ConnectionClose��ConnectionSendData��ConnectionSendFrame�ӿڣ��ڸ��°汾ʱ����Ҫ�����Ӧ�汾�Ľӿ�

4.�ص�����OnMessage�ӿڲ������䶯
5.OnInit��OnClose�Ƚӿ��ڸ���ʱ��������½ӿ����ƣ�gwSDKʼ�������µĽӿڡ�
6.OnConnectionInit��OnConnectionData��OnConnectionClose�ӿڣ��ڸ��°汾ʱ����Ҫ�����Ӧ�汾�Ľӿ�
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
//��ʼ��SDK����ʹ��sdk֮ǰ����
//
//������
//
//����ֵ����
//
DEVICE_EXPORT void SDKInit
	(	
	);

	
//////////////////////////////////////////////////////////////////////////
//����SDK�����ļ���������SDKInit֮����á�
//
//������
//pIniFilePath ini�����ļ�·����
//
//�磺pIniFilePathΪc:\(win��)  ��  /usr/(linux��)
//ini�ļ�Ϊc:\config.ini(win��)  ��  /usr/config.ini(linux��)
//��־�ļ�����·��Ϊc:\log\(win��)  ��  /usr/log/(linux��)
//
//ini�����ļ�����ʾ����
//[main]
//	name=SDKTest   //��־�ļ���ǰ׺
//	level=all      //ȫ����־
//	display=1      //��ӡ���أ�0�رգ�1��
//	outfile=1      //������ļ����ţ�0�رգ�1��
//	monthdir=0     //��Ŀ¼���أ�0�رգ�1��
//	limitsize=50   //�����ļ���С���ƣ���λMB
//	fileline=0     //�кſ��أ�0�رգ�1��
//	enable=1       //��־���أ�0�رգ�1��
//
//����ֵ��
//	SDK_RV_SUCCEEDED                    = 0,		//�ɹ�
//	SDK_RV_NULL_POINTER					= -4,		//��ָ��
//	SDK_RV_FILE_NOT_EXIST				= -17		//�ļ�������
//
DEVICE_EXPORT int SDKSetConfig
	(	
	char * pIniFilePath
	);


//////////////////////////////////////////////////////////////////////////
//SDKģʽ�ṹ����¼ȫ��ģʽ����
//
//������־����ָ��
typedef void (*fSDKExportLog)(char * pLog,unsigned int nLen);
struct stSDKMode
{
public:
	stSDKMode()
	{
		pExportLog = NULL;
	};
	fSDKExportLog pExportLog;//������־����ָ�롣
}SDKPACKED;
//////////////////////////////////////////////////////////////////////////
//����SDKģʽ��������SDKInit֮����á�
//
//������
DEVICE_EXPORT void SDKSetMode
	(
		stSDKMode * pSDKMode
	);

//////////////////////////////////////////////////////////////////////////
//����ʼ��SDK����ʹ��sdk��֮�����
//
//��������
//
//����ֵ����
//
DEVICE_EXPORT void SDKUnInit
	(
	);






//////////////////////////////////////////////////////////////////////////
/*�ַ�������/0��Ϊ��β���Ҿ�δ����*/
#define MAX_ID_LENGTH 64                      //�豸id�ַ�����󳤶�
#define MAX_IP_ADDR_LENGTH 64                  //ip��ַ��󳤶�

#define MAX_NAME_LENGTH 32                    //�û����ַ�����󳤶�
#define MAX_PASSWORD_LENGTH 32                 //�û������ַ�����󳤶�
//////////////////////////////////////////////////////////////////////////
//�豸�����ַ���
#define MAX_TYPE_STR_LENGTH 50                      //�����ַ�����󳤶�
static char * SDK_DEVICE_TYPE_STR[]=
{
	"SDK_DT_IPC" ,          //IPC�豸
	"SDK_DT_PC" ,			//PC�豸
	"SDK_DT_Mobile"      //�ƶ����豸
};
//////////////////////////////////////////////////////////////////////////
//���������ַ���
static char * SDK_CONNECTION_TYPE_STR[]=
{
	"SDK_CT_P2P"      ,         //P2P
	"SDK_CT_TRANSFER"           //��ת
};
//////////////////////////////////////////////////////////////////////////
//����ģʽ�ַ���
#define MAX_MODE_STR_LENGTH 50                      //ģʽ�ַ�����󳤶�
static char * SDK_CONNECTION_MODE_STR[]=
{
	"SDK_CM_AUTO"    ,         //�Զ����ӣ��ɷ�������֪��������Ϣ
	"SDK_CM_SPECIAL"           //ָ������
};
//////////////////////////////////////////////////////////////////////////
//�����룬��Ӧ�ڻص���nError
enum SDK_ERROR_NUM
{
	SDK_EN_SUCCEEDED                    = 0,			//�ɹ�
	SDK_EN_SVR_CONNECT_FAILED           = -10000,		//���ӷ�����ʧ��
	SDK_EN_SVR_REGISTER_REFUSED         = -10001,		//ע��ܾ�����ͻ����ʱ�ȣ�
	SDK_EN_DEV_CONNECTREQ_FAILED        = -10002,		//��������ʧ��
	SDK_EN_DEV_P2P_CONNECT_FAILED		= -10003,		//P2P����ʧ��
	SDK_EN_DEV_TRANSFER_CONNECT_FAILED  = -10004,		//��ת����ʧ��
	SDK_EN_CONNECT_FAILED				= -10005		//����ʧ��
};

//����ֵ
//����������ݣ�����ֵ����0��һ��Ϊ���ݷ��͵�ʵ�ʳ��ȡ�
enum SDK_RETURN_VALUE
{
	SDK_RV_SUCCEEDED                    = 0,		//�ɹ�
	SDK_RV_BUFFERING                    = 0,		//�ɹ��������У�ֻ��Է�������
	SDK_RV_UNKOWN_TYPE					= -1,		//���ʹ���
	SDK_RV_UNKOWN_MODE					= -2,		//ģʽ����
	SDK_RV_NAME_ERROR					= -3,		//���ƴ���
	SDK_RV_NULL_POINTER					= -4,		//��ָ��
	SDK_RV_ERROR_LENGTH					= -5,		//���ȴ���Ϊ0���򳬳����򲻹�ָ������
	SDK_RV_STRING_OVERLENGTH			= -6,		//�ַ���������󳤶�
	SDK_RV_DEVICE_NOTEXIST				= -7,		//�豸������
	SDK_RV_CONNECTION_NOTEXIST			= -8,		//���Ӳ�����
	SDK_RV_UDX_CREATE_FAILED			= -9,		//UDX����ʧ��
	SDK_RV_UDX_NOTEXIST					= -10,		//UDX���Ӳ�����
	SDK_RV_UDX_DISCONNECT				= -11,		//UDX���ӶϿ�
	SDK_RV_UDX_CONNECT_FAILED			= -12,		//UDX����ʧ��
	SDK_RV_SVR_NOTEXIST					= -13,		//���������Ӳ�����
	SDK_RV_SVR_DISCONNECT				= -14,		//���������ӶϿ�
	SDK_RV_SVR_NOTREGISTERED			= -15,		//δ�ڷ�����ע��
	SDK_RV_SEND_MODE_ERROR				= -16,		//����ģʽ����
	SDK_RV_FILE_NOT_EXIST				= -17,		//�ļ�������
	SDK_RV_MODE_ERROR					= -18		//ģʽ����
};

/* ���ݽṹ��ʼ */


//////////////////////////////////////////////////////////////////////////
struct stConnectionInfo;
//////////////////////////////////////////////////////////////////////////
//�豸��Ϣ�ṹ����¼�豸�����Ϣ
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
	char szId[MAX_ID_LENGTH];                        //�豸id������
	char szType[MAX_TYPE_STR_LENGTH];                //�豸�����ַ���
	unsigned short nPort;							//���ض˿ںţ�����6000

	char szServerIp[MAX_IP_ADDR_LENGTH];			//������ip������"192.168.1.1"
	unsigned short nServerPort;                      //�������˿ںţ�����8000

	char szUser[MAX_NAME_LENGTH];                     //�û����ַ���������"root"
	char szPwd[MAX_PASSWORD_LENGTH];                  //�����ַ���������"123456"

	bool bIsSync;									//ͬ����ʽ��trueͬ����ʽ��false�첽��ʽ

	long nDeviceContext;									//�û�����������ģ��ڻص������з��ظ��û������磬����Ϊ����ָ��
	long nDeviceId;									//�豸ID�����ڱ�ʾ�����豸���ɹ����ڻص��и�ֵ

	/************************************************************************************/
	/* �豸��ʼ���ص�����																	*/
	/* ������																			*/
	/* pInfo �豸��Ϣ�ṹָ�룬ΪDeviceInit�����������Ϣ��nDeviceId�����Ա�ʶ�����豸		*/
	/* nErrno �����룬																	*/
	/*			SDK_EN_SUCCEEDEDע��ɹ���												*/
	/*			SDK_EN_SVR_CONNECT_FAILED���ӷ�����ʧ�ܣ���������							*/
	/*			SDK_EN_SVR_REGISTER_REFUSEDע���ͻ										*/
	/************************************************************************************/
	void (*OnInit)(stDeviceInfo * pInfo,int nErrno);									//��ʼ���ص�
	/************************************************************************************/
	/* �豸������Ϣ�ص�����																*/
	/* ������																			*/
	/* pInfo �豸��Ϣ�ṹָ�룬ΪDeviceInit�����������Ϣ									*/
	/* pMessageSender ��Ϣ�����������ַ���												*/
	/* pMessageType ��Ϣ�����ַ���														*/
	/* pData ���յ�����Ϣ���ݵ�ָ��														*/
	/* nLen ���յ�����Ϣ���ݳ���															*/
	/************************************************************************************/
	void (*OnMessage)(stDeviceInfo * pInfo,char *pMessageSender,char *pMessageType,unsigned char * pData,unsigned int nLen);	//������Ϣ�ص�
	/************************************************************************************/
	/* �豸�رջص�����																	*/
	/* ������																			*/
	/* pInfo �豸��Ϣ�ṹָ�룬ΪDeviceInit�����������Ϣ									*/
	/************************************************************************************/
	void (*OnClose)(stDeviceInfo * pInfo);												//�رջص�
	/************************************************************************************/
	/* �豸��ʱ���ص�����																	*/
	/* ������																			*/
	/* pInfo �豸��Ϣ�ṹָ�룬ΪDeviceInit�����������Ϣ									*/
	/************************************************************************************/
	void (*OnTimer)(stDeviceInfo * pInfo);												//��ʱ���ص���50ms


	/************************************************************************************/
	/* ���ӳ�ʼ���ص�����																	*/
	/* ������																			*/
	/* pInfo Ϊ������Ϣ��																*/
	/* nErrno �����룬																	*/
	/*       SDK_EN_SUCCEEDED�ɹ���nConnectionId�����Ա�ʶ����������						*/
	/*		 SDK_EN_DEV_CONNECTREQ_FAILED��������ʧ��,									*/
	/*		 SDK_EN_DEV_P2P_CONNECT_FAILEDֱ������ʧ��,									*/
	/*		 SDK_EN_DEV_TRANSFER_CONNECT_FAILED��ת����ʧ��,								*/
	/************************************************************************************/
	void (*OnConnectionInit)(stConnectionInfo * pInfo,int nErrno);						//���ӳ�ʼ���ص�
	/************************************************************************************/
	/* �豸�������ݻص�����																*/
	/* ������																			*/
	/* pInfo �豸��Ϣ�ṹָ�룬ΪConnectionInit�����������Ϣ								*/
	/* pData ���յ�����Ϣ���ݵ�ָ��														*/
	/* nLen ���յ�����Ϣ���ݳ���															*/
	/************************************************************************************/
	void (*OnConnectionData)(stConnectionInfo * pInfo,unsigned char * pData,unsigned int nLen);		//�������ݻص�
	/************************************************************************************/
	/* �豸����֡�ص�����																	*/
	/* ������																			*/
	/* pInfo �豸��Ϣ�ṹָ�룬ΪConnectionInit�����������Ϣ								*/
	/* bVideo �Ƿ���Ƶ																	*/
	/* bKeyFrame �Ƿ�ؼ�֡																*/
	/* pData ���յ�����Ϣ���ݵ�ָ��														*/
	/* nLen ���յ�����Ϣ���ݳ���															*/
	/* nType2 ����2																		*/
	/************************************************************************************/
	void (*OnConnectionFrame)(stConnectionInfo * pInfo,	bool bVideo,bool bKeyFrame,unsigned char * pData,unsigned int nLen,int nType2);		//����֡�ص�
	/************************************************************************************/
	/* ���ӹرջص�����																	*/
	/* ������																			*/
	/* pInfo �豸��Ϣ�ṹָ�룬ΪConnectionInit�����������Ϣ								*/
	/************************************************************************************/
	void (*OnConnectionClose)(stConnectionInfo * pInfo);								//���ӹرջص�
	/************************************************************************************/
	/* ���Ӷ�ʱ���ص�����																	*/
	/* ������																			*/
	/* pInfo �豸��Ϣ�ṹָ�룬ΪConnectionInit�����������Ϣ								*/
	/************************************************************************************/
	void (*OnConnectionTimer)(stConnectionInfo * pInfo);												//���Ӷ�ʱ���ص���50ms

}SDKPACKED;
//////////////////////////////////////////////////////////////////////////
//������Ϣ�ṹ����¼�����豸��Ϣ
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
	bool blPromoter;						//�Ƿ����ӷ�����
	char szName[MAX_ID_LENGTH];				//Ŀ���豸����
	bool bIsSync;							//ͬ����ʽ��trueͬ����ʽ��false�첽��ʽ

 	long nContext;									//�û�����������ģ��ڻص������з��ظ��û������磬����Ϊ����ָ��
	long nDeviceId;
	long nConnectionId;								//����ID���ɹ����ڻص��и�ֵ

	char szMode[MAX_MODE_STR_LENGTH];			//����ģʽ�ַ���
	char szType[MAX_TYPE_STR_LENGTH];			//���������ַ���

	char szTargetServerIp[MAX_IP_ADDR_LENGTH];	//Ŀ�������IP
	unsigned short nTargetServerPort;			//Ŀ��������˿�

	char szRemoteIp[MAX_IP_ADDR_LENGTH];	//Զ��IP��ַ�����ӽ����ɹ���Ϊ��Ч��ip��ַ
	unsigned short nRemotePort;			//Զ�˶˿ڣ����ӽ����ɹ���Ϊ��Ч��ip�˿�

	long nDeviceContext;							//DeviceInitʱ�������ݽṹstDeviceInfo�������nContextֵ
}SDKPACKED;
//////////////////////////////////////////////////////////////////////////
//�����������ݽṹ���������ӵ�������Ϣ
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

	unsigned int nConstRate;					//�㶨����
	unsigned int nMaxRate;						//�������
	unsigned int nMinRate;						//��С����
	unsigned int nBufferSize;					//�����С
	unsigned int nMaxLostRate;					//��󶪰���
}SDKPACKED;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//��Ϣ�����ַ���
static char * SDK_MESSAGE_TYPE_STR[]=
{
	"SDK_MT_DEVICE_MSG"             //
};


#ifdef WIN32
#pragma pack(pop)
#endif



/* �����ӿڿ�ʼ */

//////////////////////////////////////////////////////////////////////////
//��ʼ���豸
//
//������
//pDeviceInfo �豸��Ϣ�ṹָ�룬����Ϊ��
//
//����ֵ��
//	SDK_RV_SUCCEEDED                    = 0,		//�ɹ�
//	SDK_RV_UNKOWN_TYPE					= -1,		//���ʹ���
//	SDK_RV_NULL_POINTER					= -4,		//��ָ��
//	SDK_RV_UDX_CREATE_FAILED			= -9,		//UDX����ʧ��
//	SDK_RV_UDX_CONNECT_FAILED			= -12,		//UDX����ʧ��
//
//�ڻص�OnInit��ȷ���豸�Ƿ������Ƿ��ڷ�����ע��ɹ�
//
DEVICE_EXPORT int DeviceInit
	(
	stDeviceInfo * pDeviceInfo
	);
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//�رձ����豸��ͬʱ���ر������������豸����
//
//������
//nDeviceId Ϊ�豸id��ΪOnInit�������ص��豸id
//
//����ֵ��
//��
//
DEVICE_EXPORT void DeviceClose
	(
	long nDeviceId
	);
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//���ӳ�ʼ�������ڱ����豸�������豸��������
//
//������
//nDeviceId Ϊ�豸id��ΪOnInit�������ص��豸id
//pConnectionInfo  ������Ϣָ�룬����Ϊ��
//
//����ֵ��
//
//	SDK_RV_SUCCEEDED                    = 0,		//�ɹ�
//	SDK_RV_UNKOWN_TYPE					= -1,		//���ʹ���
//	SDK_RV_UNKOWN_MODE					= -2,		//ģʽ����
//	SDK_RV_NAME_ERROR					= -3,		//���ƴ���
//	SDK_RV_NULL_POINTER					= -4,		//��ָ��
//	SDK_RV_STRING_OVERLENGTH			= -6,		//�ַ���������󳤶�
//	SDK_RV_DEVICE_NOTEXIST				= -7,		//�豸������
//	SDK_RV_SVR_NOTEXIST					= -13,		//���������Ӳ�����
//	SDK_RV_SVR_NOTREGISTERED			= -15		//δ�ڷ�����ע��
//
//�ڻص�OnConnectionInit��ȷ�������Ƿ���
//
DEVICE_EXPORT int ConnectionInit
	(
	long nDeviceId,
	stConnectionInfo * pConnectionInfo
	);
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//�������ã���δ���ù���ʹ��Ĭ��ֵ
//
//������
//nConnectionId ����ID��ΪOnConnectionInit�������ص�ֵ
//pCfg Ϊ�����������ݽṹָ�룬����Ϊ��
//
//����ֵ����
//
DEVICE_EXPORT void ConnectionSetConfig
	(
	long nConnectionId,
	stConnectionConfig * pCfg
	);
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//֡ģʽ���ã�Ĭ��֡ģʽΪ�ر�
//
//������
//nConnectionId ����ID��ΪOnConnectionInit�������ص�ֵ
//bEnable �Ƿ��֡ģʽ��true�򿪣�false�ر�
//
//����ֵ��
//	SDK_RV_SUCCEEDED                    = 0,		//�ɹ�
//	SDK_RV_UNKOWN_TYPE					= -1,		//���ʹ���
//	SDK_RV_CONNECTION_NOTEXIST			= -8,		//���Ӳ�����
//
DEVICE_EXPORT int ConnectionEnableFrameMode
	(
	long nConnectionId,
	bool bEnable
	);
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//�ر�����
//
//������
//nDeviceId Ϊ�豸id��ΪOnInit�������ص��豸id
//nConnectionId	����id��ΪOnInit�������ص�ֵ
//
//����ֵ��
//��
//
DEVICE_EXPORT void ConnectionClose
	(
	long nDeviceId,
	long nConnectionId
	);
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
//������Ϣ������ͨ��������ת��������Ϣ��Ŀ���豸
//
//������
//nDeviceId Ϊ�豸id��ΪOnInit�������ص��豸id
//pName         Ŀ���豸���ƣ�����Ϊ�ա����Ϊ�ַ������������ʾ���͸�������������Ϊ�����Ŀ���豸��
//pType			��Ϣ�����ַ�����ֻ��ʹ��SDK_MESSAGE_TYPE_STR��ָ��������
//pData         ��Ϣ���ݵ�ַ
//nLen          ��Ϣ���ݳ���
//
//����ֵ��
//����0 ���ͳɹ����ѷ��ͳ���
//	SDK_RV_BUFFERING                    = 0,		//�ɹ��������У�ֻ��Է�������
//	SDK_RV_UNKOWN_TYPE					= -1,		//���ʹ���
//	SDK_RV_NULL_POINTER					= -4,		//��ָ��
//	SDK_RV_ERROR_LENGTH					= -5,		//���ȴ���Ϊ0���򳬳����򲻹�ָ������
//	SDK_RV_STRING_OVERLENGTH			= -6,		//�ַ���������󳤶�
//	SDK_RV_DEVICE_NOTEXIST				= -7,		//�豸������
//	SDK_RV_UDX_NOTEXIST					= -10,		//UDX���Ӳ�����
//	SDK_RV_UDX_DISCONNECT				= -11,		//UDX���ӶϿ�
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
//��������
//
//������
//nConnectionId	����id��ΪOnConnectionInit�������ص�ֵ
//pData          ���ݵ�ַ
//nLen           ���ݳ���
//
//����ֵ��
//����0 ���ͳɹ����ѷ��ͳ���
//	SDK_RV_BUFFERING                    = 0,		//�ɹ��������У�ֻ��Է�������
//	SDK_RV_UNKOWN_TYPE					= -1,		//���ʹ���
//	SDK_RV_NULL_POINTER					= -4,		//��ָ��
//	SDK_RV_ERROR_LENGTH					= -5,		//���ȴ���Ϊ0���򳬳����򲻹�ָ������
//	SDK_RV_CONNECTION_NOTEXIST			= -8,		//���Ӳ�����
//	SDK_RV_UDX_NOTEXIST					= -10,		//UDX���Ӳ�����
//	SDK_RV_UDX_DISCONNECT				= -11,		//UDX���ӶϿ�
//  SDK_RV_SEND_MODE_ERROR				= -16		//����ģʽ����
//
DEVICE_EXPORT int ConnectionSendData
	(
	long nConnectionId,
	unsigned char *pData,
	unsigned int nLen
	);
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
//ͨ�����ӷ�������Ƶ֡���ݣ���������ΪP2P��TRANSFER
//
//������
//nConnectionId	����id��ΪOnConnectionInit�������ص�ֵ
//bVideo �Ƿ���Ƶ
//bKeyFrame �Ƿ�ؼ�֡
//pData ����ָ��
//nLen ���ݳ���
//nType2 ����2
//
//����ֵ��
//	SDK_RV_SUCCEEDED                    = 0,		//�ɹ�
// 	SDK_RV_UNKOWN_TYPE					= -1,		//���ʹ���
// 	SDK_RV_NULL_POINTER					= -4,		//��ָ��
// 	SDK_RV_ERROR_LENGTH					= -5,		//���ȴ���Ϊ0���򳬳����򲻹�ָ������
// 	SDK_RV_CONNECTION_NOTEXIST			= -8,		//���Ӳ�����
// 	SDK_RV_UDX_NOTEXIST					= -10,		//UDX���Ӳ�����
// 	SDK_RV_UDX_DISCONNECT				= -11,		//UDX���ӶϿ�
//  SDK_RV_SEND_MODE_ERROR				= -16		//����ģʽ����

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
//��ȡ�ڷ�������ע����û��б�
//
//������
//nDeviceId Ϊ�豸id��ΪOnInit�������ص��豸id
//
//����ֵ��
//	SDK_RV_SUCCEEDED                    = 0,		//�ɹ�
//	SDK_RV_NULL_POINTER					= -4,		//��ָ��
//	SDK_RV_CONNECTION_NOTEXIST			= -8,		//���Ӳ�����
//	SDK_RV_UDX_NOTEXIST					= -10,		//UDX���Ӳ�����
//	SDK_RV_UDX_DISCONNECT				= -11,		//UDX���ӶϿ�
//	SDK_RV_SVR_NOTEXIST					= -13,		//���������Ӳ�����
//	SDK_RV_SVR_NOTREGISTERED			= -15		//δ�ڷ�����ע��
//
DEVICE_EXPORT int DeviceGetUserList
	(
	long nDeviceId 
	);
//////////////////////////////////////////////////////////////////////////



#endif