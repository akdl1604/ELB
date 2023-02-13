
// LET_AlignClient.cpp : 응용 프로그램에 대한 클래스 동작을 정의합니다.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "UI_Control/Graphics/GraphicLabel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLET_AlignClientApp

BEGIN_MESSAGE_MAP(CLET_AlignClientApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CLET_AlignClientApp 생성

CLET_AlignClientApp::CLET_AlignClientApp()
{
	// 다시 시작 관리자 지원
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.	

	m_bEnforceOkFlag = FALSE;
	SWH_return = NULL;
}


// 유일한 CLET_AlignClientApp 개체입니다.

CLET_AlignClientApp theApp;
CLog theLog;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0) return CallNextHookEx(NULL, nCode, wParam, lParam);
	tagKBDLLHOOKSTRUCT* str = (tagKBDLLHOOKSTRUCT*)lParam;

	switch (str->flags)
	{
	case 48://LLKHF_ALTDOWN
		return 1;//
	case LLKHF_ALTDOWN:
		return 1;
		break;
	}

	if (wParam == WM_KEYDOWN)
	{
		switch (str->vkCode)
		{
		case VK_MENU: 
			return 1;
			break;
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}
// CLET_AlignClientApp 초기화

BOOL CLET_AlignClientApp::InitInstance()
{
	HANDLE hMutex=::CreateMutex(NULL,TRUE,m_pszExeName);
	if (GetLastError()==ERROR_ALREADY_EXISTS)
	{
		CWnd* pPreWnd = CWnd::GetDesktopWindow()->GetWindow(GW_CHILD);
		while (pPreWnd)
		{
			if (::GetProp(pPreWnd->GetSafeHwnd(),m_pszExeName))
			{
				if (pPreWnd->IsIconic())
					pPreWnd->ShowWindow(SW_RESTORE);
				pPreWnd->BringWindowToTop();
				pPreWnd->GetLastActivePopup()->BringWindowToTop();
				break;
			}
			pPreWnd = pPreWnd->GetWindow(GW_HWNDNEXT);
		}
		//AfxMessageBox(" Program Executing...");


		return FALSE;
	}
	if (hMutex) ::ReleaseMutex(hMutex);

	// 응용 프로그램 매니페스트가 ComCtl32.dll 버전 6 이상을 사용하여 비주얼 스타일을
	// 사용하도록 지정하는 경우, Windows XP 상에서 반드시 InitCommonControlsEx()가 필요합니다.
	// InitCommonControlsEx()를 사용하지 않으면 창을 만들 수 없습니다.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 응용 프로그램에서 사용할 모든 공용 컨트롤 클래스를 포함하도록
	// 이 항목을 설정하십시오.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();
	
	SWH_return = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}


	AfxEnableControlContainer();

	// 대화 상자에 셸 트리 뷰 또는
	// 셸 목록 뷰 컨트롤이 포함되어 있는 경우 셸 관리자를 만듭니다.
	CShellManager *pShellManager = new CShellManager;

	// 표준 초기화
	// 이들 기능을 사용하지 않고 최종 실행 파일의 크기를 줄이려면
	// 아래에서 필요 없는 특정 초기화
	// 루틴을 제거해야 합니다.
	// 해당 설정이 저장된 레지스트리 키를 변경하십시오.
	// TODO: 이 문자열을 회사 또는 조직의 이름과 같은
	// 적절한 내용으로 수정해야 합니다.
	SetRegistryKey(_T("로컬 응용 프로그램 마법사에서 생성된 응용 프로그램"));

	m_dumpHelp.install_self_mini_dump();

	GraphicLabel::Init();
	CLET_AlignClientDlg dlg;
	m_pMainWnd = &dlg;

	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 여기에 [확인]을 클릭하여 대화 상자가 없어질 때 처리할
		//  코드를 배치합니다.
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 여기에 [취소]를 클릭하여 대화 상자가 없어질 때 처리할
		//  코드를 배치합니다.
	}

	// 위에서 만든 셸 관리자를 삭제합니다.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// 대화 상자가 닫혔으므로 응용 프로그램의 메시지 펌프를 시작하지 않고  응용 프로그램을 끝낼 수 있도록 FALSE를
	// 반환합니다.
	return FALSE;
}
int CLET_AlignClientApp::ExitInstance()
{
	if (SWH_return) UnhookWindowsHookEx(SWH_return);

	return CWinApp::ExitInstance();
}

void CLET_AlignClientApp::OnCaptureCurrentScreen(int nType) 
{
	// 스크린 전체를 캡쳐하기 위해서 CWindowDC 형식으로 DC를 얻는다.
	// GetDC의 파라메터에 특정 윈도우 핸들을 넣지 않고 NULL을 넣으면 
	// CWindowDC(메인 DC) 형식으로 DC를 얻게 된다.
	HDC h_screen_dc = ::GetDC(NULL);
	// 현재 스크린의 해상도를 얻는다.
	int width = ::GetDeviceCaps(h_screen_dc, HORZRES);
	int height = ::GetDeviceCaps(h_screen_dc, VERTRES);

	// DIB의 형식을 정의한다. 
	BITMAPINFO dib_define;
	dib_define.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	dib_define.bmiHeader.biWidth = width;
	dib_define.bmiHeader.biHeight = height;
	dib_define.bmiHeader.biPlanes = 1;
	dib_define.bmiHeader.biBitCount = 24;
	dib_define.bmiHeader.biCompression = BI_RGB;
	dib_define.bmiHeader.biSizeImage = (((width * 24 + 31) & ~31) >> 3) * height;
	dib_define.bmiHeader.biXPelsPerMeter = 0;
	dib_define.bmiHeader.biYPelsPerMeter = 0;
	dib_define.bmiHeader.biClrImportant = 0;
	dib_define.bmiHeader.biClrUsed = 0;

	// DIB의 내부 이미지 비트 패턴을 참조할 포인터 변수
	BYTE *p_image_data = NULL;
	// dib_define에 정의된 내용으로 DIB를 생성한다.
	HBITMAP h_bitmap = ::CreateDIBSection(h_screen_dc, &dib_define, DIB_RGB_COLORS, (void **)&p_image_data, 0, 0);
	// 이미지를 추출하기 위해서 가상 DC를 생성한다. 메인 DC에서는 직접적으로 비트맵에 접근하여
	// 이미지 패턴을 얻을 수 없기 때문이다.
	HDC h_memory_dc = ::CreateCompatibleDC(h_screen_dc);
	// 가상 DC에 이미지를 추출할 비트맵을 연결한다.
	HBITMAP h_old_bitmap = (HBITMAP)::SelectObject(h_memory_dc, h_bitmap);
	// 현재 스크린 화면을 캡쳐한다.
	::BitBlt(h_memory_dc, 0, 0, width, height, h_screen_dc, 0, 0, SRCCOPY);
	// 본래의 비트맵으로 복구한다.
	::SelectObject(h_memory_dc, h_old_bitmap); 
	// 가상 DC를 제거한다.
	DeleteDC(h_memory_dc);

	// DIB 파일의 내용을 구성한다.
	BITMAPFILEHEADER dib_format_layout;
	ZeroMemory(&dib_format_layout, sizeof(BITMAPFILEHEADER));
	dib_format_layout.bfType = *(WORD*)"BM";
	dib_format_layout.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dib_define.bmiHeader.biSizeImage;
	dib_format_layout.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	// DIB 파일을 생성한다.
	CTime NowTime = CTime::GetCurrentTime();
	CString strFilePath, strDate;

	FILE *p_file = fopen( strFilePath, "wb");
	if(p_file != NULL){
		fwrite(&dib_format_layout, 1, sizeof(BITMAPFILEHEADER), p_file);
		fwrite(&dib_define, 1, sizeof(BITMAPINFOHEADER), p_file);
		fwrite(p_image_data, 1, dib_define.bmiHeader.biSizeImage, p_file);
		fclose(p_file);
	}

	if(NULL != h_bitmap) DeleteObject(h_bitmap);
	if(NULL != h_screen_dc) ::ReleaseDC(NULL, h_screen_dc); 
}
