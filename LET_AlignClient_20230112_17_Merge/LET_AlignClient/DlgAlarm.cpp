// DlgAlarm.cpp : 구현 파일입니다.
//

#include "stdafx.h"
//#include "LET_Communicator.h"
#include "LET_AlignClientDlg.h"
#include "DlgAlarm.h"
#include "afxdialogex.h"


// CDlgAlarm 대화 상자입니다.

IMPLEMENT_DYNAMIC(CDlgAlarm, CDialogEx)

CDlgAlarm::CDlgAlarm(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgAlarm::IDD, pParent)
{

	m_crBackGround[0] = COLOR_LIME;
	m_crBackGround[1] = COLOR_RED;
	m_crBackGround[2] = COLOR_PINK;
	m_crBackGround[3] = COLOR_BLUE;
	m_crBackGround[4] = COLOR_GRAY;
	m_nLastIndex = -1;
	m_nCount = 0;
	memset(m_bIsUse, 0, sizeof(m_bIsUse));
}

CDlgAlarm::~CDlgAlarm()
{
}

void CDlgAlarm::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LB_ALARM_MESSAGE, m_LbAlarmMessage);
	DDX_Control(pDX, IDC_BTN_CONFIRM, m_BtnAlarmMessage);
}


BEGIN_MESSAGE_MAP(CDlgAlarm, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_CONFIRM, &CDlgAlarm::OnBnClickedBtnConfirm)
	ON_WM_CLOSE()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CDlgAlarm 메시지 처리기입니다.
void CDlgAlarm::MainButtonInit(CButtonEx *pbutton,int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}

void CDlgAlarm::InitTitle(CLabelEx *pTitle,CString str,float size,COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}



BOOL CDlgAlarm::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	m_hbrBkg = CreateSolidBrush(COLOR_BTN_BODY);
	InitTitle(&m_LbAlarmMessage, " ", 20.f, COLOR_BTN_BODY);
	m_LbAlarmMessage.SetAlignTextLT();
	m_LbAlarmMessage.SetColorText(255, COLOR_BLACK);

	MainButtonInit(&m_BtnAlarmMessage );
	SetWindowText(m_strTitle);

	SetTimer(1, 300, NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


HBRUSH CDlgAlarm::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  여기서 DC의 특성을 변경합니다.

	// TODO:  기본값이 적당하지 않으면 다른 브러시를 반환합니다.

	return m_hbrBkg;
}


void CDlgAlarm::OnBnClickedBtnConfirm()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	ShowWindow(SW_HIDE);
}

void CDlgAlarm::showAlarmMessage(CString strMessage)
{
	m_LbAlarmMessage.SetText(strMessage);
	ShowWindow(SW_SHOW);
}


void CDlgAlarm::OnClose()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	ShowWindow(SW_HIDE);
	return;

	CDialogEx::OnClose();
}

void CDlgAlarm::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	if( IsWindowVisible() )
	{
		while(1)
		{
			int nIndex = rand() % 5;

			if( m_bIsUse[nIndex] != TRUE && m_nLastIndex != nIndex )
			{
				m_bIsUse[nIndex] = TRUE;
				m_LbAlarmMessage.SetColorBkg(255, m_crBackGround[nIndex]);
				m_nCount++;
				m_nLastIndex = nIndex;
				break;
			}
		}

		if( m_nCount == 5 )
		{
			m_nCount = 0;
			memset(m_bIsUse, 0, sizeof(m_bIsUse));
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}

