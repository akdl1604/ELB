#pragma once


// CLoginDlg 대화 상자
#include "EditEx.h"
#include "ListCtrlStyled.h"
#include "LogInDataBaseClass.h"

class CLET_AlignClientDlg;

class CLoginDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLoginDlg)

public:
	CLoginDlg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CLoginDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_LOGIN };
#endif

	CListCtrlStyled m_ListLogInID;
	CLET_AlignClientDlg* m_pMain;

	CLabelEx	m_stt_LogInInfo;
	CLabelEx	m_stt_LogIn_Name;
	CLabelEx	m_stt_LoginPassword;
	CLabelEx	m_stt_LogIn_Level;

	CEditEx		m_edt_LoginName;
	CEditEx		m_edt_LoginPassword;
	CComboBox	m_cmbLoginLevel;

	CButtonEx	m_btnLoginDlgClose;
	CButtonEx	m_btnCreateUser;
	CButtonEx	m_btnDeleteUser;
	CButtonEx	m_btnLogin;

	CString m_SelstrUserName;
	HBRUSH m_hbrBkg;
	int m_nSelPos;

	CLogInDataBaseClass m_dbUserDataBase;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnNMClickListModel(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnSetfocusEditLogInName();
	afx_msg void OnEnSetfocusEditLogInPassword();

	void initListCtrl();
	BOOL getUserList();
	void InitTitle(CLabelEx* pTitle, CString str, float size, COLORREF color);
	void EditButtonInit(CEditEx* pbutton, int size, COLORREF color = RGB(255, 0, 0));
	void MainButtonInit(CButtonEx* pbutton, int size = 15);
	afx_msg void OnBnClickedBtnCreate();
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedBtnLogin();
	afx_msg void OnBnClickedBtnDelete();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
