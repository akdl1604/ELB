#pragma once

#include "EditEx.h"
#include "CheckEx.h"
#include "GroupEx.h"
// TabModelAlgoPage 대화 상자

class CGridInspectionParamDlg;
class TabModelAlgoPage : public CDialogEx
{
	DECLARE_DYNAMIC(TabModelAlgoPage)

public:
	TabModelAlgoPage(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~TabModelAlgoPage();

	CLET_AlignClientDlg *m_pMain;

	CLabelEx m_stt_AlignSpec;
	CLabelEx m_stt_AlignSpecMinMax;
	CLabelEx m_lblAlignType;
	CLabelEx m_lblAngleCalcMethod;

	CLabelEx m_lblAlignDir;
	CLabelEx m_stt_MarkFindMethod;
	CLabelEx m_stt_RevisionOffset[3];
	CLabelEx m_stt_AlignLimitX;
	CLabelEx m_stt_AlignLimitY;
	CLabelEx m_stt_AlignLimitT;
	CLabelEx m_stt_AlignSpecOutJudge;
	CLabelEx m_stt_EnableLCheck;
	CLabelEx m_stt_LCheckTolerance;
	CLabelEx m_stt_LCheckReference;
	CLabelEx m_stt_LCheckSpecX;
	CLabelEx m_stt_LCheckSpecY;
	CLabelEx m_stt_LCheckScale;
	CLabelEx m_stt_UseIncludedAngle;
	CLabelEx m_stt_IncludedAngle;
	CLabelEx m_stt_IncludedAngleLimit;
	CLabelEx m_stt_CrackInsp;
	CLabelEx m_stt_UseDummyCornerIncludedAngle;
	CLabelEx m_stt_InputInspection;
	CLabelEx m_stt_ExistPanelGray;
	CLabelEx m_stt_ExistDummyGray;
	CLabelEx m_stt_DistanceInspLSL;
	CLabelEx m_stt_DistanceInspSubSpec;
	CLabelEx m_stt_DistanceInspUSL;
	CLabelEx m_stt_DistanceInspSubSpecTolerance;
	CLabelEx m_stt_InspectionMethod;
	CLabelEx m_stt_InspectionUseSubMethod;
	CLabelEx m_stt_UseFixture;
	CLabelEx m_stt_UseReference;
	CLabelEx m_stt_GrabDelay;
	CLabelEx m_stt_EnableRevisionReverseX;
	CLabelEx m_stt_EnableRevisionReverseY;
	CLabelEx m_stt_EnableRevisionReverseT;
	CLabelEx m_stt_EnableRevisionReverseXY;
	CLabelEx m_stt_ELB_InspRange;
	CLabelEx m_stt_ELB_Distance;
	CLabelEx m_stt_ELB_DustInsp;
	CLabelEx m_stt_ELB_DiffInsp;
	CLabelEx m_stt_ELB_RealTimeTrace;
	CLabelEx m_stt_ELB_ReverseOrder;
	CLabelEx m_stt_ELB_AutoSaveAVI;
	CLabelEx m_stt_ELB_CycleTrace;
	CLabelEx m_stt_ELB_MetalTrace;
	CLabelEx m_stt_ELB_TraceInterval;
	CLabelEx m_stt_ELB_EdgePolarity;
	CLabelEx m_stt_ELB_SearchDir;
	CLabelEx m_stt_ELB_Low_thresh;
	CLabelEx m_stt_ELB_Hi_thresh;
	CLabelEx m_stt_ELB_Result_Count;
	CLabelEx m_stt_ELB_HoleSize_C1;
	CLabelEx m_stt_ELB_HoleSize_C2;
	CLabelEx m_stt_ELB_HoleSize_C3;
	CLabelEx m_stt_ELB_Dust_Thresh;
	CLabelEx m_stt_ELB_SubInspMinSize;
	CLabelEx m_stt_DistanceInspHoleMetalSpec;
	CLabelEx m_staticMtoNSelectAlgorithm;
	CLabelEx m_staticMetalTraceFirst;
	CLabelEx m_stt_ELB_BM_BASE;
	CEditEx m_edt_AlignSpec;
	CEditEx m_edt_AlignSpecMinMax;
	CEditEx m_edt_RevisionOffset[3];
	CEditEx m_edt_AlignLimitX;
	CEditEx m_edt_AlignLimitY;
	CEditEx m_edt_AlignLimitT;
	CEditEx m_edt_LCheckTor;
	CEditEx m_edt_LCheckSpecX;
	CEditEx m_edt_GrabDelay;
	CEditEx m_edt_LCheckSpecY;
	CEditEx m_edt_LCheckScale;
	CEditEx m_edt_IncludedAngle;
	CEditEx m_edt_IncludedAngleLimit;
	CEditEx m_edt_Insp_Hi_Thresh;
	CEditEx m_edt_Insp_Low_Thresh;

	//210110
	CEditEx m_edt_ExistPanelGray;
	CEditEx m_edt_ExistDummyGray;
	CEditEx m_edt_InspRangeX;
	CEditEx m_edt_InspRangeY;
	CEditEx m_edt_InspRangeLength;
	CEditEx m_edt_InspTraceInterval;
	CEditEx m_edt_DistanceInspSpecLSL;
	CEditEx m_edt_DistanceInspSubSpec;
	CEditEx m_edt_DistanceInspSpecUSL;
	CEditEx m_edt_DistanceInspSubSpecTolerance;
	CEditEx m_edt_InspHoleSizeC1;
	CEditEx m_edt_InspHoleSizeC2;
	CEditEx m_edt_InspHoleSizeC3;
	CEditEx m_edt_InspDustThresh;
	CEditEx m_edt_InspMinSize;
	CEditEx m_edt_InspResultCount;
	CEditEx m_edt_DistanceInspHoleMetal;
	//210110

	CComboBox m_cmbAlignDir;
	CComboBox m_cmbMarkFindMethod;
	CComboBox m_selAlignType;
	CComboBox m_cmbAngleCalcMethod;
	CComboBox m_selInspectionMethod;
	CComboBox m_cmbLCheckRef;
	CComboBox m_cmbInspSpec;
	CComboBox m_cmbInspSubSpec;
	CComboBox m_cmbSearchEdgePolarity;
	CComboBox m_cmbSearchDir;
	CComboBox m_cmbManualMarkCount;

	CCheckEx m_btnEnableAlignMeasureSpecOutJudge;
	CCheckEx m_btnEnableLCheck;
	CCheckEx m_btnUseCrackInsp;
	CCheckEx m_btnUseDummyCornerInsp;
	CCheckEx m_btnUseIncludedAngle;
	CCheckEx m_btnUseSubMethod;
	CCheckEx m_btnUseFixtureMark; //210110
	CCheckEx m_btnUseReferenceMark; // hsj 2022-01-31 reference 사용유무
	CCheckEx m_btnEnableRevisionReverseX; //210110
	CCheckEx m_btnEnableRevisionReverseY; //210110
	CCheckEx m_btnEnableRevisionReverseT; //210110
	CCheckEx m_btnEnableRevisionReverseXY; //210110
	CCheckEx m_btnEnableDustCheck;
	CCheckEx m_btnEnableDiffCheck;
	CCheckEx m_btnEnableRealTimeTrace;
	CCheckEx m_btnEnableReverseOrder;
	CCheckEx m_btnEnableAutoSaveAVI;
	CCheckEx m_btnEnableCycleTrace;
	CCheckEx m_btnEnableUseSubInspAlgorithm;
	CCheckEx m_btnEnableMetalTrace;
	CCheckEx m_btnEnableMetalTraceFirst;
	CCheckEx m_btnEnableBMBase;
	CCheckEx m_btnEnableInputInspection;
	CGroupEx m_grpAlignType;
	CGroupEx m_grpRevision;
	CGroupEx m_grpAlignFunction;
	CGroupEx m_grpLCheck;
	CGroupEx m_grpExistFunction;
	CGroupEx m_grpElbInspFunction;
	CGroupEx m_grpDistanceFunction;
	CGroupEx m_grpDistanceSubFunction;
	HBRUSH m_hbrBkg;	

	// dh.jung 2021-08-02 add
	CGroupEx m_grpInspSpecPara;
	CGroupEx m_grpInspSpec;
	CGroupEx m_grpInspPara;

	//KJH 2021-11-12 Trace Parameter 작업 시작
	CGroupEx m_grpELBTracePara;
	CGroupEx m_grpTraceImagePara;

	//KJH 2021-12-01 Dust Insp Image Para 추가
	CGroupEx m_grpDiffImagePara;
	CLabelEx m_stt_DiffInspParaItem1;
	CLabelEx m_stt_DiffInspParaItem2;
	CEditEx m_edt_DiffInspParaItem1;
	CEditEx m_edt_DiffInspParaItem2;
	CLabelEx m_stt_DiffInspROIShape;
	CCheckEx m_btnDiffInspROIShape;

	CLabelEx m_stt_TraceImagePara;
	CEditEx m_edt_TraceImagePara;
	CLabelEx m_stt_FindEdgePara;
	CEditEx m_edt_FindEdgePara;

	CComboBox m_cmbInspScanPos;
	CComboBox m_cmbInspItemList;
	CComboBox m_cmbELBTracePara;

	CLabelEx m_stt_InspJudgeSpec;
	CEditEx m_edt_InspJudgeSpec;
	CLabelEx m_stt_InspJudgeSpecTorr;
	CEditEx m_edt_InspJudgeSpecTorr;
	CLabelEx m_stt_InspParaItem1;
	CEditEx m_edt_InspParaItem1;
	CLabelEx m_stt_InspParaItem2;
	CEditEx m_edt_InspParaItem2;
	CButtonEx m_btn_InspspectionParaSet;

	CRect m_rtLCheckUse;
	// end

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TAB_MODEL_ALGORITHM };
#endif
	//int m_nTabAlgoIndex;
	int m_nJobID;
	int m_nModelSelCam;
	int m_nModelSelPos;

	// dh.jung 2021-08-02 add 검사 스캔 위치 1,2,3
	int m_nInspPosition;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnViewControl(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEnSetfocusEditAlignSpec();
	afx_msg void OnEnSetfocusEditAlignSpecMinMax();
	afx_msg void OnEnSetfocusEditRevisionOffset0();
	afx_msg void OnEnSetfocusEditRevisionOffset1();
	afx_msg void OnEnSetfocusEditRevisionOffset2();
	afx_msg void OnEnSetfocusEditAlignLimitX();
	afx_msg void OnEnSetfocusEditAlignLimitY();
	afx_msg void OnEnSetfocusEditAlignLimitT();
	afx_msg void OnEnSetfocusEditLCheckTor();
	afx_msg void OnEnSetfocusEditLCheckSpecX();
	afx_msg void OnEnSetfocusEditLCheckSpecY();
	afx_msg void OnEnSetfocusEditLCheckScale();
	afx_msg void OnEnSetfocusEditGrabDelay();
	afx_msg void OnEnSetfocusEditDummyCornerIncludedAngle();
	afx_msg void OnEnSetfocusEditDummyCornerIncludedAngleLimit();
	afx_msg void OnEnSetfocusEditIncludedAngle();
	afx_msg void OnEnSetfocusEditIncludedAngleLimit();
	afx_msg void OnEnSetfocusEditProjectionAreaLeft0();
	afx_msg void OnEnSetfocusEditProjectionAreaLeft1();
	afx_msg void OnEnSetfocusEditSearchAreaLeft0();
	afx_msg void OnEnSetfocusEditSearchAreaLeft1();
	afx_msg void OnEnSetfocusEditRemoveNoiseSize0();
	afx_msg void OnEnSetfocusEditRemoveNoiseSize1();
	afx_msg void OnEnSetfocusEditSobelThreshold0();
	afx_msg void OnEnSetfocusEditSobelThreshold1();
	afx_msg void OnEnSetfocusEditExistPanelGray();
	afx_msg void OnEnSetfocusEditExistDummyGray();
	afx_msg void OnEnSetfocusEditInspRangeX();
	afx_msg void OnEnSetfocusEditInspRangeY();
	afx_msg void OnEnSetfocusEditInspLength();
	afx_msg void OnEnSetfocusEditInspTraceInterval();
	afx_msg void OnEnSetfocusEditDistanceInspSpecLSL();
	afx_msg void OnEnSetfocusEditDistanceInspSubSpec();
	afx_msg void OnEnSetfocusEditDistanceInspSpecY();
	afx_msg void OnEnSetfocusEditDistanceInspSpecUSL();
	afx_msg void OnEnSetfocusEditDistanceInspSubSpecTolerance();
	afx_msg void OnEnSetfocusEditDistanceInspSpecYminmax();
	afx_msg void OnCbnSelchangeCbModelSelectCam();
	afx_msg void OnCbnSelchangeCbModelSelectPos();
	afx_msg void OnEnSetHiThreshEdit();
	afx_msg void OnEnSetLowThreshEdit();
	afx_msg void OnEnSetfocusEditInspResultCount();
	afx_msg void OnEnSetfocusEditInspHoleC1();
	afx_msg void OnEnSetfocusEditInspHoleC2();
	afx_msg void OnEnSetfocusEditInspHoleC3();
	afx_msg void OnEnSetfocusEditInspDust();
	afx_msg void OnEnSetfocusEditInspMinSize();
	afx_msg void OnEnSetfocusEditDistanceInspHoleMetal();

	void MainButtonInit(CButtonEx *pbutton, int txtSize=12 , int size = 15);
	void InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color);
	void EditButtonInit(CEditEx *pbutton, int size, COLORREF color = RGB(255, 0, 0));
	void EditGroupInit(CGroupEx *pGroup, CString str, float size, COLORREF color);
	void enableEditControls(BOOL bEnable);
	void showModelInfo(CString strModel);
	void showMachineInfo(CString strModel);
	void showAlignInfo(CString strModel);
	void showInspSpecInfo(CString strModel);
	void setEditBox(int nID);
	void updateAlignInfoData();	
	void applyModelData();
	void NotUseVisible(BOOL bDisplay);
	void getSaveParam();
	void getUIValue(); //lhj add 220503
	afx_msg void OnCbnSelchangeComboInspSpec();
	afx_msg void OnCbnSelchangeComboInspSubSpec();
	afx_msg void OnCbnSelchangeComboInspJobPos();
	afx_msg void OnCbnSelchangeComboInspItemList();
	afx_msg void OnEnSetfocusEditInspJudgeSpec();
	afx_msg void OnEnSetfocusEditInspJudgeSpecTorr();
	afx_msg void OnEnSetfocusEditInspParaItem1();
	afx_msg void OnEnSetfocusEditInspParaItem2();
	afx_msg void OnEnSetfocusEditTraceImageThresh();
	afx_msg void OnEnSetfocusEditFindEdgeThresh();
	afx_msg void OnCbnSelchangeComboELBTraceParameter();
	afx_msg void OnEnSetfocusEditDiffInspParaItem1();
	afx_msg void OnEnSetfocusEditDiffInspParaItem2();
	afx_msg void OnCbnSelchangeComboInspMethod();
	afx_msg void OnBnClickedBtnInspSetParam();

	// kbj 2021-12-29 모델 파라미터 JOB 마다 필요 항목만 보이도록.
	void show_enable_parameter();
	void show_parameter_align_type(int nShow);
	void show_parameter_revision(int nShow);
	void show_parameter_align_function(int nShow);
	void show_parameter_length_check(int nShow);
	void show_parameter_pre_insp(int nShow);
	void show_parameter_distance_function(int nShow);
	void show_parameter_distance_function_sub(int nShow);
	void show_parameter_ELB(int nShow);

	CGridInspectionParamDlg* m_pInspectionDlg;
	//KJH 2022-04-23 LCheck Scale 기능 추가
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	BOOL IsDoubleClickLCheckUse(CPoint pt);
	BOOL m_bModify;
	afx_msg void OnBnClickedChkUseCrackInsp();
};
