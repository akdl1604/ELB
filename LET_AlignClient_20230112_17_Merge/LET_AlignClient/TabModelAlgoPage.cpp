// TabModelAlgoPage.cpp: 구현 파일
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "TabModelAlgoPage.h"
#include "afxdialogex.h"
#include "GridInspectionParamDlg.h"
// TabModelAlgoPage 대화 상자

IMPLEMENT_DYNAMIC(TabModelAlgoPage, CDialogEx)

TabModelAlgoPage::TabModelAlgoPage(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TAB_MODEL_ALGORITHM, pParent)
{
	m_nJobID = 0;
	m_nModelSelPos = 0;
	m_nModelSelCam = 0;
	m_nInspPosition = 0;
	m_bModify = false;
}

TabModelAlgoPage::~TabModelAlgoPage()
{
	delete m_pInspectionDlg;
}

void TabModelAlgoPage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_STATIC_ALIGN_SPEC, m_stt_AlignSpec);
	DDX_Control(pDX, IDC_STATIC_ALIGN_SPEC_MINMAX, m_stt_AlignSpecMinMax);
	DDX_Control(pDX, IDC_STATIC_ALIGNMENT_TYPE, m_lblAlignType);
	DDX_Control(pDX, IDC_STATIC_ANGLE_CALC_METHOD, m_lblAngleCalcMethod);

	DDX_Control(pDX, IDC_STATIC_ALIGNMENT_DIRECTION, m_lblAlignDir);
	DDX_Control(pDX, IDC_STATIC_MARK_FIND_METHOD, m_stt_MarkFindMethod);
	DDX_Control(pDX, IDC_STATIC_REVISION_OFFSET_X, m_stt_RevisionOffset[0]);
	DDX_Control(pDX, IDC_STATIC_REVISION_OFFSET_Y, m_stt_RevisionOffset[1]);
	DDX_Control(pDX, IDC_STATIC_REVISION_OFFSET_T, m_stt_RevisionOffset[2]);
	DDX_Control(pDX, IDC_STATIC_REVISION_LIMIT_X, m_stt_AlignLimitX);
	DDX_Control(pDX, IDC_STATIC_REVISION_LIMIT_Y, m_stt_AlignLimitY);
	DDX_Control(pDX, IDC_STATIC_REVISION_LIMIT_T, m_stt_AlignLimitT);
	DDX_Control(pDX, IDC_STATIC_ENABLE_ALIGN_MEASURE_SPEC_OUT_NG, m_stt_AlignSpecOutJudge);
	DDX_Control(pDX, IDC_STATIC_ENABLE_L_CHECK, m_stt_EnableLCheck);
	DDX_Control(pDX, IDC_STATIC_L_CHECK_TOLERANCE, m_stt_LCheckTolerance);
	DDX_Control(pDX, IDC_STATIC_L_CHECK_REFERENCE, m_stt_LCheckReference);

	DDX_Control(pDX, IDC_STATIC_L_CHECK_SPEC, m_stt_LCheckSpecX);
	DDX_Control(pDX, IDC_STATIC_L_CHECK_SPEC_Y, m_stt_LCheckSpecY);

	DDX_Control(pDX, IDC_STATIC_LCHECK_SCALE, m_stt_LCheckScale);
	DDX_Control(pDX, IDC_STATIC_USE_INCLUDED_ANGLE, m_stt_UseIncludedAngle);
	DDX_Control(pDX, IDC_STATIC_INCLUDED_ANGLE, m_stt_IncludedAngle);
	DDX_Control(pDX, IDC_STATIC_INCLUDED_ANGLE_LIMIT, m_stt_IncludedAngleLimit);
	DDX_Control(pDX, IDC_STATIC_USE_CRACK_INSP, m_stt_CrackInsp);
	DDX_Control(pDX, IDC_STATIC_USE_DUMMY_CORNER_INSP, m_stt_UseDummyCornerIncludedAngle);
	//210110
	DDX_Control(pDX, IDC_STATIC_USE_FIXTURE_MARK, m_stt_UseFixture);
	DDX_Control(pDX, IDC_STATIC_USE_REFERENCE_MARK, m_stt_UseReference);
	DDX_Control(pDX, IDC_STATIC_USE_GRAB_DELAY, m_stt_GrabDelay);
	DDX_Control(pDX, IDC_STATIC_EXIST_PANEL_GRAY, m_stt_ExistPanelGray);
	DDX_Control(pDX, IDC_STATIC_USE_INPUT_INSPECTION, m_stt_InputInspection);
	DDX_Control(pDX, IDC_STATIC_EXIST_DUMMY_GRAY, m_stt_ExistDummyGray);
	DDX_Control(pDX, IDC_STATIC_ENABLE_REVISION_REVERSE_X, m_stt_EnableRevisionReverseX);
	DDX_Control(pDX, IDC_STATIC_ENABLE_REVISION_REVERSE_Y, m_stt_EnableRevisionReverseY);
	DDX_Control(pDX, IDC_STATIC_ENABLE_REVISION_REVERSE_T, m_stt_EnableRevisionReverseT);
	DDX_Control(pDX, IDC_STATIC_ENABLE_REVISION_REVERSE_XY, m_stt_EnableRevisionReverseXY);

	//Distance Inspection
	DDX_Control(pDX, IDC_STATIC_INSP_METHOD, m_stt_InspectionMethod);

	DDX_Control(pDX, IDC_STATIC_DISTANCE_INSP_LSL, m_stt_DistanceInspLSL);
	DDX_Control(pDX, IDC_STATIC_DISTANCE_INSP_USL, m_stt_DistanceInspUSL);
	DDX_Control(pDX, IDC_STATIC_DISTANCE_INSP_SUB_SPEC, m_stt_DistanceInspSubSpec);
	DDX_Control(pDX, IDC_STATIC_DISTANCE_INSP_SUB_SPEC_TOLERANCE, m_stt_DistanceInspSubSpecTolerance);
	DDX_Control(pDX, IDC_STATIC_DISTANCE_INSP_HOLE_METAL, m_stt_DistanceInspHoleMetalSpec);

	DDX_Control(pDX, IDC_EDIT_ALIGN_SPEC, m_edt_AlignSpec);
	DDX_Control(pDX, IDC_EDIT_ALIGN_SPEC_MINMAX, m_edt_AlignSpecMinMax);
	DDX_Control(pDX, IDC_EDIT_REVISION_OFFSET_X, m_edt_RevisionOffset[0]);
	DDX_Control(pDX, IDC_EDIT_REVISION_OFFSET_Y, m_edt_RevisionOffset[1]);
	DDX_Control(pDX, IDC_EDIT_REVISION_OFFSET_T, m_edt_RevisionOffset[2]);
	DDX_Control(pDX, IDC_EDIT_REVISION_LIMIT_X, m_edt_AlignLimitX);
	DDX_Control(pDX, IDC_EDIT_REVISION_LIMIT_Y, m_edt_AlignLimitY);
	DDX_Control(pDX, IDC_EDIT_REVISION_LIMIT_T, m_edt_AlignLimitT);
	DDX_Control(pDX, IDC_EDIT_L_CHECK_TOLERANCE, m_edt_LCheckTor);
	DDX_Control(pDX, IDC_EDIT_L_CHECK_SPEC, m_edt_LCheckSpecX);
	DDX_Control(pDX, IDC_EDIT_GRAB_DELAY, m_edt_GrabDelay);
	DDX_Control(pDX, IDC_EDIT_L_CHECK_SPEC_Y, m_edt_LCheckSpecY);
	DDX_Control(pDX, IDC_EDIT_ELB_INSP_HI_THRESH, m_edt_Insp_Hi_Thresh);
	DDX_Control(pDX, IDC_EDIT_ELB_INSP_LOW_THRESH, m_edt_Insp_Low_Thresh);

	DDX_Control(pDX, IDC_EDIT_LCHECK_SCALE, m_edt_LCheckScale);
	DDX_Control(pDX, IDC_EDIT_INCLUDED_ANGLE, m_edt_IncludedAngle);
	DDX_Control(pDX, IDC_EDIT_INCLUDED_ANGLE_LIMIT, m_edt_IncludedAngleLimit);

	DDX_Control(pDX, IDC_COMBO_ALIGNMENT_DIR, m_cmbAlignDir);
	DDX_Control(pDX, IDC_CB_MARK_FIND_METHOD, m_cmbMarkFindMethod);
	DDX_Control(pDX, IDC_COMBO_ALIGNMENT_TYPE, m_selAlignType);
	DDX_Control(pDX, IDC_COMBO_ANGLE_CALC_METHOD, m_cmbAngleCalcMethod);

	DDX_Control(pDX, IDC_COMBO_INSP_METHOD, m_selInspectionMethod);

	DDX_Control(pDX, IDC_COMBO_LCHECK_REFERENCE, m_cmbLCheckRef);
	DDX_Control(pDX, IDC_COMBO_INSP_SPEC, m_cmbInspSpec);
	DDX_Control(pDX, IDC_COMBO_INSP_SUB_SPEC, m_cmbInspSubSpec);
	DDX_Control(pDX, IDC_COMBO_EDGE_POLARITY, m_cmbSearchEdgePolarity);
	DDX_Control(pDX, IDC_COMBO_SEARCH_DIR, m_cmbSearchDir);

	DDX_Control(pDX, IDC_CHK_ENABLE_ALIGN_MEASURE_SPEC_OUT_JUDGE, m_btnEnableAlignMeasureSpecOutJudge);
	DDX_Control(pDX, IDC_CHK_ENABLE_L_CHECK, m_btnEnableLCheck);
	DDX_Control(pDX, IDC_CHK_USE_CRACK_INSP, m_btnUseCrackInsp);
	DDX_Control(pDX, IDC_CHK_USE_DUMMY_CORNER_INSP, m_btnUseDummyCornerInsp);
	DDX_Control(pDX, IDC_CHK_USE_INCLUDED_ANGLE, m_btnUseIncludedAngle);
	DDX_Control(pDX, IDC_CHK_USE_DUST_INSP, m_btnEnableDustCheck);
	DDX_Control(pDX, IDC_CHK_USE_POST_INSP, m_btnEnableDiffCheck);
	DDX_Control(pDX, IDC_CHK_USE_REALTIME_TRACE, m_btnEnableRealTimeTrace);
	DDX_Control(pDX, IDC_CHK_USE_REVERSE_ORDER, m_btnEnableReverseOrder);
	DDX_Control(pDX, IDC_CHK_USE_AUTOSAVE_AVI, m_btnEnableAutoSaveAVI);
	DDX_Control(pDX, IDC_CHK_USE_METAL_TRACE, m_btnEnableMetalTrace);
	DDX_Control(pDX, IDC_CHK_USE_METAL_TRACE_FIRST, m_btnEnableMetalTraceFirst);
	DDX_Control(pDX, IDC_CHK_USE_CYCLE_TRACE, m_btnEnableCycleTrace);
	DDX_Control(pDX, IDC_CHK_USE_SUB_INSP_ALGORITHM, m_btnEnableUseSubInspAlgorithm);
	DDX_Control(pDX, IDC_CHK_USE_BM_BASE, m_btnEnableBMBase);
	DDX_Control(pDX, IDC_CHK_USE_INPUT_INSP, m_btnEnableInputInspection);
	DDX_Control(pDX, IDC_GROUP_ALIGN_TYPE, m_grpAlignType);
	DDX_Control(pDX, IDC_GROUP_REVISION, m_grpRevision);
	DDX_Control(pDX, IDC_GRUOP_ALING_FUNCTION, m_grpAlignFunction);
	DDX_Control(pDX, IDC_GRUOP_LCHECK, m_grpLCheck);

	DDX_Control(pDX, IDC_EXIST_FUNCTION, m_grpExistFunction);
	DDX_Control(pDX, IDC_ELB_INSPECTION, m_grpElbInspFunction);
	DDX_Control(pDX, IDC_DISTANCE_FUNCTION, m_grpDistanceFunction);
	DDX_Control(pDX, IDC_DISTANCE_SUB_FUNCTION, m_grpDistanceSubFunction);

	//210110
	DDX_Control(pDX, IDC_CHK_USE_FIXTURE_MARK, m_btnUseFixtureMark);
	DDX_Control(pDX, IDC_CHK_USE_REFERENCE_MARK, m_btnUseReferenceMark); // hsj 2022-01-31 reference 사용유무
	DDX_Control(pDX, IDC_EDIT_EXIST_PANEL_GRAY, m_edt_ExistPanelGray);
	DDX_Control(pDX, IDC_EDIT_EXIST_DUMMY_GRAY, m_edt_ExistDummyGray);

	DDX_Control(pDX, IDC_EDIT_ELB_INSP_RANGE_X, m_edt_InspRangeX);
	DDX_Control(pDX, IDC_EDIT_ELB_INSP_RANGE_Y, m_edt_InspRangeY);
	DDX_Control(pDX, IDC_EDIT_ELB_INSP_LENGTH, m_edt_InspRangeLength);
	DDX_Control(pDX, IDC_EDIT_ELB_TRACE_INTERVAL, m_edt_InspTraceInterval);
	DDX_Control(pDX, IDC_EDIT_ELB_INSP_RESULT_C, m_edt_InspResultCount);
	DDX_Control(pDX, IDC_EDIT_ELB_INSP_HOLE_C1, m_edt_InspHoleSizeC1);
	DDX_Control(pDX, IDC_EDIT_ELB_INSP_HOLE_C2, m_edt_InspHoleSizeC2);
	DDX_Control(pDX, IDC_EDIT_ELB_INSP_HOLE_C3, m_edt_InspHoleSizeC3);
	DDX_Control(pDX, IDC_EDIT_ELB_INSP_DUST, m_edt_InspDustThresh);
	DDX_Control(pDX, IDC_EDIT_ELB_INSP_MINSIZE, m_edt_InspMinSize);
	DDX_Control(pDX, IDC_CHK_ENABLE_REVISION_REVERSE_X, m_btnEnableRevisionReverseX);
	DDX_Control(pDX, IDC_CHK_ENABLE_REVISION_REVERSE_Y, m_btnEnableRevisionReverseY);
	DDX_Control(pDX, IDC_CHK_ENABLE_REVISION_REVERSE_T, m_btnEnableRevisionReverseT);
	DDX_Control(pDX, IDC_CHK_ENABLE_REVISION_REVERSE_XY, m_btnEnableRevisionReverseXY);

	//DistanceInspection
	DDX_Control(pDX, IDC_EDIT_DISTANCE_INSP_LSL, m_edt_DistanceInspSpecLSL);
	DDX_Control(pDX, IDC_EDIT_DISTANCE_INSP_USL, m_edt_DistanceInspSpecUSL);
	DDX_Control(pDX, IDC_EDIT_DISTANCE_INSP_SUB_SPEC, m_edt_DistanceInspSubSpec);
	DDX_Control(pDX, IDC_EDIT_DISTANCE_INSP_SUB_SPEC_TOLERANCE, m_edt_DistanceInspSubSpecTolerance);
	DDX_Control(pDX, IDC_EDIT_DISTANCE_INSP__HOLE_METAL, m_edt_DistanceInspHoleMetal);
	//210110
	DDX_Control(pDX, IDC_STATIC_ELB_INSP_RANGE, m_stt_ELB_InspRange);
	DDX_Control(pDX, IDC_STATIC_ELB_INSP_DISTANCE, m_stt_ELB_Distance);
	DDX_Control(pDX, IDC_STATIC_USE_DUST_INSPECTION, m_stt_ELB_DustInsp);
	DDX_Control(pDX, IDC_STATIC_USE_POST_INSPECTION, m_stt_ELB_DiffInsp);
	DDX_Control(pDX, IDC_STATIC_ELB_TRACE_INTERVAL, m_stt_ELB_TraceInterval);
	DDX_Control(pDX, IDC_STATIC_USE_REALTIME_TRACE, m_stt_ELB_RealTimeTrace);
	DDX_Control(pDX, IDC_STATIC_EDGE_POLARITY, m_stt_ELB_EdgePolarity);
	DDX_Control(pDX, IDC_STATIC_SEARCH_DIR, m_stt_ELB_SearchDir);
	DDX_Control(pDX, IDC_STATIC_USE_REVERSE_ORDER, m_stt_ELB_ReverseOrder);
	DDX_Control(pDX, IDC_STATIC_USE_METAL_TRACE, m_stt_ELB_MetalTrace);
	DDX_Control(pDX, IDC_STATIC_USE_CYCLE_TRACE, m_stt_ELB_CycleTrace);
	DDX_Control(pDX, IDC_STATIC_USE_AUTOSAVE_AVI, m_stt_ELB_AutoSaveAVI);

	DDX_Control(pDX, IDC_STATIC_ELB_INSP_HI_THRESH, m_stt_ELB_Hi_thresh);
	DDX_Control(pDX, IDC_STATIC_ELB_INSP_LOW_TH, m_stt_ELB_Low_thresh);

	DDX_Control(pDX, IDC_STATIC_ELB_INSP_RESULT_C, m_stt_ELB_Result_Count);
	DDX_Control(pDX, IDC_STATIC_ELB_INSP_HOLE_C1, m_stt_ELB_HoleSize_C1);
	DDX_Control(pDX, IDC_STATIC_ELB_INSP_HOLE_C2, m_stt_ELB_HoleSize_C2);
	DDX_Control(pDX, IDC_STATIC_ELB_INSP_HOLE_C3, m_stt_ELB_HoleSize_C3);
	DDX_Control(pDX, IDC_STATIC_ELB_INSP_DUST, m_stt_ELB_Dust_Thresh);
	DDX_Control(pDX, IDC_STATIC_ELB_INSP_MINSIZE, m_stt_ELB_SubInspMinSize);
	DDX_Control(pDX, IDC_STATIC_USE_METAL_TRACE_FIRST, m_staticMetalTraceFirst);
	DDX_Control(pDX, IDC_STATIC_SELECT_MTON_DISTANCE_ALGORITHM, m_staticMtoNSelectAlgorithm);
	DDX_Control(pDX, IDC_STATIC_USE_BM_BASE, m_stt_ELB_BM_BASE);

	//dh.jung 2021-08-02 add Insp Spec, Parameter
	DDX_Control(pDX, IDC_GB_INSPECTION_SPEC_PARAMETER, m_grpInspSpecPara);
	DDX_Control(pDX, IDC_GB_INSPECTION_SPEC, m_grpInspSpec);
	DDX_Control(pDX, IDC_GB_INSPECTION_PARA, m_grpInspPara);

	DDX_Control(pDX, IDC_COMBO_INSP_SCAN_POS, m_cmbInspScanPos);
	DDX_Control(pDX, IDC_COMBO_INSP_ITEM_LIST, m_cmbInspItemList);

	DDX_Control(pDX, IDC_STATIC_INSP_JUDGE_SPEC, m_stt_InspJudgeSpec);
	DDX_Control(pDX, IDC_EDIT_INSP_JUDGE_SPEC, m_edt_InspJudgeSpec);
	DDX_Control(pDX, IDC_STATIC_INSP_JUDGE_SPEC_TORR, m_stt_InspJudgeSpecTorr);
	DDX_Control(pDX, IDC_EDIT_INSP_JUDGE_SPEC_TORR, m_edt_InspJudgeSpecTorr);

	DDX_Control(pDX, IDC_STATIC_INSP_PARA_ITEM1, m_stt_InspParaItem1);
	DDX_Control(pDX, IDC_EDIT_INSP_PARA_ITEM1, m_edt_InspParaItem1);
	DDX_Control(pDX, IDC_STATIC_INSP_PARA_ITEM2, m_stt_InspParaItem2);
	DDX_Control(pDX, IDC_EDIT_INSP_PARA_ITEM2, m_edt_InspParaItem2);

	//KJH 2021-11-12 Trace Parameter 작업 시작
	DDX_Control(pDX, IDC_COMBO_ELB_TRACE_PARAMETER, m_cmbELBTracePara);

	DDX_Control(pDX, IDC_GB_ELB_TRACE_PARAMETER, m_grpELBTracePara);
	DDX_Control(pDX, IDC_GB_TRACE_IMAGE_PARAMETER, m_grpTraceImagePara);
	DDX_Control(pDX, IDC_STATIC_TRACE_IMAGE_THRESH, m_stt_TraceImagePara);
	DDX_Control(pDX, IDC_EDIT_TRACE_IMAGE_THRESH, m_edt_TraceImagePara);
	DDX_Control(pDX, IDC_STATIC_FIND_EDGE_THRESH, m_stt_FindEdgePara);
	DDX_Control(pDX, IDC_EDIT_FIND_EDGE_THRESH, m_edt_FindEdgePara);

	//KJH 2021-12-01 Dust Insp Image Para 추가
	DDX_Control(pDX, IDC_GB_DIFF_INSPECTION_PARA, m_grpDiffImagePara);
	DDX_Control(pDX, IDC_STATIC_DIFF_INSP_PARA_ITEM1, m_stt_DiffInspParaItem1);
	DDX_Control(pDX, IDC_STATIC_DIFF_INSP_PARA_ITEM2, m_stt_DiffInspParaItem2);
	DDX_Control(pDX, IDC_EDIT_DIFF_INSP_PARA_ITEM1, m_edt_DiffInspParaItem1);
	DDX_Control(pDX, IDC_EDIT_DIFF_INSP_PARA_ITEM2, m_edt_DiffInspParaItem2);
	DDX_Control(pDX, IDC_STATIC_DUST_INSP_ROI_SHAPE, m_stt_DiffInspROIShape);
	DDX_Control(pDX, IDC_CHK_USE_DUST_INSP_ROI_SHAPE, m_btnDiffInspROIShape);
	
	DDX_Control(pDX, IDC_BTN_INSP_SET_PARAM, m_btn_InspspectionParaSet);
	DDX_Control(pDX, IDC_COMBO_MANUAL_MARK_COUNT, m_cmbManualMarkCount);
	// end 
}


BEGIN_MESSAGE_MAP(TabModelAlgoPage, CDialogEx)
	ON_MESSAGE(WM_VIEW_CONTROL, OnViewControl)
	ON_WM_CTLCOLOR()
	ON_EN_SETFOCUS(IDC_EDIT_ALIGN_SPEC, &TabModelAlgoPage::OnEnSetfocusEditAlignSpec)
	ON_EN_SETFOCUS(IDC_EDIT_ALIGN_SPEC_MINMAX, &TabModelAlgoPage::OnEnSetfocusEditAlignSpecMinMax)
	ON_EN_SETFOCUS(IDC_EDIT_REVISION_OFFSET_X, &TabModelAlgoPage::OnEnSetfocusEditRevisionOffset0)
	ON_EN_SETFOCUS(IDC_EDIT_REVISION_OFFSET_Y, &TabModelAlgoPage::OnEnSetfocusEditRevisionOffset1)
	ON_EN_SETFOCUS(IDC_EDIT_REVISION_OFFSET_T, &TabModelAlgoPage::OnEnSetfocusEditRevisionOffset2)
	ON_EN_SETFOCUS(IDC_EDIT_REVISION_LIMIT_X, &TabModelAlgoPage::OnEnSetfocusEditAlignLimitX)
	ON_EN_SETFOCUS(IDC_EDIT_REVISION_LIMIT_Y, &TabModelAlgoPage::OnEnSetfocusEditAlignLimitY)
	ON_EN_SETFOCUS(IDC_EDIT_REVISION_LIMIT_T, &TabModelAlgoPage::OnEnSetfocusEditAlignLimitT)
	ON_EN_SETFOCUS(IDC_EDIT_L_CHECK_TOLERANCE, &TabModelAlgoPage::OnEnSetfocusEditLCheckTor)
	ON_EN_SETFOCUS(IDC_EDIT_L_CHECK_SPEC, &TabModelAlgoPage::OnEnSetfocusEditLCheckSpecX)
	ON_EN_SETFOCUS(IDC_EDIT_L_CHECK_SPEC_Y, &TabModelAlgoPage::OnEnSetfocusEditLCheckSpecY)
	ON_EN_SETFOCUS(IDC_EDIT_LCHECK_SCALE, &TabModelAlgoPage::OnEnSetfocusEditLCheckScale)
	ON_EN_SETFOCUS(IDC_EDIT_GRAB_DELAY, &TabModelAlgoPage::OnEnSetfocusEditGrabDelay)
	ON_EN_SETFOCUS(IDC_EDIT_DUMMY_INCLUDED_ANGLE, &TabModelAlgoPage::OnEnSetfocusEditDummyCornerIncludedAngle)
	ON_EN_SETFOCUS(IDC_EDIT_DUMMY_INCLUDED_ANGLE_LIMIT, &TabModelAlgoPage::OnEnSetfocusEditDummyCornerIncludedAngleLimit)
	ON_EN_SETFOCUS(IDC_EDIT_INCLUDED_ANGLE, &TabModelAlgoPage::OnEnSetfocusEditIncludedAngle)
	ON_EN_SETFOCUS(IDC_EDIT_INCLUDED_ANGLE_LIMIT, &TabModelAlgoPage::OnEnSetfocusEditIncludedAngleLimit)
	ON_EN_SETFOCUS(IDC_EDIT_PROJECTION_AREA_LEFT, &TabModelAlgoPage::OnEnSetfocusEditProjectionAreaLeft0)
	ON_EN_SETFOCUS(IDC_EDIT_PROJECTION_AREA_LEFT2, &TabModelAlgoPage::OnEnSetfocusEditProjectionAreaLeft1)
	ON_EN_SETFOCUS(IDC_EDIT_SEARCH_AREA_LEFT1, &TabModelAlgoPage::OnEnSetfocusEditSearchAreaLeft0)
	ON_EN_SETFOCUS(IDC_EDIT_SEARCH_AREA_LEFT2, &TabModelAlgoPage::OnEnSetfocusEditSearchAreaLeft1)
	ON_EN_SETFOCUS(IDC_EDIT_REMOVE_NOISE_SIZE1, &TabModelAlgoPage::OnEnSetfocusEditRemoveNoiseSize0)
	ON_EN_SETFOCUS(IDC_EDIT_REMOVE_NOISE_SIZE2, &TabModelAlgoPage::OnEnSetfocusEditRemoveNoiseSize1)
	ON_EN_SETFOCUS(IDC_EDIT_SOBEL_THRESHOLD1, &TabModelAlgoPage::OnEnSetfocusEditSobelThreshold0)
	ON_EN_SETFOCUS(IDC_EDIT_SOBEL_THRESHOLD2, &TabModelAlgoPage::OnEnSetfocusEditSobelThreshold1)
	ON_EN_SETFOCUS(IDC_EDIT_EXIST_PANEL_GRAY, &TabModelAlgoPage::OnEnSetfocusEditExistPanelGray)
	ON_EN_SETFOCUS(IDC_EDIT_EXIST_DUMMY_GRAY, &TabModelAlgoPage::OnEnSetfocusEditExistDummyGray)
	ON_EN_SETFOCUS(IDC_EDIT_ELB_INSP_RANGE_X, &TabModelAlgoPage::OnEnSetfocusEditInspRangeX)
	ON_EN_SETFOCUS(IDC_EDIT_ELB_INSP_RANGE_Y, &TabModelAlgoPage::OnEnSetfocusEditInspRangeY)
	ON_EN_SETFOCUS(IDC_EDIT_ELB_INSP_LENGTH, &TabModelAlgoPage::OnEnSetfocusEditInspLength)
	ON_EN_SETFOCUS(IDC_EDIT_ELB_TRACE_INTERVAL, &TabModelAlgoPage::OnEnSetfocusEditInspTraceInterval)
	ON_EN_SETFOCUS(IDC_EDIT_DISTANCE_INSP_LSL, &TabModelAlgoPage::OnEnSetfocusEditDistanceInspSpecLSL)
	ON_EN_SETFOCUS(IDC_EDIT_DISTANCE_INSP_SUB_SPEC, &TabModelAlgoPage::OnEnSetfocusEditDistanceInspSubSpec)
	ON_EN_SETFOCUS(IDC_EDIT_DISTANCE_INSP_SPEC_Y, &TabModelAlgoPage::OnEnSetfocusEditDistanceInspSpecY)
	ON_EN_SETFOCUS(IDC_EDIT_DISTANCE_INSP_USL, &TabModelAlgoPage::OnEnSetfocusEditDistanceInspSpecUSL)
	ON_EN_SETFOCUS(IDC_EDIT_DISTANCE_INSP_SUB_SPEC_TOLERANCE, &TabModelAlgoPage::OnEnSetfocusEditDistanceInspSubSpecTolerance)
	ON_EN_SETFOCUS(IDC_EDIT_DISTANCE_INSP_SPEC_Y_MINMAX, &TabModelAlgoPage::OnEnSetfocusEditDistanceInspSpecYminmax)
	ON_EN_SETFOCUS(IDC_EDIT_ELB_INSP_HI_THRESH, &TabModelAlgoPage::OnEnSetHiThreshEdit)
    ON_EN_SETFOCUS(IDC_EDIT_ELB_INSP_LOW_THRESH, &TabModelAlgoPage::OnEnSetLowThreshEdit)
	ON_CBN_SELCHANGE(IDC_CB_MODEL_SELECT_CAM, &TabModelAlgoPage::OnCbnSelchangeCbModelSelectCam)
	ON_CBN_SELCHANGE(IDC_CB_MODEL_SELECT_POS, &TabModelAlgoPage::OnCbnSelchangeCbModelSelectPos)
	ON_CBN_SELCHANGE(IDC_COMBO_INSP_SPEC, &TabModelAlgoPage::OnCbnSelchangeComboInspSpec)
	ON_CBN_SELCHANGE(IDC_COMBO_INSP_SUB_SPEC, &TabModelAlgoPage::OnCbnSelchangeComboInspSubSpec)
    ON_EN_SETFOCUS(IDC_EDIT_ELB_INSP_RESULT_C, &TabModelAlgoPage::OnEnSetfocusEditInspResultCount)
	ON_EN_SETFOCUS(IDC_EDIT_ELB_INSP_HOLE_C1, &TabModelAlgoPage::OnEnSetfocusEditInspHoleC1)
	ON_EN_SETFOCUS(IDC_EDIT_ELB_INSP_HOLE_C2, &TabModelAlgoPage::OnEnSetfocusEditInspHoleC2)
	ON_EN_SETFOCUS(IDC_EDIT_ELB_INSP_HOLE_C3, &TabModelAlgoPage::OnEnSetfocusEditInspHoleC3)
	ON_EN_SETFOCUS(IDC_EDIT_ELB_INSP_DUST, &TabModelAlgoPage::OnEnSetfocusEditInspDust)
	ON_EN_SETFOCUS(IDC_EDIT_ELB_INSP_MINSIZE, &TabModelAlgoPage::OnEnSetfocusEditInspMinSize)
	ON_EN_SETFOCUS(IDC_EDIT_DISTANCE_INSP__HOLE_METAL, &TabModelAlgoPage::OnEnSetfocusEditDistanceInspHoleMetal)
	ON_CBN_SELCHANGE(IDC_COMBO_INSP_SCAN_POS, &TabModelAlgoPage::OnCbnSelchangeComboInspJobPos)
	ON_CBN_SELCHANGE(IDC_COMBO_INSP_ITEM_LIST, &TabModelAlgoPage::OnCbnSelchangeComboInspItemList)
	ON_EN_SETFOCUS(IDC_EDIT_INSP_JUDGE_SPEC, &TabModelAlgoPage::OnEnSetfocusEditInspJudgeSpec)
	ON_EN_SETFOCUS(IDC_EDIT_INSP_JUDGE_SPEC_TORR, &TabModelAlgoPage::OnEnSetfocusEditInspJudgeSpecTorr)
	ON_EN_SETFOCUS(IDC_EDIT_INSP_PARA_ITEM1, &TabModelAlgoPage::OnEnSetfocusEditInspParaItem1)
	ON_EN_SETFOCUS(IDC_EDIT_INSP_PARA_ITEM2, &TabModelAlgoPage::OnEnSetfocusEditInspParaItem2)
	ON_EN_SETFOCUS(IDC_EDIT_TRACE_IMAGE_THRESH, &TabModelAlgoPage::OnEnSetfocusEditTraceImageThresh)
	ON_EN_SETFOCUS(IDC_EDIT_FIND_EDGE_THRESH, &TabModelAlgoPage::OnEnSetfocusEditFindEdgeThresh)
	ON_EN_SETFOCUS(IDC_EDIT_DIFF_INSP_PARA_ITEM1, &TabModelAlgoPage::OnEnSetfocusEditDiffInspParaItem1)
	ON_EN_SETFOCUS(IDC_EDIT_DIFF_INSP_PARA_ITEM2, &TabModelAlgoPage::OnEnSetfocusEditDiffInspParaItem2)
	ON_CBN_SELCHANGE(IDC_COMBO_ELB_TRACE_PARAMETER, &TabModelAlgoPage::OnCbnSelchangeComboELBTraceParameter)
	ON_CBN_SELCHANGE(IDC_COMBO_INSP_METHOD, &TabModelAlgoPage::OnCbnSelchangeComboInspMethod)
	ON_BN_CLICKED(IDC_BTN_INSP_SET_PARAM, &TabModelAlgoPage::OnBnClickedBtnInspSetParam)
	ON_WM_LBUTTONDBLCLK()
	ON_BN_CLICKED(IDC_CHK_USE_CRACK_INSP, &TabModelAlgoPage::OnBnClickedChkUseCrackInsp)
END_MESSAGE_MAP()


// TabModelAlgoPage 메시지 처리기

void TabModelAlgoPage::InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}
void TabModelAlgoPage::MainButtonInit(CButtonEx *pbutton, int txtSize, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeText(txtSize);
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}
void TabModelAlgoPage::EditButtonInit(CEditEx *pbutton, int size, COLORREF color)
{
	pbutton->SetEnable(false);
	pbutton->SetSizeText(size);				// 글자 크기
	pbutton->SetStyleTextBold(true);		// 글자 스타일
	pbutton->SetTextMargins(10, 10);		// 글자 옵셋
	pbutton->SetColorText(color);	// 글자 색상
	pbutton->SetText(_T("0"));				// 글자 설정
}
void TabModelAlgoPage::EditGroupInit(CGroupEx *pGroup, CString str, float size, COLORREF color)
{
	pGroup->SetSizeText(size);
	pGroup->SetColorBkg(255, 96, 96, 96);
	pGroup->SetColorBorder(255, 96, 96, 96);
	pGroup->SetText(str);
	pGroup->SetColorText(255, 255, 255, 255);
	pGroup->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}
BOOL TabModelAlgoPage::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);
	m_pMain = (CLET_AlignClientDlg*)AfxGetMainWnd();

	InitTitle(&m_stt_AlignSpec, "Align Spec (mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_AlignSpecMinMax, "Spec MinMax (mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_AlignSpecOutJudge, "Manual Mark Input", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_EnableLCheck, "Enable L Check", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_LCheckTolerance, "L Check Tolerance (mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_LCheckReference, "L Check Reference", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_LCheckSpecX, "L Check Spec X (mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_LCheckSpecY, "L Check Spec Y (mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_LCheckScale, "L Check Scale", 14.f, COLOR_BTN_BODY);
	//InitTitle(&m_stt_LCheckScale, "L Check Length(mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_CrackInsp, "Use Sub Inspection", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_MarkFindMethod, "Mark Find Method", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_lblAlignType, "Alignment Type", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_lblAngleCalcMethod, "Angle Calc Method", 14.f, COLOR_BTN_BODY);
	
	InitTitle(&m_stt_AlignLimitX, "Revision Limit X (mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_AlignLimitY, "Revision Limit Y (mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_AlignLimitT, "Revision Limit T (˚)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_RevisionOffset[0], "Revision Offset X (mm) ", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_RevisionOffset[1], "Revision Offset Y (mm) ", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_RevisionOffset[2], "Revision Offset T (mm) ", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_UseIncludedAngle, "Use Included Angle", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_IncludedAngle, "Included Angle (˚)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_IncludedAngleLimit, "Angle Tolerance (˚)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_UseDummyCornerIncludedAngle, "Use Dummy Corner Inspection", 14.f, COLOR_BTN_BODY);

	InitTitle(&m_lblAlignDir, "Alignment Direction", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_InputInspection, "Input Inspection", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ExistPanelGray, "Line Dopo Filter", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_InspRange, "Inspection Range(X,Y)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_Distance, "Inspection Length", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_DustInsp, "Dust Inspction", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_DiffInsp, "Diff Inspction", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_RealTimeTrace, "Real Time Trace", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_ReverseOrder, "Reverse Order", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_AutoSaveAVI, "AutoSave AVI", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_MetalTrace, "Metal Trace", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_CycleTrace, "Cycle Trace", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_TraceInterval, "Trace Interval", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_EdgePolarity, "Search Edge Polarity", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_SearchDir, "Search Dir", 14.f, COLOR_BTN_BODY);
	
	//KJH 2021-08-10 SCAN INSP 전처리 , 거리 측정 Thresh 값 파라미터
	if (m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_SCAN_INSP)
	{
		InitTitle(&m_stt_ELB_Hi_thresh, "Image Thresh", 14.f, COLOR_BTN_BODY);
		InitTitle(&m_stt_ELB_Low_thresh, "Dist Thresh", 14.f, COLOR_BTN_BODY);
	}
	else
	{
		InitTitle(&m_stt_ELB_Hi_thresh, "High Thresh", 14.f, COLOR_BTN_BODY);
		InitTitle(&m_stt_ELB_Low_thresh, "Low Thresh", 14.f, COLOR_BTN_BODY);
	}
	InitTitle(&m_stt_ELB_Result_Count, "Result Count", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_HoleSize_C1, "CC Size(mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_HoleSize_C2, "PN Size(mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_HoleSize_C3, "MP Size(mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_Dust_Thresh, "Dust Thresh", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_SubInspMinSize, "Insp MinSize", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_staticMetalTraceFirst, "Metal Trace First", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_staticMtoNSelectAlgorithm, "M / N Algorithm Select", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_BM_BASE, "BM BASE", 14.f, COLOR_BTN_BODY);
	
	InitTitle(&m_stt_ExistDummyGray, "Dummy Gray(>)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_DistanceInspLSL, "Distance LSL(mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_DistanceInspUSL, "Distance USL(mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_DistanceInspSubSpec, "Tilt Standard(mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_DistanceInspSubSpecTolerance, "Tilit Tolerance (mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_DistanceInspHoleMetalSpec, "H - Metal Spec (mm)", 14.f, COLOR_BTN_BODY);

	InitTitle(&m_stt_UseFixture, "Fixture Use", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_UseReference, "Reference Use", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_GrabDelay, "Grab Delay(ms)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_InspectionMethod, "Inspection Method", 14.f, COLOR_BTN_BODY);

	InitTitle(&m_stt_EnableRevisionReverseX, "Revision Reverse X", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_EnableRevisionReverseY, "Revision Reverse Y", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_EnableRevisionReverseT, "Revision Reverse T", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_EnableRevisionReverseXY, "Revision Reverse XY", 14.f, COLOR_BTN_BODY);

	EditButtonInit( &m_edt_AlignSpec, 20);
	EditButtonInit( &m_edt_AlignSpecMinMax, 20);
	EditButtonInit( &m_edt_RevisionOffset[0], 20);
	EditButtonInit( &m_edt_RevisionOffset[1], 20);
	EditButtonInit( &m_edt_RevisionOffset[2], 20);
	EditButtonInit( &m_edt_AlignLimitX, 20);
	EditButtonInit( &m_edt_AlignLimitY, 20);
	EditButtonInit( &m_edt_AlignLimitT, 20);
	EditButtonInit(&m_edt_LCheckTor, 20);
	EditButtonInit(&m_edt_LCheckSpecX, 20);
	EditButtonInit( &m_edt_LCheckSpecY, 20);
	EditButtonInit( &m_edt_LCheckScale, 20);
	EditButtonInit(&m_edt_GrabDelay, 20);

	EditButtonInit( &m_edt_IncludedAngle, 20);
	EditButtonInit( &m_edt_IncludedAngleLimit, 20);
	EditButtonInit(&m_edt_Insp_Hi_Thresh, 20);
	EditButtonInit(&m_edt_Insp_Low_Thresh, 20);

	//210110
	EditButtonInit(&m_edt_ExistPanelGray, 20);
	EditButtonInit(&m_edt_ExistDummyGray, 20);
	EditButtonInit(&m_edt_InspRangeX, 20);
	EditButtonInit(&m_edt_InspRangeY, 20);
	EditButtonInit(&m_edt_InspRangeLength, 20);
	EditButtonInit(&m_edt_InspTraceInterval, 20);
	EditButtonInit(&m_edt_DistanceInspSpecLSL, 20);
	EditButtonInit(&m_edt_DistanceInspSpecUSL, 20);
	EditButtonInit(&m_edt_DistanceInspSubSpec, 20);
	EditButtonInit(&m_edt_DistanceInspSubSpecTolerance, 20);
	EditButtonInit(&m_edt_InspHoleSizeC1, 20);
	EditButtonInit(&m_edt_InspHoleSizeC2, 20);
	EditButtonInit(&m_edt_InspHoleSizeC3, 20);
	EditButtonInit(&m_edt_InspDustThresh, 20);
	EditButtonInit(&m_edt_InspMinSize, 20);
	EditButtonInit(&m_edt_DistanceInspHoleMetal, 20);
	EditButtonInit(&m_edt_InspResultCount, 20);
	//210110

	EditGroupInit(&m_grpAlignFunction, "Align Function", 15.f, COLOR_BTN_BODY);
	EditGroupInit(&m_grpLCheck, "L Check", 15.f, COLOR_BTN_BODY);	
	EditGroupInit(&m_grpAlignType, "Align Type", 15.f, COLOR_BTN_BODY);
	EditGroupInit(&m_grpRevision, "Revision", 15.f, COLOR_BTN_BODY);
	EditGroupInit(&m_grpExistFunction, "Pre Function", 15.f, COLOR_BTN_BODY);
	EditGroupInit(&m_grpElbInspFunction, "ELB Inspection Function", 15.f, COLOR_BTN_BODY);
	EditGroupInit(&m_grpDistanceFunction, "Distance Inspection Function", 15.f, COLOR_BTN_BODY);
	EditGroupInit(&m_grpDistanceSubFunction, "Distance Inspection Sub Function", 15.f, COLOR_BTN_BODY);
	EditGroupInit(&m_grpDistanceFunction, "Distance Inspection Function", 15.f, COLOR_BTN_BODY);

	MainButtonInit(&m_btn_InspspectionParaSet, 12);	 // Tkyuha 20211210 검사 파라미터 프로퍼티 창 구현

	m_btnEnableAlignMeasureSpecOutJudge.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableLCheck.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnUseCrackInsp.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableDustCheck.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableDiffCheck.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableRealTimeTrace.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableReverseOrder.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableAutoSaveAVI.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableMetalTrace.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableMetalTraceFirst.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableCycleTrace.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableUseSubInspAlgorithm.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnUseDummyCornerInsp.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnUseIncludedAngle.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnUseFixtureMark.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnUseReferenceMark.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableRevisionReverseX.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableRevisionReverseY.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableRevisionReverseT.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableRevisionReverseXY.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableBMBase.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableInputInspection.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	

	int camCount = m_pMain->vt_job_info[m_nJobID].num_of_camera;
	int posCount = m_pMain->vt_job_info[m_nJobID].num_of_position;
	std::vector<int> camBuf = m_pMain->vt_job_info[m_nJobID].camera_index;

	CString strTemp;
	

	strTemp = "ALL";

	m_selInspectionMethod.SetCurSel(0);

	showModelInfo(m_pMain->getCurrModelName());
	
	NotUseVisible(FALSE);

	for(int i = 0; i < NUM_ALIGN_POINTS; i++)
	{
		strTemp.Format("Pos %d", i + 1);
		m_cmbInspSpec.AddString(strTemp);
	}
	m_cmbInspSpec.SetCurSel(0);

	for (int i = 0; i < 4; i++)
	{
		strTemp.Format("Pos %d", i + 1);
		m_cmbInspSubSpec.AddString(strTemp);
	}
	m_cmbInspSubSpec.SetCurSel(0);
	
	// dh.jung 2021-08-02 add
	EditGroupInit(&m_grpInspSpecPara, "Scan Inpection Parameter && Spec", 15.f, COLOR_BTN_BODY);
	EditGroupInit(&m_grpInspSpec, "Scan Inpection Spec", 15.f, COLOR_BTN_BODY);
	EditGroupInit(&m_grpInspPara, "Scan Inpection Parameter", 15.f, COLOR_BTN_BODY);

	m_cmbInspScanPos.AddString("Scan Pos 1");
	m_cmbInspScanPos.AddString("Scan Pos 2");
	m_cmbInspScanPos.AddString("Scan Pos 3");
	m_cmbInspScanPos.SetCurSel(0);

	m_cmbInspItemList.AddString("Setting Item 1");
	m_cmbInspItemList.AddString("Setting Item 2");
	m_cmbInspItemList.AddString("Setting Item 3");
	m_cmbInspItemList.AddString("Setting Item 4");
	m_cmbInspItemList.SetCurSel(0);

	InitTitle(&m_stt_InspJudgeSpec, "In Spec", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_InspJudgeSpecTorr, "In Spec Torr", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_InspParaItem1, "Start Search Point (Distance)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_InspParaItem2, "End Search Point (Distance)", 14.f, COLOR_BTN_BODY);

	EditButtonInit(&m_edt_InspJudgeSpec, 20);
	EditButtonInit(&m_edt_InspJudgeSpecTorr, 20);
	EditButtonInit(&m_edt_InspParaItem1, 20);
	EditButtonInit(&m_edt_InspParaItem2, 20);	

	//KJH 2021-11-12 Trace Parameter 작업 시작
	EditGroupInit(&m_grpELBTracePara,	"ELB Trace Insp Parameter", 15.f, COLOR_BTN_BODY);
	EditGroupInit(&m_grpTraceImagePara, "Trace Image Parameter", 15.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_TraceImagePara,	"Trace Image Thresh",	14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_FindEdgePara,		"Find Edge Thresh",		14.f, COLOR_BTN_BODY);
	EditButtonInit(&m_edt_TraceImagePara, 20);
	EditButtonInit(&m_edt_FindEdgePara, 20);

	m_cmbELBTracePara.AddString("Trace Insp Para_1");
	m_cmbELBTracePara.AddString("Trace Insp Para_2");
	m_cmbELBTracePara.AddString("Trace Insp Para_3");
	m_cmbELBTracePara.AddString("Trace Insp Para_4");
	m_cmbELBTracePara.SetCurSel(0);

	//KJH 2021-12-01 Dust Insp Image Para 추가
	EditGroupInit(&m_grpDiffImagePara, "Diff Inpection Parameter", 15.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_DiffInspParaItem1, "Dust Insp Range (mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_DiffInspParaItem2, "-", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_DiffInspROIShape, "Dust Insp ROI Shape Type (USE : CIRCLE)", 14.f, COLOR_BTN_BODY);
	EditButtonInit(&m_edt_DiffInspParaItem1, 20);
	EditButtonInit(&m_edt_DiffInspParaItem2, 20);
	m_btnDiffInspROIShape.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));

	m_pInspectionDlg = new CGridInspectionParamDlg;
	m_pInspectionDlg->SetMainParent(m_pMain,m_nJobID);
	m_pInspectionDlg->Create(IDD_DLG_SET_INSPPARA, this);

	CRect rect;
	int w, h;
	GetWindowRect(&rect);

	GetDlgItem(IDC_STATIC_ENABLE_L_CHECK)->GetWindowRect(&m_rtLCheckUse);

	w = m_rtLCheckUse.Width();
	h = m_rtLCheckUse.Height();

	m_rtLCheckUse.left = m_rtLCheckUse.left - rect.left;
	m_rtLCheckUse.top = m_rtLCheckUse.top - rect.top;
	m_rtLCheckUse.right = m_rtLCheckUse.left + w;
	m_rtLCheckUse.bottom = m_rtLCheckUse.top + h;

	//end

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}
HBRUSH TabModelAlgoPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	if (pWnd->m_hWnd == this->m_hWnd) hbr = m_hbrBkg;

	if (nCtlColor == CTLCOLOR_LISTBOX || nCtlColor == CTLCOLOR_EDIT || nCtlColor == CTLCOLOR_MSGBOX)
	{
		pDC->SetTextColor(RGB(255, 0, 0));
		pDC->SetBkColor(RGB(64, 64, 64));
	}

	if (pWnd->GetDlgCtrlID() == IDC_CB_MARK_FIND_METHOD ||
		pWnd->GetDlgCtrlID() == IDC_COMBO_ALIGNMENT_TYPE ||
		pWnd->GetDlgCtrlID() == IDC_COMBO_ANGLE_CALC_METHOD ||
		pWnd->GetDlgCtrlID() == IDC_COMBO_THETA_REFERENCE ||
		pWnd->GetDlgCtrlID() == IDC_COMBO_CALIBRATION_TYPE ||
		pWnd->GetDlgCtrlID() == IDC_COMBO_ALIGNMENT_DIR ||
		pWnd->GetDlgCtrlID() == IDC_CB_MARK_FIND_METHOD2 ||
		pWnd->GetDlgCtrlID() == IDC_CB_CHIP_RANSAC_DIRECTION ||
		pWnd->GetDlgCtrlID() == IDC_COMBO_DOPO_START_PT ||
		pWnd->GetDlgCtrlID() == IDC_COMBO_DOPO_END_PT ||
		pWnd->GetDlgCtrlID() == IDC_CB_CORNER_TYPE0 ||
		pWnd->GetDlgCtrlID() == IDC_CB_CORNER_TYPE1 ||
		pWnd->GetDlgCtrlID() == IDC_CB_EDGE_DIRECTION0 ||
		pWnd->GetDlgCtrlID() == IDC_CB_EDGE_DIRECTION1 ||
		pWnd->GetDlgCtrlID() == IDC_CB_PRE_PROCESS1 ||
		pWnd->GetDlgCtrlID() == IDC_CB_PRE_PROCESS2 ||
		pWnd->GetDlgCtrlID() == IDC_CB_EDGE_POLARITY0 ||
		pWnd->GetDlgCtrlID() == IDC_CB_EDGE_POLARITY1 ||
		pWnd->GetDlgCtrlID() == IDC_COMBO_DUMMY_CORNER_INSP_CAM ||
		pWnd->GetDlgCtrlID() == IDC_COMBO_DUMMY_CORNER_FILTER ||
		pWnd->GetDlgCtrlID() == IDC_CB_EDGE_PROCESS1 ||
		pWnd->GetDlgCtrlID() == IDC_CB_EDGE_PROCESS2 ||
		pWnd->GetDlgCtrlID() == IDC_CB_MODEL_SELECT_CAM ||
		pWnd->GetDlgCtrlID() == IDC_COMBO_INSP_METHOD ||
		pWnd->GetDlgCtrlID() == IDC_CB_MODEL_SELECT_POS||
		pWnd->GetDlgCtrlID() == IDC_COMBO_MANUAL_MARK_COUNT)
	{
		pDC->SetBkColor(COLOR_UI_BODY);
		pDC->SetTextColor(COLOR_WHITE);
	}

	return hbr;
}
LRESULT TabModelAlgoPage::OnViewControl(WPARAM wParam, LPARAM lParam)
{
	LRESULT lRet = 0;
	switch ((int)wParam) {	
	case MSG_MV_DISP_MODEL_INFO:
	{
		CString strModel = *((CString *)lParam);
		showModelInfo(strModel);
		//showAlignInfo(strModel);
		enableEditControls(FALSE);
	}
	break;
	case MSG_MV_ENABLE_CONTROL:
		enableEditControls(BOOL(lParam));
	break;
	case MSG_MV_SAVE_ALGO_DATA:
		applyModelData();
	break;
	case MSG_MV_LOAD_ALGO_DATA:
	{
		CString strModel = m_pMain->m_strCurrModel;
		showModelInfo(strModel);
		//showAlignInfo(strModel);
		enableEditControls(FALSE);
	}
		break;
	case MSG_MV_PARAM_ALGO_DATA:
		getSaveParam();
		break;
	case MSG_MV_PARAM_ALGO_DATA_SAME_CENTER_ITEM: //lhj add 220503
		getUIValue();
		break;
}

	return lRet;
}
void TabModelAlgoPage::applyModelData()
{
	int nSel = 0, nCam = 0;
	CString str, str2;
	int camCount = m_pMain->vt_job_info[m_nJobID].num_of_camera;
	int posCount = m_pMain->vt_job_info[m_nJobID].num_of_position;

	std::vector<int> camBuf = m_pMain->vt_job_info[m_nJobID].camera_index;
	int nCamIndex = camBuf.at(m_nModelSelCam);

	CModel* pModel = &m_pMain->vt_job_info[m_nJobID].model_info;

	GetDlgItem(IDC_EDIT_ALIGN_SPEC)->GetWindowTextA(str);
	GetDlgItem(IDC_EDIT_ALIGN_SPEC_MINMAX)->GetWindowTextA(str2);

	pModel->getAlignInfo().setAlignSpec(0, atof(str));
	pModel->getAlignInfo().setAlignSpecMinMax(0, atof(str));

	GetDlgItem(IDC_EDIT_ELB_INSP_HI_THRESH)->GetWindowTextA(str);
	pModel->getInspSpecParaInfo().setInspectionHighThresh(atoi(str));

	GetDlgItem(IDC_EDIT_ELB_INSP_LOW_THRESH)->GetWindowTextA(str);
	pModel->getInspSpecParaInfo().setInspectionLowThresh(atoi(str));

	GetDlgItem(IDC_EDIT_DISTANCE_INSP__HOLE_METAL)->GetWindowTextA(str);
	pModel->getAlignInfo().setDistanceInspHoleMetal(atof(str));//2022.06.22 ksm 현재 사용하지않지만 다른 값에 덮어쓰여서 삭제


	nSel = ((CComboBox*)GetDlgItem(IDC_COMBO_ALIGNMENT_DIR))->GetCurSel();
	pModel->getAlignInfo().setAlignmentTargetDir(nSel);

	nSel = ((CComboBox*)GetDlgItem(IDC_CB_MARK_FIND_METHOD))->GetCurSel();
	pModel->getAlignInfo().setMarkFindMethod(nSel);

	nSel = ((CComboBox*)GetDlgItem(IDC_COMBO_ANGLE_CALC_METHOD))->GetCurSel();
	pModel->getAlignInfo().setAngleCalcMethod(nSel);

	nSel = ((CComboBox*)GetDlgItem(IDC_COMBO_ALIGNMENT_TYPE))->GetCurSel();
	pModel->getMachineInfo().setAlignmentType(nSel);

	// Revision
	GetDlgItem(IDC_EDIT_REVISION_LIMIT_X)->GetWindowTextA(str);
	pModel->getMachineInfo().setRevisionLimit(AXIS_X, atof(str));

	GetDlgItem(IDC_EDIT_REVISION_LIMIT_Y)->GetWindowTextA(str);
	pModel->getMachineInfo().setRevisionLimit(AXIS_Y, atof(str));

	GetDlgItem(IDC_EDIT_REVISION_LIMIT_T)->GetWindowTextA(str);
	pModel->getMachineInfo().setRevisionLimit(AXIS_T, atof(str));

	GetDlgItem(IDC_EDIT_REVISION_OFFSET_X)->GetWindowTextA(str);
	pModel->getMachineInfo().setRevisionOffset(AXIS_X, atof(str));

	GetDlgItem(IDC_EDIT_REVISION_OFFSET_Y)->GetWindowTextA(str);
	pModel->getMachineInfo().setRevisionOffset(AXIS_Y, atof(str));

	GetDlgItem(IDC_EDIT_REVISION_OFFSET_T)->GetWindowTextA(str);
	pModel->getMachineInfo().setRevisionOffset(AXIS_T, atof(str));

	BOOL bCheck = m_btnEnableLCheck.GetCheck();

	// Align Function
	if (m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_FILM_INSP ||
		m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_1CAM_1SHOT_FILM ||
		m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_1CAM_1SHOT_ALIGN ||
		m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_ALIGN)						 // YCS 2022-11-08 Attach / Pre Align도 마크 카운트 파라미터 사용하도록 추가
	{
		int nMarkCount = m_cmbManualMarkCount.GetCurSel();
		pModel->getAlignInfo().setEnableAlignMeasureSpecOutJudgeCount(nMarkCount);
		if(nMarkCount > 0)
			pModel->getAlignInfo().setEnableAlignMeasureSpecOutJudge(true);
		else 
			pModel->getAlignInfo().setEnableAlignMeasureSpecOutJudge(false);
	}
	else
	{
		pModel->getAlignInfo().setEnableAlignMeasureSpecOutJudge(m_btnEnableAlignMeasureSpecOutJudge.GetCheck());
	}
	pModel->getAlignInfo().setEnableLCheck(m_btnEnableLCheck.GetCheck());
	pModel->getAlignInfo().setUseCrackInsp(m_btnUseCrackInsp.GetCheck());
	pModel->getAlignInfo().setUseDummyCornerInsp(m_btnUseDummyCornerInsp.GetCheck());
	pModel->getAlignInfo().setUseIncludedAngle(m_btnUseIncludedAngle.GetCheck());
	pModel->getAlignInfo().setUseFixtureMark(m_btnUseFixtureMark.GetCheck());
	pModel->getAlignInfo().setUseReferenceMark(m_btnUseReferenceMark.GetCheck());
	pModel->getAlignInfo().setRevisionReverseX(m_btnEnableRevisionReverseX.GetCheck());
	pModel->getAlignInfo().setRevisionReverseY(m_btnEnableRevisionReverseY.GetCheck());
	pModel->getAlignInfo().setRevisionReverseT(m_btnEnableRevisionReverseT.GetCheck());
	pModel->getAlignInfo().setRevisionReverseXY(m_btnEnableRevisionReverseXY.GetCheck());
	pModel->getAlignInfo().setUseRealTimeTrace(m_btnEnableRealTimeTrace.GetCheck());
	pModel->getAlignInfo().setUseMetalTrace(m_btnEnableMetalTrace.GetCheck());
	pModel->getAlignInfo().setUseCycleTrace(m_btnEnableCycleTrace.GetCheck());
	pModel->getAlignInfo().setUseReverseOrder(m_btnEnableReverseOrder.GetCheck());
	pModel->getAlignInfo().setUseMetalTraceFirst(m_btnEnableMetalTraceFirst.GetCheck());
	pModel->getAlignInfo().setUseSubInspAlgorithm(m_btnEnableUseSubInspAlgorithm.GetCheck());
	pModel->getAlignInfo().setUseAutoSaveAVI(m_btnEnableAutoSaveAVI.GetCheck());
	pModel->getAlignInfo().setUsePNBase(m_btnEnableBMBase.GetCheck());
	pModel->getAlignInfo().setLCheckReference(m_cmbLCheckRef.GetCurSel());
	pModel->getAlignInfo().setSearchEdgePolarity(m_cmbSearchEdgePolarity.GetCurSel());
	pModel->getAlignInfo().setSearchSearchDir(m_cmbSearchDir.GetCurSel());

	GetDlgItem(IDC_EDIT_L_CHECK_TOLERANCE)->GetWindowTextA(str);
	pModel->getAlignInfo().setLCheckTor(atof(str));

	GetDlgItem(IDC_EDIT_GRAB_DELAY)->GetWindowTextA(str);
	pModel->getAlignInfo().setGrabDelay(atoi(str));

	GetDlgItem(IDC_EDIT_L_CHECK_SPEC)->GetWindowTextA(str);
	pModel->getAlignInfo().setLCheckSpecX(atof(str));
	GetDlgItem(IDC_EDIT_L_CHECK_SPEC_Y)->GetWindowTextA(str);
	pModel->getAlignInfo().setLCheckSpecY(atof(str));
	//pModel->getAlignInfo().(atof(str));
	GetDlgItem(IDC_EDIT_LCHECK_SCALE)->GetWindowTextA(str);
	pModel->getAlignInfo().setLCheckScale(atof(str));


	GetDlgItem(IDC_EDIT_INCLUDED_ANGLE)->GetWindowTextA(str);
	pModel->getAlignInfo().setIncludedAngle(atof(str));

	GetDlgItem(IDC_EDIT_INCLUDED_ANGLE_LIMIT)->GetWindowTextA(str);
	pModel->getAlignInfo().setIncludedAngleLimit(atof(str));


	GetDlgItem(IDC_EDIT_ELB_INSP_RANGE_X)->GetWindowTextA(str);
	pModel->getAlignInfo().setInspRangeX(atof(str));
	GetDlgItem(IDC_EDIT_ELB_INSP_RANGE_Y)->GetWindowTextA(str);
	pModel->getAlignInfo().setInspRangeY(atof(str));
	GetDlgItem(IDC_EDIT_ELB_INSP_LENGTH)->GetWindowTextA(str);
	pModel->getAlignInfo().setInspRangeLength(atof(str));
	GetDlgItem(IDC_EDIT_ELB_TRACE_INTERVAL)->GetWindowTextA(str);
	pModel->getAlignInfo().setInspTraceInterval(atof(str));
	GetDlgItem(IDC_EDIT_ELB_INSP_HOLE_C1)->GetWindowTextA(str);
	pModel->getAlignInfo().setInspInHoleSize(atof(str));
	GetDlgItem(IDC_EDIT_ELB_INSP_HOLE_C2)->GetWindowTextA(str);
	pModel->getAlignInfo().setInspCamHoleSize(atof(str));
	GetDlgItem(IDC_EDIT_ELB_INSP_HOLE_C3)->GetWindowTextA(str);
	pModel->getAlignInfo().setInspSteelHoleSize(atof(str));
	GetDlgItem(IDC_EDIT_ELB_INSP_RESULT_C)->GetWindowTextA(str);
	pModel->getAlignInfo().setInspResultCount(atoi(str));
	
	/*
	//  즉시 반영 되도록. 수정
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_SUB_SPEC)->GetWindowTextA(str);
	pModel->getAlignInfo().setDistanceInspSubSpec(atof(str));
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_SPECY)->GetWindowTextA(str);
	pModel->getAlignInfo().setDistanceInspSpecY(atof(str));
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_SPEC)->GetWindowTextA(str);
	pModel->getAlignInfo().setDistanceInspSpec(atof(str));
	*/


	/* 수정 21.03.09
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_SUB_SPEC_TOLERANCE)->GetWindowTextA(str);
	pModel->getAlignInfo().setDistanceInspSubSpecXTolerance(atof(str));
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_SPEC_Y_MINMAX)->GetWindowTextA(str);
	pModel->getAlignInfo().setDistanceInspSpecYMinMax(atof(str));
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_USL)->GetWindowTextA(str);
	pModel->getAlignInfo().setDistanceInspSpecUSL(atof(str));
	*/

	nSel = ((CComboBox*)GetDlgItem(IDC_COMBO_INSP_METHOD))->GetCurSel();
	pModel->getAlignInfo().setInspectionMethod(nSel);


	// dh.jung 2021-08-02 add
	CInspSpecPara* pInspSpec = &m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo();
	int nScanPos = ((CComboBox*)GetDlgItem(IDC_COMBO_INSP_SCAN_POS))->GetCurSel();
	int nItemList = ((CComboBox*)GetDlgItem(IDC_COMBO_INSP_ITEM_LIST))->GetCurSel();

	if (nItemList == 1)
	{
		GetDlgItem(IDC_EDIT_INSP_JUDGE_SPEC)->GetWindowTextA(str);
		pInspSpec->setOutSpec(nScanPos, atof(str));
		GetDlgItem(IDC_EDIT_INSP_JUDGE_SPEC_TORR)->GetWindowTextA(str);
		pInspSpec->setOutSpecTorr(nScanPos, atof(str));
		GetDlgItem(IDC_EDIT_INSP_PARA_ITEM1)->GetWindowTextA(str);
		pInspSpec->setSerchMinMaxJudgeOffsetIn(nScanPos, atof(str));
		GetDlgItem(IDC_EDIT_INSP_PARA_ITEM2)->GetWindowTextA(str);
		pInspSpec->setSerchMinMaxJudgeOffsetOut(nScanPos, atof(str));
	}
	else if(nItemList == 2)
	{
		GetDlgItem(IDC_EDIT_INSP_JUDGE_SPEC)->GetWindowTextA(str);
		pInspSpec->setTotalSpec(nScanPos, atof(str));
		GetDlgItem(IDC_EDIT_INSP_JUDGE_SPEC_TORR)->GetWindowTextA(str);
		pInspSpec->setTotalSpecTorr(nScanPos, atof(str));
		GetDlgItem(IDC_EDIT_INSP_PARA_ITEM1)->GetWindowTextA(str);
		pInspSpec->setDustInspIn(nScanPos, atof(str));
		GetDlgItem(IDC_EDIT_INSP_PARA_ITEM2)->GetWindowTextA(str);
		pInspSpec->setDustInspOut(nScanPos, atof(str));
	}
	else if (nItemList == 3)
	{
		GetDlgItem(IDC_EDIT_INSP_JUDGE_SPEC)->GetWindowTextA(str);
		pInspSpec->setDistanceSpec(nScanPos, atof(str));
		GetDlgItem(IDC_EDIT_INSP_JUDGE_SPEC_TORR)->GetWindowTextA(str);
		pInspSpec->setDistanceSpecTorr(nScanPos, atof(str));
		GetDlgItem(IDC_EDIT_INSP_PARA_ITEM1)->GetWindowTextA(str);
		pInspSpec->setLimitLineInspOffset(nScanPos, atof(str));
//		GetDlgItem(IDC_EDIT_INSP_PARA_ITEM2)->GetWindowTextA(str);
//		pInspSpec->setInspSpec(nScanPos, atof(str));
	}
	else 
	{
		GetDlgItem(IDC_EDIT_INSP_JUDGE_SPEC)->GetWindowTextA(str);
		pInspSpec->setInSpec(nScanPos, atof(str));
		GetDlgItem(IDC_EDIT_INSP_JUDGE_SPEC_TORR)->GetWindowTextA(str);
		pInspSpec->setInSpecTorr(nScanPos, atof(str));

		GetDlgItem(IDC_EDIT_INSP_PARA_ITEM1)->GetWindowTextA(str);
		pInspSpec->setStartSerchPoint(nScanPos, atof(str));
		GetDlgItem(IDC_EDIT_INSP_PARA_ITEM2)->GetWindowTextA(str);
		pInspSpec->setEndSerchPoint(nScanPos, atof(str));
	}

	//KJH 2021-11-12 Trace Parameter 작업 시작

	GetDlgItem(IDC_EDIT_TRACE_IMAGE_THRESH)->GetWindowTextA(str);
	pInspSpec->setTraceImageThresh(atoi(str));

	GetDlgItem(IDC_EDIT_FIND_EDGE_THRESH)->GetWindowTextA(str);
	pInspSpec->setFindEdgeThresh(atoi(str));

	//KJH 2021-12-01 Dust Insp Image Para 추가

	GetDlgItem(IDC_EDIT_DIFF_INSP_PARA_ITEM1)->GetWindowTextA(str);
	pInspSpec->setDiffInspROISize(atof(str));
	pInspSpec->setDiffInspROIShape(m_btnDiffInspROIShape.GetCheck());

	//KJH 2022-01-09 검사 flag Aligninfo->inspSpecinfo로 위치변경

	pModel->getInspSpecParaInfo().setUseDustInsp(m_btnEnableDustCheck.GetCheck());
	pModel->getInspSpecParaInfo().setUseDiffInsp(m_btnEnableDiffCheck.GetCheck());
	pModel->getInspSpecParaInfo().setUseInputInspection(m_btnEnableInputInspection.GetCheck());

	GetDlgItem(IDC_EDIT_EXIST_PANEL_GRAY)->GetWindowTextA(str); //수정해야할부분
	pModel->getInspSpecParaInfo().setExistPanelGray(atoi(str));
	GetDlgItem(IDC_EDIT_EXIST_DUMMY_GRAY)->GetWindowTextA(str);
	pModel->getInspSpecParaInfo().setExistDummyGray(atoi(str));

	GetDlgItem(IDC_EDIT_ELB_INSP_DUST)->GetWindowTextA(str);
	pModel->getInspSpecParaInfo().setInspDustThresh(atoi(str));

	GetDlgItem(IDC_EDIT_ELB_INSP_MINSIZE)->GetWindowTextA(str);
	pModel->getInspSpecParaInfo().setInspMinSize(atof(str));	

	// end 
}
void TabModelAlgoPage::showModelInfo(CString strModel)
{
	CString strModelFolder;
	CFileFind findModel;

	strModelFolder.Format("%s%s", m_pMain->m_strModelDir, strModel);

	if (_access(strModelFolder, 0) != 0)
	{
		CString strError;
		strError = "'" + strModel + "' Model is Not Exist.";
		m_pMain->fnSetMessage(1, strError);
		return;
	}

	showMachineInfo(strModel);
	showAlignInfo(strModel);
	showInspSpecInfo(strModel);
}
void TabModelAlgoPage::showMachineInfo(CString strModel)
{
	CString strData;
	CString strModelPath;
	strModelPath.Format("%s%s", m_pMain->m_strModelDir, strModel);

	std::vector<int> camBuf = m_pMain->vt_job_info[m_nJobID].camera_index;
	int nCam = m_nModelSelCam;
	CMachineInfo machineInfo = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo();

	strData.Format("%.3f", machineInfo.getRevisionLimit(AXIS_X));
	GetDlgItem(IDC_EDIT_REVISION_LIMIT_X)->SetWindowTextA(strData);

	strData.Format("%.3f", machineInfo.getRevisionLimit(AXIS_Y));
	GetDlgItem(IDC_EDIT_REVISION_LIMIT_Y)->SetWindowTextA(strData);

	strData.Format("%.3f", machineInfo.getRevisionLimit(AXIS_T));
	GetDlgItem(IDC_EDIT_REVISION_LIMIT_T)->SetWindowTextA(strData);

	strData.Format("%.4f", machineInfo.getRevisionOffset(AXIS_X));
	GetDlgItem(IDC_EDIT_REVISION_OFFSET_X)->SetWindowTextA(strData);

	strData.Format("%.4f", machineInfo.getRevisionOffset(AXIS_Y));
	GetDlgItem(IDC_EDIT_REVISION_OFFSET_Y)->SetWindowTextA(strData);

	strData.Format("%.4f", machineInfo.getRevisionOffset(AXIS_T));
	GetDlgItem(IDC_EDIT_REVISION_OFFSET_T)->SetWindowTextA(strData);

	((CComboBox*)GetDlgItem(IDC_COMBO_ALIGNMENT_TYPE))->SetCurSel(machineInfo.getAlignmentType());
}
void TabModelAlgoPage::showAlignInfo(CString strModel)
{
	CString strData;
	CString strModelPath;

	show_enable_parameter();

	CAlignInfo alignInfo = m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo();
	int nCam = m_nModelSelCam;
	int pos = m_cmbInspSpec.GetCurSel();
	int pos2 = m_cmbInspSubSpec.GetCurSel();

	((CComboBox*)GetDlgItem(IDC_COMBO_ANGLE_CALC_METHOD))->SetCurSel(alignInfo.getAngleCalcMethod());
	((CComboBox*)GetDlgItem(IDC_CB_MARK_FIND_METHOD))->SetCurSel(alignInfo.getMarkFindMethod());
	((CComboBox*)GetDlgItem(IDC_COMBO_ALIGNMENT_DIR))->SetCurSel(alignInfo.getAlignmentTargetDir());

	((CComboBox*)GetDlgItem(IDC_COMBO_INSP_METHOD))->SetCurSel(alignInfo.getInspectionMethod());

	((CComboBox*)GetDlgItem(IDC_COMBO_LCHECK_REFERENCE))->SetCurSel(alignInfo.getLCheckReference());
	((CComboBox*)GetDlgItem(IDC_COMBO_EDGE_POLARITY))->SetCurSel(alignInfo.getSearchEdgePolarity());
	((CComboBox*)GetDlgItem(IDC_COMBO_SEARCH_DIR))->SetCurSel(alignInfo.getSearchSearchDir());
	

	strData.Format("%.3f", alignInfo.getAlignSpec(nCam));
	GetDlgItem(IDC_EDIT_ALIGN_SPEC)->SetWindowTextA(strData);

	strData.Format("%.3f", alignInfo.getAlignSpecMinMax(nCam));
	GetDlgItem(IDC_EDIT_ALIGN_SPEC_MINMAX)->SetWindowTextA(strData);

	strData.Format("%d", alignInfo.getGrabDelay());
	GetDlgItem(IDC_EDIT_GRAB_DELAY)->SetWindowTextA(strData);

	strData.Format("%.3f", alignInfo.getLCheckTor());
	GetDlgItem(IDC_EDIT_L_CHECK_TOLERANCE)->SetWindowTextA(strData);

	strData.Format("%.3f", alignInfo.getLCheckSpecX());
	GetDlgItem(IDC_EDIT_L_CHECK_SPEC)->SetWindowTextA(strData);

	strData.Format("%.3f", alignInfo.getLCheckSpecY());
	GetDlgItem(IDC_EDIT_L_CHECK_SPEC_Y)->SetWindowTextA(strData);

	strData.Format("%.3f", alignInfo.getLCheckScale());
	GetDlgItem(IDC_EDIT_LCHECK_SCALE)->SetWindowTextA(strData);

	strData.Format("%.3f", alignInfo.getIncludedAngle());
	GetDlgItem(IDC_EDIT_INCLUDED_ANGLE)->SetWindowTextA(strData);

	strData.Format("%.3f", alignInfo.getIncludedAngleLimit());
	GetDlgItem(IDC_EDIT_INCLUDED_ANGLE_LIMIT)->SetWindowTextA(strData);

	strData.Format("%4.4f", alignInfo.getInspRangeX());  
	GetDlgItem(IDC_EDIT_ELB_INSP_RANGE_X)->SetWindowTextA(strData);
	strData.Format("%4.4f", alignInfo.getInspRangeY());
	GetDlgItem(IDC_EDIT_ELB_INSP_RANGE_Y)->SetWindowTextA(strData);
	strData.Format("%4.4f", alignInfo.getInspRangeLength());
	GetDlgItem(IDC_EDIT_ELB_INSP_LENGTH)->SetWindowTextA(strData);
	strData.Format("%4.4f", alignInfo.getInspTraceInterval());
	GetDlgItem(IDC_EDIT_ELB_TRACE_INTERVAL)->SetWindowTextA(strData);	

	strData.Format("%.3f", alignInfo.getDistanceInspSpecLSL(pos));
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_LSL)->SetWindowTextA(strData);

	strData.Format("%4.4f", alignInfo.getInspInHoleSize());
	GetDlgItem(IDC_EDIT_ELB_INSP_HOLE_C1)->SetWindowTextA(strData);
	strData.Format("%4.4f", alignInfo.getInspCamHoleSize());
	GetDlgItem(IDC_EDIT_ELB_INSP_HOLE_C2)->SetWindowTextA(strData);
	strData.Format("%4.4f", alignInfo.getInspSteelHoleSize());
	GetDlgItem(IDC_EDIT_ELB_INSP_HOLE_C3)->SetWindowTextA(strData);
	
	strData.Format("%.3f", alignInfo.getDistanceInspSubSpec(pos2));
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_SUB_SPEC)->SetWindowTextA(strData);

	strData.Format("%.3f", alignInfo.getDistanceInspSpecUSL(pos));
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_USL)->SetWindowTextA(strData);
	strData.Format("%.3f", alignInfo.getDistanceInspSubSpecTolerance(pos2));
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_SUB_SPEC_TOLERANCE)->SetWindowTextA(strData);
	strData.Format("%.3f", alignInfo.getDistanceInspHoleMetal());
	GetDlgItem(IDC_EDIT_DISTANCE_INSP__HOLE_METAL)->SetWindowTextA(strData);
	strData.Format("%d", alignInfo.getInspResultCount());
	GetDlgItem(IDC_EDIT_ELB_INSP_RESULT_C)->SetWindowTextA(strData);

	((CComboBox*)GetDlgItem(IDC_COMBO_MANUAL_MARK_COUNT))->SetCurSel(alignInfo.getEnableAlignMeasureSpecOutJudgeCount());
	m_btnEnableAlignMeasureSpecOutJudge.SetCheck(alignInfo.getEnableAlignMeasureSpecOutJudge());
	m_btnUseCrackInsp.SetCheck(alignInfo.getUseCrackInsp());
	m_btnUseIncludedAngle.SetCheck(alignInfo.getUseIncludedAngle());
	m_btnUseDummyCornerInsp.SetCheck(alignInfo.getUseDummyCornerInsp());
	m_btnUseFixtureMark.SetCheck(alignInfo.getUseFixtureMark());
	m_btnUseReferenceMark.SetCheck(alignInfo.getUseReferenceMark());

	m_btnEnableLCheck.SetCheck(alignInfo.getEnableLCheck());
	m_btnEnableRealTimeTrace.SetCheck(alignInfo.getUseRealTimeTrace());
	m_btnEnableCycleTrace.SetCheck(alignInfo.getUseCycleTrace());
	m_btnEnableMetalTrace.SetCheck(alignInfo.getUseMetalTrace());
	m_btnEnableReverseOrder.SetCheck(alignInfo.getUseReverseOrder());
	m_btnEnableUseSubInspAlgorithm.SetCheck(alignInfo.getUseSubInspAlgorithm());	
	m_btnEnableMetalTraceFirst.SetCheck(alignInfo.getUseMetalTraceFirst());	
	m_btnEnableAutoSaveAVI.SetCheck(alignInfo.getUseAutoSaveAVI());
	m_btnEnableBMBase.SetCheck(alignInfo.getUsePNBase());
	
	
	m_btnEnableRevisionReverseX.SetCheck(alignInfo.getRevisionReverseX());
	m_btnEnableRevisionReverseY.SetCheck(alignInfo.getRevisionReverseY());
	m_btnEnableRevisionReverseT.SetCheck(alignInfo.getRevisionReverseT());
	m_btnEnableRevisionReverseXY.SetCheck(alignInfo.getRevisionReverseXY());
}
void TabModelAlgoPage::showInspSpecInfo(CString strModel) 
{
	CString strData;
	CString strModelPath;

	CInspSpecPara InspSpecParaInfo = m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo();
	//CInspSpecPara InspSpecParaInfo = m_pMain->vt_job_info[m_nJobID].insp_spec;

	// dh.jung 2021-08-02 add 
	OnCbnSelchangeComboInspJobPos();

	//KJH 2021-11-12 Trace Parameter 작업 시작

	strData.Format("%d", InspSpecParaInfo.getTraceImageThresh());
	GetDlgItem(IDC_EDIT_TRACE_IMAGE_THRESH)->SetWindowTextA(strData);

	strData.Format("%d", InspSpecParaInfo.getFindEdgeThresh());
	GetDlgItem(IDC_EDIT_FIND_EDGE_THRESH)->SetWindowTextA(strData);
	
	//KJH 2021-12-01 Dust Insp Image Para 추가
	//KJH 2021-12-11 Dust Insp ROI Para 이상현상 수정
	strData.Format("%.3f", InspSpecParaInfo.getDiffInspROISize());
	GetDlgItem(IDC_EDIT_DIFF_INSP_PARA_ITEM1)->SetWindowTextA(strData);

	m_btnDiffInspROIShape.SetCheck(InspSpecParaInfo.getDiffInspROIShape());

	//KJH 2022-01-09 검사 flag Aligninfo->inspSpecinfo로 위치변경

	m_btnEnableDustCheck.SetCheck(InspSpecParaInfo.getUseDustInsp());
	m_btnEnableDiffCheck.SetCheck(InspSpecParaInfo.getUseDiffInsp());
	m_btnEnableInputInspection.SetCheck(InspSpecParaInfo.getUseInputInspection());

	strData.Format("%d", InspSpecParaInfo.getInspectionHighThresh());
	GetDlgItem(IDC_EDIT_ELB_INSP_HI_THRESH)->SetWindowTextA(strData);
	strData.Format("%d", InspSpecParaInfo.getInspectionLowThresh());
	GetDlgItem(IDC_EDIT_ELB_INSP_LOW_THRESH)->SetWindowTextA(strData);
	
	strData.Format("%d", InspSpecParaInfo.getExistPanelGray());  //수정해야할부분
	GetDlgItem(IDC_EDIT_EXIST_PANEL_GRAY)->SetWindowTextA(strData);
	strData.Format("%d", InspSpecParaInfo.getExistDummyGray());
	GetDlgItem(IDC_EDIT_EXIST_DUMMY_GRAY)->SetWindowTextA(strData);

	strData.Format("%d", InspSpecParaInfo.getInspDustThresh());
	GetDlgItem(IDC_EDIT_ELB_INSP_DUST)->SetWindowTextA(strData);

	strData.Format("%.4f", InspSpecParaInfo.getInspMinSize());
	GetDlgItem(IDC_EDIT_ELB_INSP_MINSIZE)->SetWindowTextA(strData);
}
void TabModelAlgoPage::enableEditControls(BOOL bEnable)
{	
	m_bModify = bEnable;
	m_edt_AlignSpec.EnableWindow(bEnable);
	m_edt_AlignSpecMinMax.EnableWindow(bEnable);
	m_edt_RevisionOffset[0].EnableWindow(bEnable);
	m_edt_RevisionOffset[1].EnableWindow(bEnable);
	m_edt_RevisionOffset[2].EnableWindow(bEnable);
	m_edt_AlignLimitX.EnableWindow(bEnable);
	m_edt_AlignLimitY.EnableWindow(bEnable);
	m_edt_AlignLimitT.EnableWindow(bEnable);
	m_edt_LCheckTor.EnableWindow(bEnable);
	m_edt_LCheckSpecX.EnableWindow(bEnable);
	m_edt_LCheckSpecY.EnableWindow(bEnable);
	m_edt_LCheckScale.EnableWindow(bEnable);
	m_edt_GrabDelay.EnableWindow(bEnable);
	m_edt_IncludedAngle.EnableWindow(bEnable);
	m_edt_IncludedAngleLimit.EnableWindow(bEnable);
	m_edt_Insp_Hi_Thresh.EnableWindow(bEnable);
	m_edt_Insp_Low_Thresh.EnableWindow(bEnable);

	//210110
	m_edt_ExistPanelGray.EnableWindow(bEnable);
	m_edt_ExistDummyGray.EnableWindow(bEnable);
	m_edt_InspRangeX.EnableWindow(bEnable);
	m_edt_InspRangeY.EnableWindow(bEnable);
	m_edt_InspRangeLength.EnableWindow(bEnable);
	m_edt_InspTraceInterval.EnableWindow(bEnable);
	m_edt_DistanceInspSpecLSL.EnableWindow(bEnable);
	m_edt_DistanceInspSubSpec.EnableWindow(bEnable);
	m_edt_DistanceInspSpecUSL.EnableWindow(bEnable);
	m_edt_DistanceInspSubSpecTolerance.EnableWindow(bEnable);
	m_edt_InspHoleSizeC1.EnableWindow(bEnable);
	m_edt_InspHoleSizeC2.EnableWindow(bEnable);
	m_edt_InspHoleSizeC3.EnableWindow(bEnable);
	m_edt_InspDustThresh.EnableWindow(bEnable);
	m_edt_InspMinSize.EnableWindow(bEnable);
	m_edt_DistanceInspHoleMetal.EnableWindow(bEnable);
	m_edt_InspResultCount.EnableWindow(bEnable);
	//210110

	m_cmbAlignDir.EnableWindow(bEnable);
	m_cmbMarkFindMethod.EnableWindow(bEnable);
	m_cmbLCheckRef.EnableWindow(bEnable);
	m_cmbSearchEdgePolarity.EnableWindow(bEnable);
	m_cmbSearchDir.EnableWindow(bEnable);
	// YCS 2022-11-23 마크카운트 컨트롤 활성화 추가
	m_cmbManualMarkCount.EnableWindow(bEnable);
	m_cmbInspSpec.EnableWindow(bEnable);
	m_cmbInspSubSpec.EnableWindow(bEnable);
	m_selAlignType.EnableWindow(bEnable);
	m_cmbAngleCalcMethod.EnableWindow(bEnable);
	m_selInspectionMethod.EnableWindow(bEnable);
	m_btn_InspspectionParaSet.EnableWindow(bEnable);
	m_btnEnableAlignMeasureSpecOutJudge.EnableWindow(bEnable);
	m_btnEnableLCheck.EnableWindow(bEnable);
	m_btnUseCrackInsp.EnableWindow(bEnable);
	m_btnUseDummyCornerInsp.EnableWindow(bEnable);
	m_btnUseIncludedAngle.EnableWindow(bEnable);
	m_btnUseFixtureMark.EnableWindow(bEnable);
	m_btnUseReferenceMark.EnableWindow(bEnable);
	m_btnEnableRevisionReverseX.EnableWindow(bEnable);
	m_btnEnableRevisionReverseY.EnableWindow(bEnable);
	m_btnEnableRevisionReverseT.EnableWindow(bEnable);
	m_btnEnableRevisionReverseXY.EnableWindow(bEnable);
	m_btnEnableDustCheck.EnableWindow(bEnable);
	m_btnEnableDiffCheck.EnableWindow(bEnable);
	m_btnEnableRealTimeTrace.EnableWindow(bEnable);
	m_btnEnableCycleTrace.EnableWindow(bEnable);
	m_btnEnableMetalTrace.EnableWindow(bEnable);
	m_btnEnableReverseOrder.EnableWindow(bEnable);
	m_btnEnableUseSubInspAlgorithm.EnableWindow(bEnable);
	m_btnEnableMetalTraceFirst.EnableWindow(bEnable);
	m_btnEnableAutoSaveAVI.EnableWindow(bEnable);
	m_btnEnableBMBase.EnableWindow(bEnable);
	m_btnEnableInputInspection.EnableWindow(bEnable);
	m_btnUseIncludedAngle.RedrawWindow();
	m_btnUseDummyCornerInsp.RedrawWindow();
	m_btnUseCrackInsp.RedrawWindow();	
	m_btnEnableAlignMeasureSpecOutJudge.RedrawWindow();	
	m_btnEnableLCheck.RedrawWindow();	
	m_btnUseFixtureMark.RedrawWindow();
	m_btnUseReferenceMark.RedrawWindow();
	m_btnEnableRevisionReverseX.RedrawWindow();
	m_btnEnableRevisionReverseY.RedrawWindow();
	m_btnEnableRevisionReverseT.RedrawWindow();
	m_btnEnableRevisionReverseXY.RedrawWindow();
	m_btnEnableDustCheck.RedrawWindow();
	m_btnEnableDiffCheck.RedrawWindow();
	m_btnEnableRealTimeTrace.RedrawWindow();
	m_btnEnableCycleTrace.RedrawWindow();
	m_btnEnableMetalTrace.RedrawWindow();
	m_btnEnableReverseOrder.RedrawWindow();
	m_btnEnableUseSubInspAlgorithm.RedrawWindow();
	m_btnEnableMetalTraceFirst.RedrawWindow();
	m_btnEnableAutoSaveAVI.RedrawWindow();
	m_btnEnableBMBase.RedrawWindow();
	m_btnEnableInputInspection.RedrawWindow();
	
	// dh.jung 2021-08-02 add
	m_cmbInspScanPos.EnableWindow(bEnable);
	m_cmbInspItemList.EnableWindow(bEnable);
	m_stt_InspJudgeSpec.EnableWindow(bEnable);
	m_edt_InspJudgeSpec.EnableWindow(bEnable);
	m_stt_InspJudgeSpecTorr.EnableWindow(bEnable);
	m_edt_InspJudgeSpecTorr.EnableWindow(bEnable);
	m_stt_InspParaItem1.EnableWindow(bEnable);
	m_edt_InspParaItem1.EnableWindow(bEnable);
	m_stt_InspParaItem2.EnableWindow(bEnable);
	m_edt_InspParaItem2.EnableWindow(bEnable);

	m_cmbInspScanPos.RedrawWindow();
	m_cmbInspItemList.RedrawWindow();
	m_stt_InspJudgeSpec.RedrawWindow();
	m_edt_InspJudgeSpec.RedrawWindow();
	m_stt_InspJudgeSpecTorr.RedrawWindow();
	m_edt_InspJudgeSpecTorr.RedrawWindow();
	m_stt_InspParaItem1.RedrawWindow();
	m_edt_InspParaItem1.RedrawWindow();
	m_stt_InspParaItem2.RedrawWindow();
	m_edt_InspParaItem2.RedrawWindow();
	// end

	//KJH 2021-11-12 Trace Parameter 작업 시작
	m_cmbELBTracePara.EnableWindow(bEnable);
	m_stt_TraceImagePara.EnableWindow(bEnable);
	m_edt_TraceImagePara.EnableWindow(bEnable);
	m_stt_FindEdgePara.EnableWindow(bEnable);
	m_edt_FindEdgePara.EnableWindow(bEnable);

	m_cmbELBTracePara.RedrawWindow();
	m_stt_TraceImagePara.RedrawWindow();
	m_edt_TraceImagePara.RedrawWindow();
	m_stt_FindEdgePara.RedrawWindow();
	m_edt_FindEdgePara.RedrawWindow();

	//KJH 2021-12-01 Dust Insp Image Para 추가
	m_stt_DiffInspParaItem1.EnableWindow(bEnable);
	m_stt_DiffInspParaItem2.EnableWindow(bEnable);
	m_stt_DiffInspROIShape.EnableWindow(bEnable);
	m_edt_DiffInspParaItem1.EnableWindow(bEnable);
	m_edt_DiffInspParaItem2.EnableWindow(bEnable);
	m_btnDiffInspROIShape.EnableWindow(bEnable);

	m_stt_DiffInspParaItem1.RedrawWindow();
	m_stt_DiffInspParaItem2.RedrawWindow();
	m_stt_DiffInspROIShape.RedrawWindow();
	m_edt_DiffInspParaItem1.RedrawWindow();
	m_edt_DiffInspParaItem2.RedrawWindow();
	m_btnDiffInspROIShape.RedrawWindow();
	m_btn_InspspectionParaSet.RedrawWindow();
}
void TabModelAlgoPage::OnEnSetfocusEditAlignSpec() { setEditBox(IDC_EDIT_ALIGN_SPEC); }
void TabModelAlgoPage::OnEnSetfocusEditAlignSpecMinMax() { setEditBox(IDC_EDIT_ALIGN_SPEC_MINMAX); }
void TabModelAlgoPage::OnEnSetfocusEditRevisionOffset0() { setEditBox(IDC_EDIT_REVISION_OFFSET_X); }
void TabModelAlgoPage::OnEnSetfocusEditRevisionOffset1() { setEditBox(IDC_EDIT_REVISION_OFFSET_Y); }
void TabModelAlgoPage::OnEnSetfocusEditRevisionOffset2() { setEditBox(IDC_EDIT_REVISION_OFFSET_T); }
void TabModelAlgoPage::OnEnSetfocusEditAlignLimitX() { setEditBox(IDC_EDIT_REVISION_LIMIT_X); }
void TabModelAlgoPage::OnEnSetfocusEditAlignLimitY() { setEditBox(IDC_EDIT_REVISION_LIMIT_Y); }
void TabModelAlgoPage::OnEnSetfocusEditAlignLimitT() { setEditBox(IDC_EDIT_REVISION_LIMIT_T); }
void TabModelAlgoPage::OnEnSetfocusEditLCheckTor() { setEditBox(IDC_EDIT_L_CHECK_TOLERANCE); }
void TabModelAlgoPage::OnEnSetfocusEditLCheckSpecX() { setEditBox(IDC_EDIT_L_CHECK_SPEC); }
void TabModelAlgoPage::OnEnSetfocusEditLCheckSpecY() { setEditBox(IDC_EDIT_L_CHECK_SPEC_Y); }
void TabModelAlgoPage::OnEnSetfocusEditLCheckScale() { setEditBox(IDC_EDIT_LCHECK_SCALE); }
void TabModelAlgoPage::OnEnSetfocusEditGrabDelay() { setEditBox(IDC_EDIT_GRAB_DELAY); }
void TabModelAlgoPage::OnEnSetfocusEditDummyCornerIncludedAngle() { setEditBox(IDC_EDIT_DUMMY_INCLUDED_ANGLE); }
void TabModelAlgoPage::OnEnSetfocusEditDummyCornerIncludedAngleLimit() { setEditBox(IDC_EDIT_DUMMY_INCLUDED_ANGLE_LIMIT); }
void TabModelAlgoPage::OnEnSetfocusEditIncludedAngle() { setEditBox(IDC_EDIT_INCLUDED_ANGLE); }
void TabModelAlgoPage::OnEnSetfocusEditIncludedAngleLimit() { setEditBox(IDC_EDIT_INCLUDED_ANGLE_LIMIT); }
void TabModelAlgoPage::OnEnSetfocusEditProjectionAreaLeft0() { setEditBox(IDC_EDIT_PROJECTION_AREA_LEFT); }
void TabModelAlgoPage::OnEnSetfocusEditProjectionAreaLeft1() { setEditBox(IDC_EDIT_PROJECTION_AREA_LEFT2); }
void TabModelAlgoPage::OnEnSetfocusEditSearchAreaLeft0() { setEditBox(IDC_EDIT_SEARCH_AREA_LEFT1); }
void TabModelAlgoPage::OnEnSetfocusEditSearchAreaLeft1() { setEditBox(IDC_EDIT_SEARCH_AREA_LEFT2); }
void TabModelAlgoPage::OnEnSetfocusEditRemoveNoiseSize0() { setEditBox(IDC_EDIT_REMOVE_NOISE_SIZE1); }
void TabModelAlgoPage::OnEnSetfocusEditRemoveNoiseSize1() { setEditBox(IDC_EDIT_REMOVE_NOISE_SIZE2); }
void TabModelAlgoPage::OnEnSetfocusEditSobelThreshold0() { setEditBox(IDC_EDIT_SOBEL_THRESHOLD1); }
void TabModelAlgoPage::OnEnSetfocusEditSobelThreshold1() { setEditBox(IDC_EDIT_SOBEL_THRESHOLD2); }
void TabModelAlgoPage::OnEnSetfocusEditExistPanelGray() { setEditBox(IDC_EDIT_EXIST_PANEL_GRAY); }
void TabModelAlgoPage::OnEnSetfocusEditExistDummyGray() { setEditBox(IDC_EDIT_EXIST_DUMMY_GRAY); }
void TabModelAlgoPage::OnEnSetfocusEditInspRangeX() { setEditBox(IDC_EDIT_ELB_INSP_RANGE_X); }
void TabModelAlgoPage::OnEnSetfocusEditInspRangeY() { setEditBox(IDC_EDIT_ELB_INSP_RANGE_Y); }
void TabModelAlgoPage::OnEnSetfocusEditInspLength() { setEditBox(IDC_EDIT_ELB_INSP_LENGTH); }
void TabModelAlgoPage::OnEnSetfocusEditInspTraceInterval() { setEditBox(IDC_EDIT_ELB_TRACE_INTERVAL); }
void TabModelAlgoPage::OnEnSetHiThreshEdit() { setEditBox(IDC_EDIT_ELB_INSP_HI_THRESH); }
void TabModelAlgoPage::OnEnSetLowThreshEdit() { setEditBox(IDC_EDIT_ELB_INSP_LOW_THRESH); }
void TabModelAlgoPage::OnEnSetfocusEditInspResultCount() { setEditBox(IDC_EDIT_ELB_INSP_RESULT_C); }
void TabModelAlgoPage::OnEnSetfocusEditInspHoleC1() { setEditBox(IDC_EDIT_ELB_INSP_HOLE_C1); }
void TabModelAlgoPage::OnEnSetfocusEditInspHoleC2() { setEditBox(IDC_EDIT_ELB_INSP_HOLE_C2); }
void TabModelAlgoPage::OnEnSetfocusEditInspHoleC3() { setEditBox(IDC_EDIT_ELB_INSP_HOLE_C3); }
void TabModelAlgoPage::OnEnSetfocusEditInspDust() { setEditBox(IDC_EDIT_ELB_INSP_DUST); }
void TabModelAlgoPage::OnEnSetfocusEditInspMinSize() { setEditBox(IDC_EDIT_ELB_INSP_MINSIZE); }
void TabModelAlgoPage::OnEnSetfocusEditDistanceInspSpecLSL() { 
	int pos = m_cmbInspSpec.GetCurSel();
	setEditBox(IDC_EDIT_DISTANCE_INSP_LSL);
	CString str_temp;
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_LSL)->GetWindowTextA(str_temp);
	m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().setDistanceInspSpecLSL(pos, atof(str_temp));
}
void TabModelAlgoPage::OnEnSetfocusEditDistanceInspSpecUSL() { 
	int pos = m_cmbInspSpec.GetCurSel();
	setEditBox(IDC_EDIT_DISTANCE_INSP_USL);
	CString str_temp;
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_USL)->GetWindowTextA(str_temp);
	m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().setDistanceInspSpecUSL(pos, atof(str_temp));
}
void TabModelAlgoPage::OnEnSetfocusEditDistanceInspSubSpec() { 
	int pos = m_cmbInspSubSpec.GetCurSel();
	setEditBox(IDC_EDIT_DISTANCE_INSP_SUB_SPEC); 
	CString str_temp;
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_SUB_SPEC)->GetWindowTextA(str_temp);
	m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().setDistanceInspSubSpec(pos, atof(str_temp));
}
void TabModelAlgoPage::OnEnSetfocusEditDistanceInspSubSpecTolerance() {
	int pos = m_cmbInspSubSpec.GetCurSel();
	setEditBox(IDC_EDIT_DISTANCE_INSP_SUB_SPEC_TOLERANCE);
	CString str_temp;
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_SUB_SPEC_TOLERANCE)->GetWindowTextA(str_temp);
	m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().setDistanceInspSubSpecXTolerance(pos, atof(str_temp));
}
void TabModelAlgoPage::OnEnSetfocusEditDistanceInspHoleMetal() {
	setEditBox(IDC_EDIT_DISTANCE_INSP__HOLE_METAL);
	CString str_temp;
	GetDlgItem(IDC_EDIT_DISTANCE_INSP__HOLE_METAL)->GetWindowTextA(str_temp);
	m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().setDistanceInspHoleMetal(atof(str_temp));
}
void TabModelAlgoPage::OnEnSetfocusEditDistanceInspSpecY() { 
	int pos = m_cmbInspSpec.GetCurSel();
	setEditBox(IDC_EDIT_DISTANCE_INSP_SPEC_Y);
	CString str_temp;
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_SPEC_Y)->GetWindowTextA(str_temp);
	m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().setDistanceInspSpecY(pos, atof(str_temp));
}
void TabModelAlgoPage::OnEnSetfocusEditDistanceInspSpecYminmax() {
	int pos = m_cmbInspSpec.GetCurSel();
	setEditBox(IDC_EDIT_DISTANCE_INSP_SPEC_Y_MINMAX);
	CString str_temp;
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_SPEC_Y_MINMAX)->GetWindowTextA(str_temp);
	m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().setDistanceInspSpecYMinMax(pos, atof(str_temp));
}
void TabModelAlgoPage::setEditBox(int nID)
{
	GetDlgItem(IDC_GROUP_ALIGN_TYPE)->SetFocus(); // Focus를 다른 곳으로 이동

	CKeyPadDlg dlg;
	CString strNumber;
	GetDlgItem(nID)->GetWindowText(strNumber);
	dlg.SetValueString(false, strNumber);

	if (dlg.DoModal() != IDOK)	return;

	dlg.GetValue(strNumber);
	GetDlgItem(nID)->SetWindowTextA(strNumber);

	CModel *pModel = &m_pMain->vt_job_info[m_nJobID].model_info;
	int nCam = m_nModelSelCam;

	switch (nID)
	{
	case IDC_EDIT_PROJECTION_AREA_LEFT:
		pModel->getAlignInfo().setProjectionArea(nCam, m_nModelSelPos, 0, atoi(strNumber));			break;
	case IDC_EDIT_PROJECTION_AREA_LEFT2:
		pModel->getAlignInfo().setProjectionArea(nCam, m_nModelSelPos, 1, atoi(strNumber));			break;
	case IDC_EDIT_SEARCH_AREA_LEFT1:
		pModel->getAlignInfo().setSearchArea(nCam, m_nModelSelPos, 0, atoi(strNumber));				break;
	case IDC_EDIT_SEARCH_AREA_LEFT2:
		pModel->getAlignInfo().setSearchArea(nCam, m_nModelSelPos, 1, atoi(strNumber));				break;
	case IDC_EDIT_REMOVE_NOISE_SIZE1:
		pModel->getAlignInfo().setRemoveNoiseSize(nCam, m_nModelSelPos, 0, atoi(strNumber));		break;
	case IDC_EDIT_REMOVE_NOISE_SIZE2:
		pModel->getAlignInfo().setRemoveNoiseSize(nCam, m_nModelSelPos, 1, atoi(strNumber));		break;
	case IDC_EDIT_SOBEL_THRESHOLD1:
		pModel->getAlignInfo().setSobelTheshold(nCam, m_nModelSelPos, 0, atoi(strNumber));			break;
	case IDC_EDIT_SOBEL_THRESHOLD2:
		pModel->getAlignInfo().setSobelTheshold(nCam, m_nModelSelPos, 1, atoi(strNumber));			break;
	case IDC_EDIT_GRAB_DELAY: pModel->getAlignInfo().setGrabDelay(atoi(strNumber));                 break;
	}
}
void TabModelAlgoPage::OnCbnSelchangeCbModelSelectCam()
{

	updateAlignInfoData();
}
void TabModelAlgoPage::OnCbnSelchangeCbModelSelectPos()
{

	updateAlignInfoData();
}
void TabModelAlgoPage::updateAlignInfoData()
{
	CString strTemp;
	CModel m_tempModel = m_pMain->vt_job_info[m_nJobID].model_info;
	int nCam = m_nModelSelCam;
	int nSel = m_nModelSelPos;
	int nValue = 0;

	nValue = m_tempModel.getAlignInfo().getCornerType(nCam, nSel);
	((CComboBox*)GetDlgItem(IDC_CB_CORNER_TYPE0))->SetCurSel(nValue);

	nValue = m_tempModel.getAlignInfo().getEdgePolarity(nCam, nSel);
	((CComboBox*)GetDlgItem(IDC_CB_EDGE_POLARITY0))->SetCurSel(nValue);

	nValue = m_tempModel.getAlignInfo().getEdgeDirection(nCam, nSel);
	((CComboBox*)GetDlgItem(IDC_CB_EDGE_DIRECTION0))->SetCurSel(nValue);

	nValue = m_tempModel.getAlignInfo().getPreProcess(nCam, nSel);
	((CComboBox*)GetDlgItem(IDC_CB_PRE_PROCESS1))->SetCurSel(nValue);

	nValue = m_tempModel.getAlignInfo().getEdgeProcess(nCam, nSel);
	((CComboBox*)GetDlgItem(IDC_CB_EDGE_PROCESS1))->SetCurSel(nValue);

	strTemp.Format("%d", m_tempModel.getAlignInfo().getProjectionArea(nCam, nSel, 0));
	GetDlgItem(IDC_EDIT_PROJECTION_AREA_LEFT)->SetWindowTextA(strTemp);

	strTemp.Format("%d", m_tempModel.getAlignInfo().getProjectionArea(nCam, nSel, 1));
	GetDlgItem(IDC_EDIT_PROJECTION_AREA_LEFT2)->SetWindowTextA(strTemp);

	strTemp.Format("%d", m_tempModel.getAlignInfo().getSearchArea(nCam, nSel, 0));
	GetDlgItem(IDC_EDIT_SEARCH_AREA_LEFT1)->SetWindowTextA(strTemp);

	strTemp.Format("%d", m_tempModel.getAlignInfo().getSearchArea(nCam, nSel, 1));
	GetDlgItem(IDC_EDIT_SEARCH_AREA_LEFT2)->SetWindowTextA(strTemp);

	strTemp.Format("%d", m_tempModel.getAlignInfo().getRemoveNoiseSize(nCam, nSel, 0));
	GetDlgItem(IDC_EDIT_REMOVE_NOISE_SIZE1)->SetWindowTextA(strTemp);

	strTemp.Format("%d", m_tempModel.getAlignInfo().getRemoveNoiseSize(nCam, nSel, 1));
	GetDlgItem(IDC_EDIT_REMOVE_NOISE_SIZE2)->SetWindowTextA(strTemp);

	strTemp.Format("%d", m_tempModel.getAlignInfo().getSobelThreshold(nCam, nSel, 0));
	GetDlgItem(IDC_EDIT_SOBEL_THRESHOLD1)->SetWindowTextA(strTemp);

	strTemp.Format("%d", m_tempModel.getAlignInfo().getSobelThreshold(nCam, nSel, 1));
	GetDlgItem(IDC_EDIT_SOBEL_THRESHOLD2)->SetWindowTextA(strTemp);
}
void TabModelAlgoPage::NotUseVisible(BOOL bDisplay)
{

	return;
}
void TabModelAlgoPage::OnCbnSelchangeComboInspSpec()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int pos = m_cmbInspSpec.GetCurSel();

	CString str_temp;
	double spec = m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getDistanceInspSpecLSL(pos);
	double spec_x = m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getDistanceInspSubSpec(pos);
	double spec_y = m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getDistanceInspSpecY(pos);
	double spec_tolerance = m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getDistanceInspSpecUSL(pos);
	double spec_x_tolerance = m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(pos);
	double spec_y_tolerance = m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getDistanceInspSpecYMinMax(pos);

	str_temp.Format("%.3f", spec);
	m_edt_DistanceInspSpecLSL.SetText(str_temp);

	str_temp.Format("%.3f", spec_x);
	m_edt_DistanceInspSubSpec.SetText(str_temp);

	str_temp.Format("%.3f", spec_tolerance);
	m_edt_DistanceInspSpecUSL.SetText(str_temp);

	str_temp.Format("%.3f", spec_x_tolerance);
	m_edt_DistanceInspSubSpecTolerance.SetText(str_temp);


	str_temp.Format("%.3f", m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getDistanceInspHoleMetal());
	m_edt_DistanceInspHoleMetal.SetText(str_temp);
	
}
void TabModelAlgoPage::OnCbnSelchangeComboInspSubSpec()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int pos = m_cmbInspSubSpec.GetCurSel();

	CString str_temp;
	double spec_tilt = m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getDistanceInspSubSpec(pos);
	double spec_tilt_tolerance = m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(pos);

	str_temp.Format("%.3f", spec_tilt);
	m_edt_DistanceInspSubSpec.SetText(str_temp);

	str_temp.Format("%.3f", spec_tilt_tolerance);
	m_edt_DistanceInspSubSpecTolerance.SetText(str_temp);

	str_temp.Format("%.3f", m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getDistanceInspHoleMetal());
	m_edt_DistanceInspHoleMetal.SetText(str_temp);
}
void TabModelAlgoPage::OnCbnSelchangeComboInspJobPos()
{// dh.jung 2021-08-02 add 
	m_nInspPosition = m_cmbInspScanPos.GetCurSel();

	//m_cmbInspItemList.SetCurSel(0);
	OnCbnSelchangeComboInspItemList();
}
void TabModelAlgoPage::OnCbnSelchangeComboInspItemList()
{// dh.jung 2021-08-02 add 
	int iList = m_cmbInspItemList.GetCurSel();

	CInspSpecPara* pInspSpec = &m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo();
	CString strValue;
	
	if (iList == 1)
	{
		m_stt_InspJudgeSpec.SetText("Out Spec");
		strValue.Format("%.3f", pInspSpec->getOutSpec(m_nInspPosition));
		m_edt_InspJudgeSpec.SetText(strValue);

		m_stt_InspJudgeSpecTorr.SetText("Out Spec Torr");
		strValue.Format("%.3f", pInspSpec->getOutSpecTorr(m_nInspPosition));
		m_edt_InspJudgeSpecTorr.SetText(strValue);

		m_stt_InspParaItem1.SetText("Search Min Max Judge Offset (Pixel) - In");
		strValue.Format("%.3f", pInspSpec->getSerchMinMaxJudgeOffsetIn(m_nInspPosition));
		m_edt_InspParaItem1.SetText(strValue);

		m_stt_InspParaItem2.SetText("Search Min Max Judge Offset (Pixel) - Out");
		strValue.Format("%.3f", pInspSpec->getSerchMinMaxJudgeOffsetOut(m_nInspPosition));
		m_edt_InspParaItem2.SetText(strValue);

	}
	else if (iList == 2)
	{
		m_stt_InspJudgeSpec.SetText("Total Spec");
		strValue.Format("%.3f", pInspSpec->getTotalSpec(m_nInspPosition));
		m_edt_InspJudgeSpec.SetText(strValue);

		m_stt_InspJudgeSpecTorr.SetText("Total Spec Torr");
		strValue.Format("%.3f", pInspSpec->getTotalSpecTorr(m_nInspPosition));
		m_edt_InspJudgeSpecTorr.SetText(strValue);

		m_stt_InspParaItem1.SetText("Dust Insp - In");
		strValue.Format("%.3f", pInspSpec->getDustInspIn(m_nInspPosition));
		m_edt_InspParaItem1.SetText(strValue);

		m_stt_InspParaItem2.SetText("Dust Insp - Out");
		strValue.Format("%.3f", pInspSpec->getDustInspOut(m_nInspPosition));
		m_edt_InspParaItem2.SetText(strValue);
	}
	else if (iList == 3)
	{
		m_stt_InspJudgeSpec.SetText("Distance Spec");
		strValue.Format("%.3f", pInspSpec->getDistanceSpec(m_nInspPosition));
		m_edt_InspJudgeSpec.SetText(strValue);

		m_stt_InspJudgeSpecTorr.SetText("Distance Spec Torr");
		strValue.Format("%.3f", pInspSpec->getDistanceSpecTorr(m_nInspPosition));
		m_edt_InspJudgeSpecTorr.SetText(strValue);

		m_stt_InspParaItem1.SetText("Limit Line Offset");
		strValue.Format("%.3f", pInspSpec->getLimitLineInspOffset(m_nInspPosition));
		m_edt_InspParaItem1.SetText(strValue);

		m_stt_InspParaItem2.SetText("-");
		m_edt_InspParaItem2.SetText("0.0");
	}
	else
	{
		m_stt_InspJudgeSpec.SetText("In Spec");
		strValue.Format("%.3f", pInspSpec->getInSpec(m_nInspPosition));
		m_edt_InspJudgeSpec.SetText(strValue);

		m_stt_InspJudgeSpecTorr.SetText("In Spec Torr");
		strValue.Format("%.3f", pInspSpec->getInSpecTorr(m_nInspPosition));
		m_edt_InspJudgeSpecTorr.SetText(strValue);

		m_stt_InspParaItem1.SetText("Start Search Point (Distance)");
		strValue.Format("%.3f", pInspSpec->getStartSerchPoint(m_nInspPosition));
		m_edt_InspParaItem1.SetText(strValue);

		m_stt_InspParaItem2.SetText("End Search Point (Distance)");
		strValue.Format("%.3f", pInspSpec->getEndSerchPoint(m_nInspPosition));
		m_edt_InspParaItem2.SetText(strValue);
	}
}
void TabModelAlgoPage::OnEnSetfocusEditInspJudgeSpec()
{
	int nScanPos = m_cmbInspScanPos.GetCurSel();
	int nItemPos = m_cmbInspItemList.GetCurSel();
	setEditBox(IDC_EDIT_INSP_JUDGE_SPEC);
	CString str_temp;
	GetDlgItem(IDC_EDIT_INSP_JUDGE_SPEC)->GetWindowTextA(str_temp);

	if (nItemPos == 1)
	{
		m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().setOutSpec(nScanPos, atof(str_temp));
	}
	else if (nItemPos == 2)
	{
		m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().setTotalSpec(nScanPos, atof(str_temp));
	}
	else if (nItemPos == 3)
	{
		m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().setDistanceSpec(nScanPos, atof(str_temp));
	}
	else
	{
		m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().setInSpec(nScanPos, atof(str_temp));
	}
}
void TabModelAlgoPage::OnEnSetfocusEditInspJudgeSpecTorr()
{
	int nScanPos = m_cmbInspScanPos.GetCurSel();
	int nItemPos = m_cmbInspItemList.GetCurSel();
	setEditBox(IDC_EDIT_INSP_JUDGE_SPEC_TORR);
	CString str_temp;
	GetDlgItem(IDC_EDIT_INSP_JUDGE_SPEC_TORR)->GetWindowTextA(str_temp);

	if (nItemPos == 1)
	{
		m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().setOutSpecTorr(nScanPos, atof(str_temp));
	}
	else if (nItemPos == 2)
	{
		m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().setTotalSpecTorr(nScanPos, atof(str_temp));
	}
	else if (nItemPos == 3)
	{
		m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().setDistanceSpecTorr(nScanPos, atof(str_temp));
	}
	else
	{
		m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().setInSpecTorr(nScanPos, atof(str_temp));
	}
}
void TabModelAlgoPage::OnEnSetfocusEditInspParaItem1()
{
	int nScanPos = m_cmbInspScanPos.GetCurSel();
	int nItemPos = m_cmbInspItemList.GetCurSel();
	setEditBox(IDC_EDIT_INSP_PARA_ITEM1);
	CString str_temp;
	GetDlgItem(IDC_EDIT_INSP_PARA_ITEM1)->GetWindowTextA(str_temp);

	if (nItemPos == 1)
	{
		m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().setSerchMinMaxJudgeOffsetIn(nScanPos, atof(str_temp));
	}
	else if (nItemPos == 2)
	{
		m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().setDustInspIn(nScanPos, atof(str_temp));
	}
	else if (nItemPos == 3)
	{
//		m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().setDistanceSpecTorr(nScanPos, atof(str_temp));
	}
	else
	{
		m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().setStartSerchPoint(nScanPos, atof(str_temp));
	}
}
void TabModelAlgoPage::OnEnSetfocusEditInspParaItem2()
{
	int nScanPos = m_cmbInspScanPos.GetCurSel();
	int nItemPos = m_cmbInspItemList.GetCurSel();
	setEditBox(IDC_EDIT_INSP_PARA_ITEM2);
	CString str_temp;
	GetDlgItem(IDC_EDIT_INSP_PARA_ITEM2)->GetWindowTextA(str_temp);

	if (nItemPos == 1)
	{
		m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().setSerchMinMaxJudgeOffsetOut(nScanPos, atof(str_temp));
	}
	else if (nItemPos == 2)
	{
		m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().setDustInspOut(nScanPos, atof(str_temp));
	}
	else if (nItemPos == 3)
	{
		m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().setLimitLineInspOffset(nScanPos, atof(str_temp));
	}
	else
	{
		m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().setEndSerchPoint(nScanPos, atof(str_temp));
	}
}
void TabModelAlgoPage::OnEnSetfocusEditTraceImageThresh() 	{ setEditBox(IDC_EDIT_TRACE_IMAGE_THRESH); }
void TabModelAlgoPage::OnEnSetfocusEditFindEdgeThresh() 	{ setEditBox(IDC_EDIT_FIND_EDGE_THRESH); }
void TabModelAlgoPage::OnEnSetfocusEditDiffInspParaItem1() 	{ setEditBox(IDC_EDIT_DIFF_INSP_PARA_ITEM1); }
void TabModelAlgoPage::OnEnSetfocusEditDiffInspParaItem2() 	{ setEditBox(IDC_EDIT_DIFF_INSP_PARA_ITEM2); }

//KJH 2021-12-31 Check
void TabModelAlgoPage::OnCbnSelchangeComboInspMethod()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}

void TabModelAlgoPage::OnCbnSelchangeComboELBTraceParameter()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int m_nELBTracePara = m_cmbELBTracePara.GetCurSel();

	CInspSpecPara* pInspSpec = &m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo();
	int method = m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getMarkFindMethod();

	CString strValue;

	if (m_nELBTracePara == 0)
	{
		if (method == METHOD_LINE)
		{
			//Trace Image Thresh
			//Find Edge Thresh
			m_stt_TraceImagePara.SetText("Trace Image Thresh_L");
			strValue.Format("%d", pInspSpec->getTraceImageThresh());
			m_edt_TraceImagePara.SetText(strValue);

			m_stt_FindEdgePara.SetText("Find Edge Thresh_L");
			strValue.Format("%d", pInspSpec->getFindEdgeThresh());
			m_edt_FindEdgePara.SetText(strValue);
		}
		else if (method == METHOD_CIRCLE)
		{
			//Trace Image Thresh
			//Find Edge Thresh
			m_stt_TraceImagePara.SetText("Trace Image Thresh_C");
			strValue.Format("%d", pInspSpec->getTraceImageThresh());
			m_edt_TraceImagePara.SetText(strValue);

			m_stt_FindEdgePara.SetText("Find Edge Thresh_C");
			strValue.Format("%d", pInspSpec->getFindEdgeThresh());
			m_edt_FindEdgePara.SetText(strValue);
		}
		else if (method == METHOD_NOTCH)
		{
			//Trace Image Thresh
			//Find Edge Thresh
			m_stt_TraceImagePara.SetText("Trace Image Thresh_N");
			strValue.Format("%d", pInspSpec->getTraceImageThresh());
			m_edt_TraceImagePara.SetText(strValue);

			m_stt_FindEdgePara.SetText("Find Edge Thresh_N");
			strValue.Format("%d", pInspSpec->getFindEdgeThresh());
			m_edt_FindEdgePara.SetText(strValue);
		}
	}
	else if (m_nELBTracePara == 1)
	{
		if (method == METHOD_LINE)
		{
			//Trace Image Thresh
			//Find Edge Thresh
			m_stt_TraceImagePara.SetText("Trace Image Thresh_L_1");
			strValue.Format("%d", pInspSpec->getTraceImageThresh());
			m_edt_TraceImagePara.SetText(strValue);

			m_stt_FindEdgePara.SetText("Find Edge Thresh_L_1");
			strValue.Format("%d", pInspSpec->getFindEdgeThresh());
			m_edt_FindEdgePara.SetText(strValue);
		}
		else if (method == METHOD_CIRCLE)
		{
			//Trace Image Thresh
			//Find Edge Thresh
			m_stt_TraceImagePara.SetText("Trace Image Thresh_C_1");
			strValue.Format("%d", pInspSpec->getTraceImageThresh());
			m_edt_TraceImagePara.SetText(strValue);

			m_stt_FindEdgePara.SetText("Find Edge Thresh_C_1");
			strValue.Format("%d", pInspSpec->getFindEdgeThresh());
			m_edt_FindEdgePara.SetText(strValue);
		}
		else if (method == METHOD_NOTCH)
		{
			//Trace Image Thresh
			//Find Edge Thresh
			m_stt_TraceImagePara.SetText("Trace Image Thresh_N_1");
			strValue.Format("%d", pInspSpec->getTraceImageThresh());
			m_edt_TraceImagePara.SetText(strValue);

			m_stt_FindEdgePara.SetText("Find Edge Thresh_N_1");
			strValue.Format("%d", pInspSpec->getFindEdgeThresh());
			m_edt_FindEdgePara.SetText(strValue);
		}
	}
	else if (m_nELBTracePara == 2)
	{
		if (method == METHOD_LINE)
		{
			//Trace Image Thresh
			//Find Edge Thresh
			m_stt_TraceImagePara.SetText("Trace Image Thresh_L_2");
			strValue.Format("%d", pInspSpec->getTraceImageThresh());
			m_edt_TraceImagePara.SetText(strValue);

			m_stt_FindEdgePara.SetText("Find Edge Thresh_L_2");
			strValue.Format("%d", pInspSpec->getFindEdgeThresh());
			m_edt_FindEdgePara.SetText(strValue);
		}
		else if (method == METHOD_CIRCLE)
		{
			//Trace Image Thresh
			//Find Edge Thresh
			m_stt_TraceImagePara.SetText("Trace Image Thresh_C_2");
			strValue.Format("%d", pInspSpec->getTraceImageThresh());
			m_edt_TraceImagePara.SetText(strValue);

			m_stt_FindEdgePara.SetText("Find Edge Thresh_C_2");
			strValue.Format("%d", pInspSpec->getFindEdgeThresh());
			m_edt_FindEdgePara.SetText(strValue);
		}
		else if (method == METHOD_NOTCH)
		{
			//Trace Image Thresh
			//Find Edge Thresh
			m_stt_TraceImagePara.SetText("Trace Image Thresh_N_2");
			strValue.Format("%d", pInspSpec->getTraceImageThresh());
			m_edt_TraceImagePara.SetText(strValue);

			m_stt_FindEdgePara.SetText("Find Edge Thresh_N_2");
			strValue.Format("%d", pInspSpec->getFindEdgeThresh());
			m_edt_FindEdgePara.SetText(strValue);
		}
	}
	else if (m_nELBTracePara == 3)
	{
		if (method == METHOD_LINE)
		{
			//Trace Image Thresh
			//Find Edge Thresh
			m_stt_TraceImagePara.SetText("Trace Image Thresh_L_3");
			strValue.Format("%d", pInspSpec->getTraceImageThresh());
			m_edt_TraceImagePara.SetText(strValue);

			m_stt_FindEdgePara.SetText("Find Edge Thresh_L_3");
			strValue.Format("%d", pInspSpec->getFindEdgeThresh());
			m_edt_FindEdgePara.SetText(strValue);
		}
		else if (method == METHOD_CIRCLE)
		{
			//Trace Image Thresh
			//Find Edge Thresh
			m_stt_TraceImagePara.SetText("Trace Image Thresh_C_3");
			strValue.Format("%d", pInspSpec->getTraceImageThresh());
			m_edt_TraceImagePara.SetText(strValue);

			m_stt_FindEdgePara.SetText("Find Edge Thresh_C_3");
			strValue.Format("%d", pInspSpec->getFindEdgeThresh());
			m_edt_FindEdgePara.SetText(strValue);
		}
		else if (method == METHOD_NOTCH)
		{
			//Trace Image Thresh
			//Find Edge Thresh
			m_stt_TraceImagePara.SetText("Trace Image Thresh_N_3");
			strValue.Format("%d", pInspSpec->getTraceImageThresh());
			m_edt_TraceImagePara.SetText(strValue);

			m_stt_FindEdgePara.SetText("Find Edge Thresh_N_3");
			strValue.Format("%d", pInspSpec->getFindEdgeThresh());
			m_edt_FindEdgePara.SetText(strValue);
		}
	}
}

void TabModelAlgoPage::OnBnClickedBtnInspSetParam()
{
	if (m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_ELB_CENTER_ALIGN ||
		m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_CENTER_NOZZLE_ALIGN ||
		m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_CENTER_SIDE_YGAP ||
		m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_NOZZLE_ALIGN ||
		m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_FILM_INSP)
	{
		m_pInspectionDlg->ShowWindow(SW_SHOW);
	}
}

void TabModelAlgoPage::show_enable_parameter()
{
	int method = m_pMain->vt_job_info[m_nJobID].algo_method;

	//2022.07.01 ksm Unloading Align Angle Limit 적용
	int nConvFind 	= int(m_pMain->vt_job_info[m_nJobID].job_name.find("CONV"));
	int nUnloadFind = int(m_pMain->vt_job_info[m_nJobID].job_name.find("UNLOADING"));
	int nBuffFind 	= int(m_pMain->vt_job_info[m_nJobID].job_name.find("BUF"));

	switch (method)
	{
	case CLIENT_TYPE_ALIGN:
	case CLIENT_TYPE_1CAM_1SHOT_FILM:
	case CLIENT_TYPE_1CAM_1SHOT_ALIGN:
	case CLIENT_TYPE_1CAM_1SHOT_ALIGN_WITH_SCRATCH_INSP:
	case CLIENT_TYPE_1CAM_4POS_ROBOT:
	case CLIENT_TYPE_1CAM_2POS_REFERENCE:
	case CLIENT_TYPE_CENTER_NOZZLE_ALIGN:
	case CLIENT_TYPE_NOZZLE_SIDE_VIEW:
	{
		show_parameter_pre_insp(SW_HIDE);
		show_parameter_distance_function(SW_HIDE);
		show_parameter_distance_function_sub(SW_HIDE);
		show_parameter_ELB(SW_HIDE);

		// hsj 2022-10-17 show_parameter_ELB와 겹치는 부분이 있어서 순서 바꿈
		show_parameter_align_type(SW_SHOW);
		show_parameter_revision(SW_SHOW);
		show_parameter_align_function(SW_SHOW);
		show_parameter_length_check(SW_SHOW);

		//2022.07.01 ksm Unloading Align Angle Limit 적용
		//if (method == CLIENT_TYPE_1CAM_2POS_REFERENCE && m_pMain->vt_job_info[m_nJobID].job_name == "CONV_ALIGN")
		if (nConvFind != -1 || nUnloadFind != -1 || nBuffFind != -1)
		{
			GetDlgItem(IDC_STATIC_USE_INCLUDED_ANGLE)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_CHK_USE_INCLUDED_ANGLE)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_INCLUDED_ANGLE)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_INCLUDED_ANGLE_LIMIT)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_EDIT_INCLUDED_ANGLE)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_EDIT_INCLUDED_ANGLE_LIMIT)->ShowWindow(SW_SHOW);		
		}
	}
	break;
	case CLIENT_TYPE_FILM_INSP:
	{
		show_parameter_align_type(SW_SHOW);
		show_parameter_revision(SW_HIDE);
		show_parameter_align_function(SW_SHOW);
		show_parameter_length_check(SW_HIDE);
		show_parameter_pre_insp(SW_HIDE);
		show_parameter_distance_function(SW_HIDE);
		show_parameter_distance_function_sub(SW_SHOW);
		show_parameter_ELB(SW_HIDE);
	}
	break;
	case CLIENT_TYPE_ELB_CENTER_ALIGN:
	{
		show_parameter_align_type(SW_SHOW);
		show_parameter_revision(SW_SHOW);
		show_parameter_align_function(SW_SHOW);
		show_parameter_length_check(SW_SHOW);
		show_parameter_pre_insp(SW_SHOW);
		show_parameter_distance_function(SW_SHOW);
		show_parameter_distance_function_sub(SW_SHOW);
		show_parameter_ELB(SW_SHOW);
	}
	break;
	case CLIENT_TYPE_CENTER_SIDE_YGAP:
	{
		show_parameter_align_type(SW_SHOW);
		show_parameter_revision(SW_HIDE);
		show_parameter_align_function(SW_SHOW);
		show_parameter_length_check(SW_HIDE);
		show_parameter_pre_insp(SW_HIDE);
		show_parameter_distance_function(SW_HIDE);
		show_parameter_distance_function_sub(SW_SHOW);
		show_parameter_ELB(SW_HIDE);
	}
	break;
	default:
	{
		show_parameter_align_type(SW_HIDE);
		show_parameter_revision(SW_HIDE);
		show_parameter_align_function(SW_HIDE);
		show_parameter_length_check(SW_HIDE);
		show_parameter_pre_insp(SW_HIDE);
		show_parameter_distance_function(SW_HIDE);
		show_parameter_distance_function_sub(SW_HIDE);
		show_parameter_ELB(SW_HIDE);
	}
	break;
	}
}

void TabModelAlgoPage::show_parameter_align_type(int nShow)
{
	//AlignType
	GetDlgItem(IDC_GROUP_ALIGN_TYPE)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_ALIGN_SPEC)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_ALIGN_SPEC)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_ALIGNMENT_DIRECTION)->ShowWindow(nShow);
	GetDlgItem(IDC_COMBO_ALIGNMENT_DIR)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_ALIGN_SPEC_MINMAX)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_ALIGN_SPEC_MINMAX)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_MARK_FIND_METHOD)->ShowWindow(nShow);
	GetDlgItem(IDC_CB_MARK_FIND_METHOD)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_ANGLE_CALC_METHOD)->ShowWindow(nShow);
	GetDlgItem(IDC_COMBO_ANGLE_CALC_METHOD)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_ALIGNMENT_TYPE)->ShowWindow(nShow);
	GetDlgItem(IDC_COMBO_ALIGNMENT_TYPE)->ShowWindow(nShow);
}
void TabModelAlgoPage::show_parameter_revision(int nShow)
{
	//Revision
	GetDlgItem(IDC_GROUP_REVISION)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_REVISION_OFFSET_X)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_REVISION_OFFSET_X)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_REVISION_LIMIT_X)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_REVISION_LIMIT_X)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_REVISION_OFFSET_Y)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_REVISION_OFFSET_Y)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_REVISION_LIMIT_Y)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_REVISION_LIMIT_Y)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_REVISION_OFFSET_T)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_REVISION_OFFSET_T)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_REVISION_LIMIT_T)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_REVISION_LIMIT_T)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_ENABLE_REVISION_REVERSE_X)->ShowWindow(nShow);
	GetDlgItem(IDC_CHK_ENABLE_REVISION_REVERSE_X)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_ENABLE_REVISION_REVERSE_Y)->ShowWindow(nShow);
	GetDlgItem(IDC_CHK_ENABLE_REVISION_REVERSE_Y)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_ENABLE_REVISION_REVERSE_T)->ShowWindow(nShow);
	GetDlgItem(IDC_CHK_ENABLE_REVISION_REVERSE_T)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_ENABLE_REVISION_REVERSE_XY)->ShowWindow(nShow);
	GetDlgItem(IDC_CHK_ENABLE_REVISION_REVERSE_XY)->ShowWindow(nShow);
}
void TabModelAlgoPage::show_parameter_align_function(int nShow)
{
	//Align Function
	GetDlgItem(IDC_GRUOP_ALING_FUNCTION)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_ENABLE_ALIGN_MEASURE_SPEC_OUT_NG)->ShowWindow(nShow);

	int _findText = int(m_pMain->vt_job_info[m_nJobID].job_name.find("ATTACH_ALIGN")); // Tkyuha 20220411 별도 기능 추가
	if (m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_FILM_INSP ||
		m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_1CAM_1SHOT_FILM ||
		m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_ALIGN ||
		(m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_1CAM_1SHOT_ALIGN && _findText != -1))
	{
		GetDlgItem(IDC_CHK_ENABLE_ALIGN_MEASURE_SPEC_OUT_JUDGE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_COMBO_MANUAL_MARK_COUNT)->ShowWindow(nShow);
	}
	else
	{
		GetDlgItem(IDC_CHK_ENABLE_ALIGN_MEASURE_SPEC_OUT_JUDGE)->ShowWindow(nShow);
		GetDlgItem(IDC_COMBO_MANUAL_MARK_COUNT)->ShowWindow(SW_HIDE);
	}

	GetDlgItem(IDC_STATIC_USE_REFERENCE_MARK)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_USE_GRAB_DELAY)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_GRAB_DELAY)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_USE_FIXTURE_MARK)->ShowWindow(nShow);
	GetDlgItem(IDC_CHK_USE_FIXTURE_MARK)->ShowWindow(nShow);
	GetDlgItem(IDC_CHK_USE_REFERENCE_MARK)->ShowWindow(nShow);
	GetDlgItem(IDC_CHK_USE_CRACK_INSP)->ShowWindow(FALSE);

	GetDlgItem(IDC_STATIC_USE_CRACK_INSP)->ShowWindow(FALSE);
	GetDlgItem(IDC_STATIC_USE_INCLUDED_ANGLE)->ShowWindow(FALSE);
	GetDlgItem(IDC_CHK_USE_INCLUDED_ANGLE)->ShowWindow(FALSE);
	GetDlgItem(IDC_STATIC_INCLUDED_ANGLE)->ShowWindow(FALSE);
	GetDlgItem(IDC_STATIC_INCLUDED_ANGLE_LIMIT)->ShowWindow(FALSE);
	GetDlgItem(IDC_EDIT_INCLUDED_ANGLE)->ShowWindow(FALSE);
	GetDlgItem(IDC_EDIT_INCLUDED_ANGLE_LIMIT)->ShowWindow(FALSE);

	// hsj 2022-10-17 스크래치 체크되어있으면 파라미터 설정하는거 무조건 보이게
	//if (m_btnUseCrackInsp.GetCheck())
	if (0)
	{
		GetDlgItem(IDC_STATIC_ELB_INSP_DUST)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_ELB_INSP_DUST)->ShowWindow(TRUE);
		GetDlgItem(IDC_STATIC_ELB_INSP_HOLE_C2)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_ELB_INSP_HOLE_C2)->ShowWindow(TRUE);
		GetDlgItem(IDC_STATIC_ELB_INSP_MINSIZE)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_ELB_INSP_MINSIZE)->ShowWindow(TRUE);		
	}

}
void TabModelAlgoPage::show_parameter_length_check(int nShow)
{
	//L Check
	GetDlgItem(IDC_GRUOP_LCHECK)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_ENABLE_L_CHECK)->ShowWindow(nShow);
	GetDlgItem(IDC_CHK_ENABLE_L_CHECK)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_L_CHECK_REFERENCE)->ShowWindow(nShow);
	GetDlgItem(IDC_COMBO_LCHECK_REFERENCE)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_L_CHECK_SPEC)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_L_CHECK_SPEC)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_L_CHECK_SPEC_Y)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_L_CHECK_SPEC_Y)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_L_CHECK_TOLERANCE)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_L_CHECK_TOLERANCE)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_LCHECK_SCALE)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_LCHECK_SCALE)->ShowWindow(nShow);

	int method = m_pMain->vt_job_info[m_nJobID].algo_method;
	if (method == CLIENT_TYPE_ASSEMBLE_INSP)
	{
		GetDlgItem(IDC_STATIC_L_CHECK_SPEC)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_L_CHECK_SPEC)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_L_CHECK_SPEC_Y)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_L_CHECK_SPEC_Y)->ShowWindow(SW_SHOW);
	}

}

void TabModelAlgoPage::show_parameter_pre_insp(int nShow)
{
	//Pre Inpection Function
	GetDlgItem(IDC_STATIC_USE_INPUT_INSPECTION)->ShowWindow(nShow);
	GetDlgItem(IDC_CHK_USE_INPUT_INSP)->ShowWindow(nShow);
	GetDlgItem(IDC_EXIST_FUNCTION)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_EXIST_PANEL_GRAY)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_EXIST_PANEL_GRAY)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_EXIST_DUMMY_GRAY)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_EXIST_DUMMY_GRAY)->ShowWindow(nShow);
}

void TabModelAlgoPage::show_parameter_distance_function(int nShow)
{
	int method = m_pMain->vt_job_info[m_nJobID].algo_method;
	if (method == CLIENT_TYPE_NOZZLE_ALIGN)
	{
		GetDlgItem(IDC_DISTANCE_FUNCTION)->SetWindowTextA("PCB Bending Inspection Fuction");
		GetDlgItem(IDC_STATIC_INSP_METHOD)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_COMBO_INSP_METHOD)->ShowWindow(SW_SHOW);
	}
	else if (method == CLIENT_TYPE_ASSEMBLE_INSP)
	{
		GetDlgItem(IDC_DISTANCE_FUNCTION)->SetWindowTextA("Assemble Inspection Fuction");
		GetDlgItem(IDC_STATIC_INSP_METHOD)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_COMBO_INSP_METHOD)->ShowWindow(SW_SHOW);
	}

	// Distance Inspection Function
	GetDlgItem(IDC_DISTANCE_FUNCTION)->ShowWindow(nShow);
	GetDlgItem(IDC_COMBO_INSP_SPEC)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_INSP_METHOD)->ShowWindow(nShow);
	GetDlgItem(IDC_COMBO_INSP_METHOD)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_DISTANCE_INSP_LSL)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_LSL)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_DISTANCE_INSP_USL)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_USL)->ShowWindow(nShow);
}

void TabModelAlgoPage::show_parameter_distance_function_sub(int nShow)
{
	int method = m_pMain->vt_job_info[m_nJobID].algo_method;
	if (method == CLIENT_TYPE_NOZZLE_ALIGN)
		GetDlgItem(IDC_DISTANCE_FUNCTION)->SetWindowTextA("Wet Out Distance");
	else if(method == CLIENT_TYPE_FILM_INSP)
		GetDlgItem(IDC_DISTANCE_FUNCTION)->SetWindowTextA("Flim Distance");
	
	// Distance Inpection Sub Fuction
	GetDlgItem(IDC_DISTANCE_SUB_FUNCTION)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_DISTANCE_INSP_HOLE_METAL)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_DISTANCE_INSP__HOLE_METAL)->ShowWindow(nShow);
	GetDlgItem(IDC_BTN_INSP_SET_PARAM)->ShowWindow(nShow);
	GetDlgItem(IDC_COMBO_INSP_SUB_SPEC)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_DISTANCE_INSP_SUB_SPEC)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_SUB_SPEC)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_DISTANCE_INSP_SUB_SPEC_TOLERANCE)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_SUB_SPEC_TOLERANCE)->ShowWindow(nShow);
}

void TabModelAlgoPage::show_parameter_ELB(int nShow)
{
	GetDlgItem(IDC_STATIC_ELB_INSP_RESULT_C					  )->ShowWindow(nShow);
	GetDlgItem(	IDC_EDIT_ELB_INSP_RESULT_C					  )->ShowWindow(nShow);
	// Hole Size
	GetDlgItem(IDC_STATIC_ELB_INSP_HOLE_C1					  )->ShowWindow(nShow);
	GetDlgItem(	IDC_EDIT_ELB_INSP_HOLE_C1					  )->ShowWindow(nShow);
	GetDlgItem(	IDC_STATIC_ELB_INSP_HOLE_C2					  )->ShowWindow(nShow);
	GetDlgItem(	IDC_EDIT_ELB_INSP_HOLE_C2					  )->ShowWindow(nShow);
	GetDlgItem(	IDC_STATIC_ELB_INSP_HOLE_C3					  )->ShowWindow(nShow);
	GetDlgItem(	IDC_EDIT_ELB_INSP_HOLE_C3					  )->ShowWindow(nShow);

	GetDlgItem(	IDC_STATIC_USE_BM_BASE						  )->ShowWindow(nShow);
	GetDlgItem(	IDC_CHK_USE_BM_BASE							  )->ShowWindow(nShow);
	GetDlgItem(	IDC_STATIC_ELB_INSP_DUST					  )->ShowWindow(nShow);
	GetDlgItem(	IDC_EDIT_ELB_INSP_DUST						  )->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_ELB_INSP_MINSIZE                    )->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_ELB_INSP_MINSIZE                      )->ShowWindow(nShow);

	GetDlgItem(IDC_STATIC_USE_DUMMY_CORNER_INSP				 )->ShowWindow(nShow);
	GetDlgItem(IDC_CHK_USE_DUMMY_CORNER_INSP				 )->ShowWindow(nShow);

	GetDlgItem(IDC_STATIC_DUST_INSP_ROI_SHAPE				 )->ShowWindow(nShow);
	GetDlgItem(IDC_CHK_USE_DUST_INSP_ROI_SHAPE				 )->ShowWindow(nShow);

	// ELB Trace Insp Parameter
	GetDlgItem(IDC_GB_TRACE_IMAGE_PARAMETER)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_FIND_EDGE_THRESH)->ShowWindow(nShow);
	GetDlgItem(IDC_GB_ELB_TRACE_PARAMETER					)->ShowWindow(nShow);
	GetDlgItem(IDC_COMBO_ELB_TRACE_PARAMETER				)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_TRACE_IMAGE_THRESH					)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_USE_METAL_TRACE_FIRST)->ShowWindow(nShow);
	GetDlgItem(IDC_CHK_USE_METAL_TRACE_FIRST)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_FIND_EDGE_THRESH					)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_TRACE_IMAGE_THRESH)->ShowWindow(nShow);

	GetDlgItem(IDC_GB_DIFF_INSPECTION_PARA				   )->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_DIFF_INSP_PARA_ITEM1			   )->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_DIFF_INSP_PARA_ITEM1			   )->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_DIFF_INSP_PARA_ITEM2			   )->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_DIFF_INSP_PARA_ITEM2			   )->ShowWindow(nShow);

	// Inspection Parameter & Spec
	GetDlgItem(IDC_GB_INSPECTION_SPEC_PARAMETER			 )->ShowWindow(nShow);
	GetDlgItem(IDC_COMBO_INSP_SCAN_POS					 )->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_DASH							 )->ShowWindow(nShow);
	GetDlgItem(IDC_COMBO_INSP_ITEM_LIST					 )->ShowWindow(nShow);

	GetDlgItem(IDC_GB_INSPECTION_SPEC					   )->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_INSP_JUDGE_SPEC				   )->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_INSP_JUDGE_SPEC					   )->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_INSP_JUDGE_SPEC_TORR			   )->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_INSP_JUDGE_SPEC_TORR			   )->ShowWindow(nShow);
	GetDlgItem(IDC_GB_INSPECTION_PARA					   )->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_INSP_PARA_ITEM1				   )->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_INSP_PARA_ITEM1					   )->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_INSP_PARA_ITEM2				   )->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_INSP_PARA_ITEM2					   )->ShowWindow(nShow);

	//ELB Insepction Function
	GetDlgItem(IDC_ELB_INSPECTION										)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_USE_AUTOSAVE_AVI								)->ShowWindow(nShow);
	GetDlgItem(IDC_CHK_USE_AUTOSAVE_AVI									)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_SELECT_MTON_DISTANCE_ALGORITHM				)->ShowWindow(nShow);
	GetDlgItem(IDC_CHK_USE_SUB_INSP_ALGORITHM							)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_USE_REVERSE_ORDER								)->ShowWindow(nShow);
	GetDlgItem(IDC_CHK_USE_REVERSE_ORDER								)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_USE_CYCLE_TRACE								)->ShowWindow(nShow);
	GetDlgItem(IDC_CHK_USE_CYCLE_TRACE									)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_USE_DUST_INSPECTION							)->ShowWindow(nShow);
	GetDlgItem(IDC_CHK_USE_DUST_INSP									)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_USE_POST_INSPECTION							)->ShowWindow(nShow);
	GetDlgItem(IDC_CHK_USE_POST_INSP									)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_ELB_TRACE_INTERVAL							)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_ELB_TRACE_INTERVAL								)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_USE_REALTIME_TRACE							)->ShowWindow(nShow);
	GetDlgItem(IDC_CHK_USE_REALTIME_TRACE								)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_EDGE_POLARITY									)->ShowWindow(nShow);
	GetDlgItem(IDC_COMBO_EDGE_POLARITY									)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_SEARCH_DIR									)->ShowWindow(nShow);
	GetDlgItem(IDC_COMBO_SEARCH_DIR										)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_ELB_INSP_HI_THRESH							)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_ELB_INSP_HI_THRESH								)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_ELB_INSP_LOW_TH								)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_ELB_INSP_LOW_THRESH								)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_ELB_INSP_RANGE								)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_ELB_INSP_RANGE_X								)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_ELB_INSP_RANGE_Y								)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_ELB_INSP_DISTANCE								)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_ELB_INSP_LENGTH									)->ShowWindow(nShow);
}

void TabModelAlgoPage::getSaveParam()
{

	BOOL bWetoutEnable = m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().getUseDiffInsp();										//true					//wetout 검사
	BOOL bDustEnable = m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().getUseInputInspection();									//false					//이물 검사
	BOOL bLiquidDropsEnable = m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().getUseDustInsp();
	BOOL bCircle_ShapeDispensingModeEnable = m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getUseDummyCornerInsp(); //22.06.02

	int inputThresh = m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().getExistDummyGray();
	int iLiquidDropsThresh = m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().getInspDustThresh();

	// hsj 2022-10-17 scratch thresh 저장되게 수정
	int method = m_pMain->vt_job_info[m_nJobID].algo_method;

	if (method == CLIENT_TYPE_1CAM_1SHOT_ALIGN&& m_btnUseCrackInsp.GetCheck())
	{
		iLiquidDropsThresh = m_pMain->sUIData.iLiquidDropsThresh;
	}

	int hthresh = m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().getInspectionHighThresh();
	int lthresh = m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().getInspectionLowThresh();

	m_btnEnableDustCheck.SetCheck(bLiquidDropsEnable);
	m_btnEnableDiffCheck.SetCheck(bWetoutEnable);
	m_btnEnableInputInspection.SetCheck(bDustEnable);
	m_btnUseDummyCornerInsp.SetCheck(bCircle_ShapeDispensingModeEnable); //22.06.02

	m_edt_ExistDummyGray.SetTextInt(inputThresh);
	m_edt_InspDustThresh.SetTextInt(iLiquidDropsThresh);
	m_edt_Insp_Hi_Thresh.SetTextInt(hthresh);
	m_edt_Insp_Low_Thresh.SetTextInt(lthresh);

	m_btnEnableDustCheck.RedrawWindow();
	m_btnEnableDiffCheck.RedrawWindow();
	m_btnEnableInputInspection.RedrawWindow();
	m_btnUseDummyCornerInsp.RedrawWindow(); //22.06.02
}

void TabModelAlgoPage::getUIValue() //lhj add 220503
{
	m_pMain->sUIData.bLiquidDropsEnable = m_btnEnableDustCheck.GetCheck();
	m_pMain->sUIData.bWetoutEnable = m_btnEnableDiffCheck.GetCheck();
	m_pMain->sUIData.bDustEnable = m_btnEnableInputInspection.GetCheck();
	m_pMain->sUIData.bCircleShapeModeEnable = m_btnUseDummyCornerInsp.GetCheck(); //22.06.02

	m_pMain->sUIData.inputThresh = m_edt_ExistDummyGray.GetTextInt();
	m_pMain->sUIData.iLiquidDropsThresh = m_edt_InspDustThresh.GetTextInt();
	m_pMain->sUIData.hthresh = m_edt_Insp_Hi_Thresh.GetTextInt();
	m_pMain->sUIData.lthresh = m_edt_Insp_Low_Thresh.GetTextInt();
	m_pMain->sUIData.dCircleLength = atof(m_edt_InspRangeLength.GetText());
	m_pMain->sUIData.dCenterMetalDistanceSpec = atof(m_edt_DistanceInspHoleMetal.GetText());

	//m_btnEnableDustCheck.RedrawWindow();
	//m_btnEnableDiffCheck.RedrawWindow();
	//m_btnEnableInputInspection.RedrawWindow();
}

void TabModelAlgoPage::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	if (IsDoubleClickLCheckUse(point) && m_bModify)
	{
		if (m_pMain->fnSetMessage(2, "Calc L Check Data?") != TRUE) return;

		
		double lcheckspec_X = atof(m_edt_LCheckSpecX.GetText());
		double dblcheck = m_pMain->m_dLcheckCurrentValue[m_nJobID];

		double lcheck_scale = dblcheck != 0 ? (lcheckspec_X / dblcheck) : 1;

		m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().setLCheckScale(lcheck_scale);

		m_edt_LCheckScale.SetTextDouble(lcheck_scale);
	}

	CDialogEx::OnLButtonDblClk(nFlags, point);
}

BOOL TabModelAlgoPage::IsDoubleClickLCheckUse(CPoint pt)
{
	if (PtInRect(&m_rtLCheckUse, pt))	return TRUE;
	else								return FALSE;
}


void TabModelAlgoPage::OnBnClickedChkUseCrackInsp()
{
	return;

	BOOL nShow = SW_SHOW;

	if(!m_btnUseCrackInsp.GetCheck()) nShow = SW_HIDE;

	GetDlgItem(IDC_STATIC_ELB_INSP_DUST)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_ELB_INSP_DUST)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_ELB_INSP_HOLE_C2)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_ELB_INSP_HOLE_C2)->ShowWindow(nShow);

	GetDlgItem(IDC_STATIC_ELB_INSP_MINSIZE)->ShowWindow(nShow);
	GetDlgItem(IDC_EDIT_ELB_INSP_MINSIZE)->ShowWindow(nShow);
}
