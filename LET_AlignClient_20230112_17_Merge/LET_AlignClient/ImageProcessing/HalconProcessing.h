#ifndef __HALCON_PROCESSING_H__
#define __HALCON_PROCESSING_H__

#ifndef _U_POINT_
#define _U_POINT_
template <class T>
struct UPoint {
	T x;
	T y;
};
#endif

//#define _CPP_USE_HALCON

#ifdef _CPP_USE_HALCON
using namespace HalconCpp;
#else
using namespace Halcon;
#endif


class CHalconProcessing
{
public:
	CHalconProcessing();
	~CHalconProcessing();

	void HalconOpenWindow(HWND m_hWnd);

	bool getNccSelect() { return m_bNccSelect; }
	void halcon_releaseModel();
	void halcon_SetMarkOffset(int xoffset,int yoffset);
	UPoint<double> halcon_GetResultPos();
	
	BOOL halcon_ReadInspectModel(CString modelname);
	BOOL halcon_WriteModel(CString modelname,BYTE *pbyImage,int w,int h,CRect roiRect,CPoint ptCenterOffset=CPoint(0,0));
	double halcon_GetResultAngle();
	double halcon_GetResultRadianAngle();
	double halcon_GetResultScore();
	void halcon_SetShapeModelOrigin(double xoffset,double yoffset);
	UPoint<double> halcon_GetRobotData(UPoint<double> vecSrcPts, int w,int h);
	BOOL halcon_CameraToRobotData(std::vector<UPoint<double>> vecSrcPts,std::vector<UPoint<double>> vecDstPts,double stage_cx,double stage_cy);
	void halcon_GetThreePoint_Circle(float* r, UPoint<double> &psRelMatchPos, std::vector<UPoint<double>> psCenterPoint);
	PointF halcon_CenterPointFrom3Points(PointF startPoint, PointF middlePoint, PointF endPoint);
	void halcon_GetTwoPoint_Circle(UPoint<double> &psRelMatchPos, const float fR, std::vector<UPoint<double>> psCenterPoint, const BOOL Direction);
	void halcon_CalcRotationCenter(UPoint<double> &psRelMatchPos, std::vector<UPoint<double>> psCenterPoint,double angle);
	void halcon_CalcRotationCenter2(UPoint<double> &psRelMatchPos, std::vector<UPoint<double>> psCenterPoint,double angle);
	bool halcon_GetTwoPointTheta_Circle(UPoint<double> &psRelMatchPos, std::vector<UPoint<double>> psCenterPoint,double angle);
	bool  halcon_GetSlopeResolutionFromPoint(Point point, Point centerPoint,double &slope,double &resolution,int distance=1,int dir=0);//dir=0 가로 ,dir=1세로
	bool halcon_LinearCalibraion(std::vector<double> src,std::vector<double> dst);
	double halcon_GetLinearData(double vecSrcPts,double startAngle,double step);
	double halcon_GetLinearData(double vecSrcPts, std::vector<double> vtThetaReal);

	double GetAngleFromPoint(Point point, Point centerPoint);
	void writeMatToFile(cv::Mat& m, const char* filename);
	void readMatToFile(cv::Mat& m, const char* filename);
	UPoint<double> findAutoCenterPos(unsigned char *pImage,int w,int h,CRect rect);
	void halcon_CalcRotate(double x1, double y1, double rangle, double *x2, double *y2);
	BOOL halcon_CalcPrealign(UPoint<double> ptLeftPos, UPoint<double>ptRightPos,UPoint<double> ptMarkPitch);
	double GetPrealignResult(int nAxis) { return m_dbPrealignResult[nAxis]; }
		
	BOOL halcon_ReadModel(CString modelname, int nMinContrast = -1, int nHighContrast = -1);
	BOOL halcon_ReadModel(BYTE *psrc, int w,int h, int nMinContrast = -1, int nHighContrast = -1);
	BOOL halcon_ReadModelNew(BYTE *psrc, BYTE *pMask,int w,int h);
	BOOL halcon_ReadModelNew(CString modelname, CString modelMaskname);
	BOOL halcon_ReadScaleModel(BYTE* psrc, BYTE* pMask, int w, int h);
	BOOL halcon_ReadScaleModel(CString modelname, CString modelMaskname);
	BOOL halcon_ReadNccModel(CString modelname);
	BOOL halcon_ReadNccModel(BYTE *ptemplateSrc,int w,int h);

	BOOL halcon_ReadMaskModel(CString modelname,BOOL bTemplateShape,BOOL m_bMaskRoiUse[4],BOOL m_bMaskRoiShape[4],CRect m_crMaskRect[4],  int nMinContrast = -1, int nHighContrast = -1);
	BOOL halcon_ReadMaskModel(BYTE *ptemplateSrc,int w,int h,BOOL bTemplateShape,BOOL m_bMaskRoiUse[4],BOOL m_bMaskRoiShape[4],CRect m_crMaskRect[4], int nMinContrast = -1, int nHighContrast = -1);
	BOOL halcon_ReadMaskNCCModel(CString modelname,BOOL bTemplateShape,BOOL m_bMaskRoiUse[4],BOOL m_bMaskRoiShape[4],CRect m_crMaskRect[4]);
	BOOL halcon_ReadMaskNCCModel(BYTE *ptemplateSrc,int w,int h,BOOL bTemplateShape,BOOL m_bMaskRoiUse[4],BOOL m_bMaskRoiShape[4],CRect m_crMaskRect[4]);
	BOOL halcon_SearchNccMark(BYTE *pbyImage, int w,int h,CRect InspROI ,double score);
	BOOL halcon_SearchMark(BYTE *pbyImage, int w,int h, CRect InspROI ,double score, char * charMethod="least_squares_high" ); //"interpolation"
	BOOL halcon_SearchScaleMark(BYTE* pbyImage, int w, int h, CRect InspROI, double score, char* charMethod = "least_squares_high");

	HTuple getHalconModelID() { return hv_ModelId; }
	bool getModelRead() { return m_bModelRead; }
	bool setModelRead(bool bisopen) { return m_bModelRead=bisopen; }
	bool getLineaCalibration() { return m_bLinearCalibraion; }
	void setLineaCalibration(bool bSet) { m_bLinearCalibraion = bSet; }
	int getFatWidth() { return m_nFatWidth; }
	int getFatHeight() { return m_nFatHeight; }
	Hobject getModelContour(){ return ho_ModelContours;}

	void setSearchAngleRange (double angle){ m_nSearchAngleRange=angle;}
	void setSearchAngleStep (double anglestep){ m_nSearchAngleStep=anglestep;}

	void saveParameter(CString fname,int ncam,int npos,int nindex, int njob);
	void readParameter(CString fname,int ncam,int npos,int nindex, int njob);
	void halconWriteModel(CString modelName);
	void halconReadModel(CString modelName);

	int halcon_QRcodeDetection(BYTE *psrc, int w,int h,CRect InspROI);
	CPoint halcon_AkonMarkDetection(BYTE *psrc, int w,int h,CRect InspROI,bool bShow);
	int halcon_AkonDetection(BYTE *psrc, int w,int h,CRect InspROI,int nWhiteThreshold, int nBlackThreshold,std::vector<CRect> &rectInfo,bool bShow);
	int halcon_DefectDetection(BYTE *psrc, int w,int h,CRect InspROI,int nWhiteThreshold, int nBlackThreshold,bool bShow);
	int halcon_muraDetection(BYTE *psrc, int w,int h,CRect InspROI,bool bwhite);
	bool halcon_lines_gauss(BYTE *psrc, int w,int h,CRect InspROI,  int MinLineLength,int MaxLineWidth, int Contrast,bool bwhite, bool bShow=true,int id=0,int bProc=0,bool reverse=false);
	IplImage* halcon_linePreProcTest(BYTE *psrc, int w,int h,int bProc);
	bool halcon_laser_lines_detect(BYTE *psrc, int w,int h,CRect InspROI,bool bShow=true);

	bool halcon_DisplayShapeModel(BYTE *m_workImg,int low,int high);
	HTuple halcon_DispImage(IplImage *m_workImg,CRect rect,HWND HwndView );
	bool halcon_ShowModel(BYTE *pImg,int w,int h,CRect rect,HWND HwndView,bool bshowimg=true);
	void halcon_ShowModelClose();
	void halcon_DispImageToModelContour(cv::Mat *m_workImg,int col,int row,CRect Inspr,CRect r=CRect(0,0,0,0),HWND hWnd=NULL);

	Hobject IplImageToHImage(IplImage *pImage);
	IplImage* HImageToIplImage(Hobject &Hobj);
	void HContourToIplImage(Hobject &Hobj,Hobject &Himage);
	BOOL getPreProcessingUse() { return m_bPreProcess; }
	void setPreProcessingUse(BOOL buse) { m_bPreProcess = buse; }
	Hobject m_ContourImageResult;
	std::vector<PointF> m_pFLinePtr;
	std::vector<PointF> m_pSLinePtr;
	std::vector<PointF> m_pARLinePtr;
	std::vector<std::vector<POINT>> m_arrContourPoint;
	std::vector<RECT> m_pQRPosPtr;

	double m_dNa[2];
	double m_dNb[2];

public:
	int m_ReadNumLevel;
	double m_ReadStartAngle;
	double m_ReadEndAngle;
	double m_ReadStepAngle;
	double m_ReadSmallScale;
	CString m_ReadOptimize;
	CString m_ReadMetric;
	double m_ReadContrast;
	double m_ReadMinContrast;

	int m_SearchNumMatch;
	double m_SearchStartAngle;
	double m_SearchEndAngle;
	double m_SearchMinscore;
	double m_SearchGreedness;
	CString m_SearchSubPixel;
	int m_SearchMethod;
	BOOL m_bPreProcess;

private:
	int m_nFatWidth;
	int m_nFatHeight;
	UPoint <double> m_ptMarkPos;
	UPoint <int> m_ptMarkOffset;
	double m_dModelAngle;
	double m_dModelAngleRadian;
	double m_dModelScore;
	bool m_bModelRead;
	bool m_bCalibration;
	bool m_bLinearCalibraion;
	bool m_bNccSelect;
	double m_dResolutionX;
	double m_dResolutionY;
	PointF m_ptRotateCenter;
	double m_dbPrealignResult[3];

	double m_nFatOffsetWidth;
	double m_nFatOffsetHeight;
	double m_nSearchAngleRange;
	double m_nSearchAngleStep;

	cv::Mat m_cvTrans_mat[4];
	std::vector<cv::Mat> m_cvLinearCalib;
#ifdef _CPP_USE_HALCON
	HObject ho_TemplateImage,ho_ModelContours;
#else
	Hobject ho_TemplateImage,ho_ModelContours;
#endif	
	
	HTuple  hv_ModelId, hv_ModelRow, hv_ModelColumn;
	HTuple  hv_ModelAngle, hv_ModelScore, hv_ModelScale;
	Hlong m_lWindowID;
	HTuple m_hHalHwndView;
};

//extern CHalconProcessing g_pHalcon[MAX_PATTERN][MAX_PATTERN_INDEX];
extern CHalconProcessing g_pHalcon;
extern CHalconProcessing g_pHalconNcc;

#endif //__HALCON_PROCESSING_H__

