#pragma once

#include "UI_Control/EasySize.h"

// CTabTrendReportPage 대화 상자

class CTabTrendReportPage : public CDialogEx
{
	DECLARE_DYNAMIC(CTabTrendReportPage)
	DECLARE_EASYSIZE

public:
	CTabTrendReportPage(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CTabTrendReportPage();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TAB_PANE_TREND_REPORT };
#endif
	HBRUSH m_hbrBkg;
	BOOL m_bInitComplete;

	CButtonEx m_btnDailyClear;
	CButtonEx m_btnDailyHourClear;
	CLabelEx m_LbDailyTitle;
	CLabelEx m_LbDailyHourTitle;

	CGridCtrl grid_daily_report;
	CGridCtrl grid_time_report;

	CChartCtrl m_TrendChartCtrl;
	CChartPointsSerie* m_pPointsSeries[5];

	int m_nJobID;
	int m_nHoursOK[25], m_nHoursNG[25];
	int m_nDailysOK[32], m_nDailysNG[32];

	float *m_nScatterChartData[15];
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnDailyClear2();
	afx_msg void OnBnClickedBtnDailyClear();

	void SetTitle(CString title);
	void MainButtonInit(CButtonEx* pbutton, int size = 15);
	void InitTitle(CLabelEx* pTitle, float size, COLORREF bgcolor, COLORREF color);
	void init_report_grid(CGridCtrl& grid, int col, int row);
	void draw_grid_report(CGridCtrl& grid);
	
	void CreateScatterGraph();
	void UpdateDatabase();	
	void UpdateScatterGraph(int n=1);
};
