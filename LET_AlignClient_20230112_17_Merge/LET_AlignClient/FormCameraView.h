#pragma once
#include "stdafx.h"
#include "DlgProfile.h"	
// CFormCameraView �� ���Դϴ�.
class CLET_AlignClientDlg;

class CFormCameraView : public CFormView
{
	DECLARE_DYNCREATE(CFormCameraView)

protected:
	CFormCameraView();           // ���� ����⿡ ���Ǵ� protected �������Դϴ�.
	virtual ~CFormCameraView();

	int m_nSelViewerNum;

public:
	enum { IDD = IDD_FORM_CAMERA };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
	CLET_AlignClientDlg *m_pMain;
	CDlgProfile *m_pProfile;	

	CWinThread* m_pThread;

	CLabelEx m_LbCameraViewTitle;
	CLabelEx m_LbImageInfo;
	CLabelEx m_LbImageInfo2;
	CLabelEx m_LbImageInfo3;
	CViewerEx m_ViewerCamera;

	HBRUSH m_hbrBkg;

	BOOL m_bThreadViewDisplayEndFlag;

	int curr_viewer;
	int curr_cam;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()	
public:	
	
	friend UINT Thread_ViewerDisplay(void *pParam);
	virtual void OnInitialUpdate();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnViewControl(WPARAM wParam, LPARAM lParam);

	void MainButtonInit(CButtonEx *pbutton, int size = 15);
	void InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color);
	void updateFrame();
	void dispFocusMeasure(int n);
	double GetFocusMeasure(BYTE *pImg, int w, int h, CRect roi);
	double ImageSharpness(BYTE* img, CRect roi); //20211229
	void InspectionProcessing(int id);
	float calcAngleFromPoints(cv::Point2f _ptFirstPos, cv::Point2f _ptSecondPos);
	int CircleLineIntersection(float x, float y, float r, float a, float b, float c, float d, float xy[][2]);
	void SetViewerNum(int n = 0) { m_nSelViewerNum = n; }
	void SetViewerInit(int view);
	void init_camera_viewer(int viewer, int cam);
	void patternRegist(_stPatternSelect *pInfo);
	void patternSetROI(_stPatternSelect *pInfo);	//20.03.26
	void patternSearch(_stPatternSelect *pInfo);	
	void patternSearchCaliper(_stPatternSelect* pInfo);
	void patternSearchCaliper_matching_line_theta(_stPatternSelect *pInfo);
	void patternCalcWholeROI(_stPatternSelect *pInfo);
	void patternDisplayResult(_stPatternSelect *pInfo);
	void draw_CaliperLine(LPARAM lParam);
	void drawExistInspResult(LPARAM lParam); //210110
	void drawDistanceInspResult2(LPARAM lParam); //210208
	void drawAssembleCALC(LPARAM lParam);	//210210
	void drawReference(LPARAM lParam);
	void drawFixture(LPARAM lParam);
	void displayCartToPolar(cv::Mat magnitude, cv::Mat angle); //20210924 Tkyuha
	void drawaAllReference(LPARAM lParam);
};



