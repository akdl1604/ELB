#pragma once

#include "CheckEx.h"

// TabCameraPage 대화 상자

class TabCameraPage : public CDialogEx
{
	DECLARE_DYNAMIC(TabCameraPage)

public:
	TabCameraPage(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~TabCameraPage();

	CLET_AlignClientDlg *m_pMain;
	HBRUSH m_hbrBkg;

	CLabelEx m_LbSelectPosition;	
	CLabelEx m_LbSelectCamera;
	CLabelEx m_LbFocusEnergyTitle;
	CLabelEx m_LbFocusEnergy;
	CLabelEx m_LbPatternIndex;
	CLabelEx m_LbMatchingRate;
	CLabelEx m_lblLightSetting;
	CLabelEx m_lblLightChannel;
	CLabelEx m_lblLightIndex;
	CLabelEx m_lblRefPosX;
	CLabelEx m_lblRefPosY;
	CLabelEx m_lblRefPosT;
	CLabelEx m_lblRefPosXTitle;
	CLabelEx m_lblRefPosYTitle;
	CLabelEx m_lblRefPosTTitle;
	CLabelEx m_lblFixturePosX;
	CLabelEx m_lblFixturePosY;
	CLabelEx m_lblFixturePosT;
	CLabelEx m_lblFixturePosXTitle;
	CLabelEx m_lblFixturePosYTitle;
	CLabelEx m_lblFixturePosTTitle;
	CLabelEx m_lblMarkOffsetX;
	CLabelEx m_lblMarkOffsetY;
	CLabelEx m_lblMarkOffsetXTitle;
	CLabelEx m_lblDrawShape;
	CLabelEx m_lblImageProc;
	CLabelEx m_LbRoiX;
	CLabelEx m_LbRoiY;

	CSliderCtrl m_sliderLightSetting;
	CEdit m_edtLightValue;
	CCheckEx m_chkShowProfile;
	CCheckEx m_chkSubInsp;

	CComboBox m_cmbSelectCamera;
	CComboBox m_cmbSelectPosition;
	CComboBox m_cmbPatternIndex;

	CComboBox m_cmbDrawShape;
	CComboBox m_cmbImageProc;
	CComboBox m_cmbLightSelect;
	CComboBox m_cmbLightIndex;

	CButtonEx m_btnPatternRegist;
	CButtonEx m_btnPatternSearch;
	CButtonEx m_btnPatternView;
	CButtonEx m_btnPatternSend;
	CButtonEx m_btnTraceClear;
	CButtonEx m_btnSetRoi;
	CButtonEx m_btnImageSave;
	CButtonEx m_btnImageOpen;
	CButtonEx m_btnCameraLive;
	CButtonEx m_btnCameraStop;
	CButtonEx m_btnSizeOriginal;
	CButtonEx m_btnSizeFit;
	CButtonEx m_btnFocusMeasure;
	CButtonEx m_btnPtnDelete;
	CButtonEx m_btnPatternCopy;
	CButtonEx m_btnDrawArea;
	CButtonEx m_btnSimulation;
	CButtonEx m_btnInspSimulation;
	CButtonEx m_btnUseCaliper;
	CButtonEx m_btnMoveCaliperToCenter;
	CButtonEx m_btnSetSubMark;
	CButtonEx m_btnMakeMasking;
	CButtonEx m_btnShowGoldData;
	CButtonEx m_btnMakeTraceData;
	CButtonEx m_btnInspNozzle;
	CButtonEx m_btnAlignReference;
	CButtonEx m_btnFixturePos;
	CButtonEx m_btnScaleCalc;

	int m_nJobID;
	int sel_last_camera;
	int sel_last_position;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TAB_PANE_CAMERA };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnSizeOriginal();
	afx_msg void OnBnClickedBtnSizeFit();
	afx_msg void OnBnClickedBtnCameraLive();
	afx_msg void OnBnClickedBtnCameraStop();
	afx_msg void OnBnClickedBtnImageSave();
	afx_msg void OnBnClickedBtnImageOpen();
	afx_msg void OnBnClickedBtnShowCaliper();
	afx_msg void OnBnClickedBtnMoveToCenterCaliper();
	afx_msg void OnBnClickedBtnDrawArea();
	afx_msg void OnBnClickedBtnPatternDelete();
	afx_msg void OnBnClickedBtnFocusMeasure();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnCameraSimulation();
	afx_msg void OnBnClickedBtnPatternRegist();
	afx_msg void OnBnClickedBtnPatternView();
	afx_msg void OnBnClickedBtnPatternSearch();
	afx_msg void OnBnClickedBtnSetRoi();
	afx_msg void OnCbnSelchangeCbSelectImageProc2();
	afx_msg void OnCbnSelchangeCbSelectDrawShape();
	afx_msg void OnCbnSelchangeCbSelectPosition();
	afx_msg void OnCbnSelchangeCbPatternIndex();
	afx_msg void OnCbnSelchangeCbSelectLightChannel();
	afx_msg void OnCbnSelchangeCbLightIndex();
	afx_msg void OnEnSetfocusEditMatchingRate();
	afx_msg void OnEnSetfocusEditROIX();
	afx_msg void OnEnSetfocusEditROIY();
	afx_msg void OnEnSetfocusEditLightSetting();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedChkEnableProfile();
	afx_msg void OnBnClickedChkEnableSubInsp();

	void InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color);
	void MainButtonInit(CButtonEx *pbutton, int size = 15);
	void updateFrameDialog();
	void dispViewSize();
	void dispCameraLiveStop();
	void dispMatchingRate();
	void dispDrawArea();
	void dispReferencePos();	
	void dispFixturePos();
	void dispLightSetting();
	void dispFocusMeasure();
	void dispPatternIndex();
	void dispSelectPos();
	void dispCaliperSelect(int bDisplay = 0);
	void dispMarkOffset();
	BOOL getPassword();
	BOOL getHardPassword();

	void init_light_setting();
	afx_msg void OnBnClickedBtnCameraInspection();
	afx_msg void OnBnClickedBtnSetSubMark();
	void disEnableButton();
	afx_msg void OnCbnSelchangeCbSelectCamera();
	afx_msg void OnBnClickedBtnPatternTemp();
	CComboBox m_cbUserSet;
	CEdit m_edtExposureTime;
	CSliderCtrl m_sliderExposureTime;
	CLabelEx m_lblExposureTime;
	afx_msg void OnEnSetfocusEditExposureSetting();
	void dispExposuretime();
	void setCurrentCamLight();
	BOOL IsClickRefPosXTitle(CPoint pt);
	BOOL IsClickRefPosYTitle(CPoint pt);
	BOOL IsClickRefPosTTitle(CPoint pt);
	BOOL IsClickFixturePosXTitle(CPoint pt);
	BOOL IsClickFixturePosYTitle(CPoint pt);
	BOOL IsClickFixturePosTTitle(CPoint pt);
	BOOL IsClickImageProcessingTitle(CPoint pt);
	BOOL IsClickFocusEnergyTitle(CPoint pt);
	BOOL IsClickPatternIndexTitle(CPoint pt);
	void UpdateReferencePosition(double posX, double posY, double posT = -1);
	void UpdateFixturePosition(double posX, double posY, double posT = -1);
	afx_msg void OnBnClickedBtnCameraMake();
	afx_msg void OnBnClickedBtnCameraMakeMetalData();
	afx_msg void OnBnClickedBtnCameraMakeTraceData();
	afx_msg void OnBnClickedBtnPatternClear();
	afx_msg void OnBnClickedBtnNozzleInsp();
	afx_msg void OnBnClickedBtnPatternAlignRef();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	void make_elb_wetout_msa(int job_id, int cam_id, int caliper_id_in_hole, int caliper_id_wetout);
	afx_msg void OnBnClickedBtnFixturePos();
	afx_msg void OnBnClickedBtnScaleCalcPopup();
	BOOL IsDbClickAllAlignRefRegist(CPoint pt);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	BOOL AllAlignReferenceRegist();
	void ReferenceRegistDate(int nPos);
	void FixtureRegistDate(int nPos);
	void CopyMarkImage();

	// YCS 2022-11-28 AutoMode 변경시 Exposure 변경 시퀀스 추가
	BOOL m_bExposureTimeChange[MAX_CAMERA];
	afx_msg void OnBnClickedBtnPatternCopy();
};


