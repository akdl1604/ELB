// FormMainView.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "FormPlcShareMemoryView.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"

#include "CommPLC.h"
#include "ComPLC_RS.h"

// CFormPlcShareMemoryView

IMPLEMENT_DYNCREATE(CFormPlcShareMemoryView, CFormView)

CFormPlcShareMemoryView::CFormPlcShareMemoryView()
	: CFormView(CFormPlcShareMemoryView::IDD)
{
	InitializeCriticalSection( &m_csView );
	if (!xlsgen.IsInitialized() )
	{
		AfxMessageBox("make sure xlsgen.dll filepath is correctly declared.");
	}

	strIniFileName = "";
	m_nUnitSelectNum = 0;
	m_strProcess = " ";
	m_bBitSelection = TRUE;
	memset(m_nPlcFromPcBit, 0, sizeof(m_nPlcFromPcBit));
	memset(m_nPcFromPlcBit, 0, sizeof(m_nPcFromPlcBit));
	memset(m_nPcFromPlcBitStatus,0,sizeof(m_nPcFromPlcBitStatus));
	memset(m_nPlcFromPcBitStatus,0,sizeof(m_nPlcFromPcBitStatus));

	memset( m_ldPcFromPlcWord,0,sizeof(m_ldPcFromPlcWord));
	memset( m_ldPlcFromPcWord,0,sizeof(m_ldPlcFromPcWord));
}

CFormPlcShareMemoryView::~CFormPlcShareMemoryView()
{
	delete[] m_RowPcFromPlc_Data ;
	delete[] m_RowPlcFromPc_Data;
	DeleteCriticalSection( &m_csView );
	//xlsgen.~xlsgenLoader();
}

void CFormPlcShareMemoryView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM_GRID, m_GridInterfacePcFromPlc);
	DDX_Control(pDX,  IDC_CUSTOM_GRID2, m_GridInterfacePlcFromPc);
	DDX_Control(pDX, IDC_BUTTON_SEL_MAIN, m_btnMain);
	DDX_Control(pDX, IDC_BUTTON_SEL_INPUT_ALIGN_1, m_btnInputAlign1);
	DDX_Control(pDX, IDC_BUTTON_SEL_INPUT_ALIGN_2, m_btnInputAlign2);
	DDX_Control(pDX, IDC_BUTTON_SEL_PRECISION_ALIGN_1, m_btnPrecisionAlign1);
	DDX_Control(pDX, IDC_BUTTON_SEL_PRECISION_ALIGN_2, m_btnPrecisionAlign2);
	DDX_Control(pDX, IDC_BUTTON_SEL_PRECISION_ALIGN_3, m_btnPrecisionAlign3);
	DDX_Control(pDX, IDC_BUTTON_SEL_PRECISION_ALIGN_4, m_btnPrecisionAlign4);
	DDX_Control(pDX, IDC_BUTTON_SEL_SAMPLING_ALIGN, m_btnSamplingAlign);
	DDX_Control(pDX, IDC_BUTTON_SEL_INSPECTION, m_btnInspection);
	DDX_Control(pDX, IDC_BUTTON_SEL_LOAD_TRAY_GLASS_INSP, m_btnLoadTrayGlassInsp);
	DDX_Control(pDX, IDC_BUTTON_SEL_STAGE_DISPLACEMENT_SENSOR_1, m_btnStageDisplacementSensor1);
	DDX_Control(pDX, IDC_BUTTON_SEL_STAGE_DISPLACEMENT_SENSOR_2, m_btnStageDisplacementSensor2);
	DDX_Control(pDX, IDC_BUTTON_SEL_STAGE_DISPLACEMENT_SENSOR_3, m_btnStageDisplacementSensor3);
	DDX_Control(pDX, IDC_BUTTON_SEL_STAGE_DISPLACEMENT_SENSOR_4, m_btnStageDisplacementSensor4);
	DDX_Control(pDX, IDC_BUTTON_SEL_UV_MEASURE_1, m_btnUvMeasure1);
	DDX_Control(pDX, IDC_BUTTON_SEL_UV_MEASURE_2, m_btnUvMeasure2);
	DDX_Control(pDX, IDC_BUTTON_SEL_BCR_1, m_btnBCR1);
	DDX_Control(pDX, IDC_BUTTON_SEL_BCR_2, m_btnBCR2);
	DDX_Control(pDX, IDC_BUTTON_SEL_LOAD_TRAY_BCR, m_btnLoadTrayBCR);
	DDX_Control(pDX, IDC_BUTTON_SEL_SPARE, m_btnSpare);
	DDX_Control(pDX, IDC_BUTTON_SEL_SPARE2, m_btnSpare2);
	DDX_Control(pDX, IDC_BUTTON_SEL_SPARE3, m_btnSpare3);

	DDX_Control(pDX, IDC_STATIC_TITLE, m_LbPlcViewTitle);

	DDX_Control(pDX,  IDC_STATIC_SEL_UNIT,m_grpUnitSelect1);
	DDX_Control(pDX,  IDC_STATIC_SEL_UNIT2,m_grpUnitSelect2);
	DDX_Control(pDX,  IDC_GROUP_BOX2,m_grpPcToPlc);
	DDX_Control(pDX,  IDC_GROUP_BOX1,m_grpPlcToPC);
	DDX_Control(pDX, IDC_STATIC_PROCESS_PLC_HISTORY, m_lblHistoryTitle);
	DDX_Control(pDX, IDC_EDIT_PROCESS, m_EditProcess);
}

BEGIN_MESSAGE_MAP(CFormPlcShareMemoryView, CFormView)
	ON_MESSAGE(WM_VIEW_CONTROL, OnViewControl)
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_BUTTON_SEL_MAIN, &CFormPlcShareMemoryView::OnBnClickedButtonSelMain)
	ON_BN_CLICKED(IDC_BUTTON_SEL_INPUT_ALIGN_1, &CFormPlcShareMemoryView::OnBnClickedButtonSelInputAlign1)
	ON_BN_CLICKED(IDC_BUTTON_SEL_INPUT_ALIGN_2, &CFormPlcShareMemoryView::OnBnClickedButtonSelInputAlign2)
	ON_BN_CLICKED(IDC_BUTTON_SEL_PRECISION_ALIGN_1, &CFormPlcShareMemoryView::OnBnClickedButtonSelPrecisionAlign1)
	ON_BN_CLICKED(IDC_BUTTON_SEL_PRECISION_ALIGN_2, &CFormPlcShareMemoryView::OnBnClickedButtonSelPrecisionAlign2)
	ON_BN_CLICKED(IDC_BUTTON_SEL_PRECISION_ALIGN_3, &CFormPlcShareMemoryView::OnBnClickedButtonSelPrecisionAlign3)
	ON_BN_CLICKED(IDC_BUTTON_SEL_PRECISION_ALIGN_4, &CFormPlcShareMemoryView::OnBnClickedButtonSelPrecisionAlign4)
	ON_BN_CLICKED(IDC_BUTTON_SEL_SAMPLING_ALIGN, &CFormPlcShareMemoryView::OnBnClickedButtonSelSamplingAlign)
	ON_BN_CLICKED(IDC_BUTTON_SEL_INSPECTION, &CFormPlcShareMemoryView::OnBnClickedButtonSelInspection)
	ON_BN_CLICKED(IDC_BUTTON_SEL_LOAD_TRAY_GLASS_INSP, &CFormPlcShareMemoryView::OnBnClickedButtonSelLoadTrayGlassInsp)
	ON_BN_CLICKED(IDC_BUTTON_SEL_STAGE_DISPLACEMENT_SENSOR_1, &CFormPlcShareMemoryView::OnBnClickedButtonSelStageDisplacementSensor1)
	ON_BN_CLICKED(IDC_BUTTON_SEL_STAGE_DISPLACEMENT_SENSOR_2, &CFormPlcShareMemoryView::OnBnClickedButtonSelStageDisplacementSensor2)
	ON_BN_CLICKED(IDC_BUTTON_SEL_STAGE_DISPLACEMENT_SENSOR_3, &CFormPlcShareMemoryView::OnBnClickedButtonSelStageDisplacementSensor3)
	ON_BN_CLICKED(IDC_BUTTON_SEL_STAGE_DISPLACEMENT_SENSOR_4, &CFormPlcShareMemoryView::OnBnClickedButtonSelStageDisplacementSensor4)
	ON_BN_CLICKED(IDC_BUTTON_SEL_UV_MEASURE_1, &CFormPlcShareMemoryView::OnBnClickedButtonSelUvMeasure1)
	ON_BN_CLICKED(IDC_BUTTON_SEL_UV_MEASURE_2, &CFormPlcShareMemoryView::OnBnClickedButtonSelUvMeasure2)
	ON_BN_CLICKED(IDC_BUTTON_SEL_BCR_1, &CFormPlcShareMemoryView::OnBnClickedButtonSelBcr1)
	ON_BN_CLICKED(IDC_BUTTON_SEL_BCR_2, &CFormPlcShareMemoryView::OnBnClickedButtonSelBcr2)
	ON_BN_CLICKED(IDC_BUTTON_SEL_LOAD_TRAY_BCR, &CFormPlcShareMemoryView::OnBnClickedButtonSelLoadTrayBcr)
	ON_BN_CLICKED(IDC_BUTTON_SEL_SPARE, &CFormPlcShareMemoryView::OnBnClickedButtonSelSpare)
	ON_BN_CLICKED(IDC_BUTTON_SEL_SPARE2, &CFormPlcShareMemoryView::OnBnClickedButtonSelSpare2)
	ON_BN_CLICKED(IDC_BUTTON_SEL_SPARE3, &CFormPlcShareMemoryView::OnBnClickedButtonSelSpare3)
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


// CFormPlcShareMemoryView 진단입니다.

#ifdef _DEBUG
void CFormPlcShareMemoryView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CFormPlcShareMemoryView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CFormPlcShareMemoryView 메시지 처리기입니다.

// CPaneManual 메시지 처리기입니다.
void CFormPlcShareMemoryView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다. 
	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);

	m_pMain = (CLET_AlignClientDlg *)AfxGetMainWnd();

	InitTitle( &m_LbPlcViewTitle, "PLC Communication Client(Bit)", 24.f, RGB(64, 192, 64));
	InitTitle( &m_lblHistoryTitle, "Process History", 24.f, RGB(100,149,237));

	MainButtonInit(&m_btnMain);
	MainButtonInit(&m_btnInputAlign1);
	MainButtonInit(&m_btnInputAlign2);
	MainButtonInit(&m_btnPrecisionAlign1);
	MainButtonInit(&m_btnPrecisionAlign2);
	MainButtonInit(&m_btnPrecisionAlign3);
	MainButtonInit(&m_btnPrecisionAlign4);
	MainButtonInit(&m_btnSamplingAlign);
	MainButtonInit(&m_btnInspection);
	MainButtonInit(&m_btnLoadTrayGlassInsp);
	MainButtonInit(&m_btnStageDisplacementSensor1);
	MainButtonInit(&m_btnStageDisplacementSensor2);
	MainButtonInit(&m_btnStageDisplacementSensor3);
	MainButtonInit(&m_btnStageDisplacementSensor4);
	MainButtonInit(&m_btnUvMeasure1);
	MainButtonInit(&m_btnUvMeasure2);
	MainButtonInit(&m_btnBCR1);
	MainButtonInit(&m_btnBCR2);
	MainButtonInit(&m_btnLoadTrayBCR);
	MainButtonInit(&m_btnSpare);
	MainButtonInit(&m_btnSpare2);
	MainButtonInit(&m_btnSpare3);

	m_grpUnitSelect1.SetColorText(255,255,255,255);
	m_grpUnitSelect2.SetColorText(255,255,255,255);
	m_grpPcToPlc.SetColorText(255,255,255,255);
	m_grpPlcToPC.SetColorText(255,255,255,255);

	m_EditProcess.SetAlignLeft();				// 글자 정렬
	m_EditProcess.SetSizeText(20);				// 글자 크기
	m_EditProcess.SetStyleTextBold(true);		// 글자 스타일
	m_EditProcess.SetTextMargins(10,10);		// 글자 옵셋
	m_EditProcess.SetColorText(RGB(255,255,255));	// 글자 색상

	addProcessHistory("Program Start");

	InitGuiControl();
	ReadExcelData();

	SetTimer(9235, 100, NULL);
}

HBRUSH CFormPlcShareMemoryView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{	
	HBRUSH hbr=CFormView::OnCtlColor(pDC, pWnd, nCtlColor);  
	if(pWnd->m_hWnd==this->m_hWnd) hbr=m_hbrBkg;	

	return hbr;
}

void CFormPlcShareMemoryView::InitTitle(CLabelEx *pTitle,CString str,float size,COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}


void CFormPlcShareMemoryView::MainButtonInit(CButtonEx *pbutton,int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}


LRESULT CFormPlcShareMemoryView::OnViewControl(WPARAM wParam, LPARAM lParam)
{
	KillTimer(9235);
	return 0;
}


void CFormPlcShareMemoryView::addProcessHistory(CString str)
{
	EnterCriticalSection( &m_csView );
	CString strTime;
	SYSTEMTIME	csTime;
	::GetLocalTime(&csTime);
	strTime.Format("[%02d:%02d:%02d:%03d] ", csTime.wHour, csTime.wMinute, csTime.wSecond, csTime.wMilliseconds);
//	strTime.Format("[%02d:%02d:%02d:%03d] ", csTime.wHour, csTime.wMinute, csTime.wSecond, csTime.wMilliseconds);
	
	if( m_strProcess.GetLength() > 4096 ) 
		m_strProcess.Delete(2048, 2048);

	m_strProcess = strTime + str + m_strProcess;
	m_EditProcess.SetWindowTextA(m_strProcess);
	LeaveCriticalSection( &m_csView );
}

void CFormPlcShareMemoryView::InitGuiControl()
{

	m_GridInterfacePcFromPlc.ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_FRAMECHANGED);
	m_GridInterfacePlcFromPc.ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_FRAMECHANGED);

	int nRow = PLC_UNIT_ADDRESS_SIZE+1;
	int nCol = 2;

	m_RowPcFromPlc_Data = new CArray<CellData>[PLC_UNIT_ADDRESS_SIZE + 1];
	m_RowPcFromPlc_Data->SetSize(PLC_UNIT_ADDRESS_SIZE + 1);

	m_RowPlcFromPc_Data = new CArray<CellData>[PLC_UNIT_ADDRESS_SIZE + 1];
	m_RowPlcFromPc_Data->SetSize(PLC_UNIT_ADDRESS_SIZE + 1);


	for(int i = 0; i< PLC_UNIT_ADDRESS_SIZE+1; i++)
	{
		m_RowPcFromPlc_Data[i].SetSize(nCol);
		for(int j = 0; j < 2; j++)
		{
			CellData data;	
			data.row = i; 
			data.col = j;
			data.data.Format("%d", i * j);	
			
			m_RowPcFromPlc_Data[i].SetAt(j, data);
		}
	}

	for(int i = 0; i < PLC_UNIT_ADDRESS_SIZE+1; i++)
	{
		m_RowPlcFromPc_Data[i].SetSize(nCol);
		for(int j = 0; j < 2; j++)
		{
			CellData data;	
			data.row = i; 
			data.col = j;
			data.data.Format("%d", i * j);	
			
			m_RowPlcFromPc_Data[i].SetAt(j, data);
		}
	}

	DrawGrid(m_GridInterfacePcFromPlc, 2, PLC_UNIT_ADDRESS_SIZE + 1);
	DrawGrid(m_GridInterfacePlcFromPc, 2, PLC_UNIT_ADDRESS_SIZE + 1);
	
	DrawGridPcFromPlc(m_GridInterfacePcFromPlc, 2, PLC_UNIT_ADDRESS_SIZE + 1);
	DrawGridPlcFromPc(m_GridInterfacePlcFromPc, 2, PLC_UNIT_ADDRESS_SIZE + 1);
}

void CFormPlcShareMemoryView::DrawGrid(CGridCtrl &grid, int col, int row)
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
	grid.SetFixedRowCount(1);		

	grid.SetBkColor(RGB(197, 197, 187));
	grid.SetTextBkColor(RGB(235, 235, 235));

	for(int i=0;i<grid.GetColumnCount();i++)
	{
		grid.SetItemBkColour(0, i, RGB(146,146,141));
		grid.SetItemFgColour(0, i, RGB(255,255,255));
		if(i>0) grid.SetColumnWidth(i,300);		
	}

	for(int i=0;i<grid.GetRowCount();i++)
		grid.SetRowHeight(i, 35);

}

void CFormPlcShareMemoryView::DrawGridPcFromPlc(CGridCtrl &grid, int col, int row)
{
	DrawGrid(grid, col, row);

	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nRowCount = 1;


	for (int row = 0; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row;
			Item.col = col;

			// Fixed Cell Title
			if (row == 0 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("BIT"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 0 && col == 1)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("INFO"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE);
			}
			//else if (col < 1) 
			//{
			//	Item.nFormat = dwTextStyle;
			//	Item.strText.Format(_T("%d"), row - 1);
			//	UINT state = grid.GetItemState(row, col);
			//	grid.SetItemState(row, col, state | GVIS_READONLY);

			//} 
			else
			{
				Item.nFormat = dwTextStyle;
				//Item.strText.Format(_T("Item %d"),row*col);

				int nAddr = m_nPcFromPlcBit[m_nUnitSelectNum][row - 1];

				CellData data = m_RowPcFromPlc_Data[row].GetAt(col);
				Item.strText = data.data;

				BOOL bOnOff = g_CommPLC.GetBit(nAddr);
				if (col == 0)
				{
					if( bOnOff)		grid.SetItemBkColour(row, col, RGB(0, 255, 0));
					else			grid.SetItemBkColour(row, col, RGB(255, 0, 0));
				}
				/*
				if (col == 0)
				{
					if (data.status)		grid.SetItemBkColour(row, col, RGB(0, 255, 0));
					else					grid.SetItemBkColour(row, col, RGB(255, 0, 0));
				}
				*/
			}

			if (row > 0 && col > 0)
			{

				if (nRowCount <= 5)			grid.SetItemBkColour(row, col, RGB(235, 235, 235));
				else        				grid.SetItemBkColour(row, col, RGB(255, 255, 255));

				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state);
			}

			grid.SetItem(&Item);

		}

		if (row > 0)			nRowCount++;
		if (nRowCount > 10)		nRowCount = 1;
	}


	// 1row 1col : set read-only
	grid.SetItemState(1, 1, grid.GetItemState(1, 1) | GVIS_READONLY);
}

void CFormPlcShareMemoryView::DrawGridPlcFromPc(CGridCtrl &grid, int col, int row)
{
	DrawGrid(grid,col, row);

	DWORD dwTextStyle = DT_CENTER|DT_VCENTER|DT_SINGLELINE;    // Text Style
	int nRowCount = 1;


		for (int row = 0; row < grid.GetRowCount(); row++) 
		{
			for (int col = 0; col < grid.GetColumnCount(); col++) 
			{ 
				GV_ITEM Item;
				Item.mask = GVIF_TEXT|GVIF_FORMAT;
				Item.row = row;
				Item.col = col;

				// Fixed Cell Title
				if (row == 0 && col==0)
				{
					Item.nFormat = DT_CENTER|DT_WORDBREAK;
					Item.strText.Format(_T("BIT"), col);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_READONLY);
				} 
				else if (row == 0 && col==1) 
				{
					Item.nFormat = DT_CENTER|DT_WORDBREAK;
					Item.strText.Format(_T("INFO"), col);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_READONLY);
					//grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE);
				}
				else 
				{
					Item.nFormat = dwTextStyle;
					//Item.strText.Format(_T("Item %d"),row*col);

					CellData data = m_RowPlcFromPc_Data[row].GetAt(col);
					Item.strText = data.data;
					
					
					int nAddr = m_nPlcFromPcBit[m_nUnitSelectNum][row - 1];

					BOOL bOnOff = g_CommPLC.GetWriteBit(nAddr);
					if (col == 0)
					{
						if (bOnOff)		grid.SetItemBkColour(row, col, RGB(0, 255, 0));
						else			grid.SetItemBkColour(row, col, RGB(255, 0, 0));
					}

					
					/*if (col==0)
					{
						if(data.status)		grid.SetItemBkColour(row, col, RGB(0,255,0));
						else grid.SetItemBkColour(row, col, RGB(255,0,0));
					}*/
				}

				if(row > 0 && col>0)
				{
					if (nRowCount <= 5)
						grid.SetItemBkColour(row, col, RGB(235,235,235));
					else
						grid.SetItemBkColour(row, col, RGB(255,255,255));

					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state);
				}

				grid.SetItem(&Item);  

			}

			if(row > 0)
				nRowCount++;

			if(nRowCount > 10)
				nRowCount = 1;
		}


	// 1row 1col : set read-only
	//grid.SetItemState(1, 1, grid.GetItemState(1,1) | GVIS_READONLY);

}

void CFormPlcShareMemoryView::GridDisplay_PcFromPlc()
{
	CString strTemp;

	for (int row = 1; row < m_GridInterfacePcFromPlc.GetRowCount(); row++) 
	{
		CellData data;			
		data.row = row;
		data.col = 0;		
		data.status = m_nPcFromPlcBitStatus[m_nUnitSelectNum][row - 1]; 
		data.data.Format(_T("L%d"), m_nPcFromPlcBit[m_nUnitSelectNum][row - 1] );
		m_RowPcFromPlc_Data[row].SetAt(0, data);
		
		data.col = 1;		
		data.status = m_nPcFromPlcBitStatus[m_nUnitSelectNum][row-1]; 
		if(m_bBitSelection)
			data.data.Format(_T("%s"), m_strPcFromPlc[m_nUnitSelectNum][row-1]);
		else
		{
			data.data.Format(_T("%s(%d_%d)"), m_strPcFromPlcWord[m_nUnitSelectNum][row-1],m_ldPcFromPlcWord[row-1],row<=9?m_ldPcFromPlcWord[row-1]+(m_ldPcFromPlcWord[row]<<16):0);
		}

		m_RowPcFromPlc_Data[row].SetAt(1, data);

	}

	DrawGridPcFromPlc(m_GridInterfacePcFromPlc, 2, PLC_UNIT_ADDRESS_SIZE + 1);	

	UpdateData(FALSE);
}

void CFormPlcShareMemoryView::GridDisplay_PlcFromPc()
{
	CString strTemp;

	for (int row = 1; row < m_GridInterfacePlcFromPc.GetRowCount(); row++) 
	{
		CellData data;
		data.row = row;
		data.col = 0;
		data.status = m_nPlcFromPcBitStatus[m_nUnitSelectNum][row - 1];
		data.data.Format(_T("M%d"), m_nPlcFromPcBit[m_nUnitSelectNum][row - 1] );
		m_RowPlcFromPc_Data[row].SetAt(0, data);
		
		data.col = 1;
		data.status = m_nPlcFromPcBitStatus[m_nUnitSelectNum][row - 1]; 
		if(m_bBitSelection)		data.data.Format(_T("%s"), m_strPlcFromPc[m_nUnitSelectNum][row - 1] );
		else					data.data.Format(_T("%s(%d_0X%04X)"), m_strPlcFromPcWord[m_nUnitSelectNum][row - 1], m_ldPlcFromPcWord[row - 1], m_ldPlcFromPcWord[row - 1]);
	
		m_RowPlcFromPc_Data[row].SetAt(1, data);
	}

	DrawGridPlcFromPc(m_GridInterfacePlcFromPc, 2, PLC_UNIT_ADDRESS_SIZE + 1);	

	UpdateData(FALSE);
}

BOOL CFormPlcShareMemoryView::ReadExcelData()
{
	CString strTemp;
	CString strInterfaceMapDir = m_pMain->m_strSystemDir;

	if ( _access( strInterfaceMapDir, 0 ) != 0)	CreateDirectory(strInterfaceMapDir, NULL);

	strTemp.Format("%sInterfaceMap.xls", strInterfaceMapDir);

	int lastPt=0;

	try
	{
		CFile file;

		xlsgen::IXlsEnginePtr engine = xlsgen.Start();

		xlsgen::IXlsWorkbookPtr wbk;
		xlsgen::IXlsWorksheetPtr wksht;

		// 파일 존재 시.
		if( (_access( strTemp, 0 )) != -1 ) 
		{
			
			if(file.Open(strTemp,CFile::modeRead | CFile::modeNoTruncate))
			{
			file.m_hFile;
				file.Close();
			}
			else
			{
				theLog.logmsg(LOG_ERROR, "CFormPlcShareMemoryView::WriteExcelData() Failed");
		//	file.m_hFile;

			//	file.Close();
				if(file.Open(strTemp,CFile::modeRead | CFile::modeNoTruncate))			file.Close();
				else
				{
					theLog.logmsg(LOG_ERROR, "CFormPlcShareMemoryView::WriteExcelData() Failed");
					return TRUE;
				}
			}

			wbk = engine->Open(_bstr_t(strTemp),_bstr_t(strTemp));
			wksht = wbk->GetWorksheetByName( L"InterfaceMap");
			//lastPt = wksht->Dimensions->LastRow;
			lastPt = 1;

			int index=0;

			for(int i = 0; i < 22; i++)
			{
				for(int j = 0; j < PLC_UNIT_ADDRESS_SIZE; j++)
				{
					m_nPcFromPlcBit[i][j] = wksht->Number[lastPt + 1][2];
					strTempPcFromPlc[i][j] = wksht->Label[lastPt + 1][3];
					m_strPcFromPlc[i][j] = " ";
					m_strPcFromPlc[i][j] = (LPCTSTR)(LPTSTR) strTempPcFromPlc[i][j];

					m_nPlcFromPcBit[i][j] = wksht->Number[lastPt + 1][4];
					strTempPlcFromPc[i][j] = wksht->Label[lastPt + 1][5];
					m_strPlcFromPc[i][j] = " ";
					m_strPlcFromPc[i][j] = (LPCTSTR)(LPTSTR) strTempPlcFromPc[i][j];

					strTempPcFromPlc[i][j] = wksht->Label[lastPt + 1][8];
					m_strPcFromPlcWord[i][j] = " ";
					m_strPcFromPlcWord[i][j] = (LPCTSTR)(LPTSTR) strTempPcFromPlc[i][j];

					strTempPlcFromPc[i][j] = wksht->Label[lastPt + 1][10];
					m_strPlcFromPcWord[i][j] = " ";
					m_strPlcFromPcWord[i][j] = (LPCTSTR)(LPTSTR) strTempPlcFromPc[i][j];

					lastPt++;
				}

				switch(i)
				{
					case 0: m_btnMain.SetText((LPCTSTR)(LPTSTR)wksht->Label[i+2][1]); break;
					case 1: m_btnInputAlign1.SetText((LPCTSTR)(LPTSTR)wksht->Label[PLC_UNIT_ADDRESS_SIZE*i+2][1]); break;
					case 2: m_btnInputAlign2.SetText((LPCTSTR)(LPTSTR)wksht->Label[PLC_UNIT_ADDRESS_SIZE*i+2][1]); break;
					case 3: m_btnPrecisionAlign1.SetText((LPCTSTR)(LPTSTR)wksht->Label[PLC_UNIT_ADDRESS_SIZE*i+2][1]); break;
					case 4: m_btnPrecisionAlign2.SetText((LPCTSTR)(LPTSTR)wksht->Label[PLC_UNIT_ADDRESS_SIZE*i+2][1]); break;
					case 5: m_btnPrecisionAlign3.SetText((LPCTSTR)(LPTSTR)wksht->Label[PLC_UNIT_ADDRESS_SIZE*i+2][1]); break;
					case 6: m_btnPrecisionAlign4.SetText((LPCTSTR)(LPTSTR)wksht->Label[PLC_UNIT_ADDRESS_SIZE*i+2][1]); break;
					case 7: m_btnSamplingAlign.SetText((LPCTSTR)(LPTSTR)wksht->Label[PLC_UNIT_ADDRESS_SIZE*i+2][1]); break;
					case 8: m_btnInspection.SetText((LPCTSTR)(LPTSTR)wksht->Label[PLC_UNIT_ADDRESS_SIZE*i+2][1]); break;
					case 9: m_btnLoadTrayGlassInsp.SetText((LPCTSTR)(LPTSTR)wksht->Label[PLC_UNIT_ADDRESS_SIZE*i+2][1]); break;
					case 10: m_btnStageDisplacementSensor1.SetText((LPCTSTR)(LPTSTR)wksht->Label[PLC_UNIT_ADDRESS_SIZE*i+2][1]); break;
					case 11: m_btnStageDisplacementSensor2.SetText((LPCTSTR)(LPTSTR)wksht->Label[PLC_UNIT_ADDRESS_SIZE*i+2][1]); break;
					case 12: m_btnStageDisplacementSensor3.SetText((LPCTSTR)(LPTSTR)wksht->Label[PLC_UNIT_ADDRESS_SIZE*i+2][1]); break;
					case 13: m_btnStageDisplacementSensor4.SetText((LPCTSTR)(LPTSTR)wksht->Label[PLC_UNIT_ADDRESS_SIZE*i+2][1]); break;
					case 14: m_btnUvMeasure1.SetText((LPCTSTR)(LPTSTR)wksht->Label[PLC_UNIT_ADDRESS_SIZE*i+2][1]); break;
					case 15: m_btnUvMeasure2.SetText((LPCTSTR)(LPTSTR)wksht->Label[PLC_UNIT_ADDRESS_SIZE*i+2][1]); break;
					case 16:m_btnBCR1.SetText((LPCTSTR)(LPTSTR)wksht->Label[PLC_UNIT_ADDRESS_SIZE*i+2][1]); break;
					case 17:m_btnBCR2.SetText((LPCTSTR)(LPTSTR)wksht->Label[PLC_UNIT_ADDRESS_SIZE*i+2][1]); break;
					case 18:m_btnLoadTrayBCR.SetText((LPCTSTR)(LPTSTR)wksht->Label[PLC_UNIT_ADDRESS_SIZE*i+2][1]); break;
					case 19:m_btnSpare.SetText((LPCTSTR)(LPTSTR)wksht->Label[PLC_UNIT_ADDRESS_SIZE*i+2][1]); break;
					case 20:m_btnSpare2.SetText((LPCTSTR)(LPTSTR)wksht->Label[PLC_UNIT_ADDRESS_SIZE*i+2][1]); break;
					case 21:m_btnSpare3.SetText((LPCTSTR)(LPTSTR)wksht->Label[PLC_UNIT_ADDRESS_SIZE*i+2][1]); break;
				}
			}
			wbk->Close();
			//theLog.logmsg(LOG_INSPECTOR, "CFormPlcShareMemoryView::WriteExcelData() Start");
		}
		
	}
	catch (...)
	{
		
		OutputDebugString ("some error has occured. Make sure to review your code");
	}
	

	return TRUE;
	
}

void CFormPlcShareMemoryView::OnTimer(UINT_PTR nIDEvent)
{
	GridDisplay_PcFromPlc();
	GridDisplay_PlcFromPc();

	if(!m_bBitSelection) changeWordValue(m_nUnitSelectNum);

	//if(g_pFrame->getSocketConnect()) changeConnectStatus(1,1);
	//else changeConnectStatus(1,0);
	//if(g_pFrame->getPLCConnect()) changeConnectStatus(0,1);
	//else changeConnectStatus(0,0);
	
	CFormView::OnTimer(nIDEvent);
}

BOOL CFormPlcShareMemoryView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	
	if(wParam==(WPARAM)m_GridInterfacePcFromPlc.GetDlgCtrlID())	// PLC -> PC	L_Lami 기준 L
	{
		GV_DISPINFO *pDispInfo = (GV_DISPINFO *)lParam;
		if(pDispInfo->item.col==0 && pDispInfo->item.row>0 && pDispInfo->hdr.code == 4294967196)
		{
			int nAddr = m_nPcFromPlcBit[m_nUnitSelectNum][pDispInfo->item.row - 1];

			if (g_CommPLC.GetBit(nAddr))	g_CommPLC.SetBitL(nAddr, FALSE);
			else							g_CommPLC.SetBitL(nAddr, TRUE);

			/*if(m_nPcFromPlcBitStatus[m_nUnitSelectNum][pDispInfo->item.row - 1])
				g_CommPLC.SetBitL(m_nPcFromPlcBit[m_nUnitSelectNum][pDispInfo->item.row - 1], FALSE);
			else
				g_CommPLC.SetBitL(m_nPcFromPlcBit[m_nUnitSelectNum][pDispInfo->item.row - 1], TRUE);*/
		}
	}
	else if(wParam==(WPARAM)m_GridInterfacePlcFromPc.GetDlgCtrlID())		// PC -> PLC	L_Lami 기준 M
	{
		GV_DISPINFO *pDispInfo = (GV_DISPINFO *)lParam;
		if(pDispInfo->item.col == 0 && pDispInfo->item.row > 0 && pDispInfo->hdr.code == 4294967196)
		{
			int nAddr = m_nPlcFromPcBit[m_nUnitSelectNum][pDispInfo->item.row - 1];

			if (g_CommPLC.GetWriteBit(nAddr))	g_CommPLC.SetBit(nAddr, FALSE);
			else								g_CommPLC.SetBit(nAddr, TRUE);


			/*if(m_nPlcFromPcBitStatus[m_nUnitSelectNum][pDispInfo->item.row - 1])
				g_CommPLC.SetBit(m_nPlcFromPcBit[m_nUnitSelectNum][pDispInfo->item.row - 1], FALSE);
			else
				g_CommPLC.SetBit(m_nPlcFromPcBit[m_nUnitSelectNum][pDispInfo->item.row - 1], TRUE);*/
		}
	}

	return CFormView::OnNotify(wParam, lParam, pResult);
}

void CFormPlcShareMemoryView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	if(nFlags == MK_LBUTTON)
	{
		//::ReleaseCapture();
		//HWND hWndParent = g_pFrame->GetSafeHwnd();
		//::SendMessage(hWndParent, WM_NCLBUTTONDOWN, HTCAPTION, 0);
	}

	CFormView::OnMouseMove(nFlags, point);
}

void CFormPlcShareMemoryView::changeButtonColor(int pre, int cur)
{
	switch(pre)
	{
	case 0: m_btnMain.SetColorBkg(255, RGB(64, 64, 64));		 break;
	case 1: m_btnInputAlign1.SetColorBkg(255, RGB(64, 64, 64));		break;
	case 2: m_btnInputAlign2.SetColorBkg(255, RGB(64, 64, 64));		break;
	case 3: m_btnPrecisionAlign1.SetColorBkg(255, RGB(64, 64, 64));		break;
	case 4: m_btnPrecisionAlign2.SetColorBkg(255, RGB(64, 64, 64));		break;
	case 5: m_btnPrecisionAlign3.SetColorBkg(255, RGB(64, 64, 64));		break;
	case 6: m_btnPrecisionAlign4.SetColorBkg(255, RGB(64, 64, 64));		break;
	case 7: m_btnSamplingAlign.SetColorBkg(255, RGB(64, 64, 64));		break;
	case 8: m_btnInspection.SetColorBkg(255, RGB(64, 64, 64));		break;
	case 9: m_btnLoadTrayGlassInsp.SetColorBkg(255, RGB(64, 64, 64));		break;
	case 10: m_btnStageDisplacementSensor1.SetColorBkg(255, RGB(64, 64, 64));		break;
	case 11: m_btnStageDisplacementSensor2.SetColorBkg(255, RGB(64, 64, 64));		break;
	case 12: m_btnStageDisplacementSensor3.SetColorBkg(255, RGB(64, 64, 64));		break;
	case 13: m_btnStageDisplacementSensor4.SetColorBkg(255, RGB(64, 64, 64));		break;
	case 14: m_btnUvMeasure1.SetColorBkg(255, RGB(64, 64, 64));		break;
	case 15: m_btnUvMeasure2.SetColorBkg(255, RGB(64, 64, 64));		break;
	case 16: m_btnBCR1.SetColorBkg(255, RGB(64, 64, 64));		break;
	case 17: m_btnBCR2.SetColorBkg(255, RGB(64, 64, 64));		break;
	case 18: m_btnLoadTrayBCR.SetColorBkg(255, RGB(64, 64, 64));		break;
	case 19: m_btnSpare.SetColorBkg(255, RGB(64, 64, 64));		break;
	case 20: m_btnSpare2.SetColorBkg(255, RGB(64, 64, 64));		break;
	case 21: m_btnSpare3.SetColorBkg(255, RGB(64, 64, 64));		break;
	}
	
	switch(cur)
	{
	case 0: m_btnMain.SetColorBkg(255, RGB(0,128,0));		break;
	case 1: m_btnInputAlign1.SetColorBkg(255, RGB(0,128,0));		break;
	case 2: m_btnInputAlign2.SetColorBkg(255, RGB(0,128,0));		break;
	case 3: m_btnPrecisionAlign1.SetColorBkg(255, RGB(0,128,0));		break;
	case 4: m_btnPrecisionAlign2.SetColorBkg(255, RGB(0,128,0));		break;
	case 5: m_btnPrecisionAlign3.SetColorBkg(255, RGB(0,128,0));		break;
	case 6: m_btnPrecisionAlign4.SetColorBkg(255, RGB(0,128,0));		break;
	case 7: m_btnSamplingAlign.SetColorBkg(255, RGB(0,128,0));		break;
	case 8: m_btnInspection.SetColorBkg(255, RGB(0,128,0));		break;
	case 9: m_btnLoadTrayGlassInsp.SetColorBkg(255, RGB(0,128,0));		break;
	case 10: m_btnStageDisplacementSensor1.SetColorBkg(255, RGB(0,128,0));		break;
	case 11: m_btnStageDisplacementSensor2.SetColorBkg(255, RGB(0,128,0));		break;
	case 12: m_btnStageDisplacementSensor3.SetColorBkg(255, RGB(0,128,0));		break;
	case 13: m_btnStageDisplacementSensor4.SetColorBkg(255, RGB(0,128,0));		break;
	case 14: m_btnUvMeasure1.SetColorBkg(255, RGB(0,128,0));		break;
	case 15: m_btnUvMeasure2.SetColorBkg(255, RGB(0,128,0));		break;
	case 16: m_btnBCR1.SetColorBkg(255, RGB(0,128,0));		break;
	case 17: m_btnBCR2.SetColorBkg(255, RGB(0,128,0));		break;
	case 18: m_btnLoadTrayBCR.SetColorBkg(255, RGB(0,128,0));		break;
	case 19: m_btnSpare.SetColorBkg(255, RGB(0,128,0));		break;
	case 20: m_btnSpare2.SetColorBkg(255, RGB(0,128,0));		break;
	case 21: m_btnSpare3.SetColorBkg(255, RGB(0,128,0));		break;
	}
	
	m_nUnitSelectNum = cur;
	GridDisplay_PcFromPlc();
	GridDisplay_PlcFromPc();
}
	
void CFormPlcShareMemoryView::OnBnClickedButtonSelMain()
{
	changeButtonColor(m_nUnitSelectNum,0);
	
	if(!m_bBitSelection) changeWordValue(0);
}


void CFormPlcShareMemoryView::OnBnClickedButtonSelInputAlign1()
{
	changeButtonColor(m_nUnitSelectNum,1);
	if(!m_bBitSelection) changeWordValue(1);
}


void CFormPlcShareMemoryView::OnBnClickedButtonSelInputAlign2()
{
	changeButtonColor(m_nUnitSelectNum,2);
	if(!m_bBitSelection) changeWordValue(2);
}


void CFormPlcShareMemoryView::OnBnClickedButtonSelPrecisionAlign1()
{
	changeButtonColor(m_nUnitSelectNum,3);
	if(!m_bBitSelection) changeWordValue(3);
}


void CFormPlcShareMemoryView::OnBnClickedButtonSelPrecisionAlign2()
{
	changeButtonColor(m_nUnitSelectNum,4);
	if(!m_bBitSelection) changeWordValue(4);
}


void CFormPlcShareMemoryView::OnBnClickedButtonSelPrecisionAlign3()
{
	changeButtonColor(m_nUnitSelectNum,5);
	if(!m_bBitSelection) changeWordValue(5);
}


void CFormPlcShareMemoryView::OnBnClickedButtonSelPrecisionAlign4()
{
	changeButtonColor(m_nUnitSelectNum,6);
	if(!m_bBitSelection) changeWordValue(6);
}


void CFormPlcShareMemoryView::OnBnClickedButtonSelSamplingAlign()
{
	changeButtonColor(m_nUnitSelectNum,7);
	if(!m_bBitSelection) changeWordValue(7);
}


void CFormPlcShareMemoryView::OnBnClickedButtonSelInspection()
{
	changeButtonColor(m_nUnitSelectNum,8);
	if(!m_bBitSelection) changeWordValue(9);
}


void CFormPlcShareMemoryView::OnBnClickedButtonSelLoadTrayGlassInsp()
{
	changeButtonColor(m_nUnitSelectNum,9);
	if(!m_bBitSelection) changeWordValue(9);
}


void CFormPlcShareMemoryView::OnBnClickedButtonSelStageDisplacementSensor1()
{
	changeButtonColor(m_nUnitSelectNum,10);
	if(!m_bBitSelection) changeWordValue(10);
}


void CFormPlcShareMemoryView::OnBnClickedButtonSelStageDisplacementSensor2()
{
	changeButtonColor(m_nUnitSelectNum,11);
	if(!m_bBitSelection) changeWordValue(11);
}


void CFormPlcShareMemoryView::OnBnClickedButtonSelStageDisplacementSensor3()
{
	changeButtonColor(m_nUnitSelectNum,12);
	if(!m_bBitSelection) changeWordValue(12);
}


void CFormPlcShareMemoryView::OnBnClickedButtonSelStageDisplacementSensor4()
{
	changeButtonColor(m_nUnitSelectNum,13);
	if(!m_bBitSelection) changeWordValue(13);
}


void CFormPlcShareMemoryView::OnBnClickedButtonSelUvMeasure1()
{
	changeButtonColor(m_nUnitSelectNum,14);
	if(!m_bBitSelection) changeWordValue(14);
}


void CFormPlcShareMemoryView::OnBnClickedButtonSelUvMeasure2()
{
	changeButtonColor(m_nUnitSelectNum,15);
	if(!m_bBitSelection) changeWordValue(15);
}


void CFormPlcShareMemoryView::OnBnClickedButtonSelBcr1()
{
	changeButtonColor(m_nUnitSelectNum,16);
	if(!m_bBitSelection) changeWordValue(16);
}


void CFormPlcShareMemoryView::OnBnClickedButtonSelBcr2()
{
	changeButtonColor(m_nUnitSelectNum,17);
	if(!m_bBitSelection) changeWordValue(17);
}


void CFormPlcShareMemoryView::OnBnClickedButtonSelLoadTrayBcr()
{
	changeButtonColor(m_nUnitSelectNum,18);
	if(!m_bBitSelection) changeWordValue(18);
}


void CFormPlcShareMemoryView::OnBnClickedButtonSelSpare()
{
	changeButtonColor(m_nUnitSelectNum,19);
	if(!m_bBitSelection) changeWordValue(19);
}

void CFormPlcShareMemoryView::OnBnClickedButtonSelSpare2()
{
	changeButtonColor(m_nUnitSelectNum,20);
	if(!m_bBitSelection) changeWordValue(20);
}


void CFormPlcShareMemoryView::OnBnClickedButtonSelSpare3()
{
	changeButtonColor(m_nUnitSelectNum,21);
	if(!m_bBitSelection) changeWordValue(21);
}

void CFormPlcShareMemoryView::changeWordValue(int id)
{
	//g_CommPLC.GetWord(m_pMain->m_nStartReadWordAddr+id*10, 10,m_ldPcFromPlcWord);
	//g_CommPLC.GetWord(m_pMain->m_nStartWriteWordAddr+id*10, 10,m_ldPlcFromPcWord);
}

void CFormPlcShareMemoryView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CRect IdRect,rect;
	GetWindowRect(&rect);
	m_LbPlcViewTitle.GetWindowRect( &IdRect );
	int w = IdRect.Width();
	int h = IdRect.Height();

	IdRect.left = IdRect.left - rect.left;
	IdRect.top = IdRect.top - rect.top;
	IdRect.right = IdRect.left + w;
	IdRect.bottom = IdRect.top + h;

	if( point.x >= IdRect.left && 	point.x <= IdRect.right && 
		point.y >= IdRect.top && 	point.y <= IdRect.bottom )
	{
		 m_bBitSelection =  !m_bBitSelection;
		 changeButtonColor(m_nUnitSelectNum,m_nUnitSelectNum);
		 if(m_bBitSelection)	 m_LbPlcViewTitle.SetText("PLC Communication Client(Bit)");
		 else		  m_LbPlcViewTitle.SetText("PLC Communication Client(Word)");
	}

	CFormView::OnLButtonUp(nFlags, point);
}