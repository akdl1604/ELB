
// LET_AlignClient.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.
#include "MiniDumper.h"

// CLET_AlignClientApp:
// �� Ŭ������ ������ ���ؼ��� LET_AlignClient.cpp�� �����Ͻʽÿ�.
//
class CLET_AlignClientDlg;
class CLET_AlignClientApp : public CWinApp
{
public:
	CLET_AlignClientApp();

	MinidumpHelp m_dumpHelp;
	ULONG_PTR m_gdiplusToken;
	HHOOK SWH_return;
	BOOL m_bEnforceOkFlag;

	void OnCaptureCurrentScreen(int nType); 
// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.
	CLET_AlignClientDlg *m_pFrame;

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CLET_AlignClientApp theApp;