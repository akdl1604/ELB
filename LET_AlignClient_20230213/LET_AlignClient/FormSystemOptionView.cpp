#include "stdafx.h"
#include "FormSystemOptionView.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"


IMPLEMENT_DYNCREATE(CFormSystemOptionView, CFormView)

CFormSystemOptionView::CFormSystemOptionView() 
	: CFormView(CFormSystemOptionView::IDD)
{
	m_pMain = NULL;
	m_nCurrentSelectJob = 0;
}
CFormSystemOptionView::~CFormSystemOptionView()
{

}
void CFormSystemOptionView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LB_SELECTED_JOB_TITLE, m_LbSelectedJobTitle);
	DDX_Control(pDX, IDC_LB_SELECTED_JOB, m_LbSelectedJob);
	DDX_Control(pDX, IDC_BTN_JOB_DATA_MODIFY, m_btnJobDataModify);
	DDX_Control(pDX, IDC_BTN_JOB_DATA_SAVE, m_btnJobDataSave);
	DDX_Control(pDX, IDC_GRID_JOB_LIST, m_GridJobList);
	DDX_Control(pDX, IDC_GRID_SYSTEM_OPTION, m_GridSystemOption);
	DDX_Control(pDX, IDC_STATIC_IP_ADDRESS, m_stIpAddress);
	DDX_Control(pDX, IDC_STATIC_SUBNET_MASK, m_stSubnetMask);
	DDX_Control(pDX, IDC_STATIC_GATE, m_stGateWay);
	DDX_Control(pDX, IDC_BUTTON_CHANGE_IP, m_btnChangeIP);
}

BEGIN_MESSAGE_MAP(CFormSystemOptionView, CFormView)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_JOB_DATA_MODIFY, &CFormSystemOptionView::OnBnClickedBtnJobDataModify)
	ON_BN_CLICKED(IDC_BTN_JOB_DATA_SAVE, &CFormSystemOptionView::OnBnClickedBtnJobDataSave)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_IP, &CFormSystemOptionView::OnBnClickedButtonChangeIp)
	ON_CBN_SELCHANGE(IDC_COMBO_ADAPT_LIST, &CFormSystemOptionView::OnCbnSelchangeComboAdaptList)
	ON_CBN_SELCHANGE(IDC_COMBO_ADAPT_NAME, &CFormSystemOptionView::OnCbnSelchangeComboAdaptName)
	ON_EN_SETFOCUS(IDC_EDIT_IP_ADDRESS, &CFormSystemOptionView::OnEnSetfocusEditIpAddress)
	ON_EN_SETFOCUS(IDC_EDIT_SUBNET_MASK, &CFormSystemOptionView::OnEnSetfocusEditSubnetMask)
	ON_EN_SETFOCUS(IDC_EDIT_GATEWAY, &CFormSystemOptionView::OnEnSetfocusEditGateWay)
END_MESSAGE_MAP()

#ifdef _DEBUG
void CFormSystemOptionView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CFormSystemOptionView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

void CFormSystemOptionView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	m_pMain = (CLET_AlignClientDlg *)AfxGetMainWnd();
	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);

	InitTitle(&m_LbSelectedJobTitle, "SELETED", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_LbSelectedJob, "", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stIpAddress, " IP Address", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stSubnetMask, "Subnet Mask", 14.f, COLOR_BTN_BODY);
	InitTitle(&m_stGateWay, "Gateway", 14.f, COLOR_BTN_BODY);

	CString strSeletedJob ="Common";
	m_LbSelectedJob.SetText(strSeletedJob);

	MainButtonInit(&m_btnJobDataModify);
	MainButtonInit(&m_btnJobDataSave);
	MainButtonInit(&m_btnChangeIP);

	Init_grid_job_list(m_GridJobList, 2, GRID_ROW_JOB_LIST);
	Init_grid_system_option(m_GridSystemOption, 2, GRID_ROW_PARAMETER);
	
	enableUI(FALSE);
	sign_selected_job();
	InitNetWorkCard();
	
	UpdateFrameDialog();
}

void CFormSystemOptionView::UpdateFrameDialog()
{
	get_job_list();
	get_system_option();
}

HBRUSH CFormSystemOptionView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (pWnd->GetDlgCtrlID() == IDC_COMBO_ADAPT_NAME ||
		pWnd->GetDlgCtrlID() == IDC_COMBO_ADAPT_LIST ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_IP_ADDRESS ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_SUBNET_MASK ||
		pWnd->GetDlgCtrlID() == IDC_EDIT_GATEWAY ||
		pWnd->GetDlgCtrlID() == IDC_STATIC_NETWORK_INFO)
	{
		pDC->SetBkColor(COLOR_UI_BODY);
		pDC->SetTextColor(COLOR_WHITE);
	}

	return m_hbrBkg;
}

void CFormSystemOptionView::InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_BTN_SIDE);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}


void CFormSystemOptionView::MainButtonInit(CButtonEx *pbutton, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}

void CFormSystemOptionView::Init_grid(CGridCtrl &grid, int col, int row)
{
	BOOL bVirtualMode = FALSE;

	grid.SetEditable(FALSE);
	grid.SetEditable(TRUE);
	grid.SetVirtualMode(bVirtualMode);
	grid.SetListMode(TRUE);
	grid.EnableDragAndDrop(FALSE);
	grid.EnableSelection(FALSE);
	grid.SetFixedRowSelection(TRUE);
	grid.SetFixedColumnSelection(TRUE);
	grid.SetFrameFocusCell(FALSE);
	grid.SetTrackFocusCell(FALSE);
	grid.SetRowResize(FALSE);
	grid.SetColumnResize(FALSE);
	grid.SetRowCount(row);
	grid.SetColumnCount(col);
	grid.SetBkColor(RGB(197, 197, 187));
	grid.SetTextBkColor(RGB(235, 235, 235));

	for (int i = 0; i < grid.GetColumnCount(); i++)
	{
		grid.SetItemBkColour(0, i, RGB(146, 146, 141));
		grid.SetItemFgColour(0, i, RGB(255, 255, 255));

		for (int row = 0; row < grid.GetRowCount(); row++)
		{
			UINT state = grid.GetItemState(row, i) | GVIS_READONLY;
			grid.SetItemState(row, i, state);
		}
	}

	for (int row = 1; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{
			grid.SetItemBkColour(row, col, COLOR_BTN_BODY);
			grid.SetItemFgColour(row, col, COLOR_WHITE);
		}
	}
}

void CFormSystemOptionView::Init_grid_job_list(CGridCtrl &grid, int col, int row)
{
	CRect rect;
	grid.GetWindowRect(&rect); 
	
	int grid_width, grid_height;
	grid_width  = (int)(rect.Width()  / (double)col);
	grid_height = (int)(rect.Height() / (double)row);

	Init_grid(grid, col, row);
	
	//Grid 행 사이즈 조정
	for (int nCol = 0; nCol < grid.GetColumnCount(); nCol++)
	{
		switch (nCol)
		{
			case 0:		grid.SetColumnWidth(nCol, 65);									break;
			case 1:		grid.SetColumnWidth(nCol, rect.Width() - 70);					break;
			default:	grid.SetColumnWidth(nCol, grid_width);							break;
		}
	}

	//Grid 열 사이즈 조정
	for (int nRow = 0; nRow < grid.GetRowCount(); nRow++)
		grid.SetRowHeight(nRow, grid_height);
}

void CFormSystemOptionView::Init_grid_system_option(CGridCtrl &grid, int col, int row)
{
	CRect rect;
	grid.GetWindowRect(&rect);

	int grid_width, grid_height;
	grid_width = (int)(rect.Width() / (double)col);
	grid_height = (int)(rect.Height() / (double)row);

	Init_grid(grid, col, row);

	int nTotal_width = 0;
	//Grid 행 사이즈 조정
	for (int nCol = 0; nCol < grid.GetColumnCount(); nCol++)
	{
		switch (nCol)
		{
		case 0: grid.SetColumnWidth(nCol, 400);					break;
		case 1: grid.SetColumnWidth(nCol, rect.Width() - 400);	break;
		}
	}
		

	//Grid 열 사이즈 조정
	for (int nRow = 0; nRow < grid.GetRowCount(); nRow++)
		grid.SetRowHeight(nRow, 30);
}


void CFormSystemOptionView::get_job_list()
{
	GV_ITEM Item;
	Item.mask = GVIF_TEXT | GVIF_FORMAT;
	Item.nFormat = DT_CENTER | DT_WORDBREAK;

	Item.row = 0;
	Item.col = 0;

	UINT state = m_GridJobList.GetItemState(0, 0) | GVIS_READONLY;
	m_GridJobList.SetItemState(0, 0, state);

	Item.strText = "No.";
	m_GridJobList.SetItem(&Item);

	Item.row = 0;
	Item.col = 1;

	state = m_GridJobList.GetItemState(0, 1) | GVIS_READONLY;
	m_GridJobList.SetItemState(0, 1, state);

	Item.strText = "Job Name";
	m_GridJobList.SetItem(&Item);

	Item.row = 1;
	Item.col = 0;

	state = m_GridJobList.GetItemState(0, 0) | GVIS_READONLY;
	m_GridJobList.SetItemState(0, 0, state);

	Item.strText = "1";
	m_GridJobList.SetItem(&Item);

	Item.row = 1;
	Item.col = 1;

	state = m_GridJobList.GetItemState(0, 1) | GVIS_READONLY;
	m_GridJobList.SetItemState(0, 1, state);

	Item.strText = "Common";
	m_GridJobList.SetItem(&Item);


	for (int row= 1; row < m_pMain->vt_job_info.size() + 1; row++)
	{
		Item.row = row + 1;
		Item.col = 0;
		Item.strText.Format("%d", Item.row);
		m_GridJobList.SetItem(&Item);

		Item.row = row + 1;
		Item.col = 1;
		Item.strText = m_pMain->vt_job_info[row - 1].get_job_name();
		m_GridJobList.SetItem(&Item);
	}

	m_GridJobList.Invalidate();
}
void CFormSystemOptionView::get_system_option()
{
	int nJob = m_nCurrentSelectJob - 1;

	//common 일떄
	if (m_nCurrentSelectJob == 0)
	{
		get_system_option_common();
		return;
	}

	int nMethod = m_pMain->vt_job_info[nJob].algo_method;
	switch (nMethod)
	{
	case CLIENT_TYPE_ELB_CENTER_ALIGN:			get_system_option_ELB();				break;
	default:									get_system_option_infra();				break;
	}
}

void CFormSystemOptionView::get_system_option_infra()
{
	int nJob = m_nCurrentSelectJob - 1;

	GV_ITEM Item;
	Item.mask = GVIF_TEXT | GVIF_FORMAT;
	Item.nFormat = DT_CENTER | DT_WORDBREAK;
	Item.row = 0;
	Item.col = 0;

	UINT state = m_GridSystemOption.GetItemState(0, 0) | GVIS_READONLY;
	m_GridSystemOption.SetItemState(0, 0, state);

	Item.strText = "System Option List";

	m_GridSystemOption.SetItem(&Item);

	Item.row = 0;
	Item.col = 1;

	state = m_GridSystemOption.GetItemState(0, 1) | GVIS_READONLY;
	m_GridSystemOption.SetItemState(0, 1, state);

	Item.strText = "Parameter";
	m_GridSystemOption.SetItem(&Item);

	CString strTemp;
	Item.nFormat = DT_LEFT | DT_WORDBREAK;

	
	//첫번째 행
	int nCol = 0;
	for (int nRow = 1; nRow < m_GridSystemOption.GetRowCount(); nRow++)
	{
		switch (nRow) {
		case 1:		strTemp = " 01. Save Image Type( 0 : jpg, 1 : bmp )";						break;
		//case 2:		strTemp = " 02. JPG Compress Rate( % )";									break;//KMB 220924 결과이미지, 원본이미지 압축율 파라미터 분리
		case 2:		strTemp = " 02. JPG Compress Rate Raw(%) ";									break;
		case 3:		strTemp = " 03  JPG Compress Rate Result(%) ";								break;
		case 4:		strTemp = " 04. NG Image Save";												break;
		case 5:		strTemp = " 05. OK Image Save";												break;
		case 6:		strTemp = " 06. Raw Image Save";											break;
		case 7:		strTemp = " 07. Result Image Save";											break;
		case 8:		strTemp = " 08. Use Result Image Capture";									break;
		case 9:		strTemp = " 09. Use Light On/Off System( 0 : no use, 1: use )";				break;
		case 10:		strTemp = " 10. Show Calibration Direction( 0 : hide, 1: show )";			break;
		default:	strTemp = "";																break;
		}

		if (m_bUIEnable)	m_GridSystemOption.SetItemFgColour(nRow, 1, COLOR_WHITE);
		else				m_GridSystemOption.SetItemFgColour(nRow, 1, COLOR_DARK_GRAY);

		state = m_GridSystemOption.GetItemState(nRow, nCol) | GVIS_READONLY;
		m_GridSystemOption.SetItemState(nRow, nCol, state);

		Item.row = nRow;
		Item.col = nCol;
		Item.strText = strTemp;
		m_GridSystemOption.SetItem(&Item);
	}

	//두번째 행
	nCol = 1;
	for (int nRow = 1; nRow < m_GridSystemOption.GetRowCount(); nRow++)
	{
		switch (nRow) {
		case 1:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].save_image_type);				break;
		//case 2:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].jpg_compress_rate);			break;//KMB 220924 결과이미지, 원본이미지 압축율 파라미터 분리
		case 2:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].jpg_compress_rate_Raw);		break;
		case 3:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].jpg_compress_rate_Result);		break;
		case 4:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].ng_image_save);				break;
		case 5:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].ok_image_save);				break;
		case 6:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].raw_image_save);				break;
		case 7:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].result_image_save);			break;
		case 8:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].use_result_image_capture);		break;
		case 9:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].use_light_on_off_system);		break;
		case 10:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].show_calib_direction);			break;
		default:	strTemp = "";																		break;
		}

		if (m_bUIEnable)	m_GridSystemOption.SetItemFgColour(nRow, 1, COLOR_WHITE);
		else				m_GridSystemOption.SetItemFgColour(nRow, 1, COLOR_DARK_GRAY);

		state = m_GridSystemOption.GetItemState(nRow, nCol) | GVIS_READONLY;
		m_GridSystemOption.SetItemState(nRow, nCol, state);

		Item.row = nRow;
		Item.col = nCol;
		Item.strText = strTemp;
		m_GridSystemOption.SetItem(&Item);
	}

	m_GridSystemOption.Invalidate();
}

void CFormSystemOptionView::get_system_option_ELB()
{
	int nJob = m_nCurrentSelectJob - 1;

	GV_ITEM Item;
	Item.mask = GVIF_TEXT | GVIF_FORMAT;
	Item.nFormat = DT_CENTER | DT_WORDBREAK;
	Item.row = 0;
	Item.col = 0;

	UINT state = m_GridSystemOption.GetItemState(0, 0) | GVIS_READONLY;
	m_GridSystemOption.SetItemState(0, 0, state);

	Item.strText = "System Option List";

	m_GridSystemOption.SetItem(&Item);

	Item.row = 0;
	Item.col = 1;

	state = m_GridSystemOption.GetItemState(0, 1) | GVIS_READONLY;
	m_GridSystemOption.SetItemState(0, 1, state);

	Item.strText = "Parameter";
	m_GridSystemOption.SetItem(&Item);

	CString strTemp;
	Item.nFormat = DT_LEFT | DT_WORDBREAK;

	//첫번째 행
	int nCol = 0;
	for (int nRow = 1; nRow < m_GridSystemOption.GetRowCount(); nRow++)
	{
		switch (nRow) {
		case 1:		strTemp = " 01. Save Image Type( 0 : jpg, 1 : bmp )";						break;
			//case 2:		strTemp = " 02. JPG Compress Rate( % )";									break; //KMB 220924 결과이미지, 원본이미지 압축율 파라미터 분리
		case 2:		strTemp = " 02. JPG Compress Rate Raw(%) ";									break;
		case 3:		strTemp = " 03  JPG Compress Rate Result(%) ";								break;
		case 4:		strTemp = " 04. NG Image Save";												break;
		case 5:		strTemp = " 05. OK Image Save";												break;
		case 6:		strTemp = " 06. Raw Image Save";											break;
		case 7:		strTemp = " 07. Result Image Save";											break;
		case 8:		strTemp = " 08. Use Result Image Capture";									break;
		case 9:		strTemp = " 09. Use Light On/Off System( 0 : no use, 1: use )";				break;
		case 10:	strTemp = " 10. Align Debuging Image Save( 0 : no use, 1: use )";			break;
		case 11:	strTemp = " 11. Trace Debuging Image Save( 0 : no use, 1: use )";			break;
		case 12:	strTemp = " 12. Inspection Debuging Image Save( 0 : no use, 1: use )";		break;
		case 13:	strTemp = " 13. Auto Save Video Time (second)";								break;
		case 14:	strTemp = " 14. Show Calibration Direction( 0 : hide, 1: show )";			break;
		default:	strTemp = "";																break;
		}

		if (m_bUIEnable)	m_GridSystemOption.SetItemFgColour(nRow, 1, COLOR_WHITE);
		else				m_GridSystemOption.SetItemFgColour(nRow, 1, COLOR_DARK_GRAY);

		state = m_GridSystemOption.GetItemState(nRow, nCol) | GVIS_READONLY;
		m_GridSystemOption.SetItemState(nRow, nCol, state);

		Item.row = nRow;
		Item.col = nCol;
		Item.strText = strTemp;
		m_GridSystemOption.SetItem(&Item);
	}

	//두번째 행
	nCol = 1;
	for (int nRow = 1; nRow < m_GridSystemOption.GetRowCount(); nRow++)
	{
		switch (nRow) {
		case 1:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].save_image_type);						break;
		//case 2:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].jpg_compress_rate);					break;//KMB 220924 결과이미지, 원본이미지 압축율 파라미터 분리
		case 2:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].jpg_compress_rate_Raw);				break;
		case 3:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].jpg_compress_rate_Result);				break;
		case 4:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].ng_image_save);						break;
		case 5:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].ok_image_save);						break;
		case 6:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].raw_image_save);						break;
		case 7:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].result_image_save);					break;
		case 8:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].use_result_image_capture);				break;
		case 9:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].use_light_on_off_system);				break;
		case 10:	strTemp.Format("%d", m_pMain->vt_system_option[nJob].stELB.Align_DebuggingImage_Save);		break;
		case 11:	strTemp.Format("%d", m_pMain->vt_system_option[nJob].stELB.Trace_DebuggingImage_Save);		break;
		case 12:	strTemp.Format("%d", m_pMain->vt_system_option[nJob].stELB.Inspection_DebuggingImage_Save);	break;
		case 13:	strTemp.Format("%f", m_pMain->vt_system_option[nJob].stELB.Auto_Save_Video_Time);			break;
		case 14:	strTemp.Format("%d", m_pMain->vt_system_option[nJob].show_calib_direction);					break;
		default:	strTemp = "";																				break;
		}

		if (m_bUIEnable)	m_GridSystemOption.SetItemFgColour(nRow, 1, COLOR_WHITE);
		else				m_GridSystemOption.SetItemFgColour(nRow, 1, COLOR_DARK_GRAY);

		state = m_GridSystemOption.GetItemState(nRow, nCol) | GVIS_READONLY;
		m_GridSystemOption.SetItemState(nRow, nCol, state);

		Item.row = nRow;
		Item.col = nCol;
		Item.strText = strTemp;
		m_GridSystemOption.SetItem(&Item);
	}

	m_GridSystemOption.Invalidate();
}

void CFormSystemOptionView::get_system_option_common()
{
	GV_ITEM Item;
	Item.mask = GVIF_TEXT | GVIF_FORMAT;
	Item.nFormat = DT_CENTER | DT_WORDBREAK;
	Item.row = 0;
	Item.col = 0;

	UINT state = m_GridSystemOption.GetItemState(0, 0) | GVIS_READONLY;
	m_GridSystemOption.SetItemState(0, 0, state);

	Item.strText = "Common Sysytem Option List";

	m_GridSystemOption.SetItem(&Item);

	Item.row = 0;
	Item.col = 1;

	state = m_GridSystemOption.GetItemState(0, 1) | GVIS_READONLY;
	m_GridSystemOption.SetItemState(0, 1, state);

	Item.strText = "Parameter";
	m_GridSystemOption.SetItem(&Item);

	CString strTemp;
	Item.nFormat = DT_LEFT | DT_WORDBREAK;

	//첫번째 행
	int nCol = 0;
	for (int nRow = 1; nRow < m_GridSystemOption.GetRowCount(); nRow++)
	{
		switch (nRow) {
		case 1:		strTemp = " 01. Use PLC Connection Check ( 0 : no use, 1 : use)";		break;
		case 2:		strTemp = " 02. Use Auto PLC Model Process (0 : no use, 1 : use)";		break;
		case 3:		strTemp = " 03. History File Save Term(day)";							break;
		case 4:		strTemp = " 04. Excel	File Save Term(day)";							break;
		case 5:		strTemp = " 05. Image	File Save Term(day)";							break;
		case 6:		strTemp = " 06. Video	File Save Term(day)";							break;
		case 7:		strTemp = " 07. Cycle Time To Delete File When Auto Mode(second)";		break;
		default:	strTemp = "";															break;
		}						   

		if (m_bUIEnable)	m_GridSystemOption.SetItemFgColour(nRow, 1, COLOR_WHITE);
		else				m_GridSystemOption.SetItemFgColour(nRow, 1, COLOR_DARK_GRAY);

		state = m_GridSystemOption.GetItemState(nRow, nCol) | GVIS_READONLY;
		m_GridSystemOption.SetItemState(nRow, nCol, state);

		Item.row = nRow;
		Item.col = nCol;
		Item.strText = strTemp;
		m_GridSystemOption.SetItem(&Item);
	}

	//두번째 행
	nCol = 1;
	for (int nRow = 1; nRow < m_GridSystemOption.GetRowCount(); nRow++)
	{
		switch (nRow) {
		case 1:		strTemp.Format("%d",	m_pMain->m_system_option_common.use_plc_check);						break;
		case 2:		strTemp.Format("%d",	m_pMain->m_system_option_common.use_plc_model_process);				break;
		case 3:		strTemp.Format("%d",	m_pMain->m_system_option_common.save_term_file_history);			break;
		case 4:		strTemp.Format("%d",	m_pMain->m_system_option_common.save_term_file_excel);				break;
		case 5:		strTemp.Format("%d",	m_pMain->m_system_option_common.save_term_file_image);				break;
		case 6:		strTemp.Format("%d",	m_pMain->m_system_option_common.save_term_file_video);				break;
		case 7:		strTemp.Format("%d",	m_pMain->m_system_option_common.cycle_time_to_delete_file_auto_mode);	break;
		default:	strTemp = "";																				break;
		}

		if (m_bUIEnable)	m_GridSystemOption.SetItemFgColour(nRow, 1, COLOR_WHITE);
		else				m_GridSystemOption.SetItemFgColour(nRow, 1, COLOR_DARK_GRAY);

		state = m_GridSystemOption.GetItemState(nRow, nCol) | GVIS_READONLY;
		m_GridSystemOption.SetItemState(nRow, nCol, state);

		Item.row = nRow;
		Item.col = nCol;
		Item.strText = strTemp;
		m_GridSystemOption.SetItem(&Item);
	}

	m_GridSystemOption.Invalidate();

}

void CFormSystemOptionView::get_system_option_assemble_insp()
{
	int nJob = m_nCurrentSelectJob - 1;

	GV_ITEM Item;
	Item.mask = GVIF_TEXT | GVIF_FORMAT;
	Item.nFormat = DT_CENTER | DT_WORDBREAK;
	Item.row = 0;
	Item.col = 0;

	UINT state = m_GridSystemOption.GetItemState(0, 0) | GVIS_READONLY;
	m_GridSystemOption.SetItemState(0, 0, state);

	Item.strText = "System Option List";

	m_GridSystemOption.SetItem(&Item);

	Item.row = 0;
	Item.col = 1;

	state = m_GridSystemOption.GetItemState(0, 1) | GVIS_READONLY;
	m_GridSystemOption.SetItemState(0, 1, state);

	Item.strText = "Parameter";
	m_GridSystemOption.SetItem(&Item);

	CString strTemp;
	Item.nFormat = DT_LEFT | DT_WORDBREAK;

	//첫번째 행
	int nCol = 0;
	for (int nRow = 1; nRow < m_GridSystemOption.GetRowCount(); nRow++)
	{
		switch (nRow) {
		case 1:		strTemp = " 01. Save Image Type( 0 : jpg, 1 : bmp )";						break;
			//case 2:		strTemp = " 02. JPG Compress Rate( % )";									break;//KMB 220924 결과이미지, 원본이미지 압축율 파라미터 분리
		case 2:		strTemp = " 02. JPG Compress Rate Raw(%) ";									break;
		case 3:		strTemp = " 03  JPG Compress Rate Result(%) ";								break;
		case 4:		strTemp = " 04. NG Image Save";												break;
		case 5:		strTemp = " 05. OK Image Save";												break;
		case 6:		strTemp = " 06. Raw Image Save";											break;
		case 7:		strTemp = " 07. Result Image Save";											break;
		case 8:		strTemp = " 08. Use Result Image Capture";									break;
		case 9:		strTemp = " 09. Use Light On/Off System( 0 : no use, 1: use )";				break;
		case 10:	strTemp = " 10. Use RDP Report( 0 : no use, 1: use )";						break;
		case 11:	strTemp = " 11. RDP Image Type( 0 : Raw, 1 : Result, 2 : Both )";			break;
		case 12:	strTemp = " 12. Replacement Data for Not Find Line";						break;
		default:	strTemp = "";																break;
		}

		if (m_bUIEnable)	m_GridSystemOption.SetItemFgColour(nRow, 1, COLOR_WHITE);
		else				m_GridSystemOption.SetItemFgColour(nRow, 1, COLOR_DARK_GRAY);

		state = m_GridSystemOption.GetItemState(nRow, nCol) | GVIS_READONLY;
		m_GridSystemOption.SetItemState(nRow, nCol, state);

		Item.row = nRow;
		Item.col = nCol;
		Item.strText = strTemp;
		m_GridSystemOption.SetItem(&Item);
	}

	//두번째 행
	nCol = 1;
	for (int nRow = 1; nRow < m_GridSystemOption.GetRowCount(); nRow++)
	{
		switch (nRow) {
		case 1:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].save_image_type);				break;
		//case 2:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].jpg_compress_rate);			break;//KMB 220924 결과이미지, 원본이미지 압축율 파라미터 분리
		case 2:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].jpg_compress_rate_Raw);		break;
		case 3:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].jpg_compress_rate_Result);		break;
		case 4:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].ng_image_save);				break;
		case 5:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].ok_image_save);				break;
		case 6:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].raw_image_save);				break;
		case 7:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].result_image_save);			break;
		case 8:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].use_result_image_capture);		break;
		case 9:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].use_light_on_off_system);		break;
		case 10:	strTemp.Format("%d", m_pMain->vt_system_option[nJob].use_rdp_report);				break;
		case 11:	strTemp.Format("%d", m_pMain->vt_system_option[nJob].rdp_image_type);				break;
		case 12:	strTemp.Format("%.5f", m_pMain->vt_system_option[nJob].replacement_data);			break;
		default:	strTemp = "";						break;
		}

		if (m_bUIEnable)	m_GridSystemOption.SetItemFgColour(nRow, 1, COLOR_WHITE);
		else				m_GridSystemOption.SetItemFgColour(nRow, 1, COLOR_DARK_GRAY);

		state = m_GridSystemOption.GetItemState(nRow, nCol) | GVIS_READONLY;
		m_GridSystemOption.SetItemState(nRow, nCol, state);

		Item.row = nRow;
		Item.col = nCol;
		Item.strText = strTemp;
		m_GridSystemOption.SetItem(&Item);
	}

	m_GridSystemOption.Invalidate();
}

void CFormSystemOptionView::get_system_option_pcb_bending_insp()
{
	int nJob = m_nCurrentSelectJob - 1;

	GV_ITEM Item;
	Item.mask = GVIF_TEXT | GVIF_FORMAT;
	Item.nFormat = DT_CENTER | DT_WORDBREAK;
	Item.row = 0;
	Item.col = 0;

	UINT state = m_GridSystemOption.GetItemState(0, 0) | GVIS_READONLY;
	m_GridSystemOption.SetItemState(0, 0, state);

	Item.strText = "System Option List";

	m_GridSystemOption.SetItem(&Item);

	Item.row = 0;
	Item.col = 1;

	state = m_GridSystemOption.GetItemState(0, 1) | GVIS_READONLY;
	m_GridSystemOption.SetItemState(0, 1, state);

	Item.strText = "Parameter";
	m_GridSystemOption.SetItem(&Item);

	CString strTemp;
	Item.nFormat = DT_LEFT | DT_WORDBREAK;

	//첫번째 행
	int nCol = 0;
	for (int nRow = 1; nRow < m_GridSystemOption.GetRowCount(); nRow++)
	{
		switch (nRow) {
		case 1:		strTemp = " 01. Save Image Type( 0 : jpg, 1 : bmp )";					break;
			//case 2:		strTemp = " 02. JPG Compress Rate( % )";									break;//KMB 220924 결과이미지, 원본이미지 압축율 파라미터 분리
		case 2:		strTemp = " 02. JPG Compress Rate Raw(%) ";									break;
		case 3:		strTemp = " 03  JPG Compress Rate Result(%) ";								break;
		case 4:		strTemp = " 04. NG Image Save";											break;
		case 5:		strTemp = " 05. OK Image Save";											break;
		case 6:		strTemp = " 06. Raw Image Save";										break;
		case 7:		strTemp = " 07. Result Image Save";										break;
		case 8:		strTemp = " 08. Use Result Image Capture";								break;
		case 9:		strTemp = " 09. Use Light On/Off System( 0 : no use, 1: use )";			break;
		case 10:	strTemp = " 10. Use RDP Report";										break;
		case 11:	strTemp = " 11. RDP Image Type( 0 : Raw, 1 : Result, 2 : Both )";		break;
		case 12:	strTemp = " 12. Replacement Data for Not Find Line";					break;
		default:	strTemp = "";															break;
		}

		if (m_bUIEnable)	m_GridSystemOption.SetItemFgColour(nRow, 1, COLOR_WHITE);
		else				m_GridSystemOption.SetItemFgColour(nRow, 1, COLOR_DARK_GRAY);

		state = m_GridSystemOption.GetItemState(nRow, nCol) | GVIS_READONLY;
		m_GridSystemOption.SetItemState(nRow, nCol, state);

		Item.row = nRow;
		Item.col = nCol;
		Item.strText = strTemp;
		m_GridSystemOption.SetItem(&Item);
	}

	//두번째 행
	nCol = 1;
	for (int nRow = 1; nRow < m_GridSystemOption.GetRowCount(); nRow++)
	{
		switch (nRow) {
		case 1:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].save_image_type);				break;
		//case 2:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].jpg_compress_rate);			break;//KMB 220924 결과이미지, 원본이미지 압축율 파라미터 분리
		case 2:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].jpg_compress_rate_Raw);		break;
		case 3:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].jpg_compress_rate_Result);		break;
		case 4:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].ng_image_save);				break;
		case 5:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].ok_image_save);				break;
		case 6:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].raw_image_save);				break;
		case 7:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].result_image_save);			break;
		case 8:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].use_result_image_capture);		break;
		case 9:		strTemp.Format("%d", m_pMain->vt_system_option[nJob].use_light_on_off_system);		break;
		case 10:	strTemp.Format("%d", m_pMain->vt_system_option[nJob].use_rdp_report);				break;
		case 11:	strTemp.Format("%d", m_pMain->vt_system_option[nJob].rdp_image_type);				break;
		case 12:	strTemp.Format("%.5f", m_pMain->vt_system_option[nJob].replacement_data);			break;
		default:	strTemp = "";						break;
		}

		if (m_bUIEnable)	m_GridSystemOption.SetItemFgColour(nRow, 1, COLOR_WHITE);
		else				m_GridSystemOption.SetItemFgColour(nRow, 1, COLOR_DARK_GRAY);

		state = m_GridSystemOption.GetItemState(nRow, nCol) | GVIS_READONLY;
		m_GridSystemOption.SetItemState(nRow, nCol, state);

		Item.row = nRow;
		Item.col = nCol;
		Item.strText = strTemp;
		m_GridSystemOption.SetItem(&Item);
	}

	m_GridSystemOption.Invalidate();
}

void CFormSystemOptionView::set_system_option(int nJob, int row)
{
	//common 일떄
	if (m_nCurrentSelectJob == 0)
	{
		set_system_option_common(row);
		return;
	}

	int nMethod = m_pMain->vt_job_info[nJob].algo_method;
	switch (nMethod)
	{
	case CLIENT_TYPE_ELB_CENTER_ALIGN:			set_system_option_ELB(nJob, row);				break;
	default:									set_system_option_infra(nJob, row);				break;
	}

}

void CFormSystemOptionView::set_system_option_infra(int nJob, int row)
{
	CString strTemp;

	CKeyPadDlg dlg;
	dlg.SetValueString(FALSE, m_GridSystemOption.GetItemText(row, 1));
	dlg.DoModal();
	dlg.GetValue(strTemp);

	double dValue = 0;
	int nValue = 0;
	dValue = atof(strTemp);
	nValue = atoi(strTemp);

	switch (row) {
	case 1:		m_pMain->vt_system_option[nJob].save_image_type = nValue;					break;
	//case 2:		m_pMain->vt_system_option[nJob].jpg_compress_rate = nValue;					break;//KMB 220924 결과이미지, 원본이미지 압축율 파라미터 분리
	case 2:		m_pMain->vt_system_option[nJob].jpg_compress_rate_Raw = nValue;				break;
	case 3:		m_pMain->vt_system_option[nJob].jpg_compress_rate_Result = nValue;			break;
	case 4:		m_pMain->vt_system_option[nJob].ng_image_save = nValue;						break;
	case 5:		m_pMain->vt_system_option[nJob].ok_image_save = nValue;						break;
	case 6:		m_pMain->vt_system_option[nJob].raw_image_save = nValue;					break;
	case 7:		m_pMain->vt_system_option[nJob].result_image_save = nValue;					break;
	case 8:		m_pMain->vt_system_option[nJob].use_result_image_capture = nValue;			break;
	case 9:		m_pMain->vt_system_option[nJob].use_light_on_off_system = nValue;			break;
	case 10:		m_pMain->vt_system_option[nJob].show_calib_direction = nValue;				break;
	}
}

void CFormSystemOptionView::set_system_option_ELB(int nJob, int row)
{
	CString strTemp;

	CKeyPadDlg dlg;
	dlg.SetValueString(FALSE, m_GridSystemOption.GetItemText(row, 1));
	dlg.DoModal();
	dlg.GetValue(strTemp);

	double dValue = 0;
	int nValue = 0;
	dValue = atof(strTemp);
	nValue = atoi(strTemp);

	switch (row) {
	case 1:		m_pMain->vt_system_option[nJob].save_image_type = nValue;						break;
	//case 2:		m_pMain->vt_system_option[nJob].jpg_compress_rate = nValue;						break;//KMB 220924 결과이미지, 원본이미지 압축율 파라미터 분리
	case 2:		m_pMain->vt_system_option[nJob].jpg_compress_rate_Raw = nValue;					break;
	case 3:		m_pMain->vt_system_option[nJob].jpg_compress_rate_Result = nValue;				break;
	case 4:		m_pMain->vt_system_option[nJob].ng_image_save = nValue;							break;
	case 5:		m_pMain->vt_system_option[nJob].ok_image_save = nValue;							break;
	case 6:		m_pMain->vt_system_option[nJob].raw_image_save = nValue;						break;
	case 7:		m_pMain->vt_system_option[nJob].result_image_save = nValue;						break;
	case 8:		m_pMain->vt_system_option[nJob].result_image_save = nValue;						break;
	case 9:		m_pMain->vt_system_option[nJob].use_light_on_off_system = nValue;				break;
	case 10:	m_pMain->vt_system_option[nJob].stELB.Align_DebuggingImage_Save = nValue;		break;
	case 11:	m_pMain->vt_system_option[nJob].stELB.Trace_DebuggingImage_Save = nValue;		break;
	case 12:	m_pMain->vt_system_option[nJob].stELB.Inspection_DebuggingImage_Save = nValue;	break;
	case 13:	m_pMain->vt_system_option[nJob].stELB.Auto_Save_Video_Time = dValue;			break;
	case 14:	m_pMain->vt_system_option[nJob].show_calib_direction = nValue;					break;
	}
}


void CFormSystemOptionView::set_system_option_pcb_bending_insp(int nJob, int row)
{
	CString strTemp;

	CKeyPadDlg dlg;
	dlg.SetValueString(FALSE, m_GridSystemOption.GetItemText(row, 1));
	dlg.DoModal();
	dlg.GetValue(strTemp);

	double dValue = 0;
	int nValue = 0;
	dValue = atof(strTemp);
	nValue = atoi(strTemp);

	switch (row) {
	case 1:		m_pMain->vt_system_option[nJob].save_image_type = nValue;					break;
	//case 2:		m_pMain->vt_system_option[nJob].jpg_compress_rate = nValue;					break;//KMB 220924 결과이미지, 원본이미지 압축율 파라미터 분리
	case 2:		m_pMain->vt_system_option[nJob].jpg_compress_rate_Raw = nValue;				break;
	case 3:		m_pMain->vt_system_option[nJob].jpg_compress_rate_Result = nValue;			break;
	case 4:		m_pMain->vt_system_option[nJob].ng_image_save = nValue;						break;
	case 5:		m_pMain->vt_system_option[nJob].ok_image_save = nValue;						break;
	case 6:		m_pMain->vt_system_option[nJob].raw_image_save = nValue;					break;
	case 7:		m_pMain->vt_system_option[nJob].result_image_save = nValue;					break;
	case 8:		m_pMain->vt_system_option[nJob].use_result_image_capture = nValue;			break;
	case 9:		m_pMain->vt_system_option[nJob].use_light_on_off_system = nValue;			break;
	case 10:		m_pMain->vt_system_option[nJob].use_rdp_report = nValue;				break;
	case 11:	m_pMain->vt_system_option[nJob].rdp_image_type = nValue;					break;
	case 12:	m_pMain->vt_system_option[nJob].replacement_data = dValue;					break;
	}
}

void CFormSystemOptionView::set_system_option_assemble_insp(int nJob, int row)
{
	CString strTemp;

	CKeyPadDlg dlg;
	dlg.SetValueString(FALSE, m_GridSystemOption.GetItemText(row, 1));
	dlg.DoModal();
	dlg.GetValue(strTemp);

	double dValue = 0;
	int nValue = 0;
	dValue = atof(strTemp);
	nValue = atoi(strTemp);

	switch (row) {
	case 1:		m_pMain->vt_system_option[nJob].save_image_type = nValue;					break;
	//case 2:		m_pMain->vt_system_option[nJob].jpg_compress_rate = nValue;					break;//KMB 220924 결과이미지, 원본이미지 압축율 파라미터 분리
	case 2:		m_pMain->vt_system_option[nJob].jpg_compress_rate_Raw = nValue;				break;
	case 3:		m_pMain->vt_system_option[nJob].jpg_compress_rate_Result = nValue;			break;
	case 4:		m_pMain->vt_system_option[nJob].ng_image_save = nValue;						break;
	case 5:		m_pMain->vt_system_option[nJob].ok_image_save = nValue;						break;
	case 6:		m_pMain->vt_system_option[nJob].raw_image_save = nValue;					break;
	case 7:		m_pMain->vt_system_option[nJob].result_image_save = nValue;					break;
	case 8:		m_pMain->vt_system_option[nJob].use_result_image_capture = nValue;			break;
	case 9:		m_pMain->vt_system_option[nJob].use_light_on_off_system= nValue;			break;
	case 10:		m_pMain->vt_system_option[nJob].use_rdp_report = nValue;				break;
	case 11:	m_pMain->vt_system_option[nJob].rdp_image_type = nValue;					break;
	case 12:	m_pMain->vt_system_option[nJob].replacement_data = dValue;					break;
	}
}

void CFormSystemOptionView::set_system_option_common(int row)
{
	CString strTemp;

	CKeyPadDlg dlg;
	dlg.SetValueString(FALSE, m_GridSystemOption.GetItemText(row, 1));
	dlg.DoModal();
	dlg.GetValue(strTemp);

	double dValue = 0;
	int nValue = 0;
	dValue = atof(strTemp);
	nValue = atoi(strTemp);

	switch (row) {
	case 1:		m_pMain->m_system_option_common.use_plc_check				= nValue;		break;
	case 2:		m_pMain->m_system_option_common.use_plc_model_process		= nValue;		break;
	case 3:		m_pMain->m_system_option_common.save_term_file_history		= nValue;		break;
	case 4:		m_pMain->m_system_option_common.save_term_file_excel		= nValue;		break;
	case 5:		m_pMain->m_system_option_common.save_term_file_image		= nValue;		break;
	case 6:		m_pMain->m_system_option_common.save_term_file_video		= nValue;		break;
	case 7:		
	{
		if (nValue < 10)	m_pMain->m_system_option_common.cycle_time_to_delete_file_auto_mode = 10;
		else				m_pMain->m_system_option_common.cycle_time_to_delete_file_auto_mode = nValue;

	}break;

	//case 3:		m_pMain->m_system_option_common.daily_production_reset_hour = nValue;		break;
	}
}

BOOL CFormSystemOptionView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (wParam == (WPARAM)m_GridJobList.GetDlgCtrlID())
	{
		GV_DISPINFO *pDispInfo = (GV_DISPINFO *)lParam;

		// Click : 4294967196
		if (pDispInfo->item.col == 1 && pDispInfo->item.row > 0 && pDispInfo->hdr.code == 4294967196)
		{
			if (pDispInfo->item.row > 0 && pDispInfo->item.row <= m_pMain->vt_job_info.size() + 1)
			{
				if (m_btnJobDataModify.GetColorBkg() == COLOR_BTN_SELECT)
				{
					AfxMessageBox("Data Save First!!");
					return CFormView::OnNotify(wParam, lParam, pResult);
				}

				m_nCurrentSelectJob = pDispInfo->item.row - 1;
				sign_selected_job();
				UpdateFrameDialog();
			}
		}
	}
	else if (wParam == (WPARAM)m_GridSystemOption.GetDlgCtrlID())
	{
		GV_DISPINFO *pDispInfo = (GV_DISPINFO *)lParam;

		// Click : 4294967196
		if (pDispInfo->item.col == 1 && pDispInfo->item.row > 0 && pDispInfo->hdr.code == 4294967196)
		{
			if (m_bUIEnable != TRUE) return CFormView::OnNotify(wParam, lParam, pResult);

			if (pDispInfo->item.row > 0 && pDispInfo->item.row < GRID_ROW_PARAMETER + 1)
			{
				int nRow = pDispInfo->item.row;
				int nJob = m_nCurrentSelectJob - 1;
				if (m_nCurrentSelectJob == 0)	set_system_option_common(nRow);
				else							set_system_option(nJob, nRow);
				UpdateFrameDialog();
			}
		}
	}

	return CFormView::OnNotify(wParam, lParam, pResult);
}

void CFormSystemOptionView::sign_selected_job()
{
	//현재 Job이름 표시
	CString strSelectJob;
	if (m_nCurrentSelectJob == 0)	strSelectJob = "Common";
	else
		strSelectJob = m_pMain->vt_job_info[m_nCurrentSelectJob - 1].get_job_name();
	m_LbSelectedJob.SetText(strSelectJob);

	for (int i = 0; i < m_pMain->vt_job_info.size() + 1; i++)
	{
		if (i == m_nCurrentSelectJob)
		{
			//선택 Job 위치 색깔로 표시
			m_GridJobList.SetItemBkColour(m_nCurrentSelectJob + 1, 0, COLOR_BTN_SELECT);
			m_GridJobList.SetItemBkColour(m_nCurrentSelectJob + 1, 1, COLOR_BTN_SELECT);
		}
		else
		{
			//아닌경우 원상태
			m_GridJobList.SetItemBkColour(i + 1, 0, COLOR_BTN_BODY);
			m_GridJobList.SetItemBkColour(i + 1, 1, COLOR_BTN_BODY);
		}
	}
}


void CFormSystemOptionView::OnBnClickedBtnJobDataModify()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_pMain->fnSetMessage(2, "Modify the Data?") != TRUE) return;

	m_btnJobDataModify.SetEnable(false);
	m_btnJobDataSave.SetEnable(true);

	m_btnJobDataModify.SetColorBkg(255,COLOR_BTN_SELECT);

	enableUI(TRUE);
}


void CFormSystemOptionView::OnBnClickedBtnJobDataSave()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
		// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_pMain->fnSetMessage(2, "Save the Data?") != TRUE) return;

	int nJob = m_nCurrentSelectJob - 1;
	if(m_nCurrentSelectJob == 0)	save_system_option_common_ini();
	else							save_system_option_job_ini(nJob);

	m_btnJobDataModify.SetEnable(true);
	m_btnJobDataSave.SetEnable(false);

	m_btnJobDataModify.SetColorBkg(255, COLOR_BTN_BODY);

	enableUI(FALSE);

}
void CFormSystemOptionView::enableUI(BOOL bEnable)
{
	m_bUIEnable = bEnable;
	COLORREF color;
	if (bEnable)
	{
		color = COLOR_DARK_GRAY;
		for (int nRow = 1; nRow < m_GridJobList.GetRowCount(); nRow++)
			if(	 m_nCurrentSelectJob != nRow - 1 )	m_GridJobList.SetItemFgColour(nRow, 1, color);

		color = COLOR_WHITE;
		for (int nRow = 1; nRow < m_GridSystemOption.GetRowCount(); nRow++)
			m_GridSystemOption.SetItemFgColour(nRow, 1, color);
	}
	else
	{
		color = COLOR_WHITE;
		for (int nRow = 1; nRow < m_GridJobList.GetRowCount(); nRow++)
			m_GridJobList.SetItemFgColour(nRow, 1, color);

		color = COLOR_DARK_GRAY;
		for (int nRow = 1; nRow < m_GridSystemOption.GetRowCount(); nRow++)
			m_GridSystemOption.SetItemFgColour(nRow, 1, color);
	}

	m_GridJobList.RedrawColumn(1);
	m_GridSystemOption.RedrawColumn(1);
}

void CFormSystemOptionView::save_system_option_job_ini(int nJob)
{
	CString strData, strSection;
	CString strModelFilePath = m_pMain->m_strConfigDir + "systemoption.ini";

	strSection.Format("JOB%d_INFO", nJob + 1);
	//KMB 220924 결과이미지, 원본이미지 압축율 파라미터 분리
	//strData.Format("%d", m_pMain->vt_system_option[nJob].jpg_compress_rate);
	//::WritePrivateProfileStringA(strSection, "JPG_COMPRESS_RATE", strData, strModelFilePath);

	strData.Format("%d", m_pMain->vt_system_option[nJob].jpg_compress_rate_Raw);
	::WritePrivateProfileStringA(strSection, "JPG_COMPRESS_RATE_RAW", strData, strModelFilePath);

	strData.Format("%d", m_pMain->vt_system_option[nJob].jpg_compress_rate_Result);
	::WritePrivateProfileStringA(strSection, "JPG_COMPRESS_RATE_RESULT", strData, strModelFilePath);

	strData.Format("%d", m_pMain->vt_system_option[nJob].ng_image_save);
	::WritePrivateProfileStringA(strSection, "NG_IMAGE_SAVE", strData, strModelFilePath);

	strData.Format("%d", m_pMain->vt_system_option[nJob].ok_image_save);
	::WritePrivateProfileStringA(strSection, "OK_IMAGE_SAVE", strData, strModelFilePath);

	strData.Format("%d", m_pMain->vt_system_option[nJob].raw_image_save);
	::WritePrivateProfileStringA(strSection, "RAW_IMAGE_SAVE", strData, strModelFilePath);

	strData.Format("%d", m_pMain->vt_system_option[nJob].result_image_save);
	::WritePrivateProfileStringA(strSection, "RESULT_IMAGE_SAVE", strData, strModelFilePath);

	strData.Format("%d", m_pMain->vt_system_option[nJob].use_result_image_capture);
	::WritePrivateProfileStringA(strSection, "USE_RESULT_IMAGE_CAPTURE", strData, strModelFilePath);

	strData.Format("%d", m_pMain->vt_system_option[nJob].save_image_type);
	::WritePrivateProfileStringA(strSection, "SAVE_IMAGE_TYPE", strData, strModelFilePath);

	strData.Format("%d", m_pMain->vt_system_option[nJob].use_rdp_report);
	::WritePrivateProfileStringA(strSection, "USE_RDP_REPORT", strData, strModelFilePath);

	strData.Format("%d", m_pMain->vt_system_option[nJob].rdp_image_type);
	::WritePrivateProfileStringA(strSection, "RDP_IMAGE_TYPE", strData, strModelFilePath);

	strData.Format("%.5f", m_pMain->vt_system_option[nJob].replacement_data);
	::WritePrivateProfileStringA(strSection, "REPLACEMENT_DATA", strData, strModelFilePath);

	strData.Format("%d", m_pMain->vt_system_option[nJob].use_light_on_off_system);
	::WritePrivateProfileStringA(strSection, "USE_LIGHT_ON_OFF_SYSTEM", strData, strModelFilePath);

	strData.Format("%d", m_pMain->vt_system_option[nJob].show_calib_direction);
	::WritePrivateProfileStringA(strSection, "SHOW_CALIB_DIRECTION", strData, strModelFilePath);

	if (m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_ELB_CENTER_ALIGN)
	{
		strData.Format("%d", m_pMain->vt_system_option[nJob].stELB.Align_DebuggingImage_Save);
		::WritePrivateProfileStringA(strSection, "USE_ALIGN_DEBUGGING_SAVE", strData, strModelFilePath);
		strData.Format("%d", m_pMain->vt_system_option[nJob].stELB.Trace_DebuggingImage_Save);
		::WritePrivateProfileStringA(strSection, "USE_TRACE_DEBUGGING_SAVE", strData, strModelFilePath);
		strData.Format("%d", m_pMain->vt_system_option[nJob].stELB.Inspection_DebuggingImage_Save);
		::WritePrivateProfileStringA(strSection, "USE_INSPECTION_DEBUGGING_SAVE", strData, strModelFilePath);
		strData.Format("%f", m_pMain->vt_system_option[nJob].stELB.Auto_Save_Video_Time);
		::WritePrivateProfileStringA(strSection, "AUTO_VIDEO_SAVE_TIME", strData, strModelFilePath);
	}

}
void CFormSystemOptionView::save_system_option_common_ini()
{
	CString strData, strSection;
	CString strModelFilePath = m_pMain->m_strConfigDir + "systemoption.ini";

	strSection.Format("COMMON");
	strData.Format("%d", m_pMain->m_system_option_common.use_plc_check);
	::WritePrivateProfileStringA(strSection, "USE_PLC_CHECK", strData, strModelFilePath);

	strData.Format("%d", m_pMain->m_system_option_common.use_plc_model_process);
	::WritePrivateProfileStringA(strSection, "USE_PLC_MODEL_PROCESS", strData, strModelFilePath);

	strData.Format("%d", m_pMain->m_system_option_common.daily_production_reset_hour);
	::WritePrivateProfileStringA(strSection, "DAILY_PRODUCTION_RESET_HOUR", strData, strModelFilePath);

	strData.Format("%d", m_pMain->m_system_option_common.save_term_file_history);
	::WritePrivateProfileStringA(strSection, "SAVE_TERM_FILE_HISTORY", strData, strModelFilePath);

	strData.Format("%d", m_pMain->m_system_option_common.save_term_file_excel);
	::WritePrivateProfileStringA(strSection, "SAVE_TERM_FILE_EXCEL", strData, strModelFilePath);

	strData.Format("%d", m_pMain->m_system_option_common.save_term_file_image);
	::WritePrivateProfileStringA(strSection, "SAVE_TERM_FILE_IMAGE", strData, strModelFilePath);

	strData.Format("%d", m_pMain->m_system_option_common.save_term_file_video);
	::WritePrivateProfileStringA(strSection, "SAVE_TERM_FILE_VIDEO", strData, strModelFilePath);

	strData.Format("%d", m_pMain->m_system_option_common.cycle_time_to_delete_file_auto_mode);
	::WritePrivateProfileStringA(strSection, "CYCLE_TIME_DELETE_FILE", strData, strModelFilePath);
}

void CFormSystemOptionView::InitNetWorkCard()
{
	CString strSeletedJob;

	_netWork.GetNumberOfCard();
	_netWork.GetInterfaceNameOfCard(&_netWork._interfaceName, &_netWork._DeviceName);

	if (_netWork._interfaceName.GetSize() > 0)
	{
		for (int i = 0; i < _netWork._interfaceName.GetSize(); i++)
		{
			strSeletedJob.Format("%s", _netWork._interfaceName.GetAt(i));
			((CComboBox*)GetDlgItem(IDC_COMBO_ADAPT_LIST))->AddString(strSeletedJob);

			strSeletedJob.Format("%s", _netWork._DeviceName.GetAt(i));
			((CComboBox*)GetDlgItem(IDC_COMBO_ADAPT_NAME))->AddString(strSeletedJob);
		}

		((CComboBox*)GetDlgItem(IDC_COMBO_ADAPT_LIST))->SetCurSel(0);
		((CComboBox*)GetDlgItem(IDC_COMBO_ADAPT_NAME))->SetCurSel(0);

		setChangeValue(0);
	}
}

void CFormSystemOptionView::OnBnClickedButtonChangeIp()
{
	int n = ((CComboBox*)GetDlgItem(IDC_COMBO_ADAPT_LIST))->GetCurSel();
	if (n >= 0)
	{
		CString newIPAddress, newMaskAddress,newGateWay;
	
		GetDlgItem(IDC_EDIT_IP_ADDRESS)->GetWindowTextA(newIPAddress);
		GetDlgItem(IDC_EDIT_SUBNET_MASK)->GetWindowTextA(newMaskAddress);
		GetDlgItem(IDC_EDIT_GATEWAY)->SetWindowTextA(newGateWay);

		if (newGateWay.IsEmpty()) newGateWay = "0.0.0.0";
		CString szCmd,cardName = _netWork._DeviceName.GetAt(n);
	//	_netWork.ChangeIPAddress(cardName, LPSTR(LPCTSTR(newIPAddress)), LPSTR(LPCTSTR(newMaskAddress)));

		CString interfaceName = _netWork._interfaceName.GetAt(n);
		//szCmd.Format("netsh - c int ip set address name = \"%s\" source = static addr = %s mask = %s gateway = %s gwmetric = 0", interfaceName, newIPAddress, newMaskAddress, newGateWay);
		szCmd.Format("netsh interface ipv4 set address name=\"%s\" static %s %s %s", interfaceName, newIPAddress, newMaskAddress, newGateWay);
		//netsh interface set interface "무선 네트워크 연결" enable/disable

		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		if (!CreateProcess(NULL, LPSTR(LPCTSTR(szCmd)), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		{
			AfxMessageBox("IP Change Error !");
			return ;
		}

		WaitForSingleObject(pi.hProcess, 3000);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
}

void CFormSystemOptionView::setChangeValue(int n)
{
	CString _ip, _subnet, _gateway;
	_ip = _netWork.GetCurrentIPAdress(_netWork._DeviceName.GetAt(n));
	_subnet = _netWork.GetCurrentSubnetMask(_netWork._DeviceName.GetAt(n));
	_gateway = _netWork.GetCurrentGateWay(_netWork._DeviceName.GetAt(n));

	GetDlgItem(IDC_EDIT_IP_ADDRESS)->SetWindowTextA(_ip);
	GetDlgItem(IDC_EDIT_SUBNET_MASK)->SetWindowTextA(_subnet);
	GetDlgItem(IDC_EDIT_GATEWAY)->SetWindowTextA(_gateway);
}

void CFormSystemOptionView::OnCbnSelchangeComboAdaptList()
{
	int n = ((CComboBox*)GetDlgItem(IDC_COMBO_ADAPT_LIST))->GetCurSel();
	if (n >= 0)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO_ADAPT_NAME))->SetCurSel(n);

		setChangeValue(n);
	}
}


void CFormSystemOptionView::OnCbnSelchangeComboAdaptName()
{
	int n = ((CComboBox*)GetDlgItem(IDC_COMBO_ADAPT_NAME))->GetCurSel();
	if (n >= 0)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO_ADAPT_LIST))->SetCurSel(n);
		setChangeValue(n);
	}
}

void CFormSystemOptionView::OnEnSetfocusEditIpAddress()
{
	GetDlgItem(IDC_STATIC_IP_ADDRESS)->SetFocus();

	CString str, strTemp;

	GetDlgItem(IDC_EDIT_IP_ADDRESS)->GetWindowTextA(strTemp);

	str.Format("%s", strTemp);
	strTemp = m_pMain->GetNumberBox(str, 256, -1, 256);

	GetDlgItem(IDC_EDIT_IP_ADDRESS)->SetWindowTextA(strTemp);
}

void CFormSystemOptionView::OnEnSetfocusEditSubnetMask()
{
	GetDlgItem(IDC_STATIC_SUBNET_MASK)->SetFocus();

	CString str, strTemp;

	GetDlgItem(IDC_EDIT_SUBNET_MASK)->GetWindowTextA(strTemp);

	str.Format("%s", strTemp);
	strTemp = m_pMain->GetNumberBox(str, 256, -1, 256);

	GetDlgItem(IDC_EDIT_SUBNET_MASK)->SetWindowTextA(strTemp);
}

void CFormSystemOptionView::OnEnSetfocusEditGateWay()
{
	GetDlgItem(IDC_STATIC_GATE)->SetFocus();

	CString str, strTemp;

	GetDlgItem(IDC_EDIT_GATEWAY)->GetWindowTextA(strTemp);

	str.Format("%s", strTemp);
	strTemp = m_pMain->GetNumberBox(str, 256, -1, 256);

	GetDlgItem(IDC_EDIT_GATEWAY)->SetWindowTextA(strTemp);
}