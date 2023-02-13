// FormTrendView.cpp: 구현 파일
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "FormTrendView.h"



// CFormTrendView

IMPLEMENT_DYNCREATE(CFormTrendView, CFormView)

CFormTrendView::CFormTrendView()
	: CFormView(IDD_FORM_TREND)
{
	for (int i = 0; i < MAX_CAMERA; i++)
		c_TabTrendPage[i] = NULL;
}

CFormTrendView::~CFormTrendView()
{
	for (int i = 0; i < MAX_CAMERA; i++)
	{
		if(c_TabTrendPage[i]) delete c_TabTrendPage[i];
	}
}

void CFormTrendView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LB_TREND_VIEW_TITLE, m_LbTrendViewTitle);
}

BEGIN_MESSAGE_MAP(CFormTrendView, CFormView)
	ON_WM_CTLCOLOR()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CFormTrendView 진단

#ifdef _DEBUG
void CFormTrendView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CFormTrendView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

HBRUSH CFormTrendView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{

	return m_hbrBkg;
}

// CFormTrendView 메시지 처리기
void CFormTrendView::updateFrame(bool bshow)
{
	for (int i = 0; i < MAX_CAMERA; i++)
	{
		if (c_TabTrendPage[i] == NULL) continue;

		if (bshow)		c_TabTrendPage[i]->ShowWindow(SW_SHOW);
		else c_TabTrendPage[i]->ShowWindow(SW_HIDE);
	}
}

void CFormTrendView::updateDataBase()
{
	CLET_AlignClientDlg* m_pMain = (CLET_AlignClientDlg*)AfxGetMainWnd();

	int nJobCount = int(m_pMain->vt_job_info.size());

	for (int i = 0; i < nJobCount; i++)
	{
		c_TabTrendPage[i]->UpdateDatabase();
	}	
}

void CFormTrendView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);

	GetDlgItem(IDC_STATIC_VIEWER_BASE_TREND)->GetWindowRect(&m_rcStaticViewerBaseTrend);
	GetDlgItem(IDC_STATIC_VIEWER_BASE_TREND)->ShowWindow(SW_HIDE);

	InitTitle(&m_LbTrendViewTitle, "Trend View", 24.f, RGB(192, 64, 64));
	init_report_algorithm();
}

void CFormTrendView::InitTitle(CLabelEx* pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}

void CFormTrendView::init_report_algorithm()
{
	CLET_AlignClientDlg *m_pMain = (CLET_AlignClientDlg*)AfxGetMainWnd();

	int nJobCount = int(m_pMain->vt_job_info.size());

	for (int i = 0; i < nJobCount; i++)
	{
		c_TabTrendPage[i] = new CTabTrendReportPage;
		c_TabTrendPage[i]->Create(IDD_TAB_PANE_TREND_REPORT,this);
		c_TabTrendPage[i]->m_nJobID = i;
	}

	int w = nJobCount>0?m_rcStaticViewerBaseTrend.Width()/ nJobCount: m_rcStaticViewerBaseTrend.Width();
	int h = m_rcStaticViewerBaseTrend.Height();
	int startOffX = m_rcStaticViewerBaseTrend.left;
	int startOffY = m_rcStaticViewerBaseTrend.top;

	for (int i = 0; i < nJobCount; i++)
	{	
		c_TabTrendPage[i]->SetTitle(m_pMain->vt_viewer_info[i].viewer_name.c_str());
		c_TabTrendPage[i]->SetWindowPos(this, startOffX, startOffY, w, h, SWP_HIDEWINDOW | SWP_NOZORDER);
		startOffX += w;
	}
}

void CFormTrendView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CFormView::OnShowWindow(bShow, nStatus);

	updateFrame((bool)bShow);
}
