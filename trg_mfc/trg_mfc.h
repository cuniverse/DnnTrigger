
// trg_mfc.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// Ctrg_mfcApp:
// �� Ŭ������ ������ ���ؼ��� trg_mfc.cpp�� �����Ͻʽÿ�.
//

class Ctrg_mfcApp : public CWinApp
{
public:
	Ctrg_mfcApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern Ctrg_mfcApp theApp;