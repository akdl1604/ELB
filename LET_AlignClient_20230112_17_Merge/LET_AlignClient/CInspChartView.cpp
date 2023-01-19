// CInspChartView.cpp: ���� ����
//

//#include "pch.h"
#include <stdafx.h>
#include <afxdialogex.h>
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "CInspChartView.h"

// CInspChartView ��ȭ ����

IMPLEMENT_DYNAMIC(CInspChartView, CDialogEx)

CInspChartView::CInspChartView(int nJob ,CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_SCATTER, pParent)
{
	m_nJobID = nJob;

	for (int i = 0; i < 15; i++)
		m_nChartData[i] = NULL;
	for (int i = 0; i < 15; i++)
		m_nChartData[i] = new float[MAXIMUM_SIZE];

	m_pMain = (CLET_AlignClientDlg*)AfxGetMainWnd();

	// ���嵥���� Get
	for(int nJob = 0; nJob < 2; nJob++) InitSpec(nJob);
}

CInspChartView::~CInspChartView()
{
	for (int i = 0; i < 15; i++)
		if (m_nChartData[i] != NULL) delete m_nChartData[i];
}

void CInspChartView::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM_INSP_CHART, m_arrChartView[0]); // LX LY
	DDX_Control(pDX, IDC_CUSTOM_INSP_CHART2, m_arrChartView[1]); // LX LY DB
	DDX_Control(pDX, IDC_CUSTOM_INSP_CHART3, m_arrChartView[2]); // RX RY 
	DDX_Control(pDX, IDC_CUSTOM_INSP_CHART4, m_arrChartView[3]); // RX RY DB
	DDX_Control(pDX, IDC_TAB_CHARTVIEW, m_tabChartView[0]);
	DDX_Control(pDX, IDC_TAB_CHARTVIEW2, m_tabChartView[1]);
	DDX_Control(pDX, IDC_DATETIMEPICKER_START, m_dateTimeSearch);
}

BEGIN_MESSAGE_MAP(CInspChartView, CDialogEx)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_CHARTVIEW, &CInspChartView::OnTcnSelchangeTabChartview)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_CHARTVIEW2, &CInspChartView::OnTcnSelchangeTabChartview2)
	ON_NOTIFY(DTN_CLOSEUP, IDC_DATETIMEPICKER_START, &CInspChartView::OnDtnCloseupDatetimepickerStart)
	ON_MESSAGE(WM_VIEW_CONTROL, OnViewControl)
END_MESSAGE_MAP()

// CInspChartView �޽��� ó����
BOOL CInspChartView::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// ��Ʈ��Ʈ�ѷ� ����

	CPaneAuto* pPaneAuto = (CPaneAuto*)m_pMain->m_pPane[PANE_AUTO];

	this->SetWindowPos(NULL, pPaneAuto->m_Rect.left, pPaneAuto->m_Rect.top + 25, pPaneAuto->m_Rect.Width(), pPaneAuto->m_Rect.Height() + 55, SWP_SHOWWINDOW | SWP_NOZORDER);
	this->SetBackgroundColor(COLOR_BTN_BODY);

	//SJB 2022-08-22 DB ���� �ڵ�
	m_dbFilmInspDataBase.InitDataBase(m_pMain->m_strResultDir, m_nJobID);
	m_dbFilmInspDataBase.CreateDataBase();

	//SJB 2022-08-29 DB ������ ����
	m_dbFilmInspDataBase.DeleteFilmInspDataBase();

	CreateScatterGraph(m_nJobID);

	for (int i = 0; i < CHART_MAX; i++)	DrawSpecLine(i, m_nJobID); // ������� �׸���

	int nMethod = m_pMain->vt_job_info[0].algo_method;
	InitTabCtrl(nMethod);

	return TRUE; 
}

BOOL CInspChartView::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
			return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CInspChartView::InitSpec(int nJob)
{
	m_dSpecLX[nJob] = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSubSpec(0); // LX
	m_dSpecLY[nJob] = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSubSpec(2); // LY

	m_dSpecRX[nJob] = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSubSpec(1); // RX
	m_dSpecRY[nJob] = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSubSpec(3); // RY

	m_dSpecMarginLX[nJob] = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(0); // LX MARGIN
	m_dSpecMarginLY[nJob] = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(2); // LY MARGIN
	m_dSpecMarginRX[nJob] = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(1); // RX MARGIN
	m_dSpecMarginRY[nJob] = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(3); // RY MARGIN

	// LX MIN MAX ����
	m_dSpecMinLX[nJob] = m_dSpecLX[nJob] - m_dSpecMarginLX[nJob];	
	m_dSpecMaxLX[nJob] = m_dSpecLX[nJob] + m_dSpecMarginLX[nJob];	

	// LY MIN MAX ����
	m_dSpecMinLY[nJob] = m_dSpecLY[nJob] - m_dSpecMarginLY[nJob];	
	m_dSpecMaxLY[nJob] = m_dSpecLY[nJob] + m_dSpecMarginLY[nJob];	

	// RX MIN MAX ����
	m_dSpecMinRX[nJob] = m_dSpecRX[nJob] - m_dSpecMarginRX[nJob];
	m_dSpecMaxRX[nJob] = m_dSpecRX[nJob] + m_dSpecMarginRX[nJob];

	// RY MIN MAX ����
	m_dSpecMinRY[nJob] = m_dSpecRY[nJob] - m_dSpecMarginRY[nJob];
	m_dSpecMaxRY[nJob] = m_dSpecRY[nJob] + m_dSpecMarginRY[nJob];
}

void CInspChartView::DrawSpecLine(int nChartNum, int nJob)
{
#pragma region ���� ���� ǥ��
	int nStep_X = 0, nStep_Y = 0;

	if (nChartNum >= 2)
	{
		nStep_X = 2 * (m_dSpecMarginRX[nJob] / TEST_TICK_SIZE);
		nStep_Y = 2 * (m_dSpecMarginRY[nJob] / TEST_TICK_SIZE);
	}
	else
	{
		nStep_X = 2 * (m_dSpecMarginLX[nJob] / TEST_TICK_SIZE);
		nStep_Y = 2 * (m_dSpecMarginLY[nJob] / TEST_TICK_SIZE);
	}

	// YCS 2022-09-06 ���� ��ü �׸����� �迭 1�� �� �ʿ���
	nStep_X++;
	nStep_Y++;

	double* YValue = new double[nStep_X], * XValue = new double[nStep_X]; // BOTTOM
	double* YValue2 = new double[nStep_Y], * XValue2 = new double[nStep_Y]; // LEFT
	double* YValue3 = new double[nStep_X], * XValue3 = new double[nStep_X]; // TOP
	double* YValue4 = new double[nStep_Y], * XValue4 = new double[nStep_Y]; // RIGHT
	
	if (nChartNum >= 2)
	{
		for (int i = 0; i < nStep_X; i++)
		{
			YValue[i] = m_dSpecMinRY[nJob];		// RY_MIN
			XValue[i] = m_dSpecMinRX[nJob];		// RX_MIN

			YValue3[i] = m_dSpecMaxRY[nJob];	// RY_MAX;
			XValue3[i] = m_dSpecMinRX[nJob];	// RX_MIN;
		}

		for (int i = 0; i < nStep_Y; i++)
		{
			YValue2[i] = m_dSpecMinRY[nJob];	// RY_MIN
			XValue2[i] = m_dSpecMinRX[nJob];	// RX_MIN

			YValue4[i] = m_dSpecMinRY[nJob];	// RY_MIN
			XValue4[i] = m_dSpecMaxRX[nJob];	// RX MAX
		}
	}
	else
	{
		for (int i = 0; i < nStep_X; i++)
		{
			YValue[i] = m_dSpecMinLY[nJob];		// LY_MIN
			XValue[i] = m_dSpecMinLX[nJob];		// LX_MIN

			YValue3[i] = m_dSpecMaxLY[nJob];	// LY_MAX;
			XValue3[i] = m_dSpecMinLX[nJob];	// LX_MIN;
		}

		for (int i = 0; i < nStep_Y; i++)
		{
			YValue2[i] = m_dSpecMinLY[nJob];	// LY_MIN
			XValue2[i] = m_dSpecMinLX[nJob];	// LX_MIN

			YValue4[i] = m_dSpecMinLY[nJob];	// LY_MIN
			XValue4[i] = m_dSpecMaxLX[nJob];	// LX MAX
		}
	}

	for (int i = 0; i < nStep_X; i++)
	{
		// SIDE_BOTTOM
		XValue[i] += i * TEST_TICK_SIZE;
	}

	for (int i = 0; i < nStep_Y; i++)
	{
		// SIDE_LEFT
		YValue2[i] += i * TEST_TICK_SIZE;
	}
	for (int i = 0; i < nStep_X; i++)
	{
		// SIDE_TOP
		XValue3[i] += i * TEST_TICK_SIZE;
	}
	for (int i = 0; i < nStep_Y; i++)
	{
		// SIDE_RIGHT
		YValue4[i] += i * TEST_TICK_SIZE;
	}
#pragma endregion
#pragma region ���� ���ڰ� ǥ��
	int nStep = 10 / TEST_TICK_SIZE; // �ϵ��ڵ�

	double* testX = new double[nStep], *testY = new double[nStep]; // ������
	double* testX2 = new double[nStep], *testY2 = new double[nStep]; // ����

	if (nChartNum >= 2)
	{
		for (int i = 0; i < nStep; i++)
		{
			testX[i] = m_dSpecRX[nJob];
			testY2[i] = m_dSpecRY[nJob];

			testY[i] = 0;
			testX2[i] = 0;
		}
	}
	else
	{
		for (int i = 0; i < nStep; i++)
		{
			testX[i] = m_dSpecLX[nJob];
			testY2[i] = m_dSpecLY[nJob];

			testY[i] = 0;
			testX2[i] = 0;
		}
	}
	
	for (int i = 0; i < nStep; i++)
	{
		testY[i] += (double)i * TEST_TICK_SIZE; // ������
		testX2[i] += (double)i * TEST_TICK_SIZE; // ����
	}
#pragma endregion
#pragma region ��Ʈ �׸���
	// ���� ���� �׸���
	m_pPointsSeries[nChartNum][SIDE_BOTTOM]->SetPoints(XValue, YValue, nStep_X);
	m_pPointsSeries[nChartNum][SIDE_LEFT]->SetPoints(XValue2, YValue2, nStep_Y);
	m_pPointsSeries[nChartNum][SIDE_TOP]->SetPoints(XValue3, YValue3, nStep_X);
	m_pPointsSeries[nChartNum][SIDE_RIGHT]->SetPoints(XValue4, YValue4, nStep_Y);

	// ���� ���ڰ� �׸���
	m_pPointsSeries[nChartNum][LINE_VERT]->SetPoints(testX, testY, nStep);
	m_pPointsSeries[nChartNum][LINE_HORI]->SetPoints(testX2, testY2, nStep);

	// ��Ʈ ����
	m_arrChartView[nChartNum].RefreshCtrl();
#pragma endregion

	delete[] XValue, YValue, XValue2, YValue2, XValue3, YValue3, XValue4, YValue4;
	delete[] testX, testY, testX2, testY2;
}

void CInspChartView::InitTabCtrl(int nMethod)
{
	if (nMethod == CLIENT_TYPE_SCAN_INSP)
	{
		for (int i = 0; i < 2; i++)
		{
			m_tabChartView[i].InsertItem(0, "Spec");
			m_tabChartView[i].InsertItem(1, "Insp-1");
			m_tabChartView[i].InsertItem(2, "Insp-2");
			m_tabChartView[i].InsertItem(3, "Insp-3");
						  
			m_tabChartView[i].SetCurSel(0);
		}
		m_dateTimeSearch.ShowWindow(FALSE);

	}
	// �ʸ��˻�
	else if (nMethod == CLIENT_TYPE_FILM_INSP)
	{
		// L_SIDE
		m_tabChartView[0].InsertItem(0, "Recent_L");
		m_tabChartView[0].InsertItem(1, "Date_L");
		m_tabChartView[0].SetCurSel(0);

		// R_SIDE
		m_tabChartView[1].InsertItem(0, "Recent_R");
		m_tabChartView[1].InsertItem(1, "Date_R");
		m_tabChartView[1].SetCurSel(0);
	}
	else
	{
		
	}
	// ����Ʈ�� ������ Get
	CRect rt;
	m_tabChartView[0].GetWindowRect(&rt);
	ScreenToClient(&rt);
	for (int i = INSP_L; i < INSP_R; i++)
	{
		// L_SIDE Chart
		m_arrChartView[i].SetWindowPos(NULL, rt.left, rt.top + 20, rt.Width(), rt.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
	}

	m_tabChartView[1].GetWindowRect(&rt);
	ScreenToClient(&rt);

	for (int i = INSP_R; i < CHART_MAX; i++)
	{
		// R_SIDE Chart
		m_arrChartView[i].SetWindowPos(NULL, rt.left, rt.top + 20, rt.Width(), rt.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
	}
}

void CInspChartView::CreateScatterGraph(int nJob)
{
	// PJH
	int ViewCount = 0;
#pragma region ������� �ʱ�ȭ
	for (int i = 0; i < 2; i++)
	{
		// LX LY  �� �ʱ�ȭ
		m_pBottomAxis_L[i] = m_arrChartView[ViewCount].CreateStandardAxis(CChartCtrl::BottomAxis);
		m_pBottomAxis_L[i]->SetMinMax(m_dSpecMinLX[nJob] * GRAPH_SIZE_MIN, m_dSpecMaxLX[nJob] * GRAPH_SIZE_MAX);
		m_pBottomAxis_L[i]->SetTickIncrement(false, 0.2);

		m_pLeftAxis_L[i] = m_arrChartView[ViewCount].CreateStandardAxis(CChartCtrl::LeftAxis);
		m_pLeftAxis_L[i]->SetMinMax(m_dSpecMinLY[nJob] * GRAPH_SIZE_MIN, m_dSpecMaxLY[nJob] * GRAPH_SIZE_MAX);
		m_pLeftAxis_L[i]->SetTickIncrement(false, 0.2);
		ViewCount++;
	}

	for (int i = 0; i < 2; i++)
	{
		// RX RY  �� �ʱ�ȭ
		m_pBottomAxis_R[i] = m_arrChartView[ViewCount].CreateStandardAxis(CChartCtrl::BottomAxis);
		m_pBottomAxis_R[i]->SetMinMax(m_dSpecMinRX[nJob] * GRAPH_SIZE_MIN, m_dSpecMaxRX[nJob] * GRAPH_SIZE_MAX);
		m_pBottomAxis_R[i]->SetTickIncrement(false, 0.2);

		m_pLeftAxis_R[i] = m_arrChartView[ViewCount].CreateStandardAxis(CChartCtrl::LeftAxis);
		m_pLeftAxis_R[i]->SetMinMax(m_dSpecMinRY[nJob] * GRAPH_SIZE_MIN, m_dSpecMaxRY[nJob] * GRAPH_SIZE_MAX);
		m_pLeftAxis_R[i]->SetTickIncrement(false, 0.2);
		ViewCount++;
	}

	// Spec ����ִ� ����Ʈ �ʱ�ȭ
	for (int j = 0; j < CHART_MAX; j++)
	{
		for (int i = 0; i <= SIDE_RIGHT; i++)
		{
			m_pPointsSeries[j][i] = m_arrChartView[j].CreatePointsSerie();
			m_pPointsSeries[j][i]->SetPointType(CChartPointsSerie::ptRectangle);
			m_pPointsSeries[j][i]->SetPointSize(2, 2);
			m_pPointsSeries[j][i]->SetBorderColor(COLOR_BLUE);
		}
	}
#pragma endregion

	// Data ����ִ� ����Ʈ �ʱ�ȭ
	for (int j = 0; j < CHART_MAX; j++)
	{
		for (int i = POINT_FIRST; i < POINT_MAX; i++)
		{
			m_pPointsSeries[j][i] = m_arrChartView[j].CreatePointsSerie();
			m_pPointsSeries[j][i]->SetPointType(CChartPointsSerie::ptEllipse);
			m_pPointsSeries[j][i]->SetPointSize(6, 6);
		}
		// L, L_DB, R, R_DB ����Ʈ �� �ʷϻ�
		m_pPointsSeries[j][POINT_FIRST]->SetBorderColor(COLOR_GREEN);
		m_pPointsSeries[j][POINT_FIRST]->SetColor(COLOR_GREEN);

		m_pPointsSeries[j][POINT_RECENT_DATA]->SetBorderColor(COLOR_RED);
		m_pPointsSeries[j][POINT_RECENT_DATA]->SetColor(COLOR_RED);
	}

	// ���� ũ�ν����� ����Ʈ �ʱ�ȭ
	for (int j = 0; j < CHART_MAX; j++)
	{
		// ���� �ʱ�ȭ
		m_pPointsSeries[j][LINE_HORI] = m_arrChartView[j].CreatePointsSerie();
		m_pPointsSeries[j][LINE_HORI]->SetPointType(CChartPointsSerie::ptRectangle);
		m_pPointsSeries[j][LINE_HORI]->SetPointSize(2, 2);

		m_pPointsSeries[j][LINE_HORI]->SetBorderColor(COLOR_ORANGE);
		m_pPointsSeries[j][LINE_HORI]->SetColor(COLOR_ORANGE);

		// ������ �ʱ�ȭ
		m_pPointsSeries[j][LINE_VERT] = m_arrChartView[j].CreatePointsSerie();
		m_pPointsSeries[j][LINE_VERT]->SetPointType(CChartPointsSerie::ptRectangle);
		m_pPointsSeries[j][LINE_VERT]->SetPointSize(2, 2);

		m_pPointsSeries[j][LINE_VERT]->SetBorderColor(COLOR_ORANGE);
		m_pPointsSeries[j][LINE_VERT]->SetColor(COLOR_ORANGE);
	}

#pragma region ���ʻ���
	// ���� ���� - ��� ����
	//TChartString str = _T("LX/LY");
	//CChartXYSerie* pSerie[4] = { 0, };
	//CRect rect;
	//for (int i = INSP_L; i <= DB_L; i++)
	//{
	//	pSerie[i] = m_pPointsSeries[i][POINT_FIRST];
	//	pSerie[i]->SetName(str);
	//	//m_arrChartView[i].GetLegend()->SetBackColor(COLOR_GRAY);
	//	rect = m_arrChartView[i].GetPlottingRect();
	//	m_arrChartView[i].GetLegend()->UndockLegend(rect.left, rect.top);
	//	m_arrChartView[i].GetLegend()->SetVisible(true);
	//}

	//str = _T("RX/RY");
	//for (int i = INSP_R; i <= DB_R; i++)
	//{
	//	pSerie[i] = m_pPointsSeries[i][POINT_FIRST];
	//	pSerie[i]->SetName(str);
	//	//m_arrChartView[i].GetLegend()->SetBackColor(COLOR_GRAY);
	//	rect = m_arrChartView[i].GetPlottingRect();
	//	m_arrChartView[i].GetLegend()->UndockLegend(rect.left, rect.top);
	//	m_arrChartView[i].GetLegend()->SetVisible(true);

	//}
#pragma endregion

	// ��Ʈ ����
	for (int i = 0; i < CHART_MAX; i++)
	{
		m_arrChartView[i].RefreshCtrl();
	}
}

void CInspChartView::UpdateScatterGraph(std::vector<_stInsp_FilmLength_Result> vtFilmResult, int resultSize)
{
	// �ֽ� ������ 1����Ʈ�� ���������� ���÷���. �������� ���� �ʷϻ�.
	// ��Ʈ �ʱ�ȭ
	m_pPointsSeries[INSP_L][POINT_FIRST]->ClearSerie();
	m_pPointsSeries[INSP_R][POINT_FIRST]->ClearSerie();

	m_pPointsSeries[INSP_L][POINT_RECENT_DATA]->ClearSerie();
	m_pPointsSeries[INSP_R][POINT_RECENT_DATA]->ClearSerie();

	int nSize = resultSize - 1; // �ֱ� �����Ϳ� ���� ������ ������ ���÷��� �ϱ� ���� ���

	double *YValues = new double[nSize], *XValues= new double[nSize]; // LX LY ���� ������
	double *YValues2 = new double[nSize], *XValues2 = new double[nSize]; // RX RY ���� ������

	// ���� ������
	for (int i = 0; i < nSize; i++)
	{
		XValues[i] = vtFilmResult[i].dLX;
		YValues[i] = vtFilmResult[i].dLY;
		XValues2[i] = vtFilmResult[i].dRX;
		YValues2[i] = vtFilmResult[i].dRY;
	}

	double pRecentData_X, pRecentData_Y; // LX LY �ֱ� ������
	double pRecentData_X2, pRecentData_Y2; // RX RY �ֱ� ������

	// ���� �ֱ� ������ 
	pRecentData_X = vtFilmResult[nSize].dLX;
	pRecentData_Y = vtFilmResult[nSize].dLY;
	pRecentData_X2 = vtFilmResult[nSize].dRX;
	pRecentData_Y2 = vtFilmResult[nSize].dRY;

	// ��Ʈ�� ���� ������ set
	m_pPointsSeries[INSP_L][POINT_FIRST]->SetPoints(XValues, YValues, nSize);
	m_pPointsSeries[INSP_R][POINT_FIRST]->SetPoints(XValues2, YValues2, nSize);

	// ��Ʈ�� �ֱ� ������ 1����Ʈ set 
	m_pPointsSeries[INSP_L][POINT_RECENT_DATA]->SetPoints(&pRecentData_X, &pRecentData_Y, 1);
	m_pPointsSeries[INSP_R][POINT_RECENT_DATA]->SetPoints(&pRecentData_X2, &pRecentData_Y2, 1);

	// ��Ʈ ����
	m_arrChartView[INSP_L].RefreshCtrl();
	m_arrChartView[INSP_R].RefreshCtrl();

	delete[] YValues, YValues2, XValues, XValues2;
}

void CInspChartView::RenewTabView(int nIndex, int nCtrlNum)
{

	if (m_pMain->m_pCInspChartDlg[0]->IsWindowVisible())
	{
		int test = 0;
	}

	int nMethod = m_pMain->vt_job_info[0].algo_method;
	if (nMethod  == CLIENT_TYPE_FILM_INSP && nCtrlNum ==0)
	{
		// ���� ��(LXLY, DB_LXLY)
		switch (nIndex)
		{
		case 0:
			m_arrChartView[0].ShowWindow(TRUE); // LXLY
			m_arrChartView[1].ShowWindow(FALSE); // DB
			break;
		case 1:
			m_arrChartView[0].ShowWindow(FALSE);
			m_arrChartView[1].ShowWindow(TRUE);
			break;
		}
	}
	else if(nMethod == CLIENT_TYPE_FILM_INSP && nCtrlNum == 1)
	{
		// ������ ��(RXRY, DB_RXRY)
		switch (nIndex)
		{
		case 0:
			m_arrChartView[2].ShowWindow(TRUE);
			m_arrChartView[3].ShowWindow(FALSE);
			break;
		case 1:
			m_arrChartView[2].ShowWindow(FALSE);
			m_arrChartView[3].ShowWindow(TRUE);
			break;
		}
	}

	// ��Ʈ ����
	for (int i = 0; i < CHART_MAX; i++)
	{
		m_arrChartView[i].RefreshCtrl();
	}
}

//SJB 2022-08-26 LX,LY,RX,RY DB�� ������ ����
void CInspChartView::DBInsertInspData(double lx, double ly, double rx, double ry)
{
	CString strTemp;
	if (m_dbFilmInspDataBase.InsertFilmInspDataBase(lx, ly, rx, ry))
	{
		strTemp.Format("[%s] Save Insp Data to DB", m_pMain->vt_job_info[m_nJobID].job_name.c_str());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
	}
}

void CInspChartView::OnTcnSelchangeTabChartview(NMHDR* pNMHDR, LRESULT* pResult)
{
	// ���� ��Ʈ �̺�Ʈ
	*pResult = 0;

	int index = m_tabChartView[0].GetCurSel();

	RenewTabView(index, 0);
}

void CInspChartView::OnTcnSelchangeTabChartview2(NMHDR* pNMHDR, LRESULT* pResult)
{
	// ������ ��Ʈ �̺�Ʈ
	*pResult = 0;

	int index = m_tabChartView[1].GetCurSel();

	RenewTabView(index, 1);
}

//SJB 2022-08-22 �޷� ���ý� DB ������ ȣ�� �Լ�
void CInspChartView::OnDtnCloseupDatetimepickerStart(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	*pResult = 0;
	CString strTemp;
	int dataCount = 0;
	SYSTEMTIME stSearch;
	m_dateTimeSearch.GetTime(&stSearch);
	
	// Chart ��Ʈ�� �ʱ�ȭ
	m_pPointsSeries[DB_L][POINT_FIRST]->ClearSerie();
	m_pPointsSeries[DB_R][POINT_FIRST]->ClearSerie();

	vt_Insp_Data.clear();

	dataCount = m_dbFilmInspDataBase.getFilmInspSearchDataCount(stSearch.wYear, stSearch.wMonth, stSearch.wDay);
	// YCS 2022-08-26 ������ ���� ����
	//dataCount = MAX(MAXIMUM_SIZE, m_dbFilmInspDataBase.getFilmInspSearchDataCount(m_nJobID, stSearch.wYear, stSearch.wMonth, stSearch.wDay));

	// ������ ������� �迭
	double* YValues_L = new double[dataCount], * XValues_L = new double[dataCount]; // LY, LX
	double* YValues_R = new double[dataCount], * XValues_R = new double[dataCount]; // RX, RY

	if (dataCount >= 1)
	{
		m_dbFilmInspDataBase.getFilmInspSearchData(vt_Insp_Data, dataCount, stSearch.wYear, stSearch.wMonth, stSearch.wDay);

		for (int i = 0; i < dataCount; i++)
		{
			XValues_L[i] = vt_Insp_Data[i]._lx;
			YValues_L[i] = vt_Insp_Data[i]._ly;
			XValues_R[i] = vt_Insp_Data[i]._rx;
			YValues_R[i] = vt_Insp_Data[i]._ry;
		}

		// dataCount��ŭ ��Ʈ�� ����Ʈ ����ش�
		m_pPointsSeries[DB_L][POINT_FIRST]->SetPoints(XValues_L, YValues_L, dataCount); // DB_LX LY ��
		m_pPointsSeries[DB_R][POINT_FIRST]->SetPoints(XValues_R, YValues_R, dataCount); // DB_RX RY ��
	}
	// ��Ʈ ����
	m_arrChartView[DB_L].RefreshCtrl();
	m_arrChartView[DB_R].RefreshCtrl();


	delete[] XValues_L, XValues_R, YValues_L, YValues_R;
}

LRESULT CInspChartView::OnViewControl(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case MSG_SPEC_REFRESH:
	{
		// ���� ����
		for (int i = 0; i < 2; i++)InitSpec(i);

		// ��Ʈ ���� ���忡 �°� �缳��
		for (int i = 0; i < 2; i++)
		{
			m_pBottomAxis_L[i]->SetMinMax(m_dSpecMinLX[i] * GRAPH_SIZE_MIN, m_dSpecMaxLX[i] * GRAPH_SIZE_MAX);
			m_pLeftAxis_L[i]->SetMinMax(m_dSpecMinLY[i] * GRAPH_SIZE_MIN, m_dSpecMaxLY[i] * GRAPH_SIZE_MAX);
			m_pBottomAxis_R[i]->SetMinMax(m_dSpecMinRX[i] * GRAPH_SIZE_MIN, m_dSpecMaxRX[i] * GRAPH_SIZE_MAX);
			m_pLeftAxis_R[i]->SetMinMax(m_dSpecMinRY[i] * GRAPH_SIZE_MIN, m_dSpecMaxRY[i] * GRAPH_SIZE_MAX);
		}

		// ������� �׸���
		for (int i = 0; i < CHART_MAX; i++)
		{
			DrawSpecLine(i, lParam);
		}

		break;
	}
	case MSG_DLG_HIDE:
	{
		// ���̾�α� ����

		this->ShowWindow(SW_HIDE);
		break;
	}
	case MSG_DLG_SHOW:
	{
		this->ShowWindow(SW_SHOW);
		break;
	}
	case MSG_DLG_REFRESH:
	{
		if (m_pMain->m_pCInspChartDlg[lParam]->m_tabChartView[0].GetCurSel() == 0)
			m_pMain->m_pCInspChartDlg[lParam]->m_arrChartView[DB_L].ShowWindow(SW_HIDE);
		else
			m_pMain->m_pCInspChartDlg[lParam]->m_arrChartView[DB_L].ShowWindow(SW_SHOW);

		if (m_pMain->m_pCInspChartDlg[lParam]->m_tabChartView[1].GetCurSel() == 0)
			m_pMain->m_pCInspChartDlg[lParam]->m_arrChartView[DB_R].ShowWindow(SW_HIDE);
		else
			m_pMain->m_pCInspChartDlg[lParam]->m_arrChartView[DB_R].ShowWindow(SW_SHOW);
		break;
	}
	}
	return 0;
}