// CPFScaleCalc.cpp: 구현 파일
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "CPFScaleCalc.h"
#include "afxdialogex.h"


// CPFScaleCalc 대화 상자

IMPLEMENT_DYNAMIC(CPFScaleCalc, CDialogEx)

CPFScaleCalc::CPFScaleCalc(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_PF_SCALE_CALC, pParent)
{
	m_bCalcCheck = FALSE;
}

CPFScaleCalc::~CPFScaleCalc()
{
}

void CPFScaleCalc::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_STATIC_INSP_LX,	m_stt_Insp_LX);
	DDX_Control(pDX, IDC_STATIC_INSP_LY,	m_stt_Insp_LY);
	DDX_Control(pDX, IDC_STATIC_INSP_RX,	m_stt_Insp_RX);
	DDX_Control(pDX, IDC_STATIC_INSP_RY,	m_stt_Insp_RY);
	DDX_Control(pDX, IDC_STATIC_SCOPE_LX,	m_stt_Scope_LX);
	DDX_Control(pDX, IDC_STATIC_SCOPE_LY,	m_stt_Scope_LY);
	DDX_Control(pDX, IDC_STATIC_SCOPE_RX,	m_stt_Scope_RX);
	DDX_Control(pDX, IDC_STATIC_SCOPE_RY,	m_stt_Scope_RY);
	DDX_Control(pDX, IDC_STATIC_SCALE_LX,	m_stt_Scale_LX);
	DDX_Control(pDX, IDC_STATIC_SCALE_LY,	m_stt_Scale_LY);
	DDX_Control(pDX, IDC_STATIC_SCALE_RX,	m_stt_Scale_RX);
	DDX_Control(pDX, IDC_STATIC_SCALE_RY,	m_stt_Scale_RY);

	DDX_Control(pDX, IDC_EDIT_INSP_LX,		m_edt_Insp_LX);
	DDX_Control(pDX, IDC_EDIT_INSP_LY,		m_edt_Insp_LY);
	DDX_Control(pDX, IDC_EDIT_INSP_RX,		m_edt_Insp_RX);
	DDX_Control(pDX, IDC_EDIT_INSP_RY,		m_edt_Insp_RY);
	DDX_Control(pDX, IDC_EDIT_SCOPE_LX,		m_edt_Scope_LX);
	DDX_Control(pDX, IDC_EDIT_SCOPE_LY,		m_edt_Scope_LY);
	DDX_Control(pDX, IDC_EDIT_SCOPE_RX,		m_edt_Scope_RX);
	DDX_Control(pDX, IDC_EDIT_SCOPE_RY,		m_edt_Scope_RY);
	DDX_Control(pDX, IDC_EDIT_SCALE_LX,		m_edt_Scale_LX);
	DDX_Control(pDX, IDC_EDIT_SCALE_LY,		m_edt_Scale_LY);
	DDX_Control(pDX, IDC_EDIT_SCALE_RX,		m_edt_Scale_RX);
	DDX_Control(pDX, IDC_EDIT_SCALE_RY,		m_edt_Scale_RY);
	DDX_Control(pDX, IDC_BTN_CALC,			m_btnCalc);
	DDX_Control(pDX, IDC_BTN_SAVE,			m_btnSave);
	DDX_Control(pDX, IDC_CHECK1,			m_chkATTACH1);
	DDX_Control(pDX, IDC_CHECK2,			m_chkATTACH2);
}

BEGIN_MESSAGE_MAP(CPFScaleCalc, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_CALC,		&CPFScaleCalc::OnBnClickedCalc)
	ON_BN_CLICKED(IDC_BTN_SAVE,		&CPFScaleCalc::OnBnClickedSave)
	ON_EN_SETFOCUS(IDC_EDIT_INSP_LX, &CPFScaleCalc::OnEnSetfocusEditInspLx)
	ON_EN_SETFOCUS(IDC_EDIT_INSP_LY, &CPFScaleCalc::OnEnSetfocusEditInspLy)
	ON_EN_SETFOCUS(IDC_EDIT_INSP_RX, &CPFScaleCalc::OnEnSetfocusEditInspRx)
	ON_EN_SETFOCUS(IDC_EDIT_INSP_RY, &CPFScaleCalc::OnEnSetfocusEditInspRy)
	ON_EN_SETFOCUS(IDC_EDIT_SCOPE_LX, &CPFScaleCalc::OnEnSetfocusEditScopeLx)
	ON_EN_SETFOCUS(IDC_EDIT_SCOPE_LY, &CPFScaleCalc::OnEnSetfocusEditScopeLy)
	ON_EN_SETFOCUS(IDC_EDIT_SCOPE_RX, &CPFScaleCalc::OnEnSetfocusEditScopeRx)
	ON_EN_SETFOCUS(IDC_EDIT_SCOPE_RY, &CPFScaleCalc::OnEnSetfocusEditScopeRy)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CPFScaleCalc 메시지 처리기


void CPFScaleCalc::OnBnClickedCalc()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString strTemp;

	GetDlgItem(IDC_EDIT_INSP_LX)->GetWindowTextA(strTemp);
	Insp_LX = atof(strTemp);
	GetDlgItem(IDC_EDIT_INSP_LY)->GetWindowTextA(strTemp);
	Insp_LY = atof(strTemp);
	GetDlgItem(IDC_EDIT_INSP_RX)->GetWindowTextA(strTemp);
	Insp_RX = atof(strTemp);
	GetDlgItem(IDC_EDIT_INSP_RY)->GetWindowTextA(strTemp);
	Insp_RY = atof(strTemp);

	GetDlgItem(IDC_EDIT_SCOPE_LX)->GetWindowTextA(strTemp);
	Scope_LX = atof(strTemp);
	GetDlgItem(IDC_EDIT_SCOPE_LY)->GetWindowTextA(strTemp);
	Scope_LY = atof(strTemp);
	GetDlgItem(IDC_EDIT_SCOPE_RX)->GetWindowTextA(strTemp);
	Scope_RX = atof(strTemp);
	GetDlgItem(IDC_EDIT_SCOPE_RY)->GetWindowTextA(strTemp);
	Scope_RY = atof(strTemp);

	Scale_LX = Scope_LX / Insp_LX;
	strTemp.Format("%.4f", Scale_LX);
	GetDlgItem(IDC_EDIT_SCALE_LX)->SetWindowTextA(strTemp);
	Scale_LY = Scope_LY / Insp_LY;
	strTemp.Format("%.4f", Scale_LY);
	GetDlgItem(IDC_EDIT_SCALE_LY)->SetWindowTextA(strTemp);
	Scale_RX = Scope_RX / Insp_RX;
	strTemp.Format("%.4f", Scale_RX);
	GetDlgItem(IDC_EDIT_SCALE_RX)->SetWindowTextA(strTemp);
	Scale_RY = Scope_RY / Insp_RY;
	strTemp.Format("%.4f", Scale_RY);
	GetDlgItem(IDC_EDIT_SCALE_RY)->SetWindowTextA(strTemp);

	if (Scale_LX > 0 && Scale_LY > 0 && Scale_RX > 0 && Scale_RY > 0)
	{
		m_bCalcCheck = TRUE;
	}
}

void CPFScaleCalc::OnBnClickedSave()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	if (!m_bCalcCheck)
	{
		AfxMessageBox(_T("Can't Save , Need Scale Calc first"));
		return;
	}
	else if (Scale_LX < 0 && Scale_LY < 0 && Scale_RX < 0 && Scale_RY < 0)
	{
		AfxMessageBox(_T("Can't Save , Check Scale Calc Value"));
		return;
	}

	BOOL bCheck1 = m_chkATTACH1.GetCheck();
	BOOL bCheck2 = m_chkATTACH2.GetCheck();

	if (!bCheck1 && !bCheck2)
	{
		AfxMessageBox(_T("Can't Save , Need select Job Name"));
		return;
	}

	if (bCheck1)
	{
		int m_nJob = 0;
		m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmLXScale(Scale_LX);
		m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmLYScale(Scale_LY);
		m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmRXScale(Scale_RX);
		m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmRYScale(Scale_RY);

		CString strData;

		CString strModelFilePath = m_pMain->m_strCurrentModelPath + m_pMain->vt_job_info[m_nJob].job_name.c_str() + "\\InspSpecPara.ini";

		strData.Format("%.4f", m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmLXScale());
		::WritePrivateProfileStringA("INSP_PARAMETER_INFO", "FILM_LX_SCALE", strData, strModelFilePath);
		strData.Format("%.4f", m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmRXScale());
		::WritePrivateProfileStringA("INSP_PARAMETER_INFO", "FILM_RX_SCALE", strData, strModelFilePath);
		strData.Format("%.4f", m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmLYScale());
		::WritePrivateProfileStringA("INSP_PARAMETER_INFO", "FILM_LY_SCALE", strData, strModelFilePath);
		strData.Format("%.4f", m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmRYScale());
		::WritePrivateProfileStringA("INSP_PARAMETER_INFO", "FILM_RY_SCALE", strData, strModelFilePath);
		
		CFormModelView* pModelView = (CFormModelView*)m_pMain->m_pForm[FORM_MODEL];
		TabModelAlgoPage* pModelPage = (TabModelAlgoPage*)pModelView->c_TabModelPage[m_nJob];

		_variant_t var = pModelPage->m_pInspectionDlg->m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(10)->GetValue();
		var.dblVal = Scale_LX;
		pModelPage->m_pInspectionDlg->m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(10)->SetValue(var);

		var = pModelPage->m_pInspectionDlg->m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(11)->GetValue();
		var.dblVal = Scale_RX;
		pModelPage->m_pInspectionDlg->m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(11)->SetValue(var);

		var = pModelPage->m_pInspectionDlg->m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(12)->GetValue();
		var.dblVal = Scale_LY;
		pModelPage->m_pInspectionDlg->m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(12)->SetValue(var);

		var = pModelPage->m_pInspectionDlg->m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(13)->GetValue();
		var.dblVal = Scale_RY;
		pModelPage->m_pInspectionDlg->m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(13)->SetValue(var);
	}

	if (bCheck2)
	{
		int m_nJob = 1;
		m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmLXScale(Scale_LX);
		m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmLYScale(Scale_LY);
		m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmRXScale(Scale_RX);
		m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().setFilmRYScale(Scale_RY);

		CString strData;

		CString strModelFilePath = m_pMain->m_strCurrentModelPath + m_pMain->vt_job_info[m_nJob].job_name.c_str() + "\\InspSpecPara.ini";

		strData.Format("%.4f", m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmLXScale());
		::WritePrivateProfileStringA("INSP_PARAMETER_INFO", "FILM_LX_SCALE", strData, strModelFilePath);
		strData.Format("%.4f", m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmRXScale());
		::WritePrivateProfileStringA("INSP_PARAMETER_INFO", "FILM_RX_SCALE", strData, strModelFilePath);
		strData.Format("%.4f", m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmLYScale());
		::WritePrivateProfileStringA("INSP_PARAMETER_INFO", "FILM_LY_SCALE", strData, strModelFilePath);
		strData.Format("%.4f", m_pMain->vt_job_info[m_nJob].model_info.getInspSpecParaInfo().getFilmRYScale());
		::WritePrivateProfileStringA("INSP_PARAMETER_INFO", "FILM_RY_SCALE", strData, strModelFilePath);
		
		CFormModelView* pModelView = (CFormModelView*)m_pMain->m_pForm[FORM_MODEL];
		TabModelAlgoPage* pModelPage = (TabModelAlgoPage*)pModelView->c_TabModelPage[m_nJob];

		_variant_t var = pModelPage->m_pInspectionDlg->m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(10)->GetValue();
		var.dblVal = Scale_LX;
		pModelPage->m_pInspectionDlg->m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(10)->SetValue(var);

		var = pModelPage->m_pInspectionDlg->m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(11)->GetValue();
		var.dblVal = Scale_RX;
		pModelPage->m_pInspectionDlg->m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(11)->SetValue(var);

		var = pModelPage->m_pInspectionDlg->m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(12)->GetValue();
		var.dblVal = Scale_LY;
		pModelPage->m_pInspectionDlg->m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(12)->SetValue(var);

		var = pModelPage->m_pInspectionDlg->m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(13)->GetValue();
		var.dblVal = Scale_RY;
		pModelPage->m_pInspectionDlg->m_ctlPropGrid.GetProperty(0)->GetSubItem(1)->GetSubItem(13)->SetValue(var);
	}
}

void CPFScaleCalc::InitTitle(CLabelEx* pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}

void CPFScaleCalc::MainButtonInit(CButtonEx* pbutton, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}

void CPFScaleCalc::EditButtonInit(CEditEx* pbutton, int size, COLORREF color)
{
	pbutton->SetSizeText(size);				// 글자 크기
	pbutton->SetStyleTextBold(true);		// 글자 스타일
	pbutton->SetTextMargins(10, 10);		// 글자 옵셋
	pbutton->SetColorText(color);			// 글자 색상
	pbutton->SetText(_T("0"));				// 글자 설정
}

BOOL CPFScaleCalc::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);
	m_pMain = (CLET_AlignClientDlg*)AfxGetMainWnd();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	InitTitle(&m_stt_Insp_LX,	"Insp LX", 14.f, RGB(64, 64, 64));
	InitTitle(&m_stt_Insp_LY,	"Insp Ly", 14.f, RGB(64, 64, 64));
	InitTitle(&m_stt_Insp_RX,	"Insp RX", 14.f, RGB(64, 64, 64));
	InitTitle(&m_stt_Insp_RY,	"Insp RY", 14.f, RGB(64, 64, 64));

	InitTitle(&m_stt_Scope_LX,	"Scope LX", 14.f, RGB(64, 64, 64));
	InitTitle(&m_stt_Scope_LY,	"Scope LY", 14.f, RGB(64, 64, 64));
	InitTitle(&m_stt_Scope_RX,	"Scope RX", 14.f, RGB(64, 64, 64));
	InitTitle(&m_stt_Scope_RY,	"Scope RY", 14.f, RGB(64, 64, 64));

	InitTitle(&m_stt_Scale_LX,	"Scale LX", 14.f, RGB(64, 64, 64));
	InitTitle(&m_stt_Scale_LY,	"Scale LY", 14.f, RGB(64, 64, 64));
	InitTitle(&m_stt_Scale_RX,	"Scale RX", 14.f, RGB(64, 64, 64));
	InitTitle(&m_stt_Scale_RY,	"Scale RY", 14.f, RGB(64, 64, 64));

	GetDlgItem(IDC_EDIT_INSP_LX)->SetWindowTextA("0.0");
	GetDlgItem(IDC_EDIT_INSP_LY)->SetWindowTextA("0.0");
	GetDlgItem(IDC_EDIT_INSP_RX)->SetWindowTextA("0.0");
	GetDlgItem(IDC_EDIT_INSP_RY)->SetWindowTextA("0.0");
	GetDlgItem(IDC_EDIT_SCOPE_LX)->SetWindowTextA("0.0");
	GetDlgItem(IDC_EDIT_SCOPE_LY)->SetWindowTextA("0.0");
	GetDlgItem(IDC_EDIT_SCOPE_RX)->SetWindowTextA("0.0");
	GetDlgItem(IDC_EDIT_SCOPE_RY)->SetWindowTextA("0.0");

	MainButtonInit(&m_btnCalc);
	MainButtonInit(&m_btnSave);
	
	m_chkATTACH1.SetAlignTextCM();
	m_chkATTACH1.SetColorBkg(255, RGB(64, 64, 64));
	m_chkATTACH1.SetSizeText(12);
	m_chkATTACH1.SetSizeCheck(2);
	m_chkATTACH1.SetSizeCheckBox(3, 3, 30, 30);
	m_chkATTACH1.SetOffsetText(-5, 0);
	m_chkATTACH1.SetText(_T("ATTCH INSP 1"));
	
	m_chkATTACH2.SetAlignTextCM();
	m_chkATTACH2.SetColorBkg(255, RGB(64, 64, 64));
	m_chkATTACH2.SetSizeText(12);
	m_chkATTACH2.SetSizeCheck(2);
	m_chkATTACH2.SetSizeCheckBox(3, 3, 30, 30);
	m_chkATTACH2.SetOffsetText(-5, 0);
	m_chkATTACH2.SetText(_T("ATTCH INSP 2"));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

HBRUSH CPFScaleCalc::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	// TODO:  여기서 DC의 특성을 변경합니다.
	if (pWnd->GetDlgCtrlID() == IDC_EDIT_INSP_LX ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_INSP_LY ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_INSP_RX ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_INSP_RY ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_SCOPE_LX ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_SCOPE_LY ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_SCOPE_RX ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_SCOPE_RY ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_SCALE_LX ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_SCALE_LY ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_SCALE_RX ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_SCALE_RY
		)
	{
		pDC->SetBkColor(COLOR_UI_BODY);
		pDC->SetTextColor(COLOR_WHITE);
	}
	// TODO:  기본값이 적당하지 않으면 다른 브러시를 반환합니다.
	return m_hbrBkg;
}

void CPFScaleCalc::OnEnSetfocusEditInspLx()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GetDlgItem(IDC_STATIC_INSP_LX)->SetFocus();
	
	CString str , strTemp;
	GetDlgItem(IDC_EDIT_INSP_LX)->GetWindowTextA(str);
	strTemp = m_pMain->GetNumberBox(str, 6, 0, 10000);
	GetDlgItem(IDC_EDIT_INSP_LX)->SetWindowTextA(strTemp);
}
void CPFScaleCalc::OnEnSetfocusEditInspLy()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GetDlgItem(IDC_STATIC_INSP_LY)->SetFocus();

	CString str, strTemp;
	GetDlgItem(IDC_EDIT_INSP_LY)->GetWindowTextA(str);
	strTemp = m_pMain->GetNumberBox(str, 6, 0, 10000);
	GetDlgItem(IDC_EDIT_INSP_LY)->SetWindowTextA(strTemp);
}
void CPFScaleCalc::OnEnSetfocusEditInspRx()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GetDlgItem(IDC_STATIC_INSP_RX)->SetFocus();

	CString str, strTemp;
	GetDlgItem(IDC_EDIT_INSP_RX)->GetWindowTextA(str);
	strTemp = m_pMain->GetNumberBox(str, 6, 0, 10000);
	GetDlgItem(IDC_EDIT_INSP_RX)->SetWindowTextA(strTemp);
}
void CPFScaleCalc::OnEnSetfocusEditInspRy()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GetDlgItem(IDC_STATIC_INSP_RY)->SetFocus();

	CString str, strTemp;
	GetDlgItem(IDC_EDIT_INSP_RY)->GetWindowTextA(str);
	strTemp = m_pMain->GetNumberBox(str, 6, 0, 10000);
	GetDlgItem(IDC_EDIT_INSP_RY)->SetWindowTextA(strTemp);
}
void CPFScaleCalc::OnEnSetfocusEditScopeLx()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GetDlgItem(IDC_STATIC_SCOPE_LX)->SetFocus();

	CString str, strTemp;
	GetDlgItem(IDC_EDIT_SCOPE_LX)->GetWindowTextA(str);
	strTemp = m_pMain->GetNumberBox(str, 6, 0, 10000);
	GetDlgItem(IDC_EDIT_SCOPE_LX)->SetWindowTextA(strTemp);
}
void CPFScaleCalc::OnEnSetfocusEditScopeLy()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GetDlgItem(IDC_STATIC_SCOPE_LY)->SetFocus();

	CString str, strTemp;
	GetDlgItem(IDC_EDIT_SCOPE_LY)->GetWindowTextA(str);
	strTemp = m_pMain->GetNumberBox(str, 6, 0, 10000);
	GetDlgItem(IDC_EDIT_SCOPE_LY)->SetWindowTextA(strTemp);
}
void CPFScaleCalc::OnEnSetfocusEditScopeRx()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GetDlgItem(IDC_STATIC_SCOPE_RX)->SetFocus();

	CString str, strTemp;
	GetDlgItem(IDC_EDIT_SCOPE_RX)->GetWindowTextA(str);
	strTemp = m_pMain->GetNumberBox(str, 6, 0, 10000);
	GetDlgItem(IDC_EDIT_SCOPE_RX)->SetWindowTextA(strTemp);
}
void CPFScaleCalc::OnEnSetfocusEditScopeRy()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GetDlgItem(IDC_STATIC_SCOPE_RY)->SetFocus();

	CString str, strTemp;
	GetDlgItem(IDC_EDIT_SCOPE_RY)->GetWindowTextA(str);
	strTemp = m_pMain->GetNumberBox(str, 6, 0, 10000);
	GetDlgItem(IDC_EDIT_SCOPE_RY)->SetWindowTextA(strTemp);
}
