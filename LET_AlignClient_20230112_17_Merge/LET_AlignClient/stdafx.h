
// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������ 
// ��� �ִ� ���� �����Դϴ�.


#ifndef _STDAFX_H_
#define _STDAFX_H_

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
#endif

#ifndef _DATA_BASE
#define _DATA_BASE
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // �Ϻ� CString �����ڴ� ��������� ����˴ϴ�.

// MFC�� ���� �κа� ���� ������ ��� �޽����� ���� ����⸦ �����մϴ�.
#define _AFX_ALL_WARNINGS

#include <afx.h>
#include <afxwin.h>         // MFC �ٽ� �� ǥ�� ���� ����Դϴ�.
#include <afxext.h>         // MFC Ȯ���Դϴ�.


#include <afxdisp.h>        // MFC �ڵ�ȭ Ŭ�����Դϴ�.



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // Internet Explorer 4 ���� ��Ʈ�ѿ� ���� MFC �����Դϴ�.
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // Windows ���� ��Ʈ�ѿ� ���� MFC �����Դϴ�.
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC�� ���� �� ��Ʈ�� ���� ����


#include <afxsock.h>            // MFC ���� Ȯ��







#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


#include <io.h>
#include <Windows.h>
#include <WinUser.h>
#include <gdiplus.h>
#include <string>
#include <time.h>
#include <opencv.hpp>

enum LOG_TYPE
{
	LOG_PROCESS = 0,	// �ʱ�ȭ ���� �������� ���� ��Ȳ ���
	LOG_OPERATION,		// UI Button ���� ���� ���
	LOG_ERROR,			// Error �߻� ���� ���
	LOG_INSPECTOR,		// �˻�� ���� Data ���
	LOG_PLC_SEQ,		// PLC Sequence ���
	LOG_PLC_IF,			// PLC I/F ���
	LOG_ETC,			// ��Ÿ
	LOG_CONFIG,			// �ֺ� ��ġ ����, ���� �� ��ü ���� ���� ���� ���
	LOG_CALIBRATION,
	LOG_LOGIN,			//KJH 2022-07-02 LogIn���� �α��߰�
	LOG_INSPPARA,		//kmb 221105 Z Gap Offset ���� �α� �߰�
};

const int SIZE_PLC_WORD = 16;
const int SIZE_PLC_LIVE_CHECK_MEMORY = 4;

#ifndef _WAIT_FOR_MULTI_FLAG_
#define _WAIT_FOR_MULTI_FLAG_
DWORD WaitForMultiFlag(BOOL *pWaitFlag, float waitTime, int nFlagCount = 1);
#endif

// stdafx.h ���Ͽ� �Ʒ��� �ڵ带 �߰��Ѵ�.
extern int TW_SendDataMessage(HWND parm_wnd, HWND parm_this_wnd, int parm_message_type, char* parm_data, int parm_size);

int Delay(int ms);
std::wstring towstring(const char* src);
void ShowImage(CString strTitle, const cv::Mat& src);
void HideImage(CString strTitle);
int LastIndexOf(const CString& s1, const CString& s2);

using namespace Gdiplus;

#include "Log.h"
#include "use_opencv.h"
#include "UserDefineHeader.h"
#include "UserDefinePlcAddress.h"
#include <afxcontrolbars.h>
#endif

