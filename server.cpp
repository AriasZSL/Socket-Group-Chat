/*
 *��������ⲩ�͵�ַ��http://blog.csdn.net/qq_18297675/article/details/52819975
 *������д��2016.10.15 13��00 -- 21��10

 *�������ʵ����һ�Զ�Ķ�������ģʽ��һ���˿���ͬʱ��N�����죬ֻ����Ϊ�õ�C���ԣ����Բ�����������û���
 *����ͷ��֮��ģ�Ҫ�ü�������ķ�ʽ����ָ��Ҫ�Ի����û���
 
 *��������1V1ʱ��BUG���£�
 *CPUʹ���ʴ���½����رգ���������CPU�����������ȶ���20����

 *����bug����Ϊû��ʱ��ר�Ų��ԣ�������δ֪����
*/


#include <WinSock2.h>
#include <process.h>
#include <stdlib.h>
#include "ClientLinkList.h"
#pragma comment(lib,"ws2_32.lib")


SOCKET g_ServerSocket = INVALID_SOCKET;		 //������׽���
SOCKADDR_IN g_ClientAddr = { 0 };			 //�ͻ��˵�ַ
int g_iClientAddrLen = sizeof(g_ClientAddr);
bool g_bStartRecv = FALSE;

typedef struct _Send
{
	char FromName[16];
	char ToName[16];
	char data[128];
}Send,*pSend;






//���������߳�
unsigned __stdcall ThreadSend(void* param)
{
	pSend psend = (pSend)param;  //ת��ΪSend����
	SendData(psend->FromName, psend->ToName, psend->data); //��������
	return 0;
}


//��������
unsigned __stdcall ThreadRecv(void* param)
{
	int ret = 0;
	while (1)
	{
		if (g_bStartRecv == false)
			return 1;
		pClient pclient = (pClient)param;
		if (!pclient)
			return 1;
		ret = recv(pclient->sClient, pclient->buf, sizeof(pclient->buf), 0);
		if (ret == SOCKET_ERROR)
			return 1;
		if (pclient->buf[0] == '#' && pclient->buf[1] != '#') //#��ʾ�û�Ҫָ����һ���û���������
		{
			SOCKET socket = FindClient(&pclient->buf[1]);    //��֤һ�¿ͻ��Ƿ����
			if (socket != INVALID_SOCKET)
			{
				pClient c = (pClient)malloc(sizeof(_Client));
				c = FindClient(socket);                        //ֻҪ�ı�ChatName,������Ϣ��ʱ��ͻ��Զ�����ָ�����û���
				memset(pclient->ChatName, 0, sizeof(pclient->ChatName));   
				memcpy(pclient->ChatName , c->userName,sizeof(pclient->ChatName));
			}
			else  
				send(pclient->sClient, "The user have not online or not exits.",64,0);
			continue;
		}
			
		pSend psend = (pSend)malloc(sizeof(_Send));
		memset(psend->FromName, 0, sizeof(psend->FromName));
		memset(psend->ToName, 0, sizeof(psend->ToName));
		memset(psend->data, 0, sizeof(psend->data));
		//�ѷ����˵��û����ͽ�����Ϣ���û�����Ϣ��ֵ���ṹ�壬Ȼ������������������Ϣ������
		memcpy(psend->FromName, pclient->userName, sizeof(psend->FromName));
		memcpy(psend->ToName, pclient->ChatName, sizeof(psend->ToName));
		memcpy(psend->data, pclient->buf, sizeof(psend->data));
		_beginthreadex(NULL, 0, ThreadSend, psend, 0, NULL);
		Sleep(200);
	}

	return 0;
}

//����������Ϣ�߳�
void StartRecv()
{
	pClient pclient = GetHeadNode();
	g_bStartRecv = false;  //�ر����н����߳�
	Sleep(1000);
	g_bStartRecv = true;   //�������н����߳�		
	while (pclient = pclient->next)
		_beginthreadex(NULL, 0, ThreadRecv, pclient, 0, NULL);	
}

//��������
unsigned __stdcall ThreadManager(void* param)
{
	while (1)
	{
		CheckConnection();  //�������״��
		Sleep(2000);		//2s���һ��
	}

	return 0;
}

//��������
unsigned __stdcall ThreadAccept(void* param)
{
	_beginthreadex(NULL, 0, ThreadManager, NULL, 0, NULL);
	Init(); //��ʼ��һ����Ҫ��while������������head��һֱΪNULL������
	while (1)
	{
		//����һ���µĿͻ��˶���
		pClient pclient = (pClient)malloc(sizeof(_Client));
		InitClient(pclient);  //��ʼ���ͻ���
							  	
		//����пͻ����������Ӿͽ�������
		if ((pclient->sClient = accept(g_ServerSocket, (SOCKADDR*)&g_ClientAddr, &g_iClientAddrLen)) == INVALID_SOCKET)
		{
			printf("accept failed with error code: %d\n", WSAGetLastError());
			closesocket(g_ServerSocket);
			WSACleanup();
			return -1;
		}
		recv(pclient->sClient, pclient->userName, sizeof(pclient->userName), 0); //�����û�����ָ�����������û���
		recv(pclient->sClient, pclient->ChatName, sizeof(pclient->ChatName), 0);

		memcpy(pclient->IP, inet_ntoa(g_ClientAddr.sin_addr), sizeof(pclient->IP)); //��¼�ͻ���IP
		pclient->flag = pclient->sClient; //��ͬ��socke�в�ͬUINT_PTR���͵���������ʶ
		pclient->Port = htons(g_ClientAddr.sin_port);
		AddClient(pclient);  //���µĿͻ��˼���������

		printf("Successfuuly got a connection from IP:%s ,Port: %d,UerName: %s , ChatName: %s\n",
			pclient->IP, pclient->Port, pclient->userName,pclient->ChatName);
		
		if (CountCon() >= 2)					 //�����������û��������Ϸ�������Ž�����Ϣת��                                                          
			StartRecv();
		
		Sleep(2000);
	}
	return 0;
}

//����������
int StartServer()
{
	//����׽�����Ϣ�Ľṹ
	WSADATA wsaData = { 0 };
	SOCKADDR_IN ServerAddr = { 0 };				//����˵�ַ
	USHORT uPort = 18000;						//�����������˿�

	//��ʼ���׽���
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		printf("WSAStartup failed with error code: %d\n", WSAGetLastError());
		return -1;
	}
	//�жϰ汾
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		printf("wVersion was not 2.2\n");
		return -1;
	}
	//�����׽���
	g_ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_ServerSocket == INVALID_SOCKET)
	{
		printf("socket failed with error code: %d\n", WSAGetLastError());
		return -1;
	}

	//���÷�������ַ
	ServerAddr.sin_family = AF_INET;//���ӷ�ʽ
	ServerAddr.sin_port = htons(uPort);//�����������˿�
	ServerAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//�κοͻ��˶����������������

	//�󶨷�����
	if (SOCKET_ERROR == bind(g_ServerSocket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr)))
	{
		printf("bind failed with error code: %d\n", WSAGetLastError());
		closesocket(g_ServerSocket);
		return -1;
	}
	//���ü����ͻ���������
	if (SOCKET_ERROR == listen(g_ServerSocket, 20000))
	{
		printf("listen failed with error code: %d\n", WSAGetLastError());
		closesocket(g_ServerSocket);
		WSACleanup();
		return -1;
	}

	_beginthreadex(NULL, 0, ThreadAccept, NULL, 0, 0);
	for (int k = 0;k < 100;k++) //�����߳����ߣ��������ر�TCP����.
		Sleep(10000000);
	
	//�ر��׽���
	ClearClient();
	closesocket(g_ServerSocket);
	WSACleanup();
	return 0;
}

int main()
{
	StartServer(); //����������
	
	return 0;
}