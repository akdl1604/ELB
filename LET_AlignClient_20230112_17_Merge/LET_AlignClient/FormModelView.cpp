// FormModelView.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "FormModelView.h"
#include "LET_AlignClientDlg.h"
#include "CommPLC.h"
#include "ComPLC_RS.h"
// CFormModelView

IMPLEMENT_DYNCREATE(CFormModelView, CFormView)

CFormModelView::CFormModelView()
	: CFormView(CFormModelView::IDD)
{
	m_nJobID = 0;
	m_nOldTabSelect = 0;
}

CFormModelView::~CFormModelView()
{
#ifndef JOB_INFO
	for (int i = 0; i < MAX_CAMERA; i++)
	{
		delete c_TabModelPage[i];
	}
#else
	for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
	{
		delete c_TabModelPage[i];
	}
#endif
}

void CFormModelView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MODEL_ALGOLIST, m_hSelectAlgoTab);
	DDX_Control(pDX, IDC_LIST_MODEL, m_ListModelID);

	DDX_Control(pDX, IDC_LB_MODEL_VIEW_TITLE, m_LbModelViewTitle);
	DDX_Control(pDX, IDC_STATIC_MODEL_NAME, m_stt_ModelTitle);
	DDX_Control(pDX, IDC_STATIC_GLASS_WIDTH, m_stt_GlassWidth);
	DDX_Control(pDX, IDC_STATIC_GLASS_HEIGHT, m_stt_GlassHeight);
	DDX_Control(pDX, IDC_STATIC_FRAME_WIDTH, m_stt_FrameWidth);
	DDX_Control(pDX, IDC_STATIC_FRAME_HEIGHT, m_stt_FrameHeight);
	DDX_Control(pDX, IDC_STATIC_MODEL_PANEL_TYPE, m_stt_ModelPanelType);
	DDX_Control(pDX, IDC_STATIC_MARK_PITCH_X, m_stt_FMarkPitch);
	DDX_Control(pDX, IDC_STATIC_GRAB_SECOND, m_lblSecondGrab);
	DDX_Control(pDX, IDC_STATIC_CALIB_COUNT_X, m_stt_CalibrationCountX);
	DDX_Control(pDX, IDC_STATIC_CALIB_COUNT_Y, m_stt_CalibrationCountY);
	DDX_Control(pDX, IDC_STATIC_CALIB_RANGE_X, m_stt_CalibrationRangeX);
	DDX_Control(pDX, IDC_STATIC_CALIB_RANGE_Y, m_stt_CalibrationRangeY);
	DDX_Control(pDX, IDC_STATIC_ROTATE_RANGE, m_stt_RotateRange);
	DDX_Control(pDX, IDC_STATIC_ROTATE_COUNT, m_stt_RotateCount);
	DDX_Control(pDX, IDC_STATIC_CALIBRATION_TYPE, m_lblCalibrationType);

	DDX_Control(pDX, IDC_BTN_MODEL_DATA_SAVE, m_btnDataSave);
	DDX_Control(pDX, IDC_BTN_MODEL_DATA_MODIFY, m_btnDataModify);
	DDX_Control(pDX, IDC_BTN_MODEL_CHANGE, m_btnModelChange);

	DDX_Control(pDX, IDC_EDIT_MODEL_NAME, m_edt_ModelTitle);
	DDX_Control(pDX, IDC_EDIT_GLASS_WIDTH, m_edt_GlassWidth);
	DDX_Control(pDX, IDC_EDIT_GLASS_HEIGHT, m_edt_GlassHeight);
	DDX_Control(pDX, IDC_EDIT_FRAME_WIDTH, m_edt_FrameWidth);
	DDX_Control(pDX, IDC_EDIT_FRAME_HEIGHT, m_edt_FrameHeight);
	DDX_Control(pDX, IDC_EDIT_MARK_PITCH_X, m_edt_FMarkPitch);
	DDX_Control(pDX, IDC_EDIT_INSP_GRAB_SECOND, m_edtSecondMarkPitchY);
	DDX_Control(pDX, IDC_EDIT_CALIB_COUNT_X, m_edt_CalibrationCountX);
	DDX_Control(pDX, IDC_EDIT_CALIB_COUNT_Y, m_edt_CalibrationCountY);
	DDX_Control(pDX, IDC_EDIT_CALIB_RANGE_X, m_edt_CalibrationRangeX);
	DDX_Control(pDX, IDC_EDIT_CALIB_RANGE_Y, m_edt_CalibrationRangeY);
	DDX_Control(pDX, IDC_EDIT_ROTATE_RANGE, m_edt_RotateRange);	
	DDX_Control(pDX, IDC_EDIT_ROTATE_COUNT, m_edt_RotateCount);
	DDX_Control(pDX, IDC_EDIT_ROTATE_STARTPOS, m_edt_RotateStartPos);
	DDX_Control(pDX, IDC_EDIT_ROTATE_ENDPOS, m_edt_RotateEndPos);
	DDX_Control(pDX, IDC_GROUP_CALIBRATION, m_grpCalibration);
	DDX_Control(pDX, IDC_GROUP_GLASS, m_grpGlass);
}


BEGIN_MESSAGE_MAP(CFormModelView, CFormView)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_VIEW_CONTROL, OnViewControl)
	ON_NOTIFY(TCN_SELCHANGE, IDC_MODEL_ALGOLIST, &CFormModelView::OnSelchangeTabList)
	ON_BN_CLICKED(IDC_BTN_MODEL_DATA_MODIFY, &CFormModelView::OnBnClickedBtnModelDataModify)
	ON_BN_CLICKED(IDC_BTN_MODEL_DATA_SAVE, &CFormModelView::OnBnClickedBtnModelDataSave)
	ON_BN_CLICKED(IDC_BTN_MODEL_CHANGE, &CFormModelView::OnBnClickedBtnModelChange)

	ON_NOTIFY(NM_CLICK, IDC_LIST_MODEL, &CFormModelView::OnNMClickListModel)
	ON_EN_SETFOCUS(IDC_EDIT_MARK_PITCH_X, &CFormModelView::OnEnSetfocusEditMarkPitchX)
	ON_EN_SETFOCUS(IDC_EDIT_INSP_GRAB_SECOND, &CFormModelView::OnEnSetfocusEditInspGrabSecond)
	ON_EN_SETFOCUS(IDC_EDIT_CALIB_COUNT_X, &CFormModelView::OnEnSetfocusEditCalibCountX)
	ON_EN_SETFOCUS(IDC_EDIT_CALIB_COUNT_Y, &CFormModelView::OnEnSetfocusEditCalibCountY)
	ON_EN_SETFOCUS(IDC_EDIT_CALIB_RANGE_X, &CFormModelView::OnEnSetfocusEditCalibRangeX)
	ON_EN_SETFOCUS(IDC_EDIT_CALIB_RANGE_Y, &CFormModelView::OnEnSetfocusEditCalibRangeY)
	ON_EN_SETFOCUS(IDC_EDIT_ROTATE_RANGE, &CFormModelView::OnEnSetfocusEditRotateRange)
	ON_EN_SETFOCUS(IDC_EDIT_ROTATE_COUNT, &CFormModelView::OnEnSetfocusEditRotateCount)
	ON_EN_SETFOCUS(IDC_EDIT_ROTATE_STARTPOS, &CFormModelView::OnEnSetfocusEditRotateStartPos)
	ON_EN_SETFOCUS(IDC_EDIT_ROTATE_ENDPOS, &CFormModelView::OnEnSetfocusEditRotateEndPos)
	ON_EN_SETFOCUS(IDC_EDIT_GLASS_WIDTH, &CFormModelView::OnEnSetfocusEditGlassWidth)
	ON_EN_SETFOCUS(IDC_EDIT_GLASS_HEIGHT, &CFormModelView::OnEnSetfocusEditGlassHeight)
	ON_EN_SETFOCUS(IDC_EDIT_FRAME_WIDTH, &CFormModelView::OnEnSetfocusEditFrameWidth)
	ON_EN_SETFOCUS(IDC_EDIT_FRAME_HEIGHT, &CFormModelView::OnEnSetfocusEditFrameHeight)
END_MESSAGE_MAP()


// CFormModelView 진단입니다.

#ifdef _DEBUG
void CFormModelView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CFormModelView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


void CFormModelView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	m_pMain = (CLET_AlignClientDlg *)AfxGetMainWnd();

	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);

	EditGroupInit(&m_grpGlass, "Panel", 15.f, COLOR_BTN_BODY);
	EditGroupInit(&m_grpCalibration, "Calibration", 15.f, COLOR_BTN_BODY);

	InitTitle(&m_LbModelViewTitle, "Model View", 24.f, RGB(64, 192, 64));
	InitTitle(&m_stt_ModelTitle, "Model", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_GlassWidth, "Glass Width (mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_GlassHeight, "Glass Height (mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_FrameWidth, "Frame Width(mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_FrameHeight, "Frame Height(mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_ModelPanelType, "Panel Type", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_FMarkPitch, "Align Key Pitch (mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_lblSecondGrab, "Sub Align Key Pitch (mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_CalibrationCountX, "Calibration Count X", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_CalibrationCountY, "Calibration Count Y", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_CalibrationRangeX, "Calib. Range X (mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_CalibrationRangeY, "Calib. Range Y (mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_RotateRange, "Rotate Range(˚), Count", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_RotateCount, "Rotate Start,End Pos", 12.f, COLOR_BTN_BODY);
	InitTitle(&m_lblCalibrationType, "Rotate Calc Type", 14.f, COLOR_BTN_BODY);

	MainButtonInit(&m_btnDataModify);
	MainButtonInit(&m_btnDataSave);
	MainButtonInit(&m_btnModelChange);

	EditButtonInit(&m_edt_ModelTitle, 20);
	EditButtonInit(&m_edt_GlassWidth, 20);
	EditButtonInit(&m_edt_GlassHeight, 20);
	EditButtonInit(&m_edt_FMarkPitch, 20);
	EditButtonInit(&m_edtSecondMarkPitchY, 20);	
	EditButtonInit(&m_edt_CalibrationCountX, 20);
	EditButtonInit(&m_edt_CalibrationCountY, 20);
	EditButtonInit(&m_edt_CalibrationRangeX, 20);
	EditButtonInit(&m_edt_CalibrationRangeY, 20);
	EditButtonInit(&m_edt_RotateRange, 20);
	EditButtonInit(&m_edt_RotateCount, 20);
	EditButtonInit(&m_edt_RotateStartPos, 20);
	EditButtonInit(&m_edt_RotateEndPos, 20);

	for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
	{
		c_TabModelPage[i] = new TabModelAlgoPage;
		c_TabModelPage[i]->m_nJobID = i;
	}

	CreateGrid();

	m_pMain->setDispModelName(m_pMain->getCurrModelName());
	m_edt_ModelTitle.SetText(m_pMain->getCurrModelName());

	initListCtrl();
	getModelList();

	showModelInfo(m_pMain->getCurrModelName());
	UpdateData(FALSE);
}

void CFormModelView::initListCtrl()
{
	LV_COLUMN lstCol;

	DWORD style = m_ListModelID.GetExtendedStyle();
	style |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	m_ListModelID.SetExtendedStyle(style);

	lstCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lstCol.fmt = LVCFMT_CENTER;

	lstCol.cx = 0;
	lstCol.iSubItem = 0;
	lstCol.pszText = "";
	m_ListModelID.InsertColumn(0, &lstCol);

	lstCol.cx = 60;
	lstCol.iSubItem = 1;
	lstCol.pszText = "NO.";
	m_ListModelID.InsertColumn(1, &lstCol);

	lstCol.cx = 270;
	lstCol.iSubItem = 2;
	lstCol.pszText = "MODEL NAME";
	m_ListModelID.InsertColumn(2, &lstCol);
}

BOOL CFormModelView::getModelList()
{
	BOOL bSuccess = FALSE;

	BOOL bFind = FALSE;
	CFileFind findModel;
	CString strModelFolder = m_pMain->m_strModelDir;
	CString strTempModel;

	m_ListModelID.DeleteAllItems();

	// Model Data Folder 유무 확인.
	if (PathFileExists(strModelFolder) == FALSE)
	{
		m_pMain->fnSetMessage(1, _T("Check Model Directory !!!"));
		return bSuccess;
	}

	bFind = findModel.FindFile(strModelFolder + _T("*.*"));
	int nModelCount = 0;
	LV_ITEM lstItem;
	lstItem.mask = LVIF_TEXT;

	char cData[MAX_PATH];

	while (bFind)
	{
		bFind = findModel.FindNextFile();

		if (findModel.IsDots() || !findModel.IsDirectory())	// '.', '..', '파일' 인 경우 continue;
			continue;

		strTempModel = findModel.GetFileName();	// 찾은 Directory

		lstItem.iItem = nModelCount;
		lstItem.iSubItem = 0;
		lstItem.pszText = "";
		m_ListModelID.InsertItem(&lstItem);

		lstItem.iSubItem = 1;
		sprintf(cData, "%03d", nModelCount + 1);
		lstItem.pszText = cData;
		m_ListModelID.SetItem(&lstItem);

		lstItem.iSubItem = 2;
		sprintf(cData, "%s", (LPCTSTR)strTempModel);
		lstItem.pszText = cData;
		m_ListModelID.SetItem(&lstItem);

		if (m_pMain->getDispModelName() == strTempModel)
		{
			m_ListModelID.SetRowBgColor(nModelCount, RGB(51, 153, 255), FALSE);
			m_ListModelID.SetRowTxtColor(nModelCount, RGB(255, 255, 255));
			m_ListModelID.SetRowStyle(nModelCount, LIS_BGCOLOR | LIS_TXTCOLOR);
		}

		nModelCount++;
	}

	bSuccess = TRUE;
	return bSuccess;
}

BOOL CFormModelView::deleteModelData(CString strPath)
{
	BOOL bSuccess = TRUE;

	try {
		BOOL bFind = FALSE;
		CFileFind findModel;
		CString strFolder = strPath;
		CString strTempModel;

		bFind = findModel.FindFile(strFolder + _T("\\*.*"));

		while (bFind)
		{
			bFind = findModel.FindNextFile();

			if (findModel.IsDots())	// '.', '..', '파일' 인 경우 continue;
				continue;

			if (findModel.IsDirectory())
			{
				deleteModelData(findModel.GetFilePath());
				::RemoveDirectoryA(findModel.GetFilePath());
			}
			else
				::DeleteFileA(findModel.GetFilePath());

			Delay(10);
		}

		::RemoveDirectoryA(strFolder);
	}
	catch (...)
	{
		bSuccess = FALSE;
	}

	return bSuccess;
}

void CFormModelView::OnNMClickListModel(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	POSITION pos = m_ListModelID.GetFirstSelectedItemPosition();
	if (pos == 0) return;

	for (int i = 0; i < m_ListModelID.GetItemCount(); i++)
	{
		m_ListModelID.SetRowBgColor(i, COLOR_WHITE);
		m_ListModelID.SetRowTxtColor(i, COLOR_BLACK);
		m_ListModelID.SetRowStyle(i, LIS_BGCOLOR | LIS_TXTCOLOR);
	}
	
	int nPos = m_ListModelID.GetNextSelectedItem(pos);

	m_ListModelID.SetRowBgColor(nPos, RGB(51, 153, 255), FALSE);
	m_ListModelID.SetRowTxtColor(nPos, RGB(255, 255, 255));
	m_ListModelID.SetRowStyle(nPos, LIS_BGCOLOR | LIS_TXTCOLOR);

	CString strModel = m_ListModelID.GetItemText(nPos, 2);
	m_edt_ModelTitle.SetText(strModel);
	m_pMain->setDispModelName(strModel);

	if (strModel != m_pMain->m_strCurrentModelName)		m_btnDataModify.SetEnable(FALSE);
	else												m_btnDataModify.SetEnable(TRUE);

	showModelInfo(strModel);

	UpdateData(FALSE);

	*pResult = 0;
}

HBRUSH CFormModelView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);
	if (pWnd->m_hWnd == this->m_hWnd) hbr = m_hbrBkg;

	if (nCtlColor == CTLCOLOR_LISTBOX || nCtlColor == CTLCOLOR_EDIT || nCtlColor == CTLCOLOR_MSGBOX)
	{
		pDC->SetTextColor(RGB(255, 0, 0));
		pDC->SetBkColor(RGB(64, 64, 64));
	}
	if (pWnd->GetDlgCtrlID() == IDC_COMBO_CALIBRATION_TYPE  || pWnd->GetDlgCtrlID() == IDC_COMBO_MODEL_PANEL_TYPE)
	{
		pDC->SetBkColor(COLOR_UI_BODY);
		pDC->SetTextColor(COLOR_WHITE);
	}

	return hbr;
}

void CFormModelView::InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}

void CFormModelView::MainButtonInit(CButtonEx *pbutton, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}

void CFormModelView::EditButtonInit(CEditEx *pbutton, int size, COLORREF color)
{
	pbutton->SetEnable(false);
	pbutton->SetSizeText(size);				// 글자 크기
	pbutton->SetStyleTextBold(true);		// 글자 스타일
	pbutton->SetTextMargins(10, 10);		// 글자 옵셋
	pbutton->SetColorText(color);	// 글자 색상
	pbutton->SetText(_T("0"));				// 글자 설정
}

void CFormModelView::CreateGrid()
{
	CString str;
	CRect rt;

	m_hSelectAlgoTab.GetWindowRect(&rt);

#ifndef JOB_INFO
	for (int i = 0; i < MAX_CAMERA; i++)
	{
		if (m_pMain->m_pAlgorithmInfo.getAlgorithmCount() <= i) continue;

		str.Format("%s", m_pMain->m_pAlgorithmInfo.getAlgorithmName(i).c_str());
		m_hSelectAlgoTab.InsertItem(i, str);

		c_TabModelPage[i]->Create(IDD_TAB_MODEL_ALGORITHM, &m_hSelectAlgoTab);
		c_TabModelPage[i]->SetWindowPos(NULL, 0, 25, rt.Width(), rt.Height(), i == 0 ? (SWP_SHOWWINDOW | SWP_NOZORDER) : SWP_NOZORDER);
	}
#else
	for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
	{
		str.Format("%s", m_pMain->vt_job_info[i].job_name.c_str());
		m_hSelectAlgoTab.InsertItem(i, str);

		c_TabModelPage[i]->Create(IDD_TAB_MODEL_ALGORITHM, &m_hSelectAlgoTab);
		c_TabModelPage[i]->SetWindowPos(NULL, 0, 25, rt.Width(), rt.Height(), i == 0 ? (SWP_SHOWWINDOW | SWP_NOZORDER) : SWP_NOZORDER);
	}
#endif

	c_pWnd = c_TabModelPage[0];
	m_hSelectAlgoTab.SetCurSel(0);
}

void CFormModelView::OnSelchangeTabList(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;
	
	if (m_btnDataModify.GetColorBkg() == COLOR_BTN_SELECT)
	{
		m_hSelectAlgoTab.SetCurSel(m_nOldTabSelect);
		AfxMessageBox("Data Save First!!");
		return;
	}


	if (c_pWnd != NULL)
	{
		c_pWnd->ShowWindow(SW_HIDE);
		c_pWnd = NULL;
	}

	int index = m_hSelectAlgoTab.GetCurSel();

	c_TabModelPage[index]->ShowWindow(SW_SHOW);
	c_pWnd = c_TabModelPage[index];
	m_nJobID = index;

	m_btnDataModify.SetColorBkg(255, RGB(64, 64, 64));
	m_btnDataSave.SetColorBkg(255, RGB(64, 64, 64));
	m_btnModelChange.SetColorBkg(255, RGB(64, 64, 64));

	::SendMessage(c_pWnd->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_MV_LOAD_ALGO_DATA, 0);

	CString strModel = m_edt_ModelTitle.GetText();
	OnViewControl(MSG_MV_DISP_MODEL_INFO, (LPARAM)&strModel);

	// KBJ 2022-03-02
	c_TabModelPage[m_nOldTabSelect]->m_pInspectionDlg->ShowWindow(SW_HIDE);
	if (c_TabModelPage[index]->m_pInspectionDlg->bUse == TRUE)
		c_TabModelPage[index]->m_pInspectionDlg->ShowWindow(SW_SHOW);

	m_nOldTabSelect = index;
}

LRESULT CFormModelView::OnViewControl(WPARAM wParam, LPARAM lParam)
{
	LRESULT lRet = 0;

	switch ((int)wParam) {
	case MSG_MV_DISP_MODEL_LIST:
		getModelList();
		break;
	case MSG_MV_DELETE_MODEL:
	{
		CString strDeleteModel = *(CString *)lParam;

		deleteModelData(m_pMain->m_strModelDir + strDeleteModel);
		getModelList();

		if (strDeleteModel == m_pMain->getDispModelName())
		{
			m_pMain->setDispModelName("");
			m_edt_ModelTitle.SetText("");
		}

		UpdateData(FALSE);
	}
	break;
	case MSG_MV_DISP_MODEL_INFO:
	{
		CString strModel = *((CString *)lParam);
		showModelInfo(strModel);
	}
	break;
	case MSG_MV_DISP_MODEL_ID:
	{
		if (lParam != NULL) m_edt_ModelTitle.SetText(*(CString*)lParam);
		UpdateData(FALSE);
	}
	break;
	//case MSG_FV_INPUT_DATA:
	//{
	//	int nID = (int)lParam;

	//	CString str;
	//	GetDlgItem(m_nResID[nID])->GetWindowText(str);

	//	CKeyPadDlg dlg;
	//	if (dlg.DoModal() != IDOK)
	//		return 0;

	//	CString strNumber;
	//	dlg.GetValue(str);
	//	GetDlgItem(m_nResID[nID])->SetWindowTextA(str);
	//}
	//break;
	case MSG_MV_SEND_CURR_MODEL:
	{
		char cModelData[20] = { NULL, };
		long lData[20]{ NULL, };
		int nCount = 0;
		for (int i = 0; i < 10; i++)
		{
			if (m_pMain->m_strCurrentModelName[i] == NULL)
			{
				nCount = i;
				break;
			}
			else
			{
				cModelData[i] = m_pMain->m_strCurrentModelName[i];
			}
		}
		for (int count = nCount; count < 20; count++)
		{
			cModelData[count] = NULL;
		}
		for (int i = 0; i < 20; i++)
		{
			lData[i] = cModelData[i];
		}
		g_CommPLC.SetWord(PLC_WD_CURRENT_PPID_00, 20, lData);
	}
	break;
	default: break;
	}

	return lRet;
}

void CFormModelView::enableEditControls(BOOL bEnable)
{
	GetDlgItem(IDC_EDIT_GLASS_WIDTH)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_GLASS_HEIGHT)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_FRAME_WIDTH)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_FRAME_HEIGHT)->EnableWindow(bEnable);
	GetDlgItem(IDC_COMBO_CALIBRATION_TYPE)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_INSP_GRAB_SECOND)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_CALIB_RANGE_X)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_CALIB_RANGE_Y)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_CALIB_COUNT_X)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_CALIB_COUNT_Y)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_ROTATE_RANGE)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_ROTATE_COUNT)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_ROTATE_STARTPOS)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_ROTATE_ENDPOS)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_MARK_PITCH_X)->EnableWindow(bEnable);
	GetDlgItem(IDC_COMBO_MODEL_PANEL_TYPE)->EnableWindow(bEnable);	

	m_btnDataSave.SetEnable(bEnable);
	m_btnModelChange.SetEnable(!bEnable);
	m_ListModelID.EnableWindow(!bEnable);
	//m_hSelectAlgoTab.EnableWindow(!bEnable);

	if (bEnable)
	{
		m_btnDataModify.SetColorBkg(255, COLOR_BTN_SELECT);
		m_btnDataModify.SetEnable(TRUE);
	}	
	else
	{
		m_btnDataModify.SetColorBkg(255, COLOR_BTN_BODY);
	}

}

void CFormModelView::OnBnClickedBtnModelDataModify()
{
	if (m_pMain->fnSetMessage(2, "Do You Modify the Data?") != TRUE)	return;
	enableEditControls(TRUE);
	
	::SendMessage(c_pWnd->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_MV_ENABLE_CONTROL, TRUE);

	// KBJ 2022-03-16 SetInspPara VIew into the Model View
	c_TabModelPage[m_nJobID]->m_pInspectionDlg->EnableWindow(true);
	c_TabModelPage[m_nJobID]->m_pInspectionDlg->m_ctlPropGrid.SetCustomColors(COLOR_BTN_BODY, COLOR_WHITE, COLOR_UI_BODY, COLOR_WHITE, COLOR_BTN_BODY, COLOR_WHITE, COLOR_DDARK_GRAY);
	c_TabModelPage[m_nJobID]->m_pInspectionDlg->m_ctlPropGrid.Invalidate();
	//kmb 220905 check Master Parameter 
	c_TabModelPage[m_nJobID]->m_pInspectionDlg->ChkMasterParam(m_pMain->m_nLogInUserLevelType);
}

void CFormModelView::OnBnClickedBtnModelDataSave()
{
	if (m_pMain->fnSetMessage(2, "Do You Save the Data?") != TRUE)	return;
	
	//lhj add 220503 //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 파일에서 데이터 가져오기
	CString strPath2;
	int nJob2 = m_hSelectAlgoTab.GetCurSel();
	strPath2.Format("%s%s\\%s", m_pMain->m_strModelDir, m_pMain->vt_job_info[nJob2].model_info.getModelID(), m_pMain->vt_job_info[nJob2].job_name.c_str());
	m_pMain->readInspSpecParaSameCenterItem(nJob2, strPath2);
	m_pMain->readAlignInfoDataSameCenterItem(nJob2, strPath2);

	//UI에서 데이터 가져오기
	::SendMessage(c_pWnd->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_MV_PARAM_ALGO_DATA_SAME_CENTER_ITEM, 0);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// KBJ 2022-03-16 SetInspPara VIew into the Model View
	c_TabModelPage[m_nJobID]->m_pInspectionDlg->EnableWindow(false);
	c_TabModelPage[m_nJobID]->m_pInspectionDlg->m_ctlPropGrid.SetCustomColors(COLOR_BTN_BODY, COLOR_GRAY, COLOR_UI_BODY, COLOR_GRAY, COLOR_BTN_BODY, COLOR_GRAY, COLOR_DDARK_GRAY);
	c_TabModelPage[m_nJobID]->m_pInspectionDlg->m_ctlPropGrid.Invalidate();
	c_TabModelPage[m_nJobID]->m_pInspectionDlg->OnBnClickedOk();

	::SendMessage(c_pWnd->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_MV_PARAM_ALGO_DATA, 0);

	CSimpleSplashWnd _splash(this, IDB_BMP_LOADING);

	_splash.SetStatusText(_T("  Apply Model Data.."), 10);
	Sleep(100);

	applyModelData();

	_splash.SetStatusText(_T("  Save Algo Data.."), 30);
	Sleep(100);
	
	::SendMessage(c_pWnd->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_MV_SAVE_ALGO_DATA, 0);


#ifndef JOB_INFO	
	CString strPath;
	strPath.Format("%s%s", m_pMain->m_strModelDir, m_pMain->getModel().getModelID());
	m_pMain->saveGlassInfoData(&m_pMain->getModel().getGlassInfo(), strPath);
	m_pMain->saveAlignInfoData(&m_pMain->getModel().getAlignInfo(), strPath, FALSE);
	m_pMain->saveMachineInfoData(&m_pMain->getModel().getMachineInfo(), strPath);
	//m_pMain->saveAlignMeasureInpInfoData(&m_pMain->getModel().getMeasureInpInfo(), strPath);
	//m_pMain->GetPrealign().setAlignmentType(m_pMain->getModel().getMachineInfo().getAlignmentType());


#else
	CString strPath;
	int nJob = m_hSelectAlgoTab.GetCurSel();	
	strPath.Format("%s%s\\%s", m_pMain->m_strModelDir, m_pMain->vt_job_info[nJob].model_info.getModelID(), m_pMain->vt_job_info[nJob].job_name.c_str());

	_splash.SetStatusText(_T("  Save Glass Info Data.."), 50);
	Sleep(100);
	m_pMain->saveGlassInfoData(&m_pMain->vt_job_info[nJob].model_info.getGlassInfo(), strPath);
	
	_splash.SetStatusText(_T("  Save Align Info Data.."), 70);
	Sleep(100);
	m_pMain->saveAlignInfoData(nJob, &m_pMain->vt_job_info[nJob].model_info.getAlignInfo(), strPath, FALSE);

	_splash.SetStatusText(_T("  Save Machine Info Data.."), 80);
	Sleep(100);
	m_pMain->saveMachineInfoData(nJob, &m_pMain->vt_job_info[nJob].model_info.getMachineInfo(), strPath);

	_splash.SetStatusText(_T("  Save Light Info Data.."), 90);
	Sleep(100);
	m_pMain->saveLightInfoData(nJob, &m_pMain->vt_job_info[nJob].model_info.getLightInfo(), strPath);
	
	// dh.jung 2021-08-03 add
	_splash.SetStatusText(_T("  Save Insp Spec & Para Info Data.."), 95);
	Sleep(100);
	m_pMain->saveInspSpecPara(nJob, &m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo(), strPath);
	//m_pMain->saveInspSpecPara(nJob, &m_pMain->vt_job_info[m_nJobID].insp_spec, strPath);
	
	// end

	m_pMain->GetPrealign(nJob).setAlignmentType(m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getAlignmentType());
#endif

	enableEditControls(FALSE);
	::SendMessage(c_pWnd->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_MV_ENABLE_CONTROL, FALSE);

	if(m_pMain->vt_job_info[nJob].algo_method== CLIENT_TYPE_ELB_CENTER_ALIGN)	m_pMain->draw_NozzleY_Distance(nJob);

	_splash.SetStatusText(_T("  Save Data Complete!!"), 100);
	Sleep(100);

}

void CFormModelView::OnBnClickedBtnModelChange()
{
	/*
	CString str;
	str.Format("Do You Change the Model '%s'?", m_pMain->getDispModelName());
	BOOL bRet = m_pMain->fnSetMessage(2, str);
	if (bRet != TRUE) return;

	if (m_pMain->getDispModelName() == "")
	{
		m_pMain->fnSetMessage(1, "Select the Model to Change");
		return;
	}
	*/
	CString str;
	str.Format("Do You Change the Model '%s'?", m_pMain->getDispModelName());
	BOOL bRet = m_pMain->fnSetMessage(2, str);
	if (bRet != TRUE) return;

	if (m_pMain->allModelChange(m_pMain->getDispModelName()) != TRUE)
	{
		m_pMain->fnSetMessage(1, "Model Change Failed");
		return;
	}

	SendMessage(WM_VIEW_CONTROL, MSG_MV_DISP_MODEL_INFO, (LPARAM)& m_pMain->getDispModelName());

	m_pMain->fnSetMessage(1, "Model Change Complete");
}

void CFormModelView::showModelInfo(CString strModel)
{
	CString strModelFolder;
	CFileFind findModel;

	strModelFolder.Format("%s%s\\%s\\", m_pMain->m_strModelDir, strModel, m_pMain->vt_job_info[m_nJobID].job_name.c_str());

	if (_access(strModelFolder, 0) != 0)
	{
		CString strError;
		strError = "'" + strModel + "' Model is Not Exist.";
		m_pMain->fnSetMessage(1, strError);
		return;
	}

	showGlassInfo(strModel);
	showMachineInfo(strModel);	
	showAlignInfo(strModel);
	showInspSpecPara(strModel);		// dh.jung 2021-08-02 add

	::SendMessage(c_pWnd->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_MV_DISP_MODEL_INFO, (LPARAM)& strModel);
}

void CFormModelView::OnEnSetfocusEditGlassWidth()
{
	setEditBox(IDC_EDIT_GLASS_WIDTH);
}

void CFormModelView::OnEnSetfocusEditGlassHeight()
{
	setEditBox(IDC_EDIT_GLASS_HEIGHT);
}

void CFormModelView::OnEnSetfocusEditFrameWidth()
{
	setEditBox(IDC_EDIT_FRAME_WIDTH);
}

void CFormModelView::OnEnSetfocusEditFrameHeight()
{
	setEditBox(IDC_EDIT_FRAME_HEIGHT);
}

void CFormModelView::OnEnSetfocusEditMarkPitchX()
{
	setEditBox(IDC_EDIT_MARK_PITCH_X);
}

void CFormModelView::OnEnSetfocusEditInspGrabSecond()
{
	setEditBox(IDC_EDIT_INSP_GRAB_SECOND);
}

void CFormModelView::OnEnSetfocusEditCalibCountX()
{
	setEditBox(IDC_EDIT_CALIB_COUNT_X);
}

void CFormModelView::OnEnSetfocusEditCalibCountY()
{
	setEditBox(IDC_EDIT_CALIB_COUNT_Y);
}

void CFormModelView::OnEnSetfocusEditCalibRangeX()
{
	setEditBox(IDC_EDIT_CALIB_RANGE_X);
}

void CFormModelView::OnEnSetfocusEditCalibRangeY()
{
	setEditBox(IDC_EDIT_CALIB_RANGE_Y);
}

void CFormModelView::OnEnSetfocusEditRotateRange()
{
	setEditBox(IDC_EDIT_ROTATE_RANGE);
}
void CFormModelView::OnEnSetfocusEditRotateCount()
{
	setEditBox(IDC_EDIT_ROTATE_COUNT);
}

void CFormModelView::OnEnSetfocusEditRotateStartPos()
{
	setEditBox(IDC_EDIT_ROTATE_STARTPOS);
}
void CFormModelView::OnEnSetfocusEditRotateEndPos()
{
	setEditBox(IDC_EDIT_ROTATE_ENDPOS);
}

void CFormModelView::setEditBox(int nID)
{
	GetDlgItem(IDC_STATIC_MODEL_NAME)->SetFocus();

	CKeyPadDlg dlg;
	CString strNumber;
	GetDlgItem(nID)->GetWindowText(strNumber);
	dlg.SetValueString(false, strNumber);

	if (dlg.DoModal() != IDOK)	return;

	dlg.GetValue(strNumber);
	GetDlgItem(nID)->SetWindowTextA(strNumber);	
}

void CFormModelView::showGlassInfo(CString strModel)
{
	CString strData;
	CString strModelPath;
	strModelPath.Format("%s%s\\%s", m_pMain->m_strModelDir, strModel, m_pMain->vt_job_info[m_nJobID].job_name.c_str());

	CGlassInfo glassInfo;
	m_pMain->readGlassInfoData(&glassInfo, strModelPath);

	strData.Format("%.2f", glassInfo.getGlassWidth());
	GetDlgItem(IDC_EDIT_GLASS_WIDTH)->SetWindowTextA(strData);

	strData.Format("%.2f", glassInfo.getGlassHeight());
	GetDlgItem(IDC_EDIT_GLASS_HEIGHT)->SetWindowTextA(strData);

	strData.Format("%.2f", glassInfo.getFrameWidth());
	GetDlgItem(IDC_EDIT_FRAME_WIDTH)->SetWindowTextA(strData);

	strData.Format("%.2f", glassInfo.getFrameHeight());
	GetDlgItem(IDC_EDIT_FRAME_HEIGHT)->SetWindowTextA(strData);


	//strData.Format("%.2f", glassInfo.getFiducialMarkPitchX());
	//GetDlgItem(IDC_EDIT_MARK_PITCH_X)->SetWindowTextA(strData);

	((CComboBox*)GetDlgItem(IDC_COMBO_MODEL_PANEL_TYPE))->SetCurSel(glassInfo.getModelPanelType());
}

void CFormModelView::showMachineInfo(CString strModel)
{
	CString strData;
	CString strModelPath;
	int algo = m_hSelectAlgoTab.GetCurSel();

	strModelPath.Format("%s%s\\%s", m_pMain->m_strModelDir, strModel, m_pMain->vt_job_info[algo].job_name.c_str());

	CMachineInfo machineInfo;
	m_pMain->readMachineInfoData(algo, &machineInfo, strModelPath);

	strData.Format("%d", machineInfo.getCalibrationCountX());
	GetDlgItem(IDC_EDIT_CALIB_COUNT_X)->SetWindowTextA(strData);

	strData.Format("%d", machineInfo.getCalibrationCountY());
	GetDlgItem(IDC_EDIT_CALIB_COUNT_Y)->SetWindowTextA(strData);

	strData.Format("%.3f", machineInfo.getCalibrationRangeX());
	GetDlgItem(IDC_EDIT_CALIB_RANGE_X)->SetWindowTextA(strData);

	strData.Format("%.3f", machineInfo.getCalibrationRangeY());
	GetDlgItem(IDC_EDIT_CALIB_RANGE_Y)->SetWindowTextA(strData);

	strData.Format("%.1f", machineInfo.getRotateRange());
	GetDlgItem(IDC_EDIT_ROTATE_RANGE)->SetWindowTextA(strData);
	
	strData.Format("%d", machineInfo.getRotateCount());
	GetDlgItem(IDC_EDIT_ROTATE_COUNT)->SetWindowTextA(strData);

	strData.Format("%.1f", machineInfo.getRotateStartPos());
	GetDlgItem(IDC_EDIT_ROTATE_STARTPOS)->SetWindowTextA(strData);

	strData.Format("%.1f", machineInfo.getRotateEndPos());
	GetDlgItem(IDC_EDIT_ROTATE_ENDPOS)->SetWindowTextA(strData);	

	((CComboBox*)GetDlgItem(IDC_COMBO_CALIBRATION_TYPE))->SetCurSel(machineInfo.getCalibrationType());	
}

void CFormModelView::showAlignInfo(CString strModel)
{
#ifndef JOB_INFO
	CString strData;
	CString strModelPath;
	strModelPath.Format("%s%s", m_pMain->m_strModelDir, strModel);

	CAlignInfo alignInfo;
	m_pMain->readAlignInfoData(&alignInfo, strModelPath);

	strData.Format("%.1f", alignInfo.getFiducialSecondMarkPitchY(0));
	GetDlgItem(IDC_EDIT_INSP_GRAB_SECOND)->SetWindowTextA(strData);	
#else
	CString strData;
	CString strModelPath;

	int nAlgo = m_hSelectAlgoTab.GetCurSel();

	strModelPath.Format("%s%s\\%s", m_pMain->m_strModelDir, strModel, m_pMain->vt_job_info[nAlgo].job_name.c_str());

	CAlignInfo alignInfo;
	m_pMain->readAlignInfoData(nAlgo, &alignInfo, strModelPath);


	strData.Format("%.3f", alignInfo.getFiducialMarkPitchX());
	GetDlgItem(IDC_EDIT_MARK_PITCH_X)->SetWindowTextA(strData);

	strData.Format("%.3f", alignInfo.getFiducialSecondMarkPitchY());
	GetDlgItem(IDC_EDIT_INSP_GRAB_SECOND)->SetWindowTextA(strData);
#endif
}

// dh.jung 2021-08-02 add 
void CFormModelView::showInspSpecPara(CString strModel)
{
#ifndef JOB_INFO
	CString strData;
	CString strModelPath;
	strModelPath.Format("%s%s", m_pMain->m_strModelDir, strModel);

	CAlignInfo alignInfo;
	m_pMain->readAlignInfoData(&alignInfo, strModelPath);

	strData.Format("%.1f", alignInfo.getFiducialSecondMarkPitchY(0));
	GetDlgItem(IDC_EDIT_INSP_GRAB_SECOND)->SetWindowTextA(strData);
#else
	CString strData;
	CString strModelPath;

	int nAlgo = m_hSelectAlgoTab.GetCurSel();

	strModelPath.Format("%s%s\\%s", m_pMain->m_strModelDir, strModel, m_pMain->vt_job_info[nAlgo].job_name.c_str());

	CInspSpecPara* pInspSpec = &m_pMain->vt_job_info[m_nJobID].model_info.getInspSpecParaInfo();
	m_pMain->readInspSpecPara(nAlgo, pInspSpec, strModelPath);

	// hsj 2023-01-03 grid갱신
	c_TabModelPage[nAlgo]->m_pInspectionDlg->m_ctlPropGrid.ResetProperty();
	c_TabModelPage[nAlgo]->m_pInspectionDlg->InItDialog();
	c_TabModelPage[nAlgo]->m_pInspectionDlg->EnableWindow(false);
	c_TabModelPage[nAlgo]->m_pInspectionDlg->m_ctlPropGrid.SetCustomColors(COLOR_BTN_BODY, COLOR_WHITE, COLOR_UI_BODY, COLOR_WHITE, COLOR_BTN_BODY, COLOR_WHITE, COLOR_DDARK_GRAY);
	c_TabModelPage[nAlgo]->m_pInspectionDlg->m_ctlPropGrid.ExpandAll();
	c_TabModelPage[nAlgo]->m_pInspectionDlg->m_ctlPropGrid.Invalidate();

#endif
}

void CFormModelView::updateAlignInfoData()
{
	showModelInfo(m_pMain->getCurrModelName());	
}

void CFormModelView::applyModelData()
{
	CString str;
	int nAlgo = m_hSelectAlgoTab.GetCurSel();
	CModel *pModel = &m_pMain->vt_job_info[nAlgo].model_info;

	GetDlgItem(IDC_EDIT_CALIB_COUNT_X)->GetWindowTextA(str);
	pModel->getMachineInfo().setCalibrationCountX(atoi(str));

	GetDlgItem(IDC_EDIT_CALIB_COUNT_Y)->GetWindowTextA(str);
	pModel->getMachineInfo().setCalibrationCountY(atoi(str));

	GetDlgItem(IDC_EDIT_CALIB_RANGE_X)->GetWindowTextA(str);
	pModel->getMachineInfo().setCalibrationRangeX(atof(str));

	GetDlgItem(IDC_EDIT_CALIB_RANGE_Y)->GetWindowTextA(str);
	pModel->getMachineInfo().setCalibrationRangeY(atof(str));

	GetDlgItem(IDC_EDIT_ROTATE_RANGE)->GetWindowTextA(str);
	pModel->getMachineInfo().setRotateRange(atof(str));

	GetDlgItem(IDC_EDIT_ROTATE_COUNT)->GetWindowTextA(str);
	pModel->getMachineInfo().setRotateCount(atoi(str));

	GetDlgItem(IDC_EDIT_ROTATE_STARTPOS)->GetWindowTextA(str);
	pModel->getMachineInfo().setRotateStartPos(atof(str));

	GetDlgItem(IDC_EDIT_ROTATE_ENDPOS)->GetWindowTextA(str);
	pModel->getMachineInfo().setRotateEndPos(atof(str));

	GetDlgItem(IDC_EDIT_MARK_PITCH_X)->GetWindowTextA(str);
	//pModel->getGlassInfo().setFiducialMarkPitchX(atof(str));
	pModel->getAlignInfo().setFiducialMarkPitchX(atof(str));

	GetDlgItem(IDC_EDIT_INSP_GRAB_SECOND)->GetWindowTextA(str);
	pModel->getAlignInfo().setFiducialSecondMarkPitchY(atof(str));

	GetDlgItem(IDC_EDIT_GLASS_WIDTH)->GetWindowTextA(str);
	pModel->getGlassInfo().setGlassWidth(atof(str));

	GetDlgItem(IDC_EDIT_GLASS_HEIGHT)->GetWindowTextA(str);
	pModel->getGlassInfo().setGlassHeight(atof(str));

	GetDlgItem(IDC_EDIT_FRAME_WIDTH)->GetWindowTextA(str);
	pModel->getGlassInfo().setFrameWidth(atof(str));

	GetDlgItem(IDC_EDIT_FRAME_HEIGHT)->GetWindowTextA(str);
	pModel->getGlassInfo().setFrameHeight(atof(str));

	int n = ((CComboBox*)GetDlgItem(IDC_COMBO_MODEL_PANEL_TYPE))->GetCurSel();
	pModel->getGlassInfo().setModelPanelType(n);

	n = ((CComboBox*)GetDlgItem(IDC_COMBO_CALIBRATION_TYPE))->GetCurSel();
	pModel->getMachineInfo().setCalibrationType(n);
}

void CFormModelView::EditGroupInit(CGroupEx *pGroup, CString str, float size, COLORREF color)
{
	pGroup->SetSizeText(size);
	pGroup->SetColorBkg(255, 96, 96, 96);
	pGroup->SetColorBorder(255, 96, 96, 96);
	pGroup->SetText(str);
	pGroup->SetColorText(255, 255, 255, 255);
	pGroup->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}


BOOL CFormModelView::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_SYSKEYDOWN)
		return TRUE;

	return CFormView::PreTranslateMessage(pMsg);
}
