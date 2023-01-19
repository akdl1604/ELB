// VirtualKeyBoard.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "VirtualKeyBoard.h"
#include "afxdialogex.h"
// CVirtualKeyBoard 대화 상자입니다.

void NewTypingMessageFromCodePage(TCHAR* Message, UINT CodePage=0)
{
	TCHAR Word[2];
	TCHAR WordCode[64];
	char MultiByte[64];

	static const BYTE NumCode[10]={0x2D, 0x23, 0x28, 0x22, 0x25, 0x0C, 0x27, 0x24, 0x26, 0x21};
	int Length = int(wcslen((wchar_t *)Message));

	for(int i=0; i<Length; i++)
	{
		Word[0] = Message[i];
		Word[1] = L'\0';
		WideCharToMultiByte(CodePage, 0, (LPCWSTR)Word, -1, MultiByte, 64, NULL, NULL);
		_itow((int)(((~MultiByte[0])^0xff)<<8)+((~MultiByte[1])^0xff), (wchar_t *)WordCode, 10);
		keybd_event(VK_MENU, MapVirtualKey(VK_MENU, 0), 0, 0);
		for(unsigned int j=0; j<wcslen((wchar_t *)WordCode); j++)
		{
			keybd_event(NumCode[(int)WordCode[j]-48], MapVirtualKey(NumCode[(int)WordCode[j]-48], 0), 0, 0);
			keybd_event(NumCode[(int)WordCode[j]-48], MapVirtualKey(NumCode[(int)WordCode[j]-48], 0), KEYEVENTF_KEYUP, 0);
		}
		keybd_event(VK_MENU, MapVirtualKey(VK_MENU, 0), KEYEVENTF_KEYUP, 0);
	}
}


IMPLEMENT_DYNAMIC(CVirtualKeyBoard, CDialogEx)

CVirtualKeyBoard::CVirtualKeyBoard(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVirtualKeyBoard::IDD, pParent)
{
	m_bCapsLock = TRUE;
	m_bLanguage = FALSE;
	m_bShift = FALSE;
	m_bCtrl = FALSE;
	m_bAlt = FALSE;
	m_bIsInput = FALSE;
	m_strValue = "";
	m_strInputString = "";

	m_nOffsetX = 0;
	m_nOffsetY = 0;
}

CVirtualKeyBoard::~CVirtualKeyBoard()
{
}

void CVirtualKeyBoard::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX,IDC_BUTTON_RUDOT,m_btnKeyRudot);
	DDX_Control(pDX,IDC_BUTTON_1,m_btnKey1);
	DDX_Control(pDX,IDC_BUTTON_2,m_btnKey2);
	DDX_Control(pDX,IDC_BUTTON_3,m_btnKey3);
	DDX_Control(pDX,IDC_BUTTON_4,m_btnKey4);
	DDX_Control(pDX,IDC_BUTTON_5,m_btnKey5);
	DDX_Control(pDX,IDC_BUTTON_6,m_btnKey6);
	DDX_Control(pDX,IDC_BUTTON_7,m_btnKey7);
	DDX_Control(pDX,IDC_BUTTON_8,m_btnKey8);
	DDX_Control(pDX,IDC_BUTTON_9,m_btnKey9);
	DDX_Control(pDX,IDC_BUTTON_0,m_btnKey0);
	DDX_Control(pDX,IDC_BUTTON_MINUS,m_btnKeyMinus);
	DDX_Control(pDX,IDC_BUTTON_EQUAL,m_btnKeyEqual);
	DDX_Control(pDX,IDC_BUTTON_BACKSP,m_btnKeyBacksp);
	DDX_Control(pDX,IDC_BUTTON_TAB,	m_btnKeyTab);
	DDX_Control(pDX,IDC_BUTTON_Q,m_btnKeyQ);
	DDX_Control(pDX,IDC_BUTTON_W,m_btnKeyW);
	DDX_Control(pDX,IDC_BUTTON_E,m_btnKeyE);
	DDX_Control(pDX,IDC_BUTTON_R,m_btnKeyR);
	DDX_Control(pDX,IDC_BUTTON_T,m_btnKeyT);
	DDX_Control(pDX,IDC_BUTTON_Y,m_btnKeyY);
	DDX_Control(pDX,IDC_BUTTON_U,m_btnKeyU);
	DDX_Control(pDX,IDC_BUTTON_I,m_btnKeyI);
	DDX_Control(pDX,IDC_BUTTON_O,m_btnKeyO);
	DDX_Control(pDX,IDC_BUTTON_P,m_btnKeyP);
	DDX_Control(pDX,IDC_BUTTON_LB,m_btnKeyLb);
	DDX_Control(pDX,IDC_BUTTON_RB,m_btnKeyRb);
	DDX_Control(pDX,IDC_BUTTON_RS,m_btnKeyRs);
	DDX_Control(pDX,IDC_BUTTON_CR,m_btnKeyCr);
	DDX_Control(pDX,IDC_BUTTON_A,m_btnKeyA);
	DDX_Control(pDX,IDC_BUTTON_S,m_btnKeyS);
	DDX_Control(pDX,IDC_BUTTON_D,m_btnKeyD);
	DDX_Control(pDX,IDC_BUTTON_F,m_btnKeyF);
	DDX_Control(pDX,IDC_BUTTON_G,m_btnKeyG);
	DDX_Control(pDX,IDC_BUTTON_H,m_btnKeyH);
	DDX_Control(pDX,IDC_BUTTON_J,m_btnKeyJ);
	DDX_Control(pDX,IDC_BUTTON_K,m_btnKeyK);
	DDX_Control(pDX,IDC_BUTTON_L,m_btnKeyL);
	DDX_Control(pDX,IDC_BUTTON_SC,m_btnKeySc);
	DDX_Control(pDX,IDC_BUTTON_RUC,m_btnKeyRuc);
	DDX_Control(pDX,IDC_BUTTON_ENTER,m_btnKeyEnter);
	DDX_Control(pDX,IDC_BUTTON_LSHIFT,m_btnKeyLshift);
	DDX_Control(pDX,IDC_BUTTON_Z,m_btnKeyZ);
	DDX_Control(pDX,IDC_BUTTON_X,m_btnKeyX);
	DDX_Control(pDX,IDC_BUTTON_C,m_btnKeyC);
	DDX_Control(pDX,IDC_BUTTON_V,m_btnKeyV);
	DDX_Control(pDX,IDC_BUTTON_B,m_btnKeyB);
	DDX_Control(pDX,IDC_BUTTON_N,m_btnKeyN);
	DDX_Control(pDX,IDC_BUTTON_M,m_btnKeyM);
	DDX_Control(pDX,IDC_BUTTON_COMMA,m_btnKeyComma);
	DDX_Control(pDX,IDC_BUTTON_DOT, m_btnKeyDot);
	DDX_Control(pDX,IDC_BUTTON_SLASH,m_btnKeySlash);
	DDX_Control(pDX,IDC_BUTTON_RSHIFT,m_btnKeyRshift);
	DDX_Control(pDX,IDC_BUTTON_LCTRL,m_btnKeyLctrl);
	DDX_Control(pDX,IDC_BUTTON_ALT, m_btnKeyAlt);
	DDX_Control(pDX,IDC_BUTTON_SPACE,m_btnKeySpace);
	DDX_Control(pDX,IDC_BUTTON_HE,	m_btnKeyHe);
	DDX_Control(pDX,IDC_BUTTON_EXIT, m_btnKeyExit);
	DDX_Control(pDX,IDC_BUTTON_CLEAR, m_btnKeyClear);
	DDX_Control(pDX, IDC_EDIT_INPUT, m_ctrlNumberText);

	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CVirtualKeyBoard, CDialogEx)
	ON_WM_NCHITTEST()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_RUDOT, &CVirtualKeyBoard::OnBnClickedButtonRudot)
	ON_BN_CLICKED(IDC_BUTTON_1, &CVirtualKeyBoard::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON_2, &CVirtualKeyBoard::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON_3, &CVirtualKeyBoard::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON_4, &CVirtualKeyBoard::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON_5, &CVirtualKeyBoard::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON_6, &CVirtualKeyBoard::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON_7, &CVirtualKeyBoard::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON_8, &CVirtualKeyBoard::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON_9, &CVirtualKeyBoard::OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON_0, &CVirtualKeyBoard::OnBnClickedButton0)
	ON_BN_CLICKED(IDC_BUTTON_MINUS, &CVirtualKeyBoard::OnBnClickedButtonMinus)
	ON_BN_CLICKED(IDC_BUTTON_EQUAL, &CVirtualKeyBoard::OnBnClickedButtonEqual)
	ON_BN_CLICKED(IDC_BUTTON_BACKSP, &CVirtualKeyBoard::OnBnClickedButtonBacksp)
	ON_BN_CLICKED(IDC_BUTTON_TAB, &CVirtualKeyBoard::OnBnClickedButtonTab)
	ON_BN_CLICKED(IDC_BUTTON_Q, &CVirtualKeyBoard::OnBnClickedButtonQ)
	ON_BN_CLICKED(IDC_BUTTON_W, &CVirtualKeyBoard::OnBnClickedButtonW)
	ON_BN_CLICKED(IDC_BUTTON_E, &CVirtualKeyBoard::OnBnClickedButtonE)
	ON_BN_CLICKED(IDC_BUTTON_R, &CVirtualKeyBoard::OnBnClickedButtonR)
	ON_BN_CLICKED(IDC_BUTTON_T, &CVirtualKeyBoard::OnBnClickedButtonT)
	ON_BN_CLICKED(IDC_BUTTON_Y, &CVirtualKeyBoard::OnBnClickedButtonY)
	ON_BN_CLICKED(IDC_BUTTON_U, &CVirtualKeyBoard::OnBnClickedButtonU)
	ON_BN_CLICKED(IDC_BUTTON_I, &CVirtualKeyBoard::OnBnClickedButtonI)
	ON_BN_CLICKED(IDC_BUTTON_O, &CVirtualKeyBoard::OnBnClickedButtonO)
	ON_BN_CLICKED(IDC_BUTTON_P, &CVirtualKeyBoard::OnBnClickedButtonP)
	ON_BN_CLICKED(IDC_BUTTON_LB, &CVirtualKeyBoard::OnBnClickedButtonLb)
	ON_BN_CLICKED(IDC_BUTTON_RB, &CVirtualKeyBoard::OnBnClickedButtonRb)
	ON_BN_CLICKED(IDC_BUTTON_RS, &CVirtualKeyBoard::OnBnClickedButtonRs)
	ON_BN_CLICKED(IDC_BUTTON_CR, &CVirtualKeyBoard::OnBnClickedButtonCr)
	ON_BN_CLICKED(IDC_BUTTON_A, &CVirtualKeyBoard::OnBnClickedButtonA)
	ON_BN_CLICKED(IDC_BUTTON_S, &CVirtualKeyBoard::OnBnClickedButtonS)
	ON_BN_CLICKED(IDC_BUTTON_D, &CVirtualKeyBoard::OnBnClickedButtonD)
	ON_BN_CLICKED(IDC_BUTTON_F, &CVirtualKeyBoard::OnBnClickedButtonF)
	ON_BN_CLICKED(IDC_BUTTON_G, &CVirtualKeyBoard::OnBnClickedButtonG)
	ON_BN_CLICKED(IDC_BUTTON_H, &CVirtualKeyBoard::OnBnClickedButtonH)
	ON_BN_CLICKED(IDC_BUTTON_J, &CVirtualKeyBoard::OnBnClickedButtonJ)
	ON_BN_CLICKED(IDC_BUTTON_K, &CVirtualKeyBoard::OnBnClickedButtonK)
	ON_BN_CLICKED(IDC_BUTTON_L, &CVirtualKeyBoard::OnBnClickedButtonL)
	ON_BN_CLICKED(IDC_BUTTON_SC, &CVirtualKeyBoard::OnBnClickedButtonSc)
	ON_BN_CLICKED(IDC_BUTTON_RUC, &CVirtualKeyBoard::OnBnClickedButtonRuc)
	ON_BN_CLICKED(IDC_BUTTON_ENTER, &CVirtualKeyBoard::OnBnClickedButtonEnter)
	ON_BN_CLICKED(IDC_BUTTON_LSHIFT, &CVirtualKeyBoard::OnBnClickedButtonLshift)
	ON_BN_CLICKED(IDC_BUTTON_Z, &CVirtualKeyBoard::OnBnClickedButtonZ)
	ON_BN_CLICKED(IDC_BUTTON_X, &CVirtualKeyBoard::OnBnClickedButtonX)
	ON_BN_CLICKED(IDC_BUTTON_C, &CVirtualKeyBoard::OnBnClickedButtonC)
	ON_BN_CLICKED(IDC_BUTTON_V, &CVirtualKeyBoard::OnBnClickedButtonV)
	ON_BN_CLICKED(IDC_BUTTON_B, &CVirtualKeyBoard::OnBnClickedButtonB)
	ON_BN_CLICKED(IDC_BUTTON_N, &CVirtualKeyBoard::OnBnClickedButtonN)
	ON_BN_CLICKED(IDC_BUTTON_M, &CVirtualKeyBoard::OnBnClickedButtonM)
	ON_BN_CLICKED(IDC_BUTTON_COMMA, &CVirtualKeyBoard::OnBnClickedButtonComma)
	ON_BN_CLICKED(IDC_BUTTON_DOT, &CVirtualKeyBoard::OnBnClickedButtonDot)
	ON_BN_CLICKED(IDC_BUTTON_SLASH, &CVirtualKeyBoard::OnBnClickedButtonSlash)
	ON_BN_CLICKED(IDC_BUTTON_RSHIFT, &CVirtualKeyBoard::OnBnClickedButtonRshift)
	ON_BN_CLICKED(IDC_BUTTON_LCTRL, &CVirtualKeyBoard::OnBnClickedButtonLctrl)
	ON_BN_CLICKED(IDC_BUTTON_ALT, &CVirtualKeyBoard::OnBnClickedButtonAlt)
	ON_BN_CLICKED(IDC_BUTTON_SPACE, &CVirtualKeyBoard::OnBnClickedButtonSpace)
	ON_BN_CLICKED(IDC_BUTTON_HE, &CVirtualKeyBoard::OnBnClickedButtonHe)
	ON_BN_CLICKED(IDC_BUTTON_EXIT, &CVirtualKeyBoard::OnBnClickedButtonExit)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CVirtualKeyBoard::OnBnClickedButtonClear)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


LRESULT CVirtualKeyBoard::OnNcHitTest(CPoint point) 
{
    // TODO: Add your message handler code here and/or call default
    LRESULT nHit = CDialog::OnNcHitTest(point);
	
    if (nHit == HTCLIENT)
        nHit = HTCAPTION;
	
    return nHit;
}

BOOL CVirtualKeyBoard::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN) 
	{
		if (pMsg->wParam == VK_RETURN)		OnBnClickedButtonEnter();
		else if(pMsg->wParam == VK_ESCAPE)	OnBnClickedButtonExit();
	}
	
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CVirtualKeyBoard::MainButtonInit(CButtonEx *pbutton,int size)
{
	pbutton->SetEnable(true);	
	pbutton->SetSizeText(16.f);
	pbutton->SetColorBkg(255, COLOR_BTN_BODY);
	pbutton->SetColorBorder(255, COLOR_BTN_SIDE);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
}

HBRUSH CVirtualKeyBoard::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if( nCtlColor == CTLCOLOR_EDIT && pWnd->GetDlgCtrlID() == IDC_EDIT_INPUT )
	{
		pDC->SetBkColor( COLOR_UI_BODY );
		pDC->SetBkMode( TRANSPARENT );
		pDC->SetTextColor( COLOR_WHITE );
	}

	return m_hbrBkg;
}

BOOL CVirtualKeyBoard::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	
	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);

	if(m_nOffsetX == 0 && m_nOffsetY == 0) 
	{
		m_nOffsetX = GetSystemMetrics(SM_CXSCREEN) / 2;
		m_nOffsetY = GetSystemMetrics(SM_CYSCREEN) / 2;
	}

	CRect rect;
	GetWindowRect(&rect);
	MoveWindow(m_nOffsetX, m_nOffsetY, rect.Width(), rect.Height());

	m_Font.CreateFont(17,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS
		,DEFAULT_QUALITY,VARIABLE_PITCH | FF_SWISS,_T("Arial"));

	SetDlgItemText(IDC_EDIT_INPUT, m_strValue);

	m_ctrlNumberText.SetSizeText(35);					// 글자 크기
	m_ctrlNumberText.SetStyleTextBold(true);			// 글자 스타일
	m_ctrlNumberText.SetTextMargins(10,10);				// 글자 옵셋
	m_ctrlNumberText.SetColorText(RGB(255,255,255));	// 글자 색상

	if (m_bIsPassword == true)
	{
		m_ctrlNumberText.SetPasswordChar(_T('*'));
		m_ctrlNumberText.ModifyStyle(0, ES_PASSWORD);
	}

	MainButtonInit(&m_btnKeyRudot);
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
	MainButtonInit(&m_btnKeyMinus);
	MainButtonInit(&m_btnKeyEqual);
	MainButtonInit(&m_btnKeyBacksp);
	MainButtonInit(&m_btnKeyTab);
	MainButtonInit(&m_btnKeyQ);
	MainButtonInit(&m_btnKeyW);
	MainButtonInit(&m_btnKeyE);
	MainButtonInit(&m_btnKeyR);
	MainButtonInit(&m_btnKeyT);
	MainButtonInit(&m_btnKeyY);
	MainButtonInit(&m_btnKeyU);
	MainButtonInit(&m_btnKeyI);
	MainButtonInit(&m_btnKeyO);
	MainButtonInit(&m_btnKeyP);
	MainButtonInit(&m_btnKeyLb);
	MainButtonInit(&m_btnKeyRb);
	MainButtonInit(&m_btnKeyRs);
	MainButtonInit(&m_btnKeyCr);
	MainButtonInit(&m_btnKeyA);
	MainButtonInit(&m_btnKeyS);
	MainButtonInit(&m_btnKeyD);
	MainButtonInit(&m_btnKeyF);
	MainButtonInit(&m_btnKeyG);
	MainButtonInit(&m_btnKeyH);
	MainButtonInit(&m_btnKeyJ);
	MainButtonInit(&m_btnKeyK);
	MainButtonInit(&m_btnKeyL);
	MainButtonInit(&m_btnKeySc);
	MainButtonInit(&m_btnKeyRuc);
	MainButtonInit(&m_btnKeyEnter);
	MainButtonInit(&m_btnKeyLshift);
	MainButtonInit(&m_btnKeyZ);
	MainButtonInit(&m_btnKeyX);
	MainButtonInit(&m_btnKeyC);
	MainButtonInit(&m_btnKeyV);
	MainButtonInit(&m_btnKeyB);
	MainButtonInit(&m_btnKeyN);
	MainButtonInit(&m_btnKeyM);
	MainButtonInit(&m_btnKeyComma);
	MainButtonInit(&m_btnKeyDot);
	MainButtonInit(&m_btnKeySlash);
	MainButtonInit(&m_btnKeyRshift);
	MainButtonInit(&m_btnKeyLctrl);
	MainButtonInit(&m_btnKeyAlt);
	MainButtonInit(&m_btnKeySpace);
	MainButtonInit(&m_btnKeyHe);
	MainButtonInit(&m_btnKeyExit);
	MainButtonInit(&m_btnKeyClear);

	m_btnKeyCr.SetColorBkg(255, COLOR_BTN_SELECT);
	m_automataKr.Clear();

	ShowWindow(SW_SHOW);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// CVirtualKeyBoard 메시지 처리기입니다.

void CVirtualKeyBoard::InsertChar(int nIndex)
{
	CString strGetText;
	GetDlgItemText(IDC_EDIT_INPUT, strGetText);

	CString strSetText;

	if( m_bIsInput )
		strSetText.Format(_T("%s%c"), strGetText, nIndex);
	else
		strSetText.Format(_T("%c"), nIndex);

	SetDlgItemText(IDC_EDIT_INPUT, strSetText);

	m_bIsInput = TRUE;
}

void CVirtualKeyBoard::InsertChar(CString cStr)
{
	CString strGetText;
	GetDlgItemText(IDC_EDIT_INPUT, strGetText);

	CString strSetText;

	if( m_bIsInput )
		strSetText.Format(_T("%s%s"), strGetText, cStr);
	else
		strSetText.Format(_T("%s"), cStr);

	SetDlgItemText(IDC_EDIT_INPUT, strSetText);

	m_bIsInput = TRUE;
}

void CVirtualKeyBoard::DeleteChar()
{
	CString strGetText;
	GetDlgItemText(IDC_EDIT_INPUT, strGetText);

	if(strGetText.GetLength() == 0) return;

	CString strSetText;
	strSetText.Format(_T("%s"), strGetText.Left(strGetText.GetLength() - 1));
	SetDlgItemText(IDC_EDIT_INPUT, strSetText);
}

void CVirtualKeyBoard::SetValueString(bool bIsPassword, CString strValue)
{
	m_strValue = strValue;
	m_strInitValue = strValue;
	m_bIsPassword = bIsPassword;
}

void CVirtualKeyBoard::CustomDraw(int nOffsetX, int nOffsetY)
{
	m_nOffsetX=nOffsetX; 
	m_nOffsetY=nOffsetY;
}

void CVirtualKeyBoard::ChangeCharLowerCase()
{
	if(!m_bLanguage)
	{
		if(m_bCapsLock)
		{
			m_btnKeyQ.SetText("Q");
			m_btnKeyW.SetText("W");
			m_btnKeyE.SetText("E");
			m_btnKeyR.SetText("R");
			m_btnKeyT.SetText("T");
			m_btnKeyY.SetText("Y");
			m_btnKeyU.SetText("U");
			m_btnKeyI.SetText("I");
			m_btnKeyO.SetText("O");
			m_btnKeyP.SetText("P");
			m_btnKeyA.SetText("A");
			m_btnKeyS.SetText("S");
			m_btnKeyD.SetText("D");
			m_btnKeyF.SetText("F");
			m_btnKeyG.SetText("G");
			m_btnKeyH.SetText("H");
			m_btnKeyJ.SetText("J");
			m_btnKeyK.SetText("K");
			m_btnKeyL.SetText("L");
			m_btnKeyZ.SetText("Z");
			m_btnKeyX.SetText("X");
			m_btnKeyC.SetText("C");
			m_btnKeyV.SetText("V");
			m_btnKeyB.SetText("B");
			m_btnKeyN.SetText("N");
			m_btnKeyM.SetText("M");
			m_btnKeyRudot.SetText("`");
			m_btnKey1.SetText("1");
			m_btnKey2.SetText("2");
			m_btnKey3.SetText("3");
			m_btnKey4.SetText("4");
			m_btnKey5.SetText("5");
			m_btnKey6.SetText("6");
			m_btnKey7.SetText("7");
			m_btnKey8.SetText("8");
			m_btnKey9.SetText("9");
			m_btnKey0.SetText("0");
			m_btnKeyMinus.SetText("-");
			m_btnKeyEqual.SetText("=");
			m_btnKeyLb.SetText("[");
			m_btnKeyRb.SetText("]");
			m_btnKeyRs.SetText("\\");
			m_btnKeySc.SetText(";");
			m_btnKeyRuc.SetText("'");
			m_btnKeyComma.SetText(",");
			m_btnKeyDot.SetText(".");
			m_btnKeySlash.SetText("/");
		}
		else
		{
			m_btnKeyQ.SetText("q");
			m_btnKeyW.SetText("w");
			m_btnKeyE.SetText("e");
			m_btnKeyR.SetText("r");
			m_btnKeyT.SetText("t");
			m_btnKeyY.SetText("y");
			m_btnKeyU.SetText("u");
			m_btnKeyI.SetText("i");
			m_btnKeyO.SetText("o");
			m_btnKeyP.SetText("p");
			m_btnKeyA.SetText("a");
			m_btnKeyS.SetText("s");
			m_btnKeyD.SetText("d");
			m_btnKeyF.SetText("f");
			m_btnKeyG.SetText("g");
			m_btnKeyH.SetText("h");
			m_btnKeyJ.SetText("j");
			m_btnKeyK.SetText("k");
			m_btnKeyL.SetText("l");
			m_btnKeyZ.SetText("z");
			m_btnKeyX.SetText("x");
			m_btnKeyC.SetText("c");
			m_btnKeyV.SetText("v");
			m_btnKeyB.SetText("b");
			m_btnKeyN.SetText("n");
			m_btnKeyM.SetText("m");		
			m_btnKeyRudot.SetText("~");
			m_btnKey1.SetText("!");
			m_btnKey2.SetText("@");
			m_btnKey3.SetText("#");
			m_btnKey4.SetText("$");
			m_btnKey5.SetText("%");
			m_btnKey6.SetText("^");
			m_btnKey7.SetText("&");
			m_btnKey8.SetText("*");
			m_btnKey9.SetText("(");
			m_btnKey0.SetText(")");
			m_btnKeyMinus.SetText("_");
			m_btnKeyEqual.SetText("+");
			m_btnKeyLb.SetText("{");
			m_btnKeyRb.SetText("}");
			m_btnKeyRs.SetText("|");
			m_btnKeySc.SetText(":");
			m_btnKeyRuc.SetText("\"");
			m_btnKeyComma.SetText("<");
			m_btnKeyDot.SetText(">");
			m_btnKeySlash.SetText("?");
		}
	}
	else
	{
		if(m_bCapsLock)
		{
			m_btnKeyQ.SetText("ㅃ");
			m_btnKeyW.SetText("ㅉ");
			m_btnKeyE.SetText("ㄸ");
			m_btnKeyR.SetText("ㄲ");
			m_btnKeyT.SetText("ㅆ");
			m_btnKeyY.SetText("ㅛ");
			m_btnKeyU.SetText("ㅕ");
			m_btnKeyI.SetText("ㅑ");
			m_btnKeyO.SetText("ㅒ");
			m_btnKeyP.SetText("ㅖ");
			m_btnKeyA.SetText("ㅁ");
			m_btnKeyS.SetText("ㄴ");
			m_btnKeyD.SetText("ㅇ");
			m_btnKeyF.SetText("ㄹ");
			m_btnKeyG.SetText("ㅎ");
			m_btnKeyH.SetText("ㅗ");
			m_btnKeyJ.SetText("ㅓ");
			m_btnKeyK.SetText("ㅏ");
			m_btnKeyL.SetText("ㅣ");
			m_btnKeyZ.SetText("ㅋ");
			m_btnKeyX.SetText("ㅌ");
			m_btnKeyC.SetText("ㅊ");
			m_btnKeyV.SetText("ㅍ");
			m_btnKeyB.SetText("ㅠ");
			m_btnKeyN.SetText("ㅜ");
			m_btnKeyM.SetText("ㅡ");
		}
		else
		{
			m_btnKeyQ.SetText("ㅂ");
			m_btnKeyW.SetText("ㅈ");
			m_btnKeyE.SetText("ㄷ");
			m_btnKeyR.SetText("ㄱ");
			m_btnKeyT.SetText("ㅅ");
			m_btnKeyY.SetText("ㅛ");
			m_btnKeyU.SetText("ㅕ");
			m_btnKeyI.SetText("ㅑ");
			m_btnKeyO.SetText("ㅐ");
			m_btnKeyP.SetText("ㅔ");
			m_btnKeyA.SetText("ㅁ");
			m_btnKeyS.SetText("ㄴ");
			m_btnKeyD.SetText("ㅇ");
			m_btnKeyF.SetText("ㄹ");
			m_btnKeyG.SetText("ㅎ");
			m_btnKeyH.SetText("ㅗ");
			m_btnKeyJ.SetText("ㅓ");
			m_btnKeyK.SetText("ㅏ");
			m_btnKeyL.SetText("ㅣ");
			m_btnKeyZ.SetText("ㅋ");
			m_btnKeyX.SetText("ㅌ");
			m_btnKeyC.SetText("ㅊ");
			m_btnKeyV.SetText("ㅍ");
			m_btnKeyB.SetText("ㅠ");
			m_btnKeyN.SetText("ㅜ");
			m_btnKeyM.SetText("ㅡ");
		}
	}
}

void CVirtualKeyBoard::OnBnClickedButtonRudot()
{	
	if(m_bCapsLock)	InsertChar('`');
	else InsertChar('~');
}


void CVirtualKeyBoard::OnBnClickedButton1()
{	
	if(m_bCapsLock)	InsertChar('1');
	else InsertChar('!');
}


void CVirtualKeyBoard::OnBnClickedButton2()
{	
	if(m_bCapsLock)	InsertChar('2');
	else InsertChar('@');
}


void CVirtualKeyBoard::OnBnClickedButton3()
{	
	if(m_bCapsLock)	InsertChar('3');
	else InsertChar('#');
}


void CVirtualKeyBoard::OnBnClickedButton4()
{	
	if(m_bCapsLock)	InsertChar('4');
	else InsertChar('$');
}


void CVirtualKeyBoard::OnBnClickedButton5()
{	
	if(m_bCapsLock)	InsertChar('5');
	else InsertChar('%');
}


void CVirtualKeyBoard::OnBnClickedButton6()
{	
	if(m_bCapsLock)	InsertChar('6');
	else InsertChar('^');
}


void CVirtualKeyBoard::OnBnClickedButton7()
{	
	if(m_bCapsLock)	InsertChar('7');
	else InsertChar('&');
}


void CVirtualKeyBoard::OnBnClickedButton8()
{	
	if(m_bCapsLock)	InsertChar('8');
	else InsertChar('*');
}


void CVirtualKeyBoard::OnBnClickedButton9()
{	
	if(m_bCapsLock)	InsertChar('9');
	else InsertChar('(');
}


void CVirtualKeyBoard::OnBnClickedButton0()
{	
	if(m_bCapsLock)	InsertChar('0');
	else InsertChar(')');
}


void CVirtualKeyBoard::OnBnClickedButtonMinus()
{	
	if(m_bCapsLock)InsertChar('-');	
	else InsertChar('_');
}


void CVirtualKeyBoard::OnBnClickedButtonEqual()
{	
	if(m_bCapsLock)	InsertChar('=');
	else InsertChar('+');
}


void CVirtualKeyBoard::OnBnClickedButtonBacksp()
{
	if(!m_bLanguage)	DeleteChar();
	else AppendText(-3);
}


void CVirtualKeyBoard::OnBnClickedButtonTab()
{
	InsertChar("        ");
}


void CVirtualKeyBoard::OnBnClickedButtonQ()
{
	if(m_bCapsLock)	InsertChar('Q');
	else InsertChar('q');

	//AppendText(7);

}


void CVirtualKeyBoard::OnBnClickedButtonW()
{
	
	if(!m_bLanguage)
	{
		if(m_bCapsLock)	InsertChar('W');
		else InsertChar('w');
	}
	else AppendText(12);
}


void CVirtualKeyBoard::OnBnClickedButtonE()
{
	if(!m_bLanguage)
	{
		if(m_bCapsLock)	InsertChar('E');
		else InsertChar('e');
	}
	else	AppendText(3);
}


void CVirtualKeyBoard::OnBnClickedButtonR()
{
	if(!m_bLanguage)
	{
		if(m_bCapsLock)	InsertChar('R');
		else InsertChar('r');
	}
	else	AppendText(0);
}


void CVirtualKeyBoard::OnBnClickedButtonT()
{
	if(!m_bLanguage)
	{
		if(m_bCapsLock)	InsertChar('T');
		else InsertChar('t');
	}
	else	AppendText(9);
}


void CVirtualKeyBoard::OnBnClickedButtonY()
{
	if(!m_bLanguage)
	{
		if(m_bCapsLock)	InsertChar('Y');
		else InsertChar('y');
	}
	else	AppendText(27);
}


void CVirtualKeyBoard::OnBnClickedButtonU()
{
	if(!m_bLanguage)
	{
		if(m_bCapsLock)	InsertChar('U');
		else InsertChar('u');
	}
	else	AppendText(23);
}


void CVirtualKeyBoard::OnBnClickedButtonI()
{
	if(!m_bLanguage)
	{
		if(m_bCapsLock)	InsertChar('I');
		else InsertChar('i');
	}
	else	AppendText(19);
}


void CVirtualKeyBoard::OnBnClickedButtonO()
{

	if(m_bCapsLock)	InsertChar('O');
	else InsertChar('o');
}


void CVirtualKeyBoard::OnBnClickedButtonP()
{
	
	if(m_bCapsLock)	InsertChar('P');
	else InsertChar('p');
}		

void CVirtualKeyBoard::OnBnClickedButtonLb()
{	
	if(m_bCapsLock)	InsertChar('[');
	else InsertChar('{');
}


void CVirtualKeyBoard::OnBnClickedButtonRb()
{	
	if(m_bCapsLock)	InsertChar(']');
	else InsertChar('}');
}


void CVirtualKeyBoard::OnBnClickedButtonRs()
{	
	if(m_bCapsLock)	InsertChar('\\');
	else InsertChar('|');
}


void CVirtualKeyBoard::OnBnClickedButtonCr()
{
	m_bCapsLock = !m_bCapsLock;

	if(m_bCapsLock)		m_btnKeyCr.SetColorBkg(255, COLOR_BTN_SELECT);
	else				m_btnKeyCr.SetColorBkg(255, COLOR_BTN_BODY);

	ChangeCharLowerCase();
}


void CVirtualKeyBoard::OnBnClickedButtonA()
{
	if(!m_bLanguage)
	{
		if(m_bCapsLock)	InsertChar('A');
		else InsertChar('a');
	}
	else	AppendText(6);
}


void CVirtualKeyBoard::OnBnClickedButtonS()
{
	if(!m_bLanguage)
	{
		if(m_bCapsLock)	InsertChar('S');
		else InsertChar('s');
	}
	else	AppendText(2);
}


void CVirtualKeyBoard::OnBnClickedButtonD()
{
	if(!m_bLanguage)
	{
		if(m_bCapsLock)	InsertChar('D');
		else InsertChar('d');
	}
	else	AppendText(11);
}


void CVirtualKeyBoard::OnBnClickedButtonF()
{
	if(!m_bLanguage)
	{
		if(m_bCapsLock)	InsertChar('F');
		else InsertChar('f');
	}
	else	AppendText(5);
}


void CVirtualKeyBoard::OnBnClickedButtonG()
{
	if(!m_bLanguage)
	{
		if(m_bCapsLock)	InsertChar('G');
		else InsertChar('g');
	}
	else AppendText(18);
}


void CVirtualKeyBoard::OnBnClickedButtonH()
{
	if(!m_bLanguage)
	{
		if(m_bCapsLock)	InsertChar('H');
		else InsertChar('h');
	}
	else AppendText(27);
}


void CVirtualKeyBoard::OnBnClickedButtonJ()
{	
	if(!m_bLanguage)
	{
		if(m_bCapsLock)	InsertChar('J');
		else InsertChar('j');
	}
	else	AppendText(23);
}


void CVirtualKeyBoard::OnBnClickedButtonK()
{
	if(!m_bLanguage)
	{
		if(m_bCapsLock)	InsertChar('K');
		else InsertChar('k');
	}
	else	AppendText(19);
}


void CVirtualKeyBoard::OnBnClickedButtonL()
{	
	if(!m_bLanguage)
	{
		if(m_bCapsLock)	InsertChar('L');
		else InsertChar('l');
	}
	else	AppendText(39);
}


void CVirtualKeyBoard::OnBnClickedButtonSc()
{	
	if(m_bCapsLock)	InsertChar(';');
	else InsertChar(':');
}


void CVirtualKeyBoard::OnBnClickedButtonRuc()
{	
	if(m_bCapsLock)	InsertChar('\'');
	else InsertChar('"');
}


void CVirtualKeyBoard::OnBnClickedButtonEnter()
{
	GetDlgItemText(IDC_EDIT_INPUT, m_strValue);

	CDialogEx::OnOK();
}


void CVirtualKeyBoard::OnBnClickedButtonLshift()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CVirtualKeyBoard::OnBnClickedButtonZ()
{	
	if(!m_bLanguage)
	{
		if(m_bCapsLock)	InsertChar('Z');
		else InsertChar('z');
	}
	else	AppendText(0);
}


void CVirtualKeyBoard::OnBnClickedButtonX()
{	
	if(!m_bLanguage)
	{
		if(m_bCapsLock)	InsertChar('X');
		else InsertChar('x');
	}
	else	AppendText(3);
}


void CVirtualKeyBoard::OnBnClickedButtonC()
{	
	if(!m_bLanguage)
	{
		if(m_bCapsLock)	InsertChar('C');
		else InsertChar('c');
	}
	else	AppendText(12);
}


void CVirtualKeyBoard::OnBnClickedButtonV()
{
	if(!m_bLanguage)
	{
		if(m_bCapsLock) InsertChar('V');
		else InsertChar('v');
	}
	else	AppendText(7);
}


void CVirtualKeyBoard::OnBnClickedButtonB()
{
	if(!m_bLanguage)
	{
		if(m_bCapsLock)	InsertChar('B');
		else InsertChar('b');
	}
	else	AppendText(32);
}


void CVirtualKeyBoard::OnBnClickedButtonN()
{
	if(!m_bLanguage)
	{
		if(m_bCapsLock)	InsertChar('N');
		else InsertChar('n');
	}
	else	AppendText(32);
}


void CVirtualKeyBoard::OnBnClickedButtonM()
{
	if(!m_bLanguage)
	{
	if(m_bCapsLock)	InsertChar('M');
	else InsertChar('m');
	}
	else AppendText(39);
}

void CVirtualKeyBoard::OnBnClickedButtonComma()
{	
	if(m_bCapsLock)	InsertChar(',');
	else InsertChar('<');
}


void CVirtualKeyBoard::OnBnClickedButtonDot()
{	
	if(m_bCapsLock)	InsertChar('.');
	else InsertChar('>');
}


void CVirtualKeyBoard::OnBnClickedButtonSlash()
{	
	if(m_bCapsLock)	InsertChar('/');
	else InsertChar('?');
}


void CVirtualKeyBoard::OnBnClickedButtonRshift()
{
	m_ctrlNumberText.SetFocus();


}


void CVirtualKeyBoard::OnBnClickedButtonLctrl()
{
	m_ctrlNumberText.SetFocus();
}


void CVirtualKeyBoard::OnBnClickedButtonAlt()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CVirtualKeyBoard::OnBnClickedButtonSpace()
{
	InsertChar(' ');
}


void CVirtualKeyBoard::OnBnClickedButtonHe()
{
	m_bLanguage = !m_bLanguage;

	if(m_bLanguage)		m_btnKeyHe.SetColorBkg(255, COLOR_BTN_SELECT);
	else m_btnKeyHe.SetColorBkg(255, COLOR_BTN_BODY);

	ChangeCharLowerCase();
}

void CVirtualKeyBoard::OnBnClickedButtonExit()
{
	m_strValue = m_strInitValue;
	CDialogEx::OnCancel();
}


void CVirtualKeyBoard::OnBnClickedButtonClear()
{
	m_automataKr.Clear();
	m_ctrlNumberText.SetSel(0,-1);
	m_ctrlNumberText.Clear();
}


BOOL CVirtualKeyBoard::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	return FALSE;
	//return CDialogEx::OnEraseBkgnd(pDC);
}

void CVirtualKeyBoard::AppendText( int nCode )
{

	// 문자열 입력
	m_automataKr.SetKeyCode(nCode);

	CString strText = m_automataKr.completeText;

	if(m_automataKr.ingWord != NULL)
	{
		strText += m_automataKr.ingWord;
	}
	SetDlgItemText(IDC_EDIT_INPUT, strText);

	// Edit Focus 처리
	CEdit * pEdit = ((CEdit*)GetDlgItem(IDC_EDIT_INPUT));
	pEdit->SetSel(pEdit->GetWindowTextLength(),pEdit->GetWindowTextLength());
	pEdit->SetFocus();
}

bool CVirtualKeyBoard::GetValue(CString &strNumber)
{
	strNumber = m_strValue;
	return true;
}