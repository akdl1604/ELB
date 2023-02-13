// VAT_SimulatorDlg.cpp: 구현 파일
//


#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "VAT_SimulatorDlg.h"
#include "afxdialogex.h"
// CVAT_SimulatorDlg 대화 상자
#include <fstream>
#include "OnscreenKeyboardDlg.h"
#include "VirtualKeyBoard/VirtualKeyBoard.h"

IMPLEMENT_DYNAMIC(CVAT_SimulatorDlg, CDialogEx)

CVAT_SimulatorDlg::CVAT_SimulatorDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_SDV_VAT_SIMULATOR, pParent)
{
	m_pImage = NULL;
}

CVAT_SimulatorDlg::~CVAT_SimulatorDlg()
{

	Sleep(200);
	if (!m_pImage.empty())   m_pImage.release();

	m_vEQPList.clear();
	m_vPosNameList.clear();
}

void CVAT_SimulatorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LB_VIEW_TITLE, m_LbViewTitle);
	DDX_Control(pDX, IDC_LB_CONNECT_STATUS, m_LbConnectStatus);
	DDX_Control(pDX, IDC_LB_MODEL, m_LbModelTitle);
	DDX_Control(pDX, IDC_LB_POS_NAME, m_LbPosNamelTitle);
	DDX_Control(pDX, IDC_LB_EQP_NAME, m_LbEqpNameTitle);
	DDX_Control(pDX, IDC_LB_DLTIMEOUT, m_LbDllTitle);
	DDX_Control(pDX, IDC_LB_MC_TIMEOUT, m_LbMcTitle);
	DDX_Control(pDX, IDC_LB_ATT_TIMEOUT, m_LbAttTitle);

	DDX_Control(pDX, IDC_BUTTON_CONNECT, m_btnConnect);
	DDX_Control(pDX, IDC_BUTTON_DISCONNECT, m_btnDisconnect);
	DDX_Control(pDX, IDC_BUTTON_REQDLAGENT, m_btnReqdlagent);
	DDX_Control(pDX, IDC_BUTTON_REQMODELDATA_COPY, m_btnReqmodeldataCopy);
	DDX_Control(pDX, IDC_BUTTON_REPORTMANUAL_RST, m_btnReportmanualRst);
	DDX_Control(pDX, IDC_BUTTON_REPORTLOGIC_RST, m_btnReportlogicRst);
	DDX_Control(pDX, IDC_BUTTON_REQUPDATE_ATT, m_btnRequpdateAtt);
	DDX_Control(pDX, IDC_BUTTON_REPORT_ALLRST, m_btnReportAllrst);
	DDX_Control(pDX, IDC_BUTTON_GETEQP_LIST, m_btnGeteqpList);
	DDX_Control(pDX, IDC_BUTTON_GETPOS_LIST, m_btnGetposList);
	DDX_Control(pDX, IDC_BUTTON_MONITOR_LOG, m_btnMonitorLog);
	DDX_Control(pDX, IDC_BUTTON_IMAGE_LOAD, m_btnImageLoad);
	DDX_Control(pDX, IDC_BUTTON_CLEAR_LOG, m_btnClearLog);
	DDX_Control(pDX, IDC_BUTTON_COPY_CANCLE, m_btnCancel);

	DDX_Control(pDX, IDC_EDIT_MODEL, m_edtModel);
	DDX_Control(pDX, IDC_EDIT_DLTIMEOUT, m_edtDLTimeOut);
	DDX_Control(pDX, IDC_EDIT_MCTIMEOUT, m_edtMCTimeOut);
	DDX_Control(pDX, IDC_EDIT_ATTTIMEOUT, m_edtATTTimeOut);

	DDX_Control(pDX, IDC_COMBO_POSNAME, m_cmbSelectPosName);
	DDX_Control(pDX, IDC_COMBO_EQPNAME, m_cmbSelectEqpName);
	DDX_Control(pDX, IDC_LIST_LOG, m_listLog);
}


BEGIN_MESSAGE_MAP(CVAT_SimulatorDlg, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CVAT_SimulatorDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_DISCONNECT, &CVAT_SimulatorDlg::OnBnClickedButtonDisconnect)
	ON_BN_CLICKED(IDC_BUTTON_REQDLAGENT, &CVAT_SimulatorDlg::OnBnClickedButtonReqdlagent)
	ON_BN_CLICKED(IDC_BUTTON_REQMODELDATA_COPY, &CVAT_SimulatorDlg::OnBnClickedButtonReqmodeldataCopy)
	ON_BN_CLICKED(IDC_BUTTON_REPORTMANUAL_RST, &CVAT_SimulatorDlg::OnBnClickedButtonReportmanualRst)
	ON_BN_CLICKED(IDC_BUTTON_REPORTLOGIC_RST, &CVAT_SimulatorDlg::OnBnClickedButtonReportlogicRst)
	ON_BN_CLICKED(IDC_BUTTON_REQUPDATE_ATT, &CVAT_SimulatorDlg::OnBnClickedButtonRequpdateAtt)
	ON_BN_CLICKED(IDC_BUTTON_REPORT_ALLRST, &CVAT_SimulatorDlg::OnBnClickedButtonReportAllrst)
	ON_BN_CLICKED(IDC_BUTTON_GETEQP_LIST, &CVAT_SimulatorDlg::OnBnClickedButtonGeteqpList)
	ON_BN_CLICKED(IDC_BUTTON_GETPOS_LIST, &CVAT_SimulatorDlg::OnBnClickedButtonGetposList)
	ON_BN_CLICKED(IDC_BUTTON_MONITOR_LOG, &CVAT_SimulatorDlg::OnBnClickedButtonMonitorLog)
	ON_BN_CLICKED(IDC_BUTTON_IMAGE_LOAD, &CVAT_SimulatorDlg::OnBnClickedButtonImageLoad)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_LOG, &CVAT_SimulatorDlg::OnBnClickedButtonClearLog)
	ON_EN_SETFOCUS(IDC_EDIT_MODEL, &CVAT_SimulatorDlg::OnEnSetfocusEditModel)
	ON_EN_SETFOCUS(IDC_EDIT_DLTIMEOUT, &CVAT_SimulatorDlg::OnEnSetfocusEditDltimeout)
	ON_EN_SETFOCUS(IDC_EDIT_MCTIMEOUT, &CVAT_SimulatorDlg::OnEnSetfocusEditMctimeout)
	ON_EN_SETFOCUS(IDC_EDIT_ATTTIMEOUT, &CVAT_SimulatorDlg::OnEnSetfocusEditAtttimeout)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_COPY_CANCLE, &CVAT_SimulatorDlg::OnBnClickedButtonCopyCancle)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CVAT_SimulatorDlg 메시지 처리기

BOOL CVAT_SimulatorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);
	m_pMain = (CLET_AlignClientDlg*)AfxGetMainWnd();

	InitTitle(&m_LbViewTitle, "SDV VAT Simulator Check", 24.f, COLOR_UI_BODY);
	InitTitle(&m_LbConnectStatus, "Connect / DisConnect", 18.f, COLOR_UI_BODY);
	InitTitle(&m_LbModelTitle, "Model", 14.f, COLOR_UI_BODY);
	InitTitle(&m_LbPosNamelTitle, "Pos Name", 14.f, COLOR_UI_BODY);
	InitTitle(&m_LbEqpNameTitle, "EQP Name", 14.f, COLOR_UI_BODY);
	InitTitle(&m_LbDllTitle, "DL Timeout", 14.f, COLOR_UI_BODY);
	InitTitle(&m_LbMcTitle, "MC Timeout", 14.f, COLOR_UI_BODY);
	InitTitle(&m_LbAttTitle, "ATT Timeout", 14.f, COLOR_UI_BODY);

	MainButtonInit(&m_btnConnect);          m_btnConnect.SetSizeText(14.f);
	MainButtonInit(&m_btnDisconnect);       m_btnDisconnect.SetSizeText(14.f);
	MainButtonInit(&m_btnReqdlagent);       m_btnReqdlagent.SetSizeText(14.f);
	MainButtonInit(&m_btnReqmodeldataCopy); m_btnReqmodeldataCopy.SetSizeText(14.f);
	MainButtonInit(&m_btnReportmanualRst);  m_btnReportmanualRst.SetSizeText(14.f);
	MainButtonInit(&m_btnReportlogicRst);   m_btnReportlogicRst.SetSizeText(14.f);
	MainButtonInit(&m_btnRequpdateAtt);     m_btnRequpdateAtt.SetSizeText(14.f);
	MainButtonInit(&m_btnReportAllrst);     m_btnReportAllrst.SetSizeText(14.f);
	MainButtonInit(&m_btnGeteqpList);       m_btnGeteqpList.SetSizeText(14.f);
	MainButtonInit(&m_btnGetposList);       m_btnGetposList.SetSizeText(14.f);
	MainButtonInit(&m_btnMonitorLog);       m_btnMonitorLog.SetSizeText(14.f);
	MainButtonInit(&m_btnImageLoad);        m_btnImageLoad.SetSizeText(14.f);
	MainButtonInit(&m_btnClearLog);         m_btnClearLog.SetSizeText(14.f);
	MainButtonInit(&m_btnCancel);           m_btnCancel.SetSizeText(14.f);	

	GetDlgItem(IDC_EDIT_MODEL)->SetWindowText("AMF755ZE01");
	GetDlgItem(IDC_EDIT_DLTIMEOUT)->SetWindowText("5000");
	GetDlgItem(IDC_EDIT_MCTIMEOUT)->SetWindowText("5000");
	GetDlgItem(IDC_EDIT_ATTTIMEOUT)->SetWindowText("6000");
	
	InitEQPList();
	InitPosList();

	m_listLog.AddString("SDV VAT Program Start");

#ifdef _SDV_VAT
	KillTimer(1);
	SetTimer(1, 1000, NULL);
#endif

	return TRUE;  // return TRUE unless you set the focus to a control
					  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CVAT_SimulatorDlg::InitTitle(CLabelEx* pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}


void CVAT_SimulatorDlg::MainButtonInit(CButtonEx* pbutton, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}


HBRUSH CVAT_SimulatorDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (pWnd->GetDlgCtrlID() == IDC_EDIT_MODEL ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_DLTIMEOUT ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_MCTIMEOUT ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_ATTTIMEOUT ||
		pWnd->GetDlgCtrlID() == IDC_COMBO_POSNAME ||
		pWnd->GetDlgCtrlID() == IDC_COMBO_EQPNAME )
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

void CVAT_SimulatorDlg::OnBnClickedButtonConnect()
{
#ifdef _SDV_VAT
	CString m_modelName;
	GetDlgItem(IDC_EDIT_MODEL)->GetWindowText(m_modelName);
	m_pMain->m_classSamsungVATWraper.Agent_Initial((LPSTR)(LPCTSTR)m_modelName, "VisionVersion1.0");

	KillTimer(1);
	SetTimer(1, 1000, NULL);
#endif
}


void CVAT_SimulatorDlg::OnBnClickedButtonDisconnect()
{
#ifdef _SDV_VAT
	KillTimer(1);
	m_pMain->m_classSamsungVATWraper.Agent_Disconnect();	

	bool bconnect = m_pMain->m_classSamsungVATWraper.Agent_IsConnected();

	if (bconnect)
	{
		m_LbConnectStatus.SetText("Connected");
		m_LbConnectStatus.SetColorBkg(255, RGB(0, 255, 0));

		/*m_LbViewTitle.SetText("SDV VAT Simulator Check (" + m_pMain->m_classSamsungVATWraper.m_bLibVer + ")");
		::SendMessage(m_pMain->m_pPaneHeader->m_hWnd, WM_VIEW_CONTROL, MSG_VAT_CONNECT_STATUS_CONNECT, TRUE);*/
	}
	else
	{
		m_LbConnectStatus.SetText("DisConnected");
		m_LbConnectStatus.SetColorBkg(255, RGB(255, 0, 0));
		//::SendMessage(m_pMain->m_pPaneHeader->m_hWnd, WM_VIEW_CONTROL, MSG_VAT_CONNECT_STATUS_CONNECT, FALSE);
	}
#endif
}


void CVAT_SimulatorDlg::OnBnClickedButtonReqdlagent()
{
#ifdef _SDV_VAT
	if (m_pImage.empty()) 
	{
		AfxMessageBox("Warning!! Select Image!");
		return;
	}

	CString error="Normal", model, posName, rst, _tact0;
	int timeout=1000, size, width, stride, height;
	byte* imgbytes = m_pImage.data;
		
	width = m_pImage.cols;
	height = m_pImage.rows;
	stride = m_pImage.cols;
	size = width* height;

	//딥러닝 결과 요청
	m_listLog.AddString("Request Deep Learning");

	GetDlgItem(IDC_EDIT_DLTIMEOUT)->GetWindowText(error);
	timeout = atoi(error);
	//Mark
	m_cmbSelectPosName.GetLBText(m_cmbSelectPosName.GetCurSel(), posName); // 값 가져오기

	if (posName.IsEmpty())
	{
		AfxMessageBox("Warning!! Select Position!");
		return;
	}

	error = "Normal                                               "; // 충분한 버퍼 초기화 되어야함 
	GetDlgItem(IDC_EDIT_MODEL)->GetWindowText(model);

	model = "TT_QTM340YA01-C01";
	posName = "SA_YL";

	DLRst result0 = m_pMain->m_classSamsungVATWraper.Agent_ReqDL_Bytes("TEST1234", ToolType_Mark, (LPSTR)(LPCTSTR)model, timeout, (LPSTR)(LPCTSTR)posName, imgbytes, size, width, stride, height, error);

	CTime time = CTime::GetCurrentTime();

	_tact0.Format("%2d: %2d : %2d ", time.GetHour(), time.GetMinute(),time.GetSecond());

	rst.Format("[%s] x: %4.4f, y: %4.4f, t: %4.4f, score: %4.4f, posName: %s , error: %s\n", _tact0, result0.X, result0.Y, result0.T, result0.Score, posName, error);
	m_listLog.AddString(rst);

#endif
}


void CVAT_SimulatorDlg::OnBnClickedButtonReqmodeldataCopy()
{
#ifdef _SDV_VAT

	m_pMain->m_pCircleProgress.StartProgress();
	m_pThread = AfxBeginThread(ProgressThread, this);		// 프로그레스바 쓰레드 시작
#endif
}


void CVAT_SimulatorDlg::OnBnClickedButtonReportmanualRst()
{
#ifdef _SDV_VAT

	CString imagepath;
	ManualReason reason = ManualReason_Score;

	if (imagepath.IsEmpty())
	{
		CFileDialog dlg(TRUE, NULL, _T("Image File"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
			_T("Image Files (*.bmp;*.jpg;)|*.bmp;*.jpg|All Files (*.*)|*.*||"), this);

		if (dlg.DoModal() != IDOK)		return;

		imagepath = dlg.GetPathName();
	}

	m_listLog.AddString("Report Manual Rst_Mark");

	//GetDlgItem(IDC_EDIT_MODEL)->GetWindowText(model);
	//m_cmbSelectPosName.GetLBText(m_cmbSelectPosName.GetCurSel(), posName); // 값 가져오기

	if (m_pImage.empty())
	{
		AfxMessageBox("Warning!! Select Image!");
		return;
	}

	float rstX = 1;
	float rstY = 2;
	float sX = 3;
	float sY = 4;	
	float lX = 6;
	float lY = 7;
	char* cellID = "TEST_1234";
	char* model = "TT_340A_T143_V";
	char* pos = "GATE_ALIGN_YL";
	byte* imgbytes = m_pImage.data;
	
	int width = m_pImage.cols;
	int strid = m_pImage.cols;
	int height = m_pImage.rows;
	int size = width * height;
	
	CStringW error = L"";

	//CString error = "Normal", model, posName, rst, _tact0;
	//int timeout = 1000, size, width, stride, height;
	//byte* imgbytes = m_pImage.data;	

	//error = "";
	//error = "Normal                                               "; // 충분한 버퍼 초기화 되어야함 

	m_pMain->m_classSamsungVATWraper.Agent_SendManualRst_Bytes(rstX, rstY, sX, sY, lX, lY, cellID, model, pos, imgbytes, size, width, strid, height, reason, error);


#endif
}


void CVAT_SimulatorDlg::OnBnClickedButtonReportlogicRst()
{
#ifdef _SDV_VAT

	char description[MAX_PATH] = { 0, };
	CString error, model, posName, curtime, imagepath;
	m_listLog.AddString("Report Logic Rst_Mark");

	CTime t1 = CTime::GetCurrentTime();
	//curtime = t1.Format("%H : %M: %S");
	curtime = t1.Format("%H%M%S");
	GetDlgItem(IDC_EDIT_MODEL)->GetWindowText(model);
	m_cmbSelectPosName.GetLBText(m_cmbSelectPosName.GetCurSel(), posName); // 값 가져오기

	if (posName.IsEmpty())
	{
		AfxMessageBox("Warning!! Select Position!");
		return;
	}
	if (imagepath.IsEmpty())
	{
		CFileDialog dlg(TRUE, NULL, _T("Image File"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
			_T("Image Files (*.bmp;*.jpg;)|*.bmp;*.jpg|All Files (*.*)|*.*||"), this);

		if (dlg.DoModal() != IDOK)		return;

		imagepath = dlg.GetPathName();
	}

	//curTime
	m_pMain->m_classSamsungVATWraper.Agent_SendLogicRst((LPSTR)(LPCTSTR)curtime, "cellID", FindType_Logic_Mark, (LPSTR)(LPCTSTR)model, (LPSTR)(LPCTSTR)posName, (LPSTR)(LPCTSTR)imagepath,
		                                               Judge_OK, 1, 2, 3, 4, description);
#endif
}


void CVAT_SimulatorDlg::OnBnClickedButtonRequpdateAtt()
{
#ifdef _SDV_VAT
	CString posName, error, model;
	int att_timeout;
	Gdiplus::Bitmap* img=NULL;

	//골든마크 업데이트 요청
	m_listLog.AddString("Request Golden Mark Update");

	GetDlgItem(IDC_EDIT_ATTTIMEOUT)->GetWindowText(error);
	att_timeout = atoi(error);

	GetDlgItem(IDC_EDIT_MODEL)->GetWindowText(model);
	m_cmbSelectPosName.GetLBText(m_cmbSelectPosName.GetCurSel(), posName); // 값 가져오기

	if (posName.IsEmpty())
	{
		AfxMessageBox("Warning!! Select Position!");
		return;
	}

	if (img == NULL)
	{
		CFileDialog dlg(TRUE, NULL, _T("Image File"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
			_T("Image Files (*.bmp;*.jpg;)|*.bmp;*.jpg|All Files (*.*)|*.*||"), this);

		if (dlg.DoModal() != IDOK)		return;
		USES_CONVERSION;
		img = Gdiplus::Bitmap::FromFile(T2W(dlg.GetPathName().GetBuffer()));  

		if (img == NULL) return;
	}

	error = "Normal                                                             "; // 충분한 버퍼 초기화 되어야함 
	CString filePath = m_pMain->m_classSamsungVATWraper.Agent_ATTUpdate((LPSTR)(LPCTSTR)model, att_timeout, (LPSTR)(LPCTSTR)posName, img, error);

	m_listLog.AddString(filePath + "error: " + error);
#endif
}


void CVAT_SimulatorDlg::OnBnClickedButtonReportAllrst()
{
#ifdef _SDV_VAT

	//알고리즘 마지막 부분에서 전체 마크에 대한 결과를 보고한다 > 모니터링 용
	m_listLog.AddString("Report All Rst");

	/*CString  cellID, markPos;

	cellID = "Temp_XXXXTTXTXT";
	markPos = "U1_Upper_Object0";

	MarkFinderResult mfResults[4];

	mfResults[0].MFType = MarkFinderType_Vision_Mark;
	mfResults[0].Use = OnOff_ON;
	mfResults[0].Result = MarkFindResult_NG;
	mfResults[0].Lcheck = MarkFindResult_NG;

	mfResults[1].MFType = MarkFinderType_Golden_Mark;
	mfResults[1].Use = OnOff_ON;
	mfResults[1].Result = MarkFindResult_NG;
	mfResults[1].Lcheck = MarkFindResult_NG;

	mfResults[2].MFType = MarkFinderType_Deep_Learning;
	mfResults[2].Use = OnOff_ON;
	mfResults[2].Result = MarkFindResult_NG;
	mfResults[2].Lcheck = MarkFindResult_NG;

	mfResults[3].MFType = MarkFinderType_Manual_Align;
	mfResults[3].Use = OnOff_ON;
	mfResults[3].Result = MarkFindResult_OK;
	mfResults[3].Lcheck = MarkFindResult_OK;

	eErr_code err = m_pMain->m_classSamsungVATWraper.Agent_WriteMatchingHistoryLog(4, mfResults, eMarkFinderType::Vision_Mark, "processName", (LPSTR)(LPCTSTR)markPos, (LPSTR)(LPCTSTR)cellID);

	if (err != eErr_code(Nomal))
	{
		CString strText;
		strText.Format(_T("%d"), err);
		AfxMessageBox(strText);
	}*/

#endif
}


void CVAT_SimulatorDlg::OnBnClickedButtonGeteqpList()
{
#ifdef _SDV_VAT
	
#endif
}


void CVAT_SimulatorDlg::OnBnClickedButtonGetposList()
{
#ifdef _SDV_VAT
	
#endif
}


void CVAT_SimulatorDlg::OnBnClickedButtonMonitorLog()
{
#ifdef _SDV_VAT
	/*
#pragma region Vision_mark_parameter_log
	
	CFileDialog dlg(TRUE, NULL, _T("Image File"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Image Files (*.bmp;*.jpg;)|*.bmp;*.jpg|All Files (*.*)|*.*||"), this);

	if (dlg.DoModal() != IDOK)		return;

	CString str = dlg.GetPathName();
	LPWSTR wzStr = new WCHAR[255];
	LPTSTR mbyteStr = str.GetBuffer();

	int length = MultiByteToWideChar(CP_ACP, 0, mbyteStr, -1, NULL, NULL);
	MultiByteToWideChar(CP_ACP, 0, mbyteStr, -1, wzStr, length);

	Gdiplus::Bitmap bmpImg(wzStr);	

	m_pImage = cv::imread(string(str), 0);

	if (m_pImage.empty())
	{
		AfxMessageBox("Check the Image Type");
		return;
	}

	m_listLog.AddString("Image Loaded");

	CString error = "Normal", model, posName, rst, _tact0;
	int timeout = 1000, size, width, stride, height;
	byte* imgbytes = m_pImage.data;	

	//width = m_pImage.cols;
	//// = m_pImage.rows;
	//stride = m_pImage.cols;
	//size = width * height;


	MarkTool mTool = MarkTool();
	mTool.processName = "GATE_ALIGN";
	mTool.posName = "SA_YL";
	mTool.modelName = m_pMain->getCurrModelName();
	mTool.grabDelay = 0.05;

	PatternParam patternParam = PatternParam();
	LineParam lineParam = LineParam();
	LightInfo lightInfo = LightInfo();

	lightInfo.lightType = eLightType::Coax;
	lightInfo.setValue = 40;

	mTool.lst_light.push_back(lightInfo);
	mTool.ptnParam = patternParam;
	mTool.lst_lineParam.push_back(lineParam);

	eErr_code err;
	err = m_pMain->m_classSamsungVATWraper.Agent_UpdateMarkParameter_Bitmap(mTool, &bmpImg);
	//err = m_pMain->m_classSamsungVATWraper.Agent_UpdateMarkParameter_Bytes(mTool, imgbytes);

	delete[] wzStr;	 
#pragma endregion Vision_mark_parameter_log

#pragma region Vision_Camera_Spec_Info
	std::vector<VisionCamInfo> vtCamInfo;
	VisionCamInfo tmpCamInfo = VisionCamInfo();

	vtCamInfo.clear();

	tmpCamInfo.camPosition = L"STAGE_A_ALIGN_LEFT"; //"SA_YL";
	tmpCamInfo.camModel = L"BASLER";//"BASLER_acA3088 - 16gm";
	tmpCamInfo.expoTime = 50000; //ns
	tmpCamInfo.fov = XY(7.411, 4.953);
	tmpCamInfo.resolution = XY(3088, 2064);
	tmpCamInfo.pixelCount = XY(0.0024, 0.0024);
	tmpCamInfo.lensScale = 1.0;

	vtCamInfo.push_back(tmpCamInfo);

	tmpCamInfo.camPosition = L"STAGE_A_ALIGN_RIGHT";
	tmpCamInfo.camModel = L"BASLER_acA3088-16gm";
	tmpCamInfo.expoTime = 50000; //ns
	tmpCamInfo.fov = XY(7.411, 4.953);
	tmpCamInfo.resolution = XY(3088, 2064);
	tmpCamInfo.pixelCount = XY(0.0024, 0.0024);
	tmpCamInfo.lensScale = 1.0;

	vtCamInfo.push_back(tmpCamInfo);

	eErr_code err;
	err = m_pMain->m_classSamsungVATWraper.Agent_WriteVisionCamInfo(vtCamInfo);

	vtCamInfo.clear();	
#pragma endregion Vision_Camera_Spec_Info

#pragma region Vision_PC_Spec_Info
	PCStatus tmpPcStatus = PCStatus();
	tmpPcStatus.curHdd = 150;
	tmpPcStatus.maxHdd = 300;
	tmpPcStatus.curMemory = 12;
	tmpPcStatus.maxMemory = 32;

	err = m_pMain->m_classSamsungVATWraper.Agent_WritePCStatus(tmpPcStatus);
#pragma endregion Vision_PC_Spec_Info
*/
#pragma region Vision_Process_Image_Log

	CFileDialog dlg(TRUE, NULL, _T("Image File"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Image Files (*.bmp;*.jpg;)|*.bmp;*.jpg|All Files (*.*)|*.*||"), this);

	if (dlg.DoModal() != IDOK)		return;

	CString str = dlg.GetPathName();
	LPWSTR wzStr = new WCHAR[255];
	LPTSTR mbyteStr = str.GetBuffer();

	cv::Mat matImg = cv::imread(mbyteStr);

	int length = MultiByteToWideChar(CP_ACP, 0, mbyteStr, -1, NULL, NULL);
	MultiByteToWideChar(CP_ACP, 0, mbyteStr, -1, wzStr, length);

	Gdiplus::Bitmap bmpImg(wzStr);

	//m_pImage = cv::imread(string(str), 0);	

	eMarkFinderType mfType = eMarkFinderType::Vision_Mark;
	CStringW processName = L"GATE_ALIGN";
	CStringW camPos = L"GATE_ALIGN_YL";
	CStringW cellID = L"TEST_MARK1234";
	
	eErr_code err;
	//err = m_pMain->m_classSamsungVATWraper.Agent_SendProcessImage_Bitmap(&bmpImg, mfType, processName, camPos, cellID);
	err = m_pMain->m_classSamsungVATWraper.Agent_SendProcessImage_Mat(matImg, mfType, processName, camPos, cellID);

	delete[] wzStr;
#pragma endregion Vision_Process_Image_Log
	
#endif
}


void CVAT_SimulatorDlg::OnBnClickedButtonImageLoad()
{
#ifdef _SDV_VAT

	CFileDialog dlg(TRUE, NULL, _T("Image File"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Image Files (*.bmp;*.jpg;)|*.bmp;*.jpg|All Files (*.*)|*.*||"), this);

	if (dlg.DoModal() != IDOK)		return;

	CString str = dlg.GetPathName();
	WCHAR *Wstr = (CA2W)str;
	
	
	Gdiplus::Bitmap* bitmapImg;
	bitmapImg->FromFile(Wstr);

	m_pImage = cv::imread(string(str), 0);

	if (m_pImage.empty())
	{
		AfxMessageBox("Check the Image Type");
		return;
	}
	
	m_listLog.AddString("Image Loaded");
#endif
}


void CVAT_SimulatorDlg::OnBnClickedButtonClearLog()
{
#ifdef _SDV_VAT
	m_listLog.ResetContent();
#endif
}


void CVAT_SimulatorDlg::OnEnSetfocusEditModel()
{
	SetFocus();

	CVirtualKeyBoard m_pKeyboardDlg;

	if (m_pKeyboardDlg.DoModal() != IDOK)
		return;

	CString strNumber;
	m_pKeyboardDlg.GetValue(strNumber);
	GetDlgItem(IDC_EDIT_MODEL)->SetWindowTextA(strNumber);

}


void CVAT_SimulatorDlg::OnEnSetfocusEditDltimeout()
{
	GetDlgItem(IDC_LB_DLTIMEOUT)->SetFocus();
	CString str, strTemp;
	GetDlgItem(IDC_EDIT_DLTIMEOUT)->GetWindowText(str);
	strTemp = m_pMain->GetNumberBox(str, 7, 0, 1000000);

	GetDlgItem(IDC_EDIT_DLTIMEOUT)->SetWindowTextA(strTemp);
}


void CVAT_SimulatorDlg::OnEnSetfocusEditMctimeout()
{
	GetDlgItem(IDC_LB_MC_TIMEOUT)->SetFocus();
	CString str, strTemp;
	GetDlgItem(IDC_EDIT_MCTIMEOUT)->GetWindowText(str);
	strTemp = m_pMain->GetNumberBox(str, 7, 0, 1000000);

	GetDlgItem(IDC_EDIT_MCTIMEOUT)->SetWindowTextA(strTemp);
}


void CVAT_SimulatorDlg::OnEnSetfocusEditAtttimeout()
{
	GetDlgItem(IDC_LB_MC_TIMEOUT)->SetFocus();
	CString str, strTemp;
	GetDlgItem(IDC_EDIT_ATTTIMEOUT)->GetWindowText(str);
	strTemp = m_pMain->GetNumberBox(str, 7, 0, 1000000);

	GetDlgItem(IDC_EDIT_ATTTIMEOUT)->SetWindowTextA(strTemp);
}


void CVAT_SimulatorDlg::OnTimer(UINT_PTR nIDEvent)
{
#ifdef _SDV_VAT

	m_pMain->m_classSamsungVATWraper.m_bConnected = m_pMain->m_classSamsungVATWraper.Agent_IsConnected();

	if (m_pMain->m_classSamsungVATWraper.m_bConnected)
	{
		m_LbConnectStatus.SetText("Connected");
		m_LbConnectStatus.SetColorBkg(255, RGB(0, 255, 0));		
	}
	else
	{
		m_LbConnectStatus.SetText("DisConnected");
		m_LbConnectStatus.SetColorBkg(255, RGB(255, 0, 0));
	}
#endif
	CDialogEx::OnTimer(nIDEvent);
}

void CVAT_SimulatorDlg::InitEQPList()
{
#ifdef _SDV_VAT
	m_cmbSelectEqpName.ResetContent();
	m_vEQPList.clear();

	if (_access(Path_EqpList, 0) != 0)			return;

	ifstream fin;
	fin.open(Path_EqpList);

	std::vector<string> strEQP;
	string line;
	while (!fin.eof())
	{
		getline(fin, line);
		strEQP.push_back(line);
	}

	fin.close();


	for (int i = 0; i < strEQP.size(); i++)
	{
		m_cmbSelectEqpName.AddString(strEQP[i].c_str());
		m_vEQPList.push_back(strEQP[i].c_str());
	}

	if(m_vEQPList.size())	m_cmbSelectEqpName.SetCurSel(0);
#endif
}

void CVAT_SimulatorDlg::InitPosList()
{
#ifdef _SDV_VAT
	m_cmbSelectPosName.ResetContent();
	m_vPosNameList.clear();

	if (_access(Path_PosList, 0) != 0)			return;

	ifstream fin;
	fin.open(Path_PosList);

	std::vector<string> strPosName;
	string line;
	while (!fin.eof())
	{
		getline(fin, line);
		strPosName.push_back(line);
	}

	fin.close();


	for (int i = 0; i < strPosName.size(); i++)
	{
		m_cmbSelectPosName.AddString(strPosName[i].c_str());
		m_vPosNameList.push_back(strPosName[i].c_str());
	}

	if (m_vPosNameList.size())	m_cmbSelectPosName.SetCurSel(0);
#endif
}


void CVAT_SimulatorDlg::OnBnClickedButtonCopyCancle()
{
#ifdef _SDV_VAT
	m_pMain->m_pCircleProgress.StopProgress();
	m_pMain->m_classSamsungVATWraper.Agent_SetModelCopyCancel(true);
#endif
}

UINT CVAT_SimulatorDlg::ProgressThread(LPVOID lpParam)
{
	// 프로그레스바
	CVAT_SimulatorDlg* pModelCopyDlg = (CVAT_SimulatorDlg*)lpParam;

#ifdef _SDV_VAT
	CString error, destpath, eqp, model;
	int mc_timeout;

	//모델 복사 요청
	pModelCopyDlg->m_listLog.AddString("Request Model Copy");

	destpath = "D:\\Agent\\ModelData"; //복사해 올 모델 파일 위치

	pModelCopyDlg->GetDlgItem(IDC_EDIT_MCTIMEOUT)->GetWindowText(error);
	mc_timeout = atoi(error);
	pModelCopyDlg->GetDlgItem(IDC_EDIT_MODEL)->GetWindowText(model);
	pModelCopyDlg->m_cmbSelectEqpName.GetLBText(pModelCopyDlg->m_cmbSelectEqpName.GetCurSel(), eqp); // 값 가져오기

	if (eqp.IsEmpty())
	{
		AfxMessageBox("Warning!! Select EQP!");
		return TRUE;
	}

	//error = "Normal                                                             "; // 충분한 버퍼 초기화 되어야함
	CString filePath = pModelCopyDlg->m_pMain->m_classSamsungVATWraper.Agent_ModelCopy((LPSTR)(LPCTSTR)model, mc_timeout, (LPSTR)(LPCTSTR)destpath, (LPSTR)(LPCTSTR)eqp, error);

	pModelCopyDlg->m_listLog.AddString(filePath + "error: " + error);
	pModelCopyDlg->m_pMain->m_pCircleProgress.StopProgress();
#endif

	return TRUE;
}

void CVAT_SimulatorDlg::fnStartVMS_log_first()
{
#ifdef _SDV_VAT
	for (int idx = 0; idx < 4; idx++) {
		fnUpdate_mark_parameter_log(idx);
		fnWrite_camera_info(idx);
	}

	fnWrite_pc_status();
#endif
}

void CVAT_SimulatorDlg::fnUpdate_mark_parameter_log(int camNo)
{
#ifdef _SDV_VAT
	int jobID = 0;

	if (camNo > 1) {
		jobID = 1;
		camNo = camNo - 2;
	}

	MarkTool mTool = MarkTool();	
	
	USES_CONVERSION;	
	
	mTool.processName = A2W(m_pMain->vt_job_info[jobID].get_job_name());//m_pMain->m_classSamsungVATWraper.convertM2W(test1);//(m_pMain->vt_job_info[jobID].get_job_name());
	//mTool.posName = A2W(m_pMain->vt_job_info[jobID].camera_name.at(camNo).c_str());
	mTool.posName = A2W(m_pMain->m_stCamInfo[camNo].cName);
	mTool.modelName = A2W(m_pMain->getCurrModelName());
	mTool.grabDelay = 0.05;	// check
	mTool.index = camNo;

	PatternParam patternParam = PatternParam();
	LineParam lineParam = LineParam();
	LightInfo lightInfo = LightInfo();

	lightInfo.lightType = eLightType::Coax;
	lightInfo.setValue = m_pMain->vt_job_info[jobID].model_info.getLightInfo().getLightBright(camNo, 0, 0);
	
	patternParam.angle_High = 0.0;
	patternParam.angle_Low = 0.0;
	patternParam.scale_High = 0.0;
	patternParam.scale_Low = 0.0;

	CRect rectROI = theApp.m_pFrame->GetMatching(jobID).getSearchROI(camNo, 0);
	patternParam.roi_Origin.X = rectROI.left;
	patternParam.roi_Origin.Y = rectROI.top;
	patternParam.roi_Height = rectROI.Height();
	patternParam.roi_Width = rectROI.Width();

	CString strImgPath;
	strImgPath.Format("%s%s\\PAT%d%d%d.bmp", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[jobID].job_name.c_str(), camNo, 0, 0);

	cv::Mat matImage = cv::imread(string(strImgPath), 0);
	byte* byteImg = matImage.data;

	
	LPWSTR bmpPath = A2W(strImgPath);
	Gdiplus::Bitmap bmpImg(bmpPath);	// byte type 변경 예정

	BITBYTE bitByte;
	bitByte.bytes = byteImg;
	bitByte.format = bmpImg.GetPixelFormat();
	bitByte.height = bmpImg.GetHeight();
	bitByte.width = bmpImg.GetWidth();
	bitByte.stride = bmpImg.GetWidth();

	patternParam.mark_Origin = XY(0.0, 0.0);
	patternParam.mark_Width = matImage.cols;		//pImage->width;
	patternParam.mark_Height = matImage.rows;		//pImage->height;
	
	patternParam.origin_Point.X = m_pMain->GetMatching(jobID).getPosOffsetX(camNo, 0, 0);
	patternParam.origin_Point.Y = m_pMain->GetMatching(jobID).getPosOffsetY(camNo, 0, 0);	

	mTool.lst_light.push_back(lightInfo);
	mTool.ptnParam = patternParam;
	mTool.lst_lineParam.push_back(lineParam);

	eErr_code err;
	//err = m_pMain->m_classSamsungVATWraper.Agent_UpdateMarkParameter_Bitmap(mTool, &bmpImg);
	//err = m_pMain->m_classSamsungVATWraper.Agent_UpdateMarkParameter_Bytes(mTool, bitByte);
#ifdef _SDV_VAT
	err = m_pMain->m_classSamsungVATWraper.Agent_UpdateMarkParameter_Mat(mTool, matImage);
#endif

	//cvReleaseImage(&pImage);
#endif
}

void CVAT_SimulatorDlg::fnWrite_camera_info(int camNo)
{
#ifdef _SDV_VAT
	USES_CONVERSION;

	int jobID = 0;
	int camIndex = camNo; // Job 내부 카메라 번호

	if (camNo > 1) {
		jobID = 1;
		camIndex = camNo - 2;
	}

	m_pMain->vt_job_info[jobID].model_info.getMachineInfo().getExposureTime(camIndex);

	std::vector<VisionCamInfo> vtCamInfo;
	VisionCamInfo tmpCamInfo = VisionCamInfo();

	vtCamInfo.clear();

	int chkFlip = m_pMain->m_stCamInfo[camNo].flip_dir;
	double nResolutionX = m_pMain->GetMachine(jobID).getCameraResolutionX(camIndex, 0);
	double nResolutionY = m_pMain->GetMachine(jobID).getCameraResolutionY(camIndex, 0);
	int nCountX = m_pMain->m_stCamInfo[camNo].w;
	int nCountY = m_pMain->m_stCamInfo[camNo].h;
	double fovX = nCountX * nResolutionX;
	double fovY = nCountY * nResolutionY;

	XY fovXY = XY(fovX, fovY);
	XY countXY = XY(nCountX, nCountY);

	/*if (chkFlip == 1 || chkFlip == 2) {
		fovXY = XY(fovY, fovX);
		countXY = XY(nCountY, nCountX);
	}*/

	//tmpCamInfo.camPosition = A2W(m_pMain->vt_job_info[jobID].camera_name.at(camIndex).c_str()); // L"STAGE_A_ALIGN_LEFT"; //"SA_YL";
	tmpCamInfo.camPosition = A2W(m_pMain->m_stCamInfo[camNo].cName);
	tmpCamInfo.camModel = L"BASLER_acA3088-16gm";
	tmpCamInfo.expoTime = m_pMain->vt_job_info[jobID].model_info.getMachineInfo().getExposureTime(camIndex); //ns
	tmpCamInfo.fov = fovXY;
	tmpCamInfo.pixelCount = countXY;
	tmpCamInfo.resolution.X = nResolutionX;//m_pMain->GetMachine(jobID).getCameraResolutionX(camIndex, 0); //XY(0.0024, 0.0024);
	tmpCamInfo.resolution.Y = nResolutionY;//m_pMain->GetMachine(jobID).getCameraResolutionY(camIndex, 0); //XY(0.0024, 0.0024);
	tmpCamInfo.lensScale = 1.0;

	vtCamInfo.push_back(tmpCamInfo);	

	eErr_code err;
#ifdef _SDV_VAT
	err = m_pMain->m_classSamsungVATWraper.Agent_WriteVisionCamInfo(vtCamInfo);
#endif	

	vtCamInfo.clear();
#endif
}

void CVAT_SimulatorDlg::fnWrite_pc_status()
{	
#ifdef _SDV_VAT
	NetworkPerformanceScanner cPcPerformance;
	PCStatus tmpPcStatus = PCStatus();
	ULONGLONG ltotal = 0; 
	ULONGLONG lfree = 0;
	int total, free;	
	PULONGLONG lmem = 0;

	//::GetPhysicallyInstalledSystemMemory(lmem);

	cPcPerformance.GetMemoryUsage(ltotal, lfree);
	cPcPerformance.GetDiskSpace("D:\\", total, free);	

	double tmpTotal = ltotal / 1024 / 1024 / 1024;	// byte -> Gbyte
	double tmpFree = lfree / 1024 / 1024 / 1024;

	tmpPcStatus.curHdd = total - free;
	tmpPcStatus.maxHdd = total;
	tmpPcStatus.curMemory = (int)(tmpTotal - tmpFree);
	tmpPcStatus.maxMemory = (int)tmpTotal;

	eErr_code err;

	err = m_pMain->m_classSamsungVATWraper.Agent_WritePCStatus(tmpPcStatus);
#endif	
}

//void CVAT_SimulatorDlg::fnSend_process_image()
//{
//	
//}


void CVAT_SimulatorDlg::OnClose()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	KillTimer(1);

	Sleep(1000);
	CDialogEx::OnClose();
}
