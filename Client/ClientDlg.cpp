
// ClientDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Client.h"
#include "ClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_UPDATA (WM_USER+0x10)  //�Զ�����Ϣ
#define MAXSIZE 1024	          //��������С

// CClientDlg �Ի���

UINT ThreadRecv(LPVOID lparam)
{
	char buf[MAXSIZE] = { 0 };
	CClientDlg* pClient = (CClientDlg*)lparam;
	while (1)
	{
		if (pClient->m_bConnection == FALSE)  //�رշ��������˳��̺߳���
			return 1;
		memset(buf, 0, MAXSIZE);
		int ret = recv(pClient->m_clientSocket, buf, MAXSIZE, 0);
		if (ret == SOCKET_ERROR)
		{
			Sleep(500);
			continue;
		}
		if (strlen(buf) != 0)
		{
			
			if (buf[0] == '@')   //���Ǻ������߱�־
			{
				char name[16] = { 0 };
				sprintf(name, "%s", &buf[1]);
				pClient->SetDlgItemText(IDC_STATIC_F_STATIC, "����");
			}
			else if (buf[0] == '$')    //���Ǻ������߱�־
			{
				char name[16] = { 0 };
				sprintf(name, "%s", &buf[1]);
				pClient->SetDlgItemText(IDC_STATIC_F_STATIC, "����");
			}
			else
			{
				char b[MAXSIZE] = { 0 };
				sprintf(b, "\n\r%s\n\r", buf);
				pClient->m_showMsg.Append(b);
				pClient->SendMessage(WM_UPDATA, 0); //UpdateData(FALSE);
			}
		}
		else
			Sleep(100);
	}
	return 0;
}

UINT ThreadSend(LPVOID lparam)
{
	int ret = 0;
	CClientDlg* pClient = (CClientDlg*)lparam;
	while (1)
	{
		if (pClient->m_bConnection == FALSE)  //�رշ��������˳��̺߳���
			return 1;
		if (pClient->m_bChangeChat)      //���͸ı����������Ϣ��������
		{
			pClient->SendMessage(WM_UPDATA, 1);//UpdateData(TRUE);
			char b[17] = { 0 };
			sprintf(b, "#%s", pClient->m_chatName);
			ret = send(pClient->m_clientSocket, b, strlen(b) + 1, 0);
			if (ret == SOCKET_ERROR)
			{
				CString error;
				error.Format("send failed with error code: %d", WSAGetLastError());
				AfxMessageBox(error);
				return 1;
			}
			pClient->m_bChangeChat = FALSE;   
			continue;
		}
		
		if (pClient->m_bSend == TRUE)  //������Ϣ
		{
			pClient->SendMessage(WM_UPDATA, 1);//UpdateData(TRUE);
			pClient->m_showMsg.Append(pClient->m_userName + "  " + pClient->GetTime() + "\r\n" + pClient->m_inputMsg + "\r\n");
			pClient->SendMessage(WM_UPDATA, 0);//UpdateData(FALSE);
			ret = send(pClient->m_clientSocket, pClient->m_inputMsg, strlen(pClient->m_inputMsg) + 1, 0); //���ﲻ����sizeof,���������ĵ�ʱ������ֽڴ�С�������
			if (ret == SOCKET_ERROR)
			{
				CString error;
				error.Format("send failed with error code: %d", WSAGetLastError());
				AfxMessageBox(error);
				return 1;
			}
			pClient->m_inputMsg.Empty();  //������Ϣ��Ҫ��ձ༭��
			pClient->SendMessage(WM_UPDATA, 0);//UpdateData(FALSE);
			pClient->m_bSend = FALSE;
		}
		Sleep(100);  //�����ĺ��б�Ҫ����������ߵĻ� 1���Ӿͻ�ѭ���ܶ�ܶ�Σ�����CPUʹ���ʱ���~��
	}
	return 0;
}


CClientDlg::CClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CLIENT_DIALOG, pParent)
	, m_serverIP(_T("127.0.0.1"))
	, m_userName(_T(""))
	, m_chatName(_T(""))
	, m_inputMsg(_T(""))
	, m_showMsg(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_clientSocket = INVALID_SOCKET;
	m_bChangeChat = FALSE;
	m_bConnection = FALSE;
	m_bSend = FALSE;
}

void CClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_IP, m_serverIP);
	DDX_Text(pDX, IDC_EDIT_USERNAEM, m_userName);
	DDX_Text(pDX, IDC_EDIT_ChatName, m_chatName);
	DDX_Text(pDX, IDC_EDIT_SHOW_MSG, m_showMsg);
	DDX_Text(pDX, IDC_EDIT_INPUT_MSG, m_inputMsg);
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_CONNECT_SERVER, &CClientDlg::OnBnClickedButtonConnectServer)
	ON_BN_CLICKED(IDC_BUTTON_CHAGE_CHAT, &CClientDlg::OnBnClickedButtonChageChat)
	ON_MESSAGE(WM_UPDATA,&CClientDlg::OnUpdata)
END_MESSAGE_MAP()


// CClientDlg ��Ϣ�������

BOOL CClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CClientDlg::OnPaint()
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
HCURSOR CClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//���ӷ�����
BOOL  CClientDlg::ConnectServer()
{
	WSADATA wsaData = { 0 };//����׽�����Ϣ
	SOCKADDR_IN ServerAddr = { 0 };//����˵�ַ
	USHORT uPort = 18000;//����˶˿�
	CString error;
	//��ʼ���׽���
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		error.Format("WSAStartup failed with error code: %d", WSAGetLastError());
		AfxMessageBox(error);
		return FALSE;
	}
	//�ж��׽��ְ汾
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		AfxMessageBox("wVersion was not 2.2");
		return FALSE;
	}
	//�����׽���
	m_clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_clientSocket == INVALID_SOCKET)
	{
		error.Format("socket failed with error code: %d", WSAGetLastError());
		AfxMessageBox(error);
		return FALSE;
	}
	UpdateData(TRUE);
	//���÷�������ַ
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(uPort);						//�������˿�
	ServerAddr.sin_addr.S_un.S_addr = inet_addr(m_serverIP);//��������ַ

	SetDlgItemText(IDC_STATIC_S_STATIC, "connecting......");//��ʾ����״̬
	//���ӷ�����
	if (SOCKET_ERROR == connect(m_clientSocket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr)))
	{
		error.Format("connect failed with error code: %d", WSAGetLastError());
		AfxMessageBox(error);
		closesocket(m_clientSocket);
		WSACleanup();
		return FALSE;
	}

	SetDlgItemText(IDC_STATIC_S_STATIC, "successfully");//��ʾ����״̬
	int ret = 0;
	ret = send(m_clientSocket, m_userName, strlen(m_userName) + 1, 0);
	if (ret == SOCKET_ERROR)
	{
		error.Format("send failed with error code: %d", WSAGetLastError());
		AfxMessageBox(error);
		closesocket(m_clientSocket);
		WSACleanup();
		return FALSE;
	}
	ret = send(m_clientSocket, m_chatName, strlen(m_chatName) + 1, 0);
	if (ret == SOCKET_ERROR)
	{
		error.Format("send failed with error code: %d", WSAGetLastError());
		AfxMessageBox(error);
		closesocket(m_clientSocket);
		WSACleanup();
		return FALSE;
	}

	return TRUE;
}

//�رշ�����
void CClientDlg::CloseServer()
{
	closesocket(m_clientSocket);
	WSACleanup();
}

//���ӷ�����
void CClientDlg::OnBnClickedButtonConnectServer()
{
	if (m_bConnection == FALSE)
	{
		if (ConnectServer())  //���ӷ�����
		{
			SetDlgItemText(IDC_BUTTON_CONNECT_SERVER, "Disconnect");
			m_bConnection = TRUE;             //����Ҫע�⣬����m_bConnectionΪTRUE���ܿ����̣߳���Ȼһ�����ͻ�ر���
			AfxBeginThread(ThreadRecv, this); //�������պͷ�����Ϣ�߳�
			AfxBeginThread(ThreadSend, this);
		}
		else
			SetDlgItemText(IDC_STATIC_S_STATIC, "Connect failed");
		
	}
	else
	{
		CloseServer();   //�رշ�����
		SetDlgItemText(IDC_BUTTON_CONNECT_SERVER, "Connect Server");
		SetDlgItemText(IDC_STATIC_S_STATIC, "");
		m_bConnection = FALSE;
	}
		
}

//�ı��������
void CClientDlg::OnBnClickedButtonChageChat()
{
	if (m_bChangeChat == FALSE)
		m_bChangeChat = TRUE;
}

//��ϢԤ�������ػس���Ϣ
BOOL CClientDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN)
		{
			if(pMsg->hwnd == GetDlgItem(IDC_EDIT_INPUT_MSG)->GetSafeHwnd())  //�ж��ǲ��������Ļس���Ϣ
				m_bSend = TRUE;//������س��Ϳ��Է�����Ϣ��
			if (pMsg->hwnd == GetDlgItem(IDC_EDIT_ChatName)->GetSafeHwnd())  //���ı���ChatName�س��Ϳ��Ի���
			{
				OnBnClickedButtonChageChat();
				::SetFocus(GetDlgItem(IDC_EDIT_INPUT_MSG)->GetSafeHwnd());   //���㶨λ�������
			}
				
			return 0; //���һ��Ҫ���ϣ���Ȼ��½��ͻ�����
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

//�Զ�����Ϣ
LRESULT CClientDlg::OnUpdata(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0)
	{
		UpdateData(FALSE);
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SHOW_MSG);
		pEdit->SendMessage(WM_VSCROLL, SB_BOTTOM);			//������Ϣ���ؼ�  ���������ײ�
	}
	else
		UpdateData(TRUE);
	return 0;
}

//��ȡ���ͺͽ�����Ϣ��ʱ��
CString CClientDlg::GetTime()
{
	SYSTEMTIME st;
	GetLocalTime(&st);   //��ȡ����ʱ��
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
	CString str;
	str.Format("%s:%s:%s",h,m,s);
	return str;
}
