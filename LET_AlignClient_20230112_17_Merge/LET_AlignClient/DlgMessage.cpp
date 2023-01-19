// DlgMessage.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "DlgMessage.h"
#include "afxdialogex.h"


// CDlgMessage ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CDlgMessage, CDialogEx)

CDlgMessage::CDlgMessage(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgMessage::IDD, pParent)
{
	m_nResult = 0;
	m_nType = 0;

	m_strText1 = _T("");
	m_strText2 = _T("");
}

CDlgMessage::~CDlgMessage()
{
}

void CDlgMessage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LB_TEXT1, m_lbText1);
	DDX_Control(pDX, IDC_LB_TEXT2, m_lbText2);
	DDX_Control(pDX, IDC_BTN_MESSAGE_OK, m_btnOK);
	DDX_Control(pDX, IDC_BTN_MESSAGE_CANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CDlgMessage, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_MESSAGE_OK, &CDlgMessage::OnBnClickedBtnMessageOk)
	ON_BN_CLICKED(IDC_BTN_MESSAGE_CANCEL, &CDlgMessage::OnBnClickedBtnMessageCancel)
END_MESSAGE_MAP()


// CDlgMessage �޽��� ó�����Դϴ�.

HBRUSH CDlgMessage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{	

	return m_hbrBkg;
}

void CDlgMessage::InitTitle(CLabelEx *pTitle,CString str,float size,COLORREF color)
{
	pTitle->SetAlignTextLT();
	pTitle->SetOffsetText(10, 10);
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}

void CDlgMessage::MainButtonInit(CButtonEx *pbutton,int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
//	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}

BOOL CDlgMessage::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);

	InitTitle( &m_lbText1, "", 20.f, COLOR_BTN_BODY );
	InitTitle( &m_lbText2, "", 20.f, COLOR_BTN_BODY );

	MainButtonInit( &m_btnOK );
	MainButtonInit( &m_btnCancel );	

	m_lbText1.SetText( m_strText1 );
	m_lbText2.SetText( m_strText2 );

	if (m_nType == 1)
	{
		CRect rect;
		CRect rect2;
		m_btnOK.GetWindowRect(rect);
		m_btnCancel.GetWindowRect(rect2);
		ScreenToClient(rect);
		ScreenToClient(rect2);

		int dw = rect2.right - rect.left;
		int off_x = (dw - rect.Width()) / 2;

		m_btnCancel.ShowWindow(SW_HIDE);
		m_btnOK.MoveWindow(rect.left + off_x, rect.top, rect.Width(), rect.Height());
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}


void CDlgMessage::OnBnClickedBtnMessageOk()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	m_nResult = 1;
	CDialog::OnOK();
}


void CDlgMessage::OnBnClickedBtnMessageCancel()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	m_nResult = 0;
	CDialog::OnCancel();
}

void CDlgMessage::setMessageModaless(int nType, CString strText1, CString strText2)
{
	setType(nType);
	setMessageText(strText1, strText2);
	setResult(-1);
}