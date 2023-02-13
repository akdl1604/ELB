#include "stdafx.h"
#include "LET_AlignClient.h"
#include "PaneBottomSimulation.h"
#include "LET_AlignClientDlg.h"

IMPLEMENT_DYNCREATE(CPaneBottomSimulation, CFormView)

CPaneBottomSimulation::CPaneBottomSimulation()
	: CFormView(CPaneBottomSimulation::IDD)
{
	m_bSimulStopFlag = TRUE;
	m_bUseSimulMessage = FALSE;
}

CPaneBottomSimulation::~CPaneBottomSimulation()
{
	m_vtDirPath.clear();
}

#ifdef _DEBUG
void CPaneBottomSimulation::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPaneBottomSimulation::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

void CPaneBottomSimulation::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_BTN_SIMULATION_BOTTOM_START, m_btnSimulationStart);
	DDX_Control(pDX, IDC_BTN_SIMULATION_STOP, m_btnSimulationStop);
	DDX_Control(pDX, IDC_BTN_EXIT, m_btnExit);
}

void CPaneBottomSimulation::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);
	m_pMain = (CLET_AlignClientDlg *)AfxGetMainWnd();
	GetClientRect(&m_rcForm);
	MainButtonInit(&m_btnSimulationStart);
	MainButtonInit(&m_btnSimulationStop);
	MainButtonInit(&m_btnExit);

	update_frame_dialog();
}

HBRUSH CPaneBottomSimulation::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	return m_hbrBkg;
}


BEGIN_MESSAGE_MAP(CPaneBottomSimulation, CFormView)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BTN_SIMULATION_BOTTOM_START, &CPaneBottomSimulation::OnBnClickedBtnSimulation)
	ON_BN_CLICKED(IDC_BTN_SIMULATION_STOP, &CPaneBottomSimulation::OnBnClickedBtnSimulationStop)
	ON_BN_CLICKED(IDC_BTN_EXIT, &CPaneBottomSimulation::OnBnClickedBtnExit)
END_MESSAGE_MAP()

void CPaneBottomSimulation::InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}

void CPaneBottomSimulation::MainButtonInit(CButtonEx *pbutton, int nID, int size)
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

void CPaneBottomSimulation::OnPaint()
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

void CPaneBottomSimulation::OnBnClickedBtnExit()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	//if (m_pMain->fnSetMessage(2, "Simulation Exit?") != TRUE)	return;
	//::SendMessageA(m_pMain->m_pForm[FORM_MAIN]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_INIT_INTERFACE, 0);

	for (int nJob = 0; nJob < m_pMain->vt_job_info.size(); nJob++)
	{
		m_pMain->m_nSeqProcess[nJob] = 0;
		m_pMain->resetProcessStartFlag(nJob);
		Sleep(20);
	}
	
	m_pMain->m_bSimulationStart = FALSE;
	m_pMain->changePane(PANE_MANUAL);
	m_pMain->changeBottomPane(B_PANE_BOTTOM);

	// YCS 2022-08-25 탄착군 다이얼로그 숨김
	if (m_pMain->vt_job_info[0].algo_method == CLIENT_TYPE_FILM_INSP)
	{
		for (int i = 0; i < 2; i++)
		{
			m_pMain->m_pCInspChartDlg[i]->ShowWindow(SW_HIDE);
		}
	}
}

void CPaneBottomSimulation::OnBnClickedBtnSimulationStop()
{
	m_bSimulStopFlag = TRUE;

	CPaneSimulation *pView = (CPaneSimulation *)m_pMain->m_pPane[PANE_SIMULATION];
	pView->m_bSimulStopFlag = TRUE;
	pView->update_frame_dialog();

	update_frame_dialog();

	// YCS 2022-08-25 탄착군 다이얼로그 숨김
	if (m_pMain->vt_job_info[0].algo_method == CLIENT_TYPE_FILM_INSP)
	{
		for (int i = 0; i < 2; i++)
		{
			m_pMain->m_pCInspChartDlg[i]->ShowWindow(SW_HIDE);
		}
	}
}

void CPaneBottomSimulation::OnBnClickedBtnSimulation()
{
	if( m_pMain->m_nCurPane != PANE_SIMULATION) m_pMain->changePane(PANE_SIMULATION);
	else										m_pMain->changePane(PANE_AUTO);

	update_frame_dialog();

	// YCS 2022-08-25 탄착군 다이얼로그 숨김
	if (m_pMain->vt_job_info[0].algo_method == CLIENT_TYPE_FILM_INSP)
	{
		for (int i = 0; i < 2; i++)
		{
			m_pMain->m_pCInspChartDlg[i]->ShowWindow(SW_HIDE);
		}
	}
	/*
	// Tab으로 Job 판단
	CFormMainView *pMain = (CFormMainView *)m_pMain->m_pForm[FORM_MAIN];
	int nJob	= pMain->m_TabMainView.GetCurSel() - 1;
	//if (nJob < 0) nJob = 0;	// TotalTab은 첫번째 Job으로 설정

	CString strFilePath;

	if (nJob < 0)
	{
		//폴더 불러오기
		CFolderPickerDialog dlg;
		if (dlg.DoModal() != IDOK)
		{
			return;
		}
		strFilePath = dlg.GetPathName();
		nJob = get_job_from_img(strFilePath);

		if (nJob < 0) nJob = 0;
	}
	else
	{
		SYSTEMTIME time;
		::GetLocalTime(&time);

		CString strTime, str_modelID, str_algo, strImageDir;
		//날짜
		strTime.Format("%04d%02d%02d", time.wYear, time.wMonth, time.wDay);
		strImageDir.Format("%s%s", m_pMain->m_strImageDir, strTime);

		//모델ID
		str_modelID.Format("%s", m_pMain->vt_job_info[nJob].model_info.getModelID());
		strImageDir.Format("%s\\%s", strImageDir, str_modelID);

		//JOB이름
		str_algo.Format(_T("%s"), m_pMain->vt_job_info[nJob].job_name.c_str());
		strImageDir.Format("%s\\%s\\", strImageDir, str_algo);

		//폴더 불러오기
		CFolderPickerDialog dlg(strImageDir);
		if (dlg.DoModal() != IDOK)
		{
			return;
		}
		strFilePath = dlg.GetPathName();
	}

	//폴더 경로 불러오기
	if (get_directory_path(m_vtDirPath, strFilePath, nJob))
	{

		
		
		str.Format("Simulation start");
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
	else
	{
		return;
	}

	set_simulation_start_stop(TRUE);

	int nMethod = m_pMain->vt_job_info[nJob].algo_method;
	switch (nMethod)
	{
		case CLIENT_TYPE_2CAM_EACH_2SHOT_ALIGN:
		{
			simulation_2cam_2shot_align(nJob);
		}
		break;
		case CLIENT_TYPE_4CAM_1SHOT_ALIGN:
		{
			simulation_4cam_1shot_align(nJob);
		}
		break;
		case CLIENT_TYPE_PANEL_EXIST_INSP:
		{
			simulation_exist_isnp(nJob);
		}
		break;
		case CLIENT_TYPE_PCB_DISTANCE_INSP:
		{
			m_pMain->m_nSeqTotalInspGrabCount = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getTotalGrabCount();
			m_pMain->set_pcb_total_point(nJob, m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getTotalGrabCount());
			simulation_pcb_bending_insp(nJob);
		}
		break;
		case CLIENT_TYPE_ASSEMBLE_INSP:
		{
			simulation_assemble_insp(nJob);
		}
		break;
		default:
		{
			simulation_2cam_1shot_align(nJob);
		}
		break;
	}

	m_vtDirPath.clear();

	
	
	str.Format("Simulation End");
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	set_simulation_start_stop(FALSE);

	Delay(50);
	m_pMain->m_bSimulationStart = FALSE;
	*/
}

void CPaneBottomSimulation::set_simulation_start_stop(BOOL bSimulation)
{
	m_pMain->m_bSimulationStart = bSimulation;
	m_bSimulStopFlag = !bSimulation;

	update_frame_dialog();
}

void CPaneBottomSimulation::update_frame_dialog()
{
	if (m_bSimulStopFlag != TRUE)
	{
		m_btnSimulationStart.SetColorBkg(255, COLOR_BTN_SELECT);
		m_btnSimulationStop.SetColorBkg(255, COLOR_BTN_BODY);
		m_btnExit.SetEnable(false);
	}
	else
	{
		m_btnSimulationStart.SetColorBkg(255, COLOR_BTN_BODY);
		m_btnSimulationStop.SetColorBkg(255, COLOR_BTN_SELECT);
		m_btnExit.SetEnable(true);
	}
}