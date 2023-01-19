
// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이 
// 들어 있는 포함 파일입니다.


#ifndef _STDAFX_H_
#define _STDAFX_H_

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#endif

#ifndef _DATA_BASE
#define _DATA_BASE
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 일부 CString 생성자는 명시적으로 선언됩니다.

// MFC의 공통 부분과 무시 가능한 경고 메시지에 대한 숨기기를 해제합니다.
#define _AFX_ALL_WARNINGS

#include <afx.h>
#include <afxwin.h>         // MFC 핵심 및 표준 구성 요소입니다.
#include <afxext.h>         // MFC 확장입니다.


#include <afxdisp.h>        // MFC 자동화 클래스입니다.



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // Internet Explorer 4 공용 컨트롤에 대한 MFC 지원입니다.
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // Windows 공용 컨트롤에 대한 MFC 지원입니다.
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC의 리본 및 컨트롤 막대 지원


#include <afxsock.h>            // MFC 소켓 확장







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
	LOG_PROCESS = 0,	// 초기화 이후 전반적인 진행 상황 기록
	LOG_OPERATION,		// UI Button 조작 관련 기록
	LOG_ERROR,			// Error 발생 관련 기록
	LOG_INSPECTOR,		// 검사기 관련 Data 기록
	LOG_PLC_SEQ,		// PLC Sequence 기록
	LOG_PLC_IF,			// PLC I/F 기록
	LOG_ETC,			// 기타
	LOG_CONFIG,			// 주변 장치 연결, 해제 및 객체 생성 유무 등을 기록
	LOG_CALIBRATION,
	LOG_LOGIN,			//KJH 2022-07-02 LogIn관련 로그추가
	LOG_INSPPARA,		//kmb 221105 Z Gap Offset 변경 로그 추가
};

const int SIZE_PLC_WORD = 16;
const int SIZE_PLC_LIVE_CHECK_MEMORY = 4;

#ifndef _WAIT_FOR_MULTI_FLAG_
#define _WAIT_FOR_MULTI_FLAG_
DWORD WaitForMultiFlag(BOOL *pWaitFlag, float waitTime, int nFlagCount = 1);
#endif

// stdafx.h 파일에 아래의 코드를 추가한다.
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

