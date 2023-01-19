#pragma once


#include "TabTrendReportPage.h"

// CFormTrendView 폼 보기

class CFormTrendView : public CFormView
{
	DECLARE_DYNCREATE(CFormTrendView)

protected:
	CFormTrendView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CFormTrendView();

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FORM_TREND };
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

	HBRUSH m_hbrBkg;
	CLabelEx m_LbTrendViewTitle;

	CRect m_rcStaticViewerBaseTrend;

	CTabTrendReportPage* c_TabTrendPage[MAX_CAMERA];

	void updateFrame(bool bshow=TRUE);
	void updateDataBase();
	void init_report_algorithm();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	void InitTitle(CLabelEx* pTitle, CString str, float size, COLORREF color);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};


