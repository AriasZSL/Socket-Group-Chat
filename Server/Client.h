#ifndef _CLIENT_LINK_LIST_H_
#define _CLIENT_LINK_LIST_H_

#include <WinSock2.h>

#include <stdio.h>
#define MAXSIZE 1024

//�ͻ�����Ϣ�ṹ��
typedef struct _Client
{
	SOCKET sClient;			//�ͻ����׽���
	char buf[MAXSIZE];			//���ݻ�����
	char userName[16];		//�ͻ����û���
	char IP[20];			//�ͻ���IP
	char ChatName[16];		//ָ��Ҫ���ĸ��ͻ�������
	unsigned short Port;	//�ͻ��˶˿�
	UINT_PTR flag;			//��ǿͻ��ˣ��������ֲ�ͬ�Ŀͻ���
	bool bStatus;			//����û��Ƿ�����
	_Client* next;			//ָ����һ�����
}Client, *pClient;

//������Ϣ�ṹ��
typedef struct _Send
{
	char FromName[16];
	char ToName[16];
	char data[MAXSIZE];
}Send, *pSend;

/*
* function  ��ʼ������
* return    �޷���ֵ
*/
void Init();

/*
* function  ��ȡͷ�ڵ�
* return    ����ͷ�ڵ�
*/
pClient GetHeadNode();

/*
* function  ��ʼ���ͻ�����Ϣ
* param		�ͻ���ָ��
* return    �޷���ֵ
*/

void InitClient(pClient pclient);

/*
* function	���һ���ͻ���
* param     client��ʾһ���ͻ��˶���
* return    �޷���ֵ
*/
void AddClient(pClient client);

/*
* function	ɾ��һ���ͻ���
* param     flag��ʶһ���ͻ��˶���
* return    ����true��ʾɾ���ɹ���false��ʾʧ��
*/
bool RemoveClient(UINT_PTR flag);

/*
* function  ����name����ָ���ͻ���
* param     name��ָ���ͻ��˵��û���
* return    ����һ��client��ʾ���ҳɹ�������INVALID_SOCKET��ʾ�޴��û�
*/
SOCKET FindClient(char* name);

/*
* function  ����SOCKET����ָ���ͻ���
* param     client��ָ���ͻ��˵��׽���
* return    ����һ��pClient��ʾ���ҳɹ�������NULL��ʾ�޴��û�
*/
pClient FindClient(SOCKET client);

/*
* function  ���Ҵ��û��Ƿ���ڣ�ֻҪ��½��������������ڣ�
* param	    �����û�������
* return	����һ��pClient��ʾ���ҳɹ�������NULL��ʾ�޴��û�
*/
pClient ClientExits(char* name);

/*
* function  �����û��Ƿ�����
* param		�����û�������
* return    ����TRUE��ʾ���ߣ�����ʧ��
*/
BOOL IsOnline(char* name);

/*
* function  ����ͻ���������
* param     client��ʾһ���ͻ��˶���
* return    ����������
*/
int CountCon();

/*
* function  �������
* return    �޷���ֵ
*/
void ClearClient();

/*
* function  �������״̬���ر�һ������
* return ����ֵ
*/
void CheckConnection(CListCtrl* pList);

/*
* function  ָ�����͸��ĸ��ͻ���
* param     FromName��������
* param     ToName,   ������
* param		data,	  ���͵���Ϣ
*/
void SendData(char* FromName, char* ToName, char* data);

/*
* function  ��ȡ����ʱ��
* param		һ��������
* return	��
*/
void GetTime(char* time);

#endif //_CLIENT_LINK_LIST_H_
