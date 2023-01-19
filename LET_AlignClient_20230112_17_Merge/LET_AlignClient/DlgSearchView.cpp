// DlgSearchView.cpp: 구현 파일
//

//#include "pch.h"
#include "stdafx.h"
#include "LET_AlignClient.h"
#include "DlgSearchView.h"
#include "afxdialogex.h"


// DlgSearchView 대화 상자

IMPLEMENT_DYNAMIC(DlgSearchView, CDialogEx)

DlgSearchView::DlgSearchView(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_SEARCH_VIEW, pParent)
{
	m_pBitmapInfo = NULL;
}

DlgSearchView::~DlgSearchView()
{
}

void DlgSearchView::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PIC_SEARCH_VIEW, m_pic_search_view);
	DDX_Control(pDX, IDC_SCROLL_LOAD_V, m_sc_load_v);
	DDX_Control(pDX, IDC_SCROLL_LOAD_H, m_sc_load_h);
}


BEGIN_MESSAGE_MAP(DlgSearchView, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_IMG_LOAD, &DlgSearchView::OnBnClickedBtnImgLoad)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_BN_CLICKED(IDC_BTN_ORIGINAL, &DlgSearchView::OnBnClickedBtnOriginal)
END_MESSAGE_MAP()


// DlgSearchView 메시지 처리기

BOOL DlgSearchView::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	//DrawImagePath(0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


//로드할 이미지 파일 열기
void DlgSearchView::DrawImagePath(int nView)
{
	CFileDialog fileDlg(TRUE, NULL, NULL, OFN_READONLY, _T("image file(*.jpg;*.bmp;*.png;)|*.jpg;*.bmp;*.png;|All Files(*.*)|*.*||"));
	if (fileDlg.DoModal() == IDOK)
	{
		CString path = fileDlg.GetPathName();

		CT2CA pszString(path);
		std::string strPath(pszString);

		m_matSearchImg = cv::imread(strPath, cv::IMREAD_UNCHANGED);

		CreateBitmapInfo(m_matSearchImg.cols, m_matSearchImg.rows, m_matSearchImg.channels() * 8);

		
		DrawImage();
	}

}

//비트맵으로 만들기
void DlgSearchView::CreateBitmapInfo(int w, int h, int bpp)
{
	if (m_pBitmapInfo != NULL)
	{
		delete[]m_pBitmapInfo;
		m_pBitmapInfo = NULL;

	}

	if (bpp == 8)
		m_pBitmapInfo = (BITMAPINFO *) new BYTE[sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD)];
	else // 24 or 32bit
		m_pBitmapInfo = (BITMAPINFO *) new BYTE[sizeof(BITMAPINFO)];

	m_pBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_pBitmapInfo->bmiHeader.biPlanes = 1;
	m_pBitmapInfo->bmiHeader.biBitCount = bpp;
	m_pBitmapInfo->bmiHeader.biCompression = BI_RGB;
	m_pBitmapInfo->bmiHeader.biSizeImage = 0;
	m_pBitmapInfo->bmiHeader.biXPelsPerMeter = 0;
	m_pBitmapInfo->bmiHeader.biYPelsPerMeter = 0;
	m_pBitmapInfo->bmiHeader.biClrUsed = 0;
	m_pBitmapInfo->bmiHeader.biClrImportant = 0;

	if (bpp == 8)
	{
		for (int i = 0; i < 256; i++)
		{
			m_pBitmapInfo->bmiColors[i].rgbBlue = (BYTE)i;
			m_pBitmapInfo->bmiColors[i].rgbGreen = (BYTE)i;
			m_pBitmapInfo->bmiColors[i].rgbRed = (BYTE)i;
			m_pBitmapInfo->bmiColors[i].rgbReserved = 0;
		}
	}

	m_pBitmapInfo->bmiHeader.biWidth = w;
	m_pBitmapInfo->bmiHeader.biHeight = -h;
}

//이미지 그리기
void DlgSearchView::DrawImage()
{
	CClientDC dc(GetDlgItem(IDC_PIC_SEARCH_VIEW));
	CRect rect;
	cv::Point c;

	GetDlgItem(IDC_PIC_SEARCH_VIEW)->GetClientRect(&rect);
	ScreenToClient(&rect);
	SetStretchBltMode(dc.GetSafeHdc(), COLORONCOLOR);
	//resize(m_matSearchImg, m_matSearchImg, cv::Size(rect.Width(), rect.Height()), 0, 0, CV_INTER_LINEAR);
	//imshow("", m_matSearchImg);
	
	
	//이미지 크기의 비율에 맞게 출력
	double X, Y;
	double dbWratio, dbHratio;
	
	X = m_matSearchImg.cols;
	Y = m_matSearchImg.rows;
	
	if (ratio == 1.0)
	{
		if (X >= Y)
		{
			dbWratio = rect.Width() / X;
			X = rect.Width();
			Y = Y * dbWratio;
		}
		else
		{
			dbHratio = rect.Height() / Y;
			Y = rect.Height();
			X = X * dbHratio;
		}

	}
	else
	{
		X= rect.Width();
		Y= rect.Height();
	}

	WIDTH = int(m_matSearchImg.cols*ratio);
	HEIGHT = int(m_matSearchImg.rows*ratio);
	c.x = m_sc_load_h.GetScrollPos();
	c.y = m_matSearchImg.rows - HEIGHT - m_sc_load_v.GetScrollPos();

	StretchDIBits(dc.GetSafeHdc(), 0, 0, X, Y, c.x, c.y,WIDTH, HEIGHT, m_matSearchImg.data, m_pBitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}

//이미지 로드
void DlgSearchView::OnBnClickedBtnImgLoad()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CFileDialog fileDlg(TRUE, NULL, NULL, OFN_READONLY, _T("image file(*.jpg;*.bmp;*.png;)|*.jpg;*.bmp;*.png;|All Files(*.*)|*.*||"));
	if (fileDlg.DoModal() == IDOK)
	{
		CString path = fileDlg.GetPathName();

		CT2CA pszString(path);
		std::string strPath(pszString);

		m_matSearchImg = cv::imread(strPath, cv::IMREAD_UNCHANGED);

		CreateBitmapInfo(m_matSearchImg.cols, m_matSearchImg.rows, m_matSearchImg.channels() * 8);

		ScrollInitRange();
		ratio = 1.0;
		DrawImage();
	}
}

//가로 스크롤
void DlgSearchView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (pScrollBar->GetSafeHwnd() == m_sc_load_h.GetSafeHwnd())
	{
		int pos;
		pos = m_sc_load_h.GetScrollPos();

		if (pos >= 0 == TRUE)
		{
			if (nSBCode == SB_LINEDOWN) m_sc_load_h.SetScrollPos(pos + 10);
			else if (nSBCode == SB_LINEUP) m_sc_load_h.SetScrollPos(pos - 10);
			else if (nSBCode == SB_PAGEUP) m_sc_load_h.SetScrollPos(pos - 50);
			else if (nSBCode == SB_PAGEDOWN) m_sc_load_h.SetScrollPos(pos + 50);
			else if (nSBCode == SB_THUMBTRACK)// 스크롤 헤더를 마우스로 끌때 event
			{
				SCROLLINFO si = { 0 };
				si.cbSize = sizeof(SCROLLINFO);
				si.fMask = SIF_TRACKPOS;
				::GetScrollInfo(pScrollBar->m_hWnd, SB_CTL, &si);

				// 메시지의 nPos 대신에 그냥 트랙포스를 가져다 쓴다.
				pScrollBar->SetScrollPos(si.nTrackPos);
				//m_sb_h_cam1.SetScrollPos(si.nTrackPos);
			}

		}

		DrawImage();

	}

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

//세로 스크롤
void DlgSearchView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (pScrollBar->GetSafeHwnd() == m_sc_load_v.GetSafeHwnd()) {
		int pos;
		pos = m_sc_load_v.GetScrollPos();

		if (pos >= 0 == TRUE)
		{
			if (nSBCode == SB_LINEDOWN) m_sc_load_v.SetScrollPos(pos + 10);
			else if (nSBCode == SB_LINEUP) m_sc_load_v.SetScrollPos(pos - 10);
			else if (nSBCode == SB_PAGEUP) m_sc_load_v.SetScrollPos(pos - 50);
			else if (nSBCode == SB_PAGEDOWN) m_sc_load_v.SetScrollPos(pos + 50);
			else if (nSBCode == SB_THUMBTRACK) // 스크롤 헤더를 마우스로 끌때 event
			{
				SCROLLINFO si = { 0 };
				si.cbSize = sizeof(SCROLLINFO);
				si.fMask = SIF_TRACKPOS;
				::GetScrollInfo(pScrollBar->m_hWnd, SB_CTL, &si);

				// 메시지의 nPos 대신에 그냥 트랙포스를 가져다 쓴다.
				pScrollBar->SetScrollPos(si.nTrackPos);
				//m_sb_h_cam2.SetScrollPos(si.nTrackPos);
			}
		}

		DrawImage();

	}
	
	CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}

//마우스 휠로 줌인 줌아웃
BOOL DlgSearchView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (zDelta < 0)
	{
		//축소비율
		cv::Point c;
		CRect rect;
		GetDlgItem(IDC_PIC_SEARCH_VIEW)->GetClientRect(&rect);

		c.x = m_sc_load_h.GetScrollPos();
		c.y = m_matSearchImg.rows - HEIGHT - m_sc_load_v.GetScrollPos();
		//축소 시 너무 작아지면 이전 이미지 남는거 지우기
		if (c.x<=0|| c.y<=0)
		{
			DrawWhite();
		}

		if (c.y >= 0)
			ratio += 0.1;
		else
		{
			ratio = 1.0;
			MessageBox(_T("Can't zoom out anymore."));
			DrawImage();
			return 0;
		}

		ScrollInitRange();
		DrawImage();
	}
	if (zDelta > 0)
	{
		//확대비율
		if (ratio <= 0.2)
		{
			ratio = ratio;
			MessageBox(_T("Can't zoom in anymore."));
			ScrollInitRange();
			DrawImage();
			return 0;
		}
		else
		{
			ratio -= 0.1;
		}

		ScrollInitRange();
		DrawImage();
	}
	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}

//스크롤 범위
void DlgSearchView::ScrollInitRange()
{
	int x, y;

	x = m_matSearchImg.cols - int(m_matSearchImg.cols*ratio);
	y = m_matSearchImg.rows - int(m_matSearchImg.rows*ratio);

	m_sc_load_v.SetScrollRange(0, y);
	m_sc_load_h.SetScrollRange(0, x);
}

//축소 할 시 뒤에 이미지 지우기
void DlgSearchView::DrawWhite()
{
	CClientDC dc(GetDlgItem(IDC_PIC_SEARCH_VIEW));
	CStatic *staticSize2 = (CStatic *)GetDlgItem(IDC_PIC_SEARCH_VIEW);
	CRect rect;
	GetDlgItem(IDC_PIC_SEARCH_VIEW)->GetClientRect(&rect);

	HBRUSH myBrush = (HBRUSH)GetStockObject(GetSysColor(COLOR_3DFACE));
	HBRUSH oldBrush = (HBRUSH)SelectObject(dc.GetSafeHdc(), myBrush);
	Rectangle(dc.GetSafeHdc(), 0, 0, rect.Width(), rect.Height());

	SelectObject(dc.GetSafeHdc(), oldBrush);
	DeleteObject(myBrush);
}

//원본이미지 버튼
void DlgSearchView::OnBnClickedBtnOriginal()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	ratio = 1.0;
	DrawWhite();
	DrawImage();
}
