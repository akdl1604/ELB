// MarkFindingSheet.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "MarkFindingSheet.h"
#include "afxdialogex.h"

// CMarkFindingSheet 대화 상자입니다.

template <typename T>
  std::string NumberToString ( T Number )
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

IMPLEMENT_DYNAMIC(CMarkFindingSheet, CDialogEx)

CMarkFindingSheet::CMarkFindingSheet(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMarkFindingSheet::IDD, pParent)
	, m_edtNumMatch(0)
	, m_edtSangle(0)
	, m_edtEangle(0)
	, m_edtMinScore(0)
	, m_edtGreedness(0)
{
	m_pParentDlg = NULL;
}

CMarkFindingSheet::~CMarkFindingSheet()
{

}

void CMarkFindingSheet::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_NUMLEVEL, m_edtNumMatch);
	DDX_Text(pDX, IDC_EDIT_S_ANGLE, m_edtSangle);
	DDX_Text(pDX, IDC_EDIT_E_ANGLE, m_edtEangle);
	DDX_Text(pDX, IDC_EDIT_MINSCORE, m_edtMinScore);
	DDX_Text(pDX, IDC_EDIT_CONTRAST, m_edtGreedness);
	DDX_Text(pDX, IDC_EDIT_SCALE, m_edtNumScale);
	DDX_Control(pDX, IDC_CB_SELECT_SUBPIXEL, m_cmbSubPixel);
	DDX_Control(pDX, IDC_CB_SELECT_METHOD, m_cmbMethod);
}


BEGIN_MESSAGE_MAP(CMarkFindingSheet, CDialogEx)
	ON_STN_CLICKED(IDC_EDIT_NUMLEVEL, &CMarkFindingSheet::OnStnClickedEditNumlevel)
	ON_STN_CLICKED(IDC_EDIT_S_ANGLE, &CMarkFindingSheet::OnStnClickedEditSAngle)
	ON_STN_CLICKED(IDC_EDIT_E_ANGLE, &CMarkFindingSheet::OnStnClickedEditEAngle)
	ON_STN_CLICKED(IDC_EDIT_MINSCORE, &CMarkFindingSheet::OnStnClickedEditMinscore)
	ON_STN_CLICKED(IDC_EDIT_CONTRAST, &CMarkFindingSheet::OnStnClickedEditContrast)
	ON_STN_CLICKED(IDC_EDIT_SCALE, &CMarkFindingSheet::OnStnClickedEditScale)
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


// CMarkFindingSheet 메시지 처리기입니다.


BOOL CMarkFindingSheet::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	m_cmbSubPixel.SetDroppedWidth(m_cmbSubPixel.GetDroppedWidth()+100);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CMarkFindingSheet::OnStnClickedEditNumlevel()
{
	CString str, strTemp;

	str.Format("%d", m_edtNumMatch);
	strTemp =theApp.m_pFrame->GetNumberBox(str, 5,  0, 255);

	from_string<int>(m_edtNumMatch, std::string(strTemp), std::dec);

	UpdateData(FALSE);
}


void CMarkFindingSheet::OnStnClickedEditSAngle()
{
	CString str, strTemp;

	str.Format("%4.2f", m_edtSangle);
	strTemp =theApp.m_pFrame->GetNumberBox(str, 5,  -360, 360);

	from_string<double>(m_edtSangle, std::string(strTemp), std::dec);

	UpdateData(FALSE);
}

void CMarkFindingSheet::OnStnClickedEditEAngle()
{
	CString str, strTemp;

	str.Format("%4.2f", m_edtEangle);
	strTemp =theApp.m_pFrame->GetNumberBox(str, 5,  -360, 360);

	from_string<double>(m_edtEangle, std::string(strTemp), std::dec);

	UpdateData(FALSE);
}


void CMarkFindingSheet::OnStnClickedEditMinscore()
{
	CString str, strTemp;

	str.Format("%4.2f", m_edtMinScore);
	strTemp = theApp.m_pFrame->GetNumberBox(str, 5, 0, 100);

	from_string<double>(m_edtMinScore, std::string(strTemp), std::dec);

	UpdateData(FALSE);
}


void CMarkFindingSheet::OnStnClickedEditContrast()
{
	CString str, strTemp;

	str.Format("%4.2f", m_edtGreedness);
	strTemp =theApp.m_pFrame->GetNumberBox(str, 5,  -255, 255);

	from_string<double>(m_edtGreedness, std::string(strTemp), std::dec);

	UpdateData(FALSE);
}

void CMarkFindingSheet::OnStnClickedEditScale()
{
	CString str, strTemp;

	str.Format("%4.2f", m_edtNumScale);
	strTemp = theApp.m_pFrame->GetNumberBox(str, 5, 0, 0.9);

	from_string<double>(m_edtNumScale, std::string(strTemp), std::dec);

	UpdateData(FALSE);
}

void CMarkFindingSheet::OnLButtonUp(UINT nFlags, CPoint point)
{
	if( IsClickNumLevelTitle(point) == TRUE )
	{
		OnStnClickedEditNumlevel();
	}
	else if( IsClickSAngleTitle(point) == TRUE )
	{
		//OnStnClickedEditSAngle();
	}
	else if( IsClickEAngleTitle(point) == TRUE )
	{
		//OnStnClickedEditEAngle();
	}
	else if( IsClickMinscoreTitle(point) == TRUE )
	{
		OnStnClickedEditMinscore();
	}
	else if( IsClickGreednesslTitle(point) == TRUE )
	{
		OnStnClickedEditContrast();
	}
	else if (IsClickScaleTitle(point) == TRUE)
	{
		OnStnClickedEditScale();
	}

	CDialogEx::OnLButtonUp(nFlags, point);
}


BOOL CMarkFindingSheet::IsClickNumLevelTitle(CPoint pt)
{
	CRect IdRect,rect;
	GetWindowRect(&rect);
	GetDlgItem(IDC_EDIT_NUMLEVEL)->GetWindowRect( &IdRect );

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;	IdRect.bottom = IdRect.top + h;

	if( pt.x >= IdRect.left && 	pt.x <= IdRect.right && 	pt.y >= IdRect.top && 	pt.y <= IdRect.bottom )		return TRUE;
	else		return FALSE;
}

BOOL CMarkFindingSheet::IsClickSAngleTitle(CPoint pt)
{
	CRect IdRect,rect;
	GetWindowRect(&rect);
	GetDlgItem(IDC_EDIT_S_ANGLE)->GetWindowRect( &IdRect );

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;	IdRect.bottom = IdRect.top + h;

	if( pt.x >= IdRect.left && 	pt.x <= IdRect.right && 	pt.y >= IdRect.top && 	pt.y <= IdRect.bottom )		return TRUE;
	else		return FALSE;
}


BOOL CMarkFindingSheet::IsClickEAngleTitle(CPoint pt)
{
	CRect IdRect,rect;
	GetWindowRect(&rect);
	GetDlgItem(IDC_EDIT_E_ANGLE)->GetWindowRect( &IdRect );

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;	IdRect.bottom = IdRect.top + h;

	if( pt.x >= IdRect.left && 	pt.x <= IdRect.right && 	pt.y >= IdRect.top && 	pt.y <= IdRect.bottom )		return TRUE;
	else		return FALSE;
}


BOOL CMarkFindingSheet::IsClickMinscoreTitle(CPoint pt)
{
	CRect IdRect,rect;
	GetWindowRect(&rect);
	GetDlgItem(IDC_EDIT_MINSCORE)->GetWindowRect( &IdRect );

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;	IdRect.bottom = IdRect.top + h;

	if( pt.x >= IdRect.left && 	pt.x <= IdRect.right && 	pt.y >= IdRect.top && 	pt.y <= IdRect.bottom )		return TRUE;
	else		return FALSE;
}


BOOL CMarkFindingSheet::IsClickGreednesslTitle(CPoint pt)
{
	CRect IdRect,rect;
	GetWindowRect(&rect);
	GetDlgItem(IDC_EDIT_CONTRAST)->GetWindowRect( &IdRect );

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;	IdRect.bottom = IdRect.top + h;

	if( pt.x >= IdRect.left && 	pt.x <= IdRect.right && 	pt.y >= IdRect.top && 	pt.y <= IdRect.bottom )		return TRUE;
	else		return FALSE;
}

BOOL CMarkFindingSheet::IsClickScaleTitle(CPoint pt)
{
	CRect IdRect, rect;
	GetWindowRect(&rect);
	GetDlgItem(IDC_EDIT_SCALE)->GetWindowRect(&IdRect);

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;	IdRect.bottom = IdRect.top + h;

	if (pt.x >= IdRect.left && pt.x <= IdRect.right && pt.y >= IdRect.top && pt.y <= IdRect.bottom)		return TRUE;
	else		return FALSE;
}