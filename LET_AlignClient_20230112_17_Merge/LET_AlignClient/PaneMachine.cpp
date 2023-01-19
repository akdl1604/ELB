// PaneMachine.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "PaneMachine.h"
#include "LET_AlignClientDlg.h"
#include "CramerSRuleDlg.h"
#include "MaskMarkViewDlg.h"


// TabMachineAlgoPage 대화 상자
#define TIMER_CALIBRATION 100
#define TIMER_ROTATE_CENTER 200
#define TIMER_MANUAL_MOVE 300
#define TIMER_READ_PLC_BIT	400
#define TIMER_PREALIGN_TEST 500
#define TIMER_CALIBRATION2 600
#define TIMER_ROTATE_CENTER2 700
#define TIMER_MANUAL_MOVE2 800
#define TIMER_CALIBRATION3 900
#define TIMER_CALIBRATION4 1100
#define TIMER_ROTATE_CENTER3 1200
#define TIMER_ROTATE_CENTER4 1300
#define TIMER_IMAGE_CALIBRATION 1400


IMPLEMENT_DYNCREATE(CPaneMachine, CFormView)

CPaneMachine::CPaneMachine()
	: CFormView(CPaneMachine::IDD)
{
	m_CaliboffsetX = 0;
	m_CaliboffsetY = 0;
}

CPaneMachine::~CPaneMachine()
{
#ifndef JOB_INFO
	for (int i = 0; i < MAX_CAMERA; i++)
	{
		delete c_TabMachinePage[i];
	}
#else
	for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
	{
		delete c_TabMachinePage[i];
	}
#endif
}

void CPaneMachine::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_MACHINE_CLOSE, m_btnMachineClose);
	DDX_Control(pDX, IDC_BTN_CALIB_SAVE, m_btnMachineSave);
	DDX_Control(pDX, IDC_TAB_ALGOLIST, m_hSelectAlgoTab);

	DDX_Control(pDX, IDC_BTN_DRAW_AREA_MACHINE, m_btnDrawArea_Machine);
	DDX_Control(pDX, IDC_BTN_PATTERN_REGIST_MACHINE, m_btnPatternRegist_Machine);
	DDX_Control(pDX, IDC_BTN_PATTERN_SEARCH_MACHINE, m_btnPatternSearch_Machine);
	DDX_Control(pDX, IDC_BTN_PATTERN_VIEW_MACHINE, m_btnPatternView_Machine);
}

BEGIN_MESSAGE_MAP(CPaneMachine, CFormView)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_VIEW_CONTROL, OnViewControl)
	ON_BN_CLICKED(IDC_BTN_MACHINE_CLOSE, &CPaneMachine::OnBnClickedBtnMachineClose)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_ALGOLIST, &CPaneMachine::OnSelchangeTabList)
	ON_BN_CLICKED(IDC_BTN_CALIB_SAVE, &CPaneMachine::OnBnClickedBtnCalibSave)
	ON_BN_CLICKED(IDC_BTN_PATTERN_REGIST_MACHINE, &CPaneMachine::OnBnClickedBtnPatternRegistMachine)
	ON_BN_CLICKED(IDC_BTN_PATTERN_VIEW_MACHINE, &CPaneMachine::OnBnClickedBtnPatternViewMachine)
	ON_BN_CLICKED(IDC_BTN_PATTERN_SEARCH_MACHINE, &CPaneMachine::OnBnClickedBtnPatternSearchMachine)
	ON_BN_CLICKED(IDC_BTN_DRAW_AREA_MACHINE, &CPaneMachine::OnBnClickedBtnDrawAreaMachine)
END_MESSAGE_MAP()

// CPaneMachine 진단입니다.

#ifdef _DEBUG
void CPaneMachine::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPaneMachine::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

void CPaneMachine::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	m_pMain = (CLET_AlignClientDlg *)GetParent();

	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);

	MainButtonInit(&m_btnMachineClose);
	MainButtonInit(&m_btnMachineSave);
	MainButtonInit(&m_btnDrawArea_Machine);
	MainButtonInit(&m_btnPatternRegist_Machine);
	MainButtonInit(&m_btnPatternSearch_Machine);
	MainButtonInit(&m_btnPatternView_Machine);

#ifndef JOB_INFO
	for (int i = 0; i < MAX_CAMERA; i++)
	{
		c_TabMachinePage[i] = new TabMachineAlgoPage;
		c_TabMachinePage[i]->m_nTabAlgoIndex = i;
	}
#else
	for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
	{
		c_TabMachinePage[i] = new TabMachineAlgoPage;
		c_TabMachinePage[i]->m_nJobID = i;
	}
#endif	

	CreateGrid();
}

HBRUSH CPaneMachine::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	return m_hbrBkg;
}

void CPaneMachine::InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}


void CPaneMachine::MainButtonInit(CButtonEx *pbutton, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}


void CPaneMachine::OnBnClickedBtnMachineClose()
{
	BOOL bRunning = FALSE;

	if (m_pMain->m_pCamLightControlDlg && m_pMain->m_pCamLightControlDlg->IsWindowVisible())  // 카메라 조명 컨트롤러 숨기기
		m_pMain->m_pCamLightControlDlg->ShowWindow(SW_HIDE);

#ifndef JOB_INFO
	for (int i = 0; i < m_pMain->m_pAlgorithmInfo.getAlgorithmCount(); i++)
	{
		if(c_TabMachinePage[i]->m_bMachineControlStart) bRunning = TRUE;
	}

	if (bRunning)
	{
		if (m_pMain->fnSetMessage(2, "Running Cabliration Mode..Close the Machine Setting?") != TRUE)	return;
		for (int i = 0; i < m_pMain->m_pAlgorithmInfo.getAlgorithmCount(); i++)
			c_TabMachinePage[i]->m_bMachineControlStart = FALSE;
	}
#else

	for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
	{
		if (c_TabMachinePage[i]->m_bMachineControlStart) bRunning = TRUE;
	}

	if (bRunning)
	{
		if (m_pMain->fnSetMessage(2, "Running Cabliration Mode..Close the Machine Setting?") != TRUE)	return;
		for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
			c_TabMachinePage[i]->m_bMachineControlStart = FALSE;
	}
#endif
	
	m_pMain->changeForm(FORM_MAIN);
	m_pMain->changePane(PANE_MANUAL);

	//TNWJDDL 조명 OFF
	for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
	{
		int nMetho = m_pMain->vt_job_info[i].algo_method;
		if (nMetho == CLIENT_TYPE_ASSEMBLE_INSP || nMetho == CLIENT_TYPE_4CAM_1SHOT_ALIGN)
			m_pMain->LightUse(m_pMain->LIGHT_OFF, i);
	}


	for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
	{
		c_TabMachinePage[i]->m_nSeqCalibration = 0;
		c_TabMachinePage[i]->m_nSeqRotateCenter = 0;
		c_TabMachinePage[i]->KillTimer(TIMER_CALIBRATION2);
		c_TabMachinePage[i]->KillTimer(TIMER_CALIBRATION3);
		c_TabMachinePage[i]->KillTimer(TIMER_CALIBRATION4);

		c_TabMachinePage[i]->KillTimer(TIMER_ROTATE_CENTER2);
		c_TabMachinePage[i]->KillTimer(TIMER_ROTATE_CENTER3);
		c_TabMachinePage[i]->KillTimer(TIMER_ROTATE_CENTER4);
	}

	//m_nSeqCalibration = 0;
	//m_nSeqRotateCenter = 0;
	//SetTimer(TIMER_CALIBRATION2, 100, NULL);

	// 2022-01-10 Machine setting ExposureTime value bring back.
	for (int nJob = 0; nJob < m_pMain->vt_job_info.size(); nJob++)
	{
		for (int nCam = 0; nCam < m_pMain->vt_job_info[nJob].num_of_camera; nCam++)
		{
			int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
			if (m_pMain->m_pCamLightControlDlg->m_bExposureTimeChange[real_cam] == TRUE)
			{
				int v = m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getExposureTime(real_cam);
#ifdef _DAHUHA
				if (m_pMain->m_pDahuhaCam[real_cam].isconnected())
					m_pMain->m_pDahuhaCam[real_cam].setExposureTime(v);
#else 
				if(m_pMain->m_pBaslerCam[real_cam] != NULL) m_pMain->m_pBaslerCam[real_cam]->SetExposure(v);
				m_pMain->m_pCamLightControlDlg->m_bExposureTimeChange[real_cam] = FALSE;
#endif
			}			
		}
	}

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
					m_pMain->SetLightBright(ctrl, ch, value == 0 ? 0 : value * 10);   // dh.jung 2021-07-12
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

void CPaneMachine::CreateGrid()
{
	CString str;
	CRect rt;

	m_hSelectAlgoTab.GetWindowRect(&rt);

#ifndef JOB_INFO
	for (int i = 0; i < MAX_CAMERA; i++)
	{
		if (m_pMain->m_pAlgorithmInfo.getAlgorithmCount() <= i) continue;

		str.Format("%s", m_pMain->m_pAlgorithmInfo.getAlgorithmName(i).c_str());
		m_hSelectAlgoTab.InsertItem(i, str);

		c_TabMachinePage[i]->Create(IDD_TAB_PANE_MACHINE, &m_hSelectAlgoTab);
		c_TabMachinePage[i]->SetWindowPos(NULL, 0, 30, rt.Width(), rt.Height(), i == 0 ? (SWP_SHOWWINDOW | SWP_NOZORDER) : SWP_NOZORDER);
	}
#else
	for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
	{
		str.Format("%s", m_pMain->vt_job_info[i].job_name.c_str());
		m_hSelectAlgoTab.InsertItem(i, str);

		c_TabMachinePage[i]->Create(IDD_TAB_PANE_MACHINE, &m_hSelectAlgoTab);
		c_TabMachinePage[i]->SetWindowPos(NULL, 0, 60, rt.Width(), rt.Height(), i == 0 ? (SWP_SHOWWINDOW | SWP_NOZORDER) : SWP_NOZORDER);
	}
#endif

	c_pWnd = c_TabMachinePage[0];

	m_hSelectAlgoTab.GetItemRect(0, &rt);
	m_hSelectAlgoTab.SetItemSize(CSize(rt.Width(), 30));
	m_hSelectAlgoTab.SetCurSel(0);
}

void CPaneMachine::OnSelchangeTabList(NMHDR *pNMHDR, LRESULT *pResult)
{
	UpdateData(false);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;

	if (c_pWnd != NULL)
	{
		c_pWnd->ShowWindow(SW_HIDE);
		c_pWnd = NULL;
	}

	int index = m_hSelectAlgoTab.GetCurSel();

	c_TabMachinePage[index]->ShowWindow(SW_SHOW);
	c_pWnd = c_TabMachinePage[index];
	
	::SendMessageA(m_pMain->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_SET_ALGORITHM_POS, index);	
	c_TabMachinePage[index]->updateFrameDialog();

	if (m_pMain->m_pCamLightControlDlg && m_pMain->m_pCamLightControlDlg->IsWindowVisible())  // 카메라 조명 컨트롤러 숨기기
		m_pMain->m_pCamLightControlDlg->ShowWindow(SW_HIDE);

	//KJH 2021-12-29 Tap에 맞는 조명으로 초기화
	int camCount = m_pMain->vt_job_info[index].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[index].camera_index;
	for (int nCam = 0; nCam < camCount; nCam++)
	{
		for (int nLight = 0; nLight < m_pMain->vt_job_info[index].light_info[nCam].num_of_using_light; nLight++)
		{
			int ctrl = m_pMain->vt_job_info[index].light_info[nCam].controller_id[nLight] - 1;
			int ch = m_pMain->vt_job_info[index].light_info[nCam].channel_id[nLight];
			int value = m_pMain->vt_job_info[index].model_info.getLightInfo().getLightBright(nCam, nLight, 0);

			if (m_pMain->m_stLightCtrlInfo.nType[nCam] == LTYPE_LLIGHT_LPC_COT_232)
			{
				m_pMain->SetLightBright(ctrl, ch, value == 0 ? 0 : value * 10);   // dh.jung 2021-07-12
			}
			else
			{
				m_pMain->SetLightBright(ctrl, ch, value);
			}
			Delay(50);
		}
	}

	m_pMain->draw_calib_direction(index, FORM_MACHINE);
}

void CPaneMachine::OnBnClickedBtnCalibSave()
{
#ifndef JOB_INFO
	int index = m_hSelectAlgoTab.GetCurSel();
	int camCount = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraNum(index);
	int nCam = c_TabMachinePage[index]->getCurrentMachineCamera();

	CString strSection, szValue;
	std::vector<int> camBuf = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(index);	

	for (int i = 0; i < camCount; i++)
	{
		nCam = camBuf.at(i);
		m_pMain->GetMachine(index).saveCalibrationData(nCam, m_pMain->m_strMachineDir);
		m_pMain->GetMachine(index).saveRotateCenter(m_pMain->m_strCurrentModelPath, nCam);
		m_pMain->saveMachineInfoData(&m_pMain->getModel().getMachineInfo(), m_pMain->m_strCurrentModelPath);

		for (int j = 0; j < MAX_CAMERA; j++)
		{
			int viewerCam = m_pMain->m_pViewDisplayInfo[j].getCameraID();
			if (m_pMain->m_pViewDisplayInfo[j].getCameraID() == nCam)
			{
				m_pMain->m_pViewDisplayInfo[j].setResolutionX(m_pMain->GetMachine(index).getCameraResolutionX(nCam));
				m_pMain->m_pViewDisplayInfo[j].setResolutionY(m_pMain->GetMachine(index).getCameraResolutionY(nCam));

				strSection.Format("VIEWER%d_INFO", j + 1);
				szValue.Format("%.6f", m_pMain->GetMachine(index).getCameraResolutionX(nCam));
				m_pMain->m_iniViewerFile.WriteProfileString(strSection, "RESOLUTION_X", szValue);
				szValue.Format("%.6f", m_pMain->GetMachine(index).getCameraResolutionY(nCam));
				m_pMain->m_iniViewerFile.WriteProfileString(strSection, "RESOLUTION_Y",  szValue);

				break;;
			}
		}
	}
#else
	int algo  = m_hSelectAlgoTab.GetCurSel();
	int nSelCam = c_TabMachinePage[algo]->getCurrentMachineCamera();
	int nPos = c_TabMachinePage[algo]->getCurrentMachinePosition();
	CString path;
	CString strSection, szValue;
	std::vector<int> camBuf = m_pMain->vt_job_info[algo].camera_index;

	CFormMainView* pFormMain = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];

	//KJH2 2022-10-24 캘리브레이션, 회전중심 날짜 갱신
	if (m_pMain->fnSetMessage(1, "Save The Calib, Rotate Data?") != TRUE) return;

	m_pMain->m_bSaveFinishCheck = TRUE;

	for (int nCam = 0; nCam < camBuf.size(); nCam++)
	{
		//nCam = camBuf.at(i);
		path.Format("%s%s\\", m_pMain->m_strMachineDir, m_pMain->vt_job_info[algo].job_name.c_str());
		m_pMain->GetMachine(algo).saveCalibrationData(nCam, nPos, path);
		
		path.Format("%s%s\\%s\\", m_pMain->m_strModelDir, m_pMain->m_strCurrentModelName, m_pMain->vt_job_info[algo].job_name.c_str());
		m_pMain->GetMachine(algo).saveRotateCenter(path, nCam, nPos);

		m_pMain->saveMachineInfoData(algo, &m_pMain->vt_job_info[algo].model_info.getMachineInfo(), path);

		int viewer = m_pMain->vt_job_info[algo].viewer_index[nCam];
		m_pMain->vt_viewer_info[viewer].resolution_x = m_pMain->GetMachine(algo).getCameraResolutionX(nCam, nPos);
		m_pMain->vt_viewer_info[viewer].resolution_y = m_pMain->GetMachine(algo).getCameraResolutionY(nCam, nPos);
		//2022.09.14 ksm Resolution Set Camera 적용되게 수정
		//Hoan 코드로 변경 //Camer Tap View 
		//((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.setCameraResolutionX(m_pMain->vt_viewer_info[viewer].resolution_x);
		//((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.setCameraResolutionY(m_pMain->vt_viewer_info[viewer].resolution_y);
		((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetMetricTransform(Rigid(m_pMain->vt_viewer_info[viewer].resolution_x, 0, 0, 0, -m_pMain->vt_viewer_info[viewer].resolution_y, 0));
		//Main Tap View
		//pFormMain->m_pDlgViewerMain[nCam]->GetViewer().setCameraResolutionX(m_pMain->vt_viewer_info[viewer].resolution_x);
		//pFormMain->m_pDlgViewerMain[nCam]->GetViewer().setCameraResolutionY(m_pMain->vt_viewer_info[viewer].resolution_y);
		pFormMain->m_pDlgViewerMain[nCam]->GetViewer().SetMetricTransform(Rigid(m_pMain->vt_viewer_info[viewer].resolution_x, 0, 0, 0, -m_pMain->vt_viewer_info[viewer].resolution_y, 0));

		((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_pDlgViewerMachine[viewer]->GetViewer().SetMetricTransform(Rigid(m_pMain->vt_viewer_info[viewer].resolution_x, 0, 0, 0, -m_pMain->vt_viewer_info[viewer].resolution_y, 0));

		CString strSection, strKey, strData;
		strSection.Format("VIEWER%d_INFO", viewer + 1);

		strKey.Format("RESOLUTION_X");
		strData.Format("%f", m_pMain->vt_viewer_info[viewer].resolution_x);
		m_pMain->m_iniViewerFile.WriteProfileStringA(strSection, strKey, strData);

		strKey.Format("RESOLUTION_Y");
		strData.Format("%f", m_pMain->vt_viewer_info[viewer].resolution_y);
		m_pMain->m_iniViewerFile.WriteProfileStringA(strSection, strKey, strData);
	}
#endif
	CFormMachineView* pMachine = (CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE];
	CFormMainView* pForm = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];

	//KJH2 2022-10-24 캘리브레이션, 회전중심 날짜 갱신
	pMachine->updateFrameDialog();
	m_pMain->m_bSaveFinishCheck = FALSE;

	//SJB 2022-10-24 캘리브레이션 뷰어 Resolution 즉시 적용
	pForm->SetResCalibration();
	pMachine->SetResCalibration();

	m_pMain->fnSetMessage(1, "Save END");
}

LRESULT CPaneMachine::OnViewControl(WPARAM wParam, LPARAM lParam)
{

	switch (wParam) {
	case MSG_PMC_ADD_PROCESS_HISTORY:
	{
		::SendMessageA(c_pWnd->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, lParam);
	}
	break;
	}
	return 0;
}

int CPaneMachine::getCurrentMachineAlgorithm()
{
	int index = m_hSelectAlgoTab.GetCurSel();
	return index;
}

void CPaneMachine::initLightControl()
{
	// auto 시에 자동 조명 ON.	임시 구현
	for (int nJobCnt = 0; nJobCnt < m_pMain->vt_job_info.size(); nJobCnt++)
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

				if (m_pMain->m_stLightCtrlInfo.nType[nCam] == LTYPE_LLIGHT_LPC_COT_232)
				{
					m_pMain->SetLightBright(ctrl, ch, value == 0 ? 0 : value * 10);   // dh.jung 2021-07-12
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

#pragma region //20211204 Tkyuha 켈리브레이션에서 별도로 마크 등록 하기 위함 업데이트 진행중
void CPaneMachine::OnBnClickedBtnPatternRegistMachine()
{
	if (m_pMain->m_bAutoSearch)
	{
		AfxMessageBox("Impossible In Auto Search Mode");
		return;
	}

	if (m_pMain->fnSetMessage(2, "Do you want to register your mark?") != TRUE) return;
	
	int nJob 	= m_hSelectAlgoTab.GetCurSel();
	int nCam 	= c_TabMachinePage[nJob]->getCurrentMachineCamera();
	int nPos 	= c_TabMachinePage[nJob]->getCurrentMachinePosition();
	int nIndex 	= c_TabMachinePage[nJob]->getCurrentMachinePatternIndex();
	int viewer 	= m_pMain->vt_job_info[nJob].machine_viewer_index[nCam];
	
	c_TabMachinePage[nJob]->m_bInitMarkRead = TRUE;

	if (nCam>=4) //최대 4개까지만 허용
	{
		AfxMessageBox("The maximum number of cameras allowed is 4 !");
		return;
	}

	CViewerEx* pView = &((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_pDlgViewerMachine[viewer]->GetViewer();

	CPoint ptStart = pView->GetPointBegin(MAX_DRAW - 1);
	CPoint ptEnd = pView->GetPointEnd(MAX_DRAW - 1);

	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
	int WIDTH = m_pMain->m_stCamInfo[real_cam].w;
	int HEIGHT = m_pMain->m_stCamInfo[real_cam].h;

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

	CRect rect;
	rect.left = MIN(ptStart.x, ptEnd.x);
	rect.right = MAX(ptStart.x, ptEnd.x);
	rect.top = MIN(ptStart.y, ptEnd.y);
	rect.bottom = MAX(ptStart.y, ptEnd.y);

	rect.left = (rect.left / 4) * 4;
	rect.right = rect.left + (rect.Width() / 8 * 8);//(rect.right / 4) * 4; //2017.10.12 Mark Image Width Size 8의 배수 적용
	rect.top = (rect.top / 4) * 4;
	rect.bottom = (rect.bottom / 4) * 4;

	cv::Rect roi(rect.left, rect.top, rect.Width(), rect.Height());
	cv::Mat srcMat = cv::Mat(HEIGHT, WIDTH, CV_8UC1, pView->GetImagePtr());
	cv::Mat subMat = srcMat(roi).clone();
	
	CString strFilePath;
	strFilePath.Format("%s%s\\CALIB%d%d%d.bmp", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[nJob].job_name.c_str(), nCam, nPos, nIndex);
	cv::imwrite(strFilePath.GetString(), subMat);

	double minconst = -1;
	double highcosnt = -1;
	
	m_CalibHalcon[nCam][nIndex].halcon_ReadModel(subMat.data, subMat.cols, subMat.rows, minconst, highcosnt);

	m_CaliboffsetX = 0;
	m_CaliboffsetY = 0;

	srcMat.release();
	subMat.release();
	
	UpdatePatternIndex();
}


void CPaneMachine::OnBnClickedBtnPatternViewMachine()
{
	if (m_pMain->m_bAutoSearch)
	{
		AfxMessageBox("Impossible In Auto Search Mode");
		return;
	}

	int nJob 	= m_hSelectAlgoTab.GetCurSel();
	int nCam 	= c_TabMachinePage[nJob]->getCurrentMachineCamera();
	int nPos 	= c_TabMachinePage[nJob]->getCurrentMachinePosition();
	int nIndex 	= c_TabMachinePage[nJob]->getCurrentMachinePatternIndex();

	int viewer = m_pMain->vt_job_info[nJob].machine_viewer_index[nCam];
	CViewerEx* pView = &((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_pDlgViewerMachine[viewer]->GetViewer();

	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
	int W = m_pMain->m_stCamInfo[real_cam].w;
	int H = m_pMain->m_stCamInfo[real_cam].h;

	bool bauto = false;

	CString strFilePath;
	strFilePath.Format("%s%s\\CALIB%d%d%d.bmp", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[nJob].job_name.c_str(), nCam, nPos, nIndex);

	if (_access(strFilePath, 0) != 0)
	{
		AfxMessageBox(" Pattern is Empty!!");
		return;
	}

	if (nCam >= 4) //최대 4개까지만 허용
	{
		AfxMessageBox("The maximum number of cameras allowed is 4 !");
		return;
	}

	double offsetX = m_CaliboffsetX;
	double offsetY = m_CaliboffsetY;

	CMaskMarkViewDlg dlg;
	cv::Mat src = cv::imread(strFilePath.GetString(), cv::IMREAD_GRAYSCALE);
	// Lincoln Lee - 2022/08/22 - Fix Incorrect mark offset
	auto w2 = ((src.cols - 1) / 2.f);
	auto h2 = ((src.rows - 1) / 2.f);
	
	dlg.SetImage(src.cols, src.rows, w2 + offsetX, h2 + offsetY, src.data);
	dlg.SetCalibMode(TRUE);
	dlg.SetParam(0, FALSE);
	dlg.SetParam(&m_CalibHalcon[nCam][nIndex]);
	dlg.SetMarkId(nCam, nPos, nIndex, nJob); // 20220125 Mark Index 추가

	if (dlg.DoModal() != IDOK)
	{
		src.release();
		return;
	}

	offsetX = dlg.m_dbPatRefX - w2;
	offsetY = dlg.m_dbPatRefY - h2;

	m_CaliboffsetX = offsetX;
	m_CaliboffsetY = offsetY;

	CString strMaskPath;
	strMaskPath.Format("%s%s\\%s_%d%d%d%s", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[nJob].job_name.c_str(), "CalibMarkMask", nCam, nPos, nIndex, "Info.bmp");

	if (_access(strMaskPath, 0) == 0)
	{
		cv::Mat tmpimg, grayimg;
		tmpimg = cv::imread((LPCTSTR)strMaskPath, 1);
		cv::cvtColor(tmpimg, grayimg, CV_RGB2GRAY);
		threshold(grayimg, grayimg, 5, 255, cv::THRESH_BINARY);
		m_CalibHalcon[nCam][nIndex].halcon_ReadModelNew(src.data, grayimg.data, src.cols, src.rows);
	}
	m_CalibHalcon[nCam][nIndex].halcon_SetShapeModelOrigin(offsetX, offsetY);

	src.release();
}


void CPaneMachine::OnBnClickedBtnPatternSearchMachine()
{
	if (m_pMain->m_bAutoSearch)
	{
		AfxMessageBox("Impossible In Auto Search Mode");
		return;
	}

	int nJob	= m_hSelectAlgoTab.GetCurSel();
	int nCam 	= c_TabMachinePage[nJob]->getCurrentMachineCamera();
	int nPos 	= c_TabMachinePage[nJob]->getCurrentMachinePosition();
	int nIndex 	= c_TabMachinePage[nJob]->getCurrentMachinePatternIndex();

	int viewer = m_pMain->vt_job_info[nJob].machine_viewer_index[nCam];
	CViewerEx* pView = &((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_pDlgViewerMachine[viewer]->GetViewer();

	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
	int WIDTH = m_pMain->m_stCamInfo[real_cam].w;
	int HEIGHT = m_pMain->m_stCamInfo[real_cam].h;

	m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 0), m_pMain->getSrcBuffer(real_cam), WIDTH * HEIGHT);

	if (nCam >= 4) //최대 4개까지만 허용
	{
		AfxMessageBox("The maximum number of cameras allowed is 4 !");
		return;
	}

	CRect searchRoi = CRect(100,100, WIDTH-100, HEIGHT-100);
	bool bwhite = false;
	findPattern_Matching(m_pMain->getProcBuffer(real_cam, 0), nCam, nPos, WIDTH, HEIGHT, searchRoi);

	m_pMain->GetMatching(nJob).setFindInfo(nCam, nPos, m_CalibFindInfo);

	::SendMessageA(m_pMain->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_DISPLAY_MACHINE_VIEW, 0);
}


BOOL CPaneMachine::findPattern_Matching(BYTE* pImage, int nCam, int nPos, int m_nWidth, int m_nHeight, CRect rcInspROI)
{
	BOOL bRet = FALSE;
	BOOL bFind = FALSE;
	int nMaxIndex = 0;
	double score, dbPosX, dbPosY, dbAngle;
	double dbMaxScore = 0.0, dbMaxPosX = 0.0, dbMaxPosY = 0.0, dbMaxAngle = 0.0;
	double dbScoreLimit = m_pMain->m_dCalibrationMarkMinScore;

	CFindInfo *pFindInfo = &m_CalibFindInfo;

	for (int nIndex = 0; nIndex < MAX_PATTERN_INDEX - 1; nIndex++)
	{
		if (m_CalibHalcon[nCam][nIndex].getModelRead() != true) continue;

		bRet = m_CalibHalcon[nCam][nIndex].halcon_SearchMark(pImage, m_nWidth, m_nHeight, rcInspROI, dbScoreLimit / 100.0);

		if (bRet)
		{
			score 	= m_CalibHalcon[nCam][nIndex].halcon_GetResultScore() * 100.0;
			dbPosX 	= m_CalibHalcon[nCam][nIndex].halcon_GetResultPos().x;
			dbPosY 	= m_CalibHalcon[nCam][nIndex].halcon_GetResultPos().y;
			dbAngle = m_CalibHalcon[nCam][nIndex].halcon_GetResultAngle();

			if (score > dbMaxScore)
			{
				bFind = TRUE;

				dbMaxScore = score;
				//KJH 2022-05-28 Roi 화면 밖으로 등록시 마커 좌표 오류 수정
				dbMaxPosX = dbPosX + MAX(0, rcInspROI.left);
				dbMaxPosY = dbPosY + MAX(0, rcInspROI.top);

				nMaxIndex = nIndex;
				dbMaxAngle = dbAngle;
			}
		}
	}

	if (bFind)
	{
		if (dbMaxScore > dbScoreLimit)		pFindInfo->SetFound(FIND_OK);
		else								pFindInfo->SetFound(FIND_MATCH);

		pFindInfo->SetScore(dbMaxScore);
		pFindInfo->SetXPos(dbMaxPosX);
		pFindInfo->SetYPos(dbMaxPosY);
		pFindInfo->SetFoundPatternNum(nMaxIndex);
		pFindInfo->SetAngle(dbMaxAngle);
	}
	else
	{
		pFindInfo->SetFound(FIND_ERR);
		pFindInfo->SetXPos(0.0);
		pFindInfo->SetYPos(0.0);
		pFindInfo->SetScore(0.0);
		pFindInfo->SetAngle(0.0);
	}

	return bFind;
}


void CPaneMachine::OnBnClickedBtnDrawAreaMachine()
{
	if (m_pMain->m_pCamLightControlDlg)
	{
		int nJob = m_hSelectAlgoTab.GetCurSel();
		int nCam = c_TabMachinePage[nJob]->getCurrentMachineCamera();
		m_pMain->m_pCamLightControlDlg->updateFrameDialog(nJob, nCam);

		if(m_pMain->m_pCamLightControlDlg->IsWindowVisible())
			m_pMain->m_pCamLightControlDlg->ShowWindow(SW_HIDE);
		else
			m_pMain->m_pCamLightControlDlg->ShowWindow(SW_SHOW);
	}
}

void CPaneMachine::UpdatePatternIndex()
{
	int algo = m_hSelectAlgoTab.GetCurSel();
	 c_TabMachinePage[algo]->dispPatternIndex();
}

#pragma endregion