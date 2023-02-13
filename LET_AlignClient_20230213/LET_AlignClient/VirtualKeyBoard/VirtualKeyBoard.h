#pragma once

#include "EditEx.h"
#include "AutomataKR.h"
// CVirtualKeyBoard 대화 상자입니다.

class CVirtualKeyBoard : public CDialogEx
{
	DECLARE_DYNAMIC(CVirtualKeyBoard)

public:
	CVirtualKeyBoard(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CVirtualKeyBoard();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_VIRTUAL_KEYBOARD_DIALOG };

	CButtonEx m_btnKeyRudot;
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
	CButtonEx m_btnKeyMinus;
	CButtonEx m_btnKeyEqual;
	CButtonEx m_btnKeyBacksp;
	CButtonEx m_btnKeyTab;
	CButtonEx m_btnKeyQ;
	CButtonEx m_btnKeyW;
	CButtonEx m_btnKeyE;
	CButtonEx m_btnKeyR;
	CButtonEx m_btnKeyT;
	CButtonEx m_btnKeyY;
	CButtonEx m_btnKeyU;
	CButtonEx m_btnKeyI;
	CButtonEx m_btnKeyO;
	CButtonEx m_btnKeyP;
	CButtonEx m_btnKeyLb;
	CButtonEx m_btnKeyRb;
	CButtonEx m_btnKeyRs;
	CButtonEx m_btnKeyCr;
	CButtonEx m_btnKeyA;
	CButtonEx m_btnKeyS;
	CButtonEx m_btnKeyD;
	CButtonEx m_btnKeyF;
	CButtonEx m_btnKeyG;
	CButtonEx m_btnKeyH;
	CButtonEx m_btnKeyJ;
	CButtonEx m_btnKeyK;
	CButtonEx m_btnKeyL;
	CButtonEx m_btnKeySc;
	CButtonEx m_btnKeyRuc;
	CButtonEx m_btnKeyEnter;
	CButtonEx m_btnKeyLshift;
	CButtonEx m_btnKeyZ;
	CButtonEx m_btnKeyX;
	CButtonEx m_btnKeyC;
	CButtonEx m_btnKeyV;
	CButtonEx m_btnKeyB;
	CButtonEx m_btnKeyN;
	CButtonEx m_btnKeyM;
	CButtonEx m_btnKeyComma;
	CButtonEx m_btnKeyDot;
	CButtonEx m_btnKeySlash;
	CButtonEx m_btnKeyRshift;
	CButtonEx m_btnKeyLctrl;
	CButtonEx m_btnKeyAlt;
	CButtonEx m_btnKeySpace;
	CButtonEx m_btnKeyHe;
	CButtonEx m_btnKeyExit;
	CButtonEx m_btnKeyClear;

	CEditEx m_ctrlNumberText;
	HBRUSH m_hbrBkg;
	CFont	m_Font;
	BOOL m_bIsInput;
	bool m_bIsPassword;

	CString m_strInitValue;
	CString m_strValue;
	CString m_strInputString;

	int m_nOffsetX;
	int m_nOffsetY;

	BOOL m_bCapsLock;
	BOOL m_bLanguage;
	BOOL m_bShift;
	BOOL m_bCtrl;
	BOOL m_bAlt;
private:
	CAutomataKR m_automataKr;	// Automata

public:
	void DeleteChar();
	void InsertChar(int nIndex);
	void InsertChar(CString cStr);
	void CustomDraw(int nOffsetX, int nOffsetY);
	void SetValueString(bool bIsPassword, CString strValue);
	void ChangeCharLowerCase();
	void AppendText( int nCode );

	bool GetValue(CString &strNumber);

	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonRudot();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButton9();
	afx_msg void OnBnClickedButton0();
	afx_msg void OnBnClickedButtonMinus();
	afx_msg void OnBnClickedButtonEqual();
	afx_msg void OnBnClickedButtonBacksp();
	afx_msg void OnBnClickedButtonTab();
	afx_msg void OnBnClickedButtonQ();
	afx_msg void OnBnClickedButtonW();
	afx_msg void OnBnClickedButtonE();
	afx_msg void OnBnClickedButtonR();
	afx_msg void OnBnClickedButtonT();
	afx_msg void OnBnClickedButtonY();
	afx_msg void OnBnClickedButtonU();
	afx_msg void OnBnClickedButtonI();
	afx_msg void OnBnClickedButtonO();
	afx_msg void OnBnClickedButtonP();
	afx_msg void OnBnClickedButtonLb();
	afx_msg void OnBnClickedButtonRb();
	afx_msg void OnBnClickedButtonRs();
	afx_msg void OnBnClickedButtonCr();
	afx_msg void OnBnClickedButtonA();
	afx_msg void OnBnClickedButtonS();
	afx_msg void OnBnClickedButtonD();
	afx_msg void OnBnClickedButtonF();
	afx_msg void OnBnClickedButtonG();
	afx_msg void OnBnClickedButtonH();
	afx_msg void OnBnClickedButtonJ();
	afx_msg void OnBnClickedButtonK();
	afx_msg void OnBnClickedButtonL();
	afx_msg void OnBnClickedButtonSc();
	afx_msg void OnBnClickedButtonRuc();
	afx_msg void OnBnClickedButtonEnter();
	afx_msg void OnBnClickedButtonLshift();
	afx_msg void OnBnClickedButtonZ();
	afx_msg void OnBnClickedButtonX();
	afx_msg void OnBnClickedButtonC();
	afx_msg void OnBnClickedButtonV();
	afx_msg void OnBnClickedButtonB();
	afx_msg void OnBnClickedButtonN();
	afx_msg void OnBnClickedButtonM();
	afx_msg void OnBnClickedButtonComma();
	afx_msg void OnBnClickedButtonDot();
	afx_msg void OnBnClickedButtonSlash();
	afx_msg void OnBnClickedButtonRshift();
	afx_msg void OnBnClickedButtonLctrl();
	afx_msg void OnBnClickedButtonAlt();
	afx_msg void OnBnClickedButtonSpace();
	afx_msg void OnBnClickedButtonHe();
	afx_msg void OnBnClickedButtonExit();

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnNcHitTest( CPoint point );

	void MainButtonInit(CButtonEx *pbutton, int size = 15);
	afx_msg void OnBnClickedButtonClear();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
