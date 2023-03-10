#pragma once
#include <string>
#include "Calculator.h"
#include <math.h>

#include "EditEx.h"

#define SQRT 1
#define FACTORIAL 2
#define SQUARE 3
#define TENSQUARE 4
#define SIN 5
#define COS 6
#define TAN 7
#define LOG 8
#define FOUNTAIN 9
#define LOGEXP 10
#define ASIN 11
#define ACOS 12
#define ATAN 13
#define LN 14
#define DEGREES 15

#define KINDS 0
#define X_NSQUARE 1
#define MOD 2
#define EXP 3
#define X_SQUARE_Y 4
// An_engineering_calculator 대화 상자입니다.

class An_engineering_calculator : public CDialogEx
{
	DECLARE_DYNAMIC(An_engineering_calculator)

public:
	An_engineering_calculator(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~An_engineering_calculator();

	HBRUSH m_hbrBkg;
// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_AN_ENGINEERING_CALCULATOR };
#endif

public:
	void Clear();
	void btn_Clear();
	void PostFixExpression(string infixExpression);
	void InFixExpression(string ntext, bool Number = true);
	void EditTextPrint(CString ntext, bool Number = true);
	void Calculus(CString str1, CString str2, int kinds);
	void TwoOperation();
	void SetFirstNumber();

	double Result(string postfixExpression);
	double Factorial(double number);
	double Square(double number);
	double TenSquare(double number);
	double GetDegree(double number);
	double GetRadian(double number);
	double GetSin(double number);
	double GetCos(double number);
	double GetTan(double number);
	double GetLog(double number);
	double GetFountain(double number);
	double GetLogEXP(double number);
	
	CEditEx m_EditHistory;
	CEditEx m_EditOutput;

private:
	string m_Number;
	string m_oldNumber;
	string m_infixExpression;
	string m_postfixExpression;
	string m_strEmpty;

	CString m_HistoryText;
	CString m_Output;
	CString m_cstrEmpty;

	bool m_toggle = false;
	bool m_countNumber = false;
	bool m_Invalid = false;
	bool m_mode = false;
	bool m_operate = false;
	int m_kinds = 0;
	int m_totalparenthesis = 0;
	double m_oldFirstNum;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonEqure();
	afx_msg void OnBnClickedButtonPlus();
	afx_msg void OnBnClickedButtonMinus();
	afx_msg void OnBnClickedButtonMulti();
	afx_msg void OnBnClickedButtonNanugi();
	afx_msg void OnBnClickedButtonLeftG();
	afx_msg void OnBnClickedButtonRigthG();
	afx_msg void OnBnClickedButtonZero();
	afx_msg void OnBnClickedButtonOne();
	afx_msg void OnBnClickedButtonTwo();
	afx_msg void OnBnClickedButtonThree();
	afx_msg void OnBnClickedButtonFour();
	afx_msg void OnBnClickedButtonFive();
	afx_msg void OnBnClickedButtonSix();
	afx_msg void OnBnClickedButtonSeven();
	afx_msg void OnBnClickedButtonEhigh();
	afx_msg void OnBnClickedButtonNine();
	afx_msg void OnBnClickedButtonJum();
	afx_msg void OnBnClickedButtonClear();
	afx_msg void OnBnClickedButtonBack();
	afx_msg void OnBnClickedButtonCe();
	afx_msg void OnBnClickedButtonPAndM();
	afx_msg void OnBnClickedButtonSqurt();
	afx_msg void OnBnClickedButtonFactoryal();
	afx_msg void OnBnClickedButtonPai();
	afx_msg void OnBnClickedButtonZegop();
	afx_msg void OnBnClickedButtonYzegop();
	afx_msg void OnBnClickedButtonTenX();
	afx_msg void OnBnClickedButtonSin();
	afx_msg void OnBnClickedButtonCos();
	afx_msg void OnBnClickedButtonTan();
	afx_msg void OnBnClickedButtonLog();
	afx_msg void OnBnClickedButtonExp();
	afx_msg void OnBnClickedButtonMod();
	afx_msg void OnBnClickedButtonUp();
	afx_msg void OnBnClickedButtonCopy();
	afx_msg void OnBnClickedButtonPaste();

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	void InitTitle(CLabelEx* pTitle, CString str, float size, COLORREF color);
	void MainButtonInit(CButtonEx* pbutton, int size = 5);
	void EditButtonInit(CEditEx* pbutton, int size, COLORREF color = RGB(255, 0, 0));

	CButtonEx  m_btnButtonEqure;
	CButtonEx  m_btnButtonPlus;
	CButtonEx  m_btnButtonMinus;
	CButtonEx  m_btnButtonMulti;
	CButtonEx  m_btnButtonNanugi;
	CButtonEx  m_btnButtonLeftG;
	CButtonEx  m_btnButtonRigthG;
	CButtonEx  m_btnButtonZero;
	CButtonEx  m_btnButtonOne;
	CButtonEx  m_btnButtonTwo;
	CButtonEx  m_btnButtonThree;
	CButtonEx  m_btnButtonFour;
	CButtonEx  m_btnButtonFive;
	CButtonEx  m_btnButtonSix;
	CButtonEx  m_btnButtonSeven;
	CButtonEx  m_btnButtonEhigh;
	CButtonEx  m_btnButtonNine;
	CButtonEx  m_btnButtonJum;
	CButtonEx  m_btnButtonClear;
	CButtonEx  m_btnButtonBack;
	CButtonEx  m_btnButtonCe;
	CButtonEx  m_btnButtonPAndM;
	CButtonEx  m_btnButtonSqurt;
	CButtonEx  m_btnButtonFactoryal;
	CButtonEx  m_btnButtonPai;
	CButtonEx  m_btnButtonZegop;
	CButtonEx  m_btnButtonYzegop;
	CButtonEx  m_btnButtonTenX;
	CButtonEx  m_btnButtonSin;
	CButtonEx  m_btnButtonCos;
	CButtonEx  m_btnButtonTan;
	CButtonEx  m_btnButtonLog;
	CButtonEx  m_btnButtonExp;
	CButtonEx  m_btnButtonMod;
	CButtonEx  m_btnButtonUp;
	CButtonEx  m_btnButtonCopy;
	CButtonEx  m_btnButtonPaste;
};
