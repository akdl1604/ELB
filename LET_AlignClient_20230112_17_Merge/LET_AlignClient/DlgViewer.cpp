// DlgViewer.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "DlgViewer.h"
#include "afxdialogex.h"

#ifndef WM_VIEW_CONTROL
#define WM_VIEW_CONTROL WM_USER + 0x10
#endif
// CDlgViewer 대화 상자입니다.

IMPLEMENT_DYNAMIC(CDlgViewer, CDialogEx)

CDlgViewer::CDlgViewer(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgViewer::IDD, pParent)
{
	m_bPopup = FALSE;
	m_bSizeChanged = FALSE;

	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);

	int dlgW = 960;
	int dlgH = 720;
	m_rtDlgPopPos.left = width / 2 - dlgW / 2;
	m_rtDlgPopPos.top = height / 2 - dlgH / 2;
	m_rtDlgPopPos.right = m_rtDlgPopPos.left + dlgW;
	m_rtDlgPopPos.bottom = m_rtDlgPopPos.top + dlgH;

	m_nGridResult_H = 100;
	m_nGridInfo_H = 30;
	m_nGridInfo_W = 100;
	m_nFuncBtn_W = 131;
	m_nViewID = 0;
	m_bShowGridInfo = FALSE;
	m_bMaximize = FALSE;
	m_bShowTabCtrl = FALSE;
	m_bUsePopupWindow = FALSE;
	m_bEnableGrab = TRUE;
	m_bLiveCheck = FALSE;
	InitializeCriticalSection(&m_csWindows);
}

CDlgViewer::~CDlgViewer()
{
	DeleteCriticalSection(&m_csWindows);
}

void CDlgViewer::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CS_VIEWER, m_Viewer);
	DDX_Control(pDX, IDC_CS_GRID_INFO, m_GridInfo);
	DDX_Control(pDX, IDC_CS_GRID_RESULT, m_GridResult);
	DDX_Control(pDX, IDC_TAB_INFO, m_TabViewer);
	DDX_Control(pDX, IDC_EDIT_HISTORY, m_EditHistory);

	DDX_Control(pDX, IDC_BTN_DLG_DRAW_AREA, m_btnFunc[FUNC_DRAW_AREA]);
	DDX_Control(pDX, IDC_BTN_DLG_ORIGINAL_IMAGE, m_btnFunc[FUNC_ORIGIN_IMAGE]);
	DDX_Control(pDX, IDC_BTN_DLG_FIT_IMAGE, m_btnFunc[FUNC_FIT_IMAGE]);
	DDX_Control(pDX, IDC_BTN_DLG_CENTER_LINE, m_btnFunc[FUNC_CENTER_LINE]);
	DDX_Control(pDX, IDC_BTN_DLG_ZOOM_IN, m_btnFunc[FUNC_ZOOM_IN]);
	DDX_Control(pDX, IDC_BTN_DLG_ZOOM_OUT, m_btnFunc[FUNC_ZOOM_OUT]);
	DDX_Control(pDX, IDC_BTN_DLG_SHOW_OVERLAY, m_btnFunc[FUNC_SHOW_OVERLAY]);	
}

BEGIN_MESSAGE_MAP(CDlgViewer, CDialogEx)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_EXITSIZEMOVE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_INFO, &CDlgViewer::OnTcnSelchangeTabInfo)
	ON_MESSAGE(WM_VIEW_CONTROL, &CDlgViewer::OnViewControl)
	ON_BN_CLICKED(IDC_BTN_DLG_ORIGINAL_IMAGE, &CDlgViewer::OnBnClickedBtnOriginalImage)
	ON_BN_CLICKED(IDC_BTN_DLG_FIT_IMAGE, &CDlgViewer::OnBnClickedBtnFitImage)
	ON_BN_CLICKED(IDC_BTN_DLG_CENTER_LINE, &CDlgViewer::OnBnClickedBtnCenterLine)
	ON_BN_CLICKED(IDC_BTN_DLG_DRAW_AREA, &CDlgViewer::OnBnClickedBtnDrawArea)
	ON_WM_TIMER()
	ON_WM_MOUSEHWHEEL()
	ON_BN_CLICKED(IDC_BTN_DLG_ZOOM_IN, &CDlgViewer::OnBnClickedBtnZoomIn)
	ON_BN_CLICKED(IDC_BTN_DLG_ZOOM_OUT, &CDlgViewer::OnBnClickedBtnZoomOut)
	ON_BN_CLICKED(IDC_BTN_DLG_SHOW_OVERLAY, &CDlgViewer::OnBnClickedBtnDlgShowOverlay)
	ON_MESSAGE(WM_KICKIDLE, &CDlgViewer::OnKickIdle)
	ON_COMMAND(ID_BUTTON_FIRST, &CDlgViewer::OnCommandFirst)
	ON_COMMAND(ID_BUTTON_SECOND, &CDlgViewer::OnCommandSecond)
	ON_COMMAND(ID_BUTTON_THIRD, &CDlgViewer::OnCommandThird)
	ON_COMMAND(ID_BUTTON_FOUR, &CDlgViewer::OnCommandFour)
	ON_COMMAND(ID_BUTTON_LIVE, &CDlgViewer::OnCommandLive)
	ON_COMMAND(ID_BUTTON_RECORD, &CDlgViewer::OnCommandRecord)
	ON_COMMAND(ID_BUTTON_STOP, &CDlgViewer::OnCommandStop)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// CDlgViewer 메시지 처리기입니다.

void CDlgViewer::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 그리기 메시지에 대해서는 CDialogEx::OnPaint()을(를) 호출하지 마십시오.

}

BOOL CDlgViewer::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	
	//Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	//Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

	m_Viewer.InitControl(this);

	m_TabViewer.InsertItem(0, "Result", 1);
	m_TabViewer.InsertItem(1, "History", 0);

	InitGridInfo(m_GridInfo, 2, 1);
	InitGridResult(m_GridResult, 6, 10);

	MainButtonInit(&m_btnFunc[FUNC_DRAW_AREA], IDB_PNG_RULER);
	MainButtonInit(&m_btnFunc[FUNC_ORIGIN_IMAGE], IDB_PNG_ZOOM_DEFAULT);
	MainButtonInit(&m_btnFunc[FUNC_FIT_IMAGE], IDB_PNG_ZOOM_FIT);
	MainButtonInit(&m_btnFunc[FUNC_CENTER_LINE], IDB_PNG_CENTER_LINE);
	MainButtonInit(&m_btnFunc[FUNC_ZOOM_IN], IDB_PNG_ZOOM_IN);
	MainButtonInit(&m_btnFunc[FUNC_ZOOM_OUT], IDB_PNG_ZOOM_OUT);
	MainButtonInit(&m_btnFunc[FUNC_SHOW_OVERLAY], IDB_PNG_OVERLAY);

	SetTimer(TIMER_VIEWER_UI_CEHCK, 1000, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

BOOL CDlgViewer::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	if( pMsg->message == WM_LBUTTONDBLCLK )
	{
		CRect rcPos;
		m_Viewer.GetWindowRect(&rcPos);

		// 뷰어 영역을 더블 클릭 햇을 경우에만 
		if( PtInRect(&rcPos, pMsg->pt) )
		{
			m_bPopup = !m_bPopup;

			if( m_bPopup != TRUE )
			{
				if( m_bMaximize )
				{
					if( m_rtDlgPopPos.Width() > 0 )	
						MoveWindow(m_rtDlgPopPos);
				}
			}

			PopupWindow(m_bPopup);
		}
	}
	else if( pMsg->message == WM_NCLBUTTONDBLCLK )
	{
		//ShowWindow(SW_MAXIMIZE);
		//MoveWindow(0, 0, 1920, 1080);


		m_bMaximize = !m_bMaximize;

		if( m_bMaximize ) MoveWindow(0, 0, 1920, 1080);
		else
		{
			if( m_rtDlgPopPos.Width() > 0 )	
				MoveWindow(m_rtDlgPopPos);
		}

		fitViewer();

		ModifyStyle(WS_CHILD , WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_BORDER | WS_THICKFRAME);
		/*SetParent( GetDesktopWindow() );
		if( m_rtDlgPopPos.Width() > 0 )	MoveWindow(m_rtDlgPopPos);
		fitViewer();
		::SetWindowPos(this->m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);*/
	}


	if(pMsg->message == WM_KEYDOWN )
	{
		if(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE )
			return TRUE;
	}
	else if (pMsg->message == WM_SYSKEYDOWN)	return TRUE;

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CDlgViewer::PopupWindow(BOOL bPopup)
{
	if( m_bUsePopupWindow != TRUE ) return;

	EnterCriticalSection(&m_csWindows);

	if( m_bPopup )
	{
		CWnd* mWnd = GetDesktopWindow(); //20210910 Tkyuha 한번에 읽어 오기
		
		if (mWnd)
		{
			SetParent(mWnd); //20210910 Tkyuha 한번에 읽어 오기, 20210927 순서 바꿈
			ModifyStyle(WS_CHILD, WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_BORDER | WS_THICKFRAME);			
			if (m_rtDlgPopPos.Width() > 0)	MoveWindow(m_rtDlgPopPos);
			fitViewer();
			::SetWindowPos(this->m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			::SetWindowLong(this->m_hWnd, GWL_EXSTYLE, WS_EX_NOACTIVATE);
		}
	}
	else
	{
		GetWindowRect(&m_rtDlgPopPos);
		SetParent( m_pParent );
		ModifyStyle(WS_POPUP | WS_CAPTION | WS_BORDER | WS_THICKFRAME, WS_CHILD | WS_VISIBLE );
		MoveWindow(m_rtDlgChildPos);
		fitViewer();
		::SetWindowPos(this->m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);	
		::SetWindowLong(this->m_hWnd, GWL_EXSTYLE, WS_EX_NOACTIVATE);
	}

	LeaveCriticalSection(&m_csWindows);
}

void CDlgViewer::fitViewer()
{
	CRect rcDlg;

	GetClientRect(&rcDlg);

	// 그리드 창들을 표시하기 위한 최소 사이즈가 있어서.. 예외 처리
	int nMinSize = 150;
	//if(	rcDlg.Height() < m_nGridResult_H + m_nGridInfo_H + nMinSize )
	if(	rcDlg.Height() < m_nGridResult_H + m_nGridInfo_H )
	{
		rcDlg.bottom = rcDlg.top + m_nGridResult_H + m_nGridInfo_H + nMinSize;
		MoveWindow(rcDlg);
		GetClientRect(&rcDlg);
	}

	if( rcDlg.Width() < m_nGridInfo_W + m_nFuncBtn_W )
	{
		rcDlg.right = rcDlg.left + m_nGridInfo_W + m_nFuncBtn_W + nMinSize;
		MoveWindow(rcDlg);
		GetClientRect(&rcDlg);
	}

	////////////////////////////////////////////////////
	/////////// Compute Button Position	///////////
	
	CRect currPos[MAX_VIEWER_FUNC], destPos[MAX_VIEWER_FUNC];

	for(int i = 0; i < MAX_VIEWER_FUNC; i++)
	{
		m_btnFunc[i].GetWindowRect(&currPos[i]);
		ScreenToClient( &currPos[i] );
	}

	int offsetY = 3;
	int btnW = currPos[MAX_VIEWER_FUNC - 1].right - currPos[0].left;
	int dx = rcDlg.right - currPos[0].left - btnW;
	int dy = offsetY - currPos[0].top;

	for(int i = 0; i < MAX_VIEWER_FUNC; i++)
	{
		destPos[i] = currPos[i];
		destPos[i].left += dx;
		destPos[i].right += dx;
		destPos[i].top += dy;
		destPos[i].bottom += dy;
	}
	
	for(int i = 0; i < MAX_VIEWER_FUNC; i++)
		m_btnFunc[i].MoveWindow(destPos[i]);
	////////////////////////////////////////////////////
	////////////////////////////////////////////////////

	CRect gridPos;

	gridPos = rcDlg;

	int gridInfoH = m_nGridInfo_H;
	int gridResultH = m_nGridResult_H;

	// Grid Info Replace
	if( m_bShowGridInfo )
	{
		m_GridInfo.ShowWindow(SW_SHOW);
		gridPos = rcDlg;
		gridPos.right = destPos[0].left;
		gridPos.bottom = gridInfoH;
		m_GridInfo.MoveWindow(gridPos);
		drawGridInfo(m_GridInfo);
	}
	else
	{
		gridInfoH = 0;
		m_GridInfo.ShowWindow(SW_HIDE);
	}

	if( m_bShowTabCtrl )
	{
		m_TabViewer.ShowWindow(SW_SHOW);
		m_GridResult.ShowWindow(SW_SHOW);
		m_EditHistory.ShowWindow(SW_SHOW);

		// Grid Result Replace
		gridPos = rcDlg;
		gridPos.top = gridPos.bottom - m_nGridResult_H;
		m_TabViewer.MoveWindow(gridPos);
		gridPos.top += 20;		// Add Tab Size
		m_GridResult.MoveWindow(gridPos);
		m_EditHistory.MoveWindow(gridPos);
		GetDlgItem(IDC_EDIT_HISTORY)->MoveWindow(gridPos);
		displayTabUI();
		drawGridResult(m_GridResult);
	}
	else
	{
		gridResultH = 0;
		m_TabViewer.ShowWindow(SW_HIDE);
		m_GridResult.ShowWindow(SW_HIDE);
		m_EditHistory.ShowWindow(SW_HIDE);
	}

	int toolbarHeight = 0;
	if (m_wndViewToolBar.GetSafeHwnd())
	{
		CSize   sizeToolBar = m_wndViewToolBar.CalcFixedLayout(TRUE, TRUE);
		toolbarHeight = sizeToolBar.cy;
	}

	rcDlg.top += gridInfoH;
	rcDlg.bottom -= (gridResultH + toolbarHeight);

	m_Viewer.MoveWindow(rcDlg);
	//m_Viewer.Invalidate();
	m_Viewer.SendMessage(WM_PAINT, 0, 0);

	if (m_wndViewToolBar.GetSafeHwnd())
	{
		CRect lpRet = rcDlg;

		lpRet.top = rcDlg.bottom;
		lpRet.bottom = lpRet.top + toolbarHeight;
		m_wndViewToolBar.MoveWindow(lpRet, FALSE);
	}


	// KBJ 2022-08-22 처음에 이미지가 Fit 안되어 있어서 추가
	m_Viewer.OnFitImage();
	m_Viewer.SoftDirtyRefresh();

}

void CDlgViewer::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	m_bSizeChanged = TRUE;
}

void CDlgViewer::OnExitSizeMove()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if( m_bSizeChanged )
	{
		m_bSizeChanged = FALSE;

		if( m_bMaximize != TRUE )	fitViewer();
	}

	CDialogEx::OnExitSizeMove();
}

void CDlgViewer::childWindow()
{
	if( m_bPopup )	m_bPopup = FALSE;
	PopupWindow(m_bPopup);
}

void CDlgViewer::OnTcnSelchangeTabInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;
	displayTabUI();
}

void CDlgViewer::displayTabUI()
{
	int nSel = m_TabViewer.GetCurSel();

	if( nSel == 0 )
	{
		m_GridResult.ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_HISTORY)->ShowWindow(SW_HIDE);
	}
	else
	{
		m_GridResult.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_HISTORY)->ShowWindow(SW_SHOW);
	}
}

void CDlgViewer::setViewerName(CString name) 
{ 
	m_strName = name; 
	m_GridInfo.SetItemText(0, 0, m_strName);
	m_GridInfo.RedrawRow(0);
}

void CDlgViewer::setPanelID_Name(CString name)
{
	m_strPanelName = name;
	m_GridInfo.SetItemText(0, 1, m_strPanelName);
	m_GridInfo.RedrawRow(0);
}

void CDlgViewer::setViewerJudge(BOOL bJudge)
{
	if(bJudge)	m_GridInfo.SetItemBkColour(0, 1, COLOR_GREEN);
	else		m_GridInfo.SetItemBkColour(0, 1, COLOR_RED);
	m_GridInfo.RedrawRow(0);
}

void CDlgViewer::resetViewerJudge()
{
	m_GridInfo.SetItemBkColour(0, 1, COLOR_DARK_GRAY);
	m_GridInfo.RedrawRow(0);
}

void CDlgViewer::addHistory(CString strText)
{
	SendMessage(WM_VIEW_CONTROL, 100, (LPARAM)&strText);
}

LRESULT CDlgViewer::OnViewControl(WPARAM wParam, LPARAM lParam)
{
	switch( wParam ) {
	case 100:
		{
			CString strTime;
			CString strText = *((CString *)lParam) + "\r\n";
			SYSTEMTIME time;
			GetLocalTime(&time);

			strTime.Format("[%02d:%02d:%02d.%03d] ", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
			m_strHistory = strTime + strText + m_strHistory;
			
			if( m_strHistory.GetLength() > 8192 )	m_strHistory = m_strHistory.Left(4096);

			m_EditHistory.SetWindowTextA(m_strHistory);
		}
		break;
	case 200:// 동영상 녹화 시작
		OnCommandRecord();
		break;
	case 300: //동영상 녹화 완료
		OnCommandStop();
		break;
	}

	return 0;
}

void CDlgViewer::drawGridInfo(CGridCtrl &grid)
{
	CRect rcGrid;

	grid.GetWindowRect(&rcGrid);
	
	int nH = rcGrid.Height() / grid.GetRowCount() - 4;
	int nW = rcGrid.Width() / grid.GetColumnCount() - 4;

	for(int i = 0; i < grid.GetRowCount(); i++)
		grid.SetRowHeight(i, nH);

	for(int i = 0; i < grid.GetColumnCount(); i++)
		grid.SetColumnWidth(i, nW);
}

void CDlgViewer::drawGridResult(CGridCtrl &grid)
{
	CRect rcGrid;

	grid.GetWindowRect(&rcGrid);

	int nH = rcGrid.Height() / grid.GetRowCount() - 2;
	int nW = rcGrid.Width() / grid.GetColumnCount() - 1;
/*
	for(int i = 0; i < grid.GetRowCount(); i++)
		grid.SetRowHeight(i, nH);*/

	/*for(int i = 0; i < grid.GetColumnCount(); i++)
		grid.SetColumnWidth(i, nW);*/
}

void CDlgViewer::InitGridInfo(CGridCtrl &grid, int nCols, int nRows)
{
	BOOL bVirtualMode = FALSE;

	grid.SetEditable(FALSE);
	grid.SetVirtualMode(bVirtualMode);
	grid.SetListMode(TRUE);
	grid.EnableDragAndDrop(FALSE);
	grid.EnableSelection(FALSE);
	grid.SetFixedRowSelection(TRUE);
	grid.SetFixedColumnSelection(TRUE);
	grid.SetFrameFocusCell(FALSE);
	grid.SetTrackFocusCell(FALSE);
	grid.SetRowResize(FALSE);
	grid.SetColumnResize(FALSE);
	grid.SetBkColor(RGB(197, 197, 187));
	grid.SetTextBkColor(RGB(235, 235, 235));
	grid.SetColumnCount(nCols);
	grid.SetRowCount(nRows);

	for(int i = 0; i < grid.GetColumnCount(); i++)
		grid.SetItemBkColour(0, i, COLOR_DDARK_GRAY);

	grid.SetTextColor(RGB(255, 255, 255));
}

void CDlgViewer::InitGridResult(CGridCtrl &grid, int nCols, int nRows)
{
	BOOL bVirtualMode = FALSE;

	grid.SetEditable(FALSE);
	grid.SetVirtualMode(bVirtualMode);
	grid.SetListMode(TRUE);
	grid.EnableDragAndDrop(FALSE);
	grid.EnableSelection(FALSE);
	grid.SetFixedRowSelection(TRUE);
	grid.SetFixedColumnSelection(TRUE);
	grid.SetFrameFocusCell(FALSE);
	grid.SetTrackFocusCell(FALSE);
	grid.SetRowResize(FALSE);
	grid.SetColumnResize(FALSE);
	grid.SetBkColor(RGB(197, 197, 187));
	grid.SetTextBkColor(RGB(235, 235, 235));
	grid.SetColumnCount(nCols);
	grid.SetRowCount(nRows);

	for(int i = 0; i < grid.GetColumnCount(); i++)
	{
		grid.SetItemBkColour(0, i, RGB(128, 128, 128));
		grid.SetColumnWidth(i, 200);
	}

	grid.SetTextColor(RGB(255, 255, 255));

	for(int i = 0; i < grid.GetRowCount(); i++)
		grid.SetRowHeight(i, 20);
}

void CDlgViewer::OnBnClickedBtnOriginalImage()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	//if( m_Viewer.IsFitImage() )
	//{
	//	CRect rect;
	//	m_Viewer.GetClientRect(&rect);
	//	int w = m_Viewer.GetWidth();
	//	int h = m_Viewer.GetHeight();
	//	CPoint pt;
	//	pt.x = (w - rect.Width()) / 2;
	//	pt.y = (h - rect.Height()) / 2;
	//	m_Viewer.setResult(pt);
	//	m_Viewer.setLup(pt);
	//	m_Viewer.setLDown(pt);
	//}
	//m_Viewer.setOriginalImage();
	//m_Viewer.setNavi(true);

	m_Viewer.setOriginalImage();
	m_Viewer.SetNavMode(false);
	m_Viewer.HardDirtyRefresh();

	updateViewerFunc();
}

void CDlgViewer::OnBnClickedBtnFitImage()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	m_Viewer.OnFitImage();
	m_Viewer.SetNavMode(false);
	m_Viewer.SoftDirtyRefresh();

	updateViewerFunc();
}

void CDlgViewer::OnBnClickedBtnCenterLine()
{
	m_Viewer.ToggleDrawGuidingGraphics();
	m_Viewer.SoftDirtyRefresh();

	updateViewerFunc();
}

void CDlgViewer::OnBnClickedBtnDrawArea()
{
	if (!m_Viewer.GetModeDrawingFigure())
	{
		m_Viewer.SetRoiSelectionMode();
		m_Viewer.SetInteraction(true);
	}
	else
	{
		m_Viewer.InactiveInteraction(true);
	}

	m_Viewer.SoftDirtyRefresh();
	updateViewerFunc();
}

void CDlgViewer::InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}

void CDlgViewer::MainButtonInit(CButtonEx *pbutton, int nID, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, COLOR_BTN_BODY);	
	pbutton->SetColorBorder(255, COLOR_BTN_SIDE);
	pbutton->SetAlignTextCM();

	if( nID == -1 )
	{
		pbutton->SetSizeImage(5, 5, size, size);
		pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
	}
	else
	{
		CRect rect;
		int nOffset = 3;
		pbutton->GetClientRect(&rect);

		pbutton->SetSizeImage( nOffset, nOffset, rect.Width() - nOffset * 2, rect.Height() - nOffset * 2);
		pbutton->LoadImageFromResource(nID, TRUE);
	}
}

void CDlgViewer::updateViewerFunc()
{
	m_btnFunc[FUNC_CENTER_LINE].SetColorBkg(255, m_Viewer.GetEnableDrawCrossLine() ? COLOR_BTN_SELECT: COLOR_BTN_BODY);
	m_btnFunc[FUNC_FIT_IMAGE].SetColorBkg(255, m_Viewer.IsFitImage() ? COLOR_BTN_SELECT: COLOR_BTN_BODY);
	m_btnFunc[FUNC_ORIGIN_IMAGE].SetColorBkg(255, (m_Viewer.GetMagnification() == 1.) ? COLOR_BTN_SELECT: COLOR_BTN_BODY);
	m_btnFunc[FUNC_DRAW_AREA].SetColorBkg(255, m_Viewer.GetModeDrawingFigure() ? COLOR_BTN_SELECT : COLOR_BTN_BODY);
	m_btnFunc[FUNC_SHOW_OVERLAY].SetColorBkg(255, m_Viewer.GetDrawOverlay() ? COLOR_BTN_SELECT : COLOR_BTN_BODY);
}

void CDlgViewer::upDateControlUI(BOOL bEnable)
{
	if(bEnable)	SetTimer(TIMER_VIEWER_TOOL_USE_UPDATE, 100, NULL);
	else SetTimer(TIMER_VIEWER_TOOL_DISABLE_UPDATE, 100, NULL);
}

void CDlgViewer::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	KillTimer(nIDEvent);

	switch( nIDEvent ) {
	case TIMER_VIEWER_UI_CEHCK:
		updateViewerFunc();
		SetTimer(nIDEvent, 1000, NULL);
		break;
	case TIMER_VIEWER_TOOL_UPDATE:
		EnableToolButton(ID_BUTTON_RECORD, m_bEnableGrab);
		EnableToolButton(ID_BUTTON_LIVE, m_bEnableGrab);
		EnableToolButton(ID_BUTTON_STOP, !m_bEnableGrab);
		break;
	case 	TIMER_VIEWER_TOOL_USE_UPDATE:
		EnableToolButton(ID_BUTTON_RECORD, TRUE);
		EnableToolButton(ID_BUTTON_LIVE, TRUE);
		EnableToolButton(ID_BUTTON_STOP, TRUE);
		break;
	case TIMER_VIEWER_TOOL_DISABLE_UPDATE:
		EnableToolButton(ID_BUTTON_RECORD, FALSE);
		EnableToolButton(ID_BUTTON_LIVE, FALSE);
		EnableToolButton(ID_BUTTON_STOP, FALSE);
		break;
	}

	CDialogEx::OnTimer(nIDEvent);
}

void CDlgViewer::OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// 이 기능을 사용하려면 Windows Vista 이상이 있어야 합니다.
	// _WIN32_WINNT 기호는 0x0600보다 크거나 같아야 합니다.
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CDialogEx::OnMouseHWheel(nFlags, zDelta, pt);
}

void CDlgViewer::OnBnClickedBtnZoomIn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_Viewer.ZoomIn();
	updateViewerFunc();
}

void CDlgViewer::OnBnClickedBtnZoomOut()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_Viewer.ZoomOut();
	updateViewerFunc();
}

void CDlgViewer::OnBnClickedBtnDlgShowOverlay()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	//m_Viewer.SetDrawOverlay(!m_Viewer.GetDrawOverlay());
	//Invalidate();
	m_Viewer.ToggleDrawOverlay();
	updateViewerFunc();
	
}
void CDlgViewer::InitToolBar()
{
	if (!m_wndViewToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC | CBRS_GRIPPER | CBRS_BORDER_ANY)
		|| !m_wndViewToolBar.LoadToolBar(IDR_CONTROL_TOOLBAR))
	{
		EndDialog(IDCANCEL);
	}

	CRect rcClientStart{ 0,0,0,0 };
	CRect rcClientNow{ 0,0,0,0 };

	GetWindowRect(rcClientStart); //GetClientRect
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0, reposQuery, rcClientNow);

	CPoint ptOffset(rcClientNow.left - rcClientStart.left, rcClientNow.top - rcClientStart.top);
	CRect rcChild;
	CWnd* pwndChild = GetWindow(GW_CHILD);
	while (pwndChild)
	{
		pwndChild->GetWindowRect(rcChild);
		ScreenToClient(rcChild);
		rcChild.OffsetRect(ptOffset);
		pwndChild->MoveWindow(rcChild, FALSE);
		pwndChild = pwndChild->GetNextWindow();
	}
	CRect rcWindow;
	GetWindowRect(rcWindow);
	rcWindow.right += rcClientStart.Width() - rcClientNow.Width();
	rcWindow.bottom += rcClientStart.Height() - rcClientNow.Height();
	MoveWindow(rcWindow, FALSE);

	m_wndViewToolBar.SetButtonStyle(m_wndViewToolBar.CommandToIndex(ID_BUTTON_STOP), TBBS_DISABLED);

	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
}

BOOL CDlgViewer::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	LPNMHDR pnmh = (LPNMHDR)lParam;
	if (pnmh->hwndFrom == m_wndViewToolBar.m_hWnd)
	{
		LPNMTBCUSTOMDRAW lpNMCustomDraw = (LPNMTBCUSTOMDRAW)lParam;
		CRect rect;
		m_wndViewToolBar.GetClientRect(rect);
		FillRect(lpNMCustomDraw->nmcd.hdc, rect, (HBRUSH)GetStockObject(GRAY_BRUSH));

	}

	return CDialogEx::OnNotify(wParam, lParam, pResult);
}

void CDlgViewer::OnCommandFirst()
{
	::SendMessage(m_pParent->GetSafeHwnd(),WM_VIEW_CONTROL, MSG_FMV_TOOL_COMMAND, MAKELPARAM(MSG_DLGVIEW_TOOL_COMMAND_FIRST, m_nViewID)); // 첫번째 이미지 View
}

void CDlgViewer::OnCommandSecond()
{
	::SendMessage(m_pParent->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_FMV_TOOL_COMMAND, MAKELPARAM(MSG_DLGVIEW_TOOL_COMMAND_SECOND, m_nViewID)); // 두번째 이미지 View
}

void CDlgViewer::OnCommandThird()
{
	::SendMessage(m_pParent->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_FMV_TOOL_COMMAND, MAKELPARAM(MSG_DLGVIEW_TOOL_COMMAND_THIRD, m_nViewID)); // 세번째 이미지 View
}
void CDlgViewer::OnCommandFour()
{
	::SendMessage(m_pParent->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_FMV_TOOL_COMMAND, MAKELPARAM(MSG_DLGVIEW_TOOL_COMMAND_FOUR, m_nViewID)); // 네번째 이미지 View
}

void CDlgViewer::OnCommandLive()
{
	m_bEnableGrab = FALSE;
	::SendMessage(m_pParent->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_FMV_TOOL_COMMAND, MAKELPARAM(MSG_DLGVIEW_TOOL_COMMAND_LIVE, m_nViewID)); // 라이브 이미지 View

	SetTimer(TIMER_VIEWER_TOOL_UPDATE, 100, NULL);
}

void CDlgViewer::OnCommandRecord()
{
	m_bEnableGrab = FALSE;
	::SendMessage(m_pParent->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_FMV_TOOL_COMMAND, MAKELPARAM(MSG_DLGVIEW_TOOL_COMMAND_RECORD, m_nViewID)); // 녹화 시작
	SetTimer(TIMER_VIEWER_TOOL_UPDATE, 100, NULL);
}

void CDlgViewer::OnCommandStop()
{
	m_bEnableGrab = TRUE;
	::SendMessage(m_pParent->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_FMV_TOOL_COMMAND, MAKELPARAM(MSG_DLGVIEW_TOOL_COMMAND_STOP, m_nViewID)); // Stop VIEW
	SetTimer(TIMER_VIEWER_TOOL_UPDATE, 100, NULL);
}

LRESULT CDlgViewer::OnKickIdle(WPARAM wParam, LPARAM lParam) 
{
	CWnd::UpdateDialogControls(this, TRUE);

	return FALSE;
}


void CDlgViewer::EnableToolButton(int iCmd, BOOL bstatus)
{
	int  idx = m_wndViewToolBar.CommandToIndex(iCmd);
	if(idx == -1) return ; 

	UINT iStyle = m_wndViewToolBar.GetButtonStyle(idx);
	CMFCToolBarButton* toolbar = m_wndViewToolBar.GetButton(idx);

	if (bstatus)	m_wndViewToolBar.SetButtonStyle(idx, iStyle & !TBBS_DISABLED);
	else		m_wndViewToolBar.SetButtonStyle(idx, iStyle | TBBS_DISABLED);

	toolbar->EnableWindow(bstatus);
}

BOOL CDlgViewer::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;

	//return CDialogEx::OnEraseBkgnd(pDC);
}
