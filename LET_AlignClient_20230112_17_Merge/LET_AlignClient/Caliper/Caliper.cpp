// Caliper.cpp: 구현 파일입니다.
//

#include "stdafx.h"
#include "..\LET_AlignClient.h"
#include <math.h>
#include "Caliper.h"
#include "MatrixAlgebra.h"
#include <numeric>

#include "RANSAC_EllipseFittingAlgorithm.h"

#define _MLESAC_USE

#ifdef _MLESAC_USE
#include "..\ImageProcessing\MathUtil.h" // MLESAC Algorithm을 사용 하기 위함
#endif // _MLESAC_USE

// CCaliper

IMPLEMENT_DYNAMIC(CCaliper, CWnd)

typedef techsoft::matrix<double>	dMatrix;

CCaliper::CCaliper()
{
	m_bIsMakeLine = FALSE;
	m_dbLineX = NULL;
	m_dbLineY = NULL;
	m_dbContrast = NULL;
	m_ptCaliper_LUT = NULL;
	m_nEachCaliperSearchType = 0;
	m_nProjectionLength_OLD = 0;
	m_nSearchLength_OLD = 0;
	m_nNumOfCaliper_OLD = 0;
	m_bCircleCaliper = FALSE;
	m_nLastNumOfCaliper = 0;

	m_nProjectionLengthCircle_OLD = 0;
	m_nSearchLengthCircle_OLD = 0;
	m_nNumOfCaliperCircle_OLD = 0;
	m_nFittingMethod = 0;

	m_bDirectionSwap = FALSE;
	m_nProjectionLength = 10;
	m_nSearchLength = 50;
	m_nSearchType = 0;
	m_nNumOfCalipers = 20;
	m_nContrastDirection = 0;
	m_nEachCaliperSearchType = 0;
	m_nThreshold = 20;
	m_nHalfFilterSize = 1;
	m_nNumOfIgnore = 5;

	m_dbFixtureX = 0.0;
	m_dbFixtureY = 0.0;
	m_dbFixtureT = 0.0;	

	m_ptOrgStart.x = 100;	m_ptOrgStart.y = 100;
	m_ptOrgEnd.x = 500;		m_ptOrgEnd.y = 500;

	m_ptStart = m_ptOrgStart;
	m_ptEnd = m_ptOrgEnd;
	memset(m_bUseCaliper, 0, sizeof(m_bUseCaliper));
	memset(m_FindInfo, 0, sizeof(m_FindInfo));
	memset(m_AreaInfo, 0, sizeof(m_AreaInfo));

	m_pDC = NULL;
}

CCaliper::~CCaliper()
{
//	m_vtCaliperFindInfo.clear();

	for(int i = 0; i < MAX_CALIPER; i++ )
	{
		m_pVtLineX[i].clear();
		m_pVtLineY[i].clear();
		m_pVtLineContrast[i].clear();
	}

	if (m_ptCaliper_LUT != NULL)
	{
		for (int k = 0; k < m_nNumOfCaliper_OLD; k++)
		{
			if (m_ptCaliper_LUT[k] != NULL)
			{
				for (int i = 0; i < m_nProjectionLength_OLD; i++)
					delete m_ptCaliper_LUT[k][i];
			}

			delete[]m_ptCaliper_LUT[k];
		}

		delete[]m_ptCaliper_LUT;
		m_ptCaliper_LUT = NULL;
	}

	if( m_dbLineX != NULL )
	{
		for(int i = 0; i < m_nLastNumOfCaliper; i++)
		{
			delete m_dbLineX[i];
			delete m_dbLineY[i];
			delete m_dbContrast[i];
		}
		delete []m_dbLineX;
		delete []m_dbLineY;
		delete []m_dbContrast;

		m_dbLineX = NULL;
		m_dbLineY = NULL;
		m_dbContrast = NULL;
	}
}


BEGIN_MESSAGE_MAP(CCaliper, CWnd)
END_MESSAGE_MAP()



// CCaliper 메시지 처리기입니다.


void CCaliper::DrawCaliper_Circle(FakeDC*pDC)
{
	//if( pDC == NULL ) return;

	//CPen penBase(PS_SOLID, 10, RGB(192, 192, 192)), *pOldPen;
	//CPen penCaliper(PS_SOLID, 5, RGB(128, 128, 255));
	//CPen penCaliperNoUse(PS_SOLID, 5, RGB(255, 255, 128));
	//CBrush brBase(RGB(255, 255, 0)), *pOldBr;
	//COLORREF old_color = pDC->SetTextColor(RGB(128, 255, 128));


	//pOldPen = pDC->SelectObject( &penBase );
	//pOldBr = pDC->SelectObject( &brBase );
	//pDC->SelectStockObject(NULL_BRUSH);

	//// Draw Base Line 
	//CPoint pt1, pt2, pt3;
	//pt1.x = (int)m_ptStart.x;
	//pt1.y = (int)m_ptStart.y;
	//pt2.x = (int)m_ptEnd.x;
	//pt2.y = (int)m_ptEnd.y;

	//double dx = pt1.x - pt2.x;
	//double dy = pt1.y - pt2.y;
	//double dist = sqrt( dx * dx + dy * dy ) / 2.0;

	//pt3.x = (int)((pt1.x + pt2.x) / 2.0);
	//pt3.y = (int)((pt1.y + pt2.y) / 2.0);
	//pDC->Ellipse( pt3.x - (int)dist, pt3.y - (int)dist, pt3.x + (int)dist, pt3.y  + (int)dist);
	//
	//cv::Point2f pts[4];	
	//CvFont font;
	//font = cvFont(2, 2);
	//font.thickness = 1;
	//font.vscale = (float)0.2;

	//double baseX = 0.0;
	//double baseY = 0.0;

	//double dStepT = (PI * 2) / m_nNumOfCalipers;
	//double cx = pt3.x;
	//double cy = pt3.y;
	//double x = cx + dist + m_nSearchLength / 2 + 10;
	//double y = cy;

	//pDC->MoveTo(pt1);
	//pDC->LineTo(pt2);
	//pDC->Ellipse( pt1.x - 3, pt1.y - 3, pt1.x + 3, pt1.y  + 3);
	//pDC->Ellipse( pt2.x - 3, pt2.y - 3, pt2.x + 3, pt2.y  + 3);

	//// Draw Caliper
	//for(int i = 0; i < m_nNumOfCalipers; i++)
	//{
	//	m_AreaInfo[i].rect.points(pts);
	//	
	//	for(int n = 0; n < 4; n++)
	//	{
	//		pt1.x = (int)(pts[(n + 0) % 4].x);
	//		pt1.y = (int)(pts[(n + 0) % 4].y);

	//		pt2.x = (int)(pts[(n + 1) % 4].x);
	//		pt2.y = (int)(pts[(n + 1) % 4].y);

	//		if( m_AreaInfo[i].use != TRUE  )
	//		{
	//			pDC->SelectObject( &penCaliperNoUse );
	//			pDC->MoveTo(pt1);
	//			pDC->LineTo(pt2);
	//		}
	//		else
	//		{
	//			pDC->SelectObject( &penCaliper );
	//			pDC->MoveTo(pt1);
	//			pDC->LineTo(pt2);
	//		}
	//	}
	//	
	//	CvPoint pt;
	//	pt.x = (int)(((x - cx) * cos(dStepT * i + m_dbLineT) - (y - cy) * sin(dStepT * i + m_dbLineT) + cx));
	//	pt.y = (int)(((x - cx) * sin(dStepT * i + m_dbLineT) + (y - cy) * cos(dStepT * i + m_dbLineT) + cy));
	
	//	baseX = pts[1].x;

	//	if( pt.x < baseX )	pt.x -= 10;


		//CString strTemp;
		//strTemp.Format("%d", i + 1);
	//	pDC->TextOutA(pt.x, pt.y, strTemp);


	//	//CString strTemp;
	//	//strTemp.Format("%d", i + 1);
	//	//cvPutText(pImg, strTemp, pt, &font, cvScalar(255,255,255));
	//}

	////////////////// Draw Arrow
	//if( m_bDirectionSwap )
	//{
	//	// Base Line
	//	x = cx + (dist - m_nSearchLength / 2) / 4;
	//	y = cy;

	//	pt1.x = (int)((x - cx) * cos(m_dbLineT + PI / 2) - (y - cy) * sin(m_dbLineT + PI / 2) + cx);
	//	pt1.y = (int)((x - cx) * sin(m_dbLineT + PI / 2) + (y - cy) * cos(m_dbLineT + PI / 2) + cy);

	//	x = cx + (dist - m_nSearchLength / 2) * 3 / 4;
	//	y = cy;

	//	pt2.x = (int)((x - cx) * cos(m_dbLineT + PI / 2) - (y - cy) * sin(m_dbLineT + PI / 2) + cx);
	//	pt2.y = (int)((x - cx) * sin(m_dbLineT + PI / 2) + (y - cy) * cos(m_dbLineT + PI / 2) + cy);

	//	pDC->MoveTo(pt1);
	//	pDC->LineTo(pt2);
	//	//cvLine(pImg, pt1, pt2, cvScalar(255, 128, 255));

	//	// Arrow
	//	pt3.x = pt1.x + 10;
	//	pt3.y = pt1.y;

	//	CPoint pt4;
	//	pt4.x = (int)((pt3.x - pt1.x) * cos(m_dbLineT + PI / 2) - (pt3.x - pt1.x) * sin(m_dbLineT + PI / 2) + pt1.x);
	//	pt4.y = (int)((pt3.x - pt1.x) * sin(m_dbLineT + PI / 2) + (pt3.x - pt1.x) * cos(m_dbLineT + PI / 2) + pt1.y);

	//	pDC->MoveTo(pt1);
	//	pDC->LineTo(pt4);
	//	//cvLine(pImg, pt1, pt4, cvScalar(255, 128, 255));

	//	pt4.x = (int)((pt3.x - pt1.x) * cos(m_dbLineT) - (pt3.x - pt1.x) * sin(m_dbLineT) + pt1.x);
	//	pt4.y = (int)((pt3.x - pt1.x) * sin(m_dbLineT) + (pt3.x - pt1.x) * cos(m_dbLineT) + pt1.y);
	//	pDC->MoveTo(pt1);
	//	pDC->LineTo(pt4);
	//	//cvLine(pImg, pt1, pt4, cvScalar(255, 128, 255));

	//	// Base Line
	//	x = cx + (dist - m_nSearchLength / 2) / 4;
	//	y = cy;

	//	pt1.x = (int)((x - cx) * cos(m_dbLineT - PI / 2) - (y - cy) * sin(m_dbLineT - PI / 2) + cx);
	//	pt1.y = (int)((x - cx) * sin(m_dbLineT - PI / 2) + (y - cy) * cos(m_dbLineT - PI / 2) + cy);

	//	x = cx + (dist - m_nSearchLength / 2) * 3 / 4;
	//	y = cy;
		
	//	pt2.x = (int)((x - cx) * cos(m_dbLineT - PI / 2) - (y - cy) * sin(m_dbLineT - PI / 2) + cx);
	//	pt2.y = (int)((x - cx) * sin(m_dbLineT - PI / 2) + (y - cy) * cos(m_dbLineT - PI / 2) + cy);
	//	
	//	pDC->MoveTo(pt1);
	//	pDC->LineTo(pt2);
	//	//cvLine(pImg, pt1, pt2, cvScalar(255, 128, 255));

	//	// Arrow
	//	pt3.x = pt1.x + 10;
	//	pt3.y = pt1.y;

	//	pt4.x = (int)((pt3.x - pt1.x) * cos(m_dbLineT + PI) - (pt3.x - pt1.x) * sin(m_dbLineT + PI ) + pt1.x);
	//	pt4.y = (int)((pt3.x - pt1.x) * sin(m_dbLineT + PI) + (pt3.x - pt1.x) * cos(m_dbLineT + PI ) + pt1.y);
	//	pDC->MoveTo(pt1);
	//	pDC->LineTo(pt4);
	//	//cvLine(pImg, pt1, pt4, cvScalar(255, 128, 255));

	//	pt4.x = (int)((pt3.x - pt1.x) * cos(m_dbLineT - PI / 2) - (pt3.x - pt1.x) * sin(m_dbLineT - PI / 2) + pt1.x);
	//	pt4.y = (int)((pt3.x - pt1.x) * sin(m_dbLineT - PI / 2) + (pt3.x - pt1.x) * cos(m_dbLineT - PI / 2) + pt1.y);
	//	pDC->MoveTo(pt1);
	//	pDC->LineTo(pt4);
	//	//cvLine(pImg, pt1, pt4, cvScalar(255, 128, 255));
	//}
	//else
	//{
	//	// Base Line
	//	x = cx + (dist - m_nSearchLength / 2) / 4;
	//	y = cy;

	//	pt1.x = (int)((x - cx) * cos(m_dbLineT + PI / 2) - (y - cy) * sin(m_dbLineT + PI / 2) + cx);
	//	pt1.y = (int)((x - cx) * sin(m_dbLineT + PI / 2) + (y - cy) * cos(m_dbLineT + PI / 2) + cy);

	//	x = cx + (dist - m_nSearchLength / 2) * 3 / 4;
	//	y = cy;

	//	pt2.x = (int)((x - cx) * cos(m_dbLineT + PI / 2) - (y - cy) * sin(m_dbLineT + PI / 2) + cx);
	//	pt2.y = (int)((x - cx) * sin(m_dbLineT + PI / 2) + (y - cy) * cos(m_dbLineT + PI / 2) + cy);

	//	pDC->MoveTo(pt1);
	//	pDC->LineTo(pt2);
	//	//cvLine(pImg, pt1, pt2, cvScalar(255, 128, 255));

	//	// Arrow
	//	CPoint pt4;
	//	pt3.x = pt2.x + 10;
	//	pt3.y = pt2.y;

	//	pt4.x = (int)((pt3.x - pt2.x) * cos(m_dbLineT + PI) - (pt3.x - pt2.x) * sin(m_dbLineT + PI ) + pt2.x);
	//	pt4.y = (int)((pt3.x - pt2.x) * sin(m_dbLineT + PI) + (pt3.x - pt2.x) * cos(m_dbLineT + PI ) + pt2.y);
	//	pDC->MoveTo(pt2);
	//	pDC->LineTo(pt4);
	//	//cvLine(pImg, pt2, pt4, cvScalar(128, 255, 255), 1);

	//	pt4.x = (int)((pt3.x - pt2.x) * cos(m_dbLineT - PI / 2) - (pt3.x - pt2.x) * sin(m_dbLineT - PI / 2) + pt2.x);
	//	pt4.y = (int)((pt3.x - pt2.x) * sin(m_dbLineT - PI / 2) + (pt3.x - pt2.x) * cos(m_dbLineT - PI / 2) + pt2.y);
	//	pDC->MoveTo(pt2);
	//	pDC->LineTo(pt4);
	//	//cvLine(pImg, pt2, pt4, cvScalar(255, 128, 255));

	//	// Base Line
	//	x = cx + (dist - m_nSearchLength / 2) / 4;
	//	y = cy;

	//	pt1.x = (int)((x - cx) * cos(m_dbLineT - PI / 2) - (y - cy) * sin(m_dbLineT - PI / 2) + cx);
	//	pt1.y = (int)((x - cx) * sin(m_dbLineT - PI / 2) + (y - cy) * cos(m_dbLineT - PI / 2) + cy);

	//	x = cx + (dist - m_nSearchLength / 2) * 3 / 4;
	//	y = cy;

	//	pt2.x = (int)((x - cx) * cos(m_dbLineT - PI / 2) - (y - cy) * sin(m_dbLineT - PI / 2) + cx);
	//	pt2.y = (int)((x - cx) * sin(m_dbLineT - PI / 2) + (y - cy) * cos(m_dbLineT - PI / 2) + cy);
	//	pDC->MoveTo(pt1);
	//	pDC->LineTo(pt2);
	//	//cvLine(pImg, pt1, pt2, cvScalar(255, 128, 255));

	//	// Arrow
	//	pt3.x = pt2.x + 10;
	//	pt3.y = pt2.y;

	//	pt4.x = (int)((pt3.x - pt2.x) * cos(m_dbLineT + PI / 2) - (pt3.x - pt2.x) * sin(m_dbLineT + PI / 2) + pt2.x);
	//	pt4.y = (int)((pt3.x - pt2.x) * sin(m_dbLineT + PI / 2) + (pt3.x - pt2.x) * cos(m_dbLineT + PI / 2) + pt2.y);
	//	pDC->MoveTo(pt2);
	//	pDC->LineTo(pt4);
	//	//cvLine(pImg, pt2, pt4, cvScalar(128, 255, 255), 1);

	//	pt4.x = (int)((pt3.x - pt2.x) * cos(m_dbLineT) - (pt3.x - pt2.x) * sin(m_dbLineT) + pt2.x);
	//	pt4.y = (int)((pt3.x - pt2.x) * sin(m_dbLineT) + (pt3.x - pt2.x) * cos(m_dbLineT) + pt2.y);
	//	pDC->MoveTo(pt2);
	//	pDC->LineTo(pt4);
	//	//cvLine(pImg, pt2, pt4, cvScalar(255, 128, 255));		
	//}

	//pDC->SelectObject( pOldPen );
	//pDC->SelectObject( pOldBr );
	//pDC->SetTextColor(old_color);
}


void CCaliper::DrawCaliper_Circle(IplImage *pImg)
{
	// Draw Base Line
	CvPoint pt1, pt2, pt3;
	pt1.x = (int)m_ptStart.x;
	pt1.y = (int)m_ptStart.y;
	pt2.x = (int)m_ptEnd.x;
	pt2.y = (int)m_ptEnd.y;

	double dx = pt1.x - pt2.x;
	double dy = pt1.y - pt2.y;
	double dist = sqrt( dx * dx + dy * dy ) / 2.0;

	pt3.x = (int)((pt1.x + pt2.x) / 2.0);
	pt3.y = (int)((pt1.y + pt2.y) / 2.0);
	
	cvCircle(pImg, pt3, (int)dist, cvScalar(255, 128, 128), 1);

	cv::Point2f pts[4];
	
	CvFont font;
	font = cvFont(2, 2);
	font.thickness = 1;
	font.vscale = (float)0.2;

	double baseX = 0.0;
	double baseY = 0.0;

	double dStepT = (PI * 2) / m_nNumOfCalipers;
	double cx = pt3.x;
	double cy = pt3.y;
	double x = cx + dist + m_nSearchLength / 2 + 10;
	double y = cy;

	cvLine( pImg, cvPoint( pt1.x, pt1.y ), cvPoint(pt2.x, pt2.y), cvScalar(255, 0, 255));
	cvCircle( pImg, cvPoint( pt1.x, pt1.y ), 3, cvScalar(255, 255, 0));
	cvCircle( pImg, cvPoint(pt2.x, pt2.y), 3, cvScalar(255, 255, 0));


	// Draw Caliper
	for(int i = 0; i < m_nNumOfCalipers; i++)
	{
		m_AreaInfo[i].rect.points(pts);
		
		for(int n = 0; n < 4; n++)
		{
			pt1.x = (int)(pts[(n + 0) % 4].x);
			pt1.y = (int)(pts[(n + 0) % 4].y);

			pt2.x = (int)(pts[(n + 1) % 4].x);
			pt2.y = (int)(pts[(n + 1) % 4].y);

			if( m_AreaInfo[i].use != TRUE  )
				cvLine(pImg, pt1, pt2, cvScalar(128, 128, 255));
			else
			{
				if( i == 0 )	cvLine(pImg, pt1, pt2, cvScalar(128, 255, 128));
				else			cvLine(pImg, pt1, pt2, cvScalar(255, 128, 128));
			}
		}
		
		CvPoint pt;
		pt.x = (int)(((x - cx) * cos(dStepT * i + m_dbLineT) - (y - cy) * sin(dStepT * i + m_dbLineT) + cx));
		pt.y = (int)(((x - cx) * sin(dStepT * i + m_dbLineT) + (y - cy) * cos(dStepT * i + m_dbLineT) + cy));

		baseX = pts[1].x;

		if( pt.x < baseX )	pt.x -= 10;

		CString strTemp;
		strTemp.Format("%d", i + 1);
		cvPutText(pImg, strTemp, pt, &font, cvScalar(255,255,255));
	}

	//////////////// Draw Arrow
	if( m_bDirectionSwap )
	{
		// Base Line
		x = cx + (dist - m_nSearchLength / 2) / 4;
		y = cy;

		pt1.x = (int)((x - cx) * cos(m_dbLineT + PI / 2) - (y - cy) * sin(m_dbLineT + PI / 2) + cx);
		pt1.y = (int)((x - cx) * sin(m_dbLineT + PI / 2) + (y - cy) * cos(m_dbLineT + PI / 2) + cy);

		x = cx + (dist - m_nSearchLength / 2) * 3 / 4;
		y = cy;

		pt2.x = (int)((x - cx) * cos(m_dbLineT + PI / 2) - (y - cy) * sin(m_dbLineT + PI / 2) + cx);
		pt2.y = (int)((x - cx) * sin(m_dbLineT + PI / 2) + (y - cy) * cos(m_dbLineT + PI / 2) + cy);

		cvLine(pImg, pt1, pt2, cvScalar(255, 128, 255));

		// Arrow
		pt3.x = pt1.x + 10;
		pt3.y = pt1.y;

		CvPoint pt4;
		pt4.x = (int)((pt3.x - pt1.x) * cos(m_dbLineT + PI / 2) - (pt3.x - pt1.x) * sin(m_dbLineT + PI / 2) + pt1.x);
		pt4.y = (int)((pt3.x - pt1.x) * sin(m_dbLineT + PI / 2) + (pt3.x - pt1.x) * cos(m_dbLineT + PI / 2) + pt1.y);
		cvLine(pImg, pt1, pt4, cvScalar(255, 128, 255));

		pt4.x = (int)((pt3.x - pt1.x) * cos(m_dbLineT) - (pt3.x - pt1.x) * sin(m_dbLineT) + pt1.x);
		pt4.y = (int)((pt3.x - pt1.x) * sin(m_dbLineT) + (pt3.x - pt1.x) * cos(m_dbLineT) + pt1.y);
		cvLine(pImg, pt1, pt4, cvScalar(255, 128, 255));

		// Base Line
		x = cx + (dist - m_nSearchLength / 2) / 4;
		y = cy;

		pt1.x = (int)((x - cx) * cos(m_dbLineT - PI / 2) - (y - cy) * sin(m_dbLineT - PI / 2) + cx);
		pt1.y = (int)((x - cx) * sin(m_dbLineT - PI / 2) + (y - cy) * cos(m_dbLineT - PI / 2) + cy);

		x = cx + (dist - m_nSearchLength / 2) * 3 / 4;
		y = cy;

		pt2.x = (int)((x - cx) * cos(m_dbLineT - PI / 2) - (y - cy) * sin(m_dbLineT - PI / 2) + cx);
		pt2.y = (int)((x - cx) * sin(m_dbLineT - PI / 2) + (y - cy) * cos(m_dbLineT - PI / 2) + cy);

		cvLine(pImg, pt1, pt2, cvScalar(255, 128, 255));

		// Arrow
		pt3.x = pt1.x + 10;
		pt3.y = pt1.y;

		pt4.x = (int)((pt3.x - pt1.x) * cos(m_dbLineT + PI) - (pt3.x - pt1.x) * sin(m_dbLineT + PI ) + pt1.x);
		pt4.y = (int)((pt3.x - pt1.x) * sin(m_dbLineT + PI) + (pt3.x - pt1.x) * cos(m_dbLineT + PI ) + pt1.y);
		cvLine(pImg, pt1, pt4, cvScalar(255, 128, 255));

		pt4.x = (int)((pt3.x - pt1.x) * cos(m_dbLineT - PI / 2) - (pt3.x - pt1.x) * sin(m_dbLineT - PI / 2) + pt1.x);
		pt4.y = (int)((pt3.x - pt1.x) * sin(m_dbLineT - PI / 2) + (pt3.x - pt1.x) * cos(m_dbLineT - PI / 2) + pt1.y);
		cvLine(pImg, pt1, pt4, cvScalar(255, 128, 255));
	}
	else
	{
		// Base Line
		x = cx + (dist - m_nSearchLength / 2) / 4;
		y = cy;

		pt1.x = (int)((x - cx) * cos(m_dbLineT + PI / 2) - (y - cy) * sin(m_dbLineT + PI / 2) + cx);
		pt1.y = (int)((x - cx) * sin(m_dbLineT + PI / 2) + (y - cy) * cos(m_dbLineT + PI / 2) + cy);

		x = cx + (dist - m_nSearchLength / 2) * 3 / 4;
		y = cy;

		pt2.x = (int)((x - cx) * cos(m_dbLineT + PI / 2) - (y - cy) * sin(m_dbLineT + PI / 2) + cx);
		pt2.y = (int)((x - cx) * sin(m_dbLineT + PI / 2) + (y - cy) * cos(m_dbLineT + PI / 2) + cy);

		cvLine(pImg, pt1, pt2, cvScalar(255, 128, 255));

		// Arrow
		CvPoint pt4;
		pt3.x = pt2.x + 10;
		pt3.y = pt2.y;

		pt4.x = (int)((pt3.x - pt2.x) * cos(m_dbLineT + PI) - (pt3.x - pt2.x) * sin(m_dbLineT + PI ) + pt2.x);
		pt4.y = (int)((pt3.x - pt2.x) * sin(m_dbLineT + PI) + (pt3.x - pt2.x) * cos(m_dbLineT + PI ) + pt2.y);
		cvLine(pImg, pt2, pt4, cvScalar(128, 255, 255), 1);

		pt4.x = (int)((pt3.x - pt2.x) * cos(m_dbLineT - PI / 2) - (pt3.x - pt2.x) * sin(m_dbLineT - PI / 2) + pt2.x);
		pt4.y = (int)((pt3.x - pt2.x) * sin(m_dbLineT - PI / 2) + (pt3.x - pt2.x) * cos(m_dbLineT - PI / 2) + pt2.y);
		cvLine(pImg, pt2, pt4, cvScalar(255, 128, 255));

		// Base Line
		x = cx + (dist - m_nSearchLength / 2) / 4;
		y = cy;

		pt1.x = (int)((x - cx) * cos(m_dbLineT - PI / 2) - (y - cy) * sin(m_dbLineT - PI / 2) + cx);
		pt1.y = (int)((x - cx) * sin(m_dbLineT - PI / 2) + (y - cy) * cos(m_dbLineT - PI / 2) + cy);

		x = cx + (dist - m_nSearchLength / 2) * 3 / 4;
		y = cy;

		pt2.x = (int)((x - cx) * cos(m_dbLineT - PI / 2) - (y - cy) * sin(m_dbLineT - PI / 2) + cx);
		pt2.y = (int)((x - cx) * sin(m_dbLineT - PI / 2) + (y - cy) * cos(m_dbLineT - PI / 2) + cy);

		cvLine(pImg, pt1, pt2, cvScalar(255, 128, 255));

		// Arrow
		pt3.x = pt2.x + 10;
		pt3.y = pt2.y;

		pt4.x = (int)((pt3.x - pt2.x) * cos(m_dbLineT + PI / 2) - (pt3.x - pt2.x) * sin(m_dbLineT + PI / 2) + pt2.x);
		pt4.y = (int)((pt3.x - pt2.x) * sin(m_dbLineT + PI / 2) + (pt3.x - pt2.x) * cos(m_dbLineT + PI / 2) + pt2.y);
		cvLine(pImg, pt2, pt4, cvScalar(128, 255, 255), 1);

		pt4.x = (int)((pt3.x - pt2.x) * cos(m_dbLineT) - (pt3.x - pt2.x) * sin(m_dbLineT) + pt2.x);
		pt4.y = (int)((pt3.x - pt2.x) * sin(m_dbLineT) + (pt3.x - pt2.x) * cos(m_dbLineT) + pt2.y);
		cvLine(pImg, pt2, pt4, cvScalar(255, 128, 255));		
	}

}

void CCaliper::calcCaliperCircle()
{
	double x = 0.0;
	double y = 0.0;
	double refX = 0.0, refY = 0.0;
	double cx, cy;
	double dx, dy;
	double A, B;
	double normalA;

	int nCaliper = m_nNumOfCalipers;
	int nProjectionL = m_nProjectionLength;

	cx = (m_ptStart.x + m_ptEnd.x) / 2.0;
	cy = (m_ptStart.y + m_ptEnd.y) / 2.0;

	dx = m_ptEnd.x - m_ptStart.x;
	dy = m_ptEnd.y - m_ptStart.y;

	double diagonal = sqrt( dx * dx + dy * dy );
	double dStep = (diagonal - nProjectionL) / (nCaliper - 1);

	// 선의 기울기 계산 및 각도 계산
	// Y = AX + B
	A = dy / dx;	// 
	B = m_ptStart.y - m_ptStart.x * A;
	m_dbLineT = atan(A);

	// 법선의 기울기 계산 및 각도 계산.
	normalA = -1.0 / A;
	m_dbLineNormalT = atan(normalA);

	CString strTemp;

	double dOrgT = m_dbLineT;

	if( m_ptStart.x > m_ptEnd.x )
		m_dbLineT = m_dbLineT + PI;

	struct _CALIPER_AREA_INFO caliperRect;
	double dStepT = (PI * 2) / nCaliper;

	x = m_nSearchLength > diagonal ? m_nSearchLength / 2: diagonal / 2; //HTK 2022-05-19
	y = cy;

	for (int i = 0; i < nCaliper; i++)
	{
		double _T = dStepT * i + m_dbLineT;
		caliperRect.rect.center.x = (float)(x * cos(_T) + cx);
		caliperRect.rect.center.y = (float)(x * sin(_T) + cy);

		caliperRect.rect.size.width = (float)m_nSearchLength;
		caliperRect.rect.size.height = (float)m_nProjectionLength;
		caliperRect.rect.angle = (float)(_T * 180.0 / PI);

		//caliperRect.use = TRUE;
		caliperRect.use = getUseCaliper(i);

		memcpy(&m_AreaInfo[i], &caliperRect, sizeof(caliperRect));
	}

	for (int k = 0; k < m_nNumOfCalipers; k++)
	{
		auto _rad1 = m_dbLineT + dStepT * k;
		auto _rad2 = _rad1 + PI / 2;

		auto _cos1 = cos(_rad1);
		auto _sin1 = sin(_rad1);
		auto _cos2 = cos(_rad2);
		auto _sin2 = sin(_rad2);

		for (int j = 0; j < m_nProjectionLength; j++)
		{
			refX = j * _cos2;
			refY = j * _sin2;

			for (int i = 0; i < m_nSearchLength; i++)
			{
				m_ptCaliper_LUT[k][j][i].x = (float)(i * _cos1 + refX);
				m_ptCaliper_LUT[k][j][i].y = (float)(i * _sin1 + refY);
			}
		}
	}
}
bool CCaliper::find_in_samples (sPoint *samples, int no_samples, sPoint *data)
{
	for (int i=0; i<no_samples; ++i) {
		if (samples[i].x == data->x && samples[i].y == data->y) {
			return true;
		}
	}
	return false;
}

void CCaliper::get_samples (sPoint *samples, int no_samples, sPoint *data, int no_data)
{
	// 데이터에서 중복되지 않게 N개의 무작위 셈플을 채취한다.
	for (int i=0; i<no_samples; ) {
		int j = rand()%no_data;
		
		if (!find_in_samples(samples, i, &data[j])) {
			samples[i] = data[j];
			++i;
		}
	};
}

int CCaliper::compute_model_parameter(sPoint samples[], int no_samples, sLine &model)
{
	// PCA 방식으로 직선 모델의 파라메터를 예측한다.

	double sx  = 0, sy  = 0;
	double sxx = 0, syy = 0;
	double sxy = 0, sw  = 0;

	for(int i = 0; i < no_samples; ++i)
	{
		double &x = samples[i].x;
		double &y = samples[i].y;

		sx  += x;	
		sy  += y;
		sxx += x * x; 
		sxy += x * y;
		syy += y * y;
		sw  += 1;
	}

	//variance;
	double vxx = (sxx - sx * sx / sw) / sw;
	double vxy = (sxy - sx * sy / sw) / sw;
	double vyy = (syy - sy * sy / sw) / sw;
	
	//principal axis
	double theta = atan2(2 * vxy, vxx - vyy) / 2;
	
	model.mx = cos(theta);
	model.my = sin(theta);
	
	// model.my: a

	//center of mass(xc, yc)
	model.sx = sx / sw;
	model.sy = sy / sw;
	
	//직선의 방정식: sin(theta) * (x - sx) = cos(theta) * (y - sy);
	double x1 = model.sx - 1000 * model.mx;
	double y1 = model.sy - 1000 * model.my;
	double x2 = model.sx + 1000 * model.mx;
	double y2 = model.sy + 1000 * model.my;

	if( x1 == x2 )	// dx가 0인 경우 수직선으로 만들지 말고, 아주 근사한 수직선으로 바꾸자.
	{
		/*model.a = 0;
		model.b = x1;*/
		x1 += 0.000001;
		model.a = (y2 - y1) / (x2 - x1);
		model.b = y1 - (model.a * x1);
	}
	else if (y1 == y2)
	{
		y1 += 0.000001;
		model.a = (y2 - y1) / (x2 - x1);
		model.b = y1 - (model.a * x1);
	}
	else
	{
		model.a = (y2 - y1) / (x2 - x1);
		model.b = y1 - (model.a * x1);
	}

	return 1;
}

double CCaliper::compute_distance(sLine &line, sPoint &x)
{
	// 한 점(x)로부터 직선(line)에 내린 수선의 길이(distance)를 계산한다.

	return fabs((x.x - line.sx) * line.my - (x.y - line.sy) * line.mx) / sqrt(line.mx * line.mx + line.my * line.my);
}

double CCaliper::model_verification (sPoint *inliers, int *no_inliers, sLine &estimated_model, sPoint *data, int no_data, double distance_threshold)
{
	*no_inliers = 0;

	double cost = 0.;

	for(int i=0; i<no_data; i++){
		// 직선에 내린 수선의 길이를 계산한다.
		double distance = compute_distance(estimated_model, data[i]);
	
		// 예측된 모델에서 유효한 데이터인 경우, 유효한 데이터 집합에 더한다.
		if (distance < distance_threshold) {
			cost += 1.;

			inliers[*no_inliers] = data[i];
			++(*no_inliers);
		}
	}

	return cost;
}

double CCaliper::ransac_line_fitting(sPoint *data, int no_data, sLine &model, double distance_threshold)
{
	const int no_samples = 2;

	if (no_data < no_samples) {
		return 0.;
	}

	sPoint *samples = new sPoint[no_samples];

	int no_inliers = 0;
	sPoint *inliers = new sPoint[no_data];

	sLine estimated_model;
	double max_cost = 0.;

	int max_iteration = (int)(1 + log(1. - 0.99)/log(1. - pow(0.5, no_samples)));

	for (int i = 0; i<max_iteration; i++) {
		// 1. hypothesis

		// 원본 데이터에서 임의로 N개의 셈플 데이터를 고른다.
		get_samples (samples, no_samples, data, no_data);

		// 이 데이터를 정상적인 데이터로 보고 모델 파라메터를 예측한다.
		compute_model_parameter (samples, no_samples, estimated_model);

		// 2. Verification

		// 원본 데이터가 예측된 모델에 잘 맞는지 검사한다.
		double cost = model_verification (inliers, &no_inliers, estimated_model, data, no_data, distance_threshold);

		// 만일 예측된 모델이 잘 맞는다면, 이 모델에 대한 유효한 데이터로 새로운 모델을 구한다.
		if (max_cost < cost) {
			max_cost = cost;
	
			compute_model_parameter (inliers, no_inliers, model);
		}
	}
	
	delete [] samples;
	delete [] inliers;

	return max_cost;
}

double CCaliper::mlesac_line_fitting(sPoint* data, int no_data, sLine& model, double distance_threshold)
{
	double max_cost = 0.;
	double max_x=0, max_y=0;

	if (no_data < 2) 		return 0.;

#ifdef _MLESAC_USE
	std::vector<cv::Point> pts;
	double sx=0, sy=0;	// 직선의 중심점
	double a, b, c;
	sPoint _pt[2];

	for (int i = 0; i < no_data; i++) {
		cv::Point pt = cv::Point(data[i].x, data[i].y);
		pts.push_back(pt);
		sx += pt.x;
		sy += pt.y;
		if (max_x < pt.x)  max_x = pt.x;
		if (max_y < pt.y)  max_y = pt.y;
	}

	MLSAC_LineFitting(pts, a, b, c);
	
	_pt[0].x = data[0].x;
	_pt[0].y = a * data[0].x + b;
	
	_pt[1].x = data[no_data - 1].x;
	_pt[1].y = a * data[no_data-1].x + b;	

	model.pt[0] = _pt[0];
	model.pt[1] = _pt[1];
	model.sx = sx / no_data;
	model.sy = sy / no_data;
	//HTK 2022-06-29 MLESAC 단위벡터값 계산 방식 변경(기울기를 이용한 X단위 벡터, Y단위 벡터 계산)
	model.mx = cos(atan(a));
	model.my = sin(atan(a));

	model.a = a;
	model.b = b;

	pts.clear();

#endif
	return max_cost;
}

int CCaliper::mlesac_circle_fitting(sPoint samples[], int N, double* cx, double* cy, double* rad)
{
#ifdef _MLESAC_USE
	std::vector<cv::Point> pts;
	double _cx, _cy, _rad;

	for (int i = 0; i < N; i++) {
		cv::Point pt = cv::Point(samples[i].x, samples[i].y);
		pts.push_back(pt);
	}

	MLSAC_CircleFitting(pts, _cx, _cy, _rad);
	pts.clear();
	*cx= _cx, *cy=_cy, *rad=_rad;

#endif
	return 1;
}

void CCaliper::makeLineFromCaliperInfo(IplImage *pImg)
{
	int nSize = 0;

	for(int i = 0; i < m_nNumOfCalipers; i++)
	{
		if( m_FindInfo[i].find && m_FindInfo[i].ignore != TRUE ) 
			nSize++;
	}

	if( nSize > 1 )
	{
		sLine lineInfo;
		sPoint *pLineData = new sPoint[nSize];
		int nValidCount = 0;

		for(int i = 0; i < nSize; i++)
		{
			if( m_FindInfo[i].find != TRUE || m_FindInfo[i].ignore ) continue;

			pLineData[nValidCount].x = m_FindInfo[i].x;
			pLineData[nValidCount].y = m_FindInfo[i].y;
			nValidCount++;
		}

		if( m_nFittingMethod == 0 )	compute_model_parameter(pLineData, nValidCount, lineInfo);
		else if( m_nFittingMethod == 1 )
		{
			double cost = ransac_line_fitting (pLineData, nValidCount, lineInfo, 2);
		}
#ifdef _MLESAC_USE
		else if (m_nFittingMethod == 2) // MLESAC ADD 20220118 Tkyuha
		{
			double cost = mlesac_line_fitting(pLineData, nValidCount, lineInfo, 2);			
		}
#endif
		double dx = abs(m_ptStart.x - m_ptEnd.x);
		double dy = abs(m_ptStart.y - m_ptEnd.y);
		double aspectratio = 2;

		if( dx != 0 )	aspectratio = dy / dx;


		// y = ax + b
		if( aspectratio < 1.0 )
		{
			CvPoint ptLineInfo[2];

			ptLineInfo[0].y = (int)(lineInfo.a * 0 + lineInfo.b);
			ptLineInfo[0].x = 0;

			ptLineInfo[1].y = (int)(lineInfo.a * pImg->width + lineInfo.b);
			ptLineInfo[1].x = pImg->width;
			
			cvLine( pImg, ptLineInfo[0], ptLineInfo[1], cvScalar(255, 255, 255), 3);
		}
		else
		{
			CvPoint ptLineInfo[2];

			ptLineInfo[0].x = (int)((0 - lineInfo.b) / lineInfo.a);
			ptLineInfo[0].y = 0;

			ptLineInfo[1].x = (int)((pImg->height - lineInfo.b) / lineInfo.a);
			ptLineInfo[1].y = pImg->height;
			
			cvLine( pImg, ptLineInfo[0], ptLineInfo[1], cvScalar(255, 255, 255), 3);
		}

		delete pLineData;
	}
}

void CCaliper::makeLineFromCaliperInfo(FakeDC*pDC, int width, int height, BOOL bDraw)
{
	if( pDC == NULL ) return;

	int nSize = 0;
	/*for(unsigned int i = 0; i < m_vtCaliperFindInfo.size(); i++)
	{
		if( m_vtCaliperFindInfo[i].find && m_vtCaliperFindInfo[i].ignore != TRUE ) 
			nSize++;
	}*/
	for(int i = 0; i < m_nNumOfCalipers; i++)
	{
		if( m_FindInfo[i].find && m_FindInfo[i].ignore != TRUE ) 
			nSize++;
	}

	CPen penLine(PS_SOLID, 3, RGB(128, 255, 128)), *pOldPen;
	
	if( pDC )	pOldPen = pDC->SelectObject( &penLine);

	const int valid_size = 1;
	if( nSize > valid_size )
	{
		sLine lineInfo;
		sPoint *pLineData = new sPoint[nSize];
		int nValidCount = 0;

		for(int i = 0; i < m_nNumOfCalipers; i++)
		{
		//	if( m_vtCaliperFindInfo[i].find != TRUE || m_vtCaliperFindInfo[i].ignore ) continue;
			if( m_FindInfo[i].find != TRUE || m_FindInfo[i].ignore ) continue;

			pLineData[nValidCount].x = m_FindInfo[i].x;
			pLineData[nValidCount].y = m_FindInfo[i].y;

			/*pLineData[nValidCount].x = m_vtCaliperFindInfo[i].x;
			pLineData[nValidCount].y = m_vtCaliperFindInfo[i].y;*/
			nValidCount++;
		}

		double cost = nValidCount;

		if( m_nFittingMethod == 0 )	compute_model_parameter(pLineData, nValidCount, lineInfo);
		else if( m_nFittingMethod == 1 )	
		{
			cost = ransac_line_fitting (pLineData, nValidCount, lineInfo, 2);
		}
#ifdef _MLESAC_USE
		else if (m_nFittingMethod == 2) // MLESAC ADD 20220118 Tkyuha
		{
			double cost = mlesac_line_fitting(pLineData, nValidCount, lineInfo, 2);
		}
#endif
		double dx = abs(m_ptStart.x - m_ptEnd.x);
		double dy = abs(m_ptStart.y - m_ptEnd.y);
		double aspectratio = 2;

		if( dx != 0 )	aspectratio = dy / dx;


		// y = ax + b
		CvPoint ptLineInfo[2];
		if( aspectratio < 1.0 )
		{
			ptLineInfo[0].y = (int)(lineInfo.a * 0 + lineInfo.b);
			ptLineInfo[0].x = 0;

			ptLineInfo[1].y = (int)(lineInfo.a * width + lineInfo.b);
			ptLineInfo[1].x = width;
		}
		else
		{
			ptLineInfo[0].x = (int)((0 - lineInfo.b) / lineInfo.a);
			ptLineInfo[0].y = 0;

			ptLineInfo[1].x = (int)((height - lineInfo.b) / lineInfo.a);
			ptLineInfo[1].y = height;
		}

		if( bDraw )
		{
			if (pDC)	pDC->MoveTo(ptLineInfo[0].x, ptLineInfo[0].y);
			if (pDC)	pDC->LineTo(ptLineInfo[1].x, ptLineInfo[1].y);
		}

		lineInfo.pt[0].x = ptLineInfo[0].x;
		lineInfo.pt[0].y = ptLineInfo[0].y;
		lineInfo.pt[1].x = ptLineInfo[1].x;
		lineInfo.pt[1].y = ptLineInfo[1].y;

		delete pLineData;
		memcpy( &m_lineInfo, &lineInfo, sizeof(m_lineInfo));
		if(cost > valid_size)	setIsMakeLine(TRUE);
	}

	if (pDC)	pDC->SelectObject( pOldPen );
}

void CCaliper::processCaliper(IplImage *pImg)
{
	SYSTEMTIME stTime, endTime;
	::GetLocalTime(&stTime);

	setIsMakeLine(FALSE);

	if( m_dbLineX != NULL )
	{
		for(int i = 0; i < m_nLastNumOfCaliper; i++)
		{
			delete m_dbLineX[i];
			delete m_dbLineY[i];
			delete m_dbContrast[i];
		}
		delete []m_dbLineX;
		delete []m_dbLineY;
		delete []m_dbContrast;

		m_dbLineX = NULL;
		m_dbLineY = NULL;
		m_dbContrast = NULL;
	}

	m_dbLineX = new double *[m_nNumOfCalipers];
	m_dbLineY = new double *[m_nNumOfCalipers];
	m_dbContrast = new double *[m_nNumOfCalipers];

	for(int i = 0; i < m_nNumOfCalipers; i++)
	{
		m_dbLineX[i] = new double[m_nProjectionLength];
		m_dbLineY[i] = new double[m_nProjectionLength];
		m_dbContrast[i] = new double[m_nProjectionLength];

		for(int n = 0; n < m_nProjectionLength; n++)
		{
			m_dbLineX[i][n] = -1;
			m_dbLineY[i][n] = -1;
			m_dbContrast[i][n] = 0;
		}
	}

	m_nLastNumOfCaliper = m_nNumOfCalipers;

	searchCandidateEachCaliper((BYTE*)pImg->imageData, pImg->widthStep, pImg->height);

	decidePositionEachCaliper();

	decideDataToIgnore();

	makeLineFromCaliperInfo(pImg);
	
	/*
	// 각 Caliper의 후보군들 
	for(int i = 0; i < m_pMain->m_nNumOfCaliper; i++)
	{
		for(int n = 0; n < m_nProjectionLength; n++)
		{
			if( m_dbLineX[i][n] != -1 )
			{
				cvCircle(m_pMain->m_pDstImage, cvPoint(m_dbLineX[i][n], m_dbLineY[i][n]), 3, cvScalar(64, 64, 64), 1);
			}
		}
	}
	*/

	// 각 Caliper의 대표 좌표
	for(int i = 0; i < m_nNumOfCalipers; i++)
	{
		if( m_FindInfo[i].find )
		{
			cvCircle(pImg, cvPoint((int)m_FindInfo[i].x, (int)m_FindInfo[i].y), 5, cvScalar(64, 64, 64), 2);
			//cvCircle(m_pMain->m_pDstImage, cvPoint((int)m_vtCaliperFindInfo[i].x, (int)m_vtCaliperFindInfo[i].y), 5, cvScalar(255,255,255), 2);
		}
	}

	cv::Point2f pts[4];

	CvPoint pt1, pt2;
	for(int i = 0; i < m_nNumOfCalipers; i++)
	{
		//if( m_vtCaliperFindInfo[i].ignore )
		if( m_FindInfo[i].ignore )
		{
			m_AreaInfo[i].rect.points(pts);

			for(int n = 0; n < 4; n++)
			{
				pt1.x = (int)(pts[(n + 0) % 4].x);
				pt1.y = (int)(pts[(n + 0) % 4].y);

				pt2.x = (int)(pts[(n + 1) % 4].x);
				pt2.y = (int)(pts[(n + 1) % 4].y);


				cvLine(pImg, pt1, pt2, cvScalar(128, 128, 128), 1);
			}
		}
	}

	::GetLocalTime(&endTime);
	{
		CString strTmep;

		int diffMin = endTime.wMinute - stTime.wMinute;
		int diffSec = endTime.wSecond - stTime.wSecond;
		int diffMilli = endTime.wMilliseconds - stTime.wMilliseconds;

		int nFinalMinutes = 0;
		int nFinalSeconds = 0;
		int nFinalMilliSeconds = 0;

		// 현실적으로 분 단위까지만 따지자...
		if( diffMilli < 0 )	nFinalMilliSeconds = endTime.wMilliseconds + (1000 - stTime.wMilliseconds);
		else				nFinalMilliSeconds = diffMilli;

		if( diffSec < 0 )	nFinalSeconds = endTime.wSecond + (60 - stTime.wSecond);
		else				nFinalSeconds = diffSec;

		if( diffMin < 0 )	nFinalMinutes = endTime.wMinute + (60 - stTime.wMinute);
		else				nFinalMinutes = diffMin;

		if( diffMilli < 0 )	nFinalSeconds--;
		if( diffSec < 0 )	nFinalMinutes--;

		int nTotalMilliseconds = nFinalMinutes * 10000 + nFinalSeconds * 1000 + nFinalMilliSeconds;

//		strTmep.Format("%d ms / Dist: %.3f / X: %.3f Y: %.3f, aspect: %.3f", nTotalMilliseconds, dist_rect, maxX - minX, maxY - minY, aspectratio);
		strTmep.Format("%d ms", nTotalMilliseconds);
//		AfxMessageBox(strTmep);
	}

	return;
}


void CCaliper::processCaliper(BYTE *pImg, int width, int height)
{
	setIsMakeLine(FALSE);

	if( m_dbLineX != NULL )
	{
		for(int i = 0; i < m_nLastNumOfCaliper; i++)
		{
			delete m_dbLineX[i];
			delete m_dbLineY[i];
			delete m_dbContrast[i];
		}
		delete []m_dbLineX;
		delete []m_dbLineY;
		delete []m_dbContrast;

		m_dbLineX = NULL;
		m_dbLineY = NULL;
		m_dbContrast = NULL;
	}

	m_dbLineX = new double *[m_nNumOfCalipers];
	m_dbLineY = new double *[m_nNumOfCalipers];
	m_dbContrast = new double *[m_nNumOfCalipers];

	for(int i = 0; i < m_nNumOfCalipers; i++)
	{
		m_dbLineX[i] = new double[m_nProjectionLength];
		m_dbLineY[i] = new double[m_nProjectionLength];
		m_dbContrast[i] = new double[m_nProjectionLength];

		for(int n = 0; n < m_nProjectionLength; n++)
		{
			m_dbLineX[i][n] = -1;
			m_dbLineY[i][n] = -1;
			m_dbContrast[i][n] = 0;
		}
	}

	m_nLastNumOfCaliper = m_nNumOfCalipers;

	for(int i = 0; i < m_nNumOfCalipers; i++)
	{
		m_FindInfo[i].contrast = -1;
		m_FindInfo[i].x = -1;
		m_FindInfo[i].y = -1;
		m_FindInfo[i].dist = -1;
		m_FindInfo[i].find = FALSE;
		m_FindInfo[i].ignore = FALSE;
	}

	searchCandidateEachCaliper(pImg, width, height);

	decidePositionEachCaliper();

	decideDataToIgnore();

	makeLineFromCaliperInfo(m_pDC, width, height );
	
	/*
	// 각 Caliper의 후보군들 
	for(int i = 0; i < m_pMain->m_nNumOfCaliper; i++)
	{
		for(int n = 0; n < m_nProjectionLength; n++)
		{
			if( m_dbLineX[i][n] != -1 )
			{
				cvCircle(m_pMain->m_pDstImage, cvPoint(m_dbLineX[i][n], m_dbLineY[i][n]), 3, cvScalar(64, 64, 64), 1);
			}
		}
	}
	*/

	// 각 Caliper의 대표 좌표
	int r = 5;

	CPen penOK(PS_SOLID, 2, COLOR_GREEN);
	CPen penNG(PS_SOLID, 2, COLOR_RED);
	CPen *pOldPen;

	pOldPen = m_pDC->SelectObject( &penOK );
	for(int i = 0; i < m_nNumOfCalipers; i++)
	{
		int x = (int)m_FindInfo[i].x;
		int y = (int)m_FindInfo[i].y;

		if( m_FindInfo[i].find )
		{
			if( m_FindInfo[i].ignore )	m_pDC->SelectObject( &penNG );
			else						m_pDC->SelectObject( &penOK );

			m_pDC->MoveTo(x - r, y);
			m_pDC->LineTo(x + r, y);

			m_pDC->MoveTo(x, y - r);
			m_pDC->LineTo(x, y + r);
		}
	}

	m_pDC->SelectObject( pOldPen );

	cv::Point2f pts[4];

	CvPoint pt1, pt2;
	for(int i = 0; i < m_nNumOfCalipers; i++)
	{
		if( m_FindInfo[i].ignore )
		{
			m_AreaInfo[i].rect.points(pts);

			for(int n = 0; n < 4; n++)
			{
				pt1.x = (int)(pts[(n + 0) % 4].x);
				pt1.y = (int)(pts[(n + 0) % 4].y);

				pt2.x = (int)(pts[(n + 1) % 4].x);
				pt2.y = (int)(pts[(n + 1) % 4].y);

				m_pDC->MoveTo(pt1.x, pt1.y);
				m_pDC->LineTo(pt2.x, pt2.y);
			//	cvLine(pImg, pt1, pt2, cvScalar(128, 128, 128), 1);
			}
		}
	}

	return;
}


void CCaliper::processCaliper_circle(BYTE *pImg, int width, int height, double offsetX, double offsetY, double offsetT, BOOL bDraw)
{
	setIsMakeLine(FALSE);
	reset_caliper_memory();
	reset_find_info();

	setFixtureX(offsetX);
	setFixtureY(offsetY);
	setFixtureT(offsetT);

	calcPointsByFixture();
	calcCaliperCircle();

	BOOL write_image = FALSE;
	if (write_image)
	{
		cv::Mat img(height, width, CV_8UC1, pImg);
		cv::imwrite("D:\\calipers.jpg", img);

	}

	search_candidate_each_caliper_circle(pImg, width, height);

	decide_position_each_caliper_circle();

	decide_data_to_ignore_circle();

	make_circle_from_caliper_info(m_pDC, width, height, bDraw);

	// 각 Caliper의 대표 좌표
	if (bDraw)	draw_final_result(m_pDC);

	return;
}

void CCaliper::processCaliper(BYTE *pImg, int width, int height, double offsetX, double offsetY, double offsetT, BOOL bDraw)
{
	setIsMakeLine(FALSE);
	reset_caliper_memory();
	reset_find_info();


	setFixtureX(offsetX);
	setFixtureY(offsetY);
	setFixtureT(offsetT);

	calcPointsByFixture();
	calcCaliperRect();

	BOOL write_image = FALSE;
	if (write_image)
	{
		cv::Mat img(height, width, CV_8UC1, pImg);
		cv::imwrite("D:\\calipers.jpg", img);

	}

	if( m_dbLineX != NULL )
	{
		for(int i = 0; i < m_nLastNumOfCaliper; i++)
		{
			delete m_dbLineX[i];
			delete m_dbLineY[i];
			delete m_dbContrast[i];
		}
		delete []m_dbLineX;
		delete []m_dbLineY;
		delete []m_dbContrast;

		m_dbLineX = NULL;
		m_dbLineY = NULL;
		m_dbContrast = NULL;
	}

	m_dbLineX = new double *[m_nNumOfCalipers];
	m_dbLineY = new double *[m_nNumOfCalipers];
	m_dbContrast = new double *[m_nNumOfCalipers];

	for(int i = 0; i < m_nNumOfCalipers; i++)
	{
		m_dbLineX[i] = new double[m_nProjectionLength];
		m_dbLineY[i] = new double[m_nProjectionLength];
		m_dbContrast[i] = new double[m_nProjectionLength];

		for(int n = 0; n < m_nProjectionLength; n++)
		{
			m_dbLineX[i][n] = -1;
			m_dbLineY[i][n] = -1;
			m_dbContrast[i][n] = 0;
		}
	}

	m_nLastNumOfCaliper = m_nNumOfCalipers;

	for(int i = 0; i < m_nNumOfCalipers; i++)
	{
		m_FindInfo[i].contrast = -1;
		m_FindInfo[i].x = -1;
		m_FindInfo[i].y = -1;
		m_FindInfo[i].dist = -1;
		m_FindInfo[i].find = FALSE;
		m_FindInfo[i].ignore = FALSE;
	}

	searchCandidateEachCaliper(pImg, width, height);

	decidePositionEachCaliper();

	decideDataToIgnore();

	makeLineFromCaliperInfo(m_pDC, width, height, bDraw);
	
	/*
	// 각 Caliper의 후보군들 
	for(int i = 0; i < m_pMain->m_nNumOfCaliper; i++)
	{
		for(int n = 0; n < m_nProjectionLength; n++)
		{
			if( m_dbLineX[i][n] != -1 )
			{
				cvCircle(m_pMain->m_pDstImage, cvPoint(m_dbLineX[i][n], m_dbLineY[i][n]), 3, cvScalar(64, 64, 64), 1);
			}
		}
	}
	*/

	// 각 Caliper의 대표 좌표
	if( bDraw )	draw_final_result(m_pDC);

	return;
}

void CCaliper::decideDataToIgnore()
{
	int nFindCount = 0;
	
	for(int i = 0; i < m_nNumOfCalipers; i++)
	{
		if( m_FindInfo[i].find )
			nFindCount++;
	}

	vector<double> vtDistance;
	vector<int> vtFindIndex;

	double temp = 0.0;
	sPoint * pPoint = new sPoint[nFindCount];
	int nIndex = 0;
	
	for(int i = 0; i < m_nNumOfCalipers; i++)
	{
		if( m_FindInfo[i].find )
		{
			vtFindIndex.push_back(i);
			pPoint[nIndex].x = m_FindInfo[i].x;
			pPoint[nIndex].y = m_FindInfo[i].y;
			nIndex++;
		}
	}

	sLine tempLine;

	if (m_nSearchType == SEARCH_TYPE_PEAK_POS)
	{
		sPoint pts[2];
		pts[0].x = m_ptStart.x;
		pts[0].y = m_ptStart.y;
		pts[1].x = m_ptEnd.x;
		pts[1].y = m_ptEnd.y;

		compute_model_parameter(pts, 2, tempLine);
	}
	else
	{
		compute_model_parameter(pPoint, nIndex, tempLine);
	}

	for(int i = 0; i < nIndex; i++)
		vtDistance.push_back( compute_distance(tempLine, pPoint[i]) );

	/// <summary>
	///Tkyuha 2021-12-24 분포가 큰 경우 Ignore 기능 보완
	/// </summary>
	if (nIndex > 2)
	{
		vector<double> vtAvgDistance; // Tkyuha 20211224 분포가 큰 경우 Ignore 기능 보완
		vector<double> _vtAvgDistance; // Tkyuha 20211224 분포가 큰 경우 Ignore 기능 보완 임시변수
		double dx = abs(m_ptStart.x - m_ptEnd.x);
		double dy = abs(m_ptStart.y - m_ptEnd.y);
		double aspectratio = 2;
		if (dx != 0)	aspectratio = dy / dx;

		for (int i = 1; i < nIndex; i++)
		{
			cv::Point2f p1 = cv::Point2f(pPoint[i - 1].x, pPoint[i - 1].y);
			cv::Point2f p2 = cv::Point2f(pPoint[i].x, pPoint[i].y);

			if (aspectratio < 1.0)	p2.x = p1.x;
			else p2.y = p1.y;

			double dist = std::sqrt(double((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y)));
			vtAvgDistance.push_back(dist);
			_vtAvgDistance.push_back(dist);
		}

		std::sort(vtAvgDistance.begin(), vtAvgDistance.end());
		double avgdistance = vtAvgDistance[int(vtAvgDistance.size()*0.25)];  //1사분위수
		int margin = 3;

		for (int i = 0; i < nIndex - 1; i++)
		{
			if (_vtAvgDistance[i] < (avgdistance - margin) || _vtAvgDistance[i] > (avgdistance + margin))
				vtDistance[i] = 9999;
		}
		vtAvgDistance.clear(); // Tkyuha 20211224 분포가 큰 경우 Ignore 기능 보완
		_vtAvgDistance.clear(); // Tkyuha 20211224 분포가 큰 경우 Ignore 기능 보완
	}
	/// <summary>

	for(int i = 0; i < nIndex - 1; i++)
	{
		for(int j = i + 1; j < nIndex; j++)
		{
			if( vtDistance[i] < vtDistance[j] )
			{
				temp = vtDistance[i];
				vtDistance[i] = vtDistance[j];
				vtDistance[j] = temp;

				temp = vtFindIndex[i];
				vtFindIndex[i] = vtFindIndex[j];
				vtFindIndex[j] = temp;
			}
		}
	}

	for(int i = 0; i < nIndex; i++)
	{
		m_FindInfo[vtFindIndex[i]].dist = vtDistance[i];
	}

	// 찾은 갯수가 m_nNumOfIgnore + 2 보다 작은 경우 제일 가까운 2개로 하자
	if (nIndex < (m_nNumOfIgnore + 2) && vtDistance.size() >= 2 )
	{
		for(int i = 0; i < vtFindIndex.size() - 2; i++)
			m_FindInfo[vtFindIndex[i]].ignore = TRUE;
	}
	else
	{
		for (int i = 0; i < m_nNumOfIgnore; i++)
		{
			if (i >= nIndex) continue;
			m_FindInfo[vtFindIndex[i]].ignore = TRUE;
		}

		//Tkyuha 2021-12-24 분포가 큰 경우 Ignore 기능 보완
		for (int i = 0; i < nIndex; i++)
		{
			if(vtDistance[i] ==9999)	m_FindInfo[vtFindIndex[i]].ignore = TRUE;
		}
	}	

	vtDistance.clear();
	vtFindIndex.clear();

	delete []pPoint;
}

void CCaliper::decidePositionEachCaliper()
{
	double totalX = 0;
	double totalY = 0;
	int nCount = 0;
	double totalContrast = 0;
	double dx = abs(m_ptStart.x - m_ptEnd.x);
	double dy = abs(m_ptStart.y - m_ptEnd.y);
	double aspectratio = 2;
	
	if( dx != 0 )	aspectratio = dy / dx;

	
	struct _CALIPER_FIND_INFO caliperInfo;
	cv::Point2f pts[4];

	for(int i = 0; i < m_nNumOfCalipers; i++)
	{
		totalX = 0;
		totalY = 0;
		nCount = 0;
		totalContrast = 0;

		caliperInfo.contrast = -1;
		caliperInfo.dist = -1;
		caliperInfo.find= FALSE;
		caliperInfo.ignore = FALSE;
		caliperInfo.x = -1;
		caliperInfo.y = -1;

		if( m_nEachCaliperSearchType == 0 )			// Average
		{
			for(int n = 0; n < m_nProjectionLength; n++)
			{
				if( m_dbLineX[i][n] != -1 )
				{
					totalX += m_dbLineX[i][n];
					totalY += m_dbLineY[i][n];
					totalContrast += m_dbContrast[i][n];
					nCount++;
				}
			}

			if( nCount > m_nProjectionLength / 2 )
			{
				caliperInfo.find = TRUE;
				caliperInfo.ignore = FALSE;
				caliperInfo.x = totalX / nCount;
				caliperInfo.y = totalY / nCount;
				caliperInfo.contrast = (int)totalContrast / nCount;
			}
			else
			{
				caliperInfo.find = FALSE;
				caliperInfo.ignore = FALSE;
				caliperInfo.x = 0;
				caliperInfo.y = 0;
				caliperInfo.contrast = 0;
			}
		}
		else if( m_nEachCaliperSearchType == 1 )	// Median
		{
			vector<double> posX;
			vector<double> posY;
			vector<double> contrast;
			double temp = 0.0;

			for(int n = 0; n < m_nProjectionLength; n++)
			{
				if( m_dbLineX[i][n] != -1 )
				{
					posX.push_back(m_dbLineX[i][n]);
					posY.push_back(m_dbLineY[i][n]);
					contrast.push_back(m_dbContrast[i][n]);
				}
			}

			if( (int)posX.size() > m_nProjectionLength / 2 )
			{
				if( aspectratio < 1.0 )		// 가로 방향	y축에 대해 Sort
				{
					for(unsigned int n = 0; n < posY.size() - 1; n++)
					{
						for(unsigned int j = n + 1; j < posY.size(); j++)
						{
							if( posY[n] < posY[j] )
							{
								temp = posY[n];
								posY[n] = posY[j];
								posY[j] = temp;

								temp = posX[n];
								posX[n] = posX[j];
								posX[j] = temp;

								temp = contrast[n];
								contrast[n] = contrast[j];
								contrast[j] = temp;
							}
						}
					}
				}
				else						// 세로 방향
				{
					for(unsigned int n = 0; n < posX.size() - 1; n++)
					{
						for(unsigned int j = n + 1; j < posX.size(); j++)
						{
							if( posX[n] < posX[j] )
							{
								temp = posX[n];
								posX[n] = posX[j];
								posX[j] = temp;

								temp = posY[n];
								posY[n] = posY[j];
								posY[j] = temp;

								temp = contrast[n];
								contrast[n] = contrast[j];
								contrast[j] = temp;
							}
						}
					}
				}

				caliperInfo.find = TRUE;
				caliperInfo.ignore = FALSE;
				caliperInfo.x = posX[posX.size() / 2];
				caliperInfo.y = posY[posX.size() / 2];
				caliperInfo.contrast = (int)contrast[posX.size() / 2];
			}
			else
			{
				caliperInfo.find = FALSE;
				caliperInfo.ignore = FALSE;
				caliperInfo.x = 0;
				caliperInfo.y = 0;
				caliperInfo.contrast = 0;
			}

			posX.clear();
			posY.clear();
			contrast.clear();
		}
		else if( m_nEachCaliperSearchType == 2 )	// Peak Cut
		{
			vector<double> posX;
			vector<double> posY;
			vector<double> contrast;
			double temp = 0.0;

			for(int n = 0; n < m_nProjectionLength; n++)
			{
				if( m_dbLineX[i][n] != -1 )
				{
					posX.push_back(m_dbLineX[i][n]);
					posY.push_back(m_dbLineY[i][n]);
					contrast.push_back(m_dbContrast[i][n]);
				}
			}

			if( (int)posX.size() > m_nProjectionLength / 2 )
			{
				if( aspectratio < 1.0 )		// 가로 방향	y축에 대해 Sort
				{
					for(unsigned int n = 0; n < posY.size() - 1; n++)
					{
						for(unsigned int j = n + 1; j < posY.size(); j++)
						{
							if( posY[n] < posY[j] )
							{
								temp = posY[n];
								posY[n] = posY[j];
								posY[j] = temp;

								temp = posX[n];
								posX[n] = posX[j];
								posX[j] = temp;

								temp = contrast[n];
								contrast[n] = contrast[j];
								contrast[j] = temp;
							}
						}
					}
				}
				else						// 세로 방향
				{
					for(unsigned int n = 0; n < posX.size() - 1; n++)
					{
						for(unsigned int j = n + 1; j < posX.size(); j++)
						{
							if( posX[n] < posX[j] )
							{
								temp = posX[n];
								posX[n] = posX[j];
								posX[j] = temp;

								temp = posY[n];
								posY[n] = posY[j];
								posY[j] = temp;

								temp = contrast[n];
								contrast[n] = contrast[j];
								contrast[j] = temp;
							}
						}
					}
				}

				int nCount = 0;
				double totalX = 0.0;
				double totalY = 0.0;
				double totalContrast = 0.0;
				
				for(unsigned int k = unsigned int(posX.size() / 4); k < unsigned int(posX.size() * 3 / 4); k++)
				{
					totalX += posX[k];
					totalY += posY[k];
					totalContrast += contrast[k];
					nCount++;
				}

				caliperInfo.find = TRUE;
				caliperInfo.ignore = FALSE;
				caliperInfo.x = totalX / nCount;
				caliperInfo.y = totalY / nCount;
				caliperInfo.contrast = (int)(totalContrast / nCount);
			}
			else
			{
				caliperInfo.find = FALSE;
				caliperInfo.ignore = FALSE;
				caliperInfo.x = 0;
				caliperInfo.y = 0;
				caliperInfo.contrast = 0;
			}

			posX.clear();
			posY.clear();
			contrast.clear();
		}

		memcpy(&m_FindInfo[i], &caliperInfo, sizeof(caliperInfo));		
	}
}

void CCaliper::decidePositionEachCaliperCircle()
{
	double totalX = 0;
	double totalY = 0;
	int nCount = 0;
	double totalContrast = 0;
	double dx = abs(m_ptStart.x - m_ptEnd.x);
	double dy = abs(m_ptStart.y - m_ptEnd.y);	
	double aspectratio = 2;
	
	if( dx != 0 )	aspectratio = dy / dx;

	double minX = 9999, maxX = 0;
	double minY = 9999, maxY = 0;
	
	struct _CALIPER_FIND_INFO caliperInfo;
	cv::Point2f pts[4];

	for(int i = 0; i < m_nNumOfCalipers; i++)
	{
		m_AreaInfo[i].rect.points(pts);

		for(int n = 0; n < 4; n++)
		{
			minX = minX < pts[n].x ? minX: pts[n].x;
			maxX = maxX > pts[n].x ? maxX: pts[n].x;
			minY = minY < pts[n].y ? minY: pts[n].y;
			maxY = maxY > pts[n].y ? maxY: pts[n].y;
		}


		double dx = abs(maxX - minX);
		double dy = abs(maxY - minY);
		double aspectratio = 2;

		if( dx != 0 )	aspectratio = dy / dx;

		totalX = 0;
		totalY = 0;
		nCount = 0;
		totalContrast = 0;

		if( m_nEachCaliperSearchType == 0 )			// Average
		{
			for(int n = 0; n < m_nProjectionLength; n++)
			{
				if( m_dbLineX[i][n] != -1 )
				{
					totalX += m_dbLineX[i][n];
					totalY += m_dbLineY[i][n];
					totalContrast += m_dbContrast[i][n];
					nCount++;
				}
			}

			if( nCount > m_nProjectionLength / 2 )
			{
				caliperInfo.find = TRUE;
				caliperInfo.ignore = FALSE;
				caliperInfo.x = totalX / nCount;
				caliperInfo.y = totalY / nCount;
				caliperInfo.contrast = (int)totalContrast / nCount;
			}
			else
			{
				caliperInfo.find = FALSE;
				caliperInfo.ignore = FALSE;
				caliperInfo.x = 0;
				caliperInfo.y = 0;
				caliperInfo.contrast = 0;
			}
		}
		else if( m_nEachCaliperSearchType == 1 )	// Median
		{
			vector<double> posX;
			vector<double> posY;
			vector<double> contrast;
			double temp = 0.0;

			for(int n = 0; n < m_nProjectionLength; n++)
			{
				if( m_dbLineX[i][n] != -1 )
				{
					posX.push_back(m_dbLineX[i][n]);
					posY.push_back(m_dbLineY[i][n]);
					contrast.push_back(m_dbContrast[i][n]);
				}
			}

			if( (int)posX.size() > m_nProjectionLength / 2 )
			{
				if( aspectratio < 1.0 )		// 가로 방향	y축에 대해 Sort
				{
					for(unsigned int n = 0; n < posY.size() - 1; n++)
					{
						for(unsigned int j = n + 1; j < posY.size(); j++)
						{
							if( posY[n] < posY[j] )
							{
								temp = posY[n];
								posY[n] = posY[j];
								posY[j] = temp;

								temp = posX[n];
								posX[n] = posX[j];
								posX[j] = temp;

								temp = contrast[n];
								contrast[n] = contrast[j];
								contrast[j] = temp;
							}
						}
					}
				}
				else						// 세로 방향
				{
					for(unsigned int n = 0; n < posX.size() - 1; n++)
					{
						for(unsigned int j = n + 1; j < posX.size(); j++)
						{
							if( posX[n] < posX[j] )
							{
								temp = posX[n];
								posX[n] = posX[j];
								posX[j] = temp;

								temp = posY[n];
								posY[n] = posY[j];
								posY[j] = temp;

								temp = contrast[n];
								contrast[n] = contrast[j];
								contrast[j] = temp;
							}
						}
					}
				}

				caliperInfo.find = TRUE;
				caliperInfo.ignore = FALSE;
				caliperInfo.x = posX[posX.size() / 2];
				caliperInfo.y = posY[posX.size() / 2];
				caliperInfo.contrast = (int)contrast[posX.size() / 2];
			}
			else
			{
				caliperInfo.find = FALSE;
				caliperInfo.ignore = FALSE;
				caliperInfo.x = 0;
				caliperInfo.y = 0;
				caliperInfo.contrast = 0;
			}

			posX.clear();
			posY.clear();
			contrast.clear();
		}
		else if( m_nEachCaliperSearchType == 2 )	// Peak Cut
		{
			vector<double> posX;
			vector<double> posY;
			vector<double> contrast;
			double temp = 0.0;

			for(int n = 0; n < m_nProjectionLength; n++)
			{
				if( m_dbLineX[i][n] != -1 )
				{
					posX.push_back(m_dbLineX[i][n]);
					posY.push_back(m_dbLineY[i][n]);
					contrast.push_back(m_dbContrast[i][n]);
				}
			}

			if( (int)posX.size() > m_nProjectionLength / 2 )
			{
				if( aspectratio < 1.0 )		// 가로 방향	y축에 대해 Sort
				{
					for(unsigned int n = 0; n < posY.size() - 1; n++)
					{
						for(unsigned int j = n + 1; j < posY.size(); j++)
						{
							if( posY[n] < posY[j] )
							{
								temp = posY[n];
								posY[n] = posY[j];
								posY[j] = temp;

								temp = posX[n];
								posX[n] = posX[j];
								posX[j] = temp;

								temp = contrast[n];
								contrast[n] = contrast[j];
								contrast[j] = temp;
							}
						}
					}
				}
				else						// 세로 방향
				{
					for(unsigned int n = 0; n < posX.size() - 1; n++)
					{
						for(unsigned int j = n + 1; j < posX.size(); j++)
						{
							if( posX[n] < posX[j] )
							{
								temp = posX[n];
								posX[n] = posX[j];
								posX[j] = temp;

								temp = posY[n];
								posY[n] = posY[j];
								posY[j] = temp;

								temp = contrast[n];
								contrast[n] = contrast[j];
								contrast[j] = temp;
							}
						}
					}
				}

				int nCount = 0;
				double totalX = 0.0;
				double totalY = 0.0;
				double totalContrast = 0.0;
				
				for(unsigned int k = unsigned int(posX.size() / 4); k < unsigned int(posX.size() * 3 / 4); k++)
				{
					totalX += posX[k];
					totalY += posY[k];
					totalContrast += contrast[k];
					nCount++;
				}

				caliperInfo.find = TRUE;
				caliperInfo.ignore = FALSE;
				caliperInfo.x = totalX / nCount;
				caliperInfo.y = totalY / nCount;
				caliperInfo.contrast = (int)(totalContrast / nCount);
			}
			else
			{
				caliperInfo.find = FALSE;
				caliperInfo.ignore = FALSE;
				caliperInfo.x = 0;
				caliperInfo.y = 0;
				caliperInfo.contrast = 0;
			}

			posX.clear();
			posY.clear();
			contrast.clear();
		}

		memcpy( &m_FindInfo[i], &caliperInfo, sizeof(caliperInfo));
	}
}

void CCaliper::calcCaliperRect()
{
	double x = 0.0;
	double y = 0.0;
	double refX = 0.0, refY = 0.0;
	double cx, cy;
	double dx, dy;
	double A, B;
	double normalA;

	int nCaliper = m_nNumOfCalipers;
	int nProjectionL = m_nProjectionLength;

	cx = m_ptStart.x;
	cy = m_ptStart.y;

	dx = m_ptEnd.x - m_ptStart.x;
	dy = m_ptEnd.y - m_ptStart.y;

	double diagonal = sqrt( dx * dx + dy * dy );
	double dStep = (diagonal - nProjectionL) / (nCaliper - 1);

	if (nCaliper == 1)	dStep = 0.0;

	// 선의 기울기 계산 및 각도 계산
	// Y = AX + B
	A = dy / dx;	// 
	B = m_ptStart.y - m_ptStart.x * A;
	m_dbLineT = atan(A);

	// 법선의 기울기 계산 및 각도 계산.
	normalA = -1.0 / A;
	m_dbLineNormalT = atan(normalA);

	CString strTemp;

	double dOrgT = m_dbLineT;

	if (m_ptStart.x > m_ptEnd.x)	m_dbLineT = m_dbLineT + PI;

	struct _CALIPER_AREA_INFO caliperRect;

	auto _sin = sin(m_dbLineT);
	auto _cos = cos(m_dbLineT);
	for (int i = 0; i < nCaliper; i++)
	{
		double x = m_ptStart.x + (nProjectionL / 2.) + dStep * i;
		double y = m_ptStart.y;

		caliperRect.rect.center.x = (float)((x - cx) * _cos - (y - cy) * _sin + cx);
		caliperRect.rect.center.y = (float)((x - cx) * _sin + (y - cy) * _cos + cy);
		caliperRect.rect.size.width = (float)m_nSearchLength;
		caliperRect.rect.size.height = (float)m_nProjectionLength;
		caliperRect.rect.angle = (float)(m_dbLineNormalT * 180.0 / PI);
		//caliperRect.use = TRUE;
		caliperRect.use = getUseCaliper(i);

		memcpy(&m_AreaInfo[i], &caliperRect, sizeof(caliperRect));
	}

	double tempT = m_dbLineNormalT;

	if (m_ptStart.y > m_ptEnd.y)
		tempT = m_dbLineNormalT + PI;

	// 모든 Caliper는 각 Caliper 시작 위치에서의 위치 관계가 같으므로, 하나의 Caliper에 대한 정보만 알고 있으면 된다.

	auto _cosTMP = cos(tempT);
	auto _sinTMP = sin(tempT);

	_cos = cos(m_dbLineT + PI);
	_sin = sin(m_dbLineT + PI);

	for (int j = 0; j < m_nProjectionLength; j++)
	{
		refX = j * _cos;
		refY = j * _sin;

		if (m_ptCaliper_LUT != NULL)
		{
			if (j == 0)
			{
				for (int i = 0; i < m_nSearchLength; i++)
				{
					m_ptCaliper_LUT[0][j][i].x = (float)(i * _cosTMP + refX);
					m_ptCaliper_LUT[0][j][i].y = (float)(i * _sinTMP + refY);
				}
			}
			else
			{
				for (int i = 0; i < m_nSearchLength; i++)
				{
					m_ptCaliper_LUT[0][j][i].x = (float)(m_ptCaliper_LUT[0][0][i].x + refX);
					m_ptCaliper_LUT[0][j][i].y = (float)(m_ptCaliper_LUT[0][0][i].y + refY);
				}
			}
		}
	}
}


void CCaliper::DrawCaliper(FakeDC*pDC)
{
	//if( pDC == NULL ) return;

	//CPen penBase(PS_SOLID, 10, RGB(192, 192, 192)), *pOldPen;
	//CPen penCaliper(PS_SOLID, 5, RGB(128, 128, 255));
	//CPen penCaliperNoUse(PS_SOLID, 5, RGB(128, 128, 255));
	//CBrush brBase(RGB(255, 255, 0)), *pOldBr;

	//pOldPen = pDC->SelectObject(&penBase);
	//pOldBr = pDC->SelectObject(&brBase);



	//// Draw Base Line 
	//int nOffset = 15;
	//CPoint pt1, pt2;
	//pt1.x = (int)m_ptStart.x;
	//pt1.y = (int)m_ptStart.y;
	//pt2.x = (int)m_ptEnd.x;
	//pt2.y = (int)m_ptEnd.y;

	//pDC->MoveTo(pt1);
	//pDC->LineTo(pt2);

	//pDC->Ellipse( pt1.x - nOffset, pt1.y - nOffset, pt1.x + nOffset, pt1.y + nOffset);
	//pDC->Ellipse( pt2.x - nOffset, pt2.y - nOffset, pt2.x + nOffset, pt2.y + nOffset);

	//cv::Point2f pts[4];

	//for(int i = 0; i < m_nNumOfCalipers; i++)			// Caliper
	//{
	//	m_AreaInfo[i].rect.points(pts);

	//	for(int n = 0; n < 4; n++)
	//	{
	//		pt1.x = (int)(pts[(n + 0) % 4].x);
	//		pt1.y = (int)(pts[(n + 0) % 4].y);

	//		pt2.x = (int)(pts[(n + 1) % 4].x);
	//		pt2.y = (int)(pts[(n + 1) % 4].y);


	//		//if( m_vtCaliperAreaInfo[i].use )
	//		if( getUseCaliper(i) )
	//		{
	//			pDC->SelectObject(&penCaliper);
	//			pDC->MoveTo(pt1);
	//			pDC->LineTo(pt2);
	//		}
	//		else
	//		{
	//			pDC->SelectObject(&penCaliperNoUse);
	//			pDC->MoveTo(pt1);
	//			pDC->LineTo(pt2);
	//		}
	//	}

	//	CPoint pt;
	//	if( m_ptStart.y < m_ptEnd.y )
	//	{
	//		pt.x = (pts[0].x + pts[1].x) / 2;
	//		pt.y = (pts[0].y + pts[1].y) / 2;
	//	}
	//	else
	//	{
	//		pt.x = (pts[2].x + pts[3].x) / 2;
	//		pt.y = (pts[2].y + pts[3].y) / 2;
	//	}

	//	int dx = 20 * sin(m_dbLineT);
	//	int dy = 20 * cos(m_dbLineT);

	//	pt.x -= dx;
	//	pt.y += dy;

	//	CString strTemp;
	//	strTemp.Format("%d", i + 1);
	//	pDC->SetTextColor(COLOR_GREEN);
	//	pDC->TextOutA(pt.x, pt.y, strTemp);
	//}


	//// Draw Arrow
	//CPen penArrow(PS_SOLID, 10, RGB(192, 128, 64));
	//pDC->SelectObject(&penArrow);

	//double cX, cY;
	//double arrowStartX, arrowStartY;
	//double arrowEndX, arrowEndY;
	//double arrowLY, arrowLX, arrowRX, arrowRY;

	//cX = (m_ptStart.x + m_ptEnd.x) / 2.0;
	//cY = (m_ptStart.y + m_ptEnd.y) / 2.0;

	//double lineT = m_dbLineNormalT;

	//// Caliper 방향 전환
	//if( m_bDirectionSwap )	lineT += PI;

	//if( m_ptStart.y <= m_ptEnd.y )	lineT = lineT + PI;
	//if( lineT > PI )			lineT -= (PI * 2);
	//if( lineT < -PI )			lineT += (PI * 2);

	//double arrowLength = 100.0;
	//double arrowT = lineT + 45.0 / 180.0 * PI;
	//arrowStartX = cX + arrowLength * cos(lineT);
	//arrowStartY = cY + arrowLength * sin(lineT);
	//arrowEndX = cX + arrowLength * cos(lineT + PI);
	//arrowEndY = cY + arrowLength * sin(lineT + PI);

	//pt1.x = (int)(cX + arrowLength * cos(lineT));
	//pt1.y = (int)(cY + arrowLength * sin(lineT));

	//pt2.x = (int)cX;
	//pt2.y = (int)cY;
	//pDC->MoveTo(pt1);
	//pDC->LineTo(pt2);

	//pt1.x = (int)(cX + arrowLength * cos(lineT + PI));
	//pt1.y = (int)(cY + arrowLength * sin(lineT + PI));
	//pDC->MoveTo(pt1);
	//pDC->LineTo(pt2);

	//// 꺽쇠 좌표 계산
	//arrowLX = arrowEndX + 50 * cos(arrowT);
	//arrowLY = arrowEndY + 50 * sin(arrowT);
	//arrowRX = arrowEndX + 50 * cos(arrowT + 270.0 / 180.0 * PI);
	//arrowRY = arrowEndY + 50 * sin(arrowT + 270.0 / 180.0 * PI);

	//pt1.x = (int)arrowLX;
	//pt1.y = (int)arrowLY;
	//pt2.x = (int)arrowEndX;
	//pt2.y = (int)arrowEndY;
	//pDC->MoveTo(pt1);
	//pDC->LineTo(pt2);

	//pt1.x = (int)arrowRX;
	//pt1.y = (int)arrowRY;
	//pDC->MoveTo(pt1);
	//pDC->LineTo(pt2);

	//pDC->SelectObject(pOldPen);
	//pDC->SelectObject(pOldBr);
}


void CCaliper::DrawCaliper(IplImage *pImg)
{
	// Draw Base Line 
	CvPoint pt1, pt2;

	pt1.x = (int)m_ptStart.x;
	pt1.y = (int)m_ptStart.y;
	pt2.x = (int)m_ptEnd.x;
	pt2.y = (int)m_ptEnd.y;

	cvLine( pImg, pt1, pt2, cvScalar(128, 128, 128) );		// Base Line

	int nOffset = 3;
	
	cvLine( pImg, cvPoint( pt1.x, pt1.y ), cvPoint(pt2.x, pt2.y), cvScalar(255, 0, 255));
	cvCircle( pImg, cvPoint( pt1.x, pt1.y ), 3, cvScalar(0, 255, 255));
	cvCircle( pImg, cvPoint(pt2.x, pt2.y), 3, cvScalar(0, 255, 255));

	double cx = (pt1.x + pt2.x) / 2.0;
	double cy = (pt1.y + pt2.y) / 2.0;
	
	CvFont font;
	font = cvFont(2, 2);
	font.thickness = 1;
	font.vscale = (float)0.2;

	cv::Point2f pts[4];
	for(int i = 0; i < m_nNumOfCalipers; i++)			// Caliper
	{
		m_AreaInfo[i].rect.points(pts);
		
		for(int n = 0; n < 4; n++)
		{
			pt1.x = (int)(pts[(n + 0) % 4].x);
			pt1.y = (int)(pts[(n + 0) % 4].y);

			pt2.x = (int)(pts[(n + 1) % 4].x);
			pt2.y = (int)(pts[(n + 1) % 4].y);

			

			if( m_AreaInfo[i].use )	cvLine(pImg, pt1, pt2, cvScalar(255, 128, 128));
			else					cvLine(pImg, pt1, pt2, cvScalar(128, 128, 255));
		}

		CvPoint pt;

		if( m_ptStart.y > m_ptEnd.y)
		{
			pt.x = pts[0].x;
			pt.y = pts[0].y - 10;
		}
		else
		{
			pt.x = pts[2].x;
			pt.y = pts[2].y - 10;
		}
		
		CString strTemp;
		strTemp.Format("%d", i + 1);
		cvPutText(pImg, strTemp, pt, &font, cvScalar(255,255,255));
	}


	// Draw Arrow
	CPen penArrow(PS_SOLID, 10, RGB(0, 255, 0));

	double cX, cY;
	double arrowStartX, arrowStartY;
	double arrowEndX, arrowEndY;
	double arrowLY, arrowLX, arrowRX, arrowRY;

	cX = (m_ptStart.x + m_ptEnd.x) / 2.0;
	cY = (m_ptStart.y + m_ptEnd.y) / 2.0;

	double lineT = m_dbLineNormalT;

	// Caliper 방향 전환
	if( m_bDirectionSwap )	lineT += PI;

	if( m_ptStart.y <= m_ptEnd.y )	lineT = lineT + PI;
	if( lineT > PI )			lineT -= (PI * 2);
	if( lineT < -PI )			lineT += (PI * 2);

	double arrowLength = 50.0;
	double arrowT = lineT + 45.0 / 180.0 * PI;
	arrowStartX = cX + arrowLength * cos(lineT);
	arrowStartY = cY + arrowLength * sin(lineT);
	arrowEndX = cX + arrowLength * cos(lineT + PI);
	arrowEndY = cY + arrowLength * sin(lineT + PI);

	pt1.x = (int)(cX + arrowLength * cos(lineT));
	pt1.y = (int)(cY + arrowLength * sin(lineT));

	pt2.x = (int)cX;
	pt2.y = (int)cY;
	cvLine(pImg, pt1, pt2, cvScalar(0, 255, 0), 3);

	pt1.x = (int)(cX + arrowLength * cos(lineT + PI));
	pt1.y = (int)(cY + arrowLength * sin(lineT + PI));
	cvLine(pImg, pt1, pt2, cvScalar(0, 255, 0), 3);


	// 꺽쇠 좌표 계산
	arrowLX = arrowEndX + 10 * cos(arrowT);
	arrowLY = arrowEndY + 10 * sin(arrowT);

	arrowRX = arrowEndX + 10 * cos(arrowT + 270.0 / 180.0 * PI);
	arrowRY = arrowEndY + 10 * sin(arrowT + 270.0 / 180.0 * PI);

	pt1.x = (int)arrowLX;
	pt1.y = (int)arrowLY;
	pt2.x = (int)arrowEndX;
	pt2.y = (int)arrowEndY;
	cvLine(pImg, pt1, pt2, cvScalar(0, 255, 0), 3);

	pt1.x = (int)arrowRX;
	pt1.y = (int)arrowRY;
	cvLine(pImg, pt1, pt2, cvScalar(0, 255, 0), 3);
}

void CCaliper::searchCandidateEachCaliper(BYTE *pImage, int width, int height)
{
	cv::Point2f pts[4];
	int searchType = m_nSearchType;
	int projectL = m_nProjectionLength;
	int nPitch = m_nHalfFilterSize;
	
	int nMax = 0;
	
	int nIndex = 0;
	sLine tempLine;
	sPoint pPoint[2];
	pPoint[0].x = m_ptStart.x;
	pPoint[0].y = m_ptStart.y;
	pPoint[1].x = m_ptEnd.x;
	pPoint[1].y = m_ptEnd.y;
	compute_model_parameter(pPoint, 2, tempLine);	// searchType == 3 계산용

	for(int i = 0; i < m_nNumOfCalipers; i++)
	{
		m_pVtLineX[i].clear();
		m_pVtLineY[i].clear();
		m_pVtLineContrast[i].clear();

		double sX = 0.0, sY = 0.0;
	//	if( m_vtCaliperAreaInfo[i].use != TRUE )  continue;
		if( getUseCaliper(i) != TRUE ) continue;
		m_AreaInfo[i].rect.points(pts);

		// rotatedRect의 동일한 기준점을 가져가기 위해.
		if( m_ptStart.y <= m_ptEnd.y )
		{
			sX = pts[0].x;
			sY = pts[0].y;
		}
		else
		{
			sX = pts[2].x;
			sY = pts[2].y;
		}
		
		int nPosX = 0, nPosY = 0;
		int nValue = 0;
		int nValue2 = 0;
		int nValue3 = 0;
		int nCount = 0;
		int nCount2 = 0;
		BOOL bFirst = FALSE;
		double minDist = 9999.0;

		for(int x = 0; x < m_nProjectionLength; x++)
		{
			bFirst = FALSE;
			nMax = 0;
			minDist = 9999.0;

			CvPoint2D32f *pCaliper = m_ptCaliper_LUT[0][x];

			if( m_bDirectionSwap )
			{
				for(int y = m_nSearchLength - nPitch - 1; y > nPitch; y--)
				{
					BOOL bFind = FALSE;
					nCount = 0;
					nCount2 = 0;
					nValue = 0;
					nValue2 = 0;
					nValue3 = 0;

					for(int k = 1; k <= nPitch; k++)
					{
						nPosX = (int)(sX + pCaliper[y + k].x);
						nPosY = (int)(sY + pCaliper[y + k].y);

						if( nPosX < 0 || nPosY < 0 || nPosX >= width || nPosY >= height )
							continue;

						nCount++;
						nValue2 += pImage[nPosY * width + nPosX];
					}

					if( nCount > 0 )	nValue2 /= nCount;

					for(int k = 1; k <= nPitch; k++)
					{
						nPosX = (int)(sX + pCaliper[y - k].x);
						nPosY = (int)(sY + pCaliper[y - k].y);

						if( nPosX < 0 || nPosY < 0 || nPosX >= width || nPosY >= height )
							continue;

						nCount2++;
						nValue3 -= pImage[nPosY * width + nPosX];
					}

					if( nCount2 > 0 )	nValue3 /= nCount2;

					nValue = nValue2 + nValue3;

					if( m_nContrastDirection == 0 )		// Dark To Light
					{
						if( nValue < -m_nThreshold )
						{
							m_pVtLineX[i].push_back(nPosX);
							m_pVtLineY[i].push_back(nPosY);
							m_pVtLineContrast[i].push_back(abs(nValue));

							if( searchType == SEARCH_TYPE_FIRST )	// First
							{
								if( bFirst != TRUE )	bFind = TRUE;
								bFirst = TRUE;
							}
							else if( searchType == SEARCH_TYPE_END )
							{
								bFind = TRUE;
							}
							else if( searchType == SEARCH_TYPE_PEAK || searchType == SEARCH_TYPE_PEAK_POS )	// Peak
							{
								if( nMax < abs(nValue) )
								{
									bFind = TRUE;
									nMax = abs(nValue);
								}
							}
							else if( searchType == SEARCH_TYPE_POS )	// Pos
							{
								pPoint[0].x = nPosX;
								pPoint[0].y = nPosY;
								double tempDist = compute_distance(tempLine, pPoint[0]);

								if( minDist > tempDist )
								{
									bFind = TRUE;
									minDist = tempDist;
								}
							}

							if( bFind )
							{
								m_dbLineX[i][x] = sX + pCaliper[y].x;
								m_dbLineY[i][x] = sY + pCaliper[y].y;
								m_dbContrast[i][x] = nValue;
							}
						}
					}
					else if( m_nContrastDirection == 1 )		// Light To Dark
					{
						if( nValue > m_nThreshold )
						{
							m_pVtLineX[i].push_back(nPosX);
							m_pVtLineY[i].push_back(nPosY);
							m_pVtLineContrast[i].push_back(abs(nValue));

							if( searchType == SEARCH_TYPE_FIRST )	// First
							{
								if( bFirst != TRUE )	bFind = TRUE;
								bFirst = TRUE;
							}
							else if( searchType == SEARCH_TYPE_END )
							{
								bFind = TRUE;
							}
							else if( searchType == SEARCH_TYPE_PEAK || searchType == SEARCH_TYPE_PEAK_POS )	// Peak
							{
								if( nMax < abs(nValue) )
								{
									bFind = TRUE;
									nMax = abs(nValue);
								}
							}
							else if( searchType == SEARCH_TYPE_POS )	// Pos
							{
								pPoint[0].x = nPosX;
								pPoint[0].y = nPosY;
								double tempDist = compute_distance(tempLine, pPoint[0]);

								if( minDist > tempDist )
								{
									bFind = TRUE;
									minDist = tempDist;
								}
							}

							if( bFind )
							{
								m_dbLineX[i][x] = sX + pCaliper[y].x;
								m_dbLineY[i][x] = sY + pCaliper[y].y;
								m_dbContrast[i][x] = nValue;
							}
						}
					}
					else if( m_nContrastDirection == 2 )		// Both
					{
						if( abs(nValue) > m_nThreshold )
						{
							m_pVtLineX[i].push_back(nPosX);
							m_pVtLineY[i].push_back(nPosY);
							m_pVtLineContrast[i].push_back(abs(nValue));

							if( searchType == SEARCH_TYPE_FIRST )	// First
							{
								if( bFirst != TRUE )	bFind = TRUE;
								bFirst = TRUE;
							}
							else if( searchType == SEARCH_TYPE_END )
							{
								bFind = TRUE;
							}
							else if( searchType == SEARCH_TYPE_PEAK || searchType == SEARCH_TYPE_PEAK_POS )	// Peak 
							{
								if( nMax < abs(nValue) )
								{
									bFind = TRUE;
									nMax = abs(nValue);
								}
							}
							else if( searchType == SEARCH_TYPE_POS )	// Pos
							{
								pPoint[0].x = nPosX;
								pPoint[0].y = nPosY;
								double tempDist = compute_distance(tempLine, pPoint[0]);

								if( minDist > tempDist )
								{
									bFind = TRUE;
									minDist = tempDist;
								}
							}

							if( bFind )
							{
								m_dbLineX[i][x] = sX + pCaliper[y].x;
								m_dbLineY[i][x] = sY + pCaliper[y].y;
								m_dbContrast[i][x] = abs(nValue);
							}
						}
					}
				}
			}
			else
			{
				for(int y = nPitch; y < m_nSearchLength - nPitch - 1; y++)
				{
					BOOL bFind = FALSE;
					nCount = 0;
					nCount2 = 0;
					nValue = 0;
					nValue2 = 0;
					nValue3 = 0;

					for(int k = 1; k <= nPitch; k++)
					{
						nPosX = (int)(sX + pCaliper[y - k].x);
						nPosY = (int)(sY + pCaliper[y - k].y);

						if( nPosX < 0 || nPosY < 0 || nPosX >= width || nPosY >= height )
							continue;

						nCount++;
						nValue2 += pImage[nPosY * width + nPosX];
					}
					if( nCount > 0 )	nValue2 /= nCount;

					for(int k = 1; k <= nPitch; k++)
					{
						nPosX = (int)(sX + pCaliper[y + k].x);
						nPosY = (int)(sY + pCaliper[y + k].y);

						if( nPosX < 0 || nPosY < 0 || nPosX >= width || nPosY >= height )
							continue;

						nCount2++;
						nValue3 -= pImage[nPosY * width + nPosX];
					}

					if( nCount2 > 0 )	nValue3 /= nCount2;

					nValue = nValue2 + nValue3;

					if( m_nContrastDirection == 0 )		// Dark To Light
					{
						if( nValue < -m_nThreshold )
						{
							m_pVtLineX[i].push_back(nPosX);
							m_pVtLineY[i].push_back(nPosY);
							m_pVtLineContrast[i].push_back(abs(nValue));

							if( searchType == SEARCH_TYPE_FIRST )	// First
							{
								if( bFirst != TRUE )	bFind = TRUE;
								bFirst = TRUE;
							}
							else if( searchType == SEARCH_TYPE_END )
							{
								bFind = TRUE;
							}
							else if( searchType == SEARCH_TYPE_PEAK || searchType == SEARCH_TYPE_PEAK_POS )	// Peak
							{
								if( nMax < abs(nValue) )
								{
									bFind = TRUE;
									nMax = abs(nValue);
								}
							}
							else if( searchType == SEARCH_TYPE_POS )	// Pos
							{
								pPoint[0].x = nPosX;
								pPoint[0].y = nPosY;
								double tempDist = compute_distance(tempLine, pPoint[0]);

								if( minDist > tempDist )
								{
									bFind = TRUE;
									minDist = tempDist;
								}
							}

							if( bFind )
							{
								m_dbLineX[i][x] = sX + pCaliper[y].x;
								m_dbLineY[i][x] = sY + pCaliper[y].y;
								m_dbContrast[i][x] = abs(nValue);
							}
						}
					}
					else if( m_nContrastDirection == 1 )		// Light To Dark
					{
						if( nValue > m_nThreshold )
						{
							m_pVtLineX[i].push_back(nPosX);
							m_pVtLineY[i].push_back(nPosY);
							m_pVtLineContrast[i].push_back(abs(nValue));

							if( searchType == SEARCH_TYPE_FIRST )	// First
							{
								if( bFirst != TRUE )	bFind = TRUE;
								bFirst = TRUE;
							}
							else if( searchType == SEARCH_TYPE_END )
							{
								bFind = TRUE;
							}
							else if( searchType == SEARCH_TYPE_PEAK || searchType == SEARCH_TYPE_PEAK_POS )	// Peak
							{
								if( nMax < abs(nValue) )
								{
									bFind = TRUE;
									nMax = abs(nValue);
								}
							}
							else if( searchType == SEARCH_TYPE_POS )	// Pos
							{
								pPoint[0].x = nPosX;
								pPoint[0].y = nPosY;
								double tempDist = compute_distance(tempLine, pPoint[0]);

								if( minDist > tempDist )
								{
									bFind = TRUE;
									minDist = tempDist;
								}
							}

							if( bFind )
							{
								m_dbLineX[i][x] = sX + pCaliper[y].x;
								m_dbLineY[i][x] = sY + pCaliper[y].y;
								m_dbContrast[i][x] = abs(nValue);
							}
						}
					}
					else if( m_nContrastDirection == 2 )		// Both
					{
						if( abs(nValue) > m_nThreshold )
						{
							m_pVtLineX[i].push_back(nPosX);
							m_pVtLineY[i].push_back(nPosY);
							m_pVtLineContrast[i].push_back(abs(nValue));

							if( searchType == SEARCH_TYPE_FIRST )	// First
							{
								if( bFirst != TRUE )	bFind = TRUE;
								bFirst = TRUE;
							}
							else if( searchType == SEARCH_TYPE_END )
							{
								bFind = TRUE;
							}
							else if( searchType == SEARCH_TYPE_PEAK || searchType == SEARCH_TYPE_PEAK_POS)	// Peak
							{
								if( nMax < abs(nValue) )
								{
									bFind = TRUE;
									nMax = abs(nValue);
								}
							}
							else if( searchType == SEARCH_TYPE_POS )	// Pos
							{
								pPoint[0].x = nPosX;
								pPoint[0].y = nPosY;
								double tempDist = compute_distance(tempLine, pPoint[0]);

								if( minDist > tempDist )
								{
									bFind = TRUE;
									minDist = tempDist;
								}
							}

							if( bFind )
							{
								m_dbLineX[i][x] = sX + pCaliper[y].x;
								m_dbLineY[i][x] = sY + pCaliper[y].y;
								m_dbContrast[i][x] = abs(nValue);
							}
						}
					}
				}
			}
		}
	}
	/*
	for(int i = 0; i < m_pMain->m_nNumOfCaliper; i++)
		for(int n = 0; n < m_pVtLineX[i].size(); n++)
			cvCircle(m_pMain->m_pDstImage, cvPoint(m_pVtLineX[i][n], m_pVtLineY[i][n]), 1, cvScalar(255, 255, 255));
			*/
}

void CCaliper::calcPointsByFixture()
{
	_st32fPoint pt;

	double offX = m_dbFixtureX;
	double offY = m_dbFixtureY;
	double offT = m_dbFixtureT;
	
	pt.x = (m_ptOrgStart.x + m_ptOrgEnd.x) / 2.0;
	pt.y = (m_ptOrgStart.y + m_ptOrgEnd.y) / 2.0;

	m_ptStart.x = (m_ptOrgStart.x - pt.x) * cos(offT) - (m_ptOrgStart.y - pt.y) * sin(offT) + pt.x;
	m_ptStart.y = (m_ptOrgStart.x - pt.x) * sin(offT) + (m_ptOrgStart.y - pt.y) * cos(offT) + pt.y;

	m_ptEnd.x = (m_ptOrgEnd.x - pt.x) * cos(offT) - (m_ptOrgEnd.y - pt.y) * sin(offT) + pt.x;
	m_ptEnd.y = (m_ptOrgEnd.x - pt.x) * sin(offT) + (m_ptOrgEnd.y - pt.y) * cos(offT) + pt.y;

	m_ptStart.x += offX;
	m_ptEnd.x += offX;

	m_ptStart.y += offY;
	m_ptEnd.y += offY;
}

void CCaliper::calcOrgPointsFromOffsetPoints()
{
	_st32fPoint pt;

	double offX = -m_dbFixtureX;
	double offY = -m_dbFixtureY;
	double offT = -m_dbFixtureT;

	pt.x = (m_ptStart.x + m_ptEnd.x) / 2.0;
	pt.y = (m_ptStart.y + m_ptEnd.y) / 2.0;

	m_ptOrgStart.x = (m_ptStart.x - pt.x) * cos(offT) - (m_ptStart.y - pt.y) * sin(offT) + pt.x;
	m_ptOrgStart.y = (m_ptStart.x - pt.x) * sin(offT) + (m_ptStart.y - pt.y) * cos(offT) + pt.y;

	m_ptOrgEnd.x = (m_ptEnd.x - pt.x) * cos(offT) - (m_ptEnd.y - pt.y) * sin(offT) + pt.x;
	m_ptOrgEnd.y = (m_ptEnd.x - pt.x) * sin(offT) + (m_ptEnd.y - pt.y) * cos(offT) + pt.y;

	m_ptOrgStart.x += offX;
	m_ptOrgEnd.x += offX;

	m_ptOrgStart.y += offY;
	m_ptOrgEnd.y += offY;
}

void CCaliper::cramersRules(double a, double b, double c, double d, double p, double q, double *x, double *y)
{
	// a = slope of line1
	// c = slope of line2
	// p = y intercept of line1
	// q = y intercept of line2

	if( a == c )			// 두 선이 평행인 경우
	{
		*x = -1;
		*y = -1;
		return;
	}

	if( a == c && p == q )	// 두 선이 일치하는 경우
	{
		*x = -1;
		*y = -1;
		return;
	}
	
	*x = (p * d - b * q) / (a * d - b * c);
	*y = (a * q - c * p) / (a * d - b * c);
}


void CCaliper::draw_final_result(FakeDC* pDC)
{
	for (int i = 0; i < m_nNumOfCalipers; i++)
	{
		int x = lround(m_FindInfo[i].x);
		int y = lround(m_FindInfo[i].y);

		if (m_FindInfo[i].find)
		{
			Gdiplus::Color color = (m_FindInfo[i].ignore) ? 0xffff0000: 0xff00ff00;
			pDC->AddGraphic(new GraphicPoint(CPoint(x, y), color, 3, 1));
		}
	}
}

void CCaliper::draw_final_result(cv::Mat &Img)
{
	int r = 10;

	cv::Scalar color = cv::Scalar(0, 0, 0);
	cv::Scalar colorOK = cv::Scalar(0, 255, 0);
	cv::Scalar colorNG = cv::Scalar(0, 0, 255);

	for (int i = 0; i < m_nNumOfCalipers; i++)
	{
		int x = (int)m_FindInfo[i].x;
		int y = (int)m_FindInfo[i].y;

		if (m_FindInfo[i].find)
		{
			if (m_FindInfo[i].ignore)	color = colorNG;
			else						color = colorOK;

			cv::Point pt1, pt2, pt3, pt4;

			pt1.x = x - r;
			pt1.y = y;
			pt2.x = x + r;
			pt2.y = y;

			pt3.x = x;
			pt3.y = y - r;
			pt4.x = x;
			pt4.y = y + r;

			cv::line(Img, pt1, pt2, color, 5);
			cv::line(Img, pt3, pt4, color, 5);
		}
	}
}



int CCaliper::compute_ellipse_model_parameter(sPoint samples[], int no_samples, sEllipse &model)
{
	// 타원 방정식: x^2 + axy + by^2 + cx + dy + e = 0
	dMatrix A(no_samples, 5);
	dMatrix B(no_samples, 1);

	for (int i = 0; i < no_samples; i++) {
		double &x = samples[i].x;
		double &y = samples[i].y;

		A(i, 0) = x * y;
		A(i, 1) = y * y;
		A(i, 2) = x;
		A(i, 3) = y;
		A(i, 4) = 1.;

		B(i, 0) = -x * x;
	}

	// AX=B 형태의 해를 least squares solution으로 구하기 위해
	// Moore-Penrose pseudo-inverse를 이용한다.
	dMatrix invA = !(~A*A)*~A;
	dMatrix X = invA * B;

	// c가 1보다 클 때는 c를 1이 되도록 ratio 값을 곱해준다.
	double ratio = (X(1, 0) > 1.) ? 1. / X(1, 0): 1.;

	model.a = ratio * 1.;
	model.b = ratio * X(0, 0);
	model.c = ratio * X(1, 0);
	model.d = ratio * X(2, 0);
	model.e = ratio * X(3, 0);
	model.f = ratio * X(4, 0);

	return 1;
}


double CCaliper::ransac_ellipse_fitting(sPoint *data, int no_data, sEllipse &model, double distance_threshold)
{
	const int no_samples = 5;

	if (no_data < no_samples) {
		return 0.;
	}

	sPoint *samples = new sPoint[no_samples];

	int no_inliers = 0;
	sPoint *inliers = new sPoint[no_data];

	sEllipse estimated_model;
	double max_cost = 0.;

	int max_iteration = (int)(1 + log(1. - 0.99) / log(1. - pow(0.5, no_samples)));

	for (int i = 0; i < max_iteration; i++) {
		// 1. hypothesis

		// 원본 데이터에서 임의로 N개의 셈플 데이터를 고른다.
		get_samples(samples, no_samples, data, no_data);

		// 이 데이터를 정상적인 데이터로 보고 모델 파라메터를 예측한다.
		compute_ellipse_model_parameter(samples, no_samples, estimated_model);
		if (!estimated_model.convert_std_form()) { --i; continue; }

		// 2. Verification

		// 원본 데이터가 예측된 모델에 잘 맞는지 검사한다.
		double cost = ellipse_model_verification(inliers, &no_inliers, estimated_model, data, no_data, distance_threshold);

		// 만일 예측된 모델이 잘 맞는다면, 이 모델에 대한 유효한 데이터로 새로운 모델을 구한다.
		if (max_cost < cost) {
			max_cost = cost;

			compute_ellipse_model_parameter(inliers, no_inliers, model);
			model.convert_std_form();
		}
	}

	delete[] samples;
	delete[] inliers;

	return max_cost;
}


double CCaliper::compute_ellipse_distance(sEllipse &ellipse, sPoint &p)
{
	// 한 점 p에서 타원에 내린 수선의 길이를 계산하기 힘들다.
	// 부정확하지만, 간단히 하기위하여 대수적 거리를 계산한다.
	double &x = p.x;
	double &y = p.y;

	double e = fabs(ellipse.a*x*x + ellipse.b*x*y + ellipse.c*y*y + ellipse.d*x + ellipse.e*y + ellipse.f);
	return sqrt(e);
}

double CCaliper::ellipse_model_verification(sPoint *inliers, int *no_inliers, sEllipse &estimated_model, sPoint *data, int no_data, double distance_threshold)
{
	*no_inliers = 0;

	double cost = 0.;

	for (int i = 0; i < no_data; i++) {
		// 직선에 내린 수선의 길이를 계산한다.
		double distance = compute_ellipse_distance(estimated_model, data[i]);

		// 예측된 모델에서 유효한 데이터인 경우, 유효한 데이터 집합에 더한다.
		if (distance < distance_threshold) {
			cost += 1.;

			inliers[*no_inliers] = data[i];
			++(*no_inliers);
		}
	}

	return cost;
}


void CCaliper::search_candidate_each_caliper_circle(BYTE *pImage, int width, int height)
{
	cv::Point2f pts[4];
	int searchType = m_nSearchType;
	int projectL = m_nProjectionLength;
	int nPitch = m_nHalfFilterSize;

	int nMax = 0;

	double cx = (m_ptStart.x + m_ptEnd.x) / 2.0;
	double cy = (m_ptStart.y + m_ptEnd.y) / 2.0;
	double cr = 0.0;

	{
		double dx = m_ptStart.x - m_ptEnd.x;
		double dy = m_ptStart.y - m_ptEnd.y;
		cr = sqrt(dx * dx + dy * dy) / 2.0;
	}


	for (int i = 0; i < m_nNumOfCalipers; i++)
	{
		m_pVtLineX[i].clear();
		m_pVtLineY[i].clear();
		m_pVtLineContrast[i].clear();

		double sX = 0.0, sY = 0.0;
		m_AreaInfo[i].rect.points(pts);

		if (m_AreaInfo[i].use != TRUE)  continue;

		sX = pts[1].x;
		sY = pts[1].y;

		int nPosX = 0, nPosY = 0;
		int nValue = 0;
		int nValue2 = 0;
		int nValue3 = 0;
		int nCount = 0;
		int nCount2 = 0;
		BOOL bFirst = FALSE;
		double minDist = 9999.0;

		for (int y = 0; y < m_nProjectionLength; y++)
		{
			bFirst = FALSE;
			nMax = 0;
			minDist = 9999.0;

			CvPoint2D32f *pCaliper = m_ptCaliper_LUT[i][y];
			CvPoint2D32f *pCaliperPrev = &m_ptCaliper_LUT[i][y][-1];
			CvPoint2D32f *pCaliperNext = &m_ptCaliper_LUT[i][y][1];

			if (m_bDirectionSwap)
			{
				for (int x = m_nSearchLength - nPitch - 1; x > nPitch; x--)
				{
					BOOL bFind = FALSE;
					nCount = 0;
					nCount2 = 0;
					nValue = 0;
					nValue2 = 0;
					nValue3 = 0;

					for (int k = 0; k < nPitch; k++)
					{
						nPosX = (int)(sX + pCaliperNext[x + k].x);
						nPosY = (int)(sY + pCaliperNext[x + k].y);

						if (nPosX < 0 || nPosY < 0 || nPosX >= width || nPosY >= height)
							continue;

						nCount++;
						nValue2 += pImage[nPosY * width + nPosX];
					}

					if (nCount > 0)	nValue2 /= nCount;

					for (int k = 0; k < nPitch; k++)
					{
						nPosX = (int)(sX + pCaliperPrev[x - k].x);
						nPosY = (int)(sY + pCaliperPrev[x - k].y);

						if (nPosX < 0 || nPosY < 0 || nPosX >= width || nPosY >= height)
							continue;

						nCount2++;
						nValue3 -= pImage[nPosY * width + nPosX];
					}

					if (nCount2 > 0)	nValue3 /= nCount2;

					nValue = nValue2 + nValue3;

					if (m_nContrastDirection == 0)		// Dark To Light
					{
						if (nValue < -m_nThreshold)
						{
							m_pVtLineX[i].push_back(nPosX);
							m_pVtLineY[i].push_back(nPosY);
							m_pVtLineContrast[i].push_back(abs(nValue));

							if (searchType == 0)	// First
							{
								if (bFirst != TRUE)	bFind = TRUE;
								bFirst = TRUE;
							}
							else if (searchType == 1)
							{
								bFind = TRUE;
							}
							else if (searchType == 2)	// Peak
							{
								if (nMax < abs(nValue))
								{
									bFind = TRUE;
									nMax = abs(nValue);
								}
							}
							else if (searchType == 3)	// pos
							{
								double dx = (sX + pCaliper[x].x) - cx;
								double dy = (sY + pCaliper[x].y) - cy;
								double temp_dist = sqrt(dx * dx + dy * dy);

								if (minDist > temp_dist)
								{
									bFind = TRUE;
									minDist = temp_dist;
								}
							}

							if (bFind)
							{
								m_dbLineX[i][y] = sX + pCaliper[x].x;
								m_dbLineY[i][y] = sY + pCaliper[x].y;
								m_dbContrast[i][y] = nValue;
							}
						}
					}
					else if (m_nContrastDirection == 1)		// Light To Dark
					{
						if (nValue > m_nThreshold)
						{
							m_pVtLineX[i].push_back(nPosX);
							m_pVtLineY[i].push_back(nPosY);
							m_pVtLineContrast[i].push_back(abs(nValue));

							if (searchType == 0)	// First
							{
								if (bFirst != TRUE)	bFind = TRUE;
								bFirst = TRUE;
							}
							else if (searchType == 1)
							{
								bFind = TRUE;
							}
							else if (searchType == 2)	// Peak
							{
								if (nMax < abs(nValue))
								{
									bFind = TRUE;
									nMax = abs(nValue);
								}
							}
							else if (searchType == 3)	// pos
							{
								double dx = (sX + pCaliper[x].x) - cx;
								double dy = (sY + pCaliper[x].y) - cy;
								double temp_dist = sqrt(dx * dx + dy * dy);

								if (minDist > fabs(temp_dist - cr))
								{
									bFind = TRUE;
									minDist = fabs(temp_dist - cr);
								}
							}

							if (bFind)
							{
								m_dbLineX[i][y] = sX + pCaliper[x].x;
								m_dbLineY[i][y] = sY + pCaliper[x].y;
								m_dbContrast[i][y] = nValue;
							}
						}
					}
					else if (m_nContrastDirection == 2)		// Both
					{
						if (abs(nValue) > m_nThreshold)
						{
							m_pVtLineX[i].push_back(nPosX);
							m_pVtLineY[i].push_back(nPosY);
							m_pVtLineContrast[i].push_back(abs(nValue));

							if (searchType == 0)	// First
							{
								if (bFirst != TRUE)	bFind = TRUE;
								bFirst = TRUE;
							}
							else if (searchType == 1)
							{
								bFind = TRUE;
							}
							else if (searchType == 2)	// Peak
							{
								if (nMax < abs(nValue))
								{
									bFind = TRUE;
									nMax = abs(nValue);
								}
							}
							else if (searchType == 3)	// pos
							{
								double dx = (sX + pCaliper[x].x) - cx;
								double dy = (sY + pCaliper[x].y) - cy;
								double temp_dist = sqrt(dx * dx + dy * dy);

								if (minDist > fabs(temp_dist - cr))
								{
									bFind = TRUE;
									minDist = fabs(temp_dist - cr);
								}
							}

							if (bFind)
							{
								m_dbLineX[i][y] = sX + pCaliper[x].x;
								m_dbLineY[i][y] = sY + pCaliper[x].y;
								m_dbContrast[i][y] = nValue;
							}
						}
					}
				}
			}
			else
			{
				for (int x = nPitch; x < m_nSearchLength - nPitch - 1; x++)
				{
					BOOL bFind = FALSE;
					nCount = 0;
					nCount2 = 0;
					nValue = 0;
					nValue2 = 0;
					nValue3 = 0;

					for (int k = 0; k < nPitch; k++)
					{
						nPosX = (int)(sX + pCaliperPrev[x - k].x);
						nPosY = (int)(sY + pCaliperPrev[x - k].y);

						if (nPosX < 0 || nPosY < 0 || nPosX >= width || nPosY >= height)
							continue;

						nCount++;
						nValue2 += pImage[nPosY * width + nPosX];
					}
					if (nCount > 0)	nValue2 /= nCount;

					for (int k = 0; k < nPitch; k++)
					{
						nPosX = (int)(sX + pCaliperNext[x + k].x);
						nPosY = (int)(sY + pCaliperNext[x + k].y);


						if (nPosX < 0 || nPosY < 0 || nPosX >= width || nPosY >= height)
							continue;

						nCount2++;
						nValue3 -= pImage[nPosY * width + nPosX];
					}

					if (nCount2 > 0)	nValue3 /= nCount2;

					nValue = nValue2 + nValue3;

					if (m_nContrastDirection == 0)		// Dark To Light
					{
						if (nValue < -m_nThreshold)
						{
							m_pVtLineX[i].push_back(nPosX);
							m_pVtLineY[i].push_back(nPosY);
							m_pVtLineContrast[i].push_back(abs(nValue));

							if (searchType == 0)	// First
							{
								if (bFirst != TRUE)	bFind = TRUE;
								bFirst = TRUE;
							}
							else if (searchType == 1)
							{
								bFind = TRUE;
							}
							else if (searchType == 2)	// Peak
							{
								if (nMax < abs(nValue))
								{
									bFind = TRUE;
									nMax = abs(nValue);
								}
							}
							else if (searchType == 3)	// pos
							{
								double dx = (sX + pCaliper[x].x) - cx;
								double dy = (sY + pCaliper[x].y) - cy;
								double temp_dist = sqrt(dx * dx + dy * dy);

								if (minDist > fabs(temp_dist - cr))
								{
									bFind = TRUE;
									minDist = fabs(temp_dist - cr);
								}
							}

							if (bFind)
							{
								m_dbLineX[i][y] = sX + pCaliper[x].x;
								m_dbLineY[i][y] = sY + pCaliper[x].y;
								m_dbContrast[i][y] = nValue;
							}
						}
					}
					else if (m_nContrastDirection == 1)		// Light To Dark
					{
						if (nValue > m_nThreshold)
						{
							m_pVtLineX[i].push_back(nPosX);
							m_pVtLineY[i].push_back(nPosY);
							m_pVtLineContrast[i].push_back(abs(nValue));

							if (searchType == 0)	// First
							{
								if (bFirst != TRUE)	bFind = TRUE;
								bFirst = TRUE;
							}
							else if (searchType == 1)
							{
								bFind = TRUE;
							}
							else if (searchType == 2)	// Peak
							{
								if (nMax < abs(nValue))
								{
									bFind = TRUE;
									nMax = abs(nValue);
								}
							}
							else if (searchType == 3)	// pos
							{
								double dx = (sX + pCaliper[x].x) - cx;
								double dy = (sY + pCaliper[x].y) - cy;
								double temp_dist = sqrt(dx * dx + dy * dy);

								if (minDist > fabs(temp_dist - cr))
								{
									bFind = TRUE;
									minDist = fabs(temp_dist - cr);
								}
							}

							if (bFind)
							{
								m_dbLineX[i][y] = sX + pCaliper[x].x;
								m_dbLineY[i][y] = sY + pCaliper[x].y;
								m_dbContrast[i][y] = nValue;
							}
						}
					}
					else if (m_nContrastDirection == 2)		// Both
					{
						if (abs(nValue) > m_nThreshold)
						{
							m_pVtLineX[i].push_back(nPosX);
							m_pVtLineY[i].push_back(nPosY);
							m_pVtLineContrast[i].push_back(abs(nValue));

							if (searchType == 0)	// First
							{
								if (bFirst != TRUE)	bFind = TRUE;
								bFirst = TRUE;
							}
							else if (searchType == 1)
							{
								bFind = TRUE;
							}
							else if (searchType == 2)	// Peak
							{
								if (nMax < abs(nValue))
								{
									bFind = TRUE;
									nMax = abs(nValue);
								}
							}
							else if (searchType == 3)	// pos
							{
								double dx = (sX + pCaliper[x].x) - cx;
								double dy = (sY + pCaliper[x].y) - cy;
								double temp_dist = sqrt(dx * dx + dy * dy);

								if (minDist > fabs(temp_dist - cr))
								{
									bFind = TRUE;
									minDist = fabs(temp_dist - cr);
								}
							}

							if (bFind)
							{
								m_dbLineX[i][y] = sX + pCaliper[x].x;
								m_dbLineY[i][y] = sY + pCaliper[x].y;
								m_dbContrast[i][y] = nValue;
							}
						}
					}
				}
			}
		}
	}
	/*
	for(int i = 0; i < m_pMain->m_nNumOfCaliper; i++)
		for(int n = 0; n < m_pVtLineX[i].size(); n++)
			cvCircle(m_pMain->m_pDstImage, cvPoint(m_pVtLineX[i][n], m_pVtLineY[i][n]), 1, cvScalar(255, 255, 255));*/

}


void CCaliper::decide_position_each_caliper_circle()
{
	double totalX = 0;
	double totalY = 0;
	int nCount = 0;
	double totalContrast = 0;
	double dx = abs(m_ptStart.x - m_ptEnd.x);
	double dy = abs(m_ptStart.y - m_ptEnd.y);
	double aspectratio = 2;

	if (dx != 0)	aspectratio = dy / dx;

	double minX = 9999, maxX = 0;
	double minY = 9999, maxY = 0;

	struct _CALIPER_FIND_INFO caliperInfo;
	cv::Point2f pts[4];

	for (int i = 0; i < m_nNumOfCalipers; i++)
	{
		m_AreaInfo[i].rect.points(pts);

		for (int n = 0; n < 4; n++)
		{
			minX = minX < pts[n].x ? minX: pts[n].x;
			maxX = maxX > pts[n].x ? maxX: pts[n].x;
			minY = minY < pts[n].y ? minY: pts[n].y;
			maxY = maxY > pts[n].y ? maxY: pts[n].y;
		}


		double dx = abs(maxX - minX);
		double dy = abs(maxY - minY);
		double aspectratio = 2;

		if (dx != 0)	aspectratio = dy / dx;

		totalX = 0;
		totalY = 0;
		nCount = 0;
		totalContrast = 0;

		if (m_nEachCaliperSearchType == 0)			// Average
		{
			for (int n = 0; n < m_nProjectionLength; n++)
			{
				if (m_dbLineX[i][n] != -1)
				{
					totalX += m_dbLineX[i][n];
					totalY += m_dbLineY[i][n];
					totalContrast += m_dbContrast[i][n];
					nCount++;
				}
			}

			if (nCount > m_nProjectionLength / 2)
			{
				caliperInfo.find = TRUE;
				caliperInfo.ignore = FALSE;
				caliperInfo.x = totalX / nCount;
				caliperInfo.y = totalY / nCount;
				caliperInfo.contrast = (int)totalContrast / nCount;
			}
			else
			{
				caliperInfo.find = FALSE;
				caliperInfo.ignore = FALSE;
				caliperInfo.x = 0;
				caliperInfo.y = 0;
				caliperInfo.contrast = 0;
			}
		}
		else if (m_nEachCaliperSearchType == 1)	// Median
		{
			vector<double> posX;
			vector<double> posY;
			vector<double> contrast;
			double temp = 0.0;

			for (int n = 0; n < m_nProjectionLength; n++)
			{
				if (m_dbLineX[i][n] != -1)
				{
					posX.push_back(m_dbLineX[i][n]);
					posY.push_back(m_dbLineY[i][n]);
					contrast.push_back(m_dbContrast[i][n]);
				}
			}

			if ((int)posX.size() > m_nProjectionLength / 2)
			{
				if (aspectratio < 1.0)		// 가로 방향	y축에 대해 Sort
				{
					for (unsigned int n = 0; n < posY.size() - 1; n++)
					{
						for (unsigned int j = n + 1; j < posY.size(); j++)
						{
							if (posY[n] < posY[j])
							{
								temp = posY[n];
								posY[n] = posY[j];
								posY[j] = temp;

								temp = posX[n];
								posX[n] = posX[j];
								posX[j] = temp;

								temp = contrast[n];
								contrast[n] = contrast[j];
								contrast[j] = temp;
							}
						}
					}
				}
				else						// 세로 방향
				{
					for (unsigned int n = 0; n < posX.size() - 1; n++)
					{
						for (unsigned int j = n + 1; j < posX.size(); j++)
						{
							if (posX[n] < posX[j])
							{
								temp = posX[n];
								posX[n] = posX[j];
								posX[j] = temp;

								temp = posY[n];
								posY[n] = posY[j];
								posY[j] = temp;

								temp = contrast[n];
								contrast[n] = contrast[j];
								contrast[j] = temp;
							}
						}
					}
				}

				caliperInfo.find = TRUE;
				caliperInfo.ignore = FALSE;
				caliperInfo.x = posX[posX.size() / 2];
				caliperInfo.y = posY[posX.size() / 2];
				caliperInfo.contrast = (int)contrast[posX.size() / 2];
			}
			else
			{
				caliperInfo.find = FALSE;
				caliperInfo.ignore = FALSE;
				caliperInfo.x = 0;
				caliperInfo.y = 0;
				caliperInfo.contrast = 0;
			}

			posX.clear();
			posY.clear();
			contrast.clear();
		}
		else if (m_nEachCaliperSearchType == 2)	// Peak Cut
		{
			vector<double> posX;
			vector<double> posY;
			vector<double> contrast;
			double temp = 0.0;

			for (int n = 0; n < m_nProjectionLength; n++)
			{
				if (m_dbLineX[i][n] != -1)
				{
					posX.push_back(m_dbLineX[i][n]);
					posY.push_back(m_dbLineY[i][n]);
					contrast.push_back(m_dbContrast[i][n]);
				}
			}

			if ((int)posX.size() > m_nProjectionLength / 2)
			{
				if (aspectratio < 1.0)		// 가로 방향	y축에 대해 Sort
				{
					for (unsigned int n = 0; n < posY.size() - 1; n++)
					{
						for (unsigned int j = n + 1; j < posY.size(); j++)
						{
							if (posY[n] < posY[j])
							{
								temp = posY[n];
								posY[n] = posY[j];
								posY[j] = temp;

								temp = posX[n];
								posX[n] = posX[j];
								posX[j] = temp;

								temp = contrast[n];
								contrast[n] = contrast[j];
								contrast[j] = temp;
							}
						}
					}
				}
				else						// 세로 방향
				{
					for (unsigned int n = 0; n < posX.size() - 1; n++)
					{
						for (unsigned int j = n + 1; j < posX.size(); j++)
						{
							if (posX[n] < posX[j])
							{
								temp = posX[n];
								posX[n] = posX[j];
								posX[j] = temp;

								temp = posY[n];
								posY[n] = posY[j];
								posY[j] = temp;

								temp = contrast[n];
								contrast[n] = contrast[j];
								contrast[j] = temp;
							}
						}
					}
				}

				int nCount = 0;
				double totalX = 0.0;
				double totalY = 0.0;
				double totalContrast = 0.0;

				for (unsigned int k = unsigned int(posX.size() / 4); k < unsigned int(posX.size() * 3 / 4); k++)
				{
					totalX += posX[k];
					totalY += posY[k];
					totalContrast += contrast[k];
					nCount++;
				}

				caliperInfo.find = TRUE;
				caliperInfo.ignore = FALSE;
				caliperInfo.x = totalX / nCount;
				caliperInfo.y = totalY / nCount;
				caliperInfo.contrast = (int)(totalContrast / nCount);
			}
			else
			{
				caliperInfo.find = FALSE;
				caliperInfo.ignore = FALSE;
				caliperInfo.x = 0;
				caliperInfo.y = 0;
				caliperInfo.contrast = 0;
			}

			posX.clear();
			posY.clear();
			contrast.clear();
		}

		memcpy(&m_FindInfo[i], &caliperInfo, sizeof(caliperInfo));
	}
}


void CCaliper::decide_data_to_ignore_circle()
{
	int nFindCount = 0;

	for (int i = 0; i < m_nNumOfCalipers; i++)
	{
		if (m_FindInfo[i].find)
			nFindCount++;
	}

	vector<double> vtDistance;
	vector<int> vtFindIndex;

	double temp = 0.0;
	sPoint * pPoint = new sPoint[nFindCount];
	int nIndex = 0;

	for (int i = 0; i < m_nNumOfCalipers; i++)
	{
		if (m_FindInfo[i].find)
		{
			vtFindIndex.push_back(i);
			pPoint[nIndex].x = m_FindInfo[i].x;
			pPoint[nIndex].y = m_FindInfo[i].y;
			nIndex++;
		}
	}

	sLine tempLine;

	double cx = 0.0, cy = 0.0, r = 0.0;

	if (m_nSearchType == SEARCH_TYPE_PEAK_POS)
	{
		sPoint pts[2];
		pts[0].x = m_ptStart.x;
		pts[0].y = m_ptStart.y;
		pts[1].x = m_ptEnd.x;
		pts[1].y = m_ptEnd.y;

		compute_model_parameter(pts, 2, tempLine);
	}
	else
	{
		circle_fit_ls(pPoint, nIndex, &cx, &cy, &r);
	}

	for (int i = 0; i < nIndex; i++)
	{
		double dx = cx - pPoint[i].x;
		double dy = cy - pPoint[i].y;
		double dist = sqrt(dx * dx + dy * dy);

		vtDistance.push_back(fabs(dist - r));
	}

	/// <summary>
	///  Tkyuha 20211224 분포가 큰 경우 Ignore 기능 보완
	/// </summary>
	if (nIndex > 2)
	{
		vector<double> _vtAvgDistance;

		for (int i = 0; i < nIndex; i++)
		{
			cv::Point2f p1 = cv::Point2f(pPoint[i].x, pPoint[i].y);
			cv::Point2f p2 = cv::Point2f(cx, cy );

			double dist = std::sqrt(double((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y)));
			_vtAvgDistance.push_back(dist);
		}
				
		double sum = std::accumulate(_vtAvgDistance.begin(), _vtAvgDistance.end(), 0.0);
		double mean = sum / _vtAvgDistance.size();
		std::vector<double> diff(_vtAvgDistance.size());
		std::transform(_vtAvgDistance.begin(), _vtAvgDistance.end(), diff.begin(), [mean](double x) { return x - mean; });
		double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
		double stdev = std::sqrt(sq_sum / _vtAvgDistance.size());

		int margin = MAX(5,int(stdev*0.9));

		for (int i = 0; i < nIndex; i++)
		{
			if (_vtAvgDistance[i] < (mean - margin) || _vtAvgDistance[i] > (mean + margin))
				vtDistance[i] = 9999;
		}

		_vtAvgDistance.clear();
		diff.clear();
	}
	/// <summary>

	for (int i = 0; i < nIndex - 1; i++)
	{
		for (int j = i + 1; j < nIndex; j++)
		{
			if (vtDistance[i] < vtDistance[j])
			{
				temp = vtDistance[i];
				vtDistance[i] = vtDistance[j];
				vtDistance[j] = temp;

				temp = vtFindIndex[i];
				vtFindIndex[i] = vtFindIndex[j];
				vtFindIndex[j] = temp;
			}
		}
	}

	for (int i = 0; i < nIndex; i++)
	{
		m_FindInfo[vtFindIndex[i]].dist = vtDistance[i];
	}

	// 찾은 갯수가 m_nNumOfIgnore + 2 보다 작은 경우 제일 가까운 2개로 하자
	if (nIndex < (m_nNumOfIgnore + 2) && vtDistance.size() >= 2)
	{
		for (int i = 0; i < vtFindIndex.size() - 2; i++)
			m_FindInfo[vtFindIndex[i]].ignore = TRUE;
	}
	else
	{
		for (int i = 0; i < m_nNumOfIgnore; i++)
		{
			if (i >= nIndex) continue;
			m_FindInfo[vtFindIndex[i]].ignore = TRUE;
		}

		//Tkyuha 2021-12-24 분포가 큰 경우 Ignore 기능 보완
		for (int i = 0; i < nIndex; i++)
		{
			if (vtDistance[i] == 9999)	m_FindInfo[vtFindIndex[i]].ignore = TRUE;
		}
	}


	vtDistance.clear();
	vtFindIndex.clear();

	delete[]pPoint;
}


void CCaliper::make_circle_from_caliper_info(FakeDC*pDC, int width, int height, BOOL bDraw)
{
	if (pDC == NULL) return;

	int nSize = 0;
	for (int i = 0; i < m_nNumOfCalipers; i++)
	{
		if (m_FindInfo[i].find && m_FindInfo[i].ignore != TRUE)
			nSize++;
	}

	CPen penLine(PS_SOLID, 3, RGB(128, 255, 128)), *pOldPen;

	pOldPen = pDC->SelectObject(&penLine);

	const int valid_size = 5;
	if (nSize > valid_size)
	{
		sCircle circle_info;
		sPoint *pCircleData = new sPoint[nSize];
		int nValidCount = 0;

		for (int i = 0; i < m_nNumOfCalipers; i++)
		{
			if (m_FindInfo[i].find != TRUE || m_FindInfo[i].ignore) continue;

			pCircleData[nValidCount].x = m_FindInfo[i].x;
			pCircleData[nValidCount].y = m_FindInfo[i].y;

			nValidCount++;
		}

		double cost = nValidCount;

		if (m_nFittingMethod == 0)
		{
			circle_fit_ls(pCircleData, nValidCount, &circle_info.x, &circle_info.y, &circle_info.r);
		}
		else if (m_nFittingMethod == 1)
		{
			circle_fit_ls(pCircleData, nValidCount, &circle_info.x, &circle_info.y, &circle_info.r);
			//cost = ransac_line_fitting(pLineData, nValidCount, lineInfo, 2);
		}
#ifdef _MLESAC_USE
		else if (m_nFittingMethod == 2) // MLESAC ADD 20220118 Tkyuha
		{
			mlesac_circle_fitting(pCircleData, nValidCount, &circle_info.x, &circle_info.y, &circle_info.r);
		}
#endif
		double dx = abs(m_ptStart.x - m_ptEnd.x);
		double dy = abs(m_ptStart.y - m_ptEnd.y);
		double aspectratio = 2;

		if (dx != 0)	aspectratio = dy / dx;

		if (bDraw)
		{
			CPen pen(PS_SOLID, 3, RGB(128, 255, 128)), *pOldPen;

			pOldPen = pDC->SelectObject(&pen);

			CBrush *pOldBr = (CBrush *)pDC->SelectStockObject(NULL_BRUSH);

			int old_mode = pDC->SetBkMode(TRANSPARENT);


			pDC->Ellipse(circle_info.x - circle_info.r, circle_info.y - circle_info.r, circle_info.x + circle_info.r, circle_info.y + circle_info.r);
			pDC->SetBkMode(old_mode);
			pDC->SelectObject(pOldPen);
			pDC->SelectObject(pOldBr);
		}

		delete pCircleData;
		memcpy(&m_circle_info, &circle_info, sizeof(m_circle_info));

		if (cost > valid_size)	setIsMakeLine(TRUE);
	}

	pDC->SelectObject(pOldPen);
}


int CCaliper::circle_fit_ls(sPoint samples[], int N, double* cx, double* cy, double* rad)
{
	double sx = 0, sy = 0, sxx = 0, sxy = 0, syy = 0;
	double sxxx = 0, sxxy = 0, sxyy = 0, syyy = 0;
	for (int i = 0; i < N; i++) {
		double x = samples[i].x, y = samples[i].y;
		double xx = x * x, yy = y * y;
		sx += x;        sy += y;
		sxx += xx;      sxy += x * y;       syy += yy;
		sxxx += xx * x; sxxy += xx * y;
		sxyy += x * yy; syyy += yy * y;
	};
	double A[9], b[3], sol[3];
	A[0] = sxx, A[1] = sxy, A[2] = sx,
		A[3] = sxy, A[4] = syy, A[5] = sy,
		A[6] = sx, A[7] = sy, A[8] = N,
		b[0] = -sxxx - sxyy,
		b[1] = -sxxy - syyy,
		b[2] = -sxx - syy;
	if (!slove_linear_eq3x3(A, b, sol)) return 0;
	double det = sol[0] * sol[0] + sol[1] * sol[1] - 4 * sol[2];
	if (det <= 0) return 0;
	*cx = -sol[0] / 2;
	*cy = -sol[1] / 2;    *rad = sqrt(det) / 2.;
	return 1;
}

bool CCaliper::slove_linear_eq3x3(double A[9], double bb[3], double x[3])
{
	double invA[9];
	double det = (A[0] * (A[4] * A[8] - A[5] * A[7]) - A[1] * (A[3] * A[8] - A[5] * A[6]) + A[2] * (A[3] * A[7] - A[4] * A[6]));
	if (det != 0.) {
		det = 1. / det;
		invA[0] = (A[4] * A[8] - A[5] * A[7]) * det;
		invA[1] = (A[2] * A[7] - A[1] * A[8]) * det;
		invA[2] = (A[1] * A[5] - A[2] * A[4]) * det;
		invA[3] = (A[5] * A[6] - A[3] * A[8]) * det;
		invA[4] = (A[0] * A[8] - A[2] * A[6]) * det;
		invA[5] = (A[2] * A[3] - A[0] * A[5]) * det;
		invA[6] = (A[3] * A[7] - A[4] * A[6]) * det;
		invA[7] = (A[1] * A[6] - A[0] * A[7]) * det;
		invA[8] = (A[0] * A[4] - A[1] * A[3]) * det;
		//
		x[0] = invA[0] * bb[0] + invA[1] * bb[1] + invA[2] * bb[2];
		x[1] = invA[3] * bb[0] + invA[4] * bb[1] + invA[5] * bb[2];
		x[2] = invA[6] * bb[0] + invA[7] * bb[1] + invA[8] * bb[2];
		return true;
	}
	else {
		x[0] = x[1] = x[2] = 0;
		return false;
	}
}



void CCaliper::init_caliper_info()
{
	release_caliper_memory();
	alloc_caliper_memory();
	reset_caliper_memory();
}

void CCaliper::reset_caliper_memory()
{
	for (int k = 0; k < m_nNumOfCalipers; k++)
	{
		for (int i = 0; i < m_nProjectionLength; i++)
			memset(m_ptCaliper_LUT[k][i], 0, sizeof(CvPoint2D32f) * m_nSearchLength);
	}


	for (int i = 0; i < m_nNumOfCalipers; i++)
	{
		for (int n = 0; n < m_nProjectionLength; n++)
		{
			m_dbLineX[i][n] = -1;
			m_dbLineY[i][n] = -1;
			m_dbContrast[i][n] = 0;
		}
	}
}

void CCaliper::alloc_caliper_memory()
{
	m_ptCaliper_LUT = new CvPoint2D32f **[m_nNumOfCalipers];

	for (int k = 0; k < m_nNumOfCalipers; k++)
	{
		m_ptCaliper_LUT[k] = new CvPoint2D32f *[m_nProjectionLength];

		for (int i = 0; i < m_nProjectionLength; i++)
			m_ptCaliper_LUT[k][i] = new CvPoint2D32f[m_nSearchLength];
	}


	m_dbLineX = new double *[m_nNumOfCalipers];
	m_dbLineY = new double *[m_nNumOfCalipers];
	m_dbContrast = new double *[m_nNumOfCalipers];

	for (int i = 0; i < m_nNumOfCalipers; i++)
	{
		m_dbLineX[i] = new double[m_nProjectionLength];
		m_dbLineY[i] = new double[m_nProjectionLength];
		m_dbContrast[i] = new double[m_nProjectionLength];
	}

	m_nLastNumOfCaliper = m_nNumOfCalipers;
	m_nSearchLength_OLD = m_nSearchLength;
	m_nProjectionLength_OLD = m_nProjectionLength;
	m_nNumOfCaliper_OLD = m_nNumOfCalipers;
}

void CCaliper::release_caliper_memory()
{
	// Caliper 정보에 대한 Look Up Table을 만든다.
	if (m_ptCaliper_LUT != NULL)
	{
		for (int k = 0; k < m_nNumOfCaliper_OLD; k++)
		{
			if (m_ptCaliper_LUT[k] != NULL)
			{
				for (int i = 0; i < m_nProjectionLength_OLD; i++)
					delete[]m_ptCaliper_LUT[k][i];
			}

			delete[]m_ptCaliper_LUT[k];
		}

		delete[]m_ptCaliper_LUT;
		m_ptCaliper_LUT = NULL;
	}

	if (m_dbLineX != NULL)
	{
		for (int i = 0; i < m_nLastNumOfCaliper; i++)
		{
			delete[]m_dbLineX[i];
			delete[]m_dbLineY[i];
			delete[]m_dbContrast[i];
		}
		delete[]m_dbLineX;
		delete[]m_dbLineY;
		delete[]m_dbContrast;

		m_dbLineX = NULL;
		m_dbLineY = NULL;
		m_dbContrast = NULL;
	}
}

void CCaliper::reset_find_info()
{
	for (int i = 0; i < m_nNumOfCalipers; i++)
	{
		m_FindInfo[i].contrast = -1;
		m_FindInfo[i].x = -1;
		m_FindInfo[i].y = -1;
		m_FindInfo[i].dist = -1;
		m_FindInfo[i].find = FALSE;
		m_FindInfo[i].ignore = FALSE;
	}
}

void CCaliper::copy_caliper_info(CCaliper *caliper_info)
{
	caliper_info->setOrgStartPt(getOrgStartPt());
	caliper_info->setOrgEndPt(getOrgEndPt());
	caliper_info->setOrgMidPt(getOrgMidPt());
	caliper_info->setStartPt(getStartPt());
	caliper_info->setEndPt(getEndPt());
	caliper_info->setMidPt(getMidPt());
	caliper_info->setNumOfCalipers(getNumOfCalipers());
	caliper_info->setProjectionLength(getProjectionLength());
	caliper_info->setSearchLength(getSearchLength());
	caliper_info->setSearchType(getSearchType());
	caliper_info->setFittingMethod(getFittingMethod());
	caliper_info->setNumOfIgnore(getNumOfIgnore());
	caliper_info->setDirectionSwap(getDirectionSwap());
	caliper_info->setThreshold(getThreshold());
	caliper_info->setCaliperDC(getCaliperDC());
	caliper_info->setEachCaliperSearchType(getEachCaliperSearchType());
	caliper_info->setContrastDirection(getContrastDirection());
	caliper_info->setHalfFilterSize(getHalfFilterSize());

	caliper_info->setFixtureX(getFixtureX());
	caliper_info->setFixtureY(getFixtureY());
	caliper_info->setFixtureT(getFixtureT());

	for (int i = 0; i < getNumOfCalipers(); i++)
		caliper_info->setUseCaliper(i, getUseCaliper(i));
}

