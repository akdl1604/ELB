// FormMachineView.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "FormMachineView.h"
#include "LET_AlignClientDlg.h"

// CFormMachineView

IMPLEMENT_DYNCREATE(CFormMachineView, CFormView)

CFormMachineView::CFormMachineView(): CFormView(CFormMachineView::IDD)
{
	m_pMain = NULL;
	for (int i = 0; i < MAX_CAMERA; i++)
	{
		m_ptrCalib[i].clear();
		m_dbCalibTargetX[i] = 0;
		m_dbCalibTargetY[i] = 0;
		m_dbCalibRotateX[i] = 0;
		m_dbCalibRotateY[i] = 0;
		m_dbCalibRad[i] = 0;
	}

	m_nSelectAlgorithm = 0;
	m_nSelectAlgorithmPos = 0;
	m_nSelectAlgorithmCamera = 0;
	m_bDrawCalibration = FALSE;
	m_pThread = NULL;

	m_bThreadMachineDisplayEndFlag = FALSE;

	for (int i = 0; i < MAX_VIEWER; i++)
		m_pDlgViewerMachine[i] = NULL;

	m_nTabC = 0;
}

CFormMachineView::~CFormMachineView()
{
	for (int i = 0; i < MAX_VIEWER; i++)
	{
		if (m_pDlgViewerMachine[i] != NULL)
		{
			delete m_pDlgViewerMachine[i];
			m_pDlgViewerMachine[i] = NULL;
		}
	}
}

void CFormMachineView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LB_MACHINE_VIEW_TITLE, m_LbMachineViewTitle);
	DDX_Control(pDX, IDC_LB_MACHINE_VIEW_CALIB_DATE, m_LbMachineViewCalibTitle);
	DDX_Control(pDX, IDC_LB_MACHINE_VIEW_ROTATE_DATE, m_LbMachineViewRotateTitle);
}

BEGIN_MESSAGE_MAP(CFormMachineView, CFormView)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_VIEW_CONTROL, OnViewControl)
END_MESSAGE_MAP()


// CFormMachineView 진단입니다.

#ifdef _DEBUG
void CFormMachineView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CFormMachineView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CFormMachineView 메시지 처리기입니다.
void CFormMachineView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	m_pMain = (CLET_AlignClientDlg *)GetParent();
	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);

	GetDlgItem(IDC_STATIC_VIEWER_BASE_MACHINE)->GetWindowRect(&m_rcStaticViewerBaseMachine);
	GetDlgItem(IDC_STATIC_VIEWER_BASE_MACHINE)->ShowWindow(SW_HIDE);	

	InitTitle(&m_LbMachineViewTitle, "Machine View", 24.f, RGB(192, 64, 64));
	InitTitle(&m_LbMachineViewCalibTitle, "Calibration Date: ", 16.f, RGB(192, 64, 64));
	InitTitle(&m_LbMachineViewRotateTitle, "Rotation Date: ", 16.f, RGB(192, 64, 64));

	CString strTemp;

#ifndef JOB_INFO
	int w, h;
	for (int i = 0; i < m_pMain->m_nNumCamera; i++)
	{
		w = m_pMain->m_pViewDisplayInfo[i].getViewImageWidth();
		h = m_pMain->m_pViewDisplayInfo[i].getViewImageHeight();

		if (m_pDlgViewerMachine[i] != NULL)
		{
			delete m_pDlgViewerMachine[i];
			m_pDlgViewerMachine[i] = NULL;
		}

		m_pDlgViewerMachine[i] = new CDlgViewer;
		//strTemp.Format("Viewer - %d", i + 1);
		m_pDlgViewerMachine[i]->Create(IDD_DLG_VIEWER, this);
		
		m_pDlgViewerMachine[i]->setParent(this);
		m_pDlgViewerMachine[i]->setViewerName(m_pMain->m_pViewDisplayInfo[i].getViewerName());
		m_pDlgViewerMachine[i]->setShowGridInfo(TRUE);
		m_pDlgViewerMachine[i]->setShowTabCtrl(FALSE);
		m_pDlgViewerMachine[i]->setUsePopupWindow(m_pMain->m_pViewDisplayInfo[i].geUseViewerPopup());
		m_pDlgViewerMachine[i]->GetViewer().SetEnableDrawAllFigures(false);
		m_pDlgViewerMachine[i]->GetViewer().SetEnableFigureRectangle(0);
		m_pDlgViewerMachine[i]->GetViewer().SetEnableModifyFigure(0, true);
		m_pDlgViewerMachine[i]->GetViewer().SetEnableDrawFigure(0, true);
		m_pDlgViewerMachine[i]->GetViewer().setCameraResolutionX(m_pMain->m_pViewDisplayInfo[i].getResolutionX());
		m_pDlgViewerMachine[i]->GetViewer().setCameraResolutionY(m_pMain->m_pViewDisplayInfo[i].getResolutionY());
		m_pDlgViewerMachine[i]->GetViewer().OnInitWithCamera(w, h, 8);
		m_pDlgViewerMachine[i]->GetViewer().SetEnableDrawCrossLine(true);
		m_pDlgViewerMachine[i]->updateViewerFunc();

		m_pDlgViewerMachine[i]->GetViewer().ShowWindow(SW_HIDE);
		m_pDlgViewerMachine[i]->ShowWindow(SW_HIDE);
	}
#else
	int w, h;
	for(int i = 0; i < m_pMain->vt_viewer_info.size(); i++)
	{

#ifdef _SAPERA
		int curr_cam = m_pMain->vt_viewer_info[i].camera_index;
		w = m_pMain->m_stCamInfo[curr_cam].w;
		h = m_pMain->m_stCamInfo[curr_cam].h;
#else
		w = m_pMain->vt_viewer_info[i].image_width;
		h = m_pMain->vt_viewer_info[i].image_height;
#endif

		if (m_pDlgViewerMachine[i] != NULL)
		{
			delete m_pDlgViewerMachine[i];
			m_pDlgViewerMachine[i] = NULL;
		}

		m_pDlgViewerMachine[i] = new CDlgViewer;
		m_pDlgViewerMachine[i]->Create(IDD_DLG_VIEWER, this);
		SetWindowLong(m_pDlgViewerMachine[i]->GetSafeHwnd(), GWL_EXSTYLE, WS_EX_NOACTIVATE);
		m_pDlgViewerMachine[i]->setParent(this);
		m_pDlgViewerMachine[i]->setViewerName(m_pMain->vt_viewer_info[i].viewer_name.c_str());
		m_pDlgViewerMachine[i]->setShowGridInfo(TRUE);
		m_pDlgViewerMachine[i]->setShowTabCtrl(FALSE);
		m_pDlgViewerMachine[i]->setUsePopupWindow(m_pMain->vt_viewer_info[i].use_viewer_popup);
		//m_pDlgViewerMachine[i]->GetViewer().SetEnableDrawAllFigures(false);
		//m_pDlgViewerMachine[i]->GetViewer().SetEnableFigureRectangle(0);
		//m_pDlgViewerMachine[i]->GetViewer().SetEnableModifyFigure(0, true);
		//m_pDlgViewerMachine[i]->GetViewer().SetEnableDrawFigure(0, true);
		//m_pDlgViewerMachine[i]->GetViewer().setCameraResolutionX(m_pMain->vt_viewer_info[i].resolution_x);
		//m_pDlgViewerMachine[i]->GetViewer().setCameraResolutionY(m_pMain->vt_viewer_info[i].resolution_y);
		m_pDlgViewerMachine[i]->GetViewer().SetMetricTransform(Rigid(m_pMain->vt_viewer_info[i].resolution_x, 0, 0, 0, -m_pMain->vt_viewer_info[i].resolution_y, 0));
		m_pDlgViewerMachine[i]->GetViewer().OnInitWithCamera(w, h, 8);
		m_pDlgViewerMachine[i]->GetViewer().SetEnableDrawGuidingGraphics(true);
		m_pDlgViewerMachine[i]->updateViewerFunc();

		m_pDlgViewerMachine[i]->GetViewer().ShowWindow(SW_HIDE);
		m_pDlgViewerMachine[i]->ShowWindow(SW_HIDE);

		m_pDlgViewerMachine[i]->GetViewer().reset_to_gray_image();

	}
#endif
}

HBRUSH CFormMachineView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	return m_hbrBkg;
}

void CFormMachineView::InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}

void CFormMachineView::MainButtonInit(CButtonEx *pbutton, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}

void CFormMachineView::updateFrameDialog()
{
#ifndef JOB_INFO
	int camCount = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraNum(m_nSelectAlgorithm);
	if (camCount <= 0) return ;
	int cam;
	std::vector<int> camBuf = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(m_nSelectAlgorithm);
	
	for (int nCam = 0; nCam < camCount; nCam++)
	{
		cam = camBuf.at(nCam);
		if (m_pMain->m_pBaslerCam[cam] && m_pMain->m_pBaslerCam[cam]->IsGrabContinuous() != TRUE)
			m_pMain->m_pBaslerCam[cam]->GrabContinuous(TRUE);
		if (m_pMain->m_pSaperaCam[cam].IsOpend() && m_pMain->m_pSaperaCam[cam].IsGrabing() != TRUE)
			m_pMain->m_pSaperaCam[cam].grab(-1);
	}

	if (m_bThreadMachineDisplayEndFlag == FALSE)
			m_pThread = AfxBeginThread(Thread_MachineViewerDisplay, this);
#else
	int camCount = m_pMain->vt_job_info[m_nSelectAlgorithm].num_of_camera;
	if (camCount <= 0) return;
	int cam;
	std::vector<int> camBuf = m_pMain->vt_job_info[m_nSelectAlgorithm].camera_index;

	for (int nCam = 0; nCam < camCount; nCam++)
	{
		cam = camBuf.at(nCam);
#ifdef _DAHUHA
		if (m_pMain->m_pDahuhaCam[cam].isconnected() && !m_pMain->m_pDahuhaCam[cam].isgrabbing())		m_pMain->m_pDahuhaCam[cam].grabStart();
#else
		if (m_pMain->m_pBaslerCam[cam] && m_pMain->m_pBaslerCam[cam]->IsGrabContinuous() != TRUE)
			m_pMain->m_pBaslerCam[cam]->GrabContinuous(TRUE);
#endif
		if (m_pMain->m_pSaperaCam[cam].IsOpend() && m_pMain->m_pSaperaCam[cam].IsGrabing() != TRUE)
			m_pMain->m_pSaperaCam[cam].grab(-1);
	}

	int nCam_n = m_nSelectAlgorithmCamera;
	int nPos = m_nSelectAlgorithmPos;

	//2022-03-14 hsj 1cam 2pos 일때 pos 2개 등록 날짜 나타내기
	int nAlgoType = m_pMain->vt_job_info[m_nSelectAlgorithm].algo_method;

	if (nAlgoType == CLIENT_TYPE_1CAM_2POS_REFERENCE ||
		nAlgoType == CLIENT_TYPE_1CAM_4POS_ROBOT)
	{
		CString strCal, strRotate;
		strCal.Format("Calibration Pos1 : %s \nCalibration Pos2 : %s", m_pMain->GetMachine(m_nSelectAlgorithm).getMakeClibDateTime(nCam_n, 0), m_pMain->GetMachine(m_nSelectAlgorithm).getMakeClibDateTime(nCam_n, 1));
		m_LbMachineViewCalibTitle.SetText(strCal);
		strRotate.Format("Rotation Pos1: %s \nRotation Pos2: %s", m_pMain->GetMachine(m_nSelectAlgorithm).getMakeRotateDateTime(nCam_n, 0), m_pMain->GetMachine(m_nSelectAlgorithm).getMakeRotateDateTime(nCam_n, 1));
		m_LbMachineViewRotateTitle.SetText(strRotate);
	}
	else
	{
		m_LbMachineViewCalibTitle.SetText("Calibration : " + m_pMain->GetMachine(m_nSelectAlgorithm).getMakeClibDateTime(nCam_n, nPos));
		m_LbMachineViewRotateTitle.SetText("Rotation : " + m_pMain->GetMachine(m_nSelectAlgorithm).getMakeRotateDateTime(nCam_n, nPos));
	}

	if (m_bThreadMachineDisplayEndFlag == FALSE)
		m_pThread = AfxBeginThread(Thread_MachineViewerDisplay, this);
#endif

}

void CFormMachineView::updateUiDialog()
{
	
}

UINT Thread_MachineViewerDisplay(void *pParam)
{
	CString str;
	int nRet = 0;
	int nCam = 0;
	CFormMachineView *pView = (CFormMachineView *)pParam;
	theLog.logmsg(LOG_PROCESS, "Thread_ViewerDisplay(), MachineViewer, Thread Start");

	pView->m_bThreadMachineDisplayEndFlag = TRUE;

	BOOL bDisplay = FALSE;
	int nCamNum = 0;

#ifndef JOB_INFO
	int camCount = pView->m_pMain->m_pAlgorithmInfo.getAlgorithmCameraNum(pView->m_nSelectAlgorithm);
	if (camCount <= 0) return -1;

	std::vector<int> camBuf = pView->m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(pView->m_nSelectAlgorithm);

	while (pView->m_bThreadMachineDisplayEndFlag && pView->m_pMain->m_bProgramEnd != TRUE && pView->m_pMain->m_nCurForm == FORM_MACHINE)
	{
		for (nCam = 0; nCam < camCount; nCam++)
		{
			nCamNum = camBuf.at(nCam);

			if ((pView->m_pMain->m_pBaslerCam[nCamNum] && pView->m_pMain->m_pBaslerCam[nCamNum]->IsGrabContinuous()) ||
					(pView->m_pMain->m_pSaperaCam[nCamNum].IsOpend() && pView->m_pMain->m_pSaperaCam[nCamNum].IsGrabing()))
			{
				pView->m_pDlgViewerMachine[nCamNum]->GetViewer().OnLoadImageFromPtr(pView->m_pMain->getSrcBuffer(nCamNum));
			}			
		}
		
		::WaitForSingleObject(pView->m_pMain->m_hDummyEvent, 150);
	}

	pView->m_bThreadMachineDisplayEndFlag = FALSE;
#else
	int camCount = pView->m_pMain->vt_job_info[pView->m_nSelectAlgorithm].num_of_camera;
	if (camCount <= 0) return -1;

	std::vector<int> camBuf = pView->m_pMain->vt_job_info[pView->m_nSelectAlgorithm].camera_index;

	while (pView->m_bThreadMachineDisplayEndFlag && pView->m_pMain->m_bProgramEnd != TRUE && pView->m_pMain->m_nCurForm == FORM_MACHINE)
	{
		for (nCam = 0; nCam < camCount; nCam++)
		{
			nCamNum = camBuf.at(nCam);
#ifdef _DAHUHA
			if (pView->m_pMain->m_pDahuhaCam[nCamNum].isconnected() && pView->m_pMain->m_pDahuhaCam[nCamNum].isgrabbing())
#else
			if ((pView->m_pMain->m_pBaslerCam[nCamNum] && pView->m_pMain->m_pBaslerCam[nCamNum]->IsGrabContinuous()) ||
				(pView->m_pMain->m_pSaperaCam[nCamNum].IsOpend() && pView->m_pMain->m_pSaperaCam[nCamNum].IsGrabing()))
#endif
			{
				int viewer_id = pView->m_pMain->vt_job_info[pView->m_nSelectAlgorithm].machine_viewer_index[nCam];	
				pView->m_pDlgViewerMachine[viewer_id]->GetViewer().OnLoadImageFromPtr(pView->m_pMain->getSrcBuffer(nCamNum));			
			}
		}

		::WaitForSingleObject(pView->m_pMain->m_hDummyEvent, 150);
	}

	pView->m_bThreadMachineDisplayEndFlag = FALSE;
#endif

	return 0;
}

void CFormMachineView::replaceViewer(int id)
{
	if (m_pMain == NULL) return;

#ifndef JOB_INFO
	int n = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraNum(id);
	std::vector<int> cam = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(id);

	switch (n) {
	case 1:createViewer_Machine(1, 1,id);		break;
	case 2:
	{
		if (m_pMain->getModel().getAlignInfo().getAlignmentTargetDir(cam.at(0)))
	
				createViewer_Machine(1, 2, id);
		else	createViewer_Machine(2, 1, id);
	}
	break;
	case 3:		
	case 4:createViewer_Machine(2, 2, id);		break;
	case 5:
	case 6:createViewer_Machine(3, 2, id);		break;
	case 7:
	case 8:createViewer_Machine(4, 2, id);		break;
	}	
#else
	int num_of_view_x = m_pMain->vt_job_info[id].machine_view_count_x;
	int num_of_view_y = m_pMain->vt_job_info[id].machine_view_count_y;
	
	createViewer_Machine_Fix(num_of_view_x, num_of_view_y, id);
	

	/*
	//if (m_pMain->vt_job_info[id].model_info.getAlignInfo().getAlignmentTargetDir())
	switch (n) {
	case 1:createViewer_Machine(1, 1, id);		break;
	case 2:
	{
		if (m_pMain->vt_job_info[id].model_info.getAlignInfo().getAlignmentTargetDir())
		{
			createViewer_Machine(1, 2, id);
		}
		else
		{
			createViewer_Machine(2, 1, id);
		}
	}
	break;
	case 3:
	case 4:createViewer_Machine(2, 2, id);		break;
	case 5:
	case 6:createViewer_Machine(3, 2, id);		break;
	case 7:
	case 8:createViewer_Machine(4, 2, id);		break;
	}
	*/

#endif	


}

void CFormMachineView::showHideViewer()
{
	
}

LRESULT CFormMachineView::OnViewControl(WPARAM wParam, LPARAM lParam)
{
	int lRet = 0;

	switch (wParam) {
	case MSG_FMV_DISPLAY_INDIVIDUAL_DRAW_MARK:
	{
		displayFindMarkIndividual();
	}
	break;
	case MSG_FMV_DISPLAY_4POINT_CENTER:
	{
		display4PointCenterMark();
	}
	break;
	case MSG_FMV_DISPLAY_MACHINE_VIEW:
	{
		displayFindMark(lParam ? true : false);
	}
	break;
	case MSG_FMV_DISPLAY_MACHINE_VIEW_POS:
	{
		stDARW_MACHINE_INFO *draw_machine_info = (stDARW_MACHINE_INFO *)lParam;
		displayFindMark(draw_machine_info->nCount, draw_machine_info->nCam, draw_machine_info->nPos, draw_machine_info->bClear);
	}
	break;
	case MSG_FMV_DISPLAY_TRAGER_VIEW:  //CLIENT_TYPE_1CAM_1SHOT_ALIGN, CLIENT_TYPE_COVER_ALIGN_INSP
	{
		double pixel[4];
#ifndef JOB_INFO
		int camCount = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraNum(m_nSelectAlgorithm);
		std::vector<int> camBuf = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(m_nSelectAlgorithm);
#else
		int camCount = m_pMain->vt_job_info[m_nSelectAlgorithm].num_of_camera;
		std::vector<int> camBuf = m_pMain->vt_job_info[m_nSelectAlgorithm].camera_index;
#endif
		int nCam = camBuf.at(0);

		m_pMain->GetMachine(m_nSelectAlgorithm).WorldToPixel(0, 0, 0.0, 0.0, &pixel[0], &pixel[1]);
		m_pMain->GetMachine(m_nSelectAlgorithm).WorldToPixel(1, 0, 0.0, 0.0, &pixel[2], &pixel[3]);

		//m_pDlgViewerMachine[nCam]->GetViewer().SetSizeRectangle(0, int(pixel[0] - 80), int(pixel[1] - 80), 160, 160);
		//m_pDlgViewerMachine[nCam]->GetViewer().SetSizeRectangle(1, int(pixel[2] - 80), int(pixel[3] - 80), 160, 160);
		m_pDlgViewerMachine[nCam]->GetViewer().Invalidate();
	}
	break;
	case MSG_FMV_DISPLAY_TARGET_LINE_VIEW:  //CLIENT_TYPE_1CAM_1SHOT_ALIGN, CLIENT_TYPE_COVER_ALIGN_INSP
	{
#ifndef JOB_INFO
		int camCount = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraNum(m_nSelectAlgorithm);
		std::vector<int> camBuf = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(m_nSelectAlgorithm);
#else
		int camCount = m_pMain->vt_job_info[m_nSelectAlgorithm].num_of_camera;
		std::vector<int> camBuf = m_pMain->vt_job_info[m_nSelectAlgorithm].camera_index;
#endif

		int nCam = camBuf.at(0);
		int W = m_pMain->m_stCamInfo[nCam].w;
		int H = m_pMain->m_stCamInfo[nCam].h;

#ifndef JOB_INFO
		double pitch = m_pMain->getModel().getAlignInfo().getFiducialMarkPitchX(nCam);
#else
		double pitch = m_pMain->vt_job_info[m_nSelectAlgorithm].model_info.getAlignInfo().getFiducialMarkPitchX();
#endif

		//m_pDlgViewerMachine[nCam]->GetViewer().SetAlignTargetLine(CPoint(int(W / 2 + (pitch / 2.0 / m_pMain->GetMachine(m_nSelectAlgorithm).getCameraResolutionX(nCam, 0))), 0));
		//m_pDlgViewerMachine[nCam]->GetViewer().SetAlignTargetLine(CPoint(int(W / 2 - (pitch / 2.0 / m_pMain->GetMachine(m_nSelectAlgorithm).getCameraResolutionX(nCam, 0))), 0));
	}
	break;
	case MSG_FMV_DISPLAY_CALIBRATION_AREA_VIEW:
	{
		for (int nJob = 0; nJob < m_pMain->vt_job_info.size(); nJob++)
		{
			displayCalibration(nJob);
		}
	}
	break;
	case MSG_FMV_SET_ALGORITHM_POS:
	{
		if (m_bThreadMachineDisplayEndFlag == TRUE)
		{
			m_bThreadMachineDisplayEndFlag = FALSE;
			WaitForSingleObject(m_pThread->m_hThread, 3000);
		}
		m_nSelectAlgorithm = lParam;
		replaceViewer(lParam);

		updateFrameDialog();
	}
	break;
	case MSG_FMV_SET_ALGORITHM_POS_POSITION:
		m_nSelectAlgorithmPos = lParam;
		//KJH 2022-05-26 Display 동기화 작업진행
		//updateFrameDialog();
		break;
	case MSG_FMV_SET_ALGORITHM_POS_CAMERA:
		m_nSelectAlgorithmCamera = lParam;
		//KJH 2022-05-26 Display 동기화 작업진행
		//updateFrameDialog();
		break;
	}
	return lRet;

}

void CFormMachineView::displayFindMarkIndividual()
{
	int nCam = m_nSelectAlgorithmCamera;
	int nPos = m_nSelectAlgorithmPos;
	int nJob = m_nSelectAlgorithm;
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
	//int viewer = m_pMain->vt_job_info[nJob].viewer_index[nCam];
	int viewer = m_pMain->vt_job_info[nJob].machine_viewer_index[nCam];


	m_pDlgViewerMachine[viewer]->GetViewer().ClearOverlayDC();
	auto pDC = m_pDlgViewerMachine[viewer]->GetViewer().getOverlayDC();

	CPen pen(PS_SOLID, 10, RGB(0, 255, 0)), *oldPen;
	oldPen = pDC->SelectObject(&pen);

	double posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetXPos();
	double posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetYPos();

	pDC->MoveTo(int(posX), int(posY + 15));
	pDC->LineTo(int(posX), int(posY - 15));

	pDC->MoveTo(int(posX + 15), int(posY));
	pDC->LineTo(int(posX - 15), int(posY));

	pDC->SelectObject(oldPen);
	m_pDlgViewerMachine[viewer]->GetViewer().SoftDirtyRefresh();
}

void CFormMachineView::display4PointCenterMark()
{
	//	std::vector<int> camBuf = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(m_nSelectAlgorithm);
	int nJob = m_nSelectAlgorithm;
	int nCam = m_nSelectAlgorithmCamera;
	
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
	//int viewer = m_pMain->vt_job_info[nJob].viewer_index[nCam];
	int viewer = m_pMain->vt_job_info[nJob].machine_viewer_index[nCam];
	auto pDC = m_pDlgViewerMachine[viewer]->GetViewer().getOverlayDC();


	CPen pen(PS_SOLID, 10, RGB(0, 255, 0)), *oldPen;
	oldPen = pDC->SelectObject(&pen);

	double posX = 0.0;
	double posY = 0.0;
	double xPos = 0.0;
	double yPos = 0.0;

	for (int nPos = 0; nPos < 4; nPos++)
	{
		xPos += m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetXPos();
		yPos += m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetYPos();
	}

	posX = xPos / 4.0;
	posY = yPos / 4.0;

	pDC->MoveTo(int(posX), int(posY + 15));
	pDC->LineTo(int(posX), int(posY - 15));

	pDC->MoveTo(int(posX + 15), int(posY));
	pDC->LineTo(int(posX - 15), int(posY));

	pDC->SelectObject(oldPen);
	m_pDlgViewerMachine[viewer]->GetViewer().SoftDirtyRefresh();
}

void CFormMachineView::displayFindMark(bool clear)
{
	int real_cam, W, H;

	int nJob = m_nSelectAlgorithm;
	int camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;

	for (int i = 0; i < camCount; i++)
	{
		real_cam = camBuf.at(i);

		W = m_pMain->m_stCamInfo[real_cam].w;
		H = m_pMain->m_stCamInfo[real_cam].h;
		int viewer = m_pMain->vt_job_info[nJob].machine_viewer_index[i];

		if(clear==false)	m_pDlgViewerMachine[viewer]->GetViewer().ClearOverlayDC();
		auto pDC = m_pDlgViewerMachine[viewer]->GetViewer().getOverlayDC();

		CPen pen(PS_SOLID, 10, RGB(0, 255, 0)), *oldPen;
		CPen pen2(PS_SOLID, 10, RGB(255, 0, 0));
		oldPen = pDC->SelectObject(&pen);

		int nNumPos = m_pMain->vt_job_info[nJob].num_of_position;

		for (int nPos = 0; nPos < nNumPos; nPos++)
		{
			double posX = m_pMain->GetMatching(nJob).getFindInfo(i, nPos).GetXPos();
			double posY = m_pMain->GetMatching(nJob).getFindInfo(i, nPos).GetYPos();

			pDC->MoveTo(int(posX), int(posY + 15));
			pDC->LineTo(int(posX), int(posY - 15));

			pDC->MoveTo(int(posX + 15), int(posY));
			pDC->LineTo(int(posX - 15), int(posY));
			pDC->SelectObject(&pen2);
		}

		if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod() == METHOD_CIRCLE)
		{
			BOOL is_circle = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].getCircleCaliper();
			if (is_circle)
			{
				CBrush brBase(RGB(255, 255, 0)), * pOldBr;
				pOldBr = pDC->SelectObject(&brBase);
				pDC->SelectStockObject(NULL_BRUSH);

				double x = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].m_circle_info.x;
				double y = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].m_circle_info.y;
				double r = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].m_circle_info.r;

				pDC->Ellipse(x - r, y - r, x + r, y + r);

				pDC->SelectObject(pOldBr);
			}

		}

		if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod() == METHOD_CALIPER)
		{
			BOOL is_circle = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].getCircleCaliper();
			if (is_circle)
			{
				CBrush brBase(RGB(255, 255, 0)), * pOldBr;
				pOldBr = pDC->SelectObject(&brBase);
				pDC->SelectStockObject(NULL_BRUSH);

				double x = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].m_circle_info.x;
				double y = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].m_circle_info.y;
				double r = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].m_circle_info.r;

				pDC->Ellipse(x - r, y - r, x + r, y + r);

				pDC->SelectObject(pOldBr);
			}
		}

		pDC->SelectObject(oldPen);
		m_pDlgViewerMachine[viewer]->GetViewer().SoftDirtyRefresh();
	}
	
	if (m_bDrawCalibration)		displayCalibrationCross();
}

void CFormMachineView::displayFindMark(int nCount, int nCam, int nPos, bool clear)
{
	int real_cam, W, H;

	int nJob = m_nSelectAlgorithm;
	int camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;

	real_cam = camBuf.at(nCam);

	W = m_pMain->m_stCamInfo[real_cam].w;
	H = m_pMain->m_stCamInfo[real_cam].h;
	int viewer = m_pMain->vt_job_info[nJob].machine_viewer_index[nCam];

	if (clear == false)	m_pDlgViewerMachine[viewer]->GetViewer().ClearOverlayDC();
	auto pDC = m_pDlgViewerMachine[viewer]->GetViewer().getOverlayDC();

	CPen pen(PS_SOLID, 10, COLOR_REALGREEN), * oldPen;
	CPen pen2(PS_SOLID, 10, COLOR_GREEN);

	if( nCount == 0 )	oldPen = pDC->SelectObject(&pen);
	else				oldPen = pDC->SelectObject(&pen2);

	if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod() == METHOD_CIRCLE)
	{
		BOOL is_circle = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].getCircleCaliper();
		if (is_circle)
		{
			double x = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].m_circle_info.x;
			double y = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].m_circle_info.y;
			double r = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].m_circle_info.r;
			pDC->Ellipse(x - r, y - r, x + r, y + r);
		}
	}
	else if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod() == METHOD_CALIPER)
	{
		BOOL is_circle = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].getCircleCaliper();
		if (is_circle)
		{
			double x = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].m_circle_info.x;
			double y = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].m_circle_info.y;
			double r = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].m_circle_info.r;
			pDC->Ellipse(x - r, y - r, x + r, y + r);
		}
		else
		{
			////////////////////////////////////////////////////////
			////////////////////////Draw Caliper////////////////////
			BOOL bFindLine[2];
			sLine line_info[2];

			if (nPos == 0)
			{
				line_info[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_1].m_lineInfo;
				line_info[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_2].m_lineInfo;

				bFindLine[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_1].getIsMakeLine();
				bFindLine[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_2].getIsMakeLine();
			}

			COLORREF color;
			//////////////////// 라인 그리기 //////////////////////
			if (bFindLine[0] && bFindLine[1])	color = COLOR_GREEN;
			else								color = COLOR_RED;

			// 수직 그리기
			if (bFindLine[0])
				m_pMain->draw_line(&m_pDlgViewerMachine[viewer]->GetViewer(), color, line_info[0], H, W);

			// 수평 그리기
			if (bFindLine[1])
				m_pMain->draw_line(&m_pDlgViewerMachine[viewer]->GetViewer(), color, line_info[1], H, W);

			//////////////////// 후보군 그리기 //////////////////////
			// kbj 2022-01-03 modify draw caliper_result 
			m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][0].draw_final_result(pDC);
			m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][1].draw_final_result(pDC);

		}
	}
	else
	{
		double posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetXPos();
		double posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetYPos();

		pDC->MoveTo(int(posX), int(posY + 15));
		pDC->LineTo(int(posX), int(posY - 15));

		pDC->MoveTo(int(posX + 15), int(posY));
		pDC->LineTo(int(posX - 15), int(posY));
	}

	pDC->SelectObject(oldPen);
	m_pDlgViewerMachine[viewer]->GetViewer().SoftDirtyRefresh();

	if (m_bDrawCalibration)		displayCalibrationCross();
}

void CFormMachineView::displayCalibration(int nJob)
{
	int nCalibPoints = m_pMain->GetMachine(nJob).getNumCalPointsX() * m_pMain->GetMachine(nJob).getNumCalPointsY();
	if (nCalibPoints < 4) return;

	int real_cam, W, H;

#ifndef JOB_INFO
	int camCount = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraNum(m_nSelectAlgorithm);
	std::vector<int> camBuf = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(m_nSelectAlgorithm);
	int nAlgorithmCamCount = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraNum(m_nSelectAlgorithm);
#else
	int camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;
#endif
	std::vector<cv::Point2f> vtPoints;

	for (int nCam = 0; nCam < camCount; nCam++)
	{
		real_cam = camBuf.at(nCam);
		W = m_pMain->m_stCamInfo[real_cam].w;
		H = m_pMain->m_stCamInfo[real_cam].h;

		//int viewer = m_pMain->vt_job_info[nJob].viewer_index[nCam];
		int viewer = m_pMain->vt_job_info[nJob].machine_viewer_index[nCam];

		m_pDlgViewerMachine[viewer]->GetViewer().ClearOverlayDC();
		auto pDC = m_pDlgViewerMachine[viewer]->GetViewer().getOverlayDC();

		CPen pen(PS_SOLID, 10, COLOR_LIME), *oldPen;
		oldPen = pDC->SelectObject(&pen);
		//pDC->SetColor(COLOR_LIME);

		int nCornerX[4] = { 0, };
		int nCornerY[4] = { 0, };
		int nSize = 7;

		int nCornerIndex[4] = { 0, };

		// Calibration 영역의 네 코너에 대한 인덱스를 계산.	

		/*
		사각형 꼭지점 Offset 순서
		1		2

		0		3
		*/

		//CBrush *oldBr;
		//oldBr = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);

		int nNumPos = m_pMain->vt_job_info[nJob].num_of_position;
		vtPoints = m_pMain->GetMachine(nJob).getPtImage(nCam, 0);
		
		if (vtPoints.size() > 0)
		{
			int i = 0;
			for (i = 1; i < vtPoints.size(); i++)
			{
				if (fabs(vtPoints[0].x - vtPoints[i].x) < 15.f)
					break;
			}
			nCornerIndex[0] = 0;
			nCornerIndex[1] = i-1;
			nCornerIndex[2] = int(vtPoints.size() - 1);
			nCornerIndex[3] = int(vtPoints.size() - i);

			pDC->MoveTo(int(vtPoints[nCornerIndex[0]].x), int(vtPoints[nCornerIndex[0]].y));
			pDC->LineTo(int(vtPoints[nCornerIndex[1]].x), int(vtPoints[nCornerIndex[1]].y));

			pDC->MoveTo(int(vtPoints[nCornerIndex[1]].x), int(vtPoints[nCornerIndex[1]].y));
			pDC->LineTo(int(vtPoints[nCornerIndex[2]].x), int(vtPoints[nCornerIndex[2]].y));

			pDC->MoveTo(int(vtPoints[nCornerIndex[2]].x), int(vtPoints[nCornerIndex[2]].y));
			pDC->LineTo(int(vtPoints[nCornerIndex[3]].x), int(vtPoints[nCornerIndex[3]].y));

			pDC->MoveTo(int(vtPoints[nCornerIndex[3]].x), int(vtPoints[nCornerIndex[3]].y));
			pDC->LineTo(int(vtPoints[nCornerIndex[0]].x), int(vtPoints[nCornerIndex[0]].y));

			int nX = 0, nY = 0;
			for (int nIndex = 0; nIndex < vtPoints.size(); nIndex++)
			{
				nX = (int)vtPoints[nIndex].x;
				nY = (int)vtPoints[nIndex].y;
				//pDC->Ellipse(nX - nSize, nY - nSize, nX + nSize, nY + nSize);
				pDC->AddPoint(nX, nY, 5, 2);
			}

			vtPoints.clear();
		}
	

		//pDC->SelectObject(oldPen);
		m_pDlgViewerMachine[viewer]->GetViewer().SoftDirtyRefresh();
	}
}

void CFormMachineView::displayCalibrationCross()
{
	int nSize = 25;
	int cam, W, H;

	int camCount = m_pMain->vt_job_info[m_nSelectAlgorithm].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[m_nSelectAlgorithm].camera_index;

	for (int nCam = 0; nCam < camCount; nCam++)
	{
		cam = camBuf.at(nCam);
		W = m_pMain->m_stCamInfo[cam].w;
		H = m_pMain->m_stCamInfo[cam].h;

		//int viewer = m_pMain->vt_job_info[m_nSelectAlgorithm].viewer_index[nCam];
		int viewer = m_pMain->vt_job_info[m_nSelectAlgorithm].machine_viewer_index[nCam];

		auto pDC = m_pDlgViewerMachine[viewer]->GetViewer().getOverlayDC();
		CPen pen(PS_DASHDOTDOT, 5, COLOR_RED), *oldPen;
		CPen penB(PS_SOLID, 5, COLOR_BLUE);
		oldPen = pDC->SelectObject(&pen);

		//CBrush *oldBr;
		//CBrush *oldBr = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);

		if (m_ptrCalib[cam].size() >= 1)
		{
			int nX = 0, nY = 0;
			for (int nIndex = 0; nIndex < m_ptrCalib[cam].size(); nIndex++)
			{
				nX = (int)m_ptrCalib[cam][nIndex].x;
				nY = (int)m_ptrCalib[cam][nIndex].y;

				pDC->MoveTo(nX, 0);
				pDC->LineTo(nX, H);
				pDC->MoveTo(0, nY);
				pDC->LineTo(W, nY);
			}

			nX = (int)m_dbCalibTargetX[cam];
			nY = (int)m_dbCalibTargetY[cam];

			pDC->SelectObject(&penB);
			pDC->Ellipse(nX - nSize, nY - nSize, nX + nSize, nY + nSize);
			pDC->Ellipse(nX - nSize - 1, nY - nSize - 1, nX + nSize + 1, nY + nSize + 1);
			pDC->SetColor(COLOR_BLUE);
			pDC->AddPoint(nX, nY, 5, 2);
		}

		nSize = int(m_dbCalibRad[cam]);
		pDC->Ellipse(int(m_dbCalibRotateX[cam] - nSize), int(m_dbCalibRotateY[cam] - nSize), int(m_dbCalibRotateX[cam] + nSize), int(m_dbCalibRotateY[cam] + nSize));

		nSize = 4;
		pDC->Ellipse(int(m_dbCalibRotateX[cam] - nSize), int(m_dbCalibRotateY[cam] - nSize), int(m_dbCalibRotateX[cam] + nSize), int(m_dbCalibRotateY[cam] + nSize));

		for (int nIndex = 0; nIndex < 2; nIndex++)
		{
			int nX = (int)m_pMain->GetMachine(m_nSelectAlgorithm).getImageOrgX(nCam, 0, nIndex);
			int nY = (int)m_pMain->GetMachine(m_nSelectAlgorithm).getImageOrgY(nCam, 0, nIndex);

			pDC->MoveTo(int(nX), int(nY + 15));
			pDC->LineTo(int(nX), int(nY - 15));

			pDC->MoveTo(int(nX + 15), int(nY));
			pDC->LineTo(int(nX - 15), int(nY));
		}


		pDC->SelectObject(oldPen);
		m_pDlgViewerMachine[viewer]->GetViewer().SoftDirtyRefresh();

		pen.DeleteObject();
		penB.DeleteObject();
		//oldBr->DeleteObject();
	}
}

void CFormMachineView::createViewer_Machine(int nNumX, int nNumY,int id)
{
	std::vector<int> cam;

	int nStartX = 0;
	int nStartY = 0;
	int nWidth, nHeight;
	int gab = 5, nC = 0, nCam = 0;

#ifndef JOB_INFO
	if(id!=-1) cam = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(id);

	nWidth = (m_rcStaticViewerBaseMachine.Width() - ((nNumX - 1) * gab)) / nNumX;
	nHeight = (m_rcStaticViewerBaseMachine.Height() - ((nNumY - 1) * gab)) / nNumY;

	for (int i = 0; i < m_pMain->m_nNumCamera; i++)
	{
		m_pDlgViewerMachine[i]->GetViewer().ShowWindow(SW_HIDE);
		m_pDlgViewerMachine[i]->ShowWindow(SW_HIDE);
	}

	for (int i = 0; i < m_pMain->m_nNumCamera; i++)
	{
		if (nC >= (nNumX * nNumY))			continue;
		if (nCam >= cam.size() || cam.at(nCam) != i) continue;
		else nCam++;		

		m_pDlgViewerMachine[i]->ShowWindow(SW_SHOW);
		m_pDlgViewerMachine[i]->GetViewer().ShowWindow(SW_SHOW);

		//Machine View 위치 변경.
		int offX, offY = 0;

		if ( cam.size() <= 2 && m_pMain->GetMachine(m_nSelectAlgorithm).getMcViewerChange(id))
		{
			int nChange = 0;
			switch (nC)
			{
				case 0:		nChange = 1;		break;
				case 1:		nChange = 0;		break;
			}
			offX = m_rcStaticViewerBaseMachine.left + (nChange % nNumX) * (gab + nWidth);
			offY = m_rcStaticViewerBaseMachine.top + (nChange / nNumX) * (gab + nHeight);
		}
		else
		{
			offX = m_rcStaticViewerBaseMachine.left + (nC % nNumX) * (gab + nWidth);
			offY = m_rcStaticViewerBaseMachine.top + (nC / nNumX) * (gab + nHeight);
		}

		m_pDlgViewerMachine[i]->MoveWindow(offX, offY, nWidth, nHeight);
		m_pDlgViewerMachine[i]->GetWindowRect(&m_rcMachineViewer[i]);
		ScreenToClient(&m_rcMachineViewer[i]);

		m_pDlgViewerMachine[i]->setDlgChildPos(m_rcMachineViewer[i]);
		m_pDlgViewerMachine[i]->MoveWindow(&m_rcMachineViewer[i]);
		m_pDlgViewerMachine[i]->fitViewer();
		m_pDlgViewerMachine[i]->Invalidate();

		nC++;
	}
#else
	if (id != -1) cam = m_pMain->vt_job_info[id].camera_index;

	nWidth = (m_rcStaticViewerBaseMachine.Width() - ((nNumX - 1) * gab)) / nNumX;
	nHeight = (m_rcStaticViewerBaseMachine.Height() - ((nNumY - 1) * gab)) / nNumY;

	int total_cam = 0.0;

	for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
		total_cam += m_pMain->vt_job_info[i].num_of_camera;

	for (int i = 0; i < total_cam; i++)
	{
		m_pDlgViewerMachine[i]->GetViewer().ShowWindow(SW_HIDE);
		m_pDlgViewerMachine[i]->ShowWindow(SW_HIDE);
	}

	for (int i = 0; i < total_cam; i++)
	{
		if (nC >= (nNumX * nNumY))						continue;
		if (nCam >= cam.size() || cam.at(nCam) != i)	continue;
		else nCam++;

		m_pDlgViewerMachine[i]->ShowWindow(SW_SHOWNOACTIVATE);
		m_pDlgViewerMachine[i]->GetViewer().ShowWindow(SW_SHOW);

		//Machine View 위치 변경.
		int offX, offY = 0;

		if (cam.size() <= 2 && m_pMain->GetMachine(m_nSelectAlgorithm).getMcViewerChange())
		{
			int nChange = 0;
			switch (nC)
			{
			case 0:		nChange = 1;		break;
			case 1:		nChange = 0;		break;
			}
			offX = m_rcStaticViewerBaseMachine.left + (nChange % nNumX) * (gab + nWidth);
			offY = m_rcStaticViewerBaseMachine.top + (nChange / nNumX) * (gab + nHeight);
		}
		else
		{
			offX = m_rcStaticViewerBaseMachine.left + (nC % nNumX) * (gab + nWidth);
			offY = m_rcStaticViewerBaseMachine.top + (nC / nNumX) * (gab + nHeight);
		}

		m_pDlgViewerMachine[i]->MoveWindow(offX, offY, nWidth, nHeight);
		m_pDlgViewerMachine[i]->GetWindowRect(&m_rcMachineViewer[i]);
		ScreenToClient(&m_rcMachineViewer[i]);

		m_pDlgViewerMachine[i]->setDlgChildPos(m_rcMachineViewer[i]);
		m_pDlgViewerMachine[i]->MoveWindow(&m_rcMachineViewer[i]);
		m_pDlgViewerMachine[i]->fitViewer();
		m_pDlgViewerMachine[i]->Invalidate();

		nC++;
	}
#endif
}

void CFormMachineView::createViewer_Machine_Fix(int nNumX, int nNumY, int id)
{
	std::vector<int> cam;

	int nStartX = 0;
	int nStartY = 0;
	int nWidth = 0, nHeight = 0;
	int gab = 5;

	if (id != -1) cam = m_pMain->vt_job_info[id].camera_index;

	nWidth = (m_rcStaticViewerBaseMachine.Width() - ((nNumX - 1) * gab)) / nNumX;
	nHeight = (m_rcStaticViewerBaseMachine.Height() - ((nNumY - 1) * gab)) / nNumY;

	// machine창의 개수를 일단 main viewer 창 개수 만큼 생성해 둔다.
	for (int i = 0; i < m_pMain->vt_viewer_info.size(); i++)
	{
		m_pDlgViewerMachine[i]->GetViewer().ShowWindow(SW_HIDE);
		m_pDlgViewerMachine[i]->ShowWindow(SW_HIDE);
	}

	for (int i = 0; i < m_pMain->vt_job_info[id].machine_viewer_index.size(); i++)
	{
		int viewer = m_pMain->vt_job_info[id].machine_viewer_index[i];

		int offX = m_rcStaticViewerBaseMachine.left + (i % nNumX) * (gab + nWidth);
		int offY = m_rcStaticViewerBaseMachine.top + (i / nNumX) * (gab + nHeight);

		m_pDlgViewerMachine[viewer]->MoveWindow(offX, offY, nWidth, nHeight);
		m_pDlgViewerMachine[viewer]->GetWindowRect(&m_rcMachineViewer[viewer]);
		ScreenToClient(&m_rcMachineViewer[viewer]);

		m_pDlgViewerMachine[viewer]->setDlgChildPos(m_rcMachineViewer[viewer]);
		m_pDlgViewerMachine[viewer]->MoveWindow(&m_rcMachineViewer[viewer]);
		m_pDlgViewerMachine[viewer]->fitViewer();
		m_pDlgViewerMachine[viewer]->Invalidate();

		m_pDlgViewerMachine[viewer]->ShowWindow(SW_SHOWNOACTIVATE);
		m_pDlgViewerMachine[viewer]->GetViewer().ShowWindow(SW_SHOW);
	}
}


BOOL CFormMachineView::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_SYSKEYDOWN)
		return TRUE;

	return CFormView::PreTranslateMessage(pMsg);
}

//KMB 2022-10-24 캘리브레이션 뷰어 Resolution 즉시 적용
void CFormMachineView::SetResCalibration()
{
	for (int i = 0; i < m_pMain->vt_viewer_info.size(); i++)
	{
		//m_pDlgViewerMachine[i]->GetViewer().setCameraResolutionX(m_pMain->vt_viewer_info[i].resolution_y);
		//m_pDlgViewerMachine[i]->GetViewer().setCameraResolutionY(m_pMain->vt_viewer_info[i].resolution_x);
	}
}