#pragma once

#include "afxwin.h"
#include "LET_AlignClient.h"

// Keyboard 대화 상자입니다.

/////////////////////////////////////////////////////////////////////////////
// CKeyPadDlg dialog

class CKeyPadDlg : public CDialog
{
// Construction
public:
	CKeyPadDlg(CWnd* pParent = NULL);   // standard constructor

	void	InsertChar(int nIndex);
	void	DeleteChar();
	void	SetValueString(bool bIsPassword, CString strValue);
	bool	GetNumber(double nMinValue, double nMaxValue, CString &strNumber);
	bool	GetValue(CString &strNumber);
	void	CustomDraw(int nOffsetX, int nOffsetY);

	CFont	m_Font;
	bool	m_bIsDispCopyData;

// Dialog Data
	//{{AFX_DATA(CKeyPadDlg)
	enum { IDD = IDD_DIALOG_KEYPAD };
	CButtonEx m_btnKey1;
	CButtonEx m_btnKey2;
	CButtonEx m_btnKey3;
	CButtonEx m_btnKey4;
	CButtonEx m_btnKey5;
	CButtonEx m_btnKey6;
	CButtonEx m_btnKey7;
	CButtonEx m_btnKey8;
	CButtonEx m_btnKey9;
	CButtonEx m_btnKey0;
	CButtonEx m_btnKeyDot;
	CButtonEx m_btnKeyBS;
	CButtonEx m_btnKeyOk;
	CButtonEx m_btnKeyCancel;

	CButtonEx m_btnKeyClear;
	CButtonEx m_btnKeyPlusMinus;
	CButtonEx m_btnKeyAdd;
	CButtonEx m_btnKeySub;
	CButtonEx m_btnKeyEqual;

	//}}AFX_DATA
	
private:
	CString m_strInitValue;
	CString m_strValue;
	double m_dValue;
	bool m_bIsPassword;

	BOOL m_bCustomDrawPos;
	int m_nOffsetX;
	int m_nOffsetY;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKeyPadDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CKeyPadDlg)
	afx_msg void OnKey1();
	afx_msg void OnKey2();
	afx_msg void OnKey3();
	afx_msg void OnKey4();
	afx_msg void OnKey5();
	afx_msg void OnKey6();
	afx_msg void OnKey7();
	afx_msg void OnKey8();
	afx_msg void OnKey9();
	afx_msg void OnKey0();
	afx_msg void OnKeyDot();
	afx_msg void OnKeyBackSpace();
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg void OnBnClickedBtnClear();
	afx_msg void OnBnClickedKeyPlusMinus();
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnSub();
	afx_msg void OnBnClickedBtnEqual();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnNcHitTest( CPoint point );

	DECLARE_MESSAGE_MAP()
public:
	HBRUSH m_hbrBkg;
	CEdit m_ctrlNumberText;
	CEdit m_calculText;
	/*CLabelEx m_ctrlNumberText;
	CLabelEx m_calculText;*/
	

	bool bCheck;

	BOOL m_bIsInput;
	double m_dbValue1;
	double m_dbValue2;
	double m_dbTotalValue;
	int m_nOperator;
	void MainButtonInit(CButtonEx *pbutton, int size = 15);
	
};