
// trg_mfcDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "afxdialogex.h"

#include "trg_thread.h"

#include "trg_mfc.h"
#include "trg_mfcDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Ctrg_mfcDlg 대화 상자



Ctrg_mfcDlg::Ctrg_mfcDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(Ctrg_mfcDlg::IDD, pParent)
	, mEditKeyword(_T(""))
	, mIntWordType(0)
	, recordingFlag(false)
	, recordThread(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Ctrg_mfcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_IN, mCtrProgIn);
	DDX_Text(pDX, IDC_EDIT_KEYWORD, mEditKeyword);
	DDV_MaxChars(pDX, mEditKeyword, 20);
	DDX_Control(pDX, IDC_SHOW_STAT, m_ctrShowStat);
	DDX_Control(pDX, IDC_SHOW_RESULT, mCtlShowResult);
	DDX_Radio(pDX, IDC_RADIO_WORD, mIntWordType);
}

BEGIN_MESSAGE_MAP(Ctrg_mfcDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START, &Ctrg_mfcDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &Ctrg_mfcDlg::OnBnClickedButtonStop)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_WORD, IDC_RADIO_MONO, &Ctrg_mfcDlg::OnMessageRadioRange)

END_MESSAGE_MAP()


// Ctrg_mfcDlg 메시지 처리기

BOOL Ctrg_mfcDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	bigFont.CreatePointFont(500, _T("맑은 고딕"));
	mCtlShowResult.SetFont(&bigFont);

	InterfaceEnable(true);
	mCtrProgIn.SetRange(0, 32767);
	UpdateData(FALSE);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void Ctrg_mfcDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR Ctrg_mfcDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


// 컨트롤 활성화, 비활성화
void Ctrg_mfcDlg::InterfaceEnable(bool enable)
{
	GetDlgItem(IDC_EDIT_KEYWORD)->EnableWindow(enable);
	GetDlgItem(IDC_RADIO_WORD)->EnableWindow(enable);
	GetDlgItem(IDC_RADIO_MONO)->EnableWindow(enable);
	GetDlgItem(IDC_BUTTON_START)->EnableWindow(enable);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(!enable);

	if (enable)
	{
		if (0 == mIntWordType)
			GetDlgItem(IDC_EDIT_KEYWORD)->EnableWindow(false);

		mCtrProgIn.SetPos(0);
		SetDlgItemText(IDC_SHOW_RESULT, _T("대기중"));
	}
}


void Ctrg_mfcDlg::ShowTrigger(const char strStat[], ...)
{
	//char strShow[MAX_PATH];
	CString strShow;
	CString csStat(strStat);

	va_list vl;
	va_start(vl, strStat);
	//vswprintf_s(strShow, strStat, vl);
	strShow.FormatV(csStat, vl);
	va_end(vl);

	mCtlShowResult.SetWindowText(strShow);
}

void Ctrg_mfcDlg::ShowStat(const char strStat[], ...)
{
	if (strStat == NULL) m_ctrShowStat.ResetContent();

	//char strShow[MAX_PATH];
	CString strShow;
	CString csStat(strStat);

	va_list vl;
	va_start(vl, strStat);
	//vswprintf_s(strShow, strStat, vl);
	strShow.FormatV(csStat, vl);
	va_end(vl);

	m_ctrShowStat.InsertString(0, strShow);
	OutputDebugString(strShow);
	OutputDebugString(_T("\n"));
}

afx_msg void Ctrg_mfcDlg::OnMessageRadioRange(UINT value)
{
	UpdateData(TRUE);

	if (0 == mIntWordType)
		GetDlgItem(IDC_EDIT_KEYWORD)->EnableWindow(false);
	if (1 == mIntWordType)
		GetDlgItem(IDC_EDIT_KEYWORD)->EnableWindow(true);
}


void Ctrg_mfcDlg::OnBnClickedButtonStart()
{
	UpdateData(TRUE);

	if (recordThread)
	{
		return;
	}

	InterfaceEnable(false);

	recordingFlag = true;

	if (0 == mIntWordType)
		recordThread = CreateThread(NULL, 0, WordTriggerThread, this, 0, NULL);
	else
		recordThread = CreateThread(NULL, 0, MonoTriggerThread, this, 0, NULL);
}


void Ctrg_mfcDlg::OnBnClickedButtonStop()
{
	recordingFlag = false;
}


void Ctrg_mfcDlg::OnOK()
{
	// Prevent exiting on [Enter] key
	//CDialogEx::OnOK();
}
