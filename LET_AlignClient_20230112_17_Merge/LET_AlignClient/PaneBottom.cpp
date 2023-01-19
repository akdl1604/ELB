// PaneBottom.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "PaneBottom.h"
#include "LET_AlignClientDlg.h"
#include "LoginDlg.h"

// CPaneBottom

IMPLEMENT_DYNCREATE(CPaneBottom, CFormView)

CPaneBottom::CPaneBottom()
	: CFormView(CPaneBottom::IDD)
{
}

CPaneBottom::~CPaneBottom()
{
}

void CPaneBottom::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	
	DDX_Control(pDX, IDC_BTN_AUTO_START, m_btnAutoStart);
	DDX_Control(pDX, IDC_BTN_AUTO_STOP, m_btnAutoStop);
	DDX_Control(pDX, IDC_BTN_EXIT, m_btnExit);
}

BEGIN_MESSAGE_MAP(CPaneBottom, CFormView)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BTN_AUTO_STOP, &CPaneBottom::OnBnClickedBtnAutoStop)
	ON_BN_CLICKED(IDC_BTN_AUTO_START, &CPaneBottom::OnBnClickedBtnAutoStart)
	ON_BN_CLICKED(IDC_BTN_EXIT, &CPaneBottom::OnBnClickedBtnExit)
END_MESSAGE_MAP()


// CPaneBottom �����Դϴ�.

#ifdef _DEBUG
void CPaneBottom::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPaneBottom::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


void CPaneBottom::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);
	m_pMain = (CLET_AlignClientDlg *)AfxGetMainWnd();
	GetClientRect(&m_rcForm);

	MainButtonInit(&m_btnAutoStart);
	MainButtonInit(&m_btnAutoStop);
	MainButtonInit(&m_btnExit);

	SetAutoStartStop(FALSE);
}

HBRUSH CPaneBottom::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{

	return m_hbrBkg;
}

void CPaneBottom::InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}

void CPaneBottom::MainButtonInit(CButtonEx *pbutton, int nID, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();

	if (nID == -1)
	{
		pbutton->SetSizeImage(5, 5, size, size);
		pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
	}
	else
	{
		CRect rect;
		int nOffset = 15;
		pbutton->GetClientRect(&rect);

		pbutton->SetSizeImage(nOffset, nOffset, rect.Height() - nOffset * 2, rect.Height() - nOffset * 2);
		pbutton->LoadImageFromResource(nID, TRUE);
	}
}

void CPaneBottom::OnBnClickedBtnAutoStop()
{
	if (m_pMain->m_bAutoStart != TRUE) return;
	
	CFormMainView* pMain = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];

	if (pMain->m_pThread != NULL) // Tkyuha 20211124 ȭ�� ������ ���� �ӽ� ����
	{
		pMain->m_pThread->SuspendThread();
	}

	Delay(100);

	// KBJ 2022-09-01
	CFormMainView* pForm = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];

	// kbj 2022-01-05 Processing check
	BOOL bProcessing = FALSE;
	for (int nJob = 0; nJob < m_pMain->vt_job_info.size(); nJob++)
	{
		if (m_pMain->getProcessStartFlag(nJob))			bProcessing = TRUE;
		if (pForm->m_bHandShake[nJob] == TRUE)			bProcessing = TRUE;
	}

	if (bProcessing == TRUE)
	{
		//AfxMessageBox("Wait!! Processing is not finished.");
		//kmb 220811 ������ ������ stop �޼��� ���� AfxMSg -> fmSetMessge �� ���� EXIT �� ����
		m_pMain->fnSetMessage(1, "Wait!! Processing is not finished.");
		return;
	}

	//if (m_pMain->fnSetMessage(2, "Auto Stop?") != TRUE)
	//{
	//	if (pMain->m_pThread != NULL) // Tkyuha 20211124 �����
	//	{
	//		pMain->m_pThread->ResumeThread();
	//	}
	//	return;
	//}
	
	CString str = "Auto Mode Stoped";
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	::SendMessageA(m_pMain->m_pForm[FORM_MAIN]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_INIT_INTERFACE, 0);

	SetAutoStartStop(FALSE);
	m_pMain->changeForm(FORM_MAIN);
	m_pMain->changePane(PANE_MANUAL);

	::SendMessageA(m_pMain->m_pPane[PANE_MANUAL]->m_hWnd, WM_VIEW_CONTROL, MSG_PMN_MANUAL_COLOR_RESET, 0);

	//KJH 2021-08-11 Auto Stop�� Processflag �ʱ�ȭ
	for (int nJobCnt = 0; nJobCnt < m_pMain->vt_job_info.size(); nJobCnt++)
	{
		m_pMain->resetProcessStartFlag(nJobCnt);
		pMain->resetProcessStart(nJobCnt);
	}

	if (pMain->m_pThread != NULL)  // Tkyuha 20211124 �����
	{
		pMain->m_pThread->ResumeThread();
	}

	// YCS 2022-08-25 ź���� ���̾�α� ����
	if (m_pMain->vt_job_info[0].algo_method == CLIENT_TYPE_FILM_INSP)
	{
		for (int i = 0; i < 2; i++)
		{
			m_pMain->m_pCInspChartDlg[i]->ShowWindow(SW_HIDE);
		}
	}
}

void CPaneBottom::OnBnClickedBtnAutoStart()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if (m_pMain->m_bAutoStart) return;
	//KJH 2021-10-30 Save End Check ���� �߰�
	if (m_pMain->m_bSaveFinishCheck) return;

	CString str = "Auto Mode Start";
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	// 2022-04-26 Auto�� Machine setting ExposureTime value bring back.
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
				if (m_pMain->m_pBaslerCam[real_cam] != NULL) m_pMain->m_pBaslerCam[real_cam]->SetExposure(v);
				m_pMain->m_pCamLightControlDlg->m_bExposureTimeChange[real_cam] = FALSE;
#endif
			}
		}
	}

	CPaneCamera* pPane = (CPaneCamera*)m_pMain->m_pPane[PANE_CAMERA];

	// YCS 2022-11-28 AutoMode ����� Exposure ���� ������ �߰�
	for (int nJob = 0; nJob < m_pMain->vt_job_info.size(); nJob++)
	{
		for (int nCam = 0; nCam < m_pMain->vt_job_info[nJob].num_of_camera; nCam++)
		{
			int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
			TabCameraPage* pPage = (TabCameraPage*)pPane->c_TabCameraPage[nJob];

			if (pPage->m_bExposureTimeChange[real_cam] == TRUE)
			{
				int v = m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getExposureTime(real_cam);
#ifdef _DAHUHA
				if (m_pMain->m_pDahuhaCam[real_cam].isconnected())
					m_pMain->m_pDahuhaCam[real_cam].setExposureTime(v);
#else 
				if (m_pMain->m_pBaslerCam[real_cam] != NULL) m_pMain->m_pBaslerCam[real_cam]->SetExposure(v);
				pPage->m_bExposureTimeChange[real_cam] = FALSE;
#endif
			}
		}
	}

	if (m_pMain->m_pCamLightControlDlg && m_pMain->m_pCamLightControlDlg->IsWindowVisible())  // ī�޶� ���� ��Ʈ�ѷ� �����
		m_pMain->m_pCamLightControlDlg->ShowWindow(SW_HIDE);

	//���� â ����
	//KJH 2021-07-26 ����â ������ �� ���Ƶа�.....
	//KJH 2021-08-07 Scan Insp �б�
	if (m_pMain->vt_job_info[0].algo_method == CLIENT_TYPE_SCAN_INSP)
	{
		::SendMessageA(theApp.m_pFrame->m_pPane[PANE_AUTO]->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_PA_SCAN_SPEC_VIEW, NULL);
	}
	else if (m_pMain->vt_job_info[0].algo_method == CLIENT_TYPE_ELB_CENTER_ALIGN)
	{
		::SendMessageA(theApp.m_pFrame->m_pPane[PANE_AUTO]->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_PA_SPEC_VIEW, 1);
	}
	else 
	{
		//hsj 2022-01-10 �ּ�Ǯ��
		//KJH 2022-04-08 lParam = 0���� ���� / 1�� Center�� ����Ҷ� , 0�� ����
		::SendMessageA(theApp.m_pFrame->m_pPane[PANE_AUTO]->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_PA_SPEC_VIEW, NULL);
	}

	SetAutoStartStop(TRUE);
	m_pMain->changeForm(FORM_MAIN);
	m_pMain->changePane(PANE_AUTO);

	// YCS 2022-09-06 Auto �� ���̾�α� SHOW
	if (m_pMain->vt_job_info[0].algo_method == CLIENT_TYPE_FILM_INSP)
	{
		CPaneAuto* pAuto = (CPaneAuto*)m_pMain->m_pPane[PANE_AUTO];
		int nIndex = pAuto->m_TabSpecView.GetCurSel();

		if (nIndex == 2 || nIndex == 3)
		{
			::SendMessageA(m_pMain->m_pCInspChartDlg[nIndex - 2]->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_DLG_SHOW, NULL);
		}
	}

	// auto �ÿ� ���� ��������� üũ�ڽ� ���� �ǵ���.
	CPaneMachine *pMain = (CPaneMachine *)m_pMain->m_pPane[PANE_MACHINE];
	for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
	{
		pMain->c_TabMachinePage[i]->CheckDlgButton(IDC_CHK_MANUAL_PREALIGN, FALSE);
	}

	//KJH 2022-04-23 Auto�� Center Camera CenterAlign Expousure������ ���� ����

	if (m_pMain->vt_job_info[0].algo_method == CLIENT_TYPE_ELB_CENTER_ALIGN)
	{
		int ncamera = m_pMain->vt_job_info[0].camera_index[0];
		int nExpousure = m_pMain->vt_job_info[0].model_info.getMachineInfo().getExposureTime(ncamera);
#ifdef _DAHUHA
		if (m_pMain->m_pDahuhaCam[ncamera].isconnected())		m_pMain->m_pDahuhaCam[ncamera].setExposureTime(nExpousure);
#else
		if (m_pMain->m_pBaslerCam[ncamera])						m_pMain->m_pBaslerCam[ncamera]->SetExposure(nExpousure);
#endif
	}

	BOOL m_bRestore = FALSE;

	// auto �ÿ� �ڵ� ���� ON.	�ӽ� ����
	// ������ Nozzle Side Cam Light�� ��� Ű�� ���� ���
	// ��ü �˰����� ������ Turn On �Ѵ�.
	// KBJ 2022-03-04 opposition count �ӽ� ����

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

				//KJH 2022-04-27 ���� ��������߰�
				if (m_pMain->vt_job_info[nJobCnt].model_info.getLightInfo().getlightBrightSaveFlag() == FALSE)
				{
					m_bRestore = TRUE;
				}
				else
				{
					m_pMain->vt_job_info[nJobCnt].model_info.getLightInfo().lightBrightSaveFlag(FALSE);
				}

				if (m_bRestore)  // dh.jung 2021-05-26 add light
				{// ���� ���� �������� ����
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

	// kmb 20221020 Auto�� Cleanning Camera Light off
	if (m_pMain->vt_job_info[0].algo_method == CLIENT_TYPE_ELB_CENTER_ALIGN)
	{
		CString strJobName;
		for (int nJob = 0; nJob < m_pMain->vt_job_info.size(); nJob++) {

			strJobName.Format("%s", m_pMain->vt_job_info[nJob].job_name.c_str());
			if (strJobName.Find("CLEANING") > 0)
				m_pMain->LightControllerTurnOnOff(nJob, 0, 0, false, true);
		}
	}

	//Auto�� PLC �ð� �������� �̰� �츮������.. ����(KJH)
	//m_pMain->initPLC_Time();
}

void CPaneBottom::OnBnClickedBtnExit()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	CFormMainView* pMain = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];
	BOOL bAuto = m_pMain->m_bAutoStart;
	BOOL bProcessing = FALSE;

	if (pMain->m_pThread != NULL)  // Tkyuha 20211124 ȭ�� ������ ���� �ӽ� ����
	{
		pMain->m_pThread->SuspendThread();
	}

	//kmb 220811 Auto ���� �϶� ����� �޼��� �߰�
	if (bAuto && m_pMain->fnSetMessage(2, "Now Auto Running! Exit the Program?") != TRUE)
	{
		if (pMain->m_pThread != NULL)  // Tkyuha 20211124 �����
		{
			pMain->m_pThread->ResumeThread();
		}
		return;
	}
	//kmb 220811 Auto �ƴҽ� �޼��� �����
	else if (bAuto != TRUE){
		if (pMain->m_pThread != NULL)  // Tkyuha 20211124 �����
		{
			pMain->m_pThread->ResumeThread();
		}
	}

	// KBJ 2022-08-26
	CString str = "Exit Program";
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	// KBJ 2022-09-01
	CFormMainView* pForm = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];

	//kmb 220811 Auto ���¿��� ������ ����� Exit ���ϰ� ����ó��
	for (int nJob = 0; nJob < m_pMain->vt_job_info.size(); nJob++)
	{
		if (m_pMain->getProcessStartFlag(nJob))			bProcessing = TRUE;
		if (pForm->m_bHandShake[nJob] == TRUE)			bProcessing = TRUE;
	}

	if (bAuto == TRUE && bProcessing == TRUE)
	{
		//AfxMessageBox("Wait!! Processing is not finished.");
		m_pMain->fnSetMessage(1, "Wait!! Processing is not finished.");
		return;
	}

	if (pMain->m_pThread != NULL) // Tkyuha 20211124 �ڵ� ���� �ǵ��� ����� �س��´�.
	{
		pMain->m_pThread->ResumeThread();
	}

	if (bAuto == TRUE ) OnBnClickedBtnAutoStop();

	m_pMain->m_bProgramEnd = TRUE;
	Delay(100);

	::SendMessageA(m_pMain->m_pForm[FORM_MAIN]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_RELEASE_OBJECT, 0);

	//TNWJDDL ���α׷� ����� ���� OFF

	for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
	{
		m_pMain->LightUse(m_pMain->LIGHT_OFF, i);
	}

	GetParent()->SendMessage(WM_CLOSE);
}


void CPaneBottom::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CPen pen(PS_SOLID, 3, COLOR_DDARK_GRAY), *pOld;

	pOld = dc.SelectObject(&pen);
	dc.MoveTo(m_rcForm.left, m_rcForm.top);
	dc.LineTo(m_rcForm.right, m_rcForm.top);
	dc.LineTo(m_rcForm.right, m_rcForm.bottom);
	dc.LineTo(m_rcForm.left, m_rcForm.bottom);
	dc.LineTo(m_rcForm.left, m_rcForm.top);
	dc.SelectObject(pOld);
}

void CPaneBottom::SetAutoStartStop(BOOL bAuto)
{
	m_pMain->m_bAutoStart = bAuto;

	if (bAuto)
	{
		m_btnAutoStart.SetColorBkg(255, COLOR_BTN_SELECT);
		m_btnAutoStop.SetColorBkg(255, COLOR_BTN_BODY);
		//kmb 220811 Auto �߿��� EXIT ��ư Ȱ��ȭ
		//m_btnExit.SetEnable(true);
	}
	else
	{
		m_btnAutoStart.SetColorBkg(255, COLOR_BTN_BODY);
		m_btnAutoStop.SetColorBkg(255, COLOR_BTN_SELECT);
		//m_btnExit.SetEnable(true);
	}
}
