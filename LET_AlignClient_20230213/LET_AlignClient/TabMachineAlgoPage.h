#pragma once

#include "EditEx.h"
#include "CheckEx.h"

// TabMachineAlgoPage 대화 상자
class COffsetDlg;
class CCramerSRuleDlg;
class CFindInfo;
class TabMachineAlgoPage : public CDialogEx
{
	DECLARE_DYNAMIC(TabMachineAlgoPage)

public:
	TabMachineAlgoPage(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~TabMachineAlgoPage();

	CLET_AlignClientDlg *m_pMain;
	HBRUSH m_hbrBkg;

	CEdit m_EditProcess;

	CLabelEx m_LbMoveXTitle;
	CLabelEx m_LbMoveYTitle;
	CLabelEx m_LbMoveTTitle;
	CLabelEx m_lbCalibPixel;
	CLabelEx m_lbCalibRobt;
	CLabelEx m_lbCalibSub;
	CLabelEx m_lbCameraResolution;

	CLabelEx m_LbSelectCameraTitle;
	CLabelEx m_LbSelectOrgPosTitle;
	CLabelEx m_LbSelectPositionTitle;
	CLabelEx m_LbCameraResolutionXTitle;
	CLabelEx m_LbCameraResolutionYTitle;
	CLabelEx m_LbCameraResolutionX;
	CLabelEx m_LbCameraResolutionY;
	CLabelEx m_LbRotateXTitle;
	CLabelEx m_LbRotateX;
	CLabelEx m_LbRotateYTitle;
	CLabelEx m_LbRotateY;

	CLabelEx m_lbChessBoardTitle;
	CLabelEx m_lbChessSizeTitle;
	CLabelEx m_lbChessXTitle;
	CLabelEx m_lbChessYTitle;
	CEditEx m_LbChessBoardSize;
	CEditEx m_LbChessBoardX;
	CEditEx m_LbChessBoardY;
	CEditEx m_EditMoveX;
	CEditEx m_EditMoveY;
	CEditEx m_EditMoveT;
	CCheckEx m_btnEnableManualPreAlign;
	CCheckEx m_btnEnableIgnoreAable;

	CRITICAL_SECTION m_csProcessHistory;

	CButtonEx m_btnMove;
	CButtonEx m_btnCalibration;
	CButtonEx m_btnIndividualCalibration;
	CButtonEx m_btnIndividualRotateCenter;
	CButtonEx m_btnImageCalibration;
	CButtonEx m_btnRotateCenter;
	CButtonEx m_btnPrealignTest;
	CButtonEx m_btnMachineClose;
	CButtonEx m_btnManualRotate;
	CButtonEx m_btnCalcRotate;
	CButtonEx m_btnMoveRotate;
	CButtonEx m_btnOffsetRotate;
	
	//KJH 2021-12-29 Pattern Index 추가
	CLabelEx m_LbPatternIndex;
	CComboBox m_cmbPatternIndex;
	CButtonEx m_btnPtnDelete;
	

	CString m_strProcess;
	
	BOOL m_bMachineControlStart;

	double m_dbMotorX[MAX_CAMERA][NUM_CALIB_POINTS];
	double m_dbMotorY[MAX_CAMERA][NUM_CALIB_POINTS];
	double m_dbMotorT[MAX_CAMERA][NUM_CALIB_POINTS];

	int m_nJobID;
	//int m_nTabAlgoIndex;
	int m_nTimeOutCount;
	int m_nRotateCnt;

	int m_nSeqManualMove;
	int m_nSeqPrealignTest;
	int m_nSeqRotateCenter;
	int m_nSeqCalibration;
	int m_nSeqImageCalibration;

	int m_nCntX;
	int m_nCntY;

	int m_nAlgorithmCamCount;	
		
	std::vector<int> m_nAlgorithmCamBuff;

	double m_dRadius;
	cv::Point2d m_ptRotCenterRead;

	std::vector<cv::Point2d> m_listPtOnCircle[4] ;  // 원주상의 점들의 집합 -> 이를 활용하여 회전 중심을 찾는다. 3points 이상
	std::vector<cv::Point2d> m_listVisionPtOnCircle;  // 원주상의 점들의 집합 -> 이를 활용하여 회전 중심을 찾는다. 3points 이상

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TAB_PANE_MACHINE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();	

public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnViewControl(WPARAM wParam, LPARAM lParam);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor); 
	afx_msg void OnEnSetfocusEditMoveX();
	afx_msg void OnEnSetfocusEditMoveY();
	afx_msg void OnEnSetfocusEditMoveT();
	afx_msg void OnBnClickedBtnAutoCalibration();
	afx_msg void OnBnClickedBtnRotateCenter();
	afx_msg void OnBnClickedBtnPrealignTest();
	afx_msg void OnBnClickedBtnMove();
	afx_msg void OnBnClickedBtnIndividualCalibration();
	afx_msg void OnBnClickedBtnIndividualRotateCenter();
	afx_msg void OnCbnSelchangeCbSelectMachinePosition();
	afx_msg void OnCbnSelchangeCbSelectMachineCamera(); 
	afx_msg void OnCbnSelchangeCbSelectMachineOrgPosition();
	afx_msg void OnBnClickedBtnImageCalibration();
	afx_msg void OnEnSetfocusEditSquareSize();
	afx_msg void OnEnSetfocusEditXCount();
	afx_msg void OnEnSetfocusEditYCount();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedBtnPatternDelete();
	afx_msg void OnBnClickedBtnOffsetRotate();

public:
	void InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color);
	void MainButtonInit(CButtonEx *pbutton, int size = 15);
	void EditButtonInit(CEditEx *pbutton, int size, COLORREF color = RGB(255, 0, 0));
	void updateFrameDialog();
	void updateResultDialog();
	void addProcessHistory(CString str);
	void enableMachineButtons(bool bEnable);
	void cameraChangeTriggerMode(bool bmode = true);
	void setEditBox(int nID);
	int getCurrentMachineCamera();
	int getCurrentMachinePosition();
	int getCurrentMachinePatternIndex();
	
	void seqCalcCurrentAngle();
	int seqPrealignTest();
	int seqAutoCalibration2();
	int seqAutoCalibration4();

	int seq_manual_move();
	int seqCalculateRotateCenter();
	int seqCalculateRotateCenter2();
	int seqCalculateRotateCenter3();
	
	int seqImageCalibration();

	BOOL IsClickMoveTitle(CEditEx *Wnd, CPoint pt);
	BOOL IsClickWndTitle(CWnd* Wnd, CPoint pt);

	void dispCameraResolution();
	void dispRotateX();
	void dispRotateY();			
	void NotUseVisible(BOOL Display);

	afx_msg void OnBnClickedBtnManualRotate();
	afx_msg void OnBnClickedBtnCalcRotate();
	afx_msg void OnBnClickedBtnMoveRotate();

	CCramerSRuleDlg *m_pCramerDlg;

	double GetStdev(std::vector<double> array);
	void CalcCalibrationResult();
	void CheckReasonabilityRotate(int rotCount);

	void save_resolution_to_viewer(int job, int cam, int pos);
	void save_resolution_to_ini(int viewer, int job, int cam, int pos);

	BOOL find_pattern_caliper(BYTE* pImg, int w, int h, int job, int cam, int pos, CFindInfo* pInfo = NULL);
    void UseClibrationMarkMatching(BYTE* pImage, int nJob,int nCam, int nPos, int m_nWidth, int m_nHeight); // 20211204 Tkyuha 켈리브레이션 등록 마크를 이용하여 매칭 수행

	//KJH 2021-12-29 Pattern Index 추가
	void dispPatternIndex();
	BOOL m_bInitMarkRead;
	afx_msg void OnEnSetfocusEditCameraResolution();
	CEdit m_EditCalibPixel;
	CEdit m_EditCalibRobot;
	CEdit m_EditCalibSub;
	CEdit m_EditCameraResolution;	
};
