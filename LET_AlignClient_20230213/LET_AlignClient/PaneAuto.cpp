// PaneAuto.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "PaneAuto.h"
#include "LET_AlignClientDlg.h"
#include "DlgSearchView.h"
#include "INIReader.h"		// LYS 2023-01-03

IMPLEMENT_DYNCREATE(CPaneAuto, CFormView)

CPaneAuto::CPaneAuto()
	: CFormView(CPaneAuto::IDD)
{
	InitializeCriticalSection(&m_csProcessHistory);
	InitializeCriticalSection(&m_csGridDraw);
	m_pMain = NULL;
	nresultcnt = 0;
	m_bELBInspJudge = FALSE;
	m_nObject = 0;
	m_nAlignCount=0;
	m_nInspCount=0;

	for (int i = 0; i < MAX_JOB; i++)
	{
		m_nAlignAlgo[i];
		m_nInspAlgo[i];
	}

	m_bInsp = FALSE;
	for (int i = 0; i < MAX_JOB; i++)
	{
		m_LastReusltDate[i] = "";
	}
}

CPaneAuto::~CPaneAuto()
{
	DeleteCriticalSection(&m_csProcessHistory);
	DeleteCriticalSection(&m_csGridDraw);
}

void CPaneAuto::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PROCESS_HISTORY, m_EditProcess);
	DDX_Control(pDX, IDC_STATIC_DAILY_PRODUCTION, m_stt_DailyProduction);

	DDX_Control(pDX, IDC_STATIC_1, m_stt_Data_Title);
	DDX_Control(pDX, IDC_STATIC_2, m_stt_Data_Title_GY);
	DDX_Control(pDX, IDC_STATIC_3, m_stt_Data_Title_Day);
	DDX_Control(pDX, IDC_STATIC_4, m_stt_Data_Title_SW);
	DDX_Control(pDX, IDC_STATIC_5, m_stt_Data_Title_Count_Total);
	DDX_Control(pDX, IDC_STATIC_6, m_stt_Data_Title_Percent);

	DDX_Control(pDX, IDC_STATIC_NG, m_stt_Data_Title_NG);
	DDX_Control(pDX, IDC_STATIC_OK, m_stt_Data_Title_OK);
	DDX_Control(pDX, IDC_STATIC_TOTAL, m_stt_Data_Title_NG_Total);

	DDX_Control(pDX, IDC_STATIC_NG_GY, m_stt_Data_NG_GY);
	DDX_Control(pDX, IDC_STATIC_NG_DAY, m_stt_Data_NG_Day);
	DDX_Control(pDX, IDC_STATIC_NG_SW, m_stt_Data_NG_SW);
	DDX_Control(pDX, IDC_STATIC_NG_TOTAL, m_stt_Data_NG_Total);
	DDX_Control(pDX, IDC_STATIC_NG_PERCENT, m_stt_Data_NG_Percent);

	DDX_Control(pDX, IDC_STATIC_OK_GY, m_stt_Data_OK_GY);
	DDX_Control(pDX, IDC_STATIC_OK_DAY, m_stt_Data_OK_Day);
	DDX_Control(pDX, IDC_STATIC_OK_SW, m_stt_Data_OK_SW);
	DDX_Control(pDX, IDC_STATIC_OK_TOTAL, m_stt_Data_OK_Total);
	DDX_Control(pDX, IDC_STATIC_OK_PERCENT, m_stt_Data_OK_Percent);

	DDX_Control(pDX, IDC_STATIC_TOTAL_GY, m_stt_Data_Total_GY);
	DDX_Control(pDX, IDC_STATIC_TOTAL_DAY, m_stt_Data_Total_Day);
	DDX_Control(pDX, IDC_STATIC_TOTAL_SW, m_stt_Data_Total_SW);
	DDX_Control(pDX, IDC_STATIC_TOTAL_TOTAL, m_stt_Data_Total_Total);
	DDX_Control(pDX, IDC_STATIC_TOTAL_PERCENT, m_stt_Data_Total_Percent);
	DDX_Control(pDX, IDC_STATIC_INSPECTION, m_lblIInspectionResult);

	DDX_Control(pDX, IDC_LIST_INSPECTION, m_listInspection[0]);
	DDX_Control(pDX, IDC_LIST_INSPECTION2, m_listInspection[1]);
	DDX_Control(pDX, IDC_LIST_INSPECTION3, m_listInspection[2]);
	DDX_Control(pDX, IDC_LIST_INSPECTION4, m_listInspection[3]);
	DDX_Control(pDX, IDC_LIST_INSPECTION5, m_listInspection[4]);
	DDX_Control(pDX, IDC_LIST_INSPECTION6, m_listInspection[5]);
	DDX_Control(pDX, IDC_LIST_INSPECTION7, m_listInspection[6]);
	DDX_Control(pDX, IDC_LIST_INSPECTION8, m_listInspection[7]);

	DDX_Control(pDX, IDC_STATIC_PROCESS_HISTORY, m_lblHistoryLabel);
	DDX_Control(pDX, IDC_AUTO_RESULT_GRAPH, m_AutoChartCtrl);
	DDX_Control(pDX, IDC_BTN_AUTO_MAIN, m_btnMainView);
	DDX_Control(pDX, IDC_BTN_AUTO_PLC_INTERFACE, m_btnInterfaveView);
	DDX_Control(pDX, IDC_BTN_AUTO_TREND, m_btnMainTrendView);
	DDX_Control(pDX, IDC_BTN_AUTO_RESULT_LIST, m_btnResultList);

	DDX_Control(pDX, IDC_GRID_SPEC_VIEWER, m_grid_spec_viewer);
	DDX_Control(pDX, IDC_GRID_INSP_SPEC_VIEWER, m_grid_insp_spec_viewer);
	DDX_Control(pDX, IDC_GRID_REF_DATA_VIEWER, m_grid_ref_data_viewer);
	DDX_Control(pDX, IDC_GRID_PRCO_TIME_VIEWER, m_grid_proc_time_viewer);
	DDX_Control(pDX, IDC_STATIC_SPEC, m_lblSpec);
	DDX_Control(pDX, IDC_STATIC_MICROFLUDIC_VALUE, m_lblDopoValue);
	DDX_Control(pDX, IDC_STATIC_LEFT_PANEL_JUDGE, m_lblLeftPanelJudge);
	DDX_Control(pDX, IDC_STATIC_RIGHT_PANEL_JUDGE, m_lblRightPanelJudge);
	DDX_Control(pDX, IDC_STATIC_MICROFLUDIC_VALUE2, m_lblSuckValue);
	DDX_Control(pDX, IDC_TAB_SPECVIEW, m_TabSpecView);

	//KJH 2021-08-07 검사 Spec / Insp Result 분기
	DDX_Control(pDX, IDC_GRID_INSP_RESULT_1, m_grid_InspResult_1_viewer);
	DDX_Control(pDX, IDC_GRID_INSP_RESULT_2, m_grid_InspResult_2_viewer);
	DDX_Control(pDX, IDC_GRID_INSP_RESULT_3, m_grid_InspResult_3_viewer);

	//HSJ 2021-12-25 결과창 TAB으로
	DDX_Control(pDX, IDC_TAB_RESULTVIEW, m_TabResultView);

	DDX_Control(pDX, IDC_BTN_AUTO_CPU, m_lblCpuUse);
	DDX_Control(pDX, IDC_BTN_AUTO_MEMORY, m_lblMemoryUse);
	DDX_Control(pDX, IDC_BTN_AUTO_CDRIVE, m_lblCDriveUse);
	DDX_Control(pDX, IDC_BTN_AUTO_DDRIVE, m_lblDDriveUse);

	DDX_Control(pDX, IDC_PROGRESS_CPU, m_pCpuProgressCtrl);
	DDX_Control(pDX, IDC_PROGRESS_MEMORY, m_pMemoryProgressCtrl);
	DDX_Control(pDX, IDC_PROGRESS_CDRIVE, m_pCDriveProgressCtrl);
	DDX_Control(pDX, IDC_PROGRESS_DDRIVE, m_pDDriveProgressCtrl);
}

BEGIN_MESSAGE_MAP(CPaneAuto, CFormView)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_VIEW_CONTROL, OnViewControl)
	ON_MESSAGE(WM_MODEL_CHANGE, &CPaneAuto::OnModelChange)
	ON_WM_PAINT()
	ON_STN_DBLCLK(IDC_STATIC_DAILY_PRODUCTION, &CPaneAuto::OnStnDblclickStaticDailyProduction)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_INSPECTION, OnNMCustomdrawListControl)
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_BTN_AUTO_MAIN, &CPaneAuto::OnBnClickedBtnAutoMain)
	ON_BN_CLICKED(IDC_BTN_AUTO_PLC_INTERFACE, &CPaneAuto::OnBnClickedBtnAutoPlcInterface)
	//ON_NOTIFY(NM_RDBLCLK, IDC_LIST_INSPECTION, &CPaneAuto::OnNMRDblclkListInspection)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_INSPECTION, &CPaneAuto::OnNMDblclkListInspection)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_INSPECTION2, &CPaneAuto::OnNMDblclkListInspection2)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_INSPECTION3, &CPaneAuto::OnNMDblclkListInspection3)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_INSPECTION4, &CPaneAuto::OnNMDblclkListInspection4)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_INSPECTION5, &CPaneAuto::OnNMDblclkListInspection5)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_INSPECTION6, &CPaneAuto::OnNMDblclkListInspection6)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_INSPECTION7, &CPaneAuto::OnNMDblclkListInspection7)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_INSPECTION8, &CPaneAuto::OnNMDblclkListInspection8)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_SPECVIEW, &CPaneAuto::OnTcnSelchangeTabSpecview)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULTVIEW, &CPaneAuto::OnTcnSelchangeTabResultview)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_INSPECTION2, &CPaneAuto::OnNMCustomdrawListInspection2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_INSPECTION3, &CPaneAuto::OnNMCustomdrawListInspection3)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_INSPECTION4, &CPaneAuto::OnNMCustomdrawListInspection4)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_INSPECTION5, &CPaneAuto::OnNMCustomdrawListInspection5)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_INSPECTION6, &CPaneAuto::OnNMCustomdrawListInspection6)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_INSPECTION7, &CPaneAuto::OnNMCustomdrawListInspection7)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_INSPECTION8, &CPaneAuto::OnNMCustomdrawListInspection8)
	ON_BN_CLICKED(IDC_BTN_AUTO_TREND, &CPaneAuto::OnBnClickedBtnAutoTrend)
	ON_BN_CLICKED(IDC_BTN_AUTO_RESULT_LIST, &CPaneAuto::OnBnClickedBtnAutoResultList)
END_MESSAGE_MAP()

// CPaneAuto 진단입니다.

#ifdef _DEBUG
void CPaneAuto::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPaneAuto::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CPaneAuto 메시지 처리기입니다.

void CPaneAuto::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	m_pMain = (CLET_AlignClientDlg*)AfxGetMainWnd();
	CModel* pModel = &m_pMain->vt_job_info[0].model_info;
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);
	GetClientRect(&m_rcForm);

	InitTitle(&m_stt_Data_Title, 14.f, RGB(190, 190, 190), RGB(0, 0, 0));

	InitTitle(&m_stt_Data_Title_GY, 14.f, RGB(190, 190, 255), RGB(0, 0, 0));
	InitTitle(&m_stt_Data_Title_Day, 14.f, RGB(190, 190, 255), RGB(0, 0, 0));
	InitTitle(&m_stt_Data_Title_SW, 14.f, RGB(190, 190, 255), RGB(0, 0, 0));

	InitTitle(&m_stt_Data_NG_GY, 14.f, RGB(190, 190, 255), RGB(128, 0, 0));
	InitTitle(&m_stt_Data_OK_GY, 14.f, RGB(190, 190, 255), RGB(0, 0, 128));
	InitTitle(&m_stt_Data_Total_GY, 14.f, RGB(190, 190, 255), RGB(0, 0, 0));

	InitTitle(&m_stt_Data_NG_Day, 14.f, RGB(190, 190, 255), RGB(128, 0, 0));
	InitTitle(&m_stt_Data_OK_Day, 14.f, RGB(190, 190, 255), RGB(0, 0, 128));
	InitTitle(&m_stt_Data_Total_Day, 14.f, RGB(190, 190, 255), RGB(0, 0, 0));

	InitTitle(&m_stt_Data_NG_SW, 14.f, RGB(190, 190, 255), RGB(128, 0, 0));
	InitTitle(&m_stt_Data_OK_SW, 14.f, RGB(190, 190, 255), RGB(0, 0, 128));
	InitTitle(&m_stt_Data_Total_SW, 14.f, RGB(190, 190, 255), RGB(0, 0, 0));

	InitTitle(&m_stt_Data_Title_Count_Total, 14.f, RGB(128, 128, 192), RGB(0, 0, 0));

	InitTitle(&m_stt_Data_NG_Total, 14.f, RGB(128, 128, 192), RGB(128, 0, 0));
	InitTitle(&m_stt_Data_OK_Total, 14.f, RGB(128, 128, 192), RGB(0, 0, 128));
	InitTitle(&m_stt_Data_Total_Total, 14.f, RGB(128, 128, 192), RGB(0, 0, 0));

	InitTitle(&m_stt_Data_Title_Percent, 14.f, RGB(255, 128, 128), RGB(0, 0, 0));

	InitTitle(&m_stt_Data_NG_Percent, 14.f, RGB(255, 128, 128), RGB(128, 0, 0));
	InitTitle(&m_stt_Data_OK_Percent, 14.f, RGB(255, 128, 128), RGB(0, 0, 128));
	InitTitle(&m_stt_Data_Total_Percent, 14.f, RGB(255, 128, 128), RGB(0, 0, 0));

	InitTitle(&m_stt_Data_Title_NG, 14.f, RGB(190, 190, 190), RGB(128, 0, 0));
	InitTitle(&m_stt_Data_Title_OK, 14.f, RGB(190, 190, 190), RGB(0, 0, 128));
	InitTitle(&m_stt_Data_Title_NG_Total, 14.f, RGB(190, 190, 190), RGB(0, 0, 0));

	InitTitle(&m_stt_DailyProduction, 14.f, RGB(128, 128, 192), RGB(0, 0, 0));

	InitTitle(&m_stt_DailyProduction, 14.f, RGB(128, 128, 192), RGB(0, 0, 0));
	InitTitle(&m_stt_DailyProduction, 14.f, RGB(128, 128, 192), RGB(0, 0, 0));
	InitTitle(&m_stt_DailyProduction, 14.f, RGB(128, 128, 192), RGB(0, 0, 0));

	InitTitle(&m_lblIInspectionResult, 14.f, COLOR_BTN_BODY, RGB(255, 255, 255));
	InitTitle(&m_lblHistoryLabel, 14.f, COLOR_BTN_BODY, RGB(255, 255, 255));
	InitTitle(&m_lblSpec, 14.f, COLOR_BTN_BODY, RGB(255, 255, 255));

	InitTitle(&m_lblDopoValue, 12.f, COLOR_BTN_BODY, RGB(255, 255, 255));
	InitTitle(&m_lblSuckValue, 12.f, COLOR_BTN_BODY, RGB(255, 255, 255));
	InitTitle(&m_lblLeftPanelJudge, 50.f, COLOR_BTN_BODY, RGB(0, 255, 0));
	InitTitle(&m_lblRightPanelJudge, 50.f, COLOR_BTN_BODY, RGB(0, 255, 0));

	InitTitle(&m_lblCpuUse, 12.f, COLOR_BTN_BODY, RGB(255, 255, 255));
	InitTitle(&m_lblMemoryUse, 12.f, COLOR_BTN_BODY, RGB(255, 255, 255));
	InitTitle(&m_lblCDriveUse, 12.f, COLOR_BTN_BODY, RGB(255, 255, 255));
	InitTitle(&m_lblDDriveUse, 12.f, COLOR_BTN_BODY, RGB(255, 255, 255));

	MainButtonInit(&m_btnMainView);
	MainButtonInit(&m_btnInterfaveView);
	MainButtonInit(&m_btnMainTrendView);
	MainButtonInit(&m_btnResultList);
	
	m_pCpuProgressCtrl.SetRange(0, 100);
	m_pMemoryProgressCtrl.SetRange(0, 1000); // To make percentage more smooth
	m_pCDriveProgressCtrl.SetRange(0, 100);
	m_pDDriveProgressCtrl.SetRange(0, 100);
	m_pCpuProgressCtrl.SetBarColor(RGB(128, 100, 230));
	m_pMemoryProgressCtrl.SetBarColor(RGB(128, 100, 230));
	m_pCDriveProgressCtrl.SetBarColor(RGB(128, 100, 230));
	m_pDDriveProgressCtrl.SetBarColor(RGB(128, 100, 230));
	m_pCpuProgressCtrl.SetBkColor(COLOR_UI_BODY);
	m_pMemoryProgressCtrl.SetBkColor(COLOR_UI_BODY);
	m_pCDriveProgressCtrl.SetBkColor(COLOR_UI_BODY);
	m_pDDriveProgressCtrl.SetBkColor(COLOR_UI_BODY);

	/*	m_listInspection.InsertColumn(0, "ID", LVCFMT_LEFT, 36);
		m_listInspection.InsertColumn(1, "1", LVCFMT_LEFT, 80);
		m_listInspection.InsertColumn(2, "2", LVCFMT_LEFT, 80);
		m_listInspection.InsertColumn(3, "3", LVCFMT_LEFT, 80);
		m_listInspection.InsertColumn(4, "4", LVCFMT_LEFT, 80);
		m_listInspection.InsertColumn(5, "1-2", LVCFMT_LEFT, 80);
		m_listInspection.InsertColumn(6, "3-4", LVCFMT_LEFT, 80);
		m_listInspection.InsertColumn(7, "Theta", LVCFMT_LEFT, 80);
		m_listInspection.InsertColumn(8, "JUDGE", LVCFMT_LEFT, 80);
		*/

	//결과창 초기화
	int nMetho = m_pMain->vt_job_info[0].algo_method;

	
	if (nMetho!= CLIENT_TYPE_ELB_CENTER_ALIGN)
	{
		m_lblDopoValue.ShowWindow(SW_HIDE);
		m_lblSuckValue.ShowWindow(SW_HIDE);		
	}
	else m_lblSpec.SetText("Wetout / Sub Insp");

	if (nMetho == CLIENT_TYPE_SCAN_INSP)
	{
		m_TabSpecView.InsertItem(0, "Spec");
		m_TabSpecView.InsertItem(1, "Insp-1");
		m_TabSpecView.InsertItem(2, "Insp-2");
		m_TabSpecView.InsertItem(3, "Insp-3");
		m_TabSpecView.SetCurSel(0);
	}
	else if (nMetho == CLIENT_TYPE_FILM_INSP)
	{
		// hsj 2022-01-10 align spec, inspection spec 창 구별
		m_TabSpecView.InsertItem(0, "Align Spec");
		m_TabSpecView.InsertItem(1, "Insp Spec");
		m_TabSpecView.InsertItem(2, "Attach1");
		m_TabSpecView.InsertItem(3, "Attach2");
		// hsj 2022-01-10 사용 안해서 우선 주석처리
		//m_TabSpecView.InsertItem(2, "RefData");
		m_TabSpecView.SetCurSel(0);
	}
	else if (nMetho == CLIENT_TYPE_ELB_CENTER_ALIGN)
	{
		m_TabSpecView.InsertItem(0, "Spec");
		m_TabSpecView.InsertItem(1, "Graph");
		// hsj 2022-01-10 사용 안해서 우선 주석처리
		m_TabSpecView.InsertItem(2, "RefData");
		m_TabSpecView.InsertItem(3, "Process Time");
		m_TabSpecView.SetCurSel(0);
	}
	else
	{
		m_TabSpecView.InsertItem(0, "Spec");
		m_TabSpecView.InsertItem(1, "Graph");
		// hsj 2022-01-10 사용 안해서 우선 주석처리
		//m_TabSpecView.InsertItem(2, "RefData");
		//m_TabSpecView.InsertItem(3, "Process Time");
		m_TabSpecView.SetCurSel(0);
	}

	CRect rt;
	m_TabSpecView.GetWindowRect(&rt);
	ScreenToClient(&rt);
	m_grid_spec_viewer.SetWindowPos(NULL, rt.left, rt.top + 20, rt.Width(), rt.Height() + 55, SWP_SHOWWINDOW | SWP_NOZORDER);
	
	//KJH 2021-08-07 검사 INSP Result Grid 분기
	if (nMetho == CLIENT_TYPE_SCAN_INSP)
	{
		m_grid_InspResult_1_viewer.SetWindowPos(NULL, rt.left, rt.top + 20, rt.Width(), rt.Height() + 55, SWP_SHOWWINDOW | SWP_NOZORDER);
		m_grid_InspResult_2_viewer.SetWindowPos(NULL, rt.left, rt.top + 20, rt.Width(), rt.Height() + 55, SWP_SHOWWINDOW | SWP_NOZORDER);
		m_grid_InspResult_3_viewer.SetWindowPos(NULL, rt.left, rt.top + 20, rt.Width(), rt.Height() + 55, SWP_SHOWWINDOW | SWP_NOZORDER);
		m_grid_InspResult_1_viewer.ShowWindow(FALSE);
		m_grid_InspResult_2_viewer.ShowWindow(FALSE);
		m_grid_InspResult_3_viewer.ShowWindow(FALSE);
	}
	else if (nMetho == CLIENT_TYPE_FILM_INSP) // hsj 2022-01-10 align spec, inspection spec 창 구별
	{
		m_grid_insp_spec_viewer.SetWindowPos(NULL, rt.left, rt.top + 20, rt.Width(), rt.Height() + 55, SWP_SHOWWINDOW | SWP_NOZORDER);
		m_grid_insp_spec_viewer.ShowWindow(FALSE);
	}
	else
	{
		m_grid_ref_data_viewer.SetWindowPos(NULL, rt.left, rt.top + 20, rt.Width(), rt.Height() + 55, SWP_SHOWWINDOW | SWP_NOZORDER);
		m_grid_ref_data_viewer.ShowWindow(FALSE);
		m_grid_proc_time_viewer.SetWindowPos(NULL, rt.left, rt.top + 20, rt.Width(), rt.Height() + 55, SWP_SHOWWINDOW | SWP_NOZORDER);
		m_grid_proc_time_viewer.ShowWindow(FALSE);
	}

	//HSJ TAB초기화 
	CreateTabCount();

	for (int i = 0; i < MAX_JOB; i++)
	{
		m_bResultInit[i] = FALSE;
		m_listInspection[i].ShowWindow(FALSE);
	}

	m_listInspection[0].ShowWindow(TRUE);
	
	//HSJ 결과창 헤더 초기화
	CreateResultInit(0, nMetho);
	//CString strModelPath =  "C:\\LET_AlignClient\\Model\\TT_AMB612CH01\\CENTER_ALIGN"; //경로 수정 필요
	//CAlignInfo* alignInfo;
	
	CString strPath;
	CString strModelPath;

	TCHAR szValue[MAX_PATH];
	strPath.Format("%sLastModel.ini", m_pMain->m_strModelDir);
	::GetPrivateProfileStringA("MODEL", "LAST_MODEL", "DEFAULT", szValue, MAX_PATH, strPath);


	strModelPath.Format("%s%s\\%s", m_pMain->m_strModelDir, szValue, m_pMain->vt_job_info[0].job_name.c_str());
	CAlignInfo alignInfo = m_pMain->vt_job_info[0].model_info.getAlignInfo();

	CFileFind findFile;
	if (findFile.FindFile(strModelPath) != TRUE)
		std::string strSection = "ALIGN_INFO", strValue;
		INIReader ini(fmt_("%s/AlignInfo.ini", strModelPath.GetString()));
		std::string strSection = "ALIGN_INFO", strValue;

		int iVal = 0;
		iVal = ini.GetInteger(strSection, "MARK_FIND_METHOD", 0);

	int num_of_job = int(m_pMain->vt_job_info.size());

	if (nMetho == CLIENT_TYPE_ELB_CENTER_ALIGN)
	{
		// LYS 2023-01-03 Circle/Line Grid Col 생성 분리
		if (iVal == METHOD_LINE)
		{
			init_spec_view_setting(m_grid_spec_viewer, 10, df_MAX_CNT + 2);  // 11에서 8로 변경  Tkyuha 220318
			draw_grid_inspection_spec_viewer(m_grid_spec_viewer, 10, df_MAX_CNT + 2);   // 11에서 8로 변경  Tkyuha 220318
		}
		else
		{
			init_spec_view_setting(m_grid_spec_viewer, 9, df_MAX_CNT + 2);  // 11에서 8로 변경  Tkyuha 220318
			draw_grid_inspection_spec_viewer(m_grid_spec_viewer, 9, df_MAX_CNT + 2);   // 11에서 8로 변경  Tkyuha 220318
		}
		init_ref_data_view_setting(m_grid_ref_data_viewer, 3, 10);
		LoadRefData();
		draw_grid_ref_data_viewer(m_grid_ref_data_viewer, 3);
		init_ref_data_view_setting(m_grid_proc_time_viewer, 3, 10);	 // 공용 사용	
		m_grid_proc_time_viewer.SetEditable(FALSE);
		draw_grid_proc_time_viewer(m_grid_proc_time_viewer, 3);
	}
	else if (nMetho == CLIENT_TYPE_SCAN_INSP)
	{
		//Init Spec , Insp - 1, Insp - 2, Insp - 3
		//draw Spec , Insp - 1, Insp - 2, Insp - 3
		Init_Grid_Scan_Insp_Spec_View_Setting(m_grid_spec_viewer, 9, 7);
		Init_Grid_Scan_Insp_Result_View_Setting(m_grid_InspResult_1_viewer, 9, df_MAX_CNT + 3);
		Init_Grid_Scan_Insp_Result_View_Setting(m_grid_InspResult_2_viewer, 9, df_MAX_CNT + 3);
		Init_Grid_Scan_Insp_Result_View_Setting(m_grid_InspResult_3_viewer, 9, df_MAX_CNT + 3);
		Init_Draw_Grid_Scan_Insp_Spec_Viewer(m_grid_spec_viewer, 9, 7);
		Init_Draw_Grid_Scan_Insp_Result_1_Viewer(m_grid_InspResult_1_viewer, 9, df_MAX_CNT + 3);
		Init_Draw_Grid_Scan_Insp_Result_2_Viewer(m_grid_InspResult_2_viewer, 9, df_MAX_CNT + 3);
		Init_Draw_Grid_Scan_Insp_Result_3_Viewer(m_grid_InspResult_3_viewer, 9, df_MAX_CNT + 3);

		//Update_Grid_Scan_Insp_Spec();
	}
	else if (nMetho == CLIENT_TYPE_FILM_INSP) // hsj 2022-01-10 align spec, inspection spec 창 구별
	{
		int nMethod = 0;

		for (int i = 0; i < num_of_job; i++)
		{
			nMethod = m_pMain->vt_job_info[i].algo_method;
			if (nMethod == CLIENT_TYPE_FILM_INSP || nMethod == CLIENT_TYPE_SCAN_INSP)
			{
				m_bInsp = TRUE;
				m_nInspAlgo[m_nInspCount] = i;
				m_nInspCount++;
			}
			else
			{
				m_nAlignAlgo[m_nAlignCount] = i;
				m_nAlignCount++;
			}
		}

		init_spec_view_setting(m_grid_spec_viewer, 6, m_nAlignCount + 1);
		draw_grid_spec_viewer(m_grid_spec_viewer, 6, m_nAlignCount + 1);

		init_spec_view_setting(m_grid_insp_spec_viewer, 9, m_nInspCount + 1);
		draw_grid_insp_spec_viewer(m_grid_insp_spec_viewer, 9, m_nInspCount + 1);
	}
	else
	{
		init_spec_view_setting(m_grid_spec_viewer, 6, num_of_job + 1);
		draw_grid_spec_viewer(m_grid_spec_viewer, 6, num_of_job + 1);
	}

	SetTimer(0, 500, NULL);

	// YCS 2022-08-25 탭컨트롤 사이즈 get
	m_TabSpecView.GetWindowRect(&m_Rect);
}

HBRUSH CPaneAuto::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);
	if (pWnd->m_hWnd == this->m_hWnd) hbr = m_hbrBkg;

	if (pWnd->GetDlgCtrlID() == IDC_EDIT_PROCESS_HISTORY)
	{
		pDC->SetBkColor(COLOR_UI_BODY);
		pDC->SetTextColor(COLOR_WHITE);
	}

	return m_hbrBkg;
}

void CPaneAuto::InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}
void CPaneAuto::InitTitle(CLabelEx *pTitle, float size, COLORREF bgcolor, COLORREF color)
{
	CString str;
	pTitle->GetWindowTextA(str);

	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, bgcolor);
	pTitle->SetColorText(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}
void CPaneAuto::MainButtonInit(CButtonEx *pbutton, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
	pbutton->SetSizeText(17);
}
CString CPaneAuto::stringToCurrency(int nNum)
{
	int nLength, index = 0;
	CString strResult, str;

	str.Format("%d", nNum);

	str.TrimLeft();
	str.TrimRight();

	nLength = str.GetLength();

	if (nLength > 3) {
		for (int i = 0; i < ((nLength / 3) + 1); i++) {
			if (i == 0) {
				strResult = str.Left(nLength % 3);
				index = nLength % 3;
			}
			else {
				strResult += str.Mid(index, 3);
				index += 3;
			}

			if (i != (nLength / 3))
			{
				strResult += ",";
			}
		}
	}
	else {
		strResult = str;
	}

	return strResult;
}
LRESULT CPaneAuto::OnViewControl(WPARAM wParam, LPARAM lParam)
{
	CString str;

	switch (wParam) {
	case MSG_PA_ADD_PROCESS_HISTORY:
	{
		CString str;
		str.Format("%s\r\n", (*(CString*)lParam));
		addProcessHistory(str);
		theLog.logmsg(LOG_PROCESS, (*(CString*)lParam));
	}
	break;
	case MSG_PA_UPDATE_INSP_COUNT:
	{
		// m_InspCounter: 0 NG, 1 TOTAL			

		int nOkCount[3];// 0 DAY, 1 SW ,2 GY
		int nTotal[3];// 0 NG, 1 TOTAL ,2 OK
		for (int i = 0; i < 3; i++)
			nOkCount[i] = m_pMain->m_InspCounter.m_n1stPartTimeCount[i][1] - m_pMain->m_InspCounter.m_n1stPartTimeCount[i][0];

		nTotal[0] = m_pMain->m_InspCounter.m_n1stPartTimeCount[0][0] + m_pMain->m_InspCounter.m_n1stPartTimeCount[1][0] + m_pMain->m_InspCounter.m_n1stPartTimeCount[2][0];
		nTotal[2] = nOkCount[0] + nOkCount[1] + nOkCount[2];
		nTotal[1] = nTotal[0] + nTotal[2];

		str = stringToCurrency(m_pMain->m_InspCounter.m_n1stPartTimeCount[0][1]);
		m_stt_Data_Total_Day.SetText((LPCTSTR)str);
		str = stringToCurrency(m_pMain->m_InspCounter.m_n1stPartTimeCount[0][0]);
		m_stt_Data_NG_Day.SetText((LPCTSTR)str);
		str = stringToCurrency(nOkCount[0]);
		m_stt_Data_OK_Day.SetText((LPCTSTR)str);

		str = stringToCurrency(m_pMain->m_InspCounter.m_n1stPartTimeCount[1][1]);
		m_stt_Data_Total_SW.SetText((LPCTSTR)str);
		str = stringToCurrency(m_pMain->m_InspCounter.m_n1stPartTimeCount[1][0]);
		m_stt_Data_NG_SW.SetText((LPCTSTR)str);
		str = stringToCurrency(nOkCount[1]);
		m_stt_Data_OK_SW.SetText((LPCTSTR)str);

		str = stringToCurrency(m_pMain->m_InspCounter.m_n1stPartTimeCount[2][1]);
		m_stt_Data_Total_GY.SetText((LPCTSTR)str);
		str = stringToCurrency(m_pMain->m_InspCounter.m_n1stPartTimeCount[2][0]);
		m_stt_Data_NG_GY.SetText((LPCTSTR)str);
		str = stringToCurrency(nOkCount[2]);
		m_stt_Data_OK_GY.SetText((LPCTSTR)str);

		str = stringToCurrency(nTotal[1]);
		m_stt_Data_Total_Total.SetText((LPCTSTR)str);
		str = stringToCurrency(nTotal[0]);
		m_stt_Data_NG_Total.SetText((LPCTSTR)str);
		str = stringToCurrency(nTotal[2]);
		m_stt_Data_OK_Total.SetText((LPCTSTR)str);

		str = stringToCurrency(100);
		m_stt_Data_Total_Percent.SetText((LPCTSTR)str);
		if (!nTotal[1])str.Format("0");
		else str.Format("%0.1f", (float)nTotal[2] / (float)nTotal[1] * 100);
		m_stt_Data_OK_Percent.SetText((LPCTSTR)str);
		if (!nTotal[1])str.Format("0");
		else str.Format("%0.1f", (float)nTotal[0] / (float)nTotal[1] * 100);
		m_stt_Data_NG_Percent.SetText((LPCTSTR)str);

	}
	break;
	case MSG_PA_UPDATE_INSP_RESULT:
	{
		nresultcnt++;
		//DrawInspResultChart(int(lParam));
		DrawInsViewer(int(lParam));
	}
	break;
	case MSG_PA_UPDATE_ALIGN_RESULT:
	{
		nresultcnt++;
		DrawAlignResultChart(int(lParam));
		m_listInspection[int(lParam)].ChartColAsc(0, 1); // col, 오름차순:0 내림차순:1
	}
	break;
	case MSG_PA_RESET_RESULT_LIST:	// 하루지나면 Reset
	{
		int nJob = int(lParam);

		// 마지막 결과리스트 날짜와 현재 날짜 비교 
		if (m_pMain->m_strResultDate[nJob].GetLength() != 0 && m_pMain->m_strResultDate[nJob].Compare(m_LastReusltDate[nJob]) != 0)
		{
			m_listInspection[int(lParam)].DeleteAllItems();
			Invalidate();
			m_listInspection[int(lParam)].Complete();
		}
	}
	break;
	case MSG_PA_SPEC_VIEW:
	{
		if (lParam == 1)
		{
			draw_grid_inspection_spec_viewer(m_grid_spec_viewer, 9, 2);
			update_grid_inspection_spec();
		}
		else if (lParam == 2) update_grid_dustResult_spec();
		else DrawSpecChart();
	}
	break;
	case MSG_PA_SCAN_SPEC_VIEW:
	{
		int nJob = LOWORD(lParam);
		int nPos = HIWORD(lParam);
		DrawSpecChart_Scan_Insp(nJob, nPos);
	}
	break;
	case MSG_PA_TIME:
	{
		setResultTime(*(CString*)lParam);
	}
	break;
	case MSG_MF_UPDATE:
	{
		if (lParam == 0)
			m_lblDopoValue.SetText(*(CString*)(&m_pMain->m_strDopoValue));
		else if (lParam == 1)
			m_lblSuckValue.SetText(*(CString*)(&m_pMain->m_strSuckValue));
	}
	break;
	//KJH 2021-07-26 모델 스펙 변경에 따른 Grid 변경 추가
	case MSG_PA_JUDGE_DISPLAY:
	{
		int nPos = LOWORD(lParam);
		BOOL nJudge = HIWORD(lParam);

		if (nPos == 0)
		{
			m_lblLeftPanelJudge.SetText(nJudge ? "OK" : "NG");
			if (nJudge) m_lblLeftPanelJudge.SetColorText(255, RGB(0, 255, 0));
			else m_lblLeftPanelJudge.SetColorText(255, RGB(255, 0, 0));
		}
		else
		{
			m_lblRightPanelJudge.SetText(nJudge ? "OK" : "NG");
			if (nJudge) m_lblRightPanelJudge.SetColorText(255, RGB(0, 255, 0));
			else m_lblRightPanelJudge.SetColorText(255, RGB(255, 0, 0));
		}
	}
		break;
	// hsj 2022-02-11 판단기준으로 결과창 글씨 색 구분
	case MSG_PA_JUDGE_JOB:
	{
		int nJob = HIWORD(lParam);
		BOOL bJudge = LOWORD(lParam);
		
		int nMethod = m_pMain->vt_job_info[nJob].algo_method;

		if (nMethod == CLIENT_TYPE_1CAM_2POS_REFERENCE ||
			nMethod == CLIENT_TYPE_1CAM_4POS_ROBOT)
		{
			for(int i=0;i<2;i++)
				m_bJudge[nJob][m_listInspection[nJob].GetItemCount()+i] = bJudge;
		}
		else m_bJudge[nJob][m_listInspection[nJob].GetItemCount()] = bJudge;

	}
	break;
	// KBJ 2022-07-06 SpecGrid Title 재변경
	case MSG_PA_RENEW_TITLE:
	{
		int row = 0;
		int col = 5;

		GV_ITEM Item;
		Item.nFormat = DT_CENTER | DT_WORDBREAK;
		Item.mask = GVIF_TEXT | GVIF_FORMAT;
		Item.row = row;
		Item.col = col;

		//KJH 2022-05-25 Length -> BMinSpec으로 UI 변경
		//KJH 2022-06-06 R Display제거 / Length는 Line에서 사용함
		//KJH 2022-06-30 GridSpecView Length Title 변경
		int method = m_pMain->vt_job_info[0].model_info.getAlignInfo().getMarkFindMethod();
		if (method == METHOD_CIRCLE)	Item.strText.Format(_T("Lack"));
		else if (method == METHOD_LINE)		Item.strText.Format(_T("Length"));
		else								Item.strText.Format(_T("Length"));
		UINT state = m_grid_spec_viewer.GetItemState(row, col);
		m_grid_spec_viewer.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);

		m_grid_spec_viewer.SetItem(&Item);
		m_grid_spec_viewer.RedrawRow(row);
		m_grid_spec_viewer.ExpandColumnsToFit();

	}
	break;
	}

	return 0;
}
void CPaneAuto::addProcessHistory(CString str)
{
	EnterCriticalSection(&m_csProcessHistory);
	CString strTime;
	SYSTEMTIME	csTime;
	::GetLocalTime(&csTime);
	strTime.Format("[%02d:%02d:%02d.%03d] ", csTime.wHour, csTime.wMinute, csTime.wSecond, csTime.wMilliseconds);
	//strTime.Format("[%02d:%02d:%02d:%03d] ", csTime.wHour, csTime.wMinute, csTime.wSecond, csTime.wMilliseconds);

	if (m_strProcess.GetLength() > 20480) m_strProcess.Empty();

	m_strProcess = strTime + str + m_strProcess;
	m_EditProcess.SetWindowTextA(m_strProcess);

	LeaveCriticalSection(&m_csProcessHistory);
}
void CPaneAuto::OnPaint()
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
void CPaneAuto::OnStnDblclickStaticDailyProduction()
{
	BOOL bRet = m_pMain->fnSetMessage(2, "Reset the Daily Production?");

	if (bRet != TRUE) return;
	m_pMain->m_InspCounter.TodayCountReset();
}

//HSJ 2022-01-07 결과창 색깔 바꾸기
void CPaneAuto::OnNMCustomdrawListControl(NMHDR *pNMHDR, LRESULT *pResult)
{
	ChangeCustomResultList(0,pNMHDR, pResult);
}

void CPaneAuto::OnNMCustomdrawListInspection2(NMHDR* pNMHDR, LRESULT* pResult)
{
	ChangeCustomResultList(1,pNMHDR, pResult);
}

void CPaneAuto::OnNMCustomdrawListInspection3(NMHDR* pNMHDR, LRESULT* pResult)
{
	ChangeCustomResultList(2,pNMHDR, pResult);
}

void CPaneAuto::OnNMCustomdrawListInspection4(NMHDR* pNMHDR, LRESULT* pResult)
{
	ChangeCustomResultList(3,pNMHDR, pResult);
}

void CPaneAuto::OnNMCustomdrawListInspection5(NMHDR* pNMHDR, LRESULT* pResult)
{
	ChangeCustomResultList(4,pNMHDR, pResult);
}

void CPaneAuto::OnNMCustomdrawListInspection6(NMHDR* pNMHDR, LRESULT* pResult)
{
	ChangeCustomResultList(5,pNMHDR, pResult);
}

void CPaneAuto::OnNMCustomdrawListInspection7(NMHDR* pNMHDR, LRESULT* pResult)
{
	ChangeCustomResultList(6,pNMHDR, pResult);
}

void CPaneAuto::OnNMCustomdrawListInspection8(NMHDR* pNMHDR, LRESULT* pResult)
{
	ChangeCustomResultList(7,pNMHDR, pResult);
}

//HSJ 2022-01-07 결과창 색깔 바꾸기
void CPaneAuto::ChangeCustomResultList(int nJob,NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMCUSTOMDRAW pNMCD;
	pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	*pResult = 0;
	LPNMLVCUSTOMDRAW  lplvcd = (LPNMLVCUSTOMDRAW)pNMHDR;
	int iRow = (int)lplvcd->nmcd.dwItemSpec;
	switch (lplvcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
		return;
	}
	case CDDS_ITEMPREPAINT:
	{
		int nTotalRowCnt = m_listInspection[nJob].GetItemCount();

		if (m_listInspection[nJob].GetItemText(iRow, 2) == "NG")
		{
			lplvcd->clrText = RGB(255, 0, 0);
		}
		else
		{
			lplvcd->clrText = RGB(0, 0, 0);
		}

		*pResult = CDRF_NOTIFYSUBITEMDRAW;
		return;
	}
	case CDDS_SUBITEM | CDDS_PREPAINT | CDDS_ITEM:
	{
		if (iRow % 2) {
			lplvcd->clrTextBk = RGB(255, 255, 255);
		}
		else {
			lplvcd->clrTextBk = RGB(230, 230, 230);
		}

		*pResult = CDRF_DODEFAULT;
		return;
	}
	}
}

void CPaneAuto::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (IsClickDailyTitle(point) == TRUE)
	{
		if (!m_pMain->m_bAutoStart)
			OnStnDblclickStaticDailyProduction();
	}

	CFormView::OnLButtonUp(nFlags, point);
}
BOOL CPaneAuto::IsClickDailyTitle(CPoint pt)
{
	CRect IdRect, rect;
	GetWindowRect(&rect);
	m_stt_DailyProduction.GetWindowRect(&IdRect);

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
BOOL CPaneAuto::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	//if( pMsg->message == WM_LBUTTONDLBCLK )
	if (pMsg->message == WM_LBUTTONDBLCLK)
	{
		if (pMsg->hwnd == m_lblHistoryLabel.m_hWnd)
		{
			for (int nCam = 0; nCam < m_pMain->m_nNumCamera; nCam++)
				m_pMain->m_bLiveDisplay[nCam] = !m_pMain->m_bLiveDisplay[nCam];
		}
	}

	return CFormView::PreTranslateMessage(pMsg);
}
float CPaneAuto::GetCPKData(std::vector<float> vectorData, int AxisNum)
{
	float resultValue = 0.00;

	
	return abs(resultValue);
}
LRESULT CPaneAuto::OnModelChange(WPARAM wParam, LPARAM lParam)
{
	
	return 0;
}
////////////////////////////////////////////////////////////
void CPaneAuto::SetMinMaxValue(double alignSpec, double tolerance)
{
	
}
void CPaneAuto::UpdateChartData(std::vector<float> *vecData)
{
	
}
void CPaneAuto::OnBnClickedBtnAutoMain()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_pMain->changeForm(FORM_MAIN);
}
void CPaneAuto::OnBnClickedBtnAutoPlcInterface()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_pMain->changeForm(FORM_INTERFACE);
}
void CPaneAuto::DrawAlignResultChart(int(lParam))
{
	int algo = lParam;
	
	int nMetho = m_pMain->vt_job_info[algo].algo_method;

	switch (nMetho)
	{
	case CLIENT_TYPE_ALIGN:
	{
		Draw2camViewer(algo);
		
	}
	break;
	case CLIENT_TYPE_4CAM_1SHOT_ALIGN:
	{
		Draw4camViewer(algo);
	}
	break;
	case CLIENT_TYPE_ELB_CENTER_ALIGN:
	{
		DrawELBCenterViewer(algo);
	}
	break;
	case CLIENT_TYPE_1CAM_4POS_ROBOT:
	case CLIENT_TYPE_1CAM_2POS_REFERENCE:
	{
		//hsj 2022-01-09 ReelAlign 결과창 추가
		// hsj 2022-02-12 확인중
		for(int i=0;i<2;i++)
			DrawPFReelAlignViewer(algo,i);

		//if (m_nObject==0)
		//	m_nObject++;

	}
	break;
	case CLIENT_TYPE_CENTER_NOZZLE_ALIGN:
	{
		//hsj 2022-01-09 ReelAlign 결과창 추가
		DrawCenterNozzleAlignViewer(algo);
	}
	break;
	case CLIENT_TYPE_FILM_INSP:
	{
		// hsj 2022-01-09 검사 method에 따른 결과값 
		int method = m_pMain->vt_job_info[algo].model_info.getInspSpecParaInfo().getFilmInspMethod();

	/*	if (method == 4)
		{
			//for(int method =0; method <3; method++) DrawPFInspectionViewer(algo, method);
			//hsj 2022-02-12 All일때 그냥 센터 결과값만 나타내기
			DrawPFInspectionViewer(algo, 0);
		}
		else DrawPFInspectionViewer(algo, method);*/

		DrawPFInspectionViewer(algo, method);
	}
	break;
	case CLIENT_TYPE_CENTER_SIDE_YGAP:
	{
		DrawStageZGapViewer(algo, 3);
	}
	break;
	default:
	{
		DrawELBCenterViewer(algo);
	}
	break;
	}
	
}
void CPaneAuto::Draw2camViewer(int algo)
{
	int camCount = m_pMain->vt_job_info[algo].num_of_camera;
	//int nCamCount = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraNum(algo);
	int num = m_listInspection[algo].GetItemCount();

	//if (num > 50)  m_listInspection[algo].DeleteAllItems();

	CString stTime = "";
	CString stDate = "";
	CString stPanelID = "";
	CString strJudge = "";
	//CString stUnit = "";
	CString stRevisionDataX = "";
	CString stRevisionDataY = "";
	CString stRevisionDataT = "";
	CString stLength = "";

	m_listInspection[algo].AddItem("");

	int row = 0;


	if (m_pMain->m_bSimulationStart)
	{
		SYSTEMTIME	csTime;
		::GetLocalTime(&csTime);
		stTime.Format("%02d_%02d_%02d_S", csTime.wHour, csTime.wMinute, csTime.wSecond);
		stDate.Format("%04d%02d%02d", csTime.wYear, csTime.wMonth, csTime.wDay);
	}
	else
	{
		stTime.Format("%s", m_pMain->m_strResultTime[algo]);
		stDate.Format("%s", m_pMain->m_strResultDate[algo]);
	}
	m_listInspection[algo].SetItemText(num, row, stTime);
	m_LastReusltDate[algo] = stDate;

	row++;

	//패널아이디
	stPanelID.Format("%s", m_pMain->vt_job_info[algo].get_main_object_id());
	m_listInspection[algo].SetItemText(num, row, stPanelID);

	row++;

	strJudge.Format("%s", m_bJudge[algo][m_listInspection[algo].GetItemCount()] == TRUE ? "OK" : "NG");
	m_listInspection[algo].SetItemText(num, row, strJudge);

	row++;

	stRevisionDataX.Format("%.4f", m_pMain->m_dbRevisionData[algo][AXIS_X]);
	m_listInspection[algo].SetItemText(num, row, stRevisionDataX);

	row++;

	stRevisionDataY.Format("%.4f", m_pMain->m_dbRevisionData[algo][AXIS_Y]);
	m_listInspection[algo].SetItemText(num, row, stRevisionDataY);

	row++;

	stRevisionDataT.Format("%.4f", m_pMain->m_dbRevisionData[algo][AXIS_T]);
	m_listInspection[algo].SetItemText(num, row, stRevisionDataT);

	row++;

	stLength.Format("%.4f", m_pMain->m_dist);
	m_listInspection[algo].SetItemText(num, row, stLength);

	row++;

	m_listInspection[algo].SetItemText(num, row, "-");
	row++;

	m_listInspection[algo].SetItemText(num, row, "-");
	row++;

	m_listInspection[algo].SetItemText(num, row, "-");

	m_listInspection[algo].ChartColAsc(0, 1); // col, 오름차순:0 내림차순:1

	if (num > MAX_RESULT_ITEM - 1)
	{
		m_listInspection[algo].DeleteItem(num);
		num = m_listInspection[algo].GetItemCount();
	}

	m_listInspection[algo].Complete();
}
void CPaneAuto::Draw4camViewer(int algo)
{
	int camCount = m_pMain->vt_job_info[algo].num_of_camera;
//	int nCamCount = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraNum(algo);
	int num = m_listInspection[algo].GetItemCount();

	//if (num > 50)  m_listInspection[algo].DeleteAllItems();

	CString stIndex = "";
	CString stModel = "";
	CString stDate = "";
	//CString stUnit = "";
	CString stRevisionDataX = "";
	CString stRevisionDataY = "";
	CString stRevisionDataT = "";
	CString stTopLength = "";
	CString stBottomLength = "";
	CString stLeftLenght = "";
	CString stRightLenght = "";
	CString stTime = "";
	CString stPanelID = "";

	int row = 0;

	m_listInspection[algo].AddItem("");

	if (m_pMain->m_bSimulationStart)
	{
		SYSTEMTIME	csTime;
		::GetLocalTime(&csTime);
		stTime.Format("%02d_%02d_%02d_S", csTime.wHour, csTime.wMinute, csTime.wSecond);
		stDate.Format("%04d%02d%02d", csTime.wYear, csTime.wMonth, csTime.wDay);
	}
	else
	{
		stTime.Format("%s", m_pMain->m_strResultTime[algo]);
		stDate.Format("%s", m_pMain->m_strResultDate[algo]);
	}
	m_listInspection[algo].SetItemText(num, row, stTime);
	m_LastReusltDate[algo] = stDate;

	row++;

	//패널아이디
	stPanelID.Format("%s", m_pMain->vt_job_info[algo].get_main_object_id());
	m_listInspection[algo].SetItemText(num, row, stPanelID);

	row++;

	stRevisionDataX.Format("%.4f", m_pMain->m_dbRevisionData[algo][AXIS_X]);
	m_listInspection[algo].SetItemText(num, row, stRevisionDataX);

	row++;

	stRevisionDataY.Format("%.4f", m_pMain->m_dbRevisionData[algo][AXIS_Y]);
	m_listInspection[algo].SetItemText(num, row, stRevisionDataY);

	row++;

	stRevisionDataT.Format("%.4f", m_pMain->m_dbRevisionData[algo][AXIS_T]);
	m_listInspection[algo].SetItemText(num, row, stRevisionDataT);

	row++;

	stTopLength.Format("%.4f", m_pMain->m_dist_T);
	m_listInspection[algo].SetItemText(num, row, stTopLength);

	row++;

	stBottomLength.Format("%.4f", m_pMain->m_dist_B);
	m_listInspection[algo].SetItemText(num, row, stBottomLength);

	row++;

	stLeftLenght.Format("%.4f", m_pMain->m_dist_L);
	m_listInspection[algo].SetItemText(num, row, stLeftLenght);

	row++;

	stRightLenght.Format("%.4f", m_pMain->m_dist_R);
	m_listInspection[algo].SetItemText(num, row, stRightLenght);

	m_listInspection[algo].ChartColAsc(0, 1); // col, 오름차순:0 내림차순:1

	if (num > MAX_RESULT_ITEM - 1)
	{
		m_listInspection[algo].DeleteItem(num);
		num = m_listInspection[algo].GetItemCount();
	}


	m_listInspection[algo].Complete();
}

//PJH 2022-09-02 Z_GAP 결과창 추가
void CPaneAuto::DrawStageZGapViewer(int algo, int real_cam)
{
	int camCount = m_pMain->vt_job_info[algo].num_of_camera;
	int num = m_listInspection[algo].GetItemCount();

	BOOL bNozzleYAlignRecalc = m_pMain->vt_job_info[algo].model_info.getInspSpecParaInfo().getCenterNozzleYAlignRecalc();

	CString stIndex = "";
	CString stModel = "";
	CString strJudge = "";
	CString stRevisionDataZ = "";
	CString stRevisionDataZOri = "";
	CString stNozzleYAlignRecalc = "";
	CString stTime = "";
	CString stDate = "";

	int row = 0;

	// LYS 2022-09-05 ZGap Param 수정
	int cam = 0;
	std::vector<int> camBuf = m_pMain->vt_job_info[algo].camera_index;
	int ncam = camBuf.at(cam);

	bool _recalc = m_pMain->vt_job_info[algo].model_info.getInspSpecParaInfo().getCenterNozzleYAlignRecalc();

	int W = m_pMain->m_stCamInfo[ncam].w;
	int H = m_pMain->m_stCamInfo[ncam].h;

	double cetnerPtY = H / 2;
	double FindMark_Y = m_pMain->GetMatching(algo).getFindInfo(cam, 0).GetYPos();
	double yres = m_pMain->GetMachine(algo).getCameraResolutionY(cam, 0);
	double _offset = m_pMain->vt_job_info[algo].model_info.getInspSpecParaInfo().getNozzleToStageGapOffset();

	double zdist = fabs(cetnerPtY - FindMark_Y) * yres + _offset;
	// end

	m_listInspection[algo].AddItem("");
	if (m_pMain->m_bSimulationStart)
	{
		SYSTEMTIME	csTime;
		::GetLocalTime(&csTime);
		stTime.Format("%02d_%02d_%02d_S", csTime.wHour, csTime.wMinute, csTime.wSecond);
		stDate.Format("%04d%02d%02d", csTime.wYear, csTime.wMonth, csTime.wDay);
	}
	else
	{
		stTime.Format("%s", m_pMain->m_strResultTime[algo]);
		stDate.Format("%s", m_pMain->m_strResultDate[algo]);
	}
	m_listInspection[algo].SetItemText(num, row, stTime);
	m_LastReusltDate[algo] = stDate;

	row++;

	strJudge.Format("%s", m_bJudge[algo][m_listInspection[algo].GetItemCount()] == TRUE ? "OK" : "NG");
	m_listInspection[algo].SetItemText(num, row, strJudge);

	row++;

	stRevisionDataZ.Format("%.4f", zdist);
	m_listInspection[algo].SetItemText(num, row, stRevisionDataZ);

	row++;

	stRevisionDataZOri.Format("%.4f", zdist - _offset);
	m_listInspection[algo].SetItemText(num, row, stRevisionDataZOri);

	row++;

	stNozzleYAlignRecalc.Format("%s", _recalc == 0 ? "TRUE" : "FALSE");
	m_listInspection[algo].SetItemText(num, row, stNozzleYAlignRecalc);

	if (num > MAX_RESULT_ITEM - 1)
	{
		m_listInspection[algo].DeleteItem(num);
		num = m_listInspection[algo].GetItemCount();
	}

	m_listInspection[algo].Complete();
}

// hsj 2022-01-11 nozzle view align 보정값 결과창에 띄우기
void CPaneAuto::DrawCenterNozzleAlignViewer(int algo)
{
	int camCount = m_pMain->vt_job_info[algo].num_of_camera;
//	int nCamCount = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraNum(algo);
	int num = m_listInspection[algo].GetItemCount();

	//if (num > 50)  m_listInspection[algo].DeleteAllItems();

	CString stIndex = "";
	CString stModel = "";
	CString strJudge = "";
	CString stRevisionDataX = "";
	CString stRevisionDataY = "";
	CString stRevisionDataT = "";
	CString stRevisionDataZ = "";
	CString stTime = "";
	CString stDate = "";
	CString stPanelID = "";

	int row = 0;

	m_listInspection[algo].AddItem("");
	if (m_pMain->m_bSimulationStart)
	{
		SYSTEMTIME	csTime;
		::GetLocalTime(&csTime);
		stTime.Format("%02d_%02d_%02d_S", csTime.wHour, csTime.wMinute, csTime.wSecond);
		stDate.Format("%04d%02d%02d", csTime.wYear, csTime.wMonth, csTime.wDay);
	}
	else
	{
		stTime.Format("%s", m_pMain->m_strResultTime[algo]);
		stDate.Format("%s", m_pMain->m_strResultDate[algo]);
	}
	m_listInspection[algo].SetItemText(num, row, stTime);
	m_LastReusltDate[algo] = stDate;

	row++;

	//패널아이디
	stPanelID.Format("%s", m_pMain->vt_job_info[algo].get_main_object_id());
	m_listInspection[algo].SetItemText(num, row, stPanelID);

	row++;

	strJudge.Format("%s", m_bJudge[algo][m_listInspection[algo].GetItemCount()] == TRUE ? "OK" : "NG");
	m_listInspection[algo].SetItemText(num, row, strJudge);

	row++;

	stRevisionDataX.Format("%.5f", m_pMain->m_dbRevisionData[algo][AXIS_X]);
	m_listInspection[algo].SetItemText(num, row, stRevisionDataX);

	row++;

	stRevisionDataY.Format("%.5f", m_pMain->m_dbRevisionData[algo][AXIS_Y]);
	m_listInspection[algo].SetItemText(num, row, stRevisionDataY);

	row++;

	stRevisionDataZ.Format("%.5f", m_pMain->m_dbRevisionData[algo][AXIS_Z]);
	m_listInspection[algo].SetItemText(num, row, stRevisionDataZ);

	row++;

	stRevisionDataT.Format("%.5f", m_pMain->m_dbRevisionData[algo][AXIS_T]);
	m_listInspection[algo].SetItemText(num, row, stRevisionDataT);
	m_listInspection[algo].ChartColAsc(0, 1); // col, 오름차순:0 내림차순:1

	if (num > MAX_RESULT_ITEM - 1)
	{
		m_listInspection[algo].DeleteItem(num);
		num = m_listInspection[algo].GetItemCount();
	}


	m_listInspection[algo].Complete();
}

void CPaneAuto::DrawELBCenterViewer(int algo)
{
	int camCount = m_pMain->vt_job_info[algo].num_of_camera;
	//	int nCamCount = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraNum(algo);
	int num = m_listInspection[algo].GetItemCount();

	//if (num > 50)  m_listInspection[algo].DeleteAllItems();

	CString stIndex = "";
	//CString stUnit = "";
	CString strJudge = "";
	CString stRevisionDataX = "";
	CString stRevisionDataY = "";
	CString stRevisionDataT = "";
	CString stTime = "";
	CString stDate = "";
	CString stPanelID = "";

	int row = 0;

	m_listInspection[algo].AddItem("");
	//시간
	if (m_pMain->m_bSimulationStart)
	{
		SYSTEMTIME	csTime;
		::GetLocalTime(&csTime);
		stTime.Format("%02d_%02d_%02d_S", csTime.wHour, csTime.wMinute, csTime.wSecond);
		stDate.Format("%04d%02d%02d", csTime.wYear, csTime.wMonth, csTime.wDay);
	}
	else
	{
		stTime.Format("%s", m_pMain->m_strResultTime[algo]);
		stDate.Format("%s", m_pMain->m_strResultDate[algo]);
	}
	m_listInspection[algo].SetItemText(num, row, stTime);
	m_LastReusltDate[algo] = stDate;

	row++;

	//패널아이디
	stPanelID.Format("%s", m_pMain->vt_job_info[algo].get_main_object_id());
	m_listInspection[algo].SetItemText(num, row, stPanelID);

	row++;

	strJudge.Format("%s", m_bJudge[algo][m_listInspection[algo].GetItemCount()] == TRUE ? "OK" : "NG");
	m_listInspection[algo].SetItemText(num, row, strJudge);

	row++;

	/*stModel.Format("%s", m_pMain->getCurrModelName());
	m_listInspection.SetItemText(num, 1, stModel);*/

	//stUnit.Format("%s", m_pMain->m_pAlgorithmInfo.getAlgorithmName(algo).c_str());
	/*stUnit.Format("%s", m_pMain->vt_job_info[algo].job_name.c_str());
	m_listInspection[algo].SetItemText(num, 1, stUnit);*/

	stRevisionDataX.Format("%.4f", m_pMain->m_dbRevisionData[algo][AXIS_X]);
	m_listInspection[algo].SetItemText(num, row, stRevisionDataX);

	row++;

	stRevisionDataY.Format("%.4f", m_pMain->m_dbRevisionData[algo][AXIS_Y]);
	m_listInspection[algo].SetItemText(num, row, stRevisionDataY);

	row++;

	stRevisionDataT.Format("%.4f", m_pMain->m_dbRevisionData[algo][AXIS_T]);
	m_listInspection[algo].SetItemText(num, row, stRevisionDataT);

	m_listInspection[algo].ChartColAsc(0, 1); // col, 오름차순:0 내림차순:1

	if (num > MAX_RESULT_ITEM - 1)
	{
		m_listInspection[algo].DeleteItem(num);
		num = m_listInspection[algo].GetItemCount();
	}


	m_listInspection[algo].Complete();
}

void CPaneAuto::DrawPFInspectionViewer(int algo,int method)
{
	int num = m_listInspection[algo].GetItemCount();

	//if (num > 50)  m_listInspection[algo].DeleteAllItems();

	CString stTime = "";
	CString stDate = "";
	//CString stUnit = "";
	CString strJudge = "";
	CString strLX = "";
	CString strLY = "";
	CString strRX = "";
	CString strRY = "";
	CString strCRX = "";
	CString strCLX = "";
	//CString  strObject = "";
	CString stPanelID = "";

	m_listInspection[algo].AddItem("");

	int row = 0;

	//시간
	if (m_pMain->m_bSimulationStart)
	{
		SYSTEMTIME	csTime;
		::GetLocalTime(&csTime);
		stTime.Format("%02d_%02d_%02d_S", csTime.wHour, csTime.wMinute, csTime.wSecond);
		stDate.Format("%04d%02d%02d", csTime.wYear, csTime.wMonth, csTime.wDay);
	}
	else
	{
		stTime.Format("%s", m_pMain->m_strResultTime[algo]);
		stDate.Format("%s", m_pMain->m_strResultDate[algo]);
	}
	m_listInspection[algo].SetItemText(num, row, stTime);
	m_LastReusltDate[algo] = stDate;

	row++;

	//패널아이디
	stPanelID.Format("%s", m_pMain->vt_job_info[algo].get_main_object_id());
	m_listInspection[algo].SetItemText(num, row, stPanelID);

	row++;

	strJudge.Format("%s", m_bJudge[algo][m_listInspection[algo].GetItemCount()] == TRUE ? "OK" : "NG");
	m_listInspection[algo].SetItemText(num, row, strJudge);

	row++;

	int nFilmMethod= m_pMain->vt_job_info[algo].model_info.getInspSpecParaInfo().getFilmInspMethod();

	CString strMethod = "";

	switch (method)
	{
	case 0: strMethod = "Center"; break;
	case 1: strMethod = "Left"; break;
	case 2: strMethod = "Right"; break;
	case 3: strMethod = "Multi"; break;
	}

	if (nFilmMethod == 3)
	{
		/*strObject.Format("%s", strMethod);
		m_listInspection[algo].SetItemText(num, row, strObject);

		row++;*/

		strLX.Format("%.4f", m_pMain->vt_result_info[algo].pf_film_insp_data.dist_lx[1]);
		m_listInspection[algo].SetItemText(num, row, strLX);

		row++;

		strLY.Format("%.4f", m_pMain->vt_result_info[algo].pf_film_insp_data.dist_ly[1]);
		m_listInspection[algo].SetItemText(num, row, strLY);

		row++;

		strRX.Format("%.4f", m_pMain->vt_result_info[algo].pf_film_insp_data.dist_rx[2]);
		m_listInspection[algo].SetItemText(num, row, strRX);

		row++;

		strRY.Format("%.4f", m_pMain->vt_result_info[algo].pf_film_insp_data.dist_ry[2]);
		m_listInspection[algo].SetItemText(num, row, strRY);
	}
	else if (nFilmMethod == 4)
	{
		/*strObject.Format("%s", strMethod);
		m_listInspection[algo].SetItemText(num, row, strObject);

		row++;*/

		strLX.Format("%.4f", m_pMain->vt_result_info[algo].pf_film_insp_data.dist_lx[1]);
		m_listInspection[algo].SetItemText(num, row, strLX);

		row++;

		strLY.Format("%.4f", m_pMain->vt_result_info[algo].pf_film_insp_data.dist_ly[1]);
		m_listInspection[algo].SetItemText(num, row, strLY);

		row++;

		strRX.Format("%.4f", m_pMain->vt_result_info[algo].pf_film_insp_data.dist_rx[2]);
		m_listInspection[algo].SetItemText(num, row, strRX);

		row++;

		strRY.Format("%.4f", m_pMain->vt_result_info[algo].pf_film_insp_data.dist_ry[2]);
		m_listInspection[algo].SetItemText(num, row, strRY);

		row++;

		strCLX.Format("%.4f", m_pMain->vt_result_info[algo].pf_film_insp_data.dist_lx[0]);
		m_listInspection[algo].SetItemText(num, row, strCLX);

		row++;

		strCRX.Format("%.4f", m_pMain->vt_result_info[algo].pf_film_insp_data.dist_rx[0]);
		m_listInspection[algo].SetItemText(num, row, strCRX);
	}
	else
	{
		/*strObject.Format("%s", strMethod);
		m_listInspection[algo].SetItemText(num, row, strObject);

		row++;*/

		strLX.Format("%.4f", m_pMain->vt_result_info[algo].pf_film_insp_data.dist_lx[method]);
		m_listInspection[algo].SetItemText(num, row, strLX);

		row++;

		strLY.Format("%.4f", m_pMain->vt_result_info[algo].pf_film_insp_data.dist_ly[method]);
		m_listInspection[algo].SetItemText(num, row, strLY);

		row++;

		strRX.Format("%.4f", m_pMain->vt_result_info[algo].pf_film_insp_data.dist_rx[method]);
		m_listInspection[algo].SetItemText(num, row, strRX);

		row++;

		strRY.Format("%.4f", m_pMain->vt_result_info[algo].pf_film_insp_data.dist_ry[method]);
		m_listInspection[algo].SetItemText(num, row, strRY);
	}
	
	m_listInspection[algo].ChartColAsc(0, 1); // col, 오름차순:0 내림차순:1

	if (num > MAX_RESULT_ITEM - 1)
	{
		m_listInspection[algo].DeleteItem(num);
		num = m_listInspection[algo].GetItemCount();
	}

	m_listInspection[algo].Complete();
}

//hsj 2022-01-09 ReelAlign 결과창 추가
void CPaneAuto::DrawPFReelAlignViewer(int algo,int nObject)
{
	int num = m_listInspection[algo].GetItemCount();

	//if (num > 50)  m_listInspection[algo].DeleteAllItems();

	CString stTime = "";
	CString stDate = "";
	CString strJudge = "";
	CString stObject = "";
	CString stRevisionDataX = "";
	CString stRevisionDataY = "";
	CString stRevisionDataT = "";
	CString stPanelID = "";

	int row = 0;

	m_listInspection[algo].AddItem("");

	//시간
	if (m_pMain->m_bSimulationStart)
	{
		SYSTEMTIME	csTime;
		::GetLocalTime(&csTime);
		stTime.Format("%02d_%02d_%02d_S", csTime.wHour, csTime.wMinute, csTime.wSecond);
		stDate.Format("%04d%02d%02d", csTime.wYear, csTime.wMonth, csTime.wDay);
	}
	else
	{
		stTime.Format("%s", m_pMain->m_strResultTime[algo]);
		stDate.Format("%s", m_pMain->m_strResultDate[algo]);
	}
	m_listInspection[algo].SetItemText(num, row, stTime);
	m_LastReusltDate[algo] = stDate;

	row++;

	//패널아이디
	stPanelID.Format("%s", m_pMain->vt_job_info[algo].get_main_object_id());
	m_listInspection[algo].SetItemText(num, row, stPanelID);

	row++;

	strJudge.Format("%s", m_bJudge[algo][m_listInspection[algo].GetItemCount()] == TRUE ? "OK" : "NG");
	m_listInspection[algo].SetItemText(num, row, strJudge);

	row++;

	stObject.Format("Object %d", nObject+1);
	m_listInspection[algo].SetItemText(num, row, stObject);

	row++;

	stRevisionDataX.Format("%.4f", m_pMain->m_dbRevisionData_obejct[algo][AXIS_X][nObject]);
	m_listInspection[algo].SetItemText(num, row, stRevisionDataX);

	row++;

	stRevisionDataY.Format("%.4f", m_pMain->m_dbRevisionData_obejct[algo][AXIS_Y][nObject]);
	m_listInspection[algo].SetItemText(num, row, stRevisionDataY);

	row++;

	stRevisionDataT.Format("%.4f", m_pMain->m_dbRevisionData_obejct[algo][AXIS_T][nObject]);
	m_listInspection[algo].SetItemText(num, row, stRevisionDataT);

	m_listInspection[algo].ChartColAsc(0, 1); // col, 오름차순:0 내림차순:1

	if (num > MAX_RESULT_ITEM - 1)
	{
		m_listInspection[algo].DeleteItem(num);
		num = m_listInspection[algo].GetItemCount();
	}


	m_listInspection[algo].Complete();
}

void CPaneAuto::DrawSpecChart()
{
	int nMetho = m_pMain->vt_job_info[0].algo_method;

	if (nMetho == CLIENT_TYPE_ELB_CENTER_ALIGN)
	{
		//update_grid_inspection_spec();
		update_grid_inspection_spec_viewer();
	}
	else if (nMetho == CLIENT_TYPE_SCAN_INSP)
	{
		Update_Grid_Scan_Insp_Spec();
	}
	else
	{
		update_grid_spec_viewer();		
	}
}
void CPaneAuto::DrawSpecChart_Scan_Insp(int nJob, int nPos)
{
	if (nPos > 0 && nPos < 4 )
	{
		Update_Grid_Scan_Insp_Result_Viewer(nJob, nPos);
	}
	else
	{
		Update_Grid_Scan_Insp_Spec();
	}
}
void CPaneAuto::update_grid_inspection_spec_viewer()
{
	// JCY 2023-01-17 데이터 시트 함수 홀/라인 분기 
	move_grid_inspection_spec_viewer();

	CGridCtrl* grid = &m_grid_spec_viewer;	
	int	datasheet = 0, nJudgePos = 0, row = 2;
	int method = m_pMain->vt_job_info[0].model_info.getAlignInfo().getMarkFindMethod();
	double itemData = 0, specData = 0, distance = 0;
	double _specDataV[10] = { 0, };
	double yres = m_pMain->GetMachine(0).getCameraResolutionY(0, 0);	

	BOOL bJudge = TRUE;
	BOOL bLineDispMode = m_pMain->vt_job_info[0].model_info.getAlignInfo().getLineOverflowDispMode(); 
	CString cellId = strlen(m_pMain->vt_job_info[0].get_main_object_id()) < 13 ? "None" : m_pMain->vt_job_info[0].get_main_object_id();
	CString okNg = "NG";

	// IN MIN
	_specDataV[2] = m_pMain->vt_job_info[0].model_info.getAlignInfo().getDistanceInspSubSpec(0);
	// IN MAX
	_specDataV[3] = m_pMain->vt_job_info[0].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(0);
	// OUT MAX
	_specDataV[4] = m_pMain->vt_job_info[0].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(1);
	// LENGTH
	if (method == METHOD_CIRCLE)	_specDataV[5] = m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getMinCountCheckInspTotalCount();
	else if (method == METHOD_LINE)	_specDataV[5] = m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getLineOverflowMinSpec();	//SJB 2022-11-11 Overflow Min Grid 수정
	else							_specDataV[5] = m_pMain->vt_job_info[0].model_info.getAlignInfo().getInspRangeLength();
	//B MIN
	if (method == METHOD_LINE)		_specDataV[6] = m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getLineLengthSpec();	//SJB 2022-11-11 Overflow Min Grid 수정
	else							_specDataV[6] = m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getWetoutBMinSpec();
	//CG-start
	if (method == METHOD_LINE)		_specDataV[7] = m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getCgToStartSpec();
	else							_specDataV[7] = m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getWetoutBMaxSpec();
	//CG-end 
	_specDataV[8] = m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getCgToEndSpec();

	bool   bWetoutRJudgeModeEnable = m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getWetoutRJudgeModeEnable();
	bool   bWetoutBJudgeModeEnable = m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getWetoutBJudgeModeEnable();
	bool   bWetoutMinMaxJudgeModeEnable = m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getWetoutMinMaxJudgeModeEnable();
	bool   bMetalTraceFirst = m_pMain->vt_job_info[0].model_info.getAlignInfo().getUseMetalTraceFirst();
	const double Wetout_ASPC_Spec = m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getWetoutASPCSpec();
	double dScratchJudgeArea = m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getCGJudgeArea();

	if (method == METHOD_CIRCLE) nJudgePos = grid->GetColumnCount() - 1;
	else if (method == METHOD_LINE) nJudgePos = grid->GetColumnCount() - 1;
	else nJudgePos = grid->GetColumnCount() - 1;	

	if (method == METHOD_CIRCLE)  distance = m_pMain->m_ELB_DiffInspResult[datasheet].m_nLackOfInkAngleCount_Judge;
	else distance = (m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.y - m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.x) * yres;
		

#pragma region 검사 데이터 업데이트
	for (int col = 0; col < grid->GetColumnCount(); col++)
	{		
		GV_ITEM Item;
		Item.mask = GVIF_TEXT | GVIF_FORMAT;
		Item.row = row;		Item.col = col;

		specData = _specDataV[col];

		if (col < nJudgePos) // 검사 데이터 영역
		{
			switch (col)
			{
				case 0:	// Number
				{
					Item.nFormat = DT_CENTER;
					Item.strText.Format(_T("S%d"), m_pMain->m_nSpecRowCount - 1);
					grid->SetItemBkColour(row, col, COLOR_YELLOW);
				}
				break;
				case 1: // Cell ID
				{
					Item.nFormat = DT_CENTER;
					Item.strText.Format(_T("%s"), cellId);
					if (m_pMain->m_ELB_DiffInspResult[0].m_bResultSummary == FALSE || m_pMain->m_ELB_DiffInspResult[1].m_bResultSummary == FALSE ||
						m_pMain->m_ELB_DiffInspResult[0].bType == FALSE) grid->SetItemBkColour(row, col, COLOR_RED);
					else grid->SetItemBkColour(row, col, COLOR_YELLOW);
				}
				break;
				case 2: // Circle : Min(in), Line : Wetout Min
				{
					Item.strText.Format(_T("%.4f"), m_pMain->m_ELB_DiffInspResult[datasheet].m_vELB_ReverseRotateCenter.x);
					itemData = atof(Item.strText);

					if (method == METHOD_CIRCLE) // Circle : Min(in)
					{
						if (itemData < specData)
						{
							if (bWetoutMinMaxJudgeModeEnable)
							{
								grid->SetItemBkColour(row, col, COLOR_RED);
								bJudge = FALSE;
							}
							else grid->SetItemBkColour(row, col, COLOR_ORANGE);
						}
						else grid->SetItemBkColour(row, col, COLOR_GREEN);
					}
					else if (method == METHOD_LINE) // Line : Wetout Min
					{
						if (itemData < specData)
						{
							if (bWetoutMinMaxJudgeModeEnable)
							{
								grid->SetItemBkColour(row, col, COLOR_RED);
								bJudge = FALSE;
								okNg = "Wetout NG";
							}
							else grid->SetItemBkColour(row, col, COLOR_ORANGE);
						}
						else grid->SetItemBkColour(row, col, COLOR_GREEN);
					}
				}
				break;
				case 3:	// Circle : Max(in), Line : Wetout Max
				{
					Item.strText.Format(_T("%.4f"), m_pMain->m_ELB_DiffInspResult[datasheet].m_vELB_ReverseRotateCenter.y);
					itemData = atof(Item.strText);

					if (method == METHOD_CIRCLE) // Circle : Max(in)
					{
						if (itemData > specData)
						{
							if (bWetoutMinMaxJudgeModeEnable)
							{
								grid->SetItemBkColour(row, col, COLOR_RED);
								bJudge = FALSE;
							}
							else
								grid->SetItemBkColour(row, col, COLOR_ORANGE);
						}
						else grid->SetItemBkColour(row, col, COLOR_GREEN);
					}
					else if (method == METHOD_LINE) // Line : Wetout Max
					{
						if (itemData > specData)
						{
							if (bWetoutMinMaxJudgeModeEnable)
							{
								grid->SetItemBkColour(row, col, COLOR_RED);
								bJudge = FALSE;
								okNg = "Wetout NG";
							}
							else grid->SetItemBkColour(row, col, COLOR_ORANGE);
						}
						else grid->SetItemBkColour(row, col, COLOR_GREEN);
					}
				}
				break;
				case 4: // Circle, Line : Overflow Max
				{
					if (bLineDispMode) Item.strText.Format(_T("%.4f"), m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultAngle[9]);
					else Item.strText.Format(_T("%.4f"), m_pMain->m_ELB_DiffInspResult[datasheet].m_vELB_RotateCenter.y);
					itemData = atof(Item.strText);

					if (method == METHOD_CIRCLE) // Overflow Max
					{
						if (itemData > specData)
						{
							// YCS 2022-12-01 CINK2일 때는 무조건 Red Color이면서 WetOUt NG 띄움. (김진용 프로 요청: WetOut Spec out인데 OK로 나가면 유출이다.)
							if (bWetoutMinMaxJudgeModeEnable && bMetalTraceFirst)
							{
								grid->SetItemBkColour(row, col, COLOR_RED);
								bJudge = FALSE;
							}
							else grid->SetItemBkColour(row, col, COLOR_ORANGE);
						}
						// YCS 2022-12-01 CIRCLE일 때 검사결과 그리드 색깔 조건에 ASPC 스펙 추가
						else if (itemData <= specData && itemData > Wetout_ASPC_Spec)
						{
							grid->SetItemBkColour(row, col, COLOR_ORANGE);
						}
						else grid->SetItemBkColour(row, col, COLOR_GREEN);
					}
					else if (method == METHOD_LINE) // Overflow Max
					{
						if (itemData > specData)
						{
							if (bWetoutMinMaxJudgeModeEnable)
							{
								grid->SetItemBkColour(row, col, COLOR_RED);
								bJudge = FALSE;
							}
							else grid->SetItemBkColour(row, col, COLOR_ORANGE);
						}
						else grid->SetItemBkColour(row, col, COLOR_GREEN);
					}
				}
				break;
				case 5: // Circle : LACK, Line : Overflow Min
				{
					if (method == METHOD_LINE)	Item.strText.Format(_T("%.4f"), m_pMain->m_ELB_DiffInspResult[datasheet].m_vELB_RotateCenter.x);
					else						Item.strText.Format(_T("%.2f"), distance);
					itemData = atof(Item.strText);

					if (method == METHOD_CIRCLE) // LACK
					{
						if (itemData > specData) grid->SetItemBkColour(row, col, COLOR_RED);
						else grid->SetItemBkColour(row, col, COLOR_GREEN);
					}
					else if (method == METHOD_LINE) // Overflow Min
					{
						if (itemData < specData)
						{
							if (bWetoutMinMaxJudgeModeEnable)
							{
								grid->SetItemBkColour(row, col, COLOR_RED);
								bJudge = FALSE;
							}
							else grid->SetItemBkColour(row, col, COLOR_ORANGE);
						}
						else grid->SetItemBkColour(row, col, COLOR_GREEN);
					}
				}
				break;
				case 6: // Circle : Min(B), Line : Length
				{
					if (method == METHOD_LINE)	Item.strText.Format(_T("%.2f"), distance);
					else						Item.strText.Format(_T("%.4f"), m_pMain->m_ELB_DiffInspResult[datasheet].m_BminResult);
					itemData = atof(Item.strText);

					if (method == METHOD_CIRCLE) // Min(B)
					{
						if (itemData < specData)
						{
							if (bWetoutBJudgeModeEnable)
							{
								grid->SetItemBkColour(row, col, COLOR_RED);
								bJudge = FALSE;
							}
							else grid->SetItemBkColour(row, col, COLOR_ORANGE);
						}
						else grid->SetItemBkColour(row, col, COLOR_GREEN);
					}
					else if (method == METHOD_LINE) // Length
					{
						if (itemData > specData)
						{
							grid->SetItemBkColour(row, col, COLOR_RED);
							bJudge = FALSE;
							okNg = "Line Length NG";
						}
						else grid->SetItemBkColour(row, col, COLOR_GREEN);
					}
				}
				break;
				case 7: // Circle : Max(B), Line : CG Start
				{
					if (method == METHOD_LINE) Item.strText.Format(_T("%.3f"), m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultAngle[2]);
					else                       Item.strText.Format(_T("%.4f"), m_pMain->m_ELB_DiffInspResult[datasheet].m_BmaxResult);
					itemData = atof(Item.strText);

					if (method == METHOD_CIRCLE) // Max(B)
					{
						if (itemData > specData)
						{
							if (bWetoutBJudgeModeEnable)
							{
								grid->SetItemBkColour(row, col, COLOR_RED);
								bJudge = FALSE;
							}
							else grid->SetItemBkColour(row, col, COLOR_ORANGE);
						}
						else grid->SetItemBkColour(row, col, COLOR_GREEN);
					}
					else if (method == METHOD_LINE) // CG Start
					{
						if (itemData < (specData - dScratchJudgeArea) || (specData + dScratchJudgeArea) < itemData)
						{
							grid->SetItemBkColour(row, col, COLOR_RED);
							bJudge = FALSE;
							okNg = "CG NG";
						}
						else grid->SetItemBkColour(row, col, COLOR_GREEN);					
					}
				}
				break;
				case 8: // Circle : Judge, Line : CG End
				{
					if (method == METHOD_LINE) Item.strText.Format(_T("%.3f"), m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultAngle[2] + distance);
					itemData = atof(Item.strText);

					if (method == METHOD_CIRCLE) // Judge
					{
						// Judge 영역 Cicle, Line 통합으로 함(case9)
					}
					else if (method == METHOD_LINE) // CG End
					{
						if (itemData < (specData - dScratchJudgeArea) || (specData + dScratchJudgeArea) < itemData)
						{
							grid->SetItemBkColour(row, col, COLOR_RED);
							okNg = "CG NG";
							bJudge = FALSE;
						}
						else grid->SetItemBkColour(row, col, COLOR_GREEN);
					}
				}
				break;
				case 9: // Circle : X, Line : Judge
				{
					// Judge 영역 Cicle, Line 통합으로 함(case9)
				}
				break;
			}
		}
		else // Cicle, Line : Judge
		{
			if (method == METHOD_CIRCLE)
			{
				if (m_pMain->m_ELB_DiffInspResult[0].m_bResultSummary == FALSE)
				{
					okNg = "Overflow";
					bJudge = FALSE;
				}
				else if (m_pMain->m_ELB_DiffInspResult[1].m_bResultSummary == FALSE)
				{
					okNg = "Underflow";
					bJudge = FALSE;
				}
				else if (m_pMain->m_ELB_DiffInspResult[0].bType == FALSE)
				{
					okNg = "Not_Dispensed";
					bJudge = FALSE;
				}
				else if (m_pMain->m_ELB_DiffInspResult[0].bRJudge == FALSE)
				{
					okNg = "R_Spec NG";
					bJudge = FALSE;
				}
				else if (m_pMain->m_ELB_DiffInspResult[0].bBJudge == FALSE)
				{
					okNg = "B_Spec NG";
					bJudge = FALSE;
				}
			}
			else if (method == METHOD_LINE)
			{
				// 추후 수정 
			}			

			if (bJudge)
			{
				grid->SetItemBkColour(row, col, COLOR_GREEN);
				Item.strText.Format(_T("OK"));
			}
			else
			{
				m_InspComment.Format(okNg);
				grid->SetItemBkColour(row, col, COLOR_RED);
				Item.strText.Format(okNg);
			}
		}

		grid->SetItem(&Item);
	}
	grid->RedrawRow(row);
#pragma endregion

	for (int row = grid->GetRowCount() - 1; row >= 3; row--)
	{
		for (int col = 0; col < grid->GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row;		Item.col = col;
			Item.strText.Format(m_pAutoShiftResult[col][row]);

			grid->SetItem(&Item);
		}
		grid->RedrawRow(row);
	}
	grid->Refresh();
}

void CPaneAuto::update_grid_dustResult_spec()
{
	CGridCtrl* grid = &m_grid_spec_viewer;
	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int row = 2, col = 8;
	// YCS 2023-01-16 col 분기
	int method = m_pMain->vt_job_info[0].model_info.getAlignInfo().getMarkFindMethod();
	if (method == METHOD_LINE)	col = 9;
	m_InspComment.Format("Droplet_NG");

	GV_ITEM Item;
	Item.mask = GVIF_TEXT | GVIF_FORMAT;
	Item.nFormat = dwTextStyle;
	Item.row = row;		Item.col = col;
	Item.strText.Format(m_InspComment);

	grid->SetItemBkColour(row, col, RGB(255,0,0));
	grid->SetItem(&Item);
	grid->RedrawRow(row);
}

void CPaneAuto::move_grid_inspection_spec_viewer()
{
	CGridCtrl* grid = &m_grid_spec_viewer;
	CString itemData;
	COLORREF color;
	int row = 0;
	int col = 0;
	int _nCount = grid->GetRowCount() - 2;

	for (row = _nCount; row >=2; row--)
	{
		for (col = 0; col < grid->GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row+1;		Item.col = col;
			itemData = grid->GetItemText(row, col);
			Item.strText.Format(itemData);

			color = grid->GetItemBkColour(row, col);
			grid->SetItemBkColour(row+1, col, color);			
			grid->SetItem(&Item);
			m_pAutoShiftResult[col][row+1] = Item.strText;
		}
		grid->RedrawRow(row);
		grid->RedrawRow(row+1);
	}	

	grid->Refresh();
}

void CPaneAuto::update_grid_inspection_spec()
{
	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	CGridCtrl* grid = &m_grid_spec_viewer;

	double _spec = m_pMain->vt_job_info[0].model_info.getAlignInfo().getInspRangeLength();	
	int method = m_pMain->vt_job_info[0].model_info.getAlignInfo().getMarkFindMethod();
	
	//HTK 2022-06-29 고객요청으로 Circle시 Lack of CInk Total Count Display로 변경
	if		(method == METHOD_CIRCLE)	_spec = m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getMinCountCheckInspTotalCount();
	else if (method == METHOD_LINE)		_spec = m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getLineLengthSpec();

	BOOL bLineDispMode = m_pMain->vt_job_info[0].model_info.getAlignInfo().getLineOverflowDispMode();  // false : 

	GV_ITEM Item;
	Item.mask = GVIF_TEXT | GVIF_FORMAT;
	Item.nFormat = dwTextStyle;
	Item.row = 1;

	for (int col = 1; col < grid->GetColumnCount(); col++)
	{
		Item.col = col;

		double	B_Min = m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getWetoutBMinSpec();
		double	B_Max = m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getWetoutBMaxSpec();
		double	R_Min = m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getWetoutRMinSpec();
		double	R_Max = m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getWetoutRMaxSpec();
		double  CG_Start = m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getCgToStartSpec();
		double  CG_End = m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getCgToEndSpec();

		switch (col)
		{
		case 1:		Item.strText.Format(_T("Insp"));				                                                                            break; // 셀아이디로 변경
		case 2:		Item.strText.Format(_T("%.4f"), m_pMain->vt_job_info[0].model_info.getAlignInfo().getDistanceInspSubSpec(0));               break;
		case 3:		Item.strText.Format(_T("%.4f"), m_pMain->vt_job_info[0].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(0));		break;
		case 4:
		{
			// KBJ 2022-11-30 결과창 스펙파라미터 적용부분 수정
			//if(bLineDispMode) Item.strText.Format(_T("%.4f"), m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getMetalOverflowMargin());
			//else 
			Item.strText.Format(_T("%.4f"), m_pMain->vt_job_info[0].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(1));
		}
		break;
		//KJH 2022-05-25 Length -> BMinSpec으로 UI 변경
		//Tkyuha 2022-03-10 도포 반대편 1/2 평균값 계산 추가
		//KJH 2022-06-06 R Display제거 / Length는 Line에서 사용함
		//SJB 2022-11-11 Overflow Min Grid 수정
		case 5:
		{
			if (method == METHOD_LINE)	Item.strText.Format(_T("%.4f"), m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getLineOverflowMinSpec());
			else						Item.strText.Format(_T("%.2f"), _spec);
		}
		break;
		case 6:
		{
			if (method == METHOD_LINE)	Item.strText.Format(_T("%.2f"), _spec);
			else						Item.strText.Format(_T("%.4f"), B_Min);
		}
		break;
		case 7:
		{
			if (method == METHOD_LINE)	Item.strText.Format(_T("%.3f"), CG_Start);
			else						Item.strText.Format(_T("%.4f"), B_Max);
			
		}
		break;
		//case 5:		Item.strText.Format(_T("%.4f"), B_Min);																						break;
		//case 6:		Item.strText.Format(_T("%.4f"), R_Min);																						break;
		//case 7:		Item.strText.Format(_T("%.4f"), R_Max);																						break;
		case 8:
		{
			if (method == METHOD_LINE)	Item.strText.Format(_T("%.3f"), CG_End);
			else						Item.strText.Format(_T("Comment"));	
		}
		break;
		case 9:	    Item.strText.Format(_T("Comment"));																							break;
		}
		grid->SetItemBkColour(1, col, COLOR_YELLOW);

		grid->SetItem(&Item);
	}
	grid->RedrawRow(1);
	grid->Refresh();
}
//HTK 2022-03-30 Tact Time Display 추가
void CPaneAuto::update_grid_processing_time(int cmd,double msgTime,int pos,bool judge)
{
	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	CGridCtrl* grid = &m_grid_proc_time_viewer;
	
	if (cmd == 0)
	{
		for (int row = 1; row < grid->GetRowCount(); row++)
		{
			grid->SetItemBkColour(row, 1, COLOR_GRAY);
			grid->RedrawRow(row);
		}
	}
	else
	{
		DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
		GV_ITEM Item;
		Item.mask = GVIF_TEXT | GVIF_FORMAT;
		Item.nFormat = dwTextStyle;
		Item.row = pos;		Item.col = 2;

		Item.strText.Format(_T("%.4f"), msgTime);

		if(judge)	grid->SetItemBkColour(pos, 1, COLOR_GREEN);
		else		grid->SetItemBkColour(pos, 1, COLOR_RED);

		grid->SetItem(&Item);
		grid->RedrawRow(pos);
	}
}

void CPaneAuto::update_grid_spec_viewer()
{
	//KJH 2021-07-26 굳이 쪼갤 필요가 있을까 모르겠지만 기존 구조로 스펙 그리드 갱신 추가
	
	int nMetho = m_pMain->vt_job_info[0].algo_method;
	if (nMetho == CLIENT_TYPE_FILM_INSP) // hsj 2022-01-10 align spec, inspection spec 창 구별
	{
		draw_grid_spec_viewer(m_grid_spec_viewer, 6, m_nAlignCount+1);
		draw_grid_insp_spec_viewer(m_grid_insp_spec_viewer, 9, m_nInspCount + 1);
	}
	else
	{
		int num_of_job = int(m_pMain->vt_job_info.size());
		draw_grid_spec_viewer(m_grid_spec_viewer, 6, num_of_job + 1);
	}
	
}
void CPaneAuto::init_spec_view_setting(CGridCtrl &grid, int col, int row)
{
	BOOL bVirtualMode = FALSE;
	CRect rect;

	grid.SetEditable(FALSE);
	//grid.SetEditable(TRUE);
	grid.SetVirtualMode(bVirtualMode);
	grid.SetListMode(TRUE);
	grid.EnableDragAndDrop(FALSE);
	grid.EnableSelection(FALSE);
	grid.SetFixedRowSelection(TRUE);
	grid.SetFixedColumnSelection(TRUE);
	grid.SetFrameFocusCell(FALSE);
	grid.SetTrackFocusCell(FALSE);
	grid.SetRowResize(TRUE);
	grid.SetColumnResize(FALSE);

	grid.SetRowCount(row);
	grid.SetColumnCount(col);	

	grid.SetRowHeight(0, 25);
	grid.SetBkColor(RGB(197, 197, 187));
	grid.SetTextBkColor(RGB(235, 235, 235));
	grid.SetFixedRowCount(1);

	grid.GetWindowRect(&rect);
	int w = rect.Width() / col;

	for (int i = 0; i < grid.GetColumnCount(); i++)
	{

		grid.SetItemBkColour(0, i, RGB(146, 146, 141));
		grid.SetItemFgColour(0, i, RGB(255, 255, 255));
		grid.SetColumnWidth(i, w);
	}

	//grid.AutoSizeColumns();
	grid.ExpandColumnsToFit();
}
void CPaneAuto::init_ref_data_view_setting(CGridCtrl &grid, int col, int row)
{
	BOOL bVirtualMode = FALSE;
	CRect rect;

	grid.SetEditable(TRUE);
	grid.SetVirtualMode(bVirtualMode);
	grid.SetListMode(TRUE);
	grid.EnableDragAndDrop(FALSE);
	grid.EnableSelection(FALSE);
	grid.SetFixedRowSelection(TRUE);
	grid.SetFixedColumnSelection(TRUE);
	grid.SetFrameFocusCell(FALSE);
	grid.SetTrackFocusCell(FALSE);
	grid.SetRowResize(TRUE);
	grid.SetColumnResize(FALSE);

	grid.SetRowCount(row);
	grid.SetColumnCount(col);	

	grid.SetRowHeight(0, 25);
	grid.SetBkColor(RGB(197, 197, 187));
	grid.SetTextBkColor(RGB(235, 235, 235));
	grid.SetFixedRowCount(1);

	grid.GetWindowRect(&rect);
	int w = rect.Width() / 9;

	for (int i = 0; i < grid.GetColumnCount(); i++)
	{
		grid.SetItemBkColour(0, i, RGB(146, 146, 141));
		grid.SetItemFgColour(0, i, RGB(255, 255, 255));
		grid.SetColumnWidth(i, w);
	}

	//grid.AutoSizeColumns();
	grid.ExpandColumnsToFit();
}
void CPaneAuto::draw_grid_spec_viewer(CGridCtrl &grid, int col, int row)
{
	//example)
	//---------------------------------------------------------------------|
	//      | Limit X | Limit Y | Limit T | L Check Length Max |    +_     |
	//------|---------|---------|---------|--------------------|-----------|
	// Spec |         |         |         |                    |           |
	//------|---------|---------|---------|--------------------|-----------|
	// S1   |         |         |         |                    |           |
	// -----|---------|---------|---------|--------------------|-----------|
	// S2   |         |         |         |                    |           |
	// -----|---------|---------|---------|--------------------|-----------|

	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nRowCount = 1;
	//int nAlgocnt = m_pMain->m_pAlgorithmInfo.getAlgorithmCount();
	int nAlgocnt = int(m_pMain->vt_job_info.size());
	GV_ITEM Item;
	Item.mask = GVIF_TEXT | GVIF_FORMAT;
	Item.nFormat = dwTextStyle;

	for (int row = 0; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{			
			Item.row = row;
			Item.col = col;

			// Fixed Cell Title

			if (row == 0 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Unit"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 1)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Limit X"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 2)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Limit Y"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 3)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Limit T"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 4)
			{
				//KJH 2021-07-26 Why L Check에 카메라 간격이 있는것인가????
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("L Check Length X"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 5)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("±"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}

			else if (row != 0 && col == 0)
			{
				Item.nFormat = dwTextStyle;
				//Item.strText.Format(_T("%s"), m_pMain->m_pAlgorithmInfo.getAlgorithmName(row - 1).c_str());
				if(m_bInsp) Item.strText.Format(_T("%s"), m_pMain->vt_job_info[m_nAlignAlgo[row-1]].job_name.c_str());
				else Item.strText.Format(_T("%s"), m_pMain->vt_job_info[row - 1].job_name.c_str());

				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}

			else if (row != 0 && col == 1)
			{
				Item.nFormat = dwTextStyle;
			//	Item.strText.Format(_T("%.4f"), m_pMain->getModel().getMachineInfo().getRevisionLimit(nAlgo[row - 1][0], AXIS_X));
		
				//hsj 2022-01-11 inspection이 있을때 없을때,
				if (m_bInsp) Item.strText.Format(_T("%.4f"), m_pMain->vt_job_info[m_nAlignAlgo[row - 1]].model_info.getMachineInfo().getRevisionLimit(AXIS_X));
				else Item.strText.Format(_T("%.4f"), m_pMain->vt_job_info[row - 1].model_info.getMachineInfo().getRevisionLimit(AXIS_X));
				
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 2)
			{
				Item.nFormat = dwTextStyle;
				//Item.strText.Format(_T("%.4f"), m_pMain->getModel().getMachineInfo().getRevisionLimit(nAlgo[row - 1][0], AXIS_Y));
				//hsj 2022-01-11 inspection이 있을때 없을때,
				if (m_bInsp) Item.strText.Format(_T("%.4f"), m_pMain->vt_job_info[m_nAlignAlgo[row - 1]].model_info.getMachineInfo().getRevisionLimit(AXIS_Y));
				else Item.strText.Format(_T("%.4f"), m_pMain->vt_job_info[row - 1].model_info.getMachineInfo().getRevisionLimit(AXIS_Y));
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 3)
			{
				Item.nFormat = dwTextStyle;
				//Item.strText.Format(_T("%.4f"), m_pMain->getModel().getMachineInfo().getRevisionLimit(nAlgo[row - 1][0], AXIS_T));
				//hsj 2022-01-11 inspection이 있을때 없을때,
				if (m_bInsp) Item.strText.Format(_T("%.4f"), m_pMain->vt_job_info[m_nAlignAlgo[row - 1]].model_info.getMachineInfo().getRevisionLimit(AXIS_T));
				else Item.strText.Format(_T("%.4f"), m_pMain->vt_job_info[row - 1].model_info.getMachineInfo().getRevisionLimit(AXIS_T));
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 4)
			{
				//KJH 2021-07-26 Why L Check에 카메라 간격이 있는것인가????
				Item.nFormat = dwTextStyle;
				//Item.strText.Format(_T("%.3f"), alignInfo.getLCheckScale(nAlgo[row - 1][0]));
				//hsj 2022-01-11 inspection이 있을때 없을때,
				if (m_bInsp) Item.strText.Format(_T("%.3f"), m_pMain->vt_job_info[m_nAlignAlgo[row - 1]].model_info.getAlignInfo().getLCheckSpecX());
				else Item.strText.Format(_T("%.3f"), m_pMain->vt_job_info[row - 1].model_info.getAlignInfo().getLCheckSpecX());
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 5)
			{
				Item.nFormat = dwTextStyle;
				//Item.strText.Format(_T("%.3f"), alignInfo.getLCheckLimit(nAlgo[row - 1][0]));
				//hsj 2022-01-11 inspection이 있을때 없을때,
				if (m_bInsp) Item.strText.Format(_T("%.3f"), m_pMain->vt_job_info[m_nAlignAlgo[row - 1]].model_info.getAlignInfo().getLCheckTor());
				else Item.strText.Format(_T("%.3f"), m_pMain->vt_job_info[row - 1].model_info.getAlignInfo().getLCheckTor());
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else
			{
				Item.nFormat = dwTextStyle;
				//Item.strText.Format("%s", m_strMarcoName[row - 1]);
			}

			if (row > 0)
			{
				if (nRowCount <= nAlgocnt + 1)
					grid.SetItemBkColour(row, col, RGB(235, 235, 235));
				else
					grid.SetItemBkColour(row, col, RGB(255, 255, 255));

				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state);
			}

			grid.SetItem(&Item);

		}

		if (row > 0)
			nRowCount++;

		if (nRowCount > 10)
			nRowCount = 1;

		grid.RedrawRow(row);
	}

	grid.AutoSizeColumns();
	grid.ExpandColumnsToFit();
}

// hsj 2022-01-10 align spec, inspection spec 창 구별
void CPaneAuto::draw_grid_insp_spec_viewer(CGridCtrl& grid, int col, int row)
{
	//example)
	//--------------------------------------------------------------|
	//      | LX | LX Tor | LY | LY Tor | RX | RX Tor | RY | RY Tor |
	//------|----|--------|----|--------|----|--------|----|--------|
	// Spec |    |        |    |        |    |        |    |        |
	//------|----|--------|----|--------|----|--------|----|--------|
	// 

	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nRowCount = 1;
	GV_ITEM Item;
	Item.mask = GVIF_TEXT | GVIF_FORMAT;
	Item.nFormat = dwTextStyle;

	for (int row = 0; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{			
			Item.row = row;
			Item.col = col;

			// Fixed Cell Title

			if (row == 0 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("JOB"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 1)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("LX"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 2)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("LX Tor"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 3)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Ly"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 4)
			{
				//KJH 2021-07-26 Why L Check에 카메라 간격이 있는것인가????
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("LY Tor"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 5)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("RX"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 6)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("RX Tor"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 7)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("RY"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 8)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("RY Tor"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row != 0 && col == 0)
			{

				Item.nFormat = dwTextStyle;
				//Item.strText.Format(_T("%s"), m_pMain->m_pAlgorithmInfo.getAlgorithmName(row - 1).c_str());
				Item.strText.Format(_T("%s"), m_pMain->vt_job_info[m_nInspAlgo[row - 1]].job_name.c_str());

				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				//	Item.strText.Format(_T("%.4f"), m_pMain->getModel().getMachineInfo().getRevisionLimit(nAlgo[row - 1][0], AXIS_X));
				
				Item.strText.Format(_T("%.3f"), m_pMain->vt_job_info[m_nInspAlgo[row - 1]].model_info.getAlignInfo().getDistanceInspSubSpec(0));

				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 2)
			{
				Item.nFormat = dwTextStyle;
				//Item.strText.Format(_T("%.4f"), m_pMain->getModel().getMachineInfo().getRevisionLimit(nAlgo[row - 1][0], AXIS_Y));
				Item.strText.Format(_T("%.3f"), m_pMain->vt_job_info[m_nInspAlgo[row - 1]].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(0));
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 3)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%.3f"), m_pMain->vt_job_info[m_nInspAlgo[row - 1]].model_info.getAlignInfo().getDistanceInspSubSpec(2));
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 4)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%.3f"), m_pMain->vt_job_info[m_nInspAlgo[row - 1]].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(2));
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 5)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%.3f"), m_pMain->vt_job_info[m_nInspAlgo[row - 1]].model_info.getAlignInfo().getDistanceInspSubSpec(1));
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 6)
			{
			Item.nFormat = dwTextStyle;
			Item.strText.Format(_T("%.3f"), m_pMain->vt_job_info[m_nInspAlgo[row - 1]].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(1));
			UINT state = grid.GetItemState(row, col);
			grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 7)
			{
			Item.nFormat = dwTextStyle;
			Item.strText.Format(_T("%.3f"), m_pMain->vt_job_info[m_nInspAlgo[row - 1]].model_info.getAlignInfo().getDistanceInspSubSpec(3));
			UINT state = grid.GetItemState(row, col);
			grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 8)
			{
			Item.nFormat = dwTextStyle;
			Item.strText.Format(_T("%.3f"), m_pMain->vt_job_info[m_nInspAlgo[row - 1]].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(3));
			UINT state = grid.GetItemState(row, col);
			grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else
			{
				Item.nFormat = dwTextStyle;
				//Item.strText.Format("%s", m_strMarcoName[row - 1]);
			}

			if (row > 0)
			{
				if (nRowCount <= m_nInspCount + 1)
					grid.SetItemBkColour(row, col, RGB(235, 235, 235));
				else
					grid.SetItemBkColour(row, col, RGB(255, 255, 255));

				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state);
			}

			grid.SetItem(&Item);

		}

		if (row > 0)
			nRowCount++;

		if (nRowCount > 10)
			nRowCount = 1;

		grid.RedrawRow(row);
	}

	grid.AutoSizeColumns();
	//grid.ExpandColumnsToFit();
}

#pragma region (KJH 2021_08_07 Scan Insp Result Grid 분기함)
void CPaneAuto::Init_Grid_Scan_Insp_Spec_View_Setting(CGridCtrl& grid, int col, int row)
{
	BOOL bVirtualMode = FALSE;
	CRect rect;

	grid.SetEditable(FALSE);
	grid.SetVirtualMode(bVirtualMode);
	grid.SetListMode(TRUE);
	grid.EnableDragAndDrop(FALSE);
	grid.EnableSelection(FALSE);
	grid.SetFixedRowSelection(TRUE);
	grid.SetFixedColumnSelection(TRUE);
	grid.SetFrameFocusCell(FALSE);
	grid.SetTrackFocusCell(FALSE);
	grid.SetRowResize(TRUE);
	grid.SetColumnResize(FALSE);

	grid.SetRowCount(row);
	grid.SetColumnCount(col);

	grid.SetRowHeight(0, 25);
	grid.SetBkColor(RGB(197, 197, 187));
	grid.SetTextBkColor(RGB(235, 235, 235));
	grid.SetFixedRowCount(1);

	grid.GetWindowRect(&rect);
	int w = rect.Width() / 9;

	for (int i = 0; i < grid.GetColumnCount(); i++)
	{
		grid.SetItemBkColour(0, i, RGB(146, 146, 141));
		grid.SetItemFgColour(0, i, RGB(255, 255, 255));
		grid.SetColumnWidth(i, w);
	}

	grid.ExpandColumnsToFit();
}
void CPaneAuto::Init_Grid_Scan_Insp_Result_View_Setting(CGridCtrl& grid, int col, int row)
{
	BOOL bVirtualMode = FALSE;
	CRect rect;

	grid.SetEditable(FALSE);
	grid.SetVirtualMode(bVirtualMode);
	grid.SetListMode(TRUE);
	grid.EnableDragAndDrop(FALSE);
	grid.EnableSelection(FALSE);
	grid.SetFixedRowSelection(TRUE);
	grid.SetFixedColumnSelection(TRUE);
	grid.SetFrameFocusCell(FALSE);
	grid.SetTrackFocusCell(FALSE);
	grid.SetRowResize(TRUE);
	grid.SetColumnResize(FALSE);

	grid.SetRowCount(row);
	grid.SetColumnCount(col);

	grid.SetRowHeight(0, 25);
	grid.SetBkColor(RGB(197, 197, 187));
	grid.SetTextBkColor(RGB(235, 235, 235));
	grid.SetFixedRowCount(1);

	grid.GetWindowRect(&rect);
	int w = rect.Width() / 9;

	for (int i = 0; i < grid.GetColumnCount(); i++)
	{
		grid.SetItemBkColour(0, i, RGB(146, 146, 141));
		grid.SetItemFgColour(0, i, RGB(255, 255, 255));
		grid.SetColumnWidth(i, w);
	}

	grid.ExpandColumnsToFit();
}
void CPaneAuto::Init_Draw_Grid_Scan_Insp_Spec_Viewer(CGridCtrl& grid, int col, int row)
{
	//example)
	//-----------------------------------------------------------------------------------------------------------|
	//			| Top Min | Top Max | Bottom Min | Bottom Max | Total Min | Total Max | Length Min | Length Max  |
	//----------|---------|---------|------------|------------|-----------|-----------|------------|-------------|
	//   S1-1	|         |         |            |            |           |           |            |             |
	//----------|---------|---------|------------|------------|-----------|-----------|------------|-------------|
	//   S1-2	|         |         |            |            |           |           |            |             |
	// ---------|---------|---------|------------|------------|-----------|-----------|------------|-------------|
	//   S1-3	|         |         |            |            |           |           |            |             |
	// ---------|---------|---------|------------|------------|-----------|-----------|------------|-------------|
	//   S2-1	|         |         |            |            |           |           |            |             |
	//----------|---------|---------|------------|------------|-----------|-----------|------------|-------------|
	//   S2-2	|         |         |            |            |           |           |            |             |
	// ---------|---------|---------|------------|------------|-----------|-----------|------------|-------------|
	//   S2-3	|         |         |            |            |           |           |            |             |
	// ---------|---------|---------|------------|------------|-----------|-----------|------------|-------------|

	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nAlgocnt = int(m_pMain->vt_job_info.size());

	for (int row = 0; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row;
			Item.col = col;

			if (row == 0)
			{
				switch (col)
				{
				case 0:
				{
					Item.nFormat = DT_CENTER | DT_WORDBREAK;
					Item.strText.Format(_T(""));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 1:
				{
					Item.strText.Format(_T("Min-In"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 2:
				{
					Item.strText.Format(_T("Max-In"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 3:
				{
					Item.strText.Format(_T("Min-Out"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 4:
				{
					Item.strText.Format(_T("Max-Out"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 5:
				{
					Item.strText.Format(_T("Min-Total"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 6:
				{
					Item.strText.Format(_T("Max-Total"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 7:
				{
					Item.strText.Format(_T("Min-Length"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 8:
				{
					Item.strText.Format(_T("Max-Length"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				}
			}
			else if (row == 1 && col == 0)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("S1-1"));
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				grid.SetItemBkColour(row, col, COLOR_YELLOW);
			}
			else if (row == 2 && col == 0)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("S1-2"));
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				grid.SetItemBkColour(row, col, COLOR_YELLOW);
			}
			else if (row == 3 && col == 0)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("S1-3"));
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				grid.SetItemBkColour(row, col, COLOR_YELLOW);
			}
			else if (row == 4 && col == 0)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("S2-1"));
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				grid.SetItemBkColour(row, col, COLOR_LIME);
			}
			else if (row == 5 && col == 0)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("S2-2"));
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				grid.SetItemBkColour(row, col, COLOR_LIME);
			}
			else if (row == 6 && col == 0)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("S2-3"));
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				grid.SetItemBkColour(row, col, COLOR_LIME);
			}
			else
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("-"));
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}

			grid.SetItem(&Item);

		}
		grid.RedrawRow(row);
	}
	//grid.AutoSizeColumns();
	grid.ExpandColumnsToFit();
}
void CPaneAuto::Init_Draw_Grid_Scan_Insp_Result_1_Viewer(CGridCtrl& grid, int col, int row)
{
	//example)
	//--------------------------------------------------------------------------------------------------|
	//			| Top Min | Top Max | Bottom Min | Bottom Max | Total Min | Total Max | Length | Judge  |
	//----------|---------|---------|------------|------------|-----------|-----------|--------|--------|
	// S1-1		|         |         |            |            |           |           |        |        |
	//----------|---------|---------|------------|------------|-----------|-----------|--------|--------|
	// S2-1		|         |         |            |            |           |           |        |        |
	// ---------|---------|---------|------------|------------|-----------|-----------|--------|--------|            


	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nAlgocnt = int(m_pMain->vt_job_info.size());

	for (int row = 0; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row;
			Item.col = col;

			if (row == 0)
			{
				switch (col)
				{
				case 0:
				{
					Item.nFormat = DT_CENTER | DT_WORDBREAK;
					Item.strText.Format(_T(""));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 1:
				{
					Item.strText.Format(_T("Min-In"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 2:
				{
					Item.strText.Format(_T("Max-In"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 3:
				{
					Item.strText.Format(_T("Min-Out"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 4:
				{
					Item.strText.Format(_T("Max-Out"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 5:
				{
					Item.strText.Format(_T("Min-Total"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 6:
				{
					Item.strText.Format(_T("Max-Total"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 7:
				{
					Item.strText.Format(_T("Distance"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 8:
				{
					Item.strText.Format(_T("Judge"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				}
			}
			else
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("-"));
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}

			grid.SetItem(&Item);

		}
		grid.RedrawRow(row);
	}
	//grid.AutoSizeColumns();
	grid.ExpandColumnsToFit();
}
void CPaneAuto::Init_Draw_Grid_Scan_Insp_Result_2_Viewer(CGridCtrl& grid, int col, int row)
{
	//example)
	//--------------------------------------------------------------------------------------------------|
	//			| Top Min | Top Max | Bottom Min | Bottom Max | Total Min | Total Max | Length | Judge  |
	//----------|---------|---------|------------|------------|-----------|-----------|--------|--------|
	// S1-1		|         |         |            |            |           |           |        |        |
	//----------|---------|---------|------------|------------|-----------|-----------|--------|--------|
	// S2-1		|         |         |            |            |           |           |        |        |
	// ---------|---------|---------|------------|------------|-----------|-----------|--------|--------|            


	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nAlgocnt = int(m_pMain->vt_job_info.size());

	for (int row = 0; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row;
			Item.col = col;

			if (row == 0)
			{
				switch (col)
				{
				case 0:
				{
					Item.nFormat = DT_CENTER | DT_WORDBREAK;
					Item.strText.Format(_T(""));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 1:
				{
					Item.strText.Format(_T("Min-In"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 2:
				{
					Item.strText.Format(_T("Max-In"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 3:
				{
					Item.strText.Format(_T("Min-Out"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 4:
				{
					Item.strText.Format(_T("Max-Out"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 5:
				{
					Item.strText.Format(_T("Min-Total"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 6:
				{
					Item.strText.Format(_T("Max-Total"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 7:
				{
					Item.strText.Format(_T("Distance"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 8:
				{
					Item.strText.Format(_T("Judge"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				}
			}
			else
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("-"));
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}

			grid.SetItem(&Item);

		}
		grid.RedrawRow(row);
	}
	//grid.AutoSizeColumns();
	grid.ExpandColumnsToFit();
}
void CPaneAuto::Init_Draw_Grid_Scan_Insp_Result_3_Viewer(CGridCtrl& grid, int col, int row)
{
	//example)
	//--------------------------------------------------------------------------------------------------|
	//			| Top Min | Top Max | Bottom Min | Bottom Max | Total Min | Total Max | Length | Judge  |
	//----------|---------|---------|------------|------------|-----------|-----------|--------|--------|
	// S1-1		|         |         |            |            |           |           |        |        |
	//----------|---------|---------|------------|------------|-----------|-----------|--------|--------|
	// S2-1		|         |         |            |            |           |           |        |        |
	// ---------|---------|---------|------------|------------|-----------|-----------|--------|--------|            


	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nAlgocnt = int(m_pMain->vt_job_info.size());

	for (int row = 0; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row;
			Item.col = col;

			if (row == 0)
			{
				switch (col)
				{
				case 0:
				{
					Item.nFormat = DT_CENTER | DT_WORDBREAK;
					Item.strText.Format(_T(""));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 1:
				{
					Item.strText.Format(_T("Min-In"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 2:
				{
					Item.strText.Format(_T("Max-In"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 3:
				{
					Item.strText.Format(_T("Min-Out"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 4:
				{
					Item.strText.Format(_T("Max-Out"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 5:
				{
					Item.strText.Format(_T("Min-Total"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 6:
				{
					Item.strText.Format(_T("Max-Total"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 7:
				{
					Item.strText.Format(_T("Distance"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 8:
				{
					Item.strText.Format(_T("Judge"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				}
			}
			else
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("-"));
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}

			grid.SetItem(&Item);

		}
		grid.RedrawRow(row);
	}
	//grid.AutoSizeColumns();
	grid.ExpandColumnsToFit();
}

void CPaneAuto::Update_Grid_Scan_Insp_Spec()
{
	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	CGridCtrl* grid = &m_grid_spec_viewer;
	int nCam = 0;

	for (int row = 0; row < 3; row++)
	{
		for (int col = 1; col < grid->GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row + 1;		Item.col = col;

			double MinSpec_In		= m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getInSpec(row)		- m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getInSpecTorr(row);
			double MaxSpec_In		= m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getInSpec(row)		+ m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getInSpecTorr(row);
			double MinSpec_Out		= m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getOutSpec(row)		- m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getOutSpecTorr(row);
			double MaxSpec_Out		= m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getOutSpec(row)		+ m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getOutSpecTorr(row);
			double MinSpec_Total	= m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getTotalSpec(row)		- m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getTotalSpecTorr(row);
			double MaxSpec_Total	= m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getTotalSpec(row)		+ m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getTotalSpecTorr(row);
			double MinSpec_Distance = m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getDistanceSpec(row)	- m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getDistanceSpecTorr(row);
			double MaxSpec_Distance = m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getDistanceSpec(row)	+ m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getDistanceSpecTorr(row);

			switch (col)
			{
			case 1:		Item.strText.Format(_T("%.4f"), MinSpec_In);			break;
			case 2:		Item.strText.Format(_T("%.4f"), MaxSpec_In);			break;
			case 3:		Item.strText.Format(_T("%.4f"), MinSpec_Out);			break;
			case 4:     Item.strText.Format(_T("%.4f"), MaxSpec_Out);			break;
			case 5:		Item.strText.Format(_T("%.4f"), MinSpec_Total);			break;
			case 6:		Item.strText.Format(_T("%.4f"), MaxSpec_Total);			break;
			case 7:		Item.strText.Format(_T("%.4f"), MinSpec_Distance);		break;
			case 8:		Item.strText.Format(_T("%.4f"), MaxSpec_Distance);		break;
			}
			grid->SetItemBkColour(row + 1, col, COLOR_YELLOW);

			grid->SetItem(&Item);
		}
		grid->RedrawRow(row + 1);
	}
	
	nCam = 1;

	for (int row = 0; row < 3; row++)
	{
		for (int col = 1; col < grid->GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row + 4;		Item.col = col;

			double MinSpec_In			= m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getInSpec(row)		- m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getInSpecTorr(row);
			double MaxSpec_In			= m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getInSpec(row)		+ m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getInSpecTorr(row);
			double MinSpec_Out			= m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getOutSpec(row)		- m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getOutSpecTorr(row);
			double MaxSpec_Out			= m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getOutSpec(row)		+ m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getOutSpecTorr(row);
			double MinSpec_Total		= m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getTotalSpec(row)		- m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getTotalSpecTorr(row);
			double MaxSpec_Total		= m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getTotalSpec(row)		+ m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getTotalSpecTorr(row);
			double MinSpec_Distance		= m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getDistanceSpec(row)	- m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getDistanceSpecTorr(row);
			double MaxSpec_Distance		= m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getDistanceSpec(row)	+ m_pMain->vt_job_info[nCam].model_info.getInspSpecParaInfo().getDistanceSpecTorr(row);

			switch (col)
			{
			case 1:		Item.strText.Format(_T("%.4f"), MinSpec_In);			break;
			case 2:		Item.strText.Format(_T("%.4f"), MaxSpec_In);			break;
			case 3:		Item.strText.Format(_T("%.4f"), MinSpec_Out);			break;
			case 4:     Item.strText.Format(_T("%.4f"), MaxSpec_Out);			break;
			case 5:		Item.strText.Format(_T("%.4f"), MinSpec_Total);			break;
			case 6:		Item.strText.Format(_T("%.4f"), MaxSpec_Total);			break;
			case 7:		Item.strText.Format(_T("%.4f"), MinSpec_Distance);		break;
			case 8:		Item.strText.Format(_T("%.4f"), MaxSpec_Distance);		break;
			}
			grid->SetItemBkColour(row + 4, col, COLOR_LIME);

			grid->SetItem(&Item);
		}
		grid->RedrawRow(row + 4);
	}
}
void CPaneAuto::Update_Grid_Scan_Insp_Result_Viewer(int nJob, int nPos)
{
	EnterCriticalSection(&m_csGridDraw);

	if (nPos > 0 && nPos < 4 )
	{
		Move_Grid_Scan_Insp_Result_Viewer(nPos);
	}

	//example)
	//--------------------------------------------------------------------------------------------------|
	//			| In  Min | In  Max | Out    Min | Out    Max | Total Min | Total Max | Length | Judge  |
	//----------|---------|---------|------------|------------|-----------|-----------|--------|--------|
	// S1-1		|         |         |            |            |           |           |        |        |
	//----------|---------|---------|------------|------------|-----------|-----------|--------|--------|
	// S2-1		|         |         |            |            |           |           |        |        |
	// ---------|---------|---------|------------|------------|-----------|-----------|--------|--------|    

	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	CGridCtrl* grid;

	if (nPos == 1)
	{
		grid = &m_grid_InspResult_1_viewer;
	}
	else if (nPos == 2)
	{
		grid = &m_grid_InspResult_2_viewer;
	}
	else if (nPos == 3)
	{
		grid = &m_grid_InspResult_3_viewer;
	}
	else
	{
		LeaveCriticalSection(&m_csGridDraw); // 20210924 Tkyuha 예외처리 해주어야됨
		return;
	}

	double itemData = 0;

	int row = 1;// m_pMain->m_nSpecRowCount;
	BOOL bJudge = TRUE;
	for (int col = 0; col < grid->GetColumnCount(); col++)
	{
		GV_ITEM Item;
		Item.mask = GVIF_TEXT | GVIF_FORMAT;
		Item.row = row;		Item.col = col;

		double yres = m_pMain->GetMachine(0).getCameraResolutionY(0, 0);
		double distance = (m_pMain->m_ELB_ScanInspResult_UT[nJob][nPos].m_ELB_FindPoint.y - m_pMain->m_ELB_ScanInspResult_UT[nJob][nPos].m_ELB_FindPoint.x) * yres;

		switch (col)
		{
			case 0:
			{
				if (nPos == 1)
				{
					Item.strText.Format(_T("S%d-1"), nJob + 1);
				}
				else if (nPos == 2)
				{
					Item.strText.Format(_T("S%d-2"), nJob + 1);
				}
				else if (nPos == 3)
				{
					Item.strText.Format(_T("S%d-3"), nJob + 1);
				}
				Item.nFormat = DT_CENTER;
				if (nJob == 0) grid->SetItemBkColour(row, col, COLOR_YELLOW);
				if (nJob == 1) grid->SetItemBkColour(row, col, COLOR_LIME);
				break;
			}
			case 1:	//In-Min
			{
				Item.strText.Format(_T("%.4f"), m_pMain->m_ELB_ScanInspResult_UT[nJob][nPos].m_vELB_InMinMaxValue.x);
				double Spec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInSpec(nPos - 1) - m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInSpecTorr(nPos - 1);
				itemData = atof(Item.strText);
				if (itemData < Spec)
				{
					grid->SetItemBkColour(row, col, COLOR_RED);
					bJudge = FALSE;
				}
				else
				{
					if (nJob == 0) grid->SetItemBkColour(row, col, COLOR_YELLOW);
					if (nJob == 1) grid->SetItemBkColour(row, col, COLOR_LIME);
				}
				break;
			}
			case 2:	//In-Max
			{
				Item.strText.Format(_T("%.4f"), m_pMain->m_ELB_ScanInspResult_UT[nJob][nPos].m_vELB_InMinMaxValue.y);
				double Spec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInSpec(nPos - 1) + m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInSpecTorr(nPos - 1);
				itemData = atof(Item.strText);
				if (itemData > Spec)
				{
					grid->SetItemBkColour(row, col, COLOR_RED);
					bJudge = FALSE;
				}
				else
				{
					if (nJob == 0) grid->SetItemBkColour(row, col, COLOR_YELLOW);
					if (nJob == 1) grid->SetItemBkColour(row, col, COLOR_LIME);
				}
				break;
			}
			case 3:	//Out-Min
			{
				Item.strText.Format(_T("%.4f"), m_pMain->m_ELB_ScanInspResult_UT[nJob][nPos].m_vELB_OutMinMaxValue.x);
				double Spec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getOutSpec(nPos - 1) - m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getOutSpecTorr(nPos - 1);
				itemData = atof(Item.strText);
				if (itemData < Spec)
				{
					grid->SetItemBkColour(row, col, COLOR_RED);
					bJudge = FALSE;
				}
				else
				{
					if (nJob == 0) grid->SetItemBkColour(row, col, COLOR_YELLOW);
					if (nJob == 1) grid->SetItemBkColour(row, col, COLOR_LIME);
				}
				break;
			}
			case 4:	//Out-Max
			{
				Item.strText.Format(_T("%.4f"), m_pMain->m_ELB_ScanInspResult_UT[nJob][nPos].m_vELB_OutMinMaxValue.y);
				double Spec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getOutSpec(nPos - 1) + m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getOutSpecTorr(nPos - 1);
				itemData = atof(Item.strText);
				if (itemData > Spec)
				{
					grid->SetItemBkColour(row, col, COLOR_RED);
					bJudge = FALSE;
				}
				else
				{
					if (nJob == 0) grid->SetItemBkColour(row, col, COLOR_YELLOW);
					if (nJob == 1) grid->SetItemBkColour(row, col, COLOR_LIME);
				}
				break;
			}
			case 5:	//Total-Min
			{
				Item.strText.Format(_T("%.4f"), m_pMain->m_ELB_ScanInspResult_UT[nJob][nPos].m_vELB_TotalMinMaxValue.x);
				double Spec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getTotalSpec(nPos - 1) - m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getTotalSpec(nPos - 1);
				itemData = atof(Item.strText);
				if (itemData < Spec)
				{
					grid->SetItemBkColour(row, col, COLOR_RED);
					bJudge = FALSE;
				}
				else
				{
					if (nJob == 0) grid->SetItemBkColour(row, col, COLOR_YELLOW);
					if (nJob == 1) grid->SetItemBkColour(row, col, COLOR_LIME);
				}
				break;
			}
			case 6:	//Total-Max
			{
				Item.strText.Format(_T("%.4f"), m_pMain->m_ELB_ScanInspResult_UT[nJob][nPos].m_vELB_TotalMinMaxValue.y);
				double Spec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getTotalSpec(nPos - 1) + m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getTotalSpecTorr(nPos - 1);
				itemData = atof(Item.strText);
				if (itemData > Spec)
				{
					grid->SetItemBkColour(row, col, COLOR_RED);
					bJudge = FALSE;
				}
				else
				{
					if (nJob == 0) grid->SetItemBkColour(row, col, COLOR_YELLOW);
					if (nJob == 1) grid->SetItemBkColour(row, col, COLOR_LIME);
				}
				break;
			}
			case 7:	//Distance
			{
				Item.strText.Format(_T("%.4f"), distance);
				
				double Spec_Min = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getDistanceSpec(nPos - 1) - m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getDistanceSpecTorr(nPos - 1);
				double Spec_Max = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getDistanceSpec(nPos - 1) + m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getDistanceSpecTorr(nPos - 1);
				
				itemData = atof(Item.strText);

				if (itemData < Spec_Min || itemData > Spec_Max)
				{
					grid->SetItemBkColour(row, col, COLOR_RED);
					bJudge = FALSE;
				}
				else
				{
					if (nJob == 0) grid->SetItemBkColour(row, col, COLOR_YELLOW);
					if (nJob == 1) grid->SetItemBkColour(row, col, COLOR_LIME);
				}
				break;
			}
			case 8:	//Distance
			{
				if (bJudge)
				{
					grid->SetItemBkColour(row, col, COLOR_GREEN);
					Item.strText.Format(_T("OK"));
				}
				else
				{
					grid->SetItemBkColour(row, col, COLOR_RED);
					Item.strText.Format(_T("NG"));
				}
				break;
			}
		}
		grid->SetItem(&Item);
	}
	grid->RedrawRow(row);
	grid->Refresh();
	LeaveCriticalSection(&m_csGridDraw);
}
void CPaneAuto::Move_Grid_Scan_Insp_Result_Viewer(int nPos)
{
	CGridCtrl* grid;

	if (nPos == 1)
	{
		grid = &m_grid_InspResult_1_viewer;
	}
	else if (nPos == 2)
	{
		grid = &m_grid_InspResult_2_viewer;
	}
	else if (nPos == 3)
	{
		grid = &m_grid_InspResult_3_viewer;
	}
	else
	{
		return;
	}
	CString itemData;
	COLORREF color;
	int row = 0;
	int col = 0;
	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;

	for (row = grid->GetRowCount() - 2; row >= 1; row--)
	{
		for (col = 0; col < grid->GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.nFormat = dwTextStyle;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row + 1;		Item.col = col;
			itemData = grid->GetItemText(row, col);
			Item.strText.Format(itemData);

			color = grid->GetItemBkColour(row, col);
			grid->SetItemBkColour(row + 1, col, color);
			grid->SetItem(&Item);
		}
		grid->RedrawRow(row);
		grid->RedrawRow(row + 1);
	}

	grid->Refresh();
}
#pragma endregion

//HSJ inspection 추가
void CPaneAuto::draw_grid_inspection_spec_viewer(CGridCtrl& grid, int t_col, int t_row)
{
	//example)
	//--------------------------------------------------------------------------------------------------|
	//			| Top Min | Top Max | Bottom Min | Bottom Max | Total Min | Total Max | Length | Judge  |
	//----------|---------|---------|------------|------------|-----------|-----------|--------|--------|
	// S1-Spec	|         |         |            |            |           |           |        |        |
	//----------|---------|---------|------------|------------|-----------|-----------|--------|--------|
	// S2-Spec  |         |         |            |            |           |           |        |        |
	// ---------|---------|---------|------------|------------|-----------|-----------|--------|--------|            
	// S1		|         |         |            |            |           |           |        |        |
	// ---------|---------|---------|------------|------------|-----------|-----------|--------|--------| 
	// S2		|         |         |            |            |           |           |        |        |
	// ---------|---------|---------|------------|------------|-----------|-----------|--------|--------| 

	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nAlgocnt = int(m_pMain->vt_job_info.size());
	BOOL bLineDispMode = m_pMain->vt_job_info[0].model_info.getAlignInfo().getLineOverflowDispMode();  // false : 


	for (int row = 0; row < t_row; row++)
	{
		for (int col = 0; col < t_col; col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row;
			Item.col = col;

			if (row == 0)
			{
				switch (col)
				{
				case 0:
				{
					Item.nFormat = DT_CENTER | DT_WORDBREAK;
					Item.strText.Format(_T("/"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
					
				}
				break;
				case 1:
				{
					Item.strText.Format(_T("ID"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				case 2:
				{
					Item.strText.Format(_T("Min(in)"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemFgColour(0, col, COLOR_BLACK);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
					//KJH2 2022-08-17 Colum 색상 번경
					grid.SetItemBkColour(0, 2, RGB(173,255,47)); //SJB 2022-11-29 Wetout, Overflow 오버레이 수정(색 통일)
				}
				break;
				case 3:
				{
					Item.strText.Format(_T("Max(in)"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemFgColour(0, col, COLOR_BLACK);	//SJB 2022-11-30 Result Grid 0row 항목 텍스트 색 변경
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
					grid.SetItemBkColour(0, 3, RGB(173, 255, 47)); //SJB 2022-11-29 Wetout, Overflow 오버레이 수정(색 통일)
				}
				break;
				case 4:
				{
					if (bLineDispMode) Item.strText.Format(_T("Overflow(Max)"));
					else Item.strText.Format(_T("Max(out)"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemFgColour(0, col, COLOR_BLACK);	//SJB 2022-11-30 Result Grid 0row 항목 텍스트 색 변경
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
					grid.SetItemBkColour(0, 4, RGB(128, 128, 255)); //SJB 2022-11-29 Wetout, Overflow 오버레이 수정(색 통일)
				}
				break;
				case 5:
				{
					//KJH 2022-05-25 Length -> BMinSpec으로 UI 변경
					//KJH 2022-06-06 R Display제거 / Length는 Line에서 사용함
					//KJH 2022-06-30 GridSpecView Length Title 변경
					int method = m_pMain->vt_job_info[0].model_info.getAlignInfo().getMarkFindMethod();
					if		(method == METHOD_CIRCLE)	Item.strText.Format(_T("Lack"));
					else if (method == METHOD_LINE)
					{
						//Item.strText.Format(_T("Length"));
						//SJB 2022-11-11 Overflow Min Grid 수정
						Item.strText.Format(_T("Overflow(Min)"));
					}
					else								Item.strText.Format(_T("Length"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemFgColour(0, col, COLOR_BLACK);	//SJB 2022-11-30 Result Grid 0row 항목 텍스트 색 변경
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
					grid.SetItemBkColour(0, 5, COLOR_PINK);
					//SJB 2022-11-11 Overflow Min Grid 수정
					if (method == METHOD_LINE)
					{
						grid.SetItemBkColour(0, 5, RGB(128, 128, 255)); //SJB 2022-11-29 Wetout, Overflow 오버레이 수정(색 통일)
					}
				}
				break;
				case 6:
				{
					//KJH 2022-06-06 R Display제거 / Length는 Line에서 사용함
					//SJB 2022-11-11 Overflow Min Grid 수정
					int method = m_pMain->vt_job_info[0].model_info.getAlignInfo().getMarkFindMethod();
					if (method == METHOD_LINE)	Item.strText.Format(_T("Length"));
					else						Item.strText.Format(_T("Min(B)"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemFgColour(0, col, COLOR_BLACK);	//SJB 2022-11-30 Result Grid 0row 항목 텍스트 색 변경
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
					grid.SetItemBkColour(0, 6, COLOR_PINK);
				}
				break;
				case 7:
				{
					//KJH 2022-06-06 R Display제거 / Length는 Line에서 사용함
					int method = m_pMain->vt_job_info[0].model_info.getAlignInfo().getMarkFindMethod();
					if (method == METHOD_LINE)	Item.strText.Format(_T("CG-start"));
					else						Item.strText.Format(_T("Max(B)"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemFgColour(0, col, COLOR_BLACK);	///SJB 2022-11-30 Result Grid 0row 항목 텍스트 색 변경
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
					if (method == METHOD_LINE)  grid.SetItemBkColour(0, 7, RGB(60, 187, 242));
					else						grid.SetItemBkColour(0, 7, COLOR_PINK);
				}
				break;
				case 8:
				{
					int method = m_pMain->vt_job_info[0].model_info.getAlignInfo().getMarkFindMethod();
					if (method == METHOD_LINE)	Item.strText.Format(_T("CG-end")); //KTY 2023-01-03 Line/Circle 구분
					else						Item.strText.Format(_T("Judge"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemFgColour(0, col, COLOR_BLACK);	///SJB 2022-11-30 Result Grid 0row 항목 텍스트 색 변경
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
					if (method == METHOD_LINE)  grid.SetItemBkColour(0, 8, RGB(60, 187, 242));  //KTY 2023-01-03 Line/Circle 구분
					else						grid.SetItemFgColour(0, col, COLOR_BLACK);
				}
				break;
				case 9:
				{
					Item.strText.Format(_T("Judge"));
					UINT state = grid.GetItemState(row, col);
					grid.SetItemFgColour(0, col, COLOR_BLACK);	///SJB 2022-11-30 Result Grid 0row 항목 텍스트 색 변경
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				break;
				}
			}
			else if (row == 1 && col == 0)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("Spec"));
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				grid.SetItemBkColour(row, col, COLOR_YELLOW);
			}
			else
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("-"));
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}

			grid.SetItem(&Item);

		}

		grid.RedrawRow(row);
	}
	//grid.AutoSizeColumns();
	grid.ExpandColumnsToFit();
}
void CPaneAuto::draw_grid_ref_data_viewer(CGridCtrl& grid, int col)
{
	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nAlgocnt = int(m_pMain->vt_job_info.size());
	int row = 0;
	for (int col = 0; col < grid.GetColumnCount(); col++)
	{
		GV_ITEM Item;
		Item.mask = GVIF_TEXT | GVIF_FORMAT;
		Item.row = row;
		Item.col = col;

		switch (col)
		{
		case 0:
		{
			Item.strText.Format(_T("Pressure(kPa)"));
			UINT state = grid.GetItemState(row, col);
			grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
		}
		break;
		case 1:
		{
			Item.strText.Format(_T("Voltage(Kvolt)"));
			UINT state = grid.GetItemState(row, col);
			grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
		}
		break;
		case 2:
		{
			Item.strText.Format(_T("Thickness(mm)"));
			UINT state = grid.GetItemState(row, col);
			grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
		}
		break;
		}
		grid.SetItem(&Item);
	}
	grid.RedrawRow(row);
	//grid.AutoSizeColumns();
	grid.ExpandColumnsToFit();
}
//HTK 2022-03-30 Tact Time Display 추가
void CPaneAuto::draw_grid_proc_time_viewer(CGridCtrl& grid, int col)
{
	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style

	GV_ITEM Item;
	Item.mask = GVIF_TEXT | GVIF_FORMAT;
	Item.nFormat = dwTextStyle;

	CString ItemProcess[9] = {	"Dust or Metal(BM) Insp",
								"Center Align",
								"Trace Calc",
								"Trace Data Send",
								"Wetout Insp",
								"Drop Insp",
								"Align Total Seq",
								"Trace Total Seq",
								"Insp Total Seq"
							 };

	for (int row = 0; row < 10; row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{
			Item.row = row;
			Item.col = col;
			
			UINT state = grid.GetItemState(row, col);
			grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_READONLY);

			if (row == 0)
			{
				switch (col)
				{
				case 0:					Item.strText.Format(_T("Process"));					break;
				case 1:					Item.strText.Format(_T("Status"));					break;
				case 2:					Item.strText.Format(_T("Time(Sec)"));				break;
				}
				grid.SetItemBkColour(row, col, COLOR_ORANGE);
			}
			else
			{
				if (col == 0)
				{					
					Item.strText.Format("%s", ItemProcess[row - 1]);
					grid.SetItemBkColour(row, col, COLOR_YELLOW);
				}
				else Item.strText.Format(" - ");
			}
			grid.SetItem(&Item);
		}
		grid.RedrawRow(row);
	}

	grid.ExpandColumnsToFit();
}

void CPaneAuto::DrawInspResultChart(int(lParam))
{
	int algo = lParam;
}
void CPaneAuto::DrawInsViewer(int algo)
{
	int num = m_listInspection[algo].GetItemCount();

	if (num > 50)  m_listInspection[algo].DeleteAllItems();

	CString stIndex = "";
	CString stModel = "";
	//CString stUnit = "";
	CString stCam = "";
	CString stRevisionDataX1 = "";
	CString stRevisionDataY1 = "";
	CString stRevisionDataX2 = "";
	CString stRevisionDataY2 = "";

	//std::vector<int> camBuf = m_pAlgorithmInfo.getAlgorithmCameraIndex(algo);

//	for (int i = 0; i < m_pMain->m_pAlgorithmInfo.getAlgorithmCount(); i++)
	for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
	{
		//nAlgoNum = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(i);

		//for (int j = 0; j < m_pMain->m_pAlgorithmInfo.getAlgorithmCameraNum(i); j++)
		for (int j = 0; j < m_pMain->vt_job_info[i].num_of_camera; j++)
		{
			int real_cam = m_pMain->vt_job_info[i].camera_index[j];
			m_listInspection[algo].AddItem("");

			//Time
			stIndex.Format("%d", nresultcnt);
			m_listInspection[algo].SetItemText(num, 0, stIndex);
			//Unit
			//stUnit.Format("%s", m_pMain->m_pAlgorithmInfo.getAlgorithmName(algo).c_str());
			/*stUnit.Format("%s", m_pMain->vt_job_info[i].job_name.c_str());
			m_listInspection[algo].SetItemText(num, 1, stUnit);*/
			//Cam
			stCam.Format("%d", nAlgo[i][j]);
			m_listInspection[algo].SetItemText(num, 1, stCam);
			//x1
	//		stRevisionDataX1.Format("%4.3f", m_pMain->m_stInsp_DistanceResult[nAlgo[i][j]][0].xPos[0]);
			stRevisionDataX1.Format("%4.3f", m_pMain->m_stInsp_DistanceResult[real_cam][0].xPos[0]);
			m_listInspection[algo].SetItemText(num, 2, stRevisionDataX1);
			//y1
			stRevisionDataY1.Format("%4.3f", m_pMain->m_stInsp_DistanceResult[real_cam][0].yPos[0]);
			m_listInspection[algo].SetItemText(num, 3, stRevisionDataY1);
			//x2
			stRevisionDataX2.Format("%4.3f", m_pMain->m_stInsp_DistanceResult[real_cam][0].xPos[1]);
			m_listInspection[algo].SetItemText(num, 4, stRevisionDataX2);
			//y2
			stRevisionDataY2.Format("%4.3f", m_pMain->m_stInsp_DistanceResult[real_cam][0].yPos[1]);
			m_listInspection[algo].SetItemText(num, 5, stRevisionDataY2);

			m_listInspection[algo].Complete();

			num++;
		}
	}
	//nAlgoIndex++;
}

void CPaneAuto::PathFromList(int row)
{

	BOOL bFind = TRUE;

	CString  strJobName, strPanelID, strClikedPath, strDate, strResult, strJudge;
	SYSTEMTIME	csTime;
	::GetLocalTime(&csTime);

	int nJob = m_TabResultView.GetCurSel();
	int totalItem = m_listInspection[nJob].GetItemCount();

	strPanelID = m_listInspection[nJob].GetItemText(row, 1);
	strJobName = m_pMain->vt_job_info[nJob].get_job_name();

	if (row < totalItem)
	{
		if (m_LastReusltDate[nJob].GetLength() == 0)
		{
			strJudge = "NG";
			for (int Day = csTime.wDay; Day > 0; Day--)
			{
				bFind = TRUE;
				strDate.Format("%04d%02d%02d", csTime.wYear, csTime.wMonth, Day);
				strClikedPath.Format("%simage\\%s\\%s\\%s\\%s\\%s", m_pMain->m_strResultDir, strDate, m_pMain->m_strCurrentModelName, strJobName, strJudge, strPanelID);
				if (_access(strClikedPath, 0) == 0) break;
				bFind = FALSE;
			}
		}
		else
		{
			strJudge = "OK";
			for (int Day = csTime.wDay; Day > 0; Day--)
			{
				bFind = TRUE;
				strDate.Format("%04d%02d%02d", csTime.wYear, csTime.wMonth, Day);
				strClikedPath.Format("%simage\\%s\\%s\\%s\\%s\\%s", m_pMain->m_strResultDir, strDate, m_pMain->m_strCurrentModelName, strJobName, strJudge, strPanelID);
				if (_access(strClikedPath, 0) == 0) break;
				bFind = FALSE;
			}
		}

		if (bFind != TRUE)
		{
			strDate.Format("%04d%02d%02d", csTime.wYear, csTime.wMonth, csTime.wDay);
			strClikedPath.Format("%simage\\%s\\%s\\%s\\Simulation\\%s", m_pMain->m_strResultDir, strDate, m_pMain->m_strCurrentModelName, strJobName, strPanelID);
		}

		// 경로 복사
		int str_length = strClikedPath.GetLength() + 1;
		HANDLE h_data = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_MOVEABLE, str_length);
		char* p_data = (char*)::GlobalLock(h_data);
		if (NULL != p_data)
		{
			memcpy(p_data, strClikedPath, str_length);

			::GlobalUnlock(h_data);
			if (::OpenClipboard(m_hWnd)) {
				::EmptyClipboard();
				::SetClipboardData(CF_TEXT, h_data);
				::CloseClipboard();
			}
		}
	}
}

void CPaneAuto::OnNMDblclkListInspection(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	POINT point = pNMItemActivate->ptAction;
	int row = pNMItemActivate->iItem;
	
	PathFromList(row); // 경로 복사
}

void CPaneAuto::OnNMDblclkListInspection2(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	POINT point = pNMItemActivate->ptAction;
	int row = pNMItemActivate->iItem;

	PathFromList(row); // 경로 복사
}

void CPaneAuto::OnNMDblclkListInspection3(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	POINT point = pNMItemActivate->ptAction;
	int row = pNMItemActivate->iItem;

	PathFromList(row); // 경로 복사
}

void CPaneAuto::OnNMDblclkListInspection4(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	POINT point = pNMItemActivate->ptAction;
	int row = pNMItemActivate->iItem;

	PathFromList(row); // 경로 복사
}

void CPaneAuto::OnNMDblclkListInspection5(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	POINT point = pNMItemActivate->ptAction;
	int row = pNMItemActivate->iItem;

	PathFromList(row); // 경로 복사
}

void CPaneAuto::OnNMDblclkListInspection6(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	POINT point = pNMItemActivate->ptAction;
	int row = pNMItemActivate->iItem;

	PathFromList(row); // 경로 복사
}

void CPaneAuto::OnNMDblclkListInspection7(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	POINT point = pNMItemActivate->ptAction;
	int row = pNMItemActivate->iItem;

	PathFromList(row); // 경로 복사
}

void CPaneAuto::OnNMDblclkListInspection8(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	POINT point = pNMItemActivate->ptAction;
	int row = pNMItemActivate->iItem;

	PathFromList(row); // 경로 복사
}

void CPaneAuto::OnTcnSelchangeTabSpecview(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;

	int index = m_TabSpecView.GetCurSel();
	
	RenewSpecChart(index);
}

//***************************************************************************
//000  //* Simple Graph //*
void CPaneAuto::CreateSimpleGraph()
{
	//! Right button click to show popup menu. //
	//! Double Click to show customization dialog. //
	//! Left-Click and drag to draw zoom box. Use popup memu or 'z' to undo zoom. // 

	// Simple example show the basics of a graph object. //
	// Graph's generally only contain YData because we assume
	// data is plotted equally spaced left to right.

	RECT rect;
	//	GetClientRect(&rect);
	GetDlgItem(IDC_TAB_SPECVIEW)->GetWindowRect(&rect);
	ScreenToClient(&rect);
	rect.top += 20;
	rect.bottom += 75;
	int nSubSetSize = int(m_pMain->m_vInspWetOutResult.size());
	m_hPE = PEcreate(PECONTROL_GRAPH, WS_VISIBLE, &rect, m_hWnd, 1001);
	if (m_hPE)
	{
		// Enable Bar Glass Effect //
		PEnset(m_hPE, PEP_bBARGLASSEFFECT, TRUE);
		PEnset(m_hPE, PEP_bSHOWLEGEND, FALSE);
		// Enable Plotting style gradient and bevel features //
		PEnset(m_hPE, PEP_nAREAGRADIENTSTYLE, PEPGS_RADIAL_BOTTOM_RIGHT);
		PEnset(m_hPE, PEP_nAREABEVELSTYLE, PEBS_MEDIUM_SMOOTH);
		PEnset(m_hPE, PEP_nSPLINEGRADIENTSTYLE, PEPGS_RADIAL_BOTTOM_RIGHT);
		PEnset(m_hPE, PEP_nSPLINEBEVELSTYLE, PESBS_MEDIUM_SMOOTH);

		// v7.2 new features //
		PEnset(m_hPE, PEP_nPOINTGRADIENTSTYLE, PEPGS_VERTICAL_ASCENT_INVERSE);
		PEnset(m_hPE, PEP_dwPOINTBORDERCOLOR, PERGB(100, 0, 0, 0));
		PEnset(m_hPE, PEP_nLINESYMBOLTHICKNESS, 3);
		PEnset(m_hPE, PEP_nAREABORDER, 1);
		PEnset(m_hPE, PEP_bALLOWSVGEXPORT, 1);

		// Prepare images in memory //
		PEnset(m_hPE, PEP_bPREPAREIMAGES, TRUE);

		// Pass Data //
		PEnset(m_hPE, PEP_nSUBSETS, nSubSetSize);
		PEnset(m_hPE, PEP_nPOINTS, 6);

		float fY;
		for (int i = 0; i < nSubSetSize; i++)
		{
			for (int j = 0; j < 6; j++)
			{
				if (j == 0)
					fY = (float)m_pMain->m_vInspWetOutResult[i].dMin_In;
				else if (j == 1)
					fY = (float)m_pMain->m_vInspWetOutResult[i].dMax_In;
				else if (j == 2)
					fY = (float)m_pMain->m_vInspWetOutResult[i].dMin_Out;
				else if (j == 3)
					fY = (float)m_pMain->m_vInspWetOutResult[i].dMax_Out;
				else if (j == 4)
					fY = (float)m_pMain->m_vInspWetOutResult[i].dMin_Total;
				else if (j == 5)
					fY = (float)m_pMain->m_vInspWetOutResult[i].dMin_Total;
				
				PEvsetcellEx(m_hPE, PEP_faYDATA, i, j, &fY);
			}
		}
		
		//float fY;
		//for (int s = 0; s <= 1; s++)
		//{
		//	for (int p = 0; p <= 6; p++)
		//	{
		//		fY = (((float)(p + 1)) * 50.0F) + (((float)(s + 1)) * 10.0F);
		//		PEvsetcellEx(m_hPE, PEP_faYDATA, s, p, &fY);
		//	}
		//}

		PEnset(m_hPE, PEP_nDATASHADOWS, PEDS_SHADOWS);
		PEnset(m_hPE, PEP_bFOCALRECT, FALSE);
		PEnset(m_hPE, PEP_nPLOTTINGMETHOD, PEGPM_POINT);
		PEnset(m_hPE, PEP_nGRIDLINECONTROL, PEGLC_BOTH);
		PEnset(m_hPE, PEP_nGRIDSTYLE, PEGS_DOT);
		PEnset(m_hPE, PEP_bALLOWRIBBON, TRUE);
		PEnset(m_hPE, PEP_nALLOWZOOMING, PEAZ_HORZANDVERT);
		PEnset(m_hPE, PEP_nZOOMSTYLE, PEZS_RO2_NOT);

		// Enable middle mouse dragging //
		PEnset(m_hPE, PEP_bMOUSEDRAGGINGX, TRUE);
		PEnset(m_hPE, PEP_bMOUSEDRAGGINGY, TRUE);

		PEszset(m_hPE, PEP_szMAINTITLE, TEXT("Measure Data"));
		PEszset(m_hPE, PEP_szSUBTITLE, TEXT(""));
		PEszset(m_hPE, PEP_szYAXISLABEL, TEXT(""));
		PEszset(m_hPE, PEP_szXAXISLABEL, TEXT(""));

		// point labels //
		PEvsetcell(m_hPE, PEP_szaPOINTLABELS, 0, TEXT("Min(in)"));
		PEvsetcell(m_hPE, PEP_szaPOINTLABELS, 1, TEXT("Max(in)"));
		PEvsetcell(m_hPE, PEP_szaPOINTLABELS, 2, TEXT("Min(out)"));
		PEvsetcell(m_hPE, PEP_szaPOINTLABELS, 3, TEXT("Max(out)"));
		PEvsetcell(m_hPE, PEP_szaPOINTLABELS, 4, TEXT("Min(total)"));
		PEvsetcell(m_hPE, PEP_szaPOINTLABELS, 5, TEXT("Max(total)"));

		//// subset colors //
		//DWORD dwArray[4] = { PERGB(128,198,0,0), PERGB(128,0, 198, 198), PERGB(128,198,198,0), PERGB(128,0,198,0) };
		//PEvsetEx(m_hPE, PEP_dwaSUBSETCOLORS, 0, 4, dwArray, 0);

		//// subset line types //
		//int nLineTypes[] = { PELT_MEDIUMSOLID, PELT_MEDIUMSOLID,
		//	PELT_MEDIUMSOLID, PELT_MEDIUMSOLID, PELT_MEDIUMSOLID,
		//	PELT_MEDIUMSOLID, PELT_MEDIUMSOLID, PELT_MEDIUMSOLID };
		//PEvset(m_hPE, PEP_naSUBSETLINETYPES, nLineTypes, 8);

		//// subset point types //
		//int nPointTypes[] = { PEPT_DOTSOLID, PEPT_UPTRIANGLESOLID,
		//	PEPT_SQUARESOLID, PEPT_DOWNTRIANGLESOLID, PEPT_DOTSOLID,
		//	PEPT_SQUARESOLID, PEPT_DIAMONDSOLID, PEPT_UPTRIANGLESOLID };
		//PEvset(m_hPE, PEP_naSUBSETPOINTTYPES, nPointTypes, 8);


		// Allow stacked type graphs //
		PEnset(m_hPE, PEP_bNOSTACKEDDATA, FALSE);

		// Various other features //
		PEnset(m_hPE, PEP_bFIXEDFONTS, TRUE);
		PEnset(m_hPE, PEP_bBITMAPGRADIENTMODE, TRUE);
		PEnset(m_hPE, PEP_nQUICKSTYLE, PEQS_LIGHT_LINE);

		PEnset(m_hPE, PEP_nGRADIENTBARS, 8);
		PEnset(m_hPE, PEP_bLINESHADOWS, TRUE);
		//PEnset(m_hPE, PEP_bMAINTITLEBOLD, TRUE);
		//PEnset(m_hPE, PEP_bSUBTITLEBOLD, TRUE);
		PEnset(m_hPE, PEP_bLABELBOLD, TRUE);
		PEnset(m_hPE, PEP_nTEXTSHADOWS, PETS_BOLD_TEXT);
		PEnset(m_hPE, PEP_nFONTSIZE, PEFS_SMALL);

		PEnset(m_hPE, PEP_nDATAPRECISION, 3);
		PEnset(m_hPE, PEP_nGRAPHPLUSTABLE, PEGPT_GRAPH);
		PEnset(m_hPE, PEP_bMARKDATAPOINTS, FALSE);

		PEnset(m_hPE, PEP_nIMAGEADJUSTLEFT, 20);
		PEnset(m_hPE, PEP_nIMAGEADJUSTRIGHT, 20);
		PEnset(m_hPE, PEP_nIMAGEADJUSTTOP, 10);

		// Set export defaults //
		PEnset(m_hPE, PEP_nDPIX, 600);
		PEnset(m_hPE, PEP_nDPIY, 600);

		PEnset(m_hPE, PEP_nEXPORTSIZEDEF, PEESD_NO_SIZE_OR_PIXEL);
		PEnset(m_hPE, PEP_nEXPORTTYPEDEF, PEETD_PNG);
		PEnset(m_hPE, PEP_nEXPORTDESTDEF, PEEDD_CLIPBOARD);
		PEszset(m_hPE, PEP_szEXPORTUNITXDEF, TEXT("1280"));
		PEszset(m_hPE, PEP_szEXPORTUNITYDEF, TEXT("768"));
		PEnset(m_hPE, PEP_nEXPORTIMAGEDPI, 300);

		// These get set as part of demo controls, but your code will likely use
		//PEnset(m_hPE, PEP_nRENDERENGINE, PERE_DIRECT2D);
		//PEnset(m_hPE, PEP_bANTIALIASGRAPHICS, TRUE);
		//PEnset(m_hPE, PEP_bANTIALIASTEXT, TRUE);

		// Set Demo's RenderEngine to Direct2D // 
		//CMDIFrameWnd* pWnd = (CMDIFrameWnd*)AfxGetApp()->GetMainWnd();
		//pWnd->SendMessage(WM_CHANGE_DEMO_RENDERENGINE, RENDER_2DX, 0);
	}
}
void CPaneAuto::GraphBackgroundBitmap()
{
	// This example builds upon the basic CreateSimpleGraph '000' example chart //
	CreateSimpleGraph();

	PEnset(m_hPE, PEP_bBITMAPGRADIENTMODE, TRUE);
	PEnset(m_hPE, PEP_dwDESKCOLOR, 1);
	PEszset(m_hPE, PEP_szDESKBMPFILENAME, TEXT("cloud.jpg"));
	PEnset(m_hPE, PEP_nDESKBMPSTYLE, PEBS_TILED_BITBLT);
	PEnset(m_hPE, PEP_dwGRAPHFORECOLOR, PERGB(255, 0, 0, 0));
	PEnset(m_hPE, PEP_dwTEXTCOLOR, PERGB(255, 0, 0, 0));

	// Now sub-divide subsets into 2 axes //
	// Each axis to be transarent //
	int nArray[2] = { 2,2 };
	PEvset(m_hPE, PEP_naMULTIAXESSUBSETS, nArray, 2);
	PEnset(m_hPE, PEP_nLEGENDSTYLE, PELS_1_LINE_INSIDE_AXIS);
	PEnset(m_hPE, PEP_nMULTIAXESSIZING, TRUE);

	// Set first axis parameters //
	PEnset(m_hPE, PEP_nWORKINGAXIS, 0);
	PEszset(m_hPE, PEP_szYAXISLABEL, TEXT(""));
	PEnset(m_hPE, PEP_nPLOTTINGMETHOD, PEGPM_POINTSPLUSLINE);
	PEnset(m_hPE, PEP_dwGRAPHBACKCOLOR, 1);
	PEnset(m_hPE, PEP_nGRAPHGRADIENTSTYLE, PEGS_NO_GRADIENT);

	// Set second axis parameters //
	PEnset(m_hPE, PEP_nWORKINGAXIS, 1);
	PEszset(m_hPE, PEP_szYAXISLABEL, TEXT(""));
	PEnset(m_hPE, PEP_nPLOTTINGMETHOD, PEGPM_BAR);
	PEnset(m_hPE, PEP_dwGRAPHBACKCOLOR, 1);
	PEnset(m_hPE, PEP_nGRAPHGRADIENTSTYLE, PEGS_NO_GRADIENT);

	// Reset WorkingAxis when done //
	PEnset(m_hPE, PEP_nWORKINGAXIS, 0);

	// Set Various Other Properties //
	PEszset(m_hPE, PEP_szMAINTITLE, TEXT(""));
	PEszset(m_hPE, PEP_szSUBTITLE, TEXT(""));
	PEnset(m_hPE, PEP_nMULTIAXISSTYLE, PEMAS_SEPARATE_AXES);

	PEnset(m_hPE, PEP_dwTICKCOLOR, 1);
	PEnset(m_hPE, PEP_nBORDERTYPES, PETAB_NO_BORDER);
	PEnset(m_hPE, PEP_nPOINTSIZE, PEPS_LARGE);
	PEnset(m_hPE, PEP_nGRIDLINECONTROL, PEGLC_BOTH);
	PEnset(m_hPE, PEP_nGRIDSTYLE, PEGS_DOT);

	// Make the line types bold //
	int nLT;
	nLT = PELT_MEDIUMSOLID;
	PEvsetcell(m_hPE, PEP_naSUBSETLINETYPES, 0, &nLT);
	PEvsetcell(m_hPE, PEP_naSUBSETLINETYPES, 1, &nLT);
	PEvsetcell(m_hPE, PEP_naSUBSETLINETYPES, 2, &nLT);
	PEvsetcell(m_hPE, PEP_naSUBSETLINETYPES, 3, &nLT);

	PEnset(m_hPE, PEP_nIMAGEADJUSTTOP, 100);
	PEnset(m_hPE, PEP_nIMAGEADJUSTBOTTOM, 100);
	PEnset(m_hPE, PEP_nIMAGEADJUSTLEFT, 100);
	PEnset(m_hPE, PEP_nIMAGEADJUSTRIGHT, 100);

	PEnset(m_hPE, PEP_nGRAPHPLUSTABLE, PEGPT_GRAPH);
	PEnset(m_hPE, PEP_nDATASHADOWS, PEDS_3D);
}
void CPaneAuto::SaveRefDataCSV()
{
	m_grid_ref_data_viewer.Save(m_pMain->m_strModelDir + _T("RefData.csv"));
}
void CPaneAuto::LoadRefData()
{
	m_grid_ref_data_viewer.Load(m_pMain->m_strModelDir + _T("RefData.csv"));
	m_pMain->m_ELB_RefData.clear();

	CRect rect;

	m_grid_ref_data_viewer.GetWindowRect(&rect);
	int w = rect.Width() / 9;

	m_grid_ref_data_viewer.SetFixedRowCount(1);

	for (int i = 0; i < m_grid_ref_data_viewer.GetColumnCount(); i++)
	{
		m_grid_ref_data_viewer.SetItemBkColour(0, i, RGB(146, 146, 141));
		m_grid_ref_data_viewer.SetItemFgColour(0, i, RGB(255, 255, 255));
		m_grid_ref_data_viewer.SetColumnWidth(i, w);
	}

	CString itemData;
	for (int row = 1; row < m_grid_ref_data_viewer.GetRowCount(); row++)
	{
		_stRef_AirPresHighVolts_V _val;

		for (int col = 0; col < m_grid_ref_data_viewer.GetColumnCount(); col++)
		{

			itemData = m_grid_ref_data_viewer.GetItemText(row, col);
			switch (col)
			{
			case 0: _val.dAir = atof(itemData); break;
			case 1: _val.dHighVolt = atof(itemData); break;
			case 2: _val.dWet_Out = atof(itemData); break;
			}
		}
		
		m_pMain->m_ELB_RefData.push_back(_val);
	}
}

void CPaneAuto::OnClose()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	KillTimer(0);

	CFormView::OnClose();
}

void CPaneAuto::CreateTabCount()
{
	
	CRect rt;
	m_TabResultView.GetWindowRect(&rt);
	ScreenToClient(&rt);

	for (int algo = 0; algo < m_pMain->vt_job_info.size(); algo++)
	{
		//TAB개수 생성
		CString strJobName = "";
		strJobName = m_pMain->vt_job_info[algo].get_job_name();
		
		m_TabResultView.InsertItem(algo, strJobName);
		m_TabResultView.SetCurSel(0);

		//표 초기화 생성
		m_listInspection[algo].SetWindowPos(NULL, rt.left, rt.top + 20, rt.Width(), rt.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
	}
}

void CPaneAuto::OnTcnSelchangeTabResultview(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int index = m_TabResultView.GetCurSel();
	
	RenewResultChart(index);

	*pResult = 0;
}

//결과창 init
void CPaneAuto::CreateResultInit(int cul,int nAlgoIndex)
{
	CRect rt;
	m_TabResultView.GetWindowRect(&rt);
	ScreenToClient(&rt);

	switch (nAlgoIndex)
	{
	case CLIENT_TYPE_ALIGN:
	{
		for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
		{
			/*nCamcnt = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraNum(i);
			nAlgoNum = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(i);*/

			for (int j = 0; j < m_pMain->vt_job_info[i].num_of_camera; j++)
			{
				nAlgo[i][j] = m_pMain->vt_job_info[i].camera_index[j];
			}
		}

		int w = rt.Width() / 7;
		int colum = 0;

		m_listInspection[cul].InsertColumn(colum, "Time", LVCFMT_CENTER, w); colum++;
		//HSJ 2022-01-09 결과창에 ID표시 추가
		m_listInspection[cul].InsertColumn(colum, "ID", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "JUDGE", LVCFMT_CENTER, w); colum++;
		//m_listInspection.InsertColumn(1, "Model", LVCFMT_CENTER, 80);
		//m_listInspection[cul].InsertColumn(1, "Unit", LVCFMT_CENTER, 150);
		m_listInspection[cul].InsertColumn(colum, "X", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "Y", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "T", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "Length", LVCFMT_CENTER, w);
	}
	break;
	case CLIENT_TYPE_4CAM_1SHOT_ALIGN:
	{
		for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
		{
			/*nCamcnt = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraNum(i);
			nAlgoNum = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(i);*/

			for (int j = 0; j < m_pMain->vt_job_info[i].num_of_camera; j++)
			{
				nAlgo[i][j] = m_pMain->vt_job_info[i].camera_index[j];
			}
		}
		
		int w = rt.Width() / 9;

		m_listInspection[cul].InsertColumn(0, "Time", LVCFMT_CENTER, w);
		m_listInspection[cul].InsertColumn(1, "ID", LVCFMT_CENTER, w);
		//m_listInspection.InsertColumn(1, "Model", LVCFMT_CENTER, 80);
		//m_listInspection[cul].InsertColumn(1, "Unit", LVCFMT_CENTER, 150);
		m_listInspection[cul].InsertColumn(2, "X", LVCFMT_CENTER, w);
		m_listInspection[cul].InsertColumn(3, "Y", LVCFMT_CENTER, w);
		m_listInspection[cul].InsertColumn(4, "T", LVCFMT_CENTER, w);
		m_listInspection[cul].InsertColumn(5, "Top Length", LVCFMT_CENTER, w);
		m_listInspection[cul].InsertColumn(6, "Bottom Lenght", LVCFMT_CENTER, w);
		m_listInspection[cul].InsertColumn(7, "Left Length", LVCFMT_CENTER, w);
		m_listInspection[cul].InsertColumn(8, "Right Length", LVCFMT_CENTER, w);
	}
	break;
	case CLIENT_TYPE_ASSEMBLE_INSP:
	{
		for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
		{
			/*nCamcnt = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraNum(i);
			nAlgoNum = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(i);*/

			for (int j = 0; j < m_pMain->vt_job_info[i].num_of_camera; j++)
			{
				nAlgo[i][j] = m_pMain->vt_job_info[i].camera_index[j];
			}
		}

		int w = rt.Width() / 7;

		m_listInspection[cul].InsertColumn(0, "Time", LVCFMT_CENTER, w);
		m_listInspection[cul].InsertColumn(1, "ID", LVCFMT_CENTER, w);
		//m_listInspection[cul].InsertColumn(1, "Unit", LVCFMT_CENTER, 150);
		m_listInspection[cul].InsertColumn(2, "Cam", LVCFMT_CENTER, w);
		m_listInspection[cul].InsertColumn(3, "X1", LVCFMT_CENTER, w);
		m_listInspection[cul].InsertColumn(4, "X2", LVCFMT_CENTER, w);
		m_listInspection[cul].InsertColumn(5, "Y1", LVCFMT_CENTER, w);
		m_listInspection[cul].InsertColumn(6, "Y2", LVCFMT_CENTER, w);
	}
	break;
	case CLIENT_TYPE_ELB_CENTER_ALIGN:
	{
		int w = rt.Width() / 6;
		int colum = 0;

		m_listInspection[cul].InsertColumn(colum, "Time", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "ID", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "JUDGE", LVCFMT_CENTER, w); colum++;
	//	m_listInspection[cul].InsertColumn(1, "Unit", LVCFMT_CENTER, 150);
		m_listInspection[cul].InsertColumn(colum, "X", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "Y", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "T", LVCFMT_CENTER, w); 
	}
	break;
	case CLIENT_TYPE_1CAM_4POS_ROBOT:
	case CLIENT_TYPE_1CAM_2POS_REFERENCE:
	{
		int w = rt.Width() / 7;
		int colum = 0;

		m_listInspection[cul].InsertColumn(colum, "Time", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "ID", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "JUDGE", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "Object", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "X", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "Y", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "T", LVCFMT_CENTER, w);
	}
	break;
	case CLIENT_TYPE_CENTER_NOZZLE_ALIGN:
	{
		int w = rt.Width() / 7;
		int colum = 0;
		m_listInspection[cul].InsertColumn(colum, "Time", LVCFMT_CENTER, w); colum++;
		//HSJ 2022-01-09 결과창에 ID표시 추가
		m_listInspection[cul].InsertColumn(colum, "ID", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "JUDGE", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "X", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "Y", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "Z", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "T", LVCFMT_CENTER, w);
	}
	break;
	case CLIENT_TYPE_FILM_INSP:
	{
		int w = rt.Width() / 9;
		int colum = 0;

		m_listInspection[cul].InsertColumn(colum, "Time", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "ID", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "JUDGE", LVCFMT_CENTER, w); colum++;
		//m_listInspection[cul].InsertColumn(colum, "Object", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "LX", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "LY", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "RX", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "RY", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "CLX", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "CRX", LVCFMT_CENTER, w);
	}
	break;
	case CLIENT_TYPE_CENTER_SIDE_YGAP: // PJH
	{
		int w = rt.Width() / 5;
		int colum = 0;

		m_listInspection[cul].InsertColumn(colum, "Time", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "JUDGE", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "Z GAP", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "Z GAP_Ori", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "Recalc", LVCFMT_CENTER, w);
	}
	break;
	default:
	{
		int w = rt.Width() / 6;
		int  colum = 0;

		m_listInspection[cul].InsertColumn(colum, "Time", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "ID", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "JUDGE", LVCFMT_CENTER, w); colum++;
		//m_listInspection[cul].InsertColumn(1, "Unit", LVCFMT_CENTER, 150);
		m_listInspection[cul].InsertColumn(colum, "X", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "Y", LVCFMT_CENTER, w); colum++;
		m_listInspection[cul].InsertColumn(colum, "T", LVCFMT_CENTER, w);
	}
	break;
	}

	m_bResultInit[cul] = TRUE;
}

//Tab변경시 결과창 갱신
void CPaneAuto::RenewResultChart(int nIndex)
{
	int nAlgo = m_pMain->vt_job_info[nIndex].algo_method;

	if (m_bResultInit[nIndex] != TRUE)
		CreateResultInit(nIndex, nAlgo);

	for (int i = 0; i < MAX_JOB; i++)
		m_listInspection[i].ShowWindow(FALSE);

	m_listInspection[nIndex].ShowWindow(TRUE);
}

//Tab변경시 스펙창 갱신
void CPaneAuto::RenewSpecChart(int nIndex)
{
	SaveRefDataCSV();

	int nMetho = m_pMain->vt_job_info[0].algo_method;
	if (nMetho != CLIENT_TYPE_SCAN_INSP)
	{
		if (m_bInsp) // hsj 2022-01-11 inspection이 있을때 없을때,
		{
			switch (nIndex)
			{
			case 0:
				m_grid_spec_viewer.ShowWindow(TRUE);
				m_grid_insp_spec_viewer.ShowWindow(FALSE);
				// YCS 2022-08-22 
				if (m_pMain->m_pCInspChartDlg[0] != nullptr) m_pMain->m_pCInspChartDlg[0]->ShowWindow(SW_HIDE);
				if (m_pMain->m_pCInspChartDlg[1] != nullptr) m_pMain->m_pCInspChartDlg[1]->ShowWindow(SW_HIDE);
				break;
			case 1:
				m_grid_spec_viewer.ShowWindow(FALSE);
				m_grid_insp_spec_viewer.ShowWindow(TRUE);
				if (m_pMain->m_pCInspChartDlg[0] != nullptr) m_pMain->m_pCInspChartDlg[0]->ShowWindow(SW_HIDE);
				if (m_pMain->m_pCInspChartDlg[1] != nullptr) m_pMain->m_pCInspChartDlg[1]->ShowWindow(SW_HIDE);
				break;
			case 2:
				m_grid_spec_viewer.ShowWindow(FALSE);
				m_grid_insp_spec_viewer.ShowWindow(FALSE);
				if (m_pMain->m_pCInspChartDlg[0] != nullptr)
				{
					m_pMain->m_pCInspChartDlg[0]->ShowWindow(SW_SHOW);
					// YCS 2022-09-06 Recent와 DB 차트가 겹쳐 보이는 현상 때문에 추가. 
					::PostMessage(m_pMain->m_pCInspChartDlg[0]->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_DLG_REFRESH, 0);
				}
				if (m_pMain->m_pCInspChartDlg[1] != nullptr) m_pMain->m_pCInspChartDlg[1]->ShowWindow(SW_HIDE);
				break;
			case 3:
				m_grid_spec_viewer.ShowWindow(FALSE);
				m_grid_insp_spec_viewer.ShowWindow(FALSE);
				if (m_pMain->m_pCInspChartDlg[0] != nullptr) m_pMain->m_pCInspChartDlg[0]->ShowWindow(SW_HIDE);
				if (m_pMain->m_pCInspChartDlg[1] != nullptr)
				{
					m_pMain->m_pCInspChartDlg[1]->ShowWindow(SW_SHOW);
					// YCS 2022-09-06 Recent와 DB 차트가 겹쳐 보이는 현상 때문에 추가. 
					::PostMessage(m_pMain->m_pCInspChartDlg[1]->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_DLG_REFRESH, 1);
				}
				break;
			}
		}
		else
		{
			switch (nIndex)
			{
			case 0:
				if (m_hPE) PEdestroy(m_hPE);
				m_grid_ref_data_viewer.ShowWindow(FALSE);
				m_grid_proc_time_viewer.ShowWindow(FALSE);
				m_grid_spec_viewer.ShowWindow(TRUE);
				break;
			case 1:
				m_grid_spec_viewer.ShowWindow(FALSE);
				m_grid_ref_data_viewer.ShowWindow(FALSE);
				m_grid_proc_time_viewer.ShowWindow(FALSE);
				CreateSimpleGraph();
				//GraphBackgroundBitmap();
				break;
			case 2:
				if (m_hPE) PEdestroy(m_hPE);
				m_grid_spec_viewer.ShowWindow(FALSE);
				m_grid_proc_time_viewer.ShowWindow(FALSE);
				m_grid_ref_data_viewer.ShowWindow(TRUE);
				break;
			case 3:
				if (m_hPE) PEdestroy(m_hPE);
				m_grid_spec_viewer.ShowWindow(FALSE);
				m_grid_ref_data_viewer.ShowWindow(FALSE);
				m_grid_proc_time_viewer.ShowWindow(TRUE);
				break;
			}
		}
	}
	else
	{
		switch (nIndex)
		{
		case 0:
			if (m_hPE) PEdestroy(m_hPE);
			m_grid_spec_viewer.ShowWindow(TRUE);
			m_grid_InspResult_1_viewer.ShowWindow(FALSE);
			m_grid_InspResult_2_viewer.ShowWindow(FALSE);
			m_grid_InspResult_3_viewer.ShowWindow(FALSE);
			break;
		case 1:
			if (m_hPE) PEdestroy(m_hPE);
			m_grid_spec_viewer.ShowWindow(FALSE);
			m_grid_InspResult_1_viewer.ShowWindow(TRUE);
			m_grid_InspResult_2_viewer.ShowWindow(FALSE);
			m_grid_InspResult_3_viewer.ShowWindow(FALSE);
			break;
		case 2:
			if (m_hPE) PEdestroy(m_hPE);
			m_grid_spec_viewer.ShowWindow(FALSE);
			m_grid_InspResult_1_viewer.ShowWindow(FALSE);
			m_grid_InspResult_2_viewer.ShowWindow(TRUE);
			m_grid_InspResult_3_viewer.ShowWindow(FALSE);
			break;
		case 3:
			if (m_hPE) PEdestroy(m_hPE);
			m_grid_spec_viewer.ShowWindow(FALSE);
			m_grid_InspResult_1_viewer.ShowWindow(FALSE);
			m_grid_InspResult_2_viewer.ShowWindow(FALSE);
			m_grid_InspResult_3_viewer.ShowWindow(TRUE);
			break;
		}
	}
}

void CPaneAuto::OnTimer(UINT_PTR nIDEvent)
{
	ULONG64 ltotal, lfree;
	int total=1, free=1;
	int usage=0;

	m_cPcPerformance.GetDiskSpace("C:\\", total, free);
	m_pCDriveProgressCtrl.SetPos(100 -  (int(free / double(total) * 100)));
	m_cPcPerformance.GetDiskSpace("D:\\", total, free);
	m_pDDriveProgressCtrl.SetPos(100 - (int(free / double(total) * 100)));
	m_cPcPerformance.GetMemoryUsage(ltotal, lfree);
	m_pMemoryProgressCtrl.SetPos(lround((1.- (lfree / double(ltotal))) * 1000));

	usage = m_cPcCpuPerformance.GetUsage();
	m_pCpuProgressCtrl.SetPos(usage);

	CFormView::OnTimer(nIDEvent);
}



BOOL CPaneAuto::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (wParam == (WPARAM)m_grid_spec_viewer.GetDlgCtrlID())
	{
		int nMetho = m_pMain->vt_job_info[0].algo_method;
		if (nMetho == CLIENT_TYPE_ELB_CENTER_ALIGN)
		{
			GV_DISPINFO* pDispInfo = (GV_DISPINFO*)lParam;
			if (4294967196 == pDispInfo->hdr.code)
			{
				int row = pDispInfo->item.row;
				int col = pDispInfo->item.col;
				
				if (col == 0 && row > 1)
				{
					CString PanelID = m_grid_spec_viewer.GetItemText(row, col);
					if (PanelID == "" || PanelID.IsEmpty() || PanelID == "-") return false;

					SYSTEMTIME time;
					::GetLocalTime(&time);
					CString strDate;

					CString strImagePath;
					strImagePath.Format("%s%04d%02d%02d\\%s\\%s\\Diff_Insp\\", m_pMain->m_strImageDir, time.wYear, time.wMonth, time.wDay,
						m_pMain->vt_job_info[0].model_info.getModelID(), m_pMain->vt_job_info[0].job_name.c_str());

					if (PathFileExists(strImagePath) == FALSE) 
					{
						if (find_center_image_panel_directory(strImagePath, PanelID, &strImagePath, TRUE) == TRUE)
						{
							::ShellExecute(NULL, NULL, "explorer.exe", strDate, NULL, SW_SHOWNORMAL);
							theApp.m_pFrame->ShowWindow(SW_MINIMIZE);
						}
					}
					else
					{
						::ShellExecute(NULL, NULL, "explorer.exe", strDate, NULL, SW_SHOWNORMAL);
						theApp.m_pFrame->ShowWindow(SW_MINIMIZE);
					}

				}
				else if (col == 1 && row > 1) // 2022.06.14 동영상 OPEN
				{
					//CString PanelID = m_grid_spec_viewer.GetItemText(row, col);
					//if (PanelID == "" || PanelID.IsEmpty() || PanelID == "-") return false;

					//SYSTEMTIME time;
					//::GetLocalTime(&time);
					//CString strDate;

					//// D:\LET_AlignClient\Result\Video\날짜\PanelID
					//CString strVideoDir;
					//strDate.Format("%04d%02d%02d", time.wYear, time.wMonth, time.wDay);
					//strVideoDir.Format("%s%s\\%s\\", m_pMain->m_strVideoDir, strDate, PanelID);

					////파일명 찾기
					//if (PathFileExists(strVideoDir) == FALSE) //찾는 비디오가 오늘 날짜 폴더에 없는경우
					//{ 
					//	if(find_center_video_panel_directory(strVideoDir, PanelID, &strVideoDir) == TRUE)
					//	{
					//		return find_center_video_file(strVideoDir);
					//	}
					//}
					//else //찾는 비디오가 오늘 날짜인 경우
					//{
					//	return find_center_video_file(strVideoDir);
					//}

					//CString str;
					//str.Format("[%s] video is not find", PanelID);
					//AfxMessageBox(str);
				}
			}
		}
	}
	return CFormView::OnNotify(wParam, lParam, pResult);
}

void CPaneAuto::OnBnClickedBtnAutoTrend()
{
	m_pMain->changeForm(FORM_TREND);
}

BOOL CPaneAuto::find_center_video_file(CString strVideoDir)
{
	CString fname;
	BOOL bFind = FALSE;
	CFileFind findVideo;

	bFind = findVideo.FindFile(strVideoDir + _T("\\*.avi"));

	try {
		while (bFind)
		{
			bFind = findVideo.FindNextFile();
			fname = findVideo.GetFilePath();	// 찾은 Video

			CString str;
			str.Format("CENTER");
			if (fname.Find(str) > 0)
			{
				SHELLEXECUTEINFO sei = { sizeof(sei) };
				sei.lpVerb = "open";
				sei.lpFile = fname;
				sei.hwnd = NULL;
				sei.lpParameters = NULL;
				sei.nShow = SW_NORMAL;

				if (!ShellExecuteEx(&sei))
				{
					DWORD dwError = GetLastError();
					if (dwError == ERROR_CANCELLED)					
						return FALSE;
				}
				else
				{
					return FALSE;
				}
			}
		}
	}
	catch (...)
	{
		CString str;
		str.Format("Did catch the error to get_Viedo_path");
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		str.Format("%s", strVideoDir);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}

	if (bFind == FALSE)
	{
		CString str;
		str.Format("Video is not Find!!");
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		str.Format("%s", strVideoDir);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
	return bFind;
}

BOOL CPaneAuto::find_center_video_panel_directory(CString UpDir, CString strPanelID, CString *strPanelDir)
{
	CString strPanelFolder;

	// 날짜 폴더 찾기
	CFileFind find_date;
	CString strDateFloder;
	BOOL bFindDateFloder = FALSE;

	bFindDateFloder = find_date.FindFile(m_pMain->m_strVideoDir + _T("\\*.*"));

	while (bFindDateFloder)
	{
		bFindDateFloder = find_date.FindNextFile();

		if (find_date.IsDots() || !find_date.IsDirectory())	// '.', '..', '파일' 인 경우 continue;
			continue;

		strDateFloder = find_date.GetFilePath();

		//날짜에 PANELID 폴더 찾기
		CFileFind find_panel_folder;
		CString strPanelFloder;
		BOOL bFindPanelFloder = FALSE;

		bFindPanelFloder = find_panel_folder.FindFile(strDateFloder + _T("\\*.*"));

		while (bFindPanelFloder)
		{
			bFindPanelFloder = find_panel_folder.FindNextFile();

			if (find_panel_folder.IsDots() || !find_panel_folder.IsDirectory())	// '.', '..', '파일' 인 경우 continue;
				continue;
			
			strPanelFloder = find_panel_folder.GetFilePath();

			if (strPanelDir->Find(strPanelID) > 0)
			{
				strPanelDir = &strPanelFloder;
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CPaneAuto::find_center_image_panel_directory(CString UpDir, CString strPanelID, CString* strPanelDir, BOOL bJudge)
{
	CString strPanelFolder;

	// 날짜 폴더 찾기
	CFileFind find_date;
	CString strDateFloder;
	BOOL bFindDateFloder = FALSE;

	bFindDateFloder = find_date.FindFile(m_pMain->m_strVideoDir + _T("\\*.*"));

	while (bFindDateFloder)
	{
		bFindDateFloder = find_date.FindNextFile();

		if (find_date.IsDots() || !find_date.IsDirectory())	// '.', '..', '파일' 인 경우 continue;
			continue;

		strDateFloder = find_date.GetFilePath();

		UpDir.Format("%s\\%s\\Diff_Insp\\", strDateFloder, m_pMain->vt_job_info[0].model_info.getModelID(), m_pMain->vt_job_info[0].job_name.c_str());

		if(bJudge == TRUE)  UpDir.Format("%s\\%s\\Diff_Insp\\OK\\", strDateFloder, m_pMain->vt_job_info[0].model_info.getModelID(), m_pMain->vt_job_info[0].job_name.c_str());
		if(bJudge == FALSE) UpDir.Format("%s\\%s\\Diff_Insp\\NG\\", strDateFloder, m_pMain->vt_job_info[0].model_info.getModelID(), m_pMain->vt_job_info[0].job_name.c_str());

		//날짜에 PANELID 폴더 찾기
		CFileFind find_panel_folder;
		CString strPanelFloder;
		BOOL bFindPanelFloder = FALSE;

		bFindPanelFloder = find_panel_folder.FindFile(UpDir + _T("\\*.*"));

		while (bFindPanelFloder)
		{
			bFindPanelFloder = find_panel_folder.FindNextFile();

			if (find_panel_folder.IsDots() || !find_panel_folder.IsDirectory())	// '.', '..', '파일' 인 경우 continue;
				continue;

			strPanelFloder = find_panel_folder.GetFilePath();

			if (strPanelDir->Find(strPanelID) > 0)
			{
				strPanelDir = &strPanelFloder;
				return TRUE;
			}
		}
	}

	return FALSE;
}
// KBJ 2022-07-02 Result Image & Video List 기능 추가
void CPaneAuto::OnBnClickedBtnAutoResultList()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	//if (m_pMain->vt_job_info[0].algo_method != CLIENT_TYPE_ELB_CENTER_ALIGN)
	//	m_pMain->m_pListDlg->ShowWindow(SW_HIDE);

	if (m_pMain->m_pListDlg->IsWindowVisible())
	{
		m_pMain->m_pListDlg->ShowWindow(SW_HIDE);
	}
	else
	{
		
		if (m_pMain->vt_job_info[0].algo_method == CLIENT_TYPE_ELB_CENTER_ALIGN)
		{
			m_pMain->m_pListDlg->update_list_centeralign();
			m_pMain->m_pListDlg->ShowWindow(SW_SHOW);
		}
		else
		{
			m_pMain->m_pSelectDlg->ShowWindow(SW_SHOW);	//2022.07.05 ksm Job Select 후 List 보이기
		}
	}
}
