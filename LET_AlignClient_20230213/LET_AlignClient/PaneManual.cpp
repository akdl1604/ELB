// PaneManual.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "PaneManual.h"
#include "PaneCamera.h"
#include "LET_AlignClientDlg.h"

// CPaneManual

IMPLEMENT_DYNCREATE(CPaneManual, CFormView)

CPaneManual::CPaneManual()
	: CFormView(CPaneManual::IDD)
{

}

CPaneManual::~CPaneManual()
{
}

void CPaneManual::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_BTN_MODEL, m_btnModel);
	DDX_Control(pDX, IDC_BTN_CAMERA, m_btnCamera);
	DDX_Control(pDX, IDC_BTN_MACHINE, m_btnMachine);
	DDX_Control(pDX, IDC_BTN_INTERFACE, m_btnInterfacePlc);
	DDX_Control(pDX, IDC_BTN_SIMULATION, m_btnSimulation);
	DDX_Control(pDX, IDC_BTN_SYSTEMOPTION, m_btnSystemOption);
	DDX_Control(pDX, IDC_BTN_TREND, m_btnTrendView);
}

BEGIN_MESSAGE_MAP(CPaneManual, CFormView)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_CAMERA, &CPaneManual::OnBnClickedBtnCamera)
	ON_BN_CLICKED(IDC_BTN_MODEL, &CPaneManual::OnBnClickedBtnModel)
	ON_BN_CLICKED(IDC_BTN_MACHINE, &CPaneManual::OnBnClickedBtnMachine)
	ON_WM_PAINT()
	ON_MESSAGE(WM_VIEW_CONTROL, OnViewControl)
	ON_BN_CLICKED(IDC_BTN_INTERFACE, &CPaneManual::OnBnClickedBtnInterface)
	ON_BN_CLICKED(IDC_BTN_SIMULATION, &CPaneManual::OnBnClickedBtnSimulation)
	ON_BN_CLICKED(IDC_BTN_SYSTEMOPTION, &CPaneManual::OnBnClickedBtnSystemoption)
	ON_BN_CLICKED(IDC_BTN_TREND, &CPaneManual::OnBnClickedBtnTrend)
END_MESSAGE_MAP()


// CPaneManual 진단입니다.

#ifdef _DEBUG
void CPaneManual::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPaneManual::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CPaneManual 메시지 처리기입니다.
void CPaneManual::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);

	GetClientRect(&m_rcForm);

	MainButtonInit(&m_btnModel);
	MainButtonInit(&m_btnCamera);
	MainButtonInit(&m_btnMachine);
	MainButtonInit(&m_btnInterfacePlc);
	MainButtonInit(&m_btnSimulation);
	MainButtonInit(&m_btnSystemOption);
	MainButtonInit(&m_btnTrendView);

#ifndef _DATA_BASE
	m_btnTrendView.ShowWindow(SW_HIDE);
#endif
}


HBRUSH CPaneManual::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{

	return m_hbrBkg;
}

void CPaneManual::InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}

void CPaneManual::MainButtonInit(CButtonEx *pbutton, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}

void CPaneManual::InitTitle(CLabelEx *pTitle, float size, COLORREF bgcolor, COLORREF color)
{
	CString str;
	pTitle->GetWindowTextA(str);

	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, bgcolor);
	pTitle->SetColorText(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}

void CPaneManual::OnBnClickedBtnCamera()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLET_AlignClientDlg *pDlg = (CLET_AlignClientDlg *)GetParent();

	//tnwjddl 조명 camera들어갈때
	for (int i = 0; i < pDlg->vt_job_info.size(); i++)
	{
		int nMetho = pDlg->vt_job_info[i].algo_method;
		if (nMetho == CLIENT_TYPE_ASSEMBLE_INSP || nMetho == CLIENT_TYPE_4CAM_1SHOT_ALIGN)
			pDlg->LightUse(pDlg->LIGHT_ON, i);
	}
	
	m_btnModel.SetColorText(255, RGB(255, 255, 255));
	m_btnCamera.SetColorText(255, RGB(255, 0, 0));
	m_btnInterfacePlc.SetColorText(255, RGB(255, 255, 255));
	m_btnMachine.SetColorText(255, RGB(255, 255, 255));
	m_btnSimulation.SetColorText(255, RGB(255, 255, 255));
	m_btnTrendView.SetColorText(255, RGB(255, 255, 255));
	m_btnSystemOption.SetColorText(255, RGB(255, 255, 255));

	//KJH 2021-08-06 Pane Camera 눌렀을때 직전 Tap으로 보여지게 변경
	CPaneCamera* pCamera = (CPaneCamera*)pDlg->m_pPane[PANE_CAMERA];
	int nJob = pCamera->m_hSelectTab.GetCurSel();

	int viewer = pDlg->vt_job_info[nJob].viewer_index[0];
	int cam = pDlg->vt_job_info[nJob].camera_index[0];
	::SendMessageA(pDlg->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_SET_CAMERA_POS, MAKELPARAM(viewer, cam));

	pDlg->changeForm(FORM_CAMERA);
	pDlg->changePane(PANE_CAMERA);

	//2021-05-21 KJH 탭 변경시 화면 갱신시 강제로 Fit이미지를 기본으로 설정
	((CFormCameraView*)pDlg->m_pForm[FORM_CAMERA])->m_ViewerCamera.OnFitImage();
	((CFormCameraView*)pDlg->m_pForm[FORM_CAMERA])->m_ViewerCamera.setNavi(false);

	//KJH 2021-12-29 Tap에 맞는 조명으로 초기화
	int camCount = pDlg->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = pDlg->vt_job_info[nJob].camera_index;
	for (int nCam = 0; nCam < camCount; nCam++)
	{
		for (int nLight = 0; nLight < pDlg->vt_job_info[nJob].light_info[nCam].num_of_using_light; nLight++)
		{
			int ctrl = pDlg->vt_job_info[nJob].light_info[nCam].controller_id[nLight] - 1;
			int ch = pDlg->vt_job_info[nJob].light_info[nCam].channel_id[nLight];
			int value = pDlg->vt_job_info[nJob].model_info.getLightInfo().getLightBright(nCam, nLight, 0);

			if (pDlg->m_stLightCtrlInfo.nType[nCam] == LTYPE_LLIGHT_LPC_COT_232)
			{
				pDlg->SetLightBright(ctrl, ch, value == 0 ? 0 : value * 10);   // dh.jung 2021-07-12
			}
			else
			{
				pDlg->SetLightBright(ctrl, ch, value);
			}
			Delay(50);
		}
	}
}

void CPaneManual::OnBnClickedBtnModel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.	
	CLET_AlignClientDlg *pDlg = (CLET_AlignClientDlg *)GetParent();
	pDlg->changeForm(FORM_MODEL);
	pDlg->changePane(PANE_MODEL);

	m_btnModel.SetColorText(255, RGB(255, 0, 0));
	m_btnCamera.SetColorText(255, RGB(255, 255, 255));
	m_btnInterfacePlc.SetColorText(255, RGB(255, 255, 255));
	m_btnMachine.SetColorText(255, RGB(255, 255, 255));
	m_btnSimulation.SetColorText(255, RGB(255, 255, 255));
	m_btnTrendView.SetColorText(255, RGB(255, 255, 255));
	m_btnSystemOption.SetColorText(255, RGB(255, 255, 255));
}

void CPaneManual::OnBnClickedBtnMachine()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLET_AlignClientDlg *pDlg = (CLET_AlignClientDlg *)GetParent();
	pDlg->changeForm(FORM_MACHINE);
	pDlg->changePane(PANE_MACHINE);

	//tnwjddl 조명 camera들어갈때
	for (int i = 0; i < pDlg->vt_job_info.size(); i++)
	{
		int nMetho = pDlg->vt_job_info[i].algo_method;
		if (nMetho == CLIENT_TYPE_ASSEMBLE_INSP || nMetho == CLIENT_TYPE_4CAM_1SHOT_ALIGN)
			pDlg->LightUse(pDlg->LIGHT_ON, i);
	}

	m_btnModel.SetColorText(255, RGB(255, 255, 255));
	m_btnCamera.SetColorText(255, RGB(255, 255, 255));
	m_btnInterfacePlc.SetColorText(255, RGB(255, 255, 255));
	m_btnMachine.SetColorText(255, RGB(255, 0, 0));
	m_btnSimulation.SetColorText(255, RGB(255, 255, 255));
	m_btnTrendView.SetColorText(255, RGB(255, 255, 255));
	m_btnSystemOption.SetColorText(255, RGB(255, 255, 255));

	int index = ((CPaneMachine*)pDlg->m_pPane[PANE_MACHINE])->getCurrentMachineAlgorithm();
	::SendMessageA(pDlg->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_SET_ALGORITHM_POS, index);

	//KJH 2021-12-29 Tap에 맞는 조명으로 초기화
	int camCount = pDlg->vt_job_info[index].num_of_camera;
	std::vector<int> camBuf = pDlg->vt_job_info[index].camera_index;
	for (int nCam = 0; nCam < camCount; nCam++)
	{
		for (int nLight = 0; nLight < pDlg->vt_job_info[index].light_info[nCam].num_of_using_light; nLight++)
		{
			int ctrl = pDlg->vt_job_info[index].light_info[nCam].controller_id[nLight] - 1;
			int ch = pDlg->vt_job_info[index].light_info[nCam].channel_id[nLight];
			int value = pDlg->vt_job_info[index].model_info.getLightInfo().getLightBright(nCam, nLight, 0);

			if (pDlg->m_stLightCtrlInfo.nType[nCam] == LTYPE_LLIGHT_LPC_COT_232)
			{
				pDlg->SetLightBright(ctrl, ch, value == 0 ? 0 : value * 10);   // dh.jung 2021-07-12
			}
			else
			{
				pDlg->SetLightBright(ctrl, ch, value);
			}
			Delay(50);
		}
	}
	((CPaneMachine*)pDlg->m_pPane[PANE_MACHINE])->UpdatePatternIndex(); // Tkyha 마크 표시
	//((CPaneMachine*)pDlg->m_pPane[PANE_MACHINE])->initLightControl(); // Tkyha 211115 조명 초기화 작업
}

void CPaneManual::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 그리기 메시지에 대해서는 CFormView::OnPaint()을(를) 호출하지 마십시오.

	CPen pen(PS_SOLID, 3, COLOR_DDARK_GRAY), *pOld;

	pOld = dc.SelectObject(&pen);
	dc.MoveTo(m_rcForm.left, m_rcForm.top);
	dc.LineTo(m_rcForm.right, m_rcForm.top);
	dc.LineTo(m_rcForm.right, m_rcForm.bottom);
	dc.LineTo(m_rcForm.left, m_rcForm.bottom);
	dc.LineTo(m_rcForm.left, m_rcForm.top);
	dc.SelectObject(pOld);
}

LRESULT CPaneManual::OnViewControl(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case MSG_PMN_MANUAL_COLOR_RESET:
	{
		m_btnModel.SetColorText(255, RGB(255, 255, 255));
		m_btnCamera.SetColorText(255, RGB(255, 255, 255));
		m_btnInterfacePlc.SetColorText(255, RGB(255, 255, 255));
		m_btnMachine.SetColorText(255, RGB(255, 255, 255));
		m_btnSimulation.SetColorText(255, RGB(255, 255, 255));
	}
	break;
	}
	return 0;
}

void CPaneManual::OnBnClickedBtnInterface()
{
	CLET_AlignClientDlg *pDlg = (CLET_AlignClientDlg *)GetParent();

	pDlg->changeForm(FORM_INTERFACE);

	m_btnModel.SetColorText(255, RGB(255, 255, 255));
	m_btnCamera.SetColorText(255, RGB(255, 255, 255));
	m_btnInterfacePlc.SetColorText(255, RGB(255, 0, 0));
	m_btnMachine.SetColorText(255, RGB(255, 255, 255));
	m_btnSimulation.SetColorText(255, RGB(255, 255, 255));
	m_btnTrendView.SetColorText(255, RGB(255, 255, 255));
	m_btnSystemOption.SetColorText(255, RGB(255, 255, 255));
}

void CPaneManual::OnBnClickedBtnSimulation()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLET_AlignClientDlg *pDlg = (CLET_AlignClientDlg *)GetParent();
	
	pDlg->changeForm(FORM_MAIN);
	pDlg->changePane(PANE_SIMULATION);
	pDlg->changeBottomPane(B_PANE_SIMULATION);
	
	m_btnModel.SetColorText(255, RGB(255, 255, 255));
	m_btnCamera.SetColorText(255, RGB(255, 255, 255));
	m_btnInterfacePlc.SetColorText(255, RGB(255, 255, 255));
	m_btnMachine.SetColorText(255, RGB(255, 255, 255));
	m_btnSimulation.SetColorText(255, RGB(255, 0, 0));
	m_btnSystemOption.SetColorText(255, RGB(255, 255, 255));
	m_btnTrendView.SetColorText(255, RGB(255, 255, 255));

	//스펙 표 띄우기
//	::SendMessageA(theApp.m_pFrame->m_pPane[PANE_AUTO]->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_PA_SPEC_VIEW, NULL);

	CPaneBottomSimulation* pMain = (CPaneBottomSimulation*)pDlg->m_pPaneBottom[B_PANE_SIMULATION];
	pMain->update_frame_dialog();
}

void CPaneManual::OnBnClickedBtnSystemoption()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLET_AlignClientDlg* pDlg = (CLET_AlignClientDlg*)GetParent();

	pDlg->changeForm(FORM_SYSTEM_OPTION);

	m_btnModel.SetColorText(255, RGB(255, 255, 255));
	m_btnCamera.SetColorText(255, RGB(255, 255, 255));
	m_btnInterfacePlc.SetColorText(255, RGB(255, 255, 255));
	m_btnMachine.SetColorText(255, RGB(255, 255, 255));
	m_btnSimulation.SetColorText(255, RGB(255, 255, 255));
	m_btnSystemOption.SetColorText(255, RGB(255, 0, 0));
	m_btnTrendView.SetColorText(255, RGB(255, 255, 255));
}

void CPaneManual::OnBnClickedBtnTrend()
{
	CLET_AlignClientDlg* pDlg = (CLET_AlignClientDlg*)GetParent();
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	m_btnModel.SetColorText(255, RGB(255, 255, 255));
	m_btnCamera.SetColorText(255, RGB(255, 255, 255));
	m_btnInterfacePlc.SetColorText(255, RGB(255, 255, 255));
	m_btnMachine.SetColorText(255, RGB(255, 255, 255));
	m_btnSimulation.SetColorText(255, RGB(255, 255, 255));
	m_btnSystemOption.SetColorText(255, RGB(255, 255, 255));
	m_btnTrendView.SetColorText(255, RGB(255, 0, 0));

	pDlg->changeForm(FORM_TREND);	
}
