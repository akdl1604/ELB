#include "stdafx.h"
#include "LET_AlignClient.h"
#include "PaneSimulation.h"
#include "LET_AlignClientDlg.h"
#include "CommPLC.h"

IMPLEMENT_DYNCREATE(CPaneSimulation, CFormView)

CPaneSimulation::CPaneSimulation()
	: CFormView(CPaneSimulation::IDD)
{
	m_bSimulStopFlag = TRUE;
	m_bUseSimulMessage = FALSE;
	m_nJob = 0;
	m_bUseLive = FALSE;
	m_nCurrent_list = 1;
}

CPaneSimulation::~CPaneSimulation()
{
	m_vtDirPath.clear();
	m_vtSelectList.clear();
}

#ifdef _DEBUG
void CPaneSimulation::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPaneSimulation::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

void CPaneSimulation::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GRID_SIMULATION_LIST, m_grid_list);
	DDX_Control(pDX, IDC_LB_SELECT_JOB, m_sttJob);
	DDX_Control(pDX, IDC_BTN_LIVE_SIMULATION, m_btnLive);
	DDX_Control(pDX, IDC_BTN_APPEND_LIST, m_btnAppendList);
	DDX_Control(pDX, IDC_BTN_DELETE_LIST, m_btnDeleteList);
	DDX_Control(pDX, IDC_BTN_SIMULATION_START, m_btnSimulationStart);
	DDX_Control(pDX, IDC_CB_SELECT_JOB, m_cmbSelectJob);
	DDX_Control(pDX, IDC_BTN_REVERSE_SELECT, m_btnReverseSelect);
}


void CPaneSimulation::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);
	m_pMain = (CLET_AlignClientDlg *)AfxGetMainWnd();
	GetClientRect(&m_rcForm);

	m_bUseLive = FALSE;

	init_grid_list(m_grid_list, 2, 1000);
	InitTitle(&m_sttJob, "Select Job", 14.f, COLOR_DDARK_GRAY);
	MainButtonInit(&m_btnLive);
	MainButtonInit(&m_btnAppendList);
	MainButtonInit(&m_btnReverseSelect);
	MainButtonInit(&m_btnDeleteList);
	MainButtonInit(&m_btnSimulationStart);
	
	for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
	{
		CString str;
		str.Format("%s", m_pMain->vt_job_info[i].get_job_name());
		m_cmbSelectJob.AddString(str);
	}

	update_frame_dialog();

}

HBRUSH CPaneSimulation::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (pWnd->GetDlgCtrlID() == IDC_CB_SELECT_JOB)
	{
		pDC->SetBkColor(COLOR_UI_BODY);
		pDC->SetTextColor(COLOR_WHITE);
	}
	return m_hbrBkg;
}


BEGIN_MESSAGE_MAP(CPaneSimulation, CFormView)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BTN_LIVE_SIMULATION, &CPaneSimulation::OnBnClickedBtnLiveSimulation)
	ON_BN_CLICKED(IDC_BTN_APPEND_LIST, &CPaneSimulation::OnBnClickedBtnAppendList)
	ON_BN_CLICKED(IDC_BTN_DELETE_LIST, &CPaneSimulation::OnBnClickedBtnDeleteList)
	ON_CBN_SELCHANGE(IDC_CB_SELECT_JOB, &CPaneSimulation::OnSelchangeComboJob)
	ON_BN_CLICKED(IDC_BTN_SIMULATION_START, &CPaneSimulation::OnBnClickedBtnSimulationStart)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_REVERSE_SELECT, &CPaneSimulation::OnBnClickedBtnReverseSelcet)
END_MESSAGE_MAP()

void CPaneSimulation::InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}
void CPaneSimulation::MainButtonInit(CButtonEx *pbutton, int nID, int size)
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
void CPaneSimulation::init_grid_list(CGridCtrl &grid, int col, int row)
{
	BOOL bVirtualMode = FALSE;

	grid.SetEditable(FALSE);
	grid.SetEditable(TRUE);
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
	grid.SetRowCount(row);
	grid.SetColumnCount(col);
	grid.SetBkColor(RGB(197, 197, 187));
	grid.SetTextBkColor(RGB(235, 235, 235));

	for (int i = 0; i < grid.GetColumnCount(); i++)
	{
		grid.SetItemBkColour(0, i, RGB(146, 146, 141));
		grid.SetItemFgColour(0, i, RGB(255, 255, 255));

		for (int row = 0; row < grid.GetRowCount(); row++)
		{
			UINT state = grid.GetItemState(row, i) | GVIS_READONLY;
			grid.SetItemState(row, i, state);
		}
	}

	for (int row = 1; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{
			grid.SetItemBkColour(row, col, COLOR_BTN_BODY);
			grid.SetItemFgColour(row, col, COLOR_WHITE);
		}
	}

	CRect rect;
	grid.GetWindowRect(&rect);

	int grid_width;
	grid_width = (int)(rect.Width() / (double)col);

	//Grid 행 사이즈 조정
	for (int nCol = 0; nCol < grid.GetColumnCount(); nCol++)
	{
		switch (nCol)
		{
		case 0:		grid.SetColumnWidth(nCol, 65);									break;
		case 1:		grid.SetColumnWidth(nCol, rect.Width() - 75);					break;
		default:	grid.SetColumnWidth(nCol, grid_width);							break;
		}
	}

	//Grid 열 사이즈 조정
	for (int nRow = 0; nRow < grid.GetRowCount(); nRow++)
		grid.SetRowHeight(nRow, 30);
}

void CPaneSimulation::OnPaint()
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

void CPaneSimulation::simulation_1cam_1shot_align(int nJob)
{
	int nExpectedCount = 1;
	BOOL bFind = FALSE;

	std::vector<CString> vtImagePath;
	CString str, strTemp, strBeforeID, strNextID;

	BOOL bExit = FALSE;
	int camCount = 0;
	camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;

	// 이미지경로 저장하는 벡터 할당
	vtImagePath.assign(camCount, NULL);

	BOOL simulation_stop = FALSE;
#pragma region 폴더 갯수 만큼 시뮬레이션
	for (int nFilePathCount = 0; nFilePathCount < m_vtDirPath.size(); nFilePathCount++)
	{
		if (m_vtSelectList[nFilePathCount] != TRUE) continue;

		m_nCurrent_list = current_list(nFilePathCount);

		str.Format("[%s](%d/%d)Simulation Start ", m_pMain->vt_job_info[nJob].job_name.c_str(), nFilePathCount + 1, m_vtDirPath.size());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		// 매 패널마다 Image Path 리셋.
		for (int i = 0; i < vtImagePath.size(); i++)
			vtImagePath[i].Empty();

		// 이미지경로를 저장한다
		int nMethod = m_pMain->vt_job_info[0].algo_method;
		if(nMethod == CLIENT_TYPE_ELB_CENTER_ALIGN)				bFind = get_Image_path_elb_center_align(nJob, m_vtDirPath[nFilePathCount], vtImagePath, nExpectedCount);
		else if(nMethod == CLIENT_TYPE_ELB_CENTER_ALIGN)		bFind = get_Image_path_elb_center_align(nJob, m_vtDirPath[nFilePathCount], vtImagePath, nExpectedCount);
		else													bFind = get_Image_path_1cam_1shot(nJob, m_vtDirPath[nFilePathCount], vtImagePath, nExpectedCount);
		if (m_bSimulStopFlag == TRUE) break;
		if (!bFind) continue;

		_stPutinProBuffer pInfo[MAX_CAMERA][NUM_POS];
		// 이미지를 버퍼에 저장한다

		for (int nCam = 0; nCam < camCount; nCam++)
		{
			int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
			pInfo[real_cam][0].nCam = nCam;
			pInfo[real_cam][0].nPos = 0;
			pInfo[real_cam][0].nJob = nJob;
			pInfo[real_cam][0].strPath = vtImagePath[nCam];
			pInfo[real_cam][0].pView = this;

			m_pMain->m_bSimulGrabEnd[real_cam][0] = FALSE;
			AfxBeginThread(Thread_put_image_in_buffer, &pInfo[real_cam][0]);
		}

		// 버퍼에 저장된 이미지 완료 체크.. 10초 이후는 그냥 끝냄
		BOOL all_image_read = TRUE;
		int real_cam = m_pMain->vt_job_info[nJob].camera_index[0];
		if (m_pMain->wait_flag_over_time(10000, m_pMain->m_bSimulGrabEnd[0], FALSE, 0, 1, NUM_POS, real_cam, camCount) == -1)
		{
			str.Format("TimeOut to Wait Image Read End.");
			::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

			all_image_read = FALSE;
			continue;
		}

		////Procsss Start
		get_ID_from_loadimage(nJob, m_vtDirPath[nFilePathCount], &strBeforeID);
		m_pMain->vt_job_info[nJob].main_object_id = strBeforeID;
		m_pMain->m_nSeqProcess[nJob] = 1000;
		m_pMain->setProcessStartFlag(nJob);

		// kbj 2021-12-27 Process 끝날때 까지 대기.. 10초 이후에는 그냥 끝냄
		if (m_pMain->wait_flag_over_time(10000, &m_pMain->m_bPrealignStartFlag[nJob], TRUE, 0, 1) == -1)
		{
			m_pMain->m_nSeqProcess[nJob] = 0;
			m_pMain->resetProcessStartFlag(nJob);

			str.Format("TimeOut to Wait ProcessFlag Down.");
			::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			break;

		}
		Delay(1000);

		str.Format("[%s](%d/%d)Simulation End ", m_pMain->vt_job_info[nJob].job_name.c_str(), nFilePathCount + 1, m_vtDirPath.size());
		::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		// Stop 누를시 Break
		if (m_bSimulStopFlag == TRUE) break;

		// 시뮬레이션 메세지
		if (m_bUseSimulMessage == TRUE || m_pMain->m_bProcessResult[nJob] != TRUE)
		{
			if (nFilePathCount + 1 == m_vtDirPath.size()) break;

			CString strTemp;
			str.Format("Process %s\nWill you next simulation? (%d/%d)", m_pMain->m_bProcessResult[nJob] == TRUE ? "OK" : "NG", nFilePathCount + 2, m_vtDirPath.size());
			if (nFilePathCount == m_vtDirPath.size() - 1)  strNextID.Format("Empty");
			else get_ID_from_loadimage(nJob, m_vtDirPath[nFilePathCount + 1], &strNextID);
			strTemp.Format(" Next ID : %s\n Before ID : %s", strNextID, strBeforeID);

			if (SimulationMesaage(2, str, strTemp) == 0)
				break;
		}
	}
	vtImagePath.clear();
	camBuf.clear();
}
BOOL CPaneSimulation::get_Image_path_1cam_1shot(int nJob, CString strFilePath, std::vector<CString>& vtImagePath, int nExpectedCount)
{
	CStringArray FileArray;
	CString str, strImagePath;
	int nFind = FALSE;
	int find_count = 0;

	try {
		BOOL bFind = FALSE;
		CFileFind findImage;

		if (PathFileExists(strFilePath) == FALSE)
		{
			m_pMain->fnSetMessage(1, _T("Check Directory !!!"));
			return FALSE;
		}

		bFind = findImage.FindFile(strFilePath + _T("\\*_ImgRaw.jpg"));

		while (bFind)
		{
			int nCount = 0;

			bFind = findImage.FindNextFile();
			strImagePath = findImage.GetFilePath();	// 찾은 Image

			for (int nCam = 0; nCam < m_pMain->vt_job_info[nJob].camera_index.size(); nCam++)
			{
				int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

				str.Format("%s_ImgRaw", m_pMain->m_stCamInfo[real_cam].cName);
				if (strImagePath.Find(str) > 0)
				{
					vtImagePath[nCam] = strImagePath;
					find_count++;
				}
			}
		}
	}
	catch (...)
	{
		str.Format("Did catch the error to get_Image_path");
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		str.Format("%s", strFilePath);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}

	if (nExpectedCount != 0 && find_count != nExpectedCount)
	{
		nFind = FALSE;
		str.Format("[%s] Fail to get Image path", m_pMain->vt_job_info[nJob].job_name.c_str());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_pMain->m_pDlgMessage->setMessageModaless(2, str, strFilePath);
		m_pMain->m_pDlgMessage->ShowWindow(TRUE);
		MSG message;
		while (m_pMain->m_pDlgMessage->getResult() == -1)
		{
			if (::PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
			{
				::TranslateMessage(&message);
				::DispatchMessage(&message);
			}
		}
		if (m_pMain->m_pDlgMessage->getResult() == 0)
		{
			m_bSimulStopFlag = TRUE;
		}
	}
	else nFind = TRUE;

	return nFind;
}

BOOL CPaneSimulation::get_Image_path_elb_center_align(int nJob, CString strFilePath, std::vector<CString>& vtImagePath, int nExpectedCount)
{
	CStringArray FileArray;
	CString str, strImagePath;
	int nFind = FALSE;
	int find_count = 0;

	try {
		BOOL bFind = FALSE;
		CFileFind findImage;

		if (PathFileExists(strFilePath) == FALSE)
		{
			m_pMain->fnSetMessage(1, _T("Check Directory !!!"));
			return FALSE;
		}

		bFind = findImage.FindFile(strFilePath + _T("\\*_ImgRaw.jpg"));

		while (bFind)
		{
			int nCount = 0;

			bFind = findImage.FindNextFile();
			strImagePath = findImage.GetFilePath();	// 찾은 Image

			for (int nCam = 0; nCam < m_pMain->vt_job_info[nJob].camera_index.size(); nCam++)
			{
				int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

				str.Format("_ImgRaw", m_pMain->m_stCamInfo[real_cam].cName);
				if (strImagePath.Find(str) > 0)
				{
					vtImagePath[nCam] = strImagePath;
					find_count++;
				}
			}
		}
	}
	catch (...)
	{
		str.Format("Did catch the error to get_Image_path");
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		str.Format("%s", strFilePath);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}

	if (find_count != nExpectedCount)
	{
		nFind = FALSE;
		str.Format("[%s] Fail to get Image path", m_pMain->vt_job_info[nJob].job_name.c_str());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_pMain->m_pDlgMessage->setMessageModaless(2, str, strFilePath);
		m_pMain->m_pDlgMessage->ShowWindow(TRUE);
		MSG message;
		while (m_pMain->m_pDlgMessage->getResult() == -1)
		{
			if (::PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
			{
				::TranslateMessage(&message);
				::DispatchMessage(&message);
			}
		}
		if (m_pMain->m_pDlgMessage->getResult() == 0)
		{
			m_bSimulStopFlag = TRUE;
		}
	}
	else nFind = TRUE;

	return nFind;
}

void CPaneSimulation::simulation_2cam_1shot_align(int nJob)
{
	int nExpectedCount = 2;
	BOOL bFind = FALSE;

	std::vector<CString> vtImagePath;
	CString str, strTemp, strBeforeID, strNextID;

	BOOL bExit = FALSE;
	int camCount = 0;
	camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;

	// 이미지경로 저장하는 벡터 할당
	vtImagePath.assign(camCount, NULL);

	BOOL simulation_stop = FALSE;
#pragma region 폴더 갯수 만큼 시뮬레이션
	for (int nFilePathCount = 0; nFilePathCount < m_vtDirPath.size(); nFilePathCount++)
	{
		if (m_vtSelectList[nFilePathCount] != TRUE) continue;

		m_nCurrent_list = current_list(nFilePathCount);

		CString str;
		str.Format("[%s](%d/%d)Simulation Start ", m_pMain->vt_job_info[nJob].job_name.c_str(), nFilePathCount + 1, m_vtDirPath.size());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		// 매 패널마다 Image Path 리셋.
		for (int i = 0; i < vtImagePath.size(); i++)
			vtImagePath[i].Empty();

		// 이미지경로를 저장한다
		bFind = get_Image_path_2cam_1shot_align(nJob, m_vtDirPath[nFilePathCount], vtImagePath, nExpectedCount);
		if (m_bSimulStopFlag == TRUE) break;
		if (!bFind) continue;

		_stPutinProBuffer pInfo[MAX_CAMERA][NUM_POS];
		// 이미지를 버퍼에 저장한다

		for (int nCam = 0; nCam < camCount; nCam++)
		{
			int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
			pInfo[real_cam][0].nCam = nCam;
			pInfo[real_cam][0].nPos = 0;
			pInfo[real_cam][0].nJob = nJob;
			pInfo[real_cam][0].strPath = vtImagePath[nCam];
			pInfo[real_cam][0].pView = this;

			m_pMain->m_bSimulGrabEnd[real_cam][0] = FALSE;
			AfxBeginThread(Thread_put_image_in_buffer, &pInfo[real_cam][0]);
		}

		// 버퍼에 저장된 이미지 완료 체크.. 10초 이후는 그냥 끝냄
		BOOL all_image_read = TRUE;
		int real_cam = m_pMain->vt_job_info[nJob].camera_index[0];
		if (m_pMain->wait_flag_over_time(10000, m_pMain->m_bSimulGrabEnd[0], FALSE, 0, 1, NUM_POS, real_cam, camCount) == -1)
		{
			CString str;
			str.Format("TimeOut to Wait Image Read End.");
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

			all_image_read = FALSE;
			continue;
		}

		////Procsss Start
		get_ID_from_loadimage(nJob, m_vtDirPath[nFilePathCount], &strBeforeID);
		m_pMain->vt_job_info[nJob].main_object_id = strBeforeID;
		::SendMessageA(m_pMain->m_pForm[FORM_MAIN]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_GLASSID_UPDATE, nJob);
		m_pMain->m_nSeqProcess[nJob] = 1000;
		m_pMain->setProcessStartFlag(nJob);
		Delay(50);

		// Process 끝날때 까지 대기.. 10초 이후에는 그냥 끝냄
		if (m_pMain->wait_flag_over_time(10000, &m_pMain->m_bPrealignStartFlag[nJob], TRUE, 0, 1) == -1)
		{
			m_pMain->m_nSeqProcess[nJob] = 0;
			m_pMain->resetProcessStartFlag(nJob);

			CString str;
			str.Format("TimeOut to Wait ProcessFlag Down.");
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			break;
		}
		Delay(50);

		str.Format("[%s](%d/%d)Simulation End ", m_pMain->vt_job_info[nJob].job_name.c_str(), nFilePathCount + 1, m_vtDirPath.size());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		// Stop 누를시 Break
		if (m_bSimulStopFlag == TRUE) break;

		// 시뮬레이션 메세지
		if (m_bUseSimulMessage == TRUE || m_pMain->m_bProcessResult[nJob] != TRUE)
		{
			if (nFilePathCount + 1 == m_vtDirPath.size()) break;

			CString strTemp;
			str.Format("Process %s\nWill you next simulation? (%d/%d)", m_pMain->m_bProcessResult[nJob] == TRUE ? "OK" : "NG", nFilePathCount + 2, m_vtDirPath.size());
			if (nFilePathCount == m_vtDirPath.size() - 1)  strNextID.Format("Empty");
			else get_ID_from_loadimage(nJob, m_vtDirPath[nFilePathCount + 1], &strNextID);
			strTemp.Format(" Next ID : %s\n Before ID : %s", strNextID, strBeforeID);

			if (SimulationMesaage(2, str, strTemp) == 0)
				break;
		}
	}
	vtImagePath.clear();
	camBuf.clear();
}
BOOL CPaneSimulation::get_Image_path_2cam_1shot_align(int nJob, CString strFilePath, std::vector<CString> &vtImagePath, int nExpectedCount)
{
	CStringArray FileArray;
	CString str, strImagePath;
	int nFind = FALSE;
	int find_count = 0;

	try {
		BOOL bFind = FALSE;
		CFileFind findImage;

		if (PathFileExists(strFilePath) == FALSE)
		{
			m_pMain->fnSetMessage(1, _T("Check Directory !!!"));
			return FALSE;
		}

		bFind = findImage.FindFile(strFilePath + _T("\\*_ImgRaw.jpg"));

		while (bFind)
		{
			int nCount = 0;

			bFind = findImage.FindNextFile();
			strImagePath = findImage.GetFilePath();	// 찾은 Image

			for (int nCam = 0; nCam < m_pMain->vt_job_info[nJob].camera_index.size(); nCam++)
			{
				int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

				str.Format("%s_ImgRaw", m_pMain->m_stCamInfo[real_cam].cName);
				if (strImagePath.Find(str) > 0)
				{
					vtImagePath[nCam] = strImagePath;
					find_count++;
				}
			}
		}
	}
	catch (...)
	{
		CString str;
		str.Format("Did catch the error to get_Image_path");
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		str.Format("%s", strFilePath);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}

	if (find_count != nExpectedCount)
	{
		nFind = FALSE;
		
		CString str;
		str.Format("[%s] Fail to get Image path", m_pMain->vt_job_info[nJob].job_name.c_str());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_pMain->m_pDlgMessage->setMessageModaless(2, str, strFilePath);
		m_pMain->m_pDlgMessage->ShowWindow(TRUE);
		MSG message;
		while (m_pMain->m_pDlgMessage->getResult() == -1)
		{
			if (::PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
			{
				::TranslateMessage(&message);
				::DispatchMessage(&message);
			}
		}
		if (m_pMain->m_pDlgMessage->getResult() == 0)
		{
			m_bSimulStopFlag = TRUE;
		}
	}
	else nFind = TRUE;

	return nFind;
}

void CPaneSimulation::simulation_2cam_2shot_align(int nJob)
{
	int nPos = 0;
	int ntotal_pos = 2;
	int nExpectedCount = 4;

	BOOL bFind = FALSE;
	std::vector<CString> vtImagePath;
	CString str, strTemp, strBeforeID, strNextID;

	int camCount = 0;
	camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;
	
	// 이미지경로 저장하는 벡터 할당
	vtImagePath.assign( ntotal_pos * camCount , NULL);

#pragma region 폴더 갯수 만큼 시뮬레이션
	for (int nFilePathCount = 0; nFilePathCount < m_vtDirPath.size(); nFilePathCount++)
	{
		if (m_vtSelectList[nFilePathCount] != TRUE) continue;

		m_nCurrent_list = current_list(nFilePathCount);

		CString str;
		str.Format("[%s](%d/%d)Simulation Start ", m_pMain->vt_job_info[nJob].job_name.c_str(), nFilePathCount + 1, m_vtDirPath.size());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		// 매 패널마다 Image Path 리셋.
		for (int i = 0; i < vtImagePath.size(); i++)
			vtImagePath[i].Empty();

		// 이미지경로를 저장한다
		bFind = get_Image_path_2cam_2shot_align(nJob, m_vtDirPath[nFilePathCount], vtImagePath, nExpectedCount);
		if (m_bSimulStopFlag == TRUE) break;
		if (!bFind) continue;

		_stPutinProBuffer pInfo[MAX_CAMERA][NUM_POS];
		// 이미지를 버퍼에 저장한다
		for (int nPos = 0; nPos < 2; nPos++)
		{
			for (int nCam = 0; nCam < camCount; nCam++)
			{
				int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
				pInfo[real_cam][nPos].nCam = nCam;
				pInfo[real_cam][nPos].nPos = nPos;
				pInfo[real_cam][nPos].nJob = nJob;
				pInfo[real_cam][nPos].strPath = vtImagePath[nCam + nPos * camCount];
				pInfo[real_cam][nPos].pView = this;

				m_pMain->m_bSimulGrabEnd[real_cam][nPos] = FALSE;
				AfxBeginThread(Thread_put_image_in_buffer, &pInfo[real_cam][nPos]);
			}
		}

		// 버퍼에 저장된 이미지 완료 체크.. 10초 이후는 그냥 끝냄
		BOOL all_image_read = TRUE;
		int real_cam = m_pMain->vt_job_info[nJob].camera_index[0];
		if (m_pMain->wait_flag_over_time(10000, m_pMain->m_bSimulGrabEnd[0], FALSE, 0, 2, NUM_POS, real_cam, camCount) == -1)
		{
			CString str;
			str.Format("TimeOut to Wait Image Read End.");
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

			all_image_read = FALSE;
			continue;
		}

		////Procsss Start
		get_ID_from_loadimage(nJob, m_vtDirPath[nFilePathCount], &strBeforeID);
		m_pMain->vt_job_info[nJob].main_object_id = strBeforeID;
		::SendMessageA(m_pMain->m_pForm[FORM_MAIN]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_GLASSID_UPDATE, nJob);
		m_pMain->m_nSeqProcess[nJob] = 1000;
		m_pMain->setProcessStartFlag(nJob);
		Delay(50);

		// Process 끝날때 까지 대기.. 10초 이후에는 그냥 끝냄
		if (m_pMain->wait_flag_over_time(10000, &m_pMain->m_bPrealignStartFlag[nJob], TRUE, 0, 1) == -1)
		{
			m_pMain->m_nSeqProcess[nJob] = 0;
			m_pMain->resetProcessStartFlag(nJob);

			CString str;
			str.Format("TimeOut to Wait ProcessFlag Down.");
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			break;
		}
		Delay(50);

		str.Format("[%s](%d/%d)Simulation End ", m_pMain->vt_job_info[nJob].job_name.c_str(), nFilePathCount + 1, m_vtDirPath.size());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		// Stop 누를시 Break
		if (m_bSimulStopFlag == TRUE) break;

		// 시뮬레이션 메세지
		if (m_bUseSimulMessage == TRUE || m_pMain->m_bProcessResult[nJob] != TRUE)
		{
			if (nFilePathCount + 1 == m_vtDirPath.size()) break;

			CString strTemp;
			str.Format("Process %s\nWill you next simulation? (%d/%d)", m_pMain->m_bProcessResult[nJob] == TRUE ? "OK" : "NG", nFilePathCount + 2, m_vtDirPath.size());
			if (nFilePathCount == m_vtDirPath.size() - 1)  strNextID.Format("Empty");
			else get_ID_from_loadimage(nJob, m_vtDirPath[nFilePathCount + 1], &strNextID);
			strTemp.Format(" Next ID : %s\n Before ID : %s", strNextID, strBeforeID);

			if (SimulationMesaage(2, str, strTemp) == 0)
				break;
		}
	}
#pragma endregion
	vtImagePath.clear();
	camBuf.clear();
}
BOOL CPaneSimulation::get_Image_path_2cam_2shot_align(int nJob, CString strFilePath, std::vector <CString> &vtImagePath, int nExpectedCount)
{
	CStringArray FileArray;
	CString str, strImagePath;
	int nFind = FALSE;
	int find_count = 0;

	const int POSITION_1 = 0;
	const int POSITION_2 = 2;

	try {
		BOOL bFind = FALSE;
		CFileFind findImage;

		if (PathFileExists(strFilePath) == FALSE)
		{
			m_pMain->fnSetMessage(1, _T("Check Directory !!!"));
			return FALSE;
		}

		bFind = findImage.FindFile(strFilePath + _T("\\*_ImgRaw*.jpg"));

		while (bFind)
		{
			int nCount = 0;

			bFind = findImage.FindNextFile();
			strImagePath = findImage.GetFilePath();	// 찾은 Image

			// Pos1
			for (int nCam = 0; nCam < m_pMain->vt_job_info[nJob].camera_index.size(); nCam++)
			{
				int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

				str.Format("%s_ImgRaw1.jpg", m_pMain->m_stCamInfo[real_cam].cName);
				if (strImagePath.Find(str) > 0)
				{
					vtImagePath[nCam + POSITION_1] = strImagePath;
					find_count++;
				}
			}

			// Pos2
			for (int nCam = 0; nCam < m_pMain->vt_job_info[nJob].camera_index.size(); nCam++)
			{
				int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

				str.Format("%s_ImgRaw2.jpg", m_pMain->m_stCamInfo[real_cam].cName);
				if (strImagePath.Find(str) > 0)
				{
					vtImagePath[nCam + POSITION_2] = strImagePath;
					find_count++;
				}
			}
		}
	}
	catch (...)
	{
		CString str;
		str.Format("Did catch the error to get_Image_path");
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		str.Format("%s", strFilePath);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}

	if (find_count != nExpectedCount)
	{
		nFind = FALSE;
		
		CString str;
		str.Format("[%s] Fail to get Image path", m_pMain->vt_job_info[nJob].get_job_name());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);


		m_pMain->m_pDlgMessage->setMessageModaless(2, str, strFilePath);
		m_pMain->m_pDlgMessage->ShowWindow(TRUE);
		MSG message;
		while (m_pMain->m_pDlgMessage->getResult() == -1)
		{
			if (::PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
			{
				::TranslateMessage(&message);
				::DispatchMessage(&message);
			}
		}
		if (m_pMain->m_pDlgMessage->getResult() == 0)
		{
			m_bSimulStopFlag = TRUE;
		}
	}
	else nFind = TRUE;

	return nFind;
}

void CPaneSimulation::simulation_4cam_1shot_align(int nJob)
{
	int nExpectedCount = 4;
	BOOL bFind = FALSE;

	std::vector<CString> vtImagePath;
	CString str, strTemp, strBeforeID, strNextID;

	BOOL bExit = FALSE;
	int camCount = 0;
	camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;

	// 이미지경로 저장하는 벡터 할당
	vtImagePath.assign(camCount, NULL);

	BOOL simulation_stop = FALSE;
#pragma region 폴더 갯수 만큼 시뮬레이션
	for (int nFilePathCount = 0; nFilePathCount < m_vtDirPath.size(); nFilePathCount++)
	{
		if (m_vtSelectList[nFilePathCount] != TRUE) continue;

		m_nCurrent_list = current_list(nFilePathCount);

		CString str;
		str.Format("[%s](%d/%d)Simulation Start ", m_pMain->vt_job_info[nJob].job_name.c_str(), nFilePathCount + 1, m_vtDirPath.size());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		// 매 패널마다 Image Path 리셋.
		for (int i = 0; i < vtImagePath.size(); i++)
			vtImagePath[i].Empty();

		// 이미지경로를 저장한다
		bFind = get_Image_path_4cam_1shot_align(nJob, m_vtDirPath[nFilePathCount], vtImagePath, nExpectedCount);
		if (m_bSimulStopFlag == TRUE) break;
		if (!bFind) continue;

		_stPutinProBuffer pInfo[MAX_CAMERA][NUM_POS];
		// 이미지를 버퍼에 저장한다

		for (int nCam = 0; nCam < camCount; nCam++)
		{
			int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
			pInfo[real_cam][0].nCam = nCam;
			pInfo[real_cam][0].nPos = 0;
			pInfo[real_cam][0].nJob = nJob;
			pInfo[real_cam][0].strPath = vtImagePath[nCam];
			pInfo[real_cam][0].pView = this;

			m_pMain->m_bSimulGrabEnd[real_cam][0] = FALSE;
			AfxBeginThread(Thread_put_image_in_buffer, &pInfo[real_cam][0]);
		}

		// 버퍼에 저장된 이미지 완료 체크.. 10초 이후는 그냥 끝냄
		BOOL all_image_read = TRUE;
		int real_cam = m_pMain->vt_job_info[nJob].camera_index[0];
		if (m_pMain->wait_flag_over_time(10000, m_pMain->m_bSimulGrabEnd[0], FALSE, 0, 1, NUM_POS, real_cam, camCount) == -1)
		{
			CString str;
			str.Format("TimeOut to Wait Image Read End.");
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

			all_image_read = FALSE;
			continue;
		}
		
		////Procsss Start
		get_ID_from_loadimage(nJob, m_vtDirPath[nFilePathCount], &strBeforeID);
		m_pMain->vt_job_info[nJob].main_object_id = strBeforeID;
		::SendMessageA(m_pMain->m_pForm[FORM_MAIN]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_GLASSID_UPDATE, nJob);
		m_pMain->m_nSeqProcess[nJob] = 1000;
		m_pMain->setProcessStartFlag(nJob);
		Delay(50);

		// Process 끝날때 까지 대기.. 10초 이후에는 그냥 끝냄
		if (m_pMain->wait_flag_over_time(10000, &m_pMain->m_bPrealignStartFlag[nJob], TRUE, 0, 1) == -1)
		{
			m_pMain->m_nSeqProcess[nJob] = 0;
			m_pMain->resetProcessStartFlag(nJob);

			CString str;
			str.Format("TimeOut to Wait ProcessFlag Down.");
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			break;
		}
		Delay(50);

		str.Format("[%s](%d/%d)Simulation End ", m_pMain->vt_job_info[nJob].job_name.c_str(), nFilePathCount + 1, m_vtDirPath.size());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		// Stop 누를시 Break
		if (m_bSimulStopFlag == TRUE) break;

		// 시뮬레이션 메세지
		if (m_bUseSimulMessage == TRUE || m_pMain->m_bProcessResult[nJob] != TRUE)
		{
			if (nFilePathCount + 1 == m_vtDirPath.size()) break;

			CString strTemp;
			str.Format("Process %s\nWill you next simulation? (%d/%d)", m_pMain->m_bProcessResult[nJob] == TRUE ? "OK" : "NG", nFilePathCount + 2, m_vtDirPath.size());
			if (nFilePathCount == m_vtDirPath.size() - 1)  strNextID.Format("Empty");
			else get_ID_from_loadimage(nJob, m_vtDirPath[nFilePathCount + 1], &strNextID);
			strTemp.Format(" Next ID : %s\n Before ID : %s", strNextID, strBeforeID);

			if (SimulationMesaage(2, str, strTemp) == 0)
				break;
		}
	}
	vtImagePath.clear();
	camBuf.clear();
}
BOOL CPaneSimulation::get_Image_path_4cam_1shot_align(int nJob, CString strFilePath, std::vector<CString>& vtImagePath, int nExpectedCount)
{
	CStringArray FileArray;
	CString str, strImagePath;
	int nFind = FALSE;
	int find_count = 0;

	try {
		BOOL bFind = FALSE;
		CFileFind findImage;

		if (PathFileExists(strFilePath) == FALSE)
		{
			m_pMain->fnSetMessage(1, _T("Check Directory !!!"));
			return FALSE;
		}

		bFind = findImage.FindFile(strFilePath + _T("\\*_ImgRaw.jpg"));

		while (bFind)
		{
			int nCount = 0;

			bFind = findImage.FindNextFile();
			strImagePath = findImage.GetFilePath();	// 찾은 Image

			for (int nCam = 0; nCam < m_pMain->vt_job_info[nJob].camera_index.size(); nCam++)
			{
				int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

				str.Format("%s_ImgRaw", m_pMain->m_stCamInfo[real_cam].cName);
				if (strImagePath.Find(str) > 0)
				{
					vtImagePath[nCam] = strImagePath;
					find_count++;
				}
			}
		}
	}
	catch (...)
	{
		CString str;
		str.Format("Did catch the error to get_Image_path");
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		str.Format("%s", strFilePath);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}

	if (find_count != nExpectedCount)
	{
		nFind = FALSE;

		CString str;;
		str.Format("[%s] Fail to get Image path", m_pMain->vt_job_info[nJob].job_name.c_str());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_pMain->m_pDlgMessage->setMessageModaless(2, str, strFilePath);
		m_pMain->m_pDlgMessage->ShowWindow(TRUE);
		MSG message;
		while (m_pMain->m_pDlgMessage->getResult() == -1)
		{
			if (::PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
			{
				::TranslateMessage(&message);
				::DispatchMessage(&message);
			}
		}
		if (m_pMain->m_pDlgMessage->getResult() == 0)
		{
			m_bSimulStopFlag = TRUE;
		}
	}
	else nFind = TRUE;

	return nFind;
}

void CPaneSimulation::simulation_Elb_centerAlign(int nJob)
{
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[0];
	CString str;

	CFileDialog dlg(TRUE, NULL, _T("Image File"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Image Files (*.bmp;*.jpg;)|*.bmp;*.jpg|All Files (*.*)|*.*||"), this);

	if (dlg.DoModal() != IDOK)
		return;

	put_image_in_buffer(nJob, real_cam, (LPCTSTR)dlg.GetPathName(), 0);

	m_pMain->m_nSeqProcess[nJob] = 210;
	m_pMain->setProcessStartFlag(nJob);

	// kbj 2021-12-27 Process 끝날때 까지 대기.. 10초 이후에는 그냥 끝냄
	if (m_pMain->wait_flag_over_time(10000, &m_pMain->m_bPrealignStartFlag[nJob], TRUE, 0, 1) == -1)
	{
		m_pMain->m_nSeqProcess[nJob] = 0;
		m_pMain->resetProcessStartFlag(nJob);

		str.Format("TimeOut to Wait ProcessFlag Down.");
		::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
	Delay(50);

	str.Format("[%s](%d/%d)Simulation End ", m_pMain->vt_job_info[nJob].job_name.c_str(), real_cam, 0);
	::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
}
void CPaneSimulation::simulation_nozzle_isnp(int nJob)
{
	int camCount = 0;
	camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;

	CString str;

	for (int nCam = 0; nCam < camCount; nCam++)
	{
		int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
		CFileDialog dlg(TRUE, NULL, _T("Image File"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
			_T("Image Files (*.bmp;*.jpg;)|*.bmp;*.jpg|All Files (*.*)|*.*||"), this);

		if (dlg.DoModal() != IDOK)
			return;

		put_image_in_buffer(nJob, real_cam, (LPCTSTR)dlg.GetPathName(), 0);
	}

	m_pMain->m_nSeqProcess[nJob] = 210;
	m_pMain->setProcessStartFlag(nJob);

	// kbj 2021-12-27 Process 끝날때 까지 대기.. 10초 이후에는 그냥 끝냄
	if (m_pMain->wait_flag_over_time(10000, &m_pMain->m_bPrealignStartFlag[nJob], TRUE, 0, 1) == -1)
	{
		m_pMain->m_nSeqProcess[nJob] = 0;
		m_pMain->resetProcessStartFlag(nJob);

		str.Format("TimeOut to Wait ProcessFlag Down.");
		::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
	Delay(50);

	str.Format("[%s](%d/%d)Simulation End ", m_pMain->vt_job_info[nJob].job_name.c_str(), 0, 0);
	::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
}
void CPaneSimulation::simulation_Scan_isnp(int nJob)
{
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[0];
	CString str;

	CFileDialog dlg(TRUE, NULL, _T("Image File"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Image Files (*.bmp;*.jpg;)|*.bmp;*.jpg|All Files (*.*)|*.*||"), this);

	if (dlg.DoModal() != IDOK)
		return;

	put_image_in_buffer(nJob, real_cam, (LPCTSTR)dlg.GetPathName(), 0);

	m_pMain->m_nSeqProcess[nJob] = 210;
	m_pMain->setProcessStartFlag(nJob);

	// kbj 2021-12-27 Process 끝날때 까지 대기.. 10초 이후에는 그냥 끝냄
	if (m_pMain->wait_flag_over_time(10000, &m_pMain->m_bPrealignStartFlag[nJob], TRUE, 0, 1) == -1)
	{
		m_pMain->m_nSeqProcess[nJob] = 0;
		m_pMain->resetProcessStartFlag(nJob);

		str.Format("TimeOut to Wait ProcessFlag Down.");
		::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
	Delay(50);

	str.Format("[%s](%d/%d)Simulation End ", m_pMain->vt_job_info[nJob].job_name.c_str(), real_cam, 0);
	::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
}

// hsj 2022-01-12 center nozzle align simulation 추가
void CPaneSimulation::simulation_Center_NozzleAlign(int nJob)
{
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[0];
	int nExpectedCount = 2;
	BOOL bFind = FALSE;

	std::vector<CString> vtImagePath;
	CString str, strTemp, strBeforeID, strNextID;

	BOOL bExit = FALSE;
	int camCount = 0;
	camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;

	// 이미지경로 저장하는 벡터 할당
	vtImagePath.assign(camCount, NULL);

	BOOL simulation_stop = FALSE;
#pragma region 폴더 갯수 만큼 시뮬레이션
	for (int nFilePathCount = 0; nFilePathCount < m_vtDirPath.size(); nFilePathCount++)
	{
		if (m_vtSelectList[nFilePathCount] != TRUE) continue;

		m_nCurrent_list = current_list(nFilePathCount);

		CString str;
		str.Format("[%s](%d/%d)Simulation Start ", m_pMain->vt_job_info[nJob].job_name.c_str(), nFilePathCount + 1, m_vtDirPath.size());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		// 매 패널마다 Image Path 리셋.
		for (int i = 0; i < vtImagePath.size(); i++)
			vtImagePath[i].Empty();

		// 이미지경로를 저장한다
		bFind = get_Image_path_2cam_1shot_align(nJob, m_vtDirPath[nFilePathCount], vtImagePath, nExpectedCount);
		if (m_bSimulStopFlag == TRUE) break;
		if (!bFind) continue;

		_stPutinProBuffer pInfo[MAX_CAMERA][NUM_POS];
		// 이미지를 버퍼에 저장한다

		for (int nCam = 0; nCam < camCount; nCam++)
		{
			int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
			pInfo[real_cam][0].nCam = nCam;
			pInfo[real_cam][0].nPos = 0;
			pInfo[real_cam][0].nJob = nJob;
			pInfo[real_cam][0].strPath = vtImagePath[nCam];
			pInfo[real_cam][0].pView = this;

			m_pMain->m_bSimulGrabEnd[real_cam][0] = FALSE;

			put_image_in_buffer(nJob, nCam, vtImagePath[nCam], 0);
		}

		////Procsss Start
		get_ID_from_loadimage(nJob, m_vtDirPath[nFilePathCount], &strBeforeID);
		m_pMain->vt_job_info[nJob].main_object_id = strBeforeID;
		::SendMessageA(m_pMain->m_pForm[FORM_MAIN]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_GLASSID_UPDATE, nJob);
		m_pMain->m_nSeqProcess[nJob] = 210;
		m_pMain->setProcessStartFlag(nJob);

		// kbj 2021-12-27 Process 끝날때 까지 대기.. 10초 이후에는 그냥 끝냄
		if (m_pMain->wait_flag_over_time(10000, &m_pMain->m_bPrealignStartFlag[nJob], TRUE, 0, 1) == -1)
		{
			m_pMain->m_nSeqProcess[nJob] = 0;
			m_pMain->resetProcessStartFlag(nJob);

			str.Format("TimeOut to Wait ProcessFlag Down.");
			::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}
		Delay(50);

		str.Format("[%s](%d/%d)Simulation End ", m_pMain->vt_job_info[nJob].job_name.c_str(), real_cam, 0);
		::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		// Stop 누를시 Break
		if (m_bSimulStopFlag == TRUE) break;

		// 시뮬레이션 메세지
		if (m_bUseSimulMessage == TRUE || m_pMain->m_bProcessResult[nJob] != TRUE)
		{
			if (nFilePathCount + 1 == m_vtDirPath.size()) break;

			CString strTemp;
			str.Format("Process %s\nWill you next simulation? (%d/%d)", m_pMain->m_bProcessResult[nJob] == TRUE ? "OK" : "NG", nFilePathCount + 2, m_vtDirPath.size());
			if (nFilePathCount == m_vtDirPath.size() - 1)  strNextID.Format("Empty");
			else get_ID_from_loadimage(nJob, m_vtDirPath[nFilePathCount + 1], &strNextID);
			strTemp.Format(" Next ID : %s\n Before ID : %s", strNextID, strBeforeID);

			if (SimulationMesaage(2, str, strTemp) == 0)
				break;
		}
	}
}

// PJH 2022-09-02 Z_GAP Simulation 없어서 추가
void CPaneSimulation::simulation_zgap_insp(int nJob)
{
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[0];
	int nExpectedCount = 1;
	BOOL bFind = FALSE;

	CString  strFilePath, strImagePath;
	CFileFind findImage;

	std::vector<CString> vtImagePath;
	CString str, strTemp, strBeforeID, strNextID;

	BOOL bExit = FALSE;
	int camCount = 0;
	camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;

	// 이미지경로 저장하는 벡터 할당
	vtImagePath.assign(camCount, NULL);

	BOOL simulation_stop = FALSE;
#pragma region 폴더 갯수 만큼 시뮬레이션
	for (int nFilePathCount = 0; nFilePathCount < m_vtDirPath.size(); nFilePathCount++)
	{
		if (m_vtSelectList[nFilePathCount] != TRUE) continue;

		m_nCurrent_list = current_list(nFilePathCount);

		CString str;
		str.Format("[%s](%d/%d)Simulation Start ", m_pMain->vt_job_info[nJob].job_name.c_str(), nFilePathCount + 1, m_vtDirPath.size());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		// 매 패널마다 Image Path 리셋.
		for (int i = 0; i < vtImagePath.size(); i++)
			vtImagePath[i].Empty();


		bFind = findImage.FindFile(m_vtDirPath[nFilePathCount] + _T("\\*_ImgRaw.jpg"));

		// LYS 2022-09-12 폴더 내에 파일 갯수 카운트
		/*
		while (bFind)
		{
			int nCount = 0;

			bFind = findImage.FindNextFile();
			strImagePath = findImage.GetFilePath();	// 찾은 Image

			for (int nCam = 0; nCam < m_pMain->vt_job_info[nJob].camera_index.size(); nCam++)
			{
				int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

				str.Format("%s_ImgRaw", m_pMain->m_stCamInfo[real_cam].cName);
				if (strImagePath.Find(str) > 0)
				{
					vtImagePath[nCam] = strImagePath;
					nExpectedCount++;
				}
			}
		}
		*/


		// 이미지경로를 저장한다
//		for (int nJobCnt = 0; nJobCnt < nExpectedCount; nJobCnt++)
//		{
//			bFind = get_Image_path_1cam_1shot(nJob, m_vtDirPath[nFilePathCount], vtImagePath, nExpectedCount);
//			if (m_bSimulStopFlag == TRUE) break;
//			if (!bFind) continue;
//		}
		bFind = get_Image_path_1cam_1shot(nJob, m_vtDirPath[nFilePathCount], vtImagePath, 0);
		if (m_bSimulStopFlag == TRUE) break;
		if (!bFind) continue;

		_stPutinProBuffer pInfo[MAX_CAMERA][NUM_POS];
		// 이미지를 버퍼에 저장한다
		for (int nCam = 0; nCam < camCount; nCam++)
		{
			int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
			pInfo[real_cam][0].nCam = nCam;
			pInfo[real_cam][0].nPos = 0;
			pInfo[real_cam][0].nJob = nJob;
			pInfo[real_cam][0].strPath = vtImagePath[nCam];
			pInfo[real_cam][0].pView = this;

			m_pMain->m_bSimulGrabEnd[real_cam][0] = FALSE;

			put_image_in_buffer(nJob, nCam, vtImagePath[nCam], 0);
		}


		////Procsss Start
		get_ID_from_loadimage(nJob, m_vtDirPath[nFilePathCount], &strBeforeID);
		m_pMain->vt_job_info[nJob].main_object_id = strBeforeID;

		::SendMessageA(m_pMain->m_pForm[FORM_MAIN]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_GLASSID_UPDATE, nJob);

		int W = m_pMain->m_stCamInfo[real_cam].w;
		int H = m_pMain->m_stCamInfo[real_cam].h;
		CFormMainView* pFormMain = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];
		int nViewer = m_pMain->vt_job_info[nJob].viewer_index[0];

		cv::Mat img(H, W, CV_8UC1, m_pMain->getProcBuffer(real_cam,0));

		// KBJ 2022-11-11
		pFormMain->m_pDlgViewerMain[nViewer]->GetViewer().OnLoadImageFromPtr(m_pMain->getProcBuffer(real_cam, 0));
		pFormMain->fnSimulation_CalcNozzleGapInsp(nJob, real_cam, img, &pFormMain->m_pDlgViewerMain[nViewer]->GetViewer());
		img.release();

//		str.Format("[%s](%d/%d)Simulation End ", m_pMain->vt_job_info[nJob].job_name.c_str(), real_cam, 0);
		str.Format("[%s](%d/%d)Simulation End ", m_pMain->vt_job_info[nJob].job_name.c_str(), nFilePathCount + 1, m_vtDirPath.size());
		::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		((CPaneAuto*)m_pMain->m_pPane[PANE_AUTO])->DrawStageZGapViewer(nJob, real_cam);		// LYS 2022-09-05 add

		// Stop 누를시 Break
		if (m_bSimulStopFlag == TRUE) break;

		// 시뮬레이션 메세지
		if (m_bUseSimulMessage == TRUE || m_pMain->m_bProcessResult[nJob] != TRUE)
		{
			if (nFilePathCount + 1 == m_vtDirPath.size()) break;

			CString strTemp;
			str.Format("Process %s\nWill you next simulation? (%d/%d)", m_pMain->m_bProcessResult[nJob] == TRUE ? "OK" : "NG", nFilePathCount + 2, m_vtDirPath.size());
			if (nFilePathCount == m_vtDirPath.size() - 1)  strNextID.Format("Empty");
			else get_ID_from_loadimage(nJob, m_vtDirPath[nFilePathCount + 1], &strNextID);
			strTemp.Format(" Next ID : %s\n Before ID : %s", strNextID, strBeforeID);

			if (SimulationMesaage(2, str, strTemp) == 0)
				break;
		}
	}
}

// PJH 2022-09-02 Z_GAP Simulation 없어서 추가
void CPaneSimulation::simulation_zgap_insp_Live(int nJob)
{
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[0];
	int nExpectedCount = 1;
	BOOL bFind = FALSE;

	std::vector<CString> vtImagePath;
	CString str, strTemp, strBeforeID, strNextID;

	BOOL bExit = FALSE;
	int camCount = 0;
	camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;

	// 이미지경로 저장하는 벡터 할당
	vtImagePath.assign(camCount, NULL);

	BOOL simulation_stop = FALSE;
	int W = m_pMain->m_stCamInfo[real_cam].w;
	int H = m_pMain->m_stCamInfo[real_cam].h;

	str.Format("[%s] Simulation Start ", m_pMain->vt_job_info[nJob].job_name.c_str());
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 0), m_pMain->getSrcBuffer(real_cam), W * H);

	::SendMessageA(m_pMain->m_pForm[FORM_MAIN]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_GLASSID_UPDATE, nJob);
	
	CFormMainView* pFormMain = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];
	int nViewer = m_pMain->vt_job_info[nJob].viewer_index[0];

	cv::Mat img(H, W, CV_8UC1, m_pMain->getProcBuffer(real_cam, 0));

	pFormMain->fnSimulation_CalcNozzleGapInsp(nJob, real_cam, img, &pFormMain->m_pDlgViewerMain[nViewer]->GetViewer());
	img.release();

	str.Format("[%s](%d/%d)Simulation End ", m_pMain->vt_job_info[nJob].job_name.c_str(), real_cam, 0);
	::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
}

void CPaneSimulation::simulation_cam_position_insp(int nJob)
{
	int nExpectedCount = m_pMain->vt_job_info[nJob].num_of_camera;
	BOOL bFind = FALSE;

	std::vector<CString> vtImagePath;
	CString str, strTemp, strBeforeID, strNextID;

	BOOL bExit = FALSE;
	int camCount = 0;
	camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;

	// 이미지경로 저장하는 벡터 할당
	vtImagePath.assign(camCount, NULL);

	BOOL simulation_stop = FALSE;
#pragma region 폴더 갯수 만큼 시뮬레이션
	for (int nFilePathCount = 0; nFilePathCount < m_vtDirPath.size(); nFilePathCount++)
	{
		if (m_vtSelectList[nFilePathCount] != TRUE) continue;
		m_nCurrent_list = current_list(nFilePathCount);

		CString str;
		str.Format("[%s](%d/%d)Simulation Start ", m_pMain->vt_job_info[nJob].job_name.c_str(), nFilePathCount + 1, m_vtDirPath.size());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		// 매 패널마다 Image Path 리셋.
		for (int i = 0; i < vtImagePath.size(); i++)
			vtImagePath[i].Empty();

		// 이미지경로를 저장한다
		bFind = get_Image_path_4cam_1shot_align(nJob, m_vtDirPath[nFilePathCount], vtImagePath, nExpectedCount);
		if (m_bSimulStopFlag == TRUE) break;
		if (!bFind) continue;

		_stPutinProBuffer pInfo[MAX_CAMERA][NUM_POS];
		// 이미지를 버퍼에 저장한다

		for (int nCam = 0; nCam < camCount; nCam++)
		{
			int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
			pInfo[real_cam][0].nCam = nCam;
			pInfo[real_cam][0].nPos = 0;
			pInfo[real_cam][0].nJob = nJob;
			pInfo[real_cam][0].strPath = vtImagePath[nCam];
			pInfo[real_cam][0].pView = this;

			m_pMain->m_bSimulGrabEnd[real_cam][0] = FALSE;
			AfxBeginThread(Thread_put_image_in_buffer, &pInfo[real_cam][0]);
		}

		// 버퍼에 저장된 이미지 완료 체크.. 10초 이후는 그냥 끝냄
		BOOL all_image_read = TRUE;
		int real_cam = m_pMain->vt_job_info[nJob].camera_index[0];
		if (m_pMain->wait_flag_over_time(10000, m_pMain->m_bSimulGrabEnd[0], FALSE, 0, 1, NUM_POS, real_cam, camCount) == -1)
		{
			CString str;
			str.Format("TimeOut to Wait Image Read End.");
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

			all_image_read = FALSE;
			continue;
		}

		////Procsss Start
		get_ID_from_loadimage(nJob, m_vtDirPath[nFilePathCount], &strBeforeID);
		m_pMain->vt_job_info[nJob].main_object_id = strBeforeID;
		::SendMessageA(m_pMain->m_pForm[FORM_MAIN]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_GLASSID_UPDATE, nJob);
		m_pMain->m_nSeqProcess[nJob] = 1000;
		m_pMain->setProcessStartFlag(nJob);
		Delay(50);

		// Process 끝날때 까지 대기.. 10초 이후에는 그냥 끝냄
		if (m_pMain->wait_flag_over_time(10000, &m_pMain->m_bPrealignStartFlag[nJob], TRUE, 0, 1) == -1)
		{
			m_pMain->m_nSeqProcess[nJob] = 0;
			m_pMain->resetProcessStartFlag(nJob);

			CString str;
			str.Format("TimeOut to Wait ProcessFlag Down.");
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			break;
		}
		Delay(50);

		str.Format("[%s](%d/%d)Simulation End ", m_pMain->vt_job_info[nJob].job_name.c_str(), nFilePathCount + 1, m_vtDirPath.size());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		// Stop 누를시 Break
		if (m_bSimulStopFlag == TRUE) break;

		// 시뮬레이션 메세지
		if (m_bUseSimulMessage == TRUE || m_pMain->m_bProcessResult[nJob] != TRUE)
		{
			if (nFilePathCount + 1 == m_vtDirPath.size()) break;

			CString strTemp;
			str.Format("Process %s\nWill you next simulation? (%d/%d)", m_pMain->m_bProcessResult[nJob] == TRUE ? "OK" : "NG", nFilePathCount + 2, m_vtDirPath.size());
			if (nFilePathCount == m_vtDirPath.size() - 1)  strNextID.Format("Empty");
			else get_ID_from_loadimage(nJob, m_vtDirPath[nFilePathCount + 1], &strNextID);
			strTemp.Format(" Next ID : %s\n Before ID : %s", strNextID, strBeforeID);

			if (SimulationMesaage(2, str, strTemp) == 0)
				break;
		}
	}
	vtImagePath.clear();
	camBuf.clear();
}

void CPaneSimulation::simulation_exist_isnp(int nJob)
{
	int nExpectedCount = 1;
	BOOL bFind = FALSE;

	std::vector<CString> vtImagePath;
	CString str, strTemp, strBeforeID, strNextID;

	BOOL bExit = FALSE;
	int camCount = 0;
	camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;

	// 이미지경로 저장하는 벡터 할당
	vtImagePath.assign(camCount, NULL);

	BOOL simulation_stop = FALSE;
#pragma region 폴더 갯수 만큼 시뮬레이션
	for (int nFilePathCount = 0; nFilePathCount < m_vtDirPath.size(); nFilePathCount++)
	{
		if (m_vtSelectList[nFilePathCount] != TRUE) continue;

		m_nCurrent_list = current_list(nFilePathCount);

		CString str;
		str.Format("[%s](%d/%d)Simulation Start ", m_pMain->vt_job_info[nJob].job_name.c_str(), nFilePathCount + 1, m_vtDirPath.size());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		// 매 패널마다 Image Path 리셋.
		for (int i = 0; i < vtImagePath.size(); i++)
			vtImagePath[i].Empty();

		// 이미지경로를 저장한다
		bFind = get_Image_path_exist_insp(nJob, m_vtDirPath[nFilePathCount], vtImagePath, nExpectedCount);
		if (m_bSimulStopFlag == TRUE) break;
		if (!bFind) continue;

		_stPutinProBuffer pInfo[MAX_CAMERA][NUM_POS];
		// 이미지를 버퍼에 저장한다
		for (int nCam = 0; nCam < camCount; nCam++)
		{
			int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
			pInfo[real_cam][0].nCam = nCam;
			pInfo[real_cam][0].nPos = 0;
			pInfo[real_cam][0].nJob = nJob;
			pInfo[real_cam][0].strPath = vtImagePath[nCam];
			pInfo[real_cam][0].pView = this;

			m_pMain->m_bSimulGrabEnd[real_cam][0] = FALSE;
			AfxBeginThread(Thread_put_image_in_buffer, &pInfo[real_cam][0]);
		}

		// 버퍼에 저장된 이미지 완료 체크.. 10초 이후는 그냥 끝냄
		BOOL all_image_read = TRUE;
		int real_cam = m_pMain->vt_job_info[nJob].camera_index[0];
		if (m_pMain->wait_flag_over_time(10000, m_pMain->m_bSimulGrabEnd[real_cam], FALSE, 0, 1) == -1)
		{
			str.Format("TimeOut to Wait Image Read End.");
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

			all_image_read = FALSE;
			continue;
		}

		////Procsss Start
		get_ID_from_loadimage(nJob, m_vtDirPath[nFilePathCount], &strBeforeID);
		m_pMain->vt_job_info[nJob].main_object_id = strBeforeID;
		::SendMessageA(m_pMain->m_pForm[FORM_MAIN]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_GLASSID_UPDATE, nJob);
		m_pMain->m_nSeqProcess[nJob] = 1000;
		m_pMain->setProcessStartFlag(nJob);
		Delay(50);

		// Process 끝날때 까지 대기.. 10초 이후에는 그냥 끝냄
		//if (m_pMain->wait_flag_over_time(10000, &m_pMain->m_bPrealignStartFlag[nJob], TRUE, 0, 1) == -1)
		if (m_pMain->wait_flag_over_time(10000, &m_pMain->m_bPrealignStartFlag[nJob], TRUE, 0, 1) == -1)
		{
			m_pMain->m_nSeqProcess[nJob] = 0;
			m_pMain->resetProcessStartFlag(nJob);

			str.Format("TimeOut to Wait ProcessFlag Down.");
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			break;
		}
		Delay(50);

		str.Format("[%s](%d/%d)Simulation End ", m_pMain->vt_job_info[nJob].job_name.c_str(), nFilePathCount + 1, m_vtDirPath.size());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		// Stop 누를시 Break
		if (m_bSimulStopFlag == TRUE) break;

		// 시뮬레이션 메세지
		if (m_bUseSimulMessage == TRUE || m_pMain->m_bProcessResult[nJob] != TRUE)
		{
			if (nFilePathCount + 1 == m_vtDirPath.size()) break;

			CString strTemp;
			str.Format("Process %s\nWill you next simulation? (%d/%d)", m_pMain->m_bProcessResult[nJob] == TRUE ? "OK" : "NG", nFilePathCount + 2, m_vtDirPath.size());
			if (nFilePathCount == m_vtDirPath.size() - 1)  strNextID.Format("Empty");
			else get_ID_from_loadimage(nJob, m_vtDirPath[nFilePathCount + 1], &strNextID);
			strTemp.Format(" Next ID : %s\n Before ID : %s", strNextID, strBeforeID);

			if (SimulationMesaage(2, str, strTemp) == 0)
				break;
		}
	}
	vtImagePath.clear();
	camBuf.clear();
}
BOOL CPaneSimulation::get_Image_path_exist_insp(int nJob, CString strFilePath, std::vector <CString> &vtImagePath, int nExpectedCount)
{
	CStringArray FileArray;
	CString str, strImagePath;
	int nFind = FALSE;
	int find_count = 0;

	try {
		BOOL bFind = FALSE;
		CFileFind findImage;

		if (PathFileExists(strFilePath) == FALSE)
		{
			m_pMain->fnSetMessage(1, _T("Check Directory !!!"));
			return FALSE;
		}

		bFind = findImage.FindFile(strFilePath + _T("\\*_ImgRaw*.jpg"));

		while (bFind)
		{
			int nCount = 0;

			bFind = findImage.FindNextFile();
			strImagePath = findImage.GetFilePath();	// 찾은 Image

			for (int nCam = 0; nCam < m_pMain->vt_job_info[nJob].camera_index.size(); nCam++)
			{
				int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

				str.Format("%s_ImgRaw", m_pMain->m_stCamInfo[real_cam].cName);
				if (strImagePath.Find(str) > 0)
				{
					vtImagePath[nCam] = strImagePath;
					find_count++;
				}
			}
		}
	}
	catch (...)
	{
		
		
		str.Format("Did catch the error to get_Image_path");
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		
		str.Format("%s", strFilePath);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}

	if (find_count != nExpectedCount)
	{
		nFind = FALSE;

		
		
		str.Format("[%s] Fail to get Image path", m_pMain->vt_job_info[nJob].job_name.c_str());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_pMain->m_pDlgMessage->setMessageModaless(2, str, strFilePath);
		m_pMain->m_pDlgMessage->ShowWindow(TRUE);
		MSG message;
		while (m_pMain->m_pDlgMessage->getResult() == -1)
		{
			if (::PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
			{
				::TranslateMessage(&message);
				::DispatchMessage(&message);
			}
		}
		if (m_pMain->m_pDlgMessage->getResult() == 0)
		{
			m_bSimulStopFlag = TRUE;
		}

	}
	else nFind = TRUE;

	return nFind;
}

void CPaneSimulation::set_simulation_start_stop(BOOL bSimulation)
{
	m_pMain->m_bSimulationStart = bSimulation;
	m_bSimulStopFlag = !bSimulation;

	CPaneBottomSimulation *pView = (CPaneBottomSimulation *)m_pMain->m_pPaneBottom[B_PANE_SIMULATION];
	pView->m_bSimulStopFlag = !bSimulation;
	pView->update_frame_dialog();
}

void CPaneSimulation::update_frame_dialog()
{
	if (m_bUseLive)
	{
		m_btnLive.SetColorBkg(255, RGB(64, 64, 65));
		m_btnAppendList.SetEnable(FALSE);
		m_btnDeleteList.SetEnable(FALSE);
		m_grid_list.SetEditable(FALSE);
		m_btnReverseSelect.SetEnable(FALSE);
	}
	else
	{
		m_btnLive.SetColorBkg(255, RGB(64, 64, 65));
		m_btnAppendList.SetEnable(TRUE);
		m_btnDeleteList.SetEnable(TRUE);
		m_grid_list.SetEditable(TRUE);
		m_btnReverseSelect.SetEnable(TRUE);
	}

	m_cmbSelectJob.SetCurSel(m_nJob);

	update_list();
}

BOOL CPaneSimulation::get_directory_path(std::vector < CString > &vtFilePath, int nJob)
{
	CString str, strImageDir, str_algo, strTime, str_modelID;
	CFileFind finder;
	BOOL bFind = FALSE;
	int nFindCount = 0;

	SYSTEMTIME time;
	::GetLocalTime(&time);

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
		return FALSE;
	}
	strImageDir = dlg.GetPathName();
	delete dlg;

	bFind = finder.FindFile(strImageDir + _T("\\*.*"));
	while (bFind)
	{
		bFind = finder.FindNextFile();

		if (finder.IsDots())	// '.', '..' 인 경우 continue
		{
			continue;
		}
		else if (!finder.IsDirectory())	//'파일' 인 경우 Raw이미지 확인
		{
			CString strFilePath = finder.GetFilePath();
			for (int nCam = 0; nCam < m_pMain->vt_job_info[nJob].camera_index.size(); nCam++)
			{
				int nPathFind = 0;
				int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

				str.Format("%s_ImgRaw", m_pMain->m_stCamInfo[real_cam].cName);
				if (strFilePath.Find(str) > 0)	//상위폴더로 전환
				{
					CString  strDirPath;
					strDirPath = strFilePath.Left(strFilePath.ReverseFind('\\'));
					vtFilePath.push_back(strDirPath);
					return TRUE;
				}
			}
			continue;
		}
		
		str = finder.GetFilePath();
		vtFilePath.push_back(str);
		nFindCount++;
	}
	
	if (!nFindCount)
	{
		vtFilePath.clear();
		bFind = FALSE;

		
		
		str.Format("[%s] Fail to get Directory path", m_pMain->vt_job_info[nJob].job_name.c_str());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
	else bFind = TRUE;

	return bFind;
}
BOOL CPaneSimulation::get_directory_path(std::vector < CString > &vtFilePath, CString strFilePath, int nJob)
{
	CString str, strImageDir, str_algo, strTime, str_modelID;
	CFileFind finder;
	BOOL bFind = FALSE;
	int nFindCount = 0;

	strImageDir = strFilePath;

	bFind = finder.FindFile(strImageDir + _T("\\*.*"));
	while (bFind)
	{
		bFind = finder.FindNextFile();

		if (finder.IsDots())	// '.', '..' 인 경우 continue
		{
			continue;
		}
		else if (!finder.IsDirectory())	//'파일' 인 경우 Raw이미지 확인
		{
			CString strFilePath = finder.GetFilePath();
			for (int nCam = 0; nCam < m_pMain->vt_job_info[nJob].camera_index.size(); nCam++)
			{
				int nPathFind = 0;
				int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

				//str.Format("%s_ImgRaw", m_pMain->m_stCamInfo[real_cam].cName);
				str.Format("_ImgRaw");
				if (strFilePath.Find(str) > 0)	//상위폴더로 전환
				{
					CString  strDirPath;
					strDirPath = strFilePath.Left(strFilePath.ReverseFind('\\'));
					vtFilePath.push_back(strDirPath);
					return TRUE;
				}
			}
			continue;
		}

		str = finder.GetFilePath();
		vtFilePath.push_back(str);
		nFindCount++;
	}

	if (!nFindCount)
	{
		//vtFilePath.clear();
		bFind = FALSE;
		
		
		
		str.Format("[%s] Fail to get Directory path", m_pMain->vt_job_info[nJob].job_name.c_str());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
	else bFind = TRUE;

	return bFind;
}

void CPaneSimulation::get_ID_from_loadimage(int nJob, CString strFilePath, CString *strID)
{
	int nLength = strFilePath.GetLength();
	int nFind = strFilePath.ReverseFind(_T('\\'));
	*strID = strFilePath.Right(nLength - nFind - 1);
}

UINT Thread_put_image_in_buffer(void *pParam)
{
	struct _stPutinProBuffer *pPutinProBuffer = (struct _stPutinProBuffer *) pParam;
	CPaneSimulation *pView = (CPaneSimulation *)pPutinProBuffer->pView;

	int nJob = pPutinProBuffer->nJob;
	int nCam = pPutinProBuffer->nCam;
	int nPos = pPutinProBuffer->nPos;
	CString strFilePath = pPutinProBuffer->strPath;
	
	int real_cam = pView->m_pMain->vt_job_info[nJob].camera_index[nCam];

	BOOL bResult = FALSE;
	bResult = pView->put_image_in_buffer(nJob, nCam, strFilePath, nPos);

	if(bResult == TRUE) pView->m_pMain->m_bSimulGrabEnd[real_cam][nPos] = TRUE;

	return 0;
}

BOOL CPaneSimulation::put_image_in_buffer(int nJob, int nCam, CString strFilePath, int nPos)
{
	CString str;

	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;
	int real_cam = camBuf.at(nCam);

	int nWidth = m_pMain->m_stCamInfo[real_cam].w;
	int nHeight = m_pMain->m_stCamInfo[real_cam].h;

	cv::Mat loadImg;
	loadImg = cv::imread((LPCTSTR)strFilePath, 0);

	if (loadImg.cols != nWidth || loadImg.rows != nHeight)
	{
		str.Format("Cam %d Image Size is not Valid.	size %d x %d", nCam + 1, nWidth, nHeight);
		m_pMain->m_pDlgMessage->setMessageModaless(2, str, strFilePath);
		m_pMain->m_pDlgMessage->ShowWindow(TRUE);
		MSG message;
		while (m_pMain->m_pDlgMessage->getResult() == -1)
		{
			if (::PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
			{
				::TranslateMessage(&message);
				::DispatchMessage(&message);
			}
		}
		loadImg.release();

		
		
		str.Format("[%s] Cam %d - %d Fail to put image in buffer ", m_pMain->vt_job_info[nJob].job_name.c_str(), nCam + 1, nPos + 1);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		return FALSE;
	}
	else
	{
		//m_pMain->copyMemory(m_pMain->getSrcBuffer(real_cam), loadImg.data, loadImg.cols * loadImg.rows);
		m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, nPos), loadImg.data, loadImg.cols * loadImg.rows);
		loadImg.release();

		str.Format("[%s] Cam %d - %d Put image in buffer Complete", m_pMain->vt_job_info[nJob].job_name.c_str(), nCam + 1, nPos + 1);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		return TRUE;
	}
}

int CPaneSimulation::SimulationMesaage(int nType, CString strText1, CString strText2)
{
	MSG message;
	int nResult = 0; // 1 = OK, 0 = Cancel, Init = -1;

	m_pMain->m_pDlgMessage->setMessageModaless(2, strText1, strText2);
	m_pMain->m_pDlgMessage->ShowWindow(TRUE);

	while (m_pMain->m_pDlgMessage->getResult() == -1)
	{
		if (::PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&message);
			::DispatchMessage(&message);
		}
	}
	nResult = m_pMain->m_pDlgMessage->getResult();

	return nResult;
}

int CPaneSimulation::get_job_from_img(CString strFilePath)
{
	int nJob = -1;
	int nMethod = 0;

	CString path = strFilePath;

	for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
	{
		CString stJobName = m_pMain->vt_job_info[i].job_name.c_str();
		if (path.Find(stJobName) > 0)//JOB NAME이 포함되면
		{
			nMethod = m_pMain->vt_job_info[i].algo_method;
			nJob = i;
			break;
		}
	}

	return nJob;
}

void CPaneSimulation::update_list()
{
	GV_ITEM Item;
	Item.mask = GVIF_TEXT | GVIF_FORMAT;
	Item.nFormat = DT_CENTER | DT_WORDBREAK;
	Item.row = 0;
	Item.col = 0;

	UINT state = m_grid_list.GetItemState(0, 0) | GVIS_READONLY;
	m_grid_list.SetItemState(0, 0, state);

	Item.strText = "NO";

	m_grid_list.SetItem(&Item);

	Item.row = 0;
	Item.col = 1;

	state = m_grid_list.GetItemState(0, 1) | GVIS_READONLY;
	m_grid_list.SetItemState(0, 1, state);

	Item.strText = "Panel ID";
	m_grid_list.SetItem(&Item);

	CString strTemp;
	Item.nFormat = DT_LEFT | DT_WORDBREAK;

	//첫번째 행
	int nCol = 0;
	for (int nRow = 1; nRow < m_vtDirPath.size() + 1; nRow++)
	{
		strTemp.Format("%d", nRow);

		if (m_bUseLive)	m_grid_list.SetItemFgColour(nRow, 1, COLOR_DARK_GRAY);
		else			m_grid_list.SetItemFgColour(nRow, 1, COLOR_WHITE);

		state = m_grid_list.GetItemState(nRow, nCol) | GVIS_READONLY;
		m_grid_list.SetItemState(nRow, nCol, state);

		Item.row = nRow;
		Item.col = nCol;
		Item.strText = strTemp;
		m_grid_list.SetItem(&Item);
	}

	//두번째 행
	nCol = 1;
	for (int nRow = 1; nRow < m_vtDirPath.size() + 1; nRow++)
	{
		CString strID;
		get_ID_from_loadimage(m_nJob, m_vtDirPath[nRow - 1], &strID);
		strTemp.Format("%s", strID);

		if (m_bUseLive)	m_grid_list.SetItemFgColour(nRow, 1, COLOR_DARK_GRAY);
		else			m_grid_list.SetItemFgColour(nRow, 1, COLOR_WHITE);

		state = m_grid_list.GetItemState(nRow, nCol) | GVIS_READONLY;
		m_grid_list.SetItemState(nRow, nCol, state);

		Item.row = nRow;
		Item.col = nCol;
		Item.strText = strTemp;
		m_grid_list.SetItem(&Item);
	}

	// 선택된 리스트 표시
	for (int i = 0; i < m_vtDirPath.size(); i++)
	{
		if (m_vtSelectList[i] == TRUE)
		{
			m_grid_list.SetItemBkColour(i + 1, 0, COLOR_BTN_SELECT);
			m_grid_list.SetItemBkColour(i + 1, 1, COLOR_BTN_SELECT);
		}
		else
		{
			m_grid_list.SetItemBkColour(i + 1, 0, COLOR_BTN_BODY);
			m_grid_list.SetItemBkColour(i + 1, 1, COLOR_BTN_BODY);
		}
	}

	current_list(m_nCurrent_list-1);

	m_grid_list.Invalidate();
}

void CPaneSimulation::OnBnClickedBtnLiveSimulation()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	if (m_bUseLive)	m_bUseLive = FALSE;
	else			m_bUseLive = TRUE;

	update_frame_dialog();
}

void CPaneSimulation::OnBnClickedBtnAppendList()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	CString strFilePath;

	SYSTEMTIME time;
	::GetLocalTime(&time);

	CString strTime, str_modelID, str_algo, strImageDir;
	//날짜
	strTime.Format("%04d%02d%02d", time.wYear, time.wMonth, time.wDay);
	strImageDir.Format("%s%s", m_pMain->m_strImageDir, strTime);

	//JOB이름
	str_algo.Format(_T("%s"), m_pMain->vt_job_info[m_nJob].job_name.c_str());
	strImageDir.Format("%s\\%s\\", strImageDir, str_algo);

	//폴더 불러오기
	//CFolderPickerDialog dlg(strImageDir);
	CFolderPickerDialog dlg("");
	if (dlg.DoModal() != IDOK)
	{
		return;
	}
	strFilePath = dlg.GetPathName();

	//폴더 경로 불러오기
	if ( get_directory_path(m_vtDirPath, strFilePath, m_nJob) == FALSE)
	{
		CString str;
		str.Format("Fail get directory from the path.");
		AfxMessageBox(str);
		return;
	}

	//선택된 리스트
	m_vtSelectList.assign(m_vtDirPath.size(), TRUE);

	// 리스트 업데이트
	update_list();
}

void CPaneSimulation::OnBnClickedBtnDeleteList()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	vector < int > vtIndex;

	for (int i = 0; i < m_vtDirPath.size(); i++)
	{
		if (m_vtSelectList[i] == TRUE)
		{
			vtIndex.push_back(i);
		}
	}

	for (int i = (int)vtIndex.size() - 1; i > -1; i--)
	{
		m_vtDirPath.erase(m_vtDirPath.begin() + vtIndex[i]);
		m_vtSelectList.erase(m_vtSelectList.begin() + vtIndex[i]);
	}

	m_grid_list.DeleteAllItems();
	init_grid_list(m_grid_list, 2, 1000);
	update_list();
}

void CPaneSimulation::OnSelchangeComboJob()
{
	m_nJob = m_cmbSelectJob.GetCurSel();
}

void CPaneSimulation::OnBnClickedBtnSimulationStart()
{
	CString str;

	if (m_pMain->m_strResultTime[m_nJob].GetLength() < 10 || m_pMain->m_strResultDate[m_nJob].GetLength() < 6)
	{
		CTime NowTime;
		NowTime = CTime::GetCurrentTime();
		m_pMain->m_strResultDate[m_nJob].Format("%04d%02d%02d", NowTime.GetYear(), NowTime.GetMonth(), NowTime.GetDay());
		m_pMain->m_strResultTime[m_nJob].Format("%02d_%02d_%02d", NowTime.GetHour(), NowTime.GetMinute(), NowTime.GetSecond());
	}

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_bUseLive)
	{
		CFormMainView *pView = (CFormMainView *)m_pMain->m_pForm[FORM_MAIN];
		pView->reset_viewer(m_nJob);
		pView->read_glass_id(m_nJob);

		simulation_live(m_nJob);
		return;
	}

	if (m_vtDirPath.size() > 0)
	{
		int nJob = get_job_from_img(m_vtDirPath[0]);
		if (m_nJob == nJob)
		{
			str.Format("Simulation start");
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_btnSimulationStart.SetEnable(FALSE);
		}
		else
		{
			str.Format("Check Select Job.");
			AfxMessageBox(str);
			return;
		}
	}
	else
	{
		str.Format("Frist append List!");
		AfxMessageBox(str);
		return;
	}

	// MainForm Tab을 해당 Job으로 전환
	if (m_pMain->vt_job_info.size() > 1)
	{
		CFormMainView *pView = (CFormMainView *)m_pMain->m_pForm[FORM_MAIN];
		if (pView->m_TabMainView.GetCurSel() != m_nJob + 1)
		{
			pView->m_TabMainView.SetCurSel(m_nJob + 1);
			int x_cnt = m_pMain->vt_job_info[m_nJob].main_view_count_x;
			int y_cnt = m_pMain->vt_job_info[m_nJob].main_view_count_y;
			pView->replace_viewer_main_fix(x_cnt, y_cnt, m_nJob);
		}
	}

	CPaneAuto* pPaneAuto = (CPaneAuto*)m_pMain->m_pPane[PANE_AUTO];
	int index = m_nJob;
	pPaneAuto->m_TabResultView.SetCurSel(index);
	pPaneAuto->RenewResultChart(index);

	m_pMain->changePane(PANE_AUTO);

	set_simulation_start_stop(TRUE);

	current_list(1);

	int nMethod = m_pMain->vt_job_info[m_nJob].algo_method;
	switch (nMethod)
	{
	case CLIENT_TYPE_PANEL_EXIST_INSP:			simulation_exist_isnp(m_nJob);				break;
	case CLIENT_TYPE_NOZZLE_ALIGN:				simulation_nozzle_isnp(m_nJob);				break;
	case CLIENT_TYPE_SCAN_INSP:					simulation_Scan_isnp(m_nJob);				break;
	case CLIENT_TYPE_CENTER_NOZZLE_ALIGN:		simulation_Center_NozzleAlign(m_nJob);		break;
	case CLIENT_TYPE_CENTER_SIDE_YGAP:			simulation_zgap_insp(m_nJob);				break;
	case CLIENT_TYPE_ELB_CENTER_ALIGN:
	case CLIENT_TYPE_1CAM_1POS_REFERENCE:
	case CLIENT_TYPE_1CAM_1SHOT_FILM:
	case CLIENT_TYPE_1CAM_1SHOT_ALIGN:
	case CLIENT_TYPE_1CAM_1SHOT_ALIGN_WITH_SCRATCH_INSP:
	case CLIENT_TYPE_1CAM_4POS_ROBOT:
	case CLIENT_TYPE_1CAM_2POS_REFERENCE:
	case CLIENT_TYPE_FILM_INSP:					simulation_1cam_1shot_align(m_nJob);		break;
	case CLIENT_TYPE_1CAM_2SHOT_ALIGN:
	default:									simulation_2cam_1shot_align(m_nJob);		break;
	case CLIENT_TYPE_2CAM_EACH_2SHOT_ALIGN:		simulation_2cam_2shot_align(m_nJob);		break;
	case CLIENT_TYPE_4CAM_1SHOT_ALIGN:			simulation_4cam_1shot_align(m_nJob);		break;
	}

	m_pMain->m_nSeqProcess[m_nJob] = 0;
	m_pMain->resetProcessStartFlag(m_nJob);

	str.Format("Simulation End");
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	set_simulation_start_stop(FALSE);
	m_pMain->m_bSimulationStart = FALSE;
	m_btnSimulationStart.SetEnable(TRUE);
	update_frame_dialog();

	CPaneBottomSimulation *pView = (CPaneBottomSimulation *)m_pMain->m_pPaneBottom[B_PANE_SIMULATION];
	pView->m_bSimulStopFlag = TRUE;
	pView->update_frame_dialog();
}

void CPaneSimulation::simulation_live(int nJob)
{
	set_simulation_start_stop(TRUE);

	// MainForm Tab을 해당 Job으로 전환
	if (m_pMain->vt_job_info.size() > 1)
	{
		CFormMainView *pView = (CFormMainView *)m_pMain->m_pForm[FORM_MAIN];
		if (pView->m_TabMainView.GetCurSel() != nJob + 1)
		{
			pView->m_TabMainView.SetCurSel(nJob + 1);
			int x_cnt = m_pMain->vt_job_info[nJob].main_view_count_x;
			int y_cnt = m_pMain->vt_job_info[nJob].main_view_count_y;
			pView->replace_viewer_main_fix(x_cnt, y_cnt, nJob);
		}
	}

	CPaneAuto* pPaneAuto = (CPaneAuto*)m_pMain->m_pPane[PANE_AUTO];
	int index = m_nJob;
	pPaneAuto->m_TabResultView.SetCurSel(index);
	pPaneAuto->RenewResultChart(index);

	m_pMain->changePane(PANE_AUTO);
	
	CString str;;
	str.Format("[%s] Simulation Live Start, ID : %s -----------------------", m_pMain->vt_job_info[nJob].main_object_id.c_str(), m_pMain->vt_job_info[nJob].main_object_id.c_str());
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	// 조명 ON/OFF 시스템일때 시작하기전에 조명 킴
	//if (m_pMain->vt_job_info[nJob].num_of_shot_count == 1)
	{
		//if (m_pMain->vt_system_option[nJob].use_light_on_off_system == TRUE)
			m_pMain->LightUse(m_pMain->LIGHT_ON, nJob);
	}

	int nMethod = m_pMain->vt_job_info[nJob].algo_method;
	switch (nMethod)
	{
		case CLIENT_TYPE_2CAM_EACH_2SHOT_ALIGN:			
		{
			AfxMessageBox("No Use that");
		}
		break;
		default:										simulation_live_default(nJob);			break;
	}

	// 조명 ON/OFF 시스템일때 끝난 후 조명 끔
	if (m_pMain->vt_system_option[nJob].use_light_on_off_system == TRUE)	m_pMain->LightUse(m_pMain->LIGHT_OFF, nJob);

	
	str.Format("[%s] Simulation Live End ----------------------------------------------", m_pMain->vt_job_info[nJob].main_object_id.c_str());
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	set_simulation_start_stop(FALSE);

	m_pMain->m_bSimulationStart = FALSE;

	m_btnSimulationStart.SetEnable(TRUE);

	update_frame_dialog();

	CPaneBottomSimulation *pView = (CPaneBottomSimulation *)m_pMain->m_pPaneBottom[B_PANE_SIMULATION];
	pView->m_bSimulStopFlag = TRUE;
	pView->update_frame_dialog();
}

void CPaneSimulation::simulation_live_default(int nJob)
{
	m_pMain->m_nErrorType[nJob] = 0;
	memset(m_pMain->vt_result_info[nJob].each_Judge, 0, sizeof(m_pMain->vt_result_info[nJob].each_Judge));
	
	SendMessageA(WM_VIEW_CONTROL, MSG_FMV_GLASSID_UPDATE, m_nJob);

	AcquireSRWLockExclusive(&m_pMain->g_srwlock); // 동기화 시킴 Tkyuha 20211027
	m_pMain->m_nSeqProcess[nJob] = 100;
	ReleaseSRWLockExclusive(&m_pMain->g_srwlock);
	m_pMain->setProcessStartFlag(nJob);

	// Process 끝날때 까지 대기.. 10초 이후에는 그냥 끝냄
	if (m_pMain->wait_flag_over_time(10000, &m_pMain->m_bPrealignStartFlag[nJob], TRUE, 0, 1) == -1)
	{
		m_pMain->m_nSeqProcess[nJob] = 0;
		m_pMain->resetProcessStartFlag(nJob);

		CString str;
		str.Format("TimeOut to Wait ProcessFlag Down.");
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
	Delay(50);
}

BOOL CPaneSimulation::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	if (wParam == (WPARAM)m_grid_list.GetDlgCtrlID())
	{
		if (m_bUseLive) return CFormView::OnNotify(wParam, lParam, pResult);

		GV_DISPINFO *pDispInfo = (GV_DISPINFO *)lParam;
		if (pDispInfo->item.row > 0 && pDispInfo->hdr.code == 4294967196)
		{
			int row = pDispInfo->item.row;
			if (m_grid_list.GetItemText(row, 1) != "")
			{
				if (m_vtSelectList[row - 1] == TRUE)
				{
					m_grid_list.SetItemBkColour(row, 0, COLOR_BTN_BODY);
					m_grid_list.SetItemBkColour(row, 1, COLOR_BTN_BODY);
					m_vtSelectList[row - 1] = FALSE;
				}
				else
				{
					m_grid_list.SetItemBkColour(row, 0, COLOR_BTN_SELECT);
					m_grid_list.SetItemBkColour(row, 1, COLOR_BTN_SELECT);
					m_vtSelectList[row - 1] = TRUE;
				}
				m_grid_list.Invalidate();
			}
		}
	}

	return CFormView::OnNotify(wParam, lParam, pResult);
}

int CPaneSimulation::current_list(int pos)
{
	
	for (int i = 1; i < m_vtDirPath.size() + 1; i++)
	{
		m_grid_list.SetItemFgColour(i, 0, COLOR_WHITE);
		m_grid_list.SetItemFgColour(i, 1, COLOR_WHITE);
	}

	int row = pos + 1;
	m_grid_list.SetItemFgColour(row, 0, COLOR_RED);
	m_grid_list.SetItemFgColour(row, 1, COLOR_RED);
	m_grid_list.Invalidate();
	return row;
}

void CPaneSimulation::OnBnClickedBtnReverseSelcet()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	for (int i = 0; i < m_vtSelectList.size(); i++)
	{
		if (m_vtSelectList[i] == TRUE)	m_vtSelectList[i] = FALSE;
		else							m_vtSelectList[i] = TRUE;
	}
	update_list();
}
