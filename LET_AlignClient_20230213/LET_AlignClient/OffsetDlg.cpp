// COffsetDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "OffsetDlg.h"
#include "afxdialogex.h"

// COffsetDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(COffsetDlg, CDialogEx)

COffsetDlg::COffsetDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(COffsetDlg::IDD, pParent)
{
	m_str_INI_path = "";
	m_str_section = "";
	m_nJob =	0;
	m_nTarget = 0;
	m_nCam =	0;
	m_nPos =	0;
	m_bApply_Offset = FALSE;
	m_dOffset_x	  = 0.0;
	m_dTarget_x	  = 0.0;
	m_dResult_x	  = 0.0;
	m_dOffset_y	  = 0.0;
	m_dTarget_y	  = 0.0;
	m_dResult_y	  = 0.0;
}

COffsetDlg::~COffsetDlg()
{
}

void COffsetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(COffsetDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_OFFSET, &COffsetDlg::OnBnClickedButtonSaveOffset)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_SET_OFFSET, &COffsetDlg::OnBnClickedButtonSetOffset)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &COffsetDlg::OnBnClickedButtonClear)
END_MESSAGE_MAP()

BOOL COffsetDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_pMain = (CLET_AlignClientDlg*)AfxGetMainWnd();

	CString strEDIT = "0.0";
	GetDlgItem(IDC_EDIT_JOB)->SetWindowTextA("");
	GetDlgItem(IDC_EDIT_TARGET_X)->SetWindowTextA(strEDIT);
	GetDlgItem(IDC_EDIT_TARGET_Y)->SetWindowTextA(strEDIT);
	GetDlgItem(IDC_EDIT_OFFSET_X)->SetWindowTextA(strEDIT);
	GetDlgItem(IDC_EDIT_OFFSET_Y)->SetWindowTextA(strEDIT);
	GetDlgItem(IDC_EDIT_RESULT_X)->SetWindowTextA("");
	GetDlgItem(IDC_EDIT_RESULT_Y)->SetWindowTextA("");

	GetDlgItem(IDC_EDIT_JOB)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_CAM)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_POS)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_TARGET_X)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_TARGET_Y)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_RESULT_X)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_RESULT_Y)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_TARGET_ORIGIN_X)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_TARGET_ORIGIN_Y)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_OFFSET_X)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_OFFSET_Y)->EnableWindow(TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void COffsetDlg::OnBnClickedButtonSetOffset()
{
	Set_Result_Offset();
}

void COffsetDlg::OnBnClickedButtonSaveOffset()
{
	switch (m_nTarget)
	{
		case OFFSET_ROTATE_CENTER:		Save_RotateCenter_Offset();		 break;
	}
}

void COffsetDlg::Set_Offset_Target(int nTarget, int nJob, int nCam, int nPos)
{
	CString str;

	m_nTarget = nTarget;
	m_nJob = nJob;
	m_nCam = nCam;
	m_nPos = nPos;

	switch (m_nTarget)
	{
	case OFFSET_ROTATE_CENTER:
	{
		m_str_INI_path.Format("%s%s\\%s\\OffsetInfo.ini", m_pMain->m_strModelDir, m_pMain->m_strCurrentModelName, m_pMain->vt_job_info[m_nJob].job_name.c_str());

		str.Format("Rotate X"); GetDlgItem(IDC_STATIC_TARGET_X)->SetWindowTextA(str);
		str.Format("Rotate Y"); GetDlgItem(IDC_STATIC_TARGET_Y)->SetWindowTextA(str);

		m_str_section = "ROTATE_CENTER_OFFSET";
		Get_RotateCenter_Offset();
	}
	break;
	}
}

void COffsetDlg::Set_Result_Offset()
{
	CString strData;
	SYSTEMTIME Time;
	::GetLocalTime(&Time);

	// Reuslt
	if (m_dOrigin_x == 0.0 || m_dOrigin_y == 0.0)
	{
		TCHAR cData[MAX_PATH] = { 0, };
		CString strKey, strData;

		strKey.Format("ORIGIN_SET_TIME%d_%d", m_nCam + 1, m_nPos + 1); // Tkyuha 20211202
		strData.Format("%4d-%2d-%2d", Time.wYear, Time.wMonth, Time.wDay);
		::WritePrivateProfileStringA(_T(m_str_section), strKey, strData, m_str_INI_path);
	}

	if (m_dOrigin_x == 0.0)
	{
		m_dOrigin_x = m_dTarget_x;
		strData.Format("%.4f", m_dOrigin_x);	GetDlgItem(IDC_EDIT_TARGET_ORIGIN_X)->SetWindowTextA(strData);
	}
	if (m_dOrigin_y == 0.0)
	{
		m_dOrigin_y = m_dTarget_y;
		strData.Format("%.4f", m_dOrigin_y);	GetDlgItem(IDC_EDIT_TARGET_ORIGIN_Y)->SetWindowTextA(strData);
	}

	GetDlgItem(IDC_EDIT_OFFSET_X)->GetWindowTextA(strData); m_dOffset_x = atof(strData);
	GetDlgItem(IDC_EDIT_OFFSET_Y)->GetWindowTextA(strData); m_dOffset_y = atof(strData);

	m_dResult_x = m_dOrigin_x + m_dOffset_x;
	m_dResult_y = m_dOrigin_y + m_dOffset_y;

	UpdateData();
}

void COffsetDlg::UpdateData()
{
	CString strData;
	strData.Format("%s", m_pMain->vt_job_info[m_nJob].get_job_name());		GetDlgItem(IDC_EDIT_JOB)->SetWindowTextA(strData);
	strData.Format("%d", m_nCam+1);		 GetDlgItem(IDC_EDIT_CAM)->SetWindowTextA(strData);
	strData.Format("%d", m_nPos+1);		 GetDlgItem(IDC_EDIT_POS)->SetWindowTextA(strData);
	strData.Format("%.4f", m_dOrigin_x); GetDlgItem(IDC_EDIT_TARGET_ORIGIN_X)->SetWindowTextA(strData);
	strData.Format("%.4f", m_dOrigin_y); GetDlgItem(IDC_EDIT_TARGET_ORIGIN_Y)->SetWindowTextA(strData);
	strData.Format("%.4f", m_dTarget_x); GetDlgItem(IDC_EDIT_TARGET_X)->SetWindowTextA(strData);
	strData.Format("%.4f", m_dTarget_y); GetDlgItem(IDC_EDIT_TARGET_Y)->SetWindowTextA(strData);
	strData.Format("%.4f", m_dOffset_x); GetDlgItem(IDC_EDIT_OFFSET_X)->SetWindowTextA(strData);
	strData.Format("%.4f", m_dOffset_y); GetDlgItem(IDC_EDIT_OFFSET_Y)->SetWindowTextA(strData);
	strData.Format("%.4f", m_dResult_x); GetDlgItem(IDC_EDIT_RESULT_X)->SetWindowTextA(strData);
	strData.Format("%.4f", m_dResult_y); GetDlgItem(IDC_EDIT_RESULT_Y)->SetWindowTextA(strData);
}

BOOL COffsetDlg::Check_ApplyData()
{
	if (m_dOffset_x == 0 && m_dOffset_y == 0)
		m_bApply_Offset = FALSE;
	else
		m_bApply_Offset = TRUE;
	
	return m_bApply_Offset;
}

void COffsetDlg::Get_RotateCenter_Offset()
{
	TCHAR cData[MAX_PATH] = { 0, };
	CString strSection, strKey, strEDIT;

	double Rotate_X_Target = m_pMain->GetMachine(m_nJob).getRotateX(m_nCam, m_nPos);
	double Rotate_Y_Target = m_pMain->GetMachine(m_nJob).getRotateY(m_nCam, m_nPos);
	double Rotate_X_Origin = 0.0;
	double Rotate_Y_Origin = 0.0;
	double Rotate_X_Offset = 0.0;
	double Rotate_Y_Offset = 0.0;

	// Get INI file
	strKey.Format("ROTATE_X_ORIGIN_%d_%d", m_nCam + 1, m_nPos + 1);
	::GetPrivateProfileStringA(_T(m_str_section), strKey, "0.0", cData, MAX_PATH, m_str_INI_path);
	Rotate_X_Origin = atof(cData);

	strKey.Format("ROTATE_Y_ORIGIN_%d_%d", m_nCam + 1, m_nPos + 1);
	::GetPrivateProfileStringA(_T(m_str_section), strKey, "0.0", cData, MAX_PATH, m_str_INI_path);
	Rotate_Y_Origin = atof(cData);

	strKey.Format("ROTATE_X_OFFSET_%d_%d", m_nCam + 1, m_nPos + 1);
	::GetPrivateProfileStringA(_T(m_str_section), strKey, "0.0", cData, MAX_PATH, m_str_INI_path);
	Rotate_X_Offset = atof(cData);

	strKey.Format("ROTATE_Y_OFFSET_%d_%d", m_nCam + 1, m_nPos + 1);
	::GetPrivateProfileStringA(_T(m_str_section), strKey, "0.0", cData, MAX_PATH, m_str_INI_path);
	Rotate_Y_Offset = atof(cData);

	// Set Meber Value
	m_dOrigin_x = Rotate_X_Origin;
	m_dOrigin_y = Rotate_Y_Origin;
	m_dTarget_x = Rotate_X_Target;
	m_dTarget_y = Rotate_Y_Target;
	m_dOffset_x = Rotate_X_Offset;
	m_dOffset_y = Rotate_Y_Offset;

	// Result Offset
	m_dResult_x = m_dOrigin_x + m_dOffset_x;
	m_dResult_y = m_dOrigin_y + m_dOffset_y;

	CString strData;
	strData.Format("%.4f", m_dResult_x);	GetDlgItem(IDC_EDIT_RESULT_X)->SetWindowTextA(strData);
	strData.Format("%.4f", m_dResult_y);	GetDlgItem(IDC_EDIT_RESULT_Y)->SetWindowTextA(strData);

	// Edit Update
	UpdateData();
}

void COffsetDlg::Save_RotateCenter_Offset()
{
	if( m_pMain->UsePassword()==FALSE) return;

	CString strData;

	GetDlgItem(IDC_EDIT_TARGET_ORIGIN_X)->GetWindowTextA(strData); m_dOrigin_x = atof(strData);
	GetDlgItem(IDC_EDIT_TARGET_ORIGIN_Y)->GetWindowTextA(strData); m_dOrigin_y = atof(strData);
	GetDlgItem(IDC_EDIT_OFFSET_X)->GetWindowTextA(strData); m_dOffset_x = atof(strData);
	GetDlgItem(IDC_EDIT_OFFSET_Y)->GetWindowTextA(strData); m_dOffset_y = atof(strData);

	m_dResult_x = m_dOrigin_x + m_dOffset_x;
	m_dResult_y = m_dOrigin_y + m_dOffset_y;

	SYSTEMTIME Time;
	::GetLocalTime(&Time);

	// Save Offset.INI file
	TCHAR cData[MAX_PATH] = { 0, };
	CString strKey;

	strKey.Format("ROTATE_X_ORIGIN_%d_%d", m_nCam + 1, m_nPos + 1);
	strData.Format("%5.15f", m_dOrigin_x);
	::WritePrivateProfileStringA(_T(m_str_section), strKey, strData, m_str_INI_path);

	strKey.Format("ROTATE_Y_ORIGIN_%d_%d", m_nCam + 1, m_nPos + 1);
	strData.Format("%5.15f", m_dOrigin_y);
	::WritePrivateProfileStringA(_T(m_str_section), strKey, strData, m_str_INI_path);

	strKey.Format("ROTATE_X_OFFSET_%d_%d", m_nCam + 1, m_nPos + 1);
	strData.Format("%5.15f", m_dOffset_x);
	::WritePrivateProfileStringA(_T(m_str_section), strKey, strData, m_str_INI_path);

	strKey.Format("ROTATE_Y_OFFSET_%d_%d", m_nCam + 1, m_nPos + 1);
	strData.Format("%5.15f", m_dOffset_y);
	::WritePrivateProfileStringA(_T(m_str_section), strKey, strData, m_str_INI_path);

	strKey.Format("OFFSET_ROTATE_DATE_TIME%d_%d", m_nCam + 1, m_nPos + 1); 
	strData.Format("%4d-%2d-%2d", Time.wYear, Time.wMonth, Time.wDay);
	::WritePrivateProfileStringA(_T(m_str_section), strKey, strData, m_str_INI_path);

	// Save Machine.INI file
	CString str_path;
	m_pMain->vt_job_info[m_nJob].model_info.getMachineInfo().setRotateCenterX(m_nCam, m_nPos, m_dResult_x);
	m_pMain->vt_job_info[m_nJob].model_info.getMachineInfo().setRotateCenterY(m_nCam, m_nPos, m_dResult_y);
	m_pMain->GetMachine(m_nJob).setRotateX(m_nCam, m_nPos, m_dResult_x);
	m_pMain->GetMachine(m_nJob).setRotateY(m_nCam, m_nPos, m_dResult_y);

	str_path.Format("%s%s\\", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[m_nJob].job_name.c_str());
	m_pMain->GetMachine(m_nJob).saveRotateCenter(str_path, 0, 0);

	// Edit Update
	Get_RotateCenter_Offset();

	CPaneMachine* pPaneMachine = (CPaneMachine*)m_pMain->m_pPane[PANE_MACHINE];
	pPaneMachine->c_TabMachinePage[m_nJob]->updateFrameDialog();

	AfxMessageBox("Save Complete.");
}

void COffsetDlg::OnBnClickedButtonClear()
{
	if (m_pMain->UsePassword() == FALSE) return;

	SYSTEMTIME Time;
	::GetLocalTime(&Time);

	m_dOrigin_x = 0.0;
	m_dOrigin_y = 0.0;
	m_dOffset_x = 0.0;
	m_dOffset_y = 0.0;

	// Save Offset.INI file
	TCHAR cData[MAX_PATH] = { 0, };
	CString strKey, strData;

	strKey.Format("ROTATE_X_ORIGIN_%d_%d", m_nCam + 1, m_nPos + 1);
	strData.Format("%5.15f", m_dOrigin_x);
	::WritePrivateProfileStringA(_T(m_str_section), strKey, strData, m_str_INI_path);

	strKey.Format("ROTATE_Y_ORIGIN_%d_%d", m_nCam + 1, m_nPos + 1);
	strData.Format("%5.15f", m_dOrigin_y);
	::WritePrivateProfileStringA(_T(m_str_section), strKey, strData, m_str_INI_path);

	strKey.Format("ROTATE_X_OFFSET_%d_%d", m_nCam + 1, m_nPos + 1);
	strData.Format("%5.15f", m_dOffset_x);
	::WritePrivateProfileStringA(_T(m_str_section), strKey, strData, m_str_INI_path);

	strKey.Format("ROTATE_Y_OFFSET_%d_%d", m_nCam + 1, m_nPos + 1);
	strData.Format("%5.15f", m_dOffset_y);
	::WritePrivateProfileStringA(_T(m_str_section), strKey, strData, m_str_INI_path);

	strKey.Format("CLEAR_DATE_TIME%d_%d", m_nCam + 1, m_nPos + 1); 
	strData.Format("%4d-%2d-%2d", Time.wYear, Time.wMonth, Time.wDay);
	::WritePrivateProfileStringA(_T(m_str_section), strKey, strData, m_str_INI_path);

	Get_RotateCenter_Offset();

	AfxMessageBox("Clear Complete.");
}
