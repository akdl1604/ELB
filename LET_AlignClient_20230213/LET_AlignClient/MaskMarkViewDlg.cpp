// MarkViewDlg.cpp: 구현 파일입니다.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "MaskMarkViewDlg.h"
#include "afxdialogex.h"
#include "use_opencv.h"
#include "opencv2\imgproc\imgproc_c.h"
// CMaskMarkViewDlg 대화 상자입니다.

#define TAB_HEADER_HEIGHT		24
#define TIMER_ID 1000

IMPLEMENT_DYNAMIC(CMaskMarkViewDlg, CDialog)

CMaskMarkViewDlg::CMaskMarkViewDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMaskMarkViewDlg::IDD, pParent)
{
	for (int i = 0; i < 4; i++)
	{
		m_bIsBtnClick[i] = FALSE;
		m_crMaskRect[i] = CRect(0, 0, 0, 0);
		m_bMaskRoiUse[i] = 0;
		m_bMaskRoiShape[i] = 0;
	}

	m_bBtnClick = FALSE;
	m_nClickedBtn = -1;
	m_nClickCount = 0;
	m_pImage = NULL;
	m_MaskIMG = NULL;

	m_dbPatRefX = 0.0;
	m_dbPatRefY = 0.0;
	m_dSize = 2;
	m_dSize_OLD = 2;

	m_bMaskPen = TRUE;
	m_bMaskRect = FALSE;
	m_pBuffer = NULL;

	m_pSearhingSheet = NULL;
	m_pMaskingSheet = NULL;
	m_pFindingSheet = NULL;
	m_pSearhTesterSheet = NULL;

	m_nWidth = 0;
	m_nHeight = 0;

	m_bShape = 0;
	mark_nAlgo = 0;
	m_bClickedEvent = FALSE;
	m_bUseCalibModel = FALSE; // Tkyuha 20211204 기본은 켈리브레이션용은 사용 안함

	BnClickeds[0] = &CMaskMarkViewDlg::OnBnClickedBtnLEFT;
	BnClickeds[1] = &CMaskMarkViewDlg::OnBnClickedBtnRIGHT;
	BnClickeds[2] = &CMaskMarkViewDlg::OnBnClickedBtnTOP;
	BnClickeds[3] = &CMaskMarkViewDlg::OnBnClickedBtnBOTTOM;
}

CMaskMarkViewDlg::~CMaskMarkViewDlg()
{
	if (m_pImage != NULL)
		delete[] m_pImage; // array

	if (m_MaskIMG != NULL)
		delete[] m_MaskIMG;

	delete m_pSearhingSheet;
	delete m_pMaskingSheet;
	delete m_pFindingSheet;
	delete m_pSearhTesterSheet;
}

void CMaskMarkViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_STATIC_BTN_LEFT, m_stt_BtnPos[0]);
	DDX_Control(pDX, IDC_STATIC_BTN_RIGHT, m_stt_BtnPos[1]);
	DDX_Control(pDX, IDC_STATIC_BTN_TOP, m_stt_BtnPos[2]);
	DDX_Control(pDX, IDC_STATIC_BTN_BOTTOM, m_stt_BtnPos[3]);
	DDX_Control(pDX, IDC_CS_MARK_VIEW, m_Viewer);

	DDX_Control(pDX, IDC_BTN_SIZE_X1, m_btnSize[0]);
	DDX_Control(pDX, IDC_BTN_SIZE_X2, m_btnSize[1]);
	DDX_Control(pDX, IDC_BTN_SIZE_X3, m_btnSize[2]);
	DDX_Control(pDX, IDC_BTN_SIZE_X4, m_btnSize[3]);
	DDX_Control(pDX, IDC_BTN_SIZE_X5, m_btnSize[4]);
	DDX_Control(pDX, IDC_BTN_SIZE_X6, m_btnSize[5]);

	DDX_Control(pDX, IDC_BTN_OK, m_btnOk);
	DDX_Control(pDX, IDC_BTN_CANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_TAB_LIST, m_TabCtrl);

	DDX_Control(pDX, IDC_CHECK_SHOW_MASK, m_btnShowMask);
	DDX_Control(pDX, IDC_CHECK_SHOW_PREPROC, m_btnPreprocessing);
	DDX_Control(pDX, IDC_CHECK_SHOW_TRACKER, m_btnTracker);
}

BEGIN_MESSAGE_MAP(CMaskMarkViewDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_OK, &CMaskMarkViewDlg::OnBnClickedBtnOk)
	ON_BN_CLICKED(IDC_BTN_CANCEL, &CMaskMarkViewDlg::OnBnClickedBtnCancel)
	ON_BN_CLICKED(IDC_BTN_SIZE_X1, &CMaskMarkViewDlg::OnBnClickedBtnSizeX1)
	ON_BN_CLICKED(IDC_BTN_SIZE_X2, &CMaskMarkViewDlg::OnBnClickedBtnSizeX2)
	ON_BN_CLICKED(IDC_BTN_SIZE_X3, &CMaskMarkViewDlg::OnBnClickedBtnSizeX3)
	ON_BN_CLICKED(IDC_BTN_SIZE_X4, &CMaskMarkViewDlg::OnBnClickedBtnSizeX4)
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BTN_SIZE_X5, &CMaskMarkViewDlg::OnBnClickedBtnSizeX5)
	ON_BN_CLICKED(IDC_BTN_SIZE_X6, &CMaskMarkViewDlg::OnBnClickedBtnSizeX6)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_LIST, &CMaskMarkViewDlg::OnSelchangeTabList)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_CHECK_SHOW_MASK, &CMaskMarkViewDlg::OnBnClickedCheckShowMask)
	ON_BN_CLICKED(IDC_CHECK_SHOW_PREPROC, &CMaskMarkViewDlg::OnBnClickedCheckPreprocessing)
	ON_BN_CLICKED(IDC_CHECK_SHOW_TRACKER, &CMaskMarkViewDlg::OnBnClickedCheckTracker)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

// CMaskMarkViewDlg 메시지 처리기입니다.
BOOL CMaskMarkViewDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.	
	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);

	InitTitle(&m_stt_BtnPos[0], _T("  ←"), 24.f, UNCLICK_COLOR);
	InitTitle(&m_stt_BtnPos[1], _T("  →"), 24.f, UNCLICK_COLOR);
	InitTitle(&m_stt_BtnPos[2], _T("  ↑"), 24.f, UNCLICK_COLOR);
	InitTitle(&m_stt_BtnPos[3], _T("  ↓"), 24.f, UNCLICK_COLOR);

	GetDlgItem(IDC_STATIC_BTN_LEFT)->GetWindowRect(&m_rcBtnPos[0]);
	GetDlgItem(IDC_STATIC_BTN_RIGHT)->GetWindowRect(&m_rcBtnPos[1]);
	GetDlgItem(IDC_STATIC_BTN_TOP)->GetWindowRect(&m_rcBtnPos[2]);
	GetDlgItem(IDC_STATIC_BTN_BOTTOM)->GetWindowRect(&m_rcBtnPos[3]);

	m_btnShowMask.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));

	m_btnPreprocessing.SetAlignTextCM();
	m_btnPreprocessing.SetColorBkg(255, RGB(64, 64, 64));
	m_btnPreprocessing.SetSizeText(9);
	m_btnPreprocessing.SetSizeCheck(2);
	m_btnPreprocessing.SetSizeCheckBox(3, 3, 20, 20);
	m_btnPreprocessing.SetOffsetText(-5, 0);
	m_btnPreprocessing.SetText(_T("Processing"));

	//HTK 2022-06-29 Mark View창에 Tracker 기능 추가
	m_btnTracker.SetAlignTextCM();
	m_btnTracker.SetColorBkg(255, RGB(64, 64, 64));
	m_btnTracker.SetSizeText(9);
	m_btnTracker.SetSizeCheck(2);
	m_btnTracker.SetSizeCheckBox(3, 3, 20, 20);
	m_btnTracker.SetOffsetText(-5, 0);
	m_btnTracker.SetText(_T("Tracker"));

	ScreenToClient(&m_rcBtnPos[0]);
	ScreenToClient(&m_rcBtnPos[1]);
	ScreenToClient(&m_rcBtnPos[2]);
	ScreenToClient(&m_rcBtnPos[3]);

	GetDlgItem(IDC_CS_MARK_VIEW)->GetWindowRect(&m_rcMaskPos);
	ScreenToClient(&m_rcMaskPos);

	GetWindowRect(&m_rectDlg);

	GetDlgItem(IDC_CS_MARK_VIEW)->GetWindowRect(&m_rectViewer);
	ScreenToClient(&m_rectViewer);

	GetDlgItem(IDC_BTN_CANCEL)->GetWindowRect(&m_rectCancel);
	ScreenToClient(&m_rectCancel);

	for (int i = 0; i < 6; i++)
	{
		MainButtonInit(&m_btnSize[i]);
		m_btnSize[i].SetSizeText(14.f);
	}

	MainButtonInit(&m_btnOk);
	m_btnOk.SetSizeText(14.f);
	MainButtonInit(&m_btnCancel);
	m_btnCancel.SetSizeText(14.f);

	if (m_Viewer)
	{
		m_Viewer.InitControl(this);
		m_Viewer.SetEnableDrawGuidingLine(true);
		m_Viewer.SetEnableDrawGuidingCross(false);
		m_Viewer.ClearGraphics();
		m_Viewer.SetInterativeItem(&m_MaskDrawing);
		m_Viewer.SetInteraction();
		m_Viewer.Invalidate();
	}

	fnDispBtn();
	CreateTabGrid();

	SetParamInitial(mark_nCam, mark_nPos, mark_nIndex);

	// Lincoln Lee - 220219
	LoadMaskData();
	m_IsLoaded = TRUE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CMaskMarkViewDlg::CreateTabGrid()
{
	CString str;
	CRect rt;

	m_TabCtrl.GetWindowRect(&rt);
	m_TabCtrl.InsertItem(0, "Mask");
	m_pMaskingSheet = new CMarkMaskingSheet;
	m_pMaskingSheet->m_pParentDlg = this;
	m_pMaskingSheet->Create(IDD_DIALOG_MASKING, &m_TabCtrl);
	m_pMaskingSheet->SetWindowPos(NULL, 0, 25, rt.Width(), rt.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
	m_pMaskingSheet->SetDlgCtrlID(3010);

	m_TabCtrl.InsertItem(1, "Model");
	m_pSearhingSheet = new CMarkSearchingSheet;
	m_pSearhingSheet->m_pParentDlg = this;
	m_pSearhingSheet->Create(IDD_DIALOG_SEARCH, &m_TabCtrl);
	m_pSearhingSheet->SetWindowPos(NULL, 0, 25, rt.Width(), rt.Height(), SWP_HIDEWINDOW | SWP_NOZORDER);
	m_pSearhingSheet->SetDlgCtrlID(3011);

	m_TabCtrl.InsertItem(2, "Search");
	m_pFindingSheet = new CMarkFindingSheet;
	m_pFindingSheet->m_pParentDlg = this;
	m_pFindingSheet->Create(IDD_DIALOG_FINDING, &m_TabCtrl);
	m_pFindingSheet->SetWindowPos(NULL, 0, 25, rt.Width(), rt.Height(), SWP_HIDEWINDOW | SWP_NOZORDER);
	m_pFindingSheet->SetDlgCtrlID(3012);

	m_TabCtrl.InsertItem(3, "Test");
	m_pSearhTesterSheet = new CMarkSearchTesterSheet;
	m_pSearhTesterSheet->m_pParentDlg = this;
	m_pSearhTesterSheet->Create(IDD_DIALOG_SEARCH_TESTER, &m_TabCtrl);
	m_pSearhTesterSheet->SetWindowPos(NULL, 0, 25, rt.Width(), rt.Height(), SWP_HIDEWINDOW | SWP_NOZORDER);
	m_pSearhTesterSheet->SetDlgCtrlID(3013);

	c_pWnd = m_pMaskingSheet;
	m_TabCtrl.SetCurSel(0);

	SetParamInitial(mark_nCam, mark_nPos, mark_nIndex);
}

void CMaskMarkViewDlg::OnSelchangeTabList(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;

	if (c_pWnd != NULL)
	{
		c_pWnd->ShowWindow(SW_HIDE);
		c_pWnd = NULL;
	}

	int index = m_TabCtrl.GetCurSel();

	switch (index)
	{
	case 0:
		m_pMaskingSheet->ShowWindow(SW_SHOWNOACTIVATE);
		c_pWnd = m_pMaskingSheet;
		break;
	case 1:
		m_pSearhingSheet->ShowWindow(SW_SHOWNOACTIVATE);
		c_pWnd = m_pSearhingSheet;
		break;
	case 2:
		// Lincoln Lee - 220222
		// Pass StartAngle, EndAngle from Searching to Finding
		m_pFindingSheet->m_edtSangle = m_pSearhingSheet->m_edtSangle;
		m_pFindingSheet->m_edtEangle = m_pSearhingSheet->m_edtEangle;
		m_pFindingSheet->ShowWindow(SW_SHOWNOACTIVATE);
		c_pWnd = m_pFindingSheet;
		break;
	case 3:
		m_pSearhTesterSheet->ShowWindow(SW_SHOWNOACTIVATE);
		c_pWnd = m_pSearhTesterSheet;
		break;
	}

	m_MaskDrawing.IgnoreMouse((index == 3) && (m_pSearhTesterSheet->m_bEdgeView.GetCheck()));
	//GetDlgItem(IDC_TAB_LIST)->Invalidate();
	//GetDlgItem(IDC_TAB_LIST)->RedrawWindow();

	// Let m_pFindingSheet udate data (if startAngle and/or endAngle at m_pSearchingSheet is changed
	if (NULL != c_pWnd) c_pWnd->UpdateData(false);

	//UpdateDisplay();
}

void CMaskMarkViewDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	KillTimer(nIDEvent);

	// Repeating timer of origin button
	if ((TIMER_ID == nIDEvent) && (m_IsMouseDown))
	{
		if (m_nClickCount <= 2)		 m_nUnit = 1;
		else if (m_nClickCount <= 4)	m_nUnit = 4;
		else if (m_nClickCount <= 8)	m_nUnit = 8;
		else							m_nUnit = 15;

		if (n_LastButtonID > -1) {
			(this->*BnClickeds[n_LastButtonID % 4])(m_nUnit);
		}

		++m_nClickCount;
		SetTimer(TIMER_ID, 200, NULL);
	}

	CDialog::OnTimer(nIDEvent);
}

// Lincoln Lee - 220222
void CMaskMarkViewDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();

	if (((m_TabCtrl.GetCurSel() != 3) && !m_pSearhTesterSheet->m_bEdgeView.GetCheck()) && m_ViewerRECT.PtInRect(point))
	{
		CPoint pt;
		auto x = point.x - m_rectViewer.left;
		auto y = point.y - m_rectViewer.top;
		pt.x = MAX(0, MIN(x, m_rectViewer.Width()));
		pt.y = MAX(0, MIN(y, m_rectViewer.Height()));

		m_Viewer.OnLButtonDown(nFlags, pt);
	}

	CDialog::OnLButtonDown(nFlags, point);
}

// Lincoln Lee - 220222
void CMaskMarkViewDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();

	if (((m_TabCtrl.GetCurSel() != 3) && !m_pSearhTesterSheet->m_bEdgeView.GetCheck()) && m_ViewerRECT.PtInRect(point))
	{
		CPoint pt;
		auto x = point.x - m_rectViewer.left;
		auto y = point.y - m_rectViewer.top;
		pt.x = MAX(0, MIN(x, m_rectViewer.Width()));
		pt.y = MAX(0, MIN(y, m_rectViewer.Height()));

		m_Viewer.OnLButtonUp(nFlags, pt);
	}

	CDialog::OnLButtonUp(nFlags, point);
}

// Lincoln Lee - 220222
void CMaskMarkViewDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (((m_TabCtrl.GetCurSel() != 3) && !m_pSearhTesterSheet->m_bEdgeView.GetCheck()) && m_ViewerRECT.PtInRect(point))
	{
		CPoint pt;
		auto x = point.x - m_rectViewer.left;
		auto y = point.y - m_rectViewer.top;
		pt.x = MAX(0, MIN(x, m_rectViewer.Width()));
		pt.y = MAX(0, MIN(y, m_rectViewer.Height()));

		m_Viewer.OnMouseMove(nFlags, pt);
	}
	CDialog::OnMouseMove(nFlags, point);
}

// Lincoln Lee - 220222
void CMaskMarkViewDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (((m_TabCtrl.GetCurSel() != 3) && !m_pSearhTesterSheet->m_bEdgeView.GetCheck()) && m_ViewerRECT.PtInRect(point))
	{
		CPoint pt;
		auto x = point.x - m_rectViewer.left;
		auto y = point.y - m_rectViewer.top;
		pt.x = MAX(0, MIN(x, m_rectViewer.Width()));
		pt.y = MAX(0, MIN(y, m_rectViewer.Height()));

		m_Viewer.OnLButtonDblClk(nFlags, pt);
	}

	CDialog::OnLButtonDblClk(nFlags, point);

}

void CMaskMarkViewDlg::SetParam(int com, int shape, int id, CRect roi)
{
	if (com == 0) m_bShape = shape;
	if (com == 1) m_bMaskRoiUse[id] = shape > 0 ? 1 : 0;
	if (com == 2) m_bMaskRoiShape[id] = shape;
	else	m_crMaskRect[id] = roi;
}

void CMaskMarkViewDlg::SetParam(CHalconProcessing* pMatchingProc)
{
	if (pMatchingProc != NULL)
		m_pMatchingProc = pMatchingProc;
}

void CMaskMarkViewDlg::SetMarkId(int nCam, int nPos, int nIndex, int algo)
{
	mark_nCam = nCam;
	mark_nPos = nPos;
	mark_nIndex = nIndex;
	mark_nAlgo = algo;
}

void CMaskMarkViewDlg::SetParamInitial(int nCam, int nPos, int nIndex)
{
	CHalconProcessing* mPoc;

	if (m_bUseCalibModel)
	{
		mPoc = &((CPaneMachine*)theApp.m_pFrame->m_pPane[PANE_MACHINE])->m_CalibHalcon[nCam][nPos][nIndex];
		mPoc->readParameter(theApp.m_pFrame->m_strCurrentModelPath + "MarkCalibPatternInfo.ini", mark_nCam, mark_nPos, 0, mark_nAlgo);
	}
	else
	{
		mPoc = &theApp.m_pFrame->GetMatching(mark_nAlgo).getHalcon(nCam, nPos, nIndex);
		mPoc->readParameter(theApp.m_pFrame->m_strCurrentModelPath + "MarkPatternInfo.ini", mark_nCam, mark_nPos, mark_nIndex, mark_nAlgo);
	}

	m_pSearhingSheet->m_edtNumLevel = mPoc->m_ReadNumLevel;
	m_pSearhingSheet->m_edtSangle = mPoc->m_ReadStartAngle;
	m_pSearhingSheet->m_edtEangle = mPoc->m_ReadEndAngle;
	m_pSearhingSheet->m_edtStepAngle = mPoc->m_ReadStepAngle;
	m_pSearhingSheet->m_edtContrast = mPoc->m_ReadContrast;
	m_pSearhingSheet->m_edtMinContrast = mPoc->m_ReadMinContrast;

	int id = 0;
	if ("point_reduction_low" == mPoc->m_ReadOptimize) id = 1;
	else if ("point_reduction_medium" == mPoc->m_ReadOptimize) id = 2;
	else if ("point_reduction_high" == mPoc->m_ReadOptimize) id = 3;
	m_pSearhingSheet->m_cmbOptimize.SetCurSel(id);

	id = 0;
	if ("use_polarity" == mPoc->m_ReadMetric) id = 0;
	else if ("ignore_global_polarity" == mPoc->m_ReadMetric) id = 1;
	else if ("ignore_local_polarity" == mPoc->m_ReadMetric) id = 2;

	m_pSearhingSheet->m_cmbMetric.SetCurSel(id);
	m_pSearhingSheet->UpdateData(FALSE);

	m_pFindingSheet->m_edtNumMatch = mPoc->m_SearchNumMatch;
	m_pFindingSheet->m_edtSangle = mPoc->m_SearchStartAngle;
	m_pFindingSheet->m_edtEangle = mPoc->m_SearchEndAngle;
	m_pFindingSheet->m_edtMinScore = mPoc->m_SearchMinscore;
	m_pFindingSheet->m_edtGreedness = mPoc->m_SearchGreedness;
	m_pFindingSheet->m_edtNumScale = mPoc->m_ReadSmallScale;

	id = 0;
	if ("interpolation" == mPoc->m_SearchSubPixel) id = 1;
	else if ("least_squares" == mPoc->m_SearchSubPixel) id = 2;
	else if ("least_squares_high" == mPoc->m_SearchSubPixel) id = 3;
	else if ("least_squares_very_high" == mPoc->m_SearchSubPixel) id = 4;
	else if ("max_deformation 1" == mPoc->m_SearchSubPixel) id = 5;
	else if ("max_deformation 2" == mPoc->m_SearchSubPixel) id = 6;
	else if ("max_deformation 3" == mPoc->m_SearchSubPixel) id = 7;
	else if ("max_deformation 4" == mPoc->m_SearchSubPixel) id = 8;
	else if ("max_deformation 5" == mPoc->m_SearchSubPixel) id = 9;
	else if ("max_deformation 6" == mPoc->m_SearchSubPixel) id = 10;

	m_pFindingSheet->m_cmbSubPixel.SetCurSel(id);
	m_pFindingSheet->m_cmbMethod.SetCurSel(mPoc->m_SearchMethod);
	m_pFindingSheet->UpdateData(FALSE);

	m_btnPreprocessing.SetCheck(mPoc->m_bPreProcess);
	OnBnClickedCheckPreprocessing();

	CWnd* pButton = m_pSearhTesterSheet->GetDlgItem(IDC_BUTTON_GETPARAM);
	m_pSearhTesterSheet->PostMessage(WM_COMMAND, MAKEWPARAM(IDC_BUTTON_GETPARAM, BN_CLICKED), (LPARAM)pButton->m_hWnd);
}

GraphicMaskDrawing& CMaskMarkViewDlg::GetMaskDrawer()
{
	return m_MaskDrawing;
}

void CMaskMarkViewDlg::SetImage(int width, int height, double refX, double refY, BYTE* pImage)
{
	if (width <= 0 || height <= 0)	return;

	m_MaskDrawing
		.SetMaxSize(width, height)
		.SetSelectedPoint(refX, refY);

	if (m_pImage != NULL) delete m_pImage;

	auto size = width * height;
	m_pImage = new BYTE[size];
	m_MaskIMG = new BYTE[size];

	memcpy(m_pImage, pImage, size);
	memset(m_MaskIMG, 0, size);

	m_nWidth = width;
	m_nHeight = height;

	m_dbPatRefX = refX;
	m_dbPatRefY = refY;

	m_Viewer.SetImage(cv::Mat(height, width, CV_8U, pImage));
}

// Lincoln Lee - 220222
void CMaskMarkViewDlg::DoResize()
{
	const int maxWidth = 1900;
	const int maxHeight = 1000;
	CRect mainRect, tabRect, picRect;

	GetWindowRect(&mainRect);

	m_TabCtrl.GetWindowRect(&tabRect);
	m_Viewer.GetWindowRect(&picRect);
	ScreenToClient(&tabRect);
	ScreenToClient(&picRect);

	float w0 = MIN((m_nWidth * m_dSize), maxWidth);
	float h0 = MIN((m_nHeight * m_dSize), maxHeight);

	float fx = w0 / m_nWidth;
	float fy = h0 / m_nHeight;

	m_dSize = MIN(fx, fy);

	int ww = lround(m_nWidth * m_dSize);
	int hh = lround(m_nHeight * m_dSize);

	int w = tabRect.right + ww + 30;
	int h = MAX(480, hh + 60);

	m_ViewerRECT.left = tabRect.right - 5;
	m_ViewerRECT.top = 0;
	m_ViewerRECT.right = w;
	m_ViewerRECT.bottom = h;

	MoveWindow(mainRect.left, mainRect.top, w, h);
	m_Viewer.MoveWindow(picRect.left, picRect.top, ww, hh);
	m_Viewer
		.SetFitMode()
		.SoftDirtyRefresh();

	m_Viewer.GetWindowRect(&m_rectViewer);
	ScreenToClient(&m_rectViewer);
	//m_rectViewer = picRect;
}

// Lincoln Lee - 220210
void CMaskMarkViewDlg::UpdateDisplay(int nSheetId)
{
	m_Viewer.SoftDirtyRefresh();
}

void CMaskMarkViewDlg::NotifyUpdate(int nSheetId)
{
	//UpdateDisplay(nSheetId);
	m_Viewer.SoftDirtyRefresh();
}

void CMaskMarkViewDlg::fnDispBtn()
{
	for (int i = 0; i < 6; i++)
		m_btnSize[i].SetColorBkg(255, COLOR_BTN_BODY);

	if (m_dSize == 0.5)	m_btnSize[4].SetColorBkg(255, COLOR_BTN_SELECT);
	else if (m_dSize == 1.0 || m_dSize == 2.0 || m_dSize == 3.0 || m_dSize == 4.0)
		m_btnSize[(int)m_dSize - 1].SetColorBkg(255, COLOR_BTN_SELECT);
	else m_btnSize[5].SetColorBkg(255, COLOR_BTN_SELECT);

}

void CMaskMarkViewDlg::InitTitle(CLabelEx* pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);

	if (color == RGB(255, 255, 255)) pTitle->SetColorText(255, 0, 0, 0);
}


void CMaskMarkViewDlg::MainButtonInit(CButtonEx* pbutton, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, COLOR_BTN_BODY);
	pbutton->SetColorBorder(255, COLOR_BTN_SIDE);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	//	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}


HBRUSH CMaskMarkViewDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	return m_hbrBkg;
}

void CMaskMarkViewDlg::OnBnClickedBtnOk()
{
	SaveMaskImage(true);
	GetParameterData();

	CDialog::OnOK();
}

void CMaskMarkViewDlg::GetParameterData()
{
	CHalconProcessing* mPoc;

	if (m_bUseCalibModel)	mPoc = &((CPaneMachine*)theApp.m_pFrame->m_pPane[PANE_MACHINE])->m_CalibHalcon[mark_nCam][mark_nPos][mark_nIndex];
	else					mPoc = &theApp.m_pFrame->GetMatching(mark_nAlgo).getHalcon(mark_nCam, mark_nPos, mark_nIndex);

	mPoc->m_ReadNumLevel = m_pSearhingSheet->m_edtNumLevel;
	mPoc->m_ReadStartAngle = m_pSearhingSheet->m_edtSangle;
	mPoc->m_ReadEndAngle = m_pSearhingSheet->m_edtEangle;
	mPoc->m_ReadStepAngle = m_pSearhingSheet->m_edtStepAngle;
	mPoc->m_ReadContrast = m_pSearhingSheet->m_edtContrast;
	mPoc->m_ReadMinContrast = m_pSearhingSheet->m_edtMinContrast;

	int id = m_pSearhingSheet->m_cmbOptimize.GetCurSel();
	switch (id)
	{
	case 0: mPoc->m_ReadOptimize = "none"; break;
	case 1: mPoc->m_ReadOptimize = "point_reduction_low"; break;
	case 2: mPoc->m_ReadOptimize = "point_reduction_medium"; break;
	case 3: mPoc->m_ReadOptimize = "point_reduction_high"; break;
	}

	id = m_pSearhingSheet->m_cmbMetric.GetCurSel();
	switch (id)
	{
	case 0: mPoc->m_ReadMetric = "use_polarity"; break;
	case 1: mPoc->m_ReadMetric = "ignore_global_polarity"; break;
	case 2: mPoc->m_ReadMetric = "ignore_local_polarity"; break;
	}

	mPoc->m_SearchNumMatch = m_pFindingSheet->m_edtNumMatch;
	mPoc->m_SearchStartAngle = m_pFindingSheet->m_edtSangle;
	mPoc->m_SearchEndAngle = m_pFindingSheet->m_edtEangle;
	mPoc->m_SearchMinscore = m_pFindingSheet->m_edtMinScore;
	mPoc->m_SearchGreedness = m_pFindingSheet->m_edtGreedness;
	mPoc->m_SearchMethod = m_pFindingSheet->m_cmbMethod.GetCurSel();
	mPoc->m_ReadSmallScale = m_pFindingSheet->m_edtNumScale;

	id = m_pFindingSheet->m_cmbSubPixel.GetCurSel();
	switch (id)
	{
	case 0: mPoc->m_SearchSubPixel = "none"; break;
	case 1: mPoc->m_SearchSubPixel = "interpolation"; break;
	case 2: mPoc->m_SearchSubPixel = "least_squares"; break;
	case 3: mPoc->m_SearchSubPixel = "least_squares_high"; break;
	case 4: mPoc->m_SearchSubPixel = "least_squares_very_high"; break;
	case 5: mPoc->m_SearchSubPixel = "max_deformation 1"; break;
	case 6: mPoc->m_SearchSubPixel = "max_deformation 2"; break;
	case 7: mPoc->m_SearchSubPixel = "max_deformation 3"; break;
	case 8: mPoc->m_SearchSubPixel = "max_deformation 4"; break;
	case 9: mPoc->m_SearchSubPixel = "max_deformation 5"; break;
	case 10: mPoc->m_SearchSubPixel = "max_deformation 6"; break;
	}

	mPoc->m_bPreProcess = m_btnPreprocessing.GetCheck();

	if (m_bUseCalibModel)	mPoc->saveParameter(theApp.m_pFrame->m_strCurrentModelPath + "MarkCalibPatternInfo.ini", mark_nCam, mark_nPos, mark_nIndex, mark_nAlgo);
	else					mPoc->saveParameter(theApp.m_pFrame->m_strCurrentModelPath + "MarkPatternInfo.ini", mark_nCam, mark_nPos, mark_nIndex, mark_nAlgo);

	auto pt = m_MaskDrawing.GetSelectedPoint();

	m_dbPatRefX = pt.X;
	m_dbPatRefY = pt.Y;
}

void CMaskMarkViewDlg::OnBnClickedBtnCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialog::OnCancel();
}


void CMaskMarkViewDlg::OnBnClickedBtnSizeX1()
{
	m_dSize = 1;
	fnDispBtn();
	DoResize();
	//kmb 220811 Mark Processing Check 추가
	//OnBnClickedCheckPreprocessing();
	UpdateDisplay();
}


void CMaskMarkViewDlg::OnBnClickedBtnSizeX2()
{
	m_dSize = 2;
	fnDispBtn();
	DoResize();
	//kmb 220811 Mark Processing Check 추가
	//OnBnClickedCheckPreprocessing();
	UpdateDisplay();
}


void CMaskMarkViewDlg::OnBnClickedBtnSizeX3()
{
	m_dSize = 3;
	fnDispBtn();
	DoResize();
	//kmb 220811 Mark Processing Check 추가
	//OnBnClickedCheckPreprocessing();
	UpdateDisplay();
}


void CMaskMarkViewDlg::OnBnClickedBtnSizeX4()
{
	m_dSize = 4;
	fnDispBtn();
	DoResize();
	//kmb 220811 Mark Processing Check 추가
	//OnBnClickedCheckPreprocessing();
	UpdateDisplay();
}


void CMaskMarkViewDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}


void CMaskMarkViewDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CMaskMarkViewDlg::OnBnClickedBtnSizeX5()
{
	m_dSize = 0.5;
	fnDispBtn();
	DoResize();
	//kmb 220811 Mark Processing Check 추가
	//OnBnClickedCheckPreprocessing();
	UpdateDisplay();
}

void CMaskMarkViewDlg::OnBnClickedBtnSizeX6()
{
	double viewerWidth = double(m_rectViewer.Width());
	double viewerHeight = double(m_rectViewer.Height());

	m_dSize = MIN(viewerWidth / m_nWidth, viewerHeight / m_nHeight);
	fnDispBtn();
	DoResize();
	//kmb 220811 Mark Processing Check 추가
	//OnBnClickedCheckPreprocessing();
	UpdateDisplay();
}


void CMaskMarkViewDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	//SetTabsPosition();
}

void CMaskMarkViewDlg::SetTabsPosition()
{
	CWnd* pBaseWnd = GetDlgItem(IDC_TAB_LIST);
	// 
	if (pBaseWnd != NULL)
	{
		CRect rcTab, rcDlg;
		pBaseWnd->GetWindowRect(&rcTab);
		ScreenToClient(&rcTab);
		//GetClientRect(&rcDlg);
		//	// 
		//rcTab.right = max(rcTab.left,rcDlg.right-7);
		//rcTab.bottom = max(rcTab.top,rcDlg.bottom-6);
			// 
		m_TabCtrl.MoveWindow(&rcTab);
	}
}

// 
bool CMaskMarkViewDlg::IsExist(CWnd* pWnd) const
{
	return false;
}

void CMaskMarkViewDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
}

void CMaskMarkViewDlg::ClearMaskImage()
{
	CString strPath;
	if (m_bUseCalibModel)
		strPath.Format("%s%s\\%s_%d%d%d%s", theApp.m_pFrame->m_strCurrentModelPath, theApp.m_pFrame->vt_job_info[mark_nAlgo].job_name.c_str(), "CalibMarkMask", mark_nCam, mark_nPos, mark_nIndex, "Info.bmp");
	else
		strPath.Format("%s%s\\%s_%d%d%d%s", theApp.m_pFrame->m_strCurrentModelPath, theApp.m_pFrame->vt_job_info[mark_nAlgo].job_name.c_str(), "MarkMask", mark_nCam, mark_nPos, mark_nIndex, "Info.bmp");

	memset(m_MaskIMG, 0, m_nWidth * m_nHeight);

	m_Viewer.ClearOverlayDC();
	m_Viewer.Invalidate();

	if (_access(strPath, 0) == 0)
	{
		::DeleteFileA(strPath);
	}
}

// Lincoln Lee - 220210
void CMaskMarkViewDlg::LoadMaskData()
{
	CString strDataFile;

	if (m_bUseCalibModel)
		strDataFile.Format("%s%s\\%s_%d%d%d%s", theApp.m_pFrame->m_strCurrentModelPath, theApp.m_pFrame->vt_job_info[mark_nAlgo].job_name.c_str(), "CalibMarkMask", mark_nCam, mark_nPos, mark_nIndex, "Info.data");
	else
		strDataFile.Format("%s%s\\%s_%d%d%d%s", theApp.m_pFrame->m_strCurrentModelPath, theApp.m_pFrame->vt_job_info[mark_nAlgo].job_name.c_str(), "MarkMask", mark_nCam, mark_nPos, mark_nIndex, "Info.dat");

	m_MaskDrawing.ClearShapeObjects();
	m_MaskDrawing.ImportShapes(strDataFile);
}

// Lincoln Lee - 220210
void CMaskMarkViewDlg::SaveMaskImage(bool save)
{
	auto mask = m_MaskDrawing.GetMarkMask();
	auto mask8 = cv::Mat(m_nHeight, m_nWidth, CV_8U, m_MaskIMG);
	cv::cvtColor(mask, mask8, cv::COLOR_BGRA2GRAY);
	mask8 *= 3;

	if (save)
	{
		CString strPath;
		CString strDataFile;

		if (m_bUseCalibModel)
		{
			strPath.Format("%s%s\\%s_%d%d%d%s", theApp.m_pFrame->m_strCurrentModelPath, theApp.m_pFrame->vt_job_info[mark_nAlgo].job_name.c_str(), "CalibMarkMask", mark_nCam, mark_nPos, mark_nIndex, "Info.bmp");
			strDataFile.Format("%s%s\\%s_%d%d%d%s", theApp.m_pFrame->m_strCurrentModelPath, theApp.m_pFrame->vt_job_info[mark_nAlgo].job_name.c_str(), "CalibMarkMask", mark_nCam, mark_nPos, mark_nIndex, "Info.data");
		}
		else
		{
			strPath.Format("%s%s\\%s_%d%d%d%s", theApp.m_pFrame->m_strCurrentModelPath, theApp.m_pFrame->vt_job_info[mark_nAlgo].job_name.c_str(), "MarkMask", mark_nCam, mark_nPos, mark_nIndex, "Info.bmp");
			strDataFile.Format("%s%s\\%s_%d%d%d%s", theApp.m_pFrame->m_strCurrentModelPath, theApp.m_pFrame->vt_job_info[mark_nAlgo].job_name.c_str(), "MarkMask", mark_nCam, mark_nPos, mark_nIndex, "Info.dat");
		}

		cv::imwrite(strPath.GetString(), mask);
		m_MaskDrawing.ExportShapes(strDataFile);
	}
}

// Lincoln Lee - 220219
void CMaskMarkViewDlg::OnBnClickedCheckShowMask()
{
	UpdateDisplay();
}

void CMaskMarkViewDlg::OnBnClickedCheckPreprocessing()
{
	cv::Mat srcMAT(m_nHeight, m_nWidth, CV_8U, m_pImage);

	if (m_btnPreprocessing.GetCheck())
	{
		cv::Mat threshMAT;
		cv::threshold(srcMAT, threshMAT, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

		m_Viewer.SetImage(threshMAT);
		threshMAT.release();
	}
	else
	{
		m_Viewer.SetImage(srcMAT);
		fnDispBtn();
	}
	srcMAT.release();

	m_Viewer.SetFitMode();
	m_Viewer.Invalidate();
}

// Lincoln Lee - 220222
BOOL CMaskMarkViewDlg::IsInsideRegion(const CRect& rect, const CPoint& pt)
{
	return (pt.x >= rect.left && pt.x <= rect.right && pt.y >= rect.top && pt.y <= rect.bottom);
}

// Lincoln Lee - 220222
BOOL CMaskMarkViewDlg::CheckAndDoOriginBnClk(const CPoint& pt, int& btnId)
{
	BOOL IsOverOrgBn = FALSE;
	for (int i = 0; i < 4; i++)
	{
		if (IsInsideRegion(m_rcBtnPos[i], pt))
		{
			(this->*BnClickeds[i])(m_nUnit);

			btnId = i;
			IsOverOrgBn = TRUE;
			break;
		}
	}
	return IsOverOrgBn;
}

// Lincoln Lee - 220222
BOOL CMaskMarkViewDlg::PreTranslateMessage(MSG* pMsg)
{
	CPoint pt;
	int thisBtn = -1;

	switch (pMsg->message)
	{
	case WM_LBUTTONDOWN:
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		if (CheckAndDoOriginBnClk(pt, thisBtn))
		{
			ClickColor(thisBtn);

			if (thisBtn != n_LastButtonID)
			{
				if (n_LastButtonID > -1)
					UnClickColor(n_LastButtonID);

				n_LastButtonID = thisBtn;
				m_nClickCount = 0;
			}
			SetTimer(TIMER_ID, 200, NULL);
		}

		m_IsMouseDown = TRUE;
		return false;

	case WM_LBUTTONUP:
	case WM_MOUSELEAVE:
		if (n_LastButtonID > -1)
			UnClickColor(n_LastButtonID);

		m_nUnit = 1;
		m_nClickCount = 0;
		n_LastButtonID = -1;
		m_IsMouseDown = FALSE;
		KillTimer(TIMER_ID);

		return false;

	case WM_LBUTTONDBLCLK:
		OnLButtonDblClk(0, CPoint());
		return false;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

// Lincoln Lee - 220222
void CMaskMarkViewDlg::UnClickColor(int id)
{
	m_stt_BtnPos[id % 4].SetColorBkg(255, UNCLICK_COLOR);
	m_stt_BtnPos[id % 4].UpdateWindow();
}

// Lincoln Lee - 220222
void CMaskMarkViewDlg::ClickColor(int id)
{
	m_stt_BtnPos[id % 4].SetColorBkg(255, CLICK_COLOR);
	m_stt_BtnPos[id % 4].UpdateWindow();
}

// Lincoln Lee - 220222
void CMaskMarkViewDlg::OnBnClickedBtnLEFT(int nUnit)
{
	//m_dbPatRefX = MAX(1, MIN(m_nWidth, m_dbPatRefX - nUnit));
	m_MaskDrawing.OffsetOrign(-nUnit, 0);
	UpdateDisplay();
}

// Lincoln Lee - 220222
void CMaskMarkViewDlg::OnBnClickedBtnTOP(int nUnit)
{
	//m_dbPatRefY = MAX(1, MIN(m_nHeight, m_dbPatRefY - nUnit));
	m_MaskDrawing.OffsetOrign(0, -nUnit);
	UpdateDisplay();
}

// Lincoln Lee - 220222
void CMaskMarkViewDlg::OnBnClickedBtnRIGHT(int nUnit)
{
	//m_dbPatRefX = MAX(1, MIN(m_nWidth, m_dbPatRefX + nUnit));
	m_MaskDrawing.OffsetOrign(nUnit, 0);
	UpdateDisplay();
}

// Lincoln Lee - 220222
void CMaskMarkViewDlg::OnBnClickedBtnBOTTOM(int nUnit)
{
	//m_dbPatRefY = MAX(1, MIN(m_nHeight, m_dbPatRefY + nUnit));
	m_MaskDrawing.OffsetOrign(0, nUnit);
	UpdateDisplay();
}
//HTK 2022-06-29 Mark View창에 Tracker 기능 추가
void CMaskMarkViewDlg::OnBnClickedCheckTracker()
{
	if (m_btnTracker.GetCheck())
	{
		//m_Viewer.SetEnableDrawAllFigures(true);
		//m_ImagePattern.SetEnableFigureRectangle(0);
		//m_ImagePattern.m_bDrawRectangleInside = true;
		//m_ImagePattern.m_bDrawRectangleInsideText = false;
		//m_ImagePattern.SetModeDrawingFigure(true);

		//m_ImagePattern.SetSizeRectangle(0, CPoint(0, 0), CPoint(200, 200));
		//m_ImagePattern.SetEnableModifyFigure(0, true);
		//m_Viewer.SetEnableDrawFigure(0, true);
		m_MaskDrawing.SetPickRectMode(true);
	}
	else
	{
		//m_ImagePattern.SetModeDrawingFigure(false);
		//m_ImagePattern.SetEnableModifyFigure(0, false);
		//m_Viewer.SetEnableDrawFigure(0, false);
		m_MaskDrawing.SetPickRectMode(false);
	}

	m_Viewer.Invalidate();
}

void CMaskMarkViewDlg::PreSubclassWindow()
{
	CDialog::PreSubclassWindow();
}


void CMaskMarkViewDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	if (bShow) DoResize();
}
