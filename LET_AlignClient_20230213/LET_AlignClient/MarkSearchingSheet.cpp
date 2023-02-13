// MarkSearchingSheet.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "MarkSearchingSheet.h"
#include "afxdialogex.h"


template <typename T>
std::string NumberToString(T Number)
{
	std::ostringstream ss;
	ss << Number;
	return ss.str();
}


template <class T>
bool from_string(T& t, const std::string& s, std::ios_base& (*f)(std::ios_base&))
{
	std::istringstream iss(s);
	return !(iss >> f >> t).fail();
}
// CMarkSearchingSheet 대화 상자입니다.

IMPLEMENT_DYNAMIC(CMarkSearchingSheet, CDialogEx)

CMarkSearchingSheet::CMarkSearchingSheet(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMarkSearchingSheet::IDD, pParent)
	, m_edtNumLevel(0)
	, m_edtSangle(0)
	, m_edtEangle(0)
	, m_edtStepAngle(0)
	, m_edtContrast(0)
	, m_edtMinContrast(0)
{
	m_pParentDlg = NULL;

}

CMarkSearchingSheet::~CMarkSearchingSheet()
{
}

void CMarkSearchingSheet::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_NUMLEVEL, m_edtNumLevel);
	DDX_Text(pDX, IDC_EDIT_S_ANGLE, m_edtSangle);
	DDX_Text(pDX, IDC_EDIT_E_ANGLE, m_edtEangle);
	DDX_Text(pDX, IDC_EDIT_STEP_ANGLE, m_edtStepAngle);
	DDX_Text(pDX, IDC_EDIT_CONTRAST, m_edtContrast);
	DDX_Text(pDX, IDC_EDIT_CONTRAST2, m_edtMinContrast);
	DDX_Control(pDX, IDC_CB_SELECT_OPTIMIZE, m_cmbOptimize);
	DDX_Control(pDX, IDC_CB_SELECT_METRIC, m_cmbMetric);
}


BEGIN_MESSAGE_MAP(CMarkSearchingSheet, CDialogEx)
	ON_STN_CLICKED(IDC_EDIT_NUMLEVEL, &CMarkSearchingSheet::OnStnClickedEditNumlevel)
	ON_WM_LBUTTONUP()
	ON_STN_CLICKED(IDC_EDIT_S_ANGLE, &CMarkSearchingSheet::OnStnClickedEditSAngle)
	ON_STN_CLICKED(IDC_EDIT_E_ANGLE, &CMarkSearchingSheet::OnStnClickedEditEAngle)
	ON_STN_CLICKED(IDC_EDIT_STEP_ANGLE, &CMarkSearchingSheet::OnStnClickedEditStepAngle)
	ON_STN_CLICKED(IDC_EDIT_CONTRAST, &CMarkSearchingSheet::OnStnClickedEditContrast)
	ON_STN_CLICKED(IDC_EDIT_CONTRAST2, &CMarkSearchingSheet::OnStnClickedEditContrast2)
END_MESSAGE_MAP()


// CMarkSearchingSheet 메시지 처리기입니다.


BOOL CMarkSearchingSheet::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_cmbOptimize.SetDroppedWidth(m_cmbOptimize.GetDroppedWidth() + 80);
	m_cmbMetric.SetDroppedWidth(m_cmbMetric.GetDroppedWidth() + 80);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CMarkSearchingSheet::OnStnClickedEditNumlevel()
{
	CString str, strTemp;

	str.Format("%d", m_edtNumLevel);
	strTemp = theApp.m_pFrame->GetNumberBox(str, 5, 0, 255);

	from_string<int>(m_edtNumLevel, std::string(strTemp), std::dec);

	UpdateData(FALSE);
}


void CMarkSearchingSheet::OnStnClickedEditSAngle()
{
	CString str, strTemp;

	str.Format("%4.2f", m_edtSangle);
	strTemp = theApp.m_pFrame->GetNumberBox(str, 5, -360, 360);

	from_string<double>(m_edtSangle, std::string(strTemp), std::dec);

	UpdateData(FALSE);
}


void CMarkSearchingSheet::OnStnClickedEditEAngle()
{
	CString str, strTemp;

	str.Format("%4.2f", m_edtEangle);
	strTemp = theApp.m_pFrame->GetNumberBox(str, 5, -360, 360);

	from_string<double>(m_edtEangle, std::string(strTemp), std::dec);

	UpdateData(FALSE);
}


void CMarkSearchingSheet::OnStnClickedEditStepAngle()
{
	CString str, strTemp;

	str.Format("%4.2f", m_edtStepAngle);
	strTemp = theApp.m_pFrame->GetNumberBox(str, 5, -255, 255);

	from_string<double>(m_edtStepAngle, std::string(strTemp), std::dec);

	UpdateData(FALSE);
}


void CMarkSearchingSheet::OnStnClickedEditContrast()
{
	CString str, strTemp;

	str.Format("%4.2f", m_edtContrast);
	strTemp = theApp.m_pFrame->GetNumberBox(str, 5, -255, 255);

	from_string<double>(m_edtContrast, std::string(strTemp), std::dec);

	UpdateData(FALSE);
}

void CMarkSearchingSheet::OnStnClickedEditContrast2()
{
	CString str, strTemp;

	str.Format("%4.2f", m_edtMinContrast);
	strTemp = theApp.m_pFrame->GetNumberBox(str, 5, -255, 255);

	from_string<double>(m_edtMinContrast, std::string(strTemp), std::dec);

	UpdateData(FALSE);
}


void CMarkSearchingSheet::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (IsClickNumLevelTitle(point) == TRUE)
	{
		OnStnClickedEditNumlevel();
	}
	else if (IsClickSangleTitle(point) == TRUE)
	{
		OnStnClickedEditSAngle();
	}
	else if (IsClickEangleTitle(point) == TRUE)
	{
		OnStnClickedEditEAngle();
	}
	else if (IsClickStepAngleTitle(point) == TRUE)
	{
		OnStnClickedEditStepAngle();
	}
	else if (IsClickContrastTitle(point) == TRUE)
	{
		OnStnClickedEditContrast();
	}
	else if (IsClickMinContrastTitle(point) == TRUE)
	{
		OnStnClickedEditContrast2();
	}

	CDialogEx::OnLButtonUp(nFlags, point);
}


BOOL CMarkSearchingSheet::IsClickNumLevelTitle(CPoint pt)
{
	CRect IdRect, rect;
	GetWindowRect(&rect);
	GetDlgItem(IDC_EDIT_NUMLEVEL)->GetWindowRect(&IdRect);

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;	IdRect.bottom = IdRect.top + h;

	if (pt.x >= IdRect.left && pt.x <= IdRect.right && pt.y >= IdRect.top && pt.y <= IdRect.bottom)		return TRUE;
	else		return FALSE;
}

BOOL CMarkSearchingSheet::IsClickSangleTitle(CPoint pt)
{
	CRect IdRect, rect;
	GetWindowRect(&rect);
	GetDlgItem(IDC_EDIT_S_ANGLE)->GetWindowRect(&IdRect);

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;	IdRect.bottom = IdRect.top + h;

	if (pt.x >= IdRect.left && pt.x <= IdRect.right && pt.y >= IdRect.top && pt.y <= IdRect.bottom)		return TRUE;
	else		return FALSE;
}

BOOL CMarkSearchingSheet::IsClickEangleTitle(CPoint pt)
{
	CRect IdRect, rect;
	GetWindowRect(&rect);
	GetDlgItem(IDC_EDIT_E_ANGLE)->GetWindowRect(&IdRect);

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;	IdRect.bottom = IdRect.top + h;

	if (pt.x >= IdRect.left && pt.x <= IdRect.right && pt.y >= IdRect.top && pt.y <= IdRect.bottom)		return TRUE;
	else		return FALSE;
}


BOOL CMarkSearchingSheet::IsClickStepAngleTitle(CPoint pt)
{
	CRect IdRect, rect;
	GetWindowRect(&rect);
	GetDlgItem(IDC_EDIT_STEP_ANGLE)->GetWindowRect(&IdRect);

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;	IdRect.bottom = IdRect.top + h;

	if (pt.x >= IdRect.left && pt.x <= IdRect.right && pt.y >= IdRect.top && pt.y <= IdRect.bottom)		return TRUE;
	else		return FALSE;
}


BOOL CMarkSearchingSheet::IsClickContrastTitle(CPoint pt)
{
	CRect IdRect, rect;
	GetWindowRect(&rect);
	GetDlgItem(IDC_EDIT_CONTRAST)->GetWindowRect(&IdRect);

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;	IdRect.bottom = IdRect.top + h;

	if (pt.x >= IdRect.left && pt.x <= IdRect.right && pt.y >= IdRect.top && pt.y <= IdRect.bottom)		return TRUE;
	else		return FALSE;
}

BOOL CMarkSearchingSheet::IsClickMinContrastTitle(CPoint pt)
{
	CRect IdRect, rect;
	GetWindowRect(&rect);
	GetDlgItem(IDC_EDIT_CONTRAST2)->GetWindowRect(&IdRect);

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;	IdRect.bottom = IdRect.top + h;

	if (pt.x >= IdRect.left && pt.x <= IdRect.right && pt.y >= IdRect.top && pt.y <= IdRect.bottom)		return TRUE;
	else		return FALSE;
}