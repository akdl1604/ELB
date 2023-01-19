#pragma once



// CPaneBottom 폼 뷰입니다.
class CLET_AlignClientDlg;
class CPaneBottom : public CFormView
{
	DECLARE_DYNCREATE(CPaneBottom)

protected:
	CPaneBottom();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CPaneBottom();

public:
	enum { IDD = IDD_PANE_BOTTOM };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

	CRect m_rcForm;

	CButtonEx m_btnAutoStart;
	CButtonEx m_btnAutoStop;
	CButtonEx m_btnExit;
	CLET_AlignClientDlg *m_pMain;
	HBRUSH m_hbrBkg;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual void OnInitialUpdate();
	DECLARE_MESSAGE_MAP()

public:
	void MainButtonInit(CButtonEx *pbutton, int nID = -1, int size = 15);
	void InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color);
	void SetAutoStartStop(BOOL bAuto);

public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnAutoStop();
	afx_msg void OnBnClickedBtnAutoStart();
	afx_msg void OnBnClickedBtnExit();
	afx_msg void OnPaint();
};


