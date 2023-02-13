// ImageProcessingSimulator.cpp: 구현 파일
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "ImageProcessingSimulator.h"
#include "afxdialogex.h"
#include "SimpleMatchScript.h"
#include "../VirtualKeyBoard/VirtualKeyBoard.h"

using namespace tinyxml2;

// CImageProcessingSimulator 대화 상자
IMPLEMENT_DYNAMIC(CImageProcessingSimulator, CDialogEx)

CImageProcessingSimulator::CImageProcessingSimulator(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_IMAGE_PROCESSING, pParent)
{
	m_srcImg = NULL;
	m_pView = NULL;

	m_nItemCount = 0;

	m_listCmd = "";
}

CImageProcessingSimulator::~CImageProcessingSimulator()
{
}

void CImageProcessingSimulator::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SCRIPT_LIST, m_listCommandScriptList);
	DDX_Control(pDX, IDC_LIST_SCRIPT_HISTORY, m_listCommandHistorytList);
	DDX_Control(pDX, IDC_STATIC_SCRIPT_LIST, m_staticScriptList);
	DDX_Control(pDX, IDC_STATIC_SCRIPT_COMMAND, m_staticScriptCommand);
	DDX_Control(pDX, IDC_BUTTON_PROCESSING, m_btnProcessing);
	DDX_Control(pDX, IDC_BUTTON_PROCESSING_CLEAR, m_btnProcessingClear);
	DDX_Control(pDX, IDC_BUTTON_PROCESSING_GET, m_btnProcessingGet);
	DDX_Control(pDX, IDC_BUTTON_XML_CLEAR, m_btnDeleteFile);
	DDX_Control(pDX, IDC_LIST_SCRIPT_RUN, m_ListScriptRun);
	DDX_Control(pDX, IDC_STATIC_SCRIPT_LIST2, m_staticExplanation);
	DDX_Control(pDX, IDC_STATIC_HISTORY, m_staticHistory);
	DDX_Control(pDX, IDC_BUTTON_IMGPROC_ADD, m_btnImgProcAdd);
	DDX_Control(pDX, IDC_BUTTON_PROCESSING_ALL_CLEAR, m_btnProcessingAllClear);
	DDX_Control(pDX, IDC_EDIT_COMMAND, m_editCommand);
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
	ON_LBN_DBLCLK(IDC_LIST_SCRIPT_RUN, &CImageProcessingSimulator::OnLbnDblclkListScriptRun)
	ON_BN_CLICKED(IDC_BUTTON_PROCESSING_ALL_CLEAR, &CImageProcessingSimulator::OnBnClickedButtonProcessingAllClear)
	ON_BN_CLICKED(IDC_BUTTON_IMGPROC_ADD, &CImageProcessingSimulator::OnBnClickedButtonImgprocAdd)
	ON_LBN_SELCHANGE(IDC_LIST_SCRIPT_LIST, &CImageProcessingSimulator::OnLbnSelchangeListScriptList)
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
	m_listCommandScriptList.AddString("  - acc");
	m_listCommandScriptList.AddString("  - adapthreshold= blocksize, threshold");
	m_listCommandScriptList.AddString("  - bliateralfilter=sigmaColor, sigmaSpace");
	m_listCommandScriptList.AddString("  - bgr= 10, 20, 30");
	m_listCommandScriptList.AddString("  - canny= threshold");
	m_listCommandScriptList.AddString("  - cvt= bgr - hsv / bgr - hls / bgr - gray / hsv - bgr / hls - bgr / gray - bgr");
	m_listCommandScriptList.AddString("  - cartToPolar");
	m_listCommandScriptList.AddString("  - dilate=X,Y");
	m_listCommandScriptList.AddString("  - erode=X,Y");
	m_listCommandScriptList.AddString("  - enhance= 0 :Enhance=0, EnhanceY=1,EnhanceX=2");
	m_listCommandScriptList.AddString("  - flip");
	m_listCommandScriptList.AddString("  - findcontours= minsize");
	m_listCommandScriptList.AddString("  - gaussian");
	m_listCommandScriptList.AddString("  - gray");
	m_listCommandScriptList.AddString("  - hsv= 80, 100, 100, 90, 255, 255");
	m_listCommandScriptList.AddString("  - hls= 80, 100, 100, 90, 255, 255");
	m_listCommandScriptList.AddString("  - hough");
	m_listCommandScriptList.AddString("  - invert= threshold");	
	m_listCommandScriptList.AddString("  - medianblur");
	m_listCommandScriptList.AddString("  - morphologyEx=filtersize,method  :erode=0,dilate=1,open=2,close=3,gradient=4,tophat=5,blackhat=6");
	m_listCommandScriptList.AddString("  - otsu");
	m_listCommandScriptList.AddString("  - pseudo=2 :colormap(0~11)");
	m_listCommandScriptList.AddString("  - roi= 0, 0, 100, 100");
	m_listCommandScriptList.AddString("  - roiCopy= 0, 0, 100, 100");
	m_listCommandScriptList.AddString("  - rect= 0, 0, 100, 100");
	m_listCommandScriptList.AddString("  - removenoise=colr,minsize : white=0,black=1");
	m_listCommandScriptList.AddString("  - resize=0.5");
	m_listCommandScriptList.AddString("  - scale= 1.5");
	m_listCommandScriptList.AddString("  - shiftAdd=shift,dir : Up=1, Right=2, Down=3, Left=4");
	m_listCommandScriptList.AddString("  - shiftSub=shift,dir : Up=1, Right=2, Down=3, Left=4");
	m_listCommandScriptList.AddString("  - sobel= 0~7 :Up=0,Down=1,Left=2,Right=3,45=4,135=5,225=6,315=7");
	m_listCommandScriptList.AddString("  - sobel_magnitude");
	m_listCommandScriptList.AddString("  - threshold= 100");	
	
	InitTitle(&m_staticScriptList, "Command List", 14.f, RGB(64, 64, 64));
	InitTitle(&m_staticScriptCommand, "Command Run", 14.f, RGB(64, 64, 64));
	InitTitle(&m_staticExplanation, "Command Explanation", 14.f, RGB(64, 64, 64));
	InitTitle(&m_staticHistory, "History", 14.f, RGB(64, 64, 64));

	MainButtonInit(&m_btnProcessing);	   m_btnProcessing.SetSizeText(14.f);
	MainButtonInit(&m_btnProcessingClear); m_btnProcessingClear.SetSizeText(14.f);
	MainButtonInit(&m_btnProcessingGet); m_btnProcessingGet.SetSizeText(14.f);
	MainButtonInit(&m_btnDeleteFile); m_btnDeleteFile.SetSizeText(14.f);
	MainButtonInit(&m_btnImgProcAdd); m_btnImgProcAdd.SetSizeText(14.f);
	MainButtonInit(&m_btnProcessingAllClear); m_btnProcessingAllClear.SetSizeText(14.f);
	
	CreateHorizontalScroll();

	init_Histroy();
	
	init_command();

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
		pWnd->GetDlgCtrlID() == IDC_LIST_SCRIPT_RUN|| 
		pWnd->GetDlgCtrlID() == IDC_EDIT_COMMAND)
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
	if (m_srcImg == NULL || m_pView == NULL || m_listCmd.GetLength()<=3 ) return;

	UpdateData();

	USES_CONVERSION; // ATL Conversion 함수 사용

	// hsj 2023-01-02 문자열 끝에 엔터가 있으면 엔터 빼주기
	int nLength = m_listCmd.GetLength();
	CString str = m_listCmd.Right(2);

	if (strcmp(str,"\r\n")==0) 
	{
		m_listCmd= m_listCmd.Left(nLength - 2);
	}

	std::string cmd = std::string(CT2CA(m_listCmd)); 
	cv::Mat m_dstImg;

	CSimpleMatchScript script;
	const string errMsg = script.Match(*m_srcImg, m_dstImg, cmd, false);

	if (m_dstImg.rows * m_dstImg.cols != m_srcImg->rows * m_srcImg->cols) return;

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
	OnLbnDblclkListScriptRun();
}

// hsj 2023-01-02 더블클릭하면 리스트 업 되도록
void CImageProcessingSimulator::OnLbnDblclkListScriptList()
{
	UpdateData();

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CVirtualKeyBoard stdlg;
	CString strTemp_OLD, strTemp_NEW;

	int nIndex = m_listCommandScriptList.GetCurSel();

	CString str, strtemp;
	m_listCommandScriptList.GetText(nIndex, str);

	int nLength;
	nLength = str.GetLength();

	int equalIndex = str.Find("=");
	
	// KBJ 2023-01-12 ===================
	bool bIDOK = false;
	if (equalIndex != -1)
	{
		//4는 앞에 띄어쓰기랑 -까지가 4개임, 3은 =까지 나타내주려고..
		str = str.Mid(4, equalIndex - 3);

		if (str.Find("save") > -1)
		{
			char szFilter[] = "Jpg Files (*.jpg) | *.jpg | All Files (*.*) | *.* ||";
			CFileDialog dlg(FALSE, NULL, "..\\*.jpg", OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT, szFilter);

			if (dlg.DoModal() == IDOK)
			{
				if (dlg.GetFileExt().GetLength() <= 0)
				{
#ifdef __MESSAGE_DLG_H__
					theApp.setMessage(MT_OK, "There is no extension.");
#else
					AfxMessageBox("There is no extension.");
#endif
					return;
				}
				str = str + dlg.GetPathName() + "\r\n";
				m_ListScriptRun.AddString(str);
				bIDOK = true;
			}
			else return;
		}
		else
		{
			CString strTemp_OLD, strTemp_NEW;
			CVirtualKeyBoard m_pKeyboardDlg;

			if (m_pKeyboardDlg.DoModal() == IDOK)
			{
				m_pKeyboardDlg.GetValue(strTemp_NEW);
				strtemp = str + "" + strTemp_NEW +"\r\n";
				m_ListScriptRun.AddString(strtemp);
				str = strtemp;

				bIDOK = true;
			}
			else return;
		}
	}
	else
	{
		// 4는 앞에 띄어쓰기랑 - 까지가 4개임
		str = str.Mid(4, nLength - 4) + "\r\n";

		m_ListScriptRun.AddString(str);
		bIDOK = true;
	}
	// ==================================

	equalIndex = LastIndexOf(m_listCmd, ("\r\n"));
	int length = m_listCmd.GetLength();


	if (m_ListScriptRun.GetCurSel() > 0 && equalIndex != length - 2)
	{
		m_listCmd = m_listCmd + "\r\n";
	}
	else if (length != 0)
	{
		m_listCmd = m_listCmd + "\r\n";
	}
	m_listCmd = m_listCmd + str;
	
	m_ListScriptRun.SetCurSel(m_ListScriptRun.GetCount()-1);

	if (bIDOK == true)
	{
		OnBnClickedButtonProcessing();
	}
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
		m_listCmd = searchXMLData(&doc, str);
	}

	m_listCmd += "\r\n";

	int nCount = m_listCmd.Replace("\n\n", "\r\n")+1;
	
	m_ListScriptRun.ResetContent();
	
	CString strtemp;
	for (int i = 0; i < nCount; i++)
	{
		AfxExtractSubString(strtemp, m_listCmd, i, TCHAR('\r\n'));
		m_ListScriptRun.AddString(strtemp);
	}
	
	if (nCount > 0) m_ListScriptRun.SetCurSel(nCount-1);
}

void CImageProcessingSimulator::OnBnClickedButtonXmlClear()
{
	CString filename;

	filename.Format("%s%s", m_pMain->m_strDir, "ImageHistoryXml.xml");

	if (AfxMessageBox("Delete History Data?", MB_YESNO) != IDYES) return;
	
	::DeleteFile(filename);

	m_listCommandHistorytList.ResetContent();
}


void CImageProcessingSimulator::OnLbnDblclkListScriptRun()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nFind = 0;
	int nLength = 0;
	for (int i = 0; i < m_ListScriptRun.GetCurSel(); i++)
	{
		CString strListScriptRun;
		m_ListScriptRun.GetText(i, strListScriptRun);
		nLength = strListScriptRun.GetLength();
		
		if (strListScriptRun.Find(_T("\r\n")) > -1)
		{
		}
		
		nFind += nLength;
	}

	if (m_ListScriptRun.GetCurSel() < 0) return;

	CString strDeleteName;
	m_ListScriptRun.GetText(m_ListScriptRun.GetCurSel(), strDeleteName);

	CString Left = m_listCmd.Left(nFind);
	CString Right = m_listCmd.Right(m_listCmd.GetLength() - (nFind + strDeleteName.GetLength()));

	m_listCmd = Left + Right;

	m_ListScriptRun.DeleteString(m_ListScriptRun.GetCurSel());
	m_ListScriptRun.SetCurSel(m_ListScriptRun.GetCount() - 1);

	OnBnClickedButtonProcessing();
}

void CImageProcessingSimulator::OnBnClickedButtonProcessingAllClear()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CRect Rect;

	m_ListScriptRun.ResetContent();
	m_ListScriptRun.GetWindowRect(&Rect);
	ScreenToClient(&Rect);
	InvalidateRect(Rect);

	m_listCmd = "reload\r\n";
	OnBnClickedButtonProcessing();
	UpdateData(FALSE);
}


void CImageProcessingSimulator::OnBnClickedButtonImgprocAdd()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	OnLbnDblclkListScriptList();
}

void CImageProcessingSimulator::init_command()
{
	m_editCommand.SetReadOnly(TRUE);

	int num_of_list_count = m_listCommandScriptList.GetCount();

	TCHAR szData[MAX_PATH] = { 0, };
	//CString strFilePath = m_pMain->m_strSystemDir + "ImageProcessing.ini";

	CString strListName;
	CString strListCommand;
	pair<CString, CString> map;

	
	for (int i = 0; i < num_of_list_count; i++)
	{
		m_listCommandScriptList.GetText(i, strListName);
		
		int nLength;
		nLength = strListName.GetLength();

		int equalIndex = strListName.Find("=");

		if (equalIndex != -1)
		{
			//4는 앞에 띄어쓰기랑 -까지가 4개임, 3은 =까지 나타내주려고..
			strListName = strListName.Mid(4, equalIndex - 4);
		}
		else
		{
			// 4는 앞에 띄어쓰기랑 - 까지가 4개임
			strListName = strListName.Mid(4, nLength - 4);
		}

		if (strListName.GetLength() == 0)
		{
			strListCommand.Format("No Data");
		}
		else if (strcmp(strListName,"roi") == 0)
		{
			strListCommand.Format("ex) roi = 10,10,100,100\r\n"
				"\r\n"
				"cv::Rect roi = { 0,0,0,0 }; \r\n"
				"sscanf(param.c_str(), %%d,%%d,%%d,%%d, &roi.x, &roi.y, &roi.width, &roi.height)\r\n"
				"dst = dst(roi); \r\n"
				"\r\n"
				"영상처리할 영역을 정한다\r\n");
		}
		else if (strcmp(strListName, "roiCopy") == 0)
		{
			strListCommand.Format("ex) roiCopy=x,y,width,height\r\n"
				"\r\n"
				"cv::Rect roi = { 0,0,0,0 }; \r\n"
				"sscanf(param.c_str(), %%d,%%d,%%d,%%d, &roi.x, &roi.y, &roi.width, &roi.height); \r\n"
				"dst.copyTo(m_src(roi)); \r\n"
				"\r\n"
				"영상처리할 영역을 복사한다.\r\n"
			);
		}
		else if (strcmp(strListName, "rect") == 0)
		{
			strListCommand.Format("ex) rect=x,y,width,height\r\n"
				"\r\n"
				"cv::Rect rect = { 0,0,0,0 }; \r\n"
				"sscanf(param.c_str(), %%d,%%d,%%d,%%d, &rect.x, &rect.y, &rect.width, &rect.height); \r\n"
				"cRectContour r(rect); \r\n"
				"r.Draw(dst, cv::Scalar(0, 0, 255), 2); \r\n"
				"\r\n"
				"사각형을 그린다.\r\n"
			);
		}
		else if (strcmp(strListName, "bgr") == 0)
		{
			strListCommand.Format("ex) bgr = num1, num2, num3\r\n"
				"\r\n"
				"Mat &= Scalar(num1, num2, num3)\r\n"
				"\r\n"
				"생삭을 설정한다\r\n"
			);
		}
		else if (strcmp(strListName, "scale") == 0)
		{
			strListCommand.Format("ex) scale=num\r\n"
				"\r\n"
				"Mat *= num\r\n"
				"dst *= scale; \r\n"
				"\r\n"
				"scale을 설정한다.\r\n"
			);
		}
		else if (strcmp(strListName, "gray") == 0)
			{
				strListCommand.Format("ex) 예시없음\r\n"
					"\r\n"
					"이미지를 이진화 한다.\r\n"
			);
		}
		else if (strcmp(strListName, "threshold") == 0)
		{
			strListCommand.Format("ex) threshold = thresh1\r\n"
				"\r\n"
				"threshold(dst, dst, thresh1, 255, CV_THRESH_BINARY)\r\n"
				"\r\n"
				"임계값 기준으로 이미지를 이진화 한다.\r\n"
			);
		}
		else if (strcmp(strListName, "otsu") == 0)
		{
			strListCommand.Format("ex) 예시없음\r\n"
				"\r\n"
				"cv::threshold(dst, dst, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);\r\n"
				"\r\n"
				"방법은 경게값을 임의로 정해서 픽셀들을 두 부류로 나누고 두 부류의 명암 분포를 반복해서 구한 다음 두 부류의 명암 분포를 가장 균일하게 하는 경게 값을 선택한다.\r\n"
				"다시말해, 특정 threshold를 T라 하면, T를 기준으로 이진 분류된 픽셀의 비율의 차가 가장 작은 optimal T를 구하는 것이다.\r\n"
				"오츠 알고리즘은 모든 threshold에 대해 계산해야 하기 때문에 속도가 느리다는 단점이 있다.\r\n"
				"또한 노이즈가 많은 영상에는 오츠의 알고리즘을 적용해도 좋은 결과를 얻지 못하는 경우가 있다.\r\n"
			);
		}
		else if (strcmp(strListName, "adapthreshold") == 0)
		{
			strListCommand.Format("ex) adapthreshold = block_size, thresh_c\r\n"
				"\r\n"
				"adaptiveThreshold(dst, dst, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, block_size, thresh_c);\r\n"
				"\r\n"
				"이미지를 여러 영역으로 나눈 후에 그 주변 픽셀 값을 기준으로 계산하여 영역마다의 threshold를 지정한다. 이것을 적응형 threshold(Adaptive threshold)라고 한다.\r\n"
			);
		}
		else if (strcmp(strListName, "invert") == 0)
		{
			strListCommand.Format("ex) invert = thresh\r\n"
				"\r\n"
				"threshold(dst, dst, thresh, 255, CV_THRESH_BINARY_INV);\r\n"
				"\r\n"
				"임계값 기준으로 이미지를 역으로 이진화 한다.\r\n"
			);
		}
		else if (strcmp(strListName, "cvt") == 0)
		{
			strListCommand.Format("ex) cvt = hls/hls/bgr/...\r\n"
				"\r\n"
				"if (param == bgr - hls)	cvtColor(dst, dst, CV_BGR2hls);	\r\n"
				"else if (param == bgr - hls)	cvtColor(dst, dst, CV_BGR2HLS);	\r\n"
				"else if (param == bgr - gray)	cvtColor(dst, dst, CV_BGR2GRAY);\r\n"
				"else if (param == hls - bgr)	cvtColor(dst, dst, CV_hlsBGR);	\r\n"
				"else if (param == hls - bgr)	cvtColor(dst, dst, CV_HLS2BGR);	\r\n"
				"else if (param == gray - bgr)	cvtColor(dst, dst, CV_GRAY2BGR);\r\n"
				"\r\n"
				"이미지를 변환한다."
			);
		}
		else if (strcmp(strListName, "hsv") == 0)
		{
			strListCommand.Format("ex) hsv = hsv[0],hsv[1],hsv[2],hsv[3],hsv[4],hsv[5]\r\n"
				"\r\n"
				"cvtColor(dst, dst, CV_BGR2HSV);\r\n"
				"inRange(dst, cv::Scalar(hsv[0], hsv[1], hsv[2]), cv::Scalar(hsv[3], hsv[4], hsv[5]), dst);\r\n"
				"cvtColor(dst, dst, CV_GRAY2BGR);\r\n"
				"\r\n"
			);
		}
		else if (strcmp(strListName, "hls") == 0)
		{
			strListCommand.Format("ex) hsb = hls[0],hls[1],hls[2],hls[3],hls[4],hls[5]\r\n"
				"\r\n"
				"cvtColor(dst, dst, CV_BGR2HLS);\r\n"
				"inRange(dst, cv::Scalar(hls[0], hls[1], hls[2]), cv::Scalar(hls[3], hls[4], hls[5]), dst);\r\n"
				"cvtColor(dst, dst, CV_GRAY2BGR);\r\n"
				"\r\n"
		);
		}
		else if (strcmp(strListName, "acc") == 0)
		{
			strListCommand.Format("ex) 예시없음\r\n"
				"\r\n"
				"cv::Mat tmp(accMat.front().rows, accMat.front().cols, accMat.front().flags);\r\n"
				"for each (auto & m in accMat)\r\n"
				"	tmp += m;\b"
				"dst = tmp;\b"
				"\b"
			);
		}
		else if (strcmp(strListName, "shiftAdd") == 0)
		{
			strListCommand.Format("ex) shiftAdd = pixels, direction\r\n"
				"\r\n"
				"cv::Mat tmpSrc(dst.size(), CV_8UC1, cv::Scalar(0));\r\n"
				"dst.copyTo(tmpSrc);\r\n"
				"tmpSrc = shiftFrame(tmpSrc, pixels, direction);\r\n"
				"dst += tmpSrc; \r\n"
			);	
		}
		else if (strcmp(strListName, "shiftSub") == 0)
		{
			strListCommand.Format("ex) shiftSub = pixels, direction\r\n"
				"\r\n"
				"cv::Mat tmpSrc(dst.size(), CV_8UC1, cv::Scalar(0));\r\n"
				"dst.copyTo(tmpSrc);\r\n"
				"tmpSrc = shiftFrame(tmpSrc, pixels, direction);\r\n"
				"dst -= tmpSrc;\r\n"
			);	
		}
		else if (strcmp(strListName, "sobel_magnitude") == 0)
		{
			strListCommand.Format("ex) 예시없음\r\n"
				"\r\n"
				"cv::Mat x, y;\r\n"
				"cv::Sobel(dst, x, CV_32FC1, 1, 0);\r\n"
				"cv::Sobel(dst, y, CV_32FC1, 0, 1);\r\n"
				"cv::Mat float_mag, mag;\r\n"
				"cv::magnitude(x, y, float_mag);\r\n"
				"float_mag.convertTo(dst, CV_8UC1);\r\n"
			);
		}
		else if (strcmp(strListName, "removenoise") == 0)
		{
			strListCommand.Format("ex) removenoise = sz_color, sz_min\r\n"
				"\r\n"
				"cv::Mat tmpSrc(dst.size(), CV_8UC1, cv::Scalar(0));\r\n"
				"dst.copyTo(tmpSrc);\r\n"
				"if (sz_color == 1) cv::bitwise_not(tmpSrc, tmpSrc);\r\n"
				"\r\n"
				"std::vector<std::vector<cv::Point>> contours;\r\n"
				"std::vector<cv::Vec4i> hierarchy;\r\n"
				"cv::findContours(tmpSrc, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);\r\n"
				"\r\n"
				"for (int i = 0; i < contours.size(); i++)\r\n"
				"{\r\n"
				"	cv::Moments moms = moments(cv::Mat(contours[i]));\r\n"
				"	double area = moms.m00;\r\n"
				"\r\n"
				"	if (area < sz_min)\r\n"
				"	{\r\n"
				"		if (sz_color == 1) drawContours(dst, contours, i, cv::Scalar(255), CV_FILLED, 8, hierarchy, 0, cv::Point());\r\n"
				"		else drawContours(dst, contours, i, cv::Scalar(0), CV_FILLED, 8, hierarchy, 0, cv::Point());\r\n"
				"	}\r\n"
				"}\r\n"
			);
		}
		else if (strcmp(strListName, "reload") == 0)
		{
			strListCommand.Format("ex) 예시없음\r\n"
				"\r\n"
				"dst.release();\r\n"
				"\r\n"
			);
		}
		else if (strcmp(strListName, "save") == 0)
		{
			strListCommand.Format("ex) save = path\r\n"
				"\r\n"
				"cv::imwrite(path, dst);\r\n"
				"\r\n"
				"선택한 경로로 이미지를 저장한다.\r\n"
			);
		}
		else if (strcmp(strListName, "hough") == 0)
		{
			strListCommand.Format("ex) 예시없음\r\n"
				"\r\n"
				" std::vector<cv::Vec2f> lines;\r\n"
				"cv::HoughLines(dst, lines, 1, 0.1f, 80);\r\n"
				"\r\n"
			);
		}
		else if (strcmp(strListName, "canny") == 0)
		{
			strListCommand.Format("ex) canny = threshold1\r\n"
				"\r\n"
				"cv::Canny(dst, dst, 0, threshold1, 5)\r;\n"
				"\r\n"
			);
		}
		else if (strcmp(strListName, "findcontours") == 0)
		{
			strListCommand.Format("ex) findcontours = sz_min\r\n"
				"\r\n"
				"cv::Mat  displayImage;\r\n"
				"cv::cvtColor(dst, displayImage, cv::COLOR_GRAY2BGR);\r\n"
				"\r\n"
				"vector<cv::Point> approx;\r\n"
				"vector<vector<cv::Point>> contours;\r\n"
				"cv::findContours(dst, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);\r\n"
				"\r\n"
				"for (int i = 0; i < contours.size(); i++)\r\n"
				"{\r\n"
				"	int area = contourArea(contours[i]);\r\n"
				"\r\n"
				"	if (area > sz_min)\r\n"
				"	{\r\n"
				"		double epsilon = cv::arcLength(cv::Mat(contours[i]), true) * 0.0005;\r\n"
				"		cv::approxPolyDP(contours[i], approx, epsilon, true);\r\n"
				"		drawContours(displayImage, vector<vector<cv::Point>>(1, approx), -1, cv::Scalar(255, 0, 0), 1);\r\n"
				"		approx.clear();\r\n"
				"	}\r\n"
				"}\r\n"
				"\r\n"
				"contours.clear();\r\n"
			);
		}
		else if (strcmp(strListName, "pseudo") == 0)
		{
			strListCommand.Format("ex) pseudo = mode\r\n"
				"\r\n"
				"	0	COLORMAP_AUTUMN	colorscale_autumn\r\n"
				"	1	COLORMAP_BONE	colorscale_bone\r\n"
				"	2	COLORMAP_JET	colorscale_jet\r\n"
				"	3	COLORMAP_WINTER	colorscale_winter\r\n"
				"	4	COLORMAP_RAINBOW	colorscale_rainbow\r\n"
				"	5	COLORMAP_OCEAN	colorscale_ocean\r\n"
				"	6	COLORMAP_SUMMER	colorscale_summer\r\n"
				"	7	COLORMAP_SPRING	colorscale_spring\r\n"
				"	8	COLORMAP_COOL	colorscale_cool\r\n"
				"	9	COLORMAP_HSV	colorscale_hsv\r\n"
				"	10	COLORMAP_PINK	colorscale_pink\r\n"
				"	11	COLORMAP_HOT\r\n"
				"int mode = 2;\r\n"
				"cv::applyColorMap(dst, dst, mode);\r\n"
			);
		}
		else if (strcmp(strListName, "dilate") == 0)
		{
			strListCommand.Format("ex) dilate = X, Y\r\n"
				"\r\n"
				"int X = 3, Y = 3;\r\n"
				"sscanf(param.c_str(), %%d, %%d, &X, &Y);\r\n"
				"dilate(dst, dst, cv::Mat::ones(cv::Size(X, Y), CV_8UC1));\r\n"
				"\r\n"
			);
		}
		else if (strcmp(strListName, "erode") == 0)
		{
			strListCommand.Format("ex) erode = X, Y\r\n"
				"\r\n"
				"int X = 0, Y = 0;\r\n"
				"sscanf(param.c_str(), %%d, %%d, &X, &Y);\r\n"
				"erode(dst, dst, cv::Mat::ones(cv::Size(X, Y), CV_8UC1));\r\n"
			);
		}
		else if (strcmp(strListName, "morphologyEx") == 0)
		{
			strListCommand.Format("ex) morphologyEx = filterSize, Method\r\n"
				"\r\n"
				"cv::Mat element = cv::getStructuringElement(CV_SHAPE_RECT, cv::Size(filterSize, filterSize));\r\n"
				"\r\n"
				"switch (method)\r\n"
				"{\r\n"
				"	case 0: morphologyEx(dst, dst, CV_MOP_ERODE, element); break;\r\n"
				"	case 1: morphologyEx(dst, dst, CV_MOP_DILATE, element); break;\r\n"
				"	case 2: morphologyEx(src, dst, CV_MOP_OPEN, element); break;\r\n"
				"	case 3: morphologyEx(dst, dst, CV_MOP_CLOSE, element); break;\r\n"
				"	case 4: morphologyEx(dst, dst, CV_MOP_GRADIENT, element); break;\r\n"
				"	case 5: morphologyEx(dst, dst, CV_MOP_TOPHAT, element); break;\r\n"
				"	case 6: morphologyEx(dst, dst, CV_MOP_BLACKHAT, element); break;\r\n"
				"}\r\n"
			);
		}
		else if (strcmp(strListName, "gaussian") == 0)
		{
			strListCommand.Format("ex) 예시없음\r\n"
				"\r\n"
				"cv::GaussianBlur(dst, dst, cv::Size(0, 0), 8.);\r\n"
				"\r\n"
			);
		}
		else if (strcmp(strListName, "medianblur") == 0)
		{
			strListCommand.Format("ex) 예시없음\r\n"
				"\r\n"
				"cv::medianBlur(dst, dst,3);\r\n"
				"\r\n"
			);
		}
		else if (strcmp(strListName, "sobel") == 0)
		{
			strListCommand.Format("ex) sobel = value\r\n"
				"\r\n"
				"sobelDirection(3, value, dst, dst);\r\n"
				"\r\n"
			);
		}
		else if (strcmp(strListName, "enhance") == 0)
		{
			strListCommand.Format("ex) enhance = value\r\n"
				"\r\n"
				"InspectionEnhance(&dst, proc, false);\r\n"
			);
		}
		else if (strcmp(strListName, "flip") == 0)
		{
			strListCommand.Format("ex) 예시없음\r\n"
				"\r\n"
				"cv::flip(dst, dst, proc);\r\n"
				"\r\n"
			);
		}
		else if (strcmp(strListName, "bliateralfilter") == 0)
		{
			strListCommand.Format("ex) 예시없음\r\n"
				"\r\n"
				"cv::bilateralFilter(dst, dst, -1, sigmaColor, sigmaSpace);\r\n"
				"src : 입력 영상;\r\n"
				"d : 필터링에 사용될 이웃 픽셀의 거리(지름);\r\n"
				"음수(-1) 입력 시 sigmaSpace 값에 의해 자동 결정(추천);\r\n"
				"sigmaColor : 색 공간에서 필터의 표준 편차(너무 크면 에지 부분을 구분하지 못합니다);\r\n"
				"sigmaSpace : 좌표 공간에서 필터의 표준 편차(가우시안 필터에서 시그마와 비슷합니다);\r\n"
				"값이 클수록 연산 시간이 길어집니다.;\r\n"
				"※ sigmaSpace를 엄청 크게 줘도 sigmaColor 값이 작다면 큰 변화가 없네요.서로 값을 바꿔보면서 해보세요;\r\n"
				"\r\n"
			);
		}
		else if (strcmp(strListName, "resize") == 0)
		{
			strListCommand.Format("ex) resize = value\r\n"
				"\r\n"
				"cv::resize(dst, dst, cv::Size(), value, value, CV_INTER_CUBIC);\r\n"
				"\r\n"
			);
		}
		else if (strcmp(strListName, "cartToPolar") == 0)
		{
			strListCommand.Format("ex) 예시없음\r\n"
				"\r\n"
				"int aperture = 3;\r\n"
				"\r\n"
				"cv::Mat sobelX, sobelY;\r\n"
				"cv::Mat sobelMagnitude, sobelOrientation;\r\n"
				"\r\n"
				"cv::Sobel(dst, sobelX, CV_32F, 1, 0, aperture);\r\n"
				"cv::Sobel(dst, sobelY, CV_32F, 0, 1, aperture);\r\n"
				"\r\n"
				"cv::cartToPolar(sobelX, sobelY, sobelMagnitude, sobelOrientation, true);\r\n"
				"\r\n"
				"normalize(sobelMagnitude, dst, 0, 255, 32, CV_8UC1);\r\n"
				"\r\n"
				"sobelX.release();\r\n"
				"sobelY.release();\r\n"
				"sobelMagnitude.release();\r\n"
				"sobelOrientation.release();\r\n"
				"\r\n"
			);
		}
		else
		{
			strListCommand.Format("No Data");
		}

		map.first = strListName;
		map.second= strListCommand;

		m_vt_list_command.push_back(map);
	}
}

void CImageProcessingSimulator::OnLbnSelchangeListScriptList()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nCursel = m_listCommandScriptList.GetCurSel();
	CString strCommand = m_vt_list_command[nCursel].second;

	m_editCommand.SetSel(0, -1, TRUE);
	m_editCommand.Clear();
	
	m_editCommand.SetWindowTextA(strCommand);

	Invalidate();
}