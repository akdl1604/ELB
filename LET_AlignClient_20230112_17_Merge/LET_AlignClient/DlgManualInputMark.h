#pragma once
#include "afxwin.h"
#include "LET_AlignClient.h"

#ifndef __LABEL_EX_H__
#define __LABEL_EX_H__
#include "UI_Control\LabelEx.h"
#endif
// CDlgManualInputMark dialog

class CDlgManualInputMark : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgManualInputMark)

public:
	CDlgManualInputMark(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgManualInputMark();

// Dialog Data
	enum { IDD = IDD_DLG_MANUAL_INPUT_MARK };

	int job_id;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	BOOL m_bTimerTogle;
	BOOL m_bManualInputComplete;	// Manual 입력을 완료했다는 Flag
	BOOL m_bJudge[2];
	CRect m_rcBtnPos[6];
	HBRUSH m_hbrBkg;

	CButtonEx m_btnSelectCamera[2];
	CButtonEx m_btnInputNG;
	CButtonEx m_btnInputOK;
	CButtonEx m_btnZoomIn;
	CButtonEx m_btnZoomOut;
	CButtonEx m_btnZoomMove;

	CLabelEx m_lblLeftRotate;
	CLabelEx m_lblRightRotate;
	CLabelEx m_stt_BtnPos[4];
	CLabelEx m_stt_ManualInputTitle;
	CLabelEx m_stt_ManualInputCameraTitle[2];
	
	CViewerEx m_ImageManualInput[2];
	CViewerEx m_ImageManualInputMini[2];
	
	int m_nResult;
	int m_nSelectCamera;
	int m_nShapeType;

	double m_dbManualPosX[2];
	double m_dbManualPosY[2];		
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
	afx_msg void OnBnClickedBtnSelectCam1();
	afx_msg void OnBnClickedBtnSelectCam2();
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
	void dispSelectCamera();
	void dispCameraTitle(int nCam, BOOL bOK);
	void dispViewer();
	void initializeParameter(int shape=0);
	void setJudge(int nCam, BOOL bJudge) { m_bJudge[nCam] = bJudge; }
	void calcManualPosition(int nViewer);
	void initManualInputViewer();	
	void initManualInputTracker(int shape,CPoint *st = NULL, CPoint *et=NULL);
	void MainButtonInit(CButtonEx *pbutton,int size = 15);
	void MainButtonIconInit(CButtonEx* pbutton, int nID = -1, int size = 15);
	void fnBtnDisplay(BOOL bJudge1, BOOL bJudge2);
	afx_msg void OnBnClickedBtnDlgMove();
};
