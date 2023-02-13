// DlgManualInputMark.cpp : implementation file
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "DlgManualInputMark1Cam.h"
#include "afxdialogex.h"
#include "LET_AlignClientDlg.h"

// CDlgManualInputMark1Cam dialog

IMPLEMENT_DYNAMIC(CDlgManualInputMark1Cam, CDialogEx)

CDlgManualInputMark1Cam::CDlgManualInputMark1Cam(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgManualInputMark1Cam::IDD, pParent)
{
	m_nResult = 0;
	job_id = 0;
	m_nTotalPosition = 0;

	m_dbManualPosX[0] = 0.0;
	m_dbManualPosY[0] = 0.0;
	m_dbManualPosX[1] = 0.0;
	m_dbManualPosY[1] = 0.0;
	m_dbManualPosX[2] = 0.0;
	m_dbManualPosY[2] = 0.0;
	m_dbManualPosX[3] = 0.0;
	m_dbManualPosY[3] = 0.0;

	m_bJudge[0] = FALSE;
	m_bJudge[1] = FALSE;
	m_bJudge[2] = FALSE;
	m_bJudge[3] = FALSE;

	m_nSelectPosition = 0;
	m_nShapeType    = 0;
	m_bTimerTogle = FALSE;

	m_AlignTargetAngle = 0.0;
}

CDlgManualInputMark1Cam::~CDlgManualInputMark1Cam()
{
	
}

void CDlgManualInputMark1Cam::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_MANUAL_INPUT_MARK_TITLE_1CAM, m_stt_ManualInputTitle);
	DDX_Control(pDX, IDC_STATIC_MANUAL_INPUT_VIEWER1_1CAM, m_ImageManualInput[0]);
	DDX_Control(pDX, IDC_STATIC_MANUAL_INPUT_VIEWER2_1CAM, m_ImageManualInput[1]);
	DDX_Control(pDX, IDC_STATIC_MANUAL_INPUT_VIEWER3_1CAM, m_ImageManualInput[2]);
	DDX_Control(pDX, IDC_STATIC_MANUAL_INPUT_VIEWER4_1CAM, m_ImageManualInput[3]);

	DDX_Control(pDX, IDC_BTN_SELECT_POSITION1, m_btnSelectPosition[0]);
	DDX_Control(pDX, IDC_BTN_SELECT_POSITION2, m_btnSelectPosition[1]);
	DDX_Control(pDX, IDC_BTN_SELECT_POSITION3, m_btnSelectPosition[2]);
	DDX_Control(pDX, IDC_BTN_SELECT_POSITION4, m_btnSelectPosition[3]);


	DDX_Control(pDX, IDC_BTN_INPUT_COMPLETE_1CAM, m_btnInputOK);
	DDX_Control(pDX, IDC_BTN_MANUAL_INPUT_NG_1CAM, m_btnInputNG);
	DDX_Control(pDX, IDC_STATIC_LEFT_DIR_1CAM, m_lblLeftRotate);
	DDX_Control(pDX, IDC_STATIC_RIGHT_DIR_1CAM, m_lblRightRotate);
	DDX_Control(pDX, IDC_STATIC_BTN_LEFT_1CAM, m_stt_BtnPos[0]);
	DDX_Control(pDX, IDC_STATIC_BTN_RIGHT_1CAM, m_stt_BtnPos[1]);
	DDX_Control(pDX, IDC_STATIC_BTN_TOP_1CAM, m_stt_BtnPos[2]);
	DDX_Control(pDX, IDC_STATIC_BTN_BOTTOM_1CAM, m_stt_BtnPos[3]);

	DDX_Control(pDX, IDC_BTN_DLG_ZOOM_IN_1CAM, m_btnZoomIn);
	DDX_Control(pDX, IDC_BTN_DLG_ZOOM_OUT_1CAM, m_btnZoomOut);
	DDX_Control(pDX, IDC_BTN_DLG_MOVE_1CAM, m_btnZoomMove);
}

BEGIN_MESSAGE_MAP(CDlgManualInputMark1Cam, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_INPUT_COMPLETE_1CAM, &CDlgManualInputMark1Cam::OnBnClickedBtnInputComplete)
	ON_BN_CLICKED(IDC_BTN_MANUAL_INPUT_NG_1CAM, &CDlgManualInputMark1Cam::OnBnClickedBtnManualInputNg)
	ON_BN_CLICKED(IDC_BTN_DLG_ZOOM_IN_1CAM, &CDlgManualInputMark1Cam::OnBnClickedBtnZoomIn)
	ON_BN_CLICKED(IDC_BTN_DLG_ZOOM_OUT_1CAM, &CDlgManualInputMark1Cam::OnBnClickedBtnZoomOut)
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_BTN_DLG_MOVE_1CAM, &CDlgManualInputMark1Cam::OnBnClickedBtnDlgMove)
	ON_BN_CLICKED(IDC_BTN_SELECT_POSITION1, &CDlgManualInputMark1Cam::OnBnClickedBtnSelectPosition1)
	ON_BN_CLICKED(IDC_BTN_SELECT_POSITION2, &CDlgManualInputMark1Cam::OnBnClickedBtnSelectPosition2)
	ON_BN_CLICKED(IDC_BTN_SELECT_POSITION3, &CDlgManualInputMark1Cam::OnBnClickedBtnSelectPosition3)
	ON_BN_CLICKED(IDC_BTN_SELECT_POSITION4, &CDlgManualInputMark1Cam::OnBnClickedBtnSelectPosition4)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

HBRUSH CDlgManualInputMark1Cam::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	return m_hbrBkg;
}

void CDlgManualInputMark1Cam::InitTitle(CLabelEx* pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}

void CDlgManualInputMark1Cam::MainButtonIconInit(CButtonEx* pbutton, int nID, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, COLOR_BTN_BODY);
	pbutton->SetColorBorder(255, COLOR_BTN_SIDE);
	pbutton->SetAlignTextCM();

	if (nID == -1)
	{
		pbutton->SetSizeImage(5, 5, size, size);
		pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
	}
	else
	{
		CRect rect;
		int nOffset = 3;
		pbutton->GetClientRect(&rect);

		pbutton->SetSizeImage(nOffset, nOffset, rect.Width() - nOffset * 2, rect.Height() - nOffset * 2);
		pbutton->LoadImageFromResource(nID, TRUE);
	}
}

void CDlgManualInputMark1Cam::MainButtonInit(CButtonEx* pbutton, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}

void CDlgManualInputMark1Cam::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (nIDEvent == TIMER_MANUAL_INPUT_TITLE)
	{
		m_bTimerTogle = !m_bTimerTogle;

		if (m_bTimerTogle)	m_stt_ManualInputTitle.SetColorBkg(255, RGB(192, 128, 255));
		else				m_stt_ManualInputTitle.SetColorBkg(255, RGB(255, 128, 128));

	}
	else if (nIDEvent == TIMER_MANUAL_MARK)
	{
		KillTimer(TIMER_MANUAL_MARK);

		theApp.m_pFrame->m_nManualInputMarkResult[job_id] = 2;
		this->ShowWindow(SW_HIDE);
	}

	CDialogEx::OnTimer(nIDEvent);
}

void CDlgManualInputMark1Cam::OnLButtonDown(UINT nFlags, CPoint point)
{
	int id = -1;
	CPoint st, ed;
	auto& view = m_ImageManualInput[m_nSelectPosition];
	view.GetBeginEnd(st, ed);

	for (int i = 0; i < 6; i++)
	{
		if (m_rcBtnPos[i].PtInRect(point))
		{
			id = i;
			break;
			}
			}

	if (id > -1)
			{
		m_OffsetFunctions[id](1, 1, 1, 1, st, ed, &view);
		view.SoftDirtyRefresh();
		calcManualPosition(m_nSelectPosition);
			}
	else
			{
		CRect r;
		m_ImageManualInput[m_nSelectPosition].GetClientRect(&r);
		if(r.PtInRect(point)) calcManualPosition(m_nSelectPosition);
			}

	CDialogEx::OnLButtonDown(nFlags, point);
}

BOOL CDlgManualInputMark1Cam::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR* nmHdr = (NMHDR*)lParam;

	if (nmHdr->code == WM_DRAWING_FINISH_MESSAGE)
	{
		calcManualPosition(m_nSelectPosition);
	}

	return CDialogEx::OnNotify(wParam, lParam, pResult);
}

BOOL CDlgManualInputMark1Cam::PreTranslateMessage(MSG* pMsg)
{
	auto& view = m_ImageManualInput[m_nSelectPosition];

	if (pMsg->message == WM_KEYDOWN && view.GetModeDrawingFigure())
	{
		CPoint st, ed;
		view.GetBeginEnd(st, ed);

		if (pMsg->wParam == VK_LEFT)
		{
			if (IsSHIFTpressed())     m_OffsetFunctions[0](1, 0, 0, 0, st, ed, &view);
			else if (IsCTRLpressed()) m_OffsetFunctions[1](1, 0, 0, 0, st, ed, &view);
			else m_OffsetFunctions[0](1, 0, 1, 0, st, ed, &view);
			view.SoftDirtyRefresh();
		}
		else if (pMsg->wParam == VK_RIGHT)
		{
			if (IsSHIFTpressed())     m_OffsetFunctions[1](0, 0, 1, 0, st, ed, &view);
			else if (IsCTRLpressed()) m_OffsetFunctions[1](0, 0, 1, 0, st, ed, &view);
			else                      m_OffsetFunctions[1](1, 0, 1, 0, st, ed, &view);
			view.SoftDirtyRefresh();
		}
		else if (pMsg->wParam == VK_UP)
		{
			if (IsSHIFTpressed())     m_OffsetFunctions[2](0, 1, 0, 0, st, ed, &view);
			else if (IsCTRLpressed()) m_OffsetFunctions[3](0, 1, 0, 0, st, ed, &view);
			else                      m_OffsetFunctions[3](0, 1, 0, 1, st, ed, &view);
			view.SoftDirtyRefresh();
		}
		else if (pMsg->wParam == VK_DOWN)
		{
			if (IsSHIFTpressed())     m_OffsetFunctions[3](0, 0, 0, 1, st, ed, &view);
			else if (IsCTRLpressed()) m_OffsetFunctions[2](0, 0, 0, 1, st, ed, &view);
			else                      m_OffsetFunctions[3](0, 1, 0, 1, st, ed, &view);
			view.SoftDirtyRefresh();
		}

		//CPoint ptStart = view->GetPointBegin(0);
		//CPoint ptEnd = view->GetPointEnd(0);
		//if (pMsg->wParam == VK_LEFT)
		//{
		//	if (IsSHIFTpressed()) view->SetSizeRectangle(0, CPoint(ptStart.x - 1, ptStart.y), ptEnd);
		//	else if (IsCTRLpressed())	 view->SetSizeRectangle(0, CPoint(ptStart.x + 1, ptStart.y), ptEnd);
		//	else view->SetSizeRectangle(0, CPoint(ptStart.x - 1, ptStart.y), CPoint(ptEnd.x - 1, ptEnd.y));
		//}
		//else if (pMsg->wParam == VK_RIGHT)
		//{
		//	if (IsSHIFTpressed()) view->SetSizeRectangle(0, ptStart, CPoint(ptEnd.x + 1, ptEnd.y));
		//	else if (IsCTRLpressed())	 view->SetSizeRectangle(0, ptStart, CPoint(ptEnd.x - 1, ptEnd.y));
		//	else view->SetSizeRectangle(0, CPoint(ptStart.x + 1, ptStart.y), CPoint(ptEnd.x + 1, ptEnd.y));
		//	view->RedrawWindow();
		//}
		//else if (pMsg->wParam == VK_UP)
		//{
		//	if (IsSHIFTpressed()) view->SetSizeRectangle(0, CPoint(ptStart.x, ptStart.y - 1), ptEnd);
		//	else if (IsCTRLpressed())	 view->SetSizeRectangle(0, CPoint(ptStart.x, ptStart.y + 1), ptEnd);
		//	else view->SetSizeRectangle(0, CPoint(ptStart.x, ptStart.y - 1), CPoint(ptEnd.x, ptEnd.y - 1));
		//}
		//else if (pMsg->wParam == VK_DOWN)
		//{
		//	if (IsSHIFTpressed()) view->SetSizeRectangle(0, ptStart, CPoint(ptEnd.x, ptEnd.y + 1));
		//	else if (IsCTRLpressed())	 view->SetSizeRectangle(0, ptStart, CPoint(ptEnd.x, ptEnd.y - 1));
		//	else view->SetSizeRectangle(0, CPoint(ptStart.x, ptStart.y + 1), CPoint(ptEnd.x, ptEnd.y + 1));
		//}
		//view.RedrawWindow();
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

BOOL CDlgManualInputMark1Cam::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);

	InitTitle(&m_stt_ManualInputTitle, "", 16.f, RGB(64, 192, 64));

	initManualInputViewer();

	MainButtonInit(&m_btnSelectPosition[0]);
	MainButtonInit(&m_btnSelectPosition[1]);
	MainButtonInit(&m_btnSelectPosition[2]);
	MainButtonInit(&m_btnSelectPosition[3]);
	MainButtonInit(&m_btnInputOK);
	MainButtonInit(&m_btnInputNG);
	MainButtonInit(&m_btnZoomMove);
	MainButtonIconInit(&m_btnZoomIn, IDB_PNG_ZOOM_IN);
	MainButtonIconInit(&m_btnZoomOut, IDB_PNG_ZOOM_OUT);

	dispSelectPosition();
	dispViewer();

	InitTitle(&m_stt_BtnPos[0], _T("  ←"), 24.f, RGB(64, 64, 64));
	InitTitle(&m_stt_BtnPos[1], _T("  →"), 24.f, RGB(64, 64, 64));
	InitTitle(&m_stt_BtnPos[2], _T("  ↑"), 24.f, RGB(64, 64, 64));
	InitTitle(&m_stt_BtnPos[3], _T("  ↓"), 24.f, RGB(64, 64, 64));
	InitTitle(&m_lblLeftRotate, _T(" LR "), 24.f, RGB(64, 64, 64));
	InitTitle(&m_lblRightRotate, _T(" RR "), 24.f, RGB(64, 64, 64));

	GetDlgItem(IDC_STATIC_BTN_LEFT_1CAM)->GetWindowRect(&m_rcBtnPos[0]);
	GetDlgItem(IDC_STATIC_BTN_RIGHT_1CAM)->GetWindowRect(&m_rcBtnPos[1]);
	GetDlgItem(IDC_STATIC_BTN_TOP_1CAM)->GetWindowRect(&m_rcBtnPos[2]);
	GetDlgItem(IDC_STATIC_BTN_BOTTOM_1CAM)->GetWindowRect(&m_rcBtnPos[3]);
	GetDlgItem(IDC_STATIC_LEFT_DIR_1CAM)->GetWindowRect(&m_rcBtnPos[4]);
	GetDlgItem(IDC_STATIC_RIGHT_DIR_1CAM)->GetWindowRect(&m_rcBtnPos[5]);

	ScreenToClient(&m_rcBtnPos[0]);
	ScreenToClient(&m_rcBtnPos[1]);
	ScreenToClient(&m_rcBtnPos[2]);
	ScreenToClient(&m_rcBtnPos[3]);
	ScreenToClient(&m_rcBtnPos[4]);
	ScreenToClient(&m_rcBtnPos[5]);

	for (int nPos = 0; nPos < 4; nPos++)
	{
		m_btnSelectPosition[nPos].ShowWindow(SW_HIDE);
		m_StrPositionName[nPos].Format("Position %d",nPos + 1);
	}

	m_AlignTargetAngle = 0;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

// CDlgManualInputMark1Cam message handlers
void CDlgManualInputMark1Cam::OnBnClickedBtnInputComplete()
{
	BOOL bInputComplete = TRUE;
	CString msg;

	for (int i = 0; i < m_nTotalPosition; i++)
	{
		if (m_bJudge[i] != TRUE)
		{
				bInputComplete = FALSE;
				msg.Format("Input the Position %d Mark!!", i + 1);
				AfxMessageBox(msg);
			}
		}

	if (bInputComplete != TRUE)		return;

	m_nResult = 1;
	m_bManualInputComplete = TRUE;
	theApp.m_pFrame->m_nManualInputMarkResult[job_id] = 1;

	ShowWindow(SW_HIDE);
}

void CDlgManualInputMark1Cam::OnBnClickedBtnZoomIn()
{
	m_ImageManualInput[m_nSelectPosition].ZoomIn();
}

void CDlgManualInputMark1Cam::OnBnClickedBtnZoomOut()
{
	m_ImageManualInput[m_nSelectPosition].ZoomOut();
}

void CDlgManualInputMark1Cam::OnBnClickedBtnDlgMove()
{
	auto& view = m_ImageManualInput[m_nSelectPosition];

	if (view.IsInteraction())
	{
		m_btnZoomMove.SetColorBkg(255, COLOR_BTN_BODY);
		view.SetInteraction(false);
	}
	else
	{
		view.SetInteraction(true);
		EnableMove();
	}
}

void CDlgManualInputMark1Cam::OnBnClickedBtnManualInputNg()
{
	m_nResult = 2;
	m_bManualInputComplete = TRUE;
	ShowWindow(SW_HIDE);
	theApp.m_pFrame->m_nManualInputMarkResult[job_id] = 2;
}

void CDlgManualInputMark1Cam::OnBnClickedBtnSelectPosition1()
{
	if (m_bJudge[0] == TRUE)
	{
		if(AfxMessageBox("Will you change the Postion 1?", MB_YESNO) != IDYES) return;
	}
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelectPosition = 0;
	m_AlignTargetAngle = 0;
	dispSelectPosition();
	dispViewer();
}

void CDlgManualInputMark1Cam::OnBnClickedBtnSelectPosition2()
{
	if (m_bJudge[1] == TRUE)
	{
		if (AfxMessageBox("Will you change the Postion 2?", MB_YESNO) != IDYES) return;
	}
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelectPosition = 1;
	m_AlignTargetAngle = 0;
	dispSelectPosition();
	dispViewer();
}

void CDlgManualInputMark1Cam::OnBnClickedBtnSelectPosition3()
{
	if (m_bJudge[2] == TRUE)
	{
		if (AfxMessageBox("Will you change the Postion 3?", MB_YESNO) != IDYES) return;
	}
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelectPosition = 2;
	m_AlignTargetAngle = 0;
	dispSelectPosition();
	dispViewer();
}

void CDlgManualInputMark1Cam::OnBnClickedBtnSelectPosition4()
{
	if (m_bJudge[3] == TRUE)
	{
		if (AfxMessageBox("Will you change the Postion 4?", MB_YESNO) != IDYES) return;
	}
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelectPosition = 3;
	m_AlignTargetAngle = 0;
	dispSelectPosition();
	dispViewer();
}

void CDlgManualInputMark1Cam::initializeParameter(int shape)
{
	m_dbManualPosX[0] = 0.0;
	m_dbManualPosY[0] = 0.0;
	m_dbManualPosX[1] = 0.0;
	m_dbManualPosY[1] = 0.0;
	m_dbManualPosX[2] = 0.0;
	m_dbManualPosY[2] = 0.0;
	m_dbManualPosX[3] = 0.0;
	m_dbManualPosY[3] = 0.0;

	theApp.m_pFrame->m_nManualInputMarkResult[job_id] = 0;
	m_bManualInputComplete = FALSE;
	m_AlignTargetAngle = 0;
	m_nShapeType = shape;

	int real_cam = 0;

	int posCount = theApp.m_pFrame->vt_job_info[job_id].num_of_position;	
	std::vector<int> camBuf = theApp.m_pFrame->vt_job_info[job_id].camera_index;

	real_cam = camBuf[0];

	BOOL bJudge = TRUE;
	for (int nPos = 0; nPos < m_nTotalPosition; nPos++)
	{
		m_bJudge[nPos] = theApp.m_pFrame->GetMatching(job_id).getFindInfo(0, nPos).GetFound() == FIND_OK ? TRUE : FALSE;
		m_ImageManualInput[nPos].ClearOverlayDC();
		if (theApp.m_pFrame->vt_job_info[job_id].algo_method == CLIENT_TYPE_FILM_INSP)	 m_ImageManualInput[nPos].OnLoadImageFromPtr(theApp.m_pFrame->getProcBuffer(real_cam, 1));
		else																			 m_ImageManualInput[nPos].OnLoadImageFromPtr(theApp.m_pFrame->getProcBuffer(real_cam, 0));
		m_btnSelectPosition[nPos].ShowWindow(SW_SHOW);
		if (bJudge && m_bJudge[nPos] == FALSE)
		{
			bJudge = FALSE;
			m_nSelectPosition = nPos;
		}
	}

	SetDlgItemText(IDC_BTN_SELECT_POSITION1, m_StrPositionName[0]);
	SetDlgItemText(IDC_BTN_SELECT_POSITION2, m_StrPositionName[1]);
	SetDlgItemText(IDC_BTN_SELECT_POSITION3, m_StrPositionName[2]);
	SetDlgItemText(IDC_BTN_SELECT_POSITION4, m_StrPositionName[3]);

	dispViewer();
	dispSelectPosition();
	m_btnInputOK.SetEnable(FALSE);

	drawMarkPostion(TRUE);
	EnableMove();

	SetTimer(TIMER_MANUAL_INPUT_TITLE, 500, NULL);
}

void CDlgManualInputMark1Cam::dispSelectPosition()
{
	for (int i = 0; i < 4; i++) m_btnSelectPosition[i].SetColorText(255, RGB(255, 255, 255));

	if (m_bJudge[0] == TRUE) m_btnSelectPosition[0].SetColorBkg(255, RGB(0, 255, 0));
	else					 m_btnSelectPosition[0].SetColorBkg(255, RGB(255, 0, 0));
	if (m_bJudge[1] == TRUE) m_btnSelectPosition[1].SetColorBkg(255, RGB(0, 255, 0));
	else					 m_btnSelectPosition[1].SetColorBkg(255, RGB(255, 0, 0));
	if (m_bJudge[2] == TRUE) m_btnSelectPosition[2].SetColorBkg(255, RGB(0, 255, 0));
	else					 m_btnSelectPosition[2].SetColorBkg(255, RGB(255, 0, 0));
	if (m_bJudge[3] == TRUE) m_btnSelectPosition[3].SetColorBkg(255, RGB(0, 255, 0));
	else					 m_btnSelectPosition[3].SetColorBkg(255, RGB(255, 0, 0));
	
	m_btnSelectPosition[m_nSelectPosition].SetColorBorder(255, RGB(128, 128, 255));
	m_btnSelectPosition[m_nSelectPosition].SetColorText(255, RGB(128, 128, 255));
}

void CDlgManualInputMark1Cam::initManualInputViewer()
{
#ifndef JOB_INFO
	int camCount = theApp.m_pFrame->m_pAlgorithmInfo.getAlgorithmCameraNum(m_nAlgoID);
	std::vector<int> camBuf = theApp.m_pFrame->m_pAlgorithmInfo.getAlgorithmCameraIndex(m_nAlgoID);
#else
	int camCount = theApp.m_pFrame->vt_job_info[job_id].num_of_camera;
	std::vector<int> camBuf = theApp.m_pFrame->vt_job_info[job_id].camera_index;
#endif
	camCount = camCount > 2 ? 2 : camCount;

	for (int nPos = 0; nPos < m_nTotalPosition; nPos++)
	{
		int real_cam = camBuf.at(0);
		int W = theApp.m_pFrame->m_stCamInfo[real_cam].w;
		int H = theApp.m_pFrame->m_stCamInfo[real_cam].h;

		m_ImageManualInput[nPos].InitControl(this);
		m_ImageManualInput[nPos].SetEnableDrawGuidingGraphics(true);
		m_ImageManualInput[nPos].OnInitWithCamera(W, H, 8);
		m_ImageManualInput[nPos].SetFitMode();

		m_ImageManualInput[nPos].SetHwnd(this->m_hWnd);
	}
}

void CDlgManualInputMark1Cam::initManualInputTracker(int shape, CPoint *st, CPoint *et)
{
	int camCount = MIN(2,theApp.m_pFrame->vt_job_info[job_id].num_of_camera);
	std::vector<int> camBuf = theApp.m_pFrame->vt_job_info[job_id].camera_index;

	m_nShapeType = shape;

	for (int nPos = 0; nPos < m_nTotalPosition; nPos++)
	{	
		switch (shape)
		{
		case 0:
			m_ImageManualInput[nPos].SetRoiSelectionMode();
		break;
		case 1:
			m_ImageManualInput[nPos].SetCaliperCircleMode(false);
		break;
		case 2:
			m_ImageManualInput[nPos].SetRoiSelectionEllipseMode();
			if(st!=NULL && et!=NULL)
				m_ImageManualInput[nPos].SetBeginEnd(*st, *et);
		break;
		}
		m_ImageManualInput[nPos].SetInteraction();
	}
}

void CDlgManualInputMark1Cam::calcManualPosition(int nPos)
{
	int posX, posY;

	int real_cam = theApp.m_pFrame->vt_job_info[job_id].camera_index[0];

	int width = theApp.m_pFrame->m_stCamInfo[real_cam].w;
	int height = theApp.m_pFrame->m_stCamInfo[real_cam].h;

	CPoint ptStart = m_ImageManualInput[nPos].GetPointBegin(0);
	CPoint ptEnd = m_ImageManualInput[nPos].GetPointEnd(0);
	CRect rectROI;
	rectROI.left = MIN(ptStart.x, ptEnd.x);
	rectROI.top = MIN(ptStart.y, ptEnd.y);
	rectROI.right = MAX(ptStart.x, ptEnd.x);
	rectROI.bottom = MAX(ptStart.y, ptEnd.y);

	posX = int((ptStart.x + ptEnd.x) / 2.0);
	posY = int((ptStart.y + ptEnd.y) / 2.0);

	int ncam = 0, npos = 0;
#ifndef JOB_INFO
	int camCount = theApp.m_pFrame->m_pAlgorithmInfo.getAlgorithmCameraNum(m_nAlgoID);
	std::vector<int> camBuf = theApp.m_pFrame->m_pAlgorithmInfo.getAlgorithmCameraIndex(m_nAlgoID);
#else
	int camCount = theApp.m_pFrame->vt_job_info[job_id].num_of_camera;
	std::vector<int> camBuf = theApp.m_pFrame->vt_job_info[job_id].camera_index;
#endif
	m_dbManualPosX[nPos] = posX;
	m_dbManualPosY[nPos] = posY;

	theApp.m_pFrame->GetMatching(job_id).getFindInfo(0, nPos).SetXPos(m_dbManualPosX[nPos]);
	theApp.m_pFrame->GetMatching(job_id).getFindInfo(0, nPos).SetYPos(m_dbManualPosY[nPos]);
	theApp.m_pFrame->GetMatching(job_id).getFindInfo(0, nPos).SetFound(FIND_OK);
	theApp.m_pFrame->GetMatching(job_id).getFindInfo(0, nPos).SetScore(100.0);

	//////// 20.02.27
	theApp.m_pFrame->GetMatching(job_id).m_lineHori[0][nPos].a = 0.0;
	theApp.m_pFrame->GetMatching(job_id).m_lineHori[0][nPos].b = m_dbManualPosY[nPos];
	theApp.m_pFrame->GetMatching(job_id).m_lineVert[0][nPos].a = 0.0;
	theApp.m_pFrame->GetMatching(job_id).m_lineVert[0][nPos].b = m_dbManualPosX[nPos];

	theApp.m_pFrame->GetMatching(job_id).m_ax[0][nPos][0] = 0.0;
	theApp.m_pFrame->GetMatching(job_id).m_ax[0][nPos][1] = 0.0;
	theApp.m_pFrame->GetMatching(job_id).m_bc[0][nPos][0] = m_dbManualPosY[nPos];
	theApp.m_pFrame->GetMatching(job_id).m_bc[0][nPos][1] = m_dbManualPosX[nPos];
	theApp.m_pFrame->GetMatching(job_id).getFindInfo(0, nPos).SetLineX(2, m_dbManualPosX[nPos]);
	theApp.m_pFrame->GetMatching(job_id).getFindInfo(0, nPos).SetLineX(3, m_dbManualPosX[nPos]);
	//////////////

	m_bJudge[nPos] = TRUE;

	fnBtnDisplay();	// 20.03.21
	drawMarkPostion(TRUE);
	m_ImageManualInput[nPos].RedrawWindow();
}

void CDlgManualInputMark1Cam::dispViewer()
{
	for (int i = 0; i < 4; i++)
	{
		if( i == m_nSelectPosition ) m_ImageManualInput[i].ShowWindow(SW_SHOW);
		else						 m_ImageManualInput[i].ShowWindow(SW_HIDE);
	}

	auto& view = m_ImageManualInput[m_nSelectPosition];
	if (view.GetModeDrawingFigure())	m_btnZoomMove.SetColorBkg(255, COLOR_BTN_SELECT);
	else											m_btnZoomMove.SetColorBkg(255, COLOR_BTN_BODY);
}

void CDlgManualInputMark1Cam::drawMarkPostion(BOOL bErase)
{	
	//double dbPosX, dbPosY, dbScore;

	int result = 0;

	if (bErase)
	{
		m_ImageManualInput[0].ClearOverlayDC();
		m_ImageManualInput[1].ClearOverlayDC();
		m_ImageManualInput[2].ClearOverlayDC();
		m_ImageManualInput[3].ClearOverlayDC();
	}
	return;
//	int real_cam, npos;
//	for (int nViewer = 0; nViewer < 2; nViewer++)
//	{
//		CString str;
//		CBrush *oldBr;
//		int textX, textY;
//
//		CRect rectROI;
//		CPen penOK(PS_SOLID, 7, RGB(0, 255, 0)), penNG(PS_SOLID, 7, RGB(255, 0, 0)),
//			penMatch(PS_SOLID, 7, RGB(255, 255, 0)), *oldpen;
//		CPen penROI(PS_DOT, 7, RGB(255, 255, 255));
//
//		CDC *pDC = m_ImageManualInputMini[nViewer].getOverlayDC();
//		CDC *pDCLarge = m_ImageManualInput[nViewer].getOverlayDC();
//
//#ifndef JOB_INFO
//		int camCount = theApp.m_pFrame->m_pAlgorithmInfo.getAlgorithmCameraNum(m_nAlgoID);
//		std::vector<int> camBuf = theApp.m_pFrame->m_pAlgorithmInfo.getAlgorithmCameraIndex(m_nAlgoID);
//#else
//		int camCount = theApp.m_pFrame->vt_job_info[job_id].num_of_camera;
//		std::vector<int> camBuf = theApp.m_pFrame->vt_job_info[job_id].camera_index;
//#endif
//
//		if (camCount == 1)
//		{
//			real_cam = camBuf.at(0);
//			npos = nViewer;
//		}
//		else if (camCount == 2)
//		{
//			real_cam = camBuf.at(nViewer);  npos = 0;
//		}
//		else
//		{
//		}
//
//		int W = theApp.m_pFrame->m_stCamInfo[real_cam].w;
//		int H = theApp.m_pFrame->m_stCamInfo[real_cam].h;
//
//		result = theApp.m_pFrame->GetMatching(job_id).getFindInfo(nViewer, npos).GetFound();
//
//		int old_mode = pDC->SetBkMode(TRANSPARENT);
//		oldpen = pDC->SelectObject(&penOK);
//		pDCLarge->SelectObject(&penOK);
//		oldBr = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
//
//		if (result == FIND_OK)			pDC->SelectObject(&penOK);
//		else if (result == FIND_MATCH)	pDC->SelectObject(&penMatch);
//		else							pDC->SelectObject(&penNG);
//	
//		dbPosX = theApp.m_pFrame->GetMatching(job_id).getFindInfo(nViewer, npos).GetXPos();
//		dbPosY = theApp.m_pFrame->GetMatching(job_id).getFindInfo(nViewer, npos).GetYPos();
//		dbScore = theApp.m_pFrame->GetMatching(job_id).getFindInfo(nViewer, npos).getScore();
//		rectROI = theApp.m_pFrame->GetMatching(job_id).getSearchROI(nViewer, npos);
//
//		///// Draw Pattern Find Pos
//		pDC->MoveTo((int)(dbPosX - 20), (int)(dbPosY));
//		pDC->LineTo((int)(dbPosX + 20), (int)(dbPosY));
//		pDC->MoveTo((int)(dbPosX), (int)(dbPosY - 20));
//		pDC->LineTo((int)(dbPosX), (int)(dbPosY + 20));
//
//		pDCLarge->MoveTo((int)(dbPosX - 20), (int)(dbPosY));
//		pDCLarge->LineTo((int)(dbPosX + 20), (int)(dbPosY));
//		pDCLarge->MoveTo((int)(dbPosX), (int)(dbPosY - 20));
//		pDCLarge->LineTo((int)(dbPosX), (int)(dbPosY + 20));
//
//		///// Draw Search ROI
//		pDC->SelectObject(&penROI);
//		pDC->Rectangle(&rectROI);
//
//		// Draw Find Pattern Info.
//		if ((int)dbPosX < (W / 2))			textX = (int)dbPosX + 20;
//		else			textX = (int)dbPosX - 140;
//
//		if ((int)dbPosY < (W / 2))			textY = (int)dbPosY + 40;
//		else			textY = (int)dbPosY - 60;
//
//		str.Format("[ %.1f , %.1f ] %.1f%%", dbPosX, dbPosY, dbScore);
//
//		if (result == FIND_ERR)			pDC->SetTextColor(RGB(255, 0, 0));
//		else if (result == FIND_OK) 		pDC->SetTextColor(RGB(0, 255, 0));
//		else								pDC->SetTextColor(RGB(255, 255, 0));
//
//		pDC->TextOutA(textX, textY, str);
//
//		pDC->SetBkMode(old_mode);
//		pDC->SelectObject(oldpen);
//		pDC->SelectObject(oldBr);
//	}
}

void CDlgManualInputMark1Cam::drawMarkGuidePostion(CViewerEx *viewer,BOOL bErase)
{
	//if (bErase)
	//{
	//	viewer->ClearOverlayDC();
	//}

	//CDC *pDC = viewer->getOverlayDC();

	//CPen ScalePen, ScalePenAngle, *pOldPen;
	//ScalePen.CreatePen(PS_DOT, 1, RGB(0, 255, 0));
	//ScalePenAngle.CreatePen(PS_DOT, 1, RGB(255, 255, 0));

	//pOldPen = (CPen*)pDC->SelectObject(&ScalePenAngle);

	//CPoint ptStart = viewer->GetPointBegin(0);
	//CPoint ptEnd = viewer->GetPointEnd(0);

	//int W = viewer->GetWidth();
	//int H = viewer->GetHeight();

	//CPoint m_ptPatternPos = CPoint((ptStart.x + ptEnd.x) / 2, (ptStart.y + ptEnd.y) / 2);

	//CPoint r = viewer->RotatePt(m_AlignTargetAngle, CPoint(0, m_ptPatternPos.y), m_ptPatternPos);

	//pDC->MoveTo(0, r.y);
	//r = viewer->RotatePt(m_AlignTargetAngle, CPoint(W, H), m_ptPatternPos);
	//pDC->LineTo(W, r.y);

	//r = viewer->RotatePt(m_AlignTargetAngle, CPoint(m_ptPatternPos.x, 0), m_ptPatternPos);
	//pDC->MoveTo(r.x, 0);
	//r = viewer->RotatePt(m_AlignTargetAngle, CPoint(m_ptPatternPos.x, H), m_ptPatternPos);
	//pDC->LineTo(r.x, H);

	//pDC->SelectObject(pOldPen);

	//ScalePen.DeleteObject();
	//ScalePenAngle.DeleteObject();

	//viewer->Invalidate();
}

void CDlgManualInputMark1Cam::fnBtnDisplay()
{
	for (int nPos = 0; nPos < m_nTotalPosition; nPos++)
	{
		if (m_bJudge[nPos] == FALSE)
		{
			m_btnInputOK.SetEnable(FALSE);
			return;
		}
	}
	m_btnInputOK.SetEnable(TRUE);
}


void CDlgManualInputMark1Cam::OnClose()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	OnBnClickedBtnManualInputNg();
	CDialogEx::OnClose();
}

void CDlgManualInputMark1Cam::EnableMove()
{
	auto& view = m_ImageManualInput[m_nSelectPosition];

	m_btnZoomMove.SetColorBkg(255, COLOR_BTN_SELECT);
	switch (m_nShapeType)
	{
	case 0:
		view.SetRoiSelectionMode();
		break;
	case 1:
		view.SetCaliperCircleMode(false);
		break;
	case 2:
		view.SetRoiSelectionEllipseMode();
		break;
	}

	view.SetInteraction();
}
