// TabCameraPage.cpp: 구현 파일
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "TabCameraPage.h"
#include "afxdialogex.h"
#include "MarkViewDlg.h"
#include "MaskMarkViewDlg.h"
#include "Caliper/DlgCaliper.h"
// TabCameraPage 대화 상자
#include <fstream>

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
	DDX_Control(pDX, IDC_LB_REFERENCE_POS_X_TITLE, m_lblRefPosXTitle);
	DDX_Control(pDX, IDC_LB_REFERENCE_POS_Y_TITLE, m_lblRefPosYTitle);
	DDX_Control(pDX, IDC_BTN_REFERENCE_POS, m_btnReferencePos);
	DDX_Control(pDX, IDC_BTN_CAMERA_SIMULATION, m_btnSimulation);
	DDX_Control(pDX, IDC_BTN_CAMERA_INSPECTION, m_btnInspSimulation);
	DDX_Control(pDX, IDC_BTN_SHOW_CALIPER, m_btnUseCaliper);
	DDX_Control(pDX, IDC_BTN_MOVE_TO_CENTER_CALIPER, m_btnMoveCaliperToCenter);
	DDX_Control(pDX, IDC_BTN_SET_SUB_MARK, m_btnSetSubMark);


	DDX_Control(pDX, IDC_CB_SELECT_CAMERA, m_cmbSelectCamera);
	DDX_Control(pDX, IDC_CB_SELECT_POSITION, m_cmbSelectPosition);
	DDX_Control(pDX, IDC_CB_PATTERN_INDEX, m_cmbPatternIndex);



	DDX_Control(pDX, IDC_CHK_ENABLE_PROFILE, m_chkShowProfile);
	DDX_Control(pDX, IDC_CHK_SUB_INSP, m_chkSubInsp);

	DDX_Control(pDX, IDC_LB_MARK_OFFSET_X_TITLE, m_lblMarkOffsetXTitle);
	DDX_Control(pDX, IDC_LB_MARK_OFFSET_Y_TITLE, m_lblMarkOffsetYTitle);
	DDX_Control(pDX, IDC_LB_MARK_OFFSET_X, m_lblMarkOffsetX);
	DDX_Control(pDX, IDC_LB_MARK_OFFSET_Y, m_lblMarkOffsetY);
	DDX_Control(pDX, IDC_LB_EXPOSURETIME, m_lblExposureTime);
	DDX_Control(pDX, IDC_CB_SELECT_USER_SET, m_cbUserSet);
	DDX_Control(pDX, IDC_EDIT_EXPOSURE_SETTING, m_edtExposureTime);
	DDX_Control(pDX, IDC_SLIDER_EXPOSURE_SETTING, m_sliderExposureTime);
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
	ON_BN_CLICKED(IDC_BTN_REFERENCE_POS, &TabCameraPage::OnBnClickedBtnReferencePos)
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
	ON_EN_SETFOCUS(IDC_EDIT_LIGHT_SETTING, &TabCameraPage::OnEnSetfocusEditLightSetting)
//	ON_EN_CHANGE(IDC_EDIT_LIGHT_SETTING, &TabCameraPage::OnEnChangeEditLightSetting)
	ON_BN_CLICKED(IDC_CHK_ENABLE_PROFILE, &TabCameraPage::OnBnClickedChkEnableProfile)
	ON_BN_CLICKED(IDC_CHK_SUB_INSP, &TabCameraPage::OnBnClickedChkEnableSubInsp)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BTN_CAMERA_INSPECTION, &TabCameraPage::OnBnClickedBtnCameraInspection)
	ON_BN_CLICKED(IDC_BTN_SET_SUB_MARK, &TabCameraPage::OnBnClickedBtnSetSubMark)
	ON_CBN_SELCHANGE(IDC_CB_SELECT_CAMERA, &TabCameraPage::OnCbnSelchangeCbSelectCamera)
	ON_BN_CLICKED(IDC_CHK_SUB_INSP, &TabCameraPage::OnBnClickedChkSubInsp)
	ON_BN_CLICKED(IDC_BTN_PATTERN_TEMP, &TabCameraPage::OnBnClickedBtnPatternTemp)
	ON_EN_SETFOCUS(IDC_EDIT_EXPOSURE_SETTING, &TabCameraPage::OnEnSetfocusEditExposureSetting)
	//ON_EN_CHANGE(IDC_EDIT_EXPOSURE_SETTING, &TabCameraPage::OnEnChangeEditExposureSetting)
END_MESSAGE_MAP()


// TabCameraPage 메시지 처리기
BOOL TabCameraPage::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);
	m_pMain = (CLET_AlignClientDlg *)AfxGetMainWnd();

	int nSel = 0;
	int nCam = 0;

	InitTitle(&m_LbPatternIndex, "Pattern Index", 14.f, RGB(64, 64, 64));
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
	InitTitle(&m_lblRefPosX, "", 14.f, COLOR_UI_BODY);
	InitTitle(&m_lblRefPosY, "", 14.f, COLOR_UI_BODY);
	
	//2021-05-11 KJH Ref 디스플레이 색상 변경
	m_lblRefPosX.SetColorText(255, RGB(255, 0, 255));
	m_lblRefPosY.SetColorText(255, RGB(255, 0, 255));
	InitTitle(&m_lblMarkOffsetXTitle, "Mark Offset X", 14.f, RGB(64, 64, 64));
	InitTitle(&m_lblMarkOffsetYTitle, "Mark Offset Y", 14.f, RGB(64, 64, 64));
	InitTitle(&m_lblMarkOffsetX, "", 14.f, RGB(64, 64, 64));
	InitTitle(&m_lblMarkOffsetY, "", 14.f, RGB(64, 64, 64));
	InitTitle(&m_lblExposureTime, "Exposure Time", 14.f, RGB(64, 64, 64));

	

	MainButtonInit(&m_btnReferencePos);			m_btnReferencePos.SetSizeText(14.f);
	MainButtonInit(&m_btnPatternRegist);			m_btnPatternRegist.SetSizeText(14.f);
	MainButtonInit(&m_btnPatternSearch);			m_btnPatternSearch.SetSizeText(14.f);
	MainButtonInit(&m_btnPatternView);			m_btnPatternView.SetSizeText(14.f);
	MainButtonInit(&m_btnPatternSend);			m_btnPatternSend.SetSizeText(14.f);
	MainButtonInit(&m_btnSetRoi);					m_btnSetRoi.SetSizeText(14.f);
	MainButtonInit(&m_btnImageSave);				m_btnImageSave.SetSizeText(14.f);
	MainButtonInit(&m_btnImageOpen);				m_btnImageOpen.SetSizeText(14.f);
	MainButtonInit(&m_btnCameraLive);				m_btnCameraLive.SetSizeText(14.f);
	MainButtonInit(&m_btnCameraStop);				m_btnCameraStop.SetSizeText(14.f);
	MainButtonInit(&m_btnSizeOriginal);			m_btnSizeOriginal.SetSizeText(14.f);
	MainButtonInit(&m_btnSizeFit);				m_btnSizeFit.SetSizeText(14.f);
	MainButtonInit(&m_btnDrawArea);				m_btnDrawArea.SetSizeText(14.f);	
	MainButtonInit(&m_btnFocusMeasure);			m_btnFocusMeasure.SetSizeText(14.f);
	MainButtonInit(&m_btnPtnDelete);				m_btnPtnDelete.SetSizeText(14.f);
	MainButtonInit(&m_btnSimulation);				m_btnSimulation.SetSizeText(12.f);
	MainButtonInit(&m_btnInspSimulation);        m_btnInspSimulation.SetSizeText(12.f);
	MainButtonInit(&m_btnUseCaliper);				m_btnUseCaliper.SetSizeText(12.f);
	MainButtonInit(&m_btnMoveCaliperToCenter);		m_btnMoveCaliperToCenter.SetSizeText(12.f);
	MainButtonInit(&m_btnSetSubMark);				m_btnSetSubMark.SetSizeText(12.f);	

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

	if(m_pMain->m_stLightCtrlInfo.nType[0]==7)  m_sliderLightSetting.SetRange(0, 100);
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
			m_pMain->SetLightBright(ctrl, ch, value);
		}
	}

	//exposure time 범위 지정
	m_sliderExposureTime.SetRange(25, 10000000);
	m_sliderLightSetting.SetPos(m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getExposureTime(nCam));
	
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

	for (int nIndex = 1; nIndex < MAX_PATTERN_INDEX; nIndex++)
	{
		if (m_pMain->GetMatching(m_nJobID).getHalcon(nCam, 0, nIndex).getModelRead())			str.Format("%d : OK", nIndex);
		else			str.Format("%d : Empty", nIndex);

		((CComboBox*)GetDlgItem(IDC_CB_PATTERN_INDEX))->AddString(str);
	}

	m_cmbImageProc.SetCurSel(0);
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->SetCurSel(0);
	((CComboBox*)GetDlgItem(IDC_CB_LIGHT_INDEX))->SetCurSel(0);
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_LIGHT_CHANNEL))->SetCurSel(0);
	((CComboBox*)GetDlgItem(IDC_CB_PATTERN_INDEX))->SetCurSel(0);

	GetDlgItem(IDC_EDIT_MATCHING_RATE)->SetWindowTextA("0.0");

	for (int i = 0; i < m_pMain->vt_job_info[m_nJobID].camera_index.size(); i++)
	{
		int cam = m_pMain->vt_job_info[m_nJobID].camera_index[i];
		str.Format("%s", m_pMain->m_stCamInfo[cam].cName);
		m_cmbSelectCamera.AddString(str);
	}

	m_btnMoveCaliperToCenter.SetEnable(false);
	m_cmbSelectCamera.SetCurSel(0);
	m_cmbSelectPosition.SetCurSel(0);

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
		if (m_pMain->GetMatching(m_nJobID).getHalcon(nCam, nPos, i).getModelRead())			str.Format("%d : OK", i);
		else			str.Format("%d : Empty", i);

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


	if (m_pMain->m_pBaslerCam[real_cam] && m_pMain->m_pBaslerCam[real_cam]->IsGrabContinuous() ||
		(m_pMain->m_pSaperaCam[real_cam].IsOpend() && m_pMain->m_pSaperaCam[real_cam].IsGrabing()))
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

	//2021-05-11 KJH Ref Null일때 디스플레이 예외처리

	if (posX < 0) posX = 0;
	if (posY < 0) posY = 0;

	CString strTemp;
	strTemp.Format("%.3f", posX);
	m_lblRefPosX.SetText(strTemp);
	strTemp.Format("%.3f", posY);
	m_lblRefPosY.SetText(strTemp);
}

void TabCameraPage::OnBnClickedBtnSizeOriginal()
{
	((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.setOriginalImage();
	((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.setNavi(true);
	dispViewSize();
}


void TabCameraPage::OnBnClickedBtnSizeFit()
{
	((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.OnFitImage();
	((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.setNavi(false);
	dispViewSize();
}


void TabCameraPage::OnBnClickedBtnCameraLive()
{
	CLET_AlignClientDlg *pMain = (CLET_AlignClientDlg *)AfxGetMainWnd();
	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];

	for (int i = 0; i < MAX_CAMERA; i++)
		pMain->SendMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_STOP, i);
	
	pMain->SendMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_LIVE, real_cam);

	updateFrameDialog();
}


void TabCameraPage::OnBnClickedBtnCameraStop()
{
	CLET_AlignClientDlg *pMain = (CLET_AlignClientDlg *)AfxGetMainWnd();
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
			theApp.setMessage(MT_OK, "확장자가 없습니다.");
#else
			AfxMessageBox("확장자가 없습니다.");
#endif
			return;
		}

		int sel = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
		int cam = m_pMain->vt_job_info[m_nJobID].camera_index[sel];
		int W = m_pMain->m_stCamInfo[cam].w;
		int H = m_pMain->m_stCamInfo[cam].h;

		IplImage *pImage = cvCreateImage(cvSize(W, H), 8, 1);
		cvZero(pImage);
		m_pMain->copyMemory(pImage->imageData, m_pMain->getCameraViewBuffer(), W*H);
		cvSaveImage(dlg.GetPathName(), pImage);
		cvReleaseImage(&pImage);
	}
}


void TabCameraPage::OnBnClickedBtnImageOpen()
{
	CFileDialog dlg(TRUE, NULL, _T("Image File"),	OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Image Files (*.bmp;*.jpg;)|*.bmp;*.jpg|All Files (*.*)|*.*||"),		this);

	if (dlg.DoModal() != IDOK)		return;

	CString str = dlg.GetPathName();
	/*

	cv::Mat img = cv::imread(std::string(str), 0);

	vector<int> opt;
	opt.assign(2, 0);
	opt[0] = CV_IMWRITE_JPEG_QUALITY;
	opt[1] = 95;


	str.Format("c:\\qual_%d.jpg", opt[1]);
	cv::imwrite(std::string(str), img, opt);

	opt[1] = 90;
	str.Format("c:\\qual_%d.jpg", opt[1]);
	cv::imwrite(std::string(str), img, opt);


	opt[1] = 80;
	str.Format("c:\\qual_%d.jpg", opt[1]);
	cv::imwrite(std::string(str), img, opt);

	opt[1] = 70;
	str.Format("c:\\qual_%d.jpg", opt[1]);
	cv::imwrite(std::string(str), img, opt);

	opt[1] = 60;
	str.Format("c:\\qual_%d.jpg", opt[1]);
	cv::imwrite(std::string(str), img, opt);

	opt[1] = 50;
	str.Format("c:\\qual_%d.jpg", opt[1]);
	cv::imwrite(std::string(str), img, opt);

	opt[1] = 40;
	str.Format("c:\\qual_%d.jpg", opt[1]);
	cv::imwrite(std::string(str), img, opt);


	opt[1] = 30;
	str.Format("c:\\qual_%d.jpg", opt[1]);
	cv::imwrite(std::string(str), img, opt);

	opt[1] = 20;
	str.Format("c:\\qual_%d.jpg", opt[1]);
	cv::imwrite(std::string(str), img, opt);

	opt[1] = 10;
	str.Format("c:\\qual_%d.jpg", opt[1]);
	cv::imwrite(std::string(str), img, opt);
*/
	
	IplImage *pImage = cvLoadImage(str, 0);

	if (pImage == NULL)
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

	if (pImage->width != W || pImage->height != H)
	{
		if (m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_SCAN_INSP)
		{
			cv::Mat imgOriginal = cv::cvarrToMat(pImage);
			cv::Mat imgCut = imgOriginal(cv::Rect(0, nPos*H, W, H));
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
		cvReleaseImage(&pImage);
		return;
	}
	else
	{
		// 임시.. 테스트용
		if(pImage->width != camW || pImage->height != camH)
			((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.OnInitWithCamera(pImage->width, pImage->height, 8);

		m_pMain->copyMemory(m_pMain->getSrcBuffer(real_cam), pImage->imageData, W * H);
		m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 0), pImage->imageData, W * H);
		m_pMain->copyMemory(m_pMain->getCameraViewBuffer(), pImage->imageData, W * H);
		cvReleaseImage(&pImage);
		((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.OnLoadImageFromPtr(m_pMain->getCameraViewBuffer());
	}
}


void TabCameraPage::OnBnClickedBtnShowCaliper()
{
	CFormCameraView *pCamera = (CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA];

	if (pCamera->m_ViewerCamera.getUseCaliper())
	{
		pCamera->m_ViewerCamera.ClearOverlayDC();	//210209
		pCamera->m_ViewerCamera.Invalidate();
		pCamera->m_ViewerCamera.setUseCaliper(FALSE);
		pCamera->m_ViewerCamera.setDrawCaliper(FALSE);
		m_btnUseCaliper.SetColorBkg(255, COLOR_BTN_BODY);
		m_pMain->m_pDlgCaliper->ShowWindow(SW_HIDE);
		m_btnMoveCaliperToCenter.SetEnable(false);
		m_btnDrawArea.SetEnable(true);
	}
	else
	{
		pCamera->m_ViewerCamera.setUseCaliper(TRUE);
		pCamera->m_ViewerCamera.setDrawCaliper(TRUE);
		m_btnUseCaliper.SetColorBkg(255, COLOR_BTN_SELECT);
		m_pMain->m_pDlgCaliper->ShowWindow(SW_SHOW);
		m_btnMoveCaliperToCenter.SetEnable(true);
		m_btnDrawArea.SetEnable(false);
		pCamera->m_ViewerCamera.ClearOverlayDC();
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

	CFormCameraView *pCamera = (CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA];

	CRect rect;

	pCamera->m_ViewerCamera.GetWindowRect(&rect);
	double x = pCamera->m_ViewerCamera.getResult().x + rect.Width() / 2;
	double y = pCamera->m_ViewerCamera.getResult().y + rect.Height() / 2;

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
	m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][nLine].calcCaliperRect();
	m_pMain->m_pDlgCaliper->m_pCaliperParam->updateCoordinates();

	pCamera->m_ViewerCamera.ClearCaliperOverlayDC();
	CDC *pDC = pCamera->m_ViewerCamera.getCaliperOverlayDC();
	m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][nLine].DrawCaliper(pDC);
	pCamera->m_ViewerCamera.Invalidate();
}


void TabCameraPage::OnBnClickedBtnDrawArea()
{
	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];
	int W = m_pMain->m_stCamInfo[real_cam].w;
	int H = m_pMain->m_stCamInfo[real_cam].h;

	if (((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.GetModeDrawingFigure())
	{
		((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetModeDrawingFigure(false);
		((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetEnableModifyFigure(0, false);
		((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetEnableDrawFigure(0, false);

		m_btnUseCaliper.SetEnable(true);
	}
	else
	{
		((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetEnableDrawAllFigures(true);
		((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetEnableFigureRectangle(0);
		((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetModeDrawingFigure(true);
		((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetEnableModifyFigure(0, true);
		((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetEnableDrawFigure(0, true);

		int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();

		CRect rectROI = m_pMain->GetMatching(m_nJobID).getSearchROI(nCam, nPos);
		((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetSizeRectangle(0, CPoint(rectROI.left, rectROI.top), CPoint(rectROI.right, rectROI.bottom));

		m_btnUseCaliper.SetEnable(false);		
	}

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
	strFilePath.Format("%sPAT%d%d%d.bmp", m_pMain->m_strCurrModelDir, nCam, nPos, nIndex);

	CFileStatus fs;
	if (CFile::GetStatus(strFilePath, fs))
	{
		m_pMain->GetMatching(m_nJobID).getHalcon(nCam, nPos, nIndex).setModelRead(false);
		::DeleteFileA(strFilePath);
	}

	updateFrameDialog();
}


void TabCameraPage::OnBnClickedBtnFocusMeasure()
{
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

void TabCameraPage::OnBnClickedBtnReferencePos()
{
	_stFindPattern pattern_info;
	pattern_info.nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	pattern_info.nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
	pattern_info.nJob = m_nJobID;

	if (m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_NOZZLE_ALIGN)
	{
		if (getPassword() == FALSE) return;
	}

	if (AfxMessageBox("Set the Reference Position?", MB_YESNO) != IDYES) return;
	
	int sel = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[sel];
	int W = m_pMain->m_stCamInfo[real_cam].w;
	int H = m_pMain->m_stCamInfo[real_cam].h;

	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
	int nJob = m_nJobID;

	memcpy(m_pMain->getProcBuffer(real_cam, 0), m_pMain->getCameraViewBuffer(), W * H);

	if (m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getMarkFindMethod() == METHOD_CALIPER)
	{
		m_pMain->GetMatching(nJob).setMarkFindMethod(METHOD_MATCHING);
		m_pMain->GetMatching(nJob).findPattern(m_pMain->getProcBuffer(real_cam, 0), nCam, nPos, W, H);
		m_pMain->GetMatching(nJob).setMarkFindMethod(METHOD_CALIPER);
	}
	else
	{
		m_pMain->GetMatching(nJob).findPattern(m_pMain->getProcBuffer(real_cam, 0), nCam, nPos, W, H);
	}



	if (m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetFound() != FIND_OK)
	{
		AfxMessageBox("Find Pattern Failed!!");
		return;
	}

	double posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetXPos();
	double posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetYPos();
	m_pMain->GetMatching(nJob).setRefX(nCam, nPos, posX);
	m_pMain->GetMatching(nJob).setRefY(nCam, nPos, posY);

	updateFrameDialog();
	m_pMain->setViewerReferencePos();

	//::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_DRAW_REFERNCE_POS, MAKELPARAM(nCam, nPos));
	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_DRAW_REFERNCE_POS, (LPARAM)&pattern_info);
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

	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
	int nIndex = ((CComboBox*)GetDlgItem(IDC_CB_PATTERN_INDEX))->GetCurSel();

	_stPatternSelect pattern;
	pattern.nCam = nCam;
	pattern.nPos = nPos;
	pattern.nIndex = nIndex;
	pattern.nJob = m_nJobID;
	
	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_PATTERN_REGIST, (LPARAM)&pattern);

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

	bool bauto=false;

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
	//CMarkViewDlg dlg;
	IplImage *pImage = cvLoadImage(strFilePath, 0);
	dlg.SetImage(pImage->width, pImage->height, pImage->width / 2 + offsetX, pImage->height / 2 + offsetY, (BYTE *)pImage->imageData);

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
		cvReleaseImage(&pImage);
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

	offsetX = dlg.m_dbPatRefX - pImage->width / 2;
	offsetY = dlg.m_dbPatRefY - pImage->height / 2;

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
			m_pMain->GetMatching(nJob).getHalcon(nCam, nPos, nIndex).halcon_ReadModelNew((BYTE*)pImage->imageData, grayimg.data, pImage->width, pImage->height);
		}
	}

	m_pMain->GetMatching(nJob).getHalcon(nCam, nPos, nIndex).halcon_SetShapeModelOrigin(offsetX, offsetY);

	cvReleaseImage(&pImage);

	if (bauto) OnBnClickedBtnReferencePos();
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

	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_PATTERN_SEARCH, (LPARAM)&pattern);
	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_PATTERN_DISPLAY_RESULT, (LPARAM)&pattern);

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

	if (AfxMessageBox("Do You Regist the ROI?", MB_YESNO) != IDYES)	return;

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

	if (select)		((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetEnableFigureCircle(0);
	else		((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetEnableFigureRectangle(0);
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
		m_pMain->SetLightBright(ctrl, ch, value);
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

	m_pMain->GetMatching(nJob).setMatchingRate(nCam, nPos, nIndex, atof(strTemp));
	GetDlgItem(IDC_EDIT_MATCHING_RATE)->SetWindowTextA(strTemp);
#endif
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
	int light_sel = m_cmbLightSelect.GetCurSel();
	int ctrl = m_pMain->vt_job_info[m_nJobID].light_info[nCam].controller_id[light_sel] - 1;
	int nIndex = m_cmbLightIndex.GetCurSel();
	int ch = m_pMain->vt_job_info[m_nJobID].light_info[nCam].channel_id[light_sel];

	if (ch < 0) return;

	m_edtLightValue.GetWindowTextA(str);
	strTemp = m_pMain->GetNumberBox(str, 5, 0, 255);
	m_edtLightValue.SetWindowTextA(strTemp);

	int v = atoi(strTemp);
	// dh.jung 20210511 change
	if (m_pMain->m_stLightCtrlInfo.nType[0] == 7) m_pMain->SetLightBright(ctrl, ch, v == 0 ? 1 : v * 10);
	else m_pMain->SetLightBright(ctrl, ch, v);
	// end
	m_pMain->vt_job_info[m_nJobID].model_info.getLightInfo().setLightBright(nCam, light_sel, nIndex, v);
	
	m_sliderLightSetting.SetPos(v);
#endif


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
			
			m_pMain->SetLightBright(ctrl, ch, nPos);
			m_pMain->getModel().getMachineInfo().setLightBright(cam, sel, nIndex, nPos);
		
			updateFrameDialog();
#else

			int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
			int nSel = m_cmbSelectCamera.GetCurSel();
			int nIndex = m_cmbPatternIndex.GetCurSel();

			CString str;
			int nPos = m_sliderLightSetting.GetPos();

			str.Format("%d", nPos);
			m_edtLightValue.SetWindowTextA(str);

			int light_sel = m_cmbLightSelect.GetCurSel();
			int ctrl = m_pMain->vt_job_info[m_nJobID].light_info[nCam].controller_id[light_sel] - 1;
			int light_nIndex = m_cmbLightIndex.GetCurSel();
			int ch = m_pMain->vt_job_info[m_nJobID].light_info[nCam].channel_id[light_sel];

			if (m_pMain->m_stLightCtrlInfo.nType[0] == 7) m_pMain->SetLightBright(ctrl, ch, nPos==0? 1:nPos*10);
			else m_pMain->SetLightBright(ctrl, ch, nPos);
			m_pMain->vt_job_info[m_nJobID].model_info.getLightInfo().setLightBright(nCam, light_sel, light_nIndex, nPos);

			updateFrameDialog();
#endif
		}

		if (pScrollBar == (CScrollBar*)&m_sliderExposureTime)
		{
			CString str;
			int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CAMERA))->GetCurSel();
			int nPos = m_sliderExposureTime.GetPos();

			str.Format("%d", nPos);
			m_edtExposureTime.SetWindowTextA(str);

			int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];
			m_pMain->m_pBaslerCam[real_cam]->SetExposure(nPos);
			m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().setExposureTime(real_cam, nPos);
		}
	}
	else
	{
	}

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void TabCameraPage::dispDrawArea()
{
	if (((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.GetModeDrawingFigure())		m_btnDrawArea.SetColorBkg(255, COLOR_BTN_SELECT);
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
	CDlgCaliper *pDlgCaliper = m_pMain->m_pDlgCaliper;
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
	CFormCameraView *pCamera = (CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA];
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

	CDlgCaliper *pDlgCaliper = m_pMain->m_pDlgCaliper;
	pDlgCaliper->m_nObject = nCam;
	pDlgCaliper->m_pCaliperParam->m_pCaliper = &pDlgCaliper->m_Caliper[nCam][nPos][pDlgCaliper->m_nLine];
	pDlgCaliper->m_pCaliperResult->m_pCaliper = &pDlgCaliper->m_Caliper[nCam][nPos][pDlgCaliper->m_nLine];
	
	{
		pDlgCaliper->m_pViewer->ClearCaliperOverlayDC();
		pDlgCaliper->m_pViewer->ClearOverlayDC();
	}
	pDlgCaliper->m_Caliper[nCam][nPos][pDlgCaliper->m_nLine].DrawCaliper(pDlgCaliper->m_Caliper[nCam][nPos][pDlgCaliper->m_nLine].getCaliperDC());
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
		if (!((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.GetModeDrawingFigure())
		{
			((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetModeDrawingFigure(true);
			dispDrawArea();
		}
	}

	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_INSP_SHOW_PROFILE, bCheck);

	if (!bCheck && ((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.GetModeDrawingFigure())
	{
		((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetModeDrawingFigure(true);
		((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetEnableModifyFigure(0, true);
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

			if(nPanelType == _CORE_MODEL_CALC)	m_pMain->algorithm_AssembleInsp_CALC(m_pMain->getCameraViewBuffer(), &m_pMain->m_pPattern[real_cam], nInpsMethod, TRUE);
			else								m_pMain->algorithm_assemble_Insp(m_pMain->getCameraViewBuffer(), &m_pMain->m_pPattern[real_cam], nInpsMethod, TRUE);
		}
		break;
	case CLIENT_TYPE_ELB_CENTER_ALIGN:
		{
			::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_INSP_NOTCH, MAKELPARAM(nCam, m_nJobID));
		}
		break;
	case CLIENT_TYPE_NOZZLE_ALIGN:
		{

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
			IplImage* pImage = cvLoadImage(str, 0);
			if (pImage == NULL)
			{
				AfxMessageBox("Check the Image Type");
				return;
			}
			((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.OnInitWithCamera(pImage->width, pImage->height, 8);
			((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.OnLoadImageFromPtr((BYTE*)pImage->imageData);
			cvReleaseImage(&pImage);

			m_pMain->m_nSeqScanInspGrabCount[m_nJobID] = 1;
		}
		else		m_pMain->m_nSeqScanInspGrabCount[m_nJobID] = nPos+1;

		int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
		::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_INSP_SCAN, MAKELPARAM(real_cam, m_nJobID));
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

	if (nMarkFindMethod == METHOD_CALIPER || nMarkFindMethod == METHOD_LINE || nMarkFindMethod == METHOD_NOTCH)
	{
		m_btnUseCaliper.SetEnable(TRUE);
		m_btnSimulation.SetEnable(TRUE);
	}
	else
	{
		m_btnUseCaliper.SetEnable(FALSE);
		m_btnSimulation.SetEnable(FALSE);		
	}

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
		pCamera->m_ViewerCamera.ClearOverlayDC();	//210209
		pCamera->m_ViewerCamera.Invalidate();
	}
	
	//inspection simulation 버튼 Disable
	int nMethod = m_pMain->vt_job_info[m_nJobID].algo_method;
	if (nMethod < CLIENT_TYPE_PANEL_EXIST_INSP)
	{
		m_btnInspSimulation.SetEnable(FALSE);
	}
	else
	{
		m_btnInspSimulation.SetEnable(TRUE);
	}
}
//210117


void TabCameraPage::OnBnClickedBtnSetSubMark()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CPatternMatching *pPattern = &m_pMain->GetMatching(m_nJobID);

	int nJob = m_nJobID;
	int nCam = m_cmbSelectCamera.GetCurSel();
	int nPos = m_cmbSelectPosition.GetCurSel();
	int nIndex = m_cmbPatternIndex.GetCurSel();
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	int W = m_pMain->m_stCamInfo[real_cam].w;
	int H = m_pMain->m_stCamInfo[real_cam].h;

	m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 0), m_pMain->getCameraViewBuffer(), W * H);
	BYTE *pImg = m_pMain->getProcBuffer(real_cam, 0);

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

	strTemp.Format("offX : %.3f offY : %.3f", dx, dy);

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

	updateFrameDialog();
}


void TabCameraPage::OnBnClickedChkSubInsp()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_chkSubInsp.GetCheck())	m_pMain->m_bUseSubInspCamView = TRUE;
	else							m_pMain->m_bUseSubInspCamView = FALSE;
}


void TabCameraPage::OnBnClickedBtnPatternTemp()
{
	CFileDialog dlg(TRUE, NULL, _T("CSV File"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("CSV Files (*.csv;*.CSV;)|*.csv;*.CSV|All Files (*.*)|*.*||"), this);

	if (dlg.DoModal() != IDOK)		return;

	CString str = dlg.GetPathName();

	std::ifstream openFile(str);

	if (openFile.is_open())
	{
		for (int i = 0; i < 5; i++) m_pMain->m_ELB_InspResult.m_vTraceProfile[i].clear();

		std::string line;
		getline(openFile, line);
		vector<string> xX = split(line, ',');

		int lineCount = atoi(xX.at(0).c_str());
		double rotateX = atof(xX.at(1).c_str());
		double rotateY = atof(xX.at(2).c_str());

		m_pMain->m_ELB_InspResult.m_vELB_RotateCenter.x = rotateX;
		m_pMain->m_ELB_InspResult.m_vELB_RotateCenter.y  = rotateY;
		m_pMain->m_ELB_InspResult.m_vELB_ReverseRotateCenter.x = -rotateX;
		m_pMain->m_ELB_InspResult.m_vELB_ReverseRotateCenter.y = -rotateY;

		while (getline(openFile, line))
		{
			vector<string> x = split(line, ',');
			double rx = atof(x.at(2).c_str()) ;  //3
			double ry = (atof(x.at(3).c_str())); //4
			double rt = atof(x.at(4).c_str()); // 임시

			m_pMain->m_ELB_InspResult.m_vTraceProfile[0].push_back(rx);
			m_pMain->m_ELB_InspResult.m_vTraceProfile[1].push_back(ry);
			m_pMain->m_ELB_InspResult.m_vTraceProfile[2].push_back(rt);

			x.clear();
		}
	}

	openFile.close();

	int nJob = m_nJobID;
	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);

	double r = int(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspTraceInterval() / xres);
	double pitch = r * xres;

	m_pMain->sendTraceProfileData(m_pMain->vt_job_info[nJob].plc_addr_info.write_word_sub_start,
		m_pMain->vt_job_info[nJob].plc_addr_info.write_word_start, pitch);
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

	m_pMain->m_pBaslerCam[real_cam]->SetExposure(v);
	m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().setExposureTime(real_cam,v);
	
	m_sliderExposureTime.SetPos(v);

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
