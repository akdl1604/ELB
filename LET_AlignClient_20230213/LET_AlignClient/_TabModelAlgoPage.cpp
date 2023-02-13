// TabModelAlgoPage.cpp: 구현 파일
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "TabModelAlgoPage.h"
#include "afxdialogex.h"

// TabModelAlgoPage 대화 상자

IMPLEMENT_DYNAMIC(TabModelAlgoPage, CDialogEx)

TabModelAlgoPage::TabModelAlgoPage(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TAB_MODEL_ALGORITHM, pParent)
{
	m_nJobID = 0;
	m_nModelSelPos = 0;
	m_nModelSelCam = 0;
}

TabModelAlgoPage::~TabModelAlgoPage()
{
}

void TabModelAlgoPage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_STATIC_ALIGN_SPEC, m_stt_AlignSpec);
	DDX_Control(pDX, IDC_STATIC_ALIGN_SPEC_MINMAX, m_stt_AlignSpecMinMax);
	DDX_Control(pDX, IDC_STATIC_ALIGNMENT_TARGET, m_stt_AlignTarget);
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
	
	DDX_Control(pDX, IDC_STATIC_CAMERA_DISTANCE, m_stt_CameraDistance);
	DDX_Control(pDX, IDC_STATIC_USE_INCLUDED_ANGLE, m_stt_UseIncludedAngle);
	DDX_Control(pDX, IDC_STATIC_INCLUDED_ANGLE, m_stt_IncludedAngle);
	DDX_Control(pDX, IDC_STATIC_INCLUDED_ANGLE_LIMIT, m_stt_IncludedAngleLimit);
	DDX_Control(pDX, IDC_STATIC_USE_CRACK_INSP, m_stt_CrackInsp);
	DDX_Control(pDX, IDC_STATIC_USE_DUMMY_CORNER_INSP, m_stt_UseDummyCornerIncludedAngle);
	DDX_Control(pDX, IDC_STATIC_DUMMY_CORNER_FILTER, m_stt_DummyCornerFilter);
	DDX_Control(pDX, IDC_STATIC_DUMMY_CORNER_INSP_CAM, m_stt_DummyCornerInspCam);
	DDX_Control(pDX, IDC_STATIC_DUMMY_INCLUDED_ANGLE, m_stt_DummyCornerIncludedAngle);
	DDX_Control(pDX, IDC_STATIC_DUMMY_INCLUDED_ANGLE_LIMIT, m_stt_DummyCornerIncludedAngleLimit);
	DDX_Control(pDX, IDC_STATIC_MULTIPLEX_ROI, m_stt_UseMultiplexRoi);
	DDX_Control(pDX, IDC_STATIC_USE_BOX_INSPECTION, m_stt_UseBoxInspection);
	DDX_Control(pDX, IDC_STATIC_USE_DUMMY_SEPAR_INSPECTION, m_stt_UseDummySeparInspection);
	DDX_Control(pDX, IDC_STATIC_MODEL_SELECT_CAM, m_lbModelSelCam);
	DDX_Control(pDX, IDC_STATIC_MODEL_SELECT_POS, m_lbModelSelPos);
	DDX_Control(pDX, IDC_STATIC_CORNER_TYPE0, m_lblEdgeQuadrantCam0);
	DDX_Control(pDX, IDC_STATIC_EDGE_POLARITY0, m_lblEdgeFindDirCam0);
	DDX_Control(pDX, IDC_STATIC_EDGE_DIRECTION0, m_lblEdgeDirectionCam0);
	DDX_Control(pDX, IDC_STATIC_PRE_PROCESS1, m_lblPreProcessCam0);
	DDX_Control(pDX, IDC_STATIC_EDGE_PROCESS1, m_lblEdgeProcessCam0);
	DDX_Control(pDX, IDC_STATIC_PROJECTION_AREA_L1, m_stt_ProjectionAreaLeft[0]);
	DDX_Control(pDX, IDC_STATIC_PROJECTION_AREA_L2, m_stt_ProjectionAreaLeft[1]);
	DDX_Control(pDX, IDC_STATIC_SEARCH_AREA_L1, m_stt_SearchAreaLeft[0]);
	DDX_Control(pDX, IDC_STATIC_SEARCH_AREA_L2, m_stt_SearchAreaLeft[1]);
	DDX_Control(pDX, IDC_STATIC_REMOVE_NOISE_SIZE1, m_stt_RemoveNoiseSize[0]);
	DDX_Control(pDX, IDC_STATIC_REMOVE_NOISE_SIZE2, m_stt_RemoveNoiseSize[1]);
	DDX_Control(pDX, IDC_STATIC_SOBEL_THRESHOLD1, m_stt_SobelThreshold[0]);
	DDX_Control(pDX, IDC_STATIC_SOBEL_THRESHOLD2, m_stt_SobelThreshold[1]);
	//210110
	DDX_Control(pDX, IDC_STATIC_USE_REFERENCE_MARK, m_stt_UseReferenceMark);
	DDX_Control(pDX, IDC_STATIC_EXIST_PANEL_GRAY, m_stt_ExistPanelGray);
	DDX_Control(pDX, IDC_STATIC_EXIST_DUMMY_GRAY, m_stt_ExistDummyGray);
	DDX_Control(pDX, IDC_STATIC_ENABLE_REVISION_REVERSE_X, m_stt_EnableRevisionReverseX);
	DDX_Control(pDX, IDC_STATIC_ENABLE_REVISION_REVERSE_Y, m_stt_EnableRevisionReverseY);
	DDX_Control(pDX, IDC_STATIC_ENABLE_REVISION_REVERSE_T, m_stt_EnableRevisionReverseT);
	DDX_Control(pDX, IDC_STATIC_ENABLE_REVISION_REVERSE_XY, m_stt_EnableRevisionReverseXY);
	
	//Distance Inspection
	DDX_Control(pDX, IDC_STATIC_INSP_METHOD, m_stt_InspectionMethod);
	DDX_Control(pDX, IDC_STATIC_INSP_SUB_METHOD, m_stt_InspectionSubMethod);
	DDX_Control(pDX, IDC_STATIC_INSP_SUB_INTERVAL, m_stt_InspectionSubInterval);
	DDX_Control(pDX, IDC_STATIC_INSP_SUB_INTERVAL_TOL, m_stt_InspectionIntervalTolerance);
	DDX_Control(pDX, IDC_STATIC_INSP_SUB_DIRECTION, m_stt_InspectionSubDir);
	DDX_Control(pDX, IDC_STATIC_DISTANCE_INSP_LSL, m_stt_DistanceInspLSL);
	DDX_Control(pDX, IDC_STATIC_DISTANCE_INSP_USL, m_stt_DistanceInspUSL);
	DDX_Control(pDX, IDC_STATIC_DISTANCE_INSP_SUB_SPEC, m_stt_DistanceInspSubSpec);
	DDX_Control(pDX, IDC_STATIC_DISTANCE_INSP_SUB_SPEC_TOLERANCE, m_stt_DistanceInspSubSpecTolerance);
	DDX_Control(pDX, IDC_STATIC_DISTANCE_INSP_SPEC_Y, m_stt_DistanceInspSpecY);
	DDX_Control(pDX, IDC_STATIC_DISTANCE_INSP_SPEC_Y_MINMAX, m_stt_DistanceInspSpecYminmax);


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
	DDX_Control(pDX, IDC_EDIT_L_CHECK_SPEC_Y, m_edt_LCheckSpecY);
	
	DDX_Control(pDX, IDC_EDIT_CAMERA_DISTANCE, m_edt_CameraDistance);
	DDX_Control(pDX, IDC_EDIT_DUMMY_INCLUDED_ANGLE, m_edt_DummyCornerIncludedAngle);
	DDX_Control(pDX, IDC_EDIT_DUMMY_INCLUDED_ANGLE_LIMIT, m_edt_DummyCornerIncludedAngleLimit);
	DDX_Control(pDX, IDC_EDIT_INCLUDED_ANGLE, m_edt_IncludedAngle);
	DDX_Control(pDX, IDC_EDIT_INCLUDED_ANGLE_LIMIT, m_edt_IncludedAngleLimit);
	DDX_Control(pDX, IDC_EDIT_PROJECTION_AREA_LEFT, m_edt_ProjectionAreaLeft[0]);
	DDX_Control(pDX, IDC_EDIT_PROJECTION_AREA_LEFT2, m_edt_ProjectionAreaLeft[1]);
	DDX_Control(pDX, IDC_EDIT_SEARCH_AREA_LEFT1, m_edt_SearchAreaLeft[0]);
	DDX_Control(pDX, IDC_EDIT_SEARCH_AREA_LEFT2, m_edt_SearchAreaLeft[1]);
	DDX_Control(pDX, IDC_EDIT_REMOVE_NOISE_SIZE1, m_edt_RemoveNoiseSize[0]);
	DDX_Control(pDX, IDC_EDIT_REMOVE_NOISE_SIZE2, m_edt_RemoveNoiseSize[1]);
	DDX_Control(pDX, IDC_EDIT_SOBEL_THRESHOLD1, m_edt_SobelThreshold[0]);
	DDX_Control(pDX, IDC_EDIT_SOBEL_THRESHOLD2, m_edt_SobelThreshold[1]);

	DDX_Control(pDX, IDC_COMBO_ALIGNMENT_DIR, m_cmbAlignDir);
	DDX_Control(pDX, IDC_CB_MARK_FIND_METHOD, m_cmbMarkFindMethod);
	DDX_Control(pDX, IDC_COMBO_ALIGNMENT_TARGET, m_cmbAlignTarget);
	DDX_Control(pDX, IDC_COMBO_ALIGNMENT_TYPE, m_selAlignType);
	DDX_Control(pDX, IDC_COMBO_ANGLE_CALC_METHOD, m_cmbAngleCalcMethod);
	
	DDX_Control(pDX, IDC_COMBO_DUMMY_CORNER_FILTER, m_selCornerFilter);
	DDX_Control(pDX, IDC_COMBO_DUMMY_CORNER_INSP_CAM, m_selCornerInspCam);
	DDX_Control(pDX, IDC_COMBO_INSP_METHOD, m_selInspectionMethod);
	DDX_Control(pDX, IDC_COMBO_INSP_SUB_METHOD, m_selInspectionSubMethod);
	DDX_Control(pDX, IDC_COMBO_INSP_SUB_DIRECTION, m_selInspectionSubDir);

	DDX_Control(pDX, IDC_COMBO_LCHECK_REFERENCE, m_cmbLCheckRef);
	DDX_Control(pDX, IDC_COMBO_INSP_SPEC, m_cmbInspSpec);
	DDX_Control(pDX, IDC_COMBO_INSP_SUB_SPEC, m_cmbInspSubSpec);
	DDX_Control(pDX, IDC_COMBO_EDGE_POLARITY, m_cmbSearchEdgePolarity);	
	DDX_Control(pDX, IDC_COMBO_SEARCH_DIR, m_cmbSearchDir);
	
	DDX_Control(pDX, IDC_CB_MODEL_SELECT_CAM, m_cbModelSelCam);
	DDX_Control(pDX, IDC_CB_MODEL_SELECT_POS, m_cbModelSelPos);
	DDX_Control(pDX, IDC_CB_CORNER_TYPE0, m_cmbCornerType[0]);
	DDX_Control(pDX, IDC_CB_EDGE_POLARITY0, m_cmbEdgePolarity[0]);
	DDX_Control(pDX, IDC_CB_EDGE_DIRECTION0, m_cmbEdgeDirection[0]);
	DDX_Control(pDX, IDC_CB_PRE_PROCESS1, m_cmbPreProcess[0]);
	DDX_Control(pDX, IDC_CB_EDGE_PROCESS1, m_cmbEdgeProcess[0]);

	DDX_Control(pDX, IDC_CHK_ENABLE_ALIGN_MEASURE_SPEC_OUT_JUDGE, m_btnEnableAlignMeasureSpecOutJudge);
	DDX_Control(pDX, IDC_CHK_ENABLE_L_CHECK, m_btnEnableLCheck);
	DDX_Control(pDX, IDC_CHK_USE_CRACK_INSP, m_btnUseCrackInsp);
	DDX_Control(pDX, IDC_CHK_USE_DUMMY_CORNER_INSP, m_btnUseDummyCornerInsp);
	DDX_Control(pDX, IDC_CHK_USE_INCLUDED_ANGLE, m_btnUseIncludedAngle);
	DDX_Control(pDX, IDC_CHK_USE_MULTIPLEX_ROI, m_btnUseMultiplexRoi);
	DDX_Control(pDX, IDC_CHK_USE_BOX_INSP, m_btnUseBoxInsp);
	DDX_Control(pDX, IDC_CHK_USE_DUMMY_SEPAR_INSP, m_btnUseDummySeparInsp);
	DDX_Control(pDX, IDC_CHK_USE_DUST_INSP, m_btnEnableDustCheck);
	DDX_Control(pDX, IDC_CHK_USE_POST_INSP, m_btnEnableDiffCheck);
	DDX_Control(pDX, IDC_CHK_USE_REALTIME_TRACE, m_btnEnableRealTimeTrace);
	DDX_Control(pDX, IDC_CHK_USE_FIX_TRACE, m_btnEnableFixTrace);

	

	DDX_Control(pDX, IDC_GROUP_ALIGN_TYPE, m_grpAlignType);
	DDX_Control(pDX, IDC_GROUP_REVISION, m_grpRevision);
	DDX_Control(pDX, IDC_GRUOP_ALING_FUNCTION, m_grpAlignFunction);
	DDX_Control(pDX, IDC_GRUOP_LCHECK, m_grpLCheck);
	
	DDX_Control(pDX, IDC_GROUP_ALIGN_RANSAC, m_grpAlignRansac);
	DDX_Control(pDX, IDC_EXIST_FUNCTION, m_grpExistFunction);
	DDX_Control(pDX, IDC_ELB_INSPECTION, m_grpElbInspFunction);
	DDX_Control(pDX, IDC_DISTANCE_FUNCTION, m_grpDistanceFunction);
	DDX_Control(pDX, IDC_DISTANCE_SUB_FUNCTION, m_grpDistanceSubFunction);

	//210110
	DDX_Control(pDX, IDC_CHK_USE_REFERENCE_MARK, m_btnUseReferenceMark);
	DDX_Control(pDX, IDC_EDIT_EXIST_PANEL_GRAY, m_edt_ExistPanelGray);
	DDX_Control(pDX, IDC_EDIT_EXIST_DUMMY_GRAY, m_edt_ExistDummyGray);

	DDX_Control(pDX, IDC_EDIT_ELB_INSP_RANGE_X, m_edt_InspRangeX);
	DDX_Control(pDX, IDC_EDIT_ELB_INSP_RANGE_Y, m_edt_InspRangeY);
	DDX_Control(pDX, IDC_EDIT_ELB_INSP_LENGTH, m_edt_InspRangeLength);
	DDX_Control(pDX, IDC_EDIT_ELB_TRACE_INTERVAL,m_edt_InspTraceInterval);

	DDX_Control(pDX, IDC_CHK_ENABLE_REVISION_REVERSE_X, m_btnEnableRevisionReverseX);
	DDX_Control(pDX, IDC_CHK_ENABLE_REVISION_REVERSE_Y, m_btnEnableRevisionReverseY);
	DDX_Control(pDX, IDC_CHK_ENABLE_REVISION_REVERSE_T, m_btnEnableRevisionReverseT);
	DDX_Control(pDX, IDC_CHK_ENABLE_REVISION_REVERSE_XY, m_btnEnableRevisionReverseXY);

	//DistanceInspection
	DDX_Control(pDX, IDC_EDIT_DISTANCE_INSP_LSL, m_edt_DistanceInspSpecLSL);
	DDX_Control(pDX, IDC_EDIT_DISTANCE_INSP_USL, m_edt_DistanceInspSpecUSL);
	DDX_Control(pDX, IDC_EDIT_DISTANCE_INSP_SUB_SPEC, m_edt_DistanceInspSubSpec);
	DDX_Control(pDX, IDC_EDIT_DISTANCE_INSP_SUB_SPEC_TOLERANCE, m_edt_DistanceInspSubSpecTolerance);
	DDX_Control(pDX, IDC_EDIT_DISTANCE_INSP_SPEC_Y, m_edt_DistanceInspSpecY);
	DDX_Control(pDX, IDC_EDIT_DISTANCE_INSP_SPEC_Y_MINMAX, m_edt_DistanceInspSpecYminmax);
	DDX_Control(pDX, IDC_EDIT_INSP_SUB_INTERVAL, m_edt_DistanceInspSubInterval);
	DDX_Control(pDX, IDC_EDIT_INSP_SUB_INTERVAL_TOL, m_edt_DistanceInspSubIntervalTol);
	//210110
	DDX_Control(pDX, IDC_STATIC_ELB_INSP_RANGE, m_stt_ELB_InspRange);
	DDX_Control(pDX, IDC_STATIC_ELB_INSP_DISTANCE, m_stt_ELB_Distance);	
	DDX_Control(pDX, IDC_STATIC_USE_DUST_INSPECTION, m_stt_ELB_DustInsp);
	DDX_Control(pDX, IDC_STATIC_USE_POST_INSPECTION, m_stt_ELB_DiffInsp);
	DDX_Control(pDX, IDC_STATIC_ELB_TRACE_INTERVAL, m_stt_ELB_TraceInterval);
	DDX_Control(pDX, IDC_STATIC_USE_REALTIME_TRACE, m_stt_ELB_RealTimeTrace);
	DDX_Control(pDX, IDC_STATIC_USE_FIX_TRACE, m_stt_ELB_FixTrace);
	
	DDX_Control(pDX, IDC_STATIC_EDGE_POLARITY, m_stt_ELB_EdgePolarity);
	DDX_Control(pDX, IDC_STATIC_SEARCH_DIR, m_stt_ELB_SearchDir);
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
	ON_EN_SETFOCUS(IDC_EDIT_CAMERA_DISTANCE, &TabModelAlgoPage::OnEnSetfocusEditCameraDistance)
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
	ON_EN_SETFOCUS(IDC_EDIT_INSP_SUB_INTERVAL, &TabModelAlgoPage::OnEnSetfocusEditDistanceInspSubSpecX)
	ON_EN_SETFOCUS(IDC_EDIT_INSP_SUB_INTERVAL_TOL, &TabModelAlgoPage::OnEnSetfocusEditDistanceInspSubTolerance)
	ON_CBN_SELCHANGE(IDC_CB_MODEL_SELECT_CAM, &TabModelAlgoPage::OnCbnSelchangeCbModelSelectCam)
	ON_CBN_SELCHANGE(IDC_CB_MODEL_SELECT_POS, &TabModelAlgoPage::OnCbnSelchangeCbModelSelectPos)
	ON_CBN_SELCHANGE(IDC_COMBO_INSP_SPEC, &TabModelAlgoPage::OnCbnSelchangeComboInspSpec)
	ON_CBN_SELCHANGE(IDC_COMBO_INSP_SUB_SPEC, &TabModelAlgoPage::OnCbnSelchangeComboInspSubSpec)
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


void TabModelAlgoPage::MainButtonInit(CButtonEx *pbutton, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
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
	InitTitle(&m_stt_CameraDistance, "Camera Distance (mm)", 14.f, COLOR_BTN_BODY);
	//InitTitle(&m_stt_CameraDistance, "L Check Length(mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_CrackInsp, "Use UTG Corner Crack Inspection", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_AlignTarget, "Alignment Target", 14.f, COLOR_BTN_BODY);
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
	InitTitle(&m_stt_DummyCornerIncludedAngle, "Dummy Expected Angle (˚)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_DummyCornerIncludedAngleLimit, "Dummy Angle Tolerance (˚)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_DummyCornerInspCam, "Dummy Corner Inspection Camera", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_DummyCornerFilter, "Dummy Corner Filter", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_UseMultiplexRoi, "Use Multplex ROI", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_UseBoxInspection, "Use Box Inspection", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_UseDummySeparInspection, "Use Dummy Separ Inspection", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_RemoveNoiseSize[0], "Noise Size - Hori", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_RemoveNoiseSize[1], "Noise Size - Vert", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_SobelThreshold[0], "Threshold - Hori", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_SobelThreshold[1], "Threshold - Vert", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_SearchAreaLeft[0], "Search Area - Hori", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_SearchAreaLeft[1], "Search Area - Vert", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ProjectionAreaLeft[0], "Projection Area - Hori", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ProjectionAreaLeft[1], "Projection Area - Vert", 14.f, COLOR_BTN_BODY);	
	InitTitle(&m_lblAlignDir, "Alignment Direction", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_lblEdgeQuadrantCam0, "Corner Type", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_lblEdgeFindDirCam0, "Edge Polarity", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_lblEdgeDirectionCam0, "Search Direction", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_lblPreProcessCam0, "Pre Process", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_lblEdgeProcessCam0, "Edge Process", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_lbModelSelCam, "Select Camera", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_lbModelSelPos, "Select Position", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ExistPanelGray, "Panel Gray(<)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_InspRange, "Inspection Range(X,Y)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_Distance, "Inspection Length", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_DustInsp, "Dust Inspction", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_DiffInsp, "Diff Inspction", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_RealTimeTrace, "Real Time Trace", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_FixTrace, "Use Fix Trace", 14.f, COLOR_BTN_BODY);
	
	InitTitle(&m_stt_ELB_TraceInterval, "Trace Interval", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_EdgePolarity, "Search Edge Polarity", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ELB_SearchDir, "Search Dir", 14.f, COLOR_BTN_BODY);
	
	InitTitle(&m_stt_ExistDummyGray, "Dummy Gray(>)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_DistanceInspLSL, "Distance LSL(mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_DistanceInspUSL, "Distance USL(mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_DistanceInspSubSpec, "Tilt Standard(mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_DistanceInspSubSpecTolerance, "Tilit Tolerance (mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_DistanceInspSpecY, "Distance Y (mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_DistanceInspSpecYminmax, "Distance Y Tolerance (mm)", 14.f, COLOR_BTN_BODY);

	InitTitle(&m_stt_UseReferenceMark, "Reference Mark Use", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_InspectionMethod, "Inspection Method", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_InspectionSubMethod, "Inspection Sub Method", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_InspectionSubInterval, "PCB Interval Spec", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_InspectionSubDir, "Inspection Sub Direction", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_InspectionIntervalTolerance, "PCB Interval Tolerance", 14.f, COLOR_BTN_BODY);

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
	EditButtonInit( &m_edt_CameraDistance, 20);
	EditButtonInit( &m_edt_DummyCornerIncludedAngle, 20);
	EditButtonInit( &m_edt_DummyCornerIncludedAngleLimit, 20);
	EditButtonInit( &m_edt_IncludedAngle, 20);
	EditButtonInit( &m_edt_IncludedAngleLimit, 20);
	EditButtonInit( &m_edt_ProjectionAreaLeft[0], 20);
	EditButtonInit( &m_edt_ProjectionAreaLeft[1], 20);
	EditButtonInit( &m_edt_SearchAreaLeft[0], 20);
	EditButtonInit( &m_edt_SearchAreaLeft[1], 20);
	EditButtonInit( &m_edt_RemoveNoiseSize[0], 20);
	EditButtonInit( &m_edt_RemoveNoiseSize[1], 20);
	EditButtonInit( &m_edt_SobelThreshold[0], 20);
	EditButtonInit( &m_edt_SobelThreshold[1], 20);

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
	EditButtonInit(&m_edt_DistanceInspSpecY, 20);
	EditButtonInit(&m_edt_DistanceInspSubSpecTolerance, 20);
	EditButtonInit(&m_edt_DistanceInspSpecYminmax, 20);
	EditButtonInit(&m_edt_DistanceInspSubInterval, 20);
	EditButtonInit(&m_edt_DistanceInspSubIntervalTol, 20);
	//210110

	EditGroupInit(&m_grpAlignFunction, "Align Function", 15.f, COLOR_BTN_BODY);
	EditGroupInit(&m_grpLCheck, "L Check", 15.f, COLOR_BTN_BODY);	
	EditGroupInit(&m_grpAlignType, "Align Type", 15.f, COLOR_BTN_BODY);
	EditGroupInit(&m_grpRevision, "Revision", 15.f, COLOR_BTN_BODY);
	EditGroupInit(&m_grpExistFunction, "Exist Function", 15.f, COLOR_BTN_BODY);
	EditGroupInit(&m_grpElbInspFunction, "ELB Inspection Function", 15.f, COLOR_BTN_BODY);
	EditGroupInit(&m_grpDistanceFunction, "Distance Inspection Function", 15.f, COLOR_BTN_BODY);
	EditGroupInit(&m_grpDistanceSubFunction, "Distance Inspection Sub Function", 15.f, COLOR_BTN_BODY);
	EditGroupInit(&m_grpDistanceFunction, "Distance Inspection Function", 15.f, COLOR_BTN_BODY);
	EditGroupInit(&m_grpAlignRansac, "Align Ransac", 15.f, COLOR_BTN_BODY);

	m_btnEnableAlignMeasureSpecOutJudge.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableLCheck.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnUseCrackInsp.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableDustCheck.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableDiffCheck.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableRealTimeTrace.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableFixTrace.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnUseDummyCornerInsp.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnUseIncludedAngle.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnUseMultiplexRoi.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnUseBoxInsp.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnUseDummySeparInsp.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnUseReferenceMark.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableRevisionReverseX.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableRevisionReverseY.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableRevisionReverseT.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));
	m_btnEnableRevisionReverseXY.LoadImageFromFile(_T("check-select.png"), _T("check-normal.png"));

	m_cmbPreProcess[0].AddString("None");
	m_cmbPreProcess[0].AddString("Gaussian 3");
	m_cmbPreProcess[0].AddString("Median 3");
	m_cmbPreProcess[0].AddString("Median 5");
	m_cmbPreProcess[0].AddString("Median 7");
	m_cmbPreProcess[0].AddString("Median 11");
	m_cmbPreProcess[0].AddString("Close 1");
	m_cmbPreProcess[0].AddString("Close 3");
	m_cmbPreProcess[0].AddString("Close 5");

	m_cmbEdgeProcess[0].AddString("Sobel");			// 19.11.12
	m_cmbEdgeProcess[0].AddString("Enhance_B");		// 19.11.12
	m_cmbEdgeProcess[0].AddString("Enhance_W_V2");	// 19.11.12
	m_cmbEdgeProcess[0].AddString("Enhance_W");		// 19.11.12
	m_cmbEdgeProcess[0].AddString("Histo_W");		// 19.12.02
	m_cmbEdgeProcess[0].AddString("Histo_B");		// 19.12.02

	int camCount = m_pMain->vt_job_info[m_nJobID].num_of_camera;
	int posCount = m_pMain->vt_job_info[m_nJobID].num_of_position;
	std::vector<int> camBuf = m_pMain->vt_job_info[m_nJobID].camera_index;

	CString strTemp;
	for (int i = 0; i < posCount; i++)
	{
		strTemp.Format("%d", i + 1);
		m_cbModelSelPos.AddString(strTemp);
	}

	for (int i = 0; i < camCount; i++)
	{
		strTemp.Format("%d", camBuf.at(i) + 1);
		m_cbModelSelCam.AddString(strTemp);
	}

	for (int i = 0; i < camCount; i++)
	{
		strTemp.Format("%d", camBuf.at(i) + 1);
		((CComboBox*)GetDlgItem(IDC_COMBO_DUMMY_CORNER_INSP_CAM))->AddString(strTemp);
	}

	strTemp = "ALL";
	((CComboBox*)GetDlgItem(IDC_COMBO_DUMMY_CORNER_INSP_CAM))->AddString(strTemp);

	m_cbModelSelPos.SetCurSel(0);
	m_cbModelSelCam.SetCurSel(0);
	m_selInspectionMethod.SetCurSel(0);
	m_selInspectionSubMethod.SetCurSel(0);
	m_selInspectionSubDir.SetCurSel(0);

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
		pWnd->GetDlgCtrlID() == IDC_COMBO_ALIGNMENT_TARGET ||
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
		pWnd->GetDlgCtrlID() == IDC_COMBO_INSP_SUB_METHOD ||
		pWnd->GetDlgCtrlID() == IDC_COMBO_INSP_SUB_DIRECTION ||
		pWnd->GetDlgCtrlID() == IDC_CB_MODEL_SELECT_POS)
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
		CString strModel = m_pMain->m_strCurrModel;
		showModelInfo(strModel);
		//showAlignInfo(strModel);
		enableEditControls(FALSE);
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

	CModel *pModel = &m_pMain->vt_job_info[m_nJobID].model_info;

	GetDlgItem(IDC_EDIT_ALIGN_SPEC)->GetWindowTextA(str);
	GetDlgItem(IDC_EDIT_ALIGN_SPEC_MINMAX)->GetWindowTextA(str2);

	pModel->getAlignInfo().setAlignSpec(0, atof(str));
	pModel->getAlignInfo().setAlignSpecMinMax(0, atof(str));

	nSel = ((CComboBox*)GetDlgItem(IDC_COMBO_ALIGNMENT_TARGET))->GetCurSel();
	pModel->getAlignInfo().setAlignmentTarget(nSel);

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
	pModel->getAlignInfo().setEnableAlignMeasureSpecOutJudge(m_btnEnableAlignMeasureSpecOutJudge.GetCheck());
	pModel->getAlignInfo().setEnableLCheck(m_btnEnableLCheck.GetCheck());
	pModel->getAlignInfo().setUseCrackInsp(m_btnUseCrackInsp.GetCheck());
	pModel->getAlignInfo().setUseDummyCornerInsp(m_btnUseDummyCornerInsp.GetCheck());
	pModel->getAlignInfo().setUseIncludedAngle(m_btnUseIncludedAngle.GetCheck());
	pModel->getAlignInfo().setUseMultiplexRoi(m_btnUseMultiplexRoi.GetCheck());
	pModel->getAlignInfo().setUseBoxInsp(m_btnUseBoxInsp.GetCheck());
	pModel->getAlignInfo().setUseDummySeparInsp(m_btnUseDummySeparInsp.GetCheck());
	pModel->getAlignInfo().setUseReferenceMark(m_btnUseReferenceMark.GetCheck());
	pModel->getAlignInfo().setRevisionReverseX(m_btnEnableRevisionReverseX.GetCheck());
	pModel->getAlignInfo().setRevisionReverseY(m_btnEnableRevisionReverseY.GetCheck());
	pModel->getAlignInfo().setRevisionReverseT(m_btnEnableRevisionReverseT.GetCheck());
	pModel->getAlignInfo().setRevisionReverseXY(m_btnEnableRevisionReverseXY.GetCheck());
	pModel->getAlignInfo().setUseDustInsp(m_btnEnableDustCheck.GetCheck());
	pModel->getAlignInfo().setUseDiffInsp(m_btnEnableDiffCheck.GetCheck());
	pModel->getAlignInfo().setUseRealTimeTrace(m_btnEnableRealTimeTrace.GetCheck());
	pModel->getAlignInfo().setUseUseFixTrace(m_btnEnableFixTrace.GetCheck());

	

	pModel->getAlignInfo().setLCheckReference(m_cmbLCheckRef.GetCurSel());
	pModel->getAlignInfo().setSearchEdgePolarity(m_cmbSearchEdgePolarity.GetCurSel());
	pModel->getAlignInfo().setSearchSearchDir(m_cmbSearchDir.GetCurSel());

	GetDlgItem(IDC_EDIT_L_CHECK_TOLERANCE)->GetWindowTextA(str);
	pModel->getAlignInfo().setLCheckTor(atof(str));

	GetDlgItem(IDC_EDIT_L_CHECK_SPEC)->GetWindowTextA(str);
	pModel->getAlignInfo().setLCheckSpecX(atof(str));
	GetDlgItem(IDC_EDIT_L_CHECK_SPEC_Y)->GetWindowTextA(str);
	pModel->getAlignInfo().setLCheckSpecY(atof(str));
	//pModel->getAlignInfo().(atof(str));
	GetDlgItem(IDC_EDIT_CAMERA_DISTANCE)->GetWindowTextA(str);
	pModel->getAlignInfo().setCameraDistance(atof(str));

	nSel = ((CComboBox*)GetDlgItem(IDC_COMBO_DUMMY_CORNER_INSP_CAM))->GetCurSel();
	pModel->getAlignInfo().setDummyCornerInspCam(nSel);

	nSel = ((CComboBox*)GetDlgItem(IDC_COMBO_DUMMY_CORNER_FILTER))->GetCurSel();
	pModel->getAlignInfo().setDummyCornerFilter(nSel);

	GetDlgItem(IDC_EDIT_INCLUDED_ANGLE)->GetWindowTextA(str);
	pModel->getAlignInfo().setIncludedAngle(atof(str));

	GetDlgItem(IDC_EDIT_INCLUDED_ANGLE_LIMIT)->GetWindowTextA(str);
	pModel->getAlignInfo().setIncludedAngleLimit(atof(str));

	GetDlgItem(IDC_EDIT_DUMMY_INCLUDED_ANGLE)->GetWindowTextA(str);
	pModel->getAlignInfo().setDummyCornerIncludedAngle(atof(str));

	GetDlgItem(IDC_EDIT_DUMMY_INCLUDED_ANGLE_LIMIT)->GetWindowTextA(str);
	pModel->getAlignInfo().setDummyCornerIncludedAngleLimit(atof(str));

	GetDlgItem(IDC_EDIT_EXIST_PANEL_GRAY)->GetWindowTextA(str); //수정해야할부분
	pModel->getAlignInfo().setExistPanelGray(atoi(str));
	GetDlgItem(IDC_EDIT_EXIST_DUMMY_GRAY)->GetWindowTextA(str);
	pModel->getAlignInfo().setExistDummyGray(atoi(str));
	GetDlgItem(IDC_EDIT_ELB_INSP_RANGE_X)->GetWindowTextA(str);
	pModel->getAlignInfo().setInspRangeX(atof(str));
	GetDlgItem(IDC_EDIT_ELB_INSP_RANGE_Y)->GetWindowTextA(str);
	pModel->getAlignInfo().setInspRangeY(atof(str));
	GetDlgItem(IDC_EDIT_ELB_INSP_LENGTH)->GetWindowTextA(str);
	pModel->getAlignInfo().setInspRangeLength(atof(str));
	GetDlgItem(IDC_EDIT_ELB_TRACE_INTERVAL)->GetWindowTextA(str);
	pModel->getAlignInfo().setInspTraceInterval(atof(str));

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

	nSel = ((CComboBox*)GetDlgItem(IDC_COMBO_INSP_SUB_METHOD))->GetCurSel();
	pModel->getAlignInfo().setInspectionSubMethod(nSel);

	nSel = ((CComboBox*)GetDlgItem(IDC_COMBO_INSP_SUB_DIRECTION))->GetCurSel();
	pModel->getAlignInfo().setInspectionSubDir(nSel);

	nSel = ((CComboBox*)GetDlgItem(IDC_EDIT_INSP_SUB_INTERVAL))->GetCurSel();
	pModel->getAlignInfo().setInspectionSubInterval(nSel);

	nSel = ((CComboBox*)GetDlgItem(IDC_EDIT_INSP_SUB_INTERVAL_TOL))->GetCurSel();
	pModel->getAlignInfo().setInspectionSubIntervalTolerance(nSel);
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

	CAlignInfo alignInfo = m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo();
	int nCam = m_nModelSelCam;
	((CComboBox*)GetDlgItem(IDC_COMBO_ALIGNMENT_TARGET))->SetCurSel(alignInfo.getAlignmentTarget());
	((CComboBox*)GetDlgItem(IDC_COMBO_ANGLE_CALC_METHOD))->SetCurSel(alignInfo.getAngleCalcMethod());
	((CComboBox*)GetDlgItem(IDC_CB_MARK_FIND_METHOD))->SetCurSel(alignInfo.getMarkFindMethod());
	((CComboBox*)GetDlgItem(IDC_COMBO_ALIGNMENT_DIR))->SetCurSel(alignInfo.getAlignmentTargetDir());

	((CComboBox*)GetDlgItem(IDC_CB_CORNER_TYPE0))->SetCurSel(alignInfo.getCornerType(nCam, m_nModelSelPos));
	((CComboBox*)GetDlgItem(IDC_CB_EDGE_POLARITY0))->SetCurSel(alignInfo.getEdgePolarity(nCam, m_nModelSelPos));
	((CComboBox*)GetDlgItem(IDC_CB_EDGE_DIRECTION0))->SetCurSel(alignInfo.getEdgeDirection(nCam, m_nModelSelPos));
	((CComboBox*)GetDlgItem(IDC_CB_PRE_PROCESS1))->SetCurSel(alignInfo.getPreProcess(nCam, m_nModelSelPos));
	((CComboBox*)GetDlgItem(IDC_CB_EDGE_PROCESS1))->SetCurSel(alignInfo.getEdgeProcess(nCam, m_nModelSelPos));
	((CComboBox*)GetDlgItem(IDC_COMBO_DUMMY_CORNER_INSP_CAM))->SetCurSel(alignInfo.getDummyCornerInspCam());
	((CComboBox*)GetDlgItem(IDC_COMBO_DUMMY_CORNER_FILTER))->SetCurSel(alignInfo.getDummyCornerFilter());
	((CComboBox*)GetDlgItem(IDC_COMBO_INSP_METHOD))->SetCurSel(alignInfo.getInspectionMethod());
	((CComboBox*)GetDlgItem(IDC_COMBO_INSP_SUB_METHOD))->SetCurSel(alignInfo.getInspectionSubMethod());
	((CComboBox*)GetDlgItem(IDC_COMBO_INSP_SUB_DIRECTION))->SetCurSel(alignInfo.getInspectionSubDir());

	((CComboBox*)GetDlgItem(IDC_COMBO_LCHECK_REFERENCE))->SetCurSel(alignInfo.getLCheckReference());
	((CComboBox*)GetDlgItem(IDC_COMBO_EDGE_POLARITY))->SetCurSel(alignInfo.getSearchEdgePolarity());
	((CComboBox*)GetDlgItem(IDC_COMBO_SEARCH_DIR))->SetCurSel(alignInfo.getSearchSearchDir());
	

	strData.Format("%d", alignInfo.getRemoveNoiseSize(nCam, m_nModelSelPos, 0));
	GetDlgItem(IDC_EDIT_REMOVE_NOISE_SIZE1)->SetWindowTextA(strData);
	strData.Format("%d", alignInfo.getRemoveNoiseSize(nCam, m_nModelSelPos, 1));
	GetDlgItem(IDC_EDIT_REMOVE_NOISE_SIZE2)->SetWindowTextA(strData);

	strData.Format("%d", alignInfo.getSobelThreshold(nCam, m_nModelSelPos, 0));
	GetDlgItem(IDC_EDIT_SOBEL_THRESHOLD1)->SetWindowTextA(strData);
	strData.Format("%d", alignInfo.getSobelThreshold(nCam, m_nModelSelPos, 1));
	GetDlgItem(IDC_EDIT_SOBEL_THRESHOLD2)->SetWindowTextA(strData);

	strData.Format("%d", alignInfo.getSearchArea(nCam, m_nModelSelPos, 0));
	GetDlgItem(IDC_EDIT_SEARCH_AREA_LEFT1)->SetWindowTextA(strData);
	strData.Format("%d", alignInfo.getSearchArea(nCam, m_nModelSelPos, 1));
	GetDlgItem(IDC_EDIT_SEARCH_AREA_LEFT2)->SetWindowTextA(strData);

	strData.Format("%d", alignInfo.getProjectionArea(nCam, m_nModelSelPos, 0));
	GetDlgItem(IDC_EDIT_PROJECTION_AREA_LEFT)->SetWindowTextA(strData);
	strData.Format("%d", alignInfo.getProjectionArea(nCam, m_nModelSelPos, 1));
	GetDlgItem(IDC_EDIT_PROJECTION_AREA_LEFT2)->SetWindowTextA(strData);

	strData.Format("%.3f", alignInfo.getAlignSpec(nCam));
	GetDlgItem(IDC_EDIT_ALIGN_SPEC)->SetWindowTextA(strData);

	strData.Format("%.3f", alignInfo.getAlignSpecMinMax(nCam));
	GetDlgItem(IDC_EDIT_ALIGN_SPEC_MINMAX)->SetWindowTextA(strData);

	strData.Format("%.3f", alignInfo.getLCheckTor());
	GetDlgItem(IDC_EDIT_L_CHECK_TOLERANCE)->SetWindowTextA(strData);

	strData.Format("%.3f", alignInfo.getLCheckSpecX());
	GetDlgItem(IDC_EDIT_L_CHECK_SPEC)->SetWindowTextA(strData);

	strData.Format("%.3f", alignInfo.getLCheckSpecY());
	GetDlgItem(IDC_EDIT_L_CHECK_SPEC_Y)->SetWindowTextA(strData);

	strData.Format("%.3f", alignInfo.getCameraDistance());
	GetDlgItem(IDC_EDIT_CAMERA_DISTANCE)->SetWindowTextA(strData);

	strData.Format("%.3f", alignInfo.getIncludedAngle());
	GetDlgItem(IDC_EDIT_INCLUDED_ANGLE)->SetWindowTextA(strData);

	strData.Format("%.3f", alignInfo.getIncludedAngleLimit());
	GetDlgItem(IDC_EDIT_INCLUDED_ANGLE_LIMIT)->SetWindowTextA(strData);

	strData.Format("%.3f", alignInfo.getDummyCornerIncludedAngle());
	GetDlgItem(IDC_EDIT_DUMMY_INCLUDED_ANGLE)->SetWindowTextA(strData);

	strData.Format("%.3f", alignInfo.getDummyCornerIncludedAngleLimit());
	GetDlgItem(IDC_EDIT_DUMMY_INCLUDED_ANGLE_LIMIT)->SetWindowTextA(strData);

	strData.Format("%d", alignInfo.getExistPanelGray());  //수정해야할부분
	GetDlgItem(IDC_EDIT_EXIST_PANEL_GRAY)->SetWindowTextA(strData);
	strData.Format("%d", alignInfo.getExistDummyGray());
	GetDlgItem(IDC_EDIT_EXIST_DUMMY_GRAY)->SetWindowTextA(strData);
	strData.Format("%4.4f", alignInfo.getInspRangeX());  
	GetDlgItem(IDC_EDIT_ELB_INSP_RANGE_X)->SetWindowTextA(strData);
	strData.Format("%4.4f", alignInfo.getInspRangeY());
	GetDlgItem(IDC_EDIT_ELB_INSP_RANGE_Y)->SetWindowTextA(strData);
	strData.Format("%4.4f", alignInfo.getInspRangeLength());
	GetDlgItem(IDC_EDIT_ELB_INSP_LENGTH)->SetWindowTextA(strData);
	strData.Format("%4.4f", alignInfo.getInspTraceInterval());
	GetDlgItem(IDC_EDIT_ELB_TRACE_INTERVAL)->SetWindowTextA(strData);	

	strData.Format("%.3f", alignInfo.getDistanceInspSpecLSL(0));
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_LSL)->SetWindowTextA(strData);
	
	strData.Format("%.3f", alignInfo.getDistanceInspSubSpec(0));
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_SUB_SPEC)->SetWindowTextA(strData);
	strData.Format("%.3f", alignInfo.getDistanceInspSpecY(0));
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_SPEC_Y)->SetWindowTextA(strData);

	strData.Format("%.3f", alignInfo.getDistanceInspSpecUSL(0));
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_USL)->SetWindowTextA(strData);
	strData.Format("%.3f", alignInfo.getDistanceInspSubSpecTolerance(0));
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_SUB_SPEC_TOLERANCE)->SetWindowTextA(strData);
	strData.Format("%.3f", alignInfo.getDistanceInspSpecYMinMax(0));
	GetDlgItem(IDC_EDIT_DISTANCE_INSP_SPEC_Y_MINMAX)->SetWindowTextA(strData);
	strData.Format("%.3f", alignInfo.getInspectionSubInterval());
	GetDlgItem(IDC_EDIT_INSP_SUB_INTERVAL)->SetWindowTextA(strData);
	strData.Format("%.3f", alignInfo.getInspectionSubIntervalTolerance());
	GetDlgItem(IDC_EDIT_INSP_SUB_INTERVAL_TOL)->SetWindowTextA(strData);

	m_btnEnableAlignMeasureSpecOutJudge.SetCheck(alignInfo.getEnableAlignMeasureSpecOutJudge());
	m_btnUseCrackInsp.SetCheck(alignInfo.getUseCrackInsp());
	m_btnUseIncludedAngle.SetCheck(alignInfo.getUseIncludedAngle());
	m_btnUseDummyCornerInsp.SetCheck(alignInfo.getUseDummyCornerInsp());
	m_btnUseBoxInsp.SetCheck(alignInfo.getUseBoxInsp());
	m_btnUseDummySeparInsp.SetCheck(alignInfo.getUseDummySeparInsp());
	m_btnUseMultiplexRoi.SetCheck(alignInfo.getUseMultiplexRoi());
	m_btnUseReferenceMark.SetCheck(alignInfo.getUseReferenceMark());

	m_btnEnableLCheck.SetCheck(alignInfo.getEnableLCheck());
	m_btnEnableDustCheck.SetCheck(alignInfo.getUseDustInsp());
	m_btnEnableDiffCheck.SetCheck(alignInfo.getUseDiffInsp());	
	m_btnEnableRealTimeTrace.SetCheck(alignInfo.getUseRealTimeTrace());
	m_btnEnableFixTrace.SetCheck(alignInfo.getUseUseFixTrace());
	
	m_btnEnableRevisionReverseX.SetCheck(alignInfo.getRevisionReverseX());
	m_btnEnableRevisionReverseY.SetCheck(alignInfo.getRevisionReverseY());
	m_btnEnableRevisionReverseT.SetCheck(alignInfo.getRevisionReverseT());
	m_btnEnableRevisionReverseXY.SetCheck(alignInfo.getRevisionReverseXY());
}

void TabModelAlgoPage::enableEditControls(BOOL bEnable)
{	
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
	m_edt_CameraDistance.EnableWindow(bEnable);
	m_edt_DummyCornerIncludedAngle.EnableWindow(bEnable);
	m_edt_DummyCornerIncludedAngleLimit.EnableWindow(bEnable);
	m_edt_IncludedAngle.EnableWindow(bEnable);
	m_edt_IncludedAngleLimit.EnableWindow(bEnable);
	m_edt_ProjectionAreaLeft[0].EnableWindow(bEnable);
	m_edt_ProjectionAreaLeft[1].EnableWindow(bEnable);
	m_edt_SearchAreaLeft[0].EnableWindow(bEnable);
	m_edt_SearchAreaLeft[1].EnableWindow(bEnable);
	m_edt_RemoveNoiseSize[0].EnableWindow(bEnable);
	m_edt_RemoveNoiseSize[1].EnableWindow(bEnable);
	m_edt_SobelThreshold[0].EnableWindow(bEnable);
	m_edt_SobelThreshold[1].EnableWindow(bEnable);

	//210110
	m_edt_ExistPanelGray.EnableWindow(bEnable);
	m_edt_ExistDummyGray.EnableWindow(bEnable);
	m_edt_InspRangeX.EnableWindow(bEnable);
	m_edt_InspRangeY.EnableWindow(bEnable);
	m_edt_InspRangeLength.EnableWindow(bEnable);
	m_edt_InspTraceInterval.EnableWindow(bEnable);
	m_edt_DistanceInspSpecLSL.EnableWindow(bEnable);
	m_edt_DistanceInspSubSpec.EnableWindow(bEnable);
	m_edt_DistanceInspSpecY.EnableWindow(bEnable);
	m_edt_DistanceInspSpecUSL.EnableWindow(bEnable);
	m_edt_DistanceInspSubSpecTolerance.EnableWindow(bEnable);
	m_edt_DistanceInspSpecYminmax.EnableWindow(bEnable);
	m_edt_DistanceInspSubInterval.EnableWindow(bEnable);
	m_edt_DistanceInspSubIntervalTol.EnableWindow(bEnable);
	//210110

	m_cmbAlignDir.EnableWindow(bEnable);
	m_cmbMarkFindMethod.EnableWindow(bEnable);
	m_cmbAlignTarget.EnableWindow(bEnable);
	m_cmbLCheckRef.EnableWindow(bEnable);
	m_cmbSearchEdgePolarity.EnableWindow(bEnable);
	m_cmbSearchDir.EnableWindow(bEnable);
	m_cmbInspSpec.EnableWindow(bEnable);
	m_cmbInspSubSpec.EnableWindow(bEnable);
	m_selAlignType.EnableWindow(bEnable);
	m_cmbAngleCalcMethod.EnableWindow(bEnable);
	m_selCornerFilter.EnableWindow(bEnable);
	m_selCornerInspCam.EnableWindow(bEnable);
	m_cbModelSelCam.EnableWindow(bEnable);
	m_cbModelSelPos.EnableWindow(bEnable);
	m_cmbCornerType[0].EnableWindow(bEnable);
	m_cmbEdgePolarity[0].EnableWindow(bEnable);
	m_cmbEdgeDirection[0].EnableWindow(bEnable);
	m_cmbPreProcess[0].EnableWindow(bEnable);
	m_cmbEdgeProcess[0].EnableWindow(bEnable);
	m_selInspectionMethod.EnableWindow(bEnable);
	m_selInspectionSubMethod.EnableWindow(bEnable);
	m_selInspectionSubDir.EnableWindow(bEnable);

	m_btnEnableAlignMeasureSpecOutJudge.EnableWindow(bEnable);
	m_btnEnableLCheck.EnableWindow(bEnable);
	m_btnUseCrackInsp.EnableWindow(bEnable);
	m_btnUseDummyCornerInsp.EnableWindow(bEnable);
	m_btnUseIncludedAngle.EnableWindow(bEnable);
	m_btnUseMultiplexRoi.EnableWindow(bEnable);
	m_btnUseBoxInsp.EnableWindow(bEnable);
	m_btnUseDummySeparInsp.EnableWindow(bEnable);
	m_btnUseReferenceMark.EnableWindow(bEnable);
	m_btnEnableRevisionReverseX.EnableWindow(bEnable);
	m_btnEnableRevisionReverseY.EnableWindow(bEnable);
	m_btnEnableRevisionReverseT.EnableWindow(bEnable);
	m_btnEnableRevisionReverseXY.EnableWindow(bEnable);
	m_btnEnableDustCheck.EnableWindow(bEnable);
	m_btnEnableDiffCheck.EnableWindow(bEnable);
	m_btnEnableRealTimeTrace.EnableWindow(bEnable);
	m_btnEnableFixTrace.EnableWindow(bEnable);

	m_btnUseBoxInsp.RedrawWindow();
	m_btnUseDummySeparInsp.RedrawWindow();
	m_btnUseIncludedAngle.RedrawWindow();
	m_btnUseDummyCornerInsp.RedrawWindow();
	m_btnUseCrackInsp.RedrawWindow();	
	m_btnEnableAlignMeasureSpecOutJudge.RedrawWindow();	
	m_btnEnableLCheck.RedrawWindow();	
	m_btnUseMultiplexRoi.RedrawWindow();
	m_btnUseReferenceMark.RedrawWindow();
	m_btnEnableRevisionReverseX.RedrawWindow();
	m_btnEnableRevisionReverseY.RedrawWindow();
	m_btnEnableRevisionReverseT.RedrawWindow();
	m_btnEnableRevisionReverseXY.RedrawWindow();
	m_btnEnableDustCheck.RedrawWindow();
	m_btnEnableDiffCheck.RedrawWindow();
	m_btnEnableRealTimeTrace.RedrawWindow();
	m_btnEnableFixTrace.RedrawWindow();
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
void TabModelAlgoPage::OnEnSetfocusEditCameraDistance() { setEditBox(IDC_EDIT_CAMERA_DISTANCE); }
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
void TabModelAlgoPage::OnEnSetfocusEditDistanceInspSubSpecX() { setEditBox(IDC_EDIT_INSP_SUB_INTERVAL); }
void TabModelAlgoPage::OnEnSetfocusEditDistanceInspSubTolerance() { setEditBox(IDC_EDIT_INSP_SUB_INTERVAL_TOL); }

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
	GetDlgItem(IDC_PIC_RANSAC)->SetFocus(); // Focus를 다른 곳으로 이동

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
	}
}

void TabModelAlgoPage::OnCbnSelchangeCbModelSelectCam()
{
	m_nModelSelCam = m_cbModelSelCam.GetCurSel();
	updateAlignInfoData();
}

void TabModelAlgoPage::OnCbnSelchangeCbModelSelectPos()
{
	m_nModelSelPos = m_cbModelSelPos.GetCurSel();
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
	//Exist Fuction
	m_stt_UseBoxInspection.ShowWindow(SW_HIDE);
	m_btnUseBoxInsp.ShowWindow(SW_HIDE);
	m_stt_UseDummySeparInspection.ShowWindow(SW_HIDE);
	m_btnUseDummySeparInsp.ShowWindow(SW_HIDE);

	//Aalign Function
	m_stt_UseDummyCornerIncludedAngle.ShowWindow(SW_HIDE);
	m_btnUseDummyCornerInsp.ShowWindow(SW_HIDE);
	m_stt_DummyCornerFilter.ShowWindow(SW_HIDE);
	m_selCornerFilter.ShowWindow(SW_HIDE);
	m_stt_DummyCornerInspCam.ShowWindow(SW_HIDE);
	m_selCornerInspCam.ShowWindow(SW_HIDE);
	m_stt_DummyCornerIncludedAngle.ShowWindow(SW_HIDE);
	m_edt_DummyCornerIncludedAngle.ShowWindow(SW_HIDE);
	m_stt_DummyCornerIncludedAngleLimit.ShowWindow(SW_HIDE);
	m_edt_DummyCornerIncludedAngleLimit.ShowWindow(SW_HIDE);

	//나머지
	m_stt_InspectionSubInterval.ShowWindow(SW_HIDE);
	m_stt_InspectionIntervalTolerance.ShowWindow(SW_HIDE);
	m_edt_DistanceInspSubInterval.ShowWindow(SW_HIDE);
	m_edt_DistanceInspSubIntervalTol.ShowWindow(SW_HIDE);
	m_edt_DistanceInspSpecY.ShowWindow(SW_HIDE);
	m_edt_DistanceInspSpecYminmax.ShowWindow(SW_HIDE);
	m_stt_DistanceInspSpecY.ShowWindow(SW_HIDE);
	m_stt_DistanceInspSpecYminmax.ShowWindow(SW_HIDE);

	m_stt_InspectionSubMethod.ShowWindow(SW_HIDE);
	m_selInspectionSubMethod.ShowWindow(SW_HIDE);
	m_stt_InspectionSubDir.ShowWindow(SW_HIDE);
	m_selInspectionSubDir.ShowWindow(SW_HIDE);
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

	str_temp.Format("%.3f", spec_y);
	m_edt_DistanceInspSpecY.SetText(str_temp);

	str_temp.Format("%.3f", spec_tolerance);
	m_edt_DistanceInspSpecUSL.SetText(str_temp);

	str_temp.Format("%.3f", spec_x_tolerance);
	m_edt_DistanceInspSubSpecTolerance.SetText(str_temp);

	str_temp.Format("%.3f", spec_y_tolerance);
	m_edt_DistanceInspSpecYminmax.SetText(str_temp);
}
void TabModelAlgoPage::OnCbnSelchangeComboInspSubSpec()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int pos = m_cmbInspSubSpec.GetCurSel();

	CString str_temp;
	double spec_tilt = m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getDistanceInspSubSpec(pos);
	double spec_tilt_tolerance = m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(0);

	str_temp.Format("%.3f", spec_tilt);
	m_edt_DistanceInspSubSpec.SetText(str_temp);

	str_temp.Format("%.3f", spec_tilt_tolerance);
	m_edt_DistanceInspSubSpecTolerance.SetText(str_temp);
}
