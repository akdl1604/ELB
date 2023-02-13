// DlgMsg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "DlgMsg.h"
#include "afxdialogex.h"


// CDlgMsg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CDlgMsg, CDialogEx)

CDlgMsg::CDlgMsg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgMsg::IDD, pParent)
{
	m_nBlinking=0;
}

CDlgMsg::~CDlgMsg()
{
}

void CDlgMsg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_MESSAGE, m_stcMessage);
}


BEGIN_MESSAGE_MAP(CDlgMsg, CDialogEx)
	ON_WM_TIMER()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BUTTON_OK, &CDlgMsg::OnBnClickedButtonOk)
END_MESSAGE_MAP()


// CDlgMsg �޽��� ó�����Դϴ�.


BOOL CDlgMsg::PreTranslateMessage(MSG* pMsg)
{
	if( pMsg->message == WM_KEYDOWN )
		if( pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE )
			pMsg->message = NULL;

	return CDialogEx::PreTranslateMessage(pMsg);
}


BOOL CDlgMsg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_Font.CreateFont(40, 0,0,0, FW_HEAVY, 0,0,0, DEFAULT_CHARSET,
							OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,
							DEFAULT_PITCH | FF_DONTCARE, "Arial");

	m_stcMessage.SetFont(&m_Font);


	//m_stcMessage.SetColor(RGB(255,255,0));
	//m_stcMessage.SetGradientColor(RGB(255,255,0));
	//m_stcMessage.SetTextColor(RGB(0,0,255));
	m_stcMessage.Invalidate(false);

	/////////////////////////////////////////////////////////////////////////
	//																	   //
	//	������ ���̾�α� ����� �Ӽ� �ֱ�								   //
	//																	   //
	/////////////////////////////////////////////////////////////////////////

	ModifyStyleEx(0, WS_EX_LAYERED);

	// user32.dll�� �ִ� SetLayeredWindowAttributes()�� �����͸� ��´�
	HMODULE hUserDll = ::LoadLibrary(_T("USER32.dll"));
	typedef BOOL (WINAPI* LPSETTRANSPARENT)(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);

	LPSETTRANSPARENT lpSetTransparent = NULL;
	lpSetTransparent  = (LPSETTRANSPARENT)::GetProcAddress(hUserDll, "SetLayeredWindowAttributes");

	if(lpSetTransparent)
	// SetLayeredWindowAttributes() �Լ��� ȣ���Ѵ�.
	// ����° ������ 0 ~ 255������ ���� �����Ѵ�
	lpSetTransparent(m_hWnd, NULL, 200, LWA_ALPHA);

	// user32.dll�� �ݴ´�
	::FreeLibrary(hUserDll);
	//
	/////////////////////////////////////////////////////////////////////////
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}

void CDlgMsg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	
	switch ( nIDEvent )
	{
	case 100:

		if( m_nBlinking )
		{
			m_nBlinking = 0;
			//ShowWindow(SW_SHOW);

			// Lincoln Lee - 220528 - Error Dialog color blinking instead of show-hide
			m_stcMessage.SetColor(0x0000ff);
			SetBackgroundColor(0x0000ff);
		}else{
			m_nBlinking = 1;
			//ShowWindow(SW_HIDE);
			
			// Lincoln Lee - 220528 - Error Dialog color blinking instead of show-hide
			m_stcMessage.SetColor(0xc0c0c0);
			SetBackgroundColor(0xc0c0c0);
		}

		break;
	case 200:
		KillTimer(100);
		KillTimer(200);
		m_nBlinking=0;
		ShowWindow(SW_HIDE);
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CDlgMsg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	m_stcMessage.SetColor(m_colorBk);
	m_stcMessage.SetGradientColor(m_colorBk);
	m_stcMessage.SetTextColor(m_colorText);
	m_stcMessage.Invalidate(false);

	m_stcMessage.SetWindowTextEx(m_sMessage);
}


void CDlgMsg::OnBnClickedButtonOk()
{
	KillTimer(100);
	ShowWindow(SW_HIDE);
}
