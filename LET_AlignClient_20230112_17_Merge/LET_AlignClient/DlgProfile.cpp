// DlgProfile.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "DlgProfile.h"
#include "afxdialogex.h"
#include "ImageProcessing/persistence1d.hpp"

using namespace p1d;

enum profile_direction {
	PROFILE_HORI,
	PROFILE_VERT,
	PROFILE_ALL,
};

// CDlgProfile 대화 상자입니다.

IMPLEMENT_DYNAMIC(CDlgProfile, CDialogEx)

CDlgProfile::CDlgProfile(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgProfile::IDD, pParent)
{
	m_window_width = 694;		// left + right = 14 //  1칸당 40 * 16 = 640 -> 654
	m_window_height = 278;	// top + bottom = 38  //  1칸당 20 * 12 = 240 -> 278
	m_hori_cnt = 17;
	m_vert_cnt = 12;

	m_origin_x = 10;
	m_origin_y = 10;
	m_window_h_st = 30;
	m_window_h_end = 8;
	m_window_w_st = 7;
	m_window_w_end = 7;	

	m_nProfileDirection = PROFILE_ALL;
}

CDlgProfile::~CDlgProfile()
{
}

void CDlgProfile::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgProfile, CDialogEx)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CDlgProfile 메시지 처리기입니다.
BOOL CDlgProfile::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	m_active_width = m_window_width - m_window_w_st - m_window_w_end;
	m_active_height = m_window_height - m_window_h_st - m_window_h_end;
	
	m_Hori_Text.x = m_window_width * 2 / m_hori_cnt;
	m_Hori_Text.y = m_window_height * (m_vert_cnt - 2) / m_vert_cnt;

//	m_Vert_Text.x = m_window_width / m_hori_cnt;
	m_Vert_Text.x = 0;
	m_Vert_Text.y = m_active_height / (m_vert_cnt);

	m_Hori_Text.x = m_active_width  / m_hori_cnt;
	m_Hori_Text.y = m_active_height * (m_vert_cnt - 2) / m_vert_cnt;
	

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CDlgProfile::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CFont font;	
	dc.SetBkMode(TRANSPARENT);

	if(m_ptPFreal_st.x != 0 || m_ptPFreal_st.y != 0 || m_ptPFreal_end.x != 0 || m_ptPFreal_end.y != 0)
	{	
		fnDrawGrid(&dc);
		fnDrawProfile(&dc);
	}
}


void CDlgProfile::fnDrawProfile(CPaintDC * pDC)
{
	if(m_bmpSrcImg.empty()) return;
	if(m_ptPFreal_st.x<0 || m_ptPFreal_end.x> m_bmpSrcImg.cols 
		||m_ptPFreal_st.y<0 || m_ptPFreal_end.y> m_bmpSrcImg.rows) return;

	int diff;
	int scale;
	int min = 256, max = 0, gray;
	int profile_y_range, profile_x_range;
	int x, y;
	int j = 0; 
	double a=0,b=0;

	CPen pPen, pDotPen, pRedDotPen;
	pPen.CreatePen(PS_SOLID, 1, RGB(255, 0, 0) );
	pDotPen.CreatePen(PS_DOT, 1, RGB(0, 255, 0));	
	pRedDotPen.CreatePen(PS_DOT, 1, RGB(0, 0, 255));

	TypedMat<unsigned char> tm; 
	tm.Attach(m_bmpSrcImg);    // 연결방법 3

	pDC->SelectObject(&pPen);

	//20210928 Tkyuha Peak를 찾아서 표시
	Persistence1D _pPeak;
	std::vector<float> _pData;
	std::vector<TPairedExtrema> _pExtremaData;

	switch( m_nProfileDirection ) {
	case PROFILE_HORI:
		SetWindowText("Profile - X축");
		diff = m_ptPFreal_end.x - m_ptPFreal_st.x;
		scale = diff / m_hori_cnt;
		
		if(m_ptPFreal_end.x != m_ptPFreal_st.x)  a = (m_ptPFreal_end.y-m_ptPFreal_st.y) / double(m_ptPFreal_end.x-m_ptPFreal_st.x);
		b = m_ptPFreal_st.y - (a)*m_ptPFreal_st.x;

		for(int i = m_ptPFreal_st.x; i < m_ptPFreal_end.x; i++)
		{
			if(int(a * i + b) < 0 || int(a * i + b) > m_bmpSrcImg.cols) continue;
			gray = tm(int(a*i+b),i,0); //m_ptPFreal_st.y
			min = min < gray ? min : gray;
			max = max > gray ? max : gray;

			_pData.push_back(gray); //20210928 Tkyuha Peak 찾기위한 Raw Data
		}

		// y축 text
		for(int i = 1; i < m_vert_cnt - 2; i++)
		{
			CString str;
			scale = int((max - min) / (double)(m_vert_cnt - 4) * (i - 1));
			str.Format("%5d", max - scale );
			pDC->TextOutA(m_Vert_Text.x, m_Vert_Text.y * i - 7, str, str.GetLength() );
		}
		
		// x축 text		
		for(int i = 1; i < m_hori_cnt - 1; i+= 2)
		{
			CString str;
			scale = m_ptPFreal_st.x + diff * (i - 1) / (m_hori_cnt - 3);
			str.Format("%5d", scale);
			pDC->TextOutA(m_Hori_Text.x * i + 20, m_Hori_Text.y + 3, str, str.GetLength() );
		}

		//20210928 Tkyuha Peak Search
		_pPeak.RunPersistence(_pData);
		_pPeak.GetPairedExtrema(_pExtremaData,30);
		//draw gray
		gray = tm(m_ptPFreal_st.y,m_ptPFreal_st.x,0);
		profile_y_range = (m_vert_cnt - 4) * (m_active_height / m_vert_cnt);
		profile_x_range = (m_hori_cnt - 3) * (m_active_width / m_hori_cnt);
		x = m_Hori_Text.x + m_Hori_Text.x;
		
		if( (max - min) == 0)	y = m_Vert_Text.y;
		else	y = m_Vert_Text.y + profile_y_range * (max - gray) / (max - min);

		pDC->MoveTo(x, y);

		for(int i = m_ptPFreal_st.x + 1, j = 1; i <= m_ptPFreal_end.x; i++, j++)
		{
			if(int(a * i + b) < 0 || int(a * i + b) > m_bmpSrcImg.cols) continue;
			gray = tm(int(a * i + b), i, 0); //m_ptPFreal_st.y
			x = int(m_Hori_Text.x + m_Hori_Text.x + (profile_x_range / (double)diff * j));

			if( (max - min) == 0)				y = m_Vert_Text.y;
			else			y = m_Vert_Text.y + profile_y_range * (max - gray) / (max - min);

			pDC->LineTo(x, y);
		}

		break;
	case PROFILE_VERT:
		SetWindowText("Profile - Y축");
		diff = m_ptPFreal_end.y - m_ptPFreal_st.y;
		scale = diff / m_vert_cnt;

		if(m_ptPFreal_end.y != m_ptPFreal_st.y)  a = (m_ptPFreal_end.x-m_ptPFreal_st.x) / double(m_ptPFreal_end.y-m_ptPFreal_st.y);
		b = m_ptPFreal_st.x - (a)*m_ptPFreal_st.y;

		for(int i = m_ptPFreal_st.y; i < m_ptPFreal_end.y; i++)
		{
			//if(int(a * i + b) < 0 || int(a * i + b) > m_bmpSrcImg.rows) continue;
			if(int(a * i + b) < 0 || int(a * i + b) > m_bmpSrcImg.cols) continue;
			gray = tm(i, int(a * i + b), 0); //m_ptPFreal_st.x
			min = min < gray ? min : gray;
			max = max > gray ? max : gray;
			_pData.push_back(gray); //20210928 Tkyuha Peak 찾기위한 Raw Data
		}

		// y축 text
		for(int i = 1; i < m_vert_cnt - 2; i++)
		{
			CString str;
			scale = int((max - min) / (double)(m_vert_cnt - 4) * (i - 1));
			str.Format("%5d", max - scale );
			pDC->TextOutA(m_Vert_Text.x, m_Vert_Text.y * i - 7, str, str.GetLength() );
		}
		
		// x축 text		
		for(int i = 1; i < m_hori_cnt - 1; i+= 2)
		{
			CString str;
			scale = m_ptPFreal_st.y + diff * (i - 1) / (m_hori_cnt - 3);
			str.Format("%5d", scale);
			pDC->TextOutA(m_Hori_Text.x * i + 20, m_Hori_Text.y + 3, str, str.GetLength() );
		}

		// 20210928 Tkyuha Peak Search
		_pPeak.RunPersistence(_pData);
		_pPeak.GetPairedExtrema(_pExtremaData, 30);

		// draw gray
		gray = tm(m_ptPFreal_st.y,m_ptPFreal_st.x,0); 
		profile_y_range = (m_vert_cnt - 4) * (m_active_height / m_vert_cnt);
		profile_x_range = (m_hori_cnt - 3) * (m_active_width / m_hori_cnt);
		x = m_Hori_Text.x + m_Hori_Text.x;
		
		if( (max - min) == 0)			y = m_Vert_Text.y;
		else			y = m_Vert_Text.y + profile_y_range * (max - gray) / (max - min);

		pDC->MoveTo(x, y);		

		for(int i = m_ptPFreal_st.y + 1, j = 1; i <= m_ptPFreal_end.y; i++, j++)
		{
			//if(int(a*i+b)<0 || int(a*i+b)>m_bmpSrcImg.rows) continue;
			if(int(a * i + b) < 0 || int(a * i + b) > m_bmpSrcImg.cols) continue;
			gray =  tm(i,int(a*i+b),0); //m_ptPFreal_st.x
			x = int(m_Hori_Text.x + m_Hori_Text.x + (profile_x_range / (double)diff * j));
			if( (max - min) == 0)			y = m_Vert_Text.y;
			else				y = m_Vert_Text.y + profile_y_range * (max - gray) / (max - min);
			pDC->LineTo(x, y);
		}		
		break;
	case PROFILE_ALL:
		SetWindowText("Profile - ALL");	
		diff = 256;
		scale = diff / m_vert_cnt;

		int histo[256];
		memset(histo, 0, sizeof(histo) );

		if( m_ptROI_st.x == 0 && m_ptROI_st.y == 0 && m_ptROI_end.x == 0 && m_ptROI_end.y == 0 )
		{
			m_ptROI_end.x = m_bmpSrcImg.cols;
			m_ptROI_end.y = m_bmpSrcImg.rows;
		}

		for(y = m_ptROI_st.y; y < m_ptROI_end.y; y++)
			for(x = m_ptROI_st.x; x < m_ptROI_end.x; x++)
				histo[ (uchar)m_bmpSrcImg.data[y * m_bmpSrcImg.cols + x] ]++;

		int nMax, nMin;
		nMax = 0;
		nMin = m_bmpSrcImg.rows * m_bmpSrcImg.cols;

		for(int i = 0; i < 256; i++)
		{
			if( max < histo[i] )		max = histo[i];
			if( min > histo[i] )		min = histo[i];
		}

		// y축 text
		for(int i = 1; i < m_vert_cnt - 2; i++)
		{
			CString str;
			scale = int((max - min) / (double)(m_vert_cnt - 4) * (i - 1));
			str.Format("%5d", max - scale );
			pDC->TextOutA(m_Vert_Text.x, m_Vert_Text.y * i - 7, str, str.GetLength() );
		}
		
		// x축 text		
		for(int i = 1; i < m_hori_cnt - 1; i+= 1)
		{
			CString str;
			scale = m_ptPFreal_st.y + diff * (i - 1) / (m_hori_cnt - 3);
			str.Format("%5d", scale);
			pDC->TextOutA(m_Hori_Text.x * i + 20, m_Hori_Text.y + 3, str, str.GetLength() );
		}

		// draw gray
		gray = (uchar)m_bmpSrcImg.data[m_ptPFreal_st.y * m_bmpSrcImg.cols + m_ptPFreal_st.x];
		profile_y_range = (m_vert_cnt - 4) * (m_active_height / m_vert_cnt);
		profile_x_range = (m_hori_cnt - 3) * (m_active_width / m_hori_cnt);
		x = m_Hori_Text.x + m_Hori_Text.x;
		
		if( (max - min) == 0)			y = m_Vert_Text.y;
		else			y = m_Vert_Text.y + profile_y_range * (max - gray) / (max - min);

		pDC->MoveTo(x, y);

		for(int i = 0; i < 256; i++)
		{
			gray = histo[i];
			x = int(m_Hori_Text.x + m_Hori_Text.x + (profile_x_range / (double)diff * i));
			if( (max - min) == 0)				y = m_Vert_Text.y;
			else				y = m_Vert_Text.y + profile_y_range * (max - gray) / (max - min);
			pDC->LineTo(x, y);
		}
		break;
	};

	//20210928 Tkyuha draw Peak		
	if (_pExtremaData.size() > 0)
	{
		for (std::vector<TPairedExtrema>::iterator it = _pExtremaData.begin(); it != _pExtremaData.end(); it++)
		{
			int _x1 = (*it).MinIndex;
			int _x2 = (*it).MaxIndex;

			pDC->SelectObject(&pDotPen);
			x = int(m_Hori_Text.x + m_Hori_Text.x + (profile_x_range / (double)diff * _x1));
			pDC->MoveTo(x, m_active_height);
			pDC->LineTo(x, 0);

			pDC->SelectObject(&pRedDotPen);
			x = int(m_Hori_Text.x + m_Hori_Text.x + (profile_x_range / (double)diff * _x2));
			pDC->MoveTo(x, m_active_height);
			pDC->LineTo(x, 0);
		}
	}

	_pData.clear();
	_pExtremaData.clear();

	pPen.DeleteObject();
	pDotPen.DeleteObject();
	pRedDotPen.DeleteObject();
}


void CDlgProfile::fnDrawGrid(CPaintDC * pDC)
{
	CPen pPen;
	pPen.CreatePen(PS_DOT, 1, RGB(0, 0, 0) );
	pDC->SelectObject(&pPen);

	// 기준 Y 축
	pDC->MoveTo(m_active_width / m_hori_cnt, 0);
	pDC->LineTo(m_active_width / m_hori_cnt, m_active_height);

	// 기준 X 축
	pDC->MoveTo(0, m_active_height * (m_vert_cnt - 2) / m_vert_cnt);
	pDC->LineTo(m_active_width, m_active_height * (m_vert_cnt - 2) / m_vert_cnt);

	// X 축
	for(int i = 2; i < m_hori_cnt; i++)
	{
		pDC->MoveTo(m_active_width * i / m_hori_cnt, 0);
		pDC->LineTo(m_active_width * i / m_hori_cnt, m_active_height * (m_vert_cnt - 2) / m_vert_cnt);
	}

	// Y 축
	for(int i = 1; i < m_vert_cnt - 2; i++)
	{
		pDC->MoveTo(m_active_width / m_hori_cnt, m_active_height * i / m_vert_cnt);
		pDC->LineTo(m_active_width, m_active_height * i / m_vert_cnt);
	}

	pPen.DeleteObject();
}

void CDlgProfile::fnCalcRealCoordi(CPoint  ptSRC_st, CPoint  ptSRC_end)
{
	double d1, d2;
	CvPoint pt1, pt2;

	d1 = sqrt( (double)(ptSRC_st.x - ptSRC_end.x) * (ptSRC_st.x - ptSRC_end.x) );
	d2 = sqrt( (double)(ptSRC_st.y - ptSRC_end.y) * (ptSRC_st.y - ptSRC_end.y) );
	
	if(d1 > d2)		m_nProfileDirection = PROFILE_HORI;
	else		m_nProfileDirection = PROFILE_VERT;
	
	switch( m_nProfileDirection) {
	case PROFILE_HORI:
		if( ptSRC_st.x < ptSRC_end.x )
		{
			pt1.x = ptSRC_st.x;
			pt1.y = ptSRC_st.y;
			pt2.x = ptSRC_end.x;
			pt2.y = ptSRC_end.y;
		}
		else
		{
			pt1.x = ptSRC_end.x;
			pt1.y = ptSRC_end.y;
			pt2.x = ptSRC_st.x;
			pt2.y = ptSRC_st.y;
		}
		break;
	case PROFILE_VERT:
		if( ptSRC_st.y < ptSRC_end.y )
		{
			pt1.x = ptSRC_st.x;
			pt1.y = ptSRC_st.y;
			pt2.x = ptSRC_end.x;
			pt2.y = ptSRC_end.y;
		}
		else
		{
			pt1.x = ptSRC_end.x;
			pt1.y = ptSRC_end.y;
			pt2.x = ptSRC_st.x;
			pt2.y = ptSRC_st.y;			
		}
		break;
	};

	m_ptPFreal_st.x = pt1.x;
	m_ptPFreal_st.y = pt1.y ;
	m_ptPFreal_end.x = pt2.x;	
	m_ptPFreal_end.y = pt2.y;
}