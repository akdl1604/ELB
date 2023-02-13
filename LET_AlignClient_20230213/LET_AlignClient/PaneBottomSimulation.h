#pragma once



class CLET_AlignClientDlg;
class CPaneBottomSimulation : public CFormView
{
	DECLARE_DYNCREATE(CPaneBottomSimulation)

protected:
	CPaneBottomSimulation();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CPaneBottomSimulation();

public:
	enum { IDD = IDD_PANE_BOTTOM_SIMULATION };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

	CLET_AlignClientDlg *m_pMain;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void OnInitialUpdate();
	DECLARE_MESSAGE_MAP()

public:

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnSimulation();
	afx_msg void OnBnClickedBtnSimulationStop();
	afx_msg void OnBnClickedBtnExit();

	CButtonEx m_btnSimulationStart;
	CButtonEx m_btnSimulationStop;
	CButtonEx m_btnExit;
	CRect m_rcForm;
	HBRUSH m_hbrBkg;
	BOOL m_bSimulStopFlag;
	BOOL m_bUseSimulMessage;
	std::vector < CString > m_vtDirPath;

	void MainButtonInit(CButtonEx *pbutton, int nID = -1, int size = 15);
	void InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color);
	void OnPaint();

	void set_simulation_start_stop(BOOL bSimulation);
	void update_frame_dialog();
};
