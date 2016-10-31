
// ClientDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"

#include <WinSock2.h>
#include <process.h>
#pragma comment(lib,"ws2_32.lib")



UINT ThreadRecv(LPVOID lparam);
UINT ThreadSend(LPVOID lparam);

// CClientDlg �Ի���
class CClientDlg : public CDialogEx
{
// ����
public:
	CClientDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

public:
	BOOL  ConnectServer();   //���ӷ�����
	void CloseServer();      //�رշ�����
	CString GetTime();		 //��ȡ���ͺͽ�����Ϣ��ʱ��
// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CString m_serverIP;		  //������IP
	CString m_userName;       //�û���
	CString m_chatName;       //���������
	CString m_showMsg;        //��Ϣ��ʾ
	CString m_inputMsg;       //������Ϣ
	SOCKET m_clientSocket;    //�ͻ����׽��� 
	BOOL m_bChangeChat;       //�Ƿ��л��������
	BOOL m_bConnection;       //�Ƿ�Ͽ��������������
	BOOL m_bSend;             //�Ƿ�Ҫ������Ϣ
	
	afx_msg void OnBnClickedButtonConnectServer();
	afx_msg void OnBnClickedButtonChageChat();
	afx_msg BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg LRESULT OnUpdata(WPARAM wParam, LPARAM lParam);
};
