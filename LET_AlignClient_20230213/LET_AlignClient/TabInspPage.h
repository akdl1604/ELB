#pragma once


// TabInspPage 대화 상자

class TabInspPage : public CDialogEx
{
	DECLARE_DYNAMIC(TabInspPage)

public:
	TabInspPage(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~TabInspPage();

	CLET_AlignClientDlg *m_pMain;
	HBRUSH m_hbrBkg;
	
	CRect m_rcForm;

	CSliderCtrl m_sliderLightSettingExist;
	CComboBox m_cmbSelectCamera;
	CButtonEx m_btnSizeOriginal;
	CButtonEx m_btnSizeFit;
	CButtonEx m_btnCameraLive;
	CButtonEx m_btnCameraStop;
	CButtonEx m_btnImageSave;
	CButtonEx m_btnImageOpen;
	CButtonEx m_btnManualInsp;
	CButtonEx m_btnDrawArea;
	CButtonEx m_btnFocusMeasure;
	CButtonEx m_btnPatternSave;
	CButtonEx m_btnSetRoiExist;
	CButtonEx m_btnCalcLimitOffset;

	CLabelEx  m_lbSelectChip;
	CLabelEx  m_lbSelectPosition;
	CLabelEx  m_lbAvgGrayLimit;
	CLabelEx  m_LbFocusEnergyTitle;
	CLabelEx  m_LbFocusEnergy;
	CLabelEx  m_lbJudgeCondition;
	CLabelEx  m_lbRoiSizeX;
	CLabelEx  m_lbRoiSizeY;
	CLabelEx  m_lbRoiOffsetX;
	CLabelEx  m_lbRoiOffsetY;
	CLabelEx  m_lbLimitOffset;

	int m_nResourceID[5];
	int m_nTabIndex;
	int m_nSelectPanel;	// Panel 유무 감지에 사용할 변수 몇번째 글라스 인지..!!!!
	int m_nSelectPanelExistIndex;	// Panel 유무 감지에 사용할 변수 몇번째 글라스 인지..!!!!
	int m_nSelectPatternIndex;
	int m_nSelectCamera;
	int m_nSelectPosition;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TAB_PANE_INSP };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnViewControl(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnBnClickedBtnSizeOriginal();
	afx_msg void OnBnClickedBtnSizeFit();
	afx_msg void OnBnClickedBtnCameraLive();
	afx_msg void OnBnClickedBtnCameraStop();
	afx_msg void OnBnClickedBtnImageSave();
	afx_msg void OnBnClickedBtnImageOpen();
	afx_msg void OnBnClickedBtnDrawArea();
	afx_msg void OnBnClickedBtnFocusMeasure();
	afx_msg void OnBnClickedBtnSetRoiExist();
	afx_msg void OnBnClickedBtnCalcLimitOffset();
	afx_msg void OnBnClickedBtnManualInsp();
	afx_msg void OnBnClickedBtnPatternSave();
	afx_msg void OnCbnSelchangeCbSelectChip();
	afx_msg void OnCbnSelchangeCbSelectPosition();
	afx_msg void OnCbnSelchangeCbJudgeCondition();
	afx_msg void OnEnSetfocusEditAvgGrayLimit();
	afx_msg void OnEnSetfocusEditLightSettingExist();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	void updateFrameDialog();
	void dispCameraLiveStop();
	void dispDrawArea();
	void dispViewSize();
	void dispPatternIndex();
	void dispMatchingRate();
	void dispFocusMeasure();
	void fnSaveLightValue(int nCam, int nChannel, int nIndex);
	void dispAvgGrayLimit();
	void dispJudgeCondition();
	void dispSelectPanel();
	void dispSelectPanelExistIndex();
	void InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color);
	void MainButtonInit(CButtonEx *pbutton, int size = 15);
	
	afx_msg void OnCbnSelchangeCbSelectCamera();
};
