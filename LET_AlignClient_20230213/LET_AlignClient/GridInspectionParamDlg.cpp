// GridInspectionParamDlg.cpp: 구현 파일
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "GridInspectionParamDlg.h"
#include "afxdialogex.h"

// CGridInspectionParamDlg 대화 상자

IMPLEMENT_DYNAMIC(CGridInspectionParamDlg, CDialogEx)

CGridInspectionParamDlg::CGridInspectionParamDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_SET_INSPPARA, pParent)
{
	bUse = FALSE;
	m_pMain = NULL;
	m_nJob = 0;

	// hsj 2023-01-03 grid갱신
	m_font.CreateFont(23, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
}

CGridInspectionParamDlg::~CGridInspectionParamDlg()
{
	m_font.DeleteObject();
}

void CGridInspectionParamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCPROPERTYGRID_INSPECTION, m_ctlPropGrid);
}


BEGIN_MESSAGE_MAP(CGridInspectionParamDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CGridInspectionParamDlg::OnBnClickedOk)
	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnPropertyChanged)
	ON_STN_CLICKED(IDC_MFCPROPERTYGRID_INSPECTION, &CGridInspectionParamDlg::OnClickedMfcpropertygridInspection)
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

HBRUSH CGridInspectionParamDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	if (pWnd->m_hWnd == this->m_hWnd) hbr = m_hbrBkg;

	return hbr;
}

// CGridInspectionParamDlg 메시지 처리기
LRESULT CGridInspectionParamDlg::OnPropertyChanged(WPARAM wparam, LPARAM lparam)
{
	CMFCPropertyGridProperty* pProp = NULL;
	pProp = (CMFCPropertyGridProperty*)lparam;

	if (!pProp)		return 0L;

	_variant_t var = pProp->GetValue();

	int id = pProp->GetData();

	CString str;
	str = pProp->GetName();


	switch (var.vt)
	{
	case VT_UINT:
		str.Format(_T("Name(ID:%d):%s, Value:%d"), id, str, var.uintVal);
		break;
	case VT_INT:
		str.Format(_T("Name(ID:%d):%s, Value:%d"), id, str, var.intVal);
		break;
	case VT_I2:
		str.Format(_T("Name(ID:%d):%s, Value:%d"), id, str, var.iVal);
		break;
	case VT_I4:
		str.Format(_T("Name(ID:%d):%s, Value:%d"), id, str, var.lVal);
		break;
	case VT_R4:
		str.Format(_T("Name(ID:%d):%s, Value:%f"), id, str, var.fltVal);
		break;
	case VT_R8:
		str.Format(_T("Name(ID:%d):%s, Value:%f"), id, str, var.dblVal);
		break;
	case VT_BSTR:
		str.Format(_T("Name(ID:%d):%s, Value:%s"), id, str, var.bstrVal);
		break;
	case VT_BOOL:
		str.Format(_T("Name(ID:%d):%s, Value:%d"), id, str, var.boolVal);
		break;
	}

	return 0L;
}

void CGridInspectionParamDlg::Init()
{
	int m_nX;
	int m_nY;
	double m_dTheta;
	int m_nType;
	CString m_strDataName;
	CString m_strID;
	COLORREF m_clrColor;	

	m_strDataName = _T("1BE99CCD");
	m_nX = 10;
	m_nY = 20;
	m_dTheta = 45.9;
	m_nType = 1;
	m_clrColor = RGB(255, 192, 192);

	// hsj 2023-01-03 grid갱신
	
	CBBungGrid::stProperty GRID_STUFF[] =
	{
		P_ROOT(_T("TEXT:Cell Data;")),
			P_SUBROOT(_T("TEXT:Cell Data;")),
				P_VAR(m_strDataName, _T("TEXT:Data Name;ENABLE:FALSE;"),_T("")),
				P_VAR(m_nX, _T("TEXT:X;"),_T("")),
				P_VAR(m_nY, _T("TEXT:Y;"),_T("")),
				P_VAR(m_dTheta, _T("TEXT:Theta;"),_T("")),
				P_VAR(m_nType, _T("TEXT:Type;TYPE:1;ARR:Type1,Type2,Type3;"),_T("")),
			P_SUBROOT_END(),
			P_SUBROOT(_T("TEXT:Other;")),
				P_VAR(m_clrColor, _T("TEXT:Color;TYPE:2;"),_T("")),
			P_SUBROOT_END(),
		P_ROOT_END(),
	};
	int nSize = sizeof(GRID_STUFF) / sizeof(CBBungGrid::stProperty);

	m_ctlPropGrid.Initial(&m_font);
	m_ctlPropGrid.AppendProperty(GRID_STUFF, nSize);	
}

void CGridInspectionParamDlg::InitCenterCircle()
{
	// hsj 2023-01-03 grid갱신
	if (m_pMain == NULL) return;

	bool bWetoutEnable 					= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getUseDiffInsp();						//true										// wetout 검사
	bool bDustEnable 					= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getUseInputInspection();				//false									    // 이물 검사
	bool bLiquidDropsEnable 			= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getUseDustInsp();						//false										// 액튐 검사

	bool bDispensingStatusEnable 		= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getDispensingStatusEnable();			//m_pMain->m_bDispensingStatusEnable;		//도포된 패널 유무 검사
	bool bLamiStatusEnable 				= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getLamiStatusEnable();					//m_pMain->m_bLamiStatusEnable;				//라미틀어짐 검사
	bool bCenterDistanceEnable 			= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getCenterDistanceEnable();				//m_pMain->m_bCenterDistanceEnable;			//홀 중심점 이격 검사
	bool bCenterCCDistanceEnable 		= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getCenterCCDistanceEnable();			//m_pMain->m_bCenterCCDistanceEnable;		//홀 중심점 과 CC 이격 검사
	bool bNozzleToHoleEdgeEnable 		= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getNozzleToHoleEdgeEnable();			//m_pMain->m_bCheckNozzleToPanelDistance;	//노즐과 홀 사이 검사 Nozzle Pos Error
	bool bCircleMetalRadiusCheck 		= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getCircleMetalRadiusCheck();			//m_pMain->m_bCheckCircleMetalRadius;		//홀과 메탈 지름 비교하여 메탈이 작은 경우 에러 처리
	bool bCircleMetalCenterCheck 		= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getCircleMetalCenterCheck();			//m_pMain->m_bCheckCircleMetalCenter;		//홀과 메탈 중심점 비교하여 거리가 차이가 나는 경우 에러 처리
	bool bNozzlePanelHoleCheck 			= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getNozzlePanelHoleCheck();				//m_pMain->m_bNozzleYGapInspEnable;			//패널 홀과 노즐 Y갭 측정후 예외처리
	bool bCircleFindCenterCheck 		= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getCircleFindCenterCheck(); 														//패널 홀과 카메라 중심점 거리 30픽셀 이상 예외처리
	bool bInspectionDebugCheck 			= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck();  														// 검사 디버깅 모드
	bool bCalcWetoutMetalEdge			= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getCalcWetoutMetalEdge();  															// blue 2022.09.02 CINK1+CINK2(EGL)에서 wetout 계산시 GlassEdge 대신 MetalEdge 사용
	bool bWetoutInspectionSimulation    = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getWetoutInspectionSimulation();                        //Tkyuha 221109                              //Wetout 검사 이미지 한장으로 검사

	int  iWetoutThresh 					= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getInspectionHighThresh();				//210										//Wetout 검사 Gray값
	int  iDustThresh 					= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getExistDummyGray();					//40										//이물 검사 Gray값
	int  iLiquidDropsThresh 			= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getInspDustThresh();					//40										//액튐 검사 Gray값
	int  iDispensingStatusThresh 		= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getInkCheckThresh(0);					//40										//도포된 패널 유무 검사 Gray값
	int  iLamiStatusThresh 				= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getLamiCheckThresh(0);					//60										//라미 패널 틀어짐 검사 Gray값

	double  dCenterDistanceSpec 		= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(0);						//0.03										//홀중심과 카메라 중심점 거리
	double  dCCDistanceSpec 			= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(0);						//0.35										//홀과 CC거리 가 가까우면 NG
	double  dNozzleToHoleDistanceSpec 	= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(3);						//0.05
	double  dNozzleToStageGapMinSpec 	= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(7);						//0.08										//노즐과 스테이지 최소거리
	double  dNozzleToStageGapMaxSpec 	= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(7);						//0.13	
	double  dNozzleToXYMinSpec 			= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(8);						//0.02										//노즐과 카메라 중심 최소거리 
	double  dNozzleToXYMaxSpec 			= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(8);						//0.07										//노즐과 카메라 중심 최대거리
	double  dCenterMetalDistanceSpec 	= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspHoleMetal();					//0.05										//메탈과 원 중심점 거리
	double  dNozzlePanelHoleGapThresh 	= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getExistPanelGray();					//20										//노즐과 홀 사이 갭측정 Thresh
	double  dNozzleAngle 				= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getIncludedAngleLimit();						//30										//노즐 기본 각도
	double  dCCSearchThresh 			= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getInspectionLowThresh();				//80										//CC원 찾는 기준값
    bool	bNozzleAngleSearchMode      = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getNozzleAngleSearchMode();				//Enhance/Otsu // 노즐 기본 각도 찾는 방식
	double  dNozzleToStageGapOffset     = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getNozzleToStageGapOffset();			//0	                                        //노즐과 스테이지 Offset
	double  dCenterLiveGuideLine		= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getCenterLiveViewGuideLineOffset();		//0	  										//노즐 위치 Guide Line Display용
	bool    bCenterNozzleYAlignRecalc   = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getCenterNozzleYAlignRecalc(); 			//0											//Nozzle Align시 Mark Search이후 Y값 2진화 자동검사 모드 사용유무
	int     iCenterNozzleToPanelDistance = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getCenterNozzleToPanelDistanceLight(); 	//150		
	bool	bCenterNozzleYLightEnable	= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getUseCenterNozzleYLight();
	int		nNozzleYGapThreshold		= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getNozzleYGapThreshold();				//20
	
	// KBJ 2022-09-05 Rotate 파라미터 추가
	double	dRotateSpec					= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getRotateC_Spec();				        //20
	int		nRotateBinaryValue			= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getRotateC_binary_value();				//20
	int		nRotateLightValue			= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getRotateC_light_value();				//20

	// WETOUT 검사 파라미터
	double  dWetoutInMin 				= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSubSpec(0);						//0.15
	double  dWetoutInMax 				= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(0);			//0.35;
	double  dWetoutOutMin 				= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSubSpec(1);						//0;
	double  dWetoutOutMax 				= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(1);			//0.03;
	double  dCircleLength 				= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getInspRangeLength();							//15.0;
	int		iCCFindFilter				= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getCCFindFilter();						//30~65;
	double	iMetalOverflowMargin		= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getMetalOverflowMargin();				//20~50;
	bool	bTestImageSave				= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getTestImageSave();						//FALSE
	double	dLineLengthMinThick         = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getLineLengthMinThick();
	double	dLineLengthSpec             = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getLineLengthSpec();
	double	dLineLengthFDC_Distance     = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getLineLengthFDC_Distance();
	//SJB 2022-11-11 Overflow Min 파라미터 추가
	double	dLineOverflowMinSpec		= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getLineOverflowMinSpec();
	//SJB 2022-11-19 CG Exposure Time 파라미터 추가
	int		iCGGrabExposure				= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getCGGrabExposure();
	double	dCamCenterToCG              = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getCamCenterToCG();
	double	dInkUnderflowSpec           = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getInkUnderflowSpec();
	//HTK 2022-03-16 DummyInspSpec 추가
	double	dDummyMinSizeSpec           = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getDummyMinSizeSpec();
	//KJH 2022-04-29 MincountCheckInsp 추가
	//KJH 2022-05-09 Mincount 상시 사용으로 변경
	//bool	bMinCountCheckInspMode		= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getMinCountCheckInspModeEnable();
	int		iMinCountCheckInspContinue	= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getMinCountCheckInspContinueCount();
	int		iMinCountCheckInspTotal		= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getMinCountCheckInspTotalCount();
	double	dCInkMinWidthSpec			= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getCInkMinWidthSpec();
	bool	bWetoutRJudgeModeEnable		= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getWetoutRJudgeModeEnable();
	double	dWetoutRMinSpec             = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getWetoutRMinSpec();
	double	dWetoutRMaxSpec				= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getWetoutRMaxSpec();
	//KJH 2022-05-25 BM In <-> Cink In Insp 추가
	bool	bWetoutBJudgeModeEnable		= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getWetoutBJudgeModeEnable();
	double	dWetoutBMinSpec				= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getWetoutBMinSpec();
	double	dWetoutBMaxSpec				= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getWetoutBMaxSpec();
	bool    bWetoutJudgeEnable          = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getWetoutMinMaxJudgeModeEnable();		//Wetout Min, Max 관련 Judge 사용유무

	// blue 2022.09.09 Diff image 두개로 나뉘는 증상때문에 추가 (F=default, T=전무님코드사용)
	bool    bDiffRoiMerge = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getDiffRoiMergeEnable();

	// YCS 2022-12-01 Wetout_ASPC 스펙
	double dWetout_ASPC_Spec = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getWetoutASPCSpec();

	// SJB 2023-01-02 CG to Start, End 파라미터 추가
	double dCgToStartSpec = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getCgToStartSpec();
	double dCgToEndSpec = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getCgToEndSpec();
	double dCgToStartEndJudgeArea = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getCGJudgeArea();	

	// 이물검사 파라미터
	double  dDustSizeMin 				= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(2);
	double  dDustSizeMax 				= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(2);
	double  dDustLineRatio 				= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSubSpec(2);
	double  dDustSpecMin 				= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(2);
	double  dDustSpecMax 				= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(2);

	// 액튐검사 파라미터
	double  dDropsSizeInMin 			= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(1);						//0.15;
	double  dDropsSizeInMax 			= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(1);						//1;
	double  dDropsLineInRatio 			= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSubSpec(3);						//2.5
	double  dDropsSizeOutMin 			= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(4);						//0.5;
	double  dDropsSizeOutMax 			= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(4);						//1;
	double  dDropsLineOutRatio 			= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(3);			//2.0
	double  dDropsSpecMin 				= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(5);						//0.1;
	double  dDropsSpecMax 				= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(5);						//1.0;	
	double  dDropsAfterImageRemove 		= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(6);						//80;										//잔상 제거 파라미터 gray
	double  dFeriAlignFindThresh        = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFeriAlignFindThresh();
	bool    bFeriAlignFindPreProc       = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFeriAlignFindPreProcFlag();          //FALSE
	
		// Grab Delay 파라미터
	int iDustInspGrabDelay 				= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getDustInspGrabDelay();
	int iDropInspGrabDelay 				= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getDropInspGrabDelay();
	int iCenterAlignGrabDelay 			= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getCenterAlignGrabDelay();
	int iNozzleAlignGrabDelay 			= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getNozzleAlignGrabDelay();

	// Adaptive Dispensing 파라미터
	bool bAdaptiveDispensingFlag		= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getAdaptiveDispensingFlag();			// 사용 /미사용 선택
	double  dBaseOffset					= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getAdaptiveDispensingBaseOffset();		// Base Offset (0)
	double  dScaleOffset				= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getAdaptiveDispensingScaleOffset();		// Scale Offset (1)
	double  dDispensingRange            = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getAdaptiveDispensingRange();           // 도포 범위(90도)

	// Dispensing Mode
	bool bELBCoverCInkModeEnable			= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getELBCoverCInkMode();		
	bool bELBCG_Edge_CalcModeEnable			= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getELBCG_Edge_CalcModeEnable();	//false		
	bool bELBPartial_DispensingModeEnable	= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getELBPartial_DispensingModeEnable();	//false	
	bool bLineShift_DispensingModeEnable	= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getLineShift_DispensingModeEnable();	//false	
	bool bCircle_ShapeDispensingModeEnable = m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getUseDummyCornerInsp();  // false : Y축 사용모드  //22. 06.02
	bool bLineShift_DoubleDispensingModeEnable = m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getUseDoubleDispensingMode();  // false : 
	bool bLineOverflowDispMode             = m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getLineOverflowDispMode();  // false : 
	bool bCircle_Cink1_Cink2MergeMode = m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getCircle_Cink1_Cink2MergeMode();  // false : 
	bool bLine_InspWithTraceResultMode = m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getLine_InspWithTraceResultMode();  // false : 

	// 원찾기 파라미터 설정들
	bool bMetalSearchRemoveNoiseFlag	= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getMetalSearchRemoveNoiseFlag(); // 메탈 전처리 사용 /미사용 선택 (희린 영상은 미사용으로 해야함)
	bool bTraceUsingAlignFlag			= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getTraceUsingAlignFlag(); // 얼라인 값으로 Trace 계산 모드 활성화
	bool bSearchElbEndPosFlag			= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getSearchElbEndPosFlag();
	bool bELBInsp_ShiftSearchEnable		= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getELBInsp_ShiftSearchEnable();	//false	
	bool bAVI_Save_ImageTypeEnable		= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getAVI_Save_ImageTypeEnable();	//false	
	
	//KJH 2022-04-09 PN 반지름 비교 검사 추가
	bool	bPNSizeInspEnable			= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getPNSizeInspEnable();
	double	dPNSizeSpec					= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getPNSizeSpec();
	double	dPNSizeSpecTorr				= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getPNSizeSpecTorr();

	// KBJ 2022-12-05 LinePocketSize 추가
	double dELB_Line_Pocket_Size		= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getELB_Line_Pocket_Size();
	// YCS 2022-12-24 PN To Metal 측정모드 파라미터 추가
	bool bLine_PanelToMetalCalcMode		= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getPanelToMetalCalcEnable();

    // 20211230 Tkyuha 코멘트 기능 추가 마지막 인자에 주석으로 사용 하면 됨
	CBBungGrid::stProperty GRID_STUFF[] =
	{
		P_ROOT(_T("TEXT:Center Camera Inspection;")),
			P_SUBROOT(_T("TEXT:Wetout Inspection;")),
			    P_VAR(bWetoutEnable,				_T("TEXT:Wetout;"),							_T("Enable / Disable")),
				P_VAR(iWetoutThresh,				_T("TEXT:Wetout Thresh;"),					_T("Range 200~255 Gray")),
				P_VAR(dWetoutInMin ,				_T("TEXT:Wetout In Min;"),					_T("0 ~ 0.150 mm")),
				P_VAR(dWetoutInMax ,				_T("TEXT:Wetout In Max;"),					_T("0 ~ 0.350 mm")),
				P_VAR(dWetoutOutMin,				_T("TEXT:Wetout Out Min;"),					_T("0 ~ 0.01 mm")),
				P_VAR(dWetoutOutMax,				_T("TEXT:Wetout Out Max;"),					_T("0 ~ 0.01 mm")),
				P_VAR(dCircleLength,				_T("TEXT:Circle Length;"),					_T("14 ~ 16mm")),
				P_VAR(iCCFindFilter,				_T("TEXT:CC Filter Threshold;"),			_T("Range 30~60 Gray")),
				P_VAR(iMetalOverflowMargin,			_T("TEXT:Metal Overflow Margin;"),			_T("Range 0.05~0.2 mm")),
				P_VAR(bTestImageSave,				_T("TEXT:Test Image Save;"),				_T("Enable / Disable")),
				P_VAR(dLineLengthMinThick,			_T("TEXT:LineLengthMinThick;"),			    _T("0.0   ~ 0.150 mm")),
				P_VAR(dLineLengthSpec,			    _T("TEXT:LineLengthSpec;"),			        _T("2.5   ~ 6 mm")),
				P_VAR(dCamCenterToCG,			    _T("TEXT:CamCenterToCG;"),			        _T("33.565   ~ 4 mm")),
				P_VAR(dInkUnderflowSpec,		    _T("TEXT:InkUnderflowSpec;"),			    _T("0.1   ~ 0.2 mm")),
				P_VAR(dDummyMinSizeSpec,		    _T("TEXT:DummyMinSizeSpec;"),			    _T("0.1   ~ 1 mm")),
				//KJH 2022-04-29 MincountCheckInsp 추가
				//KJH 2022-05-09 Mincount 상시 사용으로 변경
				//P_VAR(bMinCountCheckInspMode,		_T("TEXT:MinSpecOutCheckInspMode;"),		_T("[if use to Metal Mode , Must use ELB_ENDPOS Mode] Enable / Disable")),
				P_VAR(iMinCountCheckInspContinue,	_T("TEXT:MinSpecOut ContinueCount Spec;"),	_T("3 ~ 5")),
				P_VAR(iMinCountCheckInspTotal,		_T("TEXT:MinSpecOut TotalCount Spec;"),		_T("100  ~  120")),
				P_VAR(dCInkMinWidthSpec,			_T("TEXT:CInkWetout Min Spec;"),			_T("0.070 ~ 0.150 mm")),
				P_VAR(bWetoutRJudgeModeEnable,		_T("TEXT:Wetout R Judge Mode;"),			_T("Enable / Disable")),
				P_VAR(dWetoutRMinSpec,			    _T("TEXT:CInkWetout R Min Spec;"),			_T("0.070 ~ 0.150 mm")),
				P_VAR(dWetoutRMaxSpec,			    _T("TEXT:CInkWetout R Max Spec;"),			_T("0.15 ~ 0.350 mm")),
				P_VAR(bWetoutBJudgeModeEnable,		_T("TEXT:Wetout B Judge Mode;"),			_T("Enable / Disable")),
				P_VAR(dWetoutBMinSpec,			    _T("TEXT:CInkWetout B Min Spec;"),			_T("0.030 ~ 0.035 mm")),
				P_VAR(dWetoutBMaxSpec,			    _T("TEXT:CInkWetout B Max Spec;"),			_T("0.050 ~ 0.055 mm")),
				P_VAR(bWetoutJudgeEnable,			_T("TEXT:Wetout JudgeEnable;"),				_T("Enable / Disable\n*On Cink2 EQP, this was dismissed")), // YCS 2022-12-02 파라미터 설명 수정
				P_VAR(dLineLengthFDC_Distance,		_T("TEXT:Line FDC Distance;"),			    _T("0.1   ~ 0.2 mm")),
				P_VAR(dLineOverflowMinSpec,			_T("TEXT:Line Overflow Min Spec;"),			_T("0.2mm")),
				P_VAR(iCGGrabExposure,				_T("TEXT:CG Grab Exposure;"),				_T("0:Not Use, 0 ~ 200(x100)")),
				P_VAR(bDiffRoiMerge,				_T("TEXT:DiffROI Merge(Line);"),					_T("Enable / Disable")),	// blue 2022.09.09
				P_VAR(bCalcWetoutMetalEdge,			_T("TEXT:CalcWetout Metal Edge(CINK1+CINK2);"),	_T("Enable / Disable")),
				P_VAR(bWetoutInspectionSimulation,	_T("TEXT:Wetout Inspection Simulation;"),	_T("Enable / Disable")),	//Tkyuha 221109	
				P_VAR(dWetout_ASPC_Spec,			_T("TEXT:Wetout_ASPC Spec;"),					_T("")),			// YCS 2022-12-01 Wetout_ASPC 스펙
				P_VAR(dELB_Line_Pocket_Size,		_T("TEXT:Line Pocket Size(mm);"),				_T("mm")),			// YCS 2022-12-01 Wetout_ASPC 스펙
				P_VAR(dCgToStartSpec,				_T("TEXT:CG to Start Spec(mm);"),				_T("mm")),			// SJB 2023-01-02 CG to Start, End 파라미터 추가
				P_VAR(dCgToEndSpec,					_T("TEXT:CG to End Spec(mm);"),					_T("mm")),			// SJB 2023-01-02 CG to Start, End 파라미터 추가
				P_VAR(dCgToStartEndJudgeArea,		_T("TEXT:CG Judge Area(mm);"),				_T("mm")),				// JCY 2023-01-12 CG Judge Area 파라미터 추가
			P_SUBROOT_END(),
			P_SUBROOT(_T("TEXT:Dust Inspection;")),
				P_VAR(bDustEnable,					_T("TEXT:Dust;"),							_T("Enable / Disable")),
				P_VAR(iDustThresh,					_T("TEXT:Dust Thresh;"),					_T("20 ~ 60 Gray(40)")),
				P_VAR(dDustSizeMin,					_T("TEXT:Dust Size Min;"),					_T("0.1 ~ 0.5 mm(0.2)")),
				P_VAR(dDustSizeMax,					_T("TEXT:Dust Size Max;"),					_T("1 ~ 5 mm(5)")),
				P_VAR(dDustLineRatio,				_T("TEXT:Line Ratio;"),						_T("1~4(2.5)")),
				P_VAR(dDustSpecMin,					_T("TEXT:Dust Spec Min;"),					_T("0.1 ~ 0.5 mm(0.2)")),
				P_VAR(dDustSpecMax,					_T("TEXT:Dust Spec Max;"),					_T("1 ~ 5 mm(5)")),
			P_SUBROOT_END(),
			P_SUBROOT(_T("TEXT:Liquid Drops Inspection;")),
				P_VAR(bLiquidDropsEnable,			_T("TEXT:Liquid Drops;"),					_T("Enable / Disable")),
				P_VAR(iLiquidDropsThresh,			_T("TEXT:Drops Thresh;"),					_T("20 ~ 60 Gray (40)")),
				P_VAR(dDropsSizeInMin ,				_T("TEXT:Drops Size In Min;"),				_T("0.1 ~ 0.5 mm(0.1)")),
				P_VAR(dDropsSizeInMax ,				_T("TEXT:Drops Size In Max;"),				_T("1 ~ 5 mm(5)")),
				P_VAR(dDropsLineInRatio ,			_T("TEXT:Line In Ratio;"),					_T("1~4(2.5)")),
				P_VAR(dDropsSizeOutMin ,			_T("TEXT:Drops Size Out Min;"),				_T("0.1 ~ 0.5 mm(0.1)")),
				P_VAR(dDropsSizeOutMax ,			_T("TEXT:Drops Size Out Max;"),				_T("1 ~ 5 mm(5)")),
				P_VAR(dDropsLineOutRatio,			_T("TEXT:Line Out Ratio;"),					_T("1~4(2.5)")),
				P_VAR(dDropsSpecMin ,				_T("TEXT:Drops Spec Min;"),					_T("0.1 ~ 0.5 mm(0.1)")),
				P_VAR(dDropsSpecMax ,				_T("TEXT:Drops Spec Max;"),					_T("1 ~ 5 mm(2)")),
				P_VAR(dDropsAfterImageRemove ,		_T("TEXT:AfterImage Remove Threh;"),		_T("60 ~ 100 Gray(80)")),
				P_VAR(dFeriAlignFindThresh ,	   	_T("TEXT:Feri Align Find Threh;"),		    _T("60 ~ 100 Gray(80)")),
				P_VAR(bFeriAlignFindPreProc ,	   	_T("TEXT:Feri Align Find Pre Processing;"),	_T("Enable / Disable")),				
			P_SUBROOT_END(),
			P_SUBROOT(_T("TEXT:Dispesing Status Inspection;")),
				P_VAR(bDispensingStatusEnable,		_T("TEXT:Dispesing Status;"),				_T("Enable / Disable")),
				P_VAR(iDispensingStatusThresh,		_T("TEXT:Dispesing Status Thresh;"),		_T("20 ~ 60 Gray (40)")),
			P_SUBROOT_END(),
			P_SUBROOT(_T("TEXT:Lami Status Inspection;")),
				P_VAR(bLamiStatusEnable,			_T("TEXT:Lami Status;"),					_T("Enable / Disable")),
				P_VAR(iLamiStatusThresh,			_T("TEXT:Lami Status Thresh;"),				_T("40 ~ 80 Gray (60)")),
			P_SUBROOT_END(),
			P_SUBROOT(_T("TEXT:Center Distance Inspection;")),
				P_VAR(bCenterDistanceEnable,		_T("TEXT: PN Center Distance;"),			_T("PN <-> Cam Center Distance Insp [Enable / Disable]")),
				P_VAR(bCenterCCDistanceEnable,		_T("TEXT:CC Distance;"),					_T("Enable / Disable")),
				P_VAR(dCenterDistanceSpec,			_T("TEXT:Center Distance Spec;"),			_T("0.01 ~ 0.05 mm(0.03)")),
				P_VAR(dCCDistanceSpec,				_T("TEXT:CC Distance Spec;"),				_T("0.25 ~ 0.45 mm(0.35)")),
				P_VAR(dCCSearchThresh,				_T("TEXT:CC Search Thresh;"),				_T("60 ~ 100 Gray(80)")),
				P_VAR(bPNSizeInspEnable,			_T("TEXT:PN Size Insp Enable;"),			_T("PN Size Insp [Enable / Disable]")),
				P_VAR(dPNSizeSpec,					_T("TEXT:PN Size Spec;"),					_T("PN : 2.35mm")),
				P_VAR(dPNSizeSpecTorr,				_T("TEXT:PN Size Specm Torr;"),				_T("PN : 0.05")),
			P_SUBROOT_END(),
			P_SUBROOT(_T("TEXT:RoateCenter;")),
				P_VAR(dRotateSpec,					_T("TEXT:Rotate Tolerance;"),				_T("0.005mm ~ 0.030mm")),
				P_VAR(nRotateBinaryValue,			_T("TEXT:Rotate Binary Threshhold;"),		_T("0:Not Use, 60 ~ 120")),
				P_VAR(nRotateLightValue,			_T("TEXT:Rotate Light Value;"),				_T("0:Not Use, 30 ~ 120")),
			P_SUBROOT_END(),
			P_SUBROOT(_T("TEXT:Nozzle Y Check Inspection;")),
				P_VAR(bNozzlePanelHoleCheck,		_T("TEXT:Nozzle Y Gap Insp;"),				_T("Enable / Disable")),
				P_VAR(dNozzleToXYMinSpec,			_T("TEXT:Nozzle Y Gap Min Spec;"),			_T("0.01 ~ 0.03 mm(0.02)")),
				P_VAR(dNozzleToXYMaxSpec,			_T("TEXT:Nozzle Y Gap Max Spec;"),			_T("0.06 ~ 0.08 mm(0.07)")),
				P_VAR(bCenterNozzleYLightEnable,	_T("TEXT:Nozzle Y Gap Light;"),				_T("Enable / Disable")),
				P_VAR(nNozzleYGapThreshold,			_T("TEXT:Nozzle Y Gap Threshold;"),			_T("0:Auto / 10 ~ 100")),
				P_VAR(dCenterLiveGuideLine,			_T("TEXT:Center Live VIew Guide Line;"),    _T("0 = Disable")),
			P_SUBROOT_END(),
			P_SUBROOT(_T("TEXT:Inspection Flag;")),
				P_VAR(bCircleMetalRadiusCheck,		_T("TEXT:CircleMetal Radius Check;"),		_T("Enable / Disable")),
				P_VAR(bCircleMetalCenterCheck,		_T("TEXT:CircleMetal Center Check;"),		_T("Enable / Disable")),
				P_VAR(dCenterMetalDistanceSpec,		_T("TEXT:CircleMetal DistSpec;"),			_T("0.03 ~ 0.07mm(0.05)")),
				P_VAR(bCircleFindCenterCheck,		_T("TEXT:CircleFindError Check;"),			_T("Enable (30pixel)/ Disable")),
				P_VAR(bInspectionDebugCheck,		_T("TEXT:Inspection Debug Mode Check;")	,	_T("Enable / Disable")),
			P_SUBROOT_END(),
			P_SUBROOT(_T("TEXT:Camera Grab Delay ;")),
				P_VAR(iDustInspGrabDelay,			_T("TEXT:Dust Inspection Grab;"),			_T("0 ~ 2000 ms")),
				P_VAR(iDropInspGrabDelay,			_T("TEXT:Drop Inspection Grab;"),			_T("0 ~ 2000 ms")),
				P_VAR(iCenterAlignGrabDelay,		_T("TEXT:Center Align Grab;"),				_T("0 ~ 2000 ms")),
				P_VAR(iNozzleAlignGrabDelay,		_T("TEXT:Nozzle Align Grab;"),				_T("0 ~ 2000 ms")),
			P_SUBROOT_END(),
			P_SUBROOT(_T("TEXT:Adaptive Dispensing ;")),
				P_VAR(bAdaptiveDispensingFlag,		_T("TEXT:Adaptive Enable;"),				_T("Enable / Disable")),
				P_VAR(dBaseOffset,					_T("TEXT:Base Offset;"),					_T("0 ~ 200mm(0)")),
				P_VAR(dScaleOffset,					_T("TEXT:Scale Offset;"),					_T("-2 ~ 2(1)")),
				P_VAR(dDispensingRange,				_T("TEXT:Disnpensing Range;"),				_T("0 ~ 360(90)Degree")),
			P_SUBROOT_END(),
			P_SUBROOT(_T("TEXT:Dispensing Mode ;")),
				P_VAR(bELBCoverCInkModeEnable,			_T("TEXT:ELB Cover C-INK Enable;"),				_T("Enable / Disable")),
			    P_VAR(bMetalSearchRemoveNoiseFlag,		_T("TEXT:Metal PreProcessing Enable;"),			_T("Metal Search PreProcessing Enable / Disable")),
				P_VAR(bTraceUsingAlignFlag,				_T("TEXT:Trace Using Align Value;"),			_T("Trace Using Align Value Enable / Disable")),
		        //P_VAR(bSearchElbEndPosFlag,				_T("TEXT:CINK Insp ElbEndPos;"),				_T("CINK Insp Elb EndPos search Enable / Disable")),
				P_VAR(bELBCG_Edge_CalcModeEnable,		_T("TEXT:CINK CG_Edge_CalcMode;"),				_T("CINK Insp CG_Edge_Calc search Enable / Disable")),
				P_VAR(bELBPartial_DispensingModeEnable, _T("TEXT:Partial_DispensingMode;"),				_T("CINK Partial Dispensing Mode Enable / Disable")),
				P_VAR(bELBInsp_ShiftSearchEnable,		_T("TEXT:ELBInsp_Shift Search Enable;"),		_T("ELB Insp_ShiftSearch Enable / Disable")),
				P_VAR(bAVI_Save_ImageTypeEnable,		_T("TEXT:AVI Save ImageType Enable;"),			_T("AVI Save ImageType Enable / Disable")),
				P_VAR(bLineShift_DispensingModeEnable,	_T("TEXT:Line Shift Dispensing ModeEnable;"),	_T("Line Shift Dispensing Enable / Disable")),
				P_VAR(bCircle_ShapeDispensingModeEnable, _T("TEXT:Circle_ShapeDispensingModeEnable;"),  _T("Circle Shape Dispensing Enable / Disable")), //22. 06.02
				P_VAR(bLineShift_DoubleDispensingModeEnable, _T("TEXT:Line Shift Double Dispensing ModeEnable;"), _T("Line Shift Double Dispensing Enable / Disable")),		
				P_VAR(bLineOverflowDispMode,            _T("TEXT:Line OverflowDispMode;"), _T("Line OverflowDispMode Enable / Disable")),
				P_VAR(bCircle_Cink1_Cink2MergeMode,     _T("TEXT:Circle_Cink1_Cink2 MergeMode;"), _T("Circle Cink1_Cink2 MergeMode Enable / Disable")),
				P_VAR(bLine_InspWithTraceResultMode, _T("TEXT:Line InspWithTraceResult Mode;"), _T("Line InspWithTraceResult Mode Enable / Disable")),
				P_VAR(bLine_PanelToMetalCalcMode, _T("TEXT:Line_PanelToMetalCalcMode;"), _T("Enable / Disable")), // YCS 2022-12-24 PN To Metal 측정모드 파라미터 추가
					P_SUBROOT_END(),
		P_ROOT_END(),
	}; 
	int nSize = sizeof(GRID_STUFF) / sizeof(CBBungGrid::stProperty);

	m_ctlPropGrid.Initial(&m_font);
	m_ctlPropGrid.AppendProperty(GRID_STUFF, nSize);
}

void CGridInspectionParamDlg::InitNozzleAlign()
{
	// hsj 2023-01-03 grid갱신
	if (m_pMain == NULL) return;

	double  dNozzleXYZInspMinSpec_Z		= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(7);				//Nozzle XYZ Insp Z Min Spec							  //0.08				//노즐과 스테이지 최소거리
	double  dNozzleXYZInspMaxSpec_Z		= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(7);				//Nozzle XYZ Insp Z Max Spec							  //0.13				//노즐과 스테이지 최대거리
	double  dNozzleXYZInspMinSpec_XY	= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(8);				//Nozzle XYZ Insp XY Min Spec							  //0.02				//노즐과 카메라 중심 최소거리
	double  dNozzleXYZInspMaxSpec_XY	= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(8);				//Nozzle XYZ Insp XY Max Spec							  //0.07				//노즐과 카메라 중심 최대거리
	double  dNozzleAngle				= m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getIncludedAngleLimit();				//Nozzle Align T Base Value								  //30					//노즐 기본 각도
	bool	bNozzleAngleSearchMode		= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getNozzleAngleSearchMode();		//Nozzle Search Methode Select [Otsu / Enhance]			  //Enhance/Otsu		//노즐 기본 각도 찾는 방식
	int		nNozzleAlignGrabDelay		= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getNozzleAlignGrabDelay();		//Nozzle Camera Grab Delay [미구현]
	bool    bCenterNozzleYAlignRecalc   = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getCenterNozzleYAlignRecalc(); 	//Nozzle Align
	bool    bCenterNozzleAlignMethod    = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getCenterNozzleAlignMethod(); 	//Nozzle Align Method
	bool    bSizdNozzleZAlignRecalc		= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getSideNozzleZAlignRecalc(); 	//Nozzle Align Method
	//SJB 2022-12-19 Nozzle Tip Size 파라미터 추가
	double	dNozzleTipSizeSpec			= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getNozzleTipSizeSpec();
	double	dNozzleTipSizeSpecTolerance = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getNozzleTipSizeSpecTolerance();
	// 20211230 Tkyuha 코멘트 기능 추가 마지막 인자에 주석으로 사용 하면 됨																														
	CBBungGrid::stProperty GRID_STUFF[] =																																						
	{																																															
		P_ROOT(_T("TEXT:Nozzle Camera;")),																																						
			P_SUBROOT(_T("TEXT:Nozzle Check Inspection;")),																																		
				P_VAR(dNozzleXYZInspMinSpec_Z,		_T("TEXT:Nozzle Z Min Spec;"),				_T("0 ~ -0.050um / Nozzle XYZ Insp : Spec")),
				P_VAR(dNozzleXYZInspMaxSpec_Z,		_T("TEXT:Nozzle Z Max Spec;"),				_T("0 ~ +0.050um / Nozzle XYZ Insp : Spec")),
				P_VAR(dNozzleXYZInspMinSpec_XY,		_T("TEXT:Nozzle XY Min Spec;"),				_T("0 ~ -0.050um / Nozzle XYZ Insp : Spec")),
				P_VAR(dNozzleXYZInspMaxSpec_XY,		_T("TEXT:Nozzle XY Max Spec;"),				_T("0 ~ +0.050um / Nozzle XYZ Insp : Spec")),
				P_VAR(dNozzleAngle,					_T("TEXT:Nozzle Angle;"),					_T("Default : 0 [ Not Use ] / 30 ~ 35 [Target Angle]")),
				P_VAR(bNozzleAngleSearchMode,		_T("TEXT:Nozzle Angle Search Mode;"),       _T("Otsu[False] / Enhance [True]")),
				P_VAR(nNozzleAlignGrabDelay,		_T("TEXT:Nozzle Camera Grab Delay;"),       _T("0 ~ 2000 (ms) [미구현 / Not Use]")),
				P_VAR(bCenterNozzleYAlignRecalc,	_T("TEXT:Center NozzleY AlignRecalc;"),      _T("Enable / Disable")),
				P_VAR(bCenterNozzleAlignMethod,	    _T("TEXT:Center Nozzle Align Method;"),     _T("Matching / Line")),
				P_VAR(bSizdNozzleZAlignRecalc,	    _T("TEXT:Side Nozzle Align Recalc;"),		_T("Enable / Disable")),
				P_VAR(dNozzleTipSizeSpec,			_T("TEXT:Nozzle Tip Size Spec;"),			_T("100 = 0.150, 150 = 0.200")),	//SJB 2022-12-19 Nozzle Tip Size 파라미터 추가
				P_VAR(dNozzleTipSizeSpecTolerance,	_T("TEXT:Nozzle Tip Size Spec Tolerance;"), _T("0.01 ~ 0.02")),
			P_SUBROOT_END(),
		P_ROOT_END(),
	};
	int nSize = sizeof(GRID_STUFF) / sizeof(CBBungGrid::stProperty);

	m_ctlPropGrid.Initial(&m_font);
	m_ctlPropGrid.AppendProperty(GRID_STUFF, nSize);
}
	
void CGridInspectionParamDlg::InitZGapInsp()
{
	// hsj 2023-01-03 grid갱신
	if (m_pMain == NULL) return;

	double  dNozzleToStageGapMinSpec = m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(7);			//Nozzle Z Gap Insp Z Min Torr Spec						//0.08		//노즐과 스테이지 최소거리
	double  dNozzleToStageGapMaxSpec = m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(7);			//Nozzle Z Gap Insp Z Max Torr Spec						//0.13		//노즐과 스테이지 최대거리
	double  dNozzleToStageGapOffset	 = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getNozzleToStageGapOffset();	//Nozzle Z Gap Camera Center To Nozzle Offset Value		//0			//노즐과 스테이지 Offset
	int		nNozzleAlignGrabDelay	 = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getNozzleAlignGrabDelay();		//Nozzle Z Gap Camera Grab Delay [미구현]
	bool    bCenterNozzleYAlignRecalc = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getCenterNozzleYAlignRecalc();//Nozzle Align시 Mark Search이후 Y값 2진화 자동검사 모드 사용유무
	bool    bCenterNozzleAlignMethod = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getCenterNozzleAlignMethod(); 	//Nozzle Align Method

	// 20211230 Tkyuha 코멘트 기능 추가 마지막 인자에 주석으로 사용 하면 됨																														
	CBBungGrid::stProperty GRID_STUFF[] =
	{
		P_ROOT(_T("TEXT:Z Gap Camera;")),
			P_SUBROOT(_T("TEXT:Nozzle Z Gap Inspection;")),
				P_VAR(dNozzleToStageGapMinSpec,		_T("TEXT:Nozzle Z Min Torr;"),				_T("0 ~ -0.050um / Nozzle XYZ Insp : Spec")),
				P_VAR(dNozzleToStageGapMaxSpec,		_T("TEXT:Nozzle Z Max Torr;"),				_T("0 ~ +0.050um / Nozzle XYZ Insp : Spec")),
				P_VAR(dNozzleToStageGapOffset,		_T("TEXT:Nozzle Z Gap Offset;"),			_T("0.00 ~ 0.2mm (0.00)")),
				P_VAR(nNozzleAlignGrabDelay,		_T("TEXT:Nozzle Z Camera Grab Delay;"),       _T("0 ~ 2000 (ms)")),
				P_VAR(bCenterNozzleYAlignRecalc,	_T("TEXT:Nozzle Z Insp Recalc;"),      _T("Enable / Disable")),
				P_VAR(bCenterNozzleAlignMethod,	    _T("TEXT:Nozzle Z Insp Method;"),     _T("Matching / Line")),
			P_SUBROOT_END(),
		P_ROOT_END(),
	};
	int nSize = sizeof(GRID_STUFF) / sizeof(CBBungGrid::stProperty);

	m_ctlPropGrid.Initial(&m_font);
	m_ctlPropGrid.AppendProperty(GRID_STUFF, nSize);
}
//HTK 2022-07-11 Nozzle#45 검사기능 추가로 인한 모델파라미터 생성
void CGridInspectionParamDlg::InitViewSubInspection()
{
	// hsj 2023-01-03 grid갱신
	if (m_pMain == NULL) return;

	double	dOtsuThresh = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().get45DegreeInkInspThresh();		
	int  	nStartDelay = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().get45DegreenStartDelay();
	int  	nIntervalDelay = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().get45DegreenIntervalDelay();
	bool    bEnableInspection = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().get45DegreeInkInspEnable();

	// 20211230 Tkyuha 코멘트 기능 추가 마지막 인자에 주석으로 사용 하면 됨																														
	CBBungGrid::stProperty GRID_STUFF[] =
	{
		P_ROOT(_T("TEXT:45Degree Camera;")),
			P_SUBROOT(_T("TEXT:45Degree Inspection;")),
				P_VAR(dOtsuThresh,		_T("TEXT:Check Bright;"),       _T("0 ~ 255 (gray)")),
				P_VAR(nStartDelay,		_T("TEXT:StartDelay;"),       _T("0 ~ 2000 (ms)")),
				P_VAR(nIntervalDelay,		_T("TEXT:IntervalDelay;"),       _T("0 ~ 1000 (ms)")),
				P_VAR(bEnableInspection,	_T("TEXT:Ink Inspection;"),      _T("Enable / Disable")),
			P_SUBROOT_END(),
		P_ROOT_END(),
	};
	int nSize = sizeof(GRID_STUFF) / sizeof(CBBungGrid::stProperty);

	m_ctlPropGrid.Initial(&m_font);
	m_ctlPropGrid.AppendProperty(GRID_STUFF, nSize);
}


//KJH2 2022-08-23 Attach Align(PC7), Film Inspection(PC8) 파라미터 분리
void CGridInspectionParamDlg::InitFilmInspection()
{
	// hsj 2023-01-03 grid갱신
	if (m_pMain == NULL) return;

	//HSJ 2022-01-07 필름검사 Method 파라미터 추가
	int spec_method = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmInspMethod();

	double spec_lx = m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSubSpec(0);
	double spec_lx_tor = m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(0);
	double spec_rx = m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSubSpec(1);
	double spec_rx_tor = m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(1);

	double spec_ly = m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSubSpec(2);
	double spec_ly_tor = m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(2);
	double spec_ry = m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSubSpec(3);
	double spec_ry_tor = m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(3);

	bool bPanel = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getPanelCaliperEnable();
	bool bFilm = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmCaliperEnable();
	
	//hsj 2022-01-27 필름검사 scale 파라미터 추가
	bool bScale = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmScaleEnable();
	double dLx_scale= m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmLXScale();
	double dRx_scale = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmRXScale();
	double dLy_scale = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmLYScale();
	double dRy_scale = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmRYScale();

	// hsj 2022-02-07 필름검사 L check 기능 추가
	bool bPanelLcheck = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getPanelLcheckEnable();
	double dbPanel_Lcheck = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getPanelLcheckLength();
	double dbPanel_Lcheck_Torr = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getPanelLcheckTorr();
	bool bFilmLcheck = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmLcheckEnable();
	double dbFilm_Lcheck = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmLcheckLength();
	double dbFilm_Lcheck_Torr = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmLcheckTorr();
	//bool bFilmReAttachCheck = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmReAttachCheckEnable();
	//int bFilmReAttachCheckThresh = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmReAttachCheckThresh();
	//bool bFilmUVCheck = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmUVCheckEnable();
	//int bFilmUVCheckThresh = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmUVCheckThresh();
	//int bFilmUVCheckSpec = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmUVCheckSpec();

	// hsj 2022-02-07 필름검사 threshold 기능 추가
	bool bUsePanelThresh = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getUsePanelThresh_Auto();
	int nPanelThresh = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getPanelThresh();

	//phb 2022-04-15 필름검사 fixture 기능 추가
	//bool bUseFixtureMark = m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getUseFixtureMark();
	bool bUseReverseImage = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getUseFilmReversePosition();

	// Grab Delay 파라미터
	int iFilmInspGrabDelay = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmInspGrabDelay();

	// 예외 스펙 추가
	double dFilnInspExceptionSpec = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmInspExceptionSpec();
	int nFilnInspExceptionCount = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmInspExceptionCount();


	CBBungGrid::stProperty GRID_STUFF[] =
	{
		P_ROOT(_T("TEXT:Film Attach Inspection;")),
			P_SUBROOT(_T("TEXT:Film Search Method;")),
				P_VAR(bPanel, _T("TEXT:Panel find Use Caliper;"),									_T("False : Matching, True : caliper")),
				P_VAR(bFilm, _T("TEXT:Film find Use Caliper;"),										_T("False : Matching, True : caliper")),
				P_VAR(bUsePanelThresh, _T("TEXT:Use Auto Thresh;"),									_T("False : Not Use, True : Use")),
				P_VAR(nPanelThresh, _T("TEXT:Panel Thresh;"),										_T("Range 200~255 Gray")),
				//P_VAR(bUseFixtureMark, _T("TEXT:Use Fixture Mode;"),								_T("False : Not Use, True : Use")),
				P_VAR(bUseReverseImage, _T("TEXT:Film Reverse Position;"),							_T("False : Normal, True : Reverse")),
			P_SUBROOT_END(),
			P_SUBROOT(_T("TEXT:Film Spec;")),
                P_VAR(spec_method, _T("TEXT:Method decided distance;"),								_T("0 : Center, 1 : Left, 2 : Right, 3 : Multi, 4 : All")),
				P_VAR(spec_lx, _T("TEXT:LX Spec;"),													_T("Panel X <-> Left Film X Distance")),
				P_VAR(spec_lx_tor, _T("TEXT:LX Tolerance;"),										_T("Tolerance")),
				P_VAR(spec_rx , _T("TEXT:RX Spec;"),												_T("Panel X <-> Right Film X Distance")),
				P_VAR(spec_rx_tor , _T("TEXT:RX Tolerance;"),										_T("Tolerance")),
				P_VAR(spec_ly, _T("TEXT:LY Spec;"),													_T("Panel Y <-> Left Film Y Distance")),
				P_VAR(spec_ly_tor, _T("TEXT:LY Tolerance;"),										_T("Tolerance")),
				P_VAR(spec_ry, _T("TEXT:RY Spec;"),													_T("Panel Y <-> Right Film Y Distance")),
				P_VAR(spec_ry_tor, _T("TEXT:RY Tolerance;"),										_T("Tolerance")),
				P_VAR(bScale, _T("TEXT:Use Scale;"),												_T("False : Not Use / Ture : Use")),
				P_VAR(dLx_scale, _T("TEXT:LX Scale;"),												_T("Panel X <-> Left Film X Cale")),
				P_VAR(dRx_scale, _T("TEXT:RX Scale;"),												_T("Panel X <-> Right Film X Cale")),
				P_VAR(dLy_scale, _T("TEXT:LY Scale;"),												_T("Panel Y <-> Left Film Y Cale")),
				P_VAR(dRy_scale, _T("TEXT:RY Scale;"),												_T("Panel Y <-> Right Film Y Cale")),
			P_SUBROOT_END(),
			P_SUBROOT(_T("TEXT:Film Length Check;")),
				P_VAR(bPanelLcheck, _T("TEXT:Use Panel L Check;"),									_T("False : Not Use / Ture : Use")),
				P_VAR(dbPanel_Lcheck, _T("TEXT:Panel L Check;"),									_T("Left Notch <-> Right Notch Length")),
				P_VAR(dbPanel_Lcheck_Torr, _T("TEXT:Panel L Check Tolerance;"),						_T("Tolerance")),
				P_VAR(bFilmLcheck, _T("TEXT:Use Film L Check;"),									_T("False : Not Use / Ture : Use")),
				P_VAR(dbFilm_Lcheck, _T("TEXT:Film L Check;"),										_T("Left Film <-> Right Film Length")),
				P_VAR(dbFilm_Lcheck_Torr, _T("TEXT:Film L Check Tolerance;"),						_T("Tolerance")),
			P_SUBROOT_END(),
			P_SUBROOT(_T("TEXT:Camera Grab Delay ;")),
				P_VAR(iFilmInspGrabDelay, _T("TEXT:Film Inspection Grab;"),							_T("0 ~ 2000 ms")),
			P_SUBROOT_END(),
			P_SUBROOT(_T("TEXT:Film Continue Spec;")),
				P_VAR(dFilnInspExceptionSpec, _T("TEXT:Judge Tolerance;"),								_T("Tolerance")),
				P_VAR(nFilnInspExceptionCount, _T("TEXT:Judge Count;"),									_T("0 : Not Use / 3 ~ 5")),
			P_SUBROOT_END(),
		P_ROOT_END(),
	};

	int nSize = sizeof(GRID_STUFF) / sizeof(CBBungGrid::stProperty);

	m_ctlPropGrid.Initial(&m_font);
	m_ctlPropGrid.AppendProperty(GRID_STUFF, nSize);
}


//KJH2 2022-08-23 Attach Align(PC7), Film Inspection(PC8) 파라미터 분리
void CGridInspectionParamDlg::InitAttachAlign()
{
	// hsj 2023-01-03 grid갱신
	if (m_pMain == NULL) return;

	bool bFilmReAttachCheck = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmReAttachCheckEnable();
	int nFilmReAttachCheckThresh = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmReAttachCheckThresh();
	bool bFilmUVCheck = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmUVCheckEnable();
	int nFilmUVCheckThresh = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmUVCheckThresh();
	int nFilmUVCheckSpec = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmUVCheckSpec();
	bool bUseReverseThresh = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getUseReverseThresh();
	// YCS 2022-10-31 필름 유무검사 영역 사이즈 고정
	bool bUseFixedSize = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFixedFilmReattachUse();
	double dFixedSize = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFixedFilmReattachSize();

	// hsj 2022-02-07 필름검사 threshold 기능 추가
	bool bUsePanelThresh = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getUsePanelThresh_Auto();
	int nPanelThresh = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getPanelThresh();


	CBBungGrid::stProperty GRID_STUFF[] =
	{
		P_ROOT(_T("TEXT:Attach Align;")),
			P_SUBROOT(_T("TEXT:Film Pre Insp;")),
				P_VAR(bFilmReAttachCheck, _T("TEXT:Use Film ReAttach Check;"),						_T("False : Not Use / Ture : Use")),
				P_VAR(nFilmReAttachCheckThresh, _T("TEXT:Thresh Film ReAttach Check;"),				_T("Thresh : 150 ~ 255 / 200 Gray")),
				P_VAR(bFilmUVCheck, _T("TEXT:Use Film UV Check;"),						_T("False : Not Use / Ture : Use")),
				P_VAR(nFilmUVCheckThresh, _T("TEXT:Thresh Film UV Check;"),				_T("Thresh : 150 ~ 255 / 200 Gray")),
				P_VAR(nFilmUVCheckSpec, _T("TEXT:Thresh Film UV Spec;"),				_T("Spec : 150 ~ 360 / 200 angle")),
				P_VAR(bUseReverseThresh, _T("TEXT:Film Check Object Color;"),					_T("False : White / Ture : Balck")),
				// YCS 2022-10-31 필름 유무검사 영역 사이즈 고정
				P_VAR(bUseFixedSize, _T("TEXT:Fixed Film Reattach Use;"),					_T("False : Adjustable / Ture : Fixed")),
				P_VAR(dFixedSize, _T("TEXT:Fixed Film Reattach Size;"),					_T("Recommended Size : 2.5 mm")),
			P_SUBROOT_END(),
		P_ROOT_END(),
	};

	int nSize = sizeof(GRID_STUFF) / sizeof(CBBungGrid::stProperty);

	m_ctlPropGrid.Initial(&m_font);
	m_ctlPropGrid.AppendProperty(GRID_STUFF, nSize);
}

// SJB 2022-10-03 Scratch Insp(PC1), Scratch Inspection(PC1) 파라미터 분리
void CGridInspectionParamDlg::InitScratchInspection()
{
	// hsj 2023-01-03 grid갱신
	if (m_pMain == NULL) return;

	bool bScratchCheck = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getScratchCheck();
	int nScratchThresh = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getScratchThresh();
	double dMaskingRadius = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getScratchMaskingRadius();
	int nScratchInspSpec = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getScratchInspSpec();
	int nScratchInspAreaSpec = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getScratchInspAreaSpec();
	bool bScratchInspThreadMode = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getScratchInspThreadMode();
	bool bScratchInspPeriMode = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getScratchInspPeriMode();
	double dLeftTopDistX = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getScratchLeftTopDistX();
	double dLeftTopDistY = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getScratchLeftTopDistY();
	double dInspWidth = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getScratchInspWidth();
	double dInspHeight = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getScratchInspHeight();

	CBBungGrid::stProperty GRID_STUFF[] =
	{
		P_ROOT(_T("TEXT:Pre Align;")),
			P_SUBROOT(_T("TEXT:Scratch Insp;")),
				P_VAR(bScratchCheck, _T("TEXT:Use Scratch Inspection Check;"),						_T("False : Not Use / True : Use")),
				P_VAR(nScratchThresh, _T("TEXT:Thresh Scratch Inspection Check;"),				_T("Thresh : 140 ~ 255 / 200 Gray")),
				P_VAR(dMaskingRadius, _T("TEXT:Radius Masking Circle;"),						_T("2 mm")),
				P_VAR(nScratchInspSpec, _T("TEXT:Scratch Size Spec;"),				_T("4 pixel (1px = 0.02mm)")),
				P_VAR(nScratchInspAreaSpec, _T("TEXT:Scratch Insp Area;"),				_T("50 pixel (1px = 0.02mm)")),
				P_VAR(bScratchInspThreadMode, _T("TEXT:Use Scratch Inspection ThreadMode;"),	_T("False : Not Use / True : Use")),
				P_VAR(bScratchInspPeriMode, _T("TEXT:Use Scratch Perimeter Inspection;"),	_T("False : Not Use / True : Use")),
				P_VAR(dLeftTopDistX, _T("TEXT:Scratch Perimeter LeftTopDistX;"),						_T("2.0 mm")),
				P_VAR(dLeftTopDistY, _T("TEXT:Scratch Perimeter LeftTopDistY;"),						_T("33.1 mm")),
				P_VAR(dInspWidth, _T("TEXT:Scratch Perimeter InspWidth;"),						_T("2 mm")),
				P_VAR(dInspHeight, _T("TEXT:Scratch Perimeter InspHeight;"),						_T("5 mm")),
			P_SUBROOT_END(),
		P_ROOT_END(),
	};

	int nSize = sizeof(GRID_STUFF) / sizeof(CBBungGrid::stProperty);

	m_ctlPropGrid.Initial(&m_font);
	m_ctlPropGrid.AppendProperty(GRID_STUFF, nSize);
}

BOOL CGridInspectionParamDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_hbrBkg = CreateSolidBrush(COLOR_BTN_BODY);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

BOOL CGridInspectionParamDlg::InItDialog()
{
	if (m_pMain != NULL)
	{
		switch (m_pMain->vt_job_info[m_nJob].algo_method) 
		{
		case CLIENT_TYPE_ELB_CENTER_ALIGN:
		{
#ifdef _SEPARA
			bUse = TRUE;
			InitCenterCircle();
			MoveWindow(874, 100, 560, 300);
			GetDlgItem(IDC_MFCPROPERTYGRID_INSPECTION)->MoveWindow(5, 5, 560 - 10, 300 - 10);
#else
			bUse = TRUE;
			InitCenterCircle();
			//MoveWindow(554, 10, 560, 610);
			MoveWindow(874, 100, 560, 610);
			GetDlgItem(IDC_MFCPROPERTYGRID_INSPECTION)->MoveWindow(5, 5, 560 - 10, 610 - 10);
#endif
		}
		break;
		case CLIENT_TYPE_NOZZLE_ALIGN:
		case CLIENT_TYPE_CENTER_NOZZLE_ALIGN:
		{
			bUse = TRUE;
			InitNozzleAlign();
			MoveWindow(874, 100, 560, 980);
			GetDlgItem(IDC_MFCPROPERTYGRID_INSPECTION)->MoveWindow(5, 5, 560 - 10, 980 - 10);
			break;
		}
		case CLIENT_TYPE_CENTER_SIDE_YGAP:
		{
			bUse = TRUE;
			InitZGapInsp();
			MoveWindow(874, 100, 560, 980);
			GetDlgItem(IDC_MFCPROPERTYGRID_INSPECTION)->MoveWindow(5, 5, 560 - 10, 980 - 10);
			break;
		}
		case CLIENT_TYPE_FILM_INSP:
		{
			bUse = TRUE;
			InitFilmInspection();
			//MoveWindow(554, 10, 560, 980);
			MoveWindow(874, 100, 560, 980);
			GetDlgItem(IDC_MFCPROPERTYGRID_INSPECTION)->MoveWindow(5, 5, 560 - 10, 980 - 10);
		}
		break;
		//HTK 2022-07-12 Nozzel#45에 약액상태 검사 추가
		case CLIENT_TYPE_ONLY_VIEW:
		{
			bUse = TRUE;
			InitViewSubInspection();
			MoveWindow(874, 100, 560, 980);
			GetDlgItem(IDC_MFCPROPERTYGRID_INSPECTION)->MoveWindow(5, 5, 560 - 10, 980 - 10);
		}
		break;
		// SJB 2022-10-03 Scratch Inspection 파라미터 분리
		case CLIENT_TYPE_1CAM_1SHOT_ALIGN_WITH_SCRATCH_INSP:
		{
			bUse = TRUE;
			InitScratchInspection();
			MoveWindow(874, 100, 560, 980);
			GetDlgItem(IDC_MFCPROPERTYGRID_INSPECTION)->MoveWindow(5, 5, 560 - 10, 980 - 10);
		}
		break;
		// YCS 2022-11-19 필름얼라인 각도 비교 스펙 파라미터 분리
		case CLIENT_TYPE_1CAM_1SHOT_FILM:
			bUse = TRUE;
			InitFilmAlign();
			MoveWindow(874, 100, 560, 980);
			GetDlgItem(IDC_MFCPROPERTYGRID_INSPECTION)->MoveWindow(5, 5, 560 - 10, 980 - 10);
			break;
		default:
		{
			bUse = FALSE;
		}
		break;
		}

		int _findText = int(m_pMain->vt_job_info[m_nJob].job_name.find("ATTACH_ALIGN")); // Tkyuha 20220411 별도 기능 추가
		if (_findText!=-1)
		{
			bUse = TRUE;
			//InitFilmInspection();
			InitAttachAlign();
			//MoveWindow(554, 10, 560, 980);
			MoveWindow(874, 100, 560, 980);
			GetDlgItem(IDC_MFCPROPERTYGRID_INSPECTION)->MoveWindow(5, 5, 560 - 10, 980 - 10);
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CGridInspectionParamDlg::OnBnClickedOk()
{
	switch (m_pMain->vt_job_info[m_nJob].algo_method) {
	case CLIENT_TYPE_ELB_CENTER_ALIGN:
		SaveCenterCircle();
		break;
	case CLIENT_TYPE_NOZZLE_ALIGN:
	case CLIENT_TYPE_CENTER_NOZZLE_ALIGN:
		SaveNozzleAlign();
		break;
	case CLIENT_TYPE_CENTER_SIDE_YGAP:
		SaveZGapInsp();
		break;
	case CLIENT_TYPE_FILM_INSP:
		SaveFilmInspection();
		break;
	//HTK 2022-07-11 Nozzle#45 검사기능 추가로 인한 모델파라미터 생성
	case CLIENT_TYPE_ONLY_VIEW:
		SaveViewSubInspection();
		break;
		// SJB 2022-10-03 Scratch 검사기능 추가로 인한 모델파라미터 생성
	case CLIENT_TYPE_1CAM_1SHOT_ALIGN_WITH_SCRATCH_INSP:
		SaveScratchInspection();
		break;
		// YCS 2022-11-19 필름얼라인 각도 비교 파라미터 분리
	case CLIENT_TYPE_1CAM_1SHOT_FILM:
		SaveFilmAlign();
		break;
	}

	int _findText = int(m_pMain->vt_job_info[m_nJob].job_name.find("ATTACH_ALIGN")); // Tkyuha 20220411 별도 기능 추가
	if (_findText != -1)		SaveAttachAlign();

	//CDialogEx::OnOK();
}

void CGridInspectionParamDlg::SaveCenterCircle()
{
	bool bDustEnable					= m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(0)->GetValue().boolVal;		//false         //이물 검사
	bool bLiquidDropsEnable				= m_ctlPropGrid.GetProperty(0)->GetSubItem(2)->GetSubItem(0)->GetValue().boolVal;		//false			//액튐 검사
	
	bool bDispensingStatusEnable		= m_ctlPropGrid.GetProperty(0)->GetSubItem(3)->GetSubItem(0)->GetValue().boolVal;		//도포된 패널 유무 검사
	bool bLamiStatusEnable				= m_ctlPropGrid.GetProperty(0)->GetSubItem(4)->GetSubItem(0)->GetValue().boolVal;		//라미틀어짐 검사
	bool bCenterDistanceEnable			= m_ctlPropGrid.GetProperty(0)->GetSubItem(5)->GetSubItem(0)->GetValue().boolVal;		//홀 중심점 이격 검사
	bool bCenterCCDistanceEnable		= m_ctlPropGrid.GetProperty(0)->GetSubItem(5)->GetSubItem(1)->GetValue().boolVal;		//노즐과 홀 사이 검사
	
	
	int  iDustThresh					= m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(1)->GetValue().iVal;			//40			//이물 검사 Gray값
	int  iLiquidDropsThresh				= m_ctlPropGrid.GetProperty(0)->GetSubItem(2)->GetSubItem(1)->GetValue().iVal;			//40			//액튐 검사 Gray값
	int  iDispensingStatusThresh		= m_ctlPropGrid.GetProperty(0)->GetSubItem(3)->GetSubItem(1)->GetValue().iVal;			//40			//도포된 패널 유무 검사 Gray값
	int  iLamiStatusThresh				= m_ctlPropGrid.GetProperty(0)->GetSubItem(4)->GetSubItem(1)->GetValue().iVal;			//60			//라미 패널 틀어짐 검사 Gray값
	double  dCenterDistanceSpec			= m_ctlPropGrid.GetProperty(0)->GetSubItem(5)->GetSubItem(2)->GetValue().dblVal;		//0.03			//홀중심과 카메라 중심점 거리
	double  dCCDistanceSpec				= m_ctlPropGrid.GetProperty(0)->GetSubItem(5)->GetSubItem(3)->GetValue().dblVal;		//0.35			//홀과 CC 거리 가 가까우면 NG
	double  dCCSearchThresh				= m_ctlPropGrid.GetProperty(0)->GetSubItem(5)->GetSubItem(4)->GetValue().dblVal;		//80			//CC원 찾는 기준값

	// KBJ 2022-09-05 Rotate 파라미터 추가
	double	dRotateSpec					= m_ctlPropGrid.GetProperty(0)->GetSubItem(6)->GetSubItem(0)->GetValue().dblVal;
	int		nRotateBinaryValue			= m_ctlPropGrid.GetProperty(0)->GetSubItem(6)->GetSubItem(1)->GetValue().iVal;
	int		nRotateLightValue			= m_ctlPropGrid.GetProperty(0)->GetSubItem(6)->GetSubItem(2)->GetValue().iVal;

	//bool bNozzleToHoleEdgeEnable		= m_ctlPropGrid.GetProperty(0)->GetSubItem(6)->GetSubItem(0)->GetValue().boolVal;		//노즐과 홀 사이 검사
	//double  dNozzleToHoleDistanceSpec	= m_ctlPropGrid.GetProperty(0)->GetSubItem(6)->GetSubItem(1)->GetValue().dblVal;		//0.05
	//double  dNozzleToStageGapMinSpec	= m_ctlPropGrid.GetProperty(0)->GetSubItem(6)->GetSubItem(2)->GetValue().dblVal;		//0.08			//노즐과 스테이지 최소거리
	//double  dNozzleToStageGapMaxSpec	= m_ctlPropGrid.GetProperty(0)->GetSubItem(6)->GetSubItem(3)->GetValue().dblVal;		//0.13			//노즐과 스테이지 최대거리
	bool	bNozzlePanelHoleCheck		= m_ctlPropGrid.GetProperty(0)->GetSubItem(7)->GetSubItem(0)->GetValue().boolVal;		//패널 홀과 노즐 Y갭 측정후 예외처리
	double  dNozzleToXYMinSpec			= m_ctlPropGrid.GetProperty(0)->GetSubItem(7)->GetSubItem(1)->GetValue().dblVal;		//0.02			//노즐과 카메라 중심 최소거리 
	double  dNozzleToXYMaxSpec			= m_ctlPropGrid.GetProperty(0)->GetSubItem(7)->GetSubItem(2)->GetValue().dblVal;		//0.07			//노즐과 카메라 중심 최대거리
	bool	bCenterNozzleYLight			= m_ctlPropGrid.GetProperty(0)->GetSubItem(7)->GetSubItem(3)->GetValue().boolVal;		//0				//Nozzle Y Gap Light Enable/Disable
	int		nNozzleYGapThreshold		= m_ctlPropGrid.GetProperty(0)->GetSubItem(7)->GetSubItem(4)->GetValue().boolVal;
	//double  dNozzlePanelHoleGapThresh	= m_ctlPropGrid.GetProperty(0)->GetSubItem(6)->GetSubItem(8)->GetValue().dblVal;		//20			//노즐과 홀 사이 갭측정 Thresh
	//double  dNozzleAngle				= m_ctlPropGrid.GetProperty(0)->GetSubItem(6)->GetSubItem(9)->GetValue().dblVal;		//30			//노즐 기본 각도
    //bool   bNozzleAngleSearchMode       = m_ctlPropGrid.GetProperty(0)->GetSubItem(6)->GetSubItem(10)->GetValue().boolVal;                       //Enhance/Otsu // 노즐 기본 각도
	//double  dNozzleToStageGapOffset     = m_ctlPropGrid.GetProperty(0)->GetSubItem(6)->GetSubItem(11)->GetValue().dblVal; 
	double  dCenterLiveGuideLine		= m_ctlPropGrid.GetProperty(0)->GetSubItem(7)->GetSubItem(5)->GetValue().dblVal;
	//bool    bCenterNozzleYAlignRecalc   = m_ctlPropGrid.GetProperty(0)->GetSubItem(6)->GetSubItem(6)->GetValue().boolVal;     					//Nozzle Align시 Mark Search이후 Y값 2진화 자동검사 모드 사용유무      
	//int    iCenterNozzleToPanelDistance = m_ctlPropGrid.GetProperty(0)->GetSubItem(6)->GetSubItem(7)->GetValue().iVal;

	bool bCircleMetalRadiusCheck		= m_ctlPropGrid.GetProperty(0)->GetSubItem(8)->GetSubItem(0)->GetValue().boolVal;		//홀과 메탈 지름 비교 하여 예외처리
	bool bCircleMetalCenterCheck		= m_ctlPropGrid.GetProperty(0)->GetSubItem(8)->GetSubItem(1)->GetValue().boolVal;		//홀과 메탈 중심점 거리 비교 하여 예외처리
	//bool	bNozzlePanelHoleCheck		= m_ctlPropGrid.GetProperty(0)->GetSubItem(7)->GetSubItem(2)->GetValue().boolVal;		//패널 홀과 노즐 Y갭 측정후 예외처리
	double  dCenterMetalDistanceSpec	= m_ctlPropGrid.GetProperty(0)->GetSubItem(8)->GetSubItem(2)->GetValue().dblVal;		//0.05			//메탈과 원 중심점 거리
	bool bCircleFindCenterCheck			= m_ctlPropGrid.GetProperty(0)->GetSubItem(8)->GetSubItem(3)->GetValue().boolVal;  		//카메라 중심점과 홀 거리측정 30픽셀
	bool bInspectionDebugCheck			= m_ctlPropGrid.GetProperty(0)->GetSubItem(8)->GetSubItem(4)->GetValue().boolVal;   	//검사 디버깅 모드


	// WETOUT 검사 파라미터
	bool bWetoutEnable					= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(0)->GetValue().boolVal;		//true			//wetout 검사
	int  iWetoutThresh					= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(1)->GetValue().iVal;			//210			//Wetout 검사 Gray값
	double  dWetoutInMin				= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(2)->GetValue().dblVal;		//0.1
	double  dWetoutInMax				= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(3)->GetValue().dblVal;		//0.35
	double  dWetoutOutMin				= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(4)->GetValue().dblVal;		//0
	double  dWetoutOutMax				= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(5)->GetValue().dblVal;		//0.03
	double  dCircleLength				= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(6)->GetValue().dblVal;		//15.0
	int		iCCFindFilter				= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(7)->GetValue().iVal;			//30~65
	double	iMetalOverflowMargin		= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(8)->GetValue().dblVal;		//20~50
	bool	bTestImageSave				= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(9)->GetValue().boolVal;		//FALSE
	double	dLineLengthMinThick         = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(10)->GetValue().dblVal; 
	double	dLineLengthSpec             = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(11)->GetValue().dblVal; 
	double	dCamCenterToCG              = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(12)->GetValue().dblVal;
	double	dInkUnderflowSpec           = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(13)->GetValue().dblVal;
	//HTK 2022-03-16 DummyInspSpec 추가
	double	dDummyMinSizeSpec           = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(14)->GetValue().dblVal;
	//KJH 2022-04-29 MincountCheckInsp 추가
	//KJH 2022-05-09 Mincount 상시 사용으로 변경
	//BOOL	bMinCountCheckInspMode		= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(15)->GetValue().boolVal;
	int		iMinCountCheckInspContinue	= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(15)->GetValue().iVal;
	int		iMinCountCheckInspTotal		= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(16)->GetValue().iVal;
	double	dCInkMinWidthSpec			= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(17)->GetValue().dblVal;
	BOOL	bWetoutRJudgeModeEnable		= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(18)->GetValue().boolVal;
	double	dWetoutRMinSpec				= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(19)->GetValue().dblVal;
	double	dWetoutRMaxSpec				= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(20)->GetValue().dblVal;
	//KJH 2022-05-25 BM In <-> Cink In Insp 추가
	BOOL	bWetoutBJudgeModeEnable		= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(21)->GetValue().boolVal;
	double	dWetoutBMinSpec				= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(22)->GetValue().dblVal;
	double	dWetoutBMaxSpec				= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(23)->GetValue().dblVal;
	bool    bWetoutJudgeEnable          = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(24)->GetValue().boolVal;	
	double	dLineLengthFDC_Distance     = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(25)->GetValue().dblVal;
	//SJB 2022-11-11 Overflow Min 파라미터 추가
	double	dLineOverflowMinSpec = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(26)->GetValue().dblVal;
	//SJB 2022-11-19 CG Exposure Time 파라미터 추가
	int		iCGGrabExposure = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(27)->GetValue().iVal;
	// blue 2022.09.09 Diff image 두개로 나뉘는 증상때문에 추가 (F=default, T=전무님코드사용)
	bool    bDiffRoiMerge				= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(28)->GetValue().boolVal;
	bool bCalcWetoutMetalEdge			= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(29)->GetValue().boolVal;   	// blue 2022.09.02 CINK1+CINK2(EGL)에서 wetout 계산시 GlassEdge 대신 MetalEdge 사용
	bool bWetoutInspectionSimulation    = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(30)->GetValue().boolVal;

	// YCS 2022-12-01 Wetout_ASPC 스펙
	double dWetout_ASPC_Spec = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(31)->GetValue().dblVal;

	// KBJ 2022-12-05 Pocket Size
	double dLine_Pocket_Size = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(32)->GetValue().dblVal;

	//SJB 2023-01-02 CG to Start, End 파라미터 추가
	double dCgToStartSpec = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(33)->GetValue().dblVal;
	double dCgToEndSpec = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(34)->GetValue().dblVal;
	double dCgJudgeSpec = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(35)->GetValue().dblVal;

	// 이물검사 파라미터
	double  dDustSizeMin				= m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(2)->GetValue().dblVal;		//0.2
	double  dDustSizeMax				= m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(3)->GetValue().dblVal;		//2
	double  dDustLineRatio				= m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(4)->GetValue().dblVal;
	double  dDustSpecMin				= m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(5)->GetValue().dblVal;		//0.1
	double  dDustSpecMax				= m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(6)->GetValue().dblVal;		//1.0

	// 액튐검사 파라미터
	double  dDropsSizeInMin				= m_ctlPropGrid.GetProperty(0)->GetSubItem(2)->GetSubItem(2)->GetValue().dblVal;		//0.15
	double  dDropsSizeInMax				= m_ctlPropGrid.GetProperty(0)->GetSubItem(2)->GetSubItem(3)->GetValue().dblVal;		//1
	double  dDropsLineInRatio			= m_ctlPropGrid.GetProperty(0)->GetSubItem(2)->GetSubItem(4)->GetValue().dblVal;
	double  dDropsSizeOutMin			= m_ctlPropGrid.GetProperty(0)->GetSubItem(2)->GetSubItem(5)->GetValue().dblVal;		//0.5
	double  dDropsSizeOutMax			= m_ctlPropGrid.GetProperty(0)->GetSubItem(2)->GetSubItem(6)->GetValue().dblVal;		//1
	double  dDropsLineOutRatio			= m_ctlPropGrid.GetProperty(0)->GetSubItem(2)->GetSubItem(7)->GetValue().dblVal;
	double  dDropsSpecMin				= m_ctlPropGrid.GetProperty(0)->GetSubItem(2)->GetSubItem(8)->GetValue().dblVal;		//0.1
	double  dDropsSpecMax				= m_ctlPropGrid.GetProperty(0)->GetSubItem(2)->GetSubItem(9)->GetValue().dblVal;		//1.0
	double  dDropsAfterImageRemove		= m_ctlPropGrid.GetProperty(0)->GetSubItem(2)->GetSubItem(10)->GetValue().dblVal;		//80			//잔상 제거 파라미터 gray
	double  dFeriAlignFindThresh        = m_ctlPropGrid.GetProperty(0)->GetSubItem(2)->GetSubItem(11)->GetValue().dblVal;
	bool    bFeriAlignFindPreProc       = m_ctlPropGrid.GetProperty(0)->GetSubItem(2)->GetSubItem(12)->GetValue().boolVal;
	// Flag
	// Grab Delay 파라미터
	int iDustInspGrabDelay				= m_ctlPropGrid.GetProperty(0)->GetSubItem(9)->GetSubItem(0)->GetValue().iVal;
	int iDropInspGrabDelay				= m_ctlPropGrid.GetProperty(0)->GetSubItem(9)->GetSubItem(1)->GetValue().iVal;
	int iCenterAlignGrabDelay			= m_ctlPropGrid.GetProperty(0)->GetSubItem(9)->GetSubItem(2)->GetValue().iVal;
	int iNozzleAlignGrabDelay			= m_ctlPropGrid.GetProperty(0)->GetSubItem(9)->GetSubItem(3)->GetValue().iVal;

	// Adaptive Dispensing 파라미터
	bool bAdaptiveDispensingFlag		= m_ctlPropGrid.GetProperty(0)->GetSubItem(10)->GetSubItem(0)->GetValue().boolVal; 				// 사용 /미사용 선택
	double  dBaseOffset					= m_ctlPropGrid.GetProperty(0)->GetSubItem(10)->GetSubItem(1)->GetValue().dblVal;    			// Base Offset (0)
	double  dScaleOffset				= m_ctlPropGrid.GetProperty(0)->GetSubItem(10)->GetSubItem(2)->GetValue().dblVal;  				// Scale Offset (1)
	double  dDispensingRange            = m_ctlPropGrid.GetProperty(0)->GetSubItem(10)->GetSubItem(3)->GetValue().dblVal;

	// Dispensing Mode
	bool bELBCoverCInkModeEable				= m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(0)->GetValue().boolVal;			// false				// ELB Cover C-Ink Mode
		// 원찾기 파라미터 설정들
	bool bMetalSearchRemoveNoiseFlag		= m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(1)->GetValue().boolVal;			// 사용 /미사용 선택
	bool bTraceUsingAlignFlag				= m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(2)->GetValue().boolVal;			// 얼라인 값으로 Trace 계산 모드 활성화
	//bool bSearchElbEndPosFlag				= m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(3)->GetValue().boolVal;			// 얼라인 값으로 Trace 계산 모드 활성화
	bool bELBCG_Edge_CalcModeEnable			= m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(3)->GetValue().boolVal;
	bool bELBPartial_DispensingModeEnable	= m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(4)->GetValue().boolVal;
	bool bELBInsp_ShiftSearchEnable         = m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(5)->GetValue().boolVal;
	bool bAVI_Save_ImageTypeEnable			= m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(6)->GetValue().boolVal;
	bool bLineShift_DispensingModeEnable    = m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(7)->GetValue().boolVal;
	bool bCircle_ShapeDispensingModeEnable  = m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(8)->GetValue().boolVal; //22.06.02
	bool bLineShift_DoubleDispensingModeEnable = m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(9)->GetValue().boolVal;
	bool bLineOverflowDispMode              = m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(10)->GetValue().boolVal;
	bool bCircle_Cink1_Cink2MergeMode       = m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(11)->GetValue().boolVal;
	bool bLine_InspWithTraceResultMode      = m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(12)->GetValue().boolVal;
	bool bLine_PanelToMeatalCalcMode		= m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(13)->GetValue().boolVal; // YCS 2022-12-24 PN To Metal 측정모드 파라미터 추가
	//KJH 2022-04-09 PN 반지름 비교 검사 추가
	bool	bPNSizeInspEnable				= m_ctlPropGrid.GetProperty(0)->GetSubItem(5)->GetSubItem(5)->GetValue().boolVal;			//PN Size 검사
	double	dPNSizeSpec						= m_ctlPropGrid.GetProperty(0)->GetSubItem(5)->GetSubItem(6)->GetValue().dblVal;			//PN Size	   2.35mm
	double	dPNSizeSpecTorr					= m_ctlPropGrid.GetProperty(0)->GetSubItem(5)->GetSubItem(7)->GetValue().dblVal;			//PN Size Torr 0.05mm

	//UI , Memory 비교문

#pragma region 그리드값 임시 저장
	//lhj add 220503 /////////////////////////////////////////////////////////////////////////////////////
	
	m_pMain->sGridData.bWetoutEnable		= bWetoutEnable;
	m_pMain->sGridData.bDustEnable			= bDustEnable;
	m_pMain->sGridData.bLiquidDropsEnable	= bLiquidDropsEnable;
	m_pMain->sGridData.inputThresh			= iDustThresh;
	m_pMain->sGridData.iLiquidDropsThresh	= iLiquidDropsThresh;
	m_pMain->sGridData.hthresh				= iWetoutThresh;
	m_pMain->sGridData.lthresh				= dCCSearchThresh;
	m_pMain->sGridData.dCircleLength		= dCircleLength;
	m_pMain->sGridData.bCircleShapeModeEnable = bCircle_ShapeDispensingModeEnable; //22.06.02
	m_pMain->sGridData.dCenterMetalDistanceSpec = dCenterMetalDistanceSpec;

	if (m_pMain->sUIData.bWetoutEnable != m_pMain->sGridData.bWetoutEnable)
	{
		if (m_pMain->sFileData.bWetoutEnable != m_pMain->sUIData.bWetoutEnable)
		{
			variant_t var = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(0)->GetValue();
			//variant_t var;
			//var.vt = VT_BOOL;
			var.boolVal = -1 * m_pMain->sUIData.bWetoutEnable;
			//var.iVal	= -1 * m_pMain->sUIData.bWetoutEnable;
			//var.intVal = -1 * m_pMain->sUIData.bWetoutEnable;
			//var.lVal = -1 *  m_pMain->sUIData.bWetoutEnable;

			m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(0)->SetValue(var); //lhj check 220503
			//m_ctlPropGrid.DataRefresh(TRUE);

			//m_ctlPropGrid.SetCurSel(m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(0));

			bWetoutEnable = m_pMain->sUIData.bWetoutEnable;
		}
	}

	if (m_pMain->sUIData.bDustEnable != m_pMain->sGridData.bDustEnable)
	{
		if (m_pMain->sFileData.bDustEnable != m_pMain->sUIData.bDustEnable)
		{
			variant_t var = m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(0)->GetValue();
			var.boolVal = -1 * m_pMain->sUIData.bDustEnable;
			m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(0)->SetValue(var);

			bDustEnable = m_pMain->sUIData.bDustEnable;
		}
	}

	if (m_pMain->sUIData.bLiquidDropsEnable != m_pMain->sGridData.bLiquidDropsEnable)
	{
		if (m_pMain->sFileData.bLiquidDropsEnable != m_pMain->sUIData.bLiquidDropsEnable)
		{
			variant_t var = m_ctlPropGrid.GetProperty(0)->GetSubItem(2)->GetSubItem(0)->GetValue();
			var.boolVal = -1 * m_pMain->sUIData.bLiquidDropsEnable;
			m_ctlPropGrid.GetProperty(0)->GetSubItem(2)->GetSubItem(0)->SetValue(var);

			bLiquidDropsEnable = m_pMain->sUIData.bLiquidDropsEnable;
		}
	}

	if (m_pMain->sUIData.inputThresh != m_pMain->sGridData.inputThresh)
	{
		if (m_pMain->sFileData.inputThresh != m_pMain->sUIData.inputThresh)
		{
			_variant_t var = m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(1)->GetValue();
			var.iVal = m_pMain->sUIData.inputThresh;
			m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(1)->SetValue(var);

			iDustThresh = m_pMain->sUIData.inputThresh;
		}
	}

	if (m_pMain->sUIData.iLiquidDropsThresh != m_pMain->sGridData.iLiquidDropsThresh)
	{
		if (m_pMain->sFileData.iLiquidDropsThresh != m_pMain->sUIData.iLiquidDropsThresh)
		{
			_variant_t var = m_ctlPropGrid.GetProperty(0)->GetSubItem(2)->GetSubItem(1)->GetValue();
			var.iVal = m_pMain->sUIData.iLiquidDropsThresh;
			m_ctlPropGrid.GetProperty(0)->GetSubItem(2)->GetSubItem(1)->SetValue(var);

			iLiquidDropsThresh = m_pMain->sUIData.iLiquidDropsThresh;
		}
	}

	if (m_pMain->sUIData.hthresh != m_pMain->sGridData.hthresh)
	{
		if (m_pMain->sFileData.hthresh != m_pMain->sUIData.hthresh)
		{
			_variant_t var = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(1)->GetValue();
			//var.vt = VT_I2;
			var.iVal = m_pMain->sUIData.hthresh;
			m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(1)->SetValue(var);

			iWetoutThresh = m_pMain->sUIData.hthresh;
		}
	}

	if (m_pMain->sUIData.lthresh != m_pMain->sGridData.lthresh)
	{
		if (m_pMain->sFileData.lthresh != m_pMain->sUIData.lthresh)
		{
			_variant_t var = m_ctlPropGrid.GetProperty(0)->GetSubItem(5)->GetSubItem(4)->GetValue();
			var.dblVal = m_pMain->sUIData.lthresh;
			m_ctlPropGrid.GetProperty(0)->GetSubItem(5)->GetSubItem(4)->SetValue(var);

			dCCSearchThresh = m_pMain->sUIData.lthresh;
		}
	}
	
	if (m_pMain->sUIData.dCircleLength != m_pMain->sGridData.dCircleLength)
	{
		if (m_pMain->sFileData.dCircleLength != m_pMain->sUIData.dCircleLength)
		{
			_variant_t var = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(6)->GetValue();
			var.dblVal = m_pMain->sUIData.dCircleLength;
			m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(6)->SetValue(var);

			dCircleLength = m_pMain->sUIData.dCircleLength;
		}
	}

	if (m_pMain->sUIData.dCenterMetalDistanceSpec != m_pMain->sGridData.dCenterMetalDistanceSpec)
	{
		if (m_pMain->sFileData.dCenterMetalDistanceSpec != m_pMain->sUIData.dCenterMetalDistanceSpec)
		{
			_variant_t var = m_ctlPropGrid.GetProperty(0)->GetSubItem(8)->GetSubItem(2)->GetValue();
			var.dblVal = m_pMain->sUIData.dCenterMetalDistanceSpec;
			m_ctlPropGrid.GetProperty(0)->GetSubItem(8)->GetSubItem(2)->SetValue(var);

			dCenterMetalDistanceSpec = m_pMain->sUIData.dCenterMetalDistanceSpec;
		}
	}

	// KBJ 2022-09-01 Control Param
	if (m_pMain->m_nLogInUserLevelType == MASTER)
	{
		if (m_pMain->sUIData.bCircleShapeModeEnable != m_pMain->sGridData.bCircleShapeModeEnable) //22.06.02
		{
			if (m_pMain->sFileData.bCircleShapeModeEnable != m_pMain->sUIData.bCircleShapeModeEnable)
			{
				variant_t var = m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(8)->GetValue();
				var.boolVal = -1 * m_pMain->sUIData.bCircleShapeModeEnable;
				//m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(0)->SetValue(var);
				m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(8)->SetData(var);

				bCircle_ShapeDispensingModeEnable = m_pMain->sUIData.bCircleShapeModeEnable;
			}
		}
	}
	else
	{
		variant_t b_var = m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(8)->GetValue();
		b_var.boolVal = -1 * m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getUseDummyCornerInsp();
		m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(8)->SetData(b_var);
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma endregion

	//WetoutInspThreshold Para
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setUseDiffInsp(bWetoutEnable);										//true					//wetout 검사
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setUseInputInspection(bDustEnable);									//false					//이물 검사
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setUseDustInsp(bLiquidDropsEnable);									//false					//액튐 검사

	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setDispensingStatusEnable(bDispensingStatusEnable);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setLamiStatusEnable(bLamiStatusEnable);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setCenterDistanceEnable(bCenterDistanceEnable);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setCenterCCDistanceEnable(bCenterCCDistanceEnable);
	//m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setNozzleToHoleEdgeEnable(bNozzleToHoleEdgeEnable);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setCircleMetalRadiusCheck(bCircleMetalRadiusCheck);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setCircleMetalCenterCheck(bCircleMetalCenterCheck);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setNozzlePanelHoleCheck(bNozzlePanelHoleCheck);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setCircleFindCenterCheck(bCircleFindCenterCheck);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setInspectionDebugCheck(bInspectionDebugCheck);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setDiffRoiMergeEnable(bDiffRoiMerge);	// blue 2022.09.09 
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setCalcWetoutMetalEdge(bCalcWetoutMetalEdge);	
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setWetoutInspectionSimulation(bWetoutInspectionSimulation); // Tkyuha 221109 Simualtion Mode

	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setInspectionHighThresh(iWetoutThresh);								//210					//Wetout 검사 Gray값
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setExistDummyGray(iDustThresh);										//40					//이물 검사 Gray값
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setInspDustThresh(iLiquidDropsThresh);								//40					//액튐 검사 Gray값
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setInkCheckThresh(0, iDispensingStatusThresh);						//40					//도포된 패널 유무 검사 Gray값
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setLamiCheckThresh(0, iLamiStatusThresh);								//60					//라미 패널 틀어짐 검사 Gray값

	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSpecLSL(0, dCenterDistanceSpec);								//0.03					//홀중심과 카메라 중심점 거리
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSpecUSL(0, dCCDistanceSpec);									//0.35					//홀과 BM 거리 가 가까우면 NG
	//m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSpecLSL(3, dNozzleToHoleDistanceSpec);						// 0.05
	//m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSpecLSL(7, dNozzleToStageGapMinSpec);							//0.08					//노즐과 스테이지 최소거리
	//m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSpecUSL(7, dNozzleToStageGapMaxSpec);							//0.13					//노즐과 스테이지 최대거리
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSpecLSL(8, dNozzleToXYMinSpec);								//0.02					//노즐과 카메라 중심 최소거리 
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSpecUSL(8, dNozzleToXYMaxSpec);								//0.07					//노즐과 카메라 중심 최대거리
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspHoleMetal(dCenterMetalDistanceSpec);							//0						//Nozzle Y Gap Light Enable / disable
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setUseCenterNozzleYLight(bCenterNozzleYLight);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setNozzleYGapThreshold(nNozzleYGapThreshold);							//20					//KJH2
	//m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setExistPanelGray(dNozzlePanelHoleGapThresh);							//20					//노즐과 홀 사이 갭측정 Thresh
	//m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setIncludedAngleLimit(dNozzleAngle);											//30					//노즐 기본 각도
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setInspectionLowThresh(dCCSearchThresh);
	//m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setNozzleAngleSearchMode(bNozzleAngleSearchMode);						//Enhance/Otsu			// 노즐 기본 각도 찾는 방식		
	//m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setNozzleToStageGapOffset(dNozzleToStageGapOffset);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setCenterLiveViewGuideLineOffset(dCenterLiveGuideLine);
	//m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setCenterNozzleYAlignRecalc(bCenterNozzleYAlignRecalc);
	//m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setCenterNozzleToPanelDistanceLight(iCenterNozzleToPanelDistance); 			//150	

	//WETOUT 검사 파라미터
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSubSpec(0, dWetoutInMin);										//0.15
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSubSpecXTolerance(0, dWetoutInMax);							//0.35
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSubSpec(1, dWetoutOutMin);									//0
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSubSpecXTolerance(1, dWetoutOutMax);							//0.03
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setInspRangeLength(dCircleLength);											//15.0
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setCCFindFilter(iCCFindFilter);										//30~60
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setMetalOverflowMargin(iMetalOverflowMargin);							//20~50
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setTestImageSave(bTestImageSave);										//FALSE
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setCInkMinWidthSpec(dCInkMinWidthSpec);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setLineLengthMinThick(dLineLengthMinThick);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setLineLengthSpec(dLineLengthSpec);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setCamCenterToCG(dCamCenterToCG);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setInkUnderflowSpec(dInkUnderflowSpec);
	//HTK 2022-03-16 DummyInspSpec 추가
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setDummyMinSizeSpec(dDummyMinSizeSpec);
	//KJH 2022-04-29 MincountCheckInsp 추가
	//KJH 2022-05-09 Mincount 상시 사용으로 변경
	//m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setMinCountCheckInspModeEnable(bMinCountCheckInspMode);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setMinCountCheckInspContinueCount(iMinCountCheckInspContinue);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setMinCountCheckInspTotalCount(iMinCountCheckInspTotal);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setWetoutRJudgeModeEnable(bWetoutRJudgeModeEnable);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setWetoutRMinSpec(dWetoutRMinSpec);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setWetoutRMaxSpec(dWetoutRMaxSpec);
	//KJH 2022-05-25 BM In <-> Cink In Insp 추가
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setWetoutBJudgeModeEnable(bWetoutBJudgeModeEnable);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setWetoutBMinSpec(dWetoutBMinSpec);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setWetoutBMaxSpec(dWetoutBMaxSpec);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setWetoutMinMaxJudgeModeEnable(bWetoutJudgeEnable);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setLineLengthFDC_Distance(dLineLengthFDC_Distance);
	//SJB 2022-11-11 Overflow Min 파라미터 추가
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setLineOverflowMinSpec(dLineOverflowMinSpec);
	//SJB 2022-11-19 CG Exposure Time 파라미터 추가
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setCGGrabExposure(iCGGrabExposure);

	//이물검사 파라미터
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSpecLSL(2, dDustSizeMin);										//0.2
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSpecUSL(2, dDustSizeMax);										//2
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSubSpec(2, dDustLineRatio);									//2.5
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSpecLSL(2, dDustSpecMin);										//0.1
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSpecUSL(2, dDustSpecMax);										//1.0

	// 액튐검사 파라미터
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSpecLSL(1, dDropsSizeInMin);									//0.15
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSpecUSL(1, dDropsSizeInMax);									//1
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSubSpec(3, dDropsLineInRatio);								//2.5
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSpecLSL(4, dDropsSizeOutMin);									//0.5
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSpecUSL(4, dDropsSizeOutMax);									//1
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSubSpecXTolerance(3, dDropsLineOutRatio);						//2.0
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSpecLSL(5, dDropsSpecMin);									//0.1
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSpecUSL(5, dDropsSpecMax);									//1.0
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSpecUSL(6, dDropsAfterImageRemove);							//80			//잔상 제거 파라미터 gray
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFeriAlignFindThresh(dFeriAlignFindThresh);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFeriAlignFindPreProcFlag(bFeriAlignFindPreProc);

	//KJH 2022-04-09 PN 반지름 비교 검사 추가
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setPNSizeInspEnable(bPNSizeInspEnable);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setPNSizeSpec(dPNSizeSpec);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setPNSizeSpecTorr(dPNSizeSpecTorr);

	// KBJ 2022-09-05 Rotate 파라미터 추가
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setRotateC_Spec(dRotateSpec);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setRotateC_binary_value(nRotateBinaryValue);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setRotateC_light_value(nRotateLightValue);

	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setELBCoverCInkMode(bELBCoverCInkModeEable);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setMetalSearchRemoveNoiseFlag(bMetalSearchRemoveNoiseFlag);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setTraceUsingAlignFlag(bTraceUsingAlignFlag);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setELBCG_Edge_CalcModeEnable(bELBCG_Edge_CalcModeEnable);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setELBPartial_DispensingModeEnable(bELBPartial_DispensingModeEnable);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setELBInsp_ShiftSearchEnable(bELBInsp_ShiftSearchEnable);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setAVI_Save_ImageTypeEnable(bAVI_Save_ImageTypeEnable);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setLineShift_DispensingModeEnable(bLineShift_DispensingModeEnable);
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setUseDummyCornerInsp(bCircle_ShapeDispensingModeEnable);
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setUseDoubleDispensingMode(bLineShift_DoubleDispensingModeEnable);
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setLineOverflowDispMode(bLineOverflowDispMode);
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setCircle_Cink1_Cink2MergeMode(bCircle_Cink1_Cink2MergeMode);
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setLine_InspWithTraceResultMode(bLine_InspWithTraceResultMode);

	// YCS 2022-12-01 Wetout_ASPC 스펙
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setWetoutASPCSpec(dWetout_ASPC_Spec);

	// KBJ 2022-12-05 Pocket 사이즈
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setELB_Line_Pocket_Size(dLine_Pocket_Size);

	// YCS 2022-12-24 PN To Metal 측정모드 파라미터 추가
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setPanelToMetalCalcEnable(bLine_PanelToMeatalCalcMode);

	// SJB 2023-01-02 CG to Start, End 파라미터 추가
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setCgToStartSpec(dCgToStartSpec);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setCgToEndSpec(dCgToEndSpec);

	// JCY 2023-01-12 CG Judge Area 파라미터 추가
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setCGJudgeArea(dCgJudgeSpec);

	m_pMain->saveIniFlag();
}

void CGridInspectionParamDlg::SaveNozzleAlign()
{
	double  dNozzleXYZInspMinSpec_Z		= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(0)->GetValue().dblVal;				 //Nozzle XYZ Insp Z Min Spec							//0.08				//노즐과 스테이지 최소거리
	double  dNozzleXYZInspMaxSpec_Z		= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(1)->GetValue().dblVal;				 //Nozzle XYZ Insp Z Max Spec							//0.13				//노즐과 스테이지 최대거리
	double  dNozzleXYZInspMinSpec_XY	= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(2)->GetValue().dblVal;				 //Nozzle XYZ Insp XY Min Spec							//0.02				//노즐과 카메라 중심 최소거리
	double  dNozzleXYZInspMaxSpec_XY	= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(3)->GetValue().dblVal;				 //Nozzle XYZ Insp XY Max Spec							//0.07				//노즐과 카메라 중심 최대거리
	double  dNozzleAngle				= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(4)->GetValue().dblVal;				 //Nozzle Align T Base Value							//30				//노즐 기본 각도
	bool	bNozzleAngleSearchMode		= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(5)->GetValue().boolVal;				 //Nozzle Search Methode Select [Otsu / Enhance]		//Enhance/Otsu		//노즐 기본 각도 찾는 방식
	int		nNozzleAlignGrabDelay		= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(6)->GetValue().iVal;					 //Nozzle Camera Grab Delay [미구현]
	bool    bCenterNozzleYAlignRecalc   = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(7)->GetValue().boolVal;     			 //Nozzle Align시 Mark Search이후 Y값 2진화 자동검사 모드 사용유무
	bool    bCenterNozzleAlignMethod    = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(8)->GetValue().boolVal;
	bool    bSideNozzleAlignRecalc 		= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(9)->GetValue().boolVal;	
	
	//SJB 2022-12-19 Nozzle Tip Size 파라미터 추가
	double	dNozzleTipSizeSpec			= m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(10)->GetValue().dblVal;					 //Nozzle Align시 Tip Size 검사 spec
	double	dNozzleTipSizeSpecTolerance = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(11)->GetValue().dblVal;					 //Nozzle Tip Size Tolerance

	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSpecLSL(7, dNozzleXYZInspMinSpec_Z);
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSpecUSL(7, dNozzleXYZInspMaxSpec_Z);
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSpecLSL(8, dNozzleXYZInspMinSpec_XY);
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSpecUSL(8, dNozzleXYZInspMaxSpec_XY);
	/// 

	// KBJ 2022-09-01 Control Param
	if (m_pMain->m_nLogInUserLevelType == MASTER)
	{
		m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setNozzleAngleSearchMode(bNozzleAngleSearchMode);
		m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setIncludedAngleLimit(dNozzleAngle);
		CString str;
		str.Format("%f", dNozzleAngle);

		CFormModelView* pModelView = (CFormModelView*)m_pMain->m_pForm[FORM_MODEL];
		TabModelAlgoPage* pModelPage = (TabModelAlgoPage*)pModelView->c_TabModelPage[m_nJob];
		pModelPage->GetDlgItem(IDC_EDIT_INCLUDED_ANGLE_LIMIT)->SetWindowTextA(str);
	}
	else
	{
		m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setIncludedAngleLimit(0.0);
		m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setNozzleAngleSearchMode(false);

		_variant_t var;
		var = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(5)->GetValue();
		var.boolVal = -1 * m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getNozzleAngleSearchMode();
		m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(5)->SetValue(var);
		var = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(4)->GetValue();
		var.dblVal = m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getIncludedAngleLimit();
		m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(4)->SetValue(var);

		dNozzleAngle = m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().getIncludedAngleLimit();
		CString str;
		str.Format("%f", dNozzleAngle);

		CFormModelView* pModelView = (CFormModelView*)m_pMain->m_pForm[FORM_MODEL];
		TabModelAlgoPage* pModelPage = (TabModelAlgoPage*)pModelView->c_TabModelPage[m_nJob];
		pModelPage->GetDlgItem(IDC_EDIT_INCLUDED_ANGLE_LIMIT)->SetWindowTextA(str);
	}
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setCenterNozzleYAlignRecalc(bCenterNozzleYAlignRecalc);
	
	/// 
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setNozzleAlignGrabDelay(nNozzleAlignGrabDelay);

	// KBJ 2022-12-06
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setCenterNozzleAlignMethod(bCenterNozzleAlignMethod);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setSideNozzleZAlignRecalc(bSideNozzleAlignRecalc);

	//SJB 2022-12-19 Nozzle Tip Size 파라미터 추가
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setNozzleTipSizeSpec(dNozzleTipSizeSpec);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setNozzleTipSizeSpecTolerance(dNozzleTipSizeSpecTolerance);

	m_pMain->saveIniFlag();
}

void CGridInspectionParamDlg::SaveZGapInsp()
{
	double  dNozzleToStageGapMinSpec = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(0)->GetValue().dblVal;				 //Nozzle Z Gap Insp Z Min Torr Spec						//0.08		//노즐과 스테이지 최소거리
	double  dNozzleToStageGapMaxSpec = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(1)->GetValue().dblVal;				 //Nozzle Z Gap Insp Z Max Torr Spec						//0.13		//노즐과 스테이지 최대거리
	double  dOriginToStageGapOffset = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getNozzleToStageGapOffset();	 //Nozzle Z Gap Camera Origin Offset value					//Origin Offset
	double  dNozzleToStageGapOffset  = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(2)->GetValue().dblVal;				 //Nozzle Z Gap Camera Center To Nozzle Offset Value		//0			//노즐과 스테이지 Offset
	int		nNozzleAlignGrabDelay	 = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(3)->GetValue().iVal;					 //Nozzle Z Gap Camera Grab Delay
	bool    bCenterNozzleYAlignRecalc = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(4)->GetValue().boolVal;     	 	 //Nozzle Align시 Mark Search이후 Y값 2진화 자동검사 모드 사용유무     
	bool    bCenterNozzleAlignMethod = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(5)->GetValue().boolVal;
	
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSpecLSL(7, dNozzleToStageGapMinSpec);
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSpecUSL(7, dNozzleToStageGapMaxSpec);
	
	// KBJ 2022-09-08 Zgpa Offset 저장부분 지워져서 다시 추가
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setNozzleToStageGapOffset(dNozzleToStageGapOffset);

	//kmb 221105 Z Gap Offset 변경 로그 추가
	if (dOriginToStageGapOffset != dNozzleToStageGapOffset) {
		CString strTemp;
		strTemp.Format("Before Z Gap Offset : %f -> After Z Gap Offset : %f", dOriginToStageGapOffset, dNozzleToStageGapOffset);
		theLog.InspParamsg(LOG_INSPPARA, strTemp);
	}

	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setNozzleAlignGrabDelay(nNozzleAlignGrabDelay);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setCenterNozzleYAlignRecalc(bCenterNozzleYAlignRecalc);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setCenterNozzleAlignMethod(bCenterNozzleAlignMethod); 	

	m_pMain->saveIniFlag();
}
//HTK 2022-07-11 Nozzle#45 검사기능 추가로 인한 모델파라미터 생성
void CGridInspectionParamDlg::SaveViewSubInspection()
{
	double  dOtsuThresh = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(0)->GetValue().dblVal;
	int  	nStartDelay = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(1)->GetValue().iVal;
	int  	nIntervalDelay = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(2)->GetValue().iVal;
	bool    bEnableInspection = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(3)->GetValue().boolVal;
	
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().set45DegreeInkInspThresh(dOtsuThresh);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().set45DegreenStartDelay(nStartDelay);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().set45DegreenIntervalDelay(nIntervalDelay);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().set45DegreeInkInspEnable(bEnableInspection);

}

//KJH2 2022-08-23 Attach Align(PC7), Film Inspection(PC8) 파라미터 분리
void CGridInspectionParamDlg::SaveAttachAlign()
{
	bool bFilmReAttachCheck =	   m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(0)->GetValue().boolVal;
	int bFilmReAttachCheckThresh = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(1)->GetValue().intVal;
	bool bFilmUVCheck =			   m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(2)->GetValue().boolVal;
	int bFilmUVCheckThresh =	   m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(3)->GetValue().intVal;
	int bFilmUVCheckSpec =		   m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(4)->GetValue().intVal;
	bool bUseReverseThresh=			m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(5)->GetValue().boolVal;
	// YCS 2022-10-31 필름 유무검사 영역 사이즈 고정
	bool bFixedSizeUse = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(6)->GetValue().boolVal;
	double dFixedSize = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(7)->GetValue().dblVal;

	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmReAttachCheckEnable(bFilmReAttachCheck);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmReAttachCheckThresh(bFilmReAttachCheckThresh);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmUVCheckEnable(bFilmUVCheck);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmUVCheckThresh(bFilmUVCheckThresh);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmUVCheckSpec(bFilmUVCheckSpec);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setUseReverseThresh(bUseReverseThresh);

	// YCS 2022-10-31 필름 유무검사 영역 사이즈 고정
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFixedFilmReattachUse(bFixedSizeUse);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFixedFilmReattachSize(dFixedSize);
}

void CGridInspectionParamDlg::SaveScratchInspection()
{
	bool bScratchCheck = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(0)->GetValue().boolVal;
	int nScratchThresh = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(1)->GetValue().intVal;
	double dMaskingRadius = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(2)->GetValue().dblVal;
	int nScratchInspSpec = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(3)->GetValue().intVal;
	int nScratchInspAreaSpec = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(4)->GetValue().intVal;
	bool bScratchInspThreadMode = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(5)->GetValue().boolVal;
	bool bScratchInspPeriMode = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(6)->GetValue().boolVal;
	double dLeftTopDistX = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(7)->GetValue().dblVal;
	double dLeftTopDistY = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(8)->GetValue().dblVal;
	double dInspWidth = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(9)->GetValue().dblVal;
	double dInspHeight = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(10)->GetValue().dblVal;

	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setScratchCheck(bScratchCheck);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setScratchThresh(nScratchThresh);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setScratchMaskingRadius(dMaskingRadius);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setScratchInspSpec(nScratchInspSpec);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setScratchInspAreaSpec(nScratchInspAreaSpec);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setScratchInspThreadMode(bScratchInspThreadMode);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setScratchInspPeriMode(bScratchInspPeriMode);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setScratchLeftTopDistX(dLeftTopDistX);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setScratchLeftTopDistY(dLeftTopDistY);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setScratchInspWidth(dInspWidth);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setScratchInspHeight(dInspHeight);
}

void CGridInspectionParamDlg::SaveFilmInspection()
{
	//HSJ 2022-01-17 필름검사 마크 찾을 시 캘리퍼 사용 유무
	bool bPanel = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(0)->GetValue().boolVal;
	bool bFilm = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(1)->GetValue().boolVal;
	// hsj 2022-02-07 필름검사 threshold 기능 추가
	bool bUsePanelThresh = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(2)->GetValue().boolVal;
	int nPanelThresh = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(3)->GetValue().iVal;

	// Film Reverse 파라미터
	//bool bUseFixture			 = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(4)->GetValue().iVal;
	bool bUseFilmReversePosition = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(4)->GetValue().iVal;

	//HSJ 2022-01-07 필름검사 스펙 기준 METHOD선택 추가
	int spec_method = m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(0)->GetValue().intVal;

	double spec_lx = m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(1)->GetValue().dblVal;
	double spec_lx_tor = m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(2)->GetValue().dblVal;
	
	double spec_rx = m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(3)->GetValue().dblVal;
	double spec_rx_tor = m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(4)->GetValue().dblVal;
	
	double spec_ly = m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(5)->GetValue().dblVal;
	double spec_ly_tor = m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(6)->GetValue().dblVal;
	
	double spec_ry = m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(7)->GetValue().dblVal;
	double spec_ry_tor = m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(8)->GetValue().dblVal;
	
	bool bScale =			m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(9)->GetValue().boolVal;
	double spec_lx_scale = m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(10)->GetValue().dblVal;
	double spec_rx_scale = m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(11)->GetValue().dblVal;
	double spec_ly_scale = m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(12)->GetValue().dblVal;
	double spec_ry_scale = m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(13)->GetValue().dblVal;

	// hsj 2022-02-07 필름검사 L check 기능 추가
	bool bPanelLcheck =				m_ctlPropGrid.GetProperty(0)->GetSubItem(2)->GetSubItem(0)->GetValue().boolVal;
	double dbPanel_Lcheck =			m_ctlPropGrid.GetProperty(0)->GetSubItem(2)->GetSubItem(1)->GetValue().dblVal;
	double dbPanel_Lcheck_Torr =	m_ctlPropGrid.GetProperty(0)->GetSubItem(2)->GetSubItem(2)->GetValue().dblVal;
	bool bFilmLcheck =				m_ctlPropGrid.GetProperty(0)->GetSubItem(2)->GetSubItem(3)->GetValue().boolVal;
	double dbFilm_Lcheck =			m_ctlPropGrid.GetProperty(0)->GetSubItem(2)->GetSubItem(4)->GetValue().dblVal;
	double dbFilm_Lcheck_Torr =		m_ctlPropGrid.GetProperty(0)->GetSubItem(2)->GetSubItem(5)->GetValue().dblVal;

	// Grab Delay 파라미터
	int iFilmInspGrabDelay = m_ctlPropGrid.GetProperty(0)->GetSubItem(3)->GetSubItem(0)->GetValue().iVal;

	// 예외 스펙 추가
	double dFilnInspExceptionSpec = m_ctlPropGrid.GetProperty(0)->GetSubItem(4)->GetSubItem(0)->GetValue().dblVal;
	int nFilnInspExceptionCount = m_ctlPropGrid.GetProperty(0)->GetSubItem(4)->GetSubItem(1)->GetValue().iVal;

	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setPanelCaliperEnable(bPanel);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmCaliperEnable(bFilm);

	//hsj 2022-01-27 필름검사 scale 파라미터 추가
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmScaleEnable(bScale);

	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmInspMethod(spec_method);

	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSubSpec(0, spec_lx);
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSubSpecXTolerance(0, spec_lx_tor);
	
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSubSpec(1, spec_rx);
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSubSpecXTolerance(1, spec_rx_tor);
	
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSubSpec(2, spec_ly);
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSubSpecXTolerance(2, spec_ly_tor);
	
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSubSpec(3, spec_ry);
	m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setDistanceInspSubSpecXTolerance(3, spec_ry_tor);
	
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmLXScale(spec_lx_scale);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmRXScale(spec_rx_scale);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmLYScale(spec_ly_scale);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmRYScale(spec_ry_scale);

	// hsj 2022-02-07 필름검사 L check 기능 추가
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setPanelLcheckEnable(bPanelLcheck);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setPanelLcheckLength(dbPanel_Lcheck);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setPanelLcheckTorr(dbPanel_Lcheck_Torr);

	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmLcheckEnable(bFilmLcheck);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmLcheckLength(dbFilm_Lcheck);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmLcheckTorr(dbFilm_Lcheck_Torr);
	
	// hsj 2022-02-07 필름검사 threshold 기능 추가
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setUsePanelThresh_Auto(bUsePanelThresh);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setPanelThresh(nPanelThresh);

	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setUseFilmReversePosition(bUseFilmReversePosition);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmInspGrabDelay(iFilmInspGrabDelay);
	
	//m_pMain->vt_job_info[m_nJob].model_info.getAlignInfo().setUseFixtureMark(bUseFixture);
	//CFormModelView *pModelView = (CFormModelView*)m_pMain->m_pForm[FORM_MODEL];
	//TabModelAlgoPage* pModelPage = (TabModelAlgoPage*)pModelView->c_TabModelPage[m_nJob];
	//pModelPage->m_btnUseFixtureMark.SetCheck(bUseFixture);

	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmInspExceptionSpec(dFilnInspExceptionSpec);
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmInspExceptionCount(nFilnInspExceptionCount);
}

void CGridInspectionParamDlg::OnClickedMfcpropertygridInspection()
{
	SetTimer(0, 100, NULL);
}

void CGridInspectionParamDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	KillTimer(nIDEvent);

	CMFCPropertyGridProperty* pHit = m_ctlPropGrid.GetCurSel();

	if (!pHit || pHit->IsGroup() || pHit->GetValue().vt== VT_BOOL)	return;

	_variant_t var = pHit->GetValue();
	CString str, strTemp;

	switch (var.vt)
	{
	case VT_UINT:	str.Format(_T("%d"),var.uintVal);		break;
	case VT_INT:	str.Format(_T("%d"),var.intVal);		break;
	case VT_I2:		str.Format(_T("%d"),var.iVal);		    break;
	case VT_I4:		str.Format(_T("%d"),var.lVal);		    break;
	case VT_R4:		str.Format(_T("%f"),var.fltVal);		break;
	case VT_R8:		str.Format(_T("%f"),var.dblVal);		break;
	}
	//kmb 220905 check Master Parameter 
	if (m_pMain->m_nLogInUserLevelType != MASTER && m_ctlPropGrid.GetCurSel()->IsEnabled() == FALSE)
	{
		strTemp = str;
	}
	else 
		strTemp = m_pMain->GetNumberBox(str, 5, -3000, 3000);

	switch (var.vt)
	{
	case VT_UINT:	var.uintVal = atoi(strTemp);	break;
	case VT_INT:	var.intVal = atoi(strTemp);		break;
	case VT_I2:		var.iVal = atoi(strTemp);		break;
	case VT_I4:		var.lVal = atoi(strTemp);		break;
	case VT_R4:		var.fltVal = atof(strTemp);		break;
	case VT_R8:		var.dblVal = atof(strTemp);		break;
	}

	// KBJ 2023-01-11 OAM(Once AttachFilm Mode) 추가
	int nMehtod = m_pMain->vt_job_info[m_nJob].algo_method;
	if (nMehtod == CLIENT_TYPE_1CAM_1SHOT_FILM)
	{
		int OAM_nAttach_Stage_Num = m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(1)->GetValue().iVal - 1;
		double OAM_dFilmPosition_X = m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(2)->GetValue().dblVal;
		double OAM_dFilmPosition_Y = m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(3)->GetValue().dblVal;

		double OAM_dAttatch_Stage_Rotate_X = 0.0;
		double OAM_dAttatch_Stage_Rotate_Y = 0.0;
		if (OAM_nAttach_Stage_Num >= 0)
		{
			OAM_dAttatch_Stage_Rotate_X = m_pMain->GetMachine(OAM_nAttach_Stage_Num).getRotateX(0, 0) + OAM_dFilmPosition_X;

			// stage 회전중심이 아래에 있을때
			if (m_pMain->GetMachine(OAM_nAttach_Stage_Num).getRotateY(0, 0) < 0)
			{
				OAM_dAttatch_Stage_Rotate_Y = m_pMain->GetMachine(OAM_nAttach_Stage_Num).getRotateY(0, 0) - OAM_dFilmPosition_Y;
			}
			else
			{
				OAM_dAttatch_Stage_Rotate_Y = m_pMain->GetMachine(OAM_nAttach_Stage_Num).getRotateY(0, 0) + OAM_dFilmPosition_Y;
			}
		}

		m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(4)->SetValue(OAM_dAttatch_Stage_Rotate_X);
		m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(5)->SetValue(OAM_dAttatch_Stage_Rotate_Y);
	}

	pHit->SetValue(var);

	CDialogEx::OnTimer(nIDEvent);
}

//2022.06.29 ksm Camera 및 Algorithm Dialog 창 ESC 사용 안되도록 수정
BOOL CGridInspectionParamDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
			return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}
//kmb 220905 check Master Parameter 
void CGridInspectionParamDlg::ChkMasterParam(int _USER_LEVEL)
{
	BOOL bEnable = FALSE;
	int nMehtod = m_pMain->vt_job_info[m_nJob].algo_method;
	if (_USER_LEVEL == MASTER) bEnable = TRUE;
	
	if (nMehtod == CLIENT_TYPE_ELB_CENTER_ALIGN)
	{
		m_ctlPropGrid.GetProperty(0)->GetSubItem(9)->GetSubItem(0)->Enable(bEnable);
		m_ctlPropGrid.GetProperty(0)->GetSubItem(9)->GetSubItem(1)->Enable(bEnable);
		m_ctlPropGrid.GetProperty(0)->GetSubItem(9)->GetSubItem(2)->Enable(bEnable);
		m_ctlPropGrid.GetProperty(0)->GetSubItem(9)->GetSubItem(3)->Enable(bEnable);
		m_ctlPropGrid.GetProperty(0)->GetSubItem(10)->GetSubItem(0)->Enable(bEnable);
		m_ctlPropGrid.GetProperty(0)->GetSubItem(10)->GetSubItem(1)->Enable(bEnable);
		m_ctlPropGrid.GetProperty(0)->GetSubItem(10)->GetSubItem(2)->Enable(bEnable);
		m_ctlPropGrid.GetProperty(0)->GetSubItem(10)->GetSubItem(3)->Enable(bEnable);
		m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(0)->Enable(bEnable);
		m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(1)->Enable(bEnable);
		m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(2)->Enable(bEnable);
		m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(3)->Enable(bEnable);
		m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(4)->Enable(bEnable);
		m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(5)->Enable(bEnable);
		m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(6)->Enable(bEnable);
		m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(7)->Enable(bEnable);
		m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(8)->Enable(bEnable);
		m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(9)->Enable(bEnable);
		m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(10)->Enable(bEnable);
		m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(28)->Enable(bEnable);
		m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(29)->Enable(bEnable);
		m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(8)->Enable(bEnable);	// YCS 2022-11-08 마스터 권한 시 Metal overflow margin 활성화
		m_ctlPropGrid.GetProperty(0)->GetSubItem(6)->GetSubItem(0)->Enable(bEnable);	// YCS 2022-12-02 마스터 권한 시 Rotate Center Spec 활성화
		m_ctlPropGrid.GetProperty(0)->GetSubItem(11)->GetSubItem(13)->Enable(bEnable);	// YCS 2022-12-24 마스터 권한 시 활성화
	}
	else if (nMehtod == CLIENT_TYPE_CENTER_NOZZLE_ALIGN)
	{
		m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(4)->Enable(bEnable);
		m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(5)->Enable(bEnable);
	}
	else if (nMehtod == CLIENT_TYPE_CENTER_SIDE_YGAP)
	{
		m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(2)->Enable(bEnable);
	}
	// KBJ 2023-01-11 OAM(Once AttachFilm Mode) 추가
	else if (nMehtod == CLIENT_TYPE_1CAM_1SHOT_FILM)
	{
		m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(4)->Enable(false);
		m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(5)->Enable(false);
	}
}

// YCS 2022-11-19 필름얼라인 각도 비교 스펙 파라미터 분리
void CGridInspectionParamDlg::InitFilmAlign()
{
	// hsj 2023-01-03 grid갱신
	if (m_pMain == NULL) return;

	double dFilmAngle = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmAngleSpec();
	bool bAngleInspection = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmAngleInspectionUse();

	// KBJ 2023-01-11 OAM(Once AttachFilm Mode) 추가
	bool OAM_bUseOnceAttachMode = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getOAM_Use();
	int OAM_nAttach_Stage_Num = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getOAM_Stage_Num();
	double OAM_dFilmPosition_X = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getOAM_FilmPosition_X();
	double OAM_dFilmPosition_Y = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getOAM_FilmPosition_Y();
	double OAM_dAttatch_Stage_Rotate_X = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getOAM_Rotate_X();
	double OAM_dAttatch_Stage_Rotate_Y = m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getOAM_Rotate_Y();

	CBBungGrid::stProperty GRID_STUFF[] =
	{
		P_ROOT(_T("TEXT:Film Align;")),
		P_SUBROOT(_T("TEXT:Film Angle Inspection;")),
				P_VAR(bAngleInspection, _T("TEXT:Use Film Angle Inspection;"), _T("TRUE: Inspection Use, FALSE: Inspection No Use")),
				P_VAR(dFilmAngle, _T("TEXT:Film Angle Spec;"),						_T("Suggested Angle : 0.5 (Degree)")),
		P_SUBROOT(_T("TEXT:At Once Attach Mode;")),
				P_VAR(OAM_bUseOnceAttachMode, _T("TEXT:Use at Once Attach Mode;"), _T("TRUE: Use, FALSE: No Use")),
				P_VAR(OAM_nAttach_Stage_Num, _T("TEXT:Attach Stage Num;"),  _T("0 : No Use, 1 : Stage-1, 2 : Stage-2")),
				P_VAR(OAM_dFilmPosition_X, _T("TEXT:Film Position X From Attach Align;"),  _T("LX Position = -5.77")),
				P_VAR(OAM_dFilmPosition_Y, _T("TEXT:Film Position Y From Attach Align;"),  _T("LY Position = -2.247")),
				P_VAR(OAM_dAttatch_Stage_Rotate_X, _T("TEXT:Once Mode Rotate X;"),  _T("Stage Rotate X")),
				P_VAR(OAM_dAttatch_Stage_Rotate_Y, _T("TEXT:Once Mode Rotate Y;"),  _T("Stage Rotate Y")),
		P_SUBROOT_END(),
		P_ROOT_END(),
	};

	int nSize = sizeof(GRID_STUFF) / sizeof(CBBungGrid::stProperty);

	m_ctlPropGrid.Initial(&m_font);
	m_ctlPropGrid.AppendProperty(GRID_STUFF, nSize);
}

// YCS 2022-11-19 필름얼라인 각도 비교 스펙 파라미터 분리
void	CGridInspectionParamDlg::SaveFilmAlign()
{
	bool bAngleInspectionUse = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(0)->GetValue().boolVal;
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmAngleInspectionUse(bAngleInspectionUse);
	double dAngleSpec = m_ctlPropGrid.GetProperty(0)->GetSubItem(0)->GetSubItem(1)->GetValue().dblVal;
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmAngleSpec(dAngleSpec);

	// KBJ 2023-01-11 OAM(Once AttachFilm Mode) 추가
	bool OAM_bUseOnceAttachMode = m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(0)->GetValue().boolVal;
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setOAM_Use(OAM_bUseOnceAttachMode);
	int OAM_nAttach_Stage_Num = m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(1)->GetValue().lVal;
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setOAM_Stage_Num(OAM_nAttach_Stage_Num);
	double OAM_dFilmPosition_X = m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(2)->GetValue().dblVal;
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setOAM_FilmPosition_X(OAM_dFilmPosition_X);
	double OAM_dFilmPosition_Y = m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(3)->GetValue().dblVal;
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setOAM_FilmPosition_Y(OAM_dFilmPosition_Y);
	double OAM_dAttatch_Stage_Rotate_X = m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(4)->GetValue().dblVal;
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setOAM_Rotate_X(OAM_dAttatch_Stage_Rotate_X);
	double OAM_dAttatch_Stage_Rotate_Y = m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(5)->GetValue().dblVal;
	m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setOAM_Rotate_Y(OAM_dAttatch_Stage_Rotate_Y);
}