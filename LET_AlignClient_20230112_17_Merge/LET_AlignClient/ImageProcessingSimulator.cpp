﻿// ImageProcessingSimulator.cpp: 구현 파일
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "ImageProcessingSimulator.h"
#include "afxdialogex.h"
#include "SimpleMatchScript.h"

using namespace tinyxml2;

// CImageProcessingSimulator 대화 상자
IMPLEMENT_DYNAMIC(CImageProcessingSimulator, CDialogEx)

CImageProcessingSimulator::CImageProcessingSimulator(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_IMAGE_PROCESSING, pParent)
	, m_editCmd(_T(""))
{
	m_srcImg = NULL;
	m_pView = NULL;

	m_nItemCount = 0;
}

CImageProcessingSimulator::~CImageProcessingSimulator()
{
}

void CImageProcessingSimulator::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SCRIPT_LIST, m_listCommandScriptList);
	DDX_Control(pDX, IDC_LIST_SCRIPT_HISTORY, m_listCommandHistorytList);
	DDX_Text(pDX, IDC_EDIT_SCRIPT_RUN, m_editCmd);
	DDX_Control(pDX, IDC_EDIT_SCRIPT_RUN, m_editCmdControl);
	DDX_Control(pDX, IDC_STATIC_SCRIPT_LIST, m_staticScriptList);
	DDX_Control(pDX, IDC_STATIC_SCRIPT_COMMAND, m_staticScriptCommand);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_BUTTON_PROCESSING, m_btnProcessing);
	DDX_Control(pDX, IDC_BUTTON_PROCESSING_CLEAR, m_btnProcessingClear);
	DDX_Control(pDX, IDC_BUTTON_PROCESSING_GET, m_btnProcessingGet);
	DDX_Control(pDX, IDC_BUTTON_XML_CLEAR, m_btnDeleteFile);
}


BEGIN_MESSAGE_MAP(CImageProcessingSimulator, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_PROCESSING, &CImageProcessingSimulator::OnBnClickedButtonProcessing)
	ON_BN_CLICKED(IDC_BUTTON_PROCESSING_CLEAR, &CImageProcessingSimulator::OnBnClickedButtonProcessingClear)
	ON_WM_LBUTTONDBLCLK()
	ON_LBN_DBLCLK(IDC_LIST_SCRIPT_LIST, &CImageProcessingSimulator::OnLbnDblclkListScriptList)
	ON_BN_CLICKED(IDC_BUTTON_PROCESSING_GET, &CImageProcessingSimulator::OnBnClickedButtonProcessingGet)
	ON_LBN_DBLCLK(IDC_LIST_SCRIPT_HISTORY, &CImageProcessingSimulator::OnLbnDblclkListScriptHistory)
	ON_BN_CLICKED(IDC_BUTTON_XML_CLEAR, &CImageProcessingSimulator::OnBnClickedButtonXmlClear)
END_MESSAGE_MAP()


// CImageProcessingSimulator 메시지 처리기
BOOL CImageProcessingSimulator::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.	
	m_pMain = (CLET_AlignClientDlg*)::AfxGetMainWnd();
	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);

	m_listCommandScriptList.AddString("  - reload");
	m_listCommandScriptList.AddString("  - save=filename");
	m_listCommandScriptList.AddString("  - gray");
	m_listCommandScriptList.AddString("  - erode=X,Y");
	m_listCommandScriptList.AddString("  - dilate=X,Y");
	m_listCommandScriptList.AddString("  - morphologyEx=filtersize,method  :erode=0,dilate=1,open=2,close=3,gradient=4,tophat=5,blackhat=6");
	m_listCommandScriptList.AddString("  - roi = 0, 0, 100, 100");
	m_listCommandScriptList.AddString("  - roiCopy = 0, 0, 100, 100");
	m_listCommandScriptList.AddString("  - rect = 0, 0, 100, 100");
	m_listCommandScriptList.AddString("  - scale = 1.5");
	m_listCommandScriptList.AddString("  - threshold = 100");
	m_listCommandScriptList.AddString("  - otsu");
	m_listCommandScriptList.AddString("  - adapthreshold = blocksize, threshold");
	m_listCommandScriptList.AddString("  - invert = threshold");
	m_listCommandScriptList.AddString("  - cvt = bgr - hsv / bgr - hls / bgr - gray / hsv - bgr / hls - bgr / gray - bgr");
	m_listCommandScriptList.AddString("  - hsv = 80, 100, 100, 90, 255, 255");
	m_listCommandScriptList.AddString("  - hls = 80, 100, 100, 90, 255, 255");
	m_listCommandScriptList.AddString("  - bgr = 10, 20, 30");
	m_listCommandScriptList.AddString("  - acc");
	m_listCommandScriptList.AddString("  - shiftAdd=shift,dir : Up=1, Right=2, Down=3, Left=4");
	m_listCommandScriptList.AddString("  - shiftSub=shift,dir : Up=1, Right=2, Down=3, Left=4");
	m_listCommandScriptList.AddString("  - hough");
	m_listCommandScriptList.AddString("  - canny = threshold");
	m_listCommandScriptList.AddString("  - findcontours = minsize");
	m_listCommandScriptList.AddString("  - gaussian");
	m_listCommandScriptList.AddString("  - medianblur");
	m_listCommandScriptList.AddString("  - removenoise=colr,minsize : white=0,black=1");
	m_listCommandScriptList.AddString("  - sobel = 0~7 :Up=0,Down=1,Left=2,Right=3,45=4,135=5,225=6,315=7");
	m_listCommandScriptList.AddString("  - soble_magnitude");
	m_listCommandScriptList.AddString("  - enhance = 0 :Enhance=0, EnhanceY=1,EnhanceX=2");
	m_listCommandScriptList.AddString("  - flip");
	m_listCommandScriptList.AddString("  - bliateralfilter");
	m_listCommandScriptList.AddString("  - resize=0.5");
	m_listCommandScriptList.AddString("  - pseudo=2 :colormap(0~11)");
	m_listCommandScriptList.AddString("  - cartToPolar");
	
	InitTitle(&m_staticScriptList, "Command List", 14.f, RGB(64, 64, 64));
	InitTitle(&m_staticScriptCommand, "Command Run", 14.f, RGB(64, 64, 64));

	MainButtonInit(&m_btnOK);			   m_btnOK.SetSizeText(14.f);
	MainButtonInit(&m_btnProcessing);	   m_btnProcessing.SetSizeText(14.f);
	MainButtonInit(&m_btnProcessingClear); m_btnProcessingClear.SetSizeText(14.f);
	MainButtonInit(&m_btnProcessingGet); m_btnProcessingGet.SetSizeText(14.f);
	MainButtonInit(&m_btnDeleteFile); m_btnDeleteFile.SetSizeText(14.f);

	EditButtonInit(&m_editCmdControl, 20);

	CreateHorizontalScroll();

	init_Histroy();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CImageProcessingSimulator::CreateHorizontalScroll()
{
	CString str; CSize sz; int dx = 0;
	CDC* pDC = m_listCommandScriptList.GetDC();

	for (int i = 0; i < m_listCommandScriptList.GetCount(); i++)
	{
		m_listCommandScriptList.GetText(i, str);
		sz = pDC->GetTextExtent(str);

		if (sz.cx > dx)			dx = sz.cx;
	}
	m_listCommandScriptList.ReleaseDC(pDC);

	if (m_listCommandScriptList.GetHorizontalExtent() < dx)
	{
		m_listCommandScriptList.SetHorizontalExtent(dx);
	}
}

void CImageProcessingSimulator::init_Histroy()
{
	tinyxml2::XMLDocument doc;
	CString filename;

	filename.Format("%s%s", m_pMain->m_strDir, "ImageHistoryXml.xml");

	if (XML_SUCCESS == doc.LoadFile(filename))
	{
		dumpToStdout(&doc);
	}	
}

void CImageProcessingSimulator::dumpToStdout(const XMLNode* parent)
{
	if (!parent) return;

	XMLNode* child;
	XMLElement* elem;
	XMLAttribute* attr;
	XMLDeclaration* decl;
	XMLComment* comm;

	unsigned int indent = 0;

	for (child = (XMLNode*)parent->FirstChild(); child != 0; child = (XMLNode*)child->NextSibling())
	{
		decl = child->ToDeclaration();
		elem = child->ToElement();
		comm = child->ToComment();

		if (elem) 
		{
	
			for (attr = (XMLAttribute*)elem->FirstAttribute(); attr != 0; attr = (XMLAttribute*)attr->Next())
			{
				m_nItemCount++;
				m_listCommandHistorytList.AddString(child->Value());
				//m_listCommandHistorytList.AddString(attr->Name());
			}
		}

		dumpToStdout(child);
	}
}

CString CImageProcessingSimulator::searchXMLData(const XMLNode* parent,CString Key)
{
	CString strReturn="";
	if (!parent) return strReturn;

	XMLNode* child;
	XMLElement* elem;
	XMLAttribute* attr;
	XMLDeclaration* decl;
	XMLComment* comm;

	unsigned int indent = 0;

	for (child = (XMLNode*)parent->FirstChild(); child != 0; child = (XMLNode*)child->NextSibling())
	{
		decl = child->ToDeclaration();
		elem = child->ToElement();
		comm = child->ToComment();

		if (elem)
		{
			for (attr = (XMLAttribute*)elem->FirstAttribute(); attr != 0; attr = (XMLAttribute*)attr->Next())
			{
				if (Key == child->Value())
				{
					strReturn = attr->Value();
					return strReturn;
				}				
			}
		}

		strReturn = searchXMLData(child, Key);
	}

	return strReturn;
}

HBRUSH CImageProcessingSimulator::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (pWnd->GetDlgCtrlID() == IDC_LIST_SCRIPT_LIST ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_SCRIPT_RUN)
	{
		pDC->SetBkColor(COLOR_UI_BODY);
		pDC->SetTextColor(COLOR_WHITE);
	}

	if (nCtlColor == CTLCOLOR_LISTBOX)
	{
		pDC->SetBkColor(COLOR_UI_BODY);
		pDC->SetTextColor(COLOR_WHITE);
	}
	return m_hbrBkg;
}

void CImageProcessingSimulator::InitTitle(CLabelEx* pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}


void CImageProcessingSimulator::MainButtonInit(CButtonEx * pbutton, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}

void CImageProcessingSimulator::EditButtonInit(CEditEx* pbutton, int size, COLORREF color)
{
	pbutton->SetSizeText(size);				// 글자 크기
	pbutton->SetStyleTextBold(true);		// 글자 스타일
	pbutton->SetTextMargins(10, 10);		// 글자 옵셋
	pbutton->SetColorText(color);	// 글자 색상
	pbutton->SetText(_T(""));				// 글자 설정
}

void CImageProcessingSimulator::OnBnClickedButtonProcessing()
{
	if (m_srcImg == NULL || m_pView == NULL) return;

	UpdateData();

	USES_CONVERSION; // ATL Conversion 함수 사용

	// hsj 2023-01-02 문자열 끝에 엔터가 있으면 엔터 빼주기
	int nLength = m_editCmd.GetLength();
	CString str = m_editCmd.Right(2);

	if (strcmp(str,"\r\n")==0) 
	{
		m_editCmd=m_editCmd.Left(nLength - 2);
	}

	std::string cmd = std::string(CT2CA(m_editCmd));
	cv::Mat m_dstImg;

	CSimpleMatchScript script;
	const string errMsg = script.Match(*m_srcImg, m_dstImg, cmd, false);

	if(m_dstImg.channels()!= m_pView->GetBPP()/8)
	{
		m_pView->OnInitWithCamera(m_dstImg.cols, m_dstImg.rows, m_dstImg.channels()*8);
	}
	m_pView->OnLoadImageFromPtr((BYTE*)m_dstImg.data);
	m_pView->Invalidate();
	m_dstImg.release();

	CString filename;
	SYSTEMTIME time;
	::GetLocalTime(&time);

	str.Format("%04d%02d%02d", time.wYear, time.wMonth, time.wDay);
	std::string cmdMap = std::string(CT2CA(str));
//	m_Historymgr.AddObject(cmdMap, cmd);	

	filename.Format("%s%s", m_pMain->m_strDir, "ImageHistoryXml.xml");

	tinyxml2::XMLDocument doc;
	XMLElement* root;

	if (XML_SUCCESS != doc.LoadFile(filename))
	{
		str.Format("C0_%s", str);

		XMLDeclaration* decl = doc.NewDeclaration();
		doc.LinkEndChild(decl);

		root = doc.NewElement("History");
		doc.LinkEndChild(root);

		XMLComment* comment = doc.NewComment("Settings for MyApp");
		root->LinkEndChild(comment);
	}
	else
	{
		XMLNode* child;
		XMLElement* elem;

		for (child = (XMLNode*)doc.FirstChild(); child != 0; child = (XMLNode*)child->NextSibling())
		{
			elem = child->ToElement();

			if (elem)
			{
				root = elem;
				break;
			}
		}

		str.Format("C%d_%s", m_nItemCount++,str);
	}

	XMLElement* connection = doc.NewElement(str);
	root->LinkEndChild(connection);
	connection->SetAttribute("data", cmd.c_str());

	doc.SaveFile(filename);

	m_listCommandHistorytList.AddString(str);
}


void CImageProcessingSimulator::OnBnClickedButtonProcessingClear()
{

	CRect Rect;
	m_editCmdControl.GetWindowRect(&Rect);
	ScreenToClient(&Rect);
	InvalidateRect(Rect);

	m_editCmd = "";
	UpdateData(FALSE);

}

// hsj 2023-01-02 더블클릭하면 리스트 업 되도록
void CImageProcessingSimulator::OnLbnDblclkListScriptList()
{
	UpdateData();

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nIndex = m_listCommandScriptList.GetCurSel();

	CString str,strTemp;
	m_listCommandScriptList.GetText(nIndex, str);

	int nLength;
	nLength = str.GetLength();

	int equalIndex = str.Find("=");

	if (equalIndex != -1)
	{
		//4는 앞에 띄어쓰기랑 -까지가 4개임, 3은 =까지 나타내주려고..
		str = str.Mid(4, equalIndex - 3) + "\r\n";
	}
	else
	{
		//4는 앞에 띄어쓰기랑 -까지가 4개임
		str = str.Mid(4, nLength - 4) + "\r\n"; 
	}

	equalIndex = LastIndexOf(m_editCmd,("\r\n"));
	int length = m_editCmd.GetLength();

	if (m_editCmdControl.GetSel()>0 && equalIndex != length -2)
	{
		m_editCmd = m_editCmd + "\r\n";
	}

	m_editCmd = m_editCmd + str + strTemp;

	m_editCmdControl.SetSel(0, -1);
	m_editCmdControl.ReplaceSel(m_editCmd);
	m_editCmdControl.SetFocus();
}


void CImageProcessingSimulator::OnBnClickedButtonProcessingGet()
{
	int nIndex = m_listCommandHistorytList.GetCount();
	if (nIndex > 0)
	{
		m_listCommandHistorytList.SetCurSel(nIndex-1);
		OnLbnDblclkListScriptHistory();
	}
}


void CImageProcessingSimulator::OnLbnDblclkListScriptHistory()
{
	int nIndex = m_listCommandHistorytList.GetCurSel();

	CString str;
	m_listCommandHistorytList.GetText(nIndex, str);	

	tinyxml2::XMLDocument doc;
	CString filename;

	filename.Format("%s%s", m_pMain->m_strDir, "ImageHistoryXml.xml");

	if (XML_SUCCESS == doc.LoadFile(filename))
	{
		m_editCmd = searchXMLData(&doc, str);
	}
	m_editCmd.Replace("\n\n", "\r\n");

	m_editCmdControl.SetSel(0, -1);
	m_editCmdControl.ReplaceSel(m_editCmd);
	m_editCmdControl.SetFocus();
}


void CImageProcessingSimulator::OnBnClickedButtonXmlClear()
{
	CString filename;

	filename.Format("%s%s", m_pMain->m_strDir, "ImageHistoryXml.xml");

	if (AfxMessageBox("Delete History Data?", MB_YESNO) != IDYES) return;
	
	::DeleteFile(filename);
	m_listCommandHistorytList.ResetContent();
}
