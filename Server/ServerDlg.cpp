
// ServerDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "ServerDlg.h"
#include "afxdialogex.h"
#include <WinSock2.h>
#include <process.h>
#include "Client.h"
#pragma comment(lib,"ws2_32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


SOCKET g_ServerSocket = INVALID_SOCKET;		 //������׽���
SOCKADDR_IN g_ClientAddr = { 0 };			 //�ͻ��˵�ַ
int g_iClientAddrLen = sizeof(g_ClientAddr);
bool g_bStartRecv = FALSE;
bool g_bStartServer = FALSE;				 //�������Ƿ�����

// CServerDlg �Ի���

//���������߳�
UINT ThreadSend(LPVOID lparam)
{
	pSend psend = (pSend)lparam;  //ת��ΪSend����
	SendData(psend->FromName, psend->ToName, psend->data); //��������
	return 0;
}

//��������
UINT ThreadRecv(LPVOID lparam)
{
	int ret = 0;
	while (1)
	{
		if (g_bStartRecv == false)
			return 1;
		pClient pclient = (pClient)lparam;
		if (!pclient)
			return 1;
		memset(pclient->buf, 0, MAXSIZE);
		ret = recv(pclient->sClient, pclient->buf, MAXSIZE, 0);
		if (ret == SOCKET_ERROR)
			return 1;
		if (strlen(pclient->buf) != 0)
		{
			if (pclient->buf[0] == '#' && pclient->buf[1] != '#') //#��ʾ�û�Ҫָ����һ���û���������
			{
				SOCKET socket = FindClient(&pclient->buf[1]);    //��֤һ�¿ͻ��Ƿ����
				if (socket != INVALID_SOCKET)
				{
					pClient c = (pClient)malloc(sizeof(_Client));
					c = FindClient(socket);                        //ֻҪ�ı�ChatName,������Ϣ��ʱ��ͻ��Զ�����ָ�����û���
					memset(pclient->ChatName, 0, sizeof(pclient->ChatName));
					memcpy(pclient->ChatName, c->userName, sizeof(pclient->ChatName));
				}
				else
				{
					ret = send(pclient->sClient, "The user have not online or not exits.\r\n", 64, 0);
					if (ret == SOCKET_ERROR)
					{
						CString error;
						error.Format("send failed with error code: %d", WSAGetLastError());
						AfxMessageBox(error);
						return 1;
					}
				}

				continue;
			}

			pSend psend = (pSend)malloc(sizeof(_Send));
			memset(psend->FromName, 0, sizeof(psend->FromName));
			memset(psend->ToName, 0, sizeof(psend->ToName));
			memset(psend->data, 0, MAXSIZE);
			//�ѷ����˵��û����ͽ�����Ϣ���û�����Ϣ��ֵ���ṹ�壬Ȼ������������������Ϣ������
			memcpy(psend->FromName, pclient->userName, sizeof(psend->FromName));
			memcpy(psend->ToName, pclient->ChatName, sizeof(psend->ToName));
			memcpy(psend->data, pclient->buf, MAXSIZE);
			AfxBeginThread(ThreadSend, psend);
			Sleep(200);
		}
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
		AfxBeginThread(ThreadRecv, pclient);
}

UINT ThreadAccept(LPVOID lparam)
{
	AfxBeginThread(ThreadManager, lparam);
	Init(); //��ʼ��һ����Ҫ��while������������head��һֱΪNULL������
	CServerDlg* pServer = (CServerDlg*)lparam;
	while (1)
	{
		int iCount = CountCon();  //��ȡ��������
		//����һ���µĿͻ��˶���
		pClient pclient = (pClient)malloc(sizeof(_Client));
		InitClient(pclient);  //��ʼ���ͻ���

		//����пͻ����������Ӿͽ�������
		if ((pclient->sClient = accept(g_ServerSocket, (SOCKADDR*)&g_ClientAddr, &g_iClientAddrLen)) == INVALID_SOCKET)
		{
			CString error;
			error.Format("accept failed with error code: %d", WSAGetLastError());
			closesocket(g_ServerSocket);
			WSACleanup();
			return -1;
		}
		recv(pclient->sClient, pclient->userName, sizeof(pclient->userName), 0); //�����û�����ָ�����������û���
		recv(pclient->sClient, pclient->ChatName, sizeof(pclient->ChatName), 0);

		memcpy(pclient->IP, inet_ntoa(g_ClientAddr.sin_addr), sizeof(pclient->IP)); //��¼�ͻ���IP
		pclient->flag = pclient->sClient; //��ͬ��socke�в�ͬUINT_PTR���͵���������ʶ
		pclient->Port = htons(g_ClientAddr.sin_port);
		
		pClient pOldClient = ClientExits(pclient->userName);
		if (pOldClient)   //������û�֮ǰ�Ѿ���½�����򼤻�������
		{
			pOldClient->bStatus = TRUE;
			pOldClient->sClient = pclient->sClient;
			pOldClient->Port = pclient->Port;
			pOldClient->flag = pclient->flag;
			memcpy(pOldClient->ChatName, pclient->ChatName, sizeof(pOldClient->ChatName));
			memcpy(pOldClient->IP, pclient->IP, sizeof(pOldClient->IP));
			LVFINDINFO lvFindInfo = { 0 };  //һ�����޸�
			lvFindInfo.flags = LVFI_STRING;
			lvFindInfo.psz = pOldClient->userName;
			int iItem = pServer->m_list.FindItem(&lvFindInfo);  //�ҵ�Ҫ�޸ĵ���
			pServer->m_list.SetItemText(iItem, 1, pOldClient->IP);
			CString s;
			s.Format("%d", pOldClient->Port);
			pServer->m_list.SetItemText(iItem, 2, s);
			pServer->m_list.SetItemText(iItem, 3, "����");
			if (IsOnline(pOldClient->ChatName))
			{
				char name[16];
				sprintf(name, "$%s", pOldClient->userName);
				send(pOldClient->sClient, name, sizeof(name), 0);//��������״̬���û�
				send(FindClient(pOldClient->ChatName), name, sizeof(name), 0);  
			}
		}
		else
		{
			AddClient(pclient);  //���µĿͻ��˼���������
			pServer->m_list.InsertItem(iCount, pclient->userName);
			pServer->m_list.SetItemText(iCount, 1, pclient->IP);
			CString s;
			s.Format("%d", pclient->Port);
			pServer->m_list.SetItemText(iCount, 2, s);
			pServer->m_list.SetItemText(iCount, 3, "����");
			if (IsOnline(pclient->ChatName))
			{
				char name[16];
				sprintf(name, "$%s", pclient->userName);
				SOCKET chatSocket = FindClient(pclient->ChatName);
				send(pclient->sClient, name, sizeof(name), 0);//��������״̬���û�
				if(chatSocket != INVALID_SOCKET)
					send(chatSocket, name, sizeof(name), 0);
			}
		}			

		if (CountCon() >= 2)					 //�����������û��������Ϸ�������Ž�����Ϣת��                                                          
			StartRecv();

		Sleep(2000);  //��ʵ����û�б�Ҫ˯��2s����Ϊaccept������
	}
	return 0;
}

UINT ThreadManager(LPVOID lparam)
{
	CServerDlg* pServer = (CServerDlg*)lparam;
	while (1)
	{
		CheckConnection(&pServer->m_list);  //�������״��
		pServer->SetDlgItemInt(IDC_STATIC_ONLINE_NUMS, CountCon());
		Sleep(2000);		//���ʱ����Խ�� ������Ϣ��ײ�ĸ���ԽС�����ͳɹ���Խ��
	}

	return 0;
}


CServerDlg::CServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
}

BEGIN_MESSAGE_MAP(CServerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CServerDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CServerDlg ��Ϣ�������

BOOL CServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	//����list��ͷ
	m_list.InsertColumn(0, "�û���", 0, 120);
	m_list.InsertColumn(1, "IP", 0, 120);
	m_list.InsertColumn(2, "�˿�", 0, 80);
	m_list.InsertColumn(3, "״̬", 0, 80);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//����������
BOOL CServerDlg::StartServer()
{
	//����׽�����Ϣ�Ľṹ
	WSADATA wsaData = { 0 };
	SOCKADDR_IN ServerAddr = { 0 };				//����˵�ַ
	USHORT uPort = 18000;						//�����������˿�
	CString error;
	//��ʼ���׽���
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		error.Format("WSAStartup failed with error code: %d", WSAGetLastError());
		AfxMessageBox(error);
		return FALSE;
	}
	//�жϰ汾
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		AfxMessageBox("wVersion was not 2.2");
		return FALSE;
	}
	//�����׽���
	g_ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_ServerSocket == INVALID_SOCKET)
	{
		error.Format("socket failed with error code: %d", WSAGetLastError());
		AfxMessageBox(error);
		return FALSE;
	}

	//���÷�������ַ
	ServerAddr.sin_family = AF_INET;//���ӷ�ʽ
	ServerAddr.sin_port = htons(uPort);//�����������˿�
	ServerAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//�κοͻ��˶����������������

														//�󶨷�����
	if (SOCKET_ERROR == bind(g_ServerSocket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr)))
	{
		error.Format("bind failed with error code: %d", WSAGetLastError());
		AfxMessageBox(error);
		closesocket(g_ServerSocket);
		return FALSE;
	}
	//���ü����ͻ���������
	if (SOCKET_ERROR == listen(g_ServerSocket, 20000))
	{
		error.Format("listen failed with error code: %d", WSAGetLastError());
		AfxMessageBox(error);
		closesocket(g_ServerSocket);
		WSACleanup();
		return FALSE;
	}

	AfxBeginThread(ThreadAccept, this);

	return TRUE;
}

//�رշ�����
void CServerDlg::CloseServer()
{
	//�ر��׽���
	ClearClient();
	closesocket(g_ServerSocket);
	WSACleanup();
	m_list.DeleteAllItems();
	g_bStartRecv = false;
}

//����������
void CServerDlg::OnBnClickedButton1()
{
	if (g_bStartServer == FALSE)
	{
		if (StartServer())
		{
			SetDlgItemText(IDC_BUTTON1, "�ر�");
			SetDlgItemText(IDC_STATIC_SERVER_STATUS, "����");
			g_bStartServer = TRUE;
		}
	}
	else
	{
		CloseServer();
		SetDlgItemText(IDC_BUTTON1, "����");
		SetDlgItemText(IDC_STATIC_SERVER_STATUS, "�ر�");
		g_bStartServer = FALSE;
	}

}
