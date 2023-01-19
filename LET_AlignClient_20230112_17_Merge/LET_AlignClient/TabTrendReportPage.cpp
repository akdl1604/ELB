// TabTrendReportPage.cpp: 구현 파일
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "TabTrendReportPage.h"
#include "afxdialogex.h"
#include "ChartPointsSerie.h"

// CTabTrendReportPage 대화 상자

IMPLEMENT_DYNAMIC(CTabTrendReportPage, CDialogEx)

CTabTrendReportPage::CTabTrendReportPage(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TAB_PANE_TREND_REPORT, pParent)
{
	m_bInitComplete = FALSE;
	m_nJobID = 0;

	for (int i = 0; i < 15; i++)
		m_nScatterChartData[i] = NULL;
}

CTabTrendReportPage::~CTabTrendReportPage()
{
	for (int i = 0; i < 15; i++)
		if(m_nScatterChartData[i] != NULL) delete m_nScatterChartData[i];
}

void CTabTrendReportPage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_DAILY_CLEAR, m_btnDailyClear);
	DDX_Control(pDX, IDC_BTN_DAILY_CLEAR2, m_btnDailyHourClear);
	DDX_Control(pDX, IDC_LB_DAILY_TITLE, m_LbDailyTitle);
	DDX_Control(pDX, IDC_LB_DAILY_TITLE2, m_LbDailyHourTitle);

	DDX_GridControl(pDX, IDC_CUSTOM_DAILY_REPORT, grid_daily_report);
	DDX_GridControl(pDX, IDC_CUSTOM_DAILY_REPORT2, grid_time_report);

	DDX_Control(pDX, IDC_CUSTOM_TREND_CHART, m_TrendChartCtrl);
}

BEGIN_MESSAGE_MAP(CTabTrendReportPage, CDialogEx)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_DAILY_CLEAR2, &CTabTrendReportPage::OnBnClickedBtnDailyClear2)
	ON_BN_CLICKED(IDC_BTN_DAILY_CLEAR, &CTabTrendReportPage::OnBnClickedBtnDailyClear)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CTabTrendReportPage)
	//EASYSIZE(IDC_LB_DAILY_TITLE, ES_BORDER, ES_BORDER,ES_BORDER, ES_BORDER, 0)
	//EASYSIZE(IDC_BTN_DAILY_CLEAR, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_CUSTOM_DAILY_REPORT, ES_BORDER, ES_BORDER,ES_BORDER, IDC_LB_DAILY_TITLE2, 0)
	//EASYSIZE(IDC_LB_DAILY_TITLE2, ES_BORDER, ES_BORDER,ES_BORDER, ES_BORDER, 0)
	//EASYSIZE(IDC_BTN_DAILY_CLEAR2, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_CUSTOM_DAILY_REPORT2, ES_BORDER, ES_BORDER,	ES_BORDER, ES_BORDER, 0)
	EASYSIZE(IDC_CUSTOM_TREND_CHART, ES_BORDER, IDC_CUSTOM_DAILY_REPORT2, ES_BORDER, ES_BORDER, 0)
END_EASYSIZE_MAP
// CTabTrendReportPage 메시지 처리기


BOOL CTabTrendReportPage::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	INIT_EASYSIZE;

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);

	InitTitle(&m_LbDailyTitle, 14.f, COLOR_DARK_LIME,RGB(255, 255, 255));
	InitTitle(&m_LbDailyHourTitle, 14.f, COLOR_DARK_LIME,RGB(255, 255, 255));

	MainButtonInit(&m_btnDailyClear);
	MainButtonInit(&m_btnDailyHourClear);

	init_report_grid(grid_daily_report, 4, 32);
	init_report_grid(grid_time_report, 4, 25);

	draw_grid_report(grid_daily_report);
	draw_grid_report(grid_time_report);		

	for (int i = 0; i < 15; i++)
		m_nScatterChartData[i] = new float[4096];

	m_bInitComplete = TRUE;
	CreateScatterGraph();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CTabTrendReportPage::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	UPDATE_EASYSIZE;	
}

HBRUSH CTabTrendReportPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	if (pWnd->m_hWnd == this->m_hWnd) hbr = m_hbrBkg;

	return m_hbrBkg;
}

void CTabTrendReportPage::InitTitle(CLabelEx* pTitle, float size, COLORREF bgcolor, COLORREF color)
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
void CTabTrendReportPage::MainButtonInit(CButtonEx* pbutton, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
	pbutton->SetSizeText(17);
}

void CTabTrendReportPage::SetTitle(CString title)
{
	m_LbDailyTitle.SetText(title + "  30 Days Report");
	m_LbDailyHourTitle.SetText(title + "  24 Hours Report");
}

void CTabTrendReportPage::init_report_grid(CGridCtrl& grid, int col, int row)
{
	BOOL bVirtualMode = FALSE;

	grid.SetVirtualMode(bVirtualMode);
	grid.SetListMode(TRUE);
	grid.SetEditable(FALSE);
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
	grid.SetFixedRowCount(1);

	grid.SetRowHeight(0, 25);
	grid.SetBkColor(RGB(197, 197, 187));
	grid.SetTextBkColor(RGB(235, 235, 235));

	for (int i = 0; i < grid.GetColumnCount(); i++)
	{
		grid.SetItemBkColour(0, i, COLOR_BTN_BODY);
		grid.SetItemFgColour(0, i, COLOR_WHITE);
	}

	grid.SetColumnWidth(0, 120);		// DATETIME
	grid.SetColumnWidth(1, 100);	// REPORT
	grid.SetColumnWidth(2, 60);	// TOTAL
	grid.SetColumnWidth(3, 60);		// NG

}

void CTabTrendReportPage::draw_grid_report(CGridCtrl& grid)
{

	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nRowCount = 0;
	CTimeSpan tmSpan;
	CTime time = CTime::GetCurrentTime();
	CString Week[8] = { "","Sun","Mon","Tue","Wed","Thu","Fri","Sat" };
	int nYear = time.GetYear();
	int nMonth = time.GetMonth();

	if (nMonth + 1 > 12)
	{
		CTime tmTime1(nYear+1,  1, 1, 0, 0, 0);
		CTime tmTime2(nYear, nMonth, 1, 0, 0, 0);
		tmSpan = tmTime1 - tmTime2;
	}
	else
	{
		CTime tmTime1(nYear, nMonth + 1, 1, 0, 0, 0);
		CTime tmTime2(nYear, nMonth, 1, 0, 0, 0);
		tmSpan = tmTime1 - tmTime2;
	}	

	for (int row = 0; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row;
			Item.col = col;

			// Fixed Cell Title
			if (row == 0)
			{
				if (col == 0)
				{
					Item.nFormat = DT_CENTER | DT_WORDBREAK;
					Item.strText.Format(_T("DateTime"), col);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				else if (col == 1)
				{
					Item.nFormat = DT_CENTER | DT_WORDBREAK;
					Item.strText.Format(_T("Report"), col);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
				}
				else if (col == 2)
				{
					Item.nFormat = DT_CENTER | DT_WORDBREAK;
					Item.strText.Format(_T("Total"), col);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_READONLY);
				}
				else if (col == 3)
				{
					Item.nFormat = DT_CENTER | DT_WORDBREAK;
					Item.strText.Format(_T("Ng"), col);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_READONLY);
				}
			}
			else
			{			
				Item.nFormat = dwTextStyle;
				CString str;
				if (grid == grid_daily_report)
				{					
					if (tmSpan.GetDays() < nRowCount) break;

					CString strTime = _T("");
					CTime tt = CTime(nYear, nMonth, nRowCount, 0, 0, 0);
					strTime.Format(_T("%04d-%02d-%02d(%s)"), nYear, nMonth, tt.GetDay(), Week[tt.GetDayOfWeek()]);

					int total = m_nDailysOK[nRowCount] + m_nDailysNG[nRowCount];
					switch (col) {
					case 0:		str.Format("%s", strTime);	break;
					case 1:		str.Format("%d/%d(%3.1f%%)", m_nDailysNG[nRowCount], total, total == 0 ? 0.f : float(m_nDailysNG[nRowCount]) / total *100);	break;
					case 2:		str.Format("%d", total);	break;
					case 3:		str.Format("%d", m_nDailysNG[nRowCount]);	break;
					}
				}
				else if (grid == grid_time_report)
				{
					switch (col) {
					case 0:		str.Format("%02d:00 ~ %02d:59", nRowCount, nRowCount);	break;
					case 1:		str.Format("%d/%d(%3.1f%%)", m_nHoursNG[nRowCount], m_nHoursOK[nRowCount], m_nHoursOK[nRowCount]==0?0.f:float(m_nHoursNG[nRowCount])/m_nHoursOK[nRowCount]*100);	break;
					case 2:		str.Format("%d", m_nHoursOK[nRowCount]);	break;
					case 3:		str.Format("%d", m_nHoursNG[nRowCount]);	break;
					}
				}

				Item.strText = str;				
			}

			UINT state = grid.GetItemState(row, col);
			grid.SetItemState(row, col, state);
			grid.SetItem(&Item);
		}
		nRowCount++;
		grid.RedrawRow(row);
	}	

}

void CTabTrendReportPage::CreateScatterGraph()
{
	CLET_AlignClientDlg* m_pMain = (CLET_AlignClientDlg*)AfxGetMainWnd();

	CChartStandardAxis* pBottomAxis =		m_TrendChartCtrl.CreateStandardAxis(CChartCtrl::BottomAxis);
	pBottomAxis->SetAutomaticMode(CChartAxis::FullAutomatic);
	CChartStandardAxis* pLeftAxis =			m_TrendChartCtrl.CreateStandardAxis(CChartCtrl::LeftAxis);
	pLeftAxis->SetAutomaticMode(CChartAxis::FullAutomatic);

	for (int i = 0; i < 5; i++)
	{
		m_pPointsSeries[i] = m_TrendChartCtrl.CreatePointsSerie();
		m_pPointsSeries[i]->SetPointType(CChartPointsSerie::ptEllipse);
		m_pPointsSeries[i]->SetPointSize(6, 6);
		switch (i)
		{
		case 0: m_pPointsSeries[i]->SetBorderColor(COLOR_BLUE); 	break;
		case 1: m_pPointsSeries[i]->SetBorderColor(COLOR_GREEN); 	break;
		case 2: m_pPointsSeries[i]->SetBorderColor(COLOR_RED); 		break;
		case 3: m_pPointsSeries[i]->SetBorderColor(COLOR_LIME); 	break;
		case 4: m_pPointsSeries[i]->SetBorderColor(COLOR_PURPLE); 	break;
		}
	}

	m_TrendChartCtrl.RefreshCtrl();
}

void CTabTrendReportPage::UpdateScatterGraph(int n)
{
	double YValues[4096] = { 0, }, XValues[4096] = { 0, };
	int _so = n>1 ? 2:0;

	if (n == 5) // 액튐검사 인경우 5개 각도로 분산 해서 그릴것
	{
		for (int j = 0; j < n; j++)
		{
			for (int i = 0; i < 4096; i++)
			{
				YValues[i] = m_nScatterChartData[9 + j][i];
				XValues[i] = 72 * j; // m_nScatterChartData[3 + j][i];
			}
			m_pPointsSeries[j]->SetPoints(XValues, YValues, 4096);
		}
	}
	else if(n==2)  // Film Inspection
	{
		for (int j = 0; j < n; j++)
		{
			for (int i = 0; i < 4096; i++)
			{
				YValues[i] = m_nScatterChartData[10+2*j][i];
				XValues[i] = m_nScatterChartData[9+2*j][i];
			}
			m_pPointsSeries[j]->SetPoints(XValues, YValues, 4096);
		}
	}
	else if (n == 3)
	{
		for (int i = 0; i < 4096; i++) //xy 
		{
			YValues[i] = m_nScatterChartData[10][i];
			XValues[i] = m_nScatterChartData[9][i];
		}
		m_pPointsSeries[0]->SetPoints(XValues, YValues, 4096);
	}
	m_TrendChartCtrl.RefreshCtrl();
}

void CTabTrendReportPage::OnBnClickedBtnDailyClear2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CTabTrendReportPage::OnBnClickedBtnDailyClear()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}

void CTabTrendReportPage::UpdateDatabase()
{
	CLET_AlignClientDlg* m_pMain = (CLET_AlignClientDlg*)AfxGetMainWnd();
	int nMethod = m_pMain->vt_job_info[m_nJobID].algo_method;
	int n = 1;

	for (int i = 0; i < 15; i++) memset(m_nScatterChartData[i], 0, sizeof(float) * 4096);

	m_pMain->m_dbResultDataBase.getTimeResultDataBase(m_nHoursOK, m_nHoursNG, m_nJobID);
	m_pMain->m_dbResultDataBase.getDailyResultDataBase(m_nDailysOK, m_nDailysNG, m_nJobID);
	m_pMain->m_dbResultDataBase.getScatterChartDataBase(m_nScatterChartData, m_nJobID);

	if (nMethod == CLIENT_TYPE_FILM_INSP)				n = 2;
	else if (nMethod == CLIENT_TYPE_ELB_CENTER_ALIGN)	n = 5;
	else n = 3;

	draw_grid_report(grid_daily_report);
	draw_grid_report(grid_time_report);
	UpdateScatterGraph(n);
}