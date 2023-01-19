#pragma once



// CFormMachineView 폼 뷰입니다.
class CLET_AlignClientDlg;
class CFormMachineView : public CFormView
{
	DECLARE_DYNCREATE(CFormMachineView)

protected:
	CFormMachineView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CFormMachineView();

public:
	enum { IDD = IDD_FORM_MACHINE };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
	CWinThread* m_pThread;
	CDlgViewer *m_pDlgViewerMachine[MAX_VIEWER];

	std::vector<cv::Point2f> m_ptrCalib[MAX_CAMERA];
	double m_dbCalibTargetX[MAX_CAMERA];
	double m_dbCalibTargetY[MAX_CAMERA];
	double m_dbCalibRotateX[MAX_CAMERA];
	double m_dbCalibRotateY[MAX_CAMERA];
	double m_dbCalibRad[MAX_CAMERA];

	CRect m_rcMachineViewer[MAX_VIEWER];
	CRect m_rcStaticViewerBaseMachine;
	HBRUSH m_hbrBkg;

	BOOL m_bDrawCalibration;
	BOOL m_bThreadMachineDisplayEndFlag;

	int m_nSelectAlgorithm;
	int m_nSelectAlgorithmPos;
	int m_nSelectAlgorithmCamera;

	int m_nTabC;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
private:
	CLET_AlignClientDlg *m_pMain;
	CLabelEx m_LbMachineViewTitle;
	CLabelEx m_LbMachineViewCalibTitle;
	CLabelEx m_LbMachineViewRotateTitle;
public:
	
	virtual void OnInitialUpdate();
	friend UINT Thread_MachineViewerDisplay(void *pParam);

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnViewControl(WPARAM wParam, LPARAM lParam);

	void MainButtonInit(CButtonEx *pbutton, int size = 15);
	void InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color);	
	void updateFrameDialog();
	void updateUiDialog();
	void showHideViewer();
	void replaceViewer(int id = 0);
	void displayFindMark(bool clear = FALSE);
	void displayFindMark(int nCount, int nCam, int nPos, bool clear = FALSE);
	void displayFindMarkIndividual();
	void display4PointCenterMark();	
	void displayCalibration(int nJob);
	void displayCalibrationCross();
	void createViewer_Machine(int nNumX, int nNumY, int id = -1);
	void createViewer_Machine_Fix(int nNumX, int nNumY,int id = -1);


	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void SetResCalibration();
};
