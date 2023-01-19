// KeyPadDlg.cpp : implementation file
//

#include "stdafx.h"
#include "KeyPadDlg.h"
#include <math.h>

#define ADD 0
#define SUB 1

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKeyPadDlg dialog


CKeyPadDlg::CKeyPadDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CKeyPadDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CKeyPadDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bCustomDrawPos = FALSE;
	m_strValue = "";
	bCheck = TRUE;

	m_dbValue1 = 0.0;
	m_dbValue2 = 0.0;
	m_nOperator = 0;
	m_bIsInput = FALSE;
	m_bIsDispCopyData = FALSE;
}


void CKeyPadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CKeyPadDlg)
	DDX_Control(pDX, IDC_KEY_1, m_btnKey1);
	DDX_Control(pDX, IDC_KEY_2, m_btnKey2);
	DDX_Control(pDX, IDC_KEY_3, m_btnKey3);
	DDX_Control(pDX, IDC_KEY_4, m_btnKey4);
	DDX_Control(pDX, IDC_KEY_5, m_btnKey5);
	DDX_Control(pDX, IDC_KEY_6, m_btnKey6);
	DDX_Control(pDX, IDC_KEY_7, m_btnKey7);
	DDX_Control(pDX, IDC_KEY_8, m_btnKey8);
	DDX_Control(pDX, IDC_KEY_9, m_btnKey9);
	DDX_Control(pDX, IDC_KEY_0, m_btnKey0);
	DDX_Control(pDX, IDC_KEY_DOT, m_btnKeyDot);
	DDX_Control(pDX, IDC_KEY_BACK_SPACE, m_btnKeyBS);
	DDX_Control(pDX, IDOK, m_btnKeyOk);
	DDX_Control(pDX, IDCANCEL, m_btnKeyCancel);

	DDX_Control(pDX, ID_BTN_CLEAR, m_btnKeyClear);
	DDX_Control(pDX, IDC_KEY_PLUS_MINUS, m_btnKeyPlusMinus);
	DDX_Control(pDX, IDC_BTN_ADD, m_btnKeyAdd);
	DDX_Control(pDX, IDC_BTN_SUB, m_btnKeySub);
	DDX_Control(pDX, IDC_BTN_EQUAL, m_btnKeyEqual);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_NUMBER_TEXT, m_ctrlNumberText);
	DDX_Control(pDX, IDC_CALCUL_TEXT, m_calculText);
}


BEGIN_MESSAGE_MAP(CKeyPadDlg, CDialog)
	//{{AFX_MSG_MAP(CKeyPadDlg)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_KEY_1, OnKey1)
	ON_BN_CLICKED(IDC_KEY_2, OnKey2)
	ON_BN_CLICKED(IDC_KEY_3, OnKey3)
	ON_BN_CLICKED(IDC_KEY_4, OnKey4)
	ON_BN_CLICKED(IDC_KEY_5, OnKey5)
	ON_BN_CLICKED(IDC_KEY_6, OnKey6)
	ON_BN_CLICKED(IDC_KEY_7, OnKey7)
	ON_BN_CLICKED(IDC_KEY_8, OnKey8)
	ON_BN_CLICKED(IDC_KEY_9, OnKey9)
	ON_BN_CLICKED(IDC_KEY_0, OnKey0)
	ON_BN_CLICKED(IDC_KEY_DOT, OnKeyDot)
	ON_BN_CLICKED(IDC_KEY_BACK_SPACE, OnKeyBackSpace)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_WM_NCHITTEST()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(ID_BTN_CLEAR, &CKeyPadDlg::OnBnClickedBtnClear)
	ON_BN_CLICKED(IDC_KEY_PLUS_MINUS, &CKeyPadDlg::OnBnClickedKeyPlusMinus)
	ON_BN_CLICKED(IDC_BTN_ADD, &CKeyPadDlg::OnBnClickedBtnAdd)
	ON_BN_CLICKED(IDC_BTN_SUB, &CKeyPadDlg::OnBnClickedBtnSub)
	ON_BN_CLICKED(IDC_BTN_EQUAL, &CKeyPadDlg::OnBnClickedBtnEqual)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKeyPadDlg message handlers

LRESULT CKeyPadDlg::OnNcHitTest(CPoint point) 
{
    // TODO: Add your message handler code here and/or call default
    LRESULT nHit = CDialog::OnNcHitTest(point);
	
    if (nHit == HTCLIENT)
        nHit = HTCAPTION;
	
    return nHit;
}

BOOL CKeyPadDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_RETURN) OnOK();
		else if(pMsg->wParam == VK_ESCAPE) OnCancel();
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CKeyPadDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);

	int		nScreenX, nScreenY;

	nScreenX	= GetSystemMetrics(SM_CXSCREEN);
	nScreenY	= GetSystemMetrics(SM_CYSCREEN);

	m_Font.CreateFont(17,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS
		,DEFAULT_QUALITY,VARIABLE_PITCH | FF_SWISS,_T("Arial"));

	if(m_bCustomDrawPos)
	{
		CRect rect;
		GetWindowRect(&rect);
		MoveWindow(m_nOffsetX, m_nOffsetY, rect.Width(), rect.Height());
	}

	SetDlgItemText(IDC_NUMBER_TEXT, m_strValue);
	SetDlgItemText(IDC_CALCUL_TEXT, m_strValue);

	// m_calculText.SetSel(0, -1, TRUE);

	if (m_bIsPassword == true)
	{
		//m_ctrlNumberText.SetPasswordChar(_T('*'));
		//m_ctrlNumberText.ModifyStyle(0, ES_PASSWORD);		

		m_calculText.SetPasswordChar(_T('*'));
		m_calculText.ModifyStyle(0, ES_PASSWORD);
	}
	ShowWindow(SW_SHOW);

	MainButtonInit(&m_btnKey1);
	MainButtonInit(&m_btnKey2);
	MainButtonInit(&m_btnKey3);
	MainButtonInit(&m_btnKey4);
	MainButtonInit(&m_btnKey5);
	MainButtonInit(&m_btnKey6);
	MainButtonInit(&m_btnKey7);
	MainButtonInit(&m_btnKey8);
	MainButtonInit(&m_btnKey9);
	MainButtonInit(&m_btnKey0);
	MainButtonInit(&m_btnKeyDot);
	MainButtonInit(&m_btnKeyBS);
	MainButtonInit(&m_btnKeyOk);
	MainButtonInit(&m_btnKeyCancel);

	MainButtonInit(&m_btnKeyClear);
	MainButtonInit(&m_btnKeyPlusMinus);
	MainButtonInit(&m_btnKeyAdd);
	MainButtonInit(&m_btnKeySub);
	MainButtonInit(&m_btnKeyEqual);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CKeyPadDlg::InsertChar(int nIndex)
{
	CString strGetText;
	GetDlgItemText(IDC_CALCUL_TEXT, strGetText);

	CString strSetText;

	if( m_bIsInput )
		strSetText.Format(_T("%s%c"), strGetText, nIndex);
	else
		strSetText.Format(_T("%c"), nIndex);

	SetDlgItemText(IDC_CALCUL_TEXT, strSetText);

	m_bIsInput = TRUE;
}

void CKeyPadDlg::DeleteChar()
{
	CString strGetText;
	GetDlgItemText(IDC_CALCUL_TEXT, strGetText);

	if(strGetText.GetLength() == 0) return;

	CString strSetText;
	strSetText.Format(_T("%s"), strGetText.Left(strGetText.GetLength() - 1));
	SetDlgItemText(IDC_CALCUL_TEXT, strSetText);
}

void CKeyPadDlg::SetValueString(bool bIsPassword, CString strValue)
{
	m_strValue = strValue;
	m_strInitValue = strValue;
	m_bIsPassword = bIsPassword;
}

bool CKeyPadDlg::GetNumber(double nMinValue, double nMaxValue, CString &strNumber)
{
	m_dValue = atof((LPCTSTR)m_strValue);

	if (m_dValue<nMinValue || m_dValue>nMaxValue)
	{
		AfxMessageBox("Out of Range");
		//g_Global.SetMessageDialog(1, _T("Out of Range"));
		return false;
	}

	strNumber = m_strValue;
	return true;
}

bool CKeyPadDlg::GetValue(CString &strNumber)
{
	strNumber = m_strValue;
	return true;
}

void CKeyPadDlg::CustomDraw(int nOffsetX, int nOffsetY)
{
	m_bCustomDrawPos=TRUE; 
	m_nOffsetX=nOffsetX; 
	m_nOffsetY=nOffsetY;
}

void CKeyPadDlg::OnKey1()
{
	InsertChar('1');
}

void CKeyPadDlg::OnKey2()
{
	InsertChar('2');
}

void CKeyPadDlg::OnKey3()
{
	InsertChar('3');
}

void CKeyPadDlg::OnKey4()
{
	InsertChar('4');
}

void CKeyPadDlg::OnKey5()
{
	InsertChar('5');
}

void CKeyPadDlg::OnKey6()
{
	InsertChar('6');
}

void CKeyPadDlg::OnKey7()
{
	InsertChar('7');
}

void CKeyPadDlg::OnKey8()
{
	InsertChar('8');
}

void CKeyPadDlg::OnKey9()
{
	InsertChar('9');
}

void CKeyPadDlg::OnKey0()
{
	InsertChar('0');
}

void CKeyPadDlg::OnKeyDot()
{
	InsertChar('.');
}


void CKeyPadDlg::OnBnClickedKeyPlusMinus()
{
	CString strTemp;
	CString buffer[100];

	CString strGetText;
	GetDlgItemText(IDC_CALCUL_TEXT, strGetText);

	CString strSetText;

	double m_tempValue = atof(strGetText);
	/*
	if(bCheck)
	{
		m_tempValue = -m_tempValue;

		GetDlgItem(IDC_KEY_PLUS_MINUS)->SetWindowTextW(_T("+"));
		bCheck = FALSE;
	}
	else
	{
		m_tempValue = fabs(m_tempValue);
		GetDlgItem(IDC_KEY_PLUS_MINUS)->SetWindowTextW(_T("-"));
		bCheck = TRUE;
	}
	*/
	strSetText.Format(_T("%.3f"), m_tempValue * -1.0);
	SetDlgItemText(IDC_CALCUL_TEXT, strSetText);
	UpdateData(FALSE);
}

void CKeyPadDlg::OnKeyBackSpace()
{
	DeleteChar();
}

void CKeyPadDlg::OnBnClickedBtnClear()
{
	m_calculText.SetSel(0,-1);
	m_calculText.Clear();
}

void CKeyPadDlg::OnOK()
{ 
	GetDlgItemText(IDC_CALCUL_TEXT, m_strValue);

	CDialog::OnOK();
}

void CKeyPadDlg::OnCancel() 
{
	m_strValue = m_strInitValue;
	CDialog::OnCancel();
}

void CKeyPadDlg::OnBnClickedBtnAdd()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString str;
	m_calculText.GetWindowText(str);
	m_dbValue1 = atof((LPCTSTR)str);
	m_calculText.SetWindowText(_T(" "));
	m_nOperator = 1;	// 1 : Addition
}

void CKeyPadDlg::OnBnClickedBtnSub()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString str;
	m_calculText.GetWindowText(str);
	m_dbValue1 = atof((LPCTSTR)str);
	m_calculText.SetWindowText(_T(" "));
	m_nOperator = 2;	// 2 : Subtraction
}

void CKeyPadDlg::OnBnClickedBtnEqual()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString str;
	m_calculText.GetWindowText(str);
	m_dbValue2 = atof((LPCTSTR)str);
	
	switch( m_nOperator ) {
	case 0:
		m_dbTotalValue = m_dbValue2;
		break;
	case 1:
		m_dbTotalValue = m_dbValue1 + m_dbValue2;
		break;
	case 2:
		m_dbTotalValue = m_dbValue1 - m_dbValue2;
		break;
	default:
		m_dbTotalValue = m_dbValue2;
		break;
	}

	str.Format(_T("%.3f"), m_dbTotalValue);
	m_strValue = str;
	m_calculText.SetWindowText((LPCTSTR)str);
}

void CKeyPadDlg::MainButtonInit(CButtonEx *pbutton,int size)
{
	pbutton->SetEnable(true);	
	pbutton->SetSizeText(16.f);
	pbutton->SetColorBkg(255, COLOR_BTN_BODY);
	pbutton->SetColorBorder(255, COLOR_BTN_SIDE);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
//	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}

HBRUSH CKeyPadDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if( nCtlColor == CTLCOLOR_EDIT && (pWnd->GetDlgCtrlID() == IDC_CALCUL_TEXT
		|| pWnd->GetDlgCtrlID() == IDC_NUMBER_TEXT) )
	{
		pDC->SetBkColor( COLOR_UI_BODY );
		pDC->SetBkMode( TRANSPARENT );
		pDC->SetTextColor( COLOR_WHITE );
	}

	return m_hbrBkg;
}
