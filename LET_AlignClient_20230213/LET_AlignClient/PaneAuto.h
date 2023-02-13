#pragma once


#include "CBSpeedListCtrl.h"
#include "afxwin.h"
#include "ChartCtrl/ChartCtrl.h"
#include "ChartAxisLabel.h"
#include "ChartLineSerie.h"
#include "TabCtrlEx.h"
#include "Pegrpapi.h"
#include "NetworkPerformanceItem.h"

#define GetRandom( min, max ) ((rand() % (int)(((max)+1) - (min))) + (min))
#define MAX_RESULT_ITEM 100

// CPaneAuto 폼 뷰입니다.
class CPaneAuto : public CFormView
{
	DECLARE_DYNCREATE(CPaneAuto)

protected:
	CPaneAuto();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CPaneAuto();

public:
	enum { IDD = IDD_PANE_AUTO };
	enum {
		ALIGN_RESULT,
		INSPECTION_RESULT,
	};
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
	

	CLET_AlignClientDlg *m_pMain;
	CLabelEx m_stt_DailyProduction;

	CLabelEx m_stt_Data_Title;
	CLabelEx m_stt_Data_Title_GY;
	CLabelEx m_stt_Data_Title_Day;
	CLabelEx m_stt_Data_Title_SW;
	CLabelEx m_stt_Data_Title_Count_Total;
	CLabelEx m_stt_Data_Title_Percent;

	CLabelEx m_stt_Data_Title_NG;
	CLabelEx m_stt_Data_Title_OK;
	CLabelEx m_stt_Data_Title_NG_Total;

	CLabelEx m_stt_Data_NG_GY;
	CLabelEx m_stt_Data_NG_Day;
	CLabelEx m_stt_Data_NG_SW;
	CLabelEx m_stt_Data_NG_Total;
	CLabelEx m_stt_Data_NG_Percent;

	CLabelEx m_stt_Data_OK_GY;
	CLabelEx m_stt_Data_OK_Day;
	CLabelEx m_stt_Data_OK_SW;
	CLabelEx m_stt_Data_OK_Total;
	CLabelEx m_stt_Data_OK_Percent;

	CLabelEx m_stt_Data_Total_GY;
	CLabelEx m_stt_Data_Total_Day;
	CLabelEx m_stt_Data_Total_SW;
	CLabelEx m_stt_Data_Total_Total;
	CLabelEx m_stt_Data_Total_Percent;

	CLabelEx m_lblIInspectionResult;
	CLabelEx m_lblCpuUse;
	CLabelEx m_lblMemoryUse;
	CLabelEx m_lblCDriveUse;
	CLabelEx m_lblDDriveUse;

	CString stringToCurrency(int nNum);

	CCBSpeedListCtrl	m_listInspection[MAX_JOB];
	CProgressCtrl m_pCpuProgressCtrl;
	CProgressCtrl m_pMemoryProgressCtrl;
	CProgressCtrl m_pCDriveProgressCtrl;
	CProgressCtrl m_pDDriveProgressCtrl;

	CButtonEx m_btnMainView;
	CButtonEx m_btnInterfaveView;
	CButtonEx m_btnMainTrendView;
	CButtonEx m_btnResultList;

	CTabCtrlEx m_TabSpecView;
	CTabCtrlEx m_TabResultView;

	NetworkPerformanceScanner m_cPcPerformance;
	CpuUsage  m_cPcCpuPerformance;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	DECLARE_MESSAGE_MAP()
private:
	CRITICAL_SECTION	m_csProcessHistory;
	CRITICAL_SECTION	m_csGridDraw;
	CChartCtrl			m_AutoChartCtrl;				//차트
	CChartXYSerie*		m_pAutoLineSeries[12];			//차트에 들어갈 데이터 축
	CString				m_pAutoShiftResult[11][df_MAX_CNT + 2];

public:
	CRect m_rcForm;
	CEdit m_EditProcess;
	CString m_strProcess;
	HBRUSH m_hbrBkg;
	void InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color);
	void InitTitle(CLabelEx *pTitle, float size, COLORREF bgcolor, COLORREF color);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual void OnInitialUpdate();
	void MainButtonInit(CButtonEx *pbutton, int size = 15);
	void addProcessHistory(CString str);
	afx_msg LRESULT OnViewControl(WPARAM wParam, LPARAM lPara);
	afx_msg void OnPaint();
	afx_msg void OnStnDblclickStaticDailyProduction();
	afx_msg void OnNMCustomdrawListControl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnAutoMain();
	afx_msg void OnBnClickedBtnAutoPlcInterface();
	CLabelEx m_lblHistoryLabel;
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	BOOL IsClickDailyTitle(CPoint pt);
	
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	float GetCPKData(std::vector<float> vectorData, BOOL bGetX);
	LRESULT OnModelChange(WPARAM wParam, LPARAM lParam);
	void SetMinMaxValue(double min, double max);
	void UpdateChartData(std::vector<float> *vecData);	

	void DrawAlignResultChart(int(lParam));
	int nresultcnt;
	CGridCtrl m_grid_result_viewer;
	CGridCtrl m_grid_spec_viewer;
	CGridCtrl m_grid_insp_spec_viewer;
	CGridCtrl m_grid_InspResult_1_viewer;
	CGridCtrl m_grid_InspResult_2_viewer;
	CGridCtrl m_grid_InspResult_3_viewer;
	CGridCtrl m_grid_ref_data_viewer;
	CGridCtrl m_grid_proc_time_viewer;

	void Draw2camViewer(int algo);
	void Draw4camViewer(int algo);
	void DrawELBCenterViewer(int algo);
	void DrawPFInspectionViewer(int algo,int method);
	void DrawPFReelAlignViewer(int algo, int pos);
	void DrawCenterNozzleAlignViewer(int algo);
	void DrawStageZGapViewer(int algo, int real_cam);
	void DrawSpecChart();
	void DrawSpecChart_Scan_Insp(int nJob, int nPos);
	void init_spec_view_setting(CGridCtrl& grid, int col, int row);
	void init_ref_data_view_setting(CGridCtrl& grid, int col, int row);
	void draw_grid_spec_viewer(CGridCtrl &grid, int col, int row);
	void draw_grid_insp_spec_viewer(CGridCtrl& grid, int col, int row);
	void draw_grid_inspection_spec_viewer(CGridCtrl& grid, int t_col, int t_row);
	void draw_grid_ref_data_viewer(CGridCtrl& grid, int col);
	void draw_grid_proc_time_viewer(CGridCtrl& grid, int col);
	void update_grid_inspection_spec_viewer();
	void update_grid_inspection_spec();
	void update_grid_processing_time(int cmd, double msgTime =0, int pos = 0, bool judge = true);
	
	//KJH 2021-08-07 Scan Insp Spec Grid 분기
	void Init_Grid_Scan_Insp_Spec_View_Setting(CGridCtrl& grid, int col, int row);
	void Init_Grid_Scan_Insp_Result_View_Setting(CGridCtrl& grid, int col, int row);
	void Init_Draw_Grid_Scan_Insp_Spec_Viewer(CGridCtrl& grid, int col, int row);
	void Init_Draw_Grid_Scan_Insp_Result_1_Viewer(CGridCtrl& grid, int col, int row);
	void Init_Draw_Grid_Scan_Insp_Result_2_Viewer(CGridCtrl& grid, int col, int row);
	void Init_Draw_Grid_Scan_Insp_Result_3_Viewer(CGridCtrl& grid, int col, int row);
	void Update_Grid_Scan_Insp_Spec();
	void Update_Grid_Scan_Insp_Result_Viewer(int nJob, int nPos);
	void Move_Grid_Scan_Insp_Result_Viewer(int nPos);

	void move_grid_inspection_spec_viewer();
	void update_grid_spec_viewer();
	void update_grid_dustResult_spec();
	int nCamcnt;
	std::vector<int> nAlgoNum;
	int nAlgo[MAX_CAMERA][MAX_CAMERA];
	CLabelEx m_lblSpec;
	CLabelEx m_lblDopoValue;
	CLabelEx m_lblSuckValue;
	CLabelEx m_lblLeftPanelJudge;
	CLabelEx m_lblRightPanelJudge;

	void DrawInspResultChart(int(lParam));
	void DrawInsViewer(int algo);
	void SaveRefDataCSV();
	void LoadRefData();
	CString m_strTime;
	CString getResultTime() { return m_strTime; };
	void setResultTime(CString str) { m_strTime = str; };
	afx_msg void OnNMDblclkListInspection(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListInspection2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkListInspection3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkListInspection4(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkListInspection5(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkListInspection6(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkListInspection7(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkListInspection8(NMHDR* pNMHDR, LRESULT* pResult);
	void PathFromList(int row);

	BOOL m_bELBInspJudge;
	afx_msg void OnTcnSelchangeTabSpecview(NMHDR* pNMHDR, LRESULT* pResult);
	HWND m_hPE;
	void CreateSimpleGraph();
	void GraphBackgroundBitmap();
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	
	void CreateTabCount();
	afx_msg void OnTcnSelchangeTabResultview(NMHDR* pNMHDR, LRESULT* pResult);
	void CreateResultInit(int cul,int nAlgo);
	
	void RenewResultChart(int nIndex);
	void RenewSpecChart(int nIndex);

	afx_msg void OnNMCustomdrawListInspection2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMCustomdrawListInspection3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMCustomdrawListInspection4(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMCustomdrawListInspection5(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMCustomdrawListInspection6(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMCustomdrawListInspection7(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMCustomdrawListInspection8(NMHDR* pNMHDR, LRESULT* pResult);
	void ChangeCustomResultList(int nJob,NMHDR* pNMHDR, LRESULT* pResult);

	int m_nObject;
	BOOL m_bResultInit[MAX_JOB];

	int m_nAlignCount;
	int m_nInspCount;
	int m_nAlignAlgo[MAX_JOB];
	int m_nInspAlgo[MAX_JOB];
	BOOL m_bInsp;
	CString m_LastReusltDate[MAX_JOB];

	BOOL m_bJudge[MAX_JOB][MAX_RESULT_ITEM + 2];
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg void OnBnClickedBtnAutoTrend();
	BOOL find_center_video_file(CString strVideoDir);
	BOOL find_center_video_panel_directory(CString UpDir, CString strPanelID, CString* strPanelDir);
	BOOL find_center_image_panel_directory(CString UpDir, CString strPanelID, CString* strPanelDir, BOOL bJudge);
	afx_msg void OnBnClickedBtnAutoResultList();

	CRect m_Rect;

	CString m_InspComment;
};


