// CamLightControlDlg.cpp: 구현 파일
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "CamLightControlDlg.h"
#include "afxdialogex.h"


// CCamLightControlDlg 대화 상자

IMPLEMENT_DYNAMIC(CCamLightControlDlg, CDialogEx)

CCamLightControlDlg::CCamLightControlDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_CAMLIGHT_CONTROL, pParent)
{
	m_nJobID = 0;
	m_nCam = 0;
}

CCamLightControlDlg::~CCamLightControlDlg()
{
}

void CCamLightControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LB_LIGHT_INDEX, m_lblLightIndex);
	DDX_Control(pDX, IDC_LB_LIGHT_CHANNEL, m_lblLightChannel);
	DDX_Control(pDX, IDC_LB_EXPOSURETIME, m_lblExposureTime);
	DDX_Control(pDX, IDC_CB_SELECT_LIGHT_CHANNEL, m_cmbLightSelect);
	DDX_Control(pDX, IDC_CB_LIGHT_INDEX, m_cmbLightIndex);
	DDX_Control(pDX, IDC_CB_SELECT_USER_SET, m_cbUserSet);
	DDX_Control(pDX, IDC_EDIT_LIGHT_SETTING, m_edtLightValue);
	DDX_Control(pDX, IDC_EDIT_EXPOSURE_SETTING, m_edtExposureTime);
	DDX_Control(pDX, IDC_SLIDER_LIGHT_SETTING, m_sliderLightSetting);
	DDX_Control(pDX, IDC_SLIDER_EXPOSURE_SETTING, m_sliderExposureTime);
}


BEGIN_MESSAGE_MAP(CCamLightControlDlg, CDialogEx)
	ON_WM_HSCROLL()
	ON_WM_CTLCOLOR()
	ON_CBN_SELCHANGE(IDC_CB_SELECT_LIGHT_CHANNEL, &CCamLightControlDlg::OnCbnSelchangeCbSelectLightChannel)
	ON_CBN_SELCHANGE(IDC_CB_LIGHT_INDEX, &CCamLightControlDlg::OnCbnSelchangeCbLightIndex)
	ON_EN_SETFOCUS(IDC_EDIT_LIGHT_SETTING, &CCamLightControlDlg::OnEnSetfocusEditLightSetting)
	ON_EN_SETFOCUS(IDC_EDIT_EXPOSURE_SETTING, &CCamLightControlDlg::OnEnSetfocusEditExposureSetting)
END_MESSAGE_MAP()


// CCamLightControlDlg 메시지 처리기
HBRUSH CCamLightControlDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (pWnd->GetDlgCtrlID() == IDC_EDIT_LIGHT_SETTING ||		
		pWnd->GetDlgCtrlID() == IDC_CB_SELECT_LIGHT_CHANNEL ||
		pWnd->GetDlgCtrlID() == IDC_CB_LIGHT_INDEX ||
		pWnd->GetDlgCtrlID() == IDC_CB_SELECT_USER_SET ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_EXPOSURE_SETTING)
	{
		pDC->SetBkColor(COLOR_UI_BODY);
		pDC->SetTextColor(COLOR_WHITE);
	}

	return m_hbrBkg;
}

BOOL CCamLightControlDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);
	m_pMain = (CLET_AlignClientDlg*)AfxGetMainWnd();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	InitTitle(&m_lblLightChannel, "Light Channel", 14.f, RGB(64, 64, 64));
	InitTitle(&m_lblExposureTime, "Exposure Time", 14.f, RGB(64, 64, 64));
	InitTitle(&m_lblLightIndex, "Light Index", 14.f, RGB(64, 64, 64));

	if (m_pMain->m_stLightCtrlInfo.nType[0] == LTYPE_LLIGHT_LPC_COT_232)  m_sliderLightSetting.SetRange(0, 100);
	else m_sliderLightSetting.SetRange(0, 255);

	m_sliderExposureTime.SetRange(25, 10000000);
	
	m_cbUserSet.AddString("Default");
	m_cbUserSet.AddString("User 1");
	m_cbUserSet.AddString("User 2");
	m_cbUserSet.AddString("User 3");
	m_cbUserSet.SetCurSel(1);

	CString str;
	int nCam = 0, nJobID=0;

	str.Format("%d", m_pMain->vt_job_info[nJobID].model_info.getLightInfo().getLightBright(nCam, 0, 0));
	m_edtLightValue.SetWindowTextA(str);
	
	str.Format("%d", m_pMain->vt_job_info[nJobID].model_info.getMachineInfo().getExposureTime(nCam));
	m_edtExposureTime.SetWindowTextA(str);

	m_sliderExposureTime.SetPos(m_pMain->vt_job_info[nJobID].model_info.getMachineInfo().getExposureTime(nCam));
	m_sliderLightSetting.SetPos(m_pMain->vt_job_info[nJobID].model_info.getLightInfo().getLightBright(nCam, 0, 0));

	for (int nIndex = 0; nIndex < m_pMain->vt_job_info[nJobID].light_info[0].num_of_using_light; nIndex++)
	{
		str.Format("%s", m_pMain->vt_job_info[nJobID].light_info[0].channel_name[nIndex].c_str());
		((CComboBox*)GetDlgItem(IDC_CB_SELECT_LIGHT_CHANNEL))->AddString(str);
	}

	for (int nIndex = 0; nIndex < 4; nIndex++)
		((CComboBox*)GetDlgItem(IDC_CB_LIGHT_INDEX))->AddString(m_pMain->vt_job_info[nJobID].light_info[0].index_name[nIndex].c_str());

	((CComboBox*)GetDlgItem(IDC_CB_LIGHT_INDEX))->SetCurSel(0);
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_LIGHT_CHANNEL))->SetCurSel(0);

	for (int real_cam = 0; real_cam < MAX_CAMERA; real_cam++)
		m_bExposureTimeChange[real_cam] = FALSE;

	CenterWindow();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}
void CCamLightControlDlg::InitTitle(CLabelEx* pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}

void CCamLightControlDlg::OnCbnSelchangeCbSelectLightChannel()
{
	CString str;
	int nCam = m_nCam;
	int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];

	int light_sel = m_cmbLightSelect.GetCurSel();
	int nIndex = ((CComboBox*)GetDlgItem(IDC_CB_LIGHT_INDEX))->GetCurSel();

	str.Format("%d", m_pMain->vt_job_info[m_nJobID].model_info.getLightInfo().getLightBright(nCam, light_sel, nIndex));
	m_edtLightValue.SetWindowTextA(str);

	m_pMain->vt_job_info[m_nJobID].light_info[nCam].last_channel = light_sel;
}


void CCamLightControlDlg::OnCbnSelchangeCbLightIndex()
{
	CString str;
	//KJH 2021-12-31 Machine View Camera Index Debug
	int nCam = m_nCam;
	int light_sel = m_cmbLightSelect.GetCurSel();
	int nIndex = ((CComboBox*)GetDlgItem(IDC_CB_LIGHT_INDEX))->GetCurSel();

	str.Format("%d", m_pMain->vt_job_info[m_nJobID].model_info.getLightInfo().getLightBright(nCam, light_sel, nIndex));
	m_edtLightValue.SetWindowTextA(str);

	// index 선택시 해당 index의 값으로 모든 조명을 제어하자.
	m_pMain->LightControllerTurnOnOff(m_nJobID, nCam, nIndex);
}

void CCamLightControlDlg::OnEnSetfocusEditLightSetting()
{
	GetDlgItem(IDC_LB_LIGHT_CHANNEL)->SetFocus();

	CString str, strTemp;

	if (m_pMain->vt_job_info[m_nJobID].light_info[m_nCam].num_of_using_light <= 0) return; // Tkyuha 20220125 조명 예외 처리

	int nCam = m_nCam;
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

	if (m_pMain->m_stLightCtrlInfo.nType[ctrl] == LTYPE_LLIGHT_LPC_COT_232) m_pMain->SetLightBright(ctrl, ch, v == 0 ? 0 : v * 10);   // dh.jung 2021-07-12
	else																	m_pMain->SetLightBright(ctrl, ch, v);
	
	m_pMain->vt_job_info[m_nJobID].model_info.getLightInfo().setLightBright(nCam, light_sel, nIndex, v);
	m_sliderLightSetting.SetPos(v);

	m_pMain->vt_job_info[m_nJobID].model_info.getLightInfo().lightBrightSaveFlag(FALSE);
}

void CCamLightControlDlg::OnEnSetfocusEditExposureSetting()
{
	GetDlgItem(IDC_LB_EXPOSURETIME)->SetFocus();

	CString str, strTemp;

	m_edtExposureTime.GetWindowTextA(str);
	strTemp = m_pMain->GetNumberBox(str, 5, 25, 10000000);
	m_edtExposureTime.SetWindowTextA(strTemp);

	int v = atoi(strTemp);

	int nCam = m_nCam;
	int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];
#ifdef _DAHUHA
	if (m_pMain->m_pDahuhaCam[real_cam].isconnected())		m_pMain->m_pDahuhaCam[real_cam].setExposureTime(double(v));
#else
	if (m_pMain->m_pBaslerCam != NULL && m_pMain->m_pBaslerCam[real_cam])  // 20210927 Tkyuha 예외처리
		m_pMain->m_pBaslerCam[real_cam]->SetExposure(v);
#endif
	// save not ExposureTime value.
	// m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().setExposureTime(real_cam, v);
	m_bExposureTimeChange[real_cam] = TRUE;

	m_sliderExposureTime.SetPos(v);
}

void CCamLightControlDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (nSBCode == SB_ENDSCROLL)		return;

	if (pScrollBar)
	{
		// 어떤 슬라이더인지 검사
		if (pScrollBar == (CScrollBar*)&m_sliderLightSetting)
		{
			int nCam = m_nCam;
		
			CString str;
			int value = m_sliderLightSetting.GetPos();

			str.Format("%d", nPos);
			m_edtLightValue.SetWindowTextA(str);

			if (m_pMain->vt_job_info[m_nJobID].light_info[m_nCam].num_of_using_light <= 0) return; // Tkyuha 20220125 조명 예외 처리

			int light_sel = m_cmbLightSelect.GetCurSel();
			int light_nIndex = m_cmbLightIndex.GetCurSel();
			if (m_pMain->vt_job_info[m_nJobID].light_info[nCam].num_of_using_light <= light_sel) return; // Tkyuha 20220125 조명 예외 처리
			int ctrl = m_pMain->vt_job_info[m_nJobID].light_info[nCam].controller_id[light_sel] - 1;
			int ch = m_pMain->vt_job_info[m_nJobID].light_info[nCam].channel_id[light_sel];
	
			if (m_pMain->m_stLightCtrlInfo.nType[nCam] == LTYPE_LLIGHT_LPC_COT_232)
			{
				m_pMain->SetLightBright(ctrl, ch, value == 0 ? 0 : value * 10);
			}
			else
			{
				m_pMain->SetLightBright(ctrl, ch, value);
			}

			m_pMain->vt_job_info[m_nJobID].model_info.getLightInfo().setLightBright(nCam, light_sel, light_nIndex, value);

			// dh.jung 2021-05-26 add light svae flag
			m_pMain->vt_job_info[m_nJobID].model_info.getLightInfo().lightBrightSaveFlag(FALSE);
		}

		if (pScrollBar == (CScrollBar*)&m_sliderExposureTime)
		{
			CString str;
			int nCam = m_nCam;
			int nPos = m_sliderExposureTime.GetPos();

			str.Format("%d", nPos);
			m_edtExposureTime.SetWindowTextA(str);
		
			int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];
#ifdef _DAHUHA
			if (m_pMain->m_pDahuhaCam[real_cam].isconnected())		m_pMain->m_pDahuhaCam[real_cam].setExposureTime(double(nPos));
#else
			if (m_pMain->m_pBaslerCam[real_cam])
				m_pMain->m_pBaslerCam[real_cam]->SetExposure(nPos);				
#endif
			m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().setExposureTime(real_cam, nPos);
			//KJH 2022-04-26 Exposure Restore 기능 추가
			m_bExposureTimeChange[real_cam] = TRUE;
		}
	}
	else
	{
	}

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CCamLightControlDlg::updateFrameDialog(int nJob, int nCam)
{
	CString str;

	((CComboBox*)GetDlgItem(IDC_CB_SELECT_LIGHT_CHANNEL))->ResetContent();
	((CComboBox*)GetDlgItem(IDC_CB_LIGHT_INDEX))->ResetContent();

	m_nJobID = nJob;
	m_nCam = nCam;

	for (int nIndex = 0; nIndex < m_pMain->vt_job_info[nJob].light_info[0].num_of_using_light; nIndex++)
	{
		str.Format("%s", m_pMain->vt_job_info[nJob].light_info[0].channel_name[nIndex].c_str());
		((CComboBox*)GetDlgItem(IDC_CB_SELECT_LIGHT_CHANNEL))->AddString(str);
	}

	for (int nIndex = 0; nIndex < 4; nIndex++)
		((CComboBox*)GetDlgItem(IDC_CB_LIGHT_INDEX))->AddString(m_pMain->vt_job_info[nJob].light_info[0].index_name[nIndex].c_str());

	str.Format("%d", m_pMain->vt_job_info[nJob].model_info.getLightInfo().getLightBright(nCam, 0, 0));
	m_edtLightValue.SetWindowTextA(str);

	int _nCam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];

	str.Format("%d", m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getExposureTime(_nCam));
	m_edtExposureTime.SetWindowTextA(str);

	m_sliderExposureTime.SetPos(m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getExposureTime(_nCam));
	m_sliderLightSetting.SetPos(m_pMain->vt_job_info[nJob].model_info.getLightInfo().getLightBright(nCam, 0, 0));

	((CComboBox*)GetDlgItem(IDC_CB_LIGHT_INDEX))->SetCurSel(0);
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_LIGHT_CHANNEL))->SetCurSel(0);

	UpdateData(FALSE);
	
}