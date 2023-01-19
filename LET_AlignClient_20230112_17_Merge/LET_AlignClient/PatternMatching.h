#pragma once

#include "ImageProcessing\HalconProcessing.h"

#ifndef PI
#define PI 3.141592
#endif

#ifndef MAX_PATTERN_INDEX
#define MAX_PATTERN_INDEX 5
#endif

#ifndef MAX_MASK_INDEX
#define MAX_MASK_INDEX 4
#endif

#ifndef _FIND_INFO__
#define _FIND_INFO__

#ifndef _S_POINT_
#define _S_POINT_
struct sPoint {
	double x, y;
};
#endif

#ifndef _S_LINE_
#define _S_LINE_
struct sLine {
	double mx, my;	// x, y 기울기 Radian.
	double sx, sy;	// 직선의 중심점
	double a, b;
};
#endif


class CFindInfo
{
public:
	CFindInfo() {
		Found = FIND_OK;
		XPos = YPos = XRef = YRef = Width = Height = Score = MatchingXPos = MatchingYPos = 0.0;
		FoundPatternNum = 0;
		for (int i = 0; i < 4; i++)
		{
			LineX[i] = 0;
			LineY[i] = 0;
			LineX_T[i] = 0;
			LineY_T[i] = 0;
		}
		for (int i = 0; i < MAX_CALIPER_LINES; i++)
		{
			bFindline[i] = FALSE;
		}
		memset(line, 0, sizeof(line));
	}

private:
	int Found;
	int FoundPatternNum;
	double mark_x;		// Fixture 위치 기억 목적
	double mark_y;		// Fixture 위치 기억 목적
	double mark_score;	// Fixture 위치 기억 목적
	double mark_angle;	// Fixture 위치 기억 목적
	int mark_index;
	int mark_found;
	double included_angle;

	double XPos;
	double YPos;
	double XRef;
	double YRef;
	double Width;
	double Height;
	double Score;
	double CaliperAngle;
	double angle;
	double LineX[4];	// 직선 방정식에 대한 2개 좌표	ROI 범위
	double LineY[4];	// 직선 방정식에 대한 2개 좌표	ROI 범위
	double LineX_T[4];	// 직선 방정식에 대한 2개 좌표	전체 범위
	double LineY_T[4];	// 직선 방정식에 대한 2개 좌표	전체 범위
	sLine line[MAX_CALIPER_LINES];
	BOOL bFindline[MAX_CALIPER_LINES];
	double MatchingXPos;
	double MatchingYPos;
	//KJH 2022-03-12 MatchingRate Setting Value FDC 보고 추가
	int	   FindIndex;	//찾은 Pattern의 Index정보 저장
public:
	double Left() { return XPos - XRef; }
	double Top() { return YPos - YRef; }
	double Right() { return XPos + (Width - XRef); }
	double Bottom() { return YPos + (Height - YRef); }

	void SetLineX(int nIndex, double dValue) { LineX[nIndex] = dValue; }
	void SetLineY(int nIndex, double dValue) { LineY[nIndex] = dValue; }
	void SetLineX_T(int nIndex, double dValue) { LineX_T[nIndex] = dValue; }
	void SetLineY_T(int nIndex, double dValue) { LineY_T[nIndex] = dValue; }
	void SetXPos(double value) { XPos = value; }
	void SetYPos(double value) { YPos = value; }
	void SetXRef(double value) { XRef = value; }
	void SetYRef(double value) { YRef = value; }
	//KJH 2021-08-14 Caliper Angle Log 추가
	void SetCaliperAngle(double value) { CaliperAngle = value; }
	void SetScore(double value) { Score = value; }
	void SetWidth(double value) { Width = value; }
	void SetHeight(double value) { Height = value; }
	void SetFound(int value) { Found = value; }
	void SetFoundPatternNum(int value) { FoundPatternNum = value; }
	void SetAngle(double value) { angle = value; }
	void SetLine(int nLine, sLine lineValue) { line[nLine] = lineValue; }
	void SetFindline(int nLine, BOOL value) { bFindline[nLine] = value; }
	void SetMatchingXPos(double value) { MatchingXPos = value; }
	void SetMatchingYPos(double value) { MatchingYPos = value; }
	void SetFindIndex(double value) { FindIndex = value; }

	void set_mark_x(double pos) { mark_x = pos; }
	void set_mark_y(double pos) { mark_y = pos; }
	void set_mark_score(double score) { mark_score = score; }
	void set_mark_angle(double angle) { mark_angle = angle; }
	void set_mark_found(int found) { mark_found = found; }
	void set_included_angle(double angle) { included_angle = angle; }
	void set_mark_index(int idx) { mark_index = idx; }

	double GetLineX(int nIndex) { return LineX[nIndex]; }
	double GetLineY(int nIndex) { return LineY[nIndex]; }
	double GetLineX_T(int nIndex) { return LineX_T[nIndex]; }
	double GetLineY_T(int nIndex) { return LineY_T[nIndex]; }
	double GetXPos() { return XPos; }
	double GetYPos() { return YPos; }
	double GetXRef() { return XRef; }
	double GetYRef() { return YRef; }
	double getScore() { return Score; }
	//KJH 2021-08-14 Caliper Angle Log 추가
	double getCaliperAngle() { return CaliperAngle; }
	double GetWidth() { return Width; }
	double GetHeight() { return Height; }
	int GetFound() { return Found; }
	int GetFoundPatternNum() { return FoundPatternNum; }
	double GetAngle() { return angle; }
	sLine GetLine(int nLine) { return line[nLine]; }
	BOOL GetFindline(int nLine) { return bFindline[nLine]; }
	double GetMatchingXPos() { return MatchingXPos;}
	double GetMatchingYPos() { return MatchingYPos;}
	double GetFindIndex() { return FindIndex; }

	double get_mark_x() { return mark_x; }
	double get_mark_y() { return mark_y; }
	double get_mark_score() { return mark_score; }
	double get_mark_angle() { return mark_angle; }
	int get_mark_found() { return mark_found; }
	double get_included_angle() { return included_angle; }
	int get_mark_index() { return mark_index; }

	double* GetXPosPointer() { return &XPos; }
	double* GetYPosPointer() { return &YPos; }
	double* GetXRefPointer() { return &XRef; }
	double* GetYRefPointer() { return &YRef; }
	double* GetScorePointer() { return &Score; }
	double* GetWidthPointer() { return &Width; }
	double* GetHeightPointer() { return &Height; }
	int* GetFoundPointer() { return &Found; }
};
#endif

class CLET_AlignClientDlg;
class CPatternMatching
{
public:
	CPatternMatching(void);
	~CPatternMatching(void);
	double m_ax[MAX_CAMERA][NUM_POS][MAX_INDEX];	// 직선 방정식의 기울기
	double m_bc[MAX_CAMERA][NUM_POS][MAX_INDEX];	// 직선 방정식의 Y 절편
	CRect m_rcRealROI[MAX_CAMERA][NUM_POS][MAX_INDEX];
	CRect m_rcInspArea[3];
	int m_nPatternMinContrast;
	int m_nPatternHighContrast;

	int m_nPatternResultIndex;	// 20.03.29 
private:
	int m_nProjectionArea[MAX_CAMERA][NUM_POS][MAX_INDEX];
	int m_nSearchArea[MAX_CAMERA][NUM_POS][MAX_INDEX];
	int m_nRemoveNoiseSize[MAX_CAMERA][NUM_POS][MAX_INDEX];
	int m_nSobelThreshold[MAX_CAMERA][NUM_POS][MAX_INDEX];
	int m_nMarkFindMethod;	// 0 : Matching, 1 : Edge Find
	int m_nInspFindMethod;  // inspection에서 사용하는 방식
	BOOL m_bMarkReverseFind;	// 0 : L 모양, 1: ㄱ 모양
	BOOL m_bNccModel;
	BOOL m_bMaskingModel;
	BOOL m_bVerticalCamPos;
	//L_LAMI
	int m_nCornerType[MAX_CAMERA][NUM_POS];
	int m_nEdgePolarity[MAX_CAMERA][NUM_POS];
	int m_nEdgeDirection[MAX_CAMERA][NUM_POS];
	int m_nPreProcess[MAX_CAMERA][NUM_POS];
	int m_nEdgeProcess[MAX_CAMERA][NUM_POS];

	//int m_nWidth;
	//int m_nHeight;
	int m_nJobID;

	double m_dbRefPosX[MAX_CAMERA][NUM_POS];
	double m_dbRefPosY[MAX_CAMERA][NUM_POS];
	double m_dbRefPosT[MAX_CAMERA][NUM_POS];
	double m_dbFixturePosX[MAX_CAMERA][NUM_POS];
	double m_dbFixturePosY[MAX_CAMERA][NUM_POS];
	double m_dbFixturePosT[MAX_CAMERA][NUM_POS];
	double m_dbMarkOffsetX[MAX_CAMERA][NUM_POS][MAX_PATTERN_INDEX];
	double m_dbMarkOffsetY[MAX_CAMERA][NUM_POS][MAX_PATTERN_INDEX];

	double m_dbFinalMarkOffsetX[MAX_CAMERA][NUM_POS][MAX_PATTERN_INDEX];
	double m_dbFinalMarkOffsetY[MAX_CAMERA][NUM_POS][MAX_PATTERN_INDEX];

	double m_dbPosOffsetX[MAX_CAMERA][NUM_POS][MAX_PATTERN_INDEX];
	double m_dbPosOffsetY[MAX_CAMERA][NUM_POS][MAX_PATTERN_INDEX];
	CRect m_searchROI[MAX_CAMERA][NUM_POS];
	CRect m_searchROI2[MAX_CAMERA][NUM_POS][MAX_PATTERN_INDEX - 1];		//20.03.26
	BOOL m_bUseSearchROI2[MAX_CAMERA][NUM_POS][MAX_PATTERN_INDEX - 1];
	double m_dbMatchingRate[MAX_CAMERA][NUM_POS][MAX_PATTERN_INDEX];
	double m_dbLowContrast[MAX_CAMERA][NUM_POS][MAX_PATTERN_INDEX];
	double m_dbHighContrast[MAX_CAMERA][NUM_POS][MAX_PATTERN_INDEX];
	BOOL m_bUseMask[MAX_CAMERA][NUM_POS][MAX_PATTERN_INDEX];
	CRect m_bMaskArea[MAX_CAMERA][NUM_POS][MAX_PATTERN_INDEX][MAX_MASK_INDEX];
	BOOL m_bMaskAreaUse[MAX_CAMERA][NUM_POS][MAX_PATTERN_INDEX][MAX_MASK_INDEX];
	BOOL m_bMaskAreaShape[MAX_CAMERA][NUM_POS][MAX_PATTERN_INDEX][MAX_MASK_INDEX];
	BOOL m_bTemplateAreaShape[MAX_CAMERA][NUM_POS][MAX_PATTERN_INDEX];

	CHalconProcessing m_Halcon[MAX_CAMERA][NUM_POS][MAX_PATTERN_INDEX];
	CFindInfo m_FindInfo[MAX_CAMERA][NUM_POS];
public:
	int m_nFlipDir;
	int getSearchArea(int nCam, int nPos, int nIndex) { return m_nSearchArea[nCam][nPos][nIndex]; }
	int getProjectionArea(int nCam, int nPos, int nIndex) { return m_nProjectionArea[nCam][nPos][nIndex]; }
	int getFatWidth(int nCam, int nPos, int nIndex) { return m_Halcon[nCam][nPos][nIndex].getFatWidth(); }
	int getFatHeight(int nCam, int nPos, int nIndex) { return m_Halcon[nCam][nPos][nIndex].getFatHeight(); }

	double getRefX(int nCam, int nPos) { return m_dbRefPosX[nCam][nPos]; }
	double getRefY(int nCam, int nPos) { return m_dbRefPosY[nCam][nPos]; }
	double getRefT(int nCam, int nPos) { return m_dbRefPosT[nCam][nPos]; }
	void setRefX(int nCam, int nPos, double pos) { m_dbRefPosX[nCam][nPos] = pos; }
	void setRefY(int nCam, int nPos, double pos) { m_dbRefPosY[nCam][nPos] = pos; }
	void setRefT(int nCam, int nPos, double pos) { m_dbRefPosT[nCam][nPos] = pos; }

	double getFixtureX(int nCam, int nPos) { return m_dbFixturePosX[nCam][nPos]; }
	double getFixtureY(int nCam, int nPos) { return m_dbFixturePosY[nCam][nPos]; }
	double getFixtureT(int nCam, int nPos) { return m_dbFixturePosT[nCam][nPos]; }
	void setFixtureX(int nCam, int nPos, double pos) { m_dbFixturePosX[nCam][nPos] = pos; }
	void setFixtureY(int nCam, int nPos, double pos) { m_dbFixturePosY[nCam][nPos] = pos; }
	void setFixtureT(int nCam, int nPos, double pos) { m_dbFixturePosT[nCam][nPos] = pos; }

	int getRemoveNoiseSize(int nCam, int nPos, int nIndex) { return m_nRemoveNoiseSize[nCam][nPos][nIndex]; }
	int getSobelThreshold(int nCam, int nPos, int nIndex) { return m_nSobelThreshold[nCam][nPos][nIndex]; }
	int getMarkFindMethod() { return m_nMarkFindMethod; }
	int getInspFindMethod() { return m_nInspFindMethod; }

	BOOL getMarkReverseFind() { return m_bMarkReverseFind; }
	BOOL getNccModel() { return m_bNccModel; }
	void setNccModel(BOOL bSet) { m_bNccModel = bSet; }
	BOOL getMaskingModelUse() { return m_bMaskingModel; }
	void setMaskingModelUse(BOOL bSet) { m_bMaskingModel = bSet; }
	BOOL getUseMask(int nCam, int nPos, int nIndex) { return m_bUseMask[nCam][nPos][nIndex]; }
	void setUseMask(int nCam, int nPos, int nIndex, BOOL bUse) { m_bUseMask[nCam][nPos][nIndex] = bUse; }

	void setVerticalCamPos(BOOL bSet) { m_bVerticalCamPos = bSet; }
	void setSearchArea(int nCam, int nPos, int nIndex, int nArea) { m_nSearchArea[nCam][nPos][nIndex] = nArea; }
	void setProjectionArea(int nCam, int nPos, int nIndex, int nArea) { m_nProjectionArea[nCam][nPos][nIndex] = nArea; }
	void setRemoveNoiseSize(int nCam, int nPos, int nIndex, int nSize) { m_nRemoveNoiseSize[nCam][nPos][nIndex] = nSize; }
	void setSobelThreshold(int nCam, int nPos, int nIndex, int nThreshold) { m_nSobelThreshold[nCam][nPos][nIndex] = nThreshold; }
	void setHanconSearchAngleNstep(double angle, double step);
	void setMarkFindMethod(int nMethod) { m_nMarkFindMethod = nMethod; }
	void setInspFindMethod(int nMethod) { m_nInspFindMethod = nMethod; }
	void setMarkReverseFind(BOOL nMethod) { m_bMarkReverseFind = nMethod; }
	//void setWidth(int width) { m_nWidth = width; }
	//void setHeight(int height) { m_nHeight = height; }
	void setCornerType(int nCam, int nPos, int nValue) { m_nCornerType[nCam][nPos] = nValue; }
	void setEdgePolarity(int nCam, int nPos, int nPolar) { m_nEdgePolarity[nCam][nPos] = nPolar; }
	void setEdgeDirection(int nCam, int nPos, int nDir) { m_nEdgeDirection[nCam][nPos] = nDir; }
	void setPreProcess(int nCam, int nPos, int nPre) { m_nPreProcess[nCam][nPos] = nPre; }
	void setEdgeProcess(int nCam, int nPos, int nEdge) { m_nEdgeProcess[nCam][nPos] = nEdge; }

	int getCornerType(int nCam, int nPos) { return m_nCornerType[nCam][nPos]; }
	int getEdgePolarity(int nCam, int nPos) { return m_nEdgePolarity[nCam][nPos]; }
	int getEdgeDirection(int nCam, int nPos) { return m_nEdgeDirection[nCam][nPos]; }
	int getPreProcess(int nCam, int nPos) { return m_nPreProcess[nCam][nPos]; }
	int getEdgeProcess(int nCam, int nPos) { return m_nEdgeProcess[nCam][nPos]; }

	//int getWidth() { return m_nWidth; }
	//int getHeight() { return m_nHeight; }
	void setPosOffsetX(int nCam, int nPos, int nIndex, double offset) { m_dbPosOffsetX[nCam][nPos][nIndex] = offset; }
	void setPosOffsetY(int nCam, int nPos, int nIndex, double offset) { m_dbPosOffsetY[nCam][nPos][nIndex] = offset; }
	void setMarkOffsetX(int nCam, int nPos, int nIndex, double offset) { m_dbMarkOffsetX[nCam][nPos][nIndex] = offset; }
	void setMarkOffsetY(int nCam, int nPos, int nIndex, double offset) { m_dbMarkOffsetY[nCam][nPos][nIndex] = offset; }
	void setFinalMarkOffsetX(int nCam, int nPos, int nIndex, double offset) { m_dbFinalMarkOffsetX[nCam][nPos][nIndex] = offset; }
	void setFinalMarkOffsetY(int nCam, int nPos, int nIndex, double offset) { m_dbFinalMarkOffsetY[nCam][nPos][nIndex] = offset; }
	double getPosOffsetX(int nCam, int nPos, int nIndex) { return m_dbPosOffsetX[nCam][nPos][nIndex]; }
	double getPosOffsetY(int nCam, int nPos, int nIndex) { return m_dbPosOffsetY[nCam][nPos][nIndex]; }
	double getMarkOffsetX(int nCam, int nPos, int nIndex) { return m_dbMarkOffsetX[nCam][nPos][nIndex]; }
	double getMarkOffsetY(int nCam, int nPos, int nIndex) { return m_dbMarkOffsetY[nCam][nPos][nIndex]; }
	double getFinalMarkOffsetX(int nCam, int nPos, int nIndex) { return m_dbFinalMarkOffsetX[nCam][nPos][nIndex]; }
	double getFinalMarkOffsetY(int nCam, int nPos, int nIndex) { return m_dbFinalMarkOffsetY[nCam][nPos][nIndex]; }
	double getLowContrast(int nCam, int nPos, int nIndex) { return m_dbLowContrast[nCam][nPos][nIndex]; }
	double getHighContrast(int nCam, int nPos, int nIndex) { return m_dbHighContrast[nCam][nPos][nIndex]; }
	void setLowContrast(int nCam, int nPos, int nIndex, double lcont) { m_dbLowContrast[nCam][nPos][nIndex] = lcont; }
	void setHighContrast(int nCam, int nPos, int nIndex, double hcont) { m_dbHighContrast[nCam][nPos][nIndex] = hcont; }
	CRect getSearchROI(int nCam, int nPos) { return m_searchROI[nCam][nPos]; }
	CRect getSearchROI2(int nCam, int nPos, int nIndex) { return m_searchROI2[nCam][nPos][nIndex]; } // 20.03.26
	BOOL getUseSearchROI2(int nCam, int nPos, int nIndex) { return m_bUseSearchROI2[nCam][nPos][nIndex]; } // 20.03.26
	void setSearchROI(int nCam, int nPos, CRect rcROI) { m_searchROI[nCam][nPos] = rcROI; }
	void setSearchROI2(int nCam, int nPos, CRect rcROI, int nIndex) { m_searchROI2[nCam][nPos][nIndex] = rcROI; } // 20.03.26
	void setUseSearchROI2(int nCam, int nPos, int nIndex, BOOL bUse) { m_bUseSearchROI2[nCam][nPos][nIndex] = bUse; } // 20.03.26
	double getMatchingRate(int nCam, int nPos, int nIndex) { return m_dbMatchingRate[nCam][nPos][nIndex]; }
	void setMatchingRate(int nCam, int nPos, int nIndex, double dbMatchRate) { m_dbMatchingRate[nCam][nPos][nIndex] = dbMatchRate; }

	BOOL releaseHalconModel();
	BOOL readPatternData(bool breadmodel, CString strPath, int num_of_cmaera, int num_of_position, int m_nHalconMinContrast = -1, int m_nHalconHighContrast = -1, CString fileName = "PatternInfo.ini", bool binsp = false, bool binspPanel = false);
	BOOL savePatternData(CString strPath, CString fileName = "PatternInfo.ini", bool binsp = false);
	CFindInfo &getFindInfo(int nCam, int nPos) { return m_FindInfo[nCam][nPos]; }
	void setFindInfo(int nCam, int nPos, CFindInfo findInfo) { m_FindInfo[nCam][nPos] = findInfo; }
	CHalconProcessing & getHalcon(int nCam, int nPos, int nIndex) { return m_Halcon[nCam][nPos][nIndex]; }

	BOOL findPattern_Matching(BYTE* pImage, int nCam, int nPos, int m_nWidth, int m_nHeight, CFindInfo* pFindInfo = NULL);
	BOOL findPattern_Caliper(BYTE *pImage, int nCam, int nPos, int m_nWidth, int m_nHeight, CFindInfo *pFindInfo = NULL);
	BOOL findPattern_Ransac(BYTE *pImage, int nCam, int nPos, int m_nWidth, int m_nHeight, CFindInfo *pFindInfo = NULL);
	


	void decideSobelDirection(int nCornerType, int nEdgePolarity, int nEdgeDirection, int *pSobel_Hori, int *pSobel_Vert);
	BOOL findPattern_Ransac_Multiple(BYTE *pImage, int nCam, int nPos, CFindInfo *pFindInfo, int nIndex, int m_nWidth, int m_nHeight);

	BOOL findPattern(BYTE *pImage, int nCam, int nPos, int m_nWidth, int m_nHeight, CFindInfo *pFindInfo = NULL);
	BOOL findPatternIndex(BYTE *pImage, int nCam, int nPos, int m_nWidth, int m_nHeight, int nIndex, CFindInfo *pFindInfo = NULL);
	BOOL findPatternEdgeLine(BYTE *pImage, CRect rcInspROI, int nCam, int nPos, int m_nWidth, int m_nHeight, CFindInfo *pFindInfo = NULL);
	BOOL findMatchingEdgeLine(BYTE *pImage, CRect roi, int search_hori, int search_vert, int w, int h, bool bLeft, int nRemoveNoiseSize, int nSobelTheshold, int gauss, bool white);	// Matching 후에 Glass Edge 찾는 알고리즘

	CRect getMaskRect(int nCam, int nPos, int nIndex, int id) { return m_bMaskArea[nCam][nPos][nIndex][id]; }
	CRect* getMaskRect(int nCam, int nPos, int nIndex) { return m_bMaskArea[nCam][nPos][nIndex]; }
	void setMaskRect(int nCam, int nPos, int nIndex, int id, CRect roi) { m_bMaskArea[nCam][nPos][nIndex][id] = roi; }
	BOOL getMaskRectUse(int nCam, int nPos, int nIndex, int id) { return m_bMaskAreaUse[nCam][nPos][nIndex][id]; }
	BOOL* getMaskRectUse(int nCam, int nPos, int nIndex) { return m_bMaskAreaUse[nCam][nPos][nIndex]; }
	void setMaskRectUse(int nCam, int nPos, int nIndex, int id, BOOL use) { m_bMaskAreaUse[nCam][nPos][nIndex][id] = use; }
	BOOL getMaskRectShape(int nCam, int nPos, int nIndex, int id) { return m_bMaskAreaShape[nCam][nPos][nIndex][id]; }
	BOOL* getMaskRectShape(int nCam, int nPos, int nIndex) { return m_bMaskAreaShape[nCam][nPos][nIndex]; }
	void setMaskRectShape(int nCam, int nPos, int nIndex, int id, BOOL use) { m_bMaskAreaShape[nCam][nPos][nIndex][id] = use; }
	BOOL getTemplateAreaShape(int nCam, int nPos, int nIndex) { return m_bTemplateAreaShape[nCam][nPos][nIndex]; }
	void setTemplateAreaShape(int nCam, int nPos, int nIndex, BOOL brect) { m_bTemplateAreaShape[nCam][nPos][nIndex] = brect; }

	void sobelDirection(int nMaskSize, int nDirection, IplImage *srcImage, IplImage *dstImage);
	void applyLUT(IplImage * srcImage, IplImage * dstImage, int(&LUT)[256]);
	void calcHistogram(IplImage *srcImage, int(&Histo)[256]);

	void applyLUT(BYTE * srcImage, BYTE * dstImage, int w, int h, int(&LUT)[256]);
	void calcHistogram(BYTE *srcImage, int w, int h, int(&Histo)[256]);

	void LeastSqrRegression(double* x, double* y, int obs, double &a, double &b);
	void endInSearch(cv::Mat *matsrcImage, cv::Mat *matdstImage);
	void endInSearch(IplImage * srcImage, IplImage * dstImage);
	void removeNoise(cv::Mat matImage, int nSize);
	void removeNoise(IplImage *pImage, int nSize);
	void DoRemoveNoise(int dMode, int width, int height, int widthStep, int i, int j, int threshold, int &hole, BOOL &is_hole, unsigned char *image, unsigned char *label_image);
	bool GetIntersectPoint(const UPoint <double> & AP1, const UPoint <double>& AP2, const UPoint <double>& BP1, const UPoint <double>& BP2, UPoint <double>* IP);

	void compute_orthogonal_point_on_line(sLine &line, sPoint &x, sPoint &point);
	void cramersRules(double a, double b, double c, double d, double p, double q, double *x, double *y);
	double ransac_line_fitting(sPoint *data, int no_data, sLine &model, double distance_threshold);
	double model_verification(sPoint *inliers, int *no_inliers, sLine &estimated_model, sPoint *data, int no_data, double distance_threshold);
	double compute_distance(sLine &line, sPoint &x);
	int compute_model_parameter(sPoint samples[], int no_samples, sLine &model);
	void get_samples(sPoint *samples, int no_samples, sPoint *data, int no_data);
	bool find_in_samples(sPoint *samples, int no_samples, sPoint *data);
	BOOL findCornerEdgeByRansac(BYTE *pImage, int width, int height, int nQuadrant, int nSobelDirectionHori, int nSobelDirectionVert, int nNoiseSize_Hori, int nNoiseSize_Vert, int nThreshold_Hori, int nThreshold_Vert, CRect rcROI, int nHoriSearchArea, int nVertSearchArea, sLine *pLineHori, sLine *pLineVert, int nSearchOffset, BOOL bInnerToOuter, int nCam, int nPos);
	BOOL findCornerEdgeByRansac2(BYTE *pImage, int width, int height, int nQuadrant, int nSobelDirectionHori, int nSobelDirectionVert, int nNoiseSize_Hori, int nNoiseSize_Vert, int nThreshold_Hori, int nThreshold_Vert, CRect rcROI, int nHoriProjectionArea, int nVertProjectionArea, int nHoriSearchArea, int nVertSearchArea, sLine *pLineHori, sLine *pLineVert, int nSearchOffset, BOOL bInnerToOuter, int nCam, int nPos);

	BOOL findVerticalEdgeByRansac(BYTE *pOrgImage, int width, int height, int nCornerType, int nSobelDirectionVert, int nNoiseSize_Vert, int nThreshold_Vert, CRect rcROI, int nVertProjectionArea, int nVertSearchArea, sLine *pLineVert, int nSearchOffset, BOOL bInnerToOuter, int nCam, int nPos);

	BOOL findActiveEdgeByRansac(BYTE *pImage, int width, int height, int nDirection, int nSearchRange, int nRemoveNoiseSize, int nThreshold, cv::Rect rcROI, sLine *pLine);
	BOOL findSusEdgeByRansac(BYTE *pImage, int width, int height, int nDirection, int nSearchRange, int nRemoveNoiseSize, int nThreshold, cv::Rect rcROI, sLine *pLine);
	BOOL findMoldingEdgeByRansac(BYTE *pImage, int width, int height, int nDirection, int nSearchRange, int nRemoveNoiseSize, int nThreshold, cv::Rect rcROI, sLine *pLine);
	BOOL findEdgeByRansac(BYTE *pImage, int width, int height, int nSide, int nSearchRange, int nSearchDir, int nSearchPosOffset, int nRemoveNoiseSize, int nThreshold, cv::Rect rcROI, sLine *pLine);

	void setRealROI(CRect *pRectHori, CRect *pRectVert, int width, int height, int nQuadrant, CRect rcROI, int nHoriSearchArea, int nVertSearchArea, int nSearchOffset = 0);
	void setRealROIVert(CRect *pRectHori, CRect *pRectVert, int width, int height, int nQuadrant, CRect rcROI, int nHoriSearchArea, int nVertSearchArea, int nSearchOffset = 0);
	void setRealROIVert2(CRect *pRectHori, CRect *pRectVert, int width, int height, int nQuadrant, CRect rcROI, int nHoriProjectionArea, int nVertProjectionArea, int nHoriSearchArea, int nVertSearchArea, int nSearchOffset = 0);

	void calcLineInfo(int nCam, int nPos, CRect rectROI, int m_nWidth, int m_nHeight);
	cv::Rect findChipContour(BYTE *pImage, int m_nWidth, int m_nHeight);

	void preProcessImage(int nCam, int nPos, IplImage *pImg);
	void filterProcessImage_hori(int nCam, int nPos, IplImage *pImg, cv::Mat *pMatImg, int nSobelDirection, int nThreshold);
	void filterProcessImage_vert(int nCam, int nPos, IplImage *pImg, cv::Mat *pMatImg, int nSobelDirection, int nThreshold);
	void InspectionEnhance(cv::Mat* src, cv::Mat* dst, int id, bool disp = true);
	void InspectionEnhance_xy7(cv::Mat *src, cv::Mat *dst);

	BOOL findHisto_Hori(int nCam, IplImage *pImg, int nSize, int nSearchArea, int nCornerType, cv::Rect cvRectROI, BOOL bInnerToOuter, sLine *pLine);
	BOOL findHisto_Vert(int nCam, IplImage *pImg, int nSize, int nSearchArea, int nCornerType, cv::Rect cvRectROI, BOOL bInnerToOuter, sLine *pLine);

	BOOL findLine_Hori(int nCam, IplImage *pImg, int nSize, int nSearchArea, int nCornerType, cv::Rect cvRectROI, BOOL bInnerToOuter, sLine *pLine);
	BOOL findLine_Vert(int nCam, IplImage *pImg, int nSize, int nSearchArea, int nCornerType, cv::Rect cvRectROI, BOOL bInnerToOuter, sLine *pLine);

	sLine m_lineHori[MAX_CAMERA][NUM_POS];
	sLine m_lineVert[MAX_CAMERA][NUM_POS];

	sLine m_lineDiagonal[MAX_CAMERA][NUM_POS];

	int m_nUseSearchROI2Count;
	int getUseSearchROI2Count() { return m_nUseSearchROI2Count; }	// 20.03.29 	
	void init_job(int nJob);

};
