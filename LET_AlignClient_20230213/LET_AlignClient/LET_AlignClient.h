
// LET_AlignClient.h : PROJECT_NAME 응용 프로그램에 대한 주 헤더 파일입니다.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'stdafx.h'를 포함합니다."
#endif

#include "resource.h"		// 주 기호입니다.
#include "MiniDumper.h"

// CLET_AlignClientApp:
// 이 클래스의 구현에 대해서는 LET_AlignClient.cpp을 참조하십시오.
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
// 재정의입니다.
public:
	virtual BOOL InitInstance();

// 구현입니다.
	CLET_AlignClientDlg *m_pFrame;

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CLET_AlignClientApp theApp;