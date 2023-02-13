// TabCameraPage.cpp: 구현 파일
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "TabCameraPage.h"
#include "afxdialogex.h"
#include "MaskMarkViewDlg.h"
#include "Caliper/DlgCaliper.h"
#include "ImageProcessing/MathUtil.h"
#include "ViewerEx.h"
#include "ImageProcessingSimulator.h"
#include "DlgMarkCopy.h"

// TabCameraPage 대화 상자
#include <fstream>
#include <numeric>
#include <algorithm>

template<typename Out>
void split(const string& s, char delim, Out result)
{
	stringstream ss(s);

	string item;
	while (getline(ss, item, delim))
		*(result++) = item;
}

vector<string> split(const string& s, char delim)
{
	vector<string> elems;
	split(s, delim, back_inserter(elems));
	return elems;
}

IMPLEMENT_DYNAMIC(TabCameraPage, CDialogEx)

TabCameraPage::TabCameraPage(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TAB_PANE_CAMERA, pParent)
{	
	m_nJobID = 0;
	sel_last_camera = 0;
	sel_last_position = 0;
}

TabCameraPage::~TabCameraPage()
{
}

void TabCameraPage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_BTN_DRAW_AREA, m_btnDrawArea);
	DDX_Control(pDX, IDC_LB_PATTERN_INDEX, m_LbPatternIndex);
	DDX_Control(pDX, IDC_LB_MATCHING_RATE, m_LbMatchingRate);
	DDX_Control(pDX, IDC_BTN_PATTERN_REGIST, m_btnPatternRegist);
	DDX_Control(pDX, IDC_BTN_PATTERN_SEARCH, m_btnPatternSearch);
	DDX_Control(pDX, IDC_BTN_PATTERN_VIEW, m_btnPatternView);
	DDX_Control(pDX, IDC_BTN_PATTERN_TEMP, m_btnPatternSend);
	DDX_Control(pDX, IDC_BTN_PATTERN_CLEAR, m_btnTraceClear);
	DDX_Control(pDX, IDC_BTN_SET_ROI, m_btnSetRoi);
	DDX_Control(pDX, IDC_BTN_IMAGE_SAVE, m_btnImageSave);
	DDX_Control(pDX, IDC_BTN_IMAGE_OPEN, m_btnImageOpen);
	DDX_Control(pDX, IDC_BTN_CAMERA_LIVE, m_btnCameraLive);
	DDX_Control(pDX, IDC_BTN_CAMERA_STOP, m_btnCameraStop);
	DDX_Control(pDX, IDC_BTN_SIZE_ORIGINAL, m_btnSizeOriginal);
	DDX_Control(pDX, IDC_BTN_SIZE_FIT, m_btnSizeFit);
	DDX_Control(pDX, IDC_LB_FOCUS_ENERGY_TITLE, m_LbFocusEnergyTitle);
	DDX_Control(pDX, IDC_LB_FOCUS_ENERGY, m_LbFocusEnergy);
	DDX_Control(pDX, IDC_BTN_FOCUS_MEASURE, m_btnFocusMeasure);
	DDX_Control(pDX, IDC_LB_SELECT_POSITION, m_LbSelectPosition);
	DDX_Control(pDX, IDC_LB_SELECT_CAMERA, m_LbSelectCamera);
	DDX_Control(pDX, IDC_BTN_PATTERN_DELETE, m_btnPtnDelete);
	DDX_Control(pDX, IDC_LB_SELECT_DRAW_SHAPE, m_lblDrawShape);
	DDX_Control(pDX, IDC_CB_SELECT_DRAW_SHAPE, m_cmbDrawShape);
	DDX_Control(pDX, IDC_LB_SELECT_PROCESS2, m_lblImageProc);
	DDX_Control(pDX, IDC_CB_SELECT_IMAGE_PROC2, m_cmbImageProc);
	DDX_Control(pDX, IDC_CB_SELECT_LIGHT_CHANNEL, m_cmbLightSelect);
	DDX_Control(pDX, IDC_CB_LIGHT_INDEX, m_cmbLightIndex);
	DDX_Control(pDX, IDC_EDIT_LIGHT_SETTING, m_edtLightValue);
	DDX_Control(pDX, IDC_LB_LIGHT_INDEX, m_lblLightIndex);
	DDX_Control(pDX, IDC_LB_LIGHT_CHANNEL, m_lblLightChannel);
	DDX_Control(pDX, IDC_SLIDER_LIGHT_SETTING, m_sliderLightSetting);
	DDX_Control(pDX, IDC_LB_REFERENCE_POS_X, m_lblRefPosX);
	DDX_Control(pDX, IDC_LB_REFERENCE_POS_Y, m_lblRefPosY);
	DDX_Control(pDX, IDC_LB_REFERENCE_POS_T, m_lblRefPosT);
	DDX_Control(pDX, IDC_LB_REFERENCE_POS_X_TITLE, m_lblRefPosXTitle);
	DDX_Control(pDX, IDC_LB_REFERENCE_POS_Y_TITLE, m_lblRefPosYTitle);
	DDX_Control(pDX, IDC_LB_REFERENCE_POS_T_TITLE, m_lblRefPosTTitle);
	DDX_Control(pDX, IDC_LB_FIXTURE_POS_X, m_lblFixturePosX);
	DDX_Control(pDX, IDC_LB_FIXTURE_POS_Y, m_lblFixturePosY);
	DDX_Control(pDX, IDC_LB_FIXTURE_POS_T, m_lblFixturePosT);
	DDX_Control(pDX, IDC_LB_FIXTURE_POS_X_TITLE, m_lblFixturePosXTitle);
	DDX_Control(pDX, IDC_LB_FIXTURE_POS_Y_TITLE, m_lblFixturePosYTitle);
	DDX_Control(pDX, IDC_LB_FIXTURE_POS_T_TITLE, m_lblFixturePosTTitle);
	DDX_Control(pDX, IDC_BTN_CAMERA_SIMULATION, m_btnSimulation);
	DDX_Control(pDX, IDC_BTN_CAMERA_INSPECTION, m_btnInspSimulation);
	DDX_Control(pDX, IDC_BTN_SHOW_CALIPER, m_btnUseCaliper);
	DDX_Control(pDX, IDC_BTN_MOVE_TO_CENTER_CALIPER, m_btnMoveCaliperToCenter);
	DDX_Control(pDX, IDC_BTN_SET_SUB_MARK, m_btnSetSubMark);
	DDX_Control(pDX, IDC_BTN_CAMERA_MAKE__MASKING, m_btnMakeMasking);
	DDX_Control(pDX, IDC_BTN_CAMERA_MAKE_METAL_DATA, m_btnShowGoldData);
	DDX_Control(pDX, IDC_BTN_CAMERA_MAKE_TRACE_DATA, m_btnMakeTraceData);
	DDX_Control(pDX, IDC_CB_SELECT_CAMERA, m_cmbSelectCamera);
	DDX_Control(pDX, IDC_CB_SELECT_POSITION, m_cmbSelectPosition);
	DDX_Control(pDX, IDC_CB_PATTERN_INDEX, m_cmbPatternIndex);
	DDX_Control(pDX, IDC_CHK_ENABLE_PROFILE, m_chkShowProfile);
	DDX_Control(pDX, IDC_CHK_SUB_INSP, m_chkSubInsp);
	DDX_Control(pDX, IDC_LB_MARK_OFFSET_X_TITLE, m_lblMarkOffsetXTitle);
	DDX_Control(pDX, IDC_LB_MARK_OFFSET_X, m_lblMarkOffsetX);
	DDX_Control(pDX, IDC_LB_MARK_OFFSET_Y, m_lblMarkOffsetY);
	DDX_Control(pDX, IDC_LB_EXPOSURETIME, m_lblExposureTime);
	DDX_Control(pDX, IDC_CB_SELECT_USER_SET, m_cbUserSet);
	DDX_Control(pDX, IDC_EDIT_EXPOSURE_SETTING, m_edtExposureTime);
	DDX_Control(pDX, IDC_SLIDER_EXPOSURE_SETTING, m_sliderExposureTime);
	DDX_Control(pDX, IDC_BTN_PATTERN_ALIGN_REF, m_btnAlignReference);
	DDX_Control(pDX, IDC_BTN_NOZZLE_INSP, m_btnInspNozzle);
	DDX_Control(pDX, IDC_BTN_FIXTURE_POS, m_btnFixturePos);
	DDX_Control(pDX, IDC_BTN_SCALE_CALC_POPUP, m_btnScaleCalc);
	DDX_Control(pDX, IDC_LB_ROI_X, m_LbRoiX);
	DDX_Control(pDX, IDC_LB_ROI_Y, m_LbRoiY);
	DDX_Control(pDX, IDC_BTN_PATTERN_COPY, m_btnPatternCopy);
}


BEGIN_MESSAGE_MAP(TabCameraPage, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_SIZE_ORIGINAL, &TabCameraPage::OnBnClickedBtnSizeOriginal)
	ON_BN_CLICKED(IDC_BTN_SIZE_FIT, &TabCameraPage::OnBnClickedBtnSizeFit)
	ON_BN_CLICKED(IDC_BTN_CAMERA_LIVE, &TabCameraPage::OnBnClickedBtnCameraLive)
	ON_BN_CLICKED(IDC_BTN_CAMERA_STOP, &TabCameraPage::OnBnClickedBtnCameraStop)
	ON_BN_CLICKED(IDC_BTN_IMAGE_SAVE, &TabCameraPage::OnBnClickedBtnImageSave)
	ON_BN_CLICKED(IDC_BTN_IMAGE_OPEN, &TabCameraPage::OnBnClickedBtnImageOpen)
	ON_BN_CLICKED(IDC_BTN_SHOW_CALIPER, &TabCameraPage::OnBnClickedBtnShowCaliper)
	ON_BN_CLICKED(IDC_BTN_MOVE_TO_CENTER_CALIPER, &TabCameraPage::OnBnClickedBtnMoveToCenterCaliper)
	ON_BN_CLICKED(IDC_BTN_DRAW_AREA, &TabCameraPage::OnBnClickedBtnDrawArea)
	ON_BN_CLICKED(IDC_BTN_PATTERN_DELETE, &TabCameraPage::OnBnClickedBtnPatternDelete)
	ON_BN_CLICKED(IDC_BTN_FOCUS_MEASURE, &TabCameraPage::OnBnClickedBtnFocusMeasure)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_CAMERA_SIMULATION, &TabCameraPage::OnBnClickedBtnCameraSimulation)
	ON_BN_CLICKED(IDC_BTN_PATTERN_REGIST, &TabCameraPage::OnBnClickedBtnPatternRegist)
	ON_BN_CLICKED(IDC_BTN_PATTERN_VIEW, &TabCameraPage::OnBnClickedBtnPatternView)
	ON_BN_CLICKED(IDC_BTN_PATTERN_SEARCH, &TabCameraPage::OnBnClickedBtnPatternSearch)
	ON_BN_CLICKED(IDC_BTN_SET_ROI, &TabCameraPage::OnBnClickedBtnSetRoi)
	ON_CBN_SELCHANGE(IDC_CB_SELECT_IMAGE_PROC2, &TabCameraPage::OnCbnSelchangeCbSelectImageProc2)
	ON_CBN_SELCHANGE(IDC_CB_SELECT_DRAW_SHAPE, &TabCameraPage::OnCbnSelchangeCbSelectDrawShape)
	ON_CBN_SELCHANGE(IDC_CB_SELECT_POSITION, &TabCameraPage::OnCbnSelchangeCbSelectPosition)
	ON_CBN_SELCHANGE(IDC_CB_PATTERN_INDEX, &TabCameraPage::OnCbnSelchangeCbPatternIndex)
	ON_CBN_SELCHANGE(IDC_CB_SELECT_LIGHT_CHANNEL, &TabCameraPage::OnCbnSelchangeCbSelectLightChannel)
	ON_CBN_SELCHANGE(IDC_CB_LIGHT_INDEX, &TabCameraPage::OnCbnSelchangeCbLightIndex)
	ON_EN_SETFOCUS(IDC_EDIT_MATCHING_RATE, &TabCameraPage::OnEnSetfocusEditMatchingRate)
	ON_EN_SETFOCUS(IDC_EDIT_ROI_X, &TabCameraPage::OnEnSetfocusEditROIX)
	ON_EN_SETFOCUS(IDC_EDIT_ROI_Y, &TabCameraPage::OnEnSetfocusEditROIY)
	ON_EN_SETFOCUS(IDC_EDIT_LIGHT_SETTING, &TabCameraPage::OnEnSetfocusEditLightSetting)
	ON_BN_CLICKED(IDC_CHK_ENABLE_PROFILE, &TabCameraPage::OnBnClickedChkEnableProfile)
	ON_BN_CLICKED(IDC_CHK_SUB_INSP, &TabCameraPage::OnBnClickedChkEnableSubInsp)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BTN_CAMERA_INSPECTION, &TabCameraPage::OnBnClickedBtnCameraInspection)
	ON_BN_CLICKED(IDC_BTN_SET_SUB_MARK, &TabCameraPage::OnBnClickedBtnSetSubMark)
	ON_CBN_SELCHANGE(IDC_CB_SELECT_CAMERA, &TabCameraPage::OnCbnSelchangeCbSelectCamera)
	ON_BN_CLICKED(IDC_BTN_PATTERN_TEMP, &TabCameraPage::OnBnClickedBtnPatternTemp)
	ON_EN_SETFOCUS(IDC_EDIT_EXPOSURE_SETTING, &TabCameraPage::OnEnSetfocusEditExposureSetting)
	//ON_EN_CHANGE(IDC_EDIT_EXPOSURE_SETTING, &TabCameraPage::OnEnChangeEditExposureSetting)
	ON_BN_CLICKED(IDC_BTN_CAMERA_MAKE__MASKING, &TabCameraPage::OnBnClickedBtnCameraMake)
	ON_BN_CLICKED(IDC_BTN_CAMERA_MAKE_METAL_DATA, &TabCameraPage::OnBnClickedBtnCameraMakeMetalData)
	ON_BN_CLICKED(IDC_BTN_CAMERA_MAKE_TRACE_DATA, &TabCameraPage::OnBnClickedBtnCameraMakeTraceData)
	ON_BN_CLICKED(IDC_BTN_PATTERN_CLEAR, &TabCameraPage::OnBnClickedBtnPatternClear)
	ON_BN_CLICKED(IDC_BTN_NOZZLE_INSP, &TabCameraPage::OnBnClickedBtnNozzleInsp)
	ON_BN_CLICKED(IDC_BTN_PATTERN_ALIGN_REF, &TabCameraPage::OnBnClickedBtnPatternAlignRef)
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_BTN_FIXTURE_POS, &TabCameraPage::OnBnClickedBtnFixturePos)
	ON_BN_CLICKED(IDC_BTN_SCALE_CALC_POPUP, &TabCameraPage::OnBnClickedBtnScaleCalcPopup)
	ON_BN_CLICKED(IDC_BTN_PATTERN_COPY, &TabCameraPage::OnBnClickedBtnPatternCopy)
END_MESSAGE_MAP()


// TabCameraPage 메시지 처리기
BOOL TabCameraPage::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);
	m_pMain = (CLET_AlignClientDlg*)AfxGetMainWnd();

	int nSel = 0;
	int nCam = 0;

	InitTitle(&m_LbPatternIndex, "Pattern Index(Copy)", 14.f, RGB(64, 64, 64));
	InitTitle(&m_LbMatchingRate, "Matching Rate(%)", 14.f, RGB(64, 64, 64));
	InitTitle(&m_LbFocusEnergyTitle, "Focus Energy / Avg Gray", 14.f, RGB(64, 64, 64));
	InitTitle(&m_LbSelectPosition, "Position", 14.f, RGB(64, 64, 64));
	InitTitle(&m_LbSelectCamera, "Camera", 14.f, RGB(64, 64, 64));
	
	InitTitle(&m_LbFocusEnergy, "0.0", 14.f, COLOR_UI_BODY);
	InitTitle(&m_lblDrawShape, "Draw Shape", 14.f, RGB(64, 64, 64));
	InitTitle(&m_lblLightChannel, "Light Channel", 14.f, RGB(64, 64, 64));
	InitTitle(&m_lblImageProc, "Image Processing", 14.f, RGB(64, 64, 64));
	InitTitle(&m_lblLightIndex, "Light Index", 14.f, RGB(64, 64, 64));

	InitTitle(&m_lblRefPosXTitle, "Reference Pos X", 14.f, RGB(64, 64, 64));
	InitTitle(&m_lblRefPosYTitle, "Reference Pos Y", 14.f, RGB(64, 64, 64));
	InitTitle(&m_lblRefPosTTitle, "Reference Pos T", 14.f, RGB(64, 64, 64));

	InitTitle(&m_lblRefPosX, "0.000", 14.f, COLOR_UI_BODY);
	InitTitle(&m_lblRefPosY, "0.000", 14.f, COLOR_UI_BODY);
	InitTitle(&m_lblRefPosT, "0.000", 14.f, COLOR_UI_BODY);

	InitTitle(&m_lblFixturePosX, "0.000", 14.f, COLOR_UI_BODY);
	InitTitle(&m_lblFixturePosY, "0.000", 14.f, COLOR_UI_BODY);
	InitTitle(&m_lblFixturePosT, "0.000", 14.f, COLOR_UI_BODY);

	InitTitle(&m_lblFixturePosXTitle, "Fixture Pos X", 14.f, RGB(64, 64, 64));
	InitTitle(&m_lblFixturePosYTitle, "Fixture Pos Y", 14.f, RGB(64, 64, 64));
	InitTitle(&m_lblFixturePosTTitle, "Fixture Pos T", 14.f, RGB(64, 64, 64));
	
	m_lblFixturePosX.SetColorText(255, RGB(255, 125, 0));
	m_lblFixturePosY.SetColorText(255, RGB(255, 125, 0));
	m_lblFixturePosT.SetColorText(255, RGB(255, 125, 0));

	//2021-05-11 KJH Ref 디스플레이 색상 변경
	m_lblRefPosX.SetColorText(255, RGB(255, 0, 255));
	m_lblRefPosY.SetColorText(255, RGB(255, 0, 255));
	m_lblRefPosT.SetColorText(255, RGB(255, 0, 255));

	InitTitle(&m_lblMarkOffsetXTitle, "Mark Offset X,Y", 14.f, RGB(64, 64, 64));
	InitTitle(&m_lblMarkOffsetX, "", 11.f, RGB(64, 64, 64));
	InitTitle(&m_lblMarkOffsetY, "", 11.f, RGB(64, 64, 64));
	InitTitle(&m_lblExposureTime, "Exposure Time", 14.f, RGB(64, 64, 64));
	InitTitle(&m_LbRoiX, "X", 14.f, RGB(64, 64, 64));
	InitTitle(&m_LbRoiY, "Y", 14.f, RGB(64, 64, 64));
	
	MainButtonInit(&m_btnPatternRegist);			m_btnPatternRegist.SetSizeText(14.f);
	MainButtonInit(&m_btnPatternSearch);			m_btnPatternSearch.SetSizeText(14.f);
	MainButtonInit(&m_btnPatternView);				m_btnPatternView.SetSizeText(14.f);
	MainButtonInit(&m_btnPatternSend);				m_btnPatternSend.SetSizeText(14.f);
	MainButtonInit(&m_btnTraceClear);				m_btnTraceClear.SetSizeText(14.f);
	MainButtonInit(&m_btnSetRoi);					m_btnSetRoi.SetSizeText(14.f);
	MainButtonInit(&m_btnImageSave);				m_btnImageSave.SetSizeText(14.f);
	MainButtonInit(&m_btnImageOpen);				m_btnImageOpen.SetSizeText(14.f);
	MainButtonInit(&m_btnCameraLive);				m_btnCameraLive.SetSizeText(14.f);
	MainButtonInit(&m_btnCameraStop);				m_btnCameraStop.SetSizeText(14.f);
	MainButtonInit(&m_btnSizeOriginal);				m_btnSizeOriginal.SetSizeText(14.f);
	MainButtonInit(&m_btnSizeFit);					m_btnSizeFit.SetSizeText(14.f);
	MainButtonInit(&m_btnDrawArea);					m_btnDrawArea.SetSizeText(14.f);	
	MainButtonInit(&m_btnFocusMeasure);				m_btnFocusMeasure.SetSizeText(14.f);
	MainButtonInit(&m_btnPtnDelete);				m_btnPtnDelete.SetSizeText(14.f);
	MainButtonInit(&m_btnPatternCopy);				m_btnPatternCopy.SetSizeText(14.f);
	MainButtonInit(&m_btnSimulation);				m_btnSimulation.SetSizeText(12.f);
	MainButtonInit(&m_btnInspSimulation);			m_btnInspSimulation.SetSizeText(12.f);
	MainButtonInit(&m_btnUseCaliper);				m_btnUseCaliper.SetSizeText(12.f);
	MainButtonInit(&m_btnMoveCaliperToCenter);		m_btnMoveCaliperToCenter.SetSizeText(12.f);
	MainButtonInit(&m_btnSetSubMark);				m_btnSetSubMark.SetSizeText(12.f);	
	MainButtonInit(&m_btnMakeMasking);              m_btnMakeMasking.SetSizeText(12.f);
	MainButtonInit(&m_btnShowGoldData);             m_btnShowGoldData.SetSizeText(12.f);
	MainButtonInit(&m_btnMakeTraceData);            m_btnMakeTraceData.SetSizeText(12.f);
	MainButtonInit(&m_btnInspNozzle);				m_btnInspNozzle.SetSizeText(12.f);
	MainButtonInit(&m_btnAlignReference);			m_btnAlignReference.SetSizeText(12.f);
	MainButtonInit(&m_btnFixturePos);				m_btnFixturePos.SetSizeText(12.f);
	MainButtonInit(&m_btnScaleCalc);				m_btnScaleCalc.SetSizeText(12.f);

	m_chkShowProfile.SetAlignTextCM();
	m_chkShowProfile.SetColorBkg(255, RGB(64, 64, 64));
	m_chkShowProfile.SetSizeText(12);
	m_chkShowProfile.SetSizeCheck(2);
	m_chkShowProfile.SetSizeCheckBox(3, 3, 30, 30);
	m_chkShowProfile.SetOffsetText(-5, 0);
	m_chkShowProfile.SetText(_T("Show Profile"));

	m_chkSubInsp.SetAlignTextCM();
	m_chkSubInsp.SetColorBkg(255, RGB(64, 64, 64));
	m_chkSubInsp.SetSizeText(12);
	m_chkSubInsp.SetSizeCheck(2);
	m_chkSubInsp.SetSizeCheckBox(3, 3, 30, 30);
	m_chkSubInsp.SetOffsetText(-5, 0);
	m_chkSubInsp.SetText(_T("Sub Inspection"));	

	if(m_pMain->m_stLightCtrlInfo.nType[0] == LTYPE_LLIGHT_LPC_COT_232)  m_sliderLightSetting.SetRange(0, 100);
	else m_sliderLightSetting.SetRange(0, 255);
	m_sliderLightSetting.SetPos(m_pMain->vt_job_info[m_nJobID].model_info.getLightInfo().getLightBright(nCam, 0, 0));

	int camCount = m_pMain->vt_job_info[m_nJobID].num_of_camera;
	
	//KJH 2021-05-29 전체 카메라 조명 ON
	for (int nCam = 0; nCam < camCount; nCam++)
	{
		//pjh 조명 저장값 다시 불러올때 적용
		for (int nLight = 0; nLight < m_pMain->vt_job_info[m_nJobID].light_info[0].num_of_using_light; nLight++)
		{
			int ctrl = m_pMain->vt_job_info[m_nJobID].light_info[0].controller_id[nLight] - 1;
			int ch = m_pMain->vt_job_info[m_nJobID].light_info[0].channel_id[nLight];
			int value = m_pMain->vt_job_info[m_nJobID].model_info.getLightInfo().getLightBright(nCam, nLight, nLight);

			if (m_pMain->m_stLightCtrlInfo.nType[nCam] == LTYPE_LLIGHT_LPC_COT_232)
			{
				m_pMain->SetLightBright(ctrl, ch, value == 0 ? 0 : value * 10);   // dh.jung 2021-07-12
			}
			else
			{
				m_pMain->SetLightBright(ctrl, ch, value);
			}
			Delay(50);
		}
	}

	nCam = 0;

	//exposure time 범위 지정
	m_sliderExposureTime.SetRange(25, 10000000);
	m_sliderExposureTime.SetPos(m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getExposureTime(nCam));
	
	/*CString strExposure;
	strExposure.Format("%d", m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getExposureTime(nCam));
	m_edtExposureTime.SetWindowTextA(strExposure);*/
		
	//User Set 저장 시
	m_cbUserSet.AddString("Default");
	m_cbUserSet.AddString("User 1");
	m_cbUserSet.AddString("User 2");
	m_cbUserSet.AddString("User 3");

	m_cbUserSet.SetCurSel(1);
	
	CString str;
	int pos = m_pMain->vt_job_info[m_nJobID].num_of_position;

	str.Format("%d", m_pMain->vt_job_info[m_nJobID].model_info.getLightInfo().getLightBright(nCam, 0, 0));
	m_edtLightValue.SetWindowTextA(str);

	for (int i = 0; i < pos; i++)
	{
		str.Format("Pos %d", i + 1);
		((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->AddString(str);
	}

	for (int nIndex = 0; nIndex < m_pMain->vt_job_info[m_nJobID].light_info[0].num_of_using_light; nIndex++)
	{
		str.Format("%s", m_pMain->vt_job_info[m_nJobID].light_info[0].channel_name[nIndex].c_str());
		((CComboBox*)GetDlgItem(IDC_CB_SELECT_LIGHT_CHANNEL))->AddString(str);
	}

	for (int nIndex = 0; nIndex < 4; nIndex++)
	{
		((CComboBox*)GetDlgItem(IDC_CB_LIGHT_INDEX))->AddString(m_pMain->vt_job_info[m_nJobID].light_info[0].index_name[nIndex].c_str());
	}

	for (int nIndex = 0; nIndex < MAX_PATTERN_INDEX; nIndex++)
	{
		//KJH 2021-12-20 Display와 Combo box Mark Index 매칭작업
		if (m_pMain->GetMatching(m_nJobID).getHalcon(nCam, 0, nIndex).getModelRead())			str.Format("%d : OK", nIndex + 1);
		else																					str.Format("%d : Empty", nIndex + 1);

		((CComboBox*)GetDlgItem(IDC_CB_PATTERN_INDEX))->AddString(str);
	}

	m_cmbImageProc.SetCurSel(0);
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->SetCurSel(0);
	((CComboBox*)GetDlgItem(IDC_CB_LIGHT_INDEX))->SetCurSel(0);
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_LIGHT_CHANNEL))->SetCurSel(0);
	((CComboBox*)GetDlgItem(IDC_CB_PATTERN_INDEX))->SetCurSel(0);

	GetDlgItem(IDC_EDIT_MATCHING_RATE)->SetWindowTextA("0.0");
	GetDlgItem(IDC_EDIT_ROI_X)->SetWindowTextA("2");
	GetDlgItem(IDC_EDIT_ROI_Y)->SetWindowTextA("2");

	for (int i = 0; i < m_pMain->vt_job_info[m_nJobID].camera_index.size(); i++)
	{
		int cam = m_pMain->vt_job_info[m_nJobID].camera_index[i];
		str.Format("%s", m_pMain->m_stCamInfo[cam].cName);
		m_cmbSelectCamera.AddString(str);
	}

	m_btnMoveCaliperToCenter.SetEnable(false);
	m_cmbSelectCamera.SetCurSel(0);
	m_cmbSelectPosition.SetCurSel(0);

	// YCS 2022-11-28 AutoMode 변경시 Exposure 변경 시퀀스 추가
	for (int real_cam = 0; real_cam < MAX_CAMERA; real_cam++)
		m_bExposureTimeChange[real_cam] = FALSE;

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

HBRUSH TabCameraPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (pWnd->GetDlgCtrlID() == IDC_CB_PATTERN_INDEX ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_MATCHING_RATE ||
		pWnd->GetDlgCtrlID() == IDC_CB_SELECT_CAMERA ||
		pWnd->GetDlgCtrlID() == IDC_CB_SELECT_POSITION ||
		pWnd->GetDlgCtrlID() == IDC_CB_SELECT_DRAW_SHAPE ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_LIGHT_SETTING ||
		pWnd->GetDlgCtrlID() == IDC_CB_SELECT_IMAGE_PROC2 ||
		pWnd->GetDlgCtrlID() == IDC_CB_SELECT_LIGHT_CHANNEL ||
		pWnd->GetDlgCtrlID() == IDC_CB_LIGHT_INDEX||
		pWnd->GetDlgCtrlID() == IDC_CB_SELECT_USER_SET||
		pWnd->GetDlgCtrlID() == IDC_EDIT_ROI_Y ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_ROI_X ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_EXPOSURE_SETTING)
	{
		pDC->SetBkColor(COLOR_UI_BODY);
		pDC->SetTextColor(COLOR_WHITE);
	}

	if (nCtlColor == CTLCOLOR_LISTBOX)
	{
		pDC->SetBkColor(COLOR_UI_BODY);
		pDC->SetTextColor(COLOR_WHITE);
	}
	return m_hbrBkg;
}

void TabCameraPage::InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}


void TabCameraPage::MainButtonInit(CButtonEx *pbutton, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}

void TabCameraPage::updateFrameDialog()
{
	dispViewSize();
	dispCameraLiveStop();
	dispDrawArea();
	dispMatchingRate();
	dispFocusMeasure();
	dispReferencePos();
	dispLightSetting();
	dispPatternIndex();
	dispSelectPos();
	dispCaliperSelect(TRUE);
	dispMarkOffset();
	dispExposuretime();
	dispFixturePos();
#ifndef JOB_INFO
	int cam = m_nTabIndex;
	int light_pos = m_cmbLightSelect.GetCurSel();
	int light_index = m_cmbLightIndex.GetCurSel();

	int v = m_pMain->getModel().getMachineInfo().getLightBright(cam, light_pos, light_index);
	m_sliderLightSetting.SetPos(v);
#else
	int nSel = m_cmbSelectCamera.GetCurSel();
	int nPos = m_cmbSelectPosition.GetCurSel();
	int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nSel];
	int light_pos = m_cmbLightSelect.GetCurSel();
	int light_index = m_cmbLightIndex.GetCurSel();

	int v = m_pMain->vt_job_info[m_nJobID].model_info.getLightInfo().getLightBright(nSel, light_pos, light_index);
	m_sliderLightSetting.SetPos(v);


	m_pMain->m_pDlgCaliper->m_nObject = real_cam;
	m_pMain->m_pDlgCaliper->m_nPos = nPos;
	
#endif

	disEnableButton();
}

void TabCameraPage::dispSelectPos()
{
	CString str;
	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int sel = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->ResetContent();

	int pos = m_pMain->vt_job_info[m_nJobID].num_of_position;
	for (int i = 0; i < pos; i++)
	{
		str.Format("%s", m_pMain->vt_job_info[m_nJobID].position_name[nCam][i].c_str());
		((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->AddString(str);
	}

	((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->SetCurSel(sel);
}

void TabCameraPage::dispPatternIndex()
{
	CString str;
	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
	int m_nSelectPatternIndex = ((CComboBox*)GetDlgItem(IDC_CB_PATTERN_INDEX))->GetCurSel();
	((CComboBox*)GetDlgItem(IDC_CB_PATTERN_INDEX))->ResetContent();

	for (int i = 0; i < MAX_PATTERN_INDEX; i++)
	{
		//KJH 2021-12-20 Display와 Combo box Mark Index 매칭작업
		if (m_pMain->GetMatching(m_nJobID).getHalcon(nCam, nPos, i).getModelRead())			str.Format("%d : OK", i + 1);
		else																				str.Format("%d : Empty", i + 1);

		((CComboBox*)GetDlgItem(IDC_CB_PATTERN_INDEX))->AddString(str);
	}

	((CComboBox*)GetDlgItem(IDC_CB_PATTERN_INDEX))->SetCurSel(m_nSelectPatternIndex);
}

void TabCameraPage::dispFocusMeasure()
{
	if (m_pMain->m_bFocusMeasure)	m_btnFocusMeasure.SetColorBkg(255, COLOR_BTN_SELECT);
	else							m_btnFocusMeasure.SetColorBkg(255, COLOR_BTN_BODY);
}

void TabCameraPage::dispCameraLiveStop()
{
	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];
	auto& cam = m_pMain->m_Cameras[real_cam];
	if (cam && cam->IsConnected() && cam->IsGrabbing())
	{
		m_btnCameraLive.SetColorBkg(255, COLOR_BTN_SELECT);
		m_btnCameraStop.SetColorBkg(255, RGB(64, 64, 64));
	}
	else
	{
		m_btnCameraLive.SetColorBkg(255, RGB(64, 64, 64));
		m_btnCameraStop.SetColorBkg(255, COLOR_BTN_SELECT);
	}
}

void TabCameraPage::dispViewSize()
{
	if (((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.IsFitImage())
	{
		m_btnSizeFit.SetColorBkg(255, COLOR_BTN_SELECT);
		m_btnSizeOriginal.SetColorBkg(255, RGB(64, 64, 64));
	}
	else
	{
		m_btnSizeOriginal.SetColorBkg(255, COLOR_BTN_SELECT);
		m_btnSizeFit.SetColorBkg(255, RGB(64, 64, 64));
	}
}

void TabCameraPage::dispMatchingRate()
{
	CString str;
	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
	int nIndex = ((CComboBox*)GetDlgItem(IDC_CB_PATTERN_INDEX))->GetCurSel();

	double matchingRate = m_pMain->GetMatching(m_nJobID).getMatchingRate(nCam, nPos, nIndex);
	str.Format("%.1f", matchingRate);
	GetDlgItem(IDC_EDIT_MATCHING_RATE)->SetWindowTextA(str);
}

void TabCameraPage::dispReferencePos()
{
	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();

	double posX = m_pMain->GetMatching(m_nJobID).getRefX(nCam, nPos);
	double posY = m_pMain->GetMatching(m_nJobID).getRefY(nCam, nPos);
	double posT = m_pMain->GetMatching(m_nJobID).getRefT(nCam, nPos);

	//2021-05-11 KJH Ref Null일때 디스플레이 예외처리

	//if (posX < 0) posX = 0;
	//if (posY < 0) posY = 0;

	CString strTemp;
	strTemp.Format("%.3f", posX);
	m_lblRefPosX.SetText(strTemp);
	strTemp.Format("%.3f", posY);
	m_lblRefPosY.SetText(strTemp);
	strTemp.Format("%.3f", posT);
	m_lblRefPosT.SetText(strTemp);

	m_lblRefPosXTitle.SetText(m_pMain->m_strRefDate_X[m_nJobID][nPos]);
	m_lblRefPosYTitle.SetText(m_pMain->m_strRefDate_Y[m_nJobID][nPos]);
	m_lblRefPosTTitle.SetText(m_pMain->m_strRefDate_T[m_nJobID][nPos]);
}

void TabCameraPage::dispFixturePos()
{
	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();

	double posX = m_pMain->GetMatching(m_nJobID).getFixtureX(nCam, nPos);
	double posY = m_pMain->GetMatching(m_nJobID).getFixtureY(nCam, nPos);
	double posT = m_pMain->GetMatching(m_nJobID).getFixtureT(nCam, nPos);

	//if (posX < 0) posX = 0;
	//if (posY < 0) posY = 0;

	CString strTemp;
	strTemp.Format("%.3f", posX);
	m_lblFixturePosX.SetText(strTemp);
	strTemp.Format("%.3f", posY);
	m_lblFixturePosY.SetText(strTemp);
	strTemp.Format("%.3f", posT);
	m_lblFixturePosT.SetText(strTemp);

	m_lblFixturePosXTitle.SetText(m_pMain->m_strFixtureDate_X[m_nJobID][nPos]);
	m_lblFixturePosYTitle.SetText(m_pMain->m_strFixtureDate_Y[m_nJobID][nPos]);
	m_lblFixturePosTTitle.SetText(m_pMain->m_strFixtureDate_T[m_nJobID][nPos]);
}

void TabCameraPage::OnBnClickedBtnSizeOriginal()
{
	((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.setOriginalImage();
	((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.setNavi(true);

	////////////////
	int nCam = m_cmbSelectCamera.GetCurSel();
	int org_pos = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getAlignOrginPos(nCam);
	int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];
	int W = m_pMain->m_stCamInfo[real_cam].w;
	int H = m_pMain->m_stCamInfo[real_cam].h;
	double xr = 0.5;
	double yr = 0.5;
	
	switch (org_pos) {
	case _LEFT_TOP_X25_Y25:  xr = 0.25; yr = 0.25; break;
	case _CENTER_TOP_X50_Y25:  xr = 0.5; yr = 0.25; break;
	case _RIGHT_TOP_X75_Y25:  xr = 0.75; yr = 0.25; break;
	case _LEFT_CENTER_X25_Y50:  xr = 0.25; yr = 0.5; break;
	case _CENTER_CENTER_X50_Y50:  xr = 0.5; yr = 0.5; break;
	case _RIGHT_CENTER_X75_Y50:  xr = 0.75; yr = 0.5; break;
	case _LEFT_BOTTOM_X25_Y75:  xr = 0.25; yr = 0.75; break;
	case _CENTER_BOTTOM_X50_Y75:  xr = 0.5; yr = 0.75; break;
	case _RIGHT_BOTTOM_X75_Y75:  xr = 0.75; yr = 0.75; break;
	case _NOTCH_X85_Y50:		xr = 0.85; yr = 0.5; break;
	}

	COLORREF color = COLOR_DARK_LIME;
	stFigure tempFig;
	tempFig.isDot = TRUE;
	tempFig.ptBegin.x = W * xr;
	tempFig.ptBegin.y = 0;
	tempFig.ptEnd.x = W * xr;
	tempFig.ptEnd.y = H;
	((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.addFigureLine(tempFig, 1, 1, color);
	////////////////


	dispViewSize();
}


void TabCameraPage::OnBnClickedBtnSizeFit()
{
	((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.OnFitImage();
	((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.setNavi(false);


	////////////////
	int nCam = m_cmbSelectCamera.GetCurSel();
	int org_pos = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getAlignOrginPos(nCam);
	int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];
	int W = m_pMain->m_stCamInfo[real_cam].w;
	int H = m_pMain->m_stCamInfo[real_cam].h;
	double xr = 0.5;
	double yr = 0.5;

	switch (org_pos) {
	case _LEFT_TOP_X25_Y25:  xr = 0.25; yr = 0.25; break;
	case _CENTER_TOP_X50_Y25:  xr = 0.5; yr = 0.25; break;
	case _RIGHT_TOP_X75_Y25:  xr = 0.75; yr = 0.25; break;
	case _LEFT_CENTER_X25_Y50:  xr = 0.25; yr = 0.5; break;
	case _CENTER_CENTER_X50_Y50:  xr = 0.5; yr = 0.5; break;
	case _RIGHT_CENTER_X75_Y50:  xr = 0.75; yr = 0.5; break;
	case _LEFT_BOTTOM_X25_Y75:  xr = 0.25; yr = 0.75; break;
	case _CENTER_BOTTOM_X50_Y75:  xr = 0.5; yr = 0.75; break;
	case _RIGHT_BOTTOM_X75_Y75:  xr = 0.75; yr = 0.75; break;
	case _NOTCH_X85_Y50:		xr = 0.85; yr = 0.5; break;
	}

	COLORREF color = COLOR_DARK_LIME;
	stFigure tempFig;
	tempFig.isDot = TRUE;
	tempFig.ptBegin.x = W * xr;
	tempFig.ptBegin.y = 0;
	tempFig.ptEnd.x = W * xr;
	tempFig.ptEnd.y = H;
	((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.addFigureLine(tempFig, 1, 1, color);
	////////////////


	dispViewSize();
}


void TabCameraPage::OnBnClickedBtnCameraLive()
{
	if (m_btnCameraLive.GetColorBkg() == COLOR_BTN_SELECT) return;

	CLET_AlignClientDlg* pMain = (CLET_AlignClientDlg*)AfxGetMainWnd();
	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];

	for (int i = 0; i < MAX_CAMERA; i++)
		pMain->SendMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_STOP, i);

	setCurrentCamLight();

	pMain->SendMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_LIVE, real_cam);

	updateFrameDialog();
}


void TabCameraPage::OnBnClickedBtnCameraStop()
{
	CLET_AlignClientDlg* pMain = (CLET_AlignClientDlg*)AfxGetMainWnd();
	int sel = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int cam = m_pMain->vt_job_info[m_nJobID].camera_index[sel];
	pMain->SendMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_STOP, cam);
	updateFrameDialog();
}


void TabCameraPage::OnBnClickedBtnImageSave()
{
	char szFilter[] = "Bmp Files (*.bmp) | *.bmp | All Files (*.*) | *.* ||";
	CFileDialog dlg(FALSE, NULL, "..\\*.bmp", OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT, szFilter);

	if (dlg.DoModal() == IDOK)
	{
		if (dlg.GetFileExt().GetLength() <= 0)
		{
#ifdef __MESSAGE_DLG_H__
			theApp.setMessage(MT_OK, "There is no extension.");
#else
			AfxMessageBox("There is no extension.");
#endif
			return;
		}

		int sel = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
		int cam = m_pMain->vt_job_info[m_nJobID].camera_index[sel];
		int W = m_pMain->m_stCamInfo[cam].w;
		int H = m_pMain->m_stCamInfo[cam].h;

		cv::Mat pImage(H, W, CV_8UC1, m_pMain->getCameraViewBuffer());
		cv::imwrite(std::string(dlg.GetPathName()), pImage);
		pImage.release();
	}
}


void TabCameraPage::OnBnClickedBtnImageOpen()
{
	CFileDialog dlg(TRUE, NULL, _T("Image File"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Image Files (*.bmp;*.jpg;)|*.bmp;*.jpg|All Files (*.*)|*.*||"), this);

	if (dlg.DoModal() != IDOK)		return;

	CString str = dlg.GetPathName();

	cv::Mat m_pImage = cv::imread(string(str), 0);

	if (m_pImage.empty())
	{
		AfxMessageBox("Check the Image Type");
		return;
	}

	int sel = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
	int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[sel];
	int W = m_pMain->m_stCamInfo[real_cam].w;
	int H = m_pMain->m_stCamInfo[real_cam].h;
	int camW = ((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.GetWidth();
	int camH = ((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.GetHeight();

	if (m_pImage.cols != W || m_pImage.rows != H)
	{
		if (m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_SCAN_INSP)
		{
			cv::Mat imgOriginal = m_pImage;
			cv::Mat imgCut = imgOriginal(cv::Rect(0, nPos * H, W, H));
			if (imgCut.cols * imgCut.rows != ((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.GetWidth() * ((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.GetHeight())
				((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.OnInitWithCamera(imgCut.cols, imgCut.rows, 8);
			((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.OnLoadImageFromPtr((BYTE*)imgCut.data);
			m_pMain->copyMemory(m_pMain->getSrcBuffer(real_cam), imgCut.data, W * H);
			m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 0), imgCut.data, W * H);
			m_pMain->copyMemory(m_pMain->getCameraViewBuffer(), imgCut.data, W * H);
			//((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.OnInitWithCamera(pImage->width, pImage->height, 8);
			//((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.OnLoadImageFromPtr((BYTE*)pImage->imageData);
		}
		else
		{
			CString str;
			str.Format("Image Size is not Valid.	size %d x %d", W, H);
			AfxMessageBox(str);
		}
		m_pImage.release();

		return;
	}
	else
	{
		// 임시.. 테스트용
		int bpp = ((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.GetBPP();
		if (m_pImage.cols != camW || m_pImage.rows != camH || (m_pImage.channels()*8)!=bpp)
		{
			((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.OnInitWithCamera(m_pImage.cols, m_pImage.rows, 8);
		}

		m_pMain->copyMemory(m_pMain->getSrcBuffer(real_cam), m_pImage.data, W * H);
		m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 0), m_pImage.data, W * H);
		m_pMain->copyMemory(m_pMain->getCameraViewBuffer(), m_pImage.data, W * H);
		
		((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.OnLoadImageFromPtr(m_pImage.data);
		::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_OPEN_IMAGE_FILENAME, (LPARAM)&str);

		m_pImage.release();
	}
}


void TabCameraPage::OnBnClickedBtnShowCaliper()
{
	CFormCameraView* pCamera = (CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA];

	if (pCamera->m_ViewerCamera.IsCalipersMode())
	{
		m_btnUseCaliper.SetColorBkg(255, COLOR_BTN_BODY);
		m_pMain->m_pDlgCaliper->ShowWindow(SW_HIDE);
		pCamera->m_ViewerCamera.InactiveInteraction(true);
        m_btnMoveCaliperToCenter.SetEnable(false);
	}
	else
	{
		m_btnUseCaliper.SetColorBkg(255, COLOR_BTN_SELECT);
		m_pMain->m_pDlgCaliper->ShowWindow(SW_SHOW);
		pCamera->m_ViewerCamera.ShowClipers();
		pCamera->m_ViewerCamera.InactiveInteraction(false);
        m_btnMoveCaliperToCenter.SetEnable(true);
	}

	pCamera->m_ViewerCamera.Invalidate();
}


void TabCameraPage::OnBnClickedBtnMoveToCenterCaliper()
{
	int sel = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
	int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[sel];
	int W = m_pMain->m_stCamInfo[real_cam].w;
	int H = m_pMain->m_stCamInfo[real_cam].h;


	_st32fPoint ptStart, ptEnd, ptStart2, ptEnd2;
	int nLine = m_pMain->m_pDlgCaliper->m_nLine;
	ptStart = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][nLine].getStartPt();
	ptEnd = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][nLine].getEndPt();

	double dx = (ptStart.x - ptEnd.x) / 2.0;
	double dy = (ptStart.y - ptEnd.y) / 2.0;

	CFormCameraView* pCamera = (CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA];

	CRect rect;

	pCamera->m_ViewerCamera.GetWindowRect(&rect);
	double x = rect.Width() / 2.;// pCamera->m_ViewerCamera.getResult().x + rect.Width() / 2;
	double y = rect.Height() / 2.;// pCamera->m_ViewerCamera.getResult().y + rect.Height() / 2;

	ptStart2.x = x + dx;
	ptStart2.y = y + dy;
	ptEnd2.x = x - dx;
	ptEnd2.y = y - dy;

	double fixtureX = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][nLine].getFixtureX();
	double fixtureY = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][nLine].getFixtureY();
	double fixtureT = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][nLine].getFixtureT();

	_st32fPoint ptStart3, ptEnd3;

	ptStart3.x = ptStart2.x - fixtureX;
	ptStart3.y = ptStart2.y - fixtureY;
	ptEnd3.x = ptEnd2.x - fixtureX;
	ptEnd3.y = ptEnd2.y - fixtureY;

	if (ptStart3.x > W)
	{
		int wd = ptEnd3.x - ptStart3.x;
		ptStart3.x = 0;
		ptEnd3.x = wd;
	}

	if (ptStart2.x > W)
	{
		int wd = ptEnd2.x - ptStart2.x;
		ptStart2.x = 0;
		ptEnd2.x = wd;
	}

	m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][nLine].setOrgStartPt(ptStart3);
	m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][nLine].setOrgEndPt(ptEnd3);
	m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][nLine].setStartPt(ptStart2);
	m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][nLine].setEndPt(ptEnd2);

	if (m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][nLine].getCircleCaliper())	m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][nLine].calcCaliperCircle();
	else																				m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][nLine].calcCaliperRect();
	m_pMain->m_pDlgCaliper->m_pCaliperParam->updateCoordinates();

	//pCamera->m_ViewerCamera.ClearCaliperOverlayDC();
	//CDC *pDC = pCamera->m_ViewerCamera.getCaliperOverlayDC();
	//if (m_pMain->m_pDlgCaliper->m_Caliper[sel][nPos][nLine].getCircleCaliper())	m_pMain->m_pDlgCaliper->m_Caliper[sel][nPos][nLine].DrawCaliper_Circle(pDC);
	//else																		m_pMain->m_pDlgCaliper->m_Caliper[sel][nPos][nLine].DrawCaliper(pDC);
	pCamera->m_ViewerCamera.Invalidate();
}


void TabCameraPage::OnBnClickedBtnDrawArea()
{
	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];
	int W = m_pMain->m_stCamInfo[real_cam].w;
	int H = m_pMain->m_stCamInfo[real_cam].h;
	auto view = (CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA];

	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
	CRect rectROI = m_pMain->GetMatching(m_nJobID).getSearchROI(nCam, nPos);
	CString strX, strY;
	GetDlgItem(IDC_EDIT_ROI_X)->GetWindowText(strX);
	GetDlgItem(IDC_EDIT_ROI_Y)->GetWindowText(strY);
	auto x_ = std::atof(strX);
	auto y_ = std::atof(strY);
	auto x2_ = x_ / 2.f;
	auto y2_ = y_ / 2.f;
	double xres = m_pMain->GetMachine(m_nJobID).getCameraResolutionX(nCam, 0);
	double yres = m_pMain->GetMachine(m_nJobID).getCameraResolutionY(nCam, 0);
	auto w_ = MAX(x2_ / xres, 10);
	auto h_ = MAX(y2_ / yres, 10);
	auto cx = (rectROI.left + rectROI.right) / 2.f;
	auto cy = (rectROI.top + rectROI.bottom) / 2.f;

	int viewer = m_pMain->vt_job_info[m_nJobID].viewer_index[nCam];
	((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetMetricTransform(Rigid(m_pMain->vt_viewer_info[viewer].resolution_x, 0, 0, 0, -m_pMain->vt_viewer_info[viewer].resolution_y, 0));

	view->m_ViewerCamera.SetBeginEnd(CPoint(lround(cx - w_), lround(cy - h_)), CPoint(lround(cx + w_), lround(cy + h_)));

	if (view->m_ViewerCamera.IsInteraction())
	{
		view->m_ViewerCamera.SetInteraction(false);
		view->m_ViewerCamera.InactiveInteraction();
	}
	else
	{
		auto sel = MAX(0, m_cmbDrawShape.GetCurSel());

		if(sel) view->m_ViewerCamera.SetRoiSelectionEllipseMode();
		else view->m_ViewerCamera.SetRoiSelectionMode();

		view->m_ViewerCamera.SetInteraction();
	}
	view->m_ViewerCamera.Invalidate();

	//if (((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.GetModeDrawingFigure())
	//{
	//	((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetModeDrawingFigure(false);
	//	((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetEnableModifyFigure(0, false);
	//	((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetEnableDrawFigure(0, false);

	//	m_btnUseCaliper.SetEnable(true);
	//}
	//else
	//{
	//	((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetEnableDrawAllFigures(true);
	//	((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetEnableFigureRectangle(0);
	//	((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetModeDrawingFigure(true);		

	//	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();

	//	CRect rectROI = m_pMain->GetMatching(m_nJobID).getSearchROI(nCam, nPos);
	//	
	//	//HTK 2022-06-29 Mark View창에 Tracker 기능 추가
	//	double xres = m_pMain->GetMachine(m_nJobID).getCameraResolutionX(nCam, 0);
	//	double yres = m_pMain->GetMachine(m_nJobID).getCameraResolutionY(nCam, 0);

	//	double xw = m_pMain->vt_job_info[m_nJobID].model_info.getGlassInfo().getFrameWidth() /xres;
	//	double yh = m_pMain->vt_job_info[m_nJobID].model_info.getGlassInfo().getFrameHeight()/yres;

	//	int nMethod = m_pMain->vt_job_info[m_nJobID].algo_method;
	//	if (xw<=0 || yh<=0 || nMethod== CLIENT_TYPE_ELB_CENTER_ALIGN)
	//		((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetSizeRectangle(0, CPoint(rectROI.left, rectROI.top), CPoint(rectROI.right, rectROI.bottom));
	//	else
	//	{
	//		CString str, strTemp;
	//		GetDlgItem(IDC_EDIT_ROI_X)->GetWindowText(str);			xw = atof(str)/xres;
	//		GetDlgItem(IDC_EDIT_ROI_Y)->GetWindowText(str);			yh = atof(str)/yres;
	//		((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetSizeRectangle(0, CPoint(rectROI.left, rectROI.top), CPoint(rectROI.left + xw, rectROI.top + yh));
	//	}

	//	//HTK 2022-04-28 DrawFigure 순서 변경
	//	((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetEnableModifyFigure(0, true);
	//	((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetEnableDrawFigure(0, true);

	//	m_btnUseCaliper.SetEnable(false);	
	//}

	updateFrameDialog();
}


void TabCameraPage::OnBnClickedBtnPatternDelete()
{
	CString str;
	((CComboBox*)GetDlgItem(IDC_CB_PATTERN_INDEX))->GetWindowText(str);
	int nIndex = ((CComboBox*)GetDlgItem(IDC_CB_PATTERN_INDEX))->GetCurSel();

	if (str.Find("OK") < 0)
	{
		m_pMain->fnSetMessage(1, "No Pattern to Delete");
		return;
	}

	BOOL bRet = m_pMain->fnSetMessage(2, "Do You Delete the Pattern?");
	if (bRet != TRUE) return;

	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];
	int W = m_pMain->m_stCamInfo[real_cam].w;
	int H = m_pMain->m_stCamInfo[real_cam].h;
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();

#ifndef JOB_INFO
	if (m_pMain->getModel().getAlignInfo().getUseMultiplexRoi(nCam))
#else
	if (m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getUseMultiplexRoi())
#endif
	{
		CRect rect(0, 0, 10, 10);
		m_pMain->GetMatching(m_nJobID).setSearchROI2(nCam, nPos, rect, nIndex);
		m_pMain->GetMatching(m_nJobID).setUseSearchROI2(nCam, nPos, nIndex, FALSE);
		updateFrameDialog();
		return;
	}

	CString strFilePath;
	strFilePath.Format("%s%s\\PAT%d%d%d.bmp", m_pMain->m_strCurrModelDir, m_pMain->vt_job_info[m_nJobID].job_name.c_str(), nCam, nPos, nIndex);

	CFileStatus fs;
	if (CFile::GetStatus(strFilePath, fs))
	{
		m_pMain->GetMatching(m_nJobID).getHalcon(nCam, nPos, nIndex).setModelRead(false);
		::DeleteFileA(strFilePath);
	}

	strFilePath.Format("%s%s\\MarkMask_%d%d%dInfo.bmp", m_pMain->m_strCurrModelDir, m_pMain->vt_job_info[m_nJobID].job_name.c_str(), nCam, nPos, nIndex);
	if (CFile::GetStatus(strFilePath, fs))
	{
		::DeleteFileA(strFilePath);
	}

	m_lblMarkOffsetX.SetText("0.0");
	m_lblMarkOffsetY.SetText("0.0");

	CPatternMatching* pPattern = &m_pMain->GetMatching(m_nJobID);
	pPattern->setMarkOffsetX(nCam, nPos, nIndex, 0.0);
	pPattern->setMarkOffsetY(nCam, nPos, nIndex, 0.0);

	updateFrameDialog();
}


void TabCameraPage::OnBnClickedBtnFocusMeasure()
{
	int nJob = 0;
	BOOL bReturn = FALSE;

	//	::SendMessage(m_pMain->m_hWnd, WM_VIEW_CONTROL, MSG_MD_DIFF_RESULT_WRITE, MAKELPARAM(nJob, bReturn ? TRUE: FALSE));

	if (m_pMain->m_bFocusMeasure)
	{
		m_pMain->m_bFocusMeasure = FALSE;
		KillTimer(TIMER_AUTO_FOCUS_MEASURE);
	}
	else
	{
		m_pMain->m_bFocusMeasure = TRUE;
		SetTimer(TIMER_AUTO_FOCUS_MEASURE, 500, NULL);
	}

	updateFrameDialog();
}


void TabCameraPage::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	switch (nIDEvent) {
	case TIMER_AUTO_FOCUS_MEASURE:
	{
		KillTimer(TIMER_AUTO_FOCUS_MEASURE);

		::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_FOCUS_MEASURE, 0);

		CString str;
		str.Format("%.f / %.f", m_pMain->m_dbFocusEnergy, m_pMain->m_dbAvgGray);
		m_LbFocusEnergy.SetText(str);

		if (m_pMain->m_bFocusMeasure)	SetTimer(TIMER_AUTO_FOCUS_MEASURE, 500, NULL);
		else	updateFrameDialog();

	}
	break;
	}

	CDialogEx::OnTimer(nIDEvent);
}

BOOL TabCameraPage::getPassword()
{
	CKeyPadDlg dlg;
	CString strTemp, strTime;

	strTemp.Format("Password");
	dlg.SetValueString(true, strTemp);
	dlg.DoModal();
	dlg.GetValue(strTemp);

	SYSTEMTIME time;
	::GetLocalTime(&time);
	strTime.Format("%02d%02d", time.wHour, time.wMinute);

	BOOL bFind = TRUE;
	int nLen = strTemp.GetLength();

	// 입력한 값 중에 시간 + 분 값이 있는지
	for (int i = 0; i < 4; i++)
	{
		if (strTemp.Find(strTime.GetAt(i)) < 0)
			bFind = FALSE;
	}

	if (bFind != TRUE)
	{
		return FALSE;
	}
	else return TRUE;
}
BOOL TabCameraPage::getHardPassword()
{
	CKeyPadDlg dlg;
	CString strTemp, strTime;

	strTemp.Format("PassWord");
	dlg.SetValueString(true, strTemp);
	dlg.DoModal();
	dlg.GetValue(strTemp);

	SYSTEMTIME time;
	::GetLocalTime(&time);
	strTime.Format("%02d%02d", time.wMinute, time.wHour);

	BOOL bFind = TRUE;
	int nLen = strTemp.GetLength();
	if (nLen < 8) bFind = FALSE;	// 6자리 이하 NG

	// 입력한 값 중에 시간 + 분 값이 있는지
	for (int i = 0; i < 4; i++)
	{
		if (strTemp.Find(strTime.GetAt(i)) < 0)
			bFind = FALSE;
	}

	if (bFind != TRUE)
	{
		return FALSE;
	}
	else return TRUE;
}

void TabCameraPage::OnBnClickedBtnCameraSimulation()
{
	if (m_pMain->m_bAutoSearch)
	{
		AfxMessageBox("Impossible In Auto Search Mode");
		return;
	}

	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
	int nIndex = ((CComboBox*)GetDlgItem(IDC_CB_PATTERN_INDEX))->GetCurSel();
	int nJob = m_nJobID;
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
	int W = m_pMain->m_stCamInfo[real_cam].w;
	int H = m_pMain->m_stCamInfo[real_cam].h;


#ifndef JOB_INFO
	int algoCount = m_pMain->m_pAlgorithmInfo.getAlgorithmCount();
	int algoRithm = 0;
	for (int i = 0; i < algoCount; i++)
	{
		std::vector<int> camBuf = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(i);
		for (int j = 0; j < camBuf.size(); j++)
		{
			if (nCam == camBuf.at(j))
			{
				algoRithm = i;
				break;
			}
		}
	}
#else
#endif

	BOOL bFixtureUse = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseFixtureMark();
	if (bFixtureUse)
	{
		double _posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetXPos();
		double _posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetYPos();
		double _posT = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetAngle();

		double fixtureX = _posX - m_pMain->GetMatching(nJob).getFixtureX(nCam, nPos);
		double fixtureY = _posY - m_pMain->GetMatching(nJob).getFixtureY(nCam, nPos);
		double fixtureT = _posT - m_pMain->GetMatching(nJob).getFixtureT(nCam, nPos);

		// 2022-05-10 KBJ
		m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_1].setFixtureX(fixtureX);
		m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_1].setFixtureY(fixtureY);
		m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_1].setFixtureT(fixtureT);
		m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_1].calcPointsByFixture();
	}

	_stPatternSelect pattern;
	pattern.nCam = nCam;
	pattern.nPos = nPos;
	pattern.nIndex = nIndex;
	pattern.nJob = nJob;

	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_SEARCH_CALIPER_INSP, (LPARAM)&pattern);
	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_DRAW_CALIPER_INSP, (LPARAM)&pattern);
}


void TabCameraPage::OnBnClickedBtnPatternRegist()
{
	//hsj 생산팀 요청으로 임시로 막아둠
	/*if (m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_NOZZLE_ALIGN)
	{
		if (getPassword() == FALSE) return;
	}*/

	if (m_pMain->m_bAutoSearch)
	{
		AfxMessageBox("Impossible In Auto Search Mode");
		return;
	}

	//if (AfxMessageBox("Do you want to register your mark?", MB_YESNO) != IDYES) return;
	if (m_pMain->fnSetMessage(2, "Do you want to register your mark?") != TRUE) return;

	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
	int nIndex = ((CComboBox*)GetDlgItem(IDC_CB_PATTERN_INDEX))->GetCurSel();

	_stPatternSelect pattern;
	pattern.nCam = nCam;
	pattern.nPos = nPos;
	pattern.nIndex = nIndex;
	pattern.nJob = m_nJobID;

	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_PATTERN_REGIST, (LPARAM)&pattern);
	CString strText;
	strText.Format("nJob = %d, nCam = %d, nPos = %d Mark Registered", m_nJobID, nCam, nPos);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strText);

	updateFrameDialog();
}


void TabCameraPage::OnBnClickedBtnPatternView()
{
	if (m_pMain->m_bAutoSearch)
	{
		AfxMessageBox("Impossible In Auto Search Mode");
		return;
	}

	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
	int nIndex = ((CComboBox*)GetDlgItem(IDC_CB_PATTERN_INDEX))->GetCurSel();
	int nJob = m_nJobID;
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
	int W = m_pMain->m_stCamInfo[real_cam].w;
	int H = m_pMain->m_stCamInfo[real_cam].h;

	bool bauto = false;

	CString strFilePath;
#ifndef JOB_INFO
	strFilePath.Format("%sPAT%d%d%d.bmp", m_pMain->m_strCurrentModelPath, nCam, nPos, nIndex);
#else
	strFilePath.Format("%s%s\\PAT%d%d%d.bmp", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[nJob].job_name.c_str(), nCam, nPos, nIndex);
#endif

	if (_access(strFilePath, 0) != 0)
	{
		AfxMessageBox(" Pattern is Empty!!");
		return;
	}

	double offsetX = m_pMain->GetMatching(nJob).getPosOffsetX(nCam, nPos, nIndex);
	double offsetY = m_pMain->GetMatching(nJob).getPosOffsetY(nCam, nPos, nIndex);

	// 210125 Postion 2번 등록시 찾지 못함
	CMaskMarkViewDlg dlg;
	cv::Mat srcMat = cv::imread(strFilePath.GetString(), cv::IMREAD_GRAYSCALE);
	// Lincoln Lee - 2022/08/22 - Fix Incorrect mark offset
	auto w2 = ((srcMat.cols - 1) / 2.f);
	auto h2 = ((srcMat.rows - 1) / 2.f);

	dlg.SetImage(srcMat.cols, srcMat.rows, w2 + offsetX, h2 + offsetY, srcMat.data);

	dlg.SetParam(0, m_pMain->GetMatching(nJob).getTemplateAreaShape(nCam, nPos, nIndex));
	dlg.SetParam(&(m_pMain->GetMatching(nJob).getHalcon(nCam, nPos, nIndex)));

#ifndef JOB_INFO
	dlg.SetMarkId(nCam, nPos, nIndex, m_pMain->m_pAlgorithmInfo.getAlgoFromCam(nCam));
#else
	dlg.SetMarkId(nCam, nPos, nIndex, m_nJobID);
#endif

	/*for (int i = 0; i < MAX_MASK_INDEX; i++)
	{
		dlg.SetParam(1, m_pMain->GetMatching(nJob).getMaskRectUse(nCam, nPos, nIndex, i), i);
		dlg.SetParam(2, m_pMain->GetMatching(nJob).getMaskRectShape(nCam, nPos, nIndex, i), i);
		dlg.SetParam(3, 0, i, m_pMain->GetMatching(nJob).getMaskRect(nCam, nPos, nIndex, i));
	}*/

	if (dlg.DoModal() != IDOK)
	{
		srcMat.release();
		return;
	}
	else
	{
		//hsj 생산팀 요청으로 임시로 막아둠
		/*if (m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_NOZZLE_ALIGN)
		{
			if (getPassword() == FALSE) return;

			if (AfxMessageBox("Please be sure to perform reference registration! \r\n \
				Do you want to do it automatically ?", MB_YESNO) != IDYES) bauto = false;
			else bauto = true;

		}*/
	}

	offsetX = dlg.m_dbPatRefX - w2;
	offsetY = dlg.m_dbPatRefY - h2;

	m_pMain->GetMatching(nJob).setPosOffsetX(nCam, nPos, nIndex, offsetX);
	m_pMain->GetMatching(nJob).setPosOffsetY(nCam, nPos, nIndex, offsetY);

	for (int i = 0; i < MAX_MASK_INDEX; i++)
	{
		m_pMain->GetMatching(nJob).setMaskRect(nCam, nPos, nIndex, i, dlg.m_crMaskRect[i]);
		m_pMain->GetMatching(nJob).setMaskRectShape(nCam, nPos, nIndex, i, dlg.m_bMaskRoiShape[i]);
		m_pMain->GetMatching(nJob).setMaskRectUse(nCam, nPos, nIndex, i, dlg.m_bMaskRoiUse[i]);
	}

	if (m_pMain->GetMatching(nJob).getUseMask(nCam, nPos, nIndex))
	{
		CString strMaskPath;

#ifndef JOB_INFO
		strMaskPath.Format("%s%s_%d%d%d%s", m_pMain->m_strCurrentModelPath, "MarkMask", nCam, nPos, nIndex, "Info.bmp");
#else
		strMaskPath.Format("%s%s\\%s_%d%d%d%s", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[nJob].job_name.c_str(), "MarkMask", nCam, nPos, nIndex, "Info.bmp");
#endif

		if (_access(strMaskPath, 0) == 0)
		{
			cv::Mat tmpimg, grayimg;
			tmpimg = cv::imread((LPCTSTR)strMaskPath, 1);
			cv::cvtColor(tmpimg, grayimg, CV_RGB2GRAY);
			threshold(grayimg, grayimg, 5, 255, cv::THRESH_BINARY);
			m_pMain->GetMatching(nJob).getHalcon(nCam, nPos, nIndex).halcon_ReadModelNew(srcMat.data, grayimg.data, srcMat.cols, srcMat.rows);
		}
	}

	m_pMain->GetMatching(nJob).getHalcon(nCam, nPos, nIndex).halcon_SetShapeModelOrigin(offsetX, offsetY);

	srcMat.release();

	//if (bauto) OnBnClickedBtnReferencePos();
}


void TabCameraPage::OnBnClickedBtnPatternSearch()
{
	if (m_pMain->m_bAutoSearch)
	{
		AfxMessageBox("Impossible In Auto Search Mode");
		return;
	}

#ifndef JOB_INFO
	int nCam = m_nTabIndex;

	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
	int nIndex = ((CComboBox*)GetDlgItem(IDC_CB_PATTERN_INDEX))->GetCurSel();

	int algoCount = m_pMain->m_pAlgorithmInfo.getAlgorithmCount();
	int algoRithm = 0;
	for (int i = 0; i < algoCount; i++)
	{
		std::vector<int> camBuf = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(i);
		for (int j = 0; j < camBuf.size(); j++)
		{
			if (nCam == camBuf.at(j))
			{
				algoRithm = i;
				break;
			}
		}
	}
#else

	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
	int nIndex = ((CComboBox*)GetDlgItem(IDC_CB_PATTERN_INDEX))->GetCurSel();
#endif

	_stPatternSelect pattern;
	pattern.nCam = nCam;
	pattern.nPos = nPos;
	pattern.nIndex = nIndex;
	pattern.nJob = m_nJobID;

	// hsj 2022-10-17 노즐얼라인 search방법 분기
	int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];
	int nMethod = m_pMain->vt_job_info[m_nJobID].algo_method;

	bool _calcMethod = m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().getCenterNozzleAlignMethod();

	if (_calcMethod && nMethod == CLIENT_TYPE_CENTER_NOZZLE_ALIGN)
	{
		::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_INSP_NOTCH_NOZZLE, MAKELPARAM(real_cam, m_nJobID));
		nCam = real_cam;
	}
	else
	{
		::SendMessage(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_PATTERN_SEARCH, (LPARAM)&pattern);
		
		// KBJ 2022-12-06 Side Nozzle Aling Recalc
		if (nCam == 0 && nMethod == CLIENT_TYPE_CENTER_NOZZLE_ALIGN && 
			m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().getSideNozzleZAlignRecalc())
		{
			int W = m_pMain->m_stCamInfo[real_cam].w;
			int H = m_pMain->m_stCamInfo[real_cam].h;

			double posX = m_pMain->GetMatching(m_nJobID).getFindInfo(nCam, 0).GetXPos();
			double posY = m_pMain->GetMatching(m_nJobID).getFindInfo(nCam, 0).GetYPos();
			if (m_pMain->GetMatching(m_nJobID).getFindInfo(nCam, 0).getScore() >= 50.0)
			{
				m_pMain->reCalculateMarkZGapYpos3(m_nJobID, m_pMain->getCameraViewBuffer(), W, H, &posX, &posY);
				m_pMain->GetMatching(m_nJobID).getFindInfo(nCam, 0).SetXPos(posX);
				m_pMain->GetMatching(m_nJobID).getFindInfo(nCam, 0).SetYPos(posY);
			}
		}

		::SendMessage(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_PATTERN_DISPLAY_RESULT, (LPARAM)&pattern);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Pattern 찾은 위치를 Header에 DIsplay
	double worldX, worldY;

	m_pMain->m_dCurrentSearchPos[0] = m_pMain->m_dCurrentSearchPos[2];
	m_pMain->m_dCurrentSearchPos[1] = m_pMain->m_dCurrentSearchPos[3];
	m_pMain->m_dCurrentRobotSearchPos[0] = m_pMain->m_dCurrentRobotSearchPos[2];
	m_pMain->m_dCurrentRobotSearchPos[1] = m_pMain->m_dCurrentRobotSearchPos[3];

	CFindInfo find = m_pMain->GetMatching(m_nJobID).getFindInfo(nCam, nPos);
	m_pMain->m_dCurrentSearchPos[2] = find.GetXPos();
	m_pMain->m_dCurrentSearchPos[3] = find.GetYPos();

	m_pMain->GetMachine(m_nJobID).PixelToWorld(nCam, nPos, find.GetXPos(), find.GetYPos(), &worldX, &worldY);
	m_pMain->m_dCurrentRobotSearchPos[2] = worldX;
	m_pMain->m_dCurrentRobotSearchPos[3] = worldY;

	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_SET_SEARCH_POS, NULL);

	//HTK 2022-06-29 Mark View창에 Tracker 기능 추가
	//if (((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.GetModeDrawingFigure())
	//{
	//	CRect rectROI = m_pMain->GetMatching(m_nJobID).getSearchROI(nCam, nPos);
	//	((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetSizeRectangle(0, CPoint(rectROI.left, rectROI.top), CPoint(rectROI.right, rectROI.bottom));
	//	((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetEnableModifyFigure(0, true);
	//	((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetEnableDrawFigure(0, true);
	//}	
}

void TabCameraPage::OnBnClickedBtnSetRoi()
{
	if (m_pMain->m_bAutoSearch)
	{
		AfxMessageBox("Impossible In Auto Search Mode");
		return;
	}

 	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
	int nIndex = ((CComboBox*)GetDlgItem(IDC_CB_PATTERN_INDEX))->GetCurSel();

	//if (AfxMessageBox("Do You Regist the ROI?", MB_YESNO) != IDYES)	return;
	if (m_pMain->fnSetMessage(2, "Do You Regist the ROI?") != TRUE) return;

	_stPatternSelect pattern;
	pattern.nCam = nCam;
	pattern.nPos = nPos;
	pattern.nIndex = nIndex;
	pattern.nJob = m_nJobID;

	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_PATTERN_SET_ROI, (LPARAM)&pattern);
}


void TabCameraPage::OnCbnSelchangeCbSelectImageProc2()
{
	int sel = m_cmbImageProc.GetCurSel();

	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_CAM_INSP_PROCESSING, sel);
}


void TabCameraPage::OnCbnSelchangeCbSelectDrawShape()
{
	int select = m_cmbDrawShape.GetCurSel();
	auto view = (CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA];

	if (select)
		view->m_ViewerCamera.SetRoiSelectionEllipseMode().SetInteraction();
	else view->m_ViewerCamera.SetRoiSelectionMode().SetInteraction();
}


void TabCameraPage::OnCbnSelchangeCbSelectPosition()
{
	sel_last_position = m_cmbSelectPosition.GetCurSel();
	updateFrameDialog();
}


void TabCameraPage::OnCbnSelchangeCbPatternIndex()
{
	updateFrameDialog();
}


void TabCameraPage::OnCbnSelchangeCbSelectLightChannel()
{
	CString str;
	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];

#ifndef JOB_INFO
	int sel = m_cmbLightSelect.GetCurSel();
	int nIndex = ((CComboBox*)GetDlgItem(IDC_CB_LIGHT_INDEX))->GetCurSel();

	str.Format("%d", m_pMain->getModel().getMachineInfo().getLightBright(cam, sel, nIndex));
	m_edtLightValue.SetWindowTextA(str);

	m_pMain->m_pViewDisplayInfo[cam].m_nLastLightChannel = sel;
#else
	int light_sel = m_cmbLightSelect.GetCurSel();
	int nIndex = ((CComboBox*)GetDlgItem(IDC_CB_LIGHT_INDEX))->GetCurSel();

	str.Format("%d", m_pMain->vt_job_info[m_nJobID].model_info.getLightInfo().getLightBright(nCam, light_sel, nIndex));
	m_edtLightValue.SetWindowTextA(str);

	m_pMain->vt_job_info[m_nJobID].light_info[nCam].last_channel = light_sel;
#endif

	updateFrameDialog();
}


void TabCameraPage::OnCbnSelchangeCbLightIndex()
{
	CString str;
	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int light_sel = m_cmbLightSelect.GetCurSel();
	int nIndex = ((CComboBox*)GetDlgItem(IDC_CB_LIGHT_INDEX))->GetCurSel();

	str.Format("%d", m_pMain->vt_job_info[m_nJobID].model_info.getLightInfo().getLightBright(nCam, light_sel, nIndex));
	m_edtLightValue.SetWindowTextA(str);
	updateFrameDialog();

	// index 선택시 해당 index의 값으로 모든 조명을 제어하자.
	for (int nLight = 0; nLight < m_pMain->vt_job_info[m_nJobID].light_info[nCam].num_of_using_light; nLight++)
	{
		int ctrl = m_pMain->vt_job_info[m_nJobID].light_info[nCam].controller_id[nLight] - 1;
		int ch = m_pMain->vt_job_info[m_nJobID].light_info[nCam].channel_id[nLight];
		int value = m_pMain->vt_job_info[m_nJobID].model_info.getLightInfo().getLightBright(nCam, nLight, nIndex);

		if (m_pMain->m_stLightCtrlInfo.nType[nCam] == LTYPE_LLIGHT_LPC_COT_232)
		{
			m_pMain->SetLightBright(ctrl, ch, value == 0 ? 0: value * 10);   // dh.jung 2021-07-12
		}
		else
		{
			m_pMain->SetLightBright(ctrl, ch, value);
		}
		Delay(50);
	}
}


void TabCameraPage::OnEnSetfocusEditMatchingRate()
{
	GetDlgItem(IDC_LB_MATCHING_RATE)->SetFocus();

#ifndef JOB_INFO
	CString str, strTemp;
	int nCam = m_nTabIndex;
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
	int nIndex = ((CComboBox*)GetDlgItem(IDC_CB_PATTERN_INDEX))->GetCurSel();

	str.Format("%.1f", m_pMain->GetMatching().getMatchingRate(nCam, nPos, nIndex));
	strTemp = m_pMain->GetNumberBox(str, 5, 0, 100);

	m_pMain->GetMatching().setMatchingRate(nCam, nPos, nIndex, atof(strTemp));
	GetDlgItem(IDC_EDIT_MATCHING_RATE)->SetWindowTextA(strTemp);
#else
	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	CString str, strTemp;
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
	int nIndex = ((CComboBox*)GetDlgItem(IDC_CB_PATTERN_INDEX))->GetCurSel();
	int nJob = m_nJobID;

	str.Format("%.1f", m_pMain->GetMatching(nJob).getMatchingRate(nCam, nPos, nIndex));
	strTemp = m_pMain->GetNumberBox(str, 5, 0, 100);

	if (atof(strTemp) < 70)
	{
		if (!getPassword())		strTemp.Format("70");
	}

	m_pMain->GetMatching(nJob).setMatchingRate(nCam, nPos, nIndex, atof(strTemp));
	GetDlgItem(IDC_EDIT_MATCHING_RATE)->SetWindowTextA(strTemp);
#endif 
}

void TabCameraPage::OnEnSetfocusEditROIX()
{
	GetDlgItem(IDC_LB_ROI_X)->SetFocus();
	CString str, strTemp;
	GetDlgItem(IDC_EDIT_ROI_X)->GetWindowText(str);
	strTemp = m_pMain->GetNumberBox(str, 5, -100, 100);

	GetDlgItem(IDC_EDIT_ROI_X)->SetWindowTextA(strTemp);
}

void TabCameraPage::OnEnSetfocusEditROIY()
{
	GetDlgItem(IDC_LB_ROI_Y)->SetFocus();
	CString str, strTemp;
	GetDlgItem(IDC_EDIT_ROI_Y)->GetWindowText(str);
	strTemp = m_pMain->GetNumberBox(str, 5, -100, 100);

	GetDlgItem(IDC_EDIT_ROI_Y)->SetWindowTextA(strTemp);
}

void TabCameraPage::OnEnSetfocusEditLightSetting()
{
	GetDlgItem(IDC_LB_LIGHT_CHANNEL)->SetFocus();

	CString str, strTemp;

#ifndef JOB_INFO
	int sel = m_cmbLightSelect.GetCurSel();
	int cam = m_nTabIndex;
	int ctrl = m_pMain->m_pViewDisplayInfo[m_nTabIndex].getLightController(sel) - 1;
	int nIndex = m_cmbLightIndex.GetCurSel();
	int ch = m_pMain->m_pViewDisplayInfo[cam].getLightChannel(sel);

	if (ch < 0) return;

	m_edtLightValue.GetWindowTextA(str);
	strTemp = m_pMain->GetNumberBox(str, 5, 0, 255);
	m_edtLightValue.SetWindowTextA(strTemp);

	int v = atoi(strTemp);
	m_pMain->SetLightBright(ctrl, ch, v);
	m_pMain->getModel().getMachineInfo().setLightBright(cam, sel, nIndex, v);

	m_sliderLightSetting.SetPos(v);
#else
	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	if (m_pMain->vt_job_info[m_nJobID].light_info[nCam].num_of_using_light <= 0) return; // Tkyuha 20220125 조명 예외 처리

	int light_sel = m_cmbLightSelect.GetCurSel();
	int nIndex = m_cmbLightIndex.GetCurSel();
	if (m_pMain->vt_job_info[m_nJobID].light_info[nCam].num_of_using_light <= light_sel) return; // Tkyuha 20220125 조명 예외 처리

	int ctrl = m_pMain->vt_job_info[m_nJobID].light_info[nCam].controller_id[light_sel] - 1;
	int ch = m_pMain->vt_job_info[m_nJobID].light_info[nCam].channel_id[light_sel];

	if (ch < 0) return;

	m_edtLightValue.GetWindowTextA(str);
	strTemp = m_pMain->GetNumberBox(str, 5, 0, 255);
	m_edtLightValue.SetWindowTextA(strTemp);

	int v = atoi(strTemp);
	//	dh.jung 2021-05-11 change
	//	m_pMain->vt_job_info[m_nJobID].model_info.getLightInfo().saveLightBrightRestore(nCam, light_sel, nIndex);
	//	end

	if (m_pMain->m_stLightCtrlInfo.nType[0] == LTYPE_LLIGHT_LPC_COT_232)
	{
		m_pMain->SetLightBright(ctrl, ch, v == 0 ? 0: v * 10);   // dh.jung 2021-07-12
	}
	else
	{
		m_pMain->SetLightBright(ctrl, ch, v);
	}
	// end	
	m_pMain->vt_job_info[m_nJobID].model_info.getLightInfo().setLightBright(nCam, light_sel, nIndex, v);

	m_sliderLightSetting.SetPos(v);
#endif

	// dh.jung 2021-05-26 add light svae flag
	m_pMain->vt_job_info[m_nJobID].model_info.getLightInfo().lightBrightSaveFlag(FALSE);


	updateFrameDialog();

}


void TabCameraPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (nSBCode == SB_ENDSCROLL)		return;

	if (pScrollBar)
	{
		// 어떤 슬라이더인지 검사
		if (pScrollBar == (CScrollBar*)&m_sliderLightSetting)
		{
#ifndef JOB_INFO
			CString str;
			int nPos = m_sliderLightSetting.GetPos();

			str.Format("%d", nPos);
			m_edtLightValue.SetWindowTextA(str);

			int sel = m_cmbLightSelect.GetCurSel();
			int cam = m_nTabIndex;
			int ctrl = m_pMain->m_pViewDisplayInfo[m_nTabIndex].getLightController(sel) - 1;
			int nIndex = m_cmbLightIndex.GetCurSel();
			int ch = m_pMain->m_pViewDisplayInfo[m_nTabIndex].getLightChannel(sel);
			//m_LightBright[sel][nIndex] = nPos;

			//m_pMain->SetLightBright(ctrl, ch, m_LightBright[sel][nIndex]);
			//m_pMain->getModel().getMachineInfo().setLightBright(cam, sel, nIndex, m_LightBright[sel][nIndex]);

			//m_pMain->SetLightBright(ctrl, ch, nPos);
			if (m_pMain->m_stLightCtrlInfo.nType[cam] == LTYPE_LLIGHT_LPC_COT_232)
			{
				m_pMain->SetLightBright(ctrl, ch, nPos == 0 ? 0: nPos * 10);   // dh.jung 2021-07-12
			}
			else
			{
				m_pMain->SetLightBright(ctrl, ch, nPos);
			}
			Delay(50);

			m_pMain->getModel().getMachineInfo().setLightBright(cam, sel, nIndex, nPos);

			updateFrameDialog();
#else

			int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
			int nSel = m_cmbSelectCamera.GetCurSel();
			int nIndex = m_cmbPatternIndex.GetCurSel();

			CString str;
			int value = m_sliderLightSetting.GetPos();

			str.Format("%d", value);
			m_edtLightValue.SetWindowTextA(str);

			if (m_pMain->vt_job_info[m_nJobID].light_info[nCam].num_of_using_light <= 0) return; // Tkyuha 20220125 조명 예외 처리

			int light_sel = m_cmbLightSelect.GetCurSel();
			int light_nIndex = m_cmbLightIndex.GetCurSel();
			if (m_pMain->vt_job_info[m_nJobID].light_info[nCam].num_of_using_light <= light_sel) return; // Tkyuha 20220125 조명 예외 처리
			int ctrl = m_pMain->vt_job_info[m_nJobID].light_info[nCam].controller_id[light_sel] - 1;
			int ch = m_pMain->vt_job_info[m_nJobID].light_info[nCam].channel_id[light_sel];

			//	dh.jung 2021-05-11 change
			//	m_pMain->vt_job_info[m_nJobID].model_info.getLightInfo().saveLightBrightRestore(nCam, light_sel, nIndex);
			//	end

			if (m_pMain->m_stLightCtrlInfo.nType[nCam] == LTYPE_LLIGHT_LPC_COT_232)
			{
				m_pMain->SetLightBright(ctrl, ch, value == 0 ? 0: value * 10);
			}
			else
			{
				m_pMain->SetLightBright(ctrl, ch, value);
			}

			m_pMain->vt_job_info[m_nJobID].model_info.getLightInfo().setLightBright(nCam, light_sel, light_nIndex, value);

			updateFrameDialog();

			// dh.jung 2021-05-26 add light svae flag
			m_pMain->vt_job_info[m_nJobID].model_info.getLightInfo().lightBrightSaveFlag(FALSE);
#endif
		}

		if (pScrollBar == (CScrollBar*)&m_sliderExposureTime)
		{
			
			CString str;
			int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
			int nPos = m_sliderExposureTime.GetPos();

			str.Format("%d", nPos);
			m_edtExposureTime.SetWindowTextA(str);

#ifndef _SAPERA			
			int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];
			auto& cam = m_pMain->m_Cameras[real_cam];
			if (cam && cam->IsConnected()) cam->SetExposure(nPos);
			m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().setExposureTime(real_cam, nPos);

			// YCS 2022-11-28 AutoMode 변경시 Exposure 변경 시퀀스 추가
			m_bExposureTimeChange[real_cam] = TRUE;
#endif // !_SAPERA
		}
	}
	else
	{
	}

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void TabCameraPage::dispDrawArea()
{
	if (((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.GetModeDrawingFigure())		m_btnDrawArea.SetColorBkg(255, COLOR_BTN_SELECT);
	else																								m_btnDrawArea.SetColorBkg(255, RGB(64, 64, 64));
}

void TabCameraPage::init_light_setting()
{
#ifndef JOB_INFO
	m_cmbLightSelect.ResetContent();
	m_cmbLightIndex.ResetContent();

	CString strTemp;
	int num_of_light = m_pMain->m_pViewDisplayInfo[m_nTabIndex].get_num_of_using_light();
	for (int i = 0; i < num_of_light; i++)
	{
		strTemp.Format("%s", m_pMain->m_pViewDisplayInfo[m_nTabIndex].getLightChannelName(i));
		m_cmbLightSelect.AddString(strTemp);
	}

	for (int i = 0; i < MAX_LIGHT_INDEX; i++)
	{
		strTemp.Format("%s", m_pMain->m_pViewDisplayInfo[m_nTabIndex].getLightIndexName(i));
		m_cmbLightIndex.AddString(strTemp);
	}

	if (num_of_light > 0)
	{
		m_cmbLightSelect.SetCurSel(m_pMain->m_pViewDisplayInfo[m_nTabIndex].m_nLastLightChannel);
	}

	m_cmbLightIndex.SetCurSel(m_pMain->m_pViewDisplayInfo[m_nTabIndex].m_nLastLightIndex);
#else
	int nCam = m_cmbSelectCamera.GetCurSel();
	int nPos = m_cmbSelectPosition.GetCurSel();
	int nIndex = m_cmbPatternIndex.GetCurSel();

	m_cmbLightSelect.ResetContent();
	m_cmbLightIndex.ResetContent();

	CString strTemp;
	int num_of_light = m_pMain->vt_job_info[m_nJobID].light_info[nCam].num_of_using_light;
	for (int i = 0; i < num_of_light; i++)
	{
		strTemp.Format("%s", m_pMain->vt_job_info[m_nJobID].light_info[nCam].channel_name[i].c_str());
		m_cmbLightSelect.AddString(strTemp);
	}

	for (int i = 0; i < MAX_LIGHT_INDEX; i++)
	{
		strTemp.Format("%s", m_pMain->vt_job_info[m_nJobID].light_info[nCam].index_name[i].c_str());
		m_cmbLightIndex.AddString(strTemp);
	}

	if (num_of_light > 0)
	{
		m_cmbLightSelect.SetCurSel(m_pMain->vt_job_info[m_nJobID].light_info[nCam].last_channel);
	}

	m_cmbLightIndex.SetCurSel(m_pMain->vt_job_info[m_nJobID].light_info[nCam].last_index);
#endif
}


void TabCameraPage::dispLightSetting()
{
#ifndef JOB_INFO
	// 조명값 디스플레이
	CString str;
	int sel = m_cmbLightSelect.GetCurSel();
	int nIndex = m_cmbLightIndex.GetCurSel();

	if (sel < 0) return;

	//str.Format("%d", m_LightBright[sel][nIndex]);
	str.Format("%d", m_pMain->getModel().getMachineInfo().getLightBright(m_nTabIndex, sel, nIndex));
	m_edtLightValue.SetWindowTextA(str);
#else
	int nSel = m_cmbSelectCamera.GetCurSel();
	int nCam = m_pMain->vt_job_info[m_nJobID].camera_index[nSel];
	int nPos = m_cmbSelectPosition.GetCurSel();
	int nIndex = m_cmbPatternIndex.GetCurSel();

	CString str;
	int light_sel = m_cmbLightSelect.GetCurSel();
	int light_nIndex = m_cmbLightIndex.GetCurSel();

	if (light_sel < 0) return;

	//str.Format("%d", m_LightBright[sel][nIndex]);
	str.Format("%d", m_pMain->vt_job_info[m_nJobID].model_info.getLightInfo().getLightBright(nSel, light_sel, light_nIndex));
	m_edtLightValue.SetWindowTextA(str);
#endif
}

void TabCameraPage::dispCaliperSelect(int bDisplay)
{
#ifndef JOB_INFO
	CDlgCaliper* pDlgCaliper = m_pMain->m_pDlgCaliper;
	pDlgCaliper->m_nObject = m_nTabIndex;
	pDlgCaliper->m_pCaliperParam->m_pCaliper = &pDlgCaliper->m_Caliper[m_nTabIndex][pDlgCaliper->m_nLine];
	pDlgCaliper->m_pCaliperResult->m_pCaliper = &pDlgCaliper->m_Caliper[m_nTabIndex][pDlgCaliper->m_nLine];
	if (bDisplay);
	else
	{
		pDlgCaliper->m_pViewer->ClearCaliperOverlayDC();
		pDlgCaliper->m_pViewer->ClearOverlayDC();
	}
	pDlgCaliper->m_Caliper[m_nTabIndex][pDlgCaliper->m_nLine].DrawCaliper(pDlgCaliper->m_Caliper[m_nTabIndex][pDlgCaliper->m_nLine].getCaliperDC());
	pDlgCaliper->m_pViewer->Invalidate();
	pDlgCaliper->m_pCaliperParam->updateUIFromData();
	pDlgCaliper->m_pCaliperParam->updateCoordinates();
#ifndef SKIP_XLIST
	pDlgCaliper->m_pCaliperResult->updateCaliperList();
#endif

	//Camera Tab 옮길시 무조건 Disable
	CFormCameraView* pCamera = (CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA];
	pCamera->m_ViewerCamera.setUseCaliper(FALSE);
	pCamera->m_ViewerCamera.setDrawCaliper(FALSE);
	m_btnUseCaliper.SetColorBkg(255, COLOR_BTN_BODY);
	m_pMain->m_pDlgCaliper->ShowWindow(SW_HIDE);
	m_btnMoveCaliperToCenter.SetEnable(false);
	m_btnDrawArea.SetEnable(true);
#else
	if (bDisplay != TRUE)		return;


	int nSel = m_cmbSelectCamera.GetCurSel();
	int nCam = m_pMain->vt_job_info[m_nJobID].camera_index[nSel];
	int nPos = m_cmbSelectPosition.GetCurSel();
	int nIndex = m_cmbPatternIndex.GetCurSel();

	CDlgCaliper* pDlgCaliper = m_pMain->m_pDlgCaliper;
	pDlgCaliper->m_nObject = nCam;
	pDlgCaliper->m_pCaliperParam->m_pCaliper = &pDlgCaliper->m_Caliper[nCam][nPos][pDlgCaliper->m_nLine];
	pDlgCaliper->m_pCaliperResult->m_pCaliper = &pDlgCaliper->m_Caliper[nCam][nPos][pDlgCaliper->m_nLine];
	pDlgCaliper->m_nPos = nPos;

	{
		pDlgCaliper->m_pViewer->ClearCaliperOverlayDC();
		pDlgCaliper->m_pViewer->ClearOverlayDC();
	}

	if (pDlgCaliper->m_Caliper[nCam][nPos][pDlgCaliper->m_nLine].getCircleCaliper()) pDlgCaliper->m_Caliper[nCam][nPos][pDlgCaliper->m_nLine].DrawCaliper_Circle(pDlgCaliper->m_Caliper[nCam][nPos][pDlgCaliper->m_nLine].getCaliperDC());
	else																			pDlgCaliper->m_Caliper[nCam][nPos][pDlgCaliper->m_nLine].DrawCaliper(pDlgCaliper->m_Caliper[nCam][nPos][pDlgCaliper->m_nLine].getCaliperDC());
	pDlgCaliper->m_pViewer->Invalidate();
	pDlgCaliper->m_pCaliperParam->updateUIFromData();
	pDlgCaliper->m_pCaliperParam->updateCoordinates();
#ifndef SKIP_XLIST
	pDlgCaliper->m_pCaliperResult->updateCaliperList();
#endif

	/*
	//Camera Tab 옮길시 무조건 Disable
	CFormCameraView *pCamera = (CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA];
	pCamera->m_ViewerCamera.setUseCaliper(FALSE);
	pCamera->m_ViewerCamera.setDrawCaliper(FALSE);
	m_btnUseCaliper.SetColorBkg(255, COLOR_BTN_BODY);
	m_pMain->m_pDlgCaliper->ShowWindow(SW_HIDE);
	m_btnMoveCaliperToCenter.SetEnable(false);
	m_btnDrawArea.SetEnable(true);
	*/

#endif
}

void TabCameraPage::OnBnClickedChkEnableProfile()
{
	BOOL bCheck = m_chkShowProfile.GetCheck();

	if (bCheck)
	{
		if (!((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.GetModeDrawingFigure())
		{
			((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetModeDrawingFigure(true);
			dispDrawArea();
		}

		((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetDimensionalMode();		
		((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetInteraction(true);		
	}
	else
	{
		((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.OnGrabMoveMode();
		((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.InactiveInteraction(true);
	}

	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_INSP_SHOW_PROFILE, bCheck);

	if (!bCheck && ((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.GetModeDrawingFigure())
	{
		((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetModeDrawingFigure(true);
		((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetEnableModifyFigure(0, true);
	}
}

void TabCameraPage::OnBnClickedChkEnableSubInsp()
{
	if (m_chkSubInsp.GetCheck())	m_pMain->m_bUseSubInspCamView = TRUE;
	else							m_pMain->m_bUseSubInspCamView = FALSE;
}

void TabCameraPage::OnBnClickedBtnCameraInspection()
{
	int nJob = m_nJobID;
	int nCam = m_cmbSelectCamera.GetCurSel();
	int nPos = m_cmbSelectPosition.GetCurSel();
	int nIndex = m_cmbPatternIndex.GetCurSel();
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	int w = m_pMain->m_stCamInfo[real_cam].w;
	int h = m_pMain->m_stCamInfo[real_cam].h;

	m_pMain->m_pPattern[real_cam].pDlg = this;
	m_pMain->m_pPattern[real_cam].nPos = nPos;
	m_pMain->m_pPattern[real_cam].nCam = nCam;
	m_pMain->m_pPattern[real_cam].nJob = m_nJobID;
	//210110
	int nMethod = m_pMain->vt_job_info[nJob].algo_method;
	switch (nMethod)
	{
	case CLIENT_TYPE_STAGE_INSP:
		break;
	case CLIENT_TYPE_PANEL_EXIST_INSP:
		//m_pMain->algorithm_AlignExist(m_pMain->getCameraViewBuffer(), m_nJobID, nCam, TRUE);
		m_pMain->algorithm_exist_insp_assemble(m_pMain->getCameraViewBuffer(), m_nJobID, nCam, TRUE);
		break;
	case CLIENT_TYPE_PCB_DISTANCE_INSP:
	{
		int nPanelType = m_pMain->vt_job_info[nJob].model_info.getGlassInfo().getModelPanelType();
		int nInpsMethod = 0;
		BOOL bCheck = m_chkSubInsp.GetCheck();

		if (bCheck)	nInpsMethod = m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getInspectionSubMethod();
		else		nInpsMethod = m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getInspectionMethod();

		m_pMain->algorithm_pcb_Insp(m_pMain->getCameraViewBuffer(), &m_pMain->m_pPattern[real_cam], nInpsMethod, TRUE);
	}
	break;
	case CLIENT_TYPE_ASSEMBLE_INSP:
	{
		int nPanelType = m_pMain->vt_job_info[nJob].model_info.getGlassInfo().getModelPanelType();
		int nInpsMethod = m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getInspectionMethod();
		BOOL bCheck = m_chkSubInsp.GetCheck();

		if (nPanelType == _CORE_MODEL_CALC)	m_pMain->algorithm_AssembleInsp_CALC(m_pMain->getCameraViewBuffer(), &m_pMain->m_pPattern[real_cam], nInpsMethod, TRUE);
		else								m_pMain->algorithm_assemble_Insp(m_pMain->getCameraViewBuffer(), &m_pMain->m_pPattern[real_cam], nInpsMethod, TRUE);
	}
	break;
	case CLIENT_TYPE_ELB_CENTER_ALIGN:
	{
		if (0) {
			make_elb_wetout_msa(m_nJobID, nCam, 0, 1);
			return;
		}

		BOOL bCheck = m_chkSubInsp.GetCheck();
		if (bCheck) ::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_INSP_NOTCH_DOPO, MAKELPARAM(nCam, m_nJobID));
		else
		{
			int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];
			int w = m_pMain->m_stCamInfo[real_cam].w;
			int h = m_pMain->m_stCamInfo[real_cam].h;
			memcpy(m_pMain->getProcBuffer(real_cam, 2), m_pMain->getCameraViewBuffer(), w * h);

			::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_INSP_NOTCH, MAKELPARAM(nCam, m_nJobID));
		}
	}
	break;
	case CLIENT_TYPE_NOZZLE_ALIGN:
	{
		BOOL bCheck = m_chkSubInsp.GetCheck();
		int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];
		int w = m_pMain->m_stCamInfo[real_cam].w;
		int h = m_pMain->m_stCamInfo[real_cam].h;
		memcpy(m_pMain->getProcBuffer(real_cam, 2), m_pMain->getCameraViewBuffer(), w * h);

		if (bCheck)
			::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_INSP_NOTCH_NOZZLE, MAKELPARAM(real_cam, m_nJobID));
		else
			::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_INSP_NOTCH_NOZZLE_ANGLE, MAKELPARAM(real_cam, m_nJobID));
	}
	break;
	case CLIENT_TYPE_SCAN_INSP:
	{
		if (nPos == _SCAN_GRAB_MAXCOUNT - 1)
		{
			CFileDialog dlg(TRUE, NULL, _T("Image File"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
				_T("Image Files (*.bmp;*.jpg;)|*.bmp;*.jpg|All Files (*.*)|*.*||"), this);
			if (dlg.DoModal() != IDOK)		return;

			CString str = dlg.GetPathName();
			cv::Mat m_pImage = cv::imread(string(str), 0);

			if (m_pImage.empty())
			{
				AfxMessageBox("Check the Image Type");
				return;
			}
			((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.OnInitWithCamera(m_pImage.cols, m_pImage.rows, 8);
			((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.OnLoadImageFromPtr((BYTE*)m_pImage.data);

			m_pImage.release();

			m_pMain->m_nSeqScanInspGrabCount[m_nJobID] = 1;
		}
		else		m_pMain->m_nSeqScanInspGrabCount[m_nJobID] = nPos + 1;

		int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
		::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_INSP_SCAN, MAKELPARAM(real_cam, m_nJobID));
	}
	break;
	case CLIENT_TYPE_NOZZLE_SIDE_VIEW: // 20211007 Tkyuha 노즐뷰에서 갭측정용
	{
		int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];
		int w = m_pMain->m_stCamInfo[real_cam].w;
		int h = m_pMain->m_stCamInfo[real_cam].h;
		memcpy(m_pMain->getProcBuffer(real_cam, 0), m_pMain->getCameraViewBuffer(), w * h);

		::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_INSP_NOZZLE_GAP, MAKELPARAM(real_cam, m_nJobID));
	}
	break;
	case CLIENT_TYPE_FILM_INSP: // 20211021 Tkyuha 필름검사 시뮬레이션
	{
		CViewerEx* ViewerCamera = &(((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera);

		if (0)
		{
			m_pMain->algorithm_Film_insp_assemble(m_pMain->getCameraViewBuffer(), nJob, nCam, ViewerCamera);
		}
		else
		{
			m_pMain->algorithm_PF_Film_insp(m_pMain->getCameraViewBuffer(), nJob, nCam);
			m_pMain->display_PF_Film_insp(m_pMain->getCameraViewBuffer(), nJob, nCam, ViewerCamera);
		}
	}
	break;
	case CLIENT_TYPE_CENTER_NOZZLE_ALIGN:
	{
		//if (nCam == 1) // 센터 노즐 얼라인 테스트 
		//{
		//	OnBnClickedBtnPatternSearch();
		//	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_INSP_CENTER_NOZZLE_Y_CALC, MAKELPARAM(nCam, m_nJobID));

		//	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
		//	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
		//	int nIndex = ((CComboBox*)GetDlgItem(IDC_CB_PATTERN_INDEX))->GetCurSel();

		//	_stPatternSelect pattern;
		//	pattern.nCam = nCam;
		//	pattern.nPos = nPos;
		//	pattern.nIndex = nIndex;
		//	pattern.nJob = m_nJobID;
		//	::SendMessage(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_PATTERN_DISPLAY_RESULT, (LPARAM)&pattern);
		//	break;
		//}

		BOOL bCheck = m_chkSubInsp.GetCheck();
		int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];
		if (bCheck)
		{
			::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_INSP_NOTCH_NOZZLE_ANGLE_REVERSE, MAKELPARAM(real_cam, m_nJobID));
		}
		else
		{
			::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_INSP_NOTCH_NOZZLE_ANGLE, MAKELPARAM(real_cam, m_nJobID));
		}
	}
	break;
	case CLIENT_TYPE_CENTER_SIDE_YGAP:
	{
		int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];
		::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_INSP_NOZZLE_Z_HEIGHT, MAKELPARAM(real_cam, m_nJobID));
	}
	break;
	case CLIENT_TYPE_ONLY_VIEW:
	{
		int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];
		::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_INSP_INK_INSPECTION, MAKELPARAM(real_cam, m_nJobID));
	}
	break;
	case CLIENT_TYPE_1CAM_1SHOT_ALIGN_WITH_SCRATCH_INSP:
	{
		int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];
		::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_INSP_SCRATCH_INSPECTION, MAKELPARAM(real_cam, m_nJobID));
	}
	break;
	}

	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_DRAW_DISTANCE_INSP, (LPARAM)&m_pMain->m_pPattern[real_cam]);
}

void TabCameraPage::disEnableButton()
{
	//Caliper simulation 버튼 Enable / Disable
	int nSel = m_cmbSelectCamera.GetCurSel();
	int nCam = m_pMain->vt_job_info[m_nJobID].camera_index[nSel];
	int nPos = m_cmbSelectPosition.GetCurSel();
	int nIndex = m_cmbPatternIndex.GetCurSel();

	int nMarkFindMethod = m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getMarkFindMethod();
	int nMethod = m_pMain->vt_job_info[m_nJobID].algo_method;

	// kbj 2021-01-10 caliper button always use.
	/*if (nMarkFindMethod == METHOD_CALIPER || nMarkFindMethod == METHOD_LINE || nMethod ==CLIENT_TYPE_FILM_INSP ||
		nMarkFindMethod == METHOD_NOTCH || nMarkFindMethod == METHOD_NOTCH_LINE || nMarkFindMethod == METHOD_MATCHING_LINE_THETA)
	{
		m_btnUseCaliper.SetEnable(TRUE);
		m_btnSimulation.SetEnable(TRUE);
	}
	else
	{
		m_btnUseCaliper.SetEnable(FALSE);
		m_btnSimulation.SetEnable(FALSE);
	}*/
	m_btnUseCaliper.SetEnable(TRUE);
	m_btnSimulation.SetEnable(TRUE);

	// KBJ 2022-08-18 ZGap Caliper 미사용 -> 파라미터에 따른 사용
	if (nMethod == CLIENT_TYPE_CENTER_SIDE_YGAP && 
		m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo().getCenterNozzleAlignMethod() == FALSE)
		m_btnUseCaliper.SetEnable(FALSE);

	CFormCameraView* pCamera = (CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA];

	if (m_btnUseCaliper.GetColorBkg() == COLOR_BTN_SELECT)
	{
		pCamera->m_ViewerCamera.setUseCaliper(TRUE);
		pCamera->m_ViewerCamera.setDrawCaliper(TRUE);
	}
	else
	{
		pCamera->m_ViewerCamera.setUseCaliper(FALSE);
		pCamera->m_ViewerCamera.setDrawCaliper(FALSE);
		pCamera->m_ViewerCamera.ClearOverlayDC();		//210209
		pCamera->m_ViewerCamera.clearAllFigures();		//210209
		pCamera->m_ViewerCamera.Invalidate();
	}


	if (nMethod < CLIENT_TYPE_PANEL_EXIST_INSP)
	{
		m_btnInspSimulation.SetEnable(FALSE);
	}
	else
	{
		m_btnInspSimulation.SetEnable(TRUE);
	}

	if (m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getUseFixtureMark() == TRUE)
	{
		m_btnFixturePos.SetEnable(TRUE);
	}
	else
	{
		m_btnFixturePos.SetEnable(FALSE);
	}

	// HSJ 2022-01-31 reference 사용유무
	if (m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getUseReferenceMark() == TRUE)
	{
		m_btnAlignReference.SetEnable(TRUE);
	}
	else
	{
		m_btnAlignReference.SetEnable(FALSE);
	}

	if (nMethod != CLIENT_TYPE_FILM_INSP)
	{
		m_btnScaleCalc.SetEnable(FALSE);
	}

	m_btnMakeMasking.ShowWindow(SW_HIDE);
	m_btnShowGoldData.ShowWindow(SW_HIDE);
	m_btnMakeTraceData.ShowWindow(SW_HIDE);
	m_btnTraceClear.ShowWindow(SW_HIDE);
}
//210117


void TabCameraPage::OnBnClickedBtnSetSubMark()
{
	BOOL bRet = m_pMain->fnSetMessage(2, "Would you like to register as a sub mark ?");
	if (bRet != TRUE) return;

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CPatternMatching* pPattern = &m_pMain->GetMatching(m_nJobID);

	int nJob = m_nJobID;
	int nCam = m_cmbSelectCamera.GetCurSel();
	int nPos = m_cmbSelectPosition.GetCurSel();
	int nIndex = m_cmbPatternIndex.GetCurSel();
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	int W = m_pMain->m_stCamInfo[real_cam].w;
	int H = m_pMain->m_stCamInfo[real_cam].h;

	m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 0), m_pMain->getCameraViewBuffer(), W * H);
	BYTE* pImg = m_pMain->getProcBuffer(real_cam, 0);

	pPattern->findPatternIndex(pImg, nCam, nPos, W, H, 0, 0);

	if (pPattern->getFindInfo(nCam, nPos).GetFound() != FIND_OK)
	{
		AfxMessageBox("Base Mark Find Failed");
		return;
	}

	double baseX = pPattern->getFindInfo(nCam, nPos).GetXPos();
	double baseY = pPattern->getFindInfo(nCam, nPos).GetYPos();

	pPattern->findPatternIndex(pImg, nCam, nPos, W, H, nIndex);

	if (pPattern->getFindInfo(nCam, nPos).GetFound() != FIND_OK)
	{
		AfxMessageBox("Sub Mark Find Failed");
		return;
	}

	double subX = pPattern->getFindInfo(nCam, nPos).GetXPos();
	double subY = pPattern->getFindInfo(nCam, nPos).GetYPos();

	double dx = baseX - subX;
	double dy = baseY - subY;
	CString strTemp;

	strTemp.Format("offX: %.3f offY: %.3f", dx, dy);

	if (m_pMain->fnSetMessage(2, strTemp) != TRUE) return;

	pPattern->setMarkOffsetX(nCam, nPos, nIndex, dx);
	pPattern->setMarkOffsetY(nCam, nPos, nIndex, dy);

	updateFrameDialog();


	_stPatternSelect pattern;
	pattern.nCam = nCam;
	pattern.nPos = nPos;
	pattern.nJob = nJob;
	pattern.nIndex = nIndex;
	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_PATTERN_DISPLAY_RESULT, (LPARAM)&pattern);

}


void TabCameraPage::dispMarkOffset()
{
	CString strTemp;
	int nCam = m_cmbSelectCamera.GetCurSel();
	int nPos = m_cmbSelectPosition.GetCurSel();
	int nIndex = m_cmbPatternIndex.GetCurSel();
	int nJob = m_nJobID;

	if (nIndex == 0)
	{
		m_btnSetSubMark.SetEnable(false);
	}
	else
	{
		m_btnSetSubMark.SetEnable(true);
	}

	strTemp.Format("%.3f", m_pMain->GetMatching(nJob).getMarkOffsetX(nCam, nPos, nIndex));
	m_lblMarkOffsetX.SetText(strTemp);

	strTemp.Format("%.3f", m_pMain->GetMatching(nJob).getMarkOffsetY(nCam, nPos, nIndex));
	m_lblMarkOffsetY.SetText(strTemp);
}


void TabCameraPage::OnCbnSelchangeCbSelectCamera()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	sel_last_camera = m_cmbSelectCamera.GetCurSel();

	int viewer = m_pMain->vt_job_info[m_nJobID].viewer_index[sel_last_camera];
	int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[sel_last_camera];
	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_SET_CAMERA_POS, MAKELPARAM(viewer, real_cam));

	init_light_setting();
	updateFrameDialog();

	//KJH 2022-01-05 Center Nozzle Align 시작하기 전에 ExposureTime 변경
	int nPos = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getExposureTime(real_cam);
	auto& cam = m_pMain->m_Cameras[real_cam];
	if (cam && cam->IsConnected())cam->SetExposure(nPos);

	
	//KJH 2021-12-29 Tap에 맞는 조명으로 초기화
	for (int nLight = 0; nLight < m_pMain->vt_job_info[m_nJobID].light_info[sel_last_camera].num_of_using_light; nLight++)
	{
		int ctrl = m_pMain->vt_job_info[m_nJobID].light_info[sel_last_camera].controller_id[nLight] - 1;
		int ch = m_pMain->vt_job_info[m_nJobID].light_info[sel_last_camera].channel_id[nLight];
		int value = m_pMain->vt_job_info[m_nJobID].model_info.getLightInfo().getLightBright(sel_last_camera, nLight, 0);

		if (m_pMain->m_stLightCtrlInfo.nType[sel_last_camera] == LTYPE_LLIGHT_LPC_COT_232)
		{
			m_pMain->SetLightBright(ctrl, ch, value == 0 ? 0: value * 10);   // dh.jung 2021-07-12
		}
		else
		{
			m_pMain->SetLightBright(ctrl, ch, value);
		}
		Delay(50);
	}
}

void TabCameraPage::OnBnClickedBtnPatternTemp()
{
	CFileDialog dlg(TRUE, NULL, _T("CSV File"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("CSV Files (*.csv;*.CSV;)|*.csv;*.CSV|All Files (*.*)|*.*||"), this);

	if (dlg.DoModal() != IDOK)		return;

	CString str = dlg.GetPathName();

	std::ifstream openFile(str);

	int nJob = m_nJobID;
	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(sel_last_camera, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(sel_last_camera, 0);

	if (openFile.is_open())
	{
		for (int i = 0; i < 5; i++) m_pMain->m_ELB_TraceResult.m_vTraceProfile[i].clear();

		int method = m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getMarkFindMethod();
		int cam = m_pMain->vt_job_info[m_nJobID].camera_index[sel_last_camera];
		double xw = m_pMain->m_stCamInfo[cam].w / 2;
		double yh = m_pMain->m_stCamInfo[cam].h / 2;

		std::string line;
		getline(openFile, line);
		vector<string> xX = split(line, ',');

		int lineCount = atoi(xX.at(0).c_str());
		double rotateX = atof(xX.at(1).c_str());
		double rotateY = atof(xX.at(2).c_str());
		double xof = 0, yof = 0;

		if (method == METHOD_CIRCLE || method == METHOD_LINE)
		{
			m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.x = -rotateX;
			m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.y = -rotateY;
		}
		else
		{
			/*rotateX *= -1;
			rotateY *= -1;*/
			if (xX.size() >= 5)
			{
				m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.x = atof(xX.at(3).c_str());
				m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.y = atof(xX.at(4).c_str());
			}
		}

		m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.x = rotateX;
		m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.y = rotateY;

		xof = MAX(0, MIN(xw * 2 - 1, xw + rotateX / xres)), yof = MAX(0, MIN(yh * 2 - 1, yh + rotateY / yres));
		vector<double> x_vector;
		vector<double> y_vector;

		while (getline(openFile, line))
		{
			vector<string> x = split(line, ',');
			double rx = atof(x.at(2).c_str());  //3
			double ry = (atof(x.at(3).c_str())); //4
			double rt = atof(x.at(4).c_str()); // 임시

			if (method == METHOD_CIRCLE || method == METHOD_LINE)
			{
			}
			else
			{
				/*rx *= 1;
				ry *= 1;*/
			}

			m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].push_back(rx);
			m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].push_back(ry);
			m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].push_back(rt);
			m_pMain->m_ELB_TraceResult.m_vTraceProfile[3].push_back(atof(x.at(0).c_str()) / xres + xof);
			m_pMain->m_ELB_TraceResult.m_vTraceProfile[4].push_back(atof(x.at(1).c_str()) / yres + yof);

			x_vector.push_back(xof - (atof(x.at(0).c_str()) / xres));
			y_vector.push_back(yof - (atof(x.at(1).c_str()) / yres));

			x.clear();
		}

		if (method == METHOD_CIRCLE || method == METHOD_LINE)
		{
		}
		else
		{
			for (int i = int(x_vector.size() - 1); i >= 0; i--)
			{
				m_pMain->m_ELB_TraceResult.m_vTraceProfile[3].push_back(x_vector.at(i));
				m_pMain->m_ELB_TraceResult.m_vTraceProfile[4].push_back(y_vector.at(i));
			}
		}
		x_vector.clear();
		y_vector.clear();

		m_pMain->m_strGoldenDataFile = str;
		CString strFilePath = m_pMain->m_strSystemDir + _T("LET_AlignClient.ini");
		::WritePrivateProfileString("SERVER_INFO", "TRACE_GOLD_DATA", str, strFilePath);
	}

	openFile.close();

	int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();
	double r = int(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspTraceInterval() / xres);
	double pitch = r * xres;

	if (method == METHOD_CIRCLE)
	{
		m_pMain->sendTraceProfileCircleData(m_pMain->vt_job_info[nJob].plc_addr_info.write_word_sub_start,
			m_pMain->vt_job_info[nJob].plc_addr_info.write_word_start, pitch);
		/*
		m_pMain->sendTraceProfileTwo_CircleData(m_pMain->vt_job_info[nJob].plc_addr_info.write_word_sub_start,
			m_pMain->vt_job_info[nJob].plc_addr_info.write_word_start, pitch);
		*/
	}
	else if (method == METHOD_NOTCH)
		m_pMain->sendTraceProfileNotchData(m_pMain->vt_job_info[nJob].plc_addr_info.write_word_sub_start,
			m_pMain->vt_job_info[nJob].plc_addr_info.write_word_start, pitch);
	else
		m_pMain->sendTraceProfileData(m_pMain->vt_job_info[nJob].plc_addr_info.write_word_sub_start,
			m_pMain->vt_job_info[nJob].plc_addr_info.write_word_start, pitch);

	m_pMain->fnSetMessage(1, "Trace Send End");
}

void TabCameraPage::OnEnSetfocusEditExposureSetting()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GetDlgItem(IDC_LB_EXPOSURETIME)->SetFocus();

	CString str, strTemp;

	m_edtExposureTime.GetWindowTextA(str);
	strTemp = m_pMain->GetNumberBox(str, 5, 25, 10000000);
	m_edtExposureTime.SetWindowTextA(strTemp);

	int v = atoi(strTemp);

	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];
	auto& cam = m_pMain->m_Cameras[real_cam];
	if (cam && cam->IsConnected()) cam->SetExposure(v);
	m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().setExposureTime(real_cam, v);
	
	m_sliderExposureTime.SetPos(v);

	// YCS 2022-11-28 AutoMode 변경시 Exposure 변경 시퀀스 추가
	m_bExposureTimeChange[real_cam] = TRUE;

	updateFrameDialog();
}

void TabCameraPage::dispExposuretime()
{
	int nSel = m_cmbSelectCamera.GetCurSel();
	int nCam = m_pMain->vt_job_info[m_nJobID].camera_index[nSel];

	CString strExposure;
	strExposure.Format("%d", m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getExposureTime(nCam));
	m_edtExposureTime.SetWindowTextA(strExposure);
}


void TabCameraPage::OnBnClickedBtnCameraMake()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nSel = m_cmbSelectCamera.GetCurSel();
	int nCam = m_pMain->vt_job_info[m_nJobID].camera_index[nSel];
	int method = m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getMarkFindMethod();

	CString mask_file, mask_invfile;
	mask_file.Format("%s%s\\%s_%d%s", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[m_nJobID].job_name.c_str(), "MarkMask", nCam, (method == METHOD_NOTCH) ? "Notch.jpg": "Circle.jpg");
	mask_invfile.Format("%s%s\\%s_%d%s", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[m_nJobID].job_name.c_str(), "MarkMask_Inv", nCam, (method == METHOD_NOTCH) ? "Notch.jpg": "Circle.jpg");

	if (_access(mask_file, 0) == 0)
	{
		if (AfxMessageBox("Delete Masking Data?", MB_YESNO) != IDYES) return;
		::DeleteFile(mask_file);
	}

	if (m_pMain->m_vtTraceData.size() < 20)
	{
		AfxMessageBox("Trace Data Invalid");
		return;;
	}

	if (AfxMessageBox("Make Masking Data?", MB_YESNO) != IDYES) return;

	int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[0];
	int w = m_pMain->m_stCamInfo[real_cam].w;
	int h = m_pMain->m_stCamInfo[real_cam].h;

	cv::Mat mask = cv::Mat::zeros(h, w, CV_8UC1);
	cv::Mat mask_inv = cv::Mat::zeros(h, w, CV_8UC1);
	mask = 255;

	cv::Point2f first_data, last_data, temp_data;
	int size = int(m_pMain->m_vtTraceData.size());

	vector<cv::Point2f> vtTemp;
	vtTemp = m_pMain->m_vtTraceData;

	int off_x = 7;
	int off_y = 7;
	for (int i = 0; i < size; i++)
	{
		temp_data = vtTemp[i];
		temp_data.x += off_x;
		temp_data.y += off_y;
		vtTemp[i] = temp_data;
	}

	if (method == METHOD_NOTCH)
	{
		last_data = vtTemp[size - 1];
		first_data = vtTemp[0];

		temp_data.x = 0;
		temp_data.y = first_data.y;
		m_pMain->m_vtTraceData.insert(m_pMain->m_vtTraceData.begin(), temp_data);
		m_pMain->m_vtTraceData.insert(m_pMain->m_vtTraceData.begin(), temp_data);

		temp_data.x = w - 1;
		temp_data.y = last_data.y;
		vtTemp.push_back(temp_data);
		m_pMain->m_vtTraceData.push_back(temp_data);
		m_pMain->m_vtTraceData.push_back(temp_data);

		temp_data.x = w - 1;
		temp_data.y = h - 1;
		vtTemp.push_back(temp_data);

		temp_data.x = 0;
		temp_data.y = h - 1;
		vtTemp.push_back(temp_data);

		temp_data.x = 0;
		temp_data.y = first_data.y;
		vtTemp.push_back(temp_data);
	}
	vector<cv::Point> vtContour;
	for (int i = 0; i < vtTemp.size(); i++)
	{
		vtContour.push_back(cv::Point(vtTemp[i].x, vtTemp[i].y));
	}

	int num = (int)vtContour.size();
	const cv::Point* pt4 = &(vtContour[0]); // *@#! const
	cv::fillPoly(mask, &pt4, &num, 1, cv::Scalar(0, 0, 0), 8);

	cv::imwrite(string(mask_file), mask);

	cv::bitwise_not(mask, mask_inv);
	cv::imwrite(string(mask_invfile), mask_inv);

	vtContour.clear();

	///   추가로 마스크 이미지 생성
	double xres = m_pMain->GetMachine(m_nJobID).getCameraResolutionX(nSel, 0);
	double distanceT = 0.05 / xres;
	cv::Point2f rC;

	m_pMain->m_ELB_vtMaskContour.clear();
	if (method == METHOD_NOTCH)
	{
		for (int i = 0; i < m_pMain->m_vtTraceData.size() - 1; i++)
		{
			rC = Equation_of_Lines(m_pMain->m_vtTraceData[i], m_pMain->m_vtTraceData[i + 1], distanceT);
			m_pMain->m_ELB_vtMaskContour.push_back(cv::Point(int(rC.x / 2.), int(rC.y / 2.)));
		}
		distanceT = -0.2 / xres;
		for (int i = 0; i < m_pMain->m_vtTraceData.size() - 1; i++)
		{
			rC = Equation_of_Lines(m_pMain->m_vtTraceData[i], m_pMain->m_vtTraceData[i + 1], distanceT);
			m_pMain->m_ELB_vtMaskContour.insert(m_pMain->m_ELB_vtMaskContour.begin(), cv::Point(int(rC.x / 2.), int(rC.y / 2.)));
		}
	}
	else
	{
		double rad = fabs(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y - m_pMain->m_vtTraceData[0].y) - 40;
		for (int i = 0; i < 360; i++)
		{
			rC.x = (cos(ToRadian(i)) * rad + m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x) / 2.;
			rC.y = (sin(ToRadian(i)) * rad + m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y) / 2.;

			m_pMain->m_ELB_vtMaskContour.push_back(rC);
		}
		m_pMain->m_ELB_vtMaskContour.push_back(m_pMain->m_ELB_vtMaskContour[0]);

		rad = fabs(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y - m_pMain->m_vtTraceData[0].y) + 40;
		for (int i = 0; i < 360; i++)
		{
			rC.x = (cos(ToRadian(i)) * rad + m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x) / 2.;
			rC.y = (sin(ToRadian(i)) * rad + m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y) / 2.;

			m_pMain->m_ELB_vtMaskContour.push_back(rC);
		}
		m_pMain->m_ELB_vtMaskContour.push_back(m_pMain->m_ELB_vtMaskContour[361]);
	}

	CString mask_range_file;
	mask_range_file.Format("%s%s\\%s_%d%s", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[m_nJobID].job_name.c_str(), "Trace_AreaMask", nCam, (method == METHOD_NOTCH) ? "Notch.bin": "Circle.bin");

	FILE* fp;
	fopen_s(&fp, mask_range_file, "wb");
	if (fp)
	{
		int m_nUINumber = int(m_pMain->m_ELB_vtMaskContour.size());
		fwrite(&m_nUINumber, sizeof(int), 1, fp);

		std::vector<cv::Point>::iterator it;
		cv::Point wDataRead;
		for (it = m_pMain->m_ELB_vtMaskContour.begin(); it != m_pMain->m_ELB_vtMaskContour.end(); it++)
		{
			wDataRead = *it;
			fwrite(&wDataRead, sizeof(cv::Point), 1, fp);
		}

		fclose(fp);
	}

	CFormCameraView* pCamera = (CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA];
	auto pDC = pCamera->m_ViewerCamera.getOverlayDC();
	CPen penGOLD(PS_DOT, 3, COLOR_YELLOW);
	CPen penRED(PS_DOT, 3, COLOR_RED);
	CPen* oldpen = pDC->SelectObject(&penGOLD);


	pDC->MoveTo(m_pMain->m_ELB_vtMaskContour[0].x * 2, m_pMain->m_ELB_vtMaskContour[0].y * 2);
	for (int i = 1; i < m_pMain->m_ELB_vtMaskContour.size(); i++)
	{
		pDC->LineTo(m_pMain->m_ELB_vtMaskContour[i].x * 2, m_pMain->m_ELB_vtMaskContour[i].y * 2);
	}

	pDC->SelectObject(&penRED);
	pDC->MoveTo(m_pMain->m_vtTraceData[0].x, m_pMain->m_vtTraceData[0].y);
	for (int i = 1; i < m_pMain->m_vtTraceData.size(); i++)
	{
		pDC->LineTo(m_pMain->m_vtTraceData[i].x, m_pMain->m_vtTraceData[i].y);
	}

	pDC->SelectObject(oldpen);
	DeleteObject(penRED);
	DeleteObject(penGOLD);
	pCamera->m_ViewerCamera.Invalidate();

	mask.release();
	mask_inv.release();
	vtTemp.clear();
}

void TabCameraPage::OnBnClickedBtnCameraMakeMetalData()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BOOL bCheck = m_chkSubInsp.GetCheck();
	int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[0];
	int w = m_pMain->m_stCamInfo[real_cam].w;
	int h = m_pMain->m_stCamInfo[real_cam].h;

	if (bCheck)
	{
		if (m_pMain->m_vtTraceData.size() == 0 && m_pMain->notch_wetout_data.pt_center.size() == 0)
		{
			if (m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getUseRealTimeTrace() != TRUE)
			{
				CString trace_file;
				trace_file.Format("%s%s\\%s_%d%s", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[m_nJobID].job_name.c_str(), "Trace", real_cam, "Data.ini");

				m_pMain->read_trace_data(trace_file);
			}
			else
			{
				AfxMessageBox("trace data is null!!");
				return;
			}
		}

		CFormMainView* pFormMain = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];
		CFormCameraView* pFormCamera = (CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA];

		pFormMain->process_find_notch_metal(m_pMain->getCameraViewBuffer(), m_nJobID, 0, w, h, &pFormCamera->m_ViewerCamera);
		return;
	}

	CString metal_file;
	metal_file.Format("%s%s\\%s_%d%s", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[m_nJobID].job_name.c_str(), "MetalTrace", real_cam, "Data.ini");

	if (_access(metal_file, 0) == 0)
	{
		if (AfxMessageBox("Delete Metal Data?", MB_YESNO) != IDYES) return;
		::DeleteFile(metal_file);
	}

	int size = int(m_pMain->notch_wetout_data.pt_metal.size());

	if (size <= 0)
	{
		AfxMessageBox("Metal Data is Null!!");
		return;
	}


	if (AfxMessageBox("Make Metal Data?", MB_YESNO) != IDYES) return;


	CString str_temp = metal_file;
	CString str_data, str_key;
	char cData[MAX_PATH] = { 0, };
	str_data.Format("%d", size);
	::WritePrivateProfileString("INFO", "COUNT", str_data, str_temp);

	for (int i = 0; i < size; i++)
	{
		str_key.Format("%d_X", i + 1);
		str_data.Format("%f", m_pMain->notch_wetout_data.pt_metal[i].x);
		::WritePrivateProfileString("INFO", str_key, str_data, str_temp);

		str_key.Format("%d_Y", i + 1);
		str_data.Format("%f", m_pMain->notch_wetout_data.pt_metal[i].y);
		::WritePrivateProfileString("INFO", str_key, str_data, str_temp);
	}
	return;

	CString trace_file2;
	trace_file2.Format("%s%s\\%s_%d%s", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[m_nJobID].job_name.c_str(), "Trace", real_cam, "Data.ini");

	m_pMain->read_trace_data(trace_file2);

	cv::Point2f temp_pt;
	m_pMain->m_vtTraceDataGold.clear();

	for (int i = 0; i < m_pMain->m_vtTraceData.size(); i++)
	{
		m_pMain->m_vtTraceDataGold.push_back(m_pMain->m_vtTraceData[i]);
	}

	CFormCameraView* pCamera = (CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA];
	auto pDC = pCamera->m_ViewerCamera.getOverlayDC();
	CPen penGOLD(PS_DOT, 3, COLOR_YELLOW);
	CPen* oldpen = pDC->SelectObject(&penGOLD);

	int off = 5;
	for (int i = 0; i < m_pMain->m_vtTraceDataGold.size(); i++)
	{
		int x = m_pMain->m_vtTraceDataGold[i].x;
		int y = m_pMain->m_vtTraceDataGold[i].y;

		pDC->MoveTo(x - off, y);
		pDC->LineTo(x + off, y);
		pDC->MoveTo(x, y - off);
		pDC->LineTo(x, y + off);
	}


	pDC->SelectObject(oldpen);
	pCamera->m_ViewerCamera.Invalidate();
}


void TabCameraPage::OnBnClickedBtnCameraMakeTraceData()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nSel = m_cmbSelectCamera.GetCurSel();
	int nCam = m_pMain->vt_job_info[m_nJobID].camera_index[nSel];
	CString trace_file;
	trace_file.Format("%s%s\\%s_%d%s", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[m_nJobID].job_name.c_str(), "Trace", nCam, "Data.ini");

	if (_access(trace_file, 0) == 0)
	{
		if (AfxMessageBox("Delete Trace Data?", MB_YESNO) != IDYES) return;
		::DeleteFile(trace_file);
	}

	int size = int(m_pMain->m_vtTraceData.size());

	if (size <= 0)
	{
		AfxMessageBox("Trace Data Is null!!");
		return;
	}

	if (AfxMessageBox("Make Trace Data?", MB_YESNO) != IDYES) return;

	CString str_temp = trace_file;
	CString str_data, str_key;
	char cData[MAX_PATH] = { 0, };
	str_data.Format("%d", size);
	::WritePrivateProfileString("INFO", "COUNT", str_data, str_temp);

	for (int i = 0; i < size; i++)
	{
		str_key.Format("%d_X", i + 1);
		str_data.Format("%f", m_pMain->m_vtTraceData[i].x);
		::WritePrivateProfileString("INFO", str_key, str_data, str_temp);

		str_key.Format("%d_Y", i + 1);
		str_data.Format("%f", m_pMain->m_vtTraceData[i].y);
		::WritePrivateProfileString("INFO", str_key, str_data, str_temp);
	}

	AfxMessageBox("Complete");
}


void TabCameraPage::OnBnClickedBtnPatternClear()
{
	if (m_pMain->m_ELB_vtMaskContour.size() > 0)
	{
		if (AfxMessageBox("Are You Delete Trace Data?", MB_YESNO) != IDYES) return;

		m_pMain->m_ELB_vtMaskContour.clear();

		int nSel = m_cmbSelectCamera.GetCurSel();
		int nCam = m_pMain->vt_job_info[m_nJobID].camera_index[nSel];
		int method = m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getMarkFindMethod();

		CString mask_range_file;
		mask_range_file.Format("%s%s\\%s_%d%s", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[m_nJobID].job_name.c_str(), "Trace_AreaMask", nCam, (method == METHOD_NOTCH) ? "Notch.bin": "Circle.bin");

		if (_access(mask_range_file, 0) == 0)
		{
			if (AfxMessageBox("Delete Masking Data?", MB_YESNO) != IDYES) return;
			::DeleteFile(mask_range_file);
		}
	}
}
void TabCameraPage::setCurrentCamLight()
{
	CString str;

	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();

	if (m_pMain->vt_job_info[m_nJobID].light_info[nCam].num_of_using_light <= 0) return; // Tkyuha 20220125 조명 예외 처리

	int light_sel = m_cmbLightSelect.GetCurSel();
	int nIndex = m_cmbLightIndex.GetCurSel();
	if (m_pMain->vt_job_info[m_nJobID].light_info[nCam].num_of_using_light <= light_sel) return; // Tkyuha 20220125 조명 예외 처리
	int ctrl = m_pMain->vt_job_info[m_nJobID].light_info[nCam].controller_id[light_sel] - 1;
	int ch = m_pMain->vt_job_info[m_nJobID].light_info[nCam].channel_id[light_sel];

	if (ch < 0) return;

	m_edtLightValue.GetWindowTextA(str);
	int v = atoi(str);

	if (m_pMain->m_stLightCtrlInfo.nType[nCam] == LTYPE_LLIGHT_LPC_COT_232)
	{
		m_pMain->SetLightBright(ctrl, ch, v == 0 ? 0: v * 10);
	}
	else
	{
		m_pMain->SetLightBright(ctrl, ch, v);
	}
}

void TabCameraPage::OnBnClickedBtnNozzleInsp()
{
	//KJH 2021-11-06 Nozzle Insp 관련 Test용 버튼 생성

	int nJob = m_nJobID;
	int nCam = m_cmbSelectCamera.GetCurSel();
	int nPos = m_cmbSelectPosition.GetCurSel();
	int nIndex = m_cmbPatternIndex.GetCurSel();
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	m_pMain->m_pPattern[real_cam].pDlg = this;
	m_pMain->m_pPattern[real_cam].nPos = nPos;
	m_pMain->m_pPattern[real_cam].nCam = nCam;
	m_pMain->m_pPattern[real_cam].nJob = m_nJobID;
	//210110
	int nMethod = m_pMain->vt_job_info[nJob].algo_method;
	int w = m_pMain->m_stCamInfo[real_cam].w;
	int h = m_pMain->m_stCamInfo[real_cam].h;

	BOOL bCheck = m_chkSubInsp.GetCheck();
	memcpy(m_pMain->getProcBuffer(real_cam, 2), m_pMain->getCameraViewBuffer(), w * h);

	switch (nMethod)
	{
	case CLIENT_TYPE_ELB_CENTER_ALIGN:
	{
		::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_INSP_NOZZLE_DISTANCE_POLAR, MAKELPARAM(real_cam, m_nJobID));
	}
	break;
	case CLIENT_TYPE_CENTER_NOZZLE_ALIGN:
	{
		if (nCam == 0)
		{
			if (bCheck)
			{
				::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_INSP_NOZZLE_DISTANCE_REVERSE_SIDE_REF, MAKELPARAM(nCam, m_nJobID));
			}
			else
			{
				::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_INSP_NOZZLE_DISTANCE_REVERSE_SIDE, MAKELPARAM(nCam, m_nJobID));
			}
		}
		else
		{
			if (bCheck)
			{
				::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_INSP_NOZZLE_DISTANCE_REVERSE_CENTER_REF, MAKELPARAM(nCam, m_nJobID));
			}
			else
			{
				::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_INSP_NOZZLE_DISTANCE_REVERSE_CENTER, MAKELPARAM(nCam, m_nJobID));
			}
		}
	}
	break;
	case CLIENT_TYPE_ALIGN:
	{
		::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_INSP_NOZZLE_VIEW, MAKELPARAM(real_cam, m_nJobID));
	}
	break;
	//case CLIENT_TYPE_1CAM_1SHOT_ALIGN:  //20220929 스크레치 검사 테스트 Tkyuha
	//{
	//	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_INSP_SCRATCH_INSPECTION, MAKELPARAM(real_cam, m_nJobID));
	//}
	//break;
	default:
	{
		// Tkyuha 221109 여러개 이미지 테스트용
	}
		break;
	}
}
void TabCameraPage::OnBnClickedBtnPatternAlignRef()
{
	if (getHardPassword() == FALSE) return;

	//if (AfxMessageBox("Set the Align Reference Position?", MB_YESNO) != IDYES) return;
	if (m_pMain->fnSetMessage(2, "Set the Align Reference Position?") != TRUE) return;

	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
	int nIndex = ((CComboBox*)GetDlgItem(IDC_CB_PATTERN_INDEX))->GetCurSel();
	int nJob = m_nJobID;
	double posX, posY, posT = 0.0;

	sLine line_info[2];

	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	int nMethod = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();

	// KBJ 2022-07-19 Reference 등록할때 한번 찾겠끔 수정
	_stPatternSelect pattern;
	pattern.nCam = nCam;
	pattern.nPos = nPos;
	pattern.nIndex = nIndex;
	pattern.nJob = m_nJobID;
	::SendMessage(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_PATTERN_SEARCH, (LPARAM)&pattern);

	if (nMethod == METHOD_MATCHING)
	{
		posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetXPos();
		posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetYPos();
		posT = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetAngle();
	}
	else
	{
		line_info[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_1].m_lineInfo;
		line_info[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_2].m_lineInfo;
		sLine lineHori = line_info[0];
		sLine lineVert = line_info[1];

		CCaliper::cramersRules(-lineHori.a, 1, -lineVert.a, 1, lineHori.b, lineVert.b, &posX, &posY);
	}

	m_pMain->GetMatching(nJob).setRefX(nCam, nPos, posX);
	m_pMain->GetMatching(nJob).setRefY(nCam, nPos, posY);
	m_pMain->GetMatching(nJob).setRefT(nCam, nPos, posT);

	updateFrameDialog();
	m_pMain->setViewerReferencePos();

	_stFindPattern pattern_info;
	pattern_info.nCam = nCam;
	pattern_info.nPos = nPos;
	pattern_info.nJob = nJob;

	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_DRAW_REFERNCE_POS, (LPARAM)&pattern_info);

	//hsj 2022-02-14 reference 변경 유무
	m_pMain->m_bChangeReference[nJob][nPos] = TRUE;

	CString strText;
	strText.Format("nJob = %d, nCam = %d, nPos = %d Ref Registered", nJob, nCam, nPos);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strText);

}

BOOL TabCameraPage::IsClickRefPosXTitle(CPoint pt)
{
	CRect IdRect, rect;
	GetWindowRect(&rect);
	m_lblRefPosXTitle.GetWindowRect(&IdRect);

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;
	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;
	IdRect.bottom = IdRect.top + h;

	if (pt.x >= IdRect.left && pt.x <= IdRect.right && pt.y >= IdRect.top && pt.y <= IdRect.bottom)		return TRUE;
	else		return FALSE;
}

BOOL TabCameraPage::IsClickRefPosYTitle(CPoint pt)
{
	CRect IdRect, rect;
	GetWindowRect(&rect);
	m_lblRefPosYTitle.GetWindowRect(&IdRect);

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;
	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;
	IdRect.bottom = IdRect.top + h;

	if (pt.x >= IdRect.left && pt.x <= IdRect.right && pt.y >= IdRect.top && pt.y <= IdRect.bottom)		return TRUE;
	else		return FALSE;
}
BOOL TabCameraPage::IsClickRefPosTTitle(CPoint pt)
{
	CRect IdRect, rect;
	GetWindowRect(&rect);
	m_lblRefPosTTitle.GetWindowRect(&IdRect);

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;
	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;
	IdRect.bottom = IdRect.top + h;

	if (pt.x >= IdRect.left && pt.x <= IdRect.right && pt.y >= IdRect.top && pt.y <= IdRect.bottom)		return TRUE;
	else		return FALSE;
}

BOOL TabCameraPage::IsClickFixturePosXTitle(CPoint pt)
{
	CRect IdRect, rect;
	GetWindowRect(&rect);
	m_lblFixturePosXTitle.GetWindowRect(&IdRect);

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;
	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;
	IdRect.bottom = IdRect.top + h;

	if (pt.x >= IdRect.left && pt.x <= IdRect.right && pt.y >= IdRect.top && pt.y <= IdRect.bottom)		return TRUE;
	else		return FALSE;
}

BOOL TabCameraPage::IsClickFixturePosYTitle(CPoint pt)
{
	CRect IdRect, rect;
	GetWindowRect(&rect);
	m_lblFixturePosYTitle.GetWindowRect(&IdRect);

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;
	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;
	IdRect.bottom = IdRect.top + h;

	if (pt.x >= IdRect.left && pt.x <= IdRect.right && pt.y >= IdRect.top && pt.y <= IdRect.bottom)		return TRUE;
	else		return FALSE;
}
BOOL TabCameraPage::IsClickFixturePosTTitle(CPoint pt)
{
	CRect IdRect, rect;
	GetWindowRect(&rect);
	m_lblFixturePosTTitle.GetWindowRect(&IdRect);

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;
	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;
	IdRect.bottom = IdRect.top + h;

	if (pt.x >= IdRect.left && pt.x <= IdRect.right && pt.y >= IdRect.top && pt.y <= IdRect.bottom)		return TRUE;
	else		return FALSE;
}

BOOL TabCameraPage::IsClickImageProcessingTitle(CPoint pt)
{
	CRect IdRect, rect;
	GetWindowRect(&rect);
	m_lblImageProc.GetWindowRect(&IdRect);

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;
	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;
	IdRect.bottom = IdRect.top + h;

	if (pt.x >= IdRect.left && pt.x <= IdRect.right && pt.y >= IdRect.top && pt.y <= IdRect.bottom)		return TRUE;
	else		return FALSE;
}

BOOL TabCameraPage::IsClickFocusEnergyTitle(CPoint pt)
{
	CRect IdRect, rect;
	GetWindowRect(&rect);
	m_LbFocusEnergyTitle.GetWindowRect(&IdRect);

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;
	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;
	IdRect.bottom = IdRect.top + h;

	if (pt.x >= IdRect.left && pt.x <= IdRect.right && pt.y >= IdRect.top && pt.y <= IdRect.bottom)		return TRUE;
	else		return FALSE;
}

BOOL TabCameraPage::IsClickPatternIndexTitle(CPoint pt)
{
	CRect IdRect, rect;
	GetWindowRect(&rect);
	m_LbPatternIndex.GetWindowRect(&IdRect);

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;
	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;
	IdRect.bottom = IdRect.top + h;

	if (pt.x >= IdRect.left && pt.x <= IdRect.right && pt.y >= IdRect.top && pt.y <= IdRect.bottom)		return TRUE;
	else		return FALSE;
}

void TabCameraPage::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (IsClickRefPosXTitle(point) == TRUE)
	{
		if (m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getUseReferenceMark() != TRUE) return;
		if (m_pMain->fnSetMessage(2, "Clear the Align X Reference Position?") != TRUE) return;
		if (getHardPassword() == FALSE) return;
		UpdateReferencePosition(0, -1);
	}
	else if (IsClickRefPosYTitle(point) == TRUE)
	{
		if (m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getUseReferenceMark() != TRUE) return;
		if (m_pMain->fnSetMessage(2, "Clear the Align Y Reference Position?") != TRUE) return;
		if (getHardPassword() == FALSE) return;
		UpdateReferencePosition(-1, 0);
	}
	else if (IsClickRefPosTTitle(point) == TRUE)
	{
		if (m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getUseReferenceMark() != TRUE) return;
		if (m_pMain->fnSetMessage(2, "Clear the Align T Reference Position?") != TRUE) return;
		if (getHardPassword() == FALSE) return;
		UpdateReferencePosition(-1, -1, 0);
	}
	else if (IsClickFixturePosXTitle(point) == TRUE)
	{
		if (m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getUseFixtureMark() != TRUE) return;
		if (m_pMain->fnSetMessage(2, "Clear the Fixture X Position?") != TRUE) return;
		if (getHardPassword() == FALSE) return;
		UpdateFixturePosition(0, -1);
	}
	else if (IsClickFixturePosYTitle(point) == TRUE)
	{
		if (m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getUseFixtureMark() != TRUE) return;
		if (m_pMain->fnSetMessage(2, "Clear the Fixture Y Position?") != TRUE) return;
		if (getHardPassword() == FALSE) return;
		UpdateFixturePosition(-1, 0);
	}
	else if (IsClickFixturePosTTitle(point) == TRUE)
	{
		if (m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getUseFixtureMark() != TRUE) return;
		if (m_pMain->fnSetMessage(2, "Clear the Fixture T Position?") != TRUE) return;
		if (getHardPassword() == FALSE) return;
		UpdateFixturePosition(-1, -1, 0);
	}
	else if (IsClickImageProcessingTitle(point) == TRUE) // 20221221 이미지 처리 시뮬레이션용 Tkyuha
	{
		CImageProcessingSimulator dlg;
		int sel = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
		int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[sel];
		int W = m_pMain->m_stCamInfo[real_cam].w;
		int H = m_pMain->m_stCamInfo[real_cam].h;

		cv::Mat srcImg(H, W, CV_8UC1, m_pMain->getCameraViewBuffer());
		CFormCameraView* pCamera = (CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA];

		dlg.m_srcImg = &srcImg;
		dlg.m_pView = &pCamera->m_ViewerCamera;

		dlg.DoModal();

		srcImg.release();
	}
	else if (IsClickFocusEnergyTitle(point) == TRUE)
	{
		if (m_pMain->m_pCalculator_Dlg->IsWindowVisible())
		{
			m_pMain->m_pCalculator_Dlg->ShowWindow(SW_HIDE);
		}
		else
		{
			m_pMain->m_pCalculator_Dlg->ShowWindow(SW_SHOW);
		}
	}
	else if (IsClickPatternIndexTitle(point) == TRUE)
	{
		CopyMarkImage();
	}

	CDialogEx::OnLButtonUp(nFlags, point);
}

void TabCameraPage::UpdateReferencePosition(double posX, double posY, double posT)
{
	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
	int nJob = m_nJobID;

	if (posX != -1) m_pMain->GetMatching(nJob).setRefX(nCam, nPos, posX);
	if (posY != -1) m_pMain->GetMatching(nJob).setRefY(nCam, nPos, posY);
	if (posT != -1) m_pMain->GetMatching(nJob).setRefT(nCam, nPos, posT);

	updateFrameDialog();
	m_pMain->setViewerReferencePos();

	_stFindPattern pattern_info;
	pattern_info.nCam = nCam;
	pattern_info.nPos = nPos;
	pattern_info.nJob = nJob;

	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_DRAW_REFERNCE_POS, (LPARAM)&pattern_info);
}

void TabCameraPage::UpdateFixturePosition(double posX, double posY, double posT)
{
	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
	int nJob = m_nJobID;

	if (posX != -1) m_pMain->GetMatching(nJob).setFixtureX(nCam, nPos, posX);
	if (posY != -1) m_pMain->GetMatching(nJob).setFixtureY(nCam, nPos, posY);
	if (posT != -1) m_pMain->GetMatching(nJob).setFixtureT(nCam, nPos, posT);

	updateFrameDialog();

	_stFindPattern pattern_info;
	pattern_info.nCam = nCam;
	pattern_info.nPos = nPos;
	pattern_info.nJob = nJob;

	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_DRAW_FIXTURE_POS, (LPARAM)&pattern_info);
}

void TabCameraPage::make_elb_wetout_msa(int job_id, int cam_id, int caliper_id_in_hole, int caliper_id_wetout)
{
	int real_cam = m_pMain->vt_job_info[job_id].camera_index[cam_id];
	BYTE* pImage = m_pMain->getCameraViewBuffer();
	int w = m_pMain->vt_job_info[job_id].camera_size[cam_id].x;
	int h = m_pMain->vt_job_info[job_id].camera_size[cam_id].y;


	m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][caliper_id_in_hole].processCaliper_circle(pImage, w, h, 0.0, 0.0, 0.0);
	m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][caliper_id_wetout].processCaliper_circle(pImage, w, h, 0.0, 0.0, 0.0);

	CViewerEx* pViewer = &((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera;

	auto pDC = pViewer->getOverlayDC();
	pViewer->ClearOverlayDC();
	m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][caliper_id_in_hole].draw_final_result(pDC);
	m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][caliper_id_wetout].draw_final_result(pDC);

	sCircle circle;

	CString str_path;
	str_path.Format("d:\\wetout.csv");
	FILE* fp = fopen(str_path, "a");
	char cData[MAX_PATH] = { 0, };

	BOOL new_file = FALSE;
	if (_access(str_path, 0) != 0)
	{
		new_file = TRUE;
	}

	double dist[10] = { 0, };
	double x, y;

	x = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][caliper_id_in_hole].m_circle_info.x;
	y = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][caliper_id_in_hole].m_circle_info.y;
	circle = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][caliper_id_in_hole].m_circle_info;
	double dx, dy;

	double res_x = m_pMain->GetMachine(job_id).getCameraResolutionX(0, 0);
	double res_y = m_pMain->GetMachine(job_id).getCameraResolutionY(0, 0);
	int index = 0;

	dx = (x - m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][caliper_id_wetout].get_find_info(0).x) * res_x;
	dy = (y - m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][caliper_id_wetout].get_find_info(0).y) * res_y;
	dist[0] = sqrt(dx * dx + dy * dy);

	dx = (x - m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][caliper_id_wetout].get_find_info(12).x) * res_x;
	dy = (y - m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][caliper_id_wetout].get_find_info(12).y) * res_y;
	dist[1] = sqrt(dx * dx + dy * dy);

	dx = (x - m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][caliper_id_wetout].get_find_info(15).x) * res_x;
	dy = (y - m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][caliper_id_wetout].get_find_info(15).y) * res_y;
	dist[2] = sqrt(dx * dx + dy * dy);

	dx = (x - m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][caliper_id_wetout].get_find_info(24).x) * res_x;
	dy = (y - m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][caliper_id_wetout].get_find_info(24).y) * res_y;
	dist[3] = sqrt(dx * dx + dy * dy);

	dx = (x - m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][caliper_id_wetout].get_find_info(30).x) * res_x;
	dy = (y - m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][caliper_id_wetout].get_find_info(30).y) * res_y;
	dist[4] = sqrt(dx * dx + dy * dy);

	dx = (x - m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][caliper_id_wetout].get_find_info(36).x) * res_x;
	dy = (y - m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][caliper_id_wetout].get_find_info(36).y) * res_y;
	dist[5] = sqrt(dx * dx + dy * dy);

	dx = (x - m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][caliper_id_wetout].get_find_info(45).x) * res_x;
	dy = (y - m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][caliper_id_wetout].get_find_info(45).y) * res_y;
	dist[6] = sqrt(dx * dx + dy * dy);

	dx = (x - m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][caliper_id_wetout].get_find_info(48).x) * res_x;
	dy = (y - m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][caliper_id_wetout].get_find_info(48).y) * res_y;
	dist[7] = sqrt(dx * dx + dy * dy);

	sprintf(cData, "x,y,r,0,72,90,144,180,216,270,288\n");
	if (new_file)	fwrite(cData, 1, strlen(cData), fp);

	sprintf(cData, "%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n",
		circle.x, circle.y, circle.r, dist[0], dist[1], dist[2], dist[3], dist[4], dist[5], dist[6], dist[7]);
	fwrite(cData, 1, strlen(cData), fp);
	fclose(fp);
}

void TabCameraPage::OnBnClickedBtnFixturePos()
{
	if (getHardPassword() == FALSE) return;

	//if (AfxMessageBox("Set the Fixture Position?", MB_YESNO) != IDYES) return;
	if (m_pMain->fnSetMessage(2, "Set the Fixture Position?") != TRUE) return;

	int sel = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[sel];
	int W = m_pMain->m_stCamInfo[real_cam].w;
	int H = m_pMain->m_stCamInfo[real_cam].h;
	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
	int nJob = m_nJobID;

	memcpy(m_pMain->getProcBuffer(real_cam, 0), m_pMain->getCameraViewBuffer(), W * H);

	m_pMain->GetMatching(nJob).findPattern(m_pMain->getProcBuffer(real_cam, 0), nCam, nPos, W, H);
	if (m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetFound() != FIND_OK)
	{
		AfxMessageBox("Find Pattern Failed!!");
		return;
	}

	double posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetXPos();
	double posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetYPos();
	double posT = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetAngle();
	m_pMain->GetMatching(nJob).setFixtureX(nCam, nPos, posX);
	m_pMain->GetMatching(nJob).setFixtureY(nCam, nPos, posY);
	m_pMain->GetMatching(nJob).setFixtureT(nCam, nPos, posT);

	updateFrameDialog();

	theLog.logmsg(LOG_OPERATION, "[%s][Cam: %d][Pos: %d] Fixture Pos Set = X: %.3f, Y: %.3f, T: %.3f", m_pMain->vt_job_info[m_nJobID].get_job_name(), nCam, nPos, posX, posY, posT);

	//hsj 2022-02-14 reference 변경 유무
	m_pMain->m_bChangeFixture[nJob][nPos] = TRUE;

	_stFindPattern pattern_info;
	pattern_info.nCam = nCam;
	pattern_info.nPos = nPos;
	pattern_info.nJob = nJob;

	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_DRAW_FIXTURE_POS, (LPARAM)&pattern_info);
}

BOOL TabCameraPage::IsDbClickAllAlignRefRegist(CPoint pt)
{
	CRect IdRect, rect;
	GetWindowRect(&IdRect);

	if (pt.x >= IdRect.left && pt.x <= IdRect.right && pt.y >= IdRect.top && pt.y <= IdRect.bottom)		return TRUE;
	else		return FALSE;

}

// hsj 2022-03-10 reference 한번에 등록
BOOL TabCameraPage::AllAlignReferenceRegist()
{
	if (getHardPassword() == FALSE) return TRUE;

	if (m_pMain->fnSetMessage(2, "Set the All Align Reference Position?") != TRUE) return TRUE;

	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
	int nJob = m_nJobID;
	double posX, posY;

	sLine line_info[2];

	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	int nMethod = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();

	for (int nPos = 0; nPos < m_pMain->vt_job_info[nJob].num_of_position; nPos++)
	{
		// KBJ 220623 한번씩 다 찾겠끔 추가.
		if (nMethod == METHOD_MATCHING)
		{
			_stPatternSelect pattern;
			pattern.nCam = nCam;
			pattern.nPos = nPos;
			pattern.nIndex = 0;
			pattern.nJob = nJob;
			::SendMessage(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_PATTERN_SEARCH, (LPARAM)&pattern);

			posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetXPos();
			posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetYPos();
		}
		else
		{
			BOOL bFixtureUse = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseFixtureMark();
			if (bFixtureUse)
			{
				double _posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetXPos();
				double _posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetYPos();
				double _posT = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetAngle();

				double fixtureX = _posX - m_pMain->GetMatching(nJob).getFixtureX(nCam, nPos);
				double fixtureY = _posY - m_pMain->GetMatching(nJob).getFixtureY(nCam, nPos);
				double fixtureT = _posT - m_pMain->GetMatching(nJob).getFixtureT(nCam, nPos);

				// 2022-05-10 KBJ
				m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_1].setFixtureX(fixtureX);
				m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_1].setFixtureY(fixtureY);
				m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_1].setFixtureT(fixtureT);
				m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_1].calcPointsByFixture();
			}

			_stPatternSelect pattern;
			pattern.nCam = nCam;
			pattern.nPos = nPos;
			pattern.nIndex = 0;
			pattern.nJob = nJob;

			::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_SEARCH_CALIPER_INSP, (LPARAM)&pattern);

			line_info[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_1].m_lineInfo;
			line_info[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_2].m_lineInfo;
			sLine lineHori = line_info[0];
			sLine lineVert = line_info[1];

			CCaliper::cramersRules(-lineHori.a, 1, -lineVert.a, 1, lineHori.b, lineVert.b, &posX, &posY);
		}

		m_pMain->GetMatching(nJob).setRefX(nCam, nPos, posX);
		m_pMain->GetMatching(nJob).setRefY(nCam, nPos, posY);

		//hsj 2022-02-14 reference 변경 유무
		m_pMain->m_bChangeReference[nJob][nPos] = TRUE;
	}
	updateFrameDialog();
	m_pMain->setViewerReferencePos();

	_stFindPattern pattern_info;
	pattern_info.nCam = nCam;
	pattern_info.nPos = nPos;
	pattern_info.nJob = nJob;

	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_DRAW_ALL_REFERNCE_POS, (LPARAM)&pattern_info);

	CString strText;
	strText.Format("All Align Reference Position Registered");
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strText);

	m_pMain->AllReferSaveFlag(TRUE); //2022.06.18 ksm 전체 Reference 추가 시 Flag


	return FALSE;

}

BOOL TabCameraPage::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_RBUTTONDBLCLK && m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getUseReferenceMark())
	{
		if (IsDbClickAllAlignRefRegist(pMsg->pt) == TRUE)
		{
			BOOL bRet = FALSE;
			bRet = AllAlignReferenceRegist();
			return bRet;
		}
		else return TRUE;
	}
	//2022.06.29 ksm Camera 및 Algorithm Dialog 창 ESC 사용 안되도록 수정
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
			return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

// hsj 2022-03-11 Reference 등록시 날짜 표시
void TabCameraPage::ReferenceRegistDate(int nPos)
{
	SYSTEMTIME time;
	::GetLocalTime(&time);

	CString strDateX, strDateY, strDateT;

	strDateX.Format("Reference Pos X [%02d/%02d]", time.wMonth, time.wDay);
	strDateY.Format("Reference Pos Y [%02d/%02d]", time.wMonth, time.wDay);
	strDateT.Format("Reference Pos T [%02d/%02d]", time.wMonth, time.wDay);

	m_lblRefPosXTitle.SetText(strDateX);
	m_lblRefPosYTitle.SetText(strDateY);
	m_lblRefPosTTitle.SetText(strDateT);

	m_pMain->m_strRefDate_X[m_nJobID][nPos] = strDateX;
	m_pMain->m_strRefDate_Y[m_nJobID][nPos] = strDateY;
	m_pMain->m_strRefDate_T[m_nJobID][nPos] = strDateT;
}

//KJH 2022-05-10 Fixture 등록시 날짜 표시
void TabCameraPage::FixtureRegistDate(int nPos)
{
	SYSTEMTIME time;
	::GetLocalTime(&time);

	CString strDateX, strDateY, strDateT;

	strDateX.Format("Fixture Pos X [%02d/%02d]", time.wMonth, time.wDay);
	strDateY.Format("Fixture Pos Y [%02d/%02d]", time.wMonth, time.wDay);
	strDateT.Format("Fixture Pos T [%02d/%02d]", time.wMonth, time.wDay);

	m_lblFixturePosXTitle.SetText(strDateX);
	m_lblFixturePosYTitle.SetText(strDateY);
	m_lblFixturePosTTitle.SetText(strDateT);

	m_pMain->m_strFixtureDate_X[m_nJobID][nPos] = strDateX;
	m_pMain->m_strFixtureDate_Y[m_nJobID][nPos] = strDateY;
	m_pMain->m_strFixtureDate_T[m_nJobID][nPos] = strDateT;
}

void TabCameraPage::OnBnClickedBtnScaleCalcPopup()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_pMain->m_pPFScaleCalc->IsWindowVisible())
	{
		m_pMain->m_pPFScaleCalc->ShowWindow(SW_HIDE);
	}
	else
	{
		m_pMain->m_pPFScaleCalc->ShowWindow(SW_SHOW);
	}
}

// hsj 2023-01-02 Mark Copy Dialog 추가
void TabCameraPage::OnBnClickedBtnPatternCopy()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	DlgMarkCopy pDlg;
	pDlg.DoModal();
}

void TabCameraPage::CopyMarkImage()
{
	if (m_pMain->fnSetMessage(2, "Would you like to import an existing mark ?") != TRUE) return;
	if (getHardPassword() == FALSE) return;

	CFileDialog dlg(TRUE, NULL, _T("Image File"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Image Files (*.bmp;)|*.bmp|All Files (*.*)|*.*||"), this);

	if (dlg.DoModal() != IDOK)		return;

	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
	int nIndex = ((CComboBox*)GetDlgItem(IDC_CB_PATTERN_INDEX))->GetCurSel();

	CString str = dlg.GetPathName(), strFilePath;

	if (str.IsEmpty()) return;

	strFilePath.Format("%s%s\\PAT%d%d%d.bmp", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[m_nJobID].job_name.c_str(), nCam, nPos, nIndex);
	::CopyFileA(str, strFilePath, FALSE);

	double minconst = m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getHalconMinContrast(nCam);
	double highcosnt = m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getHalconHighContrast(nCam);

	if (m_pMain->GetMatching(m_nJobID).getMaskingModelUse())
		m_pMain->GetMatching(m_nJobID).getHalcon(nCam, nPos, nIndex).halcon_ReadMaskModel(strFilePath, m_pMain->GetMatching(m_nJobID).getTemplateAreaShape(nCam, nPos, nIndex),
			m_pMain->GetMatching(m_nJobID).getMaskRectUse(nCam, nPos, nIndex), m_pMain->GetMatching(m_nJobID).getMaskRectShape(nCam, nPos, nIndex), m_pMain->GetMatching(m_nJobID).getMaskRect(nCam, nPos, nIndex), minconst, highcosnt);

	else m_pMain->GetMatching(m_nJobID).getHalcon(nCam, nPos, nIndex).halcon_ReadModel(strFilePath, minconst, highcosnt);

	// Mark 등록시 mark offset은 0으로 초기화
	m_pMain->GetMatching(m_nJobID).setMarkOffsetX(nCam, nPos, nIndex, 0.0);
	m_pMain->GetMatching(m_nJobID).setMarkOffsetY(nCam, nPos, nIndex, 0.0);

	//2022.10.18 ksm ROI가 없는 경우 이미지 전체 스캔()
	CRect rcInspROI = m_pMain->GetMatching(m_nJobID).getSearchROI(nCam, nPos);
	if (rcInspROI.IsRectEmpty() || rcInspROI.IsRectNull())
	{
		int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];
		int W = m_pMain->m_stCamInfo[real_cam].w;
		int H = m_pMain->m_stCamInfo[real_cam].h;

		rcInspROI.top = 0;
		rcInspROI.left = 0;
		rcInspROI.right = W;
		rcInspROI.bottom = H;

		m_pMain->GetMatching(m_nJobID).setSearchROI(nCam, nPos, rcInspROI);
	}

	dispPatternIndex();
}