
// ServerDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"


UINT ThreadSend(LPVOID lparam);
UINT ThreadRecv(LPVOID lparam);
UINT ThreadAccept(LPVOID lparam);
UINT ThreadManager(LPVOID lparam);

// CServerDlg �Ի���
class CServerDlg : public CDialogEx
{
// ����
public:
	CServerDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


public:
	BOOL StartServer();//����������
	void CloseServer();//�رշ�����
// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	CListCtrl m_list;
};
