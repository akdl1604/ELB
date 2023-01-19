// FormCameraView.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "FormCameraView.h"
#include "LET_AlignClientDlg.h"
#include "FormMainView.h"
#include "ImageProcessing/MathUtil.h" // 20211012 Tkyuha 전역함수 사용

#include <fstream>
#include <iostream>

// CFormCameraView
//20211229
#define PIX_SORT(a,b) { if ((a) > (b)) PIX_SWAP((a), (b));} 
#define PIX_SWAP(a,b) { BYTE tmp = (a); (a) = (b); (b) = tmp;} 

BYTE opt_med9(BYTE p[9]) {
	PIX_SORT(p[1], p[2]); PIX_SORT(p[4], p[5]); PIX_SORT(p[7], p[8]);
	PIX_SORT(p[0], p[1]); PIX_SORT(p[3], p[4]); PIX_SORT(p[6], p[7]);
	PIX_SORT(p[1], p[2]); PIX_SORT(p[4], p[5]); PIX_SORT(p[7], p[8]);
	PIX_SORT(p[0], p[3]); PIX_SORT(p[5], p[8]); PIX_SORT(p[4], p[7]);
	PIX_SORT(p[3], p[6]); PIX_SORT(p[1], p[4]); PIX_SORT(p[2], p[5]);
	PIX_SORT(p[4], p[7]); PIX_SORT(p[4], p[2]); PIX_SORT(p[6], p[4]);
	PIX_SORT(p[4], p[2]); return(p[4]);
}
//20211229

UINT Thread_ViewerDisplay(void *pParam)
{
	CString str;
	int nRet = 0;
	CFormCameraView *pView = (CFormCameraView *)pParam;
	theLog.logmsg(LOG_PROCESS, "Thread_ViewerDisplay(), CameraViewer, Thread Start");

#ifndef JOB_INFO
	pView->m_bThreadViewDisplayEndFlag = TRUE;

	int w = pView->m_pMain->m_pViewDisplayInfo[pView->m_nSelViewerNum].getViewImageWidth();
	int h = pView->m_pMain->m_pViewDisplayInfo[pView->m_nSelViewerNum].getViewImageHeight();

	BOOL bDisplay = FALSE;
	while (pView->m_bThreadViewDisplayEndFlag && pView->m_pMain->m_bProgramEnd != TRUE && pView->m_pMain->m_nCurForm == FORM_CAMERA)
	{
		if ((pView->m_pMain->m_pBaslerCam[pView->m_nSelViewerNum] && pView->m_pMain->m_pBaslerCam[pView->m_nSelViewerNum]->IsGrabContinuous()) ||
			(pView->m_pMain->m_pSaperaCam[pView->m_nSelViewerNum].IsOpend() && pView->m_pMain->m_pSaperaCam[pView->m_nSelViewerNum].IsGrabing()))
		{

			pView->m_ViewerCamera.OnLoadImageFromPtr(pView->m_pMain->getSrcBuffer(pView->m_nSelViewerNum));
			pView->m_pMain->copyMemory(pView->m_pMain->getCameraViewBuffer(), pView->m_pMain->getSrcBuffer(pView->m_nSelViewerNum), w * h);
		}
		::WaitForSingleObject(pView->m_pMain->m_hDummyEvent, 100);
	}
	::WaitForSingleObject(pView->m_pMain->m_hDummyEvent, 100);
}

	pView->m_bThreadViewDisplayEndFlag = FALSE;
#else
	pView->m_bThreadViewDisplayEndFlag = TRUE;
	
	int w = pView->m_pMain->m_stCamInfo[pView->curr_cam].w;
	int h = pView->m_pMain->m_stCamInfo[pView->curr_cam].h;

	BOOL bDisplay = FALSE;
	while (pView->m_bThreadViewDisplayEndFlag && pView->m_pMain->m_bProgramEnd != TRUE && pView->m_pMain->m_nCurForm == FORM_CAMERA)
	{
#ifdef _DAHUHA
		if (pView->m_pMain->m_pDahuhaCam[pView->curr_cam].isconnected() && pView->m_pMain->m_pDahuhaCam[pView->curr_cam].isgrabbing())
#else
		if ((pView->m_pMain->m_pBaslerCam[pView->curr_cam] && pView->m_pMain->m_pBaslerCam[pView->curr_cam]->IsGrabContinuous()) || 
			(pView->m_pMain->m_pSaperaCam[pView->curr_cam].IsOpend() && pView->m_pMain->m_pSaperaCam[pView->curr_cam].IsGrabing()))
#endif
		{
			AcquireSRWLockExclusive(&pView->m_pMain->g_srwlock); // 동기화 시킴 Tkyuha 20211027				
			pView->m_ViewerCamera.OnLoadImageFromPtr(pView->m_pMain->getSrcBuffer(pView->curr_cam));
			pView->m_pMain->copyMemory(pView->m_pMain->getCameraViewBuffer(), pView->m_pMain->getSrcBuffer(pView->curr_cam), w * h);
			ReleaseSRWLockExclusive(&pView->m_pMain->g_srwlock);
		}
		::WaitForSingleObject(pView->m_pMain->m_hDummyEvent, 100);
	}

	pView->m_bThreadViewDisplayEndFlag = FALSE;
#endif

	return 0;
}

IMPLEMENT_DYNCREATE(CFormCameraView, CFormView)

CFormCameraView::CFormCameraView()
	: CFormView(CFormCameraView::IDD)
{
	m_nSelViewerNum = 0;
	m_pProfile = NULL;
	curr_viewer = 0;
	curr_cam = 0;
	m_bThreadViewDisplayEndFlag = FALSE;
}

CFormCameraView::~CFormCameraView()
{
	//if (m_bThreadViewDisplayEndFlag = TRUE)
	//{
	//	m_bThreadViewDisplayEndFlag = FALSE;
	//	WaitForSingleObject(m_pThread->m_hThread, 3000);
	//}

	if (m_pProfile != NULL)
	{
		m_pProfile->DestroyWindow();
		delete m_pProfile;
		m_pProfile = NULL;
	}
}

void CFormCameraView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CS_VIEWER_CAMERA, m_ViewerCamera);
	DDX_Control(pDX, IDC_LB_CAMERA_VIEW_TITLE, m_LbCameraViewTitle);
	DDX_Control(pDX, IDC_LB_IMAGE_INFO, m_LbImageInfo);
	DDX_Control(pDX, IDC_LB_IMAGE_INFO2, m_LbImageInfo2);
	DDX_Control(pDX, IDC_LB_IMAGE_INFO3, m_LbImageInfo3);
}

BEGIN_MESSAGE_MAP(CFormCameraView, CFormView)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_VIEW_CONTROL, OnViewControl)
END_MESSAGE_MAP()


// CFormCameraView 진단입니다.

#ifdef _DEBUG
void CFormCameraView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CFormCameraView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CFormCameraView 메시지 처리기입니다.
void CFormCameraView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);
	m_pMain = (CLET_AlignClientDlg*)AfxGetMainWnd();

	InitTitle(&m_LbCameraViewTitle, "Camera View", 24.f, RGB(64, 192, 64));
	InitTitle(&m_LbImageInfo, "", 16.f, RGB(64, 192, 64));
	m_LbImageInfo.SetAlignTextCM();

	InitTitle(&m_LbImageInfo2, "", 16.f, RGB(64, 192, 64));
	m_LbImageInfo2.SetAlignTextCM();
	InitTitle(&m_LbImageInfo3, "", 16.f, RGB(64, 192, 64));
	m_LbImageInfo3.SetAlignTextCM();

	m_ViewerCamera.InitControl(this);
	m_ViewerCamera.SetEnableDrawGuidingGraphics(true);


	SetViewerInit(m_nSelViewerNum);
}

HBRUSH CFormCameraView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	return m_hbrBkg;
}

void CFormCameraView::InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}

void CFormCameraView::MainButtonInit(CButtonEx *pbutton, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}

BOOL CFormCameraView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	NMHDR *nmHdr = (NMHDR*)lParam;

	if (nmHdr->code == WM_DRAWING_FINISH_MESSAGE)
	{
		GetDlgItem(IDC_CS_VIEWER_CAMERA)->SetFocus();
		
		if (m_pProfile != NULL && m_pProfile->IsWindowVisible())
		{
			int WIDTH = m_pMain->m_stCamInfo[curr_cam].w;
			int HEIGHT = m_pMain->m_stCamInfo[curr_cam].h;

			//CPoint TL = m_ViewerCamera.getStFigure(1).ptBegin;
			//CPoint BR = m_ViewerCamera.getStFigure(1).ptEnd;
			CPoint TL, BR;
			m_ViewerCamera.GetBeginEnd(TL, BR);

			m_pProfile->m_bmpSrcImg.release();
			m_pProfile->m_bmpSrcImg = cv::Mat::zeros(HEIGHT, WIDTH, CV_8U);
			memcpy(m_pProfile->m_bmpSrcImg.data, m_ViewerCamera.GetImagePtr(), HEIGHT * WIDTH);
			//cv::flip(m_pProfile->m_bmpSrcImg, m_pProfile->m_bmpSrcImg, 0);
			m_pProfile->fnCalcRealCoordi(TL, BR);

			m_pProfile->Invalidate();
			m_pProfile->InvalidateRect(FALSE);
		}
	}
	return CFormView::OnNotify(wParam, lParam, pResult);
}

// Need modify this later
BOOL CFormCameraView::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_SYSKEYDOWN)
		return TRUE;

	if (pMsg->message == WM_KEYDOWN && m_ViewerCamera.GetModeDrawingFigure())
	{
		if (m_ViewerCamera.IsInteraction())
		{
			CPoint st, ed;
			m_ViewerCamera.GetBeginEnd(st, ed);

			if (pMsg->wParam == VK_LEFT)
			{
				if (IsSHIFTpressed()) m_ViewerCamera.SetBeginEnd(CPoint(st.x - 1, st.y), ed);
				else if (IsCTRLpressed())	 m_ViewerCamera.SetBeginEnd(CPoint(st.x + 1, st.y), ed);
				else m_ViewerCamera.SetBeginEnd(CPoint(st.x - 1, st.y), CPoint(ed.x - 1, ed.y));
			}
			else if (pMsg->wParam == VK_RIGHT)
			{
				if (IsSHIFTpressed()) m_ViewerCamera.SetBeginEnd(st, CPoint(ed.x + 1, ed.y));
				else if (IsCTRLpressed())	 m_ViewerCamera.SetBeginEnd(st, CPoint(ed.x - 1, ed.y));
				else m_ViewerCamera.SetBeginEnd(CPoint(st.x + 1, st.y), CPoint(ed.x + 1, ed.y));
			}
			else if (pMsg->wParam == VK_UP)
			{
				if (IsSHIFTpressed()) m_ViewerCamera.SetBeginEnd(CPoint(st.x, st.y - 1), ed);
				else if (IsCTRLpressed())	 m_ViewerCamera.SetBeginEnd(CPoint(st.x, st.y + 1), ed);
				else m_ViewerCamera.SetBeginEnd(CPoint(st.x, st.y - 1), CPoint(ed.x, ed.y - 1));
			}
			else if (pMsg->wParam == VK_DOWN)
			{
				if (IsSHIFTpressed()) m_ViewerCamera.SetBeginEnd(st, CPoint(ed.x, ed.y + 1));
				else if (IsCTRLpressed())	 m_ViewerCamera.SetBeginEnd(st, CPoint(ed.x, ed.y - 1));
				else m_ViewerCamera.SetBeginEnd(CPoint(st.x, st.y + 1), CPoint(ed.x, ed.y + 1));
			}
			m_ViewerCamera.SoftDirtyRefresh();
		}
	}

#pragma region Old Code
	//if (pMsg->message == WM_KEYDOWN && m_ViewerCamera.GetModeDrawingFigure())
	//{
	//	CPoint ptStart = m_ViewerCamera.GetPointBegin(0);
	//	CPoint ptEnd = m_ViewerCamera.GetPointEnd(0);

	//	if (pMsg->wParam == VK_LEFT)
	//	{
	//		if (m_ViewerCamera.IsEnableFigureCircle(0))
	//		{
	//			if (IsSHIFTpressed()) m_ViewerCamera.SetSizeCircle(0, CPoint(ptStart.x - 1, ptStart.y), ptEnd);
	//			else if (IsCTRLpressed())	 m_ViewerCamera.SetSizeCircle(0, CPoint(ptStart.x + 1, ptStart.y), ptEnd);
	//			else m_ViewerCamera.SetSizeCircle(0, CPoint(ptStart.x - 1, ptStart.y), CPoint(ptEnd.x - 1, ptEnd.y));
	//		}
	//		else
	//		{
	//			if (IsSHIFTpressed()) m_ViewerCamera.SetSizeRectangle(0, CPoint(ptStart.x - 1, ptStart.y), ptEnd);
	//			else if (IsCTRLpressed())	 m_ViewerCamera.SetSizeRectangle(0, CPoint(ptStart.x + 1, ptStart.y), ptEnd);
	//			else m_ViewerCamera.SetSizeRectangle(0, CPoint(ptStart.x - 1, ptStart.y), CPoint(ptEnd.x - 1, ptEnd.y));
	//		}
	//	}
	//	else if (pMsg->wParam == VK_RIGHT)
	//	{
	//		if (m_ViewerCamera.IsEnableFigureCircle(0))
	//		{
	//			if (IsSHIFTpressed()) m_ViewerCamera.SetSizeCircle(0, ptStart, CPoint(ptEnd.x + 1, ptEnd.y));
	//			else if (IsCTRLpressed())	 m_ViewerCamera.SetSizeCircle(0, ptStart, CPoint(ptEnd.x - 1, ptEnd.y));
	//			else m_ViewerCamera.SetSizeCircle(0, CPoint(ptStart.x + 1, ptStart.y), CPoint(ptEnd.x + 1, ptEnd.y));
	//		}
	//		else
	//		{
	//			if (IsSHIFTpressed()) m_ViewerCamera.SetSizeRectangle(0, ptStart, CPoint(ptEnd.x + 1, ptEnd.y));
	//			else if (IsCTRLpressed())	 m_ViewerCamera.SetSizeRectangle(0, ptStart, CPoint(ptEnd.x - 1, ptEnd.y));
	//			else m_ViewerCamera.SetSizeRectangle(0, CPoint(ptStart.x + 1, ptStart.y), CPoint(ptEnd.x + 1, ptEnd.y));
	//		}
	//	}
	//	else if (pMsg->wParam == VK_UP)
	//	{
	//		if (m_ViewerCamera.IsEnableFigureCircle(0))
	//		{
	//			if (IsSHIFTpressed()) m_ViewerCamera.SetSizeCircle(0, CPoint(ptStart.x, ptStart.y - 1), ptEnd);
	//			else if (IsCTRLpressed())	 m_ViewerCamera.SetSizeCircle(0, CPoint(ptStart.x, ptStart.y + 1), ptEnd);
	//			else m_ViewerCamera.SetSizeCircle(0, CPoint(ptStart.x, ptStart.y - 1), CPoint(ptEnd.x, ptEnd.y - 1));
	//		}
	//		else
	//		{
	//			if (IsSHIFTpressed()) m_ViewerCamera.SetSizeRectangle(0, CPoint(ptStart.x, ptStart.y - 1), ptEnd);
	//			else if (IsCTRLpressed())	 m_ViewerCamera.SetSizeRectangle(0, CPoint(ptStart.x, ptStart.y + 1), ptEnd);
	//			else m_ViewerCamera.SetSizeRectangle(0, CPoint(ptStart.x, ptStart.y - 1), CPoint(ptEnd.x, ptEnd.y - 1));
	//		}
	//	}
	//	else if (pMsg->wParam == VK_DOWN)
	//	{
	//		if (m_ViewerCamera.IsEnableFigureCircle(0))
	//		{
	//			if (IsSHIFTpressed()) m_ViewerCamera.SetSizeCircle(0, ptStart, CPoint(ptEnd.x, ptEnd.y + 1));
	//			else if (IsCTRLpressed())	 m_ViewerCamera.SetSizeCircle(0, ptStart, CPoint(ptEnd.x, ptEnd.y - 1));
	//			else m_ViewerCamera.SetSizeCircle(0, CPoint(ptStart.x, ptStart.y + 1), CPoint(ptEnd.x, ptEnd.y + 1));
	//		}
	//		else
	//		{
	//			if (IsSHIFTpressed()) m_ViewerCamera.SetSizeRectangle(0, ptStart, CPoint(ptEnd.x, ptEnd.y + 1));
	//			else if (IsCTRLpressed())	 m_ViewerCamera.SetSizeRectangle(0, ptStart, CPoint(ptEnd.x, ptEnd.y - 1));
	//			else m_ViewerCamera.SetSizeRectangle(0, CPoint(ptStart.x, ptStart.y + 1), CPoint(ptEnd.x, ptEnd.y + 1));
	//		}
	//	}
	//	m_ViewerCamera.RedrawWindow();
	//}
#pragma endregion

	return CFormView::PreTranslateMessage(pMsg);
}

LRESULT CFormCameraView::OnViewControl(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case MSG_FCV_SET_MOUSE_POS:
	{
		CString str;
		CPoint pt = m_ViewerCamera.GetMousePoint();

		double wx = 0, wy = 0;
		try {
			m_pMain->GetMachine(0).PixelToWorld(curr_cam, 0, pt.x, pt.y, &wx, &wy, TRUE);  // 알고리즘 선택 해주어야 함
			str.Format("(%04d, %04d) - %03d \n %4.2f, %4.2f", pt.x, pt.y, m_ViewerCamera.GetValueY(pt.x, pt.y), wx, wy);
			m_LbImageInfo.SetText(str);
		}
		catch (...)
		{
			CString str;
			str.Format("x: %d, y: %d - Point Error", pt.x, pt.y);
			AfxMessageBox(str);
		}
	}
	break;
	case MSG_FCV_PATTERN_REGIST:
	{
		_stPatternSelect* pInfo = (_stPatternSelect*)lParam;
		patternRegist(pInfo);
	}
	break;
	case MSG_FCV_PATTERN_SEARCH:
	{
		_stPatternSelect* pInfo = (_stPatternSelect*)lParam;
		patternSearch(pInfo);
	}
	break;
	case MSG_FCV_PATTERN_SET_ROI:
	{
		_stPatternSelect* pInfo = (_stPatternSelect*)lParam;
		patternSetROI(pInfo);
	}
	break;
	case MSG_FCV_PATTERN_DISPLAY_RESULT:
	{
		_stPatternSelect* pInfo = (_stPatternSelect*)lParam;
		patternDisplayResult(pInfo);
	}
	break;
	case MSG_FCV_FOCUS_MEASURE:
	{
		int nCam = lParam;
		dispFocusMeasure(nCam);
	}
	break;
	case MSG_FCV_SET_SEARCH_POS:
	{
		CString str;

		try {
			str.Format("(%4.2f, %4.2f) - (%4.2f, %4.2f)", m_pMain->m_dCurrentSearchPos[0], m_pMain->m_dCurrentSearchPos[1], m_pMain->m_dCurrentSearchPos[2], m_pMain->m_dCurrentSearchPos[3]);
			m_LbImageInfo2.SetText(str);
			str.Format("(%4.2f, %4.2f) - (%4.2f, %4.2f)", m_pMain->m_dCurrentRobotSearchPos[0], m_pMain->m_dCurrentRobotSearchPos[1], m_pMain->m_dCurrentRobotSearchPos[2], m_pMain->m_dCurrentRobotSearchPos[3]);
			m_LbImageInfo3.SetText(str);
		}
		catch (...)
		{

		}
	}
	break;
	case MSG_FCV_SET_CAMERA_POS:
	{
		if (m_bThreadViewDisplayEndFlag == TRUE)
		{
			m_bThreadViewDisplayEndFlag = FALSE;
			WaitForSingleObject(m_pThread->m_hThread, 3000);
		}

		int real_cam = HIWORD(lParam);
		int viewer = LOWORD(lParam);

#ifndef JOB_INFO
		SetViewerInit(num);
#else
		init_camera_viewer(viewer, real_cam);
#endif
		curr_viewer = viewer;
		curr_cam = real_cam;

		updateFrame();
	}
	break;
	case MSG_FCV_CAM_INSP_PROCESSING:
		InspectionProcessing(int(lParam));
		break;
	case MSG_FCV_INSP_SHOW_PROFILE:
	{
		BOOL bCheck = BOOL(lParam);

		if (bCheck)
		{
			if (m_pProfile == NULL)
			{
				m_pProfile = new CDlgProfile();
				m_pProfile->Create(IDD_DIALOG_PROFILE);
				m_pProfile->ShowWindow(SW_SHOW);
				m_pProfile->MoveWindow(800, 200, m_pProfile->m_window_width, m_pProfile->m_window_height);
			}

			//m_ViewerCamera.SetEnableDrawFigure(1, true);
			//m_ViewerCamera.SetEnableFigureLine(1);
			//m_ViewerCamera.SetEnableModifyFigure(1, true);
		}
		else
		{
			if (m_pProfile != NULL)
			{
				m_pProfile->DestroyWindow();
				delete m_pProfile;
				m_pProfile = NULL;
			}

			//m_ViewerCamera.SetEnableDrawFigure(1, false);
			//m_ViewerCamera.SetEnableModifyFigure(1, false);
		}
	}
	break;
	case MSG_FCV_CALC_WHOLE_ROI:
	{
		_stPatternSelect* pInfo = (_stPatternSelect*)lParam;
		patternCalcWholeROI(pInfo);
	}
	break;
	case MSG_FCV_MANUAL_INSPECTION:
	{
		//if (m_pMain->m_nClientType == CLIENT_TYPE_PANEL_EXIST_INSP)
		//{
		//	m_pMain->copyMemory(m_pMain->getIplSrcBuffer()->imageData, m_pMain->getCameraViewBuffer(), m_pMain->m_nImageSize);
		//	m_pMain->GetPanelExist().inspectionPanelExist(m_pMain->getIplSrcBuffer());
		//	SendMessageA(WM_VIEW_CONTROL, MSG_FCV_DISPLAY_PANEL_EXIST_INSP_RESULT, NULL);
		//}
		//else if (m_pMain->m_nClientType == CLIENT_TYPE_ALIGN_EXIST)
		//{
		//	m_pMain->copyMemory(m_pMain->getIplSrcBuffer()->imageData, m_pMain->getCameraViewBuffer(), m_pMain->m_nImageSize);
		//	m_pMain->algorithm_AlignExist5((BYTE*)m_pMain->getIplSrcBuffer()->imageData, TRUE);
		//	SendMessageA(WM_VIEW_CONTROL, MSG_FCV_DISPLAY_PANEL_ALIGN_EXIST, NULL);
		//}
	}
	break;
	case MSG_FCV_SEARCH_CALIPER_INSP:
	{
		_stPatternSelect* pInfo = (_stPatternSelect*)lParam;

		if (m_pMain->vt_job_info[pInfo->nJob].model_info.getAlignInfo().getMarkFindMethod() == METHOD_MATCHING_LINE_THETA)
		{
			patternSearchCaliper_matching_line_theta(pInfo);
		}
		else
		{
			patternSearchCaliper(pInfo);
		}
		m_ViewerCamera.Invalidate();
	}
	break;
	case MSG_FCV_DRAW_CALIPER_INSP:
	{
		draw_CaliperLine(lParam);
	}
	break;
	case MSG_FCV_DRAW_DISTANCE_INSP:
	{
		// 보류
		_stFindPattern* pPattern = (_stFindPattern*)lParam;

		int nJob = pPattern->nJob;

		switch (m_pMain->vt_job_info[nJob].algo_method)
		{
		case CLIENT_TYPE_PANEL_EXIST_INSP:
			drawExistInspResult(lParam);		break;
		case CLIENT_TYPE_PCB_DISTANCE_INSP:
		case CLIENT_TYPE_ASSEMBLE_INSP:
		{
			if (m_pMain->vt_job_info[nJob].model_info.getGlassInfo().getModelPanelType() == _CORE_MODEL_CALC)
			{
				drawAssembleCALC(lParam);
			}
			else
			{
				drawDistanceInspResult2(lParam);
			}
		}
		break;
		}
	}
	break;
	case MSG_FCV_DRAW_REFERNCE_POS:
	{
		drawReference(lParam);
	}
	break;
	case MSG_FCV_DRAW_ALL_REFERNCE_POS: //2022-03-11 hsj All reference 등록 그리기 
	{
		drawaAllReference(lParam);
	}
	break;
	case MSG_FCV_DRAW_FIXTURE_POS:
	{
		drawFixture(lParam);
	}
	break;
	case MSG_FCV_INSP_NOTCH_DOPO:
	{
		int nCam = LOWORD(lParam);
		int nJob = HIWORD(lParam);
		int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
		int W = m_pMain->m_stCamInfo[real_cam].w;
		int H = m_pMain->m_stCamInfo[real_cam].h;
		CPoint ptStart = m_ViewerCamera.GetPointBegin(0);
		CPoint ptEnd = m_ViewerCamera.GetPointEnd(0);
		CFormMainView* pFormMain = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];

		int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();

		if (method == METHOD_LINE)
		{
			cv::Mat img(H, W, CV_8UC1, m_pMain->getCameraViewBuffer());
			double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(nCam, 0);
			double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(nCam, 0);

			int x = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeX() / xres / 2; //  중심점 기준 좌우 분리
			int y = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeY() / yres / 2;  //  중심점 기준 상하 분리
			double l = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeLength(); //도포 길이
			int cx = (ptStart.x + ptEnd.x) / 2, cy = (ptStart.y + ptEnd.y) / 2;
#ifdef _UT_LINE
			pFormMain->InspLineEdgeDetection(nJob, img, CPoint(cx - x, cy - y), CPoint(cx + x, cy + y), l, &m_ViewerCamera, true, true);
#else
			CFileDialog dlg(TRUE, NULL, _T("Image File"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
				_T("Image Files (*.bmp;*.jpg;)|*.bmp;*.jpg|All Files (*.*)|*.*||"), this);

			if (dlg.DoModal() == IDOK)
			{
				if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getELBCG_Edge_CalcModeEnable())
				{
					CFileDialog dlg2(TRUE, NULL, _T("Image File"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
						_T("Image Files (*.bmp;*.jpg;)|*.bmp;*.jpg|All Files (*.*)|*.*||"), this);
					cv::Mat loadImg2 = cv::imread((LPCTSTR)dlg2.GetPathName(), 0);
					pFormMain->InspLineCGDetection(nJob, nCam, &loadImg2, &m_ViewerCamera);
					loadImg2.release();
				}

				cv::Mat loadImg = cv::imread((LPCTSTR)dlg.GetPathName(), 0);
				m_pMain->copyMemory(m_pMain->getProcBuffer(nCam, 2), loadImg.data, W * H);
				pFormMain->fnDiffWetOutInspection(nJob, 1, &m_ViewerCamera, TRUE);
				if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getUseDustInsp())
					pFormMain->fnDiffDustInspection(nJob, 1, &m_ViewerCamera);
			}
#endif
		}
		else
		{
			CFileDialog dlg(TRUE, NULL, _T("Image File"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
				_T("Image Files (*.bmp;*.jpg;)|*.bmp;*.jpg|All Files (*.*)|*.*||"), this);

			if (dlg.DoModal() != IDOK)
			{
				pFormMain->fnExtractProfileData(nJob, &m_ViewerCamera);
				break;
			}
			cv::Mat loadImg = cv::imread((LPCTSTR)dlg.GetPathName(), 0);
			m_pMain->copyMemory(m_pMain->getProcBuffer(nCam, 2), loadImg.data, W * H);
			pFormMain->fnDiffWetOutInspection(nJob, 1, &m_ViewerCamera, TRUE);
			if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getUseDustInsp())
				pFormMain->fnDiffDustInspection(nJob, 1, &m_ViewerCamera);
		}

	}
	break;
	case MSG_FCV_INSP_NOTCH:
	{
		int nCam = LOWORD(lParam);
		int nJob = HIWORD(lParam);
		int W = m_pMain->m_stCamInfo[nCam].w;
		int H = m_pMain->m_stCamInfo[nCam].h;
		CFormMainView* pFormMain = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];
		/////////////////
		CRect rect_roi = m_pMain->GetMatching(nJob).getSearchROI(nCam, 0);
		CPoint ptStart(rect_roi.left, rect_roi.top);
		CPoint ptEnd(rect_roi.right, rect_roi.bottom);

		TCHAR szValue[MAX_PATH] = { 0, };
		m_pMain->m_iniFile.GetProfileStringA("SERVER_INFO", "NOTCH_LEFT_DISTANCE_FROM_CENTER", "12.7", szValue, MAX_PATH);
		m_pMain->notch_left_distance_from_center = atof(szValue);
		m_pMain->m_iniFile.GetProfileStringA("SERVER_INFO", "NOTCH_RIGHT_DISTANCE_FROM_CENTER", "1.2", szValue, MAX_PATH);
		m_pMain->notch_right_distance_from_center = atof(szValue);

		int org_pos = m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getAlignOrginPos(nCam);
		int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

		double res_x = m_pMain->GetMachine(nJob).getCameraResolutionX(nCam, 0);
		W = m_pMain->m_stCamInfo[real_cam].w;
		H = m_pMain->m_stCamInfo[real_cam].h;
		double xr = 0.5;
		double yr = 0.5;

		switch (org_pos) {
		case _LEFT_TOP_X25_Y25:  xr = 0.25; yr = 0.25; break;
		case _CENTER_TOP_X50_Y25:  xr = 0.5; yr = 0.25; break;
		case _RIGHT_TOP_X75_Y25:  xr = 0.75; yr = 0.25; break;
		case _LEFT_CENTER_X25_Y50:  xr = 0.25; yr = 0.5; break;
		case _CENTER_CENTER_X50_Y50:  xr = 0.5; yr = 0.5; break;
		case _RIGHT_CENTER_X75_Y50:  xr = 0.75; yr = 0.5; break;
		case _LEFT_BOTTOM_X25_Y75:  xr = 0.25; yr = 0.75; break;
		case _CENTER_BOTTOM_X50_Y75:  xr = 0.5; yr = 0.75; break;
		case _RIGHT_BOTTOM_X75_Y75:  xr = 0.75; yr = 0.75; break;
		case _NOTCH_X85_Y50:		xr = 0.85; yr = 0.5; break;
		}

		if (org_pos == _NOTCH_X85_Y50)
		{
			ptStart.x = W * xr - (m_pMain->notch_left_distance_from_center / res_x);
			ptEnd.x = W * xr + (m_pMain->notch_right_distance_from_center / res_x);
			///////////////

			if (ptStart.x < 0)	ptStart.x = 1;
			if (ptEnd.x >= W)	ptEnd.x = W - 1;
		}
		else
		{
			ptStart = m_ViewerCamera.GetPointBegin(0);
			ptEnd = m_ViewerCamera.GetPointEnd(0);
		}

		if (ptStart.x > 0 && ptStart.y > 0 && ptEnd.x - ptStart.x > 0 && ptEnd.y - ptStart.y > 0)
		{
			memcpy(m_pMain->getProcBuffer(nCam, 0), m_pMain->getCameraViewBuffer(), W * H);
			memcpy(m_pMain->getProcBuffer(nCam, 1), m_pMain->getCameraViewBuffer(), W * H);

			int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();
			if (method == METHOD_LINE)
			{
				cv::Mat img(H, W, CV_8UC1, m_pMain->getProcBuffer(nCam, nJob));
				double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(nCam, 0);
				double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(nCam, 0);

				int x = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeX() / xres / 2; //  중심점 기준 좌우 분리
				int y = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeY() / yres / 2;  //  중심점 기준 상하 분리
				double l = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeLength(); //도포 길이
				int cx = (ptStart.x + ptEnd.x) / 2, cy = (ptStart.y + ptEnd.y) / 2;

				//pFormMain->InspLineEdgeDetection(nJob, img, CPoint(cx - x, cy - y), CPoint(cx + x, cy + y), l, &m_ViewerCamera, false);
				pFormMain->fnExtractProfileData(nJob, &m_ViewerCamera, false);
				if (!m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getTraceUsingAlignFlag())
				{
#ifndef _UT_LINE
					pFormMain->InspLineEdgeDetection(nJob, img, CPoint(cx - x, cy - y), CPoint(cx + x, cy + y), l, &m_ViewerCamera, true, true); // Trace
#endif
				}
			}
			else if (method == METHOD_CIRCLE)
			{
				m_pMain->m_strResultTime[nJob] = "";
				m_pMain->m_strResultDate[nJob] = "";
				cv::Mat img(H, W, CV_8UC1, m_pMain->getProcBuffer(nCam, nJob));
				pFormMain->InspCircleEdgeDetection(nJob, img, ptStart, ptEnd, &m_ViewerCamera);
				pFormMain->fnExtractProfileData(nJob, &m_ViewerCamera, false);

				img.release();
			}
			else if (method == METHOD_NOTCH)
			{
				pFormMain->InspNotchEdgeDetection(true, false, nJob, nCam, ptStart, ptEnd, &m_ViewerCamera);
			}
			else if (method == METHOD_DIFF_INSP)  //Center Align Camera 검사
			{
				CFileDialog dlg(TRUE, NULL, _T("Image File"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					_T("Image Files (*.bmp;*.jpg;)|*.bmp;*.jpg|All Files (*.*)|*.*||"), this);

				if (dlg.DoModal() != IDOK) 			break;

				cv::Mat loadImg = cv::imread((LPCTSTR)dlg.GetPathName(), 0);

				m_pMain->copyMemory(m_pMain->getProcBuffer(nCam, 2), loadImg.data, W * H);
				pFormMain->fnDiffWetOutInspection(nJob, 1, &m_ViewerCamera, TRUE);
				if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getUseDustInsp())
					pFormMain->fnDiffDustInspection(nJob, 1, &m_ViewerCamera);
			}
			else if (method == METHOD_NOTCH_LINE)
			{
				double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(nCam, 0);
				double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(nCam, 0);
				int x = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeX() / xres / 2; //  중심점 기준 좌우 분리
				int y = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeY() / yres / 2;  //  중심점 기준 상하 분리
				double l = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeLength(); //도포 길이
				int cx = W / 2, cy = H / 2;

				pFormMain->InspNotchEdgeDetection(true, false, nJob, nCam, CPoint(cx - x, cy - y), CPoint(cx + x, cy + y), &m_ViewerCamera, true);
			}
			else if (method == METHOD_NOTCH_INSP)
			{
				//20210927 Tkyuha 노즐 높이 측정 테스트용
				//KJH2 2022-08-17 Panel Gray -> Nozzle YGapThreshold 값으로 측정
				int _thresh = m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getNozzleYGapThreshold();
				//if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseMetalTraceFirst() == FALSE)
				// KJH2 2022-08-17 Metal_Trace_First -> Y Light Enable/Disalbe 으로 판단 
				pFormMain->fnCalcNozzleToPN_Distance2(m_pMain->getProcBuffer(nCam, nJob), W, H, nJob, nCam, _thresh, &m_ViewerCamera);
			}
			else if (method == METHOD_HOLE_INSP)
			{
				cv::Mat img(H, W, CV_8UC1, m_pMain->getProcBuffer(nCam, nJob));
				//pFormMain->InspCircleDummyDetection(nJob, nCam, &img, &m_ViewerCamera);
				pFormMain->InspLineDummyDetection(nJob, nCam, &img, &m_ViewerCamera);
			}
			else if (method == METHOD_UT_ELB)
			{
				pFormMain->InspUT_EdgeDetection(false, true, nJob, nCam, ptStart, ptEnd, &m_ViewerCamera, true);
			}
		}
	}
	break;
	case MSG_FCV_INSP_SCAN:
	{
		int nCam = LOWORD(lParam);
		int nJob = HIWORD(lParam);
		int W = m_ViewerCamera.GetWidth();
		int H = m_ViewerCamera.GetHeight();

		CFormMainView* pFormMain = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];
		CPoint ptStart = m_ViewerCamera.GetPointBegin(0);
		CPoint ptEnd = m_ViewerCamera.GetPointEnd(0);

		cv::Mat matImage(H, W, CV_8UC1, m_ViewerCamera.GetImagePtr());
		cv::Mat flipImg = matImage.clone();
		//flip(flipImg, flipImg, 0);

		if (H / W < 3 && m_pMain->m_nSeqScanInspGrabCount[nJob] == 1) break;
		m_pMain->m_pSaperaInspWorker[nCam].algorithm_WetOut_OnesInsp(flipImg.data, nJob, nCam, W, H, &m_ViewerCamera);

		////////// 테스트 디스플레이
		_stAlignResult Info;
		int nMethod = m_pMain->vt_job_info[nJob].algo_method;
		int camCount = m_pMain->vt_job_info[nJob].num_of_camera;
		std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;

		Info.nCam = camBuf[0];
		Info.nPos = 0;
		Info.nViewer = Info.nCam * 3;
		Info.nAlgorithm = nJob;
		Info.bErase = TRUE;
		::SendMessageA(m_pMain->m_pForm[FORM_MAIN]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_DISPLAY_SCAN_INSP, (LPARAM)&Info);

	}
	break;
	case MSG_FCV_INSP_NOTCH_NOZZLE:
	{
		int nCam = LOWORD(lParam);
		int nJob = HIWORD(lParam);
		int W = m_pMain->m_stCamInfo[nCam].w;
		int H = m_pMain->m_stCamInfo[nCam].h;
		CFormMainView* pFormMain = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];
		cv::Point2f matchPt;
		
		bool _calcMethod = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getCenterNozzleAlignMethod();
		if (_calcMethod)
			pFormMain->InspNozzleSearchCaliper_Use(m_pMain->getCameraViewBuffer(), W, H, nJob, nCam, matchPt, false, &m_ViewerCamera);
		else pFormMain->InspNozzleSearchInspection(m_pMain->getCameraViewBuffer(), W, H, nJob, nCam, &m_ViewerCamera);
	}
	break;
	case MSG_FCV_INSP_NOTCH_NOZZLE_ANGLE:
	{
		int nCam = LOWORD(lParam);
		int nJob = HIWORD(lParam);
		int W = m_pMain->m_stCamInfo[nCam].w;
		int H = m_pMain->m_stCamInfo[nCam].h;
		CFormMainView* pFormMain = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];

		pFormMain->InspNozzleSearchAngle_Bee(m_pMain->getCameraViewBuffer(), W, H, nJob, nCam, &m_ViewerCamera);
	}
	break;
	case MSG_FCV_INSP_NOZZLE_GAP:
	{
		int nCam = LOWORD(lParam);
		int nJob = HIWORD(lParam);
		int W = m_pMain->m_stCamInfo[nCam].w;
		int H = m_pMain->m_stCamInfo[nCam].h;
		CFormMainView* pFormMain = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];
		pFormMain->fnCalcNozzleGap_Distance(m_pMain->getProcBuffer(nCam, 0), W, H, nJob, nCam, 0, &m_ViewerCamera);
	}
	break;
	case MSG_FCV_INSP_NOZZLE_DISTANCE_POLAR:
	{
		int nCam = LOWORD(lParam);
		int nJob = HIWORD(lParam);
		int W = m_pMain->m_stCamInfo[nCam].w;
		int H = m_pMain->m_stCamInfo[nCam].h;
		CFormMainView* pFormMain = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];

		//KJH2 2022-08-17 Panel Gray -> Nozzle YGapThreshold 값으로 측정
		int _thresh = m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getNozzleYGapThreshold();

		//if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseMetalTraceFirst() == FALSE)
		pFormMain->fnCalcNozzleToPN_Distance2(m_pMain->getCameraViewBuffer(), W, H, nJob, nCam, _thresh, &m_ViewerCamera);
	}
	break;
	case MSG_FCV_INSP_NOZZLE_VIEW:
	{
		int nCam = LOWORD(lParam);
		int nJob = HIWORD(lParam);
		int W = m_pMain->m_stCamInfo[nCam].w;
		int H = m_pMain->m_stCamInfo[nCam].h;
		CFormMainView* pFormMain = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];

		int _thresh = 200;

		pFormMain->fnCalcNozzleViewPos(m_pMain->getCameraViewBuffer(), W, H, nJob, nCam, _thresh, &m_ViewerCamera);
	}
	break;
	case MSG_FCV_INSP_NOTCH_NOZZLE_ANGLE_REVERSE:
	{
		int nCam = LOWORD(lParam);
		int nJob = HIWORD(lParam);
		int W = m_pMain->m_stCamInfo[nCam].w;
		int H = m_pMain->m_stCamInfo[nCam].h;
		CFormMainView* pFormMain = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];
		if (nCam == 0)
		{
			pFormMain->InspNozzleSearchAngle_Bee(m_pMain->getCameraViewBuffer(), W, H, nJob, nCam, &m_ViewerCamera, TRUE);
		}
		else
		{
			pFormMain->InspNozzleSearchAngle_Bee(m_pMain->getCameraViewBuffer(), W, H, nJob, nCam, &m_ViewerCamera);
		}
	}
	break;
	case MSG_FCV_INSP_NOZZLE_DISTANCE_REVERSE_CENTER:
	{
		int nCam = LOWORD(lParam);
		int nJob = HIWORD(lParam);
		int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
		int W = m_pMain->m_stCamInfo[real_cam].w;
		int H = m_pMain->m_stCamInfo[real_cam].h;
		CFormMainView* pFormMain = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];

		cv::Mat img(H, W, CV_8UC1, m_pMain->getCameraViewBuffer());
		//KJH 2022-01-19 White Nozzle Search
		pFormMain->InspNozzleDistance_align(nJob, nCam, FALSE, FALSE, img, &m_ViewerCamera);

		img.release();
	}
	break;
	case MSG_FCV_INSP_NOZZLE_DISTANCE_REVERSE_CENTER_REF:
	{
		int nCam = LOWORD(lParam);
		int nJob = HIWORD(lParam);
		int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
		int W = m_pMain->m_stCamInfo[real_cam].w;
		int H = m_pMain->m_stCamInfo[real_cam].h;
		CFormMainView* pFormMain = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];

		cv::Mat img(H, W, CV_8UC1, m_pMain->getCameraViewBuffer());
		//KJH 2022-01-19 White Nozzle Search
		pFormMain->InspNozzleDistance_align(nJob, nCam, FALSE, TRUE, img, &m_ViewerCamera);

		img.release();
	}
	break;
	case MSG_FCV_INSP_NOZZLE_DISTANCE_REVERSE_SIDE:
	{
		int nCam = LOWORD(lParam);
		int nJob = HIWORD(lParam);
		int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
		int W = m_pMain->m_stCamInfo[real_cam].w;
		int H = m_pMain->m_stCamInfo[real_cam].h;
		CFormMainView* pFormMain = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];

		cv::Mat img(H, W, CV_8UC1, m_pMain->getCameraViewBuffer());
		//KJH 2022-01-19 Black Nozzle Search
		pFormMain->InspNozzleDistance_align(nJob, nCam, TRUE, FALSE, img, &m_ViewerCamera);

		img.release();
	}
	break;
	case MSG_FCV_INSP_NOZZLE_DISTANCE_REVERSE_SIDE_REF:
	{
		int nCam = LOWORD(lParam);
		int nJob = HIWORD(lParam);
		int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
		int W = m_pMain->m_stCamInfo[real_cam].w;
		int H = m_pMain->m_stCamInfo[real_cam].h;
		CFormMainView* pFormMain = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];

		cv::Mat img(H, W, CV_8UC1, m_pMain->getCameraViewBuffer());
		//KJH 2022-01-19 Black Nozzle Search
		pFormMain->InspNozzleDistance_align(nJob, nCam, TRUE, TRUE, img, &m_ViewerCamera);

		img.release();
	}
	break;
	case MSG_FCV_OPEN_IMAGE_FILENAME:
	{
		CString str;
		str.Format("Camera View\r\n (%s)", (*(CString*)lParam));
		m_LbCameraViewTitle.SetText(str);
	}
	break;
	case MSG_FCV_INSP_NOZZLE_Z_HEIGHT:
	{
		int nCam = LOWORD(lParam);
		int nJob = HIWORD(lParam);
		int W = m_pMain->m_stCamInfo[nCam].w;
		int H = m_pMain->m_stCamInfo[nCam].h;
		CFormMainView* pFormMain = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];

		cv::Mat img(H, W, CV_8UC1, m_pMain->getCameraViewBuffer());

		pFormMain->fnSimulation_CalcNozzleGapInsp(nJob, nCam, img, &m_ViewerCamera);
		img.release();
	}
	break;
	//HTK 2022-06-17 Nozzle XY 검사중 다시 한번 Y만 정밀 Search, 검증후 적용 예정
	case MSG_FCV_INSP_CENTER_NOZZLE_Y_CALC:
	{
		int nCam = LOWORD(lParam);
		int nJob = HIWORD(lParam);
		int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
		int W = m_pMain->m_stCamInfo[real_cam].w;
		int H = m_pMain->m_stCamInfo[real_cam].h;
		CFormMainView* pFormMain = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];

		cv::Mat img(H, W, CV_8UC1, m_pMain->getCameraViewBuffer());

		bool _recalc = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getCenterNozzleYAlignRecalc();

		if (m_pMain->GetMatching(nJob).getFindInfo(nCam, 0).GetFound() == FIND_OK && _recalc)
		{
			CFindInfo find = m_pMain->GetMatching(nJob).getFindInfo(nCam, 0);
			double posX = find.GetXPos();
			double posY = find.GetYPos();
			double rv = m_pMain->reCalculateMarkYpos(img.data, W, H, posX, posY);
			if (rv > 0 && fabs(rv - posY) > 2.0)  m_pMain->GetMatching(nJob).getFindInfo(nCam, 0).SetYPos(rv);
		}
		img.release();
	}
	break;
	//HTK 2022-07-12 Nozzel#45에 약액상태 검사 추가
	case MSG_FCV_INSP_INK_INSPECTION:
	{
		int nCam = LOWORD(lParam);
		int nJob = HIWORD(lParam);
		int real_cam = m_pMain->vt_job_info[nJob].camera_index[0];
		int W = m_pMain->m_stCamInfo[real_cam].w;
		int H = m_pMain->m_stCamInfo[real_cam].h;
		CString strTemp;

		CRect realRoi = m_pMain->GetMatching(nJob).getSearchROI(0, 0);

		if (realRoi.Width() > 0 && realRoi.Width() < W && realRoi.Height() > 0 && realRoi.Height() < H)
		{
			cv::Mat img(H, W, CV_8UC1, m_pMain->getCameraViewBuffer());

			double hthresh = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().get45DegreeInkInspThresh();
			cv::Mat InspImg = img(cv::Rect(realRoi.left, realRoi.top, realRoi.Width(), realRoi.Height()));
			cv::Scalar scalar = cv::mean(InspImg);

			InspImg.release();
			img.release();

			strTemp.Format("[%s] Find Gray= %.3f", m_pMain->vt_job_info[nJob].job_name.c_str(), scalar.val[0]);
			m_LbCameraViewTitle.SetText(strTemp);
		}
	}
	break;
	case MSG_FCV_INSP_SCRATCH_INSPECTION:
	{
		int nCam = LOWORD(lParam);
		int nJob = HIWORD(lParam);

			CFormMainView* pFormMain = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];
			pFormMain->Inspection_ScratchHole(nJob, 0, m_pMain->getCameraViewBuffer(), &m_ViewerCamera, true); //20220929 Tkyuha
	}
	break;
	}

	return 0;
}

void CFormCameraView::InspectionProcessing(int id)
{
	int W = m_pMain->m_stCamInfo[curr_cam].w;
	int H = m_pMain->m_stCamInfo[curr_cam].h;

	cv::Mat sgray(H, W, CV_8UC1, m_pMain->getCameraViewBuffer());
	cv::Mat iproc;

	switch (id)
	{
	case 0://Gaussian
		GaussianBlur(sgray, iproc, cv::Size(0, 0), 8.);
		m_ViewerCamera.OnLoadImageFromPtr((BYTE *)iproc.data);
		break;
	case 1: //SobelLeft
	{
		IplImage *pOrgImage = cvCreateImage(cvSize(W, H), 8, 1);
		memcpy(pOrgImage->imageData, sgray.data, W*H);
		m_pMain->m_ManualAlignMeasure.sobelDirection(3, SOBEL_LEFT, pOrgImage, pOrgImage);
		m_ViewerCamera.OnLoadImageFromPtr((BYTE *)pOrgImage->imageData);
		cvReleaseImage(&pOrgImage);
	}
	break;
	case 2: //SobelRight
	{
		IplImage *pOrgImage = cvCreateImage(cvSize(W, H), 8, 1);
		memcpy(pOrgImage->imageData, sgray.data, W*H);
		m_pMain->m_ManualAlignMeasure.sobelDirection(3, SOBEL_RIGHT, pOrgImage, pOrgImage);
		m_ViewerCamera.OnLoadImageFromPtr((BYTE *)pOrgImage->imageData);
		cvReleaseImage(&pOrgImage);
	}
	break;
	case 3://SobelDown
	{
		IplImage *pOrgImage = cvCreateImage(cvSize(W, H), 8, 1);
		memcpy(pOrgImage->imageData, sgray.data, W*H);
		m_pMain->m_ManualAlignMeasure.sobelDirection(3, SOBEL_DN, pOrgImage, pOrgImage);
		m_ViewerCamera.OnLoadImageFromPtr((BYTE *)pOrgImage->imageData);
		cvReleaseImage(&pOrgImage);
	}
	break;
	case 4: //SobelUp
	{
		IplImage *pOrgImage = cvCreateImage(cvSize(W, H), 8, 1);
		memcpy(pOrgImage->imageData, sgray.data, W*H);
		m_pMain->m_ManualAlignMeasure.sobelDirection(3, SOBEL_UP, pOrgImage, pOrgImage);
		m_ViewerCamera.OnLoadImageFromPtr((BYTE *)pOrgImage->imageData);
		cvReleaseImage(&pOrgImage);
	}
	break;
	case 5: //Enhance			
		GaussianBlur(sgray, iproc, cv::Size(0, 0), 3.);
		m_pMain->m_ManualAlignMeasure.InspectionEnhance(&iproc, 0, false);
		m_ViewerCamera.OnLoadImageFromPtr((BYTE *)iproc.data);
		break;
	case 6: //EnhanceX
		GaussianBlur(sgray, iproc, cv::Size(0, 0), 3.);
		m_pMain->m_ManualAlignMeasure.InspectionEnhance(&iproc, 2, false);
		m_ViewerCamera.OnLoadImageFromPtr((BYTE *)iproc.data);
		break;
	case 7: //EnhanceY
		GaussianBlur(sgray, iproc, cv::Size(0, 0), 3.);
		m_pMain->m_ManualAlignMeasure.InspectionEnhance(&iproc, 1, false);
		m_ViewerCamera.OnLoadImageFromPtr((BYTE *)iproc.data);
		break;
	case 8: //RemoveNoise
		m_pMain->m_ManualAlignMeasure.removeNoise(sgray, 10);
		m_ViewerCamera.OnLoadImageFromPtr((BYTE *)sgray.data);
		break;
	case 9: //CannyEdge
	{
		double sigma = 0.33;
		cv::Scalar v = mean(sgray);

		int lower = int(MAX(0, (1.0 - sigma) * v.val[0]));
		int upper = int(MAX(0, (1.0 - sigma) * v.val[0]));
		
		Canny(sgray, iproc, lower, upper);
		m_ViewerCamera.OnLoadImageFromPtr((BYTE*)iproc.data);
	}
		break;
	case 10: //Thresh
		cv::threshold(sgray, iproc, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
		m_ViewerCamera.OnLoadImageFromPtr((BYTE *)iproc.data);
		break;
	case 11: //Flip
		cv::flip(sgray, iproc, 0);
		m_ViewerCamera.OnLoadImageFromPtr((BYTE *)iproc.data);
		break;
	case 12: //Orgin
		if(m_ViewerCamera.GetBPP()!=8)	m_ViewerCamera.OnInitWithCamera(sgray.cols, sgray.rows, 8);
		m_ViewerCamera.OnLoadImageFromPtr((BYTE *)m_pMain->getCameraViewBuffer());
		break;
	case 13: //rotate
	{
		CString strTemp;
		strTemp = m_pMain->GetNumberBox("45.0", 5, -360, 360);
		cv::Point2f pt(sgray.cols / 2.f, sgray.rows / 2.f);
		cv::Mat r = getRotationMatrix2D(pt, atof(strTemp), 1.0);
		cv::warpAffine(sgray, iproc, r, cv::Size(sgray.cols, sgray.rows));

		m_ViewerCamera.OnLoadImageFromPtr((BYTE *)iproc.data);
	}
	break;
	case 14: //thresholding
	{
		CString strTemp;
		strTemp = m_pMain->GetNumberBox("50", 5, 0, 255);
		cv::threshold(sgray, iproc, atoi(strTemp), 255, CV_THRESH_BINARY);
		m_ViewerCamera.OnLoadImageFromPtr((BYTE *)iproc.data);
	}
	break;
	case 15: //save image
		m_ViewerCamera.OnSaveImage();
		break;
	case 16: //cartToPolar
		{
			int aperture = 3;
			cv::Mat sobelX,sobelY;
			cv::Mat sobelMagnitude, sobelOrientation;
			cv::resize(sgray, iproc,cv::Size(), 0.5, 0.5, CV_INTER_CUBIC);
			//sgray.convertTo(iproc, CV_32F, 1 / 255.0);

			cv::Sobel(iproc, sobelX, CV_32F, 1, 0, aperture);
			cv::Sobel(iproc, sobelY, CV_32F, 0, 1, aperture);
	
			cv::cartToPolar(sobelX, sobelY, sobelMagnitude, sobelOrientation,true);
			displayCartToPolar(sobelMagnitude, sobelOrientation);

			normalize(sobelMagnitude, iproc,0, 255, NORM_MINMAX,CV_8UC1);
			cv::resize(iproc, iproc, cv::Size(), 2, 2, CV_INTER_CUBIC);
			m_ViewerCamera.OnLoadImageFromPtr((BYTE*)iproc.data);

			sobelX.release();
			sobelY.release();
			sobelMagnitude.release();
			sobelOrientation.release();
		}
		break;
	case 17://  bliateralFilter
	{
		CString strTemp;
		strTemp = m_pMain->GetNumberBox("10", 5, 0, 255);
		int sigmaColor = atoi(strTemp);

		cv::bilateralFilter(sgray, iproc, -1, sigmaColor, sigmaColor/2);
		m_ViewerCamera.OnLoadImageFromPtr((BYTE*)iproc.data);
	}
	break;
	case 18: //Histogram  // 20211012 Tkyuha 
	{
		CPoint ptStart = m_ViewerCamera.GetPointBegin(0);
		CPoint ptEnd = m_ViewerCamera.GetPointEnd(0);
		if (ptStart.x > 0 && ptStart.y > 0 && ptEnd.x - ptStart.x > 0 && ptEnd.y - ptStart.y > 0)
		{
			sgray(cv::Rect(ptStart.x, ptStart.y, abs(ptStart.x - ptEnd.x), abs(ptStart.y - ptEnd.y))).copyTo(iproc);
			Histogram1D h; // 히스토그램을 위한 객체 							 
			// 히스토그램을 영상으로 띄우기 
			HWND hwnd = ::FindWindow(NULL, "Histogram"); 			

			if (NULL != hwnd)	::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			else	cv::namedWindow("Histogram"); 
					
			cv::imshow("Histogram", h.getHistogramImage(iproc, 2));
		}
	}
	break;
	case 19: //AdaptiveThreshold  // 20211020 Tkyuha 
	{
		CString strTemp;
		strTemp = m_pMain->GetNumberBox("5", 5, 0, 255);
		if (atoi(strTemp) % 2 == 0)
		{
			AfxMessageBox("Use odd numbers(3,5,7,9)");
		}
		else
		{
			cv::adaptiveThreshold(sgray, iproc, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, atoi(strTemp), 5);
			m_ViewerCamera.OnLoadImageFromPtr((BYTE*)iproc.data);
		}
	}
	break;
	case 20: //Inv thresholding
	{
		CString strTemp;
		strTemp = m_pMain->GetNumberBox("50", 5, 0, 255);
		cv::threshold(sgray, iproc, atoi(strTemp), 255, CV_THRESH_BINARY_INV);
		m_ViewerCamera.OnLoadImageFromPtr((BYTE*)iproc.data);
	}
	break;
	case 21: //pyrMeanShiftFiltering
	{
		CString strTemp;
		strTemp = m_pMain->GetNumberBox("20", 5, 1, 255);

		cv::cvtColor(sgray, iproc, COLOR_GRAY2BGR);
		cv::pyrMeanShiftFiltering(iproc, iproc, 5, atoi(strTemp), 2);
		cv::cvtColor(iproc, iproc, COLOR_BGR2GRAY);
		m_ViewerCamera.OnLoadImageFromPtr((BYTE*)iproc.data);
	}
	break;
	}

	m_ViewerCamera.Invalidate();
	iproc.release();
	sgray.release();
}

void CFormCameraView::dispFocusMeasure(int n)
{
	CPoint ptStart = m_ViewerCamera.GetPointBegin(0);
	CPoint ptEnd = m_ViewerCamera.GetPointEnd(0);
	int W = m_pMain->m_stCamInfo[curr_cam].w;
	int H = m_pMain->m_stCamInfo[curr_cam].h;

	CRect rect;
	rect.left = MIN(ptStart.x, ptEnd.x);
	rect.right = MIN(MAX(ptStart.x, ptEnd.x),W);
	rect.top = MIN(ptStart.y, ptEnd.y);
	rect.bottom = MIN(MAX(ptStart.y, ptEnd.y),H);

	if (rect.Width() <= 0 || rect.Height() <= 0)
	{
		m_pMain->m_bFocusMeasure = FALSE;
		AfxMessageBox("Draw Area First!!!!");
		return;
	}	
	//KJH 2022-03-19 n -> curr_Cam으로 인자 변경
	m_pMain->copyMemory(m_pMain->getProcBuffer(curr_cam, 0), m_pMain->getCameraViewBuffer(), W*H);

	cv::Mat img(H, W, CV_8UC1, m_pMain->getProcBuffer(curr_cam, 0));
	cv::Rect rectROI;

	rectROI.x = MAX(0,rect.left);
	rectROI.y = MAX(0, rect.top);
	rectROI.width = MIN(rect.Width(),W- rectROI.x);
	rectROI.height = MIN(rect.Height(),H- rectROI.y);


	cv::Scalar data = cv::mean(img(rectROI));
	m_pMain->m_dbAvgGray = data[0];
	//20211229
	m_pMain->m_dbFocusEnergy = GetFocusMeasure(m_pMain->getProcBuffer(curr_cam, 0), W, H, rect);
	//m_pMain->m_dbFocusEnergy = ImageSharpness(img(rectROI).data, rect);
}

double CFormCameraView::GetFocusMeasure(BYTE *pImg, int w, int h, CRect roi)
{
	cv::Mat Img_Proc_tmp;
	cv::Mat Img_src(h, w, CV_8UC1, pImg);

	cv::GaussianBlur(Img_src, Img_Proc_tmp, cv::Size(3, 3), 0, 0, cv::BORDER_DEFAULT);

	BYTE *pBuffer = Img_Proc_tmp.data;

	double energy = 0.0;  // Focus Measure Energy..
	double tmp_Buffer;

	for (int row = roi.top + 4; row < roi.bottom - 4; row++)
	{
		for (int col = roi.left + 4; col < roi.right - 4; col++)
		{
			tmp_Buffer = pBuffer[(row * w) + (col - 2)] + (-2 * pBuffer[(row * w) + col]) + pBuffer[(row * w) + (col + 2)];
			energy += tmp_Buffer * tmp_Buffer;
		}
	}

	return sqrt(energy);
}

//20211229
double CFormCameraView::ImageSharpness(BYTE* img,CRect roi)
{
	int w = roi.Width();
	int h = roi.Height();

	const int npixs = w * h;
	const int xend = w - 1, yend = h - 1;
	const int nn[] = { -w - 1, -w, -w + 1, -1, 0, 1, w - 1, w , w + 1 };
	const int sobelX[] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
	const int sobelY[] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 };
	//median filter;
	BYTE arr[9];
	BYTE* medimg = new BYTE[npixs];
	memset(medimg, 0, npixs);
	for (int y = 1, pos = y * w; y < yend; y++) {
		pos++;
		for (int x = 1; x < xend; x++, pos++) {
			for (int k = 0; k < 9; k++) arr[k] = img[pos + nn[k]];
			medimg[pos] = opt_med9(arr);
		}
		pos++;
	}
	// Tenenbaum gradient;
	double sharpness = 0;
	for (int y = 1, pos = y * w; y < yend; y++) {
		pos++;
		for (int x = 1; x < xend; x++, pos++) {
			double gx = 0, gy = 0;
			for (int k = 0; k < 9; k++) {
				int v = medimg[pos + nn[k]];
				gx += sobelX[k] * v;
				gy += sobelY[k] * v;
			}
			sharpness += gx * gx + gy * gy;
		}
		pos++;
	}
	delete[] medimg;

	return sqrt(sharpness);
}

void CFormCameraView::updateFrame()
{
	if(m_bThreadViewDisplayEndFlag == FALSE)
		m_pThread = AfxBeginThread(Thread_ViewerDisplay, this);

	this->SetFocus();
}


void CFormCameraView::init_camera_viewer(int viewer, int real_cam)
{
	SetViewerNum(viewer);
	
	int W = m_pMain->m_stCamInfo[real_cam].w;
	int H = m_pMain->m_stCamInfo[real_cam].h;

	if (m_pMain->m_pCameraViewImage != NULL)
	{
		delete[] m_pMain->m_pCameraViewImage;
		m_pMain->m_pCameraViewImage = NULL;
	}

	m_pMain->m_pCameraViewImage = new BYTE[W * H];
	memset(m_pMain->m_pCameraViewImage, 0, W * H);

	m_ViewerCamera.OnInitWithCamera(W, H, 8);
	m_ViewerCamera.SetFitMode();

	//m_ViewerCamera.SetEnableDrawFigure(0, false);
	//m_ViewerCamera.SetEnableFigureRectangle(0);

	m_ViewerCamera.SetHwnd(this->m_hWnd);
	//m_ViewerCamera.SetEnableDrawCrossLine(true);
	//m_ViewerCamera.SetEnableDrawTargetLine(true);
	//m_ViewerCamera.SetEnableModifyFigure(0, false);

	//m_ViewerCamera.setCameraResolutionX(m_pMain->vt_viewer_info[viewer].resolution_x);
	//m_ViewerCamera.setCameraResolutionY(m_pMain->vt_viewer_info[viewer].resolution_y);

	//m_ViewerCamera.setUseRefPos(TRUE);

	m_ViewerCamera.OnLoadImageFromPtr(m_pMain->getSrcBuffer(real_cam));
	memcpy(m_pMain->m_pCameraViewImage, m_pMain->getSrcBuffer(real_cam), W * H);
}


void CFormCameraView::SetViewerInit(int view)
{
	SetViewerNum(view);

	int W = m_pMain->m_stCamInfo[curr_cam].w;
	int H = m_pMain->m_stCamInfo[curr_cam].h;	

	if (m_pMain->m_pCameraViewImage != NULL)
	{
		delete[] m_pMain->m_pCameraViewImage;
		m_pMain->m_pCameraViewImage = NULL;
	}

	m_pMain->m_pCameraViewImage = new BYTE[W * H];
	memset(m_pMain->m_pCameraViewImage, 0, W * H);

	//m_ViewerCamera.OnInitWithCamera(m_pMain->m_pViewDisplayInfo[view].getViewImageWidth(), m_pMain->m_pViewDisplayInfo[view].getViewImageHeight(), 8);
	m_ViewerCamera.OnInitWithCamera(m_pMain->m_stCamInfo[curr_cam].w, m_pMain->m_stCamInfo[curr_cam].h, 8);
	m_ViewerCamera.SetFitMode();

	//m_ViewerCamera.SetEnableDrawFigure(0, false);
	//m_ViewerCamera.SetEnableFigureRectangle(0);

	m_ViewerCamera.SetHwnd(this->m_hWnd);
	//m_ViewerCamera.SetEnableDrawCrossLine(true);
	//m_ViewerCamera.SetEnableDrawTargetLine(true);
	//m_ViewerCamera.SetEnableModifyFigure(0, false);

	//m_ViewerCamera.setCameraResolutionX(m_pMain->vt_viewer_info[view].resolution_x);
	//m_ViewerCamera.setCameraResolutionY(m_pMain->vt_viewer_info[view].resolution_y);

	//m_ViewerCamera.setUseRefPos(TRUE);
	m_ViewerCamera.SetMetricTransform(Rigid(m_pMain->vt_viewer_info[view].resolution_x, 0, 0, 0, -m_pMain->vt_viewer_info[view].resolution_y, 0));
}

void CFormCameraView::patternRegist(_stPatternSelect* pInfo)
{
	int nCam = pInfo->nCam;
	int nJob = pInfo->nJob;

	CPoint ptStart = m_ViewerCamera.GetPointBegin(0);
	CPoint ptEnd = m_ViewerCamera.GetPointEnd(0);

#ifndef JOB_INFO
	int WIDTH = m_pMain->m_pViewDisplayInfo[nCam].getViewImageWidth();
	int HEIGHT = m_pMain->m_pViewDisplayInfo[nCam].getViewImageHeight();
#else
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
	int WIDTH = m_pMain->m_stCamInfo[real_cam].w;
	int HEIGHT = m_pMain->m_stCamInfo[real_cam].h;
#endif

	int w = abs(ptStart.x - ptEnd.x);
	int h = abs(ptStart.y - ptEnd.y);

	int nMarkHeightRatio = HEIGHT * m_pMain->m_dMarkRegistRatio;
	int nMarkWidthRatio = WIDTH * m_pMain->m_dMarkRegistRatio;

	if (w < 10 || h < 10)
	{
		AfxMessageBox("Area is to Small!!!");
		return;
	}

	// 20.02.25
	if (w > nMarkWidthRatio || h > nMarkHeightRatio)
	{
		AfxMessageBox("Area is to Big!!!");
		return;
	}

	// Lincoln Lee - 220528 - More precisely pattern region
	int left = MIN(ptStart.x, ptEnd.x);
	int top = MIN(ptStart.y, ptEnd.y);

	cv::Rect roi(left, top, w, h);
	cv::Mat srcMat = cv::Mat(HEIGHT, WIDTH, CV_8U, m_ViewerCamera.GetImagePtr());
	cv::Mat subMat = srcMat(roi).clone();

	CString strFilePath;
#ifndef JOB_INFO
	strFilePath.Format("%sPAT%d%d%d.bmp", m_pMain->m_strCurrentModelPath, pInfo->nCam, pInfo->nPos, pInfo->nIndex);
	cvSaveImage(strFilePath, pImage);
	m_pMain->GetMatching().setTemplateAreaShape(pInfo->nCam, pInfo->nPos, pInfo->nIndex, pInfo->nShape);
#else
	strFilePath.Format("%s%s\\PAT%d%d%d.bmp", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[nJob].job_name.c_str(),  pInfo->nCam, pInfo->nPos, pInfo->nIndex);
	// Lincoln Lee - 220528 - More precisely pattern region
	//cvSaveImage(strFilePath, pImage);
	cv::imwrite(strFilePath.GetString(), subMat);
	m_pMain->GetMatching(nJob).setTemplateAreaShape(pInfo->nCam, pInfo->nPos, pInfo->nIndex, pInfo->nShape);
#endif
	
#ifndef JOB_INFO
	if (m_pMain->GetMatching().getNccModel())
		m_pMain->GetMatching().getHalcon(pInfo->nCam, pInfo->nPos, pInfo->nIndex).halcon_ReadNccModel((BYTE*)pPatImage->imageData, pPatImage->width, pPatImage->height);
	else
	{
		double minconst = m_pMain->getModel().getAlignInfo().getHalconMinContrast(nCam);
		double highcosnt = m_pMain->getModel().getAlignInfo().getHalconHighContrast(nCam);

		if (m_pMain->GetMatching().getMaskingModelUse())
			m_pMain->GetMatching().getHalcon(pInfo->nCam, pInfo->nPos, pInfo->nIndex).halcon_ReadMaskModel((BYTE*)pPatImage->imageData, pPatImage->width, pPatImage->height, m_pMain->GetMatching().getTemplateAreaShape(pInfo->nCam, pInfo->nPos, pInfo->nIndex),
				m_pMain->GetMatching().getMaskRectUse(pInfo->nCam, pInfo->nPos, pInfo->nIndex), m_pMain->GetMatching().getMaskRectShape(pInfo->nCam, pInfo->nPos, pInfo->nIndex), m_pMain->GetMatching().getMaskRect(pInfo->nCam, pInfo->nPos, pInfo->nIndex), minconst, highcosnt);

		else m_pMain->GetMatching().getHalcon(pInfo->nCam, pInfo->nPos, pInfo->nIndex).halcon_ReadModel((BYTE*)pPatImage->imageData, pPatImage->width, pPatImage->height, minconst, highcosnt);
	}
#else
	if (m_pMain->GetMatching(nJob).getNccModel())
	{
		// Lincoln Lee - 220528 - More precisely pattern region
		m_pMain->GetMatching(nJob).getHalcon(pInfo->nCam, pInfo->nPos, pInfo->nIndex).halcon_ReadNccModel(subMat.data, subMat.cols, subMat.rows);
	}
	else
	{
		double minconst = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getHalconMinContrast(nCam);
		double highcosnt = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getHalconHighContrast(nCam);

		// Lincoln Lee - 220528 - More precisely pattern region
		if (m_pMain->GetMatching(nJob).getMaskingModelUse())
			m_pMain->GetMatching(nJob).getHalcon(pInfo->nCam, pInfo->nPos, pInfo->nIndex).halcon_ReadMaskModel(subMat.data, subMat.cols, subMat.rows, m_pMain->GetMatching(nJob).getTemplateAreaShape(pInfo->nCam, pInfo->nPos, pInfo->nIndex),
				m_pMain->GetMatching(nJob).getMaskRectUse(pInfo->nCam, pInfo->nPos, pInfo->nIndex), m_pMain->GetMatching(nJob).getMaskRectShape(pInfo->nCam, pInfo->nPos, pInfo->nIndex), m_pMain->GetMatching(nJob).getMaskRect(pInfo->nCam, pInfo->nPos, pInfo->nIndex), minconst, highcosnt);

		else m_pMain->GetMatching(nJob).getHalcon(pInfo->nCam, pInfo->nPos, pInfo->nIndex).halcon_ReadModel(subMat.data, subMat.cols, subMat.rows, minconst, highcosnt);
	}
#endif

	// Mark 등록시 mark offset은 0으로 초기화
	m_pMain->GetMatching(nJob).setMarkOffsetX(pInfo->nCam, pInfo->nPos, pInfo->nIndex, 0.0);
	m_pMain->GetMatching(nJob).setMarkOffsetY(pInfo->nCam, pInfo->nPos, pInfo->nIndex, 0.0);

	//2022.10.18 ksm ROI가 없는 경우 이미지 전체 스캔()
	CRect rcInspROI = m_pMain->GetMatching(nJob).getSearchROI(pInfo->nCam,pInfo->nPos);	
	if (rcInspROI.IsRectEmpty() || rcInspROI.IsRectNull())
	{
		rcInspROI.top = 0;
		rcInspROI.left = 0;
		rcInspROI.right = WIDTH;
		rcInspROI.bottom = HEIGHT;

		m_pMain->GetMatching(nJob).setSearchROI(pInfo->nCam, pInfo->nPos, rcInspROI);
	}

	srcMat.release();
	subMat.release();
}

void CFormCameraView::patternSearch(_stPatternSelect *pInfo)
{
	int nCam = pInfo->nCam;
	int nPos = pInfo->nPos;
	int nJob = pInfo->nJob;

	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	int WIDTH = m_pMain->m_stCamInfo[real_cam].w;
	int HEIGHT = m_pMain->m_stCamInfo[real_cam].h;

	CPatternMatching *pPattern = &m_pMain->GetMatching(nJob);
	m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 0), m_pMain->getCameraViewBuffer(), WIDTH * HEIGHT);

	BOOL bReverse = FALSE;
	if (m_pMain->GetMatching(nJob).getMarkFindMethod() == METHOD_RANSAC)
	{
		CRect rectROI		= m_pMain->GetMatching(nJob).getSearchROI(nCam, nPos);
		int nAreaHori		= m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getSearchArea(nCam, nPos, 0);
		int nAreaVert		= m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getSearchArea(nCam, nPos, 1);
		int nRemoveNoise	= m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getRemoveNoiseSize(nCam, nPos, 0);
		int nThreshold		= m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getSobelThreshold(nCam, nPos, 0);
		int nCornerType		= m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getCornerType(nCam, nPos);		// 19.10.28
		int nEdgePolarity	= m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getEdgePolarity(nCam, nPos);		// 19.10.28
		int nEdgeDirection	= m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getEdgeDirection(nCam, nPos);	// 19.10.28

		int bwhite = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkReverseFind();

		int nIndex = pInfo->nIndex;																					//20.03.27
		if (theApp.m_pFrame->vt_job_info[nJob].model_info.getAlignInfo().getUseMultiplexRoi() && theApp.m_pFrame->GetMatching(nJob).getUseSearchROI2(nCam, nPos, nIndex))
			theApp.m_pFrame->GetMatching(nJob).findPattern_Ransac_Multiple(m_pMain->getProcBuffer(real_cam, 0), nCam, nPos, (CFindInfo *)0, nIndex, WIDTH, HEIGHT);
		else m_pMain->GetMatching(nJob).findPattern_Ransac(m_pMain->getProcBuffer(real_cam, 0), nCam, nPos, WIDTH, HEIGHT);
	}
	else
	{
		if(pInfo->nIndex == 4)	
			m_pMain->GetMatching(nJob).findPatternIndex(m_pMain->getProcBuffer(real_cam, 0), nCam, nPos, WIDTH, HEIGHT, 4);
		else
		{
			CRect realRoi = m_pMain->GetMatching(nJob).getSearchROI(nCam, nPos);
			bool bwhite = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getEnableAlignBlackWhiteSelect() ? true : false;
			pPattern->findPattern(m_pMain->getProcBuffer(real_cam, 0), nCam, nPos, WIDTH, HEIGHT);
			m_pMain->GetMatching(nJob).setSearchROI(nCam, nPos, realRoi);
		}

		// KBJ 2022-02-23 Search Button is not use that
		/*
		if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod() == METHOD_CALIPER)
		{
			patternSearchCaliper(pInfo);

			BOOL bFindLine[2];
			sLine line_info[2];

			line_info[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_1].m_lineInfo;
			line_info[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_2].m_lineInfo;
			bFindLine[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_1].getIsMakeLine();
			bFindLine[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_2].getIsMakeLine();

			if (bFindLine[0] && bFindLine[1])
			{
				sLine lineHori = line_info[0];
				sLine lineVert = line_info[1];
				double posx=0,  posy=0;

				CCaliper::cramersRules(-lineHori.a, 1, -lineVert.a, 1, lineHori.b, lineVert.b, &posx, &posy);

				CFindInfo pFindInfo = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos);
				pFindInfo.SetXPos(posx);
				pFindInfo.SetYPos(posy);
				pFindInfo.SetFound(FIND_OK);
				pFindInfo.SetScore(100.0);
				m_pMain->GetMatching(nJob).setFindInfo(nCam, nPos, pFindInfo);
			}
			else
			{
				CFindInfo pFindInfo = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos);
				pFindInfo.SetXPos(0.0);
				pFindInfo.SetYPos(0.0);
				pFindInfo.SetFound(FIND_ERR);
				pFindInfo.SetScore(0.0);
				m_pMain->GetMatching(nJob).setFindInfo(nCam, nPos, pFindInfo);
			}
		}
		*/
	}
}


void CFormCameraView::patternSearchCaliper_matching_line_theta(_stPatternSelect* pInfo)
{
	int nCam = pInfo->nCam;
	int nPos = pInfo->nPos;
	int nJob = pInfo->nJob;
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	int w = m_pMain->m_stCamInfo[real_cam].w;
	int h = m_pMain->m_stCamInfo[real_cam].h;


	double dx = 0.0, dy = 0.0, dt = 0.0;

	BYTE* pImage = m_pMain->getCameraViewBuffer();

#ifndef JOB_INFO
	BOOL bMarkUse = m_pMain->getModel().getAlignInfo().getUseFixtureMark(nCam);
#else
	BOOL bFixtureUse = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseFixtureMark();
#endif

	if (bFixtureUse)
	{
		double fixtureX = m_pMain->GetMatching(nJob).getFixtureX(nCam, nPos);
		double fixtureY = m_pMain->GetMatching(nJob).getFixtureY(nCam, nPos);

		m_pMain->GetMatching(nJob).findPattern(pImage, nCam, nPos, w, h);

		double posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetXPos();
		double posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetYPos();

		dx = posX - fixtureX;
		dy = posY - fixtureY;
		dt = 0.0;
	}

	m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][0].processCaliper(pImage, w, h, dx, dy, dt, TRUE);

	m_pMain->m_pDlgCaliper->m_pCaliperParam->updateUIFromData();
	m_pMain->m_pDlgCaliper->m_pCaliperParam->updateCoordinates();
}

void CFormCameraView::patternSearchCaliper(_stPatternSelect *pInfo)
{
	int nCam = pInfo->nCam;
	int nPos = pInfo->nPos;
	int nJob = pInfo->nJob;
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	int w = m_pMain->m_stCamInfo[real_cam].w;
	int h = m_pMain->m_stCamInfo[real_cam].h;


	double dx = 0.0, dy = 0.0, dt = 0.0;

	BYTE *pImage = m_pMain->getCameraViewBuffer();

#ifndef JOB_INFO
	BOOL bMarkUse = m_pMain->getModel().getAlignInfo().getUseFixtureMark(nCam);
#else
	BOOL bFixtureUse = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseFixtureMark();
#endif

	if (bFixtureUse)
	{
		double fixtureX = m_pMain->GetMatching(nJob).getFixtureX(nCam, nPos);
		double fixtureY = m_pMain->GetMatching(nJob).getFixtureY(nCam, nPos);

		m_pMain->GetMatching(nJob).findPattern(pImage, nCam, nPos, w, h);
	   
		double posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetXPos();
		double posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetYPos();

		if (fixtureX != 0.0 && fixtureY != 0.0)
		{
			dx = posX - fixtureX;
			dy = posY - fixtureY;
			dt = 0.0;
		}
	}	

	if (nPos == 0)
	{
		m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_1].processCaliper(pImage, w, h, dx, dy, dt, TRUE);
		m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_2].processCaliper(pImage, w, h, dx, dy, dt, TRUE);
		m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_3].processCaliper(pImage, w, h, dx, dy, dt, TRUE);
	}
	else
	{
		int nAlgoType = m_pMain->vt_job_info[nJob].algo_method;

		if (nAlgoType == CLIENT_TYPE_1CAM_1SHOT_ALIGN || 
			nAlgoType == CLIENT_TYPE_1CAM_2POS_REFERENCE || 
			nAlgoType == CLIENT_TYPE_1CAM_4POS_ROBOT ||
			nAlgoType == CLIENT_TYPE_1CAM_1SHOT_FILM ||
			nAlgoType == CLIENT_TYPE_1CAM_1SHOT_ALIGN_WITH_SCRATCH_INSP)
		{
			m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_1].processCaliper(pImage, w, h, dx, dy, dt, TRUE);
			m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_2].processCaliper(pImage, w, h, dx, dy, dt, TRUE);
			m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_3].processCaliper(pImage, w, h, dx, dy, dt, TRUE);
		}
		else
		{
			m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_4].processCaliper(pImage, w, h, dx, dy, dt, TRUE);
			m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_5].processCaliper(pImage, w, h, dx, dy, dt, TRUE);
		}



	}
	m_pMain->m_pDlgCaliper->m_pCaliperParam->updateUIFromData();
	m_pMain->m_pDlgCaliper->m_pCaliperParam->updateCoordinates();
}

void CFormCameraView::patternSetROI(_stPatternSelect* pInfo)
{
	CPoint ptStart = m_ViewerCamera.GetPointBegin(0);
	CPoint ptEnd = m_ViewerCamera.GetPointEnd(0);

	int w = abs(ptStart.x - ptEnd.x);
	int h = abs(ptStart.y - ptEnd.y);

	if (w < 5 || h < 5)
	{
		AfxMessageBox("Area is to Small!!!");
		return;
	}

	int nCam = pInfo->nCam;
	int nPos = pInfo->nPos;
	int nJob = pInfo->nJob;
	CRect rectROI;
	rectROI.left	= MIN(m_ViewerCamera.GetWidth() - 4, 		MAX(0, MIN(ptStart.x, ptEnd.x)));
	rectROI.top		= MIN(m_ViewerCamera.GetHeight() - 4, 		MAX(0,MIN(ptStart.y, ptEnd.y)));
	rectROI.right	= MAX(0,		MIN(m_ViewerCamera.GetWidth()-4, 	MAX(ptStart.x, ptEnd.x)));
	rectROI.bottom	= MAX(0,		MIN(m_ViewerCamera.GetHeight()-4, 	MAX(ptStart.y, ptEnd.y)));

	rectROI.left = (rectROI.left / 4) * 4;
	rectROI.right = rectROI.left + (rectROI.Width() / 8 * 8);//(rect.right / 4) * 4; //2017.10.12 Mark Image Width Size 8의 배수 적용
	rectROI.top = (rectROI.top / 4) * 4;
	rectROI.bottom = (rectROI.bottom / 4) * 4;

	m_pMain->GetMatching(nJob).setSearchROI(nCam, nPos, rectROI);
}

void CFormCameraView::patternCalcWholeROI(_stPatternSelect *pInfo)
{
	CPoint ptStart = m_ViewerCamera.GetPointBegin(0);
	CPoint ptEnd = m_ViewerCamera.GetPointEnd(0);

	int w = abs(ptStart.x - ptEnd.x);
	int h = abs(ptStart.y - ptEnd.y);

	if (w < 5 || h < 5)
	{
		AfxMessageBox("Area is to Small!!!");
		return;
	}

	CRect rectROI;
	rectROI.left = MIN(ptStart.x, ptEnd.x);
	rectROI.top = MIN(ptStart.y, ptEnd.y);
	rectROI.right = MAX(ptStart.x, ptEnd.x);
	rectROI.bottom = MAX(ptStart.y, ptEnd.y);


	CPaneCamera *pCamera = (CPaneCamera *)m_pMain->m_pPane[PANE_CAMERA];
	int nJob = pCamera->m_hSelectTab.GetCurSel();

	int nRow = m_pMain->vt_job_info[nJob].model_info.getGlassInfo().getPanelRowCount();
	int nCol = m_pMain->vt_job_info[nJob].model_info.getGlassInfo().getPanelColumnCount();

	double unitX = 0, unitY = 0;

	if (m_pMain->m_nClientID == 3)
	{
		unitX = w / (double)nCol;
		unitY = h / (double)nRow;
	}
	else
	{
		unitX = w / (double)nRow;
		unitY = h / (double)nCol;
	}

	double x = 0.0, y = 0.0;

	int nRoiSizeX = pInfo->nTemp[0];
	int nRoiSizeY = pInfo->nTemp[1];
	int nRoiCenterOffsetX = pInfo->nTemp[2];
	int nRoiCenterOffsetY = pInfo->nTemp[3];

	if (nRoiSizeX == 0) nRoiSizeX = int(unitX);
	if (nRoiSizeY == 0) nRoiSizeY = int(unitY);

	int nPanel = 0;
	CRect rcROI;
	for (int row = 0; row < nRow; row++)
	{
		for (int col = 0; col < nCol; col++)
		{
			nPanel = row * nCol + col;

			if (m_pMain->m_nClientID == 3)	// Panel Exist
			{
				// 좌상 -> 우하
				rcROI.left = LONG(rectROI.left + (col * unitX) + nRoiCenterOffsetX);
				rcROI.top = LONG(rectROI.top + unitY * row + nRoiCenterOffsetY);
				rcROI.right = LONG(rcROI.left + nRoiSizeX);
				rcROI.bottom = LONG(rcROI.top + nRoiSizeY);
			}
			else
			{
				rcROI.left = LONG(rectROI.left + (row * unitX) + nRoiCenterOffsetX);
				rcROI.top = LONG(rectROI.bottom - (unitY * (col + 1)) + nRoiCenterOffsetY);
				rcROI.right = LONG(rcROI.left + nRoiSizeX);
				rcROI.bottom = LONG(rcROI.top + nRoiSizeY);
			}

			m_pMain->GetPanelExist().setSearchROI(nPanel, rcROI);
		}
	}
}

void CFormCameraView::patternDisplayResult(_stPatternSelect *pInfo)
{
	int nJob = pInfo->nJob;
	int nCam = pInfo->nCam;
	int nPos = pInfo->nPos;
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	int nWidth = m_pMain->m_stCamInfo[curr_cam].w;
	int nHeight = m_pMain->m_stCamInfo[curr_cam].h;
	int nRate = double(nHeight / 2064) + double(nWidth / 3088) - 1;

	int nPenThickness	= CAM_VIEW_PEN_THICKNESS * nRate;
	int nPenLength		= CAM_VIEW_PEN_LENGTH * nRate;
	int nFontWidth		= CAM_VIEW_FONT_WIDTH * nRate;
	int nFontHeight		= CAM_VIEW_FONT_HEIGHT * nRate;

	m_ViewerCamera.ClearOverlayDC();
	auto pDC = m_ViewerCamera.getOverlayDC();

	CPen penOK(PS_SOLID, nPenThickness, RGB(0, 255, 0)), penMatch(PS_SOLID, nPenThickness, RGB(255, 255, 0)), *oldPen;
	CPen penNG(PS_SOLID, nPenThickness, RGB(255, 0, 0));
	CPen penDraw(PS_SOLID, nPenThickness, RGB(255, 0, 255));
	CPen penROI(PS_SOLID, nPenThickness, RGB(255, 255, 255));

	CFindInfo find = m_pMain->GetMatching(nJob).getFindInfo(pInfo->nCam, pInfo->nPos);

	CRect rectROI;	//20.03.29

#ifndef JOB_INFO
	if (theApp.m_pFrame->getModel().getAlignInfo().getUseMultiplexRoi(pInfo->nCam) && theApp.m_pFrame->GetMatching().getUseSearchROI2(pInfo->nCam, pInfo->nPos, pInfo->nIndex))
		rectROI = m_pMain->GetMatching().getSearchROI2(pInfo->nCam, pInfo->nPos, pInfo->nIndex);
	else rectROI = m_pMain->GetMatching().getSearchROI(pInfo->nCam, pInfo->nPos);
#else
	if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseMultiplexRoi() && theApp.m_pFrame->GetMatching(nJob).getUseSearchROI2(pInfo->nCam, pInfo->nPos, pInfo->nIndex))
		rectROI = m_pMain->GetMatching(nJob).getSearchROI2(pInfo->nCam, pInfo->nPos, pInfo->nIndex);
	else rectROI = m_pMain->GetMatching(nJob).getSearchROI(pInfo->nCam, pInfo->nPos);
#endif

	if (find.GetFound() == FIND_OK)				oldPen = pDC->SelectObject(&penOK);
	else if (find.GetFound() == FIND_MATCH)		oldPen = pDC->SelectObject(&penMatch);
	else										oldPen = pDC->SelectObject(&penNG);

	double pos_x = 0;
	double pos_y = 0;

	if (find.GetFound() == FIND_OK || find.GetFound() == FIND_MATCH)	// 19.11.11 NG의 경우 Line 표시는 제외 ( 이전값으로 Line 표시됨 )
	{
		//if (m_pMain->getModel().getAlignInfo().getMarkFindMethod(pInfo->nCam) == METHOD_MATCHING)
		{
			int find_index = find.GetFoundPatternNum();
			double mark_offset_x = m_pMain->GetMatching(nJob).getMarkOffsetX(pInfo->nCam, pInfo->nPos, find_index);
			double mark_offset_y = m_pMain->GetMatching(nJob).getMarkOffsetY(pInfo->nCam, pInfo->nPos, find_index);

			pos_x = find.GetXPos() + mark_offset_x;
			pos_y = find.GetYPos() + mark_offset_y;

			m_ViewerCamera.AddSoftGraphic(new GraphicPoint(cv::Point2f(pos_x, pos_y), 0xff00ff00, 7, 2));

			if (fabs(mark_offset_x)>0.1 || fabs(mark_offset_y)>0.1)
			{
				Gdiplus::Color color = 0xffffff00;
				pDC->AddGraphic(new GraphicPoint(Gdiplus::PointF(find.GetXPos(), find.GetYPos()), color, 5, 1));
			}
		}


#ifndef JOB_INFO
		if (m_pMain->getModel().getAlignInfo().getMarkFindMethod(pInfo->nCam) == METHOD_RANSAC)
#else
		if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod() == METHOD_RANSAC)
#endif
		{
			double x1, y1, x2, y2;
			double x3, y3, x4, y4;

			x1 = rectROI.left;
			x2 = rectROI.right;
			y1 = x1 * m_pMain->GetMatching(nJob).m_ax[pInfo->nCam][pInfo->nPos][0] + m_pMain->GetMatching(nJob).m_bc[pInfo->nCam][pInfo->nPos][0];
			y2 = x2 * m_pMain->GetMatching(nJob).m_ax[pInfo->nCam][pInfo->nPos][0] + m_pMain->GetMatching(nJob).m_bc[pInfo->nCam][pInfo->nPos][0];

			y3 = rectROI.top;
			y4 = rectROI.bottom;
			if (m_pMain->GetMatching(nJob).m_ax[pInfo->nCam][pInfo->nPos][1] == 0)
			{
				x3 = m_pMain->GetMatching(nJob).m_bc[pInfo->nCam][pInfo->nPos][1];
				x4 = m_pMain->GetMatching(nJob).m_bc[pInfo->nCam][pInfo->nPos][1];
			}
			else
			{
				x3 = (y3 - m_pMain->GetMatching(nJob).m_bc[pInfo->nCam][pInfo->nPos][1]) / m_pMain->GetMatching(nJob).m_ax[pInfo->nCam][pInfo->nPos][1];
				x4 = (y4 - m_pMain->GetMatching(nJob).m_bc[pInfo->nCam][pInfo->nPos][1]) / m_pMain->GetMatching(nJob).m_ax[pInfo->nCam][pInfo->nPos][1];
			}

			pDC->MoveTo(int(x1), int(y1));
			pDC->LineTo(int(x2), int(y2));

			pDC->MoveTo(int(x3), int(y3));
			pDC->LineTo(int(x4), int(y4));
		}
	}


	pDC->AddGraphic(new GraphicRectangle(rectROI, 0xff00ff00, 1));


#ifndef JOB_INFO
	if (m_pMain->getModel().getAlignInfo().getMarkFindMethod(pInfo->nCam) == METHOD_RANSAC)
#else
	if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod() == METHOD_RANSAC)
#endif
	{
		CPen penRealROI(PS_SOLID, nPenThickness, RGB(255, 255, 0));
		pDC->SelectObject(&penRealROI);

		CRect rcRealROI = m_pMain->GetMatching(nJob).m_rcRealROI[pInfo->nCam][pInfo->nPos][0];

		pDC->AddGraphic(new GraphicRectangle(rcRealROI, 0xff00ff00, 1));
		rcRealROI = m_pMain->GetMatching(nJob).m_rcRealROI[pInfo->nCam][pInfo->nPos][1];
		pDC->AddGraphic(new GraphicRectangle(rcRealROI, 0xff00ff00, 1));
	}

	int nTextX = int(find.GetXPos());
	int nTextY = int(find.GetYPos());

	
	CString str;
	str.Format("(%.1f, %.1f, %.1f) - %.1f%% [%d]", find.GetXPos(), find.GetYPos(), find.GetAngle(), find.getScore(), find.GetFoundPatternNum() + 1);

	pDC->AddGraphic(new GraphicLabel(float(pos_x), float(pos_y), str, 0xff00ff00, Anchor::BottomCenter));	

	pDC->SelectObject(oldPen);

	penOK.DeleteObject();
	penMatch.DeleteObject();
	penNG.DeleteObject();
	penROI.DeleteObject();
	oldPen->DeleteObject();
	penDraw.DeleteObject();

	m_ViewerCamera.Invalidate();
}

void CFormCameraView::draw_CaliperLine(LPARAM lParam)
{
	m_ViewerCamera.ClearOverlayDC();
	//m_ViewerCamera.clearAllFigures();

	_stPatternSelect* pInfo = (_stPatternSelect*)lParam;

	int nCam = pInfo->nCam;
	int nPos = pInfo->nPos;
	int nJob = pInfo->nJob;
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	int nWidth = m_pMain->m_stCamInfo[real_cam].w;
	int nHeight = m_pMain->m_stCamInfo[real_cam].h;
	int nRate = double(nHeight / 2064) + double(nWidth / 3088) - 1;

	int nPenThickness = CAM_VIEW_PEN_THICKNESS * nRate;
	int nPenLength = CAM_VIEW_PEN_LENGTH * nRate;
	int nFontWidth = CAM_VIEW_FONT_WIDTH * nRate;
	int nFontHeight = CAM_VIEW_FONT_HEIGHT * nRate;

	double posx[2], posy[2];

	auto pDC = m_ViewerCamera.getOverlayDC();

	BOOL bFindLine[2];
	sLine line_info[2];

	if (nPos == 0)
	{
		line_info[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_1].m_lineInfo;
		line_info[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_2].m_lineInfo;

		bFindLine[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_1].getIsMakeLine();
		bFindLine[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_2].getIsMakeLine();
	}
	else
	{
		int nAlgoType = m_pMain->vt_job_info[nJob].algo_method;

		if (nAlgoType == CLIENT_TYPE_1CAM_1SHOT_ALIGN || 
			nAlgoType == CLIENT_TYPE_1CAM_2POS_REFERENCE ||
			nAlgoType == CLIENT_TYPE_1CAM_4POS_ROBOT ||
			nAlgoType == CLIENT_TYPE_FILM_INSP ||
			nAlgoType == CLIENT_TYPE_1CAM_1SHOT_FILM ||
			nAlgoType == CLIENT_TYPE_1CAM_1SHOT_ALIGN_WITH_SCRATCH_INSP)
		{
			line_info[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_1].m_lineInfo;
			line_info[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_2].m_lineInfo;

			bFindLine[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_1].getIsMakeLine();
			bFindLine[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_2].getIsMakeLine();
		}
		else
		{
			line_info[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_4].m_lineInfo;
			line_info[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_5].m_lineInfo;

			bFindLine[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_4].getIsMakeLine();
			bFindLine[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_5].getIsMakeLine();
		}


	}

	//////////////////////////////////////////////////////////////////////////
	////////////////////////Draw Caliper//////////////////////////////////////

	CPoint ptStart, ptEnd;

	CPen penLine(PS_SOLID, nPenThickness, COLOR_LIME), *pOldPen;
	pOldPen = pDC->SelectObject(&penLine);

	if (bFindLine[0])	// 수직
	{
		// y = ax + b;
		// x = (y - a) / b;
		ptStart.y = 0;
		if (line_info[0].a == 0)	ptStart.x = line_info[0].b;
		else						ptStart.x = (ptStart.y - line_info[0].b) / line_info[0].a;

		ptEnd.y = nHeight;
		if (line_info[0].a == 0)	ptEnd.x = line_info[0].b;
		else						ptEnd.x = (ptEnd.y - line_info[0].b) / line_info[0].a;

		pDC->MoveTo(ptStart.x, ptStart.y);
		pDC->LineTo(ptEnd.x, ptEnd.y);
	}

	if (bFindLine[1])	// Top
	{
		// y = ax + b;
		// x = (y - a) / b;
		ptStart.x = 0;
		ptStart.y = line_info[1].a * ptStart.x + line_info[1].b;

		ptEnd.x = nWidth;
		ptEnd.y = line_info[1].a * ptEnd.x + line_info[1].b;

		pDC->MoveTo(ptStart.x, ptStart.y);
		pDC->LineTo(ptEnd.x, ptEnd.y);
	}

	double dAngle = 0;
	double dRate = 0;

	if (bFindLine[0] && bFindLine[1])
	{
		sLine lineHori = line_info[0];
		sLine lineVert = line_info[1];

		CCaliper::cramersRules(-lineHori.a, 1, -lineVert.a, 1, lineHori.b, lineVert.b, &posx[0], &posy[0]);

		dAngle = m_pMain->calcIncludedAngle(lineHori, lineVert);
		dRate = m_pMain->make_included_angle_to_matching(dAngle, nJob);
	}

	CFont font, *oldFont;

	//////////////////////////////////////////////////////////////////////////
	////////////////////////Draw Caliper//////////////////////////////////////

	// caliper에서 개별로 찾은 정보를 그리자.
	for (int i = 0; i < 2; i++)
	{
		m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][i].draw_final_result(pDC);
	}

	//Text 표시
	font.CreateFont(nFontHeight, nFontWidth, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
	oldFont = pDC->SelectObject(&font);

	CString strTemp;
	if (bFindLine[0] && bFindLine[1])
	{
		pDC->SetTextColor(COLOR_GREEN);
		strTemp.Format("(%.1f , %.1f) - %.2f%% [%.2f˚]", posx[0], posy[0], dRate, dAngle);
		pDC->TextOutA(40 , 40, strTemp);
	}
	else
	{
		CString strText;
		/*if (!bFindLine[0])	strText.Format("%s", m_pMain->m_pDlgCaliper->m_cCaliperName[real_cam][0]);
		else				strText.Format("%s", m_pMain->m_pDlgCaliper->m_cCaliperName[real_cam][1]);*/
		if (!bFindLine[0])	strText.Format("%s", m_pMain->m_pDlgCaliper->m_cCaliperName[real_cam][nPos][nPos * 3]);
		else				strText.Format("%s", m_pMain->m_pDlgCaliper->m_cCaliperName[real_cam][nPos][nPos * 3 + 1]);
		pDC->SetTextColor(COLOR_RED);
		strTemp.Format("Fail to find Line : [%s]", strText);
		pDC->TextOutA(40, 40, strTemp);
	}

	//Reference 마크 표시
#ifndef JOB_INFO
	BOOL bMarkUse = m_pMain->getModel().getAlignInfo().getUseFixtureMark(nCam);
#else
	CPaneCamera *pCamera = (CPaneCamera *)m_pMain->m_pPane[PANE_CAMERA];
	BOOL bFixtureUse = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseFixtureMark();
#endif
	double dx = 0.0, dy = 0.0, dt = 0.0;

	if (bFixtureUse)
	{
		double fixtureX = m_pMain->GetMatching(nJob).getFixtureX(nCam, nPos);
		double fixtureY = m_pMain->GetMatching(nJob).getFixtureY(nCam, nPos);

		CPen penMark(PS_SOLID, nPenThickness, COLOR_ORANGE), *pOldPen;
		pOldPen = pDC->SelectObject(&penMark);

		pDC->MoveTo(fixtureX - nPenLength, fixtureY);
		pDC->LineTo(fixtureX + nPenLength, fixtureY);
		pDC->MoveTo(fixtureX, fixtureY - nPenLength);
		pDC->LineTo(fixtureX, fixtureY + nPenLength);
	}
	
	//찾은 마크 표시
	nPenLength += 10;
	CPen penMark(PS_SOLID, nPenThickness * 1.2, COLOR_LIME);
	pOldPen = pDC->SelectObject(&penMark);

	double posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetXPos();
	double posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetYPos();

	pDC->MoveTo(posX - nPenLength, posY);
	pDC->LineTo(posX + nPenLength, posY);
	pDC->MoveTo(posX, posY - nPenLength);
	pDC->LineTo(posX, posY + nPenLength);

	pDC->SelectObject(oldFont);
	pDC->SelectObject(pOldPen);

	font.DeleteObject();
	penLine.DeleteObject();

	m_ViewerCamera.Invalidate();
}

void CFormCameraView::drawExistInspResult(LPARAM lParam)
{
	_stFindPattern *pPattern = (_stFindPattern *)lParam;

	m_ViewerCamera.ClearOverlayDC();
	//m_ViewerCamera.clearAllFigures();

	int nCam = pPattern->nCam;
	int nPos = pPattern->nPos;
	int nJob = pPattern->nJob;

	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	int nWidth = m_pMain->m_stCamInfo[real_cam].w;
	int nHeight = m_pMain->m_stCamInfo[real_cam].h;

	int nRate = double(nHeight / 2064) + double(nWidth / 3088) - 1;

	int nPenThickness = CAM_VIEW_PEN_THICKNESS * nRate;
	int nPenLength    = CAM_VIEW_PEN_LENGTH * nRate;
	int nFontWidth    = CAM_VIEW_FONT_WIDTH * nRate;
	int nFontHeight   = CAM_VIEW_FONT_HEIGHT * nRate;

	COLORREF color = COLOR_GREEN;

	CString strText, strDataText;
	CFindInfo find = m_pMain->GetMatching(nJob).getFindInfo(nCam, 0);
	////////////////////////////////////////////////////////////////////////
	////////////////////////Draw Exist//////////////////////////////////////

	//ROI
	color = COLOR_WHITE;
	stFigure tempFig;
	CRect rcROI;
	rcROI = m_pMain->GetMatching(nJob).getSearchROI(nCam, 0);
	tempFig.ptBegin.x = rcROI.left;
	tempFig.ptBegin.y = rcROI.top;
	tempFig.ptEnd.x = rcROI.right;
	tempFig.ptEnd.y = rcROI.bottom;
	m_ViewerCamera.addFigureRect(tempFig, 1, 1, color);

	//찾은 Blob 표시
	if (m_pMain->m_nExistResult == 2)
	{
		long x = 0, y = 0;
		x = rcROI.left;
		y = rcROI.top;
		color = COLOR_GREEN;
		stFigure tempFig;
		for(int i = 0; i < m_pMain->m_RectExistBlob.size(); i++)
		{
			tempFig.ptBegin.x = x + m_pMain->m_RectExistBlob[i].left;
			tempFig.ptBegin.y = y + m_pMain->m_RectExistBlob[i].top;
			tempFig.ptEnd.x = x + m_pMain->m_RectExistBlob[i].right;
			tempFig.ptEnd.y = y + m_pMain->m_RectExistBlob[i].bottom;
			m_ViewerCamera.addFigureRect(tempFig, 1, 1, color);
		}
	}

	//Text
	if (m_pMain->m_nExistResult > 0)	color = COLOR_GREEN;
	else 									color = COLOR_RED;
	if (m_pMain->m_nExistResult == 3)		strText.Format("Result: Empty");
	else if (m_pMain->m_nExistResult == 1)		strText.Format("Result: GLASS");
	else if (m_pMain->m_nExistResult == 2)		strText.Format("Result: Separator");
	else if (m_pMain->m_nExistResult == -1)	strText.Format("Result: Light Error");
	//m_pMain->draw_text(&m_ViewerCamera, strText, color, 15, 15, rcROI.left + 30, rcROI.top + 30);
	m_ViewerCamera.AddSoftGraphic(new GraphicLabel(rcROI.left + 30, rcROI.top + 30, strText, Gdiplus::Color(color | 0xff000000)));

	strText.Format("%s\nAvg Brigtness: %.0f", strText, m_pMain->m_dExistResultData);
	m_ViewerCamera.AddSoftGraphic(new GraphicLabel(rcROI.left, rcROI.top, strText, Gdiplus::Color(color | 0xff000000)));

	m_ViewerCamera.OnLoadImageFromPtr(m_pMain->getProcBuffer(real_cam, 0));
	m_ViewerCamera.Invalidate();

}

void CFormCameraView::drawDistanceInspResult2(LPARAM lParam)
{
	auto pDC = m_ViewerCamera.getOverlayDC();
	m_ViewerCamera.ClearOverlayDC();
	//m_ViewerCamera.clearAllFigures();

	_stFindPattern* pPattern = (_stFindPattern*)lParam;
	// 보류
	int nCam = pPattern->nCam;
	int nJob = pPattern->nJob;
	int nPos = pPattern->nPos;
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	int nWidth = m_pMain->m_stCamInfo[real_cam].w;
	int nHeight = m_pMain->m_stCamInfo[real_cam].h;
	int nRate = double(nHeight / 2064) + double(nWidth / 3088) - 1;

	int nPenThickness = CAM_VIEW_PEN_THICKNESS * nRate;
	int nPenLength    = CAM_VIEW_PEN_LENGTH * nRate;
	int nFontWidth    = CAM_VIEW_FONT_WIDTH * nRate;
	int nFontHeight   = CAM_VIEW_FONT_HEIGHT * nRate;

	int nLineSize = 10, nLineL = 50;
	int dX = 0, dY = 0, dX1 = 0, dY1 = 0;

	CPen* pOldPen;
	CPen penOK(PS_SOLID, nPenThickness, RGB(0, 255, 0));
	CPen penNG(PS_SOLID, nPenThickness, RGB(255, 0, 0));
	CPen penROI(PS_SOLID, nPenThickness, RGB(255, 255, 255));
	CPen PenLINE(PS_SOLID, nPenThickness, RGB(0, 0, 255));		//파란색
	CPen PenCross(PS_SOLID, nPenThickness, RGB(255, 125, 0));	//주황색
	COLORREF color;

	CFont font, *oldFont;

	font.CreateFont(nFontHeight, nFontWidth, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
	oldFont = pDC->SelectObject(&font);

	int nJobnMethod = m_pMain->vt_job_info[nJob].algo_method;
	int nMethod, nMethodDir = 0;
	if (m_pMain->m_bUseSubInspCamView) 
	{
		nMethod = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspectionSubMethod();
		nMethodDir = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspectionSubDir();
	}
	else {
		nMethod = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspectionMethod();
		nMethodDir = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getAlignmentTargetDir();
	}

	////////////////////////////////////////////////////////////////////////
	////////////////////////Draw PCBInspection//////////////////////////////
	int nFind = 0;
	CString strText;

	// Pattern 찾은 여부 확인
	for (int nIndex = 0; nIndex < 2; nIndex++) {
		if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[nIndex]) nFind++;
	}

	if (nFind == 2) { pDC->SelectObject(&penOK); pDC->SetTextColor(COLOR_GREEN); color = COLOR_GREEN; }
	else			{ pDC->SelectObject(&penNG); pDC->SetTextColor(COLOR_RED); color = COLOR_GREEN; }

	/////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////첫번째 위치 그리기
	switch (nMethod)
	{
	case DISTANCE_INSP_METHOD_M_TO_M: // Mark
	case DISTANCE_INSP_METHOD_M_TO_E: // Mark
	{
		// Draw Mark
		if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_PANEL])
		{
			m_pMain->draw_mark(&m_ViewerCamera, color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].xPos[POINT_PANEL], m_pMain->m_stInsp_DistanceResult[real_cam][nPos].yPos[POINT_PANEL], CAM_VIEW_PEN_THICKNESS, CAM_VIEW_PEN_THICKNESS/2, nPenLength);
		}
		else strText.Format("Fail to find Panel Mark");
	}
	break;
	case DISTANCE_INSP_METHOD_L_TO_M: // Line
	case DISTANCE_INSP_METHOD_L_TO_E: // Line
	{
		pOldPen = pDC->SelectObject(&PenLINE);
		// Position 1 Edge 

		if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_PANEL])
		{
			CPoint ptStart, ptEnd;

			if (nMethodDir == TRUE)		//방향이 vertical 일때
			{
				// Line Veritical
				m_pMain->draw_line(&m_ViewerCamera, color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].line[FIND_CALIPER_PANEL_VERT], nHeight, nWidth);

				// 후보군 그리기
				m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][CALIPER_PANEL_VERT].draw_final_result(pDC);

			}
			else //방향이 Horizontal 일때
			{
				// Line Horizontal
				m_pMain->draw_line(&m_ViewerCamera, color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].line[FIND_CALIPER_PANEL_HORI], nHeight, nWidth);

				// 후보군 그리기
				m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][CALIPER_PANEL_HORI].draw_final_result(pDC);
			}
		}
		else strText.Format("Fail to find Panel Line");

		pDC->SelectObject(pOldPen);
	}
	break;
	case DISTANCE_INSP_METHOD_E_TO_E: // Edge
	{
		if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_PANEL])
		{
			// Line Veritical
			//m_pMain->draw_line(pDC, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].line[FIND_CALIPER_PANEL_VERT], nHeight, nWidth);
			m_pMain->draw_line(&m_ViewerCamera, color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].line[FIND_CALIPER_PANEL_VERT], nHeight, nWidth);

			// 후보군 그리기
			m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][CALIPER_PANEL_VERT].draw_final_result(pDC);

			// Line Horizontal
			//m_pMain->draw_line(pDC, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].line[FIND_CALIPER_PANEL_HORI], nHeight, nWidth);
			m_pMain->draw_line(&m_ViewerCamera, color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].line[FIND_CALIPER_PANEL_HORI], nHeight, nWidth);

			// 후보군 그리기
			m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][CALIPER_PANEL_HORI].draw_final_result(pDC);
		}
		else strText.Format("Fail to find Panel Edge");
	}
	break;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////두번째 위치 그리기
	switch (nMethod)
	{
	case DISTANCE_INSP_METHOD_M_TO_M: // Mark
	case DISTANCE_INSP_METHOD_L_TO_M: // Mark
	{
		// Draw Mark
		if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[POINT_PCB_BOTTOM])
		{
			m_pMain->draw_mark(&m_ViewerCamera, color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].xPos[POINT_PCB_BOTTOM], m_pMain->m_stInsp_DistanceResult[real_cam][nPos].yPos[POINT_PCB_BOTTOM], nPenThickness, nPenThickness / 2, nPenLength);
		}
		else
		{
			if (nJobnMethod == CLIENT_TYPE_PCB_DISTANCE_INSP)	strText.Format("Fail to find PCB Edge");
			else												strText.Format("Fail to find BOTTOM Edge");
		}
	}
	break;
	case DISTANCE_INSP_METHOD_M_TO_E: // Edge
	case DISTANCE_INSP_METHOD_E_TO_E: // Edge
	case DISTANCE_INSP_METHOD_L_TO_E: // Edge
	{
		if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[POINT_PCB_BOTTOM])
		{
			// Line Veritical
			//m_pMain->draw_line(pDC, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].line[FIND_CALIPER_PCB_VERT], nHeight, nWidth);
			m_pMain->draw_line(&m_ViewerCamera, color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].line[FIND_CALIPER_PCB_VERT], nHeight, nWidth);

			// 후보군 그리기
			m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][CALIPER_PCB_VERT].draw_final_result(pDC);

			// Line Horizontal
			//m_pMain->draw_line(pDC, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].line[FIND_CALIPER_PCB_HORI], nHeight, nWidth);
			m_pMain->draw_line(&m_ViewerCamera, color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].line[FIND_CALIPER_PCB_HORI], nHeight, nWidth);

			// 후보군 그리기
			m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][CALIPER_PCB_HORI].draw_final_result(pDC);
			
		}
		else
		{
			if (nJobnMethod == CLIENT_TYPE_PCB_DISTANCE_INSP)	strText.Format("Fail to find PCB Edge");
			else												strText.Format("Fail to find BOTTOM Edge");
		}
	}
	break;
	}

	
	//교차점 그리기
	color = RGB(255, 125, 0);
	switch (nMethod)
	{
	case DISTANCE_INSP_METHOD_E_TO_E: // Edge 교차점
	{
		if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_PANEL] && m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_PCB_BOTTOM])
		{
			m_pMain->draw_mark(&m_ViewerCamera, color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].xPos[POINT_CROSS_1], m_pMain->m_stInsp_DistanceResult[real_cam][nPos].yPos[POINT_CROSS_1], nPenThickness, nPenThickness / 2, nPenLength);
			m_pMain->draw_mark(&m_ViewerCamera, color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].xPos[POINT_CROSS_2], m_pMain->m_stInsp_DistanceResult[real_cam][nPos].yPos[POINT_CROSS_2], nPenThickness, nPenThickness / 2, nPenLength);
		}
	}
	break;

	case DISTANCE_INSP_METHOD_L_TO_E: // Line 교차점
	{
		if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindLine[FIND_CALIPER_PANEL_VERT])
		{
			m_pMain->draw_mark(&m_ViewerCamera, color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].xPos[POINT_CROSS_1], m_pMain->m_stInsp_DistanceResult[real_cam][nPos].yPos[POINT_CROSS_1], nPenThickness, nPenThickness / 2, nPenLength);
		}
		else if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_CALIPER_PANEL_HORI])
		{
			m_pMain->draw_mark(&m_ViewerCamera, color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].xPos[POINT_CROSS_1], m_pMain->m_stInsp_DistanceResult[real_cam][nPos].yPos[POINT_CROSS_1], nPenThickness, nPenThickness / 2, nPenLength);
		}
	}
	}

	if (nFind == 2) { pDC->SelectObject(&penOK); pDC->SetTextColor(COLOR_GREEN); color = COLOR_GREEN; }
	else { pDC->SelectObject(&penNG); pDC->SetTextColor(COLOR_RED); color = COLOR_RED; }

	stFigureText tempFigText;
	// Text
	if (nFind == 2) {
		switch (nMethod)
		{
		case DISTANCE_INSP_METHOD_M_TO_M:
		case DISTANCE_INSP_METHOD_M_TO_E:
		case DISTANCE_INSP_METHOD_E_TO_E:
		{
			strText.Format("Width=%.3f", m_pMain->m_stInsp_DistanceResult[real_cam][nPos].dWidthDistance);
			m_pMain->draw_text(&m_ViewerCamera, strText, color, 40, 40);

			strText.Format("Width: %.3f\nHeight: %.3f", m_pMain->m_stInsp_DistanceResult[real_cam][nPos].dWidthDistance, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].dHeightDistance);
			m_ViewerCamera.AddHardGraphic(new GraphicLabel(20, 20, strText, Gdiplus::Color(color | 0xff000000)));
		}
		break;
		case DISTANCE_INSP_METHOD_L_TO_M:
		case DISTANCE_INSP_METHOD_L_TO_E:
		{
			//strText.Format("Distance=%.3f", m_pMain->m_stInsp_DistanceResult[real_cam][nPos].dDistance);
			//m_pMain->draw_text(&m_ViewerCamera, strText, color, 40, 40);
			strText.Format("Distance: %.3f", m_pMain->m_stInsp_DistanceResult[real_cam][nPos].dDistance);
			m_ViewerCamera.AddHardGraphic(new GraphicLabel(20, 20, strText, Gdiplus::Color(color | 0xff000000)));
		}
		break;
		}
	}
	else
	{
		m_pMain->draw_text(&m_ViewerCamera, strText, color, 40, 40);
	}

	m_ViewerCamera.Invalidate();

	font.DeleteObject();
	penOK.DeleteObject();
	penNG.DeleteObject();
	penROI.DeleteObject();
	PenCross.DeleteObject();
}

void CFormCameraView::drawAssembleCALC(LPARAM lParam)
{
	auto pDC = m_ViewerCamera.getOverlayDC();
	m_ViewerCamera.ClearOverlayDC();
	//m_ViewerCamera.clearAllFigures();

	_stFindPattern* pPattern = (_stFindPattern*)lParam;
	// 보류
	int nCam = pPattern->nCam;
	int nJob = pPattern->nJob;
	int nPos = pPattern->nPos;
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	int nWidth = m_pMain->m_stCamInfo[real_cam].w;
	int nHeight = m_pMain->m_stCamInfo[real_cam].h;
	int nRate = double(nHeight / 2064) + double(nWidth / 3088) - 1;

	int nPenThickness = CAM_VIEW_PEN_THICKNESS * nRate;
	int nPenLength    = CAM_VIEW_PEN_LENGTH * nRate;
	int nFontWidth    = CAM_VIEW_FONT_WIDTH * nRate;
	int nFontHeight   = CAM_VIEW_FONT_HEIGHT * nRate;

	int nLineSize = 10, nLineL = 50;
	int dX = 0, dY = 0, dX1 = 0, dY1 = 0;

//	CPen* pOldPen;
	CPen penOK(PS_SOLID, nPenThickness, RGB(0, 255, 0));
	CPen penNG(PS_SOLID, nPenThickness, RGB(255, 0, 0));
	CPen penROI(PS_SOLID, nPenThickness, RGB(255, 255, 255));
	CPen PenLINE(PS_SOLID, nPenThickness, RGB(0, 0, 255));		//파란색
	CPen PenCross(PS_SOLID, nPenThickness, RGB(255, 125, 0));	//주황색

	CFont font, *oldFont;

	font.CreateFont(nFontHeight, nFontWidth, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
	oldFont = pDC->SelectObject(&font);

	int nMethod, nMethodDir = 0;

	if (m_pMain->m_bUseSubInspCamView) 
	{
		nMethod = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspectionSubMethod();
		nMethodDir = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspectionSubDir();
	}
	else 
	{
		nMethod = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspectionMethod();
		nMethodDir = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getAlignmentTargetDir();
	}

	////////////////////////////////////////////////////////////////////////
	////////////////////////Draw drawAssembleCALC//////////////////////////////
	int nFind = 0;
	CString strText;
	COLORREF color;

	// Pattern 찾은 여부 확인
	for (int nIndex = 0; nIndex < 2; nIndex++) 
	{
		if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[nIndex]) nFind++;
	}

	if (nFind == 2) { pDC->SelectObject(&penOK); pDC->SetTextColor(COLOR_GREEN); color = COLOR_GREEN; }
	else			{ pDC->SelectObject(&penNG); pDC->SetTextColor(COLOR_RED); color = COLOR_RED;}

	const int FIND_PANEL = 0;
	const int FIND_PCB_BOTTOM = 1;

	const int POINT_PANEL = 0;
	const int POINT_PCB_BOTTOM = 1;
	const int POINT_CROSS_1 = 2;
	const int POINT_CROSS_2 = 3;
	const int POINT_PROTRUDING = 4;

	const int CALIPER_PANEL_VERT = 0;
	const int CALIPER_PANEL_HORI = 1;
	const int CALIPER_PCB_VERT = 3;
	const int CALIPER_PCB_HORI = 4;

	const int FIND_CALIPER_PANEL_VERT = 0;
	const int FIND_CALIPER_PANEL_HORI = 1;
	const int FIND_CALIPER_PCB_VERT = 2;
	const int FIND_CALIPER_PCB_HORI = 3;

	/////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////Panel 찾은 포인트 그리기
	switch (nMethod)
	{
	case DISTANCE_INSP_METHOD_M_TO_M: // Mark
	case DISTANCE_INSP_METHOD_M_TO_E: // Mark
	{
		// Draw Mark
		if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_PANEL])
		{
			m_pMain->draw_mark(&m_ViewerCamera, color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].xPos[POINT_PANEL], m_pMain->m_stInsp_DistanceResult[real_cam][nPos].yPos[POINT_PANEL], nPenThickness, nPenThickness / 2, nPenLength);
		}
		else strText.Format("Fail to find Panel Mark");
	}
	break;
	case DISTANCE_INSP_METHOD_E_TO_E: // Edge
	{
		if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_PANEL])
		{
			// Line Veritical
			m_pMain->draw_line(pDC, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].line[FIND_CALIPER_PANEL_VERT], nHeight, nWidth);

			// 후보군 그리기
			m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][CALIPER_PANEL_VERT].draw_final_result(pDC);

			// Line Horizontal
			m_pMain->draw_line(pDC, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].line[FIND_CALIPER_PANEL_HORI], nHeight, nWidth);

			// 후보군 그리기
			m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][CALIPER_PANEL_HORI].draw_final_result(pDC);
		}
		else strText.Format("Fail to find Panel Edge");
	}
	break;
	default:
	{
		strText.Format("Need to Insp Method Chage");
	}
	break;
	}
	
	/////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////계산된 위치 그리기
	if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_PCB_BOTTOM] == TRUE)
	{
		// 돌출부분 찾은데 표시
		m_pMain->draw_mark(&m_ViewerCamera, color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].xPos[POINT_PROTRUDING], m_pMain->m_stInsp_DistanceResult[real_cam][nPos].yPos[POINT_PCB_BOTTOM], nPenThickness, nPenThickness / 2, nPenLength);

		// 계산된 PCB 위치 표시
		m_pMain->draw_mark(&m_ViewerCamera, color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].xPos[POINT_PCB_BOTTOM], m_pMain->m_stInsp_DistanceResult[real_cam][nPos].yPos[POINT_PCB_BOTTOM], nPenThickness, nPenThickness / 2, nPenLength);
	}
	else
	{
		strText.Format("Fail to find Data");
	}

	// Text
	if (nFind == 2) { color = COLOR_GREEN; }
	else { color = COLOR_RED; }
	
	if (nFind == 2) {
		switch (nMethod)
		{
		case DISTANCE_INSP_METHOD_M_TO_M:
		case DISTANCE_INSP_METHOD_M_TO_E:
		case DISTANCE_INSP_METHOD_E_TO_E:
		{
			strText.Format("Width=%.3f", m_pMain->m_stInsp_DistanceResult[real_cam][nPos].dWidthDistance);
			m_pMain->draw_text(&m_ViewerCamera, strText, color, 40, 40);
			strText.Format("Height=%.3f", m_pMain->m_stInsp_DistanceResult[real_cam][nPos].dHeightDistance);
			m_pMain->draw_text(&m_ViewerCamera, strText, color, 40, 40);
		}
		break;
		}
	}
	else	pDC->TextOutA(40, 40, strText);

	m_ViewerCamera.Invalidate();

	font.DeleteObject();
	penOK.DeleteObject();
	penNG.DeleteObject();
	penROI.DeleteObject();
	PenCross.DeleteObject();
}

void CFormCameraView::drawReference(LPARAM lParam)
{
	_stFindPattern *pPattern = (_stFindPattern *)lParam;

	m_ViewerCamera.ClearOverlayDC();
	auto pDC = m_ViewerCamera.getOverlayDC();
	//m_ViewerCamera.clearAllFigures();

	int nCam = pPattern->nCam;
	int nPos = pPattern->nPos;
	int nJob = pPattern->nJob;
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
	int nWidth = m_pMain->m_stCamInfo[real_cam].w;
	int nHeight = m_pMain->m_stCamInfo[real_cam].h;
	int nRate = double(nHeight / 2064) + double(nWidth / 3088) - 1;

	int nPenThickness = CAM_VIEW_PEN_THICKNESS * nRate;
	int nPenLength    = CAM_VIEW_PEN_LENGTH * nRate;
	int nFontWidth    = CAM_VIEW_FONT_WIDTH * nRate;
	int nFontHeight   = CAM_VIEW_FONT_HEIGHT * nRate;

	CPen penMark(PS_SOLID, nPenThickness, COLOR_BLUE), *pOldPen;
	pOldPen = pDC->SelectObject(&penMark);

	CFont font, *oldFont;
	font.CreateFont(nFontHeight, nFontWidth, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
	oldFont = pDC->SelectObject(&font);

	//Reference 마크 표시
	//BOOL bRefeanceUse = FALSE;
	//double refX = m_pMain->GetMatching(nJob).getRefX(nCam, nPos);
	//double refY = m_pMain->GetMatching(nJob).getRefY(nCam, nPos);
	//if (refX > 0 || refY > 0) bRefeanceUse = TRUE;

	double dx = 0.0, dy = 0.0, dt = 0.0;

	//if (bRefeanceUse)
	// hsj 2022-01-31 reference 사용유무
	if(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseReferenceMark())
	{
		double refX = m_pMain->GetMatching(nJob).getRefX(nCam, nPos);
		double refY = m_pMain->GetMatching(nJob).getRefY(nCam, nPos);

		pOldPen = pDC->SelectObject(&penMark);

		pDC->MoveTo(refX - nPenLength, refY);
		pDC->LineTo(refX + nPenLength, refY);
		pDC->MoveTo(refX, refY - nPenLength);
		pDC->LineTo(refX, refY + nPenLength);

		CString strText;
		pDC->SetTextColor(COLOR_BLUE);
		strText.Format("X = %.2f, Y = %.2f", refX, refY);
		pDC->TextOutA(refY, refY + 20, strText);
	}

	m_ViewerCamera.Invalidate();

	font.DeleteObject();
	penMark.DeleteObject();
}

//2022-03-11 hsj All reference 등록 그리기 
void CFormCameraView::drawaAllReference(LPARAM lParam)
{
	_stFindPattern* pPattern = (_stFindPattern*)lParam;

	m_ViewerCamera.ClearOverlayDC();
	auto pDC = m_ViewerCamera.getOverlayDC();
	//m_ViewerCamera.clearAllFigures();

	int nCam = pPattern->nCam;
	int nJob = pPattern->nJob;
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
	int nWidth = m_pMain->m_stCamInfo[real_cam].w;
	int nHeight = m_pMain->m_stCamInfo[real_cam].h;
	int nRate = double(nHeight / 2064) + double(nWidth / 3088) - 1;

	int nPenThickness = CAM_VIEW_PEN_THICKNESS * nRate;
	int nPenLength = CAM_VIEW_PEN_LENGTH * nRate;
	int nFontWidth = CAM_VIEW_FONT_WIDTH * nRate;
	int nFontHeight = CAM_VIEW_FONT_HEIGHT * nRate;

	CPen penMark(PS_SOLID, nPenThickness, COLOR_BLUE), * pOldPen;
	pOldPen = pDC->SelectObject(&penMark);

	CFont font, * oldFont;
	font.CreateFont(nFontHeight, nFontWidth/4*1.5, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
	oldFont = pDC->SelectObject(&font);

	double dx = 0.0, dy = 0.0, dt = 0.0;

	if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseReferenceMark())
	{
		for (int nPos = 0; nPos < m_pMain->vt_job_info[nJob].num_of_position; nPos++)
		{
			double refX = m_pMain->GetMatching(nJob).getRefX(nCam, nPos);
			double refY = m_pMain->GetMatching(nJob).getRefY(nCam, nPos);

			pOldPen = pDC->SelectObject(&penMark);

			pDC->MoveTo(refX - nPenLength, refY);
			pDC->LineTo(refX + nPenLength, refY);
			pDC->MoveTo(refX, refY - nPenLength);
			pDC->LineTo(refX, refY + nPenLength);

			CString strText;
			pDC->SetTextColor(COLOR_BLUE);
			strText.Format("X = %.2f, Y = %.2f", refX, refY);
			pDC->TextOutA(refX-200, refY + 20, strText);
		}
	}

	m_ViewerCamera.Invalidate();

	font.DeleteObject();
	penMark.DeleteObject();
}

void CFormCameraView::drawFixture(LPARAM lParam)
{
	_stFindPattern* pPattern = (_stFindPattern*)lParam;

	m_ViewerCamera.ClearOverlayDC();
	auto pDC = m_ViewerCamera.getOverlayDC();
	//m_ViewerCamera.clearAllFigures();

	int nCam = pPattern->nCam;
	int nPos = pPattern->nPos;
	int nJob = pPattern->nJob;
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	int nWidth = m_pMain->m_stCamInfo[real_cam].w;
	int nHeight = m_pMain->m_stCamInfo[real_cam].h;
	int nRate = double(nHeight / 2064) + double(nWidth / 3088) - 1;

	int nPenThickness = CAM_VIEW_PEN_THICKNESS * nRate;
	int nPenLength = CAM_VIEW_PEN_LENGTH * nRate;
	int nFontWidth = CAM_VIEW_FONT_WIDTH * nRate;
	int nFontHeight = CAM_VIEW_FONT_HEIGHT * nRate;

	CPen penMark(PS_SOLID, nPenThickness, COLOR_ORANGE), * pOldPen;
	pOldPen = pDC->SelectObject(&penMark);

	CFont font, * oldFont;
	font.CreateFont(nFontHeight, nFontWidth, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
	oldFont = pDC->SelectObject(&font);

	//Fixture 마크 표시
	BOOL bFixtureUse = FALSE;
	double fixtureX = m_pMain->GetMatching(nJob).getFixtureX(nCam, nPos);
	double fixtureY = m_pMain->GetMatching(nJob).getFixtureY(nCam, nPos);
	double fixtureT = m_pMain->GetMatching(nJob).getFixtureT(nCam, nPos);

	if (fixtureX > 0 || fixtureY > 0) bFixtureUse = TRUE;

	double dx = 0.0, dy = 0.0, dt = 0.0;

	if (bFixtureUse)
	{
		pOldPen = pDC->SelectObject(&penMark);

		pDC->MoveTo(fixtureX - nPenLength, fixtureY);
		pDC->LineTo(fixtureX + nPenLength, fixtureY);
		pDC->MoveTo(fixtureX, fixtureY - nPenLength);
		pDC->LineTo(fixtureX, fixtureY + nPenLength);

		CString strText;
		pDC->SetTextColor(COLOR_ORANGE);
		strText.Format("X = %.2f, Y = %.2f, T = %.2f", fixtureX, fixtureY, fixtureT);
		pDC->TextOutA(fixtureY, fixtureY + 20, strText);
	}

	m_ViewerCamera.Invalidate();

	font.DeleteObject();
	penMark.DeleteObject();
}
 //210117
float CFormCameraView::calcAngleFromPoints(cv::Point2f _ptFirstPos, cv::Point2f _ptSecondPos)
{
	float fAngle;
	float fdX = _ptFirstPos.x - _ptSecondPos.x;
	float fdY = _ptFirstPos.y - _ptSecondPos.y;


	float dRad = fdX!=0?atan(fdY/fdX):0;
	return fAngle = (dRad * 180.f) / CV_PI;
}

int CFormCameraView::CircleLineIntersection(float x, float y, float r, float a, float b, float c, float d, float xy[][2])
{
	float m, n;

	// A,B1,C 원과 직선으로부터 얻어지는 2차방정식의 계수들
	// D: 판별식
	// X,Y: 교점의 좌표
	float A, B1, C, D;
	float X, Y;

	// A,B1,C,D게산
	if (c != a)
	{
		// m, n계산
		m = (d - b) / (c - a);
		n = (b*c - a * d) / (c - a);

		A = m * m + 1;
		B1 = (m*n - m * y - x);
		C = (x*x + y * y - r * r + n * n - 2 * n*y);
		D = B1 * B1 - A * C;

		if (D < 0)
			return 0;
		else if (D == 0)
		{
			X = -B1 / A;
			Y = m * X + n;
			xy[0][0] = X;
			xy[0][1] = Y;
			return 1;
		}
		else
		{
			X = -(B1 + sqrt(D)) / A;
			Y = m * X + n;
			xy[0][0] = X;
			xy[0][1] = Y;

			X = -(B1 - sqrt(D)) / A;
			Y = m * X + n;
			xy[1][0] = X;
			xy[1][1] = Y;
			return 2;
		}
	}
	else
	{
		// a == c 인 경우는 수직선이므로
		// 근을 가지려면 a >= (x-r) && a <=(x+r) )
		// (a-x)*(a-x)
		// 1. 근이 없는 경우
		// a < (x-r) || a > (x+r)

		// 근이 없음
		if (a < (x - r) || a >(x + r))
			return 0;
		// 하나의 중근
		else if (a == (x - r) || a == (x + r))
		{
			X = a;
			Y = y;
			xy[0][0] = X;
			xy[0][1] = Y;

			return 1;
		}
		// 두개의 근
		else
		{
			// x = a를 대입하여 Y에 대하여 풀면
			X = a;
			Y = y + sqrt(r*r - (a - x)*(a - x));
			xy[0][0] = X;
			xy[0][1] = Y;

			Y = y - sqrt(r*r - (a - x)*(a - x));
			xy[1][0] = X;
			xy[1][1] = Y;

			return 2;
		}
	}
}

void CFormCameraView::displayCartToPolar(cv::Mat magnitude, cv::Mat angle)
{
	cv::Mat _mag;
	//translate magnitude to range [0;1]
	double mag_max;
	cv::minMaxLoc(magnitude, 0, &mag_max);
	cv::normalize(magnitude, _mag, 255, 0, cv::NORM_MINMAX, CV_8UC1);

	magnitude.convertTo(magnitude, -1, 1.0 / mag_max);

	//build hsv image
	cv::Mat _hsv[3], hsv;
	_hsv[0] = angle;
	_hsv[1] = cv::Mat::ones(angle.size(), CV_32F);
	_hsv[2] = magnitude;
	cv::merge(_hsv, 3, hsv);

	cv::Mat bgr;//CV_32FC3 matrix
	cv::cvtColor(hsv, bgr, cv::COLOR_HSV2BGR);
	
	cv::Point p2;
	double _rad = 15;

	for (int y = 0; y < _mag.rows; y += 4)
	{
		uchar* _ptr = _mag.ptr<uchar>(y);
		float* _angptr = angle.ptr<float>(y);

		for (int x = 0; x < _mag.cols; x++)
		{
			if (_ptr[x] > 60)
			{
				p2.x = (cos(_angptr[x] * PI / 180.0) * _rad + x);
				p2.y = (sin(_angptr[x] * PI / 180.0) * _rad + y);

				cv::arrowedLine(bgr,cv::Point(x,y), p2,cv::Scalar(255,0,255),1);
			}
		}
	}
	cv::imshow("optical flow", bgr);
}