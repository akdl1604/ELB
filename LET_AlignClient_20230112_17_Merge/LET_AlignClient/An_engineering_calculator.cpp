// An_engineering_calculator.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "An_engineering_calculator.h"
#include "afxdialogex.h"

// An_engineering_calculator 대화 상자입니다.

IMPLEMENT_DYNAMIC(An_engineering_calculator, CDialogEx)

// 클립보드에 문자열을 넣는다.


An_engineering_calculator::An_engineering_calculator(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_AN_ENGINEERING_CALCULATOR, pParent)
{
	m_infixExpression = "";
	m_postfixExpression = "";
	m_Number = "";
	m_HistoryText = "";
	m_Output = "";
	m_cstrEmpty = "";

	m_countNumber = false;
	m_toggle = false;
	m_Invalid = false;
	m_mode = false;
	m_operate = false;
	m_kinds = 0;
	m_oldFirstNum = 0;
	m_totalparenthesis = 0;
}

An_engineering_calculator::~An_engineering_calculator()
{
}

void An_engineering_calculator::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_HISTORY, m_HistoryText);
	DDX_Control(pDX, IDC_EDIT_HISTORY, m_EditHistory);
	DDX_Text(pDX, IDC_EDIT_FIRST_PRINT, m_Output);
	DDX_Control(pDX, IDC_EDIT_FIRST_PRINT, m_EditOutput);
	DDX_Control(pDX, IDC_BUTTON_EQURE, m_btnButtonEqure);
	DDX_Control(pDX, IDC_BUTTON_PLUS, m_btnButtonPlus);
	DDX_Control(pDX, IDC_BUTTON_MINUS, m_btnButtonMinus);
	DDX_Control(pDX, IDC_BUTTON_MULTI, m_btnButtonMulti);
	DDX_Control(pDX, IDC_BUTTON_NANUGI, m_btnButtonNanugi);
	DDX_Control(pDX, IDC_BUTTON_LEFT_G, m_btnButtonLeftG);
	DDX_Control(pDX, IDC_BUTTON_RIGTH_G, m_btnButtonRigthG);
	DDX_Control(pDX, IDC_BUTTON_ZERO, m_btnButtonZero);
	DDX_Control(pDX, IDC_BUTTON_ONE, m_btnButtonOne);
	DDX_Control(pDX, IDC_BUTTON_TWO, m_btnButtonTwo);
	DDX_Control(pDX, IDC_BUTTON_THREE, m_btnButtonThree);
	DDX_Control(pDX, IDC_BUTTON_FOUR, m_btnButtonFour);
	DDX_Control(pDX, IDC_BUTTON_FIVE, m_btnButtonFive);
	DDX_Control(pDX, IDC_BUTTON_SIX, m_btnButtonSix);
	DDX_Control(pDX, IDC_BUTTON_SEVEN, m_btnButtonSeven);
	DDX_Control(pDX, IDC_BUTTON_EHIGH, m_btnButtonEhigh);
	DDX_Control(pDX, IDC_BUTTON_NINE, m_btnButtonNine);
	DDX_Control(pDX, IDC_BUTTON_JUM, m_btnButtonJum);
	DDX_Control(pDX, IDC_BUTTON_CLEAR, m_btnButtonClear);
	DDX_Control(pDX, IDC_BUTTON_BACK, m_btnButtonBack);
	DDX_Control(pDX, IDC_BUTTON_CE, m_btnButtonCe);
	DDX_Control(pDX, IDC_BUTTON_P_AND_M, m_btnButtonPAndM);
	DDX_Control(pDX, IDC_BUTTON_SQURT, m_btnButtonSqurt);
	DDX_Control(pDX, IDC_BUTTON_FACTORYAL, m_btnButtonFactoryal);
	DDX_Control(pDX, IDC_BUTTON_PAI, m_btnButtonPai);
	DDX_Control(pDX, IDC_BUTTON_ZEGOP, m_btnButtonZegop);
	DDX_Control(pDX, IDC_BUTTON_YZEGOP, m_btnButtonYzegop);
	DDX_Control(pDX, IDC_BUTTON_TEN_X, m_btnButtonTenX);
	DDX_Control(pDX, IDC_BUTTON_SIN, m_btnButtonSin);
	DDX_Control(pDX, IDC_BUTTON_COS, m_btnButtonCos);
	DDX_Control(pDX, IDC_BUTTON_TAN, m_btnButtonTan);
	DDX_Control(pDX, IDC_BUTTON_LOG, m_btnButtonLog);
	DDX_Control(pDX, IDC_BUTTON_EXP, m_btnButtonExp);
	DDX_Control(pDX, IDC_BUTTON_Mod, m_btnButtonMod);
	DDX_Control(pDX, IDC_BUTTON_UP, m_btnButtonUp);
	DDX_Control(pDX, IDC_BUTTON_COPY, m_btnButtonCopy);
	DDX_Control(pDX, IDC_BUTTON_PASTE, m_btnButtonPaste);
}

BEGIN_MESSAGE_MAP(An_engineering_calculator, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_EQURE, &An_engineering_calculator::OnBnClickedButtonEqure)
	ON_BN_CLICKED(IDC_BUTTON_PLUS, &An_engineering_calculator::OnBnClickedButtonPlus)
	ON_BN_CLICKED(IDC_BUTTON_MINUS, &An_engineering_calculator::OnBnClickedButtonMinus)
	ON_BN_CLICKED(IDC_BUTTON_MULTI, &An_engineering_calculator::OnBnClickedButtonMulti)
	ON_BN_CLICKED(IDC_BUTTON_NANUGI, &An_engineering_calculator::OnBnClickedButtonNanugi)
	ON_BN_CLICKED(IDC_BUTTON_LEFT_G, &An_engineering_calculator::OnBnClickedButtonLeftG)
	ON_BN_CLICKED(IDC_BUTTON_RIGTH_G, &An_engineering_calculator::OnBnClickedButtonRigthG)
	ON_BN_CLICKED(IDC_BUTTON_ZERO, &An_engineering_calculator::OnBnClickedButtonZero)
	ON_BN_CLICKED(IDC_BUTTON_ONE, &An_engineering_calculator::OnBnClickedButtonOne)
	ON_BN_CLICKED(IDC_BUTTON_TWO, &An_engineering_calculator::OnBnClickedButtonTwo)
	ON_BN_CLICKED(IDC_BUTTON_THREE, &An_engineering_calculator::OnBnClickedButtonThree)
	ON_BN_CLICKED(IDC_BUTTON_FOUR, &An_engineering_calculator::OnBnClickedButtonFour)
	ON_BN_CLICKED(IDC_BUTTON_FIVE, &An_engineering_calculator::OnBnClickedButtonFive)
	ON_BN_CLICKED(IDC_BUTTON_SIX, &An_engineering_calculator::OnBnClickedButtonSix)
	ON_BN_CLICKED(IDC_BUTTON_SEVEN, &An_engineering_calculator::OnBnClickedButtonSeven)
	ON_BN_CLICKED(IDC_BUTTON_EHIGH, &An_engineering_calculator::OnBnClickedButtonEhigh)
	ON_BN_CLICKED(IDC_BUTTON_NINE, &An_engineering_calculator::OnBnClickedButtonNine)
	ON_BN_CLICKED(IDC_BUTTON_JUM, &An_engineering_calculator::OnBnClickedButtonJum)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &An_engineering_calculator::OnBnClickedButtonClear)
	ON_BN_CLICKED(IDC_BUTTON_BACK, &An_engineering_calculator::OnBnClickedButtonBack)
	ON_BN_CLICKED(IDC_BUTTON_CE, &An_engineering_calculator::OnBnClickedButtonCe)
	ON_BN_CLICKED(IDC_BUTTON_P_AND_M, &An_engineering_calculator::OnBnClickedButtonPAndM)
	ON_BN_CLICKED(IDC_BUTTON_SQURT, &An_engineering_calculator::OnBnClickedButtonSqurt)
	ON_BN_CLICKED(IDC_BUTTON_FACTORYAL, &An_engineering_calculator::OnBnClickedButtonFactoryal)
	ON_BN_CLICKED(IDC_BUTTON_PAI, &An_engineering_calculator::OnBnClickedButtonPai)
	ON_BN_CLICKED(IDC_BUTTON_ZEGOP, &An_engineering_calculator::OnBnClickedButtonZegop)
	ON_BN_CLICKED(IDC_BUTTON_YZEGOP, &An_engineering_calculator::OnBnClickedButtonYzegop)
	ON_BN_CLICKED(IDC_BUTTON_TEN_X, &An_engineering_calculator::OnBnClickedButtonTenX)
	ON_BN_CLICKED(IDC_BUTTON_SIN, &An_engineering_calculator::OnBnClickedButtonSin)
	ON_BN_CLICKED(IDC_BUTTON_COS, &An_engineering_calculator::OnBnClickedButtonCos)
	ON_BN_CLICKED(IDC_BUTTON_TAN, &An_engineering_calculator::OnBnClickedButtonTan)
	ON_BN_CLICKED(IDC_BUTTON_LOG, &An_engineering_calculator::OnBnClickedButtonLog)
	ON_BN_CLICKED(IDC_BUTTON_EXP, &An_engineering_calculator::OnBnClickedButtonExp)
	ON_BN_CLICKED(IDC_BUTTON_Mod, &An_engineering_calculator::OnBnClickedButtonMod)
	ON_BN_CLICKED(IDC_BUTTON_UP, &An_engineering_calculator::OnBnClickedButtonUp)
	ON_BN_CLICKED(IDC_BUTTON_COPY, &An_engineering_calculator::OnBnClickedButtonCopy)
	ON_BN_CLICKED(IDC_BUTTON_PASTE, &An_engineering_calculator::OnBnClickedButtonPaste)
END_MESSAGE_MAP()

BOOL An_engineering_calculator::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);

	MainButtonInit(&m_btnButtonEqure);
	MainButtonInit(&m_btnButtonPlus);
	MainButtonInit(&m_btnButtonMinus);
	MainButtonInit(&m_btnButtonMulti);
	MainButtonInit(&m_btnButtonNanugi);
	MainButtonInit(&m_btnButtonLeftG);
	MainButtonInit(&m_btnButtonRigthG);
	MainButtonInit(&m_btnButtonZero);
	MainButtonInit(&m_btnButtonOne);
	MainButtonInit(&m_btnButtonTwo);
	MainButtonInit(&m_btnButtonThree);
	MainButtonInit(&m_btnButtonFour);
	MainButtonInit(&m_btnButtonFive);
	MainButtonInit(&m_btnButtonSix);
	MainButtonInit(&m_btnButtonSeven);
	MainButtonInit(&m_btnButtonEhigh);
	MainButtonInit(&m_btnButtonNine);
	MainButtonInit(&m_btnButtonJum);
	MainButtonInit(&m_btnButtonClear);
	MainButtonInit(&m_btnButtonBack);
	MainButtonInit(&m_btnButtonCe);
	MainButtonInit(&m_btnButtonPAndM);
	MainButtonInit(&m_btnButtonSqurt);
	MainButtonInit(&m_btnButtonFactoryal);
	MainButtonInit(&m_btnButtonPai);
	MainButtonInit(&m_btnButtonZegop);
	MainButtonInit(&m_btnButtonYzegop);
	MainButtonInit(&m_btnButtonTenX);
	MainButtonInit(&m_btnButtonSin);
	MainButtonInit(&m_btnButtonCos);
	MainButtonInit(&m_btnButtonTan);
	MainButtonInit(&m_btnButtonLog);
	MainButtonInit(&m_btnButtonExp);
	MainButtonInit(&m_btnButtonMod);
	MainButtonInit(&m_btnButtonUp);
	MainButtonInit(&m_btnButtonCopy);
	MainButtonInit(&m_btnButtonPaste);

	EditButtonInit(&m_EditHistory, 20);
	EditButtonInit(&m_EditOutput, 20);

	return TRUE;
}

HBRUSH An_engineering_calculator::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (pWnd->GetDlgCtrlID() == IDC_EDIT_HISTORY ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_FIRST_PRINT)
	{
		pDC->SetBkColor(COLOR_UI_BODY);
		pDC->SetTextColor(COLOR_WHITE);
	}

	return m_hbrBkg;
}

void An_engineering_calculator::InitTitle(CLabelEx* pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}


void An_engineering_calculator::MainButtonInit(CButtonEx * pbutton, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}

void An_engineering_calculator::EditButtonInit(CEditEx* pbutton, int size, COLORREF color)
{
	pbutton->SetSizeText(size);				// 글자 크기
	pbutton->SetStyleTextBold(true);		// 글자 스타일
	pbutton->SetTextMargins(10, 10);		// 글자 옵셋
	pbutton->SetColorText(color);	// 글자 색상
	pbutton->SetText(_T("0"));				// 글자 설정
}

void An_engineering_calculator::InFixExpression(string ntext, bool Number/*1*/) //string 변수에 대입
{
	if (Number) //숫자 클릭, "."클릭
	{
		m_Number += ntext;
		m_operate = true;
		m_Invalid = false;
	}
	else if (ntext == "=" && Number == false) //"="클릭
	{
		m_infixExpression += m_Number;
	}
	else 
	{
		m_infixExpression += (m_Number + ntext);
		m_toggle = false;
		m_Number = "";
	}

	if (m_countNumber)
	{
		TwoOperation();
	}
}

void An_engineering_calculator::TwoOperation()
{
	double number;
	double result = 1;
	string newNumber;

	number = atof(m_Number.c_str());

	switch (m_kinds)
	{
	case X_NSQUARE:
		for (double i = 0; i < number; i++)
		{
			result *= m_oldFirstNum;
		}
		break;

	case MOD:
		//visual studio 에서 제공해주는 %연산자는 실수만 가능
		//math.h 에서 제공해주는 fmod함수를 사용하여 정수도 가능하도록 변경
		result = fmod(m_oldFirstNum, number);
		break;

	case EXP:
		for (double i = 0; i < number; i++)
		{
			m_oldFirstNum *= 10;
		}
		result = m_oldFirstNum;
		break;

	case X_SQUARE_Y:
		result = pow(number, (1.0 / m_oldFirstNum));
		break;

	default:
		break;
	}

	newNumber = to_string(result);
	m_Number.clear();
	m_Number = newNumber;

	m_kinds = KINDS;
	m_countNumber = false;
}

void An_engineering_calculator::PostFixExpression(string infixExpression) //정렬
{
	m_postfixExpression = GetPostFix(infixExpression);
}

double An_engineering_calculator::Result(string postfixExpression) //결과도출
{
	double result;

	result = Calculate(postfixExpression);

	return result;
}

void An_engineering_calculator::OnBnClickedButtonEqure() //"="
{
	double result;
	CString str;
	str = "";

	InFixExpression("=", false);
	EditTextPrint(str, false);

	if (m_totalparenthesis > 0 || m_Invalid)
	{
		MessageBox(m_HistoryText + _T("\nInvalid formula."), _T("Warning"), MB_ICONEXCLAMATION | MB_ICONWARNING);
		btn_Clear();
		return;
	}

	PostFixExpression(m_infixExpression);

	result = Result(m_postfixExpression);
	m_Output.Format(_T("%f"), result);

	SetDlgItemText(IDC_EDIT_FIRST_PRINT, m_Output);

	Clear();
}

void An_engineering_calculator::EditTextPrint(CString ntext, bool Number/*1*/) //버튼 클릭시 EditBoxPrint
{
	if (Number)
	{
		m_HistoryText += "";
		SetDlgItemText(IDC_EDIT_HISTORY, m_HistoryText);
		m_Output += ntext;
		SetDlgItemText(IDC_EDIT_FIRST_PRINT, m_Output);
	}
	else
	{
		m_HistoryText += (m_Output + ntext);
		SetDlgItemText(IDC_EDIT_HISTORY, m_HistoryText);
		m_Output = "";
		SetDlgItemText(IDC_EDIT_FIRST_PRINT, m_Output);
	}
}

void An_engineering_calculator::OnBnClickedButtonPlus() //"+"
{
	if (m_operate == false)
	{
		return;
	}

	CString str;
	str = "+";
	
	InFixExpression((LPSTR)(LPCTSTR)str, false);
	EditTextPrint(str, false);

	m_Invalid = true;
	m_operate = false;
}

void An_engineering_calculator::OnBnClickedButtonMinus() //"-"
{
	if (m_operate == false)
	{
		return;
	}

	CString str;
	str = "-";

	InFixExpression((LPSTR)(LPCTSTR)str, false);
	EditTextPrint(str, false);

	m_Invalid = true;
	m_operate = false;
}

void An_engineering_calculator::OnBnClickedButtonMulti() //"*"
{
	if (m_operate == false)
	{
		return;
	}

	CString str;
	str = "*";

	InFixExpression((LPSTR)(LPCTSTR)str, false);
	EditTextPrint(str, false);

	m_Invalid = true;
	m_operate = false;
}

void An_engineering_calculator::OnBnClickedButtonNanugi() //"/"
{
	if (m_operate == false)
	{
		return;
	}

	CString str;
	str = "/";

	InFixExpression((LPSTR)(LPCTSTR)str, false);
	EditTextPrint(str, false);

	m_Invalid = true;
	m_operate = false;
}

void An_engineering_calculator::OnBnClickedButtonLeftG() //"("
{
	CString str;
	str = "(";

	m_totalparenthesis++;

	InFixExpression((LPSTR)(LPCTSTR)str, false);
	EditTextPrint(str, false);
}

void An_engineering_calculator::OnBnClickedButtonRigthG() //")"
{
	if (m_totalparenthesis <= 0)
	{
		return;
	}

	CString str;
	str = ")";

	m_totalparenthesis--;

	InFixExpression((LPSTR)(LPCTSTR)str, false);
	EditTextPrint(str, false);
}

void An_engineering_calculator::OnBnClickedButtonZero() //0
{
	CString str;
	str = "0";

	InFixExpression((LPSTR)(LPCTSTR)str);
	EditTextPrint(str);
}

void An_engineering_calculator::OnBnClickedButtonOne() //1
{
	CString str;
	str = "1";

	InFixExpression((LPSTR)(LPCTSTR)str);
	EditTextPrint(str);
}

void An_engineering_calculator::OnBnClickedButtonTwo() //2
{
	CString str;
	str = "2";

	InFixExpression((LPSTR)(LPCTSTR)str);
	EditTextPrint(str);
}

void An_engineering_calculator::OnBnClickedButtonThree() //3
{
	CString str;
	str = "3";

	InFixExpression((LPSTR)(LPCTSTR)str);
	EditTextPrint(str);
}

void An_engineering_calculator::OnBnClickedButtonFour() //4
{
	CString str;
	str = "4";

	InFixExpression((LPSTR)(LPCTSTR)str);
	EditTextPrint(str);
}

void An_engineering_calculator::OnBnClickedButtonFive() //5
{
	CString str;
	str = "5";

	InFixExpression((LPSTR)(LPCTSTR)str);
	EditTextPrint(str);
}

void An_engineering_calculator::OnBnClickedButtonSix() //6
{
	CString str;
	str = "6";

	InFixExpression((LPSTR)(LPCTSTR)str);
	EditTextPrint(str);
}

void An_engineering_calculator::OnBnClickedButtonSeven() //7
{
	CString str;
	str = "7";

	InFixExpression((LPSTR)(LPCTSTR)str);
	EditTextPrint(str);
}

void An_engineering_calculator::OnBnClickedButtonEhigh() //8
{
	CString str;
	str = "8";

	InFixExpression((LPSTR)(LPCTSTR)str);
	EditTextPrint(str);
}

void An_engineering_calculator::OnBnClickedButtonNine() //9
{
	CString str;
	str = "9";

	InFixExpression((LPSTR)(LPCTSTR)str);
	EditTextPrint(str);
}

void An_engineering_calculator::OnBnClickedButtonJum() //"."
{
	if (m_Number == m_strEmpty)
	{
		return;
	}

	CString str;
	str = ".";

	InFixExpression((LPSTR)(LPCTSTR)str);
	EditTextPrint(str);
}

void An_engineering_calculator::Clear()
{
	m_Output = "";
	m_HistoryText = "";
	m_infixExpression.clear();
	m_postfixExpression.clear();
	m_Number.clear();

	m_countNumber = false;
	m_toggle = false;
	m_Invalid = false;
	m_operate = false;
	m_kinds = 0;
	m_oldFirstNum = 0;
	m_totalparenthesis = 0;
}

void An_engineering_calculator::btn_Clear()
{
	Clear();
	SetDlgItemText(IDC_EDIT_HISTORY, m_HistoryText);
	SetDlgItemText(IDC_EDIT_FIRST_PRINT, m_Output);
}

void An_engineering_calculator::OnBnClickedButtonClear() //C
{
	btn_Clear();
}

void An_engineering_calculator::OnBnClickedButtonBack() //←
{
	int n_size, output_length; //n_size : string 변수, output_length : CString 변수

	n_size = int(m_Number.size());
	if (n_size <= 0) return;
	m_Number.erase(n_size - 1, 1); //문자 삭제

	output_length = m_Output.GetLength();
	m_Output.Delete(output_length - 1, 1); //문자 삭제

	SetDlgItemText(IDC_EDIT_FIRST_PRINT, m_Output);
}

void An_engineering_calculator::OnBnClickedButtonCe() //CE
{
	m_Number.clear();
	m_Output = "";
	SetDlgItemText(IDC_EDIT_FIRST_PRINT, m_Output);
}

void An_engineering_calculator::Calculus(CString str1, CString str2, int kinds)
{
	double number;
	string newNumber;

	m_Number == m_strEmpty ? m_Number = "0" : m_Number;
	number = atof(m_Number.c_str()); //c_str() : string의 멤버함수로써 string -> char*로 변환
									 //atof() : char* -> double로 변환
	//newNumber = to_string(number); //double -> string 변환

	switch (kinds)
	{
	case SQRT:
		newNumber = to_string(sqrt(number));
		break;

	case FACTORIAL:
		newNumber = to_string(Factorial(number));
		break;

	case SQUARE:
		newNumber = to_string(Square(number));
		break;

	case TENSQUARE:
		newNumber = to_string(TenSquare(number));
		break;

	case SIN:
	case ASIN:
		newNumber = to_string(GetSin(number));
		break;

	case COS:
	case ACOS:
		newNumber = to_string(GetCos(number));
		break;

	case TAN:
	case ATAN:
		newNumber = to_string(GetTan(number));
		break;

	case LOG:
	case LN:
		newNumber = to_string(GetLog(number));
		break;

	case FOUNTAIN:
		newNumber = to_string(GetFountain(number));
		break;

	case LOGEXP:
		newNumber = to_string(GetLogEXP(number));
		break;

	case DEGREES:
		newNumber = to_string(GetDegree(number));
		break;

	default:
		break;
	}
	
	m_Number.clear();
	m_Number = newNumber;

	m_Output == m_cstrEmpty ? m_Output = str1 + "0" + str2 : m_Output = str1 + m_Output + str2;
	SetDlgItemText(IDC_EDIT_FIRST_PRINT, m_Output);
}

double An_engineering_calculator::Factorial(double number)
{
	if (number == 1 || number == 0)
	{
		return 1;
	}
	return number * Factorial(number - 1);
}

double An_engineering_calculator::Square(double number)
{
	return number * number;
}

double An_engineering_calculator::TenSquare(double number)
{
	double result = 1;

	for (double i = 0; i < number; i++)
	{
		result *= 10;
	}

	return result;
}

double An_engineering_calculator::GetDegree(double number)
{
	return number * (3.14159265 / 180.0);
}

double An_engineering_calculator::GetRadian(double number)
{
	return number * (180.0 / 3.14159265);
}

double An_engineering_calculator::GetSin(double number)
{
	if (m_mode)
	{
		return GetRadian(asin(number)); // Rad로 표시됨
	}
	else
	{
		return sin(GetDegree(number));;
	}
}

double An_engineering_calculator::GetCos(double number)
{
	if (m_mode)
	{
		return GetRadian(acos(number));
	}
	else
	{
		return cos(GetDegree(number));
	}
}

double An_engineering_calculator::GetTan(double number)
{
	if (m_mode)
	{
		return GetRadian(atan(number));
	}
	else
	{
		return tan(GetDegree(number));
	}
}

double An_engineering_calculator::GetLog(double number)
{
	return m_mode == true ? log(number) : log10(number);
}

double An_engineering_calculator::GetFountain(double number)
{
	return 1.0 / number;
}

double An_engineering_calculator::GetLogEXP(double number)
{
	return exp(number);
}

void An_engineering_calculator::OnBnClickedButtonSqurt() 
{
	if (m_mode) // 1/x
	{
		if (m_Number == m_strEmpty)
		{
			CString str;

			str = "Input is invalid.";
			SetDlgItemText(IDC_EDIT_FIRST_PRINT, str);
			return;
		}

		CString str1, str2;

		str1 = "1/(";
		str2 = ")";

		Calculus(str1, str2, FOUNTAIN);
	}
	else //√
	{
		CString str1, str2;

		str1 = "√(";
		str2 = ")";

		Calculus(str1, str2, SQRT);
	}
}

void An_engineering_calculator::OnBnClickedButtonFactoryal() //n! (자연수만가능)
{
	CString str1, str2;

	str1 = "fact(";
	str2 = ")";

	Calculus(str1, str2, FACTORIAL);
}

void An_engineering_calculator::OnBnClickedButtonPai() //pi
{
	CString str1;
	str1 = "π";

	InFixExpression("3.14159265");
	EditTextPrint(str1);
}

void An_engineering_calculator::OnBnClickedButtonZegop() //Square(x^2)
{
	CString str1, str2;

	str1 = "(";
	str2 = ")²";

	Calculus(str1, str2, SQUARE);
}

void An_engineering_calculator::OnBnClickedButtonTenX() 
{
	if (m_mode) //e^x
	{
		CString str1, str2;

		str1 = "e^(";
		str2 = ")";

		Calculus(str1, str2, LOGEXP);
	}
	else //10^n
	{
		CString str1, str2;

		str1 = "10^(";
		str2 = ")";

		Calculus(str1, str2, TENSQUARE);
	}
}

void An_engineering_calculator::OnBnClickedButtonSin() 
{
	if (m_mode) //asin
	{
		CString str1, str2;

		str1 = "sin-¹(";
		str2 = ")";

		Calculus(str1, str2, ASIN);
	}
	else //SIN
	{
		CString str1, str2;

		str1 = "sin(";
		str2 = ")";

		Calculus(str1, str2, SIN);
	}
}

void An_engineering_calculator::OnBnClickedButtonCos()
{
	if (m_mode) //acos
	{
		CString str1, str2;

		str1 = "cos-¹(";
		str2 = ")";

		Calculus(str1, str2, ACOS);
	}
	else //COS
	{
		CString str1, str2;

		str1 = "cos(";
		str2 = ")";

		Calculus(str1, str2, COS);
	}
}

void An_engineering_calculator::OnBnClickedButtonTan()
{
	if (m_mode) //atan
	{
		CString str1, str2;

		str1 = "tan-¹(";
		str2 = ")";

		Calculus(str1, str2, ATAN);
	}
	else //TAN
	{
		CString str1, str2;

		str1 = "tan(";
		str2 = ")";

		Calculus(str1, str2, TAN);
	}
}

void An_engineering_calculator::OnBnClickedButtonLog() 
{
	if (m_Number == m_strEmpty)
	{
		CString str;

		str = "Input is invalid.";
		SetDlgItemText(IDC_EDIT_FIRST_PRINT, str);
		return;
	}

	if (m_mode) //ln
	{
		CString str1, str2;

		str1 = "ln(";
		str2 = ")";

		Calculus(str1, str2, LN);
	}
	else //LOG
	{
		CString str1, str2;

		str1 = "Log(";
		str2 = ")";

		Calculus(str1, str2, LOG);
	}
}

void An_engineering_calculator::SetFirstNumber()
{
	m_oldFirstNum = atof(m_Number.c_str()); //c_str() : string의 멤버함수로써 string -> char*로 변환
									 //atof() : char* -> double로 변환
	m_countNumber = true;

	m_Number.clear();
}

void An_engineering_calculator::OnBnClickedButtonYzegop()
{
	if (m_Number == m_strEmpty)
	{
		return;
	}

	if (m_mode) //xsqrtn
	{
		string newNumber;
		CString str;

		str = "yroot";

		SetFirstNumber();
		m_kinds = X_SQUARE_Y;

		m_Output += str;
		SetDlgItemText(IDC_EDIT_FIRST_PRINT, m_Output);
	}
	else //X^n
	{
		string newNumber;
		CString str;

		str = "^";

		SetFirstNumber();
		m_kinds = X_NSQUARE;

		m_Output += str;
		SetDlgItemText(IDC_EDIT_FIRST_PRINT, m_Output);
	}
}

void An_engineering_calculator::OnBnClickedButtonMod()
{
	if (m_Number == m_strEmpty)
	{
		return;
	}

	if (m_mode) //deg Radian 값을 받아 Degree로 표시
	{
		CString str1, str2;

		str1 = "degrees(";
		str2 = ")";

		Calculus(str1, str2, DEGREES);
	}
	else  //MOD
	{
		string newNumber;
		CString str;

		str = "Mod";

		SetFirstNumber();
		m_kinds = MOD;

		m_Output += str;
		SetDlgItemText(IDC_EDIT_FIRST_PRINT, m_Output);
	}
}

void An_engineering_calculator::OnBnClickedButtonExp()
{
	if (m_Number == m_strEmpty)
	{
		return;
	}

	if (m_mode) //dms
	{
		CString str;

		str = "I do not know what function";
		SetDlgItemText(IDC_EDIT_FIRST_PRINT, str);
		Clear();
		return;
	}
	else  //EXP
	{
		string newNumber;
		CString str;

		str = "Exp";

		SetFirstNumber();
		m_kinds = EXP;

		m_Output += str;
		SetDlgItemText(IDC_EDIT_FIRST_PRINT, m_Output);
	}
}

void An_engineering_calculator::OnBnClickedButtonPAndM() //±
{
	if (m_Number == m_strEmpty)
	{
		return;
	}

	CString str1, str2;

	if (m_toggle)
	{
		m_toggle = false;
		str1 = m_cstrEmpty;
		str2 = m_cstrEmpty;
		m_Output = m_oldNumber.c_str();
		m_Number = m_oldNumber;
	}
	else
	{
		m_toggle = true;
		str1 = "(-";
		str2 = ")";
		m_oldNumber = m_Number;
		m_Number = "(0-" + m_oldNumber + ")";
	}

	m_Output = str1 + m_Output + str2;
	SetDlgItemText(IDC_EDIT_FIRST_PRINT, m_Output);
}

void An_engineering_calculator::OnBnClickedButtonUp() //↑
{
	CButtonEx* pBtn;
	m_mode == false ? m_mode = true : m_mode = false;

	if (m_mode)
	{
		pBtn = (CButtonEx*)GetDlgItem(IDC_BUTTON_SQURT);
		pBtn->SetText(_T("1/X"));
		pBtn = (CButtonEx*)GetDlgItem(IDC_BUTTON_TEN_X);
		pBtn->SetText(_T("eⁿ"));
		pBtn = (CButtonEx*)GetDlgItem(IDC_BUTTON_YZEGOP);
		pBtn->SetText(_T("ⁿ√x"));
		pBtn = (CButtonEx*)GetDlgItem(IDC_BUTTON_SIN);
		pBtn->SetText(_T("sin-¹"));
		pBtn = (CButtonEx*)GetDlgItem(IDC_BUTTON_COS);
		pBtn->SetText(_T("cos-¹"));
		pBtn = (CButtonEx*)GetDlgItem(IDC_BUTTON_TAN);
		pBtn->SetText(_T("tan-¹"));
		pBtn = (CButtonEx*)GetDlgItem(IDC_BUTTON_LOG);
		pBtn->SetText(_T("ln"));
		pBtn = (CButtonEx*)GetDlgItem(IDC_BUTTON_EXP);
		pBtn->SetText(_T("dms"));
		pBtn = (CButtonEx*)GetDlgItem(IDC_BUTTON_Mod);
		pBtn->SetText(_T("deg"));
	}
	else
	{
		pBtn = (CButtonEx*)GetDlgItem(IDC_BUTTON_SQURT);
		pBtn->SetText(_T("√"));
		pBtn = (CButtonEx*)GetDlgItem(IDC_BUTTON_TEN_X);
		pBtn->SetText(_T("10ⁿ"));
		pBtn = (CButtonEx*)GetDlgItem(IDC_BUTTON_YZEGOP);
		pBtn->SetText(_T("Xⁿ"));
		pBtn = (CButtonEx*)GetDlgItem(IDC_BUTTON_SIN);
		pBtn->SetText(_T("sin"));
		pBtn = (CButtonEx*)GetDlgItem(IDC_BUTTON_COS);
		pBtn->SetText(_T("cos"));
		pBtn = (CButtonEx*)GetDlgItem(IDC_BUTTON_TAN);
		pBtn->SetText(_T("tan"));
		pBtn = (CButtonEx*)GetDlgItem(IDC_BUTTON_LOG);
		pBtn->SetText(_T("log"));
		pBtn = (CButtonEx*)GetDlgItem(IDC_BUTTON_EXP);
		pBtn->SetText(_T("Exp"));
		pBtn = (CButtonEx*)GetDlgItem(IDC_BUTTON_Mod);
		pBtn->SetText(_T("Mod"));
	}
}

void An_engineering_calculator::OnBnClickedButtonCopy()
{
	UpdateData(TRUE);

	char* ap_string=m_Output.GetBuffer();
	int string_length = int(strlen(ap_string) + 1);

	HANDLE h_data = ::GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, string_length);

	char* p_data = (char*)::GlobalLock(h_data);
	if (NULL != p_data) {

		memcpy(p_data, ap_string, string_length);

		::GlobalUnlock(h_data);
		if (::OpenClipboard(m_hWnd)) {           
			::EmptyClipboard();                  
			::SetClipboardData(CF_TEXT, h_data); 
			::CloseClipboard();                  
		}
	}
}

void An_engineering_calculator::OnBnClickedButtonPaste()
{
	unsigned int priority_list = CF_TEXT;
	char* p_string = NULL;

	if (::GetPriorityClipboardFormat(&priority_list, 1) == CF_TEXT) {

		if (::OpenClipboard(m_hWnd)) {

			HANDLE h_clipboard_data = ::GetClipboardData(CF_TEXT);
			if (h_clipboard_data != NULL) {

				char* p_clipboard_data = (char*)::GlobalLock(h_clipboard_data);
				int string_len = int(strlen(p_clipboard_data) + 1);

				p_string = new char[string_len];
				memcpy(p_string, p_clipboard_data, string_len);
				m_Output.Format("%s", p_string);

				delete p_string;
				::GlobalUnlock(h_clipboard_data);
			}

			::CloseClipboard();
		}
	}

	
	UpdateData(FALSE);
}