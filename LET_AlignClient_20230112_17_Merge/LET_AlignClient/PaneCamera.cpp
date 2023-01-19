// PaneCamera.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "PaneCamera.h"
#include "LET_AlignClientDlg.h"

// CPaneCamera

IMPLEMENT_DYNCREATE(CPaneCamera, CFormView)

CPaneCamera::CPaneCamera()
	: CFormView(CPaneCamera::IDD)
{
	old_select_job = 0;
}

CPaneCamera::~CPaneCamera()
{
	for (int i = 0; i < MAX_CAMERA; i++)
	{
		delete c_TabCameraPage[i];
		delete c_TabInspPage[i];
	}
}

void CPaneCamera::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	
	DDX_Control(pDX, IDC_TAB_LIST, m_hSelectTab);
	DDX_Control(pDX, IDC_BTN_CAMERA_CLOSE, m_btnCameraClose);
	DDX_Control(pDX, IDC_BTN_PATTERN_SAVE, m_btnPatternSave);
}

BEGIN_MESSAGE_MAP(CPaneCamera, CFormView)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_CAMERA_CLOSE, &CPaneCamera::OnBnClickedBtnCameraClose)
	ON_BN_CLICKED(IDC_BTN_PATTERN_SAVE, &CPaneCamera::OnBnClickedBtnPatternSave)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_LIST, &CPaneCamera::OnSelchangeTabList)
END_MESSAGE_MAP()


// CPaneCamera 진단입니다.

#ifdef _DEBUG
void CPaneCamera::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPaneCamera::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

// CPaneCamera 메시지 처리기입니다.
void CPaneCamera::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);
	m_pMain = (CLET_AlignClientDlg *)AfxGetMainWnd();

	MainButtonInit(&m_btnCameraClose);
	MainButtonInit(&m_btnPatternSave);

	for (int i = 0; i < MAX_CAMERA; i++)
	{
		c_TabCameraPage[i] = new TabCameraPage;
		c_TabInspPage[i] = new TabInspPage;
	}	

	CreateGrid();

	// dh.jung 2021-05-26 add light svae flag
	m_pMain->vt_job_info[0].model_info.getLightInfo().lightBrightSaveFlag(FALSE);
}

void CPaneCamera::CreateGrid()
{
	CString str;
	CRect rt;

	m_hSelectTab.GetWindowRect(&rt);

#ifndef JOB_INFO
	for (int i = 0; i < MAX_CAMERA; i++)
	{
		if (m_pMain->m_nNumCamera <=i) continue;

		str.Format("%s", m_pMain->m_stCamInfo[i].cName);
		m_hSelectTab.InsertItem(i, str);

		if (m_pMain->m_pViewDisplayInfo[i].getMethod() < _INSP_CREATE_TAB_RANGE)
		{
			c_TabCameraPage[i]->Create(IDD_TAB_PANE_CAMERA, &m_hSelectTab);
			c_TabCameraPage[i]->SetWindowPos(NULL, 0, 30, rt.Width(), rt.Height(), i == 0 ? (SWP_SHOWWINDOW | SWP_NOZORDER) : SWP_NOZORDER);
			c_TabCameraPage[i]->m_nTabIndex = i;
		}
		else
		{
			c_TabInspPage[i]->Create(IDD_TAB_PANE_INSP, &m_hSelectTab);
			c_TabInspPage[i]->SetWindowPos(NULL, 0, 30, rt.Width(), rt.Height(), i == 0 ? (SWP_SHOWWINDOW | SWP_NOZORDER) : SWP_NOZORDER);
			c_TabInspPage[i]->m_nTabIndex = i;
		}
	}

	if (m_pMain->m_pViewDisplayInfo[0].getMethod()< _INSP_CREATE_TAB_RANGE)	c_pWnd = c_TabCameraPage[0];
	else 	c_pWnd = c_TabInspPage[0];
#else

	for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
	{
		str.Format("%s", m_pMain->vt_job_info[i].job_name.c_str());
		m_hSelectTab.InsertItem(i, str);

		if (1)
		{
			c_TabCameraPage[i]->m_nJobID = i;
			c_TabCameraPage[i]->Create(IDD_TAB_PANE_CAMERA, &m_hSelectTab);
			c_TabCameraPage[i]->SetWindowPos(NULL, 0, 60, rt.Width(), rt.Height(), i == 0 ? (SWP_SHOWWINDOW | SWP_NOZORDER) : SWP_NOZORDER);
			
		}
		else
		{
			c_TabInspPage[i]->Create(IDD_TAB_PANE_INSP, &m_hSelectTab);
			c_TabInspPage[i]->SetWindowPos(NULL, 0, 60, rt.Width(), rt.Height(), i == 0 ? (SWP_SHOWWINDOW | SWP_NOZORDER) : SWP_NOZORDER);
			c_TabInspPage[i]->m_nTabIndex = i;
		}
	}

	if (1)		c_pWnd = c_TabCameraPage[0];
	else		c_pWnd = c_TabInspPage[0];
#endif	

	m_hSelectTab.GetItemRect(0, &rt);
	m_hSelectTab.SetItemSize(CSize(rt.Width(), 30));
	m_hSelectTab.SetCurSel(0);
}

void CPaneCamera::OnSelchangeTabList(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;

	if (c_pWnd != NULL)
	{
		c_pWnd->ShowWindow(SW_HIDE);
		c_pWnd = NULL;
	}

	int nJob = m_hSelectTab.GetCurSel();

#ifndef JOB_INFO
	if (m_pMain->m_pViewDisplayInfo[index].getMethod() < _INSP_CREATE_TAB_RANGE)
#else
	if (m_pMain->vt_job_info[nJob].algo_method < _INSP_CREATE_TAB_RANGE)
#endif
	{
		c_TabCameraPage[nJob]->ShowWindow(SW_SHOW);
		c_pWnd = c_TabCameraPage[nJob];
		c_TabCameraPage[nJob]->init_light_setting();
		c_TabCameraPage[nJob]->updateFrameDialog();
	}
	else
	{
		c_TabInspPage[nJob]->ShowWindow(SW_SHOW);
		c_pWnd = c_TabInspPage[nJob];
		c_TabInspPage[nJob]->updateFrameDialog();
	}

#ifndef JOB_INFO
	if (m_pMain->m_nNumCamera > index)
		::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_SET_CAMERA_POS, index);
#else
	int last_cam = c_TabCameraPage[nJob]->sel_last_camera;
	int viewer = m_pMain->vt_job_info[nJob].viewer_index[last_cam];
	int cam = m_pMain->vt_job_info[nJob].camera_index[last_cam];
	//::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_SET_CAMERA_POS, MAKELPARAM(viewer, cam));
	//KJH 2022-01-05 Center Nozzle Align 시작하기 전에 ExposureTime 변경
	int nPos = m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getExposureTime(cam);
#ifdef _DAHUHA
	if (m_pMain->m_pDahuhaCam[cam].isconnected())
		m_pMain->m_pDahuhaCam[cam].setExposureTime(nPos);
#else 	
	if (m_pMain->m_pBaslerCam[cam])
	{
		m_pMain->m_pBaslerCam[cam]->SetExposure(nPos);
	}
#endif	
	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_SET_CAMERA_POS, MAKELPARAM(viewer, cam));
#endif
	m_pMain->LightControllerTurnOnOff(nJob, last_cam);

	//2021-05-21 KJH 탭 변경시 화면 갱신시 강제로 Fit이미지를 기본으로 설정
	((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.OnFitImage();
	((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.ClearGraphics();
	((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.setNavi(false);
	((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.Invalidate();

	old_select_job = nJob;

	Invalidate();
}

HBRUSH CPaneCamera::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{

	return m_hbrBkg;
}

void CPaneCamera::InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}


void CPaneCamera::MainButtonInit(CButtonEx *pbutton, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}

void CPaneCamera::OnBnClickedBtnPatternSave()
{
	if (m_pMain->fnSetMessage(1, "Save the Pattern Data?") != TRUE) return; //200320 Save버튼 Ui 변경.
	
	//KJH 2021-10-30 Save End Check 변수 추가
	m_pMain->m_bSaveFinishCheck = TRUE;

#ifndef JOB_INFO
	m_pMain->GetMatching().savePatternData(m_pMain->m_strCurrentModelPath);
	m_pMain->saveMachineInfoData(&m_pMain->getModel().getMachineInfo(), m_pMain->m_strCurrentModelPath);
#else

	int nJob = m_hSelectTab.GetCurSel();
	//int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int nPos = c_TabCameraPage[nJob]->m_cmbSelectPosition.GetCurSel();

	CString str_path;
	str_path.Format("%s%s\\", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[nJob].job_name.c_str());	

	m_pMain->GetMatching(nJob).savePatternData(str_path);
	//m_pMain->saveMachineInfoData(nJob, &m_pMain->vt_job_info[nJob].model_info.getMachineInfo(), str_path);
	m_pMain->saveLightInfoData(nJob, &m_pMain->vt_job_info[nJob].model_info.getLightInfo(), str_path);
	//exposure time 저장
	m_pMain->saveMachineInfoData(nJob, &m_pMain->vt_job_info[nJob].model_info.getMachineInfo(), str_path);

	// HSJ 2022-02-14 레퍼런스저장
	if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseReferenceMark() && m_pMain->m_bChangeReference[nJob][nPos])
	{
		//2022.06.18 ksm 전체 Reference 추가 시 Flag
		m_pMain->write_result_Reference(nJob);

		if (!m_pMain->getAllReferFlag())
		{
			c_TabCameraPage[nJob]->ReferenceRegistDate(nPos);
		}
		else
		{
			for (int i = 0; i < m_pMain->vt_job_info[nJob].num_of_position; i++)
				c_TabCameraPage[nJob]->ReferenceRegistDate(i);
		}

		m_pMain->saveAlignInfoData(nJob, &m_pMain->vt_job_info[nJob].model_info.getAlignInfo(), str_path, FALSE);
		m_pMain->AllReferSaveFlag(FALSE);
		/*m_pMain->write_result_Reference(nJob);
		c_TabCameraPage[nJob]->ReferenceRegistDate(nPos);
		m_pMain->saveAlignInfoData(nJob, &m_pMain->vt_job_info[nJob].model_info.getAlignInfo(), str_path, FALSE);*/

		// KBJ 2022-07-12 Reference 이미지 저장 기능 추가
		ReferenceImageSave();

		// KBJ 2022-07-12 Reference flag Reset 추가
		for (int i = 0; i < m_pMain->vt_job_info[nJob].num_of_position; i++)
			m_pMain->m_bChangeReference[nJob][i] = FALSE;
	}

	//KJH 2022-05-10 Fixture 날짜 저장
	if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseFixtureMark() && m_pMain->m_bChangeFixture[nJob][nPos])
	{
		m_pMain->write_result_Fixture(nJob);
		c_TabCameraPage[nJob]->FixtureRegistDate(nPos);
		m_pMain->saveAlignInfoData(nJob, &m_pMain->vt_job_info[nJob].model_info.getAlignInfo(), str_path, FALSE);
	}

	for (int nCam = 0; nCam < MAX_CAMERA; nCam++)
	{
		for (int n = 0; n < m_pMain->vt_job_info[nJob].light_info.size(); n++)
		{
			for (int nLight = 0; nLight < m_pMain->vt_job_info[nJob].light_info[n].num_of_using_light; nLight++)
			{
				m_pMain->vt_job_info[nJob].model_info.getLightInfo().saveLightBrightRestore(nCam, nLight, 0);
			}
		}
	}
	//}
	m_pMain->vt_job_info[nJob].model_info.getLightInfo().lightBrightSaveFlag(TRUE);
	// end

#ifdef _SAPERA
	//KJH 2021-10-30 Save End Check 변수 추가
	m_pMain->m_bSaveFinishCheck = FALSE;
	m_pMain->fnSetMessage(1, "Save END");
#else
	//HSJ exposuretime 저장
	//KJH 2021-08-23 Exposuretime Camera UseSet1에 저장되게 변경
	//for (int nJobCnt = 0; nJobCnt < m_pMain->vt_job_info.size(); nJobCnt++)
	//{
		for (int nCam = 0; nCam < m_pMain->vt_job_info[nJob].num_of_camera; nCam++)
		{
			int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
			int nExposuretime = m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getExposureTime(real_cam);
#ifdef _DAHUHA
			if (m_pMain->m_pDahuhaCam[real_cam].isconnected())
				m_pMain->m_pDahuhaCam[real_cam].setExposureTime(double(nExposuretime));
#else
			if (m_pMain->m_pBaslerCam[real_cam])
			{
				if (m_pMain->m_pBaslerCam[real_cam]->IsGrabContinuous())
				{
					m_pMain->m_pBaslerCam[real_cam]->GrabContinuous(FALSE);
					m_pMain->m_pBaslerCam[real_cam]->SaveExposureTime(nExposuretime);
					m_pMain->m_pBaslerCam[real_cam]->GrabContinuous(TRUE);
				}
				else
				{
					m_pMain->m_pBaslerCam[real_cam]->SaveExposureTime(nExposuretime);
				}
			}
#endif
		}

		//KJH 2021-10-30 Save End Check 변수 추가
		m_pMain->m_bSaveFinishCheck = FALSE;
		m_pMain->fnSetMessage(1, "Save END");
	//}
#endif

#endif
}

void CPaneCamera::OnBnClickedBtnCameraClose()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	//KJH 2021-10-30 Save End Check 변수 추가
	if (m_pMain->m_bSaveFinishCheck) return;

	CLET_AlignClientDlg *pDlg = (CLET_AlignClientDlg *)GetParent();
	pDlg->changeForm(FORM_MAIN);
	pDlg->changePane(PANE_MANUAL);

	BOOL m_bRestore = FALSE;

	for (int nJobCnt = int(m_pMain->vt_job_info.size() - 1); nJobCnt >= 0; nJobCnt--)
	{
		int camCount = m_pMain->vt_job_info[nJobCnt].num_of_camera;
		std::vector<int> camBuf = m_pMain->vt_job_info[nJobCnt].camera_index;

		for (int nCam = 0; nCam < camCount; nCam++)
		{
			for (int nLight = 0; nLight < m_pMain->vt_job_info[nJobCnt].light_info[nCam].num_of_using_light; nLight++)
			{
				int ctrl = m_pMain->vt_job_info[nJobCnt].light_info[nCam].controller_id[nLight] - 1;
				int ch = m_pMain->vt_job_info[nJobCnt].light_info[nCam].channel_id[nLight];
				int value = m_pMain->vt_job_info[nJobCnt].model_info.getLightInfo().getLightBright(nCam, nLight, 0);

				//KJH 2022-04-27 조명 원복기능추가
				if (m_pMain->vt_job_info[nJobCnt].model_info.getLightInfo().getlightBrightSaveFlag() == FALSE)
				{
					m_bRestore = TRUE;
				}
				else
				{
					m_pMain->vt_job_info[nJobCnt].model_info.getLightInfo().lightBrightSaveFlag(FALSE);
				}

				if (m_bRestore)  // dh.jung 2021-05-26 add light
				{// 기존 조명 설정으로 복원
					value = m_pMain->vt_job_info[nJobCnt].model_info.getLightInfo().getLightBrightRestore(nCam, nLight, 0);
					m_pMain->vt_job_info[nJobCnt].model_info.getLightInfo().loadLightBrightRestore(nCam, nLight, 0);
					m_bRestore = FALSE;
				}

				if (m_pMain->m_stLightCtrlInfo.nType[nCam] == LTYPE_LLIGHT_LPC_COT_232)
				{
					m_pMain->SetLightBright(ctrl, ch, value == 0 ? 0: value * 10);   // dh.jung 2021-07-12
				}
				else
				{
					m_pMain->SetLightBright(ctrl, ch, value);
				}
				Delay(50);
			}
		}
	}
}

void CPaneCamera::UpdateFrameDlg()
{
	int nJob = m_hSelectTab.GetCurSel();
	int nSel = 0;
	int cam = m_pMain->vt_job_info[nJob].camera_index[0];

#ifndef JOB_INFO
	if (m_pMain->m_pViewDisplayInfo[cam].getMethod() < _INSP_CREATE_TAB_RANGE)
#else
	if (m_pMain->vt_job_info[nJob].algo_method < _INSP_CREATE_TAB_RANGE)
#endif
	{
		c_TabCameraPage[nJob]->updateFrameDialog();

		//KJH 2021-10-30 Pane Cam Index 0번 고정현상 수정
		int sel_last_camera = c_TabCameraPage[nJob]->m_cmbSelectCamera.GetCurSel();

		int viewer = m_pMain->vt_job_info[nJob].viewer_index[sel_last_camera];
		int real_cam = m_pMain->vt_job_info[nJob].camera_index[sel_last_camera];
		::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_SET_CAMERA_POS, MAKELPARAM(viewer, real_cam));
	}
	else
	{
		c_TabInspPage[nJob]->updateFrameDialog();
	}	   

	memset(m_pMain->m_pDlgCaliper->m_bChanged, 0, sizeof(m_pMain->m_pDlgCaliper->m_bChanged));

	int nObj = cam;
	int nPos = 0;
	int nLine = m_pMain->m_pDlgCaliper->m_nLine;

	CFormCameraView *pCamera = (CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA];
	//pCamera->m_ViewerCamera.ClearCaliperOverlayDC();
	//m_pMain->m_pDlgCaliper->m_Caliper[nObj][nPos][nLine].DrawCaliper(m_pMain->m_pDlgCaliper->m_pDC);

	m_pMain->m_pDlgCaliper->m_pCaliperParam->updateUIFromData();
	m_pMain->m_pDlgCaliper->m_pCaliperParam->updateCoordinates();
#ifndef SKIP_XLIST
	m_pMain->m_pDlgCaliper->m_pCaliperResult->updateCaliperList();
#endif
}

void CPaneCamera::CameraCaliperClose()
{
	int nJob = m_hSelectTab.GetCurSel();
	int cam = m_pMain->vt_job_info[nJob].camera_index[0];

	if (m_pMain->vt_job_info[nJob].algo_method < _INSP_CREATE_TAB_RANGE)
	{
		CWnd* pMain = c_TabCameraPage[nJob];
		CWnd* pButton = pMain->GetDlgItem(IDC_BTN_SHOW_CALIPER);
		pMain->PostMessage(WM_COMMAND, MAKEWPARAM(IDC_BTN_SHOW_CALIPER, BN_CLICKED), (LPARAM)pButton->m_hWnd);
	}
	else
	{
	}
}
// KBJ 2022-07-12 Reference 이미지 저장 기능 추가
void CPaneCamera::ReferenceImageSave()
{
	int nJob = m_hSelectTab.GetCurSel();
	int nCam = c_TabCameraPage[nJob]->m_cmbSelectCamera.GetCurSel();
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
	int W = m_pMain->m_stCamInfo[real_cam].w;
	int H = m_pMain->m_stCamInfo[real_cam].h;

	CString strRefImagePath;

	//Ref 폴더 확인 후 생성
	strRefImagePath.Format("%s\\Reference", m_pMain->m_strResultDir);
	if (_access(strRefImagePath, 0) != 0)	CreateDirectory(strRefImagePath, NULL);
	
	//모델ID
	strRefImagePath.Format("%s\\%s", strRefImagePath, m_pMain->vt_job_info[nJob].model_info.getModelID());
	if (_access(strRefImagePath, 0) != 0)	CreateDirectory(strRefImagePath, NULL);

	//JOB이름
	strRefImagePath.Format("%s\\%s", strRefImagePath, m_pMain->vt_job_info[nJob].job_name.c_str());
	if (_access(strRefImagePath, 0) != 0)	CreateDirectory(strRefImagePath, NULL);

	SYSTEMTIME time;
	GetLocalTime(&time);
	CString strDate, strTime;
	strDate.Format("%04d%02d%02d", time.wYear, time.wMonth, time.wDay);
	strTime.Format("%02d%02d%02d", time.wHour, time.wMinute, time.wSecond);

	// 날짜
	strRefImagePath.Format("%s\\%s", strRefImagePath, strDate);
	if (_access(strRefImagePath, 0) != 0)	CreateDirectory(strRefImagePath, NULL);

	// 이미지 이름
	strRefImagePath.Format("%s\\%s_%s_%s_ref_%d%d%d%d_ImgRaw.jpg", strRefImagePath, strDate, strTime, m_pMain->m_stCamInfo[real_cam].cName, m_pMain->m_bChangeReference[nJob][0], m_pMain->m_bChangeReference[nJob][1], m_pMain->m_bChangeReference[nJob][2], m_pMain->m_bChangeReference[nJob][3]);
	IplImage* pImage = cvCreateImage(cvSize(W, H), 8, 1);
	cvZero(pImage);
	m_pMain->copyMemory(pImage->imageData, m_pMain->getCameraViewBuffer(), W * H);
	cvSaveImage(strRefImagePath, pImage);
	cvReleaseImage(&pImage);
}