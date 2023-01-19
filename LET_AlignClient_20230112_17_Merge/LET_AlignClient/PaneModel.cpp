// PaneModel.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "PaneModel.h"
#include "LET_AlignClientDlg.h"
#include "DlgCreateModel.h"
#include "DlgINISetting.h"

// CPaneModel

IMPLEMENT_DYNCREATE(CPaneModel, CFormView)

CPaneModel::CPaneModel()	: CFormView(CPaneModel::IDD)
{

}

CPaneModel::~CPaneModel()
{
}

void CPaneModel::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_BTN_MODEL_CLOSE, m_btnModelClose);
	DDX_Control(pDX, IDC_BTN_CREATE_MODEL, m_btnCreateModel);
	DDX_Control(pDX, IDC_BTN_DELETE_MODEL, m_btnDeleteModel);
	DDX_Control(pDX, IDC_BTN_SYSTEM_OPTION, m_btnSystemOption);
}

BEGIN_MESSAGE_MAP(CPaneModel, CFormView)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_MODEL_CLOSE, &CPaneModel::OnBnClickedBtnModelClose)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BTN_CREATE_MODEL, &CPaneModel::OnBnClickedBtnCreateModel)
	ON_BN_CLICKED(IDC_BTN_DELETE_MODEL, &CPaneModel::OnBnClickedBtnDeleteModel)
	ON_BN_CLICKED(IDC_BTN_SYSTEM_OPTION, &CPaneModel::OnBnClickedBtnSystemOption)
END_MESSAGE_MAP()


// CPaneModel 진단입니다.

#ifdef _DEBUG
void CPaneModel::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPaneModel::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

void CPaneModel::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);
	m_pMain = (CLET_AlignClientDlg*)AfxGetMainWnd();

	GetClientRect(&m_rcForm);

	MainButtonInit(&m_btnCreateModel);
	MainButtonInit(&m_btnDeleteModel);
	MainButtonInit(&m_btnSystemOption);
	MainButtonInit(&m_btnModelClose);
}

HBRUSH CPaneModel::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	return m_hbrBkg;
}

void CPaneModel::InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}


void CPaneModel::MainButtonInit(CButtonEx *pbutton, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}


void CPaneModel::OnBnClickedBtnModelClose()
{
	m_pMain->changeForm(FORM_MAIN);
	m_pMain->changePane(PANE_MANUAL);
}


void CPaneModel::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 그리기 메시지에 대해서는 CFormView::OnPaint()을(를) 호출하지 마십시오.

	CPen pen(PS_SOLID, 3, COLOR_DDARK_GRAY), *pOld;

	pOld = dc.SelectObject(&pen);
	dc.MoveTo(m_rcForm.left, m_rcForm.top);
	dc.LineTo(m_rcForm.right, m_rcForm.top);
	dc.LineTo(m_rcForm.right, m_rcForm.bottom);
	dc.LineTo(m_rcForm.left, m_rcForm.bottom);
	dc.LineTo(m_rcForm.left, m_rcForm.top);
	dc.SelectObject(pOld);
}


void CPaneModel::OnBnClickedBtnCreateModel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	if (m_pMain->fnSetMessage(2, "Do You Create the Model?") != TRUE)	return;

	CDlgCreateModel dlg;

	//m_pMain->m_pKeyboardDlg->ShowWindow(SW_HIDE);

	if (dlg.DoModal() == IDOK)
	{
		m_pMain->process_model_copy(m_pMain->m_strCurrentModelName, dlg.m_strEditModel);
	}
	else
	{
		m_pMain->fnSetMessage(1, "Cancelled..!!");
	}

	::SendMessageA(m_pMain->m_pForm[FORM_MODEL]->m_hWnd, WM_VIEW_CONTROL, MSG_MV_DISP_MODEL_LIST, 0);

}

void CPaneModel::OnBnClickedBtnDeleteModel()
{
	BOOL bPass = FALSE;
	bPass = m_pMain->UsePassword();

	if (bPass != TRUE) {
		return;
	}

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_pMain->getDispModelName() == "")
	{
		m_pMain->fnSetMessage(1, "Select the Model to Delete");
		return;
	}

	if (m_pMain->getDispModelName() == m_pMain->m_strCurrentModelName)
	{
		m_pMain->fnSetMessage(1, "Cannot Delete Current Model");
		return;
	}

	BOOL bRet = m_pMain->fnSetMessage(2, "Do You Delete '" + m_pMain->getDispModelName() + "' ?");

	if (bRet != TRUE) return;

	CString strDeleteModel = m_pMain->m_strModelDir + m_pMain->getDispModelName();

	if (_access(strDeleteModel, 0) != 0)
	{
		m_pMain->fnSetMessage(1, m_pMain->getDispModelName() + ", Not Exist Model Data");
		return;
	}

	try {
		::SendMessageA(m_pMain->m_pForm[FORM_MODEL]->m_hWnd, WM_VIEW_CONTROL, MSG_MV_DELETE_MODEL, (LPARAM)&m_pMain->getDispModelName());
	}
	catch (...) {
		m_pMain->fnSetMessage(1, m_pMain->getDispModelName() + ", Not Exist Model Data");
		return;
	}
}



void CPaneModel::OnBnClickedBtnSystemOption()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BOOL bPass = FALSE;
	bPass = m_pMain->UsePassword();

	if (bPass != TRUE) {
		return;
	}

	DlgINISetting pDlg;
	pDlg.DoModal();
}



