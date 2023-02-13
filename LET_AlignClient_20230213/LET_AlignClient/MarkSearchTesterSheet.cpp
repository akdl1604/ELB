// MarkSearchingSheet.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "MarkSearchTesterSheet.h"
#include "afxdialogex.h"

// CMarkSearchTesterSheet 대화 상자입니다.
template <typename T>
std::string NumberToString(T Number)
{
	std::ostringstream ss;
	ss << Number;
	return ss.str();
}

IMPLEMENT_DYNAMIC(CMarkSearchTesterSheet, CDialogEx)

CMarkSearchTesterSheet::CMarkSearchTesterSheet(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMarkSearchTesterSheet::IDD, pParent)
	, m_edtSimMincontrast(100)
	, m_edtSimContrast(50)
{
	m_pParentDlg = NULL;
}

CMarkSearchTesterSheet::~CMarkSearchTesterSheet()
{
	m_pParentDlg->m_pMatchingProc->halcon_ShowModelClose();
}

void CMarkSearchTesterSheet::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_EDGE_VIEW, m_bEdgeView);
	DDX_Control(pDX, IDC_SLIDER_LOW, m_ctrlLowThresh);
	DDX_Control(pDX, IDC_SLIDER_HIGH, m_ctrlHighThresh);
	DDX_Text(pDX, IDC_EDIT_CONTRAST2, m_edtSimMincontrast);
	DDX_Text(pDX, IDC_EDIT_CONTRAST, m_edtSimContrast);
}

BEGIN_MESSAGE_MAP(CMarkSearchTesterSheet, CDialogEx)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CHECK_EDGE_VIEW, &CMarkSearchTesterSheet::OnBnClickedCheckEdgeView)
	ON_BN_CLICKED(IDC_BUTTON_GETPARAM, &CMarkSearchTesterSheet::OnBnClickedButtonGetparam)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, &CMarkSearchTesterSheet::OnBnClickedButtonSearch)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, &CMarkSearchTesterSheet::OnBnClickedButtonApply)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


BOOL CMarkSearchTesterSheet::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_ctrlLowThresh.SetRange(0, 255);
	m_ctrlLowThresh.SetRangeMin(0);
	m_ctrlLowThresh.SetRangeMax(255);
	m_ctrlLowThresh.SetPos(100);
	m_ctrlLowThresh.SetTicFreq(10);
	m_ctrlLowThresh.SetLineSize(1);
	m_ctrlLowThresh.SetPageSize(10);
	m_ctrlLowThresh.SetPrimaryColor(RGB(227, 91, 91));	// reddish 

	m_ctrlHighThresh.SetRange(0, 255);
	m_ctrlHighThresh.SetRangeMin(0);
	m_ctrlHighThresh.SetRangeMax(255);
	m_ctrlHighThresh.SetPos(50);
	m_ctrlHighThresh.SetTicFreq(10);
	m_ctrlHighThresh.SetLineSize(1);
	m_ctrlHighThresh.SetPageSize(10);
	m_ctrlHighThresh.SetPrimaryColor(RGB(227, 91, 91));	// reddish 	

	m_bEdgeView.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_bEdgeView.SetCheck(FALSE);
	m_bEdgeView.RedrawWindow();

	this->GetDlgItem(IDC_EDIT_CONTRAST2)->SetWindowTextA(NumberToString(m_edtSimMincontrast).c_str());
	this->GetDlgItem(IDC_EDIT_CONTRAST)->SetWindowTextA(NumberToString(m_edtSimContrast).c_str());

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}
void CMarkSearchTesterSheet::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar)
	{
		AcquireSRWLockExclusive(&theApp.m_pFrame->g_srwlock); // 동기화 시킴 Tkyuha 20211027				

		if (pScrollBar == (CScrollBar*)&m_ctrlLowThresh)
		{
			int nPos = m_ctrlLowThresh.GetPos();
			int high = m_ctrlHighThresh.GetPos();

			m_edtSimMincontrast = nPos;
			this->GetDlgItem(IDC_EDIT_CONTRAST2)->SetWindowTextA(NumberToString(m_edtSimMincontrast).c_str());
			m_pParentDlg->m_pMatchingProc->halcon_DisplayShapeModel(m_pParentDlg->m_pImage, nPos, high);
		}
		else if (pScrollBar == (CScrollBar*)&m_ctrlHighThresh)
		{
			int nPos = m_ctrlHighThresh.GetPos();
			int low = m_ctrlLowThresh.GetPos();

			m_edtSimContrast = nPos;

			m_pParentDlg->m_pMatchingProc->halcon_DisplayShapeModel(m_pParentDlg->m_pImage, low, nPos);
			this->GetDlgItem(IDC_EDIT_CONTRAST)->SetWindowTextA(NumberToString(m_edtSimContrast).c_str());
		}

		ReleaseSRWLockExclusive(&theApp.m_pFrame->g_srwlock);
	}

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CMarkSearchTesterSheet::OnBnClickedCheckEdgeView()
{
	BOOL bCheck = m_bEdgeView.GetCheck();
	m_pParentDlg->m_Viewer.SetFitMode();

	//m_pParentDlg->GetMaskDrawer().IgnoreMouse(bCheck);
	if (bCheck)
	{
		CRect rect;
		m_pParentDlg->m_Viewer.GetClientRect(&rect);

		m_pParentDlg->m_pMatchingProc->halcon_ShowModel(m_pParentDlg->m_pImage, m_pParentDlg->m_nWidth, m_pParentDlg->m_nHeight, rect, m_pParentDlg->m_Viewer.GetSafeHwnd());
	}
	else
	{
		m_pParentDlg->m_pMatchingProc->halcon_ShowModelClose();
	}
}


void CMarkSearchTesterSheet::OnBnClickedButtonGetparam()
{
	m_pParentDlg->GetParameterData();

	m_edtSimContrast = int(m_pParentDlg->m_pMatchingProc->m_ReadContrast);
	m_edtSimMincontrast = int(m_pParentDlg->m_pMatchingProc->m_ReadMinContrast);

	this->GetDlgItem(IDC_EDIT_CONTRAST2)->SetWindowTextA(NumberToString(m_edtSimMincontrast).c_str());
	this->GetDlgItem(IDC_EDIT_CONTRAST)->SetWindowTextA(NumberToString(m_edtSimContrast).c_str());

	m_ctrlLowThresh.SetPos(m_edtSimMincontrast);
	m_ctrlHighThresh.SetPos(m_edtSimContrast);
}


void CMarkSearchTesterSheet::OnBnClickedButtonSearch()
{
	_stPatternSelect pattern;
	pattern.nCam = m_pParentDlg->mark_nCam;
	pattern.nPos = m_pParentDlg->mark_nPos;
	pattern.nIndex = m_pParentDlg->mark_nIndex;
	pattern.nJob = m_pParentDlg->mark_nAlgo;

	if (m_pParentDlg->m_bUseCalibModel)
	{
		CWnd* pMain = theApp.m_pFrame->m_pPane[PANE_MACHINE];
		CWnd* pButton = pMain->GetDlgItem(IDC_BTN_PATTERN_SEARCH_MACHINE);
		pMain->PostMessage(WM_COMMAND, MAKEWPARAM(IDC_BTN_PATTERN_SEARCH_MACHINE, BN_CLICKED), (LPARAM)pButton->m_hWnd);
	}
	else
	{
		::SendMessageA(theApp.m_pFrame->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_PATTERN_SEARCH, (LPARAM)&pattern);
		::SendMessageA(theApp.m_pFrame->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_PATTERN_DISPLAY_RESULT, (LPARAM)&pattern);
	}
}

void CMarkSearchTesterSheet::OnBnClickedButtonApply()
{
	int nCam = m_pParentDlg->mark_nCam;
	int nPos = m_pParentDlg->mark_nPos;
	int nIndex = m_pParentDlg->mark_nIndex;
	int nJob = m_pParentDlg->mark_nAlgo;
	double offsetX = 0, offsetY = 0;

	ClearShapeModel();

	m_pParentDlg->SaveMaskImage();
	m_pParentDlg->UpdateDisplay();

	if (m_pParentDlg->m_pMatchingProc->getPreProcessingUse())
	{
		cv::Mat sgray;
		cv::Mat matImage(m_pParentDlg->m_nHeight, m_pParentDlg->m_nWidth, CV_8UC1, m_pParentDlg->m_pImage);
		cv::threshold(matImage, sgray, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
		m_pParentDlg->m_pMatchingProc->halcon_ReadModelNew(sgray.data, m_pParentDlg->m_MaskIMG, m_pParentDlg->m_nWidth, m_pParentDlg->m_nHeight);
		sgray.release();
	}
	else	m_pParentDlg->m_pMatchingProc->halcon_ReadModelNew(m_pParentDlg->m_pImage, m_pParentDlg->m_MaskIMG, m_pParentDlg->m_nWidth, m_pParentDlg->m_nHeight);

	if (m_pParentDlg->m_bUseCalibModel)
	{
		offsetX = ((CPaneMachine*)theApp.m_pFrame->m_pPane[PANE_MACHINE])->m_CaliboffsetX;
		offsetY = ((CPaneMachine*)theApp.m_pFrame->m_pPane[PANE_MACHINE])->m_CaliboffsetY;
	}
	else
	{
		offsetX = theApp.m_pFrame->GetMatching(nJob).getPosOffsetX(nCam, nPos, nIndex);
		offsetY = theApp.m_pFrame->GetMatching(nJob).getPosOffsetY(nCam, nPos, nIndex);
	}

	m_pParentDlg->m_pMatchingProc->halcon_SetShapeModelOrigin(offsetX, offsetY);

	OnBnClickedCheckEdgeView();
}

void CMarkSearchTesterSheet::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if (bShow) OnBnClickedCheckEdgeView();
	else ClearShapeModel();
}

// Lincoln Lee - 220221
void CMarkSearchTesterSheet::ClearShapeModel()
{
	m_pParentDlg->m_pMatchingProc->halcon_ShowModelClose();
}