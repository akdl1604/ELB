// LoginDlg.cpp: 구현 파일
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "LoginDlg.h"
#include "afxdialogex.h"
#include "OnscreenKeyboardDlg.h"
#include "VirtualKeyBoard/VirtualKeyBoard.h"

// CLoginDlg 대화 상자

IMPLEMENT_DYNAMIC(CLoginDlg, CDialogEx)

CLoginDlg::CLoginDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_LOGIN, pParent)
{
	 m_nSelPos = 0;
}

CLoginDlg::~CLoginDlg()
{
}

void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LIST_REGIST_ID, m_ListLogInID);
	DDX_Control(pDX, IDC_STATIC_LOGIN_INFO, m_stt_LogInInfo);
	DDX_Control(pDX, IDC_STATIC_LOGIN_NAME, m_stt_LogIn_Name);
	DDX_Control(pDX, IDC_STATIC_LOGIN_PASSWORD, m_stt_LoginPassword);
	DDX_Control(pDX, IDC_STATIC_LOGIN_LEVEL, m_stt_LogIn_Level);

	DDX_Control(pDX, IDC_EDIT_LOGIN_NAME, m_edt_LoginName); 
	DDX_Control(pDX, IDC_EDIT_LOGIN_PASSWORD, m_edt_LoginPassword);
	DDX_Control(pDX, IDC_COMBO_LEVEL, m_cmbLoginLevel);

	DDX_Control(pDX, IDC_BTN_CLOSE, m_btnLoginDlgClose);
	DDX_Control(pDX, IDC_BTN_CREATE, m_btnCreateUser);
	DDX_Control(pDX, IDC_BTN_DELETE, m_btnDeleteUser);
	DDX_Control(pDX, IDC_BTN_LOGIN, m_btnLogin);
}


BEGIN_MESSAGE_MAP(CLoginDlg, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_CREATE, &CLoginDlg::OnBnClickedBtnCreate)	
	ON_BN_CLICKED(IDC_BTN_CLOSE, &CLoginDlg::OnBnClickedBtnClose)
	ON_BN_CLICKED(IDC_BTN_LOGIN, &CLoginDlg::OnBnClickedBtnLogin)
	ON_NOTIFY(NM_CLICK, IDC_LIST_REGIST_ID, &CLoginDlg::OnNMClickListModel)
	ON_EN_SETFOCUS(IDC_EDIT_LOGIN_NAME, &CLoginDlg::OnEnSetfocusEditLogInName)
	ON_EN_SETFOCUS(IDC_EDIT_LOGIN_PASSWORD, &CLoginDlg::OnEnSetfocusEditLogInPassword)
	ON_BN_CLICKED(IDC_BTN_DELETE, &CLoginDlg::OnBnClickedBtnDelete)
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CLoginDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);
	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	m_pMain = (CLET_AlignClientDlg*)AfxGetMainWnd();

	m_dbUserDataBase.InitDataBase(m_pMain->m_strSystemDir);
	m_dbUserDataBase.CreateDataBase();

	InitTitle(&m_stt_LogInInfo, "Login Information", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_LogIn_Name, "LogIn_Name", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_LoginPassword, "Password", 14.f, COLOR_BTN_BODY);	
	InitTitle(&m_stt_LogIn_Level, "LogIn_Level", 14.f, COLOR_BTN_BODY);

	EditButtonInit(&m_edt_LoginName, 20);
	EditButtonInit(&m_edt_LoginPassword, 20);

	MainButtonInit(&m_btnLogin);
	MainButtonInit(&m_btnCreateUser);
	MainButtonInit(&m_btnDeleteUser);
	MainButtonInit(&m_btnLoginDlgClose);

	initListCtrl();
	getUserList();

	m_edt_LoginName.SetText("");
	m_edt_LoginPassword.SetPasswordChar('*');
	m_cmbLoginLevel.SetCurSel(m_pMain->m_nLogInUserLevelType);
	
	m_btnCreateUser.SetEnable(FALSE);
	m_btnDeleteUser.SetEnable(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

// CLoginDlg 메시지 처리기
void CLoginDlg::MainButtonInit(CButtonEx* pbutton, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}

HBRUSH CLoginDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	if (pWnd->m_hWnd == this->m_hWnd) hbr = m_hbrBkg;

	return hbr;
}

void CLoginDlg::InitTitle(CLabelEx* pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}

void CLoginDlg::EditButtonInit(CEditEx* pbutton, int size, COLORREF color)
{
	pbutton->SetSizeText(size);				// 글자 크기
	pbutton->SetStyleTextBold(true);		// 글자 스타일
	pbutton->SetTextMargins(10, 10);		// 글자 옵셋
	pbutton->SetColorText(color);	// 글자 색상
	pbutton->SetText(_T("0"));				// 글자 설정
}

void CLoginDlg::initListCtrl()
{
	LV_COLUMN lstCol;

	DWORD style = m_ListLogInID.GetExtendedStyle();
	style |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	m_ListLogInID.SetExtendedStyle(style);

	lstCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lstCol.fmt = LVCFMT_CENTER;

	lstCol.cx = 0;
	lstCol.iSubItem = 0;
	lstCol.pszText = "";
	m_ListLogInID.InsertColumn(0, &lstCol);

	lstCol.cx = 120;
	lstCol.iSubItem = 1;
	lstCol.pszText = "LEVEL";
	m_ListLogInID.InsertColumn(1, &lstCol);

	lstCol.cx = 180;
	lstCol.iSubItem = 2;
	lstCol.pszText = "USER NAME";
	m_ListLogInID.InsertColumn(2, &lstCol);
}

void CLoginDlg::OnNMClickListModel(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	POSITION pos = m_ListLogInID.GetFirstSelectedItemPosition();
	if (pos == 0) return;

	for (int i = 0; i < m_ListLogInID.GetItemCount(); i++)
	{
		m_ListLogInID.SetRowBgColor(i, COLOR_WHITE);
		m_ListLogInID.SetRowTxtColor(i, COLOR_BLACK);
		m_ListLogInID.SetRowStyle(i, LIS_BGCOLOR | LIS_TXTCOLOR);
	}

	int nPos = m_ListLogInID.GetNextSelectedItem(pos);

	m_ListLogInID.SetRowBgColor(nPos, RGB(51, 153, 255), FALSE);
	m_ListLogInID.SetRowTxtColor(nPos, RGB(255, 255, 255));
	m_ListLogInID.SetRowStyle(nPos, LIS_BGCOLOR | LIS_TXTCOLOR);

	if (nPos >= 0)
	{
		m_SelstrUserName = m_ListLogInID.GetItemText(m_nSelPos, 2);
		m_edt_LoginName.SetText(m_SelstrUserName);
	}

	m_nSelPos = nPos;

	*pResult = 0;
}

void CLoginDlg::OnBnClickedBtnCreate()
{
	_st_LOGIN_DATA user;
	user._userLevel = m_cmbLoginLevel.GetCurSel();
	user._userName = m_edt_LoginName.GetText();
	user._userPassword = m_edt_LoginPassword.GetText();

	if (user._userName.empty() || user._userPassword.empty())
	{
		AfxMessageBox("Input name And password !!!");
		return;
	}

	m_dbUserDataBase.insertLogInDataBase(user);

	theLog.logmsg(LOG_LOGIN, "[Insert] UserName : %s ", user._userName);

	getUserList();
}


void CLoginDlg::OnBnClickedBtnClose()
{
	CDialogEx::OnCancel();
}

void CLoginDlg::OnBnClickedBtnLogin()
{
	int level = -1;
	string userName = m_edt_LoginName.GetText();
	string userpassword = m_edt_LoginPassword.GetText();

	if (userName.empty() || userpassword.empty())
	{
		AfxMessageBox("Input name And password !!!");
		return;
	}
 	else if (m_ListLogInID.GetItemCount() == 0)
	{
		if (userName == "HBS" && userpassword == "hbskrcom12@")
		{
			level = 0;

			AfxMessageBox("Master Level Access Success !!!");

			theLog.logmsg(LOG_LOGIN, "[LogIn] Master Create Level");
		}
		else
		{
			level = 2;

			AfxMessageBox("Master Level Access fail !!!");
		}

		if (level == 0)
		{
			m_btnCreateUser.SetEnable(TRUE);
			m_btnDeleteUser.SetEnable(TRUE);
		}
		else
		{
			m_btnCreateUser.SetEnable(FALSE);
			m_btnDeleteUser.SetEnable(FALSE);
		}

		m_cmbLoginLevel.SetCurSel(level);
		m_pMain->m_nLogInUserLevelType = level;

	}
	else if (m_dbUserDataBase.getLogInDataBase(userName, userpassword, level))
	{
		if (level == 0)
		{
			m_btnCreateUser.SetEnable(TRUE);
			m_btnDeleteUser.SetEnable(TRUE);
		}
		else
		{
			m_btnCreateUser.SetEnable(FALSE);
			m_btnDeleteUser.SetEnable(FALSE);
		}

		//m_edt_LoginName.SetText("");
		m_edt_LoginPassword.SetText("");
		m_cmbLoginLevel.SetCurSel(level);
		m_pMain->m_nLogInUserLevelType = level;

		::SendMessageA(m_pMain->m_pForm[FORM_MAIN]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_AUTO_LOGOFF_ADMIN_MODE, NULL);

		AfxMessageBox("Log In Success !!!");
		
		if (m_pMain->m_nLogInUserLevelType != USER)
		{
			theLog.logmsg(LOG_LOGIN, "[LogIn] Change UserName : %s", userName);
		}
		else
		{
			theLog.logmsg(LOG_LOGIN, "[LogIn] UserName : %s", userName);
		}
	}
	else
	{
		if (m_pMain->m_nLogInUserLevelType != USER)
		{
			theLog.logmsg(LOG_LOGIN, "[LogOut] LogOff");
		}

		m_pMain->m_nLogInUserLevelType = USER;
		AfxMessageBox("Log In fail !!!");
	}

	::SendMessageA(m_pMain->m_pForm[FORM_MAIN]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_LOGIN_STATUS_DISPLAY, 0);
}

void CLoginDlg::OnEnSetfocusEditLogInName()
{
	SetFocus();

	CVirtualKeyBoard m_pKeyboardDlg;

	if (m_pKeyboardDlg.DoModal() != IDOK)
		return;

	CString strNumber;
	m_pKeyboardDlg.GetValue(strNumber);
	GetDlgItem(IDC_EDIT_LOGIN_NAME)->SetWindowTextA(strNumber);

}

void CLoginDlg::OnEnSetfocusEditLogInPassword()
{
	SetFocus();

	CVirtualKeyBoard m_pKeyboardDlg;

	m_pKeyboardDlg.m_bIsPassword = true;

	if (m_pKeyboardDlg.DoModal() != IDOK)
		return;

	CString strNumber;
	m_pKeyboardDlg.GetValue(strNumber);
	GetDlgItem(IDC_EDIT_LOGIN_PASSWORD)->SetWindowTextA(strNumber);
}

BOOL CLoginDlg::getUserList()
{
	BOOL bSuccess = TRUE;

	try {
		_st_LOGIN_DATA logInList[200];

		bool bread = m_dbUserDataBase.getLogInListDataBase(logInList);

		m_ListLogInID.DeleteAllItems();

		LV_ITEM lstItem;
		lstItem.mask = LVIF_TEXT;
		char cData[MAX_PATH];
		string strtemp;

		for (int i=0;i<200;i++)
		{
			if (logInList[i]._userName.empty()) continue;

			lstItem.iItem = i;
			lstItem.iSubItem = 0;
			lstItem.pszText = "";
			m_ListLogInID.InsertItem(&lstItem);

			lstItem.iSubItem = 1;
			if (logInList[i]._userLevel == 0)
			{
				strtemp = "Master";
			}
			else if (logInList[i]._userLevel == 1)
			{
				strtemp = "Super";
			}
			else
			{
				strtemp = "User";
			}
			sprintf(cData, "%s", strtemp.c_str());
			lstItem.pszText = cData;
			m_ListLogInID.SetItem(&lstItem);

			lstItem.iSubItem = 2;
			sprintf(cData, "%s", logInList[i]._userName.c_str());
			lstItem.pszText = cData;
			m_ListLogInID.SetItem(&lstItem);
		}
	}
	catch (...)
	{
		bSuccess = FALSE;
	}

	return bSuccess;
}

void CLoginDlg::OnBnClickedBtnDelete()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_pMain->m_nLogInUserLevelType != MASTER)
	{
		AfxMessageBox("Do not able User level !!!");
		return;
	}

	int level = m_pMain->m_nLogInUserLevelType;
	string userName = m_edt_LoginName.GetText();
	string userpassword = m_edt_LoginPassword.GetText();

	if (userName.empty() || userpassword.empty())
	{
		AfxMessageBox("Input name And password !!!");
		return;
	}
	else if (m_dbUserDataBase.getLogInDataBase(userName, userpassword, level))
	{
		m_dbUserDataBase.deleteLogInDataBase(userName);

		theLog.logmsg(LOG_LOGIN, "[Delete] UserName : %s ", userName);
	}
	else AfxMessageBox("Delete User fail !!!");

	getUserList();
}


void CLoginDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	//switch (nIDEvent)
	//{

	//}
	CDialogEx::OnTimer(nIDEvent);
}
