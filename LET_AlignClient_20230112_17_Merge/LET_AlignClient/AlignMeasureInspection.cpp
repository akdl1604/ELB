#include "StdAfx.h"
#include "AlignMeasureInspection.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "opencv2\imgproc\imgproc_c.h"
#include "ImageProcessing\blob.h"
#include "ImageProcessing\BlobResult.h"
#include "Hough.h"
#include <omp.h>   // 20180713 pixelcount

const int NUMCORES = 2;

BOOL PtInTriangle( POINT *Verts, POINT pt )
{
     HRGN hRgn =CreatePolygonRgn( Verts, 4, WINDING);

    if( PtInRegion( hRgn, pt.x, pt.y ) )
    {
        DeleteObject( hRgn );
        return TRUE;
    }
 
    DeleteObject( hRgn );
    return FALSE;
}

CAlignMeasureInspection::CAlignMeasureInspection(void)
{
	for (int i = 0; i < 4; i++)
	{
		m_RectInspArray[i] = CRect(-1, -1, -1, -1);
		m_InspMeasureResult[i] = 0.0;
		m_InspMeasureSubResult[i] = 0.0;
		m_ResultLine[i] = cv::Point2f(-1, -1);
		m_Max_CalcDistance[i] = 0.0;
		m_Min_CalcDistance[i] = 0.0;
		m_InspMeasureOctagon[i] = 0.0;
	}

	m_dNozzleShiftX = 0;
	m_dNozzleShiftY = 0;
	m_dNozzleFindX = 0;
	m_dNozzleFindY = 0;

	for (int i = 0; i < 8; i++)
	{
		m_pt8AnglePoint[i].x = 0;
		m_pt8AnglePoint[i].y = 0;
	}

	m_bDebugShow = FALSE;
	m_bOctagonAngleModel = FALSE;
	for (int i = 0; i < 12; i++) m_bInspeJudge[i] = FALSE;
	for (int i = 0; i < 4; i++)
	{
		m_bUnDispencingNG_D[i] = FALSE;
		m_bDopoInspTotalJudge[i] = FALSE;
	}
	m_bInspResultDopoOverDisp = FALSE;
	m_bInsideMatchingLineUse = FALSE;
	m_bOutsideMatchingLineUse = FALSE;

	m_crSearchAutoRoiRect = CRect(0, 0, 100, 100);
	m_crSearchAutoInspRoiRect = CRect(0, 0, 100, 100);
}


CAlignMeasureInspection::~CAlignMeasureInspection(void)
{
	m_vSelectEdge.clear();
}


static bool comp(const cv::Point2f &t1, const cv::Point2f &t2) {
	return (t1.y > t2.y);
}

static bool compR(const cv::Point2f &t1, const cv::Point2f &t2) {
	return (t1.y < t2.y);
}

void CAlignMeasureInspection::LeastSqrRegression(double* x, double* y, int obs, double &a, double &b)
{
	double sum_x = 0;
	double sum_y = 0;
	double sum_xy = 0;
	double sum_xx = 0;
	double sum_squared_residue = 0;
	double residue_sqaured = 0;
	double slope = 0;
	double intercept = 0;
	double sum_squared_discrepancies = 0;
	double discrepancies_squared = 0;
	double avg_x = 0;
	double avg_y = 0;
	double r_squared = 0;

	for (int i = 0; i < obs; i++)
	{
		sum_x += x[i];
		sum_y += y[i];
		sum_xy += x[i] * y[i];
		sum_xx += x[i] * x[i];
	}

	avg_x = sum_x / obs;
	avg_y = sum_y / obs;

	a = slope = (obs*sum_xy - sum_x * sum_y) / (obs*sum_xx - sum_x * sum_x);
	b = intercept = avg_y - slope * avg_x;

	for (int i = 0; i < obs; i++)
	{
		discrepancies_squared = pow(y[i] - intercept - (slope*x[i]), 2);
		sum_squared_discrepancies += discrepancies_squared;
		residue_sqaured = pow(y[i] - avg_y, 2);
		sum_squared_residue += residue_sqaured;
	}
	r_squared = (sum_squared_residue - sum_squared_discrepancies) / sum_squared_residue;
}

void CAlignMeasureInspection::calcHistogram(IplImage *srcImage, int(&Histo)[256])
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

void CAlignMeasureInspection::applyLUT(IplImage * srcImage, IplImage * dstImage, int(&LUT)[256])
{
	for (int y = 0; y < srcImage->height; y++)
	{
		for (int x = 0; x < srcImage->width; x++)
		{
			dstImage->imageData[y * dstImage->width + x] = (char)LUT[(uchar)srcImage->imageData[y * srcImage->width + x]];
		}
	}
}


void CAlignMeasureInspection::endInSearch(cv::Mat matsrcImage, cv::Mat matdstImage)
{
	int i;

	int histogram[256];
	int LUT[256];
	int lowthresh, highthresh;
	float scale_factor;

	lowthresh = 0;
	highthresh = 255;//히스토그램명도값의빈도수조사


	IplImage* srcImage = &IplImage(matsrcImage);
	IplImage *dstImage = cvCreateImage(cvSize(matsrcImage.cols, matsrcImage.rows), 8, 1);
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

	for (i = 255; i >= 0; i--)
	{
		if (histogram[i])
		{
			highthresh = i;
			break;
		}
	}


	for (i = 0; i < lowthresh; i++) 		LUT[i] = 0;
	for (i = 255; i > highthresh; i--) 		LUT[i] = 255;

	scale_factor = (float)(255.0 / (highthresh - lowthresh));

	for (i = lowthresh; i <= highthresh; i++) 	LUT[i] = int((i - lowthresh) * scale_factor);

	applyLUT(srcImage, dstImage, LUT);
	matdstImage = cv::cvarrToMat(dstImage);

	cvReleaseImage(&dstImage);
}

void CAlignMeasureInspection::DoRemoveNoise(int dMode, int width, int height, int widthStep, int i, int j, int threshold, int &hole, BOOL &is_hole, unsigned char *image, unsigned char *label_image)
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

void CAlignMeasureInspection::removeNoise(cv::Mat matImage, int nSize)
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

int compare(const void *a, const void *b)
{
	if (*(double*)a < *(double*)b)        return -1;
	else if (*(double*)a > *(double*)b)        return 1;
	else         return 0;
}

void setLabel(cv::Mat& image, cv::string str, std::vector<cv::Point> contour)
{
	int fontface = cv::FONT_HERSHEY_SIMPLEX;
	double scale = 0.5;
	int thickness = 1;
	int baseline = 0;

	cv::Size text = cv::getTextSize(str, fontface, scale, thickness, &baseline);
	cv::Rect r = cv::boundingRect(contour);

	cv::Point pt(r.x + ((r.width - text.width) / 2), r.y + ((r.height + text.height) / 2));
	cv::rectangle(image, pt + cv::Point(0, baseline), pt + cv::Point(text.width, -text.height), CV_RGB(200, 200, 200), CV_FILLED);
	putText(image, str, pt, fontface, scale, CV_RGB(0, 0, 0), thickness, 8);
}


int GetAngleABC(cv::Point a, cv::Point b, cv::Point c)
{
	cv::Point ab = cv::Point(b.x - a.x, b.y - a.y);
	cv::Point cb = cv::Point(b.x - c.x, b.y - c.y);

	float dot = float(ab.x * cb.x + ab.y * cb.y);
	float cross = float(ab.x * cb.y - ab.y * cb.x);

	float alpha = atan2(cross, dot);
	return (int)floor(alpha * 180.0 / CV_PI + 0.5);
}

cv::string intToString(int n)
{
	std::stringstream s;
	s << n;
	return s.str();
}

int compareCpoint(const void *elem1, const void *elem2)
{
	CPoint *p1 = (CPoint*)elem1;
	CPoint *p2 = (CPoint*)elem2;

	if (p1->y < p2->y)        return -1;
	else if (p1->y > p2->y)        return 1;
	else         return 0;
}

int comparePoint2F(const void *elem1, const void *elem2)
{
	cv::Point2f *p1 = (cv::Point2f*)elem1;
	cv::Point2f *p2 = (cv::Point2f*)elem2;

	if (p1->y < p2->y)        return -1;
	else if (p1->y > p2->y)        return 1;
	else         return 0;
}

int comparePoint2X(const void *elem1, const void *elem2)
{
	cv::Point2f *p1 = (cv::Point2f*)elem1;
	cv::Point2f *p2 = (cv::Point2f*)elem2;

	if (p1->x < p2->x)        return -1;
	else if (p1->x > p2->x)        return 1;
	else         return 0;
}
void CAlignMeasureInspection::sobelDirection(int nMaskSize, int nDirection, IplImage *srcImage, IplImage *dstImage)
{
	CvMat mtSobel;
	float *mask;
	mask = (float *)malloc(sizeof(float) * nMaskSize * nMaskSize);

	if (nMaskSize == 3)
	{
		switch (nDirection) {
		case SOBEL_UP:
			mask[0] = 1;	mask[1] = 2;	mask[2] = 1;
			mask[3] = 0;	mask[4] = 0;	mask[5] = 0;
			mask[6] = -1;	mask[7] = -2;	mask[8] = -1;
			break;
		case SOBEL_DN:
			mask[0] = -1;	mask[1] = -2;	mask[2] = -1;
			mask[3] = 0;	mask[4] = 0;	mask[5] = 0;
			mask[6] = 1;	mask[7] = 2;	mask[8] = 1;
			break;
		case SOBEL_LEFT:
			mask[0] = 1;	mask[1] = 0;	mask[2] = -1;
			mask[3] = 2;	mask[4] = 0;	mask[5] = -2;
			mask[6] = 1;	mask[7] = 0;	mask[8] = -1;
			break;
		case SOBEL_RIGHT:
			mask[0] = -1;	mask[1] = 0;	mask[2] = 1;
			mask[3] = -2;	mask[4] = 0;	mask[5] = 2;
			mask[6] = -1;	mask[7] = 0;	mask[8] = 1;
			break;
		};
	}

	cvInitMatHeader(&mtSobel, nMaskSize, nMaskSize, CV_32FC1, mask);
	cvFilter2D(srcImage, dstImage, &mtSobel);

	free(mask);
}

void CAlignMeasureInspection::removeNoise(IplImage *pImage, int nSize)
{
	// noise 제거
	unsigned char *pLabelBuf, *pBuffer;
	int		nWidthStep;
	int		nHoleCount;
	BOOL	bIsHole;
	int		i, j;
	int		nCropW, nCropH;
	pBuffer = (BYTE*)pImage->imageData;
	nWidthStep = pImage->widthStep;
	int nThresholdSize = nSize;

	nCropW = pImage->width;
	nCropH = pImage->height;
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


void CAlignMeasureInspection::endInSearch(IplImage * srcImage, IplImage * dstImage)
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

cv::Mat CAlignMeasureInspection::fnEnhanceImagCalc(cv::Mat *src)
{
	if (src->empty()) return *src;

	cv::Mat dst, dst2;
	float nKernelGUS[7] = { 23,43,63,71,63,43,23 };
	float nKernelLOG[13] = { -4,-12,-20,-20,0,33,50,33,0,-20,-20,-12,-4 }; //32

	cv::Mat KrnlGusX = cv::Mat(7, 1, CV_32FC1, nKernelGUS) / 329.f;
	cv::Mat KrnlLOGX = cv::Mat(13, 1, CV_32FC1, nKernelLOG) / 32.f;
	cv::Mat KrnlGusY = cv::Mat(1, 7, CV_32FC1, nKernelGUS) / 329.f;
	cv::Mat KrnlLOGY = cv::Mat(1, 13, CV_32FC1, nKernelLOG) / 32.f;

	cv::Mat Proc1Buf = cv::Mat::zeros(src->rows, src->cols, CV_32FC1);
	cv::Mat Proc2Buf = cv::Mat::zeros(src->rows, src->cols, CV_32FC1);
	cv::Mat Proc3Buf = cv::Mat::zeros(src->rows, src->cols, CV_32FC1);
	cv::Mat TempoBuf = cv::Mat::zeros(src->rows, src->cols, CV_32FC1);

	src->convertTo(TempoBuf, CV_32F, 1., -128.);
	sepFilter2D(TempoBuf, Proc1Buf, CV_32F, KrnlGusX, KrnlLOGY, cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
	sepFilter2D(TempoBuf, Proc2Buf, CV_32F, KrnlLOGX, KrnlGusY, cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
	add(Proc1Buf, Proc2Buf, Proc3Buf);
	Proc3Buf.convertTo(dst, CV_8U, 1., 128.);

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

void CAlignMeasureInspection::InspectionEnhance(cv::Mat *src, int id, bool disp)		// 0 : All, 1 : Y, 2 : X
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

	m_Proc3Buf.convertTo(*src, CV_8U, 1., 128.);

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
