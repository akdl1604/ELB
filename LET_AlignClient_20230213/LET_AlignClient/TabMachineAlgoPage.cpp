// TabMachineAlgoPage.cpp: 구현 파일
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "TabMachineAlgoPage.h"
#include "afxdialogex.h"
#include "CramerSRuleDlg.h"
#include "ImageProcessing/MathUtil.h"
#include "Sequence.h"
#include <fstream>
#include "CommPLC.h"
#include "ComPLC_RS.h"

#include "Job/CJob_1Cam1ShotAlign.h"

#define _USE_CALIB_MARK // 기존 Calibration 사용시 제거, 삭제시 카메라 창에서 모델 생성 해줘야 함

// TabMachineAlgoPage 대화 상자
#define TIMER_CALIBRATION 100
#define TIMER_ROTATE_CENTER 200
#define TIMER_MANUAL_MOVE 300
#define TIMER_READ_PLC_BIT	400
#define TIMER_PREALIGN_TEST 500
#define TIMER_CALIBRATION2 600
#define TIMER_ROTATE_CENTER2 700
#define TIMER_MANUAL_MOVE2 800
#define TIMER_CALIBRATION3 900
#define TIMER_CALIBRATION4 1100
#define TIMER_ROTATE_CENTER3 1200
#define TIMER_ROTATE_CENTER4 1300
#define TIMER_IMAGE_CALIBRATION 1400
#define TIMER_WAIT_COMPLETE 1500

extern CJob_1Cam1ShotAlign g_Job_1Cam1ShotAlign[MAX_JOB];


template<typename ... Args> std::string string_format(const std::string& format, Args ... args)
{
	size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0' 
	if (size <= 0) { throw std::runtime_error("Error during formatting."); }
	std::unique_ptr<char[]> buf(new char[size]);
	snprintf(buf.get(), size, format.c_str(), args ...);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside 
}

bool compareFloatX(cv::Point2f avec, cv::Point2f bvec)
{
	return avec.x < bvec.x;
}
bool compareFloatY(cv::Point2f avec, cv::Point2f bvec)
{
	return avec.y < bvec.y;
}

IMPLEMENT_DYNAMIC(TabMachineAlgoPage, CDialogEx)

TabMachineAlgoPage::TabMachineAlgoPage(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TAB_PANE_MACHINE, pParent)
{
	m_bMachineControlStart = FALSE;
	m_nJobID = 0;

	m_nSeqCalibration = 0;
	m_nSeqRotateCenter = 0;
	m_nSeqPrealignTest = 0;
	m_nSeqImageCalibration = 0;
	m_nSeqManualMove = 0;
	m_nRotateCnt = 0;
	m_dRadius = 0;

	InitializeCriticalSection(&m_csProcessHistory);
	m_pCramerDlg = NULL;

	m_bInitMarkRead = FALSE;
}

TabMachineAlgoPage::~TabMachineAlgoPage()
{
	if (m_pCramerDlg)
	{
		delete m_pCramerDlg;
	}

	for(int i=0;i<4;i++) m_listPtOnCircle[i].clear();  
	m_listVisionPtOnCircle.clear();  

	DeleteCriticalSection(&m_csProcessHistory);
}

void TabMachineAlgoPage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EDIT_MACHINE_PROCESS_HISTORY, m_EditProcess);
	DDX_Control(pDX, IDC_BTN_AUTO_CALIBRATION, m_btnCalibration);
	DDX_Control(pDX, IDC_BTN_INDIVIDUAL_CALIBRATION, m_btnIndividualCalibration);
	DDX_Control(pDX, IDC_BTN_IMAGE_CALIBRATION, m_btnImageCalibration);
	DDX_Control(pDX, IDC_BTN_INDIVIDUAL_ROTATE_CENTER, m_btnIndividualRotateCenter);
	DDX_Control(pDX, IDC_BTN_ROTATE_CENTER, m_btnRotateCenter);
	DDX_Control(pDX, IDC_BTN_PREALIGN_TEST, m_btnPrealignTest);
	DDX_Control(pDX, IDC_BTN_MOVE, m_btnMove);
	DDX_Control(pDX, IDC_LB_SELECT_MACHINE_CAMERA_TITLE, m_LbSelectCameraTitle);
	DDX_Control(pDX, IDC_LB_SELECT_MACHINE_ORGINPOS_TITLE, m_LbSelectOrgPosTitle);
	DDX_Control(pDX, IDC_LB_SELECT_MACHINE_POSITION_TITLE, m_LbSelectPositionTitle);
	DDX_Control(pDX, IDC_LB_CAMERA_RESOLUTION_X_TITLE, m_LbCameraResolutionXTitle);
	DDX_Control(pDX, IDC_LB_CAMERA_RESOLUTION_Y_TITLE, m_LbCameraResolutionYTitle);
	DDX_Control(pDX, IDC_LB_ROTATE_X_TITLE, m_LbRotateXTitle);
	DDX_Control(pDX, IDC_LB_ROTATE_Y_TITLE, m_LbRotateYTitle);
	DDX_Control(pDX, IDC_LB_CAMERA_RESOLUTION_X, m_LbCameraResolutionX);
	DDX_Control(pDX, IDC_LB_CAMERA_RESOLUTION_Y, m_LbCameraResolutionY);
	DDX_Control(pDX, IDC_LB_ROTATE_X, m_LbRotateX);
	DDX_Control(pDX, IDC_LB_ROTATE_Y, m_LbRotateY);
	DDX_Control(pDX, IDC_LB_MOVE_X, m_LbMoveXTitle);
	DDX_Control(pDX, IDC_LB_MOVE_Y, m_LbMoveYTitle);
	DDX_Control(pDX, IDC_LB_MOVE_T, m_LbMoveTTitle);
	DDX_Control(pDX, IDC_LB_CHESS_BOARD_TITLE, m_lbChessBoardTitle);
	DDX_Control(pDX, IDC_LB_CHESS_SQUARE_TITLE, m_lbChessSizeTitle);
	DDX_Control(pDX, IDC_LB_CHESS_X_TITLE, m_lbChessXTitle);
	DDX_Control(pDX, IDC_LB_CHESS_Y_TITLE, m_lbChessYTitle);
	DDX_Control(pDX, IDC_LB_SQUARE_SIZE, m_LbChessBoardSize);
	DDX_Control(pDX, IDC_LB_CHESS_X2, m_LbChessBoardX);
	DDX_Control(pDX, IDC_LB_CHESS_Y2, m_LbChessBoardY);
	DDX_Control(pDX, IDC_EDIT_MOVE_X, m_EditMoveX);
	DDX_Control(pDX, IDC_EDIT_MOVE_Y, m_EditMoveY);
	DDX_Control(pDX, IDC_EDIT_MOVE_T, m_EditMoveT);
	DDX_Control(pDX, IDC_BTN_MANUAL_ROTATE, m_btnManualRotate);
	DDX_Control(pDX, IDC_BTN_CALC_ROTATE, m_btnCalcRotate);
	DDX_Control(pDX, IDC_CHK_MANUAL_PREALIGN, m_btnEnableManualPreAlign);
	DDX_Control(pDX, IDC_CHECK_ABLE_IGNORE, m_btnEnableIgnoreAable);
	DDX_Control(pDX, IDC_BTN_MOVE_ROTATE, m_btnMoveRotate);
	DDX_Control(pDX, IDC_BTN_OFFSET_ROTATE, m_btnOffsetRotate);
	//KJH 2021-12-29 Pattern Index 추가
	DDX_Control(pDX, IDC_LB_SELECT_MACHINE_PATTERN_INDEX, m_LbPatternIndex);
	DDX_Control(pDX, IDC_CB_SELECT_MACHINE_PATTERN_INDEX, m_cmbPatternIndex);
	DDX_Control(pDX, IDC_BTN_PATTERN_DELETE, m_btnPtnDelete);
	DDX_Control(pDX, IDC_EDIT_CALIB_PIXEL, m_EditCalibPixel);
	DDX_Control(pDX, IDC_EDIT_CALIB_ROBOT, m_EditCalibRobot);
	DDX_Control(pDX, IDC_EDIT_CALIB_SUB, m_EditCalibSub);
	DDX_Control(pDX, IDC_EDIT_CAMERA_RESOLUTION, m_EditCameraResolution);
	DDX_Control(pDX, IDC_LB_CALIB_PIXEL, m_lbCalibPixel);
	DDX_Control(pDX, IDC_LB_CALIB_ROBOT, m_lbCalibRobt);
	DDX_Control(pDX, IDC_LB_CALIB_SUB, m_lbCalibSub);
	DDX_Control(pDX, IDC_LB_CAMERA_RESOLUTION, m_lbCameraResolution);
}


BEGIN_MESSAGE_MAP(TabMachineAlgoPage, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_MESSAGE(WM_VIEW_CONTROL, OnViewControl)
	ON_EN_SETFOCUS(IDC_EDIT_MOVE_X, &TabMachineAlgoPage::OnEnSetfocusEditMoveX)
	ON_EN_SETFOCUS(IDC_EDIT_MOVE_Y, &TabMachineAlgoPage::OnEnSetfocusEditMoveY)
	ON_EN_SETFOCUS(IDC_EDIT_MOVE_T, &TabMachineAlgoPage::OnEnSetfocusEditMoveT)
	ON_BN_CLICKED(IDC_BTN_AUTO_CALIBRATION, &TabMachineAlgoPage::OnBnClickedBtnAutoCalibration)
	ON_BN_CLICKED(IDC_BTN_ROTATE_CENTER, &TabMachineAlgoPage::OnBnClickedBtnRotateCenter)
	ON_BN_CLICKED(IDC_BTN_PREALIGN_TEST, &TabMachineAlgoPage::OnBnClickedBtnPrealignTest)
	ON_BN_CLICKED(IDC_BTN_MOVE, &TabMachineAlgoPage::OnBnClickedBtnMove)
	ON_BN_CLICKED(IDC_BTN_INDIVIDUAL_CALIBRATION, &TabMachineAlgoPage::OnBnClickedBtnIndividualCalibration)
	ON_BN_CLICKED(IDC_BTN_INDIVIDUAL_ROTATE_CENTER, &TabMachineAlgoPage::OnBnClickedBtnIndividualRotateCenter)
	ON_CBN_SELCHANGE(IDC_CB_SELECT_MACHINE_POSITION, &TabMachineAlgoPage::OnCbnSelchangeCbSelectMachinePosition)
	ON_CBN_SELCHANGE(IDC_CB_SELECT_MACHINE_CAMERA, &TabMachineAlgoPage::OnCbnSelchangeCbSelectMachineCamera)
	ON_CBN_SELCHANGE(IDC_CB_SELECT_MACHINE_ORG_POSITION, &TabMachineAlgoPage::OnCbnSelchangeCbSelectMachineOrgPosition)
	ON_BN_CLICKED(IDC_BTN_IMAGE_CALIBRATION, &TabMachineAlgoPage::OnBnClickedBtnImageCalibration)
	ON_EN_SETFOCUS(IDC_LB_SQUARE_SIZE, &TabMachineAlgoPage::OnEnSetfocusEditSquareSize)
	ON_EN_SETFOCUS(IDC_LB_CHESS_X2, &TabMachineAlgoPage::OnEnSetfocusEditXCount)
	ON_EN_SETFOCUS(IDC_LB_CHESS_Y2, &TabMachineAlgoPage::OnEnSetfocusEditYCount)
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_BTN_MANUAL_ROTATE, &TabMachineAlgoPage::OnBnClickedBtnManualRotate)
	ON_BN_CLICKED(IDC_BTN_CALC_ROTATE, &TabMachineAlgoPage::OnBnClickedBtnCalcRotate)
	ON_BN_CLICKED(IDC_BTN_MOVE_ROTATE, &TabMachineAlgoPage::OnBnClickedBtnMoveRotate)
	ON_BN_CLICKED(IDC_BTN_PATTERN_DELETE, &TabMachineAlgoPage::OnBnClickedBtnPatternDelete)
	ON_BN_CLICKED(IDC_BTN_OFFSET_ROTATE, &TabMachineAlgoPage::OnBnClickedBtnOffsetRotate)
	ON_EN_SETFOCUS(IDC_EDIT_CAMERA_RESOLUTION, &TabMachineAlgoPage::OnEnSetfocusEditCameraResolution)
END_MESSAGE_MAP()


// TabMachineAlgoPage 메시지 처리기
BOOL TabMachineAlgoPage::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);
	m_pMain = (CLET_AlignClientDlg *)AfxGetMainWnd();	

	MainButtonInit(&m_btnCalibration, 14);		m_btnCalibration.SetSizeText(14.f);
	MainButtonInit(&m_btnIndividualCalibration, 14);	m_btnIndividualCalibration.SetSizeText(14.f);
	MainButtonInit(&m_btnIndividualRotateCenter, 14);	m_btnIndividualRotateCenter.SetSizeText(14.f);
	MainButtonInit(&m_btnImageCalibration, 14);	m_btnImageCalibration.SetSizeText(14.f);

	MainButtonInit(&m_btnRotateCenter);			m_btnRotateCenter.SetSizeText(14.f);
	MainButtonInit(&m_btnPrealignTest);			m_btnPrealignTest.SetSizeText(14.f);
	MainButtonInit(&m_btnMove);					m_btnMove.SetSizeText(14.f);
	MainButtonInit(&m_btnManualRotate);			m_btnManualRotate.SetSizeText(14.f);
	MainButtonInit(&m_btnCalcRotate);			m_btnCalcRotate.SetSizeText(14.f);
	MainButtonInit(&m_btnMoveRotate);			m_btnMoveRotate.SetSizeText(14.f);
	MainButtonInit(&m_btnOffsetRotate);			m_btnOffsetRotate.SetSizeText(14.f);
	MainButtonInit(&m_btnPtnDelete);			m_btnPtnDelete.SetSizeText(14.f);

	InitTitle(&m_LbSelectCameraTitle, "Select Camera", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_LbSelectOrgPosTitle, "Orgin Position", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_LbSelectPositionTitle, "Select Position", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_LbCameraResolutionXTitle, "Resolution X(mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_LbCameraResolutionYTitle, "Resolution Y(mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_LbCameraResolutionX, "", 14.f, COLOR_UI_BODY);
	InitTitle(&m_LbCameraResolutionY, "", 14.f, COLOR_UI_BODY);
	InitTitle(&m_LbRotateXTitle, "Rotate X", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_LbRotateX, "", 14.f, COLOR_UI_BODY);
	InitTitle(&m_LbRotateYTitle, "Rotate Y", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_LbRotateY, "", 14.f, COLOR_UI_BODY);

	InitTitle(&m_LbMoveXTitle, "X", 14.f, COLOR_UI_BODY);
	InitTitle(&m_LbMoveYTitle, "Y", 14.f, COLOR_UI_BODY);
	InitTitle(&m_LbMoveTTitle, "T", 14.f, COLOR_UI_BODY);

	InitTitle(&m_lbCalibPixel, "PIXEL", 14.f, COLOR_UI_BODY);
	InitTitle(&m_lbCalibRobt, "ROBOT", 14.f, COLOR_UI_BODY);
	InitTitle(&m_lbCalibSub, "-", 14.f, COLOR_UI_BODY);
	InitTitle(&m_lbCameraResolution, "Camera Resolution", 14.f, COLOR_UI_BODY);

	InitTitle(&m_lbChessBoardTitle, "Chess Board", 14.f, COLOR_UI_BODY);
	InitTitle(&m_lbChessSizeTitle, "Size(mm)", 14.f, COLOR_UI_BODY);
	InitTitle(&m_lbChessXTitle, "X(count)", 14.f, COLOR_UI_BODY);
	InitTitle(&m_lbChessYTitle, "Y(count)", 14.f, COLOR_UI_BODY);

	InitTitle(&m_LbPatternIndex, "Pattern Index", 14.f, COLOR_BTN_BODY);

	EditButtonInit(&m_EditMoveX, 20);
	EditButtonInit(&m_EditMoveY, 20);
	EditButtonInit(&m_EditMoveT, 20);
	EditButtonInit(&m_LbChessBoardSize, 20);
	EditButtonInit(&m_LbChessBoardX, 20);
	EditButtonInit(&m_LbChessBoardY, 20);

	m_btnEnableManualPreAlign.SetAlignTextCM();
	m_btnEnableManualPreAlign.SetColorBkg(255, RGB(64, 64, 64));
	m_btnEnableManualPreAlign.SetSizeText(10);
	m_btnEnableManualPreAlign.SetSizeCheck(2);
	m_btnEnableManualPreAlign.SetSizeCheckBox(3, 3, 20, 20);
	m_btnEnableManualPreAlign.SetOffsetText(-5, 0);
	m_btnEnableManualPreAlign.SetText(_T(""));

	m_btnEnableIgnoreAable.SetAlignTextCM();
	m_btnEnableIgnoreAable.SetColorBkg(255, RGB(64, 64, 64));
	m_btnEnableIgnoreAable.SetSizeText(10);
	m_btnEnableIgnoreAable.SetSizeCheck(2);
	m_btnEnableIgnoreAable.SetSizeCheckBox(3, 3, 20, 20);
	m_btnEnableIgnoreAable.SetOffsetText(-5, 0);
	m_btnEnableIgnoreAable.SetText(_T("Ignore Able"));	

	CString str;
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->ResetContent();

	std::vector<int> cam = m_pMain->vt_job_info[m_nJobID].camera_index;

	for (int i = 0; i < cam.size(); i++)
	{
		int ncam = cam.at(i);
		str.Format("%s", m_pMain->m_stCamInfo[ncam].cName);
		((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->AddString(str);
	}

	for (int i = 0; i < m_pMain->vt_job_info[m_nJobID].num_of_position; i++)
	{
		str.Format("Pos %d", i + 1);
		((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->AddString(str);
	}
/*
	for (int i = 0; i < 4; i++)
	{
		str.Format("Pos %d", i + 1);
		((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->AddString(str);
	}
*/
	CString strFilePath;
	CFileStatus fs;
	
	for (int nIndex = 0; nIndex < MAX_PATTERN_INDEX - 1; nIndex++)
	{		
		strFilePath.Format("%s%s\\CALIB%d%d%d.bmp", m_pMain->m_strCurrModelDir, m_pMain->vt_job_info[m_nJobID].job_name.c_str(), 0, 0, nIndex);

		if (CFile::GetStatus(strFilePath, fs))			str.Format("%d : OK", nIndex + 1);
		else											str.Format("%d : Empty", nIndex + 1);

		((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_PATTERN_INDEX))->AddString(str);
	}


	((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->SetCurSel(0);
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->SetCurSel(0);
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_ORG_POSITION))->SetCurSel(4);
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_PATTERN_INDEX))->SetCurSel(0);

	GetDlgItem(IDC_EDIT_CAMERA_RESOLUTION)->SetWindowTextA("0.0024");

	GetDlgItem(IDC_EDIT_MOVE_X)->SetWindowTextA("0.0");
	GetDlgItem(IDC_EDIT_MOVE_Y)->SetWindowTextA("0.0");
	GetDlgItem(IDC_EDIT_MOVE_T)->SetWindowTextA("0.0");

	GetDlgItem(IDC_LB_SQUARE_SIZE)->SetWindowTextA("3.96");
	GetDlgItem(IDC_LB_CHESS_X2)->SetWindowTextA("9");
	GetDlgItem(IDC_LB_CHESS_Y2)->SetWindowTextA("6");

	updateFrameDialog();

	m_pCramerDlg = new CCramerSRuleDlg;
	m_pCramerDlg->Create(IDD_CRAMERSRULE_DIALOG, this);
	m_pCramerDlg->ShowWindow(SW_HIDE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

HBRUSH TabMachineAlgoPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (pWnd->GetDlgCtrlID() == IDC_CB_PATTERN_INDEX ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_MATCHING_RATE ||
		pWnd->GetDlgCtrlID() == IDC_CB_SELECT_POSITION ||
		pWnd->GetDlgCtrlID() == IDC_CB_SELECT_DRAW_SHAPE ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_LIGHT_SETTING ||
		pWnd->GetDlgCtrlID() == IDC_CB_SELECT_IMAGE_PROC2 ||
		pWnd->GetDlgCtrlID() == IDC_CB_SELECT_LIGHT_CHANNEL ||
		pWnd->GetDlgCtrlID() == IDC_CB_LIGHT_INDEX)
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

void TabMachineAlgoPage::InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}

void TabMachineAlgoPage::EditButtonInit(CEditEx *pbutton, int size, COLORREF color)
{
	pbutton->SetEnable(false);
	pbutton->SetSizeText(size);				// 글자 크기
	pbutton->SetStyleTextBold(true);		// 글자 스타일
	pbutton->SetTextMargins(10, 10);		// 글자 옵셋
	pbutton->SetColorText(color);	// 글자 색상
	pbutton->SetText(_T("0"));				// 글자 설정
}

void TabMachineAlgoPage::MainButtonInit(CButtonEx *pbutton, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}

void TabMachineAlgoPage::updateResultDialog()
{
	dispCameraResolution();
	dispRotateX();
	dispRotateY();
	dispPatternIndex();
	//CalcCalibrationResult();
}

void TabMachineAlgoPage::updateFrameDialog()
{
	CString str;
	int nCam_index = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->GetCurSel();
	int nPos_index = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->ResetContent();

#ifndef JOB_INFO
	std::vector<int> cam = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(m_nTabAlgoIndex);
#else
	std::vector<int> cam = m_pMain->vt_job_info[m_nJobID].camera_index;
#endif

	/*for (int i = 0; i < cam.size(); i++)
	{
		str.Format("Camera %d", cam.at(i) + 1);
		((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->AddString(str);
	}*/
	for (int i = 0; i < cam.size(); i++)
	{
		int ncam = cam.at(i);
		str.Format("%s", m_pMain->m_stCamInfo[ncam].cName);
		//str.Format("Camera %d", cam.at(i) + 1);
		((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->AddString(str);
	}

	// KBJ 2022-02-21 
	//((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->SetCurSel(0);
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->SetCurSel(nCam_index);

	if (((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel() < 0)
		((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->SetCurSel(nPos_index);
	
#ifndef JOB_INFO
	m_nAlgorithmCamCount = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraNum(m_nTabAlgoIndex);
#else
	m_nAlgorithmCamCount = m_pMain->vt_job_info[m_nJobID].num_of_camera;
#endif

	m_nAlgorithmCamBuff.clear();
#ifndef JOB_INFO
	m_nAlgorithmCamBuff = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(m_nTabAlgoIndex);
#else
	m_nAlgorithmCamBuff = m_pMain->vt_job_info[m_nJobID].camera_index;
#endif

	if (m_pMain->m_pForm[FORM_MACHINE] != NULL)
	{
		::SendMessageA(m_pMain->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_SET_ALGORITHM_POS_POSITION, 0);
		::SendMessageA(m_pMain->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_SET_ALGORITHM_POS_CAMERA, 0);
	}

	updateResultDialog();

	m_bMachineControlStart = FALSE;
}

void TabMachineAlgoPage::addProcessHistory(CString str)
{
	EnterCriticalSection(&m_csProcessHistory);
	CString strTime;
	SYSTEMTIME	csTime;
	::GetLocalTime(&csTime);
	//strTime.Format("[%02d:%02d:%02d:%03d] ", csTime.wHour, csTime.wMinute, csTime.wSecond, csTime.wMilliseconds);
	strTime.Format("[%02d:%02d:%02d.%03d] ", csTime.wHour, csTime.wMinute, csTime.wSecond, csTime.wMilliseconds);

	if (m_strProcess.GetLength() > 2048) m_strProcess.Empty();

	m_strProcess = strTime + str + m_strProcess;
	m_EditProcess.SetWindowTextA(m_strProcess);

	LeaveCriticalSection(&m_csProcessHistory);
}

LRESULT TabMachineAlgoPage::OnViewControl(WPARAM wParam, LPARAM lParam)
{

	switch (wParam) {
	case MSG_PMC_ADD_PROCESS_HISTORY:
	{
		CString str;
		str.Format("%s\r\n", (*(CString*)lParam));
		addProcessHistory(str);
		//KJH 2022-04-13 Calibration Log 추가
		theLog.logmsg(LOG_CALIBRATION, (*(CString*)lParam));
	}
	break;
	}
	return 0;
}

void TabMachineAlgoPage::enableMachineButtons(bool bEnable)
{
	m_btnCalibration.SetEnable(bEnable);
	m_btnRotateCenter.SetEnable(bEnable);
	m_btnIndividualCalibration.SetEnable(bEnable);
	m_btnIndividualRotateCenter.SetEnable(bEnable);
	m_btnImageCalibration.SetEnable(bEnable);
}

void TabMachineAlgoPage::cameraChangeTriggerMode(bool bmode)
{
}

void TabMachineAlgoPage::dispCameraResolution()
{
	CString str;
#ifndef JOB_INFO
	int cam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->GetCurSel();
	int pos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();

	int nIndex = m_nAlgorithmCamBuff.at(cam);

	str.Format("%f", m_pMain->GetMachine().getCameraResolutionX(nIndex, pos));
	m_LbCameraResolutionX.SetText(str);

	str.Format("%f", m_pMain->GetMachine().getCameraResolutionY(nIndex, pos));
	m_LbCameraResolutionY.SetText(str);

	//int n = m_pMain->getModel().getMachineInfo().getAlignOrginPos(m_nAlgorithmCamBuff.at(cam));
	int n = m_pMain->vt_job_info[m_nTabAlgoIndex].model_info.getMachineInfo().getAlignOrginPos(m_nAlgorithmCamBuff.at(cam));
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_ORG_POSITION))->SetCurSel(n);
#else

	int cam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->GetCurSel();
	int pos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();
	int nJob = m_nJobID;

	str.Format("%f", m_pMain->GetMachine(nJob).getCameraResolutionX(cam, pos));
	m_LbCameraResolutionX.SetText(str);

	str.Format("%f", m_pMain->GetMachine(nJob).getCameraResolutionY(cam, pos));
	m_LbCameraResolutionY.SetText(str);

	int n = m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getAlignOrginPos(cam);
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_ORG_POSITION))->SetCurSel(n);
#endif
}

void TabMachineAlgoPage::save_resolution_to_ini(int viewer, int job, int cam, int pos)
{
	SYSTEMTIME time;
	::GetLocalTime(&time);
	CString strSection, strKey, strData;
	strSection.Format("VIEWER%d_INFO", viewer + 1);

	strKey.Format("RESOLUTION_X");
	strData.Format("%f", m_pMain->vt_viewer_info[viewer].resolution_x);
	m_pMain->m_iniViewerFile.WriteProfileStringA(strSection, strKey, strData);

	strKey.Format("RESOLUTION_Y");
	strData.Format("%f", m_pMain->vt_viewer_info[viewer].resolution_y);
	m_pMain->m_iniViewerFile.WriteProfileStringA(strSection, strKey, strData);

	//KJH 2022-05-26 Individual Calibration 날짜 저장
	strData.Format("%04d-%02d-%02d_%02d:%02d:%02d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
	m_pMain->m_iniViewerFile.WriteProfileStringA(strSection, "CALIBRATION_DATE", strData);

	strData.Format("%d", job + 1);
	m_pMain->m_iniViewerFile.WriteProfileStringA(strSection, "CALIBRATION_JOB", strData);

	strData.Format("%d", cam + 1);
	m_pMain->m_iniViewerFile.WriteProfileStringA(strSection, "CALIBRATION_CAM", strData);

	strData.Format("%d", pos + 1);
	m_pMain->m_iniViewerFile.WriteProfileStringA(strSection, "CALIBRATION_POS", strData);
}

void TabMachineAlgoPage::dispRotateX()
{
#ifndef JOB_INFO
	CString str;
	int cam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->GetCurSel();
	int pos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();
	int nIndex = m_nAlgorithmCamBuff.at(cam);
	str.Format("%.3f", m_pMain->GetMachine().getRotateX(nIndex));
	m_LbRotateX.SetText(str);
#else
	CString str;
	int nJob = m_nJobID;
	int cam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->GetCurSel();
	int pos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();
	
	str.Format("%.4f", m_pMain->GetMachine(nJob).getRotateX(cam, pos));
	m_LbRotateX.SetText(str);

#endif
}

void TabMachineAlgoPage::dispRotateY()
{
#ifndef JOB_INFO
	CString str;
	int cam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->GetCurSel();
	int pos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();

	int nIndex = m_nAlgorithmCamBuff.at(cam);
	str.Format("%.3f", m_pMain->GetMachine().getRotateY(nIndex));
	m_LbRotateY.SetText(str);
#else
	CString str;
	int nJob = m_nJobID;
	int cam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->GetCurSel();
	int pos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();
	str.Format("%.4f", m_pMain->GetMachine(nJob).getRotateY(cam, pos));
	m_LbRotateY.SetText(str);
#endif
}

void TabMachineAlgoPage::OnBnClickedBtnAutoCalibration()
{
	if (m_pMain->fnSetMessage(2, "Run the Auto Calibration?") != TRUE)	return;

	if (m_bMachineControlStart != TRUE)
	{
		CString str = "Calibration Sequence Entry";
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_nTimeOutCount = 0;
		m_bMachineControlStart = TRUE;
		m_nSeqCalibration = 100;

		cameraChangeTriggerMode();

#ifndef JOB_INFO
		std::vector<int> cam = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(m_nTabAlgoIndex);
#else
		std::vector<int> cam = m_pMain->vt_job_info[m_nJobID].camera_index;		
#endif

		for (int i = 0; i < cam.size(); i++)
		{
			int nCam = cam.at(i);
			((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_ptrCalib[nCam].clear();
		}

		int addr = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;
		g_CommPLC.SetBit(addr + 6, FALSE);
		g_CommPLC.SetBit(addr + 7, FALSE);

		((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_bDrawCalibration = TRUE;

		str = "Camera Trigger Mode";
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		SetTimer(TIMER_CALIBRATION2, 100, NULL);

		str = "Calibration Sequence Start";
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
	else
	{
		m_pMain->fnSetMessage(1, "Already Machine Running..");
	}
}

void TabMachineAlgoPage::OnBnClickedBtnRotateCenter()
{
	if (m_pMain->fnSetMessage(2, "Calculate the Rotate Center?") != TRUE)	return;

	CString str_path;
	str_path.Format("%s%s\\", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[m_nJobID].job_name.c_str());
	m_pMain->GetMachine(m_nJobID).saveRotateCenter(str_path, 0, 0);

	CString str;
	if (m_bMachineControlStart != TRUE)
	{
		cameraChangeTriggerMode();

		((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_bDrawCalibration = TRUE;

		m_nRotateCnt = 0;
		m_bMachineControlStart = TRUE;
		m_nSeqRotateCenter = 100;

		int addr = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;
		g_CommPLC.SetBit(addr + 6, FALSE);
		g_CommPLC.SetBit(addr + 7, FALSE);

		if (m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getRotateCount() > 2)
		{
			for(int i = 0; i < 4; i++)				m_listPtOnCircle[i].clear();
			SetTimer(TIMER_ROTATE_CENTER, 100, NULL);
		}
		else	SetTimer(TIMER_ROTATE_CENTER2, 100, NULL);

		CString str = "Rotate Center Sequence Start";
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
	else
		AfxMessageBox("Already Machine Running..");
}

void TabMachineAlgoPage::OnBnClickedBtnPrealignTest()
{
	CString str;
	if (m_bMachineControlStart != TRUE)
	{
		m_bMachineControlStart = TRUE;
		m_nSeqPrealignTest = 100;

		SetTimer(TIMER_PREALIGN_TEST, 1, NULL);
	}
	else
		AfxMessageBox("Already Machine Running..");
}

void TabMachineAlgoPage::OnBnClickedBtnMove()
{
	CString str;
	GetDlgItem(IDC_EDIT_MOVE_X)->GetWindowTextA(str);
	m_pMain->m_dbRevisionData[m_nJobID][0] = atof(str);
	GetDlgItem(IDC_EDIT_MOVE_Y)->GetWindowTextA(str);
	m_pMain->m_dbRevisionData[m_nJobID][1] = atof(str);
	GetDlgItem(IDC_EDIT_MOVE_T)->GetWindowTextA(str);
	m_pMain->m_dbRevisionData[m_nJobID][2] = atof(str);

	if (0)
	{
		int addr_write_bit = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;
		int addr_read_bit = m_pMain->vt_job_info[m_nJobID].plc_addr_info.read_bit_start;
		int addr_write_word = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_word_start;
		m_pMain->SendServer(TCP_SEND_CSCAM_REVISION_DATA, addr_write_bit, addr_write_word, &addr_read_bit);
	}
	else
	{
		m_nSeqManualMove = 100;
		SetTimer(TIMER_MANUAL_MOVE, 10, NULL);
	}	
}

void TabMachineAlgoPage::OnTimer(UINT_PTR nIDEvent)
{
	CString str;	

	int address=0;

	switch (nIDEvent) {
	case TIMER_MANUAL_MOVE:
	{
		KillTimer(TIMER_MANUAL_MOVE);

		int nRet = seq_manual_move();
		if (nRet == 1)		// OK
		{
			str = "Manual Move Complete";
			SendMessage(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);


			int addr_write_bit = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;
			g_CommPLC.SetBit(addr_write_bit + 6, FALSE);
			g_CommPLC.SetBit(addr_write_bit + 7, FALSE);
		}
		else if (nRet == 2)
		{
			str = "Manual Move NG";
			SendMessage(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_bMachineControlStart = FALSE;
			
			int addr_write_bit = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;
			g_CommPLC.SetBit(addr_write_bit + 6, FALSE);
			g_CommPLC.SetBit(addr_write_bit + 7, FALSE);
		}
		else
			SetTimer(TIMER_MANUAL_MOVE, 100, NULL);
	}
	break;
	case TIMER_CALIBRATION:
	{

	}
	break;
	case TIMER_ROTATE_CENTER:
	{
		KillTimer(TIMER_ROTATE_CENTER);

		int nRet = seqCalculateRotateCenter();
		if (nRet == 1)		// OK
		{
			str = "Rotate Center Complete";
			SendMessage(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_bMachineControlStart = FALSE;
			cameraChangeTriggerMode(false);


#ifndef JOB_INFO
			address = m_pMain->m_pAlgorithmInfo.getAlgorithmWriteBitStart(m_nTabAlgoIndex);
#else
			address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;
#endif
			m_pMain->SendServer(TCP_SEND_CALIB_END, address + 6, FALSE);	// Calibration Mode Off
			updateFrameDialog();
			m_pMain->fnSetMessage(0, "Rotate Center Calculate Complete !");
			// Calibration 유효성 체크 20211015 Tkyuha
			CheckReasonabilityRotate(m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getRotateCount());
		}
		else if (nRet == 2)
		{
			str = "Rotate Center NG";
			SendMessage(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_bMachineControlStart = FALSE;
#ifndef JOB_INFO
			address = m_pMain->m_pAlgorithmInfo.getAlgorithmWriteBitStart(m_nTabAlgoIndex);
#else
			address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;
#endif
			m_pMain->SendServer(TCP_SEND_CALIB_END, address + 6, FALSE);	// Calibration Mode Off
			cameraChangeTriggerMode(false);
			m_pMain->fnSetMessage(0, str);
		}
		else
			SetTimer(TIMER_ROTATE_CENTER, 100, NULL);
	}
	break;
	case TIMER_ROTATE_CENTER2:
	{
		KillTimer(TIMER_ROTATE_CENTER2);

		int nRet = seqCalculateRotateCenter2();
		if (nRet == 1)		// OK
		{
			str = "Rotate Center Complete";
			SendMessage(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_bMachineControlStart = FALSE;
			cameraChangeTriggerMode(false);

			int nMethod = m_pMain->vt_job_info[m_nJobID].algo_method;
			int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();
			// TKyuha 211115 한카메라 에서2개 얼라인 처리
			if (nMethod != CLIENT_TYPE_1CAM_2POS_REFERENCE && nMethod != CLIENT_TYPE_1CAM_4POS_ROBOT)  nPos = 0; // KBJ 2022-08-22 수정
			address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start + (20 * nPos);
			
			g_CommPLC.SetBit(address + 6, FALSE);		// mode on off
			g_CommPLC.SetBit(address + 7, FALSE);		// move request off


			updateFrameDialog();
			m_pMain->fnSetMessage(0, "Rotate Center Calculate Complete !");
			// Calibration 유효성 체크 20211015 Tkyuha
			CheckReasonabilityRotate(2);
		}
		else if (nRet == 2)
		{
			str = "Rotate Center NG";
			SendMessage(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_bMachineControlStart = FALSE;

			address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;
			g_CommPLC.SetBit(address + 6, FALSE);		// mode on off
			g_CommPLC.SetBit(address + 7, FALSE);		// move request off

			cameraChangeTriggerMode(false);
			m_pMain->fnSetMessage(0, str);
		}
		else
			SetTimer(TIMER_ROTATE_CENTER2, 100, NULL);
	}
	break;
	case TIMER_CALIBRATION2:
	{
		KillTimer(TIMER_CALIBRATION2);

		int nRet = seqAutoCalibration2();
		if (nRet == 1)		// OK
		{
			str = "Calibration Complete";
			SendMessage(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_bMachineControlStart = FALSE;

			int nMethod = m_pMain->vt_job_info[m_nJobID].algo_method;
			int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();
			// TKyuha 211115 한카메라 에서2개 얼라인 처리
			if (nMethod != CLIENT_TYPE_1CAM_2POS_REFERENCE && nMethod != CLIENT_TYPE_1CAM_4POS_ROBOT)  nPos = 0; // KBJ 2022-08-22 수정
			address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start + (20 * nPos);

			g_CommPLC.SetBit(address + 6, FALSE);		// mode on off
			g_CommPLC.SetBit(address + 7, FALSE);		// move req off

			cameraChangeTriggerMode(false);
			m_pMain->fnSetMessage(0, "Auto Calibration Complete !");
			// Calibration 유효성 체크 20211015 Tkyuha
			CalcCalibrationResult();
		}
		else if (nRet == 2)
		{
			str = "Calibration NG";
			SendMessage(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_bMachineControlStart = FALSE;

			address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;

			g_CommPLC.SetBit(address + 6, FALSE);		// mode on off
			g_CommPLC.SetBit(address + 7, FALSE);		// move req off

			cameraChangeTriggerMode(false);
			m_pMain->fnSetMessage(0, str);
		}
		else
			SetTimer(TIMER_CALIBRATION2, 100, NULL);
	}
	break;
	
	case TIMER_CALIBRATION3:
	{

	}
	break;
	case TIMER_ROTATE_CENTER3:
	{
		KillTimer(TIMER_ROTATE_CENTER3);

		int nRet = seqCalculateRotateCenter3();
		if (nRet == 1)		// OK
		{
			str = "Rotate Center Complete";
			SendMessage(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_bMachineControlStart = FALSE;
			cameraChangeTriggerMode(false);

#ifndef JOB_INFO
			address = m_pMain->m_pAlgorithmInfo.getAlgorithmWriteBitStart(m_nTabAlgoIndex);
#else
			address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;
#endif
			m_pMain->SendServer(TCP_SEND_CALIB_END, address + 6, FALSE);	// Calibration Mode Off

			updateFrameDialog();
			m_pMain->fnSetMessage(0, "Rotate Center Calculate Complete !");
		}
		else if (nRet == 2)
		{
			str = "Rotate Center NG";
			SendMessage(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_bMachineControlStart = FALSE;
#ifndef JOB_INFO
			address = m_pMain->m_pAlgorithmInfo.getAlgorithmWriteBitStart(m_nTabAlgoIndex);
#else
			address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;
#endif
			m_pMain->SendServer(TCP_SEND_CALIB_END, address + 6, FALSE);	// Calibration Mode Off
			cameraChangeTriggerMode(false);
			m_pMain->fnSetMessage(0, str);
		}
		else
			SetTimer(TIMER_ROTATE_CENTER3, 100, NULL);
	}
	break;
	case TIMER_CALIBRATION4:
	{
		KillTimer(TIMER_CALIBRATION4);

		int nRet = seqAutoCalibration4();
		if (nRet == 1)		// OK
		{
			str = "Calibration Complete";
			SendMessage(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_bMachineControlStart = FALSE;
#ifndef JOB_INFO
			address = m_pMain->m_pAlgorithmInfo.getAlgorithmWriteBitStart(m_nTabAlgoIndex);
#else
			address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;
#endif
			m_pMain->SendServer(TCP_SEND_CALIB_END, address + 6, FALSE);	// Calibration Mode Off
			cameraChangeTriggerMode(false);
			m_pMain->fnSetMessage(0, "Auto Calibration Complete !");
		}
		else if (nRet == 2)
		{
			str = "Calibration NG";
			SendMessage(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_bMachineControlStart = FALSE;
#ifndef JOB_INFO
			address = m_pMain->m_pAlgorithmInfo.getAlgorithmWriteBitStart(m_nTabAlgoIndex);
#else
			address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;
#endif
			m_pMain->SendServer(TCP_SEND_CALIB_END, address + 6, FALSE);	// Calibration Mode Off
			cameraChangeTriggerMode(false);
			m_pMain->fnSetMessage(0, str);
		}
		else
			SetTimer(TIMER_CALIBRATION4, 100, NULL);
	}
	break;	
	case TIMER_PREALIGN_TEST:
	{
		KillTimer(TIMER_PREALIGN_TEST);
		int nRet = seqPrealignTest();
		if (nRet == 1)		// OK
		{
			m_bMachineControlStart = FALSE;
			enableMachineButtons(true);
			cameraChangeTriggerMode(false);
		}
		else if (nRet == 2) // NG
		{
			m_bMachineControlStart = FALSE;
			enableMachineButtons(true);
			cameraChangeTriggerMode(false);
		}
		else
		{
			SetTimer(TIMER_PREALIGN_TEST, 1, NULL);
		}
	}
	break;
	case TIMER_IMAGE_CALIBRATION:
	{
		KillTimer(TIMER_IMAGE_CALIBRATION);
		int nRet = seqImageCalibration();
		if (nRet == 1)		// OK
		{
			m_bMachineControlStart = FALSE;
			enableMachineButtons(true);
			cameraChangeTriggerMode(false);
		}
		else if (nRet == 2) // NG
		{
			m_bMachineControlStart = FALSE;
			enableMachineButtons(true);
			cameraChangeTriggerMode(false);
		}
		else
		{
			SetTimer(TIMER_IMAGE_CALIBRATION, 1, NULL);
		}
	}
	break;
	case TIMER_WAIT_COMPLETE: // 211119 Tkyuha 회전 중심 자동 찾기 적용
	{
		KillTimer(TIMER_WAIT_COMPLETE);

		int addr_read_bit = m_pMain->vt_job_info[m_nJobID].plc_addr_info.read_bit_start;
		int addr_write_bit = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;

		if (g_CommPLC.GetBit(addr_read_bit + 14))
		{
			g_CommPLC.SetBit(addr_write_bit + 14, FALSE);
			str.Format("Clear PC → PLC Mode On(L%d) Signal....", addr_write_bit + 14);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}
		else
		{
			SetTimer(TIMER_WAIT_COMPLETE, 100, NULL);
		}
	}
	break;
	
	}

	CDialogEx::OnTimer(nIDEvent);
}

int TabMachineAlgoPage::seqPrealignTest()
{
	CString str;
	int nRet = 0, real_cam, W, H;

#ifndef JOB_INFO
	int camCount = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraNum(m_nTabAlgoIndex);
	std::vector<int> camBuf = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(m_nTabAlgoIndex);
#else
	int camCount = m_pMain->vt_job_info[m_nJobID].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[m_nJobID].camera_index;
#endif

	switch (m_nSeqPrealignTest) {
	case 0:	break;
	case 100:
	{
		m_nTimeOutCount = 0;

		for (int nCam = 0; nCam < m_nAlgorithmCamCount; nCam++)
		{
			real_cam = m_nAlgorithmCamBuff.at(nCam);
			auto& cam = m_pMain->m_Cameras[real_cam];
			if (cam && cam->IsConnected())
			{
				AcquireSRWLockExclusive(&cam->g_bufsrwlock);
				m_pMain->m_bGrabEnd[real_cam] = FALSE;
				ReleaseSRWLockExclusive(&cam->g_bufsrwlock);
		}
			else m_pMain->m_bGrabEnd[real_cam] = FALSE;
			if (cam && cam->IsConnected())
				m_pMain->SendMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_SINGLE_FRAME, real_cam);
			else
			{
				CFileDialog dlg(TRUE, NULL, _T("Image File"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					_T("Image Files (*.bmp;*.jpg;)|*.bmp;*.jpg|All Files (*.*)|*.*||"), this);

				if (dlg.DoModal() != IDOK) {
					nRet = 2;
					break;
				}

				cv::Mat loadImg = cv::imread((LPCTSTR)dlg.GetPathName(),0);

				W = m_pMain->m_stCamInfo[real_cam].w;
				H = m_pMain->m_stCamInfo[real_cam].h;

				if (loadImg.cols != W || loadImg.rows != H)
				{
					str.Format("Image Size is not Valid.	size %d x %d", W, H);
					AfxMessageBox(str);

					loadImg.release();
					nRet = 2;
					break;
				}
				else
				{
					m_pMain->copyMemory(m_pMain->getSrcBuffer(real_cam), loadImg.data, W * H);
					m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 0), loadImg.data, W * H);
					m_pMain->copyMemory(m_pMain->getCameraViewBuffer(), loadImg.data, W * H);
					loadImg.release();

					//int viewer = m_pMain->vt_job_info[m_nJobID].viewer_index[nCam];
					int viewer = m_pMain->vt_job_info[m_nJobID].machine_viewer_index[nCam];
					((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_pDlgViewerMachine[viewer]->GetViewer().OnLoadImageFromPtr(m_pMain->getCameraViewBuffer());
					m_pMain->m_bGrabEnd[real_cam] = TRUE;
					
				}
			}
		}


		m_nSeqPrealignTest = 200;
	}
	break;
	case 200:
	{
		BOOL bGrabEnd = TRUE;

		for (int nCam = 0; nCam < m_nAlgorithmCamCount; nCam++)
		{
			real_cam = m_nAlgorithmCamBuff.at(nCam);
			if (m_pMain->m_bGrabEnd[real_cam] != TRUE)
				bGrabEnd = FALSE;
		}

		if (++m_nTimeOutCount > 500)
		{
			str = "Grab TimeOut";
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			nRet = 2;
			break;
		}

		if (bGrabEnd != TRUE)	break;

		m_nSeqPrealignTest = 300;
	}
	break;
	case 300:
	{
		BOOL bFind = TRUE;
		int nPos = 0, nPosOffset = 0;

		for (int nCam = 0; nCam < m_nAlgorithmCamCount; nCam++)
		{
			real_cam = m_nAlgorithmCamBuff.at(nCam);
			W = m_pMain->m_stCamInfo[real_cam].w;
			H = m_pMain->m_stCamInfo[real_cam].h;
			m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 0), m_pMain->getSrcBuffer(real_cam), W * H);

			int nJob = m_nJobID;

			// 마크 찾기
loopOneCam:
			{
				int nMethod = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();

				//	if (bMethod == METHOD_MATCHING)
				{

					m_pMain->GetMatching(nJob).findPattern(m_pMain->getProcBuffer(real_cam, 0), nCam, nPos, W, H);
				}

				if (nMethod == METHOD_MATCHING_LINE_THETA)
				{
					double dx = 0.0, dy = 0.0, dt = 0.0;
					BOOL bFixtureUse = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseFixtureMark();
					if (bFixtureUse)
					{
						double fixtureX = m_pMain->GetMatching(nJob).getFixtureX(nCam, nPos);
						double fixtureY = m_pMain->GetMatching(nJob).getFixtureY(nCam, nPos);

						double posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetXPos();
						double posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetYPos();

							dx = posX - fixtureX;
							dy = posY - fixtureY;
							dt = 0.0;
					}

					BYTE* pImage = m_pMain->getProcBuffer(real_cam, 0);
					m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][0].processCaliper(pImage, W, H, dx, dy, dt);
					if (m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][0].getIsMakeLine() != TRUE) m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).SetFound(FIND_ERR);
				}

				if (nMethod == METHOD_CALIPER)
				{
					BOOL bFindLine[2];
					sLine line_info[2];
					double dx = 0.0, dy = 0.0, dt = 0.0, posx, posy;
					BYTE *pImage = m_pMain->getProcBuffer(real_cam, 0);

					BOOL bFixtureUse = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseFixtureMark();
					BOOL result = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetFound();

					if (result == FIND_OK || result == FIND_MATCH)		bFixtureUse = bFixtureUse & result;
					else bFixtureUse = FALSE;

					if (bFixtureUse)
					{
						double fixtureX = m_pMain->GetMatching(nJob).getFixtureX(nCam, nPos);
						double fixtureY = m_pMain->GetMatching(nJob).getFixtureY(nCam, nPos);

						double posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetXPos();
						double posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetYPos();

						if (fixtureX != 0.0 && fixtureY != 0.0)
						{
							dx = posX - fixtureX;
							dy = posY - fixtureY;
							dt = 0.0;
						}
					}
					m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_1].processCaliper(pImage, W, H, dx, dy, dt, TRUE);
					m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_2].processCaliper(pImage, W, H, dx, dy, dt, TRUE);

					line_info[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_1].m_lineInfo;
					line_info[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_2].m_lineInfo;
					bFindLine[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_1].getIsMakeLine();
					bFindLine[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_2].getIsMakeLine();

					if (bFindLine[0] && bFindLine[1])
					{
						sLine lineHori = line_info[0];
						sLine lineVert = line_info[1];

						CCaliper::cramersRules(-lineHori.a, 1, -lineVert.a, 1, lineHori.b, lineVert.b, &posx, &posy);

						CFindInfo pFindInfo = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos);
						pFindInfo.SetXPos(posx);
						pFindInfo.SetYPos(posy);
						pFindInfo.SetFound(FIND_OK);
						pFindInfo.SetScore(100.0);
						m_pMain->GetMatching(nJob).setFindInfo(nCam, nPos, pFindInfo);
					}
					else
					{
						CFindInfo pFindInfo = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos);
						pFindInfo.SetXPos(0.0);
						pFindInfo.SetYPos(0.0);
						pFindInfo.SetFound(FIND_ERR);
						pFindInfo.SetScore(0.0);
						m_pMain->GetMatching(nJob).setFindInfo(nCam, nPos, pFindInfo);
					}
				}
			}
			
			if (m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetFound() != FIND_OK)
				bFind = FALSE;
			
			if (m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_1SHOT_ALIGN || m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_1SHOT_FILM || m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_1SHOT_ALIGN_WITH_SCRATCH_INSP)
			{
				if (m_nAlgorithmCamCount == 1 && nPos == 0)
				{
					nPos = 1;
					goto loopOneCam;
				}
			}
			else if (m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_2POS_REFERENCE ||
					 m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_4POS_ROBOT)
			{
				if (m_nAlgorithmCamCount == 1 && nPos <3)
				{
					nPos++;
					goto loopOneCam;
				}
			}
		}
		
		if (bFind != TRUE)
		{
			str = "Find Pattern Failed";
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			nRet = 2;
			break;
		}

		double posX = 0.0;
		double posY = 0.0;
		double distance = 0;

		int nCamL = 0, nCamR = 0, nCamL2 = 0, nCamR2 = 0;
		nPos = 0;

		switch (m_nAlgorithmCamCount) //카메라 수량에 따른 얼라인
		{
		case 1:
		{
			nCamL = 0;
			nCamL2 = nCamR2 = nCamR = nCamL;
			nPosOffset = 1;
		}
		break;
		case 2:
		{
			nCamL2 = 0;
			nCamR2 = nCamR = 1;
		}
		break;
		case 4:
		{
			nCamL = 0;
			nCamR = 1;
			nCamL2 = 2;
			nCamR2 = 3;
		}
		break;
		}

loopOneCam_Two:
		int nJob = m_nJobID;
		m_pMain->GetPrealign(nJob).setPatternPitchX(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getFiducialMarkPitchX());
		m_pMain->GetPrealign(nJob).setRotateX(nCamL, m_pMain->GetMachine(nJob).getRotateX(nCamL, nPos));
		m_pMain->GetPrealign(nJob).setRotateY(nCamL, m_pMain->GetMachine(nJob).getRotateY(nCamL, nPos));
		m_pMain->GetPrealign(nJob).setRotateX(nCamR, m_pMain->GetMachine(nJob).getRotateX(nCamR, nPos));
		m_pMain->GetPrealign(nJob).setRotateY(nCamR, m_pMain->GetMachine(nJob).getRotateY(nCamR, nPos));
		m_pMain->GetPrealign(nJob).setRotateX(nCamL2, m_pMain->GetMachine(nJob).getRotateX(nCamL2, nPos));
		m_pMain->GetPrealign(nJob).setRotateY(nCamL2, m_pMain->GetMachine(nJob).getRotateY(nCamL2, nPos));
		m_pMain->GetPrealign(nJob).setRotateX(nCamR2, m_pMain->GetMachine(nJob).getRotateX(nCamR2, nPos));
		m_pMain->GetPrealign(nJob).setRotateY(nCamR2, m_pMain->GetMachine(nJob).getRotateY(nCamR2, nPos));

		if (m_nAlgorithmCamCount==1)
		{
			m_pMain->GetPrealign(nJob).setRotateX(nCamR + nPosOffset, m_pMain->GetMachine(nJob).getRotateX(nCamR, nPos));
			m_pMain->GetPrealign(nJob).setRotateY(nCamR + nPosOffset, m_pMain->GetMachine(nJob).getRotateY(nCamR, nPos));
		}

		CString strTemp;
		double angle1 = m_pMain->GetMatching(nJob).getFindInfo(nCamL, nPos).GetAngle();
		double angle2 = m_pMain->GetMatching(nJob).getFindInfo(nCamR, nPos + nPosOffset).GetAngle();
		double worldX, worldY, refX, refY, refworldX, refworldY;

		strTemp.Format("Angle1 = %f", angle1);
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
		strTemp.Format("Angle2 = %f", angle2);
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);

		// 이미지 좌표계 → 실 좌표계
		for (int nCam = 0; nCam < m_nAlgorithmCamCount; nCam++)
		{
			/*refX = m_pMain->GetMatching(nJob).getRefX(nCam, nPos * 2);
			refY = m_pMain->GetMatching(nJob).getRefY(nCam, nPos*2);

			posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos*2).GetXPos();
			posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos*2).GetYPos();
			
			m_pMain->GetMachine(nJob).PixelToWorld(nCam, nPos, posX, posY, &worldX, &worldY);
			m_pMain->GetMachine(nJob).PixelToWorld(nCam, nPos, refX, refY, &refworldX, &refworldY);
			
			if (refX != 0 && refY != 0)
			{
				worldX -= refworldX;
				worldY -= refworldY;
			}
			*/

			// hsj 2022-01-31 reference 사용유무 ----- start
			posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos * 2).GetXPos();
			posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos * 2).GetYPos();

			m_pMain->GetMachine(nJob).PixelToWorld(nCam, nPos, posX, posY, &worldX, &worldY);

			if (m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getUseReferenceMark())
			{
				refX = m_pMain->GetMatching(nJob).getRefX(nCam, nPos * 2);
				refY = m_pMain->GetMatching(nJob).getRefY(nCam, nPos * 2);

				m_pMain->GetMachine(nJob).PixelToWorld(nCam, nPos, refX, refY, &refworldX, &refworldY);

				if (refX != 0 && refY != 0)
				{
					worldX -= refworldX;
					worldY -= refworldY;
				}
			}

			// ----- end

			m_pMain->GetPrealign(m_nJobID).setPosX(nCam, worldX);
			m_pMain->GetPrealign(m_nJobID).setPosY(nCam, worldY);

			if (m_nAlgorithmCamCount == 1)
			{
				/*refX = m_pMain->GetMatching(nJob).getRefX(nCam, nPos * 2 + 1);
				refY = m_pMain->GetMatching(nJob).getRefY(nCam, nPos * 2 + 1);
				posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos * 2 + 1).GetXPos();
				posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos * 2 + 1).GetYPos();

				m_pMain->GetMachine(nJob).PixelToWorld(nCam, nPos, posX, posY, &worldX, &worldY);
				m_pMain->GetMachine(nJob).PixelToWorld(nCam, nPos, refX, refY, &refworldX, &refworldY);
				if (refX != 0 && refY != 0)
				{
					worldX -= refworldX;
					worldY -= refworldY;
				}*/

				// hsj 2022-01-31 reference 사용유무 ----- start
				posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos * 2 + 1).GetXPos();
				posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos * 2 + 1).GetYPos();

				m_pMain->GetMachine(nJob).PixelToWorld(nCam, nPos, posX, posY, &worldX, &worldY);

				if (m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getUseReferenceMark())
				{
					refX = m_pMain->GetMatching(nJob).getRefX(nCam, nPos * 2 + 1);
					refY = m_pMain->GetMatching(nJob).getRefY(nCam, nPos * 2 + 1);

					m_pMain->GetMachine(nJob).PixelToWorld(nCam, nPos, refX, refY, &refworldX, &refworldY);

					worldX -= refworldX;
					worldY -= refworldY;
				}

				// ----- end

				m_pMain->GetPrealign(m_nJobID).setPosX(nCam + nPosOffset, worldX);
				m_pMain->GetPrealign(m_nJobID).setPosY(nCam + nPosOffset, worldY);
			}
		}

		distance = fabs(m_pMain->GetMatching(nJob).getFindInfo(nCamL, nPos).GetXPos() - m_pMain->GetMatching(nJob).getFindInfo(nCamR, nPos + nPosOffset).GetXPos());
		distance *= m_pMain->GetMachine(nJob).getCameraResolutionX(nCamL, nPos);

		// 4점 얼라인 인경우 패널 크기를 pitch 로 사용
		int bMethod = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();

		m_pMain->GetPrealign(m_nJobID).setRevisionOffset(0, m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getRevisionOffset(0));
		m_pMain->GetPrealign(m_nJobID).setRevisionOffset(1, m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getRevisionOffset(1));
		m_pMain->GetPrealign(m_nJobID).setRevisionOffset(2, m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getRevisionOffset(2));

		// 방향 결정
		m_pMain->GetPrealign(m_nJobID).setReverseAxisX(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getRevisionReverseX());
		m_pMain->GetPrealign(m_nJobID).setReverseAxisY(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getRevisionReverseY());
		m_pMain->GetPrealign(m_nJobID).setReverseAxisT(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getRevisionReverseT());
		m_pMain->GetPrealign(m_nJobID).setReverseAxisXY(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getRevisionReverseXY());

		BOOL bTrue = false;
		if (m_nAlgorithmCamCount == 4)
		{
			//	bTrue = m_pMain->GetPrealign(m_nTabAlgoIndex).calcRevision4Cam(nCamL, nCamR, nCamL2, nCamR2, m_pMain->getModel().getAlignInfo().getAlignmentTargetDir(m_nTabAlgoIndex), m_pMain->getModel().getAlignInfo().getRevisionReverse(m_nTabAlgoIndex));

			switch (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getAngleCalcMethod()) {
			case 1:			// Glass Size
			{
				m_pMain->GetPrealign(nJob).setPatternPitchX(m_pMain->vt_job_info[nJob].model_info.getGlassInfo().getGlassWidth());
				m_pMain->GetPrealign(nJob).setPatternPitchY(m_pMain->vt_job_info[nJob].model_info.getGlassInfo().getGlassHeight());
			}
			break;
			case 2:			// Frame Size
			{
				m_pMain->GetPrealign(nJob).setPatternPitchX(m_pMain->vt_job_info[nJob].model_info.getGlassInfo().getFrameWidth());
				m_pMain->GetPrealign(nJob).setPatternPitchY(m_pMain->vt_job_info[nJob].model_info.getGlassInfo().getFrameHeight());
			}
			break;
			case 3:			// Rotate Center
			{
				// LT RT LB RB
				double pitch_x = fabs(m_pMain->GetMachine(nJob).getRotateX(1, 0) - m_pMain->GetMachine(nJob).getRotateX(0, 0));
				double pitch_x2 = fabs(m_pMain->GetMachine(nJob).getRotateX(3, 0) - m_pMain->GetMachine(nJob).getRotateX(2, 0));
				double pitch_y = fabs(m_pMain->GetMachine(nJob).getRotateY(2, 0) - m_pMain->GetMachine(nJob).getRotateY(0, 0));
				double pitch_y2 = fabs(m_pMain->GetMachine(nJob).getRotateY(3, 0) - m_pMain->GetMachine(nJob).getRotateY(1, 0));

				m_pMain->GetPrealign(nJob).setPatternPitchX((pitch_x + pitch_x2) / 2.0);
				m_pMain->GetPrealign(nJob).setPatternPitchY((pitch_y + pitch_y2) / 2.0);
			}
			break;
			default:		//	Align Key
			{	
				m_pMain->GetPrealign(nJob).setPatternPitchX(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getFiducialMarkPitchX());
				m_pMain->GetPrealign(nJob).setPatternPitchY(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getFiducialSecondMarkPitchY());

			}
			break;
			}

			m_pMain->GetPrealign(m_nJobID).setAlignmentType(m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getAlignmentType());
			bTrue = m_pMain->GetPrealign(m_nJobID).calcRevision4Cam_Center(nCamL, nCamR, nCamL2, nCamR2, m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getAlignmentTargetDir(), m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getRevisionReverse());
		}
		else
		{
			switch (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getAngleCalcMethod()) {
			case 1:			// Glass Size
			{
				m_pMain->GetPrealign(nJob).setPatternPitchX(m_pMain->vt_job_info[nJob].model_info.getGlassInfo().getGlassWidth());
				m_pMain->GetPrealign(nJob).setPatternPitchY(m_pMain->vt_job_info[nJob].model_info.getGlassInfo().getGlassHeight());
			}
			break;
			case 2:			// Frame Size
			{
				m_pMain->GetPrealign(nJob).setPatternPitchX(m_pMain->vt_job_info[nJob].model_info.getGlassInfo().getFrameWidth());
				m_pMain->GetPrealign(nJob).setPatternPitchY(m_pMain->vt_job_info[nJob].model_info.getGlassInfo().getFrameHeight());
			}
			break;
			case 3:			// Rotate Center
			{
				// LT RT LB RB
				double pitch_x = fabs(m_pMain->GetMachine(nJob).getRotateX(1, 0) - m_pMain->GetMachine(nJob).getRotateX(0, 0));
				double pitch_y = fabs(m_pMain->GetMachine(nJob).getRotateY(2, 0) - m_pMain->GetMachine(nJob).getRotateY(0, 0));

				if (m_nAlgorithmCamCount == 1)
				{
					pitch_x = fabs(m_pMain->GetMachine(nJob).getRotateX(0, 1) - m_pMain->GetMachine(nJob).getRotateX(0, 0));
					pitch_y = fabs(m_pMain->GetMachine(nJob).getRotateY(0, 1) - m_pMain->GetMachine(nJob).getRotateY(0, 0));
				}

				m_pMain->GetPrealign(nJob).setPatternPitchX(pitch_x);
				m_pMain->GetPrealign(nJob).setPatternPitchY(pitch_y);
			}
			break;
			default:		//	Align Key
			{
				int nSelPos = ((CComboBox *)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();
				if (nSelPos == 1)
				{
					m_pMain->GetPrealign(nJob).setPatternPitchX(m_pMain->vt_job_info[nJob].model_info.getGlassInfo().getFrameWidth());
					m_pMain->GetPrealign(nJob).setPatternPitchY(m_pMain->vt_job_info[nJob].model_info.getGlassInfo().getFrameHeight());

					m_pMain->GetPrealign(nJob).setRotateX(nCamR, m_pMain->GetMachine(nJob).getRotateX(nCamR, 1));
					m_pMain->GetPrealign(nJob).setRotateY(nCamR, m_pMain->GetMachine(nJob).getRotateY(nCamR, 1));
					m_pMain->GetPrealign(nJob).setRotateX(nCamL, m_pMain->GetMachine(nJob).getRotateX(nCamL, 1));
					m_pMain->GetPrealign(nJob).setRotateY(nCamL, m_pMain->GetMachine(nJob).getRotateY(nCamL, 1));
				}
				else
				{
					m_pMain->GetPrealign(nJob).setPatternPitchX(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getFiducialMarkPitchX());
					m_pMain->GetPrealign(nJob).setPatternPitchY(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getFiducialSecondMarkPitchY());
				}

			}
			break;
			}

			m_pMain->GetPrealign(m_nJobID).setAlignmentType(m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getAlignmentType());

			BOOL bUse = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getUseImageTheta();
			m_pMain->GetPrealign(nJob).set_use_image_theta(bUse);

			if (m_nAlgorithmCamCount == 1)
			{
				if (bUse)
				{
					double posX[2], posY[2];
					double theta = 0.0;
					posX[0] = m_pMain->GetMatching(nJob).getFindInfo(0, 0).GetXPos();
					posY[0] = m_pMain->GetMatching(nJob).getFindInfo(0, 0).GetYPos();
					posX[1] = m_pMain->GetMatching(nJob).getFindInfo(0, 1).GetXPos();
					posY[1] = m_pMain->GetMatching(nJob).getFindInfo(0, 1).GetYPos();

					double dx = posX[1] - posX[0];
					double dy = posY[1] - posY[0];

					if(m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getAlignmentTargetDir() == 0 )		theta = atan(dy / dx);
					else				theta = atan(dx / dy);

					m_pMain->GetPrealign(nJob).set_image_theta(theta);
				}

				if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod() == METHOD_MATCHING_LINE_THETA)
				{
					int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[0];

					sLine line_info[2];
					line_info[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].m_lineInfo;
					line_info[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][1][0].m_lineInfo;

					double robot_x[2];
					double robot_y[2];

					double theta = 0.0;
					if (1)
					{
						m_pMain->GetMachine(m_nJobID).PixelToWorld(0, 0, line_info[0].sx, line_info[0].sy, &robot_x[0], &robot_y[0]);
						m_pMain->GetMachine(m_nJobID).PixelToWorld(0, 0, line_info[1].sx, line_info[1].sy, &robot_x[1], &robot_y[1]);

						double dx = robot_x[0] - robot_x[1];
						double dy = robot_y[0] - robot_y[1];

						if( dx != 0.0 )	theta = -atan(dy / dx) / 3.141592 * 180.0;
					}
					else
					{
						double dx = line_info[0].sx - line_info[1].sx;
						double dy = line_info[0].sy - line_info[1].sy;

						if (dx != 0.0)	theta = atan(dy / dx) / 3.141592 * 180.0;
					}
					

					bTrue = m_pMain->GetPrealign(m_nJobID).calcRevision(nCamL, nCamR + nPosOffset, theta);
				}
				else
				{
					bTrue = m_pMain->GetPrealign(m_nJobID).calcRevision(nCamL, nCamR + nPosOffset, m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getAlignmentTargetDir(), m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getRevisionReverse(), FALSE);
				}
			}
			else
			{
				bTrue = m_pMain->GetPrealign(m_nJobID).calcRevision(nCamL, nCamR, m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getAlignmentTargetDir(), m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getRevisionReverse(), FALSE);
			}						
			int view = m_pMain->vt_job_info[m_nJobID].machine_viewer_index[0];
			CViewerEx *Viewer = &((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_pDlgViewerMachine[view]->GetViewer();

			Viewer->ClearOverlayDC();
			Viewer->clearAllFigures();


			if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod() == METHOD_MATCHING_LINE_THETA)
			{
				int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[0];

				sLine line_info[2];
				line_info[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].m_lineInfo;
				line_info[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][1][0].m_lineInfo;

				struct stFigure figure;
				figure.ptBegin.x = line_info[0].sx - 30;
				figure.ptBegin.y = line_info[0].sy;
				figure.ptEnd.x = line_info[0].sx + 30;
				figure.ptEnd.y = line_info[0].sy;
				Viewer->addFigureLine(figure, 3, 3, COLOR_BLUE);
				figure.ptBegin.x = line_info[0].sx;
				figure.ptBegin.y = line_info[0].sy - 30;
				figure.ptEnd.x = line_info[0].sx;
				figure.ptEnd.y = line_info[0].sy + 30;
				Viewer->addFigureLine(figure, 3, 3, COLOR_BLUE);

				figure.ptBegin.x = line_info[1].sx - 30;
				figure.ptBegin.y = line_info[1].sy;
				figure.ptEnd.x   = line_info[1].sx + 30;
				figure.ptEnd.y   = line_info[1].sy;
				Viewer->addFigureLine(figure, 3, 3, COLOR_BLUE);
				figure.ptBegin.x = line_info[1].sx;
				figure.ptBegin.y = line_info[1].sy - 30;
				figure.ptEnd.x   = line_info[1].sx;
				figure.ptEnd.y   = line_info[1].sy + 30;
				Viewer->addFigureLine(figure, 3, 3, COLOR_BLUE);
			}


			struct stFigureText figure;
			figure.nOrgSize = 3;
			figure.nFitSize = 5;			

			str.Format("[Cam %d] x:%.4f y:%.4f", nCamL + 1, m_pMain->GetPrealign(m_nJobID).getPosX(nCamL), m_pMain->GetPrealign(m_nJobID).getPosY(nCamL));
			figure.ptBegin = CPoint(100, nPos*300+100);
			figure.ptBeginFit = figure.ptBegin;
			figure.textString = str;

			Viewer->addFigureText(figure, 10, 10, COLOR_BLUE);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

			str.Format("[Cam %d] rot_x:%.4f rot_y:%.4f", nCamL + 1, m_pMain->GetPrealign(m_nJobID).getRotateX(nCamL), m_pMain->GetPrealign(m_nJobID).getRotateY(nCamL));
			figure.ptBegin = CPoint(100, nPos * 300 + 150);
			figure.ptBeginFit = figure.ptBegin;
			figure.textString = str;
			Viewer->addFigureText(figure, 10, 10, COLOR_BLUE);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

			str.Format("[Cam %d] x:%.4f y:%.4f", nCamR + 1, m_pMain->GetPrealign(m_nJobID).getPosX(nCamR + nPosOffset), m_pMain->GetPrealign(m_nJobID).getPosY(nCamR + nPosOffset));
			figure.ptBegin = CPoint(100, nPos * 300 + 200);
			figure.ptBeginFit = figure.ptBegin;
			figure.textString = str;
			Viewer->addFigureText(figure, 10, 10, COLOR_BLUE);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

			str.Format("[Cam %d] rot_x:%.4f rot_y:%.4f", nCamR + 1, m_pMain->GetPrealign(m_nJobID).getRotateX(nCamR + nPosOffset), m_pMain->GetPrealign(m_nJobID).getRotateY(nCamR + nPosOffset));
			figure.ptBegin = CPoint(100, nPos * 300 + 250);
			figure.ptBeginFit = figure.ptBegin;
			figure.textString = str;
			Viewer->addFigureText(figure, 10, 10, COLOR_BLUE);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

			str.Format("rev_x: %.4f, rev_y:%.4f, rev_t:%.4f", m_pMain->GetPrealign(m_nJobID).getRevisionX(), m_pMain->GetPrealign(m_nJobID).getRevisionY(), m_pMain->GetPrealign(m_nJobID).getRevisionT());
			figure.ptBegin = CPoint(100, nPos * 300 + 300);
			figure.ptBeginFit = figure.ptBegin;
			figure.textString = str;
			Viewer->addFigureText(figure, 10, 10, COLOR_BLUE);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}
		//else bTrue = m_pMain->GetPrealign(m_nJobID).calcRevision(nCamL, nCamR, m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getAlignmentTargetDir(), m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getRevisionReverse());
		if (bTrue != 0)
		{
			str = "calculate Revision Data Failed";
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			nRet = 2;
			break;
		}
		str.Format("X=%f,Y=%f,T=%f, D=%f", m_pMain->GetPrealign(m_nJobID).getRevisionX(), m_pMain->GetPrealign(m_nJobID).getRevisionY(),m_pMain->GetPrealign(m_nJobID).getRevisionT(), distance);
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		
		// Tkyuha 20211021 두개 얼라인 할 경우 사용
		if ((m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_2POS_REFERENCE ||
			m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_4POS_ROBOT) && nPos == 0)
		{
			nPos = 1;
			goto loopOneCam_Two;
		}
			
		///////// L Check 검사
		if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getEnableLCheck())
		{
			BOOL bError = FALSE;
			CString str_path;
			str_path.Format("%s%s\\%s", m_pMain->m_strModelDir, m_pMain->m_strCurrentModelName, m_pMain->vt_job_info[m_nJobID].get_job_name());

			m_pMain->readMachineInfoData(m_nJobID, &m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo(), str_path);

			if (camCount == 2)			bError = m_pMain->calc_lcheck_2cam_1pos(m_nJobID, TRUE);
			else if (camCount == 4)		bError = m_pMain->calc_lcheck_4cam_1pos(m_nJobID, TRUE);
			else if (camCount == 1 && m_pMain->vt_job_info[nJob].num_of_position > 1)
			{
				if (m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_2SHOT_ALIGN) 
				{
					bError = m_pMain->calc_lcheck_1cam_2pos(m_nJobID, TRUE);
				}
				else if (m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_2POS_REFERENCE ||
						 m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_4POS_ROBOT)
				{
					bError = m_pMain->calc_lcheck_1cam_1shot_2object(m_nJobID, TRUE);
				}
				else 
				{
					bError = m_pMain->calc_lcheck_1cam_1shot(m_nJobID, TRUE);
				}
			}

		}

		::SendMessageA(m_pMain->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_DISPLAY_MACHINE_VIEW, 1);		

		str.Format("%f", m_pMain->GetPrealign(m_nJobID).getRevisionX());
		GetDlgItem(IDC_EDIT_MOVE_X)->SetWindowTextA(str);
		
		str.Format("%f", m_pMain->GetPrealign(m_nJobID).getRevisionY());
		GetDlgItem(IDC_EDIT_MOVE_Y)->SetWindowTextA(str);

		str.Format("%f", m_pMain->GetPrealign(m_nJobID).getRevisionT());
		GetDlgItem(IDC_EDIT_MOVE_T)->SetWindowTextA(str);

		nRet = 1;
	}
	break;
	};

	return nRet;
}

int TabMachineAlgoPage::seqImageCalibration()
{
	CString str;
	int nRet = 0, real_cam, W, H;
	int nPos = 0;

#ifndef JOB_INFO
	int camCount = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraNum(m_nTabAlgoIndex);
	std::vector<int> camBuf = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(m_nTabAlgoIndex);
#else
	int camCount = m_pMain->vt_job_info[m_nJobID].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[m_nJobID].camera_index;
#endif

	switch (m_nSeqImageCalibration) {
	case 0:	break;
	case 100:  // 이미지 Grab
	{
		m_nTimeOutCount = 0;

		for (int nCam = 0; nCam < m_nAlgorithmCamCount; nCam++)
		{
			real_cam = m_nAlgorithmCamBuff.at(nCam);
			auto& cam = m_pMain->m_Cameras[real_cam];
			if (cam && cam->IsConnected())
			{
				AcquireSRWLockExclusive(&cam->g_bufsrwlock);
				m_pMain->m_bGrabEnd[real_cam] = FALSE;
				ReleaseSRWLockExclusive(&cam->g_bufsrwlock);
			}
			else m_pMain->m_bGrabEnd[real_cam] = FALSE;
			if (cam && cam->IsConnected())
				m_pMain->SendMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_SINGLE_FRAME, real_cam);
			else
			{
				CFileDialog dlg(TRUE, NULL, _T("Image File"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					_T("Image Files (*.bmp;*.jpg;)|*.bmp;*.jpg|All Files (*.*)|*.*||"), this);

				if (dlg.DoModal() != IDOK) {
					nRet = 2;
					break;
				}

				cv::Mat loadImg = cv::imread((LPCTSTR)dlg.GetPathName(), 0);
/*
				W = m_pMain->m_pViewDisplayInfo[cam].getViewImageWidth();
				H = m_pMain->m_pViewDisplayInfo[cam].getViewImageHeight();
*/
				W = m_pMain->m_stCamInfo[real_cam].w;
				H = m_pMain->m_stCamInfo[real_cam].h;

				if (loadImg.cols != W || loadImg.rows != H)
				{
					str.Format("Image Size is not Valid.	size %d x %d", W, H);
					AfxMessageBox(str);

					loadImg.release();
					nRet = 2;
					break;
				}
				else
				{
					m_pMain->copyMemory(m_pMain->getSrcBuffer(real_cam), loadImg.data, W * H);
					m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 0), loadImg.data, W * H);
					m_pMain->copyMemory(m_pMain->getCameraViewBuffer(), loadImg.data, W * H);
					loadImg.release();

					int viewer = m_pMain->vt_job_info[m_nJobID].machine_viewer_index[nCam];
					((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_pDlgViewerMachine[viewer]->GetViewer().OnLoadImageFromPtr(m_pMain->getCameraViewBuffer());
					m_pMain->m_bGrabEnd[real_cam] = TRUE;

				}
			}
		}


		m_nSeqImageCalibration = 200;
	}
	break;
	case 200:
	{
		BOOL bGrabEnd = TRUE;

		for (int nCam = 0; nCam < m_nAlgorithmCamCount; nCam++)
		{
			real_cam = m_nAlgorithmCamBuff.at(nCam);
			if (m_pMain->m_bGrabEnd[real_cam] != TRUE)
				bGrabEnd = FALSE;
		}

		if (++m_nTimeOutCount > 500)
		{
			str = "Grab TimeOut";
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			nRet = 2;
			break;
		}

		if (bGrabEnd != TRUE)	break;

		m_nSeqImageCalibration = 300;
	}
	break;
	case 300:
	{
		CString str;
		bool found = false;

		int CHESSBOARD_WIDTH = 9;
		int CHESSBOARD_HEIGHT = 6;
		float squareSize = 3.96f;
		
		m_LbChessBoardX.GetWindowTextA(str); CHESSBOARD_WIDTH = atoi(str);
		m_LbChessBoardY.GetWindowTextA(str);  CHESSBOARD_HEIGHT = atoi(str);
		m_LbChessBoardSize.GetWindowTextA(str);  squareSize = atof(str);

		cv::Size cornerSize = cv::Size(CHESSBOARD_WIDTH, CHESSBOARD_HEIGHT);
		
		vector<cv::Point2f> pointBuf;
		cv::Mat img_frame;

		int nSelCam = ((CComboBox *)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->GetCurSel();
		int nCalibCam = camBuf.at(nSelCam);

		W = m_pMain->m_stCamInfo[nCalibCam].w;
		H = m_pMain->m_stCamInfo[nCalibCam].h;

		cv::Mat gray_image1;
		cv::Mat img_frameOrg(H, W, CV_8U, m_pMain->getSrcBuffer(nCalibCam));
		/*cv::threshold(img_frameOrg, img_frame, 128, 255, CV_THRESH_BINARY);
		cv::Mat mask = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3), cv::Point(1, 1));
		dilate(img_frame, img_frame, mask, cv::Point(-1, -1), 3);	*/	
		cv::resize(img_frameOrg, gray_image1, cv::Size(img_frameOrg.cols / 2, img_frameOrg.rows / 2));
		cv::bilateralFilter(gray_image1, img_frame, 5, 75, 75);
		
		m_pMain->GetMachine(m_nJobID).getPtImage(nSelCam, nPos).clear();
		m_pMain->GetMachine(m_nJobID).getPtMotor(nSelCam, nPos).clear();

		found = findChessboardCorners(img_frame, cornerSize, pointBuf, cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_NORMALIZE_IMAGE);
		if (!found)
		{
			cv::goodFeaturesToTrack(img_frame, pointBuf, 200, 0.01, 120);
			if (pointBuf.size() > 0) found = true;
			else found = false;
		}
		if (found && CHESSBOARD_HEIGHT* CHESSBOARD_WIDTH== pointBuf.size())
		{
			vector<vector<cv::Point2f>> image_points;
			vector<vector<cv::Point3f>> object_points;			

			cornerSubPix(img_frame, pointBuf, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.001));
			drawChessboardCorners(img_frame, cv::Size(CHESSBOARD_WIDTH, CHESSBOARD_HEIGHT), cv::Mat(pointBuf), found);

			sort(pointBuf.begin(), pointBuf.end(), compareFloatY);
			for (int i = 0; i < CHESSBOARD_HEIGHT; ++i)
			{
				vector<cv::Point2f>::iterator itr = pointBuf.begin() + CHESSBOARD_WIDTH*i;
				vector<cv::Point2f>::iterator itrend = pointBuf.begin() + CHESSBOARD_WIDTH * (i+1);
				sort(itr, itrend, compareFloatX);
			}
			/*std::vector<cv::Point3d> obj;
			for (int j = 0; j < CHESSBOARD_WIDTH*CHESSBOARD_HEIGHT; j++)
				obj.push_back(cv::Point3d((j / CHESSBOARD_WIDTH)* squareSize, (j%CHESSBOARD_WIDTH)*squareSize, 0.0f));
            */
			std::vector<cv::Point3f> obj;
			for (int i = 0; i < CHESSBOARD_HEIGHT; ++i)
				for (int j = 0; j < CHESSBOARD_WIDTH; ++j)
					obj.push_back(cv::Point3f(j*squareSize, i*squareSize, 0));			

			image_points.push_back(pointBuf);
			object_points.push_back(obj);

			cv::Mat intrinsic = cv::Mat(3, 3, CV_32FC1); // Mat::eye(3, 3, CV_64F);
			cv::Mat distCoeffs; //Mat::zeros(8, 1, CV_64F);
			vector<cv::Mat> rvecs;
			vector<cv::Mat> tvecs;

			intrinsic.ptr<float>(0)[0] = 1;
			intrinsic.ptr<float>(1)[1] = 1;

			cv::calibrateCamera(object_points, image_points, img_frame.size(), intrinsic, distCoeffs, rvecs, tvecs);

			cv::Mat imageUndistorted;
			cv::undistort(img_frame, imageUndistorted, intrinsic, distCoeffs);

			cv::Point2f pt;
			for (int j = 0; j < pointBuf.size(); j++)
			{
				pt.x = float(pointBuf.at(j).x) * 2;
				pt.y = float(pointBuf.at(j).y) * 2;
				m_pMain->GetMachine(m_nJobID).addImagePoint(nSelCam, nPos, pt);

				pt.x = float((j%CHESSBOARD_WIDTH)*squareSize);
				pt.y = float((j / CHESSBOARD_WIDTH)* squareSize);				
				m_pMain->GetMachine(m_nJobID).addMotorPoint(nSelCam, nPos, pt);
			}			

			m_pMain->GetMachine(m_nJobID).setNumCalPointsX(CHESSBOARD_WIDTH);
			m_pMain->GetMachine(m_nJobID).setNumCalPointsY(CHESSBOARD_HEIGHT);

			for (int nCam = 0; nCam < m_nAlgorithmCamCount; nCam++)
			{
				real_cam = m_nAlgorithmCamBuff.at(nCam);
				if (m_pMain->GetMachine(m_nJobID).calculateCalibration(nCam, nPos) == 0)
				{					
					m_pMain->GetMachine(m_nJobID).calculateResolution(nCam, nPos, CHESSBOARD_WIDTH, CHESSBOARD_HEIGHT);

					CString str_path;
					str_path.Format("%s%s\\", m_pMain->m_strMachineDir, m_pMain->vt_job_info[m_nJobID].job_name.c_str());
					m_pMain->GetMachine(m_nJobID).saveCalibrationData(nCam, nPos, str_path);

					int viewer = m_pMain->vt_job_info[m_nJobID].viewer_index[nCam];
					m_pMain->vt_viewer_info[viewer].resolution_x = m_pMain->GetMachine(m_nJobID).getCameraResolutionX(nCam, nPos);
					m_pMain->vt_viewer_info[viewer].resolution_y = m_pMain->GetMachine(m_nJobID).getCameraResolutionY(nCam, nPos);

					dispCameraResolution();
				}
			}

			::SendMessageA(m_pMain->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_DISPLAY_CALIBRATION_AREA_VIEW, 0);
			m_pMain->draw_calib_direction(m_nJobID, FORM_MACHINE);

			nRet = 1;
		}
		else
		{
			AfxMessageBox("Find NG!!");
			nRet = 2;
		}
	}
	break;
	};

	return nRet;
}

int TabMachineAlgoPage::seq_manual_move()
{
	int nRet = 0;
	CString str;

	switch (m_nSeqManualMove) {
	case 0: KillTimer(TIMER_MANUAL_MOVE); break;
	case 100:		// Calibration 가능 상태 확인
	{
		BOOL is_able_ignore = ((CButton*)GetDlgItem(IDC_CHECK_ABLE_IGNORE))->GetCheck();


		// ignrore 체크박스가 체크 되어 있을 경우 calibration able 신호를 무시한다.
		// 시퀀스가 지나고 나면 무조건 check box는 해제하도록.
		// 안전상의 문제로 체크 했을 경우에만 한번 무시하도록.
		if (is_able_ignore)
		{
			m_nSeqManualMove = 200;

			str.Format("PLC Calib Able Signal Ignore!!");
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			((CButton*)GetDlgItem(IDC_CHECK_ABLE_IGNORE))->SetCheck(FALSE);
			break;
		}


		int address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.read_bit_start + 5;
		BOOL enable = g_CommPLC.GetBit(address);

		if (enable == FALSE)
		{
			str.Format("PLC Calib Enable Signal (L%d) Off!!", address);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			nRet = 2;
			break;
		}
		else
		{
			str.Format("PLC Calib Enable Signal (L%d) On!!", address);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}

		m_nSeqManualMove = 200;
	}
	break;
	case 200:		// Calibration Mode ON
	{
		int address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start + 6;

		g_CommPLC.SetBit(address, TRUE);

		str.Format("PC → PLC Calib Mode On (M%d)!!", address);
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_nTimeOutCount = 0;
		m_nSeqManualMove = 300;
	}
	break;
	case 300:
	{
		int address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.read_bit_start + 6;

		BOOL mode_on_ack = g_CommPLC.GetBit(address);

		if (m_nTimeOutCount % 100 == 0)
		{
			str.Format("Wait PLC → PC Calib Mode On Ack (M%d)!!", address);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}

		if (m_nTimeOutCount++ > 1000)
		{
			str.Format("Timeout PLC → PC Calib Mode On Ack (M%d)!!", address);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

			nRet = 2;
			m_nSeqManualMove = 0;
			break;
		}

		if (mode_on_ack == TRUE)
		{
			m_nTimeOutCount = 0;
			m_nSeqManualMove = 1000;
		}
	}
	break;
	case 1000:		// 이동값 전송
	{
		m_pMain->m_nCalibRevisionAck = 0;
		int address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start + 7;
		int address3 = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_word_start;

		long lRevisionData[6] = { 0, };
		lRevisionData[0] = LOWORD(m_pMain->m_dbRevisionData[m_nJobID][AXIS_X] * MOTOR_SCALE);
		lRevisionData[1] = HIWORD(m_pMain->m_dbRevisionData[m_nJobID][AXIS_X] * MOTOR_SCALE);
		lRevisionData[2] = LOWORD(m_pMain->m_dbRevisionData[m_nJobID][AXIS_Y] * MOTOR_SCALE);
		lRevisionData[3] = HIWORD(m_pMain->m_dbRevisionData[m_nJobID][AXIS_Y] * MOTOR_SCALE);
		lRevisionData[4] = LOWORD(m_pMain->m_dbRevisionData[m_nJobID][AXIS_T] * MOTOR_SCALE);
		lRevisionData[5] = HIWORD(m_pMain->m_dbRevisionData[m_nJobID][AXIS_T] * MOTOR_SCALE);

		g_CommPLC.SetWord(address3, 6, lRevisionData);
		g_CommPLC.SetBit(address, TRUE);

		str.Format("PC → PLC Send Calib Data(D%d)", address3);
		::SendMessageA(this->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		str.Format("PC → PLC Revision Pos Move Signal(M%d) Req", address);
		::SendMessageA(this->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_nTimeOutCount = 0;
		m_nSeqManualMove = 1100;
	}
	break;
	case 1100:		// 보정 위치 이동 완료 확인
	{
		int address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start + 7;
		int address2 = m_pMain->vt_job_info[m_nJobID].plc_addr_info.read_bit_start + 7;

		BOOL revision_ack = g_CommPLC.GetBit(address2);

		if (m_nTimeOutCount % 100 == 0)
		{
			str.Format("Wait PLC → PC Manual Move(L%d) Complete", address2);
			SendMessageA(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}

		if (m_nTimeOutCount++ > 1000)
		{
			str.Format("Timeout PLC → PC Manual Move(L%d) Complete", address2);
			SendMessageA(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

			nRet = 2;
			m_nTimeOutCount = 0;
			m_nSeqManualMove = 0;
			g_CommPLC.SetBit(address, FALSE);
			break;
		}

		if (revision_ack == TRUE)
		{
			str.Format("PLC → PC Revision Pos Move(L%d) Complete", address2);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_nSeqManualMove = 0;
			g_CommPLC.SetBit(address, FALSE);
			nRet = 1;
		}
	}
	break;
	}

	return nRet;
}

int TabMachineAlgoPage::seqAutoCalibration2()
{
	CString str;

	double dbData[3] = { 0, };
	double dbStepX, dbStepY;	// 1.2mm,	0.9mm
	double dbOffsetX = 0.0, dbOffsetY = 0.0;
	int dbMultiOffset = 0;
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();
	int nMethod = m_pMain->vt_job_info[m_nJobID].algo_method;
	int nRet = 0,address=0,  address2 = 0, address3 = 0,real_cam = 0, W, H;
	int nLimitX = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getCalibrationCountX();
	int nLimitY = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getCalibrationCountY();
	int nOriLimitX = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getCalibrationCountX();
	int nOriLimitY = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getCalibrationCountY();

	// TKyuha 211115 한카메라 에서2개 얼라인 처리
	dbMultiOffset = nPos * 20;
	if (nMethod != CLIENT_TYPE_1CAM_2POS_REFERENCE && nMethod != CLIENT_TYPE_1CAM_4POS_ROBOT)  dbMultiOffset = 0; // KBJ 2022-08-22 수정

	int camCount = m_pMain->vt_job_info[m_nJobID].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[m_nJobID].camera_index;

	if (nLimitX <= 1)	dbStepX = 0.0;
	else				dbStepX = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getCalibrationRangeX() / (nLimitX - 1);

	if (nLimitY <= 1)	dbStepY = 0.0;
	else				dbStepY = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getCalibrationRangeY() / (nLimitY - 1);

	if (nLimitX == 0)
	{
		dbStepX = 0;
		nLimitX = 3;
	}
	if (nLimitY == 0)
	{
		dbStepY = 0;
		nLimitY = 3;
	}

	switch (m_nSeqCalibration) {
	case 0:
		KillTimer(TIMER_CALIBRATION2);
		nRet = 2;
		break;
	case 100:	// Calibration 가능 상태 확인
	{
		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.read_bit_start + dbMultiOffset + 5;
		BOOL bEnable = g_CommPLC.GetBit(address);

		if (bEnable != TRUE)
		{
			str.Format("PLC Calib Enable(L%d) Signal Off!!", address);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			nRet = 2;
		}
		else
		{
			str.Format("PLC Calib Enable(L%d) Signal On", address);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}

		m_nSeqCalibration = 200;
	}
	break;
	case 200:	// Calibration Mode ON
	{	
		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start + dbMultiOffset + 6;
		g_CommPLC.SetBit(address, TRUE);

		m_nSeqCalibration = 250;

		m_nTimeOutCount = 0;

		str.Format("Calib Mode On(M%d)", address);
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
	break;
	case 250:	// Wait Calib Mode On Ack Signal
	{
		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.read_bit_start + dbMultiOffset + 6;
		BOOL bAck = g_CommPLC.GetBit(address);

		if (m_nTimeOutCount % 100 == 0)
		{
			str.Format("Wait PLC → PC Mode On(L%d) Ack Signal....", address);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}

		if (m_nTimeOutCount++ > 1000)
		{
			m_nTimeOutCount = 0;
			str.Format("Timeout - Wait PLC → PC Mode On Ack Signal");
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

			nRet = 2;
			break;
		}

		if (bAck)
		{
			m_nTimeOutCount = 0;
			m_nSeqCalibration = 300;
			break;
		}
	}
	break;
	case 300:	// 원점에서 Grab
	{
		str = "Grab Start";
		::SendMessageA(this->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		for (int nCam = 0; nCam < m_nAlgorithmCamCount; nCam++)
		{
			real_cam = m_nAlgorithmCamBuff.at(nCam);
			auto& cam = m_pMain->m_Cameras[real_cam];
			if (cam && cam->IsConnected())
			{
				AcquireSRWLockExclusive(&cam->g_bufsrwlock);
				m_pMain->m_bGrabEnd[real_cam] = FALSE;
				ReleaseSRWLockExclusive(&cam->g_bufsrwlock);
			}
			else m_pMain->m_bGrabEnd[real_cam] = FALSE;
			if (cam && cam->IsConnected())
				m_pMain->SendMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_SINGLE_FRAME, real_cam);
		}

		Sleep(300);

		m_nTimeOutCount = 0;
		m_nSeqCalibration = 400;
	}
	break;
	case 400:	// Grab 완료 대기
	{
		BOOL bGrabEnd = TRUE;

		for (int nCam = 0; nCam < m_nAlgorithmCamCount; nCam++)
		{
			real_cam = m_nAlgorithmCamBuff.at(nCam);
			if (m_pMain->m_bGrabEnd[real_cam] != TRUE)
				bGrabEnd = FALSE;
		}
		
		if (++m_nTimeOutCount > 1000)
		{
			m_nSeqCalibration = 0;
			nRet = 2;
			str = "Grab Timeout";
			::SendMessageA(this->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			break;
		}

		if (bGrabEnd != TRUE) break;

		str = "Grab Complete";
		::SendMessageA(this->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_nSeqCalibration = 500;
	}
	break;
	case 500:	// Find Pattern
	{
		Sleep(1000);
		BOOL bFind = TRUE;

		//int nPos = 0;  // 20211020 Tkyuha 개별 켈리브레이션을 수행하기 위해 삭제
		CFindInfo *pInfo;

		for (int nCam = 0; nCam < m_nAlgorithmCamCount; nCam++)
		{
			real_cam = m_nAlgorithmCamBuff.at(nCam);
			pInfo = &m_pMain->GetMatching(m_nJobID).getFindInfo(nCam, nPos);
			
			W = m_pMain->m_stCamInfo[real_cam].w;
			H = m_pMain->m_stCamInfo[real_cam].h;

			m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 0), m_pMain->getSrcBuffer(real_cam), W * H);
#ifdef _USE_CALIB_MARK
		   UseClibrationMarkMatching((BYTE*)m_pMain->getProcBuffer(real_cam, 0), m_nJobID, nCam, nPos, W, H);
#else
			m_pMain->GetMatching(m_nJobID).findPattern((BYTE *)m_pMain->getProcBuffer(real_cam, 0), nCam, nPos, W, H);
#endif

			if (pInfo->GetFound() == FIND_ERR)				bFind = FALSE;

			m_pMain->GetMachine(m_nJobID).setCalibStartX(nCam, nPos, pInfo->GetXPos());
			m_pMain->GetMachine(m_nJobID).setCalibStartY(nCam, nPos, pInfo->GetYPos());

			((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_dbCalibTargetX[real_cam] = pInfo->GetXPos();
			((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_dbCalibTargetY[real_cam] = pInfo->GetYPos();

			str.Format("Cam %d) %.1f%% %s", real_cam + 1, pInfo->getScore(), pInfo->GetFound() == FIND_OK ? "OK" : "NG");
			::SendMessageA(this->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

			// KBJ 2022-02-22 Machine draw info
			stDARW_MACHINE_INFO draw_info;
			draw_info.nCount = 0;
			draw_info.nCam = nCam;
			draw_info.nPos = nPos;
			draw_info.bClear = true;
			::SendMessageA(m_pMain->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_DISPLAY_MACHINE_VIEW_POS, (LPARAM)&draw_info);
		}		

		// KBJ 2022-02-22 Machine draw info
		//::SendMessageA(m_pMain->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_DISPLAY_MACHINE_VIEW, 0);
		


		if (bFind != TRUE)
		{
			nRet = 2;	// Calibration NG
			break;
		}

		m_nSeqCalibration = 1000;
	}
	break;
	case 1000:	// Calibration 관련 데이터 초기화
	{
		// Reset Calib Data
		for (int nCam = 0; nCam < m_nAlgorithmCamCount; nCam++)
		{
			m_pMain->GetMachine(m_nJobID).getPtImage(nCam, nPos).clear();
			m_pMain->GetMachine(m_nJobID).getPtMotor(nCam, nPos).clear();
		}		

		m_nCntX = -1;
		m_nCntY = -1;

		m_nSeqCalibration = 1100;
	}
	break;
	case 1100:
	{
		if (++m_nCntY == nLimitY)	// Y 초기 위치
		{
			m_nSeqCalibration = 2000;
			break;
		}

		m_nSeqCalibration = 1200;
	}
	break;
	case 1200:	// 보정 위치 이동 요청
	{
		if (++m_nCntX == nOriLimitX)
		{
			m_nCntX = -1;
			m_nSeqCalibration = 1100;
			break;
		}

		// 상대 좌표계 이동 
		if (m_nCntX == 0)
		{
			if (m_nCntY == 0)
			{
				dbData[AXIS_X] = -dbStepX * ((nLimitX - 1) / 2.0);
				dbData[AXIS_Y] = -dbStepY * ((nLimitY - 1) / 2.0);
				dbData[AXIS_T] = 0;
			}
			else
			{
				dbData[AXIS_X] = (-dbStepX) * (nLimitX - 1);
				dbData[AXIS_Y] = dbStepY;
				dbData[AXIS_T] = 0;
			}
		}
		else
		{
			dbData[AXIS_X] = dbStepX;
			dbData[AXIS_Y] = 0;
			dbData[AXIS_T] = 0;
		}	

		for (int nCam = 0; nCam < m_nAlgorithmCamCount; nCam++)
		{
			cv::Point2f pt;
			pt.x = float((m_nCntX - (nLimitX - 1) / 2.0) * dbStepX + dbOffsetX);
			pt.y = float((m_nCntY - (nLimitY - 1) / 2.0) * dbStepY + dbOffsetY);
			m_pMain->GetMachine(m_nJobID).addMotorPoint(nCam, nPos, pt);

			// hsj 2023-01-02 calibration 값 나타내기 , 검증해야함..
			CString str;
			str.Format("%f, %f", pt.x, pt.y);
			m_EditCalibRobot.SetWindowTextA(str);
		}

		// hsj 2023-01-02 calibration 값 나타내기 , 검증해야함..
		if (m_nCntX == 0 && m_nCntY == 0)
		{
			CString str;
			str.Format("-");
			m_EditCalibSub.SetWindowTextA(str);
		}
		else
		{
			int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->GetCurSel();
			int size = int(m_pMain->GetMachine(m_nJobID).getPtImage(nCam, nPos).size());
			Point2f first, second,sub;

			// KBJ 2023-01-14 확인 필요
			if (size > 1)
			{
				first = m_pMain->GetMachine(m_nJobID).getPtImage(nCam, nPos).at(size - 1);
				second = m_pMain->GetMachine(m_nJobID).getPtImage(nCam, nPos).at(size - 2);

				sub.x = fabs(first.x - second.x);
				sub.y = fabs(first.y - second.y);

				CString str;
				m_EditCameraResolution.GetWindowTextA(str);

				double dCamResolution = _tstof(str);
				sub.x = sub.x * dCamResolution;
				sub.y = sub.y * dCamResolution;

				str.Format("%f, %f", sub.x, sub.y);

				m_EditCalibSub.SetWindowTextA(str);
			}
		}

		m_pMain->m_nCalibRevisionAck = 0;
		if (nLimitX == 0)	dbData[AXIS_X] = 0;
		if (nLimitY == 0)	dbData[AXIS_Y] = 0;

		m_pMain->m_dbRevisionData[m_nJobID][AXIS_X] = dbData[AXIS_X];
		m_pMain->m_dbRevisionData[m_nJobID][AXIS_Y] = dbData[AXIS_Y];
		m_pMain->m_dbRevisionData[m_nJobID][AXIS_T] = dbData[AXIS_T];

		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start + dbMultiOffset + 7;
		address3 = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_word_start + dbMultiOffset;

		long lRevisionData[6] = { 0, };
		lRevisionData[0] = LOWORD(dbData[AXIS_X] * MOTOR_SCALE);
		lRevisionData[1] = HIWORD(dbData[AXIS_X] * MOTOR_SCALE);
		lRevisionData[2] = LOWORD(dbData[AXIS_Y] * MOTOR_SCALE);
		lRevisionData[3] = HIWORD(dbData[AXIS_Y] * MOTOR_SCALE);
		lRevisionData[4] = LOWORD(dbData[AXIS_T] * MOTOR_SCALE);
		lRevisionData[5] = HIWORD(dbData[AXIS_T] * MOTOR_SCALE);

		g_CommPLC.SetWord(address3, 6, lRevisionData);
		g_CommPLC.SetBit(address, TRUE);
			   
		str.Format("PC → PLC Send Calib Data(D%d)", address3);
		::SendMessageA(this->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		str.Format("PC → PLC Revision Pos Move Signal(M%d) Req", address);
		::SendMessageA(this->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		
		str.Format("X : %.1f, Y : %.1f", dbData[AXIS_X], dbData[AXIS_Y]);
		::SendMessageA(this->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_nTimeOutCount = 0;
		m_nSeqCalibration = 1300;
	}
	break;
	case 1300:	// 보정 위치 이동 완료 확인
	{
		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start + dbMultiOffset + 7;
		address2 = m_pMain->vt_job_info[m_nJobID].plc_addr_info.read_bit_start + dbMultiOffset + 7;

		BOOL revision_ack = g_CommPLC.GetBit(address2);

		if (m_nTimeOutCount % 100 == 0)
		{
			str.Format("Wait PLC → PC Revision Ack(L%d) Signal....", address2);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}

		if (++m_nTimeOutCount > 1000)
		{
			m_nSeqCalibration = 0;

			str.Format("PLC → Revision Pos Moving(L%d) Time Out", address2);
			::SendMessageA(this->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			nRet = 2;
			g_CommPLC.SetBit(address, FALSE);
			break;
		}

		if (revision_ack == FALSE) break;

		if (revision_ack == TRUE)
		{
			str.Format("PLC → PC Revision Pos Move(L%d) Complete", address2);
			::SendMessageA(this->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_nSeqCalibration = 1400;
			g_CommPLC.SetBit(address, FALSE);
		}

		Sleep(100);
	}
	break;
	case 1400:	// Grab 요청
	{
		Sleep(2000);
		for (int nCam = 0; nCam < m_nAlgorithmCamCount; nCam++)
		{
			real_cam = m_nAlgorithmCamBuff.at(nCam);
			auto& cam = m_pMain->m_Cameras[real_cam];
			if (cam && cam->IsConnected())
			{
				AcquireSRWLockExclusive(&cam->g_bufsrwlock);
				m_pMain->m_bGrabEnd[real_cam] = FALSE;
				ReleaseSRWLockExclusive(&cam->g_bufsrwlock);
			}
			else m_pMain->m_bGrabEnd[real_cam] = FALSE;
			if (cam && cam->IsConnected())
				m_pMain->SendMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_SINGLE_FRAME, real_cam);
		}

		str = "Grab Start";
		::SendMessageA(m_pMain->m_pPane[PANE_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		Sleep(100);

		m_nTimeOutCount = 0;
		m_nSeqCalibration = 1500;
	}
	break;
	case 1500:	// Grab 완료 대기
	{
		BOOL bGrabEnd = TRUE;

		for (int nCam = 0; nCam < m_nAlgorithmCamCount; nCam++)
		{
			real_cam = m_nAlgorithmCamBuff.at(nCam);
			if (m_pMain->m_bGrabEnd[real_cam] != TRUE)
				bGrabEnd = FALSE;
		}
		
		if (++m_nTimeOutCount > 1000)
		{
			m_nSeqCalibration = 0;
			nRet = 2;
			str = "Grab Timeout";
			::SendMessageA(this->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			break;
		}

		if (bGrabEnd != TRUE) break;

		str = "Grab Complete";
		::SendMessageA(this->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_nSeqCalibration = 1600;
	}
	break;
	case 1600:	// Find Pattern
	{
		Sleep(1000);

		BOOL bFind = TRUE;

		//int nPos = 0;  // 20211020 Tkyuha 개별 켈리브레이션을 수행하기 위해 삭제
		CFindInfo *pInfo;

		for (int nCam = 0; nCam < m_nAlgorithmCamCount; nCam++)
		{
			real_cam = m_nAlgorithmCamBuff.at(nCam);
			W = m_pMain->m_stCamInfo[real_cam].w;
			H = m_pMain->m_stCamInfo[real_cam].h;

			pInfo = &m_pMain->GetMatching(m_nJobID).getFindInfo(nCam, nPos);
			m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 0), m_pMain->getSrcBuffer(real_cam), W * H);
#ifdef _USE_CALIB_MARK
			UseClibrationMarkMatching((BYTE*)m_pMain->getProcBuffer(real_cam, 0), m_nJobID, nCam, nPos, W, H);
#else
			m_pMain->GetMatching(m_nJobID).findPattern((BYTE*)m_pMain->getProcBuffer(real_cam, 0), nCam, nPos, W, H);
#endif
			if (pInfo->GetFound() == FIND_ERR)
				bFind = FALSE;

			str.Format("Cam %d) %.1f%% %s", real_cam + 1, pInfo->getScore(), pInfo->GetFound() == FIND_OK ? "OK" : "NG");
			::SendMessageA(this->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

			// KBJ 2022-02-22 Machine draw info
			stDARW_MACHINE_INFO draw_info;
			draw_info.nCount = 1;
			draw_info.nCam = nCam;
			draw_info.nPos = nPos;
			draw_info.bClear = false;
			::SendMessageA(m_pMain->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_DISPLAY_MACHINE_VIEW_POS, (LPARAM)&draw_info);
		}		

		//::SendMessageA(m_pMain->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_DISPLAY_MACHINE_VIEW, 0);

		if (bFind != TRUE)
		{
			nRet = 2;	// Calibration NG
			break;
		}

		for (int nCam = 0; nCam < m_nAlgorithmCamCount; nCam++)
		{
			real_cam = m_nAlgorithmCamBuff.at(nCam);
			pInfo = &m_pMain->GetMatching(m_nJobID).getFindInfo(nCam, nPos);
			cv::Point2f pt;
			pt.x = float(pInfo->GetXPos());
			pt.y = float(pInfo->GetYPos());
			m_pMain->GetMachine(m_nJobID).addImagePoint(nCam, nPos, pt);

			// hsj 2023-01-02 calibration 값 나타내기 , 검증해야함..
			CString str;
			str.Format("%f, %f", pt.x, pt.y);
			m_EditCalibPixel.SetWindowTextA(str);

			str.Format("Cam %d : (%d , %d) (%.1f %.1f)", real_cam + 1, m_nCntX, m_nCntY, pInfo->GetXPos(), pInfo->GetYPos());
			::SendMessageA(m_pMain->m_pPane[PANE_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_ptrCalib[real_cam].push_back(pt);
		}

		m_nSeqCalibration = 1200;	// 반복
	}
	break;
	case 2000:	// Cabliration 좌표계 계산
	{
		BOOL bSuccess = TRUE;
		BOOL bError = FALSE;

		if (nOriLimitX == 1)
		{
			int nDir = m_pMain->m_iniFile.GetProfileIntA("SERVER_INFO", "VIRTUAL_COORDI_DIR", -1);

			for (int nCam = 0; nCam < m_nAlgorithmCamCount; nCam++)
			{
				real_cam = m_nAlgorithmCamBuff.at(nCam);
				m_pMain->GetMachine(m_nJobID).MakeVirtualCoordinates(&m_pMain->GetMachine(m_nJobID).getPtImage(nCam, nPos), &m_pMain->GetMachine(m_nJobID).getPtMotor(nCam, nPos), nDir, 0);
			}

			nLimitX = nLimitY;
		}
		else if (nOriLimitY == 1)
		{
			int nDir = m_pMain->m_iniFile.GetProfileIntA("SERVER_INFO", "VIRTUAL_COORDI_DIR", -1);

			for (int nCam = 0; nCam < m_nAlgorithmCamCount; nCam++)
			{
				m_pMain->GetMachine(m_nJobID).MakeVirtualCoordinates(&m_pMain->GetMachine(m_nJobID).getPtImage(nCam, nPos), &m_pMain->GetMachine(m_nJobID).getPtMotor(nCam, nPos), nDir, 1);
			}

			nLimitY = nLimitX;
		}

		m_pMain->GetMachine(m_nJobID).setNumCalPointsX(nLimitX);
		m_pMain->GetMachine(m_nJobID).setNumCalPointsY(nLimitY);

		for (int nCam = 0; nCam < m_nAlgorithmCamCount; nCam++)
		{
			real_cam = m_nAlgorithmCamBuff.at(nCam);
			if (m_pMain->GetMachine(m_nJobID).calculateCalibration(nCam, nPos) == 0)
			{
				double cX, cY;
				W = m_pMain->m_stCamInfo[real_cam].w;
				H = m_pMain->m_stCamInfo[real_cam].h;

				int n = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getAlignOrginPos(nCam);
				double xr = 0.5;
				double yr = 0.5;
				switch (n)
				{
				case _LEFT_TOP_X25_Y25:  xr = 0.25; yr = 0.25; break;
				case _CENTER_TOP_X50_Y25:  xr = 0.5; yr = 0.25; break;
				case _RIGHT_TOP_X75_Y25:  xr = 0.75; yr = 0.25; break;
				case _LEFT_CENTER_X25_Y50:  xr = 0.25; yr = 0.5; break;
				case _CENTER_CENTER_X50_Y50:  xr = 0.5; yr = 0.5; break;
				case _RIGHT_CENTER_X75_Y50:  xr = 0.75; yr = 0.5; break;
				case _LEFT_BOTTOM_X25_Y75:  xr = 0.25; yr = 0.75; break;
				case _CENTER_BOTTOM_X50_Y75:  xr = 0.5; yr = 0.75; break;
				case _RIGHT_BOTTOM_X75_Y75:  xr = 0.75; yr = 0.75; break;
				case _NOTCH_X85_Y50:		xr = 0.85; yr = 0.5; break;
				}

				m_pMain->GetMachine(m_nJobID).PixelToWorld(nCam, nPos, W * xr, H * yr, &cX, &cY, TRUE);
					
				m_pMain->GetMachine(m_nJobID).setOffsetX(nCam, nPos, cX);
				m_pMain->GetMachine(m_nJobID).setOffsetY(nCam, nPos, cY);
				m_pMain->GetMachine(m_nJobID).calculateResolution(nCam, nPos, nLimitX, nLimitY);
				// 20211202 Tkyuha Calibration 수행 일자 저장
				CTime time = CTime::GetCurrentTime();
				CString strTime = _T("");
				strTime.Format(_T("%04d/%02d/%02d  %02d:%02d:%02d"), time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute(), time.GetSecond());
				m_pMain->GetMachine(m_nJobID).setMakeClibDateTime(nCam, nPos, strTime);

				CString str_path;
				str_path.Format("%s%s\\", m_pMain->m_strMachineDir, m_pMain->vt_job_info[m_nJobID].job_name.c_str());
				m_pMain->GetMachine(m_nJobID).saveCalibrationData(nCam, nPos, str_path);

#ifndef JOB_INFO
				m_pMain->m_pViewDisplayInfo[cam].setResolutionX(m_pMain->GetMachine(m_nJobID).getCameraResolutionX(cam, nPos));
				m_pMain->m_pViewDisplayInfo[cam].setResolutionY(m_pMain->GetMachine(m_nJobID).getCameraResolutionY(cam, nPos));
#else
				save_resolution_to_viewer(m_nJobID, nCam, nPos);

#endif
				dispCameraResolution();
			}
			else
			{
				str.Format("Cam %d Calculate Calibration Data Failed.", nCam + 1);
				::SendMessageA(this->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

				bError = TRUE;
			}
		}

		if (bError != TRUE)	nRet = 1;
		else				nRet = 2;
	}
	break;
	}

	Sleep(100);
	return nRet;
}

void TabMachineAlgoPage::save_resolution_to_viewer(int job, int cam, int pos)
{
	CString str_section, str_value;
	int viewer = 0;
	int algo_method = m_pMain->vt_job_info[job].algo_method;
	SYSTEMTIME time;
	::GetLocalTime(&time);

	switch (algo_method) {
	case CLIENT_TYPE_2CAM_EACH_2SHOT_ALIGN:
		{
			for (int i = 0; i < 2; i++)
			{
				viewer = m_pMain->vt_job_info[job].viewer_index[cam * 2 + i];
				m_pMain->vt_viewer_info[viewer].resolution_x = m_pMain->GetMachine(m_nJobID).getCameraResolutionX(cam, pos);
				m_pMain->vt_viewer_info[viewer].resolution_y = m_pMain->GetMachine(m_nJobID).getCameraResolutionY(cam, pos);

				str_section.Format("VIEWER%d_INFO", viewer + 1);
				str_value.Format("%f", m_pMain->vt_viewer_info[viewer].resolution_x);
				m_pMain->m_iniViewerFile.WriteProfileStringA(str_section, "RESOLUTION_X", str_value);

				str_value.Format("%f", m_pMain->vt_viewer_info[viewer].resolution_y);
				m_pMain->m_iniViewerFile.WriteProfileStringA(str_section, "RESOLUTION_Y", str_value);

				str_value.Format("%04d-%02d-%02d_%02d:%02d:%02d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
				m_pMain->m_iniViewerFile.WriteProfileStringA(str_section, "CALIBRATION_DATE", str_value);

				str_value.Format("%d", job + 1);
				m_pMain->m_iniViewerFile.WriteProfileStringA(str_section, "CALIBRATION_JOB", str_value);

				str_value.Format("%d", cam + 1);
				m_pMain->m_iniViewerFile.WriteProfileStringA(str_section, "CALIBRATION_CAM", str_value);

				str_value.Format("%d", pos + 1);
				m_pMain->m_iniViewerFile.WriteProfileStringA(str_section, "CALIBRATION_POS", str_value);
			}
		}
		break;
	case CLIENT_TYPE_1CAM_2SHOT_ALIGN:
		{
			viewer = m_pMain->vt_job_info[job].viewer_index[pos];
			m_pMain->vt_viewer_info[viewer].resolution_x = m_pMain->GetMachine(m_nJobID).getCameraResolutionX(cam, pos);
			m_pMain->vt_viewer_info[viewer].resolution_y = m_pMain->GetMachine(m_nJobID).getCameraResolutionY(cam, pos);

			str_section.Format("VIEWER%d_INFO", viewer + 1);
			str_value.Format("%f", m_pMain->vt_viewer_info[viewer].resolution_x);
			m_pMain->m_iniViewerFile.WriteProfileStringA(str_section, "RESOLUTION_X", str_value);

			str_value.Format("%f", m_pMain->vt_viewer_info[viewer].resolution_y);
			m_pMain->m_iniViewerFile.WriteProfileStringA(str_section, "RESOLUTION_Y", str_value);

			str_value.Format("%04d-%02d-%02d_%02d:%02d:%02d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
			m_pMain->m_iniViewerFile.WriteProfileStringA(str_section, "CALIBRATION_DATE", str_value);

			str_value.Format("%d", job + 1);
			m_pMain->m_iniViewerFile.WriteProfileStringA(str_section, "CALIBRATION_JOB", str_value);

			str_value.Format("%d", cam + 1);
			m_pMain->m_iniViewerFile.WriteProfileStringA(str_section, "CALIBRATION_CAM", str_value);

			str_value.Format("%d", pos + 1);
			m_pMain->m_iniViewerFile.WriteProfileStringA(str_section, "CALIBRATION_POS", str_value);
		}
		break;
	case CLIENT_TYPE_ELB_CENTER_ALIGN:
	{
		int nv = m_pMain->vt_job_info[job].num_of_viewer;
		for (int i = 0; i < nv; i++)
		{
			viewer = m_pMain->vt_job_info[job].viewer_index[i];
			m_pMain->vt_viewer_info[viewer].resolution_x = m_pMain->GetMachine(m_nJobID).getCameraResolutionX(cam, pos);
			m_pMain->vt_viewer_info[viewer].resolution_y = m_pMain->GetMachine(m_nJobID).getCameraResolutionY(cam, pos);

			str_section.Format("VIEWER%d_INFO", viewer + 1);
			str_value.Format("%f", m_pMain->vt_viewer_info[viewer].resolution_x);
			m_pMain->m_iniViewerFile.WriteProfileStringA(str_section, "RESOLUTION_X", str_value);

			str_value.Format("%f", m_pMain->vt_viewer_info[viewer].resolution_y);
			m_pMain->m_iniViewerFile.WriteProfileStringA(str_section, "RESOLUTION_Y", str_value);

			str_value.Format("%04d-%02d-%02d_%02d:%02d:%02d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
			m_pMain->m_iniViewerFile.WriteProfileStringA(str_section, "CALIBRATION_DATE", str_value);

			str_value.Format("%d", job + 1);
			m_pMain->m_iniViewerFile.WriteProfileStringA(str_section, "CALIBRATION_JOB", str_value);

			str_value.Format("%d", cam + 1);
			m_pMain->m_iniViewerFile.WriteProfileStringA(str_section, "CALIBRATION_CAM", str_value);

			str_value.Format("%d", pos + 1);
			m_pMain->m_iniViewerFile.WriteProfileStringA(str_section, "CALIBRATION_POS", str_value);
		}
	}
		break;
	case CLIENT_TYPE_1CAM_1SHOT_FILM:
	case CLIENT_TYPE_1CAM_1SHOT_ALIGN:
	case CLIENT_TYPE_1CAM_1SHOT_ALIGN_WITH_SCRATCH_INSP:
	case CLIENT_TYPE_ALIGN:
	default:
		{
			m_pMain->vt_viewer_info[viewer].resolution_x = m_pMain->GetMachine(m_nJobID).getCameraResolutionX(cam, pos);
			m_pMain->vt_viewer_info[viewer].resolution_y = m_pMain->GetMachine(m_nJobID).getCameraResolutionY(cam, pos);

			str_section.Format("VIEWER%d_INFO", viewer + 1);
			str_value.Format("%f", m_pMain->vt_viewer_info[viewer].resolution_x);
			m_pMain->m_iniViewerFile.WriteProfileStringA(str_section, "RESOLUTION_X", str_value);

			str_value.Format("%f", m_pMain->vt_viewer_info[viewer].resolution_y);
			m_pMain->m_iniViewerFile.WriteProfileStringA(str_section, "RESOLUTION_Y", str_value);

			str_value.Format("%04d-%02d-%02d_%02d:%02d:%02d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
			m_pMain->m_iniViewerFile.WriteProfileStringA(str_section, "CALIBRATION_DATE", str_value);

			str_value.Format("%d", job + 1);
			m_pMain->m_iniViewerFile.WriteProfileStringA(str_section, "CALIBRATION_JOB", str_value);

			str_value.Format("%d", cam + 1);
			m_pMain->m_iniViewerFile.WriteProfileStringA(str_section, "CALIBRATION_CAM", str_value);

			str_value.Format("%d", pos + 1);
			m_pMain->m_iniViewerFile.WriteProfileStringA(str_section, "CALIBRATION_POS", str_value);
		}
		break;
	}
}

int TabMachineAlgoPage::seqAutoCalibration4()
{
	//individual calibration

	CString str;
	int nRet = 0, address = 0, address2 = 0, address3, cam = 0, W, H;;
	double dbData[3] = { 0, };
	double dbStepX, dbStepY;	// 1.2mm,	0.9mm
	int nLimitX = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getCalibrationCountX();
	int nLimitY = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getCalibrationCountY();
	double dbOffsetX = 0.0, dbOffsetY = 0.0;

#ifndef JOB_INFO
	int camCount = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraNum(m_nTabAlgoIndex);
	std::vector<int> camBuf = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(m_nTabAlgoIndex);
#else
	int camCount = m_pMain->vt_job_info[m_nJobID].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[m_nJobID].camera_index;
#endif

	int nCam = ((CComboBox *)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->GetCurSel();
	int nSelPos = ((CComboBox *)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();

	int nCalibCam = camBuf.at(nCam);

	if (nLimitX <= 1)	dbStepX = 0.0;
	else				dbStepX = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getCalibrationRangeX() / (nLimitX - 1);

	if (nLimitY <= 1)	dbStepY = 0.0;
	else				dbStepY = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getCalibrationRangeY() / (nLimitY - 1);

	if (nLimitX == 0)
	{
		dbStepX = 0;
		nLimitX = 3;
	}
	if (nLimitY == 0)
	{
		dbStepY = 0;
		nLimitY = 3;
	}

	switch (m_nSeqCalibration) {
	case 0:
		KillTimer(TIMER_CALIBRATION4);
		nRet = 2;
		break;
	case 100:	// 1. Calibration 가능 상태 확인
	{
		//str = "Calibration Request";
		//OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		//address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.read_bit_start;
		//m_pMain->SendServer(TCP_SEND_CALIBRATION_REQUEST, address + 5);

		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.read_bit_start + 5;
		BOOL bEnable = g_CommPLC.GetBit(address);

		if (bEnable != TRUE)
		{
			str.Format("PLC Calib Enable(L%d) Signal Off!!", address);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			nRet = 2;
		}
		else
		{
			str.Format("PLC Calib Enable(L%d) Signal On", address);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}

		m_nSeqCalibration = 200;
	}
	break;	
	case 200:	// Calibration Mode ON
	{
		//KJH 2022-05-19 Nozzle Calibration시 Cam Bit 미리 켜주기
		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;

		if (m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_NOZZLE_ALIGN ||
			m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_CENTER_NOZZLE_ALIGN ||
			m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_CENTER_SIDE_YGAP)
		{
			for (int ci = 0; ci < 3; ci++) m_pMain->SendServer(TCP_SEND_CSCAM_CALIB_ON, address + 8 + ci, FALSE);
			//KJH 2022-01-10 캘리브 bit변경 ( 48 : Side , 49 : Center, 50 : Gap)
			if (m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_CENTER_SIDE_YGAP)
				m_pMain->SendServer(TCP_SEND_CSCAM_CALIB_ON, address + 8 + nCam + 2, TRUE);
			else m_pMain->SendServer(TCP_SEND_CSCAM_CALIB_ON, address + 8 + nCam, TRUE);
			Sleep(3000);
		}

		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start + 6;

		g_CommPLC.SetBit(address, TRUE);

		m_nSeqCalibration = 250;
		m_nTimeOutCount = 0;

		str.Format("Calib Mode On(M%d)", address);
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
	break;
	case 250:	// Wait Calib Mode On Ack Signal
	{
		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.read_bit_start  + 6;
		BOOL bAck = g_CommPLC.GetBit(address);

		if (m_nTimeOutCount % 100 == 0)
		{
			str.Format("Wait PLC → PC Mode On(L%d) Ack Signal....", address);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}

		if (m_nTimeOutCount++ > 1000)
		{
			m_nTimeOutCount = 0;
			str.Format("Timeout - Wait PLC → PC Mode On Ack Signal");
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

			nRet = 2;
			break;
		}

		if (bAck)
		{
			m_nTimeOutCount = 0;
			m_nSeqCalibration = 300;
			break;
		}
	}
	break;
	case 300:
	{
		str = "Grab Start";
		::SendMessageA(m_pMain->m_pPane[PANE_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;

		if (m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_NOZZLE_ALIGN ||
			m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_CENTER_NOZZLE_ALIGN ||
			m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_CENTER_SIDE_YGAP)
		{
			for(int ci=0;ci<3;ci++) m_pMain->SendServer(TCP_SEND_CSCAM_CALIB_ON, address + 8 + ci, FALSE);
			//KJH 2022-01-10 캘리브 bit변경 ( 48 : Side , 49 : Center, 50 : Gap)
			if(m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_CENTER_SIDE_YGAP)
				m_pMain->SendServer(TCP_SEND_CSCAM_CALIB_ON, address + 8 + nCam + 2, TRUE);
			else m_pMain->SendServer(TCP_SEND_CSCAM_CALIB_ON, address + 8 + nCam, TRUE);
			Sleep(3000);
        }

		auto& cam = m_pMain->m_Cameras[nCalibCam];
		if (cam && cam->IsConnected())
		{
			AcquireSRWLockExclusive(&cam->g_bufsrwlock);
			m_pMain->m_bGrabEnd[nCalibCam] = FALSE;
			ReleaseSRWLockExclusive(&cam->g_bufsrwlock);
		}
		else m_pMain->m_bGrabEnd[nCalibCam] = FALSE;
		if (cam && cam->IsConnected())
			m_pMain->SendMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_SINGLE_FRAME, nCalibCam);

		Sleep(300);

		m_nTimeOutCount = 0;
		m_nSeqCalibration = 400;
	}
	break;
	case 400:	// 8. Grab 완료 대기
	{
		BOOL bGrabEnd = TRUE;

		if (m_pMain->m_bGrabEnd[nCalibCam] != TRUE)
			bGrabEnd = FALSE;

		if (bGrabEnd != TRUE) break;

		str = "Grab Complete";
		::SendMessageA(m_pMain->m_pPane[PANE_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_nSeqCalibration = 500;
	}
	break;
	case 500:	// 9. Find Pattern
	{
		Sleep(1000);
		BOOL bFind = TRUE;

		int nPos = nSelPos;

		CFindInfo *pInfo;
		W = m_pMain->m_stCamInfo[nCalibCam].w;
		H = m_pMain->m_stCamInfo[nCalibCam].h;

		pInfo = &m_pMain->GetMatching(m_nJobID).getFindInfo(nCam, nPos);
		m_pMain->copyMemory(m_pMain->getProcBuffer(nCalibCam, 0), m_pMain->getSrcBuffer(nCalibCam), W * H);
#ifdef _USE_CALIB_MARK
		UseClibrationMarkMatching((BYTE*)m_pMain->getProcBuffer(nCalibCam, 0), m_nJobID, nCam, nPos, W, H);
#else
		m_pMain->GetMatching(m_nJobID).findPattern((BYTE*)m_pMain->getProcBuffer(nCalibCam, 0), nCam, nPos, W, H);
#endif
		if (pInfo->GetFound() == FIND_ERR)
			bFind = FALSE;

		m_pMain->GetMachine(m_nJobID).setCalibStartX(nCam, nPos, pInfo->GetXPos());
		m_pMain->GetMachine(m_nJobID).setCalibStartY(nCam, nPos, pInfo->GetYPos());

		((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_dbCalibTargetX[nCalibCam] = pInfo->GetXPos();
		((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_dbCalibTargetY[nCalibCam] = pInfo->GetYPos();

		str.Format("Cam %d) %.1f%% %s", nCalibCam + 1, pInfo->getScore(), pInfo->GetFound() == FIND_OK ? "OK" : "NG");
		::SendMessageA(m_pMain->m_pPane[PANE_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		::SendMessageA(m_pMain->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_DISPLAY_INDIVIDUAL_DRAW_MARK, 0);

		if (bFind != TRUE)
		{
			nRet = 2;	// Calibration NG
			break;
		}

		m_nSeqCalibration = 1000;
	}
	break;
	case 1000:	// 4. Calibration 관련 데이터 초기화
	{
		// Reset Calib Data
		int nPos = nSelPos;

		m_pMain->GetMachine(m_nJobID).getPtImage(nCam, nPos).clear();
		m_pMain->GetMachine(m_nJobID).getPtMotor(nCam, nPos).clear();

		m_nCntX = -1;
		m_nCntY = -1;

		m_nSeqCalibration = 1100;
	}
	break;
	case 1100:
	{
		if (++m_nCntY == nLimitY)	// Y 초기 위치
		{
			m_nSeqCalibration = 2000;
			break;
		}

		m_nSeqCalibration = 1200;
	}
	break;
	case 1200:	// 5. 보정 위치 이동 요청
	{
		if (++m_nCntX == nLimitX)
		{
			m_nCntX = -1;
			m_nSeqCalibration = 1100;
			break;
		}

		// 상대 좌표계 이동 
		if (m_nCntX == 0)
		{
			if (m_nCntY == 0)
			{
				dbData[AXIS_X] = -dbStepX * ((nLimitX - 1) / 2.0);
				dbData[AXIS_Y] = -dbStepY * ((nLimitY - 1) / 2.0);
				dbData[AXIS_T] = 0;
			}
			else
			{
				dbData[AXIS_X] = (-dbStepX) * (nLimitX - 1);
				dbData[AXIS_Y] = dbStepY;
				dbData[AXIS_T] = 0;
			}
		}
		else
		{
			dbData[AXIS_X] = dbStepX;
			dbData[AXIS_Y] = 0;
			dbData[AXIS_T] = 0;
		}

		cv::Point2f pt;
		pt.x = float((m_nCntX - (nLimitX - 1) / 2.0) * dbStepX + dbOffsetX);
		pt.y = float((m_nCntY - (nLimitY - 1) / 2.0) * dbStepY + dbOffsetY);

		int nPos = nSelPos;

		m_pMain->GetMachine(m_nJobID).addMotorPoint(nCam, nPos, pt);

		m_pMain->m_nCalibRevisionAck = 0;

		m_pMain->m_dbRevisionData[m_nJobID][AXIS_X] = dbData[AXIS_X];
		m_pMain->m_dbRevisionData[m_nJobID][AXIS_Y] = dbData[AXIS_Y];
		m_pMain->m_dbRevisionData[m_nJobID][AXIS_T] = dbData[AXIS_T];

#ifndef JOB_INFO
		address = m_pMain->m_pAlgorithmInfo.getAlgorithmWriteBitStart(m_nTabAlgoIndex);
		address2 = m_pMain->m_pAlgorithmInfo.getAlgorithmReadBitStart(m_nTabAlgoIndex);
		address3 = m_pMain->m_pAlgorithmInfo.getAlgorithmWriteWordStart(m_nTabAlgoIndex);
		m_pMain->SendServer(TCP_SEND_CSCAM_CALIB_DATA, address, address3, &address2);
#else
		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;
		address2 = m_pMain->vt_job_info[m_nJobID].plc_addr_info.read_bit_start;
		address3 = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_word_start;
		m_pMain->SendServer(TCP_SEND_CSCAM_CALIB_DATA, address, address3, &address2);
#endif

		str = "PC → PLC Send Calib Data";
		::SendMessageA(m_pMain->m_pPane[PANE_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		str.Format("X : %.1f, Y : %.1f", dbData[AXIS_X], dbData[AXIS_Y]);
		::SendMessageA(m_pMain->m_pPane[PANE_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		str = "PC → PLC Revision Pos Move Req";
		::SendMessageA(m_pMain->m_pPane[PANE_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_nSeqCalibration = 1300;
	}
	break;
	case 1300:	// 6. 보정 위치 이동 완료 확인
	{
		/*if( ++m_nTimeOutCount > 100 )
		{
			m_nSeqCalibration = 0;

			str = "PLC → Revision Pos Moving Time Out";
			::SendMessageA(m_pMain->m_pPane[PANE_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			nRet = 2;
			break;
		}*/

		if (m_pMain->m_nCalibRevisionAck == 0) break;

		if (m_pMain->m_nCalibRevisionAck == 1)
		{
			str = "PLC → PC Revision Pos Move Complete";
			::SendMessageA(m_pMain->m_pPane[PANE_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_nSeqCalibration = 1400;
		}
		else
		{
			str = "PLC → PC Revision Pos Move Failed";
			::SendMessageA(m_pMain->m_pPane[PANE_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_nSeqCalibration = 0;
		}

		Sleep(100);
	}
	break;
	case 1400:	// 7. Grab 요청
	{
		if (m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_NOZZLE_ALIGN ||
			m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_CENTER_NOZZLE_ALIGN ||
			m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_CENTER_SIDE_YGAP)
		{
			Sleep(2000);
		}

		auto& cam = m_pMain->m_Cameras[nCalibCam];
		if (cam && cam->IsConnected())
		{
			AcquireSRWLockExclusive(&cam->g_bufsrwlock);
			m_pMain->m_bGrabEnd[nCalibCam] = FALSE;
			ReleaseSRWLockExclusive(&cam->g_bufsrwlock);
		}
		else m_pMain->m_bGrabEnd[nCalibCam] = FALSE;
		if (cam && cam->IsConnected())
			m_pMain->SendMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_SINGLE_FRAME, nCalibCam);

		str = "Grab Start";
		::SendMessageA(m_pMain->m_pPane[PANE_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		Sleep(100);

		m_nTimeOutCount = 0;
		m_nSeqCalibration = 1500;
	}
	break;
	case 1500:	// 8. Grab 완료 대기
	{
		BOOL bGrabEnd = TRUE;

		if (m_pMain->m_bGrabEnd[nCalibCam] != TRUE)
			bGrabEnd = FALSE;

		/*if( ++m_nTimeOutCount > 100 )
		{
			m_nSeqCalibration = 0;
			nRet = 2;
			str = "Grab Timeout";
			::SendMessageA(m_pMain->m_pPane[PANE_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			break;
		}*/

		if (bGrabEnd != TRUE) break;

		str = "Grab Complete";
		::SendMessageA(m_pMain->m_pPane[PANE_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_nSeqCalibration = 1600;
	}
	break;
	case 1600:	// 9. Find Pattern
	{
		Sleep(1000);

		BOOL bFind = TRUE;

		CFindInfo *pInfo;
		int nPos = nSelPos;

		W = m_pMain->m_stCamInfo[nCalibCam].w;
		H = m_pMain->m_stCamInfo[nCalibCam].h;

		pInfo = &m_pMain->GetMatching(m_nJobID).getFindInfo(nCam, nPos);
		m_pMain->copyMemory(m_pMain->getProcBuffer(nCalibCam, 0), m_pMain->getSrcBuffer(nCalibCam), W * H);
#ifdef _USE_CALIB_MARK
		UseClibrationMarkMatching((BYTE*)m_pMain->getProcBuffer(nCalibCam, 0), m_nJobID, nCam, nPos, W, H);
#else
		m_pMain->GetMatching(m_nJobID).findPattern((BYTE*)m_pMain->getProcBuffer(nCalibCam, 0), nCam, nPos, W, H);
#endif
		if (pInfo->GetFound() == FIND_ERR)
			bFind = FALSE;

		str.Format("Cam %d) %.1f%% %s", nCalibCam + 1, pInfo->getScore(), pInfo->GetFound() == FIND_OK ? "OK" : "NG");
		::SendMessageA(m_pMain->m_pPane[PANE_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		::SendMessageA(m_pMain->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_DISPLAY_INDIVIDUAL_DRAW_MARK, 0);

		if (bFind != TRUE)
		{
			nRet = 2;	// Calibration NG
			break;
		}

		pInfo = &m_pMain->GetMatching(m_nJobID).getFindInfo(nCam, nPos);
		cv::Point2f pt;
		pt.x = float(pInfo->GetXPos());
		pt.y = float(pInfo->GetYPos());
		m_pMain->GetMachine(m_nJobID).addImagePoint(nCam, nPos, pt);

		str.Format("Cam %d : (%d , %d) (%.1f %.1f)", nCalibCam, m_nCntX, m_nCntY, pInfo->GetXPos(), pInfo->GetYPos());
		::SendMessageA(m_pMain->m_pPane[PANE_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_ptrCalib[nCalibCam].push_back(pt);

		m_nSeqCalibration = 1200;	// 반복
	}
	break;
	case 2000:	// 10. Cabliration 좌표계 계산
	{
		BOOL bSuccess = TRUE;
		BOOL bError = FALSE;

		m_pMain->GetMachine(m_nJobID).setNumCalPointsX(nLimitX);
		m_pMain->GetMachine(m_nJobID).setNumCalPointsY(nLimitY);

		int nPos = nSelPos;

		if (nLimitX == 1 || nLimitY)  // Tkyuha 20221125 한축만을 사용 하여 Calibration을 수행 할때 사용 하기 위함
			m_pMain->GetMachine(m_nJobID).calculate_Calibration_1_Axis_data(nCam, 0);

		if (m_pMain->GetMachine(m_nJobID).calculateCalibration(nCam, 0) == 0)
		{
			double cX, cY;
			int nLimitX = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getCalibrationCountX();
			int nLimitY = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getCalibrationCountY();

			W = m_pMain->m_stCamInfo[nCalibCam].w;
			H = m_pMain->m_stCamInfo[nCalibCam].h;

			m_pMain->GetMachine(m_nJobID).PixelToWorld(nCam, nPos, W / 2.0, H / 2.0, &cX, &cY, TRUE);

			m_pMain->GetMachine(m_nJobID).setOffsetX(nCam, nPos, cX);
			m_pMain->GetMachine(m_nJobID).setOffsetY(nCam, nPos, cY);
			m_pMain->GetMachine(m_nJobID).calculateResolution(nCam, nPos, nLimitX, nLimitY);

			// 20211202 Tkyuha Calibration 수행 일자 저장
			CTime time = CTime::GetCurrentTime();
			CString strTime = _T("");
			strTime.Format(_T("%04d/%02d/%02d  %02d:%02d:%02d"), time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute(), time.GetSecond());
			m_pMain->GetMachine(m_nJobID).setMakeClibDateTime(nCam, nPos, strTime);

			CString str_path;
			str_path.Format("%s%s\\", m_pMain->m_strMachineDir, m_pMain->vt_job_info[m_nJobID].job_name.c_str());
			m_pMain->GetMachine(m_nJobID).saveCalibrationData(nCam, nPos, str_path);

#ifndef JOB_INFO
			m_pMain->m_pViewDisplayInfo[nCalibCam].setResolutionX(m_pMain->GetMachine(m_nTabAlgoIndex).getCameraResolutionX(nCalibCam, nPos));
			m_pMain->m_pViewDisplayInfo[nCalibCam].setResolutionY(m_pMain->GetMachine(m_nTabAlgoIndex).getCameraResolutionY(nCalibCam, nPos));
#else
			int viewer = m_pMain->vt_job_info[m_nJobID].viewer_index[nCam];
			m_pMain->vt_viewer_info[viewer].resolution_x = m_pMain->GetMachine(m_nJobID).getCameraResolutionX(nCam, nPos);
			m_pMain->vt_viewer_info[viewer].resolution_y = m_pMain->GetMachine(m_nJobID).getCameraResolutionY(nCam, nPos);
			save_resolution_to_ini(viewer, m_nJobID, nCam, nPos); // Tkyuha 20211127 해상도 ini저장 함
#endif

			dispCameraResolution();
		//	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_DISPLAY_DRAW_VIEW, 0);
		}
		else
		{
			str.Format("Cam %d Calculate Calibration Data Failed.", nCalibCam + 1);
			::SendMessageA(m_pMain->m_pPane[PANE_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

			bError = TRUE;
		}

		if (m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_NOZZLE_ALIGN ||
			m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_CENTER_NOZZLE_ALIGN ||
			m_pMain->vt_job_info[m_nJobID].algo_method == CLIENT_TYPE_CENTER_SIDE_YGAP)
		{
			for (int ci = 0; ci < 3; ci++) m_pMain->SendServer(TCP_SEND_CSCAM_CALIB_ON, address + 8 + ci, FALSE);
		}

		if (bError != TRUE)	nRet = 1;
		else				nRet = 2;

	}
	break;
	}

	Sleep(100);
	return nRet;
}

int TabMachineAlgoPage::seqCalculateRotateCenter()
{
	CString str;
	BOOL bFind = TRUE;
	int nRet = 0, address, address2, address3, real_cam, W, H;
	double dbData[3] = { 0, 0, 0 };
	double  dbRange = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getRotateRange();
	int nRotateCount = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getRotateCount();
	double  dbStepT = dbRange/( nRotateCount==0?1: nRotateCount);

	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();;
	int num_of_camera = m_pMain->vt_job_info[m_nJobID].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[m_nJobID].camera_index;

	int tm = STM_PN_ALIGN + m_nJobID;

#ifndef NO_MIL
	switch (m_nSeqRotateCenter) {
	case 0: KillTimer(TIMER_ROTATE_CENTER); break;
	case 100:		// Calibration 가능 상태 확인
	{
		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.read_bit_start;

		m_pMain->SendServer(TCP_SEND_CALIBRATION_REQUEST, address + 5);
		m_nSeqRotateCenter = 200;

		str.Format("Rotation Center Request(%d)", address + 5);
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
	break;
	case 200:		// Calibration Mode ON
	{
		if (m_pMain->m_nCalibrationAck == 0)	break;
		else if (m_pMain->m_nCalibrationAck == 1)
		{
			str = "Recv Rotate center Ack";
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_nSeqRotateCenter = 1000;
		}
		else if (m_pMain->m_nCalibrationAck == 2)
		{
			str = "PLC Enable Signal Off";
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_nSeqRotateCenter = 0;
			nRet = 2;
		}
	}
	break;
	case 1000:		// 이동값 전송
	{
		dbData[AXIS_T] = dbStepT;	// 나머지 얼라인은 현재 위치 기준 보정

		for (int nCam = 0; nCam < num_of_camera; nCam++)
		{
			real_cam = camBuf.at(nCam);
			m_dbMotorT[real_cam][m_nRotateCnt] = dbData[AXIS_T];
		}

		str.Format("T : %.1f", dbData[AXIS_T]);
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_pMain->m_nCalibRevisionAck = 0;
		m_pMain->m_dbRevisionData[m_nJobID][AXIS_X] = 0;
		m_pMain->m_dbRevisionData[m_nJobID][AXIS_Y] = 0;
		m_pMain->m_dbRevisionData[m_nJobID][AXIS_T] = dbData[AXIS_T];

		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;
		address2 = m_pMain->vt_job_info[m_nJobID].plc_addr_info.read_bit_start;
		address3 = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_word_start;
		m_pMain->SendServer(TCP_SEND_CSCAM_CALIB_DATA, address, address3, &address2);

		str = "PC → PLC Calibration Move Reqeust";
		SendMessageA(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_nSeqRotateCenter = 1100;
	}
	break;
	case 1100:		// 보정 위치 이동 완료 확인
	{
		if (m_pMain->m_nCalibRevisionAck == 0)	break;

		str = "PLC → PC Rotation Center Move Complete";
		SendMessageA(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		if (m_pMain->m_nCalibRevisionAck == 1)
		{
			str = "PLC → PC Revision Pos Move Complete";
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_nSeqRotateCenter = 1110;
			seq_SetTimer(tm, 500);
		}
		else
		{
			str = "PLC → PC Revision Pos Move Failed";
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_nSeqRotateCenter = 0;
		}
	}
	break;
	case 1110:
		if (seq_ChkTimer(tm))
		{
			seq_ResetChkTimer(tm);
			m_nSeqRotateCenter = 1200;
		}
		break;
	case 1200:		// Grab 요청
	{
		for (int nCam = 0; nCam < num_of_camera; nCam++)
		{
			real_cam = camBuf.at(nCam);
			auto& cam = m_pMain->m_Cameras[real_cam];
			if (cam && cam->IsConnected())
			{
				AcquireSRWLockExclusive(&cam->g_bufsrwlock);
				m_pMain->m_bGrabEnd[real_cam] = FALSE;
				ReleaseSRWLockExclusive(&cam->g_bufsrwlock);
			}
			else m_pMain->m_bGrabEnd[real_cam] = FALSE;
			if (cam && cam->IsConnected())
				m_pMain->SendMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_SINGLE_FRAME, real_cam);
		}

		m_nSeqRotateCenter = 1300;
	}
	break;
	case 1300:		// Grab 완료 대기
	{
		BOOL bGrabEnd = TRUE;

		for (int nCam = 0; nCam < num_of_camera; nCam++)
		{
			real_cam = camBuf.at(nCam);
			if (m_pMain->m_bGrabEnd[real_cam] != TRUE)
				bGrabEnd = FALSE;
		}

		if (bGrabEnd != TRUE) break;

		m_nSeqRotateCenter = 1400;
	}
	break;
	case 1400:		// Find Pattern
	{
		BOOL bFind2 = TRUE;
		CFindInfo* pInfo;

		for (int nCam = 0; nCam < num_of_camera; nCam++)
		{
			real_cam = camBuf.at(nCam);
			W = m_pMain->m_stCamInfo[real_cam].w;
			H = m_pMain->m_stCamInfo[real_cam].h;

			pInfo = &m_pMain->GetMatching(m_nJobID).getFindInfo(nCam, nPos);
			m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 0), m_pMain->getSrcBuffer(real_cam), W * H);
#ifdef _USE_CALIB_MARK
			UseClibrationMarkMatching((BYTE*)m_pMain->getProcBuffer(real_cam, 0), m_nJobID, nCam, nPos, W, H);
#else
			m_pMain->GetMatching(m_nJobID).findPattern((BYTE*)m_pMain->getProcBuffer(real_cam, 0), nCam, nPos, W, H);
#endif
			if (pInfo->GetFound() == FIND_ERR)
				bFind2 = FALSE;

			str.Format("Cam %d) %.1f%% %s", nCam + 1, pInfo->getScore(), pInfo->GetFound() == FIND_OK ? "OK" : "NG");
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}

		::SendMessageA(m_pMain->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_DISPLAY_MACHINE_VIEW, 0);

		if (bFind2 != TRUE)
		{
			nRet = 2;
			break;
		}

		for (int nCam = 0; nCam < num_of_camera; nCam++)
		{
			real_cam = camBuf.at(nCam);

			pInfo = &m_pMain->GetMatching(m_nJobID).getFindInfo(nCam, nPos);
			double posX = pInfo->GetXPos();
			double posY = pInfo->GetYPos();

			m_listPtOnCircle[nCam].push_back(cv::Point2d(posX,posY));

			if (m_nRotateCnt == 0)
			{
				m_pMain->GetMachine(m_nJobID).setImageOrgX(nCam, nPos, m_nRotateCnt, posX);
				m_pMain->GetMachine(m_nJobID).setImageOrgY(nCam, nPos, m_nRotateCnt, posY);
				m_pMain->GetMachine(m_nJobID).PixelToWorld(nCam, nPos, posX, posY, &posX, &posY);
				m_pMain->GetMachine(m_nJobID).setImageX(nCam, nPos, m_nRotateCnt, posX);
				m_pMain->GetMachine(m_nJobID).setImageY(nCam, nPos, m_nRotateCnt, posY);
			}

			str.Format("Cam %d : (%.1f %.1f) (%.3f, %.3f)", nCam + 1, pInfo->GetXPos(), pInfo->GetYPos(), posX, posY);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}

		m_nRotateCnt++;
		if(nRotateCount<= m_nRotateCnt) m_nSeqRotateCenter = 2000;		// 종료
		else m_nSeqRotateCenter = 1000;		// 반복 수행
	}
	break;	
	case 2000:	// 9. Rotate Center 계산
	{
		BOOL bSuccess = TRUE;
		m_pMain->GetMachine(m_nJobID).setMotorT(dbStepT * 2);
	
		for (int nCam = 0; nCam < num_of_camera; nCam++)
		{
			real_cam = camBuf.at(nCam);
			cv::Point2d rotateCenter = CalcRotationCenterMulti(m_nRotateCnt, m_listPtOnCircle[nCam], m_dRadius);

			if (bSuccess != TRUE)
			{
				nRet = 2;
				break;
			}

			m_pMain->GetMachine(m_nJobID).PixelToWorld(nCam, nPos, rotateCenter.x, rotateCenter.y, &rotateCenter.x, &rotateCenter.y);

			m_pMain->GetMachine(m_nJobID).setRotateX(nCam, nPos, rotateCenter.x);
			m_pMain->GetMachine(m_nJobID).setRotateY(nCam, nPos, rotateCenter.y);

			m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().setRotateCenterX(nCam, nPos, rotateCenter.x);
			m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().setRotateCenterY(nCam, nPos, rotateCenter.y);

			CString str_path;
			str_path.Format("%s%s\\", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[m_nJobID].job_name.c_str());
			m_pMain->GetMachine(m_nJobID).saveRotateCenter(str_path, nCam, nPos);
			str.Format("Cam %d) dbRotateX = %.2f, dbRotateY = %.2f", real_cam + 1, rotateCenter.x, rotateCenter.y);

			double pixel[4];

			pixel[0] = m_pMain->GetMachine(m_nJobID).getImageX(nCam, nPos, 1);
			pixel[1] = m_pMain->GetMachine(m_nJobID).getImageY(nCam, nPos, 1);
			pixel[2] = m_pMain->GetMachine(m_nJobID).getImageX(nCam, nPos, 0);
			pixel[3] = m_pMain->GetMachine(m_nJobID).getImageY(nCam, nPos, 0);

			((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_dbCalibRad[real_cam] = (sqrt(pow(pixel[0] - pixel[2], 2) + pow(pixel[1] - pixel[3], 2)) /
				((m_pMain->GetMachine(m_nJobID).getCameraResolutionX(nCam, nPos) + m_pMain->GetMachine(m_nJobID).getCameraResolutionY(nCam, nPos)) / 2.)) / 2.;
			//m_pMain->GetMachine(m_nJobID).WorldToPixel(nCam, pos, xr, yr, &pixel[0], &pixel[1]);

			pixel[0] = rotateCenter.x/ m_pMain->GetMachine(m_nJobID).getCameraResolutionX(nCam, nPos) + m_pMain->m_stCamInfo[real_cam].w / 2.;
			pixel[1] = rotateCenter.y / m_pMain->GetMachine(m_nJobID).getCameraResolutionY(nCam, nPos) + m_pMain->m_stCamInfo[real_cam].h / 2.;

			((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_dbCalibRotateX[real_cam] = pixel[0];
			((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_dbCalibRotateY[real_cam] = pixel[1];

			::SendMessageA(m_pMain->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_DISPLAY_MACHINE_VIEW, 0);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}

		nRet = 1;

	}
	break;
	}

#endif

	updateResultDialog();

	return nRet;
}

int TabMachineAlgoPage::seqCalculateRotateCenter2()
{
	int nMethod = m_pMain->vt_job_info[m_nJobID].algo_method;

	if (m_pMain->each_job_method)
	{
		if (nMethod == CLIENT_TYPE_1CAM_1SHOT_ALIGN || nMethod == CLIENT_TYPE_1CAM_1SHOT_FILM || nMethod == CLIENT_TYPE_1CAM_1SHOT_ALIGN_WITH_SCRATCH_INSP)	return g_Job_1Cam1ShotAlign[m_nJobID].do_calibation_seq();
	}

	CString str;
	BOOL bFind = TRUE;
	int nRet = 0, address, address2, address3, real_cam, W, H;
	double dbData[3] = { 0, 0, 0 };
	double dbStepT = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getRotateRange();

	int dbMultiOffset = 0;
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();

	// 왼쪽 반시계 방향으로 회전 기준 //220908 Tkyuha 임의의 지점으로 회전 중심 계산 하기 위함
	// dbStartAngle 이 dbEndAngle 보다 작은 값 이어야함
	int  iCalibType = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getCalibrationType();
	double dbStartAngle = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getRotateStartPos();
	double dbEndAngle = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getRotateEndPos();

	// TKyuha 211115 한카메라 에서2개 얼라인 처리
	dbMultiOffset = nPos * 20;
	if (nMethod != CLIENT_TYPE_1CAM_2POS_REFERENCE && nMethod != CLIENT_TYPE_1CAM_4POS_ROBOT)  dbMultiOffset = 0; // KBJ 2022-08-22 수정

	int num_of_camera = m_pMain->vt_job_info[m_nJobID].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[m_nJobID].camera_index;

#ifndef NO_MIL
	switch (m_nSeqRotateCenter) {
	case 0: KillTimer(TIMER_ROTATE_CENTER2); break;
	case 100:		// Calibration 가능 상태 확인
	{
		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.read_bit_start + dbMultiOffset + 5;

		BOOL enable = g_CommPLC.GetBit(address);

		if (enable == FALSE)
		{
			str.Format("PLC Calib Enable Signal (L%d) Off!!", address);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			nRet = 2;
			break;
		}
		else
		{
			str.Format("PLC Calib Enable Signal (L%d) On!!", address);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}

		m_nSeqRotateCenter = 200;
	}
	break;
	case 200:		// Calibration Mode ON
	{
		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start + dbMultiOffset + 6;

		g_CommPLC.SetBit(address, TRUE);
		
		str.Format("PC → PLC Calib Mode On (M%d)!!", address);
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_nTimeOutCount = 0;
		m_nSeqRotateCenter = 300;
	}
	break;
	case 300:
	{
		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.read_bit_start + dbMultiOffset + 6;
		
		BOOL mode_on_ack = g_CommPLC.GetBit(address);

		if (m_nTimeOutCount % 100 == 0)
		{
			str.Format("Wait PLC → PC Calib Mode On Ack (M%d)!!", address);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}

		if (m_nTimeOutCount++ > 1000)
		{
			str.Format("Timeout PLC → PC Calib Mode On Ack (M%d)!!", address);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

			nRet = 2;
			m_nSeqRotateCenter = 0;
			break;
		}

		if ( mode_on_ack == TRUE)
		{
			m_nTimeOutCount = 0;
			m_nSeqRotateCenter = 1000;
		}
	}
	break;
	case 1000:		// 이동값 전송
	{
		if (iCalibType == 2) dbData[AXIS_T] = dbStartAngle;
		else dbData[AXIS_T] = dbStepT;	// 나머지 얼라인은 현재 위치 기준 보정		

		for (int nCam = 0; nCam < num_of_camera; nCam++)
		{
			real_cam = camBuf.at(nCam);
			m_dbMotorT[real_cam][m_nRotateCnt] = dbStepT;
		}
	
		str.Format("T: %.1f", dbData[AXIS_T]);
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_pMain->m_nCalibRevisionAck = 0;
		m_pMain->m_dbRevisionData[m_nJobID][AXIS_X] = 0;
		m_pMain->m_dbRevisionData[m_nJobID][AXIS_Y] = 0;
		m_pMain->m_dbRevisionData[m_nJobID][AXIS_T] = dbData[AXIS_T];

		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start + dbMultiOffset + 7;
		address3 = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_word_start + dbMultiOffset;

		long lRevisionData[6] = { 0, };
		lRevisionData[0] = LOWORD(dbData[AXIS_X] * MOTOR_SCALE);
		lRevisionData[1] = HIWORD(dbData[AXIS_X] * MOTOR_SCALE);
		lRevisionData[2] = LOWORD(dbData[AXIS_Y] * MOTOR_SCALE);
		lRevisionData[3] = HIWORD(dbData[AXIS_Y] * MOTOR_SCALE);
		lRevisionData[4] = LOWORD(dbData[AXIS_T] * MOTOR_SCALE);
		lRevisionData[5] = HIWORD(dbData[AXIS_T] * MOTOR_SCALE);

		g_CommPLC.SetWord(address3, 6, lRevisionData);
		g_CommPLC.SetBit(address, TRUE);

		str.Format("PC → PLC Send Calib Data(D%d)", address3);
		::SendMessageA(this->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		str.Format("PC → PLC Revision Pos Move Signal(M%d) Req", address);
		::SendMessageA(this->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_nTimeOutCount = 0;
		m_nSeqRotateCenter = 1100;
	}
	break;
	case 1100:		// 보정 위치 이동 완료 확인
	{
		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start + dbMultiOffset + 7;
		address2 = m_pMain->vt_job_info[m_nJobID].plc_addr_info.read_bit_start + dbMultiOffset + 7;
		
		BOOL revision_ack = g_CommPLC.GetBit(address2);

		if (m_nTimeOutCount % 100 == 0)
		{
			str.Format("Wait PLC → PC Rotation Center Move(L%d) Complete", address2);
			SendMessageA(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}
		
		if (m_nTimeOutCount++ > 1000 )
		{
			str.Format("Timeout PLC → PC Rotation Center Move(L%d) Complete", address2);
			SendMessageA(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

			nRet = 2;
			m_nTimeOutCount = 0;
			m_nSeqRotateCenter = 0;
			g_CommPLC.SetBit(address, FALSE);
			break;
		}

		if (revision_ack == TRUE)
		{
			str.Format("PLC → PC Revision Pos Move(L%d) Complete", address2);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_nSeqRotateCenter = 1110;
			g_CommPLC.SetBit(address, FALSE);
		}
	}
	break;
	case 1110:
	case 1120:
	case 1130:
	case 1140:
	case 1150:
	case 1160:
	case 1170:
	case 1180:
	case 1190:
		m_nSeqRotateCenter += 10;
		break;
	case 1200:		// Grab 요청
	{
		for (int nCam = 0; nCam < num_of_camera; nCam++)
		{
			real_cam = camBuf.at(nCam);
			auto& cam = m_pMain->m_Cameras[real_cam];
			if (cam && cam->IsConnected())
			{
				AcquireSRWLockExclusive(&cam->g_bufsrwlock);
				m_pMain->m_bGrabEnd[real_cam] = FALSE;
				ReleaseSRWLockExclusive(&cam->g_bufsrwlock);
			}
			else m_pMain->m_bGrabEnd[real_cam] = FALSE;
			if (cam && cam->IsConnected())
				m_pMain->SendMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_SINGLE_FRAME, real_cam);
		}
		
		m_nSeqRotateCenter = 1300;
	}
	break;
	case 1300:		// Grab 완료 대기
	{
		BOOL bGrabEnd = TRUE;

		for (int nCam = 0; nCam < num_of_camera; nCam++)
		{
			real_cam = camBuf.at(nCam);
			if (m_pMain->m_bGrabEnd[real_cam] != TRUE)
				bGrabEnd = FALSE;
		}

		if (bGrabEnd != TRUE) break;

		m_nSeqRotateCenter = 1400;
	}
	break;
	case 1400:		// Find Pattern
	{
		BOOL bFind2 = TRUE;
		CFindInfo *pInfo;

		for (int nCam = 0; nCam < num_of_camera; nCam++)
		{
			real_cam = camBuf.at(nCam);
			W = m_pMain->m_stCamInfo[real_cam].w;
			H = m_pMain->m_stCamInfo[real_cam].h;

			pInfo = &m_pMain->GetMatching(m_nJobID).getFindInfo(nCam, nPos);
			m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 0), m_pMain->getSrcBuffer(real_cam), W * H);

			//KJH 2022-01-07 Rotate Auto Calc (Caliper -> Circle)
			if (m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getMarkFindMethod() == METHOD_CIRCLE)
			{
				BYTE* pImg = m_pMain->getProcBuffer(real_cam, 0);
				find_pattern_caliper(pImg, W, H, m_nJobID, nCam, nPos);
			}
			else 
			{
#ifdef _USE_CALIB_MARK
			UseClibrationMarkMatching((BYTE*)m_pMain->getProcBuffer(real_cam, 0), m_nJobID, nCam, nPos, W, H);
#else
			m_pMain->GetMatching(m_nJobID).findPattern((BYTE*)m_pMain->getProcBuffer(real_cam, 0), nCam, nPos, W, H);
#endif
			}

			if (pInfo->GetFound() == FIND_ERR)
				bFind2 = FALSE;

			str.Format("Cam %d) %.1f%% %s", nCam + 1, pInfo->getScore(), pInfo->GetFound() == FIND_OK ? "OK" : "NG");
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}

		::SendMessageA(m_pMain->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_DISPLAY_MACHINE_VIEW, 0);

		if (bFind2 != TRUE)
		{
			nRet = 2;
			break;
		}

		for (int nCam = 0; nCam < num_of_camera; nCam++)
		{
			real_cam = camBuf.at(nCam);

			pInfo = &m_pMain->GetMatching(m_nJobID).getFindInfo(nCam, nPos);
			double posX = pInfo->GetXPos();
			double posY = pInfo->GetYPos();

			m_listPtOnCircle[nCam].push_back(cv::Point2d(posX, posY));
			// 왜 pos + 1인지 모르겠음. -_- 일단 주석 21.01.25
/*
			m_pMain->GetMatching().getFindInfo(cam, nPos + 1).SetXPos(posX);
			m_pMain->GetMatching().getFindInfo(cam, nPos + 1).SetYPos(posY);
			m_pMain->GetMatching().getFindInfo(cam, nPos + 1).SetScore(100.0);
			m_pMain->GetMatching().getFindInfo(cam, nPos + 1).SetFound(FIND_OK);*/

			m_pMain->GetMachine(m_nJobID).setImageOrgX(nCam, nPos, m_nRotateCnt, posX);
			m_pMain->GetMachine(m_nJobID).setImageOrgY(nCam, nPos, m_nRotateCnt, posY);
			m_pMain->GetMachine(m_nJobID).PixelToWorld(nCam, nPos, posX, posY, &posX, &posY);

			m_pMain->GetMachine(m_nJobID).setImageX(nCam, nPos, m_nRotateCnt, posX);
			m_pMain->GetMachine(m_nJobID).setImageY(nCam, nPos, m_nRotateCnt, posY);

			str.Format("Cam %d : (%.1f %.1f) (%.3f, %.3f)", nCam + 1, pInfo->GetXPos(), pInfo->GetYPos(), posX, posY);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}

		m_nRotateCnt++;

		if (m_pMain->m_bUseRAlignFlag) 
		{
			m_nSeqRotateCenter = 1700;		//210131 Bending RAlingn 동작 안되서 바꿈
		}
		else
		{
			m_nSeqRotateCenter = 1600;
		}
	}
	break;
	case 1600:		// 원점에서 반대 방향으로 회전
	{
		// 절대 좌표
		// dbData[AXIS_T] = -dbStepT;

		if (iCalibType == 2) dbData[AXIS_T] = -dbStartAngle;
		else dbData[AXIS_T] = -dbStepT;		// 상대 좌표
		

		for (int nCam = 0; nCam < num_of_camera; nCam++)
		{
			real_cam = camBuf.at(nCam);
			m_dbMotorT[real_cam][m_nRotateCnt] = dbData[AXIS_T];
		}

		str.Format("T : %.1f", dbData[AXIS_T]);
		SendMessageA(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start + dbMultiOffset + 7;
		address3 = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_word_start + dbMultiOffset;

		long lRevisionData[6] = { 0, };
		lRevisionData[0] = LOWORD(dbData[AXIS_X] * MOTOR_SCALE);
		lRevisionData[1] = HIWORD(dbData[AXIS_X] * MOTOR_SCALE);
		lRevisionData[2] = LOWORD(dbData[AXIS_Y] * MOTOR_SCALE);
		lRevisionData[3] = HIWORD(dbData[AXIS_Y] * MOTOR_SCALE);
		lRevisionData[4] = LOWORD(dbData[AXIS_T] * MOTOR_SCALE);
		lRevisionData[5] = HIWORD(dbData[AXIS_T] * MOTOR_SCALE);

		g_CommPLC.SetWord(address3, 6, lRevisionData);
		g_CommPLC.SetBit(address, TRUE);

		str.Format("PC → PLC Send Calib Data(D%d)", address3);
		::SendMessageA(this->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		str.Format("PC → PLC Revision Pos Move Signal(M%d) Req", address);
		::SendMessageA(this->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_nTimeOutCount = 0;
		m_nSeqRotateCenter = 1610;
	}
	break;
	case 1610:		// 보정 위치 이동 완료 확인
	{
		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start + dbMultiOffset + 7;
		address2 = m_pMain->vt_job_info[m_nJobID].plc_addr_info.read_bit_start + dbMultiOffset + 7;

		BOOL revision_ack = g_CommPLC.GetBit(address2);

		if (m_nTimeOutCount % 100 == 0)
		{
			str.Format("Wait PLC → PC Rotation Center Move(L%d) Complete", address2);
			SendMessageA(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}

		if (m_nTimeOutCount++ > 1000)
		{
			str.Format("Timeout PLC → PC Rotation Center Move(L%d) Complete", address2);
			SendMessageA(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

			nRet = 2;
			m_nTimeOutCount = 0;
			m_nSeqRotateCenter = 0;
			g_CommPLC.SetBit(address, FALSE);
			break;
		}

		if (revision_ack == TRUE)
		{
			str.Format("PLC → PC Revision Pos Move(L%d) Complete", address2);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_nSeqRotateCenter = 1700;
			g_CommPLC.SetBit(address, FALSE);
		}
	}
	break;
	case 1700:		// 원점에서 반대 방향으로 회전
	{
		if(m_pMain->m_bUseRAlignFlag)
		{
			// 절대 좌표
			//dbData[AXIS_T] = -dbStepT;
			// 상대 좌표
			dbData[AXIS_T] = -dbStepT * 2;
		}
		else
		{
			// 절대 좌표
			
			// 상대 좌표
			//dbData[AXIS_T] = -dbStepT * 2;
			if (iCalibType == 2) dbData[AXIS_T] = -(360.0 - dbEndAngle);
			else dbData[AXIS_T] = -dbStepT;
		}		

		for (int nCam = 0; nCam < num_of_camera; nCam++)
		{
			real_cam = camBuf.at(nCam);
			m_dbMotorT[real_cam][m_nRotateCnt] = dbData[AXIS_T];
		}
		
		str.Format("T : %.1f", dbData[AXIS_T]);
		SendMessageA(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_pMain->m_dbRevisionData[m_nJobID][AXIS_X] = 0;
		m_pMain->m_dbRevisionData[m_nJobID][AXIS_Y] = 0;
		m_pMain->m_dbRevisionData[m_nJobID][AXIS_T] = dbData[AXIS_T];

		long lRevisionData[6] = { 0, };
		lRevisionData[0] = LOWORD(dbData[AXIS_X] * MOTOR_SCALE);
		lRevisionData[1] = HIWORD(dbData[AXIS_X] * MOTOR_SCALE);
		lRevisionData[2] = LOWORD(dbData[AXIS_Y] * MOTOR_SCALE);
		lRevisionData[3] = HIWORD(dbData[AXIS_Y] * MOTOR_SCALE);
		lRevisionData[4] = LOWORD(dbData[AXIS_T] * MOTOR_SCALE);
		lRevisionData[5] = HIWORD(dbData[AXIS_T] * MOTOR_SCALE);
		
		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start + dbMultiOffset + 7;
		address3 = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_word_start + dbMultiOffset;

		g_CommPLC.SetWord(address3, 6, lRevisionData);
		g_CommPLC.SetBit(address, TRUE);

		str.Format("PC → PLC Send Calib Data(D%d)", address3);
		::SendMessageA(this->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		str.Format("PC → PLC Revision Pos Move Signal(M%d) Req", address);
		::SendMessageA(this->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_nTimeOutCount = 0;
		m_nSeqRotateCenter = 1710;
	}
	break;
	case 1710:		// 보정 위치 이동 완료 확인
	{
		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start + dbMultiOffset + 7;
		address2 = m_pMain->vt_job_info[m_nJobID].plc_addr_info.read_bit_start + dbMultiOffset + 7;

		BOOL revision_ack = g_CommPLC.GetBit(address2);

		if (m_nTimeOutCount % 100 == 0)
		{
			str.Format("Wait PLC → PC Rotation Center Move(L%d) Complete", address2);
			SendMessageA(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}

		if (m_nTimeOutCount++ > 1000)
		{
			str.Format("Timeout PLC → PC Rotation Center Move(L%d) Complete", address2);
			SendMessageA(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

			nRet = 2;
			m_nTimeOutCount = 0;
			m_nSeqRotateCenter = 0;
			g_CommPLC.SetBit(address, FALSE);
			break;
		}

		if (revision_ack == TRUE)
		{
			str.Format("PLC → PC Revision Pos Move(L%d) Complete", address2);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_nSeqRotateCenter = 1720;
			g_CommPLC.SetBit(address, FALSE);
		}
	}
	break;
	case 1720:
	case 1730:
	case 1740:
	case 1750:
	case 1760:
	case 1770:
	case 1780:
	case 1790:
		m_nSeqRotateCenter += 10;
		break;
	case 1800:	// Grab 
	{
		for (int nCam = 0; nCam < num_of_camera; nCam++)
		{
			real_cam = camBuf.at(nCam);
			auto& cam = m_pMain->m_Cameras[real_cam];
			if (cam && cam->IsConnected())
			{
				AcquireSRWLockExclusive(&cam->g_bufsrwlock);
				m_pMain->m_bGrabEnd[real_cam] = FALSE;
				ReleaseSRWLockExclusive(&cam->g_bufsrwlock);
			}
			else m_pMain->m_bGrabEnd[real_cam] = FALSE;
			if (cam && cam->IsConnected())
				m_pMain->SendMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_SINGLE_FRAME, real_cam);
		}

		str = "Grab Start";
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		Sleep(100);

		m_nTimeOutCount = 0;
		m_nSeqRotateCenter = 1900;
	}
	break;
	case 1900:
	{
		BOOL bGrabEnd = TRUE;

		for (int nCam = 0; nCam < num_of_camera; nCam++)
		{
			real_cam = camBuf.at(nCam);
			if (m_pMain->m_bGrabEnd[real_cam] != TRUE)
				bGrabEnd = FALSE;
		}

		if (bGrabEnd != TRUE) break;

		m_nSeqRotateCenter = 1950;
	}
	break;
	case 1950:		// Find Pattern
	{
		//int nPos = 0; // 20211020 Tkyuha 개별 켈리브레이션을 수행하기 위해 삭제
		CFindInfo *pInfo;
		BOOL bFind2 = TRUE;

		for (int nCam = 0; nCam < num_of_camera; nCam++)
		{
			real_cam = camBuf.at(nCam);
			W = m_pMain->m_stCamInfo[real_cam].w;
			H = m_pMain->m_stCamInfo[real_cam].h;

			pInfo = &m_pMain->GetMatching(m_nJobID).getFindInfo(nCam, nPos);
			m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 0), m_pMain->getSrcBuffer(real_cam), W * H);
		//	m_pMain->GetMatching(m_nJobID).findPattern((BYTE *)m_pMain->getProcBuffer(real_cam, 0), nCam, nPos, W, H);
			
			//KJH 2022-01-07 Rotate Auto Calc (Caliper -> Circle)
			if (m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getMarkFindMethod() == METHOD_CIRCLE)
			{
				BYTE* pImg = m_pMain->getProcBuffer(real_cam, 0);
				find_pattern_caliper(pImg, W, H, m_nJobID, nCam, nPos);
			}
			else
			{
#ifdef _USE_CALIB_MARK
			UseClibrationMarkMatching((BYTE*)m_pMain->getProcBuffer(real_cam, 0), m_nJobID, nCam, nPos, W, H);
#else
			m_pMain->GetMatching(m_nJobID).findPattern((BYTE*)m_pMain->getProcBuffer(real_cam, 0), nCam, nPos, W, H);
#endif
			}

			if (pInfo->GetFound() == FIND_ERR)				bFind2 = FALSE;

			str.Format("Cam %d) %.1f%% %s", nCam + 1, pInfo->getScore(), pInfo->GetFound() == FIND_OK ? "OK" : "NG");
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}

		::SendMessageA(m_pMain->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_DISPLAY_MACHINE_VIEW, 0);
		if (bFind2 != TRUE)
		{
			nRet = 2;
			break;
		}

		for (int nCam = 0; nCam < num_of_camera; nCam++)
		{
			real_cam = camBuf.at(nCam);

			pInfo = &m_pMain->GetMatching(m_nJobID).getFindInfo(nCam, nPos);
			double posX = pInfo->GetXPos();
			double posY = pInfo->GetYPos();
			m_listPtOnCircle[nCam].push_back(cv::Point2d(posX, posY));

			m_pMain->GetMachine(m_nJobID).setImageOrgX(nCam, nPos, m_nRotateCnt, posX);
			m_pMain->GetMachine(m_nJobID).setImageOrgY(nCam, nPos, m_nRotateCnt, posY);
			m_pMain->GetMachine(m_nJobID).PixelToWorld(nCam, nPos, posX, posY, &posX, &posY);

			m_pMain->GetMachine(m_nJobID).setImageX(nCam, nPos, m_nRotateCnt, posX);
			m_pMain->GetMachine(m_nJobID).setImageY(nCam, nPos, m_nRotateCnt, posY);

			str.Format("Cam %d : (%.1f %.1f) (%.3f, %.3f)", nCam + 1, pInfo->GetXPos(), pInfo->GetYPos(), posX, posY);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}		

		m_nSeqRotateCenter = 2000;
	}
	break;
	case 2000:	// 9. Rotate Center 계산
	{
		BOOL bSuccess = TRUE;
		// 샘플링 얼라인은 1캠으로 2개의 스테이지에 대한 회전 중심값을 저장 해야 되므로
		// 선택한 스테이지를 참고하여 저장.

		m_pMain->GetMachine(m_nJobID).setMotorT(dbStepT * 2);
		/*int nPos = 0;*/

		for (int nCam = 0; nCam < num_of_camera; nCam++)
		{
			real_cam = camBuf.at(nCam);
			bSuccess = m_pMain->GetMachine(m_nJobID).halcon_GetTwoPointTheta_Circle(nCam, nPos);

			if (bSuccess != TRUE)
			{
				nRet = 2;
				break;
			}

			double xr = m_pMain->GetMachine(m_nJobID).getRotateX(nCam, nPos);
			double yr = m_pMain->GetMachine(m_nJobID).getRotateY(nCam, nPos);

			m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().setRotateCenterX(nCam, nPos, xr);
			m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().setRotateCenterY(nCam, nPos, yr);

			// 20211202 Tkyuha Calibration 수행 일자 저장
			CTime time = CTime::GetCurrentTime();
			CString strTime = _T("");
			strTime.Format(_T("%04d/%02d/%02d  %02d:%02d:%02d"), time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute(), time.GetSecond());
			m_pMain->GetMachine(m_nJobID).setMakeRotateDateTime(nCam, nPos, strTime);

			//m_pMain->GetMachine(m_nJobID).saveRotateCenter(m_pMain->m_strCurrentModelPath, nCam, nPos);
			CString str_path;
			str_path.Format("%s%s\\", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[m_nJobID].job_name.c_str());
			m_pMain->GetMachine(m_nJobID).saveRotateCenter(str_path, nCam, nPos);
			str.Format("Cam %d - M(%.2f)) dbRotateX = %.2f, dbRotateY = %.2f", real_cam + 1, dbStepT,xr, yr);

			// 회전 중심 전송하는 부분은 일단 생략 하기로.. 21.01.05
			/*
			long sendRotateCenter[4] = { 0, };
			sendRotateCenter[0] = LOWORD((long)(xr * MOTOR_SCALE));
			sendRotateCenter[1] = HIWORD((long)(xr * MOTOR_SCALE));
			sendRotateCenter[2] = LOWORD((long)(yr * MOTOR_SCALE));
			sendRotateCenter[3] = HIWORD((long)(yr * MOTOR_SCALE));

			address2 = m_pMain->m_pAlgorithmInfo.getAlgorithmWriteWordStart(m_nTabAlgoIndex);
			m_pMain->SendServer(TCP_SEND_ROTATE_CENTER, address2 + 6, 0, sendRotateCenter);
*/

			double pixel[4];
			int pos = 0;			
/*
			pixel[0] = m_pMain->GetMachine(m_nJobID).getImageX(nCam, nPos, 1);
			pixel[1] = m_pMain->GetMachine(m_nJobID).getImageY(nCam, nPos, 1);
			pixel[2] = m_pMain->GetMachine(m_nJobID).getImageX(nCam, nPos, 0);
			pixel[3] = m_pMain->GetMachine(m_nJobID).getImageY(nCam, nPos, 0);

			((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_dbCalibRad[real_cam] = (sqrt(pow(pixel[0] - pixel[2], 2) + pow(pixel[1] - pixel[3], 2))/
			             ((m_pMain->GetMachine(m_nJobID).getCameraResolutionX(nCam, nPos)+ m_pMain->GetMachine(m_nJobID).getCameraResolutionY(nCam, nPos))/2.))/2.;
			pixel[0]= xr/ m_pMain->GetMachine(m_nJobID).getCameraResolutionX(nCam, nPos) + m_pMain->m_stCamInfo[real_cam].w /2.;
			pixel[1]= yr/ m_pMain->GetMachine(m_nJobID).getCameraResolutionY(nCam, nPos) + m_pMain->m_stCamInfo[real_cam].h /2.;
			*/

			m_pMain->GetMachine(m_nJobID).WorldToPixel(nCam, pos, xr, yr, &pixel[0], &pixel[1]);

			pixel[2] = m_pMain->GetMachine(m_nJobID).getImageOrgX(nCam, nPos, 0);
			pixel[3] = m_pMain->GetMachine(m_nJobID).getImageOrgY(nCam, nPos, 0);

			((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_dbCalibRad[real_cam] = sqrt(pow(pixel[0] - pixel[2], 2) + pow(pixel[1] - pixel[3], 2));
			((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_dbCalibRotateX[real_cam] = pixel[0];
			((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_dbCalibRotateY[real_cam] = pixel[1];

			::SendMessageA(m_pMain->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_DISPLAY_MACHINE_VIEW, 0);
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}

		nRet = 1;

	}
	break;
	}

#endif

	updateResultDialog();

	return nRet;
}

int TabMachineAlgoPage::seqCalculateRotateCenter3()
{
	CString str;
	BOOL bFind = TRUE;
	int nRet = 0, address, address2, address3,  W, H;
	double dbData[3] = { 0, 0, 0 };
	double dbStepT = m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getRotateRange();

#ifndef JOB_INFO
	int camCount = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraNum(m_nTabAlgoIndex);
	std::vector<int> camBuf = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(m_nTabAlgoIndex);
#else
	int camCount = m_pMain->vt_job_info[m_nJobID].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[m_nJobID].camera_index;
#endif
	int nCam = ((CComboBox *)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->GetCurSel();
	int nSelPos = ((CComboBox *)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();
	int nPos = nSelPos;

	int nCalibCam = camBuf.at(nCam);

#ifndef NO_MIL
	switch (m_nSeqRotateCenter) {
	case 0: KillTimer(TIMER_ROTATE_CENTER3); break;
	case 100:		// Calibration 가능 상태 확인
	{
#ifndef JOB_INFO
		address = m_pMain->m_pAlgorithmInfo.getAlgorithmReadBitStart(m_nTabAlgoIndex);
#else
		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.read_bit_start;
#endif
		m_pMain->SendServer(TCP_SEND_CALIBRATION_REQUEST, address + 5);
		m_nSeqRotateCenter = 200;

		str = "Calibration Request";
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
	break;
	case 200:		// Calibration Mode ON
	{
		if (m_pMain->m_nCalibrationAck == 0)	break;
		else if (m_pMain->m_nCalibrationAck == 1)
		{
			str = "Recv Rotate center Ack";
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_nSeqRotateCenter = 1000;
		}
		else if (m_pMain->m_nCalibrationAck == 2)
		{
			str = "PLC Enable Signal Off";
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_nSeqRotateCenter = 0;
			nRet = 2;
		}
	}
	break;
	case 1000:		// 이동값 전송
	{
		dbData[AXIS_T] = dbStepT;	// 나머지 얼라인은 현재 위치 기준 보정
		//dbData[AXIS_T] = -dbStepT;	// 나머지 얼라인은 현재 위치 기준 보정

		//m_dbMotorT[m_nCalibCam][m_nRotateCnt] = dbStepT;
		m_dbMotorT[nCalibCam][m_nRotateCnt] = dbData[AXIS_T];

		str.Format("1 - T : %.1f", dbData[AXIS_T]);
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_pMain->m_nCalibRevisionAck = 0;
		m_pMain->m_dbRevisionData[m_nJobID][AXIS_X] = 0;
		m_pMain->m_dbRevisionData[m_nJobID][AXIS_Y] = 0;
		m_pMain->m_dbRevisionData[m_nJobID][AXIS_T] = dbData[AXIS_T];

#ifndef JOB_INFO
		address = m_pMain->m_pAlgorithmInfo.getAlgorithmWriteBitStart(m_nTabAlgoIndex);
		address2 = m_pMain->m_pAlgorithmInfo.getAlgorithmReadBitStart(m_nTabAlgoIndex);
		address3 = m_pMain->m_pAlgorithmInfo.getAlgorithmWriteWordStart(m_nTabAlgoIndex);
#else
		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;
		address2 = m_pMain->vt_job_info[m_nJobID].plc_addr_info.read_bit_start;
		address3 = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;
#endif
		m_pMain->SendServer(TCP_SEND_CSCAM_CALIB_DATA, address, address3, &address2);

		str = "PC → PLC Rotate Center Move Reqeust";
		SendMessageA(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_nSeqRotateCenter = 1100;
	}
	break;
	case 1100:		// 보정 위치 이동 완료 확인
	{
		if (m_pMain->m_nCalibRevisionAck == 0)	break;

		str = "PLC → PC Rotate Center Move Complete";
		SendMessageA(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		if (m_pMain->m_nCalibRevisionAck == 1)
		{
			str = "PLC → PC Revision Pos Move Complete";
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_nSeqRotateCenter = 1200;
		}
		else
		{
			str = "PLC → PC Revision Pos Move Failed";
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_nSeqRotateCenter = 0;
		}

		Sleep(1000);
	}
	break;
	case 1200:		// Grab 요청
	{
		auto& cam = m_pMain->m_Cameras[nCalibCam];
		if (cam && cam->IsConnected())
		{
			AcquireSRWLockExclusive(&cam->g_bufsrwlock);
			m_pMain->m_bGrabEnd[nCalibCam] = FALSE;
			ReleaseSRWLockExclusive(&cam->g_bufsrwlock);
		}
		else m_pMain->m_bGrabEnd[nCalibCam] = FALSE;

		if (cam && cam->IsConnected())
			m_pMain->SendMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_SINGLE_FRAME, nCalibCam);
		m_nSeqRotateCenter = 1300;
	}
	break;
	case 1300:		// Grab 완료 대기
	{
		BOOL bGrabEnd = TRUE;

		if (m_pMain->m_bGrabEnd[nCalibCam] != TRUE)
			bGrabEnd = FALSE;

		if (bGrabEnd != TRUE) break;

		m_nSeqRotateCenter = 1400;
	}
	break;
	case 1400:		// Find Pattern
	{
		BOOL bFind = TRUE;

		int nPos = 0;
		CFindInfo *pInfo;

		W = m_pMain->m_stCamInfo[nCalibCam].w;
		H = m_pMain->m_stCamInfo[nCalibCam].h;

		pInfo = &m_pMain->GetMatching(m_nJobID).getFindInfo(nCam, nPos);
		m_pMain->copyMemory(m_pMain->getProcBuffer(nCalibCam, 0), m_pMain->getSrcBuffer(nCalibCam), W * H);
#ifdef _USE_CALIB_MARK
		UseClibrationMarkMatching((BYTE*)m_pMain->getProcBuffer(nCalibCam, 0), m_nJobID, nCam, nPos, W, H);
#else
		m_pMain->GetMatching(m_nJobID).findPattern((BYTE*)m_pMain->getProcBuffer(nCalibCam, 0), nCam, nPos, W, H);
#endif
		if (pInfo->GetFound() == FIND_ERR)
			bFind = FALSE;

		str.Format("Cam %d) %.1f%% %s - %.3f", nCalibCam + 1, pInfo->getScore(), pInfo->GetFound() == FIND_OK ? "OK" : "NG", pInfo->GetAngle());
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		::SendMessageA(m_pMain->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_DISPLAY_MACHINE_VIEW, 0);

		if (bFind != TRUE)
		{
			nRet = 2;
			break;
		}

		pInfo = &m_pMain->GetMatching(m_nJobID).getFindInfo(nCam, nPos);
		double posX = pInfo->GetXPos();
		double posY = pInfo->GetYPos();

		m_pMain->GetMachine(m_nJobID).setImageOrgX(nCam, nPos, m_nRotateCnt, posX);
		m_pMain->GetMachine(m_nJobID).setImageOrgY(nCam, nPos, m_nRotateCnt, posY);
		m_pMain->GetMachine(m_nJobID).PixelToWorld(nCam, nPos, posX, posY, &posX, &posY);
		m_pMain->GetMachine(m_nJobID).setImageX(nCam, nPos, m_nRotateCnt, posX);
		m_pMain->GetMachine(m_nJobID).setImageY(nCam, nPos, m_nRotateCnt, posY);

		str.Format("Cam %d : (%.1f %.1f) (%.3f, %.3f)", nCalibCam + 1, pInfo->GetXPos(), pInfo->GetYPos(), posX, posY);
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_nRotateCnt++;
		m_nSeqRotateCenter = 1700;
	}
	break;
	case 1700:		// 원점에서 반대 방향으로 회전
	{
		// 절대 좌표
		// dbData[AXIS_T] = -dbStepT;

		// 상대 좌표
		dbData[AXIS_T] = -dbStepT * 2;
		//dbData[AXIS_T] = dbStepT * 2;

		m_dbMotorT[nCalibCam][m_nRotateCnt] = dbData[AXIS_T];

		str.Format("2 - T : %.1f", dbData[AXIS_T]);
		SendMessageA(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_pMain->m_dbRevisionData[m_nJobID][AXIS_X] = 0;
		m_pMain->m_dbRevisionData[m_nJobID][AXIS_Y] = 0;
		m_pMain->m_dbRevisionData[m_nJobID][AXIS_T] = dbData[AXIS_T];

#ifndef JOB_INFO
		address = m_pMain->m_pAlgorithmInfo.getAlgorithmWriteBitStart(m_nTabAlgoIndex);
		address2 = m_pMain->m_pAlgorithmInfo.getAlgorithmReadBitStart(m_nTabAlgoIndex);
		address3 = m_pMain->m_pAlgorithmInfo.getAlgorithmWriteWordStart(m_nTabAlgoIndex);
#else
		address = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;
		address2 = m_pMain->vt_job_info[m_nJobID].plc_addr_info.read_bit_start;
		address3 = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;
#endif
		m_pMain->SendServer(TCP_SEND_CSCAM_CALIB_DATA, address, address3, &address2);

		str = "PC → PLC Calibration Move Reqeust";
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		m_pMain->m_nCalibRevisionAck = 0;
		m_nSeqRotateCenter = 1710;
		break;
	}
	case 1710:		// 보정 위치 이동 완료 확인
	{
		if (m_pMain->m_nCalibRevisionAck == 0)	break;

		str = "PLC → PC Calibration Move Complete";
		SendMessageA(WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		if (m_pMain->m_nCalibRevisionAck == 1)
		{
			str = "PLC → PC Revision Pos Move Complete";
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_nSeqRotateCenter = 1800;
		}
		else
		{
			str = "PLC → PC Revision Pos Move Failed";
			OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			m_nSeqRotateCenter = 0;
		}

		Sleep(1000);
	}
	break;
	case 1800:	// Grab 
	{
		auto& cam = m_pMain->m_Cameras[nCalibCam];
		if (cam && cam->IsConnected())
		{
			AcquireSRWLockExclusive(&cam->g_bufsrwlock);
			m_pMain->m_bGrabEnd[nCalibCam] = FALSE;
			ReleaseSRWLockExclusive(&cam->g_bufsrwlock);
		}
		else m_pMain->m_bGrabEnd[nCalibCam] = FALSE;
		if (cam && cam->IsConnected())
			m_pMain->SendMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_SINGLE_FRAME, nCalibCam);

		str = "Grab Start";
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		Sleep(100);

		m_nTimeOutCount = 0;
		m_nSeqRotateCenter = 1900;
	}
	break;
	case 1900:
	{
		BOOL bGrabEnd = TRUE;

		if (m_pMain->m_bGrabEnd[nCalibCam] != TRUE)
			bGrabEnd = FALSE;

		if (bGrabEnd != TRUE) break;

		m_nSeqRotateCenter = 1950;
	}
	break;
	case 1950:		// Find Pattern
	{
		int nPos = 0;
		CFindInfo *pInfo;
		BOOL bFind2 = TRUE;

		W = m_pMain->m_stCamInfo[nCalibCam].w;
		H = m_pMain->m_stCamInfo[nCalibCam].h;

		pInfo = &m_pMain->GetMatching(m_nJobID).getFindInfo(nCam, nPos);
		m_pMain->copyMemory(m_pMain->getProcBuffer(nCalibCam, 0), m_pMain->getSrcBuffer(nCalibCam), W * H);
#ifdef _USE_CALIB_MARK
		UseClibrationMarkMatching((BYTE*)m_pMain->getProcBuffer(nCalibCam, 0), m_nJobID, nCam, nPos, W, H);
#else
		m_pMain->GetMatching(m_nJobID).findPattern((BYTE*)m_pMain->getProcBuffer(nCalibCam, 0), nCam, nPos, W, H);
#endif
		if (pInfo->GetFound() == FIND_ERR)				bFind2 = FALSE;

		str.Format("Cam %d) %.1f%% %s - %.3f", nCalibCam + 1, pInfo->getScore(), pInfo->GetFound() == FIND_OK ? "OK" : "NG", pInfo->GetAngle());
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		::SendMessageA(m_pMain->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_DISPLAY_MACHINE_VIEW, 0);
		if (bFind2 != TRUE)
		{
			nRet = 2;
			break;
		}

		pInfo = &m_pMain->GetMatching(m_nJobID).getFindInfo(nCam, nPos);
		double posX = pInfo->GetXPos();
		double posY = pInfo->GetYPos();
		m_pMain->GetMachine(m_nJobID).setImageOrgX(nCam, nPos, m_nRotateCnt, posX);
		m_pMain->GetMachine(m_nJobID).setImageOrgY(nCam, nPos, m_nRotateCnt, posY);
		m_pMain->GetMachine(m_nJobID).PixelToWorld(nCam, nPos, posX, posY, &posX, &posY);
		m_pMain->GetMachine(m_nJobID).setImageX(nCam, nPos, m_nRotateCnt, posX);
		m_pMain->GetMachine(m_nJobID).setImageY(nCam, nPos, m_nRotateCnt, posY);

		str.Format("Cam %d : (%.1f %.1f) (%.3f, %.3f)", nCalibCam + 1, pInfo->GetXPos(), pInfo->GetYPos(), posX, posY);
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_nSeqRotateCenter = 2000;
	}
	break;
	case 2000:	// 9. Rotate Center 계산
	{
		BOOL bSuccess = TRUE;
		// 샘플링 얼라인은 1캠으로 2개의 스테이지에 대한 회전 중심값을 저장 해야 되므로
		// 선택한 스테이지를 참고하여 저장.

		m_pMain->GetMachine(m_nJobID).setMotorT(dbStepT * 2);

		bSuccess = m_pMain->GetMachine(m_nJobID).halcon_GetTwoPointTheta_Circle(nCam, nPos);

		if (bSuccess != TRUE)
		{
			nRet = 2;
			break;
		}

		double xr = m_pMain->GetMachine(m_nJobID).getRotateX(nCam, nPos);
		double yr = m_pMain->GetMachine(m_nJobID).getRotateY(nCam, nPos);

		m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().setRotateCenterX(nCam, nPos, xr);
		m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().setRotateCenterY(nCam, nPos, yr);

		CString str_path;
		str_path.Format("%s%s\\", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[m_nJobID].job_name.c_str());
		m_pMain->GetMachine(m_nJobID).saveRotateCenter(str_path, nCam, nPos);
		str.Format("Cam %d) dbRotateX = %.2f, dbRotateY = %.2f", nCalibCam + 1, xr, yr);


		//PLC에 회전중심 전달부 추가 2019.07.31 KimJongHo

		long sendRotateCenter[4] = { 0, };
		sendRotateCenter[0] = LOWORD((long)(xr * MOTOR_SCALE));
		sendRotateCenter[1] = HIWORD((long)(xr * MOTOR_SCALE));
		sendRotateCenter[2] = LOWORD((long)(yr * MOTOR_SCALE));
		sendRotateCenter[3] = HIWORD((long)(yr * MOTOR_SCALE));

#ifndef JOB_INFO
		address2 = m_pMain->m_pAlgorithmInfo.getAlgorithmWriteWordStart(m_nTabAlgoIndex);
#else
		address2 = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_word_start;
#endif
		m_pMain->SendServer(TCP_SEND_ROTATE_CENTER, address2 + 6, 0, sendRotateCenter);


		double pixel[4];
		int pos = 0;
		pixel[0] = m_pMain->GetMachine(m_nJobID).getImageX(nCam, nPos, 1);
		pixel[1] = m_pMain->GetMachine(m_nJobID).getImageY(nCam, nPos, 1);
		pixel[2] = m_pMain->GetMachine(m_nJobID).getImageX(nCam, nPos, 0);
		pixel[3] = m_pMain->GetMachine(m_nJobID).getImageY(nCam, nPos, 0);

		((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_dbCalibRad[nCalibCam] = (sqrt(pow(pixel[0] - pixel[2], 2) + pow(pixel[1] - pixel[3], 2)) /
			((m_pMain->GetMachine(m_nJobID).getCameraResolutionX(nCalibCam, nPos) + m_pMain->GetMachine(m_nJobID).getCameraResolutionY(nCalibCam, nPos)) / 2.)) / 2.;
		//m_pMain->GetMachine(m_nJobID).WorldToPixel(nCam, pos, xr, yr, &pixel[0], &pixel[1]);

		pixel[0] = xr / m_pMain->GetMachine(m_nJobID).getCameraResolutionX(nCam, nPos) + m_pMain->m_stCamInfo[nCalibCam].w / 2.;
		pixel[1] = yr / m_pMain->GetMachine(m_nJobID).getCameraResolutionY(nCam, nPos) + m_pMain->m_stCamInfo[nCalibCam].h / 2.;

		((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_dbCalibRotateX[nCalibCam] = pixel[0];
		((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_dbCalibRotateY[nCalibCam] = pixel[1];

		::SendMessageA(m_pMain->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_DISPLAY_MACHINE_VIEW, 0);
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		nRet = 1;
	}
	break;
	}

#endif

	updateResultDialog();

	return nRet;
}

void TabMachineAlgoPage::OnEnSetfocusEditMoveX()
{
	setEditBox(IDC_EDIT_MOVE_X);
}

void TabMachineAlgoPage::OnEnSetfocusEditMoveY()
{
	setEditBox(IDC_EDIT_MOVE_Y);
}

void TabMachineAlgoPage::OnEnSetfocusEditMoveT()
{
	setEditBox(IDC_EDIT_MOVE_T);
}

void TabMachineAlgoPage::OnEnSetfocusEditSquareSize() 
{
	setEditBox(IDC_LB_SQUARE_SIZE);
}

void TabMachineAlgoPage::OnEnSetfocusEditXCount() 
{
	setEditBox(IDC_LB_CHESS_X2);
}

void TabMachineAlgoPage::OnEnSetfocusEditYCount() 
{
	setEditBox(IDC_LB_CHESS_Y2);
}

void TabMachineAlgoPage::OnEnSetfocusEditCameraResolution()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	setEditBox(IDC_EDIT_CAMERA_RESOLUTION);
}

void TabMachineAlgoPage::OnBnClickedBtnIndividualCalibration()
{
	CString strTemp;
#ifndef JOB_INFO	
	std::vector<int> camBuf = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(m_nTabAlgoIndex);
#else
	std::vector<int> camBuf = m_pMain->vt_job_info[m_nJobID].camera_index;
#endif

	int nSelCam = ((CComboBox *)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->GetCurSel();
	int nSelPos = ((CComboBox *)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();

	int nCam = camBuf.at(nSelCam);

	strTemp.Format("Cam %d Calibration?", nCam + 1);
	if (m_pMain->fnSetMessage(2, strTemp) != TRUE)	return;


	if (m_bMachineControlStart != TRUE)
	{
		CString str = "Calibration Sequence Entry";
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_nTimeOutCount = 0;
		m_bMachineControlStart = TRUE;
		m_nSeqCalibration = 100;


		int addr = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;
		g_CommPLC.SetBit(addr + 6, FALSE);
		g_CommPLC.SetBit(addr + 7, FALSE);

		cameraChangeTriggerMode();

		((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_ptrCalib[nCam].clear();
		((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_bDrawCalibration = TRUE;

		str = "Camera Trigger Mode";
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

		SetTimer(TIMER_CALIBRATION4, 100, NULL);

		str = "Calibration Sequence Start";
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
	else
	{
		m_pMain->fnSetMessage(1, "Already Machine Running..");
	}
}

void TabMachineAlgoPage::OnBnClickedBtnIndividualRotateCenter()
{
#ifndef JOB_INFO	
	std::vector<int> camBuf = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(m_nTabAlgoIndex);
#else
	std::vector<int> camBuf = m_pMain->vt_job_info[m_nJobID].camera_index;
#endif

	int nSelCam = ((CComboBox *)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->GetCurSel();
	int nSelPos = ((CComboBox *)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();

	int nCam = camBuf.at(nSelCam);

	CString strTemp;
	strTemp.Format("Cam %d Rotate Canter?", nCam + 1);
	if (m_pMain->fnSetMessage(2, strTemp) != TRUE)	return;

	CString str;
	if (m_bMachineControlStart != TRUE)
	{
		cameraChangeTriggerMode();

		((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_bDrawCalibration = TRUE;

		int addr = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;
		g_CommPLC.SetBit(addr + 6, FALSE);
		g_CommPLC.SetBit(addr + 7, FALSE);

		m_nRotateCnt = 0;
		m_bMachineControlStart = TRUE;
		m_nSeqRotateCenter = 100;
		SetTimer(TIMER_ROTATE_CENTER3, 100, NULL);

		CString str = "Rotate Center Sequence Start";
		OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
	else
		AfxMessageBox("Already Machine Running..");
}

void TabMachineAlgoPage::OnCbnSelchangeCbSelectMachinePosition()
{
	int index = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();
	::SendMessageA(m_pMain->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_SET_ALGORITHM_POS_POSITION, index);
	updateResultDialog();
}

void TabMachineAlgoPage::OnCbnSelchangeCbSelectMachineCamera()
{
	int index = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();
	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->GetCurSel();
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_ORG_POSITION))->SetCurSel(m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().getAlignOrginPos(nCam));

	::SendMessageA(m_pMain->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_SET_ALGORITHM_POS_CAMERA, nCam);
	updateResultDialog();

	if (m_pMain->m_pCamLightControlDlg)
	{
		m_pMain->m_pCamLightControlDlg->updateFrameDialog(m_nJobID, nCam);
	}
}

void TabMachineAlgoPage::OnCbnSelchangeCbSelectMachineOrgPosition()
{
	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->GetCurSel();
	int posalign = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_ORG_POSITION))->GetCurSel();

	m_pMain->vt_job_info[m_nJobID].model_info.getMachineInfo().setAlignOrginPos(nCam, posalign);
}

int TabMachineAlgoPage::getCurrentMachineCamera()
{
	int index = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->GetCurSel();
	return index;
}

int TabMachineAlgoPage::getCurrentMachinePosition()
{
	int index = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();
	return index;
}
//KJH 2021-12-31 Check
int TabMachineAlgoPage::getCurrentMachinePatternIndex()
{
	int index = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_PATTERN_INDEX))->GetCurSel();
	return index;
}

void TabMachineAlgoPage::OnBnClickedBtnImageCalibration()
{

	CString str;
	if (m_bMachineControlStart != TRUE)
	{
		m_bMachineControlStart = TRUE;
		m_nSeqImageCalibration = 100;

		SetTimer(TIMER_IMAGE_CALIBRATION, 1, NULL);
	}
	else
		AfxMessageBox("Already Machine Running..");	
}

void TabMachineAlgoPage::setEditBox(int nID)
{
	GetDlgItem(IDC_LB_CHESS_BOARD_TITLE)->SetFocus();

	CKeyPadDlg dlg;
	CString strNumber;
	GetDlgItem(nID)->GetWindowText(strNumber);
	dlg.SetValueString(false, strNumber);

	if (dlg.DoModal() != IDOK)	return;

	dlg.GetValue(strNumber);
	GetDlgItem(nID)->SetWindowTextA(strNumber);
}

void TabMachineAlgoPage::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	if (IsClickMoveTitle(&m_EditMoveX,point) == TRUE)
	{
		setEditBox(IDC_EDIT_MOVE_X);
	}
	else if (IsClickMoveTitle(&m_EditMoveY, point) == TRUE)
	{
		setEditBox(IDC_EDIT_MOVE_Y);
	}
	else if (IsClickMoveTitle(&m_EditMoveT, point) == TRUE)
	{
		setEditBox(IDC_EDIT_MOVE_T);
	}
	else if (IsClickMoveTitle(&m_LbChessBoardSize, point) == TRUE)
	{
		setEditBox(IDC_LB_SQUARE_SIZE);
	}
	else if (IsClickMoveTitle(&m_LbChessBoardX, point) == TRUE)
	{
		setEditBox(IDC_LB_CHESS_X2);
	}
	else if (IsClickMoveTitle(&m_LbChessBoardY, point) == TRUE)
	{
		setEditBox(IDC_LB_CHESS_Y2);
	}
	else if (IsClickWndTitle(&m_LbMoveTTitle, point) == TRUE) // 20221222 Tkyuha 각도 계산하기 위함
	{
		seqCalcCurrentAngle();
	}

	CDialogEx::OnLButtonUp(nFlags, point);
}

BOOL TabMachineAlgoPage::IsClickMoveTitle(CEditEx *Wnd,CPoint pt)
{
	CRect IdRect, rect;
	GetWindowRect(&rect);
	Wnd->GetWindowRect(&IdRect);

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;
	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;
	IdRect.bottom = IdRect.top + h;

	if (pt.x >= IdRect.left &&		pt.x <= IdRect.right &&
		pt.y >= IdRect.top &&		pt.y <= IdRect.bottom)
		return TRUE;
	else
		return FALSE;
}

BOOL TabMachineAlgoPage::IsClickWndTitle(CWnd* Wnd,CPoint pt)
{
	CRect IdRect, rect;
	GetWindowRect(&rect);
	Wnd->GetWindowRect(&IdRect);

	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;
	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;
	IdRect.bottom = IdRect.top + h;

	if (pt.x >= IdRect.left && pt.x <= IdRect.right && pt.y >= IdRect.top && pt.y <= IdRect.bottom)		return TRUE;
	else		return FALSE;
}

void TabMachineAlgoPage::OnBnClickedBtnManualRotate()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString str_temp;
	TCHAR szValue[MAX_PATH] = { 0, };
	double data_x = 0.0;
	double data_y = 0.0;
	double robot_x = 0.0;
	double robot_y = 0.0;

	int camCount = m_pMain->vt_job_info[m_nJobID].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[m_nJobID].camera_index;

	int nCamL = 0;
	int nCamR = 1;
	int nPos = 0;

	m_pMain->m_iniFile.GetProfileStringA("MANUAL_ROTATE_CENTER", "IMAGE_X1", "0.0", szValue, MAX_PATH);
	data_x = atof(szValue);
	m_pMain->m_iniFile.GetProfileStringA("MANUAL_ROTATE_CENTER", "IMAGE_Y1", "0.0", szValue, MAX_PATH);
	data_y = atof(szValue);

	// 3088		- 1544
	// 2064		- 1032
	m_pMain->GetMachine(m_nJobID).PixelToWorld(nCamL, nPos, data_x, data_y, &robot_x, &robot_y);
	m_pMain->GetMachine(m_nJobID).setImageX(nCamL, nPos, 0, robot_x);
	m_pMain->GetMachine(m_nJobID).setImageY(nCamL, nPos, 0, robot_y);


	m_pMain->m_iniFile.GetProfileStringA("MANUAL_ROTATE_CENTER", "IMAGE_X2", "0.0", szValue, MAX_PATH);
	data_x = atof(szValue);
	m_pMain->m_iniFile.GetProfileStringA("MANUAL_ROTATE_CENTER", "IMAGE_Y2", "0.0", szValue, MAX_PATH);
	data_y = atof(szValue);

	m_pMain->GetMachine(m_nJobID).PixelToWorld(nCamL, nPos, data_x, data_y, &robot_x, &robot_y);
	m_pMain->GetMachine(m_nJobID).setImageX(nCamL, nPos, 1, robot_x);
	m_pMain->GetMachine(m_nJobID).setImageY(nCamL, nPos, 1, robot_y);

	m_pMain->m_iniFile.GetProfileStringA("MANUAL_ROTATE_CENTER", "IMAGE_X3", "0.0", szValue, MAX_PATH);
	data_x = atof(szValue);
	m_pMain->m_iniFile.GetProfileStringA("MANUAL_ROTATE_CENTER", "IMAGE_Y3", "0.0", szValue, MAX_PATH);
	data_y = atof(szValue);

	m_pMain->GetMachine(m_nJobID).PixelToWorld(nCamR, nPos, data_x, data_y, &robot_x, &robot_y);
	m_pMain->GetMachine(m_nJobID).setImageX(nCamR, nPos, 0, robot_x);
	m_pMain->GetMachine(m_nJobID).setImageY(nCamR, nPos, 0, robot_y);

	m_pMain->m_iniFile.GetProfileStringA("MANUAL_ROTATE_CENTER", "IMAGE_X4", "0.0", szValue, MAX_PATH);
	data_x = atof(szValue);
	m_pMain->m_iniFile.GetProfileStringA("MANUAL_ROTATE_CENTER", "IMAGE_Y4", "0.0", szValue, MAX_PATH);
	data_y = atof(szValue);

	m_pMain->GetMachine(m_nJobID).PixelToWorld(nCamR, nPos, data_x, data_y, &robot_x, &robot_y);
	m_pMain->GetMachine(m_nJobID).setImageX(nCamR, nPos, 1, robot_x);
	m_pMain->GetMachine(m_nJobID).setImageY(nCamR, nPos, 1, robot_y);

	m_pMain->m_iniFile.GetProfileStringA("MANUAL_ROTATE_CENTER", "MOTOR_T", "0.0", szValue, MAX_PATH);
	m_pMain->GetMachine(m_nJobID).setMotorT(atof(szValue));

	for (int i = 0; i < camBuf.size(); i++)
	{
		int real_cam = camBuf[i];
		m_pMain->GetMachine(m_nJobID).halcon_GetTwoPointTheta_Circle(i, 0);
	}
}

void TabMachineAlgoPage::OnBnClickedBtnCalcRotate()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_pCramerDlg->IsWindowVisible())
	{
		m_pCramerDlg->ShowWindow(SW_HIDE);
	}
	else
	{
		m_pCramerDlg->ShowWindow(SW_SHOW);
	}


}

double TabMachineAlgoPage::GetStdev(std::vector<double> array)
{

	if (array.size() <= 0) return 0;

	double squareSum = 0;
	double average = 0;

	for (int i = 0; i < array.size(); i++)
	average += array[i];
	average /= array.size();

	for (int i = 0; i < array.size(); i++)
	{
		squareSum += pow(array[i] - average, 2);
	}

	double stdev = sqrt(squareSum / (array.size() - 1));
	return stdev;
}

//Calibration Data 정확성 계산
void TabMachineAlgoPage::CalcCalibrationResult()
{
	//std::vector<int> camBuf = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(m_nTabAlgoIndex);
	std::vector<int> camBuf = m_pMain->vt_job_info[m_nJobID].camera_index;

	int nSelCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->GetCurSel();
	int nCam = camBuf.at(nSelCam);

	std::vector<cv::Point2f> vtPoints, rtPoints;

	//vtPoints = m_pMain->GetMachine().getPtImage(nCam);
	//rtPoints = m_pMain->GetMachine().getPtMotor(nCam);
	int nSelPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();
	vtPoints = m_pMain->GetMachine(m_nJobID).getPtImage(nSelCam, nSelPos);
	rtPoints = m_pMain->GetMachine(m_nJobID).getPtMotor(nSelCam, nSelPos);

	double worldX, worldY, rX, rY;
	double diffX_Min = 9999, diffX_Max = -99999;
	double diffY_Min = 9999, diffY_Max = -99999;

	std::vector<double> diffX;
	std::vector<double> diffY;

	for (int i = 0; i < vtPoints.size(); i++)
	{
		//m_pMain->GetMachine().PixelToWorld(nCam, vtPoints.at(i).x, vtPoints.at(i).y, &worldX, &worldY);
		m_pMain->GetMachine(m_nJobID).PixelToWorld(nSelCam, nSelPos, vtPoints.at(i).x, vtPoints.at(i).y, &worldX, &worldY);
		rX = rtPoints[i].x - worldX;
		rY = rtPoints[i].y - worldY;

		diffX.push_back(rX);
		diffY.push_back(rY);

		if (diffX_Min >= rX) diffX_Min = rX;
		if (diffX_Max <= rX) diffX_Max = rX;
		if (diffY_Min >= rY) diffY_Min = rY;
		if (diffY_Max <= rY) diffY_Max = rY;
	}

	CString xyCalibration_X_Text, xyCalibration_Y_Text;

	xyCalibration_X_Text.Format("Diff_X : Min (%3.3f) Max (%3.3f) Sdv (%3.3f)", diffX_Min, diffX_Max, 3 * GetStdev(diffX));
	xyCalibration_Y_Text.Format("Diff_Y : Min (%3.3f) Max (%3.3f) Sdv (%3.3f)", diffY_Min, diffY_Max, 3 * GetStdev(diffY));

	OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&xyCalibration_X_Text);
	OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&xyCalibration_Y_Text);
}

void TabMachineAlgoPage::CheckReasonabilityRotate(int rotCount)
{
	std::vector<int> camBuf = m_pMain->vt_job_info[m_nJobID].camera_index;

	int nSelCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->GetCurSel();
	int nCam = camBuf.at(nSelCam);

	if (m_listPtOnCircle[nSelCam].size() < (rotCount - 1) * 2) return;

	std::vector <double> Length ;
	std::vector <double> Dist ;

	for (int i = 0; i < rotCount - 1; i++)
	{
		CPointF<double> p1(m_listPtOnCircle[nSelCam][i].x, m_listPtOnCircle[nSelCam][i].y);
		CPointF<double> p2(m_listPtOnCircle[nSelCam][i + 1].x, m_listPtOnCircle[nSelCam][i + 1].y);
		double L = GetDistance(p1, p2);
		Length.push_back(L);
	}

	double minV = *min_element(Length.begin(), Length.end());
	double maxV = *max_element(Length.begin(), Length.end());

	double diff = maxV - minV;
	CString exceptionText;

	if (diff > 10.0)
	{
		exceptionText = "Difference of Angle is too big";
	}
	else
	{
		for (int i = 0; i < rotCount; i++)
		{
			CPointF<double> p1(m_listPtOnCircle[nSelCam][i].x, m_listPtOnCircle[nSelCam][i].y);
			CPointF<double> p2(m_ptRotCenterRead.x, m_ptRotCenterRead.y);
			double L = GetDistance(p1, p2);
			Dist.push_back(L);
		}

		exceptionText.Format("StDev : %.4f", GetStdev(Dist));
	}

	OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&exceptionText);
}

#pragma region 회전중심으로 스테이지를 이동  Tkyuha 20211015
//  이버튼 사용후 회전 중심 티칭위치가 바뀌었기 때문에 Calibration 및 회전 중심을 다시 계산 해줄것
void TabMachineAlgoPage::OnBnClickedBtnMoveRotate()
{
	// KBJ 2022-02-25 usedPassWord
	if (m_pMain->UsePassword() == FALSE) return;

	CString str;
	str.Format("The rotation center position PLC setting value is changed.\n Do you want to Save?", m_pMain->getDispModelName());
	BOOL bRet = m_pMain->fnSetMessage(2, str,"Please use it with caution !!");
	if (bRet != TRUE) return;

	str = m_LbRotateX.GetText();
	m_pMain->m_dbRevisionData[m_nJobID][0] = atof(str) * -1;
	str = m_LbRotateY.GetText();
	m_pMain->m_dbRevisionData[m_nJobID][1] = atof(str) * -1;

	m_pMain->m_dbRevisionData[m_nJobID][2] = 0;

	int addr_write_bit = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start;
	int addr_read_bit = m_pMain->vt_job_info[m_nJobID].plc_addr_info.read_bit_start;
	int addr_write_word = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_word_start;


	long lRevisionData[4] = { 0, };
	lRevisionData[0] = LOWORD(m_pMain->m_dbRevisionData[m_nJobID][AXIS_X] * MOTOR_SCALE);
	lRevisionData[1] = HIWORD(m_pMain->m_dbRevisionData[m_nJobID][AXIS_X] * MOTOR_SCALE);
	lRevisionData[2] = LOWORD(m_pMain->m_dbRevisionData[m_nJobID][AXIS_Y] * MOTOR_SCALE);
	lRevisionData[3] = HIWORD(m_pMain->m_dbRevisionData[m_nJobID][AXIS_Y] * MOTOR_SCALE);

	g_CommPLC.SetWord(addr_write_word + 14, 4, lRevisionData);
	g_CommPLC.SetBit(addr_write_bit + 14, TRUE);

	str.Format("Request PC → PLC Mode On(L%d) Ack Signal....", addr_write_bit + 14);
	OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

	SetTimer(TIMER_WAIT_COMPLETE, 100, NULL);

	str.Format("Wait PLC → PC (L%d) Ack Signal....", addr_read_bit + 14);
	OnViewControl(MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);

	int nCam = 0;
	int nJob = m_nJobID;
	int viewer = m_pMain->vt_job_info[m_nJobID].machine_viewer_index[nCam];
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();

	double posX, posY, worldX, worldY;

	CViewerEx *pView = &(((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_pDlgViewerMachine[viewer]->GetViewer());
	pView->clearAllFigures();

	posX=pView->GetWidth()/2., posY = pView->GetHeight()/2.;

	m_pMain->GetMachine(nJob).PixelToWorld(nCam, nPos, posX, posY, &worldX, &worldY);
	
	struct stFigureText figure;
	figure.nOrgSize = 3;
	figure.nFitSize = 5;
	str.Format("[Cam %d] x:%.4f y:%.4f [Robot]  x:%.4f y:%.4f", nCam + 1, worldX, worldY, m_pMain->m_dbRevisionData[nJob][0], m_pMain->m_dbRevisionData[nJob][1]);
	figure.ptBeginFit = figure.ptBegin = CPoint(100, 100);
	figure.textString = str;
	pView->addFigureText(figure, 10, 10, COLOR_BLUE);

	m_pMain->GetMachine(nJob).WorldToPixel(nCam, nPos, m_pMain->m_dbRevisionData[nJob][0], m_pMain->m_dbRevisionData[nJob][1], &worldX, &worldY);
	str.Format("[Pixel] x:%.2f y:%.2f [Robot]  x:%.2f y:%.2f", worldX, worldY, pView->GetWidth() / 2., pView->GetHeight() / 2.);
	figure.ptBeginFit = figure.ptBegin = CPoint(100, 150);
	figure.textString = str;
	pView->addFigureText(figure, 10, 10, COLOR_BLUE);

	COLORREF color = COLOR_BLUE;
	stFigure tempFig;
	tempFig.ptBegin = CPoint(int(worldX-40), int(worldY));
	tempFig.ptEnd = CPoint(int(worldX+40), int(worldY));
	pView->addFigureLine(tempFig, 5, 3, color);

	tempFig.ptBegin = CPoint(int(worldX), int(worldY - 40));
	tempFig.ptEnd = CPoint(int(worldX ), int(worldY + 40));
	pView->addFigureLine(tempFig, 5, 3, color);

	pView->Invalidate();
}
#pragma endregion

BOOL TabMachineAlgoPage::find_pattern_caliper(BYTE *pImg, int w, int h, int job, int cam, int pos, CFindInfo* pInfo)
{
	int real_cam = m_pMain->vt_job_info[job].camera_index[cam];
	CCaliper* pCaliper = &m_pMain->m_pDlgCaliper->m_Caliper[real_cam][pos][0];

	if (pInfo == NULL)	pInfo = &m_pMain->GetMatching(job).getFindInfo(cam, pos);

	BOOL is_circle_caliper = pCaliper->getCircleCaliper();
	BOOL bFind = FALSE;

	if (is_circle_caliper)	
	{
		pCaliper->processCaliper_circle(pImg, w, h, 0.0, 0.0, 0.0, TRUE);
		bFind = pCaliper->getIsMakeLine();

		if (bFind)
		{
			pInfo->SetXPos(pCaliper->m_circle_info.x);
			pInfo->SetYPos(pCaliper->m_circle_info.y);
			pInfo->SetScore(100.0);
			pInfo->SetFound(FIND_OK);
		}
		else
		{
			pInfo->SetXPos(0.0);
			pInfo->SetYPos(0.0);
			pInfo->SetScore(0.0);
			pInfo->SetFound(FIND_ERR);
		}
	}
	else
	{
		m_pMain->m_pDlgCaliper->m_Caliper[real_cam][pos][0].processCaliper(pImg, w, h, 0.0, 0.0, 0.0, TRUE);
		m_pMain->m_pDlgCaliper->m_Caliper[real_cam][pos][1].processCaliper(pImg, w, h, 0.0, 0.0, 0.0, TRUE);

		double pos_x, pos_y;
		if (m_pMain->m_pDlgCaliper->m_Caliper[real_cam][pos][0].getIsMakeLine() && m_pMain->m_pDlgCaliper->m_Caliper[real_cam][pos][1].getIsMakeLine())
		{
			bFind = TRUE;
		}


		if (bFind)
		{
			sLine lineInfo[2];
			lineInfo[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][pos][0].m_lineInfo;
			lineInfo[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][pos][1].m_lineInfo;

			pCaliper->cramersRules(-lineInfo[0].a, 1, -lineInfo[1].a, 1, lineInfo[0].b, lineInfo[1].b, &pos_x, &pos_y);

			pInfo->SetXPos(pos_x);
			pInfo->SetYPos(pos_y);
			pInfo->SetScore(100.0);
			pInfo->SetFound(FIND_OK);
		}
		else
		{
			pInfo->SetXPos(0.0);
			pInfo->SetYPos(0.0);
			pInfo->SetScore(0.0);
			pInfo->SetFound(FIND_ERR);
		}
	}

	return bFind;
}

void TabMachineAlgoPage::UseClibrationMarkMatching(BYTE* pImage, int nJob, int nCam, int nPos, int m_nWidth, int m_nHeight)
{
	CRect rcInspROI = CRect(100, 100, m_nWidth - 100, m_nHeight - 100);
	((CPaneMachine*)m_pMain->m_pPane[PANE_MACHINE])->findPattern_Matching(pImage, nCam, nPos, m_nWidth, m_nHeight,rcInspROI);
	m_pMain->GetMatching(nJob).setFindInfo(nCam, nPos, ((CPaneMachine*)m_pMain->m_pPane[PANE_MACHINE])->m_CalibFindInfo);
}

//KJH 2021-12-29 Pattern Index 추가
void TabMachineAlgoPage::dispPatternIndex()
{
	if (m_pMain->m_pPane[PANE_MACHINE] == NULL) return;

	CString str;
	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->GetCurSel();
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();
	int m_nSelectPatternIndex = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_PATTERN_INDEX))->GetCurSel();
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_PATTERN_INDEX))->ResetContent();

	CString strFilePath, strMaskPath;
	CFileStatus fs;

	for (int i = 0; i < MAX_PATTERN_INDEX; i++)
	{
		//KJH 2021-12-20 Display와 Combo box Mark Index 매칭작업
		strFilePath.Format("%s%s\\CALIB%d%d%d.bmp", m_pMain->m_strCurrModelDir, m_pMain->vt_job_info[m_nJobID].job_name.c_str(), nCam, nPos, i);

		if (CFile::GetStatus(strFilePath, fs) && ((CPaneMachine*)m_pMain->m_pPane[PANE_MACHINE])->m_CalibHalcon[nCam][nPos][i].getModelRead()==false)
		{
			strMaskPath.Format("%s%s\\%s_%d%d%d%s", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[m_nJobID].job_name.c_str(), "CalibMarkMask", nCam, nPos, i, "Info.bmp");
			if (CFile::GetStatus(strMaskPath, fs))
				((CPaneMachine*)m_pMain->m_pPane[PANE_MACHINE])->m_CalibHalcon[nCam][nPos][i].halcon_ReadModelNew(strFilePath, strMaskPath);
			else ((CPaneMachine*)m_pMain->m_pPane[PANE_MACHINE])->m_CalibHalcon[nCam][nPos][i].halcon_ReadModel(strFilePath, -1, -1);
		}

		if (((CPaneMachine*)m_pMain->m_pPane[PANE_MACHINE])->m_CalibHalcon[nCam][nPos][i].getModelRead())			str.Format("%d : OK", i + 1);
		else																				                str.Format("%d : Empty", i + 1);

		((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_PATTERN_INDEX))->AddString(str);
	}

	((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_PATTERN_INDEX))->SetCurSel(m_nSelectPatternIndex);
}

void TabMachineAlgoPage::OnBnClickedBtnPatternDelete()
{
	// TODO: Add your control notification handler code here

	CString str;
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_PATTERN_INDEX))->GetWindowText(str);
	int nIndex = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_PATTERN_INDEX))->GetCurSel();

	if (str.Find("OK") < 0)
	{
		m_pMain->fnSetMessage(1, "No Pattern to Delete");
		return;
	}

	BOOL bRet = m_pMain->fnSetMessage(2, "Do You Delete the Pattern?");
	if (bRet != TRUE) return;

	int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->GetCurSel();
	int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();

	CString strFilePath;
	strFilePath.Format("%s%s\\CALIB%d%d%d.bmp", m_pMain->m_strCurrModelDir, m_pMain->vt_job_info[m_nJobID].job_name.c_str(), nCam, nPos, nIndex);

	CFileStatus fs;
	if (CFile::GetStatus(strFilePath, fs))
	{
		((CPaneMachine*)m_pMain->m_pPane[PANE_MACHINE])->m_CalibHalcon[nCam][nPos][nIndex].setModelRead(false);
		
		::DeleteFileA(strFilePath);
	}

	strFilePath.Format("%s%s\\CalibMarkMask_%d%d%dInfo.bmp", m_pMain->m_strCurrModelDir, m_pMain->vt_job_info[m_nJobID].job_name.c_str(), nCam, nPos, nIndex);
	if (CFile::GetStatus(strFilePath, fs))
	{
		::DeleteFileA(strFilePath);
	}
	
	updateFrameDialog();
}


void TabMachineAlgoPage::OnBnClickedBtnOffsetRotate()
{
	// KBJ 2022-02-25 Add RotateCenter Offset Dlg. 
	if (m_pMain->m_pOffsetDlg->IsWindowVisible())
	{
		m_pMain->m_pOffsetDlg->ShowWindow(SW_HIDE);
		if (m_pMain->m_pOffsetDlg->Check_ApplyData() == TRUE)
		{
			m_LbRotateX.SetColorText(255, RGB(255, 0, 0));
			m_LbRotateY.SetColorText(255, RGB(255, 0, 0));
		}
		else
		{
			m_LbRotateX.SetColorText(255, RGB(255, 255, 255));
			m_LbRotateY.SetColorText(255, RGB(255, 255, 255));
		}
	}
	else
	{
		int nCam = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_CAMERA))->GetCurSel();
		int nPos = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_MACHINE_POSITION))->GetCurSel();
		m_pMain->m_pOffsetDlg->Set_Offset_Target(OFFSET_ROTATE_CENTER, m_nJobID, nCam, nPos);
		m_pMain->m_pOffsetDlg->ShowWindow(SW_SHOW);
	}
}

void TabMachineAlgoPage::seqCalcCurrentAngle()
{
	CString str;
	int nRet = 0, real_cam, W, H;

#ifndef JOB_INFO
	int camCount = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraNum(m_nTabAlgoIndex);
	std::vector<int> camBuf = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(m_nTabAlgoIndex);
#else
	int camCount = m_pMain->vt_job_info[m_nJobID].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[m_nJobID].camera_index;
#endif
	int nPos = 0,nMoveCount=0;
	int nSequence = 100,_tDelay=0;
	double vCalcRotateData[2] = { 0, };

	BOOL bFind = TRUE;
	CFindInfo* pInfo;

	double posX[4] = { 0, }, posY[4] = { 0, };

	do
	{
		switch (nSequence)
		{
			case 100:
			{	
				for (int nCam = 0; nCam < m_nAlgorithmCamCount; nCam++)
				{
					real_cam = m_nAlgorithmCamBuff.at(nCam);
					pInfo = &m_pMain->GetMatching(m_nJobID).getFindInfo(nCam, nPos);

					W = m_pMain->m_stCamInfo[real_cam].w;
					H = m_pMain->m_stCamInfo[real_cam].h;

					m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 0), m_pMain->getSrcBuffer(real_cam), W * H);

					UseClibrationMarkMatching((BYTE*)m_pMain->getProcBuffer(real_cam, 0), m_nJobID, nCam, nPos, W, H);

					if (pInfo->GetFound() == FIND_ERR)				bFind = FALSE;

					str.Format("Cam %d(%.3f,%.3f)) %.1f%% %s", real_cam + 1, pInfo->GetXPos(), pInfo->GetYPos(), pInfo->getScore(), pInfo->GetFound() == FIND_OK ? "OK" : "NG");
					::SendMessageA(this->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)& str);

					// KBJ 2022-02-22 Machine draw info
					stDARW_MACHINE_INFO draw_info;
					draw_info.nCount = 0;
					draw_info.nCam = nCam;
					draw_info.nPos = nPos;
					draw_info.bClear = true;
					::SendMessageA(m_pMain->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_DISPLAY_MACHINE_VIEW_POS, (LPARAM)& draw_info);

					posX[nCam + nPos] = (W/2 - pInfo->GetXPos())* m_pMain->GetMachine(m_nJobID).getCameraResolutionX(nCam, 0);
					posY[nCam + nPos] = (H/2 - pInfo->GetYPos()) * m_pMain->GetMachine(m_nJobID).getCameraResolutionY(nCam, 0);

					if (m_nAlgorithmCamCount == 1 && nPos == 0) nPos = 1;
					else if(nCam == m_nAlgorithmCamCount-1) nSequence = 200;
				}
			}
			break;
			case 200: //각도 계산
			{
				if (bFind)
				{					
					double theta = 0.0, dbDiagonal=-1, dbDist=0;
					double dx = posX[1] - posX[0];
					double dy = posY[1] - posY[0];

					dbDiagonal = m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getFiducialMarkPitchX();
					if (m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getAlignmentTargetDir() == 0)		dbDist = dy;
					else				dbDist = dx;

					if(dbDiagonal!=0)	theta = asin(dbDist / dbDiagonal) / CV_PI * 180.0;

					str.Format("%.4f",theta);
					GetDlgItem(IDC_EDIT_MOVE_T)->SetWindowTextA(str);
					vCalcRotateData[nMoveCount] = theta;

					if(nMoveCount==0)			nSequence = 300; // 한번더 반복
					else
					{
						str.Format("%.4f", vCalcRotateData[1] - vCalcRotateData[0]);
						GetDlgItem(IDC_EDIT_MOVE_T)->SetWindowTextA(str);
					}
				}
				else
				{
					str.Format("Mark Find Error");
					::SendMessageA(this->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)& str);
				}
				nSequence = -1;
			}
			break;
			case 250:
			{
				_tDelay++;
				if (_tDelay > 50) // 10이 1초여서 5초 대기
				{
					nPos = 0;
					nSequence = 100;
				}
			}
			break;
			case 300:
			{
				GetDlgItem(IDC_EDIT_MOVE_X)->SetWindowTextA("0");
				GetDlgItem(IDC_EDIT_MOVE_Y)->SetWindowTextA("0");
				GetDlgItem(IDC_EDIT_MOVE_T)->SetWindowTextA("2");
				OnBnClickedBtnMove();

				_tDelay = 0;
				nMoveCount++;
				nSequence = 250;
			}
			break;			
		}

		Delay(100);

	} while (nSequence!=-1);

	str.Format("Rotate Angle = (%3.4f) CalcAngle = (%3.4f) \r\n Angle calculation done !", 2.0,vCalcRotateData[1] - vCalcRotateData[0]);
	if (m_pMain->fnSetMessage(2, str) != TRUE)	return;
}


