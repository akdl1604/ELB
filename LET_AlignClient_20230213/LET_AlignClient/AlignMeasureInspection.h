#pragma once

class CPatternMatching;

class CAlignMeasureInspection
{
public:
	CAlignMeasureInspection(void);
	~CAlignMeasureInspection(void);
	
	CRect m_RectInspArray[4];
	CRect m_RectAutoRoiArray[4];
	double m_InspMeasureResult[4];
	double m_InspMeasureSubResult[4];
	double m_InspMeasureOctagon[4];


	cv::Point2f m_ResultLine[4];
	cv::Mat m_ResultColorImage;

	double m_Max_CalcDistance[4];
	double m_Min_CalcDistance[4];
	double m_chipInResultPosX[4];
	double m_chipInResultPosY[4];
	double m_chipOutResultPosX[4];
	double m_chipOutResultPosY[4];
	double m_chipdFinalResult[8];
	double m_chipdFinalThetaResult[8];
	double m_chipdLength[8];

	struct _stBlob {
		cv::Rect boundRect;
		cv::Rect newBoundRect;
		int id;
		double Perimeter;
		double Circularity;
		double Area;
		double Size;
		double mean;
		double stddev;
		double density;
		double Moment;
		double Contrast;
		int cx;
		int cy;
		BOOL bShow;
		int reCalc[2];
	};

	struct _stBlobCountPixel {  // 20180713 pixelcount
		int onebyone;
		int twobytwo;
		int threebythree;
		int fourbyfour;
		int fivebyfive;
		int pixincluster;
	};

	_stBlobCountPixel m_blobPixelCount;  // 20180713 pixelcount

	std::vector<_stBlob> m_myBlobs[2];

public:

	CLET_AlignClientDlg *m_pMain;

	cv::Point2f m_DopoLinePoint[4][2][2];	// [pos][start/end][in/out];	0 : start, 1 : end // 0 : in	, 1: out
	double m_ax_Out[4];
	double m_bc_Out[4];
	double m_ax_In[4];
	double m_bc_In[4];
	double m_Max_Distance[4];

	double m_dNozzleShiftX;
	double m_dNozzleShiftY;
	double m_dNozzleFindX;
	double m_dNozzleFindY;
	CPoint m_pt8AnglePoint[8];
	cv::Point2f m_dOctagonMinMaxDistance[4];
	cv::Point2f m_dOctagonMinPos_S[4];
	cv::Point2f m_dOctagonMaxPos_S[4];
	cv::Point2f m_dOctagonMinPos_E[4];
	cv::Point2f m_dOctagonMaxPos_E[4];
	double m_dOctagonDistance[32];
	double m_dOctagonDistanceAxis[32];
	double m_dOctagonDopoResult[32];
	double m_dMeasureDistance[12];

	std::map<int, double> m_mapDooototalLength[4];
	int m_nPeelerCheckDistance;

	BOOL m_bOverDispNG[8];	// 각 변별 액올라탐 불량 판정 (top, topright, right 순)
	BOOL m_bDebugShow;
	BOOL m_bOctagonAngleModel;
	BOOL  m_bInspeJudge[12];
	BOOL m_bUnDispencingNG_D[4];
	BOOL m_bDopoInspTotalJudge[4];
	BOOL m_bInspResultDopoOverDisp;
	BOOL m_bInsideMatchingLineUse;
	BOOL m_bOutsideMatchingLineUse;

	CRect m_crSearchAutoRoiRect;
	CRect m_crSearchAutoInspRoiRect;
	CPoint m_ptLeftPtr;
	CPoint m_ptRightPtr;
	CRect m_newRect;

	std::vector<cv::Point> m_vSelectEdge;

	void LeastSqrRegression(double* x, double* y, int obs, double &a, double &b);
	void calcHistogram(cv::Mat& srcImage, int(&Histo)[256]);
	void applyLUT(cv::Mat& srcImage, cv::Mat& dstImage, int(&LUT)[256]);
	void endInSearch(cv::Mat& srcImage, cv::Mat& dstImage);
	void DoRemoveNoise(int dMode, int width, int height, int widthStep, int i, int j, int threshold, int &hole, BOOL &is_hole, unsigned char *image, unsigned char *label_image);
	void removeNoise(cv::Mat matImage, int nSize);
	void sobelDirection(int nMaskSize, int nDirection, cv::Mat& srcImage, cv::Mat& dstImage);
	cv::Mat fnEnhanceImagCalc(cv::Mat *src);	
	void InspectionEnhance(cv::Mat *src, int id, bool disp);		// 0 : All, 1 : Y, 2 : X;
};

