// DlgCaliper.cpp: 구현 파일입니다.
//

#include "stdafx.h"
#include "..\LET_AlignClient.h"
#include "DlgCaliper.h"
#include "afxdialogex.h"
#include "..\LET_AlignClientDlg.h"
#include "KeyPadDlg.h"
#include "INIReader.h"
#include "JXINIFile.h"

enum LineDirection {
	LINE_HORI = 0,
	LINE_VERT,
};

// CDlgCaliper 대화 상자입니다.

IMPLEMENT_DYNAMIC(CDlgCaliper, CDialogEx)

CDlgCaliper::CDlgCaliper(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgCaliper::IDD, pParent)
{
	m_pMain = NULL;
	m_bHitStart = FALSE;
	m_bHitMid = FALSE;
	m_bHitEnd = FALSE;
	m_bMouseClicked = FALSE;
	m_pViewer = NULL;
	m_pCaliperParam = NULL;
	m_pCaliperResult = NULL;	
	m_nLine = 0;
	m_nPos = 0;
	m_nObject = 0;

	for (int i = 0; i < MAX_CALIPER_OBJECT; i++)
	{
		for (int nPos = 0; nPos < NUM_POS; nPos++)
		{
			for (int nLine = 0; nLine < MAX_CALIPER_LINES; nLine++)
			{
				sprintf(m_cCaliperName[i][nPos][nLine], "Line %d", nLine + 1);
			}
		}
	}
}

CDlgCaliper::~CDlgCaliper()
{
	if( m_pCaliperParam != NULL ) delete m_pCaliperParam;
	if( m_pCaliperResult != NULL ) delete m_pCaliperResult;
}

void CDlgCaliper::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CALIPER_TAB, m_ctrlCaliperTab);
	DDX_Control(pDX, IDC_PIC_CALIPER, m_picCaliper);
}


BEGIN_MESSAGE_MAP(CDlgCaliper, CDialogEx)	
	ON_MESSAGE(WM_VIEW_CONTROL, OnViewControl)
	ON_NOTIFY(CTCN_SELCHANGE, IDC_CALIPER_TAB, OnSelchangeTab)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

BOOL CDlgCaliper::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	/*m_pCaliperParam = NULL;
	m_pCaliperResult = NULL;*/

	CRect rect;
	m_picCaliper.GetClientRect(&rect);

	m_pCaliperParam = new CSheetCaliperParam;
	m_pCaliperParam->m_pDlgCaliper = this;
	m_pCaliperParam->Create(IDD_SHEET_CALIPER_PARAM, &m_picCaliper);

	m_pCaliperResult = new CSheetCaliperResult;
	m_pCaliperResult->m_pDlgCaliper = this;
	m_pCaliperResult->Create(IDD_SHEET_CALIPER_RESULT, &m_picCaliper);	

	m_pCaliperParam->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
	m_pCaliperParam->ShowWindow(SW_SHOW);
	m_pCaliperResult->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
	m_pCaliperResult->ShowWindow(SW_HIDE);
	

	m_ctrlCaliperTab.InsertItem(0, "     Parameter  ");
	m_ctrlCaliperTab.SetItemData(0, SS_BLACKRECT);
	m_ctrlCaliperTab.InsertItem(1, "     Result     ");
	m_ctrlCaliperTab.SetItemData(1, SS_GRAYRECT);
	m_ctrlCaliperTab.SetCurSel(0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgCaliper::MouseMove(CPoint point, FakeDC*pDC)
{
	// Lincoln Lee - 220530 - Easier to moving caliper
	//int nSize = 6;
	int nSize = 12;

	CRect rect, rect2;
	rect.left = m_Caliper[m_nObject][m_nPos][m_nLine].getStartPt().x;
	rect.top = m_Caliper[m_nObject][m_nPos][m_nLine].getStartPt().y;
	rect.right = m_Caliper[m_nObject][m_nPos][m_nLine].getStartPt().x;
	rect.bottom = m_Caliper[m_nObject][m_nPos][m_nLine].getStartPt().y;

	rect2.left = m_Caliper[m_nObject][m_nPos][m_nLine].getEndPt().x;
	rect2.top = m_Caliper[m_nObject][m_nPos][m_nLine].getEndPt().y;
	rect2.right = m_Caliper[m_nObject][m_nPos][m_nLine].getEndPt().x;
	rect2.bottom = m_Caliper[m_nObject][m_nPos][m_nLine].getEndPt().y;

	rect.left -= nSize;
	rect.top -= nSize;
	rect.right += nSize;
	rect.bottom += nSize;

	rect2.left -= nSize;
	rect2.top -= nSize;
	rect2.right += nSize;
	rect2.bottom += nSize;
	
	if( m_bMouseClicked != TRUE )
	{
		setHitStart(FALSE);
		setHitMid(FALSE);
		setHitEnd(FALSE);

		if( PtInRect(&rect, point) )	// Start 위치
		{
			::SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
			setHitStart(TRUE);
		}
		else if( PtInRect(&rect2, point) )	// End 위치
		{
			::SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
			setHitEnd(TRUE);
		}
		else if( IsHitLine(point) )	// 직선 상
		{
			::SetCursor(LoadCursor(NULL, IDC_SIZEALL));
			setHitMid(TRUE);

			_st32fPoint pt;
			pt.x = point.x;
			pt.y = point.y;
			m_Caliper[m_nObject][m_nPos][m_nLine].setMidPt(pt);
		}

		if (m_Caliper[m_nObject][m_nPos][m_nLine].getCircleCaliper())	m_Caliper[m_nObject][m_nPos][m_nLine].DrawCaliper_Circle(pDC);
		else															m_Caliper[m_nObject][m_nPos][m_nLine].DrawCaliper(pDC);
	}
	else
	{
		_st32fPoint pt;

		if( getHitStart() ) ::SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
		if( getHitEnd() ) ::SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
		if( getHitMid() ) ::SetCursor(LoadCursor(NULL, IDC_SIZEALL));

		if( getHitStart() )
		{
			pt.x = point.x;
			pt.y = point.y;
			m_Caliper[m_nObject][m_nPos][m_nLine].setStartPt(pt);
		}

		if( getHitEnd() )
		{
			pt.x = point.x;
			pt.y = point.y;
			m_Caliper[m_nObject][m_nPos][m_nLine].setEndPt(pt);
		}

		if( getHitMid() )
		{
			_st32fPoint tempPt;

			pt.x = m_Caliper[m_nObject][m_nPos][m_nLine].getMidPt().x - point.x;
			pt.y = m_Caliper[m_nObject][m_nPos][m_nLine].getMidPt().y - point.y;

			tempPt = m_Caliper[m_nObject][m_nPos][m_nLine].getStartPt();
			tempPt.x -= pt.x;
			tempPt.y -= pt.y;
			m_Caliper[m_nObject][m_nPos][m_nLine].setStartPt(tempPt);

			tempPt = m_Caliper[m_nObject][m_nPos][m_nLine].getEndPt();
			tempPt.x -= pt.x;
			tempPt.y -= pt.y;
			m_Caliper[m_nObject][m_nPos][m_nLine].setEndPt(tempPt);

			tempPt = m_Caliper[m_nObject][m_nPos][m_nLine].getMidPt();
			tempPt.x -= pt.x;
			tempPt.y -= pt.y;
			m_Caliper[m_nObject][m_nPos][m_nLine].setMidPt(tempPt);
		}

		if (m_Caliper[m_nObject][m_nPos][m_nLine].getCircleCaliper())	m_Caliper[m_nObject][m_nPos][m_nLine].calcCaliperCircle();
		else															m_Caliper[m_nObject][m_nPos][m_nLine].calcCaliperRect();

		if (m_Caliper[m_nObject][m_nPos][m_nLine].getCircleCaliper())	m_Caliper[m_nObject][m_nPos][m_nLine].DrawCaliper_Circle(pDC);
		else															m_Caliper[m_nObject][m_nPos][m_nLine].DrawCaliper(pDC);
	}
}

void CDlgCaliper::LButtonDblClk(CPoint point, FakeDC*pDC)
{
	//if (m_Caliper[m_nObject][m_nPos][m_nLine].getCircleCaliper())	m_Caliper[m_nObject][m_nPos][m_nLine].DrawCaliper_Circle(m_pViewer->getCaliperOverlayDC());
	//else															m_Caliper[m_nObject][m_nPos][m_nLine].DrawCaliper(m_pViewer->getCaliperOverlayDC());

	m_pViewer->SetCaliperCircleMode(m_Caliper[m_nObject][m_nPos][m_nLine].getCircleCaliper());
	m_pViewer->SetInteraction(true);
	m_pViewer->Invalidate();
}


void CDlgCaliper::LButtonDown(CPoint point, FakeDC* pDC)
{
	if( m_bMouseClicked != TRUE )
	{
		if( m_bHitStart ) ::SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
		if( m_bHitEnd ) ::SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
		if( m_bHitMid ) ::SetCursor(LoadCursor(NULL, IDC_SIZEALL));

		m_Caliper[m_nObject][m_nPos][m_nLine].calcPointsByFixture();
		//if (m_Caliper[m_nObject][m_nPos][m_nLine].getCircleCaliper())	m_Caliper[m_nObject][m_nPos][m_nLine].DrawCaliper_Circle(m_pViewer->getCaliperOverlayDC());
		//else															m_Caliper[m_nObject][m_nPos][m_nLine].DrawCaliper(m_pViewer->getCaliperOverlayDC());
		//m_pViewer->Invalidate();

		m_pViewer->SetCaliperCircleMode(m_Caliper[m_nObject][m_nPos][m_nLine].getCircleCaliper());

		m_pViewer->SetInteraction(true);
		m_pViewer->Invalidate();

		m_bMouseClicked = TRUE;
	}
}

void CDlgCaliper::LButtonUp(CPoint point, FakeDC*pDC)
{
	if( m_bMouseClicked )
	{
		m_Caliper[m_nObject][m_nPos][m_nLine].calcOrgPointsFromOffsetPoints();
		m_bMouseClicked = FALSE;
		m_pCaliperParam->updateCoordinates();

		m_bChanged[m_nObject][m_nPos][m_nLine] = TRUE;
		//if (m_Caliper[m_nObject][m_nPos][m_nLine].getCircleCaliper())	m_Caliper[m_nObject][m_nPos][m_nLine].DrawCaliper_Circle(m_pViewer->getCaliperOverlayDC());
		//else															m_Caliper[m_nObject][m_nPos][m_nLine].DrawCaliper(m_pViewer->getCaliperOverlayDC());
		//m_pViewer->Invalidate();
	
		m_pViewer->SetCaliperCircleMode(m_Caliper[m_nObject][m_nPos][m_nLine].getCircleCaliper());
		m_pViewer->SetInteraction(true);
		m_pViewer->Invalidate();
	}
}


BOOL CDlgCaliper::IsHitLine(CPoint point)
{
	_st32fPoint pt1 = m_Caliper[m_nObject][m_nPos][m_nLine].getStartPt();
	_st32fPoint pt2 = m_Caliper[m_nObject][m_nPos][m_nLine].getEndPt();

	int dxap = (int)(point.x - pt1.x);	// Vector AP
	int dyap = (int)(point.y - pt1.y);
	int dxab = (int)(pt2.x - pt1.x);	// Vector AB
	int dyab = (int)(pt2.y - pt1.y);

	double ab2 = dxab * dxab + dyab * dyab; // Magnitude of AB

	double t; // This will hold the parameter for the Point of projection of P on AB

	if (ab2 <= 2) {
		t = 0;   // A and B coincide
	} else {
		t = (dxap * dxab + dyap * dyab) / ab2;
	}

 // Above equation maps to (AP dot normalized(AB)) / magnitude(AP dot normalized(AB))
	if (t < 0) {
		t = 0;   // Projection is beyond A so nearest point is A	
	} else {
		if (t > 1) t = 1; // Projection is beyond B so nearest point is B
	}

	double xf = pt1.x + t * dxab; // Projection point on Seg AB
	double yf = pt1.y + t * dyab; //

	double dxfp = point.x - xf;
	double dyfp = point.y - yf;

	int nDist;
	nDist = (int)sqrt(dxfp * dxfp + dyfp * dyfp);

	if ( nDist < 8 ) {
		return TRUE;
	}

	 return FALSE;
}


BOOL CDlgCaliper::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	
	if( pMsg->message == WM_KEYDOWN )
	{
		if( pMsg->wParam == VK_RETURN  || pMsg->wParam == VK_ESCAPE )
			return TRUE;
	}
	

	return CDialogEx::PreTranslateMessage(pMsg);
}

LRESULT CDlgCaliper::OnViewControl(WPARAM wParam, LPARAM lParam)
{
	CPaneCamera* pCamera = (CPaneCamera*)m_pMain->m_pPane[PANE_CAMERA];
	pCamera->CameraCaliperClose();

	return 0;
}

void CDlgCaliper::OnSelchangeTab(  NMHDR* pNMHDR, LRESULT* pResult) 
{
	switch(m_ctrlCaliperTab.GetCurSel())  {
	case 0:  // Black rectangle
		m_pCaliperResult->saveCaliperList();
		m_pCaliperParam->ShowWindow(SW_SHOW);		
		m_pCaliperResult->ShowWindow(SW_HIDE);
		break;
	case 1: // Gray rectangle		
		m_pCaliperParam->ShowWindow(SW_HIDE);
		m_pCaliperResult->ShowWindow(SW_SHOW);
		break;
	default: // White rectangle
		break;
	}

	*pResult = 0;
}


void CDlgCaliper::saveCaliperInfo(CString strPath, CCaliper *pCaliper, int nObject, int nPos, int nLine)
{
	CString filePath = strPath + "CaliperInfo.ini";
	CString strKey, strSection = "CALIPER_INFO", strData;

	::JXWritePrivateProfileString(strSection, fmt("NUM_OF_CALIPER_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1), fmt("%d", pCaliper->getNumOfCalipers()), filePath);
	::JXWritePrivateProfileString(strSection, fmt("PROJECTION_LENGTH_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1), fmt("%d", pCaliper->getProjectionLength()), filePath);
	::JXWritePrivateProfileString(strSection, fmt("SEARCH_LENGTH_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1), fmt("%d", pCaliper->getSearchLength()), filePath);
	::JXWritePrivateProfileString(strSection, fmt("DIRECTION_SWAP_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1), fmt("%d", pCaliper->getDirectionSwap()), filePath);
	::JXWritePrivateProfileString(strSection, fmt("SEARCH_TYPE_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1), fmt("%d", pCaliper->getSearchType()), filePath);
	::JXWritePrivateProfileString(strSection, fmt("FITTING_METHOD_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1), fmt("%d", pCaliper->getFittingMethod()), filePath);
	::JXWritePrivateProfileString(strSection, fmt("EACH_CALIPER_SEARCH_TYPE_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1), fmt("%d", pCaliper->getEachCaliperSearchType()), filePath);
	::JXWritePrivateProfileString(strSection, fmt("EDGE_POLARITY_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1), fmt("%d", pCaliper->getContrastDirection()), filePath);
	::JXWritePrivateProfileString(strSection, fmt("HALF_FILTER_SIZE_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1), fmt("%d", pCaliper->getHalfFilterSize()), filePath);
	::JXWritePrivateProfileString(strSection, fmt("NUM_OF_IGNORE_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1), fmt("%d", pCaliper->getNumOfIgnore()), filePath);
	::JXWritePrivateProfileString(strSection, fmt("THRESHOLD_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1), fmt("%d", pCaliper->getThreshold()), filePath);
	::JXWritePrivateProfileString(strSection, fmt("USE_CIRCLE_CALIPER_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1), fmt("%d", pCaliper->getCircleCaliper()), filePath);
	::JXWritePrivateProfileString(strSection, fmt("START_POINT_X_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1), fmt("%f", pCaliper->getOrgStartPt().x), filePath);
	::JXWritePrivateProfileString(strSection, fmt("START_POINT_Y_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1), fmt("%f", pCaliper->getOrgStartPt().y), filePath);
	::JXWritePrivateProfileString(strSection, fmt("END_POINT_X_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1), fmt("%f", pCaliper->getOrgEndPt().x), filePath);
	::JXWritePrivateProfileString(strSection, fmt("END_POINT_Y_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1), fmt("%f", pCaliper->getOrgEndPt().y), filePath);

	for (int i = 0; i < MAX_CALIPER; i++)
	{
		::JXWritePrivateProfileString(strSection, fmt("USE_CALIPER_%d_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1, i + 1), fmt("%d", pCaliper->getUseCaliper(i)), filePath);
	}

	::JXUnloadPrivateProfile();


	/* strKey.Format("NUM_OF_CALIPER_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1);
	strData.Format("%d", pCaliper->getNumOfCalipers());
	::WritePrivateProfileStringA(strSection, strKey, strData, filePath);
	strKey.Format("PROJECTION_LENGTH_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1);
	strData.Format("%d", pCaliper->getProjectionLength());
	::WritePrivateProfileStringA(strSection, strKey, strData, filePath);
	strKey.Format("SEARCH_LENGTH_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1);
	strData.Format("%d", pCaliper->getSearchLength());
	::WritePrivateProfileStringA(strSection, strKey, strData, filePath);
	strKey.Format("DIRECTION_SWAP_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1);
	strData.Format("%d", pCaliper->getDirectionSwap());
	::WritePrivateProfileStringA(strSection, strKey, strData, filePath);
	strKey.Format("SEARCH_TYPE_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1);
	strData.Format("%d", pCaliper->getSearchType());
	::WritePrivateProfileStringA(strSection, strKey, strData, filePath);
	strKey.Format("FITTING_METHOD_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1);
	strData.Format("%d", pCaliper->getFittingMethod());
	::WritePrivateProfileStringA(strSection, strKey, strData, filePath);
	strKey.Format("EACH_CALIPER_SEARCH_TYPE_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1);
	strData.Format("%d", pCaliper->getEachCaliperSearchType());
	::WritePrivateProfileStringA(strSection, strKey, strData, filePath);
	strKey.Format("EDGE_POLARITY_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1);
	strData.Format("%d", pCaliper->getContrastDirection());
	::WritePrivateProfileStringA(strSection, strKey, strData, filePath);
	strKey.Format("HALF_FILTER_SIZE_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1);
	strData.Format("%d", pCaliper->getHalfFilterSize());
	::WritePrivateProfileStringA(strSection, strKey, strData, filePath);
	strKey.Format("NUM_OF_IGNORE_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1);
	strData.Format("%d", pCaliper->getNumOfIgnore());
	::WritePrivateProfileStringA(strSection, strKey, strData, filePath);
	strKey.Format("THRESHOLD_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1);
	strData.Format("%d", pCaliper->getThreshold());
	::WritePrivateProfileStringA(strSection, strKey, strData, filePath);
	strKey.Format("USE_CIRCLE_CALIPER_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1);
	strData.Format("%d", pCaliper->getCircleCaliper());
	::WritePrivateProfileStringA(strSection, strKey, strData, filePath);
	strKey.Format("START_POINT_X_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1);
	strData.Format("%f", pCaliper->getOrgStartPt().x);
	::WritePrivateProfileStringA(strSection, strKey, strData, filePath);
	strKey.Format("START_POINT_Y_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1);
	strData.Format("%f", pCaliper->getOrgStartPt().y);
	::WritePrivateProfileStringA(strSection, strKey, strData, filePath);
	strKey.Format("END_POINT_X_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1);
	strData.Format("%f", pCaliper->getOrgEndPt().x);
	::WritePrivateProfileStringA(strSection, strKey, strData, filePath);
	strKey.Format("END_POINT_Y_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1);
	strData.Format("%f", pCaliper->getOrgEndPt().y);
	::WritePrivateProfileStringA(strSection, strKey, strData, filePath);

	for(int i = 0; i < MAX_CALIPER; i++)
	{
		strKey.Format("USE_CALIPER_%d_%d_%d_%d", nObject + 1, nPos + 1, nLine + 1, i + 1);
		strData.Format("%d", pCaliper->getUseCaliper(i));
		::WritePrivateProfileStringA(strSection, strKey, strData, filePath);
	}
	*/
}

void CDlgCaliper::readCaliperInfo(const INIReader *_ini, CCaliper* pCaliper, int nObject, int nPos, int nLine) // Tkyuha 20221227 속도 올리기 위함
{
	int iVal;
	_st32fPoint st, ed;

	int pos = nPos + 1;
	int line = nLine + 1;
	int obj = nObject + 1;
	
	std::string strSection = "CALIPER_INFO";
//	INIReader ini(fmt_("%s/CaliperInfo.ini", strPath.GetString())); // Tkyuha 20221227 속도 올리기 위함

	iVal = _ini->GetInteger(strSection, fmt_("NUM_OF_CALIPER_%d_%d_%d", obj, pos, line), 20);	pCaliper->setNumOfCalipers(iVal);
	iVal = _ini->GetInteger(strSection, fmt_("PROJECTION_LENGTH_%d_%d_%d", obj, pos, line), 20);	pCaliper->setProjectionLength(iVal);
	iVal = _ini->GetInteger(strSection, fmt_("SEARCH_LENGTH_%d_%d_%d", obj, pos, line), 50);	pCaliper->setSearchLength(iVal);
	iVal = _ini->GetInteger(strSection, fmt_("DIRECTION_SWAP_%d_%d_%d", obj, pos, line), 0);	pCaliper->setDirectionSwap(iVal);
	iVal = _ini->GetInteger(strSection, fmt_("SEARCH_TYPE_%d_%d_%d", obj, pos, line), 2);	pCaliper->setSearchType(iVal);
	iVal = _ini->GetInteger(strSection, fmt_("FITTING_METHOD_%d_%d_%d", obj, pos, line), 1);	pCaliper->setFittingMethod(iVal);
	iVal = _ini->GetInteger(strSection, fmt_("EACH_CALIPER_SEARCH_TYPE_%d_%d_%d", obj, pos, line), 1);	pCaliper->setEachCaliperSearchType(iVal);
	iVal = _ini->GetInteger(strSection, fmt_("EDGE_POLARITY_%d_%d_%d", obj, pos, line), 0);	pCaliper->setContrastDirection(iVal);
	iVal = _ini->GetInteger(strSection, fmt_("HALF_FILTER_SIZE_%d_%d_%d", obj, pos, line), 1);	pCaliper->setHalfFilterSize(iVal);
	iVal = _ini->GetInteger(strSection, fmt_("NUM_OF_IGNORE_%d_%d_%d", obj, pos, line), 5);	pCaliper->setNumOfIgnore(iVal);
	iVal = _ini->GetInteger(strSection, fmt_("THRESHOLD_%d_%d_%d", obj, pos, line), 15);	pCaliper->setThreshold(iVal);
	iVal = _ini->GetInteger(strSection, fmt_("USE_CIRCLE_CALIPER_%d_%d_%d", obj, pos, line), 0);	pCaliper->setCircleCaliper(iVal);
	st.x = _ini->GetReal(strSection, fmt_("START_POINT_X_%d_%d_%d", obj, pos, line), 800.);
	st.y = _ini->GetReal(strSection, fmt_("START_POINT_Y_%d_%d_%d", obj, pos, line), 800.);
	ed.x = _ini->GetReal(strSection, fmt_("END_POINT_X_%d_%d_%d", obj, pos, line), 800.);
	ed.y = _ini->GetReal(strSection, fmt_("END_POINT_Y_%d_%d_%d", obj, pos, line), 800.);

	pCaliper->setOrgStartPt(st);
	pCaliper->setStartPt(st);
	pCaliper->setOrgEndPt(ed);
	pCaliper->setEndPt(ed);

	for (int i = 0; i < MAX_CALIPER; i++)
	{
		iVal = _ini->GetInteger(strSection, fmt_("USE_CALIPER_%d_%d_%d_%d", obj, pos, line, i + 1), 1);
		pCaliper->setUseCaliper(i, iVal);
	}
}

BOOL CDlgCaliper::makeCrossPointFromLines(int nObj1, int nObj2, int nPos1, int nPos2, int nLine1, int nLine2, _st32fPoint *pt)
{
	BOOL bMake[2] = {FALSE, };
	sLine lineInfo[2];

	bMake[0] =		m_Caliper[nObj1][nPos1][nLine1].getIsMakeLine();
	bMake[1] =		m_Caliper[nObj2][nPos2][nLine2].getIsMakeLine();
	lineInfo[0] =	m_Caliper[nObj1][nPos1][nLine1].m_lineInfo;
	lineInfo[1] =	m_Caliper[nObj2][nPos2][nLine2].m_lineInfo;

	if( bMake[0] != TRUE || bMake[1] != TRUE ) return FALSE;	// 둘 중에 하나라도 찾지 못하면 NG
	m_Caliper[nObj1][nPos1][nLine1].cramersRules(-lineInfo[0].a, 1, -lineInfo[1].a, 1, lineInfo[0].b, lineInfo[1].b, &pt->x, &pt->y);

	return TRUE;
}

void CDlgCaliper::copy_caliper_data(int src_obj, int src_pos, int src_line, int dst_job, int dst_cam, int dst_pos, int dst_line)
{
	m_pMain->copy_caliper_data(src_obj, src_pos, src_line, dst_job, dst_cam, dst_pos, dst_line);
}

void CDlgCaliper::display_copy_caliper_info()
{
	m_pMain->display_copy_caliper_info();
}

void CDlgCaliper::init_copy_caliper_info()
{
	m_pMain->init_copy_caliper_info();
}


void CDlgCaliper::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	if (bShow) m_pCaliperParam->OnCbnSelchangeComboSelectLine();
}
