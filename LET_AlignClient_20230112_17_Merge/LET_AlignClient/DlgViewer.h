#pragma once


// CDlgViewer 대화 상자입니다.
#include "UI_Control\ViewerEx.h"
#include "UI_Control\GridCtrl_src\GridCtrl.h"

enum _BTN_FUNC_ {
	FUNC_ZOOM_IN = 0,
	FUNC_ZOOM_OUT,
	FUNC_DRAW_AREA,
	FUNC_ORIGIN_IMAGE,
	FUNC_FIT_IMAGE,
	FUNC_CENTER_LINE,
	FUNC_SHOW_OVERLAY,
	MAX_VIEWER_FUNC,
};

const int TIMER_VIEWER_UI_CEHCK = 12345;
const int TIMER_VIEWER_TOOL_UPDATE = 12346;
const int TIMER_VIEWER_TOOL_USE_UPDATE = 12347;
const int TIMER_VIEWER_TOOL_DISABLE_UPDATE = 12348;

class CDlgViewer : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgViewer)

public:
	CDlgViewer(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgViewer();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DLG_VIEWER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
private:
	CMFCToolBar m_wndViewToolBar;

	CEdit m_EditHistory;
	CTabCtrl m_TabViewer;
	int m_nGridInfo_H;
	int m_nGridInfo_W;
	int m_nFuncBtn_W;
	int m_nGridResult_H;
	int m_nViewID;
	CGridCtrl m_GridResult;
	ULONG_PTR m_gdiplusToken;
	CViewerEx m_Viewer;
	CButtonEx m_btnFunc[MAX_VIEWER_FUNC];
	CString m_strName;
	CWnd * m_pParent;
	BOOL m_bPopup;
	CRect m_rtDlgPopPos;
	CRect m_rtDlgChildPos;
	CRect m_rcViewerPopPos;
	BOOL m_bSizeChanged;
	BOOL m_bUsePopupWindow;
	BOOL m_bShowGridInfo;
	BOOL m_bShowTabCtrl;
	CString m_strHistory;
	BOOL m_bMaximize;
private:
	void PopupWindow(BOOL bPopup);
public:
	BOOL m_bLiveCheck;
	BOOL m_bEnableGrab;
	CString m_strPanelName;

	CViewerEx &GetViewer() { return m_Viewer; }
	void setUsePopupWindow(BOOL bSet) { m_bUsePopupWindow = bSet; }
	void setShowTabCtrl(BOOL bSet) { m_bShowTabCtrl = bSet; }
	void setShowGridInfo(BOOL bSet) { m_bShowGridInfo = bSet; }
	void setDlgChildPos(CRect rectPos) { m_rtDlgChildPos = rectPos; }
	CRect getDlgChildPos() { return m_rtDlgChildPos; }
	void setParent(CWnd *pParent) { m_pParent = pParent; }
	void setViewerName(CString name);
	void setPanelID_Name(CString name);
	void setViewID(int id = 0) { m_nViewID = id; }
	int  getViewID() { return m_nViewID; }

	void setViewerJudge(BOOL bJudge);
	void resetViewerJudge();

	void addHistory(CString strText);
	
	void updateViewerFunc();
	void displayTabUI();
	void upDateControlUI(BOOL bEnable);
	void fitViewer();
	void childWindow();
	void InitGridInfo(CGridCtrl &grid, int nCols, int nRows);
	void drawGridInfo(CGridCtrl &grid);
	void InitToolBar();
	void EnableToolButton(int iCmd,BOOL bstatus);
	
	void InitGridResult(CGridCtrl &grid, int nCols, int nRows);
	void drawGridResult(CGridCtrl &grid);
	void InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color);
	void MainButtonInit(CButtonEx *pbutton, int nID = -1, int size = 15);

	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnExitSizeMove();
	afx_msg void OnTcnSelchangeTabInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnViewControl(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt);
	
	afx_msg void OnCommandFirst();
	afx_msg void OnCommandSecond();
	afx_msg void OnCommandThird();
	afx_msg void OnCommandFour();
	afx_msg void OnCommandLive();
	afx_msg void OnCommandRecord();
	afx_msg void OnCommandStop();
	afx_msg LRESULT OnKickIdle(WPARAM wParam, LPARAM lParam);

	afx_msg void OnBnClickedBtnOriginalImage();
	afx_msg void OnBnClickedBtnFitImage();
	afx_msg void OnBnClickedBtnCenterLine();
	afx_msg void OnBnClickedBtnDrawArea();
	afx_msg void OnBnClickedBtnZoomIn();
	afx_msg void OnBnClickedBtnZoomOut();
	afx_msg void OnBnClickedBtnDlgShowOverlay();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	CRITICAL_SECTION m_csWindows;

	CGridCtrl m_GridInfo; // KBJ private -> public
};

