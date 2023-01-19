#pragma once
#include <vector>

using namespace std;
// CDlgCaliper 대화 상자입니다.
#define MAX_CALIPER 360

#ifndef PI
#define PI 3.141592 
#endif

#ifndef _ST32FPOINT_
#define _ST32FPOINT_

struct _st32fPoint {
	double x;
	double y;
};
#endif

struct _CALIPER_FIND_INFO {
	BOOL find;
	BOOL ignore;
	double x;
	double y;
	int contrast;
	double dist;
};

struct _CALIPER_AREA_INFO {
	cv::RotatedRect rect;
	BOOL use;
};

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
	sPoint pt[2];	// [START/END]
};
#endif

#ifndef _S_CIRCLE_
#define _S_CIRCLE_
struct sCircle {
	double x, y, r;
};
#endif

// CCaliper

enum CALIPER_SEARCH_TYPE {
	SEARCH_TYPE_FIRST = 0,
	SEARCH_TYPE_END,
	SEARCH_TYPE_PEAK,
	SEARCH_TYPE_POS,
	SEARCH_TYPE_PEAK_POS,
};

struct sEllipse;


// CCaliper
class CCaliper: public CWnd
{
	DECLARE_DYNAMIC(CCaliper)

public:
	CCaliper();
	virtual ~CCaliper();

protected:
	DECLARE_MESSAGE_MAP()
private:
	BOOL m_bIsMakeLine;
	FakeDC *m_pDC = NULL;
	
	double m_dbFixtureX;
	double m_dbFixtureY;
	double m_dbFixtureT;

	double **m_dbLineX = NULL;
	double **m_dbLineY = NULL;
	double **m_dbContrast = NULL;
	CvPoint2D32f ***m_ptCaliper_LUT = NULL;
	CvPoint2D32f ***m_ptCaliperCircle_LUT = NULL;
	
	_st32fPoint m_ptOrgStart;
	_st32fPoint m_ptOrgEnd;
	_st32fPoint m_ptOrgMid;
	
	_st32fPoint m_ptStart;
	_st32fPoint m_ptEnd;
	_st32fPoint m_ptMid;

	double m_dbLineT;
	double m_dbLineNormalT;

	int m_nProjectionLength_OLD;
	int m_nSearchLength_OLD;
	int m_nNumOfCaliper_OLD;
	int m_nProjectionLengthCircle_OLD;
	int m_nSearchLengthCircle_OLD;
	int m_nNumOfCaliperCircle_OLD;
	int m_nLastNumOfCaliper;
	/*vector<struct _CALIPER_FIND_INFO> m_vtCaliperFindInfo;
	vector<struct _CALIPER_AREA_INFO> m_vtCaliperAreaInfo;	*/
	vector<double> m_pVtLineX[MAX_CALIPER];
	vector<double> m_pVtLineY[MAX_CALIPER];
	vector<double> m_pVtLineContrast[MAX_CALIPER];
	struct _CALIPER_FIND_INFO m_FindInfo[MAX_CALIPER];
	struct _CALIPER_AREA_INFO m_AreaInfo[MAX_CALIPER];
	BOOL m_bUseCaliper[MAX_CALIPER];
	BOOL m_bCircleCaliper;
	BOOL m_bDirectionSwap;
	int m_nThreshold;
	int m_nSearchType;
	int m_nProjectionLength;
	int m_nSearchLength;
	int m_nNumOfCalipers;
	int m_nContrastDirection;
	int m_nEachCaliperSearchType;	// 0: avg, 1: median, 2: peak cut
	int m_nHalfFilterSize;
	int m_nNumOfIgnore;
	int m_nFittingMethod;
public:

	double get_line_x(int caliper, int nIndex) { return m_dbLineX[caliper][nIndex]; }
	double get_line_y(int caliper, int nIndex) { return m_dbLineY[caliper][nIndex]; }
	_CALIPER_FIND_INFO get_find_info(int caliper) { return m_FindInfo[caliper]; }

	sLine m_lineInfo;
	sCircle m_circle_info;
	void setIsMakeLine(BOOL bMake) { m_bIsMakeLine = bMake;; }
	BOOL getIsMakeLine() { return m_bIsMakeLine ; }

	_CALIPER_FIND_INFO &getFindInfo(int nCaliper) { return m_FindInfo[nCaliper]; }
	BOOL getUseCaliper(int nIndex) { return m_bUseCaliper[nIndex]; }
	void setUseCaliper(int nIndex, BOOL bUse) { m_bUseCaliper[nIndex] = bUse; }

	FakeDC *getCaliperDC() { return m_pDC; }
	void setCaliperDC(FakeDC*pDC) { m_pDC = pDC; }

	double getFixtureX() { return m_dbFixtureX; }
	double getFixtureY() { return m_dbFixtureY; }
	double getFixtureT() { return m_dbFixtureT; }

	struct _st32fPoint getOrgStartPt() { return m_ptOrgStart; }
	struct _st32fPoint getOrgEndPt() { return m_ptOrgEnd; }
	struct _st32fPoint getOrgMidPt() { return m_ptOrgMid; }
	struct _st32fPoint getStartPt() { return m_ptStart; }
	struct _st32fPoint getEndPt() { return m_ptEnd; }
	struct _st32fPoint getMidPt() { return m_ptMid; }

	double getLineT() { return m_dbLineT; }
	double getLineNormalT() { return m_dbLineNormalT; }
	int getProjectionLength() { return m_nProjectionLength; }
	int getSearchLength() { return m_nSearchLength; }
	int getNumOfCalipers() { return m_nNumOfCalipers; }
	int getNumOfIgnore() { return m_nNumOfIgnore; }
	int getContrastDirection() { return m_nContrastDirection; }
	int getSearchType() { return m_nSearchType; }
	int getThreshold() { return m_nThreshold; }
	BOOL getCircleCaliper() { return m_bCircleCaliper; }
	int getHalfFilterSize() { return m_nHalfFilterSize; }
	BOOL getDirectionSwap() { return m_bDirectionSwap; }
	int getEachCaliperSearchType() { return m_nEachCaliperSearchType; }
	int getFittingMethod() { return m_nFittingMethod; }

	void setFixtureX(double value) { m_dbFixtureX = value; }
	void setFixtureY(double value) { m_dbFixtureY = value; }
	void setFixtureT(double value) { m_dbFixtureT = value; }
	
	void setOrgStartPt(struct _st32fPoint pt) { m_ptOrgStart = pt; }
	void setOrgEndPt(struct _st32fPoint pt) { m_ptOrgEnd = pt; }
	void setOrgMidPt(struct _st32fPoint pt) { m_ptOrgMid = pt; }
	void setStartPt(struct _st32fPoint pt) { m_ptStart = pt; }
	void setEndPt(struct _st32fPoint pt) { m_ptEnd = pt; }
	void setMidPt(struct _st32fPoint pt) { m_ptMid = pt; }
	void setLineT(double t) { m_dbLineT = t; }
	void setLineNormalT(double t) { m_dbLineNormalT = t; }
	void setProjectionLength(int length) { m_nProjectionLength = length; }
	void setSearchLength(int length) { m_nSearchLength = length; }
	void setNumOfCalipers(int num) { m_nNumOfCalipers = num; }
	void setNumOfIgnore(int num) { m_nNumOfIgnore = num; }
	void setContrastDirection(int dir) { m_nContrastDirection = dir; }
	void setSearchType(int type) { m_nSearchType = type; }
	void setThreshold(int thresh) { m_nThreshold = thresh; }
	void setCircleCaliper(BOOL set) { m_bCircleCaliper = set; }
	void setHalfFilterSize(int size) { m_nHalfFilterSize = size; }
	void setDirectionSwap(BOOL set) { m_bDirectionSwap = set; }
	void setEachCaliperSearchType(int type) { m_nEachCaliperSearchType = type; }
	void setFittingMethod(int method) { m_nFittingMethod = method; }


	////////// Ransac ////////////
	double model_verification (sPoint *inliers, int *no_inliers, sLine &estimated_model, sPoint *data, int no_data, double distance_threshold);
	double compute_distance(sLine &line, sPoint &x);
	int compute_model_parameter(sPoint samples[], int no_samples, sLine &model);
	void get_samples (sPoint *samples, int no_samples, sPoint *data, int no_data);
	bool find_in_samples (sPoint *samples, int no_samples, sPoint *data);
	double ransac_line_fitting(sPoint *data, int no_data, sLine &model, double distance_threshold);
	double mlesac_line_fitting(sPoint* data, int no_data, sLine& model, double distance_threshold);
	///////////////////////////////
	static 	void cramersRules(double a, double b, double c, double d, double p, double q, double *x, double *y);
	

	void calcPointsByFixture();
	void calcOrgPointsFromOffsetPoints();

	void processCaliper(IplImage *pImg);
	void processCaliper(BYTE *pImg, int width, int height);
	void processCaliper(BYTE *pImg, int width, int height, double offsetX, double offsetY, double offsetT, BOOL bDraw = FALSE);
	void processCaliper_circle(BYTE *pImg, int width, int height, double offsetX, double offsetY, double offsetT, BOOL bDraw = FALSE);

	void decide_position_each_caliper_circle();											// 각 Caliper에서의 대표값을 결정한다.
	void decide_data_to_ignore_circle();
	void make_circle_from_caliper_info(FakeDC*pDC, int width, int height, BOOL bDraw = FALSE);

	void searchCandidateEachCaliper(BYTE *pImg, int width, int height);	// 각 Caliper에서의 후보군들을 찾는다.
	void search_candidate_each_caliper_circle(BYTE *pImg, int width, int height);		// 각 Caliper에서의 후보군들을 찾는다.
	void decidePositionEachCaliper();									// 각 Caliper에서의 대표값을 결정한다.
	void decideDataToIgnore();
	void decidePositionEachCaliperCircle();									// 각 Caliper에서의 대표값을 결정한다.
	void makeLineFromCaliperInfo(IplImage *pImg);
	void makeLineFromCaliperInfo(FakeDC*pDC, int width, int height, BOOL bDraw = FALSE);
	void calcCaliperRect();												// Caliper의 정보를 계산한다.
	void calcCaliperCircle();											// Caliper의 정보를 계산한다.
	void DrawCaliper(IplImage *pImg);
	void DrawCaliper(FakeDC*pDC);
	void DrawCaliper_Circle(IplImage *pImg);
	void DrawCaliper_Circle(FakeDC*pDC);

	void draw_final_result(FakeDC *pDC);
	void draw_final_result(cv::Mat &Img);

	int mlesac_circle_fitting(sPoint samples[], int N, double* cx, double* cy, double* rad);
	int circle_fit_ls(sPoint samples[], int N, double* cx, double* cy, double* rad);
	bool slove_linear_eq3x3(double A[9], double bb[3], double x[3]);


	int compute_ellipse_model_parameter(sPoint samples[], int no_samples, sEllipse &model);
	double ransac_ellipse_fitting(sPoint *data, int no_data, sEllipse &model, double distance_threshold);
	double compute_ellipse_distance(sEllipse &ellipse, sPoint &p);
	double ellipse_model_verification(sPoint *inliers, int *no_inliers, sEllipse &estimated_model, sPoint *data, int no_data, double distance_threshold);

	void init_caliper_info();
	void alloc_caliper_memory();
	void release_caliper_memory();
	void reset_find_info();
	void reset_caliper_memory();
	void copy_caliper_info(CCaliper *caliper_info);

};


