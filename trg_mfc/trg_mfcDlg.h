
// trg_mfcDlg.h : ��� ����
//

#pragma once
#include "afxcmn.h"
#include "resource.h"
#include "afxwin.h"

// Ctrg_mfcDlg ��ȭ ����
class Ctrg_mfcDlg : public CDialogEx
{
// �����Դϴ�.
public:
	Ctrg_mfcDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_TRG_MFC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.


// �����Դϴ�.
protected:
	HICON m_hIcon;

	CFont bigFont;

	// ������ �޽��� �� �Լ�
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()


public:
	bool recordingFlag;
	HANDLE recordThread;

	void InterfaceEnable(bool enable);
	void ShowStat(const char strStat[], ...);
	void ShowTrigger(const char strStat[], ...);

	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnMessageRadioRange(UINT value);
	// ����ũ �Է� ũ�� ǥ��
	CProgressCtrl mCtrProgIn;
	// Ű���� �Է¶�
	CString mEditKeyword;
	// �α�â
	CListBox m_ctrShowStat;
	// ������ǥ��â
	CStatic mCtlShowResult;
	int mIntWordType;
	virtual void OnOK();
};
