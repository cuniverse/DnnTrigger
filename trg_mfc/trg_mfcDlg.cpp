
// trg_mfcDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "afxdialogex.h"

#include "trg_thread.h"

#include "trg_mfc.h"
#include "trg_mfcDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Ctrg_mfcDlg ��ȭ ����



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


// Ctrg_mfcDlg �޽��� ó����

BOOL Ctrg_mfcDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �� ��ȭ ������ �������� �����մϴ�.  ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	bigFont.CreatePointFont(500, _T("���� ���"));
	mCtlShowResult.SetFont(&bigFont);

	InterfaceEnable(true);
	mCtrProgIn.SetRange(0, 32767);
	UpdateData(FALSE);

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�.  ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void Ctrg_mfcDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR Ctrg_mfcDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


// ��Ʈ�� Ȱ��ȭ, ��Ȱ��ȭ
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
		SetDlgItemText(IDC_SHOW_RESULT, _T("�����"));
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
