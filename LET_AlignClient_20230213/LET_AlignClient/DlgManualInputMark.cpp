// DlgManualInputMark.cpp : implementation file
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "DlgManualInputMark.h"
#include "afxdialogex.h"
#include "LET_AlignClientDlg.h"

// CDlgManualInputMark dialog

IMPLEMENT_DYNAMIC(CDlgManualInputMark, CDialogEx)

CDlgManualInputMark::CDlgManualInputMark(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgManualInputMark::IDD, pParent)
{
	m_nResult = 0;
	job_id = 0;

	m_dbManualPosX[0] = 0.0;
	m_dbManualPosY[0] = 0.0;
	m_dbManualPosX[1] = 0.0;
	m_dbManualPosY[1] = 0.0;

	m_bJudge[0] = FALSE;
	m_bJudge[1] = FALSE;

	m_nSelectCamera = 0;
	m_nShapeType    = 0;
	m_bTimerTogle = FALSE;

	m_AlignTargetAngle = 0.0;
}

CDlgManualInputMark::~CDlgManualInputMark()
{
	
}

void CDlgManualInputMark::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_MANUAL_INPUT_MARK_TITLE, m_stt_ManualInputTitle);
	DDX_Control(pDX, IDC_STATIC_MANUAL_INPUT_VIEWER, m_ImageManualInput[0]);
	DDX_Control(pDX, IDC_STATIC_MANUAL_INPUT_VIEWER2, m_ImageManualInput[1]);

	DDX_Control(pDX, IDC_STATIC_MANUAL_MINI_VIEWER1, m_ImageManualInputMini[0]);
	DDX_Control(pDX, IDC_STATIC_MANUAL_MINI_VIEWER2, m_ImageManualInputMini[1]);	

	DDX_Control(pDX, IDC_STATIC_SELECT_MINI_VIEWER_TITLE1, m_stt_ManualInputCameraTitle[0]);
	DDX_Control(pDX, IDC_STATIC_SELECT_MINI_VIEWER_TITLE2, m_stt_ManualInputCameraTitle[1]);


	DDX_Control(pDX, IDC_BTN_SELECT_CAM1, m_btnSelectCamera[0]);
	DDX_Control(pDX, IDC_BTN_SELECT_CAM2, m_btnSelectCamera[1]);

	DDX_Control(pDX, IDC_BTN_INPUT_COMPLETE, m_btnInputOK);
	DDX_Control(pDX, IDC_BTN_MANUAL_INPUT_NG, m_btnInputNG);	
	DDX_Control(pDX, IDC_STATIC_LEFT_DIR, m_lblLeftRotate);
	DDX_Control(pDX, IDC_STATIC_RIGHT_DIR, m_lblRightRotate);
	DDX_Control(pDX, IDC_STATIC_BTN_LEFT, m_stt_BtnPos[0]);
	DDX_Control(pDX, IDC_STATIC_BTN_RIGHT2, m_stt_BtnPos[1]);
	DDX_Control(pDX, IDC_STATIC_BTN_TOP2, m_stt_BtnPos[2]);
	DDX_Control(pDX, IDC_STATIC_BTN_BOTTOM2, m_stt_BtnPos[3]);

	DDX_Control(pDX, IDC_BTN_DLG_ZOOM_IN, m_btnZoomIn);
	DDX_Control(pDX, IDC_BTN_DLG_ZOOM_OUT, m_btnZoomOut);
	DDX_Control(pDX, IDC_BTN_DLG_MOVE, m_btnZoomMove);
}


BEGIN_MESSAGE_MAP(CDlgManualInputMark, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_INPUT_COMPLETE, &CDlgManualInputMark::OnBnClickedBtnInputComplete)
	ON_BN_CLICKED(IDC_BTN_MANUAL_INPUT_NG, &CDlgManualInputMark::OnBnClickedBtnManualInputNg)
	ON_BN_CLICKED(IDC_BTN_SELECT_CAM1, &CDlgManualInputMark::OnBnClickedBtnSelectCam1)
	ON_BN_CLICKED(IDC_BTN_SELECT_CAM2, &CDlgManualInputMark::OnBnClickedBtnSelectCam2)
	ON_BN_CLICKED(IDC_BTN_DLG_ZOOM_IN, &CDlgManualInputMark::OnBnClickedBtnZoomIn)
	ON_BN_CLICKED(IDC_BTN_DLG_ZOOM_OUT, &CDlgManualInputMark::OnBnClickedBtnZoomOut)
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_BTN_DLG_MOVE, &CDlgManualInputMark::OnBnClickedBtnDlgMove)
END_MESSAGE_MAP()


// CDlgManualInputMark message handlers
void CDlgManualInputMark::OnBnClickedBtnInputComplete()
{
	BOOL bInputComplete = TRUE;

	if (m_bJudge[0] != TRUE)
	{
			bInputComplete = FALSE;
			AfxMessageBox("Input the Left Mark!!");		
	}

	if (m_bJudge[1] != TRUE)
	{
			bInputComplete = FALSE;
			AfxMessageBox("Input the Right Mark!!");
	}

	if (bInputComplete != TRUE)
		return;


	m_nResult = 1;
	m_bManualInputComplete = TRUE;
	theApp.m_pFrame->m_nManualInputMarkResult[job_id] = 1;

	ShowWindow(SW_HIDE);
}

void CDlgManualInputMark::initializeParameter(int shape)
{
	m_dbManualPosX[0] = 0.0;
	m_dbManualPosY[0] = 0.0;
	m_dbManualPosX[1] = 0.0;
	m_dbManualPosY[1] = 0.0;

	theApp.m_pFrame->m_nManualInputMarkResult[job_id] = 0;
	m_bManualInputComplete = FALSE;
	m_AlignTargetAngle = 0;
	m_nShapeType = shape;

	int ncam = 0, npos = 0;
	int ncam2 = 0, npos2 = 0;
	int real_cam = 0, real_cam2;
#ifndef JOB_INFO
	int camCount = theApp.m_pFrame->m_pAlgorithmInfo.getAlgorithmCameraNum(m_nAlgoID);
	std::vector<int> camBuf = theApp.m_pFrame->m_pAlgorithmInfo.getAlgorithmCameraIndex(m_nAlgoID);
	if (camCount == 1)
	{
		ncam = camBuf.at(0), npos = 0;
		ncam2 = camBuf.at(0), npos2 = 1;
	}
	else if (camCount == 2)
	{
		ncam = camBuf.at(0), npos = 0;
		ncam2 = camBuf.at(1), npos2 = 0;
	}
	else
	{
	}
#else
	int camCount = theApp.m_pFrame->vt_job_info[job_id].num_of_camera;	
	std::vector<int> camBuf = theApp.m_pFrame->vt_job_info[job_id].camera_index;
	if (camCount == 1)
	{
		ncam = 0, npos = 0;
		ncam2 = 0, npos2 = 1;

		real_cam = camBuf[ncam];
		real_cam2 = camBuf[ncam2];
	}
	else if (camCount == 2)
	{
		ncam = 0, npos = 0;
		ncam2 = 1, npos2 = 0;

		real_cam = camBuf[ncam];
		real_cam2 = camBuf[ncam2];
	}
	else
	{
		ncam = 0, npos = 0;
		ncam2 = 1, npos2 = 0;

		real_cam = camBuf[ncam];
		real_cam2 = camBuf[ncam2];
	}
#endif

	m_bJudge[0] = theApp.m_pFrame->GetMatching(job_id).getFindInfo(ncam, npos).GetFound() == FIND_OK ? TRUE : FALSE;
	m_bJudge[1] = theApp.m_pFrame->GetMatching(job_id).getFindInfo(ncam2, npos2).GetFound() == FIND_OK ? TRUE : FALSE;

	//m_ImageManualInput[0].ClearOverlayDC();
	//m_ImageManualInput[1].ClearOverlayDC();
	
	m_ImageManualInput[0].OnLoadImageFromPtr(theApp.m_pFrame->getSrcBuffer(real_cam));
	m_ImageManualInput[1].OnLoadImageFromPtr(theApp.m_pFrame->getSrcBuffer(real_cam2));

	m_ImageManualInputMini[0].OnLoadImageFromPtr(theApp.m_pFrame->getSrcBuffer(real_cam));
	m_ImageManualInputMini[1].OnLoadImageFromPtr(theApp.m_pFrame->getSrcBuffer(real_cam2));

	if (m_bJudge[0] == FALSE)	m_nSelectCamera = 0;
	else						m_nSelectCamera = 1;

	dispCameraTitle(0, m_bJudge[0]);
	dispCameraTitle(1, m_bJudge[1]);

	dispViewer();
	dispSelectCamera();
	fnBtnDisplay(m_bJudge[0], m_bJudge[1]);	// 20.03.21

	drawMarkPostion(TRUE);
	OnBnClickedBtnDlgMove();
}

void CDlgManualInputMark::dispCameraTitle(int nCam, BOOL bOK)
{
	CString strHeader,msg;
	if( nCam == 0 )	strHeader = "LEFT";
	else			strHeader = "RIGHT";

	if( bOK )
	{
		msg.Format("%s CAMERA - OK", strHeader);
		m_stt_ManualInputCameraTitle[nCam].SetText(msg);
		m_stt_ManualInputCameraTitle[nCam].SetColorBkg(255, RGB(0, 255, 0));
		m_bJudge[nCam] = TRUE;
	}
	else
	{
		msg.Format("%s CAMERA - NG", strHeader);
		m_stt_ManualInputCameraTitle[nCam].SetText(msg);
		m_stt_ManualInputCameraTitle[nCam].SetColorBkg(255,RGB(255, 0, 0));
	}

	m_stt_ManualInputCameraTitle[nCam].Invalidate();
}

void CDlgManualInputMark::InitTitle(CLabelEx *pTitle,CString str,float size,COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}

void CDlgManualInputMark::MainButtonIconInit(CButtonEx* pbutton, int nID, int size)
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

void CDlgManualInputMark::MainButtonInit(CButtonEx *pbutton,int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}

HBRUSH CDlgManualInputMark::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{	

	return m_hbrBkg;
}

BOOL CDlgManualInputMark::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);

	InitTitle( &m_stt_ManualInputTitle, "", 16.f, RGB(64, 192, 64));

	initManualInputViewer();

	MainButtonInit(&m_btnSelectCamera[0]);
	MainButtonInit(&m_btnSelectCamera[1]);
	MainButtonInit(&m_btnInputOK);
	MainButtonInit(&m_btnInputNG);
	MainButtonInit(&m_btnZoomMove);
	MainButtonIconInit(&m_btnZoomIn, IDB_PNG_ZOOM_IN);
	MainButtonIconInit(&m_btnZoomOut, IDB_PNG_ZOOM_OUT);
	
	dispSelectCamera();
	dispViewer();

	InitTitle( &m_stt_BtnPos[0], _T("  ←"), 24.f, RGB( 64, 64, 64));
	InitTitle( &m_stt_BtnPos[1], _T("  →"), 24.f, RGB( 64, 64, 64));
	InitTitle( &m_stt_BtnPos[2], _T("  ↑"), 24.f, RGB( 64, 64, 64));
	InitTitle( &m_stt_BtnPos[3], _T("  ↓"), 24.f, RGB( 64, 64, 64));
	InitTitle( &m_lblLeftRotate, _T(" LR "), 24.f, RGB( 64, 64, 64));
	InitTitle( &m_lblRightRotate, _T(" RR "), 24.f, RGB( 64, 64, 64));
	
	GetDlgItem(IDC_STATIC_BTN_LEFT)->GetWindowRect(&m_rcBtnPos[0]);
	GetDlgItem(IDC_STATIC_BTN_RIGHT2)->GetWindowRect(&m_rcBtnPos[1]);
	GetDlgItem(IDC_STATIC_BTN_TOP2)->GetWindowRect(&m_rcBtnPos[2]);
	GetDlgItem(IDC_STATIC_BTN_BOTTOM2)->GetWindowRect(&m_rcBtnPos[3]);
	GetDlgItem(IDC_STATIC_LEFT_DIR)->GetWindowRect(&m_rcBtnPos[4]);
	GetDlgItem(IDC_STATIC_RIGHT_DIR)->GetWindowRect(&m_rcBtnPos[5]);

	ScreenToClient(&m_rcBtnPos[0]);
	ScreenToClient(&m_rcBtnPos[1]);
	ScreenToClient(&m_rcBtnPos[2]);
	ScreenToClient(&m_rcBtnPos[3]);
	ScreenToClient(&m_rcBtnPos[4]);
	ScreenToClient(&m_rcBtnPos[5]);

	SetTimer(TIMER_MANUAL_INPUT_TITLE, 500, NULL);

	m_AlignTargetAngle = 0;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgManualInputMark::dispSelectCamera()
{
	if(m_bJudge[0])	m_btnSelectCamera[0].SetColorBkg(255, RGB(0, 255, 0));
	else m_btnSelectCamera[0].SetColorBkg(255, RGB(255, 0, 0));
	m_btnSelectCamera[0].SetColorText(255, RGB(255, 255, 255));

	if(m_bJudge[1])	m_btnSelectCamera[1].SetColorBkg(255, RGB(0, 255, 0));
	else m_btnSelectCamera[1].SetColorBkg(255, RGB(255, 0, 0));
	m_btnSelectCamera[1].SetColorText(255, RGB(255, 255, 255));

	m_btnSelectCamera[m_nSelectCamera].SetColorBorder(255, RGB(128, 128, 255));
	m_btnSelectCamera[m_nSelectCamera].SetColorText(255, RGB(128, 128, 255));
}


void CDlgManualInputMark::initManualInputViewer()
{
#ifndef JOB_INFO
	int camCount = theApp.m_pFrame->m_pAlgorithmInfo.getAlgorithmCameraNum(m_nAlgoID);
	std::vector<int> camBuf = theApp.m_pFrame->m_pAlgorithmInfo.getAlgorithmCameraIndex(m_nAlgoID);
#else
	int camCount = theApp.m_pFrame->vt_job_info[job_id].num_of_camera;
	std::vector<int> camBuf = theApp.m_pFrame->vt_job_info[job_id].camera_index;
#endif
	camCount = camCount > 2 ? 2 : camCount;

	for (int Cam = 0; Cam < camCount; Cam++)
	{
		int real_cam = camBuf.at(Cam);
		int W = theApp.m_pFrame->m_stCamInfo[real_cam].w;
		int H = theApp.m_pFrame->m_stCamInfo[real_cam].h;

		m_ImageManualInput[Cam].InitControl(this);
		m_ImageManualInput[Cam].SetEnableDrawGuidingGraphics(true);
		m_ImageManualInput[Cam].OnInitWithCamera(W, H, 8);
		m_ImageManualInput[Cam].SetFitMode();

		//m_ImageManualInput[Cam].SetEnableDrawAllFigures(true);
		//m_ImageManualInput[Cam].SetEnableDrawFigure(0, true);
		//m_ImageManualInput[Cam].SetEnableFigureRectangle(0);
		//m_ImageManualInput[Cam].SetEnableModifyFigure(0, true);	//2017.10.13 Draw 영역 크기 조절
		m_ImageManualInput[Cam].SetHwnd(this->m_hWnd);
		//m_ImageManualInput[Cam].SetModeDrawingFigure(true);
		//m_ImageManualInput[Cam].SetEnableModifyFigure(0, true);

		m_ImageManualInputMini[Cam].InitControl(this);
		m_ImageManualInputMini[Cam].SetEnableDrawGuidingGraphics(true);
		m_ImageManualInputMini[Cam].OnInitWithCamera(W, H, 8);
		m_ImageManualInputMini[Cam].SetFitMode();
		//m_ImageManualInputMini[Cam].SetEnableDrawFigure(0, true);
		//m_ImageManualInputMini[Cam].SetEnableFigureRectangle(0);
		//m_ImageManualInputMini[Cam].SetEnableModifyFigure(0, true);	//2017.10.13 Draw 영역 크기 조절
		m_ImageManualInputMini[Cam].SetHwnd(this->m_hWnd);
	}
}

void CDlgManualInputMark::initManualInputTracker(int shape, CPoint *st, CPoint *et)
{
	int camCount = MIN(2,theApp.m_pFrame->vt_job_info[job_id].num_of_camera);
	std::vector<int> camBuf = theApp.m_pFrame->vt_job_info[job_id].camera_index;

	m_nShapeType = shape;

	for (int Cam = 0; Cam < camCount; Cam++)
	{	
		switch (shape)
		{
		case 0:
			m_ImageManualInput[Cam].SetRoiSelectionMode();
		break;
		case 1:
			m_ImageManualInput[Cam].SetCaliperCircleMode(false);
		break;
		case 2:
			m_ImageManualInput[Cam].SetRoiSelectionEllipseMode();
			if(st!=NULL && et!=NULL)
				m_ImageManualInput[Cam].SetBeginEnd(*st, *et);
		break;
		}
		m_ImageManualInput[Cam].SetInteraction();
	}
}

void CDlgManualInputMark::calcManualPosition(int nViewer)
{
	int posX, posY;

	int real_cam = theApp.m_pFrame->vt_job_info[job_id].camera_index[nViewer];

	int width = theApp.m_pFrame->m_stCamInfo[real_cam].w;
	int height = theApp.m_pFrame->m_stCamInfo[real_cam].h;

	CPoint ptStart = m_ImageManualInput[nViewer].GetPointBegin(0);
	CPoint ptEnd = m_ImageManualInput[nViewer].GetPointEnd(0);
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

	if (camCount == 1)
	{
		ncam = camBuf.at(0);
		npos = nViewer;
	}
	else if (camCount == 2)
	{
		ncam = camBuf.at(nViewer);  npos = 0;
	}
	else
	{
	}

	m_dbManualPosX[nViewer] = posX;
	m_dbManualPosY[nViewer] = posY;

	theApp.m_pFrame->GetMatching(job_id).getFindInfo(nViewer, npos).SetXPos(m_dbManualPosX[nViewer]);
	theApp.m_pFrame->GetMatching(job_id).getFindInfo(nViewer, npos).SetYPos(m_dbManualPosY[nViewer]);
	theApp.m_pFrame->GetMatching(job_id).getFindInfo(nViewer, npos).SetFound(FIND_OK);
	theApp.m_pFrame->GetMatching(job_id).getFindInfo(nViewer, npos).SetScore(100.0);

	//////// 20.02.27
	theApp.m_pFrame->GetMatching(job_id).m_lineHori[nViewer][npos].a = 0.0;
	theApp.m_pFrame->GetMatching(job_id).m_lineHori[nViewer][npos].b = m_dbManualPosY[nViewer];
	theApp.m_pFrame->GetMatching(job_id).m_lineVert[nViewer][npos].a = 0.0;
	theApp.m_pFrame->GetMatching(job_id).m_lineVert[nViewer][npos].b = m_dbManualPosX[nViewer];

	theApp.m_pFrame->GetMatching(job_id).m_ax[nViewer][npos][0] = 0.0;
	theApp.m_pFrame->GetMatching(job_id).m_ax[nViewer][npos][1] = 0.0;
	theApp.m_pFrame->GetMatching(job_id).m_bc[nViewer][npos][0] = m_dbManualPosY[nViewer];
	theApp.m_pFrame->GetMatching(job_id).m_bc[nViewer][npos][1] = m_dbManualPosX[nViewer];
	theApp.m_pFrame->GetMatching(job_id).getFindInfo(nViewer, npos).SetLineX(2, m_dbManualPosX[nViewer]);
	theApp.m_pFrame->GetMatching(job_id).getFindInfo(nViewer, npos).SetLineX(3, m_dbManualPosX[nViewer]);
	//////////////

	dispCameraTitle(nViewer, TRUE);
	fnBtnDisplay(m_bJudge[0], m_bJudge[1]);	// 20.03.21
	drawMarkPostion(TRUE);
	m_ImageManualInput[nViewer].RedrawWindow();
}


void CDlgManualInputMark::OnBnClickedBtnManualInputNg()
{
	m_nResult = 2;
	m_bManualInputComplete = TRUE;
	ShowWindow(SW_HIDE);
	theApp.m_pFrame->m_nManualInputMarkResult[job_id] = 2;
}


void CDlgManualInputMark::OnBnClickedBtnSelectCam1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelectCamera = 0;
	m_AlignTargetAngle = 0;
	dispSelectCamera();
	dispViewer();
}


void CDlgManualInputMark::OnBnClickedBtnSelectCam2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelectCamera = 1;
	m_AlignTargetAngle = 0;
	dispSelectCamera();
	dispViewer();
}

void CDlgManualInputMark::dispViewer()
{
	m_ImageManualInput[m_nSelectCamera].ShowWindow(SW_SHOW);
	m_ImageManualInput[!m_nSelectCamera].ShowWindow(SW_HIDE);
}

void CDlgManualInputMark::drawMarkPostion(BOOL bErase)
{	
	double dbPosX, dbPosY, dbScore;

	int result = 0;

	if (bErase)
	{
		m_ImageManualInputMini[0].ClearOverlayDC();
		m_ImageManualInputMini[1].ClearOverlayDC();
		m_ImageManualInput[0].ClearOverlayDC();
		m_ImageManualInput[1].ClearOverlayDC();
	}
	return;
	int real_cam, npos;
	for (int nViewer = 0; nViewer < 2; nViewer++)
	{
		CString str;
		CBrush *oldBr;
		int textX, textY;

		CRect rectROI;
		CPen penOK(PS_SOLID, 7, RGB(0, 255, 0)), penNG(PS_SOLID, 7, RGB(255, 0, 0)),
			penMatch(PS_SOLID, 7, RGB(255, 255, 0)), *oldpen;
		CPen penROI(PS_DOT, 7, RGB(255, 255, 255));

		auto pDC = m_ImageManualInputMini[nViewer].getOverlayDC();
		auto pDCLarge = m_ImageManualInput[nViewer].getOverlayDC();

#ifndef JOB_INFO
		int camCount = theApp.m_pFrame->m_pAlgorithmInfo.getAlgorithmCameraNum(m_nAlgoID);
		std::vector<int> camBuf = theApp.m_pFrame->m_pAlgorithmInfo.getAlgorithmCameraIndex(m_nAlgoID);
#else
		int camCount = theApp.m_pFrame->vt_job_info[job_id].num_of_camera;
		std::vector<int> camBuf = theApp.m_pFrame->vt_job_info[job_id].camera_index;
#endif

		if (camCount == 1)
		{
			real_cam = camBuf.at(0);
			npos = nViewer;
		}
		else if (camCount == 2)
		{
			real_cam = camBuf.at(nViewer);  npos = 0;
		}
		else
		{
		}

		int W = theApp.m_pFrame->m_stCamInfo[real_cam].w;
		int H = theApp.m_pFrame->m_stCamInfo[real_cam].h;

		result = theApp.m_pFrame->GetMatching(job_id).getFindInfo(nViewer, npos).GetFound();

		int OLD_BKMODE = pDC->SetBkMode(TRANSPARENT);
		oldpen = pDC->SelectObject(&penOK);
		pDCLarge->SelectObject(&penOK);
		oldBr = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);

		if (result == FIND_OK)			pDC->SelectObject(&penOK);
		else if (result == FIND_MATCH)	pDC->SelectObject(&penMatch);
		else							pDC->SelectObject(&penNG);
	
		dbPosX = theApp.m_pFrame->GetMatching(job_id).getFindInfo(nViewer, npos).GetXPos();
		dbPosY = theApp.m_pFrame->GetMatching(job_id).getFindInfo(nViewer, npos).GetYPos();
		dbScore = theApp.m_pFrame->GetMatching(job_id).getFindInfo(nViewer, npos).getScore();
		rectROI = theApp.m_pFrame->GetMatching(job_id).getSearchROI(nViewer, npos);

		///// Draw Pattern Find Pos
		pDC->MoveTo((int)(dbPosX - 20), (int)(dbPosY));
		pDC->LineTo((int)(dbPosX + 20), (int)(dbPosY));
		pDC->MoveTo((int)(dbPosX), (int)(dbPosY - 20));
		pDC->LineTo((int)(dbPosX), (int)(dbPosY + 20));

		pDCLarge->MoveTo((int)(dbPosX - 20), (int)(dbPosY));
		pDCLarge->LineTo((int)(dbPosX + 20), (int)(dbPosY));
		pDCLarge->MoveTo((int)(dbPosX), (int)(dbPosY - 20));
		pDCLarge->LineTo((int)(dbPosX), (int)(dbPosY + 20));

		///// Draw Search ROI
		pDC->SelectObject(&penROI);
		pDC->Rectangle(&rectROI);

		// Draw Find Pattern Info.
		if ((int)dbPosX < (W / 2))			textX = (int)dbPosX + 20;
		else			textX = (int)dbPosX - 140;

		if ((int)dbPosY < (W / 2))			textY = (int)dbPosY + 40;
		else			textY = (int)dbPosY - 60;

		str.Format("[ %.1f, %.1f ] %.1f%%", dbPosX, dbPosY, dbScore);

		if (result == FIND_ERR)			pDC->SetTextColor(RGB(255, 0, 0));
		else if (result == FIND_OK) 		pDC->SetTextColor(RGB(0, 255, 0));
		else								pDC->SetTextColor(RGB(255, 255, 0));

		pDC->TextOutA(textX, textY, str);

		pDC->SetBkMode(OLD_BKMODE);
		pDC->SelectObject(oldpen);
		pDC->SelectObject(oldBr);
	}
}

void CDlgManualInputMark::drawMarkGuidePostion(CViewerEx *viewer,BOOL bErase)
{
	//if (bErase)
	//{
	//	viewer->ClearOverlayDC();
	//}

	//auto *pDC = viewer->getOverlayDC();

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

	viewer->Invalidate();
}

void CDlgManualInputMark::OnTimer(UINT_PTR nIDEvent)
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


BOOL CDlgManualInputMark::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR *nmHdr = (NMHDR*)lParam;

	if(	nmHdr->code == WM_DRAWING_FINISH_MESSAGE)
	{
		calcManualPosition(m_nSelectCamera);
	}

	return CDialogEx::OnNotify(wParam, lParam, pResult);
}


BOOL CDlgManualInputMark::PreTranslateMessage(MSG* pMsg)
{
	auto& view = m_ImageManualInput[m_nSelectCamera];
	
	if (pMsg->message == WM_KEYDOWN && view.GetModeDrawingFigure())
	{
		CPoint st, ed;
		view.GetBeginEnd(st, ed);

		if(pMsg->wParam == VK_LEFT)
		{
			if (IsSHIFTpressed())     m_OffsetFunctions[0](1, 0, 0, 0, st, ed, &view);
			else if (IsCTRLpressed()) m_OffsetFunctions[1](1, 0, 0, 0, st, ed, &view);
			else m_OffsetFunctions[0](1, 0, 1, 0, st, ed, &view);
			view.SoftDirtyRefresh();
		}
		else if(pMsg->wParam == VK_RIGHT)
		{
			if (IsSHIFTpressed())     m_OffsetFunctions[1](0, 0, 1, 0, st, ed, &view);
			else if (IsCTRLpressed()) m_OffsetFunctions[1](0, 0, 1, 0, st, ed, &view);
			else                      m_OffsetFunctions[1](1, 0, 1, 0, st, ed, &view);
			view.SoftDirtyRefresh();
		}
		else if(pMsg->wParam == VK_UP)
		{
			if (IsSHIFTpressed())     m_OffsetFunctions[2](0, 1, 0, 0, st, ed, &view);
			else if (IsCTRLpressed()) m_OffsetFunctions[3](0, 1, 0, 0, st, ed, &view);
			else                      m_OffsetFunctions[3](0, 1, 0, 1, st, ed, &view);
			view.SoftDirtyRefresh();
		}
		else if(pMsg->wParam == VK_DOWN)
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


void CDlgManualInputMark::OnLButtonDown(UINT nFlags, CPoint point)
{
	int id = -1;
	CPoint st, ed;
	auto& view = m_ImageManualInput[m_nSelectCamera];
	view.GetBeginEnd(st, ed);

	for(int i = 0; i < 6; i++)
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
		calcManualPosition(m_nSelectCamera);
			}
	else
			{
		CRect r;
		m_ImageManualInput[m_nSelectCamera].GetClientRect(&r);
		if (r.PtInRect(point)) calcManualPosition(m_nSelectCamera);
	}
	

	CDialogEx::OnLButtonDown(nFlags, point);
}

void CDlgManualInputMark::fnBtnDisplay(BOOL bJudge1, BOOL bJudge2)
{
	if(bJudge1 == TRUE && bJudge2 == TRUE)	m_btnInputOK.SetEnable(TRUE);
	else									m_btnInputOK.SetEnable(FALSE);
}

void CDlgManualInputMark::OnBnClickedBtnZoomIn()
{
	CViewerEx* m_ViewerCamera = NULL;
	m_ViewerCamera = &m_ImageManualInput[m_nSelectCamera];
	m_ViewerCamera->ZoomIn();
}

void CDlgManualInputMark::OnBnClickedBtnZoomOut()
{
	CViewerEx* m_ViewerCamera = NULL;
	m_ViewerCamera = &m_ImageManualInput[m_nSelectCamera];
	m_ViewerCamera->ZoomOut();
}

void CDlgManualInputMark::OnBnClickedBtnDlgMove()
{
	CViewerEx* m_ViewerCamera = NULL;
	m_ViewerCamera = &m_ImageManualInput[m_nSelectCamera];

	//if (m_ViewerCamera->GetModeDrawingFigure())
	//{
	//	m_btnZoomMove.SetColorBkg(255, COLOR_BTN_BODY);
	//	m_ViewerCamera->SetModeDrawingFigure(false);
	//	m_ViewerCamera->SetEnableModifyFigure(0, false);
	//	m_ViewerCamera->SetEnableDrawFigure(0, false);
	//}
	//else
	//{
	//	m_btnZoomMove.SetColorBkg(255, COLOR_BTN_SELECT);
	//	m_ViewerCamera->SetEnableDrawAllFigures(true);
	//	if (m_nShapeType == 0)			m_ViewerCamera->SetEnableFigureRectangle(0);
	//	else if (m_nShapeType == 1)		m_ViewerCamera->SetEnableFigureLine(0);
	//	else if (m_nShapeType == 2)     m_ViewerCamera->SetEnableFigureCircle(0);
	//	
	//	m_ViewerCamera->SetModeDrawingFigure(true);
	//	m_ViewerCamera->SetEnableModifyFigure(0, true);
	//	m_ViewerCamera->SetEnableDrawFigure(0, true);
	//}
}