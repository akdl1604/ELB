// DlgCreateModel.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "DlgCreateModel.h"
#include "KeyPadDlg.h"
#include "LET_AlignClientDlg.h"
#include "OnscreenKeyboardDlg.h"
#include "VirtualKeyBoard/VirtualKeyBoard.h"


// DlgCreateModel.cpp : ���� �����Դϴ�.
//


IMPLEMENT_DYNAMIC(CDlgCreateModel, CDialog)

CDlgCreateModel::CDlgCreateModel(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgCreateModel::IDD, pParent)
	, nCopyExistModel(TRUE)
{

	m_strEditModel = _T("");
}

CDlgCreateModel::~CDlgCreateModel()
{
}

void CDlgCreateModel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LIST_MODEL, m_ListModelID);
	DDX_Control(pDX, IDC_STATIC_MODEL_INFO, m_stt_ModelInfo);
	DDX_Control(pDX, IDC_STATIC_MODEL_NAME, m_stt_Model_Name);
	DDX_Control(pDX, IDC_STATIC_INCH_NAME, m_stt_Inch_Name);
	DDX_Control(pDX, IDC_STATIC_GLASS_WIDTH_NAME, m_stt_GlassWidth_Name);
	DDX_Control(pDX, IDC_STATIC_GLASS_HEIGHT_NAME, m_stt_GlassHeight_Name);

	DDX_Text(pDX, IDC_EDIT_CREATE_MODEL, m_strEditModel);
	DDX_Text(pDX, IDC_EDIT_CREATE_INCH, m_strEditInch);
	DDX_Text(pDX, IDC_EDIT_CREATE_GLASS_WIDTH, m_strEditGlassWidth);
	DDX_Text(pDX, IDC_EDIT_CREATE_GLASS_HEIGHT, m_strEditGlassHeight);

	DDX_Control(pDX, IDC_EDIT_CREATE_MODEL, m_edt_ModelTitle);
	DDX_Control(pDX, IDC_EDIT_CREATE_INCH, m_edt_GlassInch);
	DDX_Control(pDX, IDC_EDIT_CREATE_GLASS_WIDTH, m_edt_GlassWidth);
	DDX_Control(pDX, IDC_EDIT_CREATE_GLASS_HEIGHT, m_edt_GlassHeight);
	DDX_Check(pDX, IDC_CHECK_COPY, nCopyExistModel);

	DDX_Control(pDX, IDC_BTN_CLOSE, m_btnModelClose);
	DDX_Control(pDX, IDC_BTN_CREATE, m_btnCreateModel);
}


BEGIN_MESSAGE_MAP(CDlgCreateModel, CDialog)
	ON_BN_CLICKED(IDC_BTN_CREATE, &CDlgCreateModel::OnBnClickedBtnCreate)
	ON_BN_CLICKED(IDC_BTN_CLOSE, &CDlgCreateModel::OnBnClickedBtnClose)

	ON_EN_SETFOCUS(IDC_EDIT_CREATE_INCH, &CDlgCreateModel::OnEnSetfocusEditInch)
	ON_EN_SETFOCUS(IDC_EDIT_CREATE_GLASS_WIDTH, &CDlgCreateModel::OnEnSetfocusEditGlassWidth)
	ON_EN_SETFOCUS(IDC_EDIT_CREATE_GLASS_HEIGHT, &CDlgCreateModel::OnEnSetfocusEditGlassHeight)
	ON_WM_CTLCOLOR()
	ON_EN_SETFOCUS(IDC_EDIT_CREATE_MODEL, &CDlgCreateModel::OnEnSetfocusEditCreateModel)
	ON_NOTIFY(NM_CLICK, IDC_LIST_MODEL, &CDlgCreateModel::OnNMClickListModel)
END_MESSAGE_MAP()


// CDlgCreateModel �޽��� ó�����Դϴ�.

void CDlgCreateModel::MainButtonInit(CButtonEx *pbutton, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}


void CDlgCreateModel::OnNMClickListModel(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
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

	if (nPos >= 0)			m_SelstrModel = m_ListModelID.GetItemText(m_nSelPos, 2);
	m_nSelPos = nPos;

	*pResult = 0;
}


void CDlgCreateModel::OnBnClickedBtnCreate()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);

	if (m_nSelPos >= 0)
	{
		m_SelstrModel = m_ListModelID.GetItemText(m_nSelPos, 2);
	}
	else
	{
		if (nCopyExistModel)
		{
			m_pMain->fnSetMessage(1, _T("First  => Select Copy Model ! !!!"));
			return;
		}
		if (m_ListModelID.GetItemCount() != 0)
		{
			m_nSelPos = 0;
			m_SelstrModel = m_ListModelID.GetItemText(m_nSelPos, 2);
		}
	}

	//m_pMain->m_pKeyboardDlg->ShowWindow(SW_HIDE);

	CDialog::OnOK();
}


void CDlgCreateModel::OnBnClickedBtnClose()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

	CDialog::OnCancel();
}

HBRUSH CDlgCreateModel::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	if (pWnd->m_hWnd == this->m_hWnd) hbr = m_hbrBkg;

	return hbr;
}
BOOL CDlgCreateModel::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);
	// TODO:  ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.

	m_pMain = (CLET_AlignClientDlg*)AfxGetMainWnd();

	InitTitle(&m_stt_ModelInfo, "Model Information", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_Model_Name, "Model", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_Inch_Name, "Inch ( \" )", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_GlassWidth_Name, "Glass Width(mm)", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stt_GlassHeight_Name, "Glass Height(mm)", 14.f, COLOR_BTN_BODY);	

	EditButtonInit(&m_edt_ModelTitle, 20);
	EditButtonInit(&m_edt_GlassInch, 20);
	EditButtonInit(&m_edt_GlassWidth, 20);
	EditButtonInit(&m_edt_GlassHeight, 20);
	MainButtonInit(&m_btnCreateModel);
	MainButtonInit(&m_btnModelClose);

	initListCtrl();
	getModelList();

	return TRUE;  // return TRUE unless you set the focus to a control
	// ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}

void CDlgCreateModel::InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}

void CDlgCreateModel::EditButtonInit(CEditEx *pbutton, int size, COLORREF color)
{
	pbutton->SetSizeText(size);				// ���� ũ��
	pbutton->SetStyleTextBold(true);		// ���� ��Ÿ��
	pbutton->SetTextMargins(10, 10);		// ���� �ɼ�
	pbutton->SetColorText(color);	// ���� ����
	pbutton->SetText(_T("0"));				// ���� ����
}

void CDlgCreateModel::initListCtrl()
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

	lstCol.cx = 240;
	lstCol.iSubItem = 2;
	lstCol.pszText = "MODEL NAME";
	m_ListModelID.InsertColumn(2, &lstCol);
}

BOOL CDlgCreateModel::getModelList()
{
	BOOL bSuccess = TRUE;

	try {
		BOOL bFind = FALSE;
		CFileFind findModel;
		CString strModelFolder = m_pMain->m_strModelDir;
		CString strTempModel;

		m_ListModelID.DeleteAllItems();

		// Model Data Folder ���� Ȯ��.
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

			if (findModel.IsDots() || !findModel.IsDirectory())	// '.', '..', '����' �� ��� continue;
				continue;

			strTempModel = findModel.GetFileName();	// ã�� Directory

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

			// Model Data Folder ���� Ȯ��.
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

				if (findModel.IsDots() || !findModel.IsDirectory())	// '.', '..', '����' �� ��� continue;
					continue;

				strTempModel = findModel.GetFileName();	// ã�� Directory

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
					m_SelstrModel = strTempModel;
					m_nSelPos = nModelCount;
				}

				nModelCount++;
			}
		}
	}
	catch (...)
	{
		bSuccess = FALSE;
	}

	return bSuccess;
}

//void CDlgCreateModel::OnEnSetfocusEditModel()
//{
//	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
//	//SetFocus();
//
//
//	//CKeyPadDlg dlg;
//	//if( dlg.DoModal() != IDOK )
//	//	return;
//	//CString strNumber;
//	//dlg.GetValue(strNumber) ;
//	//GetDlgItem(IDC_EDIT_CREATE_MODEL)->SetWindowTextA( strNumber);
//
//	m_pMain->m_pKeyboardDlg->ShowWindow(SW_SHOW);
//}


void CDlgCreateModel::OnEnSetfocusEditInch()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	SetFocus();

	CKeyPadDlg dlg;

	if (dlg.DoModal() != IDOK)
		return;

	CString strNumber;
	dlg.GetValue(strNumber);
	GetDlgItem(IDC_EDIT_CREATE_INCH)->SetWindowTextA(strNumber);
}


void CDlgCreateModel::OnEnSetfocusEditGlassWidth()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	SetFocus();

	CKeyPadDlg dlg;

	if (dlg.DoModal() != IDOK)
		return;
	CString strNumber;
	dlg.GetValue(strNumber);
	GetDlgItem(IDC_EDIT_CREATE_GLASS_WIDTH)->SetWindowTextA(strNumber);
}


void CDlgCreateModel::OnEnSetfocusEditGlassHeight()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	SetFocus();

	CKeyPadDlg dlg;
	if (dlg.DoModal() != IDOK)
		return;
	CString strNumber;
	dlg.GetValue(strNumber);
	GetDlgItem(IDC_EDIT_CREATE_GLASS_HEIGHT)->SetWindowTextA(strNumber);
}


CModel CDlgCreateModel::fnCreateModelData(int algo)
{
//	CString dstModel = m_pMain->m_strModelDir + m_strEditModel;
//	CString dst_model_path;
//	if (_access(dstModel, 0) != 0)			CreateDirectory(dstModel, 0);
//
//	dst_model_path.Format("%s%s\\%s", m_pMain->m_strModelDir, m_strEditModel, m_pMain->vt_job_info[algo].get_job_name());
//	if (_access(dst_model_path, 0) != 0)			CreateDirectory(dst_model_path, 0);
//
//	if (nCopyExistModel)
//	{
//		if ((int)m_nSelPos == -1) { m_pMain->fnSetMessage(1, _T("First  => Select Copy Model ! !!!")); }
//		else
//		{
//			CString strModel = m_SelstrModel;
//			CString strCopyModelPath, str;
//
//			//strCopyModelPath.Format("%s%s", m_pMain->m_strModelDir, strModel);
//			strCopyModelPath.Format("%s%s\\%s", m_pMain->m_strModelDir, strModel, m_pMain->vt_job_info[algo].get_job_name());
//
//			if (_access(strCopyModelPath, 0) != 0)		// ������ ���� �����ϴ��� üũ
//			{
//				str.Format("%s Model is Not Exist", strCopyModelPath);
//			}
//			else
//			{
//				m_pMain->copyModelFiles(strCopyModelPath, dst_model_path);
//				m_pMain->m_NewModel.setModelID(m_strEditModel);
//
//#ifndef JOB_INFO
//				m_pMain->readAllModelData(&m_pMain->m_NewModel);	// �����ϴ� ��� �� ������ ����
//#else
//		
//				// �ϴ� ����	21.15.42
//				m_pMain->readAllModelData(algo, dst_model_path);	// �����ϴ� ��� �� ������ ����
//#endif
//			}
//		}
//	}
//
//	m_pMain->m_NewModel.setModelID(m_strEditModel);
//	m_pMain->m_NewModel.getGlassInfo().setInch(atof(m_strEditInch));
//	m_pMain->m_NewModel.getGlassInfo().setGlassWidth(atof(m_strEditGlassWidth));
//	m_pMain->m_NewModel.getGlassInfo().setGlassHeight(atof(m_strEditGlassHeight));
//
	return m_pMain->m_NewModel;
}


void CDlgCreateModel::OnEnSetfocusEditCreateModel()
{
	//::SetForegroundWindow(m_edt_ModelTitle.GetSafeHwnd());
	//::SetFocus(m_edt_ModelTitle.GetSafeHwnd());

	//m_pMain->m_pKeyboardDlg->ShowWindow(SW_SHOW);
	//m_pMain->m_pKeyboardDlg->DrawKeyboard();

	SetFocus();

	CVirtualKeyBoard m_pKeyboardDlg;

	if (m_pKeyboardDlg.DoModal() != IDOK)
		return;

	CString strNumber;
	m_pKeyboardDlg.GetValue(strNumber);
	GetDlgItem(IDC_EDIT_CREATE_MODEL)->SetWindowTextA(strNumber);

}
