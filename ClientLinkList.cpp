#include "ClientLinkList.h"

pClient head = (pClient)malloc(sizeof(_Client)); //����һ��ͷ���

/*
* function  ��ʼ������
* return    �޷���ֵ
*/
void Init()
{
	head->next = NULL;
}

/*
* function  ��ȡͷ�ڵ�
* return    ����ͷ�ڵ�
*/
pClient GetHeadNode()
{
	return head;
}

/*
* function  ��ʼ���ͻ�����Ϣ
* param		�ͻ���ָ��
* return    �޷���ֵ
*/

void InitClient(pClient pclient)
{
	memset(pclient->buf, 0, sizeof(pclient->buf));
	memset(pclient->userName, 0, sizeof(pclient->userName));
	memset(pclient->ChatName, 0, sizeof(pclient->ChatName));
	memset(pclient->IP, 0, sizeof(pclient->IP));
	pclient->sClient = INVALID_SOCKET;
}

/*
* function	���һ���ͻ���
* param     client��ʾһ���ͻ��˶���
* return    �޷���ֵ
*/
void AddClient(pClient client)
{
	client->next = head->next;  //���磺head->1->2,Ȼ�����һ��3��������
	head->next = client;        //3->1->2,head->3->1->2
}

/*
* function	ɾ��һ���ͻ���
* param     flag��ʶһ���ͻ��˶���
* return    ����true��ʾɾ���ɹ���false��ʾʧ��
*/
bool RemoveClient(UINT_PTR flag)
{
	//��ͷ������һ�����Ƚ�
	pClient pCur = head->next;//pCurָ���һ�����
	pClient pPre = head;      //pPreָ��head 
	while (pCur)
	{
		// head->1->2->3->4,Ҫɾ��2����ֱ����1->3
		if (pCur->flag == flag)
		{
			pPre->next = pCur->next;
			closesocket(pCur->sClient);  //�ر��׽���
			free(pCur);   //�ͷŸý��
			return true;
		}
		pPre = pCur;
		pCur = pCur->next;
	}
	return false;
}

/*
* function  ����ָ���ͻ���
* param     name��ָ���ͻ��˵��û���
* return    ����socket��ʾ���ҳɹ�������INVALID_SOCKET��ʾ�޴��û�
*/
SOCKET FindClient(char* name)
{
	//��ͷ������һ�����Ƚ�
	pClient pCur = head;
	while (pCur = pCur->next)
	{
		if (strcmp(pCur->userName, name) == 0)
			return pCur->sClient;
	}
	return INVALID_SOCKET;
}

/*
* function  ����SOCKET����ָ���ͻ���
* param     client��ָ���ͻ��˵��׽���
* return    ����һ��pClient��ʾ���ҳɹ�������NULL��ʾ�޴��û�
*/
pClient FindClient(SOCKET client)
{
	//��ͷ������һ�����Ƚ�
	pClient pCur = head;
	while (pCur = pCur->next)
	{
		if (pCur->sClient == client)
			return pCur;
	}
	return NULL;
}

/*
* function  ����ͻ���������
* param     client��ʾһ���ͻ��˶���
* return    ����������
*/
int CountCon()
{
	int iCount = 0;
	pClient pCur = head;
	while (pCur = pCur->next)
		iCount++;
	return iCount;
}

/*
* function  �������
* return    �޷���ֵ
*/
void ClearClient()
{
	pClient pCur = head->next;
	pClient pPre = head;
	while (pCur)
	{
		//head->1->2->3->4,��ɾ��1��head->2,Ȼ��free 1
		pClient p = pCur;
		pPre->next = p->next;
		free(p);
		pCur = pPre->next;
	}
}

/*
* function �������״̬���ر�һ������
* return ����ֵ
*/
void CheckConnection()
{
	pClient pclient = GetHeadNode();
	while (pclient = pclient->next)
	{
		if (send(pclient->sClient, "", sizeof(""), 0) == SOCKET_ERROR)
		{
			if (pclient->sClient != 0)
			{
				printf("Disconnect from IP: %s,UserName: %s\n", pclient->IP, pclient->userName);
				char error[128] = { 0 };   //����������Ϣ������Ϣ����
				sprintf(error, "The %s was downline.\n", pclient->userName);
				send(FindClient(pclient->ChatName), error, sizeof(error), 0);
				closesocket(pclient->sClient);   //����򵥵��жϣ���������Ϣʧ�ܣ�����Ϊ�����ж�(��ԭ���ж���)���رո��׽���
				RemoveClient(pclient->flag);
				break;
			}
		}
	}
}


/*
* function  ��ȡ����ʱ��
* param		һ��������
* return	��
*/
void GetTime(char* time)
{
	SYSTEMTIME st = { 0 };
	GetLocalTime(&st);
	char h[3] = { 0 };
	char m[3] = { 0 };
	char s[3] = { 0 };
	if (st.wHour < 10)
		sprintf(h, "0%d", st.wHour);
	else
		sprintf(h, "%d", st.wHour);
	if (st.wMinute < 10)
		sprintf(m, "0%d", st.wMinute);
	else
		sprintf(m, "%d", st.wMinute);
	if (st.wSecond < 10)
		sprintf(s, "0%d", st.wSecond);
	else
		sprintf(s, "%d", st.wSecond);
	sprintf(time, "%s:%s:%s", h, m, s);
}

/*
* function  ָ�����͸��ĸ��ͻ���
* param     FromName��������
* param     ToName,   ������
* param		data,	  ���͵���Ϣ
*/
void SendData(char* FromName, char* ToName, char* data)
{
	SOCKET client = FindClient(ToName);   //�����Ƿ��д��û�
	char error[128] = { 0 };
	int ret = 0;
	if (client != INVALID_SOCKET && strlen(data) != 0)
	{
		char buf[128] = { 0 };
		char time[13] = { 0 };
		GetTime(time);     //��ȡ����ʱ��
		sprintf(buf, "%s  %s\r\n%s", FromName,time, data);   //��ӷ�����Ϣ���û���
		ret = send(client, buf, strlen(buf) + 1, 0);
	}
	else//���ʹ�����Ϣ������Ϣ����
	{
		if(client == INVALID_SOCKET)
			sprintf(error, "The %s was downline.\n", ToName);
		else
			sprintf(error, "Send to %s message not allow empty, Please try again!\n", ToName);
		send(FindClient(FromName), error, sizeof(error), 0);
	}
	if (ret == SOCKET_ERROR)//����������Ϣ������Ϣ����
	{
		sprintf(error, "The %s was downline.\n", ToName);
		send(FindClient(FromName), error, sizeof(error), 0);
	}

}
