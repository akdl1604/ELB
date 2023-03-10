// SheetCaliperParam.cpp: 구현 파일입니다.
//

#include "stdafx.h"
#include "..\LET_AlignClient.h"
#include "SheetCaliperParam.h"
#include "afxdialogex.h"
#include "..\LET_AlignClientDlg.h"

// CSheetCaliperParam 대화 상자입니다.

IMPLEMENT_DYNAMIC(CSheetCaliperParam, CDialogEx)

#define MSG_INPUT_CALIPER_PARAMS 1080

CSheetCaliperParam::CSheetCaliperParam(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSheetCaliperParam::IDD, pParent)
{
	m_pDlgCaliper = NULL;
	m_nCaliperResouceID[RC_NUM_OF_CALIPER] 		= IDC_EDIT_NUM_OF_CALIPER;
	m_nCaliperResouceID[RC_PROJECTION_LENGTH] 	= IDC_EDIT_PROJECTION_LENGTH;
	m_nCaliperResouceID[RC_SEARCH_LENGTH] 		= IDC_EDIT_SEARCH_LENGTH;
	m_nCaliperResouceID[RC_THRESHOLD] 			= IDC_EDIT_THRESHOLD;
	m_nCaliperResouceID[RC_HALF_FILTER_SIZE] 	= IDC_EDIT_HALF_FILTER_SIZE;
	m_nCaliperResouceID[RC_NUM_OF_IGNORE] 		= IDC_EDIT_NUM_OF_IGNORE;
	m_nCaliperResouceID[RC_FIXTURE_X] 			= IDC_EDIT_FIXTURE_X;
	m_nCaliperResouceID[RC_FIXTURE_Y] 			= IDC_EDIT_FIXTURE_Y;
	m_nCaliperResouceID[RC_FIXTURE_T] 			= IDC_EDIT_FIXTURE_T;
	m_nCaliperResouceID[RC_ORG_START_X] 		= IDC_EDIT_ORG_START_X;
	m_nCaliperResouceID[RC_ORG_START_Y] 		= IDC_EDIT_ORG_START_Y;
	m_nCaliperResouceID[RC_ORG_END_X] 			= IDC_EDIT_ORG_END_X;
	m_nCaliperResouceID[RC_ORG_END_Y] 			= IDC_EDIT_ORG_END_Y;

}

CSheetCaliperParam::~CSheetCaliperParam()
{
}

void CSheetCaliperParam::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_SELECT_LINE, m_cbSelectLine);

	DDX_Control(pDX, IDC_COMBO_COPY_JOB, m_cbCopyJob);
	DDX_Control(pDX, IDC_COMBO_COPY_CAM, m_cbCopyCam);
	DDX_Control(pDX, IDC_COMBO_COPY_POS, m_cbCopyPos);
	DDX_Control(pDX, IDC_COMBO_COPY_LINE, m_cbCopyLine);
}


BEGIN_MESSAGE_MAP(CSheetCaliperParam, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_CALIPER_DIRECTION_SWAP, &CSheetCaliperParam::OnBnClickedBtnCaliperDirectionSwap)
	ON_BN_CLICKED(IDC_BTN_CALIPER_FIND, &CSheetCaliperParam::OnBnClickedBtnCaliperFind)
	ON_CBN_SELCHANGE(IDC_COMBO_CALIPER_SEARCH_TYPE, &CSheetCaliperParam::OnCbnSelchangeComboCaliperSearchType)
	ON_BN_CLICKED(IDC_CHECK_CIRCLE_FITTING, &CSheetCaliperParam::OnBnClickedCheckCircleFitting)
	ON_CBN_SELCHANGE(IDC_COMBO_FITTING_METHOD, &CSheetCaliperParam::OnCbnSelchangeComboFittingMethod)
	ON_MESSAGE(WM_VIEW_CONTROL, OnViewControl)
	ON_CBN_SELCHANGE(IDC_COMBO_SELECT_LINE, &CSheetCaliperParam::OnCbnSelchangeComboSelectLine)
	ON_BN_CLICKED(IDC_BTN_CALIPER_INFO_SAVE, &CSheetCaliperParam::OnBnClickedBtnCaliperInfoSave)
	ON_CBN_SELCHANGE(IDC_COMBO_EDGE_POLARITY, &CSheetCaliperParam::OnCbnSelchangeComboEdgePolarity)
	ON_CBN_SELCHANGE(IDC_COMBO_SEARCH_TYPE, &CSheetCaliperParam::OnCbnSelchangeComboSearchType)
	ON_BN_CLICKED(IDC_BTN_CALIPER_COPY, &CSheetCaliperParam::OnBnClickedBtnCaliperCopy)
	ON_CBN_SELCHANGE(IDC_COMBO_COPY_JOB, &CSheetCaliperParam::OnCbnSelchangeComboCopyJob)
	ON_BN_CLICKED(IDC_BTN_CALIPER_INFO_CLOSE, &CSheetCaliperParam::OnBnClickedBtnCaliperInfoClose)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CSheetCaliperParam 메시지 처리기입니다.
void CSheetCaliperParam::OnBnClickedBtnCaliperDirectionSwap()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BOOL bSwap = m_pCaliper->getDirectionSwap();

	m_pCaliper->setDirectionSwap(!bSwap);
	//m_pDlgCaliper->m_pViewer->ClearCaliperOverlayDC();

	//if (m_pCaliper->getCircleCaliper())	m_pCaliper->DrawCaliper_Circle(m_pDlgCaliper->m_pDC);
	//else								m_pCaliper->DrawCaliper(m_pDlgCaliper->m_pDC);

	//m_pDlgCaliper->m_pViewer->GetInteractiveItems().m_InteractCaliper.SetReverseMode(bSwap);
	//m_pDlgCaliper->m_pViewer->Invalidate();
	m_pDlgCaliper->m_pViewer->SetCaliperReverseMode(!bSwap);

	int nObj = m_pDlgCaliper->m_nObject;
	int nPos = m_pDlgCaliper->m_nPos;
	int nLine = m_cbSelectLine.GetCurSel();
	m_pDlgCaliper->m_bChanged[nObj][nPos][nLine] = TRUE;
}

void CSheetCaliperParam::OnBnClickedBtnCaliperFind()
{
	//	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString strTemp;

	CPoint st, ed;
	_st32fPoint st_, ed_;
	m_pDlgCaliper->m_pViewer->GetCaliperBeginEnd(st, ed);

	double fixtureX = m_pCaliper->getFixtureX();
	double fixtureY = m_pCaliper->getFixtureY();
	double fixtureT = m_pCaliper->getFixtureT();

	st_.x = st.x - fixtureX;
	st_.y = st.y - fixtureY;
	ed_.x = ed.x - fixtureX;
	ed_.y = ed.y - fixtureY;

	m_pCaliper->setOrgStartPt(st_);
	m_pCaliper->setOrgEndPt(ed_);

	m_pCaliper->calcPointsByFixture();

	GetDlgItem(IDC_EDIT_NUM_OF_CALIPER)->GetWindowTextA(strTemp);
	m_pCaliper->setNumOfCalipers( atoi(strTemp) );
	GetDlgItem(IDC_EDIT_PROJECTION_LENGTH)->GetWindowTextA(strTemp);
	m_pCaliper->setProjectionLength( atoi(strTemp) );
	GetDlgItem(IDC_EDIT_SEARCH_LENGTH)->GetWindowTextA(strTemp);
	m_pCaliper->setSearchLength( atoi(strTemp) );
	GetDlgItem(IDC_EDIT_THRESHOLD)->GetWindowTextA(strTemp);
	m_pCaliper->setThreshold(atoi(strTemp));
	GetDlgItem(IDC_EDIT_HALF_FILTER_SIZE)->GetWindowTextA(strTemp);
	m_pCaliper->setHalfFilterSize( atoi(strTemp) );
	GetDlgItem(IDC_EDIT_NUM_OF_IGNORE)->GetWindowTextA(strTemp);
	m_pCaliper->setNumOfIgnore( atoi(strTemp) );
	GetDlgItem(IDC_EDIT_FIXTURE_X)->GetWindowTextA(strTemp);
	m_pCaliper->setFixtureX(atof(strTemp));
	GetDlgItem(IDC_EDIT_FIXTURE_Y)->GetWindowTextA(strTemp);
	m_pCaliper->setFixtureY(atof(strTemp));
	GetDlgItem(IDC_EDIT_FIXTURE_T)->GetWindowTextA(strTemp);
	m_pCaliper->setFixtureT(atof(strTemp));

	int nSel = ((CComboBox*)GetDlgItem(IDC_COMBO_EDGE_POLARITY))->GetCurSel();
	m_pCaliper->setContrastDirection(nSel);
	nSel = ((CComboBox*)GetDlgItem(IDC_COMBO_SEARCH_TYPE))->GetCurSel();
	m_pCaliper->setSearchType(nSel);

#ifndef SKIP_XLIST
	for (int i = 0; i < m_pCaliper->getNumOfCalipers(); i++)
		m_pCaliper->setUseCaliper(i, m_pDlgCaliper->m_pCaliperResult->m_listCaliper.GetCheckbox(i, 1));
#else
	for(int i = 0; i < m_pCaliper->getNumOfCalipers(); i++)
		m_pCaliper->setUseCaliper(i, TRUE);
#endif

	m_pDlgCaliper->m_pViewer->ClearOverlayDC();
	m_pCaliper->setCaliperDC(m_pDlgCaliper->m_pViewer->getOverlayDC());
	//	m_pDlgCaliper->m_pViewer->ClearOverlayDC();
	if (m_pCaliper->getCircleCaliper())	m_pCaliper->calcCaliperCircle();
	else								m_pCaliper->calcCaliperRect();
	//	m_pCaliper->setCaliperDC( m_pDlgCaliper->m_pViewer->getOverlayDC() );
	//
	BYTE *pImage = new BYTE[m_pDlgCaliper->m_pViewer->GetWidth() * m_pDlgCaliper->m_pViewer->GetHeight()];

	memcpy(pImage, m_pDlgCaliper->m_pViewer->GetImagePtr(), m_pDlgCaliper->m_pViewer->GetWidth() * m_pDlgCaliper->m_pViewer->GetHeight());
	cv::Mat img(m_pDlgCaliper->m_pViewer->GetHeight(), m_pDlgCaliper->m_pViewer->GetWidth(), CV_8UC1, pImage);
	//
	//	cv::flip(img, img, 0);
	//	
	//	//cv::imwrite("C:\\Caliper.jpg", img);
	//
	if (m_pCaliper->getCircleCaliper())
	{
		m_pCaliper->processCaliper_circle(pImage, m_pDlgCaliper->m_pViewer->GetWidth(), m_pDlgCaliper->m_pViewer->GetHeight(), fixtureX, fixtureY, fixtureT, TRUE);
	}
	else
	{
		m_pCaliper->processCaliper(pImage, m_pDlgCaliper->m_pViewer->GetWidth(), m_pDlgCaliper->m_pViewer->GetHeight(), fixtureX, fixtureY, fixtureT, TRUE);
	}

	m_pDlgCaliper->m_pViewer->Invalidate();
	delete pImage;

	int nObj = m_pDlgCaliper->m_nObject;
	int nPos = m_pDlgCaliper->m_nPos;
	int nLine = m_cbSelectLine.GetCurSel();
	m_pDlgCaliper->m_bChanged[nObj][nPos][nLine] = TRUE;

	//
#ifndef SKIP_XLIST
	m_pDlgCaliper->m_pCaliperResult->updateCaliperList();
#endif
	//
	//	//if( m_Caliper.getCircleCaliper() )	m_Caliper.processCaliper_Circle(m_pViewer->GetImaagePtr());
	//	//else									m_Caliper.processCaliper(m_pViewer->GetImaagePtr());
	//
	//	/*if( m_Caliper.getCircleCaliper() )	m_Caliper.prprocessCaliper_Circle(m_pMain->m_pDstImage);
	//	else									m_Caliper.processCaliper(m_pMain->m_pDstImage);*/
}


void CSheetCaliperParam::OnCbnSelchangeComboCaliperSearchType()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nSel = ((CComboBox*)GetDlgItem(IDC_COMBO_CALIPER_SEARCH_TYPE))->GetCurSel();
	m_pCaliper->setEachCaliperSearchType( nSel );

	int nObj = m_pDlgCaliper->m_nObject;
	int nPos = m_pDlgCaliper->m_nPos;
	int nLine = m_cbSelectLine.GetCurSel();
	m_pDlgCaliper->m_bChanged[nObj][nPos][nLine] = TRUE;
}

void CSheetCaliperParam::OnBnClickedCheckCircleFitting()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BOOL bCheck = IsDlgButtonChecked(IDC_CHECK_CIRCLE_FITTING);
	m_pCaliper->setCircleCaliper( bCheck );

	if (m_pCaliper->getCircleCaliper())	m_pCaliper->calcCaliperCircle();
	else								m_pCaliper->calcCaliperRect();

	//m_pDlgCaliper->m_pViewer->ClearCaliperOverlayDC();

	//if (m_pCaliper->getCircleCaliper())	m_pCaliper->DrawCaliper_Circle(m_pDlgCaliper->m_pDC);
	//else								m_pCaliper->DrawCaliper(m_pDlgCaliper->m_pDC);

	m_pDlgCaliper->m_pViewer->SetCaliperCircleMode(m_pCaliper->getCircleCaliper());
	m_pDlgCaliper->m_pViewer->Invalidate();

#ifndef SKIP_XLIST
	m_pDlgCaliper->m_pCaliperResult->updateCaliperList();
#endif
	int nObj = m_pDlgCaliper->m_nObject;
	int nPos = m_pDlgCaliper->m_nPos;
	int nSel = m_cbSelectLine.GetCurSel();
	m_pDlgCaliper->m_bChanged[nObj][nPos][nSel] = TRUE;
}

void CSheetCaliperParam::OnCbnSelchangeComboFittingMethod()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nSel = ((CComboBox*)GetDlgItem(IDC_COMBO_FITTING_METHOD))->GetCurSel();
	m_pCaliper->setFittingMethod( nSel );

	int nObj = m_pDlgCaliper->m_nObject;
	int nPos = m_pDlgCaliper->m_nPos;
	int nLine = m_cbSelectLine.GetCurSel();
	m_pDlgCaliper->m_bChanged[nObj][nPos][nLine] = TRUE;
}

void CSheetCaliperParam::updateCoordinates()
{
	CString strTemp;

	strTemp.Format("%.3f", m_pCaliper->getOrgStartPt().x);
	GetDlgItem(IDC_EDIT_ORG_START_X)->SetWindowTextA(strTemp);

	strTemp.Format("%.3f", m_pCaliper->getOrgStartPt().y);
	GetDlgItem(IDC_EDIT_ORG_START_Y)->SetWindowTextA(strTemp);

	strTemp.Format("%.3f", m_pCaliper->getOrgEndPt().x);
	GetDlgItem(IDC_EDIT_ORG_END_X)->SetWindowTextA(strTemp);

	strTemp.Format("%.3f", m_pCaliper->getOrgEndPt().y);
	GetDlgItem(IDC_EDIT_ORG_END_Y)->SetWindowTextA(strTemp);
	
	strTemp.Format("%.3f", m_pCaliper->getStartPt().x);
	GetDlgItem(IDC_EDIT_START_X)->SetWindowTextA(strTemp);

	strTemp.Format("%.3f", m_pCaliper->getStartPt().y);
	GetDlgItem(IDC_EDIT_START_Y)->SetWindowTextA(strTemp);

	strTemp.Format("%.3f", m_pCaliper->getEndPt().x);
	GetDlgItem(IDC_EDIT_END_X)->SetWindowTextA(strTemp);

	strTemp.Format("%.3f", m_pCaliper->getEndPt().y);
	GetDlgItem(IDC_EDIT_END_Y)->SetWindowTextA(strTemp);
}


LRESULT CSheetCaliperParam::OnViewControl(WPARAM wParam, LPARAM lParam)
{
	switch( wParam ) {
	case MSG_INPUT_CALIPER_PARAMS:
		{
			int index = (int)lParam;
			CKeyPadDlg dlg;
			CString strTemp;
			GetDlgItem(m_nCaliperResouceID[index])->GetWindowTextA(strTemp);
			dlg.SetValueString(false, strTemp);

			if( dlg.DoModal() != IDOK ) return 0;
			
			dlg.GetValue(strTemp);
			GetDlgItem(m_nCaliperResouceID[index])->SetWindowTextA(strTemp);

			switch( index ) {
			case RC_NUM_OF_CALIPER:			// Num Of Calipers
				{
					int nCaliper = atoi(strTemp);
					if( nCaliper < 2 || nCaliper > MAX_CALIPER )	
					{
						CString strTemp;
						strTemp.Format("%d", m_pCaliper->getNumOfCalipers());
						GetDlgItem(m_nCaliperResouceID[index])->SetWindowTextA(strTemp);

						strTemp.Format("Input 2 ~ %d", MAX_CALIPER);
						AfxMessageBox(strTemp);
						return 0;
					}

			m_pDlgCaliper->m_pViewer->SetNumOfCalipers(nCaliper);
			m_pCaliper->setNumOfCalipers(nCaliper);
					m_pCaliper->init_caliper_info();
			}
				break;
			case RC_PROJECTION_LENGTH:			// Projection Length
				{
					int nLength = atoi(strTemp);
					if( nLength < 2 ) 
					{
						strTemp.Format("%d", m_pCaliper->getProjectionLength());
						GetDlgItem(m_nCaliperResouceID[index])->SetWindowTextA(strTemp);
						return 0;
					}
			m_pDlgCaliper->m_pViewer->SetCaliperProjectionLength(nLength);
			m_pCaliper->setProjectionLength(nLength);
					m_pCaliper->init_caliper_info();
			}
				break;		// Search Length
			case RC_SEARCH_LENGTH:
				{
					int nLength = atoi(strTemp);
					if( nLength < 2 ) 
					{
						strTemp.Format("%d", m_pCaliper->getSearchLength());
						GetDlgItem(m_nCaliperResouceID[index])->SetWindowTextA(strTemp);
						return 0;
					}

			m_pDlgCaliper->m_pViewer->SetCaliperSearchLength(nLength);
			m_pCaliper->setSearchLength(nLength);
					m_pCaliper->init_caliper_info();
			}
				break;
			case RC_THRESHOLD:			// Threshold
				{
					int nThresh = atoi(strTemp);

					if( nThresh < 1 || nThresh > 255 )
					{
						strTemp.Format("%d", m_pCaliper->getThreshold());
						GetDlgItem(m_nCaliperResouceID[index])->SetWindowTextA(strTemp);
						return 0;
					}
					m_pCaliper->setThreshold(atoi(strTemp));
				}
				break;
			case RC_HALF_FILTER_SIZE:			// Half Filter Size
				{
					m_pCaliper->setHalfFilterSize(atoi(strTemp));
					break;
				}
			case RC_NUM_OF_IGNORE:			// Num Of Ignore
				{
					int nNum = atoi(strTemp);
					m_pCaliper->setNumOfIgnore(atoi(strTemp));
				}
				break;
			case RC_FIXTURE_X:			// Fixture X
			{
				double fixture = atof(strTemp);
				m_pCaliper->setFixtureX(fixture);
			}
			break;
			case RC_FIXTURE_Y:			// Fixture Y
			{
				double fixture = atof(strTemp);
				m_pCaliper->setFixtureY(fixture);
			}
			break;
			case RC_FIXTURE_T:			// Fixture T
			{
				double fixture = atof(strTemp);
				m_pCaliper->setFixtureT(fixture);
			}
			break;
			case RC_ORG_START_X:
			{
				double pos = atof(strTemp);
				_st32fPoint pt = m_pCaliper->getOrgStartPt();
				pt.x = pos;
				m_pCaliper->setOrgStartPt(pt);
			}
			break;
			case RC_ORG_START_Y:
			{
				double pos = atof(strTemp);
				_st32fPoint pt = m_pCaliper->getOrgStartPt();
				pt.y = pos;
				m_pCaliper->setOrgStartPt(pt);
			}
			break;
			case RC_ORG_END_X:
			{
				double pos = atof(strTemp);
				_st32fPoint pt = m_pCaliper->getOrgEndPt();
				pt.x = pos;
				m_pCaliper->setOrgEndPt(pt);
			}
			break;
			case RC_ORG_END_Y:
			{
				double pos = atof(strTemp);
				_st32fPoint pt = m_pCaliper->getOrgEndPt();
				pt.y = pos;
				m_pCaliper->setOrgEndPt(pt);
			}
			break;
			}

			switch( index ) {
			case RC_NUM_OF_CALIPER:
			case RC_PROJECTION_LENGTH:
			case RC_SEARCH_LENGTH:

			if (m_pCaliper->getCircleCaliper())
			{
				m_pCaliper->calcCaliperCircle();
				m_pDlgCaliper->m_pViewer->SetCaliperCircleMode(true);
			}
			else
			{
				m_pCaliper->calcCaliperRect();
				m_pDlgCaliper->m_pViewer->SetCaliperCircleMode(false);
			}

				
			//m_pDlgCaliper->m_pViewer->ClearCaliperOverlayDC();
			//if (m_pCaliper->getCircleCaliper())	m_pCaliper->DrawCaliper_Circle(m_pDlgCaliper->m_pDC);
			//else								m_pCaliper->DrawCaliper(m_pDlgCaliper->m_pDC);
				m_pDlgCaliper->m_pViewer->Invalidate();

#ifndef SKIP_XLIST
				m_pDlgCaliper->m_pCaliperResult->updateCaliperList();
#endif
				break;
			}

			switch (index) {
			case RC_ORG_START_X:
			case RC_ORG_START_Y:
			case RC_ORG_END_X:
			case RC_ORG_END_Y:
			{
				m_pCaliper->calcPointsByFixture();

			auto st = m_pCaliper->getStartPt();
			auto ed = m_pCaliper->getEndPt();
			m_pDlgCaliper->m_pViewer->SetCaliperBeginEnd(CPoint(st.x, st.y), CPoint(ed.x, ed.y));

			if (m_pCaliper->getCircleCaliper())
			{
				m_pCaliper->calcCaliperCircle();
				m_pDlgCaliper->m_pViewer->SetCaliperCircleMode(true);
			}
			else
			{
				m_pCaliper->calcCaliperRect();
				m_pDlgCaliper->m_pViewer->SetCaliperCircleMode(false);
			}

			//if (m_pCaliper->getCircleCaliper())	m_pCaliper->calcCaliperCircle();
			//else								m_pCaliper->calcCaliperRect();

			//m_pDlgCaliper->m_pViewer->ClearCaliperOverlayDC();

			//if (m_pCaliper->getCircleCaliper())	m_pCaliper->DrawCaliper_Circle(m_pDlgCaliper->m_pDC);
			//else								m_pCaliper->DrawCaliper(m_pDlgCaliper->m_pDC);

				m_pDlgCaliper->m_pViewer->Invalidate();
			}
			break;
			}

			int nObj = m_pDlgCaliper->m_nObject;
			int nPos = m_pDlgCaliper->m_nPos;
			int nSel = m_cbSelectLine.GetCurSel();
			m_pDlgCaliper->m_bChanged[nObj][nPos][nSel] = TRUE;
		}
		break;
	}

	m_pDlgCaliper->m_pViewer->Invalidate();
	return 0;
}


BOOL CSheetCaliperParam::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	if( m_pDlgCaliper != NULL ) m_pCaliper = &m_pDlgCaliper->m_Caliper[0][0][0];

	for(int i = 0; i < NUM_OF_CALIPER_RESOURCE; i++)
		GetDlgItem(m_nCaliperResouceID[i])->GetWindowRect(&m_rcCaliperEditBox[i]);

	if( m_pCaliper->getCircleCaliper() )	m_pCaliper->calcCaliperCircle();
	else									m_pCaliper->calcCaliperRect();	

	updateUIFromData();
	updateCoordinates();


	CString strTemp;
	/*for(int i = 0; i < MAX_CALIPER_LINES; i++)
	{
		switch(i) {
		case 0:	strTemp = "LEFT";			break;
		case 1:	strTemp = "LEFT TOP";		break;
		case 2:	strTemp = "LEFT BOTTOM";	break;
		case 3:	strTemp = "RIGHT";			break;
		case 4:	strTemp = "RIGHT TOP";		break;
		case 5:	strTemp = "RIGHT BOTTOM";	break;
		}
		m_cbSelectLine.AddString(strTemp);
	}*/

	for (int i = 0; i < MAX_CALIPER_LINES; i++)
	{
		strTemp.Format("%s", m_pDlgCaliper->m_cCaliperName[0][0][i]);
		m_cbSelectLine.AddString(strTemp);
	}

	m_cbSelectLine.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


BOOL CSheetCaliperParam::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if( pMsg->message == WM_LBUTTONUP )
	{
		CPoint tempPt = pMsg->pt;
		CRect rect;
		GetWindowRect(&rect);

		tempPt.x -= rect.left;
		tempPt.y -= rect.top;

		for(int i = 0; i < NUM_OF_CALIPER_RESOURCE; i++)
		{
			GetDlgItem(m_nCaliperResouceID[i])->GetWindowRect(&m_rcCaliperEditBox[i]);
			ScreenToClient(&m_rcCaliperEditBox[i]);
	
			if( PtInRect( m_rcCaliperEditBox[i], tempPt ) )
			{
				PostMessage(WM_VIEW_CONTROL, MSG_INPUT_CALIPER_PARAMS, i);
			}
		}
	}
	else if( pMsg->message == WM_KEYDOWN )
	{
		if( pMsg->wParam == VK_RETURN  || pMsg->wParam == VK_ESCAPE )
			return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CSheetCaliperParam::updateUIFromData()
{
	CString strTemp;

	strTemp.Format("%d", m_pCaliper->getNumOfCalipers() );
	GetDlgItem(IDC_EDIT_NUM_OF_CALIPER)->SetWindowTextA(strTemp);
	
	strTemp.Format("%d", m_pCaliper->getProjectionLength() );
	GetDlgItem(IDC_EDIT_PROJECTION_LENGTH)->SetWindowTextA(strTemp);
	
	strTemp.Format("%d", m_pCaliper->getSearchLength() );
	GetDlgItem(IDC_EDIT_SEARCH_LENGTH)->SetWindowTextA(strTemp);

	strTemp.Format("%d", m_pCaliper->getThreshold() );
	GetDlgItem(IDC_EDIT_THRESHOLD)->SetWindowTextA(strTemp);

	strTemp.Format("%d", m_pCaliper->getHalfFilterSize() );
	GetDlgItem(IDC_EDIT_HALF_FILTER_SIZE)->SetWindowTextA(strTemp);

	strTemp.Format("%d", m_pCaliper->getNumOfIgnore() );
	GetDlgItem(IDC_EDIT_NUM_OF_IGNORE)->SetWindowTextA(strTemp);
	
	strTemp.Format("%.3f", m_pCaliper->getFixtureX() );
	GetDlgItem(IDC_EDIT_FIXTURE_X)->SetWindowTextA(strTemp);

	strTemp.Format("%.3f", m_pCaliper->getFixtureY() );
	GetDlgItem(IDC_EDIT_FIXTURE_Y)->SetWindowTextA(strTemp);

	strTemp.Format("%.3f", m_pCaliper->getFixtureT() );
	GetDlgItem(IDC_EDIT_FIXTURE_T)->SetWindowTextA(strTemp);

	((CComboBox*)GetDlgItem(IDC_COMBO_CALIPER_SEARCH_TYPE))->SetCurSel( m_pCaliper->getEachCaliperSearchType() );
	((CComboBox*)GetDlgItem(IDC_COMBO_FITTING_METHOD))->SetCurSel( m_pCaliper->getFittingMethod() );
	((CComboBox*)GetDlgItem(IDC_COMBO_EDGE_POLARITY))->SetCurSel( m_pCaliper->getContrastDirection() );
	((CComboBox*)GetDlgItem(IDC_COMBO_SEARCH_TYPE))->SetCurSel( m_pCaliper->getSearchType() );
	((CButton*)GetDlgItem(IDC_CHECK_CIRCLE_FITTING))->SetCheck(m_pCaliper->getCircleCaliper()); //HTK 2022-05-19

	//210120 
	int nObj = m_pDlgCaliper->m_nObject;
	int nPos = m_pDlgCaliper->m_nPos;
	int nLine = m_cbSelectLine.GetCurSel();
	m_cbSelectLine.ResetContent();
	for (int i = 0; i < MAX_CALIPER_LINES; i++)
	{
		strTemp.Format("%s", m_pDlgCaliper->m_cCaliperName[nObj][nPos][i]);
		m_cbSelectLine.AddString(strTemp);
	}
	m_cbSelectLine.SetCurSel(nLine);
}

void CSheetCaliperParam::updateDataFromUI()
{
	int nSel = 0;
	CString strTemp;

	GetDlgItem(IDC_EDIT_NUM_OF_CALIPER)->GetWindowTextA(strTemp);
	m_pCaliper->setNumOfCalipers( atoi(strTemp) );
	GetDlgItem(IDC_EDIT_PROJECTION_LENGTH)->GetWindowTextA(strTemp);
	m_pCaliper->setProjectionLength( atoi(strTemp) );
	GetDlgItem(IDC_EDIT_SEARCH_LENGTH)->GetWindowTextA(strTemp);
	m_pCaliper->setSearchLength( atoi(strTemp) );
	GetDlgItem(IDC_EDIT_THRESHOLD)->GetWindowTextA(strTemp);
	m_pCaliper->setThreshold(atoi(strTemp));
	GetDlgItem(IDC_EDIT_HALF_FILTER_SIZE)->GetWindowTextA(strTemp);
	m_pCaliper->setHalfFilterSize( atoi(strTemp) );
	GetDlgItem(IDC_EDIT_NUM_OF_IGNORE)->GetWindowTextA(strTemp);
	m_pCaliper->setNumOfIgnore( atoi(strTemp) );
	GetDlgItem(IDC_EDIT_FIXTURE_X)->GetWindowTextA(strTemp);
	m_pCaliper->setFixtureX(atof(strTemp));
	GetDlgItem(IDC_EDIT_FIXTURE_Y)->GetWindowTextA(strTemp);
	m_pCaliper->setFixtureY(atof(strTemp));
	GetDlgItem(IDC_EDIT_FIXTURE_T)->GetWindowTextA(strTemp);
	m_pCaliper->setFixtureT(atof(strTemp));

	nSel = ((CComboBox*)GetDlgItem(IDC_COMBO_EDGE_POLARITY))->GetCurSel();
	m_pCaliper->setContrastDirection(nSel);
	nSel = ((CComboBox*)GetDlgItem(IDC_COMBO_SEARCH_TYPE))->GetCurSel();
	m_pCaliper->setSearchType(nSel);
	nSel = ((CComboBox*)GetDlgItem(IDC_COMBO_CALIPER_SEARCH_TYPE))->GetCurSel();
	m_pCaliper->setEachCaliperSearchType(nSel);
	nSel = ((CComboBox*)GetDlgItem(IDC_COMBO_FITTING_METHOD))->GetCurSel();
	m_pCaliper->setFittingMethod(nSel);
}

void CSheetCaliperParam::OnCbnSelchangeComboSelectLine()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nObj = m_pDlgCaliper->m_nObject;
	int nPos = m_pDlgCaliper->m_nPos;
	int nSel = MAX(0, m_cbSelectLine.GetCurSel());

	// 캘리퍼 L, R 선택 했을 경우 화면 전환 하도록.
	/*
	CFormTeach *pTeach = (CFormTeach *)m_pDlgCaliper->m_pMain->m_pForm[FORM_TEACH];
	CPoint resultpt = pTeach->m_TeachViewer.getResult();
	if( nSel % 2 == 0 )	resultpt.x = 0;
	else				resultpt.x = pTeach->m_TeachViewer.GetWidth();
	pTeach->m_TeachViewer.setResult(resultpt);
	pTeach->m_TeachViewer.Invalidate();
	*/

	m_pDlgCaliper->m_nLine = nSel;
	m_pCaliper = &m_pDlgCaliper->m_Caliper[nObj][nPos][nSel];

#ifndef SKIP_XLIST
	m_pDlgCaliper->m_pCaliperResult->m_pCaliper = &m_pDlgCaliper->m_Caliper[nObj][nPos][nSel];
	m_pDlgCaliper->m_pCaliperResult->updateCaliperList();

	// YCS 2022-11-14 TestTemp
	//m_pDlgCaliper->m_pViewer->ClearCaliperOverlayDC();
#endif

	// YCS 2022-11-14 testTemp 주석 해제
	//if(!m_pCaliper->getCircleCaliper() )		m_pCaliper->DrawCaliper(m_pDlgCaliper->m_pViewer->getCaliperOverlayDC());
	//else										m_pCaliper->DrawCaliper_Circle(m_pDlgCaliper->m_pViewer->getCaliperOverlayDC());

	auto st = m_pCaliper->getStartPt();
	auto ed = m_pCaliper->getEndPt();

	//m_pDlgCaliper->m_pViewer->SetCaliperBeginEnd(CPoint(st.x, st.y), CPoint(ed.x, ed.y));
	m_pDlgCaliper->m_pViewer->SettingCalipers(CPoint(st.x, st.y), CPoint(ed.x, ed.y), m_pCaliper->getNumOfCalipers(), m_pCaliper->getProjectionLength(), m_pCaliper->getSearchLength(), m_pCaliper->getDirectionSwap(), !m_pCaliper->getCircleCaliper());
	//m_pDlgCaliper->m_pViewer->SetCaliperCircleMode(m_pCaliper->getCircleCaliper());

	updateUIFromData();
	updateCoordinates();

	m_pDlgCaliper->m_pViewer->Invalidate();
}

void CSheetCaliperParam::OnBnClickedBtnCaliperInfoSave()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를/
	CString strTemp;
	strTemp.Format("Save the Caliper Data?");
	if( AfxMessageBox(strTemp, MB_YESNO) != IDYES ) return;

	CString strModelPath = m_pDlgCaliper->m_pMain->m_strCurrentModelPath;

	CPoint st, ed;
	_st32fPoint st_, ed_;
	m_pDlgCaliper->m_pViewer->GetCaliperBeginEnd(st, ed);

	double fixtureX = m_pCaliper->getFixtureX();
	double fixtureY = m_pCaliper->getFixtureY();
	double fixtureT = m_pCaliper->getFixtureT();

	st_.x = st.x - fixtureX;
	st_.y = st.y - fixtureY;
	ed_.x = ed.x - fixtureX;
	ed_.y = ed.y - fixtureY;

	m_pCaliper->setOrgStartPt(st_);
	m_pCaliper->setOrgEndPt(ed_);

	m_pCaliper->calcPointsByFixture();

	for(int nObj = 0; nObj < MAX_CALIPER_OBJECT; nObj++)
	{
		for (int nPos = 0; nPos < NUM_POS; nPos++)
		{
			for (int nLine = 0; nLine < MAX_CALIPER_LINES; nLine++)
			{
				if (m_pDlgCaliper->m_bChanged[nObj][nPos][nLine])
				{
					m_pDlgCaliper->saveCaliperInfo(strModelPath, &m_pDlgCaliper->m_Caliper[nObj][nPos][nLine], nObj, nPos, nLine);
					m_pDlgCaliper->m_bChanged[nObj][nPos][nLine] = FALSE;
				}
			}
		}
	}
}


void CSheetCaliperParam::OnCbnSelchangeComboEdgePolarity()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nSel = ((CComboBox*)GetDlgItem(IDC_COMBO_EDGE_POLARITY))->GetCurSel();
	m_pCaliper->setContrastDirection(nSel);

	int nObj = m_pDlgCaliper->m_nObject;
	int nPos = m_pDlgCaliper->m_nPos;
	int nLine = m_cbSelectLine.GetCurSel();
	m_pDlgCaliper->m_bChanged[nObj][nPos][nLine] = TRUE;
}


void CSheetCaliperParam::OnCbnSelchangeComboSearchType()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nSel = ((CComboBox*)GetDlgItem(IDC_COMBO_SEARCH_TYPE))->GetCurSel();
	m_pCaliper->setSearchType(nSel);
	
	int nObj = m_pDlgCaliper->m_nObject;
	int nPos = m_pDlgCaliper->m_nPos;
	int nLine = m_cbSelectLine.GetCurSel();
	m_pDlgCaliper->m_bChanged[nObj][nPos][nLine] = TRUE;
}


void CSheetCaliperParam::OnBnClickedBtnCaliperCopy()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	if (AfxMessageBox("Caliper Copy?", MB_YESNO) != IDYES) return;

	int curr_obj = m_pDlgCaliper->m_nObject;
	int curr_pos = m_pDlgCaliper->m_nPos;
	int curr_line = m_cbSelectLine.GetCurSel();

	int dst_obj = m_cbCopyJob.GetCurSel();
	int dst_cam = m_cbCopyCam.GetCurSel();
	int dst_pos = m_cbCopyPos.GetCurSel();
	int dst_line = m_cbCopyLine.GetCurSel();

	if (dst_obj < 0 || dst_cam < 0 || dst_pos < 0 || dst_line < 0)	AfxMessageBox("Data Invalid!!");
	else															copy_caliper_data(curr_obj, curr_pos, curr_line, dst_obj, dst_cam, dst_pos, dst_line);
}

void CSheetCaliperParam::copy_caliper_data(int src_obj, int src_pos, int src_line, int dst_job, int dst_cam, int dst_pos, int dst_line)
{
	m_pDlgCaliper->copy_caliper_data(src_obj, src_pos, src_line, dst_job, dst_cam, dst_pos, dst_line);
}


void CSheetCaliperParam::OnCbnSelchangeComboCopyJob()
{
	m_pDlgCaliper->display_copy_caliper_info();
}


void CSheetCaliperParam::OnBnClickedBtnCaliperInfoClose()
{
	m_pDlgCaliper->PostMessage(WM_VIEW_CONTROL, 0, 0);
}


void CSheetCaliperParam::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	if (bShow)
	{
		OnCbnSelchangeComboSelectLine();
	}
}
