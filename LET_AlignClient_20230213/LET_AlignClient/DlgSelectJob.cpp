
#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "DlgSelectJob.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(CDlgSelectJob, CDialogEx)

CDlgSelectJob::CDlgSelectJob(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CDlgSelectJob::IDD, pParent)
{
	m_pMain = NULL;
	m_hbrBkg = NULL;
}

CDlgSelectJob::~CDlgSelectJob()
{
}

void CDlgSelectJob::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_JOB_LIST, m_cmbJob);
	DDX_Control(pDX, IDC_BTN_SELECT, m_btnSelect);
	DDX_Control(pDX, IDC_LB_SELECET, m_lbSelect);
}


BEGIN_MESSAGE_MAP(CDlgSelectJob, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_SELECT, &CDlgSelectJob::OnBnClickedBtnSelect)
END_MESSAGE_MAP()


void CDlgSelectJob::InitTitle(CLabelEx* pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);

	if (color == RGB(255, 255, 255)) pTitle->SetColorText(255, 0, 0, 0);
}

void CDlgSelectJob::MainButtonInit(CButtonEx* pbutton, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}


HBRUSH CDlgSelectJob::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (pWnd->GetDlgCtrlID() == IDC_COMBO_JOB_LIST)
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

BOOL CDlgSelectJob::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);

	m_pMain = (CLET_AlignClientDlg*)AfxGetMainWnd();
	MainButtonInit(&m_btnSelect);
	InitTitle(&m_lbSelect, _T("Select Job"), 14.f, COLOR_BTN_BODY);

	for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
	{
		CString sJobName;
		sJobName.Format("%s", m_pMain->vt_job_info[i].get_job_name());

		m_cmbJob.AddString(sJobName);
	}

	m_cmbJob.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgSelectJob::OnBnClickedBtnSelect()
{
	CDlgList DlgList;
	int iSelect = m_cmbJob.GetCurSel();

	m_pMain->m_pListDlg->ShowWindow(SW_SHOW);
	m_pMain->m_pListDlg->update_list_align(iSelect);
	ShowWindow(SW_HIDE);
}
