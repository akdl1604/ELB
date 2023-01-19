#pragma once
#include <stdafx.h>
#include <opencv.hpp>
#include <opencv2/features2d.hpp>

struct _stDefectInfo {
	int s_iImageIndex;
	double s_dArea;					// 크기
	double s_dCircularity;			// 진원도
	double s_dInertia;				// 불량의 
	double s_dConvexity;			// 볼록 정도
	CPoint s_ptLocation;			// 좌표
	double s_dRadius;				// 반지름
	double s_dConfidence;			// 
	int s_iEnhanceMin;
	int s_iEnhanceMax;
	int s_iOriginalMin;
	int s_iOriginalMax;
	double s_dEnhanceAvg;
	double s_dOriginalAvg;
	int s_iSideEnhanceMin;
	int s_iSideEnhanceMax;
	int s_iSideOriginalMin;
	int s_iSideOriginalMax;
	double s_dSideEnhanceAvg;
	double s_dSideOriginalAvg;
	double s_dEnhanceAreaMin;
	double s_dEnhanceAreaMax;
	double s_dOrginAreaMin;
	double s_dOrginAreaMax;
	double s_dEnhanceWidth;
	double s_dEnhanceHeight;
	double s_dEspectRatio;
	BOOL s_bPassDefect;
	int s_iPassCode;
	double s_dSideLeftAvg;
	double s_dSideRightAvg;
	double s_dDispensingAvg;
	double s_dSideEnhanceLeftAvg;
	double s_dSideEnhanceRightAvg;
	double s_dContrast;
	BOOL s_bOverflowDefect;
};

struct Center
{
	cv::Point2d location;
	double radius;
	double confidence;
};

class BetterBlobDetector : public cv::SimpleBlobDetector
{
public:
	BetterBlobDetector(const cv::SimpleBlobDetector::Params &parameters = cv::SimpleBlobDetector::Params());
	int m_bFind;
	const std::vector < std::vector<cv::Point> > getContours();
	const std::vector < _stDefectInfo > getDefectInfo();
protected:
	virtual void detectImpl( const cv::Mat& image, std::vector<cv::KeyPoint>& keypoints, const cv::Mat& mask=cv::Mat()) const;
	virtual void findBlobs(const cv::Mat &image, const cv::Mat &binaryImage,
		std::vector<Center> &centers, std::vector < std::vector<cv::Point> >&contours,
		std::vector<_stDefectInfo> &curDefectInfo ) const;

private:
	SimpleBlobDetector::Params params;
};


#define BLOBERR_NONE		0x0000
#define BLOBERR_MAXLINE		0x00000001
#define BLOBERR_MAXLINK		0x00000002
#define BLOBERR_MAXINDEX	0x00000004
#define BLOBERR_MAXBLOB		0x00000008

typedef struct _STLINE
{
	WORD y;
	WORD x1;
	WORD x2;
	WORD index;
} STLINE, * PSTLINE;

typedef struct _STINDEX
{
	WORD LinkPtr;
	WORD label;
	WORD check;
} STINDEX, * PSTINDEX;

typedef struct _STBLOB
{
	WORD top;
	WORD bottom;
	WORD left;
	WORD right;    // (real right + 1)
	DWORD pxlSize;
	POINT seed;    // Blob 영역의 
	POINT centerOfMass;
	double orientation;  // 수직(12시방향)을 기준으로 시계방향으로 기울어진 각도(degree)
	double Sx;
	double Sy;
	double Sxx;
	double Syy;
	double Sxy;
	double strength;
	WORD label;
	POINT rectpoint[4];
	//BOOL valid;
	int state;
} STBLOB, * PSTBLOB;

class CMBlob
{
private:
	int m_ImgWidth;
	int m_ImgHeight;
	int m_StepWidth;
	int m_ByteDepth;
	int m_RoiLeft;
	int m_RoiTop;
	int m_RoiRight;
	int m_RoiBottom;

	int m_MaxLine;
	int m_NumLine;
	PSTLINE m_aPtrLine;

	int m_MaxIndex;
	int m_NumIndex;
	PSTINDEX m_aPtrIndex;

	int m_MaxBlob;
	int m_NumBlob;       // 1-based Blob Number
	PSTBLOB m_aPtrBlob;  // 1-based Blob Number

	LPWORD m_aMap;
	BOOL m_bCheckDone;   //  CheckAcceptable() 함수의 호출 여부
	BOOL* m_bRegBlobSeed; //  Register Blob Seed

public:
	CMBlob();
	~CMBlob();
	CMBlob(int maxline, int maxblob, int maxindex);

	void Initialize();
	void InitValue();
	BOOL Create(int maxline, int maxblob, int maxindex);
	void Free();

	void SetImgInfo(int width, int height, int byteDepth, int stepwidth);
	int BlobScan(LPBYTE pData);
	void SetRoi(LPRECT pRect);
	void DrawLabel(LPBYTE pData);
	void ExtractBlob();
	void EraseLabel(LPBYTE pData);
	PSTBLOB GetBlob(int ind);
	void SetBlobStatus(int ind, int status);  // BOOL bValid
	void SortingBlob();
	void SetRealPoint(LPPOINT pPoint, int pos);

	int CheckAcceptable(LPBYTE pData);
	void DrawCheck(LPBYTE pData, int numdraw);
	//////////////////////////////////////////////////////

	int GetBlobNum() { return m_NumBlob; };
	int GetLineNum() { return m_NumLine; };

	int ExtractLine(LPBYTE pData);
	PSTLINE GetLine(int ind);
	BOOL ExtractBlobFromCheck(int numBlob);
	void DrawBlob(LPBYTE pData, int lbl, BOOL bOriginLeftBottom = TRUE); // ID = lbl인 blob을 255로 칠하기..

	void SetBlobChangeValue(int ind, int l, int t, int r, int b, int s);
	void AddBlob(int l, int t, int r, int b, int s);
	void BlobSizeControl(int ind, int size);
private:
	int IndexLine();
	int LabelBlobObj();

	BOOL PointInPolygon(float testx, float testy);
	float m_pVertx[4], m_pVerty[4];
	POINT m_pPtRect[4];
};