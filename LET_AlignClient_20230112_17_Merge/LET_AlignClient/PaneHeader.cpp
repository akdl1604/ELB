// PaneHeader.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "PaneHeader.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
// CPaneHeader

IMPLEMENT_DYNCREATE(CPaneHeader, CFormView)

CPaneHeader::CPaneHeader(): CFormView(CPaneHeader::IDD)
{
	
}

CPaneHeader::~CPaneHeader()
{
}

void CPaneHeader::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_STATIC_NOW_TIME, m_LbNowTime);
	DDX_Control(pDX, IDC_STATIC_PROGRAM_TITLE, m_LbProgramTitle);
	DDX_Control(pDX, IDC_STATIC_PROGRAM_VERSION, m_LbProgramVersion);

	DDX_Control(pDX, IDC_STATIC_CONNECT_SERVER, m_LbConnectServer);

	DDX_Control(pDX, IDC_STATIC_CONNECT_CAM1, m_LbConnectCamera[0]);
	DDX_Control(pDX, IDC_STATIC_CONNECT_CAM2, m_LbConnectCamera[1]);
	DDX_Control(pDX, IDC_STATIC_CONNECT_CAM3, m_LbConnectCamera[2]);
	DDX_Control(pDX, IDC_STATIC_CONNECT_CAM4, m_LbConnectCamera[3]);
	DDX_Control(pDX, IDC_STATIC_CONNECT_CAM6, m_LbConnectCamera[4]);
	DDX_Control(pDX, IDC_STATIC_CONNECT_CAM7, m_LbConnectCamera[5]);
	DDX_Control(pDX, IDC_STATIC_CONNECT_CAM8, m_LbConnectCamera[6]);
	DDX_Control(pDX, IDC_STATIC_CONNECT_CAM9, m_LbConnectCamera[7]);

	DDX_Control(pDX, IDC_STATIC_CONNECT_LIGHT1, m_lbLight[0]);
	DDX_Control(pDX, IDC_STATIC_CONNECT_LIGHT2, m_lbLight[1]);
	DDX_Control(pDX, IDC_STATIC_CONNECT_TRIGGER1, m_lblTrigger[0]);
	DDX_Control(pDX, IDC_STATIC_CONNECT_TRIGGER2, m_lblTrigger[1]);

	DDX_Control(pDX, IDC_STATIC_CONNECT_PLC, m_lblPlcConnected);
	DDX_Control(pDX, IDC_STATIC_EQUIPMENT_TITLE, m_lbEquipTitle);
}

BEGIN_MESSAGE_MAP(CPaneHeader, CFormView)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_VIEW_CONTROL, OnViewControl)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_STN_DBLCLK(IDC_STATIC_PROGRAM_TITLE, &CPaneHeader::OnStnDblclickStaticProgramTitle)
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


// CPaneHeader 진단입니다.

#ifdef _DEBUG
void CPaneHeader::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPaneHeader::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CPaneHeader 메시지 처리기입니다.

void CPaneHeader::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);
	GetClientRect(&m_rcForm);

	m_pMain = (CLET_AlignClientDlg *)GetParent();
	m_pMain->m_hHeaderBar = this->m_hWnd;

	CString verstr, str;
	verstr.Format(_T("Build - %s, %s"), (CString)__DATE__, (CString)__TIME__);

	InitTitle(&m_LbNowTime, "", 14.f, RGB(64, 64, 64));
	InitTitle(&m_lbEquipTitle, m_pMain->m_strEquipName, 16.f, RGB(64, 64, 64));
	InitTitle(&m_LbProgramTitle, m_pMain->m_strClientName, 16.f, RGB(64, 64, 64));
	InitTitle(&m_LbProgramVersion, verstr, 14.f, RGB(64, 64, 64));
	InitTitle(&m_LbConnectServer, "Server", 14.f, RGB(64, 64, 64));
	InitTitle(&m_LbConnectCamera[0], "Cam1", 14.f, RGB(64, 64, 64));
	InitTitle(&m_LbConnectCamera[1], "Cam2", 14.f, RGB(64, 64, 64));
	InitTitle(&m_LbConnectCamera[2], "Cam3", 14.f, RGB(64, 64, 64));
	InitTitle(&m_LbConnectCamera[3], "Cam4", 14.f, RGB(64, 64, 64));
	InitTitle(&m_LbConnectCamera[4], "Cam5", 14.f, RGB(64, 64, 64));
	InitTitle(&m_LbConnectCamera[5], "Cam6", 14.f, RGB(64, 64, 64));
	InitTitle(&m_LbConnectCamera[6], "Cam7", 14.f, RGB(64, 64, 64));
	InitTitle(&m_LbConnectCamera[7], "Cam8", 14.f, RGB(64, 64, 64));
	InitTitle(&m_lblPlcConnected, "PLC", 14.f, RGB(64, 64, 64));

	InitTitle(&m_lbLight[0], "LC1", 14.f, RGB(64, 64, 64));
	InitTitle(&m_lbLight[1], "LC2", 14.f, RGB(64, 64, 64));
	InitTitle(&m_lblTrigger[0], "T1", 14.f, RGB(64, 64, 64));
	InitTitle(&m_lblTrigger[1], "T2", 14.f, RGB(64, 64, 64));
	//KJH 2022-04-25 Camera별 Display추가
#if _DAHUHA
	m_LbProgramVersion.SetColorBkg(255, COLOR_PURPLE);
#else
	m_LbProgramVersion.SetColorBkg(255, COLOR_BLUE);
#endif // _DAHUHA

}

HBRUSH CPaneHeader::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{

	return m_hbrBkg;
}

void CPaneHeader::InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}

void CPaneHeader::OnUpdateTime()
{
	CTime time = CTime::GetCurrentTime();

	CString strTime = _T("");
	strTime.Format(_T("%04d / %02d / %02d  %02d:%02d:%02d"), time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute(), time.GetSecond());

	m_LbNowTime.SetText(strTime);
}

LRESULT CPaneHeader::OnViewControl(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case MSG_HB_CONNECT_SERVER:
	{
		//클라이언트가 서버에 접속되면 가지고있는 회전중심을 서버로 전송한다.
		BOOL bConnect = (BOOL)lParam;
		if (bConnect)	m_LbConnectServer.SetColorBkg(255, COLOR_CONNECT);
		else			m_LbConnectServer.SetColorBkg(255, COLOR_DISCONNECT);
	}
	break;
	case MSG_HB_CONNECT_CAMERA:
	{
		int nCam = lParam & 0xFFFF;
		BOOL bConnect = lParam >> 16 & 0xFFFF;

		if (bConnect)	m_LbConnectCamera[nCam].SetColorBkg(255, COLOR_CONNECT);
		else			m_LbConnectCamera[nCam].SetColorBkg(255, COLOR_DISCONNECT);
	}
	break;
	case MSG_HB_CONNECT_TRIGGER:
	{
		int nID = LOWORD(lParam);
		BOOL bConnect = HIWORD(lParam);

		if (bConnect)	m_lblTrigger[nID].SetColorBkg(255, COLOR_CONNECT);
		else			m_lblTrigger[nID].SetColorBkg(255, COLOR_DISCONNECT);
	}
	break;
	case MSG_HB_CONNECT_LIGHT:
	{
		int nID = LOWORD(lParam);
		BOOL bConnect = HIWORD(lParam);

		if (bConnect)	m_lbLight[nID].SetColorBkg(255, COLOR_CONNECT);
		else			m_lbLight[nID].SetColorBkg(255, COLOR_DISCONNECT);
	}
	break;
	case MSG_PLC_CONNECT_STATUS:
	{
		BOOL bConnect = (BOOL)lParam;
		if (bConnect)	m_lblPlcConnected.SetColorBkg(255, COLOR_CONNECT);
		else			m_lblPlcConnected.SetColorBkg(255, COLOR_DISCONNECT);
	}
	break;
	case MSG_PLC_CONNECT_STATUS_CONNECT:
	{
		CString s;
		s.Format("%d", atoi(m_lblPlcConnected.GetText()) + 1);
		m_lblPlcConnected.SetText(s);
	}
	break;
	case MSG_VAT_CONNECT_STATUS_CONNECT:
	{
#ifdef _SDV_VAT
		BOOL bConnect = (BOOL)lParam;
		CString s;

		s.Format("%s_VAT", m_pMain->m_strClientName);

		if (bConnect)	m_LbProgramTitle.SetColorBkg(255, COLOR_CONNECT);			
		else			m_LbProgramTitle.SetColorBkg(255, COLOR_DISCONNECT);

		m_LbProgramTitle.SetText(s);
#endif
	}
	break;
	};

	return 0;
}

void CPaneHeader::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 그리기 메시지에 대해서는 CFormView::OnPaint()을(를) 호출하지 마십시오.

	CPen pen(PS_SOLID, 3, COLOR_DDARK_GRAY), *pOld;

	pOld = dc.SelectObject(&pen);
	dc.MoveTo(m_rcForm.left, m_rcForm.top);
	dc.LineTo(m_rcForm.right, m_rcForm.top);
	dc.LineTo(m_rcForm.right, m_rcForm.bottom);
	dc.LineTo(m_rcForm.left, m_rcForm.bottom);
	dc.LineTo(m_rcForm.left, m_rcForm.top);
	dc.SelectObject(pOld);
}


void CPaneHeader::OnStnDblclickStaticProgramTitle()
{
	if (theApp.m_bEnforceOkFlag == FALSE)
	{
		theApp.m_bEnforceOkFlag = TRUE;
		m_LbProgramTitle.SetColorBkg(255, RGB(0, 255, 0));
	}
	else if (theApp.m_bEnforceOkFlag == TRUE)
	{
		theApp.m_bEnforceOkFlag = FALSE;
		m_LbProgramTitle.SetColorBkg(255, RGB(64, 64, 64));
	}
}

void CPaneHeader::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (nIDEvent == 8234)
	{
		CString str;
		SYSTEMTIME m_csTime;

		::GetLocalTime(&m_csTime);
		str.Format("%d / %02d / %02d   %02d : %02d : %02d", m_csTime.wYear, m_csTime.wMonth, m_csTime.wDay,
			m_csTime.wHour, m_csTime.wMinute, m_csTime.wSecond);

		OnUpdateTime();

		if (theApp.m_pFrame->m_InspCounter.m_nToday != m_csTime.wDay)
		{
			if (m_csTime.wHour >= GY_START_TIME)
			{
				theApp.m_pFrame->m_InspCounter.TodayStart(); //Daily Count 초기화 
			}
		}

		CTime today(theApp.m_pFrame->m_InspCounter.m_nLastInspYear, theApp.m_pFrame->m_InspCounter.m_nLastInspMonth, theApp.m_pFrame->m_InspCounter.m_nLastInspDay,
			theApp.m_pFrame->m_InspCounter.m_nLastInspHour, theApp.m_pFrame->m_InspCounter.m_nLastInspMinute, theApp.m_pFrame->m_InspCounter.m_nLastInspSecond);

		CTime now = CTime::GetCurrentTime();
		CTimeSpan timespan = now - today;

		if (timespan.GetDays() >= 1)
		{
			theApp.m_pFrame->m_InspCounter.TodayCountReset();

			theApp.m_pFrame->m_InspCounter.m_nLastInspYear = now.GetYear();
			theApp.m_pFrame->m_InspCounter.m_nLastInspMonth = now.GetMonth();
			theApp.m_pFrame->m_InspCounter.m_nLastInspDay = now.GetDay();
			theApp.m_pFrame->m_InspCounter.m_nLastInspHour = now.GetHour();
			theApp.m_pFrame->m_InspCounter.m_nLastInspMinute = now.GetMinute();
			theApp.m_pFrame->m_InspCounter.m_nLastInspSecond = now.GetSecond();
		}
	}

	CFormView::OnTimer(nIDEvent);
}


BOOL CPaneHeader::IsClickHeaderTitle(CPoint pt)
{
	CRect IdRect, rect;
	GetWindowRect(&rect);
	m_LbProgramTitle.GetWindowRect(&IdRect);

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;
	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;
	IdRect.bottom = IdRect.top + h;

	if (pt.x >= IdRect.left &&
		pt.x <= IdRect.right &&
		pt.y >= IdRect.top &&
		pt.y <= IdRect.bottom)
		return TRUE;
	else
		return FALSE;
}

BOOL CPaneHeader::IsClickServerTitle(CPoint pt)
{
	CRect IdRect, rect;
	GetWindowRect(&rect);
	m_LbConnectServer.GetWindowRect(&IdRect);

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;
	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;
	IdRect.bottom = IdRect.top + h;

	if (pt.x >= IdRect.left &&		pt.x <= IdRect.right &&	pt.y >= IdRect.top &&		pt.y <= IdRect.bottom)		return TRUE;
	else		return FALSE;
}


BOOL CPaneHeader::IsClickVersionTitle(CPoint pt)
{
	CRect IdRect, rect;
	GetWindowRect(&rect);
	m_LbProgramVersion.GetWindowRect(&IdRect);

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;
	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;
	IdRect.bottom = IdRect.top + h;

	if (pt.x >= IdRect.left &&	pt.x <= IdRect.right &&	pt.y >= IdRect.top &&	pt.y <= IdRect.bottom)		return TRUE;
	else		return FALSE;
}

BOOL CPaneHeader::IsClickPLCTitle(CPoint pt)
{
	CRect IdRect, rect;
	GetWindowRect(&rect);
	m_lblPlcConnected.GetWindowRect(&IdRect);

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;
	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;
	IdRect.bottom = IdRect.top + h;

	if (pt.x >= IdRect.left && pt.x <= IdRect.right && pt.y >= IdRect.top && pt.y <= IdRect.bottom)		return TRUE;
	else		return FALSE;
}

void CPaneHeader::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	if (IsClickHeaderTitle(point) == TRUE || IsClickServerTitle(point) == TRUE)
	{
		theApp.m_pFrame->ShowWindow(SW_MINIMIZE);
	}
	else if (IsClickPLCTitle(point) == TRUE)
	{
		if (m_pMain->m_pVAT_SimulatorDlg->IsWindowVisible())
		{
			m_pMain->m_pVAT_SimulatorDlg->ShowWindow(SW_HIDE);
		}
		else
		{
			m_pMain->m_pVAT_SimulatorDlg->ShowWindow(SW_SHOW);
		}
	}

	if (IsClickVersionTitle(point) == TRUE)
	{
		//KJH 2022-07-15 강제 OK Mode관련 권한설정
		if (m_pMain->m_nLogInUserLevelType == MASTER)
		{
			//KJH 2021-11-30 강제 OK모드관련 암호 기능 추가
			CKeyPadDlg dlg;
			CString strTemp, strTime;

			dlg.SetValueString(true, strTemp);
			dlg.DoModal();
			dlg.GetValue(strTemp);

			SYSTEMTIME time;
			::GetLocalTime(&time);
			strTime.Format("%02d%02d", time.wHour, time.wMinute);

			BOOL bFind = TRUE;
			int nLen = strTemp.GetLength();

			// 입력한 값 중에 시간 + 분 값이 있는지
			for (int i = 0; i < 4; i++)
			{
				if (strTemp.Find(strTime.GetAt(i)) < 0)
					bFind = FALSE;
			}

			if (bFind != TRUE)
			{
				return;
			}

			OnStnDblclickStaticProgramTitle();
		}
		else
		{
			// YCS 22022-12-12 버전정보 get
			//const char* FileDirectory = "C:\\LET_AlignClient\\LET_AlignClient.exe";

			//DWORD VersionSize = GetFileVersionInfoSize(FileDirectory, 0);

			//char* VersionBuffer = new char[VersionSize];

			//if (GetFileVersionInfo(FileDirectory, 0, VersionSize, VersionBuffer))
			//{
			//	VS_FIXEDFILEINFO* pFileInfo;

			//	//if(VerQueryValue(FileDirectory,_T("\\"), (LPVOID*)pFileInfo, ))
			//}

			//delete VersionBuffer;
		}
	}

	CFormView::OnLButtonUp(nFlags, point);
}
