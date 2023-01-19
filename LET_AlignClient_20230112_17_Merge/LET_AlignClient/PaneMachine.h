#pragma once
#include "afxwin.h"
#include "ListCtrlStyled.h"
#include "TabCtrlEx.h"
#include "TabMachineAlgoPage.h"
#include "PatternMatching.h"

#ifndef _U_POINT_
#define _U_POINT_
template <class T>
struct UPoint {
	T x;
	T y;
};
#endif


// CPaneMachine 폼 뷰입니다.
class CLET_AlignClientDlg;

class CPaneMachine : public CFormView
{
	DECLARE_DYNCREATE(CPaneMachine)

protected:
	CPaneMachine();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CPaneMachine();

public:
	enum { IDD = IDD_PANE_MACHINE };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

	CWnd *c_pWnd;
	CLET_AlignClientDlg *m_pMain;

	TabMachineAlgoPage *c_TabMachinePage[MAX_CAMERA];

	CButtonEx m_btnMachineClose;
	CButtonEx m_btnMachineSave;
	CButtonEx m_btnDrawArea_Machine;
	CButtonEx m_btnPatternRegist_Machine;
	CButtonEx m_btnPatternSearch_Machine;
	CButtonEx m_btnPatternView_Machine;

	CTabCtrlEx m_hSelectAlgoTab;
	
	HBRUSH m_hbrBkg;	

	double m_CaliboffsetX;
	double m_CaliboffsetY;
	CHalconProcessing m_CalibHalcon[4][MAX_PATTERN_INDEX]; //20211204  Tkyuha 최대 카메라 4개 동시에 켈리브레이션 수행 //KJH 2021-12-29 MarkIndex추가
	CFindInfo m_CalibFindInfo;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
private:	
	

public:	
	virtual void OnInitialUpdate();

	afx_msg void OnSelchangeTabList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnViewControl(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedBtnMachineClose();
	afx_msg void OnBnClickedBtnCalibSave();
	afx_msg void OnBnClickedBtnPatternRegistMachine();
	afx_msg void OnBnClickedBtnPatternViewMachine();
	afx_msg void OnBnClickedBtnPatternSearchMachine();

	void InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color);	
	void MainButtonInit(CButtonEx *pbutton, int size = 15);
	void CreateGrid();	
	int getCurrentMachineAlgorithm();
	void initLightControl();
	BOOL findPattern_Matching(BYTE* pImage, int nCam, int nPos, int m_nWidth, int m_nHeight, CRect rcInspROI);
	void UpdatePatternIndex();
	
	afx_msg void OnBnClickedBtnDrawAreaMachine();
};


