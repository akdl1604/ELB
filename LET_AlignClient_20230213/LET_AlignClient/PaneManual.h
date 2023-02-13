#pragma once



// CPaneManual 폼 뷰입니다.

class CPaneManual : public CFormView
{
	DECLARE_DYNCREATE(CPaneManual)

protected:
	CPaneManual();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CPaneManual();

public:
	enum { IDD = IDD_PANE_MANUAL };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

	HBRUSH m_hbrBkg;
	CRect m_rcForm;
	CButtonEx m_btnModel;
	CButtonEx m_btnCamera;
	CButtonEx m_btnMachine;
	CButtonEx m_btnInterfacePlc;
	CButtonEx m_btnSimulation;
	CButtonEx m_btnSystemOption;
	CButtonEx m_btnTrendView;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnViewControl(WPARAM wParam, LPARAM lPara);
	afx_msg void OnBnClickedBtnCamera();
	afx_msg void OnBnClickedBtnModel();
	afx_msg void OnBnClickedBtnMachine();
	afx_msg void OnBnClickedBtnInterface();
	afx_msg void OnPaint();

	void InitTitle(CLabelEx *pTitle, float size, COLORREF bgcolor = RGB(64, 64, 64), COLORREF color = RGB(255, 255, 255));
	void InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color = RGB(64, 64, 64));
	void MainButtonInit(CButtonEx *pbutton, int size = 15);
	afx_msg void OnBnClickedBtnSimulation();
	afx_msg void OnBnClickedBtnSystemoption();
	afx_msg void OnBnClickedBtnTrend();
};


