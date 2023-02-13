
// LET_AlignClient.cpp : ���� ���α׷��� ���� Ŭ���� ������ �����մϴ�.
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


// CLET_AlignClientApp ����

CLET_AlignClientApp::CLET_AlignClientApp()
{
	// �ٽ� ���� ������ ����
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: ���⿡ ���� �ڵ带 �߰��մϴ�.
	// InitInstance�� ��� �߿��� �ʱ�ȭ �۾��� ��ġ�մϴ�.	

	m_bEnforceOkFlag = FALSE;
	SWH_return = NULL;
}


// ������ CLET_AlignClientApp ��ü�Դϴ�.

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
// CLET_AlignClientApp �ʱ�ȭ

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

	// ���� ���α׷� �Ŵ��佺Ʈ�� ComCtl32.dll ���� 6 �̻��� ����Ͽ� ���־� ��Ÿ����
	// ����ϵ��� �����ϴ� ���, Windows XP �󿡼� �ݵ�� InitCommonControlsEx()�� �ʿ��մϴ�.
	// InitCommonControlsEx()�� ������� ������ â�� ���� �� �����ϴ�.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ���� ���α׷����� ����� ��� ���� ��Ʈ�� Ŭ������ �����ϵ���
	// �� �׸��� �����Ͻʽÿ�.
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

	// ��ȭ ���ڿ� �� Ʈ�� �� �Ǵ�
	// �� ��� �� ��Ʈ���� ���ԵǾ� �ִ� ��� �� �����ڸ� ����ϴ�.
	CShellManager *pShellManager = new CShellManager;

	// ǥ�� �ʱ�ȭ
	// �̵� ����� ������� �ʰ� ���� ���� ������ ũ�⸦ ���̷���
	// �Ʒ����� �ʿ� ���� Ư�� �ʱ�ȭ
	// ��ƾ�� �����ؾ� �մϴ�.
	// �ش� ������ ����� ������Ʈ�� Ű�� �����Ͻʽÿ�.
	// TODO: �� ���ڿ��� ȸ�� �Ǵ� ������ �̸��� ����
	// ������ �������� �����ؾ� �մϴ�.
	SetRegistryKey(_T("���� ���� ���α׷� �����翡�� ������ ���� ���α׷�"));

	m_dumpHelp.install_self_mini_dump();

	GraphicLabel::Init();
	CLET_AlignClientDlg dlg;
	m_pMainWnd = &dlg;

	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: ���⿡ [Ȯ��]�� Ŭ���Ͽ� ��ȭ ���ڰ� ������ �� ó����
		//  �ڵ带 ��ġ�մϴ�.
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: ���⿡ [���]�� Ŭ���Ͽ� ��ȭ ���ڰ� ������ �� ó����
		//  �ڵ带 ��ġ�մϴ�.
	}

	// ������ ���� �� �����ڸ� �����մϴ�.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// ��ȭ ���ڰ� �������Ƿ� ���� ���α׷��� �޽��� ������ �������� �ʰ�  ���� ���α׷��� ���� �� �ֵ��� FALSE��
	// ��ȯ�մϴ�.
	return FALSE;
}
int CLET_AlignClientApp::ExitInstance()
{
	if (SWH_return) UnhookWindowsHookEx(SWH_return);

	return CWinApp::ExitInstance();
}

void CLET_AlignClientApp::OnCaptureCurrentScreen(int nType) 
{
	// ��ũ�� ��ü�� ĸ���ϱ� ���ؼ� CWindowDC �������� DC�� ��´�.
	// GetDC�� �Ķ���Ϳ� Ư�� ������ �ڵ��� ���� �ʰ� NULL�� ������ 
	// CWindowDC(���� DC) �������� DC�� ��� �ȴ�.
	HDC h_screen_dc = ::GetDC(NULL);
	// ���� ��ũ���� �ػ󵵸� ��´�.
	int width = ::GetDeviceCaps(h_screen_dc, HORZRES);
	int height = ::GetDeviceCaps(h_screen_dc, VERTRES);

	// DIB�� ������ �����Ѵ�. 
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

	// DIB�� ���� �̹��� ��Ʈ ������ ������ ������ ����
	BYTE *p_image_data = NULL;
	// dib_define�� ���ǵ� �������� DIB�� �����Ѵ�.
	HBITMAP h_bitmap = ::CreateDIBSection(h_screen_dc, &dib_define, DIB_RGB_COLORS, (void **)&p_image_data, 0, 0);
	// �̹����� �����ϱ� ���ؼ� ���� DC�� �����Ѵ�. ���� DC������ ���������� ��Ʈ�ʿ� �����Ͽ�
	// �̹��� ������ ���� �� ���� �����̴�.
	HDC h_memory_dc = ::CreateCompatibleDC(h_screen_dc);
	// ���� DC�� �̹����� ������ ��Ʈ���� �����Ѵ�.
	HBITMAP h_old_bitmap = (HBITMAP)::SelectObject(h_memory_dc, h_bitmap);
	// ���� ��ũ�� ȭ���� ĸ���Ѵ�.
	::BitBlt(h_memory_dc, 0, 0, width, height, h_screen_dc, 0, 0, SRCCOPY);
	// ������ ��Ʈ������ �����Ѵ�.
	::SelectObject(h_memory_dc, h_old_bitmap); 
	// ���� DC�� �����Ѵ�.
	DeleteDC(h_memory_dc);

	// DIB ������ ������ �����Ѵ�.
	BITMAPFILEHEADER dib_format_layout;
	ZeroMemory(&dib_format_layout, sizeof(BITMAPFILEHEADER));
	dib_format_layout.bfType = *(WORD*)"BM";
	dib_format_layout.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dib_define.bmiHeader.biSizeImage;
	dib_format_layout.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	// DIB ������ �����Ѵ�.
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
