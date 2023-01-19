#include "StdAfx.h"
#include "PatternMatching.h"
#include "PanelExistInspection.h"
#include "LET_AlignClientDlg.h"
#include "opencv2\imgproc\imgproc_c.h"

CPanelExistInspection::CPanelExistInspection(void)
{
	for (int nPattern = 0; nPattern < MAX_PANEL; nPattern++)
		m_dbAvgGrayLimit[nPattern] = 35.0;

	memset(m_dbAvgBrightness, 0, sizeof(double) * MAX_PANEL);

	m_dbResolution = 0.286;
	m_nJudgeCondition = 0;
	m_bValidPosTotalJudge = TRUE;
	m_bEmptyLineDir = FALSE;
}

CPanelExistInspection::~CPanelExistInspection(void)
{

}

BOOL CPanelExistInspection::calcPanelAvgBrightness(IplImage *pSrcImage, int nPanel, CRect rectROI)
{
	BOOL bSuccess = FALSE;

	if (pSrcImage == NULL) return bSuccess;

	cvSetImageROI(pSrcImage, cvRect(rectROI.left, rectROI.top, rectROI.Width(), rectROI.Height()));
	CvScalar avg = cvAvg(pSrcImage);
	cvResetImageROI(pSrcImage);

	m_dbAvgBrightness[nPanel] = avg.val[0];

	if (m_nJudgeCondition == HIGHER_THAN_LIMIT)
		setPanelExistJudge(nPanel, m_dbAvgBrightness[nPanel] >= m_dbAvgGrayLimit[nPanel] ? TRUE : FALSE);
	else
		setPanelExistJudge(nPanel, m_dbAvgBrightness[nPanel] <= m_dbAvgGrayLimit[nPanel] ? TRUE : FALSE);

	if (theApp.m_bEnforceOkFlag) setPanelExistJudge(nPanel, TRUE);

	bSuccess = TRUE;

	return bSuccess;
}

/*
BOOL CPanelExistInspection::panelValidPosCheck(IplImage *pSrcImage)
{
	BOOL bSuccess = FALSE;
	if( pSrcImage == NULL) return bSuccess;

	setValidPosTotalJudge(TRUE);	// 올라탐 검사 결과 초기화
	//for(int nPanel = 0; nPanel < m_pMain->m_nNumPanelInTray; nPanel++)
	for(int nPanel = 0; nPanel < m_pMain->GetPanelExist().getNumPanelInTray(); nPanel++)
	{
		if( m_pMain->GetPanelExist().getExistJudge(nPanel) != TRUE )	continue;	// Glass가 있는 것에 대해서만 올라탐 검사

		BOOL bFind = m_pMain->GetPanelExist().findPanelExistPosition((BYTE *)pSrcImage->imageData, nPanel);

		double posX = 0.0;
		double posY = 0.0;

		int nRangeX = 5;
		int nRangeY = 3;
		BOOL bValidPos = TRUE;

		if( bFind )
		{
			posX = m_pMain->GetPanelExist().getFindInfo(nPanel).GetXPos();
			posY = m_pMain->GetPanelExist().getFindInfo(nPanel).GetYPos();

			// 유효 영역을 벗어나면 NG 처리
			if( posX < m_pMain->getModel().getPanelExistInfo().getPanelValidPosLeft(nPanel) - nRangeX ||
				posX > m_pMain->getModel().getPanelExistInfo().getPanelValidPosRight(nPanel) + nRangeX ||
				posY < m_pMain->getModel().getPanelExistInfo().getPanelValidPosTop(nPanel) - nRangeY ||
				posY > m_pMain->getModel().getPanelExistInfo().getPanelValidPosBottom(nPanel) + nRangeY )
			{
				m_pMain->GetPanelExist().getFindInfo(nPanel).SetFound(FIND_ERR);
				bValidPos = FALSE;
				setValidPosTotalJudge(FALSE);
			}
		}
		else	// 패턴을 찾지 못할 경우 NG 처리
		{
			bValidPos = FALSE;
			setValidPosTotalJudge(FALSE);
		}


		m_pMain->GetPanelExist().setPanelValidPosJudge(nPanel, bValidPos);
	}

	bSuccess = TRUE;
	return bSuccess;
}
*/

void CPanelExistInspection::inspectionPanelExist(IplImage *pSrcImage)
{
	// Panel 유무 감지 검사
	if (0)
	{
		for (int nPanel = 0; nPanel < m_iNumPanelInTray; nPanel++)
			calcPanelAvgBrightness(pSrcImage, nPanel, getSearchROI(nPanel));
	}

	BOOL bFind = searchUTG(pSrcImage, getSearchROI(0));

	if (bFind)
	{
		m_bPanelExistJudge[0] = 1;
	}
	else
	{
		m_bPanelExistJudge[0] = searchSeparator(pSrcImage, getSearchROI(0));
	}
}

int CPanelExistInspection::searchSeparator(IplImage *pSrcImage, CRect rectROI)
{
	int nResult = 0;
	IplImage *pImage = cvCreateImage(cvSize(pSrcImage->width, pSrcImage->height), 8, 1);
	cvCopy(pSrcImage, pImage);

	IplImage *pCropImage = cvCreateImage(cvSize(rectROI.Width(), rectROI.Height()), 8, 1);

	CvRect rtROI;
	rtROI.x = rectROI.left;
	rtROI.y = rectROI.top;
	rtROI.width = rectROI.Width();
	rtROI.height = rectROI.Height();

	cvSetImageROI(pImage, rtROI);
	cvCopy(pImage, pCropImage);

	cvSmooth(pCropImage, pCropImage, CV_GAUSSIAN, 5, 5);
	endInSearch(pCropImage, pCropImage);

	cv::Mat enhanceImg(pCropImage->height, pCropImage->width, CV_8UC1, pCropImage->imageData);
	//InspectionEnhance(&enhanceImg, 1);	// 세로 방향

	if (m_bEmptyLineDir == 0)	InspectionEnhance(&enhanceImg, 2);	// 가로 방향
	else								InspectionEnhance(&enhanceImg, 1);	// 세로 방향

	cvThreshold(pCropImage, pCropImage, 0, 255, CV_THRESH_OTSU);
	removeNoise(enhanceImg, 100);
	cvDilate(pCropImage, pCropImage, NULL, 3);
	cvErode(pCropImage, pCropImage, NULL, 3);

	CvScalar scalar = cvAvg(pCropImage);

	cvReleaseImage(&pCropImage);
	cvReleaseImage(&pImage);

	m_dbAvgBrightness[0] = scalar.val[0];

	if (scalar.val[0] < 128)	return 0;
	else						return 2;
}

int CPanelExistInspection::searchSeparator2(BYTE *pImg, CRect rectROI, int nThresh)
{
	int nResult = 0;

	int W = m_pMain->m_stCamInfo[0].w;
	int H = m_pMain->m_stCamInfo[0].h;

	IplImage *pImage = cvCreateImage(cvSize(W, H), 8, 1);
	memcpy((void*)pImage->imageData, pImg, W * H);

	IplImage *pCropImage = cvCreateImage(cvSize(rectROI.Width(), rectROI.Height()), 8, 1);

	CvRect rtROI;
	rtROI.x = rectROI.left;
	rtROI.y = rectROI.top;
	rtROI.width = rectROI.Width();
	rtROI.height = rectROI.Height();

	cvSetImageROI(pImage, rtROI);
	cvCopy(pImage, pCropImage);

	cvSmooth(pCropImage, pCropImage, CV_GAUSSIAN, 5, 5);
	//endInSearch(pCropImage, pCropImage);

	cv::Mat enhanceImg(pCropImage->height, pCropImage->width, CV_8UC1, pCropImage->imageData);
	//InspectionEnhance(&enhanceImg, 1);	// 세로 방향

	if (m_bEmptyLineDir == 0)
	{
		InspectionEnhance(&enhanceImg, 2);	// 가로 방향
		InspectionEnhance(&enhanceImg, 2);	// 가로 방향
	}
	else
	{
		InspectionEnhance(&enhanceImg, 1);	// 세로 방향
		InspectionEnhance(&enhanceImg, 1);	// 세로 방향
	}

	cvThreshold(pCropImage, pCropImage, nThresh, 255, CV_THRESH_BINARY_INV);
	removeNoise(enhanceImg, 100);

	int *histoHori = new int[enhanceImg.rows];
	memset(histoHori, 0, sizeof(int) * enhanceImg.rows);

	int *histoVert = new int[enhanceImg.cols];
	memset(histoVert, 0, sizeof(int) * enhanceImg.cols);

	for (int y = 0; y < enhanceImg.rows; y++)
	{
		for (int x = 0; x < enhanceImg.cols; x++)
		{
			if ((UCHAR)enhanceImg.data[y * enhanceImg.cols + x] == 255)
			{
				histoHori[y]++;
				histoVert[x]++;
			}
		}
	}

	int countHori = 0;
	int countVert = 0;

	for (int y = 0; y < enhanceImg.rows; y++)
	{
		if (histoHori[y] > enhanceImg.cols / 2)
		{
			y += 10;
			countHori++;
		}
	}

	for (int x = 0; x < enhanceImg.cols; x++)
	{
		if (histoVert[x] > enhanceImg.rows / 2)
		{
			x += 10;
			countVert++;
		}
	}

	delete[]histoHori;
	delete[]histoVert;

	cvReleaseImage(&pImage);
	cvReleaseImage(&pCropImage);

	if (countHori > 1 || countVert > 1)	return 0;
	else									return 2;
}

int CPanelExistInspection::searchUTG(IplImage *pSrcImage, CRect rectROI)
{
	int nResult = 0;
	IplImage *pImage = cvCreateImage(cvSize(pSrcImage->width, pSrcImage->height), 8, 1);
	cvCopy(pSrcImage, pImage);

	IplImage *pCropImage = cvCreateImage(cvSize(rectROI.Width(), rectROI.Height()), 8, 1);

	CvRect rtROI;
	rtROI.x = rectROI.left;
	rtROI.y = rectROI.top;
	rtROI.width = rectROI.Width();
	rtROI.height = rectROI.Height();

	cvSetImageROI(pImage, rtROI);
	cvCopy(pImage, pCropImage);

	cvSmooth(pCropImage, pCropImage, CV_GAUSSIAN, 5, 5);

	cv::Mat enhanceImg(pCropImage->height, pCropImage->width, CV_8UC1, pCropImage->imageData);
	InspectionEnhance(&enhanceImg, 0);


	cvMorphologyEx(pCropImage, pCropImage, NULL, NULL, CV_MOP_GRADIENT, 1);
	cvThreshold(pCropImage, pCropImage, 20, 255, CV_THRESH_BINARY);

	//cvThreshold( pCropImage, pCropImage, m_dbAvgGrayLimit[0], 255, CV_THRESH_BINARY_INV );

	BOOL bFind_Hori = FALSE;
	BOOL bFind_Vert = FALSE;

	int *pHori = new int[pCropImage->width];
	int *pVert = new int[pCropImage->height];

	memset(pHori, 0, sizeof(int) * pCropImage->width);
	memset(pVert, 0, sizeof(int) * pCropImage->height);

	int filterOff = 6;
	// 세로 선을 찾자.
	for (int x = filterOff; x < pCropImage->width - filterOff; x++)
	{
		for (int y = filterOff; y < pCropImage->height - filterOff; y++)
		{
			if ((UCHAR)pCropImage->imageData[y * pCropImage->widthStep + x] == 255)
				pHori[x]++;
		}
	}

	for (int x = 0; x < pCropImage->width; x++)
	{
		if (pHori[x] > pCropImage->width / 2)
		{
			bFind_Vert = TRUE;
			break;
		}
	}

	// 가로 선을 찾자.
	for (int y = filterOff; y < pCropImage->height - filterOff; y++)
	{
		for (int x = filterOff; x < pCropImage->width - filterOff; x++)
		{
			if ((UCHAR)pCropImage->imageData[y * pCropImage->widthStep + x] == 255)
				pVert[y]++;
		}
	}

	for (int y = 0; y < pCropImage->height; y++)
	{
		if (pVert[y] > pCropImage->height / 2)
		{
			bFind_Hori = TRUE;
			break;
		}
	}

	cvReleaseImage(&pCropImage);
	cvReleaseImage(&pImage);
	delete[]pHori;
	delete[]pVert;

	if (bFind_Hori || bFind_Vert)	return 1;

	return 0;
}


int CPanelExistInspection::searchUTG2(IplImage *pSrcImage, CRect rectROI)
{
	int nResult = 0;
	IplImage *pImage = cvCreateImage(cvSize(pSrcImage->width, pSrcImage->height), 8, 1);
	cvCopy(pSrcImage, pImage);

	IplImage *pCropImage = cvCreateImage(cvSize(rectROI.Width(), rectROI.Height()), 8, 1);

	CvRect rtROI;
	rtROI.x = rectROI.left;
	rtROI.y = rectROI.top;
	rtROI.width = rectROI.Width();
	rtROI.height = rectROI.Height();

	cvSetImageROI(pImage, rtROI);
	cvCopy(pImage, pCropImage);

	cvSmooth(pCropImage, pCropImage, CV_GAUSSIAN, 5, 5);

	cv::Mat enhanceImg(pCropImage->height, pCropImage->width, CV_8UC1, pCropImage->imageData);
	InspectionEnhance(&enhanceImg, 0);


	cvMorphologyEx(pCropImage, pCropImage, NULL, NULL, CV_MOP_GRADIENT, 1);
	cvThreshold(pCropImage, pCropImage, 20, 255, CV_THRESH_BINARY);

	//cvThreshold( pCropImage, pCropImage, m_dbAvgGrayLimit[0], 255, CV_THRESH_BINARY_INV );

	BOOL bFind_Hori = FALSE;
	BOOL bFind_Vert = FALSE;

	int *pHori = new int[pCropImage->width];
	int *pVert = new int[pCropImage->height];

	memset(pHori, 0, sizeof(int) * pCropImage->width);
	memset(pVert, 0, sizeof(int) * pCropImage->height);

	int nCountLine = 0;
	int filterOff = 6;
	// 세로 선을 찾자.
	for (int x = filterOff; x < pCropImage->width - filterOff; x++)
	{
		for (int y = filterOff; y < pCropImage->height - filterOff; y++)
		{
			if ((UCHAR)pCropImage->imageData[y * pCropImage->widthStep + x] == 255)
				pHori[x]++;
		}
	}

	for (int x = 0; x < pCropImage->width; x++)
	{
		if (pHori[x] > pCropImage->width / 2)
		{
			nCountLine++;
			bFind_Vert = TRUE;

			x += 10;
			//	break;
		}
	}

	// 가로 선을 찾자.
	for (int y = filterOff; y < pCropImage->height - filterOff; y++)
	{
		for (int x = filterOff; x < pCropImage->width - filterOff; x++)
		{
			if ((UCHAR)pCropImage->imageData[y * pCropImage->widthStep + x] == 255)
				pVert[y]++;
		}
	}

	for (int y = 0; y < pCropImage->height; y++)
	{
		if (pVert[y] > pCropImage->height / 2)
		{
			bFind_Hori = TRUE;
			break;
		}
	}

	cvReleaseImage(&pCropImage);
	cvReleaseImage(&pImage);
	delete[]pHori;
	delete[]pVert;



	if (nCountLine > 3) return 0;
	if (bFind_Hori || bFind_Vert)	return 1;

	return 0;
}

void CPanelExistInspection::endInSearch(IplImage * srcImage, IplImage * dstImage)
{
	int i;

	int histogram[256];
	int LUT[256];
	int lowthresh, highthresh;
	float scale_factor;
	//scaling factor for contrast stretch
	//intialalize thresholds
	lowthresh = 0;
	highthresh = 255;//히스토그램명도값의빈도수조사

	// 히스토그램 계산
	calcHistogram(srcImage, histogram);

	//히스토그램중가장낮은명도값계산.
	for (i = 0; i < 256; i++)
	{
		if (histogram[i])
		{
			lowthresh = i;
			break;
		}
	}

	//히스토그램중가장높은명도값계산.
	for (i = 255; i >= 0; i--)
	{
		if (histogram[i])
		{
			highthresh = i;
			break;
		}
	}

	//룩업테이블계산.
	for (i = 0; i < lowthresh; i++)
		LUT[i] = 0;

	for (i = 255; i > highthresh; i--)
		LUT[i] = 255;

	//스트레칭의강도를조절할수있는factor계산.
	scale_factor = (float)(255.0 / (highthresh - lowthresh));

	for (i = lowthresh; i <= highthresh; i++)
		LUT[i] = int(((i - lowthresh) * scale_factor));

	//룩업테이블에서계산된출력값할당.
	applyLUT(srcImage, dstImage, LUT);
}


void CPanelExistInspection::calcHistogram(IplImage *srcImage, int(&Histo)[256])
{
	int i, x, y;

	for (i = 0; i < 256; i++) Histo[i] = 0;

	for (y = 0; y < srcImage->height; y++)
	{
		for (x = 0; x < srcImage->width; x++)
		{
			Histo[(uchar)srcImage->imageData[y * srcImage->width + x]]++;
		}
	}
}

void CPanelExistInspection::applyLUT(IplImage * srcImage, IplImage * dstImage, int(&LUT)[256])
{
	for (int y = 0; y < srcImage->height; y++)
	{
		for (int x = 0; x < srcImage->width; x++)
		{
			dstImage->imageData[y * dstImage->width + x] = (char)LUT[(uchar)srcImage->imageData[y * srcImage->width + x]];
		}
	}
}



cv::Mat CPanelExistInspection::fnEnhanceImagCalc(cv::Mat *src)
{
	if (src->empty()) return *src;

	float nKernelGUS[12] = { 10, 18, 29, 43, 57, 67, 71, 67, 57, 43, 29, 18 }; //509
	float nKernelLOG[24] = { -1, -2, -4, -9, -15, -20, -22, -16, 0, 22, 42, 50, 42, 22, 0, -16, -22, -20, -15, -9, -4, -2, -1, 0 }; //32

	cv::Mat dst, dst2;
	cv::Mat KrnlGusX = cv::Mat(12, 1, CV_32FC1, nKernelGUS) / 509.f; 	cv::Mat KrnlLOGX = cv::Mat(24, 1, CV_32FC1, nKernelLOG) / 32.f;
	cv::Mat KrnlGusY = cv::Mat(1, 12, CV_32FC1, nKernelGUS) / 509.f;	cv::Mat KrnlLOGY = cv::Mat(1, 24, CV_32FC1, nKernelLOG) / 32.f;

	cv::Mat Proc1Buf = cv::Mat::zeros(src->rows, src->cols, CV_32FC1);
	cv::Mat Proc2Buf = cv::Mat::zeros(src->rows, src->cols, CV_32FC1);
	cv::Mat Proc3Buf = cv::Mat::zeros(src->rows, src->cols, CV_32FC1);
	cv::Mat TempoBuf = cv::Mat::zeros(src->rows, src->cols, CV_32FC1);

	src->convertTo(TempoBuf, CV_32F, 1., -128.);
	sepFilter2D(TempoBuf, Proc1Buf, CV_32F, KrnlGusX, KrnlLOGY, cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
	sepFilter2D(TempoBuf, Proc2Buf, CV_32F, KrnlLOGX, KrnlGusY, cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
	add(Proc1Buf, Proc2Buf, Proc3Buf);
	Proc3Buf.convertTo(*src, CV_8U, 1., 128.);

	Proc1Buf.release();
	Proc2Buf.release();
	Proc3Buf.release();
	TempoBuf.release();

	KrnlGusX.release();
	KrnlLOGX.release();
	KrnlGusY.release();
	KrnlLOGY.release();
	dst2.release();
	return dst;
}



void CPanelExistInspection::InspectionEnhance(cv::Mat *src, int id, bool disp)
{
	cv::Mat m_KrnlGusX[3];
	cv::Mat m_KrnlLOGX[3];
	cv::Mat m_KrnlGusY[3];
	cv::Mat m_KrnlLOGY[3];

	cv::Mat m_Proc1Buf;
	cv::Mat m_Proc2Buf;
	cv::Mat m_Proc3Buf;
	cv::Mat m_TempoBuf;

	float nKernelGUS[12] = { 10, 18, 29, 43, 57, 67, 71, 67, 57, 43, 29, 18 }; //509
	float nKernelGUS2[12] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }; // 12
	float nKernelLOG[24] = { -1, -2, -4, -9, -15, -20, -22, -16, 0, 22, 42, 50, 42, 22, 0, -16, -22, -20, -15, -9, -4, -2, -1, 0 }; //32

	m_KrnlGusX[0] = cv::Mat(12, 1, CV_32FC1, nKernelGUS) / 509.f;
	m_KrnlLOGX[0] = cv::Mat(24, 1, CV_32FC1, nKernelLOG) / 32.f;
	m_KrnlGusY[0] = cv::Mat(1, 12, CV_32FC1, nKernelGUS) / 509.f;
	m_KrnlLOGY[0] = cv::Mat(1, 24, CV_32FC1, nKernelLOG) / 32.f;

	m_KrnlLOGX[1] = cv::Mat(24, 1, CV_32FC1, nKernelLOG) / 32.f;
	m_KrnlGusY[1] = cv::Mat(1, 12, CV_32FC1, nKernelGUS2) / 12.f;

	m_KrnlGusX[2] = cv::Mat(12, 1, CV_32FC1, nKernelGUS2) / 12.f;
	m_KrnlLOGY[2] = cv::Mat(1, 24, CV_32FC1, nKernelLOG) / 32.f;

	int w = src->cols;
	int h = src->rows;

	m_Proc1Buf = cv::Mat::zeros(h, w, CV_32FC1);
	m_Proc2Buf = cv::Mat::zeros(h, w, CV_32FC1);
	m_Proc3Buf = cv::Mat::zeros(h, w, CV_32FC1);
	m_TempoBuf = cv::Mat::zeros(h, w, CV_32FC1);

	cv::Mat dst;
	cv::Mat element(3, 3, CV_8U, cv::Scalar(1));
	cv::Mat srcImage;

	src->copyTo(srcImage);
	src->convertTo(m_TempoBuf, CV_32F, 1., -128.);

	if (id == 0)	// 양방향 Enhance
	{
		cv::sepFilter2D(m_TempoBuf, m_Proc1Buf, CV_32F, m_KrnlGusX[0], m_KrnlLOGY[0], cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
		cv::sepFilter2D(m_TempoBuf, m_Proc2Buf, CV_32F, m_KrnlLOGX[0], m_KrnlGusY[0], cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
		cv::add(m_Proc1Buf, m_Proc2Buf, m_Proc3Buf);
	}
	else if (id == 1)
	{
		// 단방향 Enhance
		cv::sepFilter2D(m_TempoBuf, m_Proc3Buf, CV_32F, m_KrnlGusX[0], m_KrnlLOGY[0], cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
	}
	else
	{
		// 단방향 Enhance
		cv::sepFilter2D(m_TempoBuf, m_Proc3Buf, CV_32F, m_KrnlLOGX[1], m_KrnlGusY[1], cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
	}



	if (disp)
	{
		m_Proc3Buf.convertTo(dst, CV_8U, 1., 128.);
		//m_ViewerCamera.OnLoadImageFromPtr( (BYTE *) dst.data);
	}
	else m_Proc3Buf.convertTo(*src, CV_8U, 1., 128.);

	for (int i = 0; i < 3; i++)
	{
		m_KrnlGusX[i].release();
		m_KrnlLOGX[i].release();
		m_KrnlGusY[i].release();
		m_KrnlLOGY[i].release();
	}

	m_Proc1Buf.release();
	m_Proc2Buf.release();
	m_Proc3Buf.release();
	m_TempoBuf.release();
	dst.release();
	element.release();
	srcImage.release();
}



void CPanelExistInspection::DoRemoveNoise(int dMode, int width, int height, int widthStep, int i, int j, int threshold, int &hole, BOOL &is_hole, unsigned char *image, unsigned char *label_image)
{
	if (i <= 0 || j <= 0 || i >= height - 1 || j >= width - 1) return;

	switch (dMode)
	{
	case 0:			// check     
		if (is_hole == FALSE) return;

		hole++;

		label_image[i*width + j] = 255;

		if (hole > threshold)
		{
			is_hole = FALSE;
			return;
		}
		if (label_image[(i - 1)*width + j] != 255)
		{
			if (image[(i - 1)*widthStep + j] == 255) DoRemoveNoise(dMode, width, height, widthStep, i - 1, j, threshold, hole, is_hole, image, label_image);
		}

		if (label_image[i*width + j - 1] != 255)
		{
			if (image[i*widthStep + j - 1] == 255) DoRemoveNoise(dMode, width, height, widthStep, i, j - 1, threshold, hole, is_hole, image, label_image);
		}

		if (label_image[i*width + j + 1] != 255)
		{
			if (image[i*widthStep + j + 1] == 255) DoRemoveNoise(dMode, width, height, widthStep, i, j + 1, threshold, hole, is_hole, image, label_image);
		}

		if (label_image[(i + 1)*width + j] != 255)
		{
			if (image[(i + 1)*widthStep + j] == 255) DoRemoveNoise(dMode, width, height, widthStep, i + 1, j, threshold, hole, is_hole, image, label_image);
		}
		break;

	case 1:			// remove
		image[i*widthStep + j] = 0;

		if (label_image[(i - 1)*width + j] == 255)
		{
			if (image[(i - 1)*widthStep + j] == 255)
			{
				image[(i - 1)*widthStep + j] = 255;
				DoRemoveNoise(dMode, width, height, widthStep, i - 1, j, threshold, hole, is_hole, image, label_image);
			}
		}

		if (label_image[i*width + j - 1] == 255)
		{
			if (image[i*widthStep + j - 1] == 255)
			{
				image[i*widthStep + j - 1] = 0;
				DoRemoveNoise(dMode, width, height, widthStep, i, j - 1, threshold, hole, is_hole, image, label_image);
			}
		}

		if (label_image[i*width + j + 1] == 255)
		{
			if (image[i*widthStep + j + 1] == 255)
			{
				image[i*widthStep + j + 1] = 255;
				DoRemoveNoise(dMode, width, height, widthStep, i, j + 1, threshold, hole, is_hole, image, label_image);
			}
		}

		if (label_image[(i + 1)*width + j] == 255)
		{
			if (image[(i + 1)*widthStep + j] == 255)
			{
				image[(i + 1)*widthStep + j] = 255;
				DoRemoveNoise(dMode, width, height, widthStep, i + 1, j, threshold, hole, is_hole, image, label_image);
			}
		}
		break;

	case 2:			// clear
		label_image[i*width + j] = 0;
		if (label_image[(i - 1)*width + j] == 255)
		{
			if (image[(i - 1)*widthStep + j] == 255)
			{
				label_image[(i - 1)*width + j] = 0;
				DoRemoveNoise(dMode, width, height, widthStep, i - 1, j, threshold, hole, is_hole, image, label_image);
			}
		}

		if (label_image[i*width + j - 1] == 255)
		{
			if (image[i*widthStep + j - 1] == 255)
			{
				label_image[i*width + j - 1] = 0;
				DoRemoveNoise(dMode, width, height, widthStep, i, j - 1, threshold, hole, is_hole, image, label_image);
			}
		}

		if (label_image[i*width + j + 1] == 255)
		{
			if (image[i*widthStep + j + 1] == 255)
			{
				label_image[i*width + j + 1] = 0;
				DoRemoveNoise(dMode, width, height, widthStep, i, j + 1, threshold, hole, is_hole, image, label_image);
			}
		}

		if (label_image[(i + 1)*width + j] == 255)
		{
			if (image[(i + 1)*widthStep + j] == 255)
			{
				label_image[(i + 1)*width + j] = 0;
				DoRemoveNoise(dMode, width, height, widthStep, i + 1, j, threshold, hole, is_hole, image, label_image);
			}
		}
		break;
	}
}

void CPanelExistInspection::removeNoise(cv::Mat matImage, int nSize)
{
	// noise 제거
	unsigned char *pLabelBuf, *pBuffer;
	int		nWidthStep;
	int		nHoleCount;
	BOOL	bIsHole;
	int		i, j;
	int		nCropW, nCropH;
	pBuffer = (BYTE*)matImage.data;
	nWidthStep = matImage.cols;
	int nThresholdSize = nSize;

	nCropW = matImage.cols;
	nCropH = matImage.rows;

	pLabelBuf = new unsigned char[nCropW * nCropH];
	ZeroMemory(pLabelBuf, sizeof(BYTE)*nCropW * nCropH);

	for (i = 1; i < nCropH - 1; i++)
	{
		for (j = 1; j < nCropW - 1; j++)
		{
			if (pBuffer[i*nWidthStep + (j - 1)] == 0 && pBuffer[(i - 1)*nWidthStep + j] == 0 && pBuffer[i*nWidthStep + j] == 255 && pLabelBuf[i*nCropW + j] == 0)
			{
				bIsHole = TRUE;
				nHoleCount = 0;
				DoRemoveNoise(0, nCropW, nCropH, nWidthStep, i, j, nThresholdSize, nHoleCount, bIsHole, pBuffer, pLabelBuf);

				if (bIsHole == TRUE)
				{
					DoRemoveNoise(1, nCropW, nCropH, nWidthStep, i, j, nThresholdSize, nHoleCount, bIsHole, pBuffer, pLabelBuf);
				}
				else
				{
					DoRemoveNoise(2, nCropW, nCropH, nWidthStep, i, j, nThresholdSize, nHoleCount, bIsHole, pBuffer, pLabelBuf);
				}
			}
		}
	}

	delete[] pLabelBuf;
}
