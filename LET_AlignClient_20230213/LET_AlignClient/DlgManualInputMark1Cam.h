#pragma once
#include "afxwin.h"
#include "LET_AlignClient.h"

#ifndef __LABEL_EX_H__
#define __LABEL_EX_H__
#include "UI_Control\LabelEx.h"
#endif
// CDlgManualInputMark1Cam dialog

class CDlgManualInputMark1Cam : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgManualInputMark1Cam)

public:
	CDlgManualInputMark1Cam(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgManualInputMark1Cam();

// Dialog Data
	enum { IDD = IDD_DLG_MANUAL_INPUT_MARK_1CAM };

	int job_id;
	int m_nTotalPosition;
	CString m_StrPositionName[4];

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	BOOL m_bTimerTogle;
	BOOL m_bManualInputComplete;	// Manual 입력을 완료했다는 Flag
	BOOL m_bJudge[4];
	CRect m_rcBtnPos[6];
	HBRUSH m_hbrBkg;
	
	CButtonEx m_btnSelectPosition[4];
	CButtonEx m_btnInputNG;
	CButtonEx m_btnInputOK;
	CButtonEx m_btnZoomIn;
	CButtonEx m_btnZoomOut;
	CButtonEx m_btnZoomMove;

	CLabelEx m_lblLeftRotate;
	CLabelEx m_lblRightRotate;
	CLabelEx m_stt_BtnPos[4];
	CLabelEx m_stt_ManualInputTitle;
	
	CViewerEx m_ImageManualInput[4];
	
	int m_nResult;
	int m_nSelectPosition;
	int m_nShapeType;

	double m_dbManualPosX[4];
	double m_dbManualPosY[4];		
	double m_AlignTargetAngle;

	std::vector<std::function<void(int, int, int, int, CPoint&, CPoint&, CViewerEx*)>> m_OffsetFunctions = {
		[&](int dx, int dy, int dx1, int dy1, CPoint& start, CPoint& end, CViewerEx* viewer) { viewer->SetBeginEnd(CPoint(start.x - dx, start.y), CPoint(end.x - dx1, end.y)); },
		[&](int dx, int dy, int dx1, int dy1, CPoint& start, CPoint& end, CViewerEx* viewer) { viewer->SetBeginEnd(CPoint(start.x + dx, start.y), CPoint(end.x + dx1, end.y)); },
		[&](int dx, int dy, int dx1, int dy1, CPoint& start, CPoint& end, CViewerEx* viewer) { viewer->SetBeginEnd(CPoint(start.x, start.y - dy), CPoint(end.x, end.y - dy1)); },
		[&](int dx, int dy, int dx1, int dy1, CPoint& start, CPoint& end, CViewerEx* viewer) { viewer->SetBeginEnd(CPoint(start.x, start.y + dy), CPoint(end.x, end.y + dy1)); },
		[&](int dx, int dy, int dx1, int dy1, CPoint& start, CPoint& end, CViewerEx* viewer) { m_AlignTargetAngle -= 0.2; drawMarkGuidePostion(viewer, TRUE); },
		[&](int dx, int dy, int dx1, int dy1, CPoint& start, CPoint& end, CViewerEx* viewer) { m_AlignTargetAngle += 0.2; drawMarkGuidePostion(viewer, TRUE); }
	};

public:
	afx_msg void OnBnClickedBtnInputComplete();
	afx_msg void OnBnClickedBtnManualInputNg();
	afx_msg void OnBnClickedBtnZoomIn();
	afx_msg void OnBnClickedBtnZoomOut();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

	virtual BOOL OnInitDialog();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	void drawMarkPostion(BOOL bErase);
	void drawMarkGuidePostion(CViewerEx *viewer,BOOL bErase);
	void InitTitle(CLabelEx *pTitle,CString str,float size,COLORREF color);
	void dispSelectPosition();
	void dispViewer();
	void initializeParameter(int shape=0);
	void setJudge(int nCam, BOOL bJudge) { m_bJudge[nCam] = bJudge; }
	void calcManualPosition(int nPos);
	void initManualInputViewer();	
	void initManualInputTracker(int shape,CPoint *st = NULL, CPoint *et=NULL);
	void MainButtonInit(CButtonEx *pbutton,int size = 15);
	void MainButtonIconInit(CButtonEx* pbutton, int nID = -1, int size = 15);
	void fnBtnDisplay();
	void EnableMove();
	afx_msg void OnBnClickedBtnDlgMove();
	afx_msg void OnBnClickedBtnSelectPosition1();
	afx_msg void OnBnClickedBtnSelectPosition2();
	afx_msg void OnBnClickedBtnSelectPosition3();
	afx_msg void OnBnClickedBtnSelectPosition4();
	afx_msg void OnClose();
};
