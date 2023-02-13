#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "DlgList.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(CDlgList, CDialogEx)

CDlgList::CDlgList(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgList::IDD, pParent)
{
	m_pMain = NULL;
	m_nJob = 0;
}

CDlgList::~CDlgList()
{
	list_clear();
}

void CDlgList::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LB_LIST_TITLE, m_lbTitle);
	DDX_Control(pDX, IDC_GRID_LIST, m_grid_list);
	DDX_Control(pDX, IDC_DATETIMEPICKER_LIST, m_dateTimeCtrl);
}

BEGIN_MESSAGE_MAP(CDlgList, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER_LIST, &CDlgList::OnDatetimechangeDatetimepickerList)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

// CDlgList 메시지 처리기입니다.

HBRUSH CDlgList::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{	
	return m_hbrBkg;
}

void CDlgList::InitTitle(CLabelEx *pTitle,CString str,float size,COLORREF color)
{
	pTitle->SetAlignTextCT();
	pTitle->SetOffsetText(10, 10);
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}

void CDlgList::MainButtonInit(CButtonEx *pbutton,int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
//	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}

BOOL CDlgList::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);
	m_pMain = (CLET_AlignClientDlg*)AfxGetMainWnd();

	InitTitle( &m_lbTitle, "", 20.f, COLOR_BTN_BODY );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

BOOL CDlgList::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (wParam == (WPARAM)m_grid_list.GetDlgCtrlID())
	{
		GV_DISPINFO* pDispInfo = (GV_DISPINFO*)lParam;

		int row = pDispInfo->item.row;
		int col = pDispInfo->item.col;

		// Click : 4294967196
		if( pDispInfo->hdr.code == 4294967196)
		{
			if (col == 3 && row > 0 )
			{
				if (strcmp(m_grid_list.GetItemText(row, col),"O") == 0)
				{
					SHELLEXECUTEINFO sei = { sizeof(sei) };
					sei.lpVerb = "open";
					//2022.07.06 ksm Vector 분리
					if(!m_vt_TOTAL_CA.empty())
						sei.lpFile = m_vt_TOTAL_CA[pDispInfo->item.row - 1].strImagePath;
					else
						sei.lpFile = m_vt_TOTAL[pDispInfo->item.row - 1].strImagePath;

					sei.hwnd = NULL;
					sei.lpParameters = NULL;
					sei.nShow = SW_NORMAL;

					if (!ShellExecuteEx(&sei))
					{
						DWORD dwError = GetLastError();
						if (dwError == ERROR_CANCELLED)
							return CDialogEx::OnNotify(wParam, lParam, pResult);
					}
					else
					{
						return CDialogEx::OnNotify(wParam, lParam, pResult);
					}
				}
			}
			if (col == 4 && row > 0)
			{
				if (strcmp(m_grid_list.GetItemText(row, col), "O") == 0)
				{
					SHELLEXECUTEINFO sei = { sizeof(sei) };
					sei.lpVerb = "open";
					if (!m_vt_TOTAL_CA.empty())
						sei.lpFile = m_vt_TOTAL_CA[pDispInfo->item.row - 1].strCenterVideoPath;
					else
						sei.lpFile = m_vt_TOTAL[pDispInfo->item.row - 1].strVideoPath;
					
					sei.hwnd = NULL;
					sei.lpParameters = NULL;
					sei.nShow = SW_NORMAL;

					if (!ShellExecuteEx(&sei))
					{
						DWORD dwError = GetLastError();
						if (dwError == ERROR_CANCELLED)
							return CDialogEx::OnNotify(wParam, lParam, pResult);
					}
					else
					{
						return CDialogEx::OnNotify(wParam, lParam, pResult);
					}
				}
			}
			if (col == 5 && row > 0)
			{
				if (strcmp(m_grid_list.GetItemText(row, col), "O") == 0)
				{
					SHELLEXECUTEINFO sei = { sizeof(sei) };
					sei.lpVerb = "open";
					sei.lpFile = m_vt_TOTAL_CA[pDispInfo->item.row - 1].str45VideoPath;
					sei.hwnd = NULL;
					sei.lpParameters = NULL;
					sei.nShow = SW_NORMAL;

					if (!ShellExecuteEx(&sei))
					{
						DWORD dwError = GetLastError();
						if (dwError == ERROR_CANCELLED)
							return CDialogEx::OnNotify(wParam, lParam, pResult);
					}
					else
					{
						return CDialogEx::OnNotify(wParam, lParam, pResult);
					}
				}
			}
		}
	}

	return CDialogEx::OnNotify(wParam, lParam, pResult);
}


void CDlgList::list_clear()
{
	m_grid_list.DeleteAllItems();

	m_vt_OK_CA.clear();
	m_vt_NG_CA.clear();
	m_vt_TOTAL_CA.clear();

	m_vt_OK.clear();
	m_vt_NG.clear();
	m_vt_TOTAL.clear();
}

void CDlgList::get_ID_from_loadimage(int nJob, CString strFilePath, CString* strID)
{
	int nLength = strFilePath.GetLength();
	int nFind = strFilePath.ReverseFind(_T('\\'));
	*strID = strFilePath.Right(nLength - nFind - 1);
}

void CDlgList::get_file_create_Day(CString strFilePath, CString* strDay)
{
	CString strCreateDay;
	struct _stat buf;
	struct tm* time;

	_stat(strFilePath, &buf);
	time = localtime(&buf.st_ctime);
	strCreateDay.Format("%04d%02d%02d", time->tm_year, time->tm_mon, time->tm_mday);
	*strDay = strCreateDay;
}

void CDlgList::get_file_create_time(CString strFilePath, CString* strTime)
{
	CString strCreateTime;
	struct _stat buf;
	struct tm* time;

	_stat(strFilePath, &buf);
	time = localtime(&buf.st_ctime);
	strCreateTime.Format("%02d:%02d:%02d", time->tm_hour, time->tm_min, time->tm_sec);
	*strTime = strCreateTime;
}

#pragma region CenterAlign 
// ========================= Center Align =====================================
void CDlgList::update_list_centeralign(WORD year, WORD month, WORD day)
{
	// vecter 초기화
	list_clear();

	// grid 초기화
	init_grid_centeralign(m_grid_list);

	// 첫째줄 표시
	display_list_headline_cetneralign();
	
	// 제목 표시
	int nJob = 0;
	CString strJob = m_pMain->vt_job_info[nJob].job_name.c_str();
	InitTitle(&m_lbTitle, strJob, 20.f, COLOR_BTN_BODY);

	CString str;
	CString OkPath;
	CString NGPath;

	// Center 이미지 경로 찾기
	get_image_directory_centeralign(&OkPath, true, nJob , year, month, day);
	get_image_directory_centeralign(&NGPath, false, nJob, year, month, day);

	// OK 폴더 에서 List 요소 찾기
	get_list_item_centeralign(m_vt_OK_CA, OkPath, true, nJob);
	if (m_vt_OK_CA.size() == 0)
	{
		str.Format("Empty OK List on today.");
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}

	// NG 폴더 에서 List 요소 찾기
	get_list_item_centeralign(m_vt_NG_CA, NGPath, false, nJob);
	if (m_vt_NG_CA.size() == 0)
	{
		str.Format("Empty NG List on today.");
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}

	// 없으면 리턴
	if ( m_vt_OK_CA.size() == 0 && m_vt_NG_CA.size() == 0) return;

	if (m_vt_OK_CA.size() > 0) sort_centeralign_list(m_vt_OK_CA);
	if (m_vt_NG_CA.size() > 0) sort_centeralign_list(m_vt_NG_CA);

	// OK 폴더 NG 폴더 찾은 List 합치기
	if ( m_vt_OK_CA.size() > 0 ) for (int i = 0; i < m_vt_OK_CA.size(); i++) m_vt_TOTAL_CA.push_back(m_vt_OK_CA[i]);
	if ( m_vt_NG_CA.size() > 0 ) for (int i = 0; i < m_vt_NG_CA.size(); i++) m_vt_TOTAL_CA.push_back(m_vt_NG_CA[i]);
	
	// 벡터 생성날짜 기준으로 내림차순 정렬
	sort_centeralign_list(m_vt_TOTAL_CA);

	// 리스트 업데이트
	display_list_update_centeralign(m_vt_TOTAL_CA);
}

void CDlgList::init_grid_centeralign(CGridCtrl& grid)
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
	grid.SetRowCount(MAX_LIST);
	grid.SetColumnCount(6);
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
	grid_width = (int)(rect.Width() / (double)2);

	//Grid 행 사이즈 조정
	for (int nCol = 0; nCol < grid.GetColumnCount(); nCol++)
	{
		switch (nCol)
		{
		case 0:		grid.SetColumnWidth(nCol, 50);							break;
		case 1:		grid.SetColumnWidth(nCol, 100);							break;
		case 2:		grid.SetColumnWidth(nCol, 250);							break;
		default:	grid.SetColumnWidth(nCol, 80);							break;
		}
	}

	//Grid 열 사이즈 조정
	for (int nRow = 0; nRow < grid.GetRowCount(); nRow++)
		grid.SetRowHeight(nRow, 40);
}

void CDlgList::get_image_directory_centeralign(CString* strImagePath, bool bJudge, int nJob,WORD year,WORD month, WORD day)
{
	SYSTEMTIME time;
	::GetLocalTime(&time);

	CString str_modelID;
	CString str_algo;
	CString strImageDir;
	CString strDate;

	if (year == 0 || month==0 || day==0)
	{
		year = time.wYear;
		month = time.wMonth;
		day = time.wDay;
	}

	// KBJ 2022-09-16
	m_Year = int(year);
	m_Month = int(month);
	m_Day = int(day);

	//JOB이름
	str_algo.Format("%s", m_pMain->vt_job_info[nJob].job_name.c_str());
	strImageDir.Format("%s\\%s\\", strImageDir, str_algo);

	//날짜
	strDate.Format("%04d%02d%02d", year, month, day);
	strImageDir.Format("%s%s", m_pMain->m_strImageDir, strDate);
	if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);

	//모델ID
	str_modelID.Format("%s", m_pMain->vt_job_info[nJob].model_info.getModelID());
	strImageDir.Format("%s\\%s", strImageDir, str_modelID);
	if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);

	//JOB이름
	str_algo.Format("%s", m_pMain->vt_job_info[nJob].job_name.c_str());
	strImageDir.Format("%s\\%s", strImageDir, str_algo);
	if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);

	//Align
	strImageDir.Format("%s\\Diff_Insp", strImageDir, str_algo);
	if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);

	if (bJudge == true)
	{
		//OK
		strImageDir.Format("%s\\OK", strImageDir, str_algo);
		if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);
	}
	else
	{
		//NG
		strImageDir.Format("%s\\NG", strImageDir, str_algo);
		if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);
	}

	*strImagePath = strImageDir;
}

void CDlgList::get_list_item_centeralign(std::vector < CenterAlign_List_Item >& vt_List, CString strPath,bool bJudge, int nJob)
{
	CString str;
	CFileFind Finder;
	BOOL bFind = FALSE;

	bFind = Finder.FindFile(strPath + _T("\\*.*"));
	while (bFind)
	{
		bFind = Finder.FindNextFile();
		
		if (Finder.IsDots() || !Finder.IsDirectory())	// '.', '..', '파일' 인 경우 continue;
			continue;

		if (Finder.IsDirectory())	//폴더
		{
			// KBJ 2022-07-27 리스트 아이템 읽는속도 줄이기
			BOOL bChange = FALSE;

			CenterAlign_List_Item List_Item;
			CString strImageFilePath, strVideoFilePath;
			List_Item.strPanelID = "-";
			List_Item.strTime = "-";
			List_Item.strCenterVideoPath = "-";
			List_Item.str45VideoPath = "-";
			List_Item.strImagePath = "-";
			List_Item.bJudge = bJudge;

			strImageFilePath = Finder.GetFilePath();											// 이미지 패널 디렉토리 경로

			get_file_create_time(strImageFilePath, &List_Item.strTime);							// 패널 디렉토리 생성 시간
			if (vt_List.size() > MAX_LIST)
			{
				if (strcmp(vt_List[MAX_LIST].strTime, List_Item.strTime) >= 0)			  continue;
				else															bChange = TRUE;
			}

			get_ID_from_loadimage(nJob, strImageFilePath, &List_Item.strPanelID);				// 패널 디렉토리 이름
			get_list_image_path_centeralign(strImageFilePath, &List_Item.strImagePath);			// 패널 이미지 경로
			get_list_center_video_path(List_Item.strPanelID, &List_Item.strCenterVideoPath);    // 패널 비디오 경로
			get_list_45_video_path(List_Item.strPanelID, &List_Item.str45VideoPath);			// 패널 비디오 경로
			if (List_Item.strPanelID != "-")
			{
				if(vt_List.size() <= MAX_LIST)
				{
					vt_List.push_back(List_Item);

					if (vt_List.size() % MAX_LIST == 0)
					{
						sort_centeralign_list(vt_List);
					}
				}
				else
				{ 
					if (bChange == TRUE)
					{
						vt_List.push_back(List_Item);
						sort_centeralign_list(vt_List);
						vt_List.pop_back();
					}
				}
			}
			else continue;
		}
	}
}

void CDlgList::get_list_image_path_centeralign(CString strImageDirPath, CString* strImagePath)
{
	CString fname;
	BOOL bFind = FALSE;
	CFileFind findImage;

	bFind = findImage.FindFile(strImageDirPath + _T("\\*.*"));
	try {
		while (bFind)
		{
			bFind = findImage.FindNextFile();
			
			if (findImage.IsDots() || findImage.IsDirectory())	// '.', '..', '파일' 인 경우 continue;
				continue;

			fname = findImage.GetFilePath();	// 찾은 Image

			CString str;
			str.Format("_result");
			if (fname.Find(str) > 0)
			{
				*strImagePath = fname;
				return;
			}
		}
	}
	catch (...)
	{
		CString str;
		str.Format("Did catch the error to get_center_Image_path");
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		str.Format("%s", strImageDirPath + _T("\\*.*"));
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
}

void CDlgList::get_list_center_video_path(CString strPanelID, CString* strVideoPath)
{
	// D:\LET_AlignClient\Result\Video\날짜\PanelID
	CString strVideoDir, strDate;
	strDate.Format("%04d%02d%02d", m_Year, m_Month, m_Day);
	strVideoDir.Format("%s%s\\%s", m_pMain->m_strVideoDir, strDate, strPanelID);

	CString fname;
	BOOL bFind = FALSE;
	CFileFind findVideo;

	bFind = findVideo.FindFile(strVideoDir + _T("\\*.*"));

	try {
		while (bFind)
		{
			bFind = findVideo.FindNextFile();

			if (findVideo.IsDots() || findVideo.IsDirectory())	// '.', '..', '파일' 인 경우 continue;
				continue;

			fname = findVideo.GetFilePath();	// 찾은 Video

			CString str;
			str.Format("CENTER");
			if (fname.Find(str) > 0)
			{
				*strVideoPath = fname;
				// KBJ 2022-11-30 가장 마지막에 찾은 걸로 되도록 수정
				//return;
			}
		}
	}
	catch (...)
	{
		CString str;
		str.Format("Did catch the error to get_45_Viedo_path");
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		str.Format("%s", strVideoDir + _T("\\*.*"));
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
}

void CDlgList::get_list_45_video_path(CString strPanelID, CString* strVideoPath)
{
	// D:\LET_AlignClient\Result\Video\날짜\PanelID
	CString strVideoDir, strDate;
	strDate.Format("%04d%02d%02d", m_Year, m_Month, m_Day);
	strVideoDir.Format("%s%s\\%s", m_pMain->m_strVideoDir, strDate, strPanelID);

	CString fname;
	BOOL bFind = FALSE;
	CFileFind findVideo;

	bFind = findVideo.FindFile(strVideoDir + _T("\\*.*"));

	try {
		while (bFind)
		{
			bFind = findVideo.FindNextFile();

			if (findVideo.IsDots() || findVideo.IsDirectory())	// '.', '..', '파일' 인 경우 continue;
				continue;

			fname = findVideo.GetFilePath();	// 찾은 Video

			CString str;
			// KBJ 2022-07-23 찾는 비디오 이름 NOZZLE# -> NOZZLE로 변경
			str.Format("NOZZLE");
			if (fname.Find(str) > 0)
			{
				*strVideoPath = fname;
				// KBJ 2022-11-30 가장 마지막에 찾은 걸로 되도록 수정
				// return;
			}
		}
	}
	catch (...)
	{
		CString str;
		str.Format("Did catch the error to get_Viedo_path");
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		str.Format("%s", strVideoDir + _T("\\*.*"));
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
}

void CDlgList::display_list_update_centeralign(std::vector < CenterAlign_List_Item >& vt_List)
{
	CString strTemp;
	GV_ITEM Item;
	Item.mask = GVIF_TEXT | GVIF_FORMAT;
	Item.nFormat = DT_CENTER | DT_WORDBREAK;

	Item.row = 0;
	Item.col = 0;
	UINT state = m_grid_list.GetItemState(0, 0) | GVIS_READONLY;

	//첫번째 행
	int nCol = 0;
	for (int nRow = 1; nRow < vt_List.size() + 1; nRow++)
	{
		if (nRow == MAX_LIST) break;

		state = m_grid_list.GetItemState(nRow, nCol) | GVIS_READONLY;
		m_grid_list.SetItemState(nRow, nCol, state);

		CString strNumber;
		strNumber.Format("%d", nRow);
		Item.row = nRow;
		Item.col = nCol;
		Item.strText = strNumber;
		m_grid_list.SetItem(&Item);
	}
	
	//두번째 행
	nCol = 1;
	for (int nRow = 1; nRow < vt_List.size()+1; nRow++)
	{
		if (nRow == MAX_LIST) break;

		state = m_grid_list.GetItemState(nRow, nCol) | GVIS_READONLY;
		m_grid_list.SetItemState(nRow, nCol, state);

		Item.row = nRow;
		Item.col = nCol;
		Item.strText = vt_List[nRow - 1].strTime;
		m_grid_list.SetItem(&Item);
	}

	//세번째 행
	nCol = 2;
	for (int nRow = 1; nRow < vt_List.size() + 1; nRow++)
	{
		if (nRow == MAX_LIST) break;

		if (vt_List[nRow - 1].bJudge)	m_grid_list.SetItemFgColour(nRow, nCol, COLOR_WHITE);
		else							m_grid_list.SetItemFgColour(nRow, nCol, COLOR_RED);

		state = m_grid_list.GetItemState(nRow, nCol) | GVIS_READONLY;
		m_grid_list.SetItemState(nRow, nCol, state);

		Item.row = nRow;
		Item.col = nCol;
		Item.strText = vt_List[nRow - 1].strPanelID;
		m_grid_list.SetItem(&Item);
	}

	//네번째 행
	nCol = 3;
	for (int nRow = 1; nRow < vt_List.size() + 1; nRow++)
	{
		if (nRow == MAX_LIST) break;

		state = m_grid_list.GetItemState(nRow, nCol) | GVIS_READONLY;
		m_grid_list.SetItemState(nRow, nCol, state);

		Item.row = nRow;
		Item.col = nCol;
		if (vt_List[nRow - 1].strImagePath.GetLength() > 5)		  Item.strText = "O";
		else													  Item.strText = "X";
		m_grid_list.SetItem(&Item);
	}

	//다섯번째 행
	nCol = 4;
	for (int nRow = 1; nRow < vt_List.size() + 1; nRow++)
	{
		if (nRow == MAX_LIST) break;

		state = m_grid_list.GetItemState(nRow, nCol) | GVIS_READONLY;
		m_grid_list.SetItemState(nRow, nCol, state);

		Item.row = nRow;
		Item.col = nCol;
		if (vt_List[nRow - 1].strCenterVideoPath.GetLength() > 5) Item.strText = "O";
		else														Item.strText = "X";
		m_grid_list.SetItem(&Item);
	}

	//여섯번째 행
	nCol = 5;
	for (int nRow = 1; nRow < vt_List.size() + 1; nRow++)
	{
		if (nRow == MAX_LIST) break;

		state = m_grid_list.GetItemState(nRow, nCol) | GVIS_READONLY;
		m_grid_list.SetItemState(nRow, nCol, state);

		Item.row = nRow;
		Item.col = nCol;
		if (vt_List[nRow - 1].str45VideoPath.GetLength() > 5)	Item.strText = "O";
		else													Item.strText = "X";
		m_grid_list.SetItem(&Item);
	}

	m_grid_list.Invalidate();
}

void CDlgList::display_list_headline_cetneralign()
{
	GV_ITEM Item;
	Item.mask = GVIF_TEXT | GVIF_FORMAT;
	Item.nFormat = DT_CENTER | DT_WORDBREAK;

	Item.row = 0;
	Item.col = 0;
	UINT state = m_grid_list.GetItemState(0, 0) | GVIS_READONLY;
	m_grid_list.SetItemState(0, 0, state);
	Item.strText = "No";
	m_grid_list.SetItem(&Item);

	Item.row = 0;
	Item.col = 1;
	state = m_grid_list.GetItemState(0, 0) | GVIS_READONLY;
	m_grid_list.SetItemState(0, 0, state);
	Item.strText = "Time";
	m_grid_list.SetItem(&Item);

	Item.row = 0;
	Item.col = 2;
	state = m_grid_list.GetItemState(0, 1) | GVIS_READONLY;
	m_grid_list.SetItemState(0, 1, state);
	Item.strText = "Panel ID";
	m_grid_list.SetItem(&Item);

	Item.row = 0;
	Item.col = 3;
	state = m_grid_list.GetItemState(0, 1) | GVIS_READONLY;
	m_grid_list.SetItemState(0, 1, state);
	Item.strText = "Image Exist";
	m_grid_list.SetItem(&Item);

	Item.row = 0;
	Item.col = 4;
	state = m_grid_list.GetItemState(0, 1) | GVIS_READONLY;
	m_grid_list.SetItemState(0, 1, state);
	Item.strText = "Center Video";
	m_grid_list.SetItem(&Item);

	Item.row = 0;
	Item.col = 5;
	state = m_grid_list.GetItemState(0, 1) | GVIS_READONLY;
	m_grid_list.SetItemState(0, 1, state);
	Item.strText = "#45 Video";
	m_grid_list.SetItem(&Item);

	m_grid_list.Invalidate();
}

void CDlgList::sort_centeralign_list(std::vector < CenterAlign_List_Item >&vt_List)
{
	CenterAlign_List_Item temp;

	for (unsigned int n = 0; n < vt_List.size() - 1; n++)
	{
		for (unsigned int j = n + 1; j < vt_List.size(); j++)
		{
			if (strcmp(vt_List[n].strTime, vt_List[j].strTime) <= 0) // 내림차순
			{
				temp = vt_List[n];
				vt_List[n]= vt_List[j];
				vt_List[j]= temp;
			}
		}
	}
}
// =============================================================================
#pragma endregion


#pragma region Align 
// ========================= Job Align =====================================
void CDlgList::update_list_align(int nJob, WORD year, WORD month, WORD day)
{
	m_nJob = nJob;

	// vecter 초기화
	list_clear();

	// grid 초기화
	init_grid_centeralign(m_grid_list);

	// 첫째줄 표시
	display_list_headline_align();

	// 제목 표시
	CString strJob = m_pMain->vt_job_info[nJob].job_name.c_str();
	InitTitle(&m_lbTitle, strJob, 20.f, COLOR_BTN_BODY);

	CString str;
	CString OkPath;
	CString NGPath;

	// 이미지 경로 찾기
	get_image_directory_align(&OkPath, true, nJob, year, month, day);
	get_image_directory_align(&NGPath, false, nJob, year, month, day);

	// OK 폴더 에서 List 요소 찾기
	get_list_item_align(m_vt_OK, OkPath, true, nJob);
	if (m_vt_OK.size() == 0)
	{
		str.Format("Empty OK List on today.");
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}

	// OK 폴더 에서 List 요소 찾기
	get_list_item_align(m_vt_NG, NGPath, false, nJob);
	if (m_vt_NG.size() == 0)
	{
		str.Format("Empty NG List on tdoay.");
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}

	// 없으면 리턴
	if (m_vt_OK.size() == 0 && m_vt_NG.size() == 0) return;

	if (m_vt_OK.size() > 0) sort_align_list(m_vt_OK);
	if (m_vt_NG.size() > 0) sort_align_list(m_vt_NG);

	// OK 폴더 NG 폴더 찾은 List 합치기
	if (m_vt_OK.size() > 0) for (int i = 0; i < m_vt_OK.size(); i++) m_vt_TOTAL.push_back(m_vt_OK[i]);
	if (m_vt_NG.size() > 0) for (int i = 0; i < m_vt_NG.size(); i++) m_vt_TOTAL.push_back(m_vt_NG[i]);

	// 벡터 생성날짜 기준으로 내림차순 정렬
	sort_align_list(m_vt_TOTAL);

	// 리스트 업데이트
	display_list_update_align(m_vt_TOTAL);
}

void CDlgList::get_image_directory_align(CString* strImagePath, bool bJudge, int nJob, WORD year, WORD month, WORD day)
{
	SYSTEMTIME time;
	::GetLocalTime(&time);

	CString str_modelID;
	CString str_algo;
	CString strImageDir;
	CString strDate;

	if (year == 0 || month == 0 || day == 0)
	{
		year = time.wYear;
		month = time.wMonth;
		day = time.wDay;
	}

	// KBJ 2022-09-16
	m_Year = int(year);
	m_Month = int(month);
	m_Day = int(day);

	//JOB이름
	str_algo.Format("%s", m_pMain->vt_job_info[nJob].job_name.c_str());
	strImageDir.Format("%s\\%s\\", strImageDir, str_algo);

	//날짜
	strDate.Format("%04d%02d%02d", year, month, day);
	strImageDir.Format("%s%s", m_pMain->m_strImageDir, strDate);
	if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);

	//모델ID
	str_modelID.Format("%s", m_pMain->vt_job_info[nJob].model_info.getModelID());
	strImageDir.Format("%s\\%s", strImageDir, str_modelID);
	if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);

	//JOB이름
	str_algo.Format("%s", m_pMain->vt_job_info[nJob].job_name.c_str());
	strImageDir.Format("%s\\%s", strImageDir, str_algo);
	if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);

	if (bJudge == true)
	{
		//OK
		strImageDir.Format("%s\\OK", strImageDir, str_algo);
		if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);
	}
	else
	{
		//NG
		strImageDir.Format("%s\\NG", strImageDir, str_algo);
		if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);
	}

	*strImagePath = strImageDir;
}

void CDlgList::get_list_item_align(std::vector < Align_List_Item >& vt_List, CString strPath, bool bJudge, int nJob)
{
	CString str;
	CFileFind Finder;
	BOOL bFind = FALSE;

	bFind = Finder.FindFile(strPath + _T("\\*.*"));
	while (bFind)
	{
		bFind = Finder.FindNextFile();

		if (Finder.IsDots() || !Finder.IsDirectory())	// '.', '..', '파일' 인 경우 continue;
			continue;

		if (Finder.IsDirectory())	//폴더
		{
			// KBJ 2022-07-27 리스트 아이템 읽는속도 줄이기
			BOOL bChange = FALSE;

			Align_List_Item List_Item;
			CString strImageFilePath, strVideoFilePath;
			List_Item.strPanelID = "-";
			List_Item.strTime = "-";
			List_Item.strVideoPath = "-";
			List_Item.strImagePath = "-";
			List_Item.bJudge = bJudge;

			strImageFilePath = Finder.GetFilePath();										// 이미지 패널 디렉토리 경로

			get_file_create_time(strImageFilePath, &List_Item.strTime);							// 패널 디렉토리 생성 시간
			get_ID_from_loadimage(nJob, strImageFilePath, &List_Item.strPanelID);			// 패널 디렉토리 이름

			if (vt_List.size() > MAX_LIST)
			{
				if (strcmp(vt_List[MAX_LIST].strTime, List_Item.strTime) >= 0)			continue;
				else																	bChange = TRUE;
			}
			
			get_list_image_path_align(strImageFilePath, &List_Item.strImagePath, nJob);		// 패널 이미지 경로
			get_list_video_path(List_Item.strPanelID, &List_Item.strVideoPath, nJob);		// 패널 비디오 경로
			
			if (List_Item.strPanelID != "-")
			{
				if (vt_List.size() <= MAX_LIST)
				{
					vt_List.push_back(List_Item);

					if (vt_List.size() % MAX_LIST == 0)
					{
						sort_align_list(vt_List);
					}
				}
				else
				{
					if (bChange == TRUE)
					{
						vt_List.push_back(List_Item);
						sort_align_list(vt_List);
						vt_List.pop_back();
					}
				}
			}
			else continue;
		}
	}
}

void CDlgList::get_list_image_path_align(CString strImageDirPath, CString* strImagePath,int nJob)
{
	CString fname;
	BOOL bFind = FALSE;
	CFileFind findImage;

	CString sJobName, sJobType;
	sJobName.Format("%s", m_pMain->vt_job_info[nJob].job_name.c_str());

	if (m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_FILM_INSP)
		sJobType = sJobName.Mid(sJobName.GetLength() - 12, 4); // Attach_Inspection-1  Total Count-22 Find 'INSP' 
	else
		sJobType = sJobName.Left(1);

	bFind = findImage.FindFile(strImageDirPath + _T("\\*.*"));
	try {
		while (bFind)
		{
			bFind = findImage.FindNextFile();

			if (findImage.IsDots() || findImage.IsDirectory())	// '.', '..', '파일' 인 경우 continue;
				continue;

			fname = findImage.GetFilePath();	// 찾은 Image

			if (fname.Find("_"+sJobType) > 0)
			{
				*strImagePath = fname;
				return;
			}
		}
	}
	catch (...)
	{
		CString str;
		str.Format("Did catch the error to get_Image_path");
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		str.Format("%s", strImageDirPath + _T("\\*.*"));
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
}

void CDlgList::get_list_video_path(CString strPanelID, CString* strVideoPath, int nJob)
{
	SYSTEMTIME time;
	::GetLocalTime(&time);

	// D:\LET_AlignClient\Result\Video\날짜\PanelID
	CString strVideoDir, strDate;
	strDate.Format("%04d%02d%02d", time.wYear, time.wMonth, time.wDay);
	strVideoDir.Format("%s%s\\%s", m_pMain->m_strVideoDir, strDate, strPanelID);

	CString fname;
	BOOL bFind = FALSE;
	CFileFind findVideo;

	bFind = findVideo.FindFile(strVideoDir + _T("\\*.*"));

	CString sJobName, sJobType;
	sJobName.Format("%s", m_pMain->vt_job_info[nJob].job_name.c_str());

	if (m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_FILM_INSP)
		sJobType = sJobName.Mid(sJobName.GetLength() - 12, 4);// Attach_Inspection-1  Total Count-22 Find 'INSP' 
	else
		sJobType = sJobName.Left(1);

	try {
		while (bFind)
		{
			bFind = findVideo.FindNextFile();

			if (findVideo.IsDots() || findVideo.IsDirectory())	// '.', '..', '파일' 인 경우 continue;
				continue;

			fname = findVideo.GetFilePath();	// 찾은 Video

			if (fname.Find("_" + sJobType) > 0)
			{
				*strVideoPath = fname;
				// KBJ 2022-11-30 가장 마지막에 찾은 걸로 되도록 수정
				// return;
			}
		}
	}
	catch (...)
	{
		CString str;
		str.Format("Did catch the error to get_Viedo_path");
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		str.Format("%s", strVideoDir + _T("\\*.*"));
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
}

void CDlgList::display_list_update_align(std::vector < Align_List_Item >& vt_List)
{
	CString strTemp;
	GV_ITEM Item;
	Item.mask = GVIF_TEXT | GVIF_FORMAT;
	Item.nFormat = DT_CENTER | DT_WORDBREAK;

	Item.row = 0;
	Item.col = 0;
	UINT state = m_grid_list.GetItemState(0, 0) | GVIS_READONLY;

	//첫번째 행
	int nCol = 0;
	for (int nRow = 1; nRow < vt_List.size() + 1; nRow++)
	{
		if (nRow == MAX_LIST) break;

		state = m_grid_list.GetItemState(nRow, nCol) | GVIS_READONLY;
		m_grid_list.SetItemState(nRow, nCol, state);

		CString strNumber;
		strNumber.Format("%d", nRow);
		Item.row = nRow;
		Item.col = nCol;
		Item.strText = strNumber;
		m_grid_list.SetItem(&Item);
	}

	//두번째 행
	nCol = 1;
	for (int nRow = 1; nRow < vt_List.size() + 1; nRow++)
	{
		if (nRow == MAX_LIST) break;

		state = m_grid_list.GetItemState(nRow, nCol) | GVIS_READONLY;
		m_grid_list.SetItemState(nRow, nCol, state);

		Item.row = nRow;
		Item.col = nCol;
		Item.strText = vt_List[nRow - 1].strTime;
		m_grid_list.SetItem(&Item);
	}

	//세번째 행
	nCol = 2;
	for (int nRow = 1; nRow < vt_List.size() + 1; nRow++)
	{
		if (nRow == MAX_LIST) break;

		if (vt_List[nRow - 1].bJudge)	m_grid_list.SetItemFgColour(nRow, nCol, COLOR_WHITE);
		else							m_grid_list.SetItemFgColour(nRow, nCol, COLOR_RED);

		state = m_grid_list.GetItemState(nRow, nCol) | GVIS_READONLY;
		m_grid_list.SetItemState(nRow, nCol, state);

		Item.row = nRow;
		Item.col = nCol;
		Item.strText = vt_List[nRow - 1].strPanelID;
		m_grid_list.SetItem(&Item);
	}

	//네번째 행
	nCol = 3;
	for (int nRow = 1; nRow < vt_List.size() + 1; nRow++)
	{
		if (nRow == MAX_LIST) break;

		state = m_grid_list.GetItemState(nRow, nCol) | GVIS_READONLY;
		m_grid_list.SetItemState(nRow, nCol, state);

		Item.row = nRow;
		Item.col = nCol;
		if (vt_List[nRow - 1].strImagePath.GetLength() > 5) Item.strText = "O";
		else												Item.strText = "X";
		m_grid_list.SetItem(&Item);
	}

	//다섯번째 행
	nCol = 4;
	for (int nRow = 1; nRow < vt_List.size() + 1; nRow++)
	{
		if (nRow == MAX_LIST) break;

		state = m_grid_list.GetItemState(nRow, nCol) | GVIS_READONLY;
		m_grid_list.SetItemState(nRow, nCol, state);

		Item.row = nRow;
		Item.col = nCol;
		if (vt_List[nRow - 1].strVideoPath.GetLength() > 5) Item.strText = "O";
		else														Item.strText = "X";
		m_grid_list.SetItem(&Item);
	}
	m_grid_list.Invalidate();
}

void CDlgList::display_list_headline_align()
{
	GV_ITEM Item;
	Item.mask = GVIF_TEXT | GVIF_FORMAT;
	Item.nFormat = DT_CENTER | DT_WORDBREAK;

	Item.row = 0;
	Item.col = 0;
	UINT state = m_grid_list.GetItemState(0, 0) | GVIS_READONLY;
	m_grid_list.SetItemState(0, 0, state);
	Item.strText = "No";
	m_grid_list.SetItem(&Item);

	Item.row = 0;
	Item.col = 1;
	state = m_grid_list.GetItemState(0, 0) | GVIS_READONLY;
	m_grid_list.SetItemState(0, 0, state);
	Item.strText = "Time";
	m_grid_list.SetItem(&Item);

	Item.row = 0;
	Item.col = 2;
	state = m_grid_list.GetItemState(0, 1) | GVIS_READONLY;
	m_grid_list.SetItemState(0, 1, state);
	Item.strText = "Panel ID";
	m_grid_list.SetItem(&Item);

	Item.row = 0;
	Item.col = 3;
	state = m_grid_list.GetItemState(0, 1) | GVIS_READONLY;
	m_grid_list.SetItemState(0, 1, state);
	Item.strText = "Image Exist";
	m_grid_list.SetItem(&Item);

	Item.row = 0;
	Item.col = 4;
	state = m_grid_list.GetItemState(0, 1) | GVIS_READONLY;
	m_grid_list.SetItemState(0, 1, state);
	Item.strText = "Video";
	m_grid_list.SetItem(&Item);

	m_grid_list.Invalidate();
}

void CDlgList::sort_align_list(std::vector < Align_List_Item >& vt_List)
{
	Align_List_Item temp;

	for (unsigned int n = 0; n < vt_List.size() - 1; n++)
	{
		for (unsigned int j = n + 1; j < vt_List.size(); j++)
		{
			if (strcmp(vt_List[n].strTime, vt_List[j].strTime) <= 0) // 내림차순
			{
				temp = vt_List[n];
				vt_List[n] = vt_List[j];
				vt_List[j] = temp;
			}
		}
	}
}
// =============================================================================
#pragma endregion

void CDlgList::OnDatetimechangeDatetimepickerList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;

	WORD year = pDTChange->st.wYear;
	WORD month = pDTChange->st.wMonth;
	WORD day = pDTChange->st.wDay;

	if (m_pMain->vt_job_info[0].algo_method == CLIENT_TYPE_ELB_CENTER_ALIGN)
	{
		update_list_centeralign(year, month, day);
	}
	else
	{
		update_list_align(m_nJob, year, month, day);
	}

	
}


void CDlgList::OnClose()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	list_clear();

	CDialogEx::OnClose();
}
