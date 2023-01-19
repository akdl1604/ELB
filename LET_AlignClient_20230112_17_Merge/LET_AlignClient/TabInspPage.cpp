// TabInspPage.cpp: 구현 파일
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "TabInspPage.h"

// TabInspPage 대화 상자

IMPLEMENT_DYNAMIC(TabInspPage, CDialogEx)

TabInspPage::TabInspPage(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TAB_PANE_INSP, pParent)
{
	m_nResourceID[0] = IDC_EDIT_ROI_SIZE_X;
	m_nResourceID[1] = IDC_EDIT_ROI_SIZE_Y;
	m_nResourceID[2] = IDC_EDIT_ROI_CENTER_OFFSET_X;
	m_nResourceID[3] = IDC_EDIT_ROI_CENTER_OFFSET_Y;
	m_nResourceID[4] = IDC_EDIT_LIMIT_OFFSET;

	m_nTabIndex = 0;
	m_nSelectPanel = 0;
	m_nSelectPanelExistIndex = 0;
	m_nSelectPatternIndex = 0;
	m_nSelectCamera = 0;
	m_nSelectPosition =0;
}

TabInspPage::~TabInspPage()
{
}

void TabInspPage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_BTN_SIZE_ORIGINAL, m_btnSizeOriginal);
	DDX_Control(pDX, IDC_BTN_SIZE_FIT, m_btnSizeFit);
	DDX_Control(pDX, IDC_BTN_CAMERA_LIVE, m_btnCameraLive);
	DDX_Control(pDX, IDC_BTN_CAMERA_STOP, m_btnCameraStop);
	DDX_Control(pDX, IDC_BTN_IMAGE_SAVE, m_btnImageSave);
	DDX_Control(pDX, IDC_BTN_IMAGE_OPEN, m_btnImageOpen);
	DDX_Control(pDX, IDC_LB_SELECT_CHIP, m_lbSelectChip);
	DDX_Control(pDX, IDC_LB_SELECT_POSITION, m_lbSelectPosition);
	DDX_Control(pDX, IDC_LB_AVG_GRAY_LIMIT, m_lbAvgGrayLimit);
	DDX_Control(pDX, IDC_LB_FOCUS_ENERGY_TITLE, m_LbFocusEnergyTitle);
	DDX_Control(pDX, IDC_LB_FOCUS_ENERGY, m_LbFocusEnergy);
	DDX_Control(pDX, IDC_LB_JUDGE_CONDITION, m_lbJudgeCondition);
	DDX_Control(pDX, IDC_BTN_DRAW_AREA, m_btnDrawArea);
	DDX_Control(pDX, IDC_BTN_FOCUS_MEASURE, m_btnFocusMeasure);
	DDX_Control(pDX, IDC_BTN_MANUAL_INSP, m_btnManualInsp);
	DDX_Control(pDX, IDC_BTN_PATTERN_SAVE, m_btnPatternSave);
	DDX_Control(pDX, IDC_BTN_SET_ROI_EXIST, m_btnSetRoiExist);
	DDX_Control(pDX, IDC_BTN_CALC_LIMIT_OFFSET, m_btnCalcLimitOffset);

	DDX_Control(pDX, IDC_STATIC_ROI_SIZE_X, m_lbRoiSizeX);
	DDX_Control(pDX, IDC_STATIC_ROI_SIZE_Y, m_lbRoiSizeY);
	DDX_Control(pDX, IDC_STATIC_ROI_CENTER_OFFSET_X, m_lbRoiOffsetX);
	DDX_Control(pDX, IDC_STATIC_ROI_CENTER_OFFSET_Y, m_lbRoiOffsetY);
	DDX_Control(pDX, IDC_STATIC_LIMIT_OFFSET, m_lbLimitOffset);

	DDX_Control(pDX, IDC_SLIDER_LIGHT_SETTING_EXIST, m_sliderLightSettingExist);

	DDX_Control(pDX, IDC_CB_SELECT_CAMERA, m_cmbSelectCamera);

	
}


BEGIN_MESSAGE_MAP(TabInspPage, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	ON_MESSAGE(WM_VIEW_CONTROL, OnViewControl)
	ON_BN_CLICKED(IDC_BTN_SIZE_ORIGINAL, &TabInspPage::OnBnClickedBtnSizeOriginal)
	ON_BN_CLICKED(IDC_BTN_SIZE_FIT, &TabInspPage::OnBnClickedBtnSizeFit)
	ON_BN_CLICKED(IDC_BTN_CAMERA_LIVE, &TabInspPage::OnBnClickedBtnCameraLive)
	ON_BN_CLICKED(IDC_BTN_CAMERA_STOP, &TabInspPage::OnBnClickedBtnCameraStop)
	ON_BN_CLICKED(IDC_BTN_IMAGE_SAVE, &TabInspPage::OnBnClickedBtnImageSave)
	ON_BN_CLICKED(IDC_BTN_IMAGE_OPEN, &TabInspPage::OnBnClickedBtnImageOpen)
	ON_BN_CLICKED(IDC_BTN_DRAW_AREA, &TabInspPage::OnBnClickedBtnDrawArea)
	ON_BN_CLICKED(IDC_BTN_FOCUS_MEASURE, &TabInspPage::OnBnClickedBtnFocusMeasure)
	ON_BN_CLICKED(IDC_BTN_SET_ROI_EXIST, &TabInspPage::OnBnClickedBtnSetRoiExist)
	ON_BN_CLICKED(IDC_BTN_CALC_LIMIT_OFFSET, &TabInspPage::OnBnClickedBtnCalcLimitOffset)
	ON_BN_CLICKED(IDC_BTN_MANUAL_INSP, &TabInspPage::OnBnClickedBtnManualInsp)
	ON_BN_CLICKED(IDC_BTN_PATTERN_SAVE, &TabInspPage::OnBnClickedBtnPatternSave)
	ON_CBN_SELCHANGE(IDC_CB_SELECT_CHIP, &TabInspPage::OnCbnSelchangeCbSelectChip)
	ON_CBN_SELCHANGE(IDC_CB_SELECT_POSITION, &TabInspPage::OnCbnSelchangeCbSelectPosition)
	ON_CBN_SELCHANGE(IDC_CB_JUDGE_CONDITION, &TabInspPage::OnCbnSelchangeCbJudgeCondition)
	ON_EN_SETFOCUS(IDC_EDIT_AVG_GRAY_LIMIT, &TabInspPage::OnEnSetfocusEditAvgGrayLimit)
	ON_EN_SETFOCUS(IDC_EDIT_LIGHT_SETTING_EXIST, &TabInspPage::OnEnSetfocusEditLightSettingExist)
	ON_CBN_SELCHANGE(IDC_CB_SELECT_CAMERA, &TabInspPage::OnCbnSelchangeCbSelectCamera)
END_MESSAGE_MAP()


// TabInspPage 메시지 처리기
BOOL TabInspPage::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);
	m_pMain = (CLET_AlignClientDlg *)AfxGetMainWnd();

	InitTitle(&m_LbFocusEnergyTitle, "Focus Energy", 16.f, RGB(64, 64, 64));
	InitTitle(&m_lbSelectPosition, "SelectPosition", 16.f, RGB(64, 64, 64));
	InitTitle(&m_lbSelectChip, "Select Position", 16.f, RGB(64, 64, 64));
	InitTitle(&m_lbAvgGrayLimit, "Threshold", 16.f, RGB(64, 64, 64));
	InitTitle(&m_lbJudgeCondition, "Judge Condition", 16.f, RGB(64, 64, 64));
	InitTitle(&m_LbFocusEnergy, "", 16.f, COLOR_UI_BODY);
	InitTitle(&m_lbRoiSizeX, "ROI Size X", 16.f, RGB(64, 64, 64));
	InitTitle(&m_lbRoiSizeY, "ROI Size Y", 16.f, RGB(64, 64, 64));
	InitTitle(&m_lbRoiOffsetX, "ROI Offset X", 16.f, RGB(64, 64, 64));
	InitTitle(&m_lbRoiOffsetY, "ROI Offset Y", 16.f, RGB(64, 64, 64));
	InitTitle(&m_lbLimitOffset, "Limit Offset", 16.f, RGB(64, 64, 64));

	MainButtonInit(&m_btnSizeOriginal);
	MainButtonInit(&m_btnSizeFit);
	MainButtonInit(&m_btnCameraLive);
	MainButtonInit(&m_btnCameraStop);
	MainButtonInit(&m_btnImageSave);
	MainButtonInit(&m_btnImageOpen);
	MainButtonInit(&m_btnManualInsp);
	MainButtonInit(&m_btnDrawArea);
	MainButtonInit(&m_btnFocusMeasure);
	MainButtonInit(&m_btnPatternSave);
	MainButtonInit(&m_btnSetRoiExist);
	MainButtonInit(&m_btnCalcLimitOffset);

	m_btnDrawArea.SetSizeText(16.f);
	m_btnFocusMeasure.SetSizeText(16.f);

	CRect rect;
	((CEdit *)GetDlgItem(IDC_EDIT_AVG_GRAY_LIMIT))->GetRect(&rect);
	rect.top += 3;	rect.bottom += 3;
	((CEdit *)GetDlgItem(IDC_EDIT_AVG_GRAY_LIMIT))->SetRect(&rect);

	GetDlgItem(IDC_BTN_FOCUS_MEASURE)->ShowWindow(SW_HIDE);
	//GetDlgItem(IDC_BTN_SET_ROI_EXIST)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_LB_FOCUS_ENERGY_TITLE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_LB_FOCUS_ENERGY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CHECK_CALC_WHOLE_ROI)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_LB_SELECT_POSITION)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CB_SELECT_POSITION)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_STATIC_ROI_SIZE_X)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_ROI_SIZE_X)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_ROI_CENTER_OFFSET_X)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_ROI_CENTER_OFFSET_X)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_ROI_SIZE_Y)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_ROI_SIZE_Y)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_ROI_CENTER_OFFSET_Y)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_ROI_CENTER_OFFSET_Y)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_STATIC_LIMIT_OFFSET)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_LIMIT_OFFSET)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BTN_CALC_LIMIT_OFFSET)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_LB_JUDGE_CONDITION)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CB_JUDGE_CONDITION)->ShowWindow(SW_HIDE);

	((CComboBox*)GetDlgItem(IDC_CB_SELECT_CHIP))->ResetContent();
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_CHIP))->AddString("Pos 1");
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_CHIP))->AddString("Pos 2");
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_CHIP))->SetCurSel(0);

	((CComboBox*)GetDlgItem(IDC_LB_SELECT_CHIP))->SetWindowTextA("Select Position");
	CString str;

	for (int i = 0; i < 2; i++)
	{
		if (i == 0)			str.Format("Higher");
		else if (i == 1)	str.Format("Lower");

		((CComboBox*)GetDlgItem(IDC_CB_JUDGE_CONDITION))->AddString(str);
	}

	m_sliderLightSettingExist.SetRange(0, 255);
	m_sliderLightSettingExist.SetPos(m_pMain->vt_job_info[m_nTabIndex].model_info.getLightInfo().getLightBright(m_nSelectCamera, 0, 0));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

HBRUSH TabInspPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (pWnd->GetDlgCtrlID() == IDC_CB_SELECT_CHIP ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_AVG_GRAY_LIMIT ||
		pWnd->GetDlgCtrlID() == IDC_CB_SELECT_POSITION ||
		pWnd->GetDlgCtrlID() == IDC_CB_JUDGE_CONDITION ||
		pWnd->GetDlgCtrlID() == IDC_CHECK_CALC_WHOLE_ROI ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_LIGHT_SETTING_EXIST ||
		pWnd->GetDlgCtrlID() == m_nResourceID[0] ||
		pWnd->GetDlgCtrlID() == m_nResourceID[1] ||
		pWnd->GetDlgCtrlID() == m_nResourceID[2] ||
		pWnd->GetDlgCtrlID() == m_nResourceID[3] ||
		pWnd->GetDlgCtrlID() == m_nResourceID[4])
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

void TabInspPage::updateFrameDialog()
{
	dispSelectPanel();
	dispViewSize();
	dispCameraLiveStop();
	dispDrawArea();
	dispSelectPanelExistIndex();
	dispAvgGrayLimit();
	dispJudgeCondition();
	dispPatternIndex();
	dispFocusMeasure();

	//int sel = 0;
	//int nCam = m_pMain->m_nSelectCamera;
	//int nIndex = 0;

	//m_sliderLightSettingExist.SetPos(m_tempMachineInfo.getLightBright(nCam, sel, nIndex));

	//CString str;

	//str.Format("%d", m_tempMachineInfo.getLightBright(nCam, sel, nIndex));
	//GetDlgItem(IDC_EDIT_LIGHT_SETTING_EXIST)->SetWindowTextA(str);
}

void TabInspPage::MainButtonInit(CButtonEx *pbutton, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}

void TabInspPage::InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}

void TabInspPage::OnPaint()
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

BOOL TabInspPage::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	if (pMsg->message == WM_LBUTTONUP)
	{
		for (int i = 0; i < 5; i++)
		{
			if (pMsg->hwnd == GetDlgItem(m_nResourceID[i])->m_hWnd)
			{
				//::PostMessageA(m_pMain->m_pPane[PANE_EXIST]->m_hWnd, WM_VIEW_CONTROL, MSG_PCE_INPUT_PARAM, i);
			}
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void TabInspPage::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	switch (nIDEvent) {
	case TIMER_AUTO_FOCUS_MEASURE:
	{
		KillTimer(TIMER_AUTO_FOCUS_MEASURE);

		/*::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_FOCUS_MEASURE, 0);

		CString str;
		str.Format("%.1f", m_pMain->m_dbFocusEnergy);
		m_LbFocusEnergy.SetText(str);

		if (m_pMain->m_bFocusMeasure)	SetTimer(TIMER_AUTO_FOCUS_MEASURE, 500, NULL);
		else	updateFrameDialog();*/

	}
	break;
	}


	CDialogEx::OnTimer(nIDEvent);
}

LRESULT TabInspPage::OnViewControl(WPARAM wParam, LPARAM lParam)
{
	/*switch (wParam) {
	case MSG_PCE_INPUT_PARAM:
	{
		CString str;
		int nID = int(lParam);

		GetDlgItem(m_nResourceID[nID])->GetWindowTextA(str);

		CKeyPadDlg dlg;
		dlg.SetValueString(false, str);

		if (dlg.DoModal() != IDOK)
			return 0;

		CString strNumber;
		dlg.GetValue(strNumber);
		GetDlgItem(m_nResourceID[nID])->SetWindowTextA(strNumber);
	}
	break;
	}*/

	return 0;
}

void TabInspPage::OnBnClickedBtnSizeOriginal()
{
	((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.setOriginalImage();
	((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.setNavi(true);
	dispViewSize();
}


void TabInspPage::OnBnClickedBtnSizeFit()
{
	((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.OnFitImage();
	((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.setNavi(false);
	dispViewSize();
}


void TabInspPage::OnBnClickedBtnCameraLive()
{

	for (int i = 0; i < MAX_CAMERA; i++)
		m_pMain->SendMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_STOP, i);

	m_pMain->SendMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_LIVE, m_nTabIndex);

	updateFrameDialog();
}


void TabInspPage::OnBnClickedBtnCameraStop()
{
	m_pMain->SendMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_STOP, m_nTabIndex);
	updateFrameDialog();
}


void TabInspPage::OnBnClickedBtnImageSave()
{
	char szFilter[] = "Bmp Files (*.bmp) | *.bmp | All Files (*.*) | *.* ||";
	CFileDialog dlg(FALSE, NULL, "..\\*.bmp", OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT, szFilter);

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

		int nJob = m_nTabIndex;
		int real_cam = m_pMain->vt_job_info[nJob].camera_index[0];


		int W = m_pMain->m_stCamInfo[real_cam].w;
		int H = m_pMain->m_stCamInfo[real_cam].h;

		IplImage *pImage = cvCreateImage(cvSize(W,H), 8, 1);
		cvZero(pImage);
		m_pMain->copyMemory(pImage->imageData, m_pMain->getCameraViewBuffer(), W*H);
		cvSaveImage(dlg.GetPathName(), pImage);
		cvReleaseImage(&pImage);
	}
}


void TabInspPage::OnBnClickedBtnImageOpen()
{
	CFileDialog dlg(TRUE, NULL, _T("Image File"),
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Image Files (*.bmp;*.jpg;)|*.bmp;*.jpg|All Files (*.*)|*.*||"),
		this);

	if (dlg.DoModal() != IDOK)
		return;

	CString str = dlg.GetPathName();
	IplImage *pImage = cvLoadImage(str, 0);


	if (pImage == NULL)
	{
		AfxMessageBox("Check the Image Type");
		return;
	}

	int real_cam = m_pMain->vt_job_info[m_nTabIndex].camera_index[0];

	int W = m_pMain->m_stCamInfo[real_cam].w;
	int H = m_pMain->m_stCamInfo[real_cam].h;

	if (pImage->width != W || pImage->height != H)
	{
		CString str;
		str.Format("Image Size is not Valid.	size %d x %d", W, H);
		AfxMessageBox(str);

		cvReleaseImage(&pImage);
		return;
	}
	else
	{
		// 임시.. 테스트용
		m_pMain->copyMemory(m_pMain->getSrcBuffer(real_cam), pImage->imageData, W * H);
		m_pMain->copyMemory(m_pMain->getCameraViewBuffer(), pImage->imageData, W * H);
		cvReleaseImage(&pImage);
		((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.OnLoadImageFromPtr(m_pMain->getCameraViewBuffer());

	}
}


void TabInspPage::OnBnClickedBtnDrawArea()
{
	if (((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.GetModeDrawingFigure())
	{
		((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetModeDrawingFigure(false);
		((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetEnableModifyFigure(0, false);
		((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetEnableDrawFigure(0, false);
	}
	else
	{
		((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetEnableDrawAllFigures(true);
		((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetEnableFigureRectangle(0);
		((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetModeDrawingFigure(true);
		((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetEnableModifyFigure(0, true);
		((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.SetEnableDrawFigure(0, true);
	}


	updateFrameDialog();
}


void TabInspPage::OnBnClickedBtnFocusMeasure()
{
	if (m_pMain->m_bFocusMeasure)
	{
		m_pMain->m_bFocusMeasure = FALSE;
		KillTimer(TIMER_AUTO_FOCUS_MEASURE);
	}
	else
	{
		m_pMain->m_bFocusMeasure = TRUE;
		SetTimer(TIMER_AUTO_FOCUS_MEASURE, 500, NULL);
	}

	updateFrameDialog();
}


void TabInspPage::OnBnClickedBtnSetRoiExist()
{
	if (m_pMain->m_bAutoSearch)
	{
		m_pMain->fnSetMessage(MT_OK, "Impossible In Auto Search Mode");
		return;
	}

	if (IsDlgButtonChecked(IDC_CHECK_CALC_WHOLE_ROI))
	{
		if (m_pMain->fnSetMessage(1, "Do You Calc the Whole ROI?") != TRUE) return;

		_stPatternSelect pattern;
		pattern.nCam = m_nTabIndex;

		CString str;
		GetDlgItem(m_nResourceID[0])->GetWindowTextA(str);
		pattern.nTemp[0] = atoi(str);

		GetDlgItem(m_nResourceID[1])->GetWindowTextA(str);
		pattern.nTemp[1] = atoi(str);

		GetDlgItem(m_nResourceID[2])->GetWindowTextA(str);
		pattern.nTemp[2] = atoi(str);

		GetDlgItem(m_nResourceID[3])->GetWindowTextA(str);
		pattern.nTemp[3] = atoi(str);
		::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_CALC_WHOLE_ROI, (LPARAM)&pattern);
	}
	else
	{
		if (m_pMain->fnSetMessage(1, "Do You Set the Search ROI?") != TRUE) return;

		int nIndex = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
		CFormCameraView *pForm = (CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA];

		CPoint ptStart = pForm->m_ViewerCamera.GetPointBegin(0);
		CPoint ptEnd = pForm->m_ViewerCamera.GetPointEnd(0);

		int w = abs(ptStart.x - ptEnd.x);
		int h = abs(ptStart.y - ptEnd.y);

		if (w < 5 || h < 5)
		{
			AfxMessageBox("Area is to Small!!!");
			return;
		}

		int nCam = 0;
		int nPos = 0;
		CRect rectROI;
		rectROI.left = MIN(ptStart.x, ptEnd.x);
		rectROI.top = MIN(ptStart.y, ptEnd.y);
		rectROI.right = MAX(ptStart.x, ptEnd.x);
		rectROI.bottom = MAX(ptStart.y, ptEnd.y);

		rectROI.left = (rectROI.left / 4) * 4;
		rectROI.right = rectROI.left + (rectROI.Width() / 8 * 8);//(rect.right / 4) * 4; //2017.10.12 Mark Image Width Size 8의 배수 적용
		rectROI.top = (rectROI.top / 4) * 4;
		rectROI.bottom = (rectROI.bottom / 4) * 4;

		m_pMain->GetPanelExist().setSearchROI(m_nSelectPanel, rectROI);
		m_pMain->vt_job_info[m_nTabIndex].model_info.getPanelExistInfo().setSearchROI(m_nSelectPanel, rectROI);
	}
}


void TabInspPage::OnBnClickedBtnCalcLimitOffset()
{
	if (m_pMain->fnSetMessage(2, "Caculate the Limit Offset?") != TRUE) return;


	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_MANUAL_INSPECTION, 0);

	double dAvg = 0.0;
	double offset = 0.0;

	CString str;
	GetDlgItem(IDC_EDIT_LIMIT_OFFSET)->GetWindowTextA(str);
	offset = atof(str);

	for (int nPanel = 0; nPanel < m_pMain->GetPanelExist().getNumPanelInTray(); nPanel++)
	{
		dAvg = m_pMain->GetPanelExist().getAvgBrightness(nPanel);

		if (m_pMain->GetPanelExist().getJudgeCondition() == 0)			// Higher
		{
			if (dAvg + offset > 255)	dAvg = 255;
			else						dAvg += offset;

			m_pMain->GetPanelExist().setAvgGrayLimit(nPanel, dAvg);
		}
		else
		{
			if (dAvg - offset < 0)		dAvg = 0;
			else						dAvg -= offset;

			m_pMain->GetPanelExist().setAvgGrayLimit(nPanel, dAvg);
		}
	}

	updateFrameDialog();
}


void TabInspPage::OnBnClickedBtnManualInsp()
{
	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_MANUAL_INSPECTION, 0);
}


void TabInspPage::OnBnClickedBtnPatternSave()
{
	if (m_pMain->fnSetMessage(2, "Save the Pattern Data?") != TRUE) return;

	for (int nCam = 0; nCam < MAX_CAMERA; nCam++)
	{
		for (int nChannel = 0; nChannel < NUM_AXIS; nChannel++)
		{
			for (int nIndex = 0; nIndex < 4; nIndex++)
			{
				fnSaveLightValue(nCam, nChannel, nIndex);
			}
		}
	}

	m_pMain->savePanelExistInfoData(&m_pMain->vt_job_info[m_nTabIndex].model_info.getPanelExistInfo(), m_pMain->m_strCurrentModelPath);
}


void TabInspPage::OnCbnSelchangeCbSelectChip()
{
	m_nSelectPanel = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_CHIP))->GetCurSel();
	updateFrameDialog();
}


void TabInspPage::OnCbnSelchangeCbSelectPosition()
{
	m_nSelectPanelExistIndex = ((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->GetCurSel();
	updateFrameDialog();
}


void TabInspPage::OnCbnSelchangeCbJudgeCondition()
{
	int nSel = ((CComboBox*)GetDlgItem(IDC_CB_JUDGE_CONDITION))->GetCurSel();
	m_pMain->GetPanelExist().setJudgeCondition(nSel);
}

void TabInspPage::dispCameraLiveStop()
{
#ifdef _DAHUHA
	if (m_pMain->m_pDahuhaCam[m_nSelectCamera].isconnected() && m_pMain->m_pDahuhaCam[m_nSelectCamera].isgrabbing())
#else
	if (m_pMain->m_pBaslerCam[m_nSelectCamera] && m_pMain->m_pBaslerCam[m_nSelectCamera]->IsGrabContinuous() ||
		(m_pMain->m_pSaperaCam[m_nSelectCamera].IsOpend() && m_pMain->m_pSaperaCam[m_nSelectCamera].IsGrabing()))
#endif
	{
		m_btnCameraLive.SetColorBkg(255, COLOR_BTN_SELECT);
		m_btnCameraStop.SetColorBkg(255, RGB(64, 64, 64));
	}
	else
	{
		m_btnCameraLive.SetColorBkg(255, RGB(64, 64, 64));
		m_btnCameraStop.SetColorBkg(255, COLOR_BTN_SELECT);
	}

}
void TabInspPage::dispViewSize()
{
	if (((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.IsFitImage())
	{
		m_btnSizeFit.SetColorBkg(255, COLOR_BTN_SELECT);
		m_btnSizeOriginal.SetColorBkg(255, RGB(64, 64, 64));
	}
	else
	{
		m_btnSizeOriginal.SetColorBkg(255, COLOR_BTN_SELECT);
		m_btnSizeFit.SetColorBkg(255, RGB(64, 64, 64));
	}
}

void TabInspPage::dispDrawArea()
{
	if (((CFormCameraView *)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera.GetModeDrawingFigure())
		m_btnDrawArea.SetColorBkg(255, COLOR_BTN_SELECT);
	else
		m_btnDrawArea.SetColorBkg(255, RGB(64, 64, 64));

}

void TabInspPage::dispPatternIndex()
{
	CString str;
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->ResetContent();

	for (int i = 0; i < MAX_PATTERN_INDEX; i++)
	{
		if (m_pMain->GetMatching(m_nTabIndex).getHalcon(m_nSelectCamera, m_nSelectPosition, i).getModelRead())
			str.Format("%d : OK", i + 1);
		else
			str.Format("%d : Empty", i + 1);

		((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->AddString(str);
	}

	((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->SetCurSel(m_nSelectPatternIndex);
}

void TabInspPage::dispMatchingRate()
{
	//CString str;
	//int nCam = m_pMain->m_nSelectCamera;
	//int nPos = m_pMain->m_nSelectPosition;

	//double matchingRate = m_pMain->GetMatching().getMatchingRate(nCam, nPos);
	//str.Format("%.1f", matchingRate);
	//GetDlgItem(IDC_EDIT_MATCHING_RATE)->SetWindowTextA(str);
}

void TabInspPage::dispFocusMeasure()
{
	if (m_pMain->m_bFocusMeasure)	m_btnFocusMeasure.SetColorBkg(255, COLOR_BTN_SELECT);
	else							m_btnFocusMeasure.SetColorBkg(255, COLOR_BTN_BODY);
}

void TabInspPage::dispAvgGrayLimit()
{
	CString str;
	str.Format("%.1f", m_pMain->GetPanelExist().getAvgGrayLimit(m_nSelectPanel));
	GetDlgItem(IDC_EDIT_AVG_GRAY_LIMIT)->SetWindowTextA(str);
}

void TabInspPage::dispJudgeCondition()
{
	((CComboBox*)GetDlgItem(IDC_CB_JUDGE_CONDITION))->SetCurSel(m_pMain->GetPanelExist().getJudgeCondition());
}

void TabInspPage::dispSelectPanelExistIndex()
{
	CString strPath, str;
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->ResetContent();

	for (int nIndex = 0; nIndex < 5; nIndex++)
	{
		strPath.Format("%sPAT%d%d.bmp", m_pMain->m_strCurrModelDir, m_nSelectPanel + 1, nIndex + 1);

		if (_access(strPath, 0) != 0)	str.Format("%d : Empty", nIndex + 1);
		else							str.Format("%d : OK", nIndex + 1);
		((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->AddString(str);
	}

	((CComboBox*)GetDlgItem(IDC_CB_SELECT_POSITION))->SetCurSel(m_nSelectPanelExistIndex);
}

void TabInspPage::dispSelectPanel()
{
	CString str;
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_CHIP))->ResetContent();
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_CHIP))->AddString("Glass Exist");
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_CHIP))->AddString("Separator Exist");
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_CHIP))->AddString("Crack");
	((CComboBox*)GetDlgItem(IDC_CB_SELECT_CHIP))->SetCurSel(m_nSelectPanel);
	
}

void TabInspPage::fnSaveLightValue(int nCam, int nChannel, int nIndex)
{
	CString strKey, strData, strModelFilePath;
	strModelFilePath.Format("%s%s\\MachineInfo.ini", m_pMain->m_strModelDir, m_pMain->vt_job_info[m_nTabIndex].model_info.getModelID());
	strKey.Format("LIGHT_BRIGHT_%d_%d_%d", nCam + 1, nChannel + 1, nIndex + 1);
	strData.Format("%d", m_pMain->vt_job_info[m_nTabIndex].model_info.getLightInfo().getLightBright(nCam, nChannel, nIndex));
	::WritePrivateProfileStringA("MACHINE_INFO", strKey, strData, strModelFilePath);	
}

void TabInspPage::OnEnSetfocusEditAvgGrayLimit()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GetDlgItem(IDC_LB_AVG_GRAY_LIMIT)->SetFocus();

	CKeyPadDlg dlg;
	CString str;
	GetDlgItem(IDC_EDIT_AVG_GRAY_LIMIT)->GetWindowTextA(str);
	
	dlg.SetValueString(false, str);

	if (dlg.DoModal() != IDOK)		return;

	CString strNumber;
	dlg.GetValue(strNumber);
	GetDlgItem(IDC_EDIT_AVG_GRAY_LIMIT)->SetWindowTextA(strNumber);

	m_pMain->GetPanelExist().setAvgGrayLimit(m_nSelectPanel, atof(strNumber));
	m_pMain->vt_job_info[m_nTabIndex].model_info.getPanelExistInfo().setAvgGrayLimit(m_nSelectPanel, atof(strNumber));
}

void TabInspPage::OnEnSetfocusEditLightSettingExist()
{
	
	/*if (m_pMain->m_bConnectServer != TRUE)
	{
		AfxMessageBox("Server Is Not Disconnected");
		return;
	}

	CString str, strTemp;

	int sel = 0;
	int nCam = m_nSelectCamera;
	int nCtrl = m_nAlignLightCtrl[nCam];
	int nIndex = 0;
	if (sel < 0) return;

	str.Format("%d", m_tempMachineInfo.getLightBright(nCam, sel, nIndex));
	strTemp = m_pMain->GetNumberBox(str, 5, 0, 255);

	GetDlgItem(IDC_EDIT_LIGHT_SETTING_EXIST)->SetWindowTextA(strTemp);

	int v = atoi(strTemp);

	m_pMain->SendServer(TCP_SEND_LIGHT_CONTROL_ACK, nCtrl - 1, m_pMain->m_nAlignLightChannel[nCam][sel] - 1, &v);
	m_tempMachineInfo.setLightBright(nCam, sel, nIndex, v);

	updateFrameDialog();*/
}

void TabInspPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar)
	{
		if (pScrollBar == (CScrollBar*)&m_sliderLightSettingExist)
		{
			//if (m_pMain->m_bConnectServer != TRUE)
			//{
			//	AfxMessageBox("Server Is Not Disconnected");
			//	return;
			//}

			//CString str;
			//int nPos = m_sliderLightSettingExist.GetPos();

			//str.Format("%d", nPos);
			//GetDlgItem(IDC_EDIT_LIGHT_SETTING_EXIST)->SetWindowTextA(str);

			//int sel = 0;
			//int nCam = m_nSelectCamera;
			//int nCtrl = m_nAlignLightCtrl[nCam];
			//int nIndex = 0;

			//m_pMain->SendServer(TCP_SEND_LIGHT_CONTROL_ACK, nCtrl - 1, m_pMain->m_nAlignLightChannel[nCam][sel] - 1, &nPos);
	
			//m_tempMachineInfo.setLightBright(nCam, sel, nIndex, nPos);
			//updateFrameDialog();
		}
	}


	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

void TabInspPage::OnCbnSelchangeCbSelectCamera()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelectCamera = m_cmbSelectCamera.GetCurSel();
}
