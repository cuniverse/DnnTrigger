
// trg_mfcDlg.h : 헤더 파일
//

#pragma once
#include "afxcmn.h"
#include "resource.h"
#include "afxwin.h"

// Ctrg_mfcDlg 대화 상자
class Ctrg_mfcDlg : public CDialogEx
{
// 생성입니다.
public:
	Ctrg_mfcDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_TRG_MFC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	CFont bigFont;

	// 생성된 메시지 맵 함수
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
	// 마이크 입력 크기 표시
	CProgressCtrl mCtrProgIn;
	// 키워드 입력란
	CString mEditKeyword;
	// 로그창
	CListBox m_ctrShowStat;
	// 검출결과표시창
	CStatic mCtlShowResult;
	int mIntWordType;
	virtual void OnOK();
};
