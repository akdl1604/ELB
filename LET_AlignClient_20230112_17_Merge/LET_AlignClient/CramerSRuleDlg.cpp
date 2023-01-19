// CramerSRuleDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "CramerSRuleDlg.h"
#include "afxdialogex.h"


// CCramerSRuleDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CCramerSRuleDlg, CDialogEx)

CCramerSRuleDlg::CCramerSRuleDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCramerSRuleDlg::IDD, pParent)
{
	camX = camY = revisionX = revisionY = remainX = remainY = theta = 0.0;
	m_drotateX = m_drotateY = 0.0;
}

CCramerSRuleDlg::~CCramerSRuleDlg()
{
}

void CCramerSRuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCramerSRuleDlg, CDialogEx)
	ON_BN_CLICKED(1009, &CCramerSRuleDlg::OnBnClicked1009)
END_MESSAGE_MAP()


// CCramerSRuleDlg 메시지 처리기입니다.


void CCramerSRuleDlg::OnBnClicked1009()
{
	CString str;
	double C1, C2, C3, C4, temp1, temp2;
	double a, b, c, d, p, q;
	double degree, radian;
	double rotateX, rotateY;

	// X1
	GetDlgItem(IDC_EDIT1)->GetWindowText(str);
	C1 = atof(str);

	// Y1
	GetDlgItem(IDC_EDIT2)->GetWindowText(str);
	C2 = atof(str);

	// RevisionX
	GetDlgItem(IDC_EDIT3)->GetWindowText(str);
	temp1 = -atof(str);

	// RemainX
	GetDlgItem(IDC_EDIT5)->GetWindowText(str);
	temp2 = atof(str);
	
	// Total X
	C3 = temp1 + temp2;

	// RevisionY
	GetDlgItem(IDC_EDIT4)->GetWindowText(str);
	temp1 = -atof(str);

	// RemainY
	GetDlgItem(IDC_EDIT6)->GetWindowText(str);
	temp2 = atof(str);

	// Total Y
	C4 = temp1 + temp2;

	// Theta
	GetDlgItem(IDC_EDIT7)->GetWindowText(str);
	degree = -atof(str);
	radian = degree * CV_PI / 180.0;
	
	a = 1 - cos(radian);
	b = sin(radian);
	c = -sin(radian);
	d = 1 - cos(radian);	

	p = C3 - C1 * cos(radian) + C2 * sin(radian);
	q = C4 - C1 * sin(radian) - C2 * cos(radian);
	rotateX = (p * d - b * q) / (a * d - b * c);
	rotateY = (a * q - c * p) / (a * d - b * c);

	str.Format("%f", rotateX);
	GetDlgItem(IDC_EDIT8)->SetWindowTextA(str);

	str.Format("%f", rotateY);
	GetDlgItem(IDC_EDIT9)->SetWindowTextA(str);

	m_drotateX = rotateX, m_drotateY = rotateY;

	//EndDialog(IDOK); 
}


BOOL CCramerSRuleDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	CString str = "0";
	str.Format("%f", camX); GetDlgItem(IDC_EDIT1)->SetWindowTextA(str);
	str.Format("%f", camY); GetDlgItem(IDC_EDIT2)->SetWindowTextA(str);
	str.Format("%f", revisionX); GetDlgItem(IDC_EDIT3)->SetWindowTextA(str);
	str.Format("%f", revisionY); GetDlgItem(IDC_EDIT4)->SetWindowTextA(str);
	str.Format("%f", remainX); GetDlgItem(IDC_EDIT5)->SetWindowTextA(str);
	str.Format("%f", remainY); GetDlgItem(IDC_EDIT6)->SetWindowTextA(str);
	str.Format("%f", theta); GetDlgItem(IDC_EDIT7)->SetWindowTextA(str);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CCramerSRuleDlg::SetCameraXY(double x, double y)
{
	camX = x, camY = y;	
}

void CCramerSRuleDlg::SetRevisionXY(double x, double y)
{
	revisionX = x, revisionY = y;	
}

void CCramerSRuleDlg::SetRemainXY(double x, double y)
{
	remainX = x, remainY = y;	
}

void CCramerSRuleDlg::SetTheta(double angle)
{
	theta = angle;	
}

void CCramerSRuleDlg::GetRotateXY(double *x, double *y)
{
	*x = m_drotateX;
	*y = m_drotateY;
}
