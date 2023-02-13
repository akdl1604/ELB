// Util.h
#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <array>
#include <chrono>

using namespace chrono;
using namespace std;
using namespace cv;

typedef std::vector<std::array<float, 2>> TPointList2D;
typedef std::vector<std::array<float, 5>> TPointList5D;

// Lincoln Lee - 220725 - Adding BMIN inspection at PF inspection
enum class LIGHTING_POLARITY
{
    ANY_POLARITY,
    DARK_TO_LIGHT,
    LIGHT_TO_DARK
};

struct CfPt {
    double x, y;
    CfPt() : x(0), y(0) { }
    CfPt(double num1, double num2) : x(num1), y(num2) { }
    double DistanceTo(CfPt P) { return sqrt((P.x - x) * (P.x - x) + (P.y - y) * (P.y - y)); }
    CfPt operator+(CfPt& pt) const {   CfPt _pt(x + pt.x, y+ pt.y);    return _pt;  }
    CfPt operator-(CfPt& pt) const {   CfPt _pt(x - pt.x, y - pt.y);    return _pt; }
};

// Lincoln Lee - 220725 - Adding BMIN inspection at PF inspection
std::vector<cv::Point> circularScanning(const cv::Mat& src, cv::Point2f center, float startRadius, float endRadius, float deltaTheta, int thresh, LIGHTING_POLARITY polarity = LIGHTING_POLARITY::ANY_POLARITY);
std::vector<cv::Point> circularScanning(const cv::Mat& src, cv::Point2f center, float startRadius, float endRadius, float deltaTheta, int thresh, cv::Point offs, LIGHTING_POLARITY polarity = LIGHTING_POLARITY::ANY_POLARITY, UINT* valids = NULL);
std::vector<cv::Point> circularScanning(const cv::Mat& src, cv::Point2f center, float startRadius, float endRadius, float deltaTheta, int thresh, cv::Point offs, std::vector<bool>& founds, LIGHTING_POLARITY polarity = LIGHTING_POLARITY::ANY_POLARITY, UINT* valids = NULL);

// Lincoln Lee - 220725 - Adding BMIN inspection at PF inspection
template<class T1, class T2>
CRect GetRect(T1 cx, T2 cy, float radius)
{
    return CRect(lround(cx - radius), lround(cy - radius), lround(cx + radius), lround(cy + radius));
}

// Lincoln Lee - 220725 - Adding BMIN inspection at PF inspection
template<class T0>
CRect GetRect(cv::Point_<T0> pt, float radius)
{
    return CRect(lround(pt.x - radius), lround(pt.y - radius), lround(pt.x + radius), lround(pt.y + radius));
}

// Lincoln Lee - 220725 - Adding BMIN inspection at PF inspection
template<class T0>
void PlotPoints(CDC* dc, const std::vector<cv::Point_<T0>>& pts, CPen* pen, cv::Point offs)
{
    auto ptr = dc->SelectObject(pen);
    if ((NULL == dc) || (pts.size() > 0))
    {
        for (const auto& pt: pts)
            dc->Ellipse(GetRect(pt.x + offs.x, pt.y + offs.y, 2));
    }
    dc->SelectObject(ptr);
}

// Lincoln Lee - 220725 - Adding BMIN inspection at PF inspection
template<class T0>
void PlotPoints(CDC* dc, const std::vector<cv::Point_<T0>>& pts, CPen* pen)
{
    auto ptr = dc->SelectObject(pen);
    if ((NULL == dc) || (pts.size() > 0))
    {
        for (const auto& pt: pts) dc->Ellipse(GetRect(pt.x, pt.y, 2));
    }
    dc->SelectObject(ptr);
}

// Lincoln Lee - 220725 - Adding BMIN inspection at PF inspection
bool InspFilm_BMIN_check(const cv::Mat& src, cv::Rect roi, double mmScale = 0.010210f, double fistSpec = 0.1f, double secondSpec = 0.06f, CDC* pDC = NULL);

template<typename T>
class CPointF
{
public:
    T x;
    T y;

public:
    CPointF() : x(0), y(0) { }
    CPointF(T _x, T _y) : x(_x), y(_y) { }
};

template<typename T1, typename T2>
double GetDistance(const T1& p1, const T2& p2)
{
    return std::sqrt(double((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y)));
}

// Lincoln Lee - 220210
template<class T1, class T2, class T3, class T4, class T5>
void GetCircle(const cv::Point_<T1>& p1, const cv::Point_<T2>& p2, const cv::Point_<T3>& p3, cv::Point_<T4>& center, T5& radius)
{
    // PLEASE CHECK FOR TYPOS IN THE FORMULA :)
    center.x = (p1.x * p1.x + p1.y * p1.y) * (p2.y - p3.y) + (p2.x * p2.x + p2.y * p2.y) * (p3.y - p1.y) + (p3.x * p3.x + p3.y * p3.y) * (p1.y - p2.y);
    center.x /= (2. * (p1.x * (p2.y - p3.y) - p1.y * (p2.x - p3.x) + p2.x * p3.y - p3.x * p2.y));

    center.y = (p1.x * p1.x + p1.y * p1.y) * (p3.x - p2.x) + (p2.x * p2.x + p2.y * p2.y) * (p1.x - p3.x) + (p3.x * p3.x + p3.y * p3.y) * (p2.x - p1.x);
    center.y /= (2. * (p1.x * (p2.y - p3.y) - p1.y * (p2.x - p3.x) + p2.x * p3.y - p3.x * p2.y));

    radius = sqrt((center.x - p1.x) * (center.x - p1.x) + (center.y - p1.y) * (center.y - p1.y));
}

// Lincoln Lee - 220210
template<class T1, class T2>
int EvaluateCircle(const std::vector<cv::Point_<T1>>& pts, const cv::Point_<T2>& cr, float radius, float term = 0.2f)
{
    int count = 0;
    for (const auto& pt : pts)
        if (std::abs(GetDistance(pt, cr) - radius) < term) ++count;

    return count;
}

template<class T>
void getCircle(cv::Point_<T>& p1, cv::Point_<T>& p2, cv::Point_<T>& p3, cv::Point2f& center, float& radius)
{
    float x1 = p1.x;
    float x2 = p2.x;
    float x3 = p3.x;

    float y1 = p1.y;
    float y2 = p2.y;
    float y3 = p3.y;

    // PLEASE CHECK FOR TYPOS IN THE FORMULA :)
    center.x = (x1 * x1 + y1 * y1) * (y2 - y3) + (x2 * x2 + y2 * y2) * (y3 - y1) + (x3 * x3 + y3 * y3) * (y1 - y2);
    center.x /= (2 * (x1 * (y2 - y3) - y1 * (x2 - x3) + x2 * y3 - x3 * y2));

    center.y = (x1 * x1 + y1 * y1) * (x3 - x2) + (x2 * x2 + y2 * y2) * (x1 - x3) + (x3 * x3 + y3 * y3) * (x2 - x1);
    center.y /= (2 * (x1 * (y2 - y3) - y1 * (x2 - x3) + x2 * y3 - x3 * y2));

    radius = sqrt((center.x - x1) * (center.x - x1) + (center.y - y1) * (center.y - y1));
}

// Lincoln Lee - 220210
// BestFitCircle with radius contrains
template<class T1, class T2>
bool BestFitCircleX(const std::vector<cv::Point_<T1>>& pts, cv::Point_<T2>& Center, double& Radius, float dError = 0.2f, int Epochs = 20)
{
    int len = int(pts.size());
    int epochNo = 0;
    int bestCount = 0;
    bool found = FALSE;
    double radius, expRadius = Radius; // Expectation radius

    cv::Point2d center;

    Center.x = T2(DBL_MIN);
    Center.x = T2(DBL_MIN);
    Radius = DBL_MIN;

    if (len < 3) return false;

    std::srand(len);
    while (epochNo < Epochs)
    {
        unsigned int idx1 = std::rand() % len;
        unsigned int idx2 = std::rand() % len;
        unsigned int idx3 = std::rand() % len;

        if (idx1 == idx2) continue;
        if (idx1 == idx3) continue;
        if (idx3 == idx2) continue;

        GetCircle(pts[idx1], pts[idx2], pts[idx3], center, radius);
        if (std::abs(radius - expRadius) > dError)
        {
            ++epochNo;
            continue;
        }

        int count = EvaluateCircle(pts, center, radius, dError);
        if ((count > bestCount) && (std::abs(radius - expRadius) < dError))
        {
            Center.x = center.x;
            Center.y = center.y;
            Radius = radius;
            bestCount = count;
            found = TRUE;
        }

        ++epochNo;
    }
    return found;
}

template <typename T>
inline T GetIndexClamped(const std::vector<T>& points, int index)
{
	if (index < 0)		return points[0];
	else if (index >= int(points.size()))		return points.back();
	else		return points[index];
}


struct contourStats {
    double area, perimeter, axisMin, axisMax, axisAvg, eccentricity, axisRatio, circularity;
    bool bdefect;
    double xw;
    double yw;

    Point2d center;
    RotatedRect rr;

    contourStats() {}
    contourStats(const vector<cv::Point>& contour)
    {
        calculateStats(contour);
    }
    void calculateStats(const vector<cv::Point>& contour)
    {
        Moments m = cv::moments(contour, true);
        area = m.m00;
        center = Point2f(-1, -1);
        if (area > 0) {
            center.x = cvRound(m.m10 / m.m00);
            center.y = cvRound(m.m01 / m.m00);
        }
        eccentricity = DBL_MAX;
        if ((m.m20 + m.m02) > 0)
            eccentricity = (pow((m.m20 - m.m02), 2) - 4 * m.m11 * m.m11) / pow((m.m20 + m.m02), 2);
        // axis ratio:circles have ratio=1 lines have ratio->0
        rr = minAreaRect(contour);
        axisMin = min(rr.size.height, rr.size.width);
        axisMax = max(rr.size.height, rr.size.width);
        axisAvg = (axisMin + axisMax) / 2.0;
        axisRatio = axisMax > 0 ? axisMin / axisMax : 0;
        perimeter = arcLength(contour, false);
        circularity = perimeter > 0 ? 4 * CV_PI * area / pow(perimeter, 2) : 0;
        xw = rr.size.width;
        yw = rr.size.height;
    }
    void printStats(const string& title)
    {
        cout << endl << title << endl
            << "\tArea [px^2]: " << area << endl
            << "\tAxis[px]: " << axisMin << "/" << axisMax << endl
            << "\tAxis Ratio: " << axisRatio << endl
            << "\tCircularity: " << circularity << endl
            << "\tAbs(eccentricity): " << abs(eccentricity) << endl;
    }
};

namespace Peaks {
    const float EPS = 2.2204e-16f;
    void findPeaks(vector<float> x0, vector<int>& peakInds);
}

class Histogram1D 
{
private: 
    int histSize[1]; // 히스토그램의 빈 개수 
    float hranges[2]; // 히스토그램 값의 범위 
    const float* ranges[1]; // 값 범위를 가리키는 포인터 
    int channels[1]; // 조사할 채널 번호 
public: 
    Histogram1D() 
    { 
        histSize[0]= 256; // 256개의 빈 
        hranges[0]= 0.0; // 0부터 (포함) 
        hranges[1]= 256.0; // 256까지 (제외) 
        ranges[0]= hranges; 
        channels[0]= 0; // 채널 0에서 봄 
    } 

      void setChannel(int c) { channels[0]= c; } 
      int getChannel() { return channels[0]; } 
      void setRange(float minValue, float maxValue) 
      { 
          hranges[0]= minValue; 
          hranges[1]= maxValue; 
      } 

      float getMinValue() { return hranges[0]; } 
      float getMaxValue() { return hranges[1]; } 
      void setNBins(int nbins) { histSize[0]= nbins; } 
      int getNBins() { return histSize[0]; } 

      // 1D 히스토그램 계산 
      cv::Mat getHistogram(const cv::Mat &image) 
      { 
          cv::Mat hist; 
          cv::calcHist(&image, 1, channels, cv::Mat(), hist, 1, histSize, ranges); 
          return hist; 
      } 
      // 1D 히스토그램을 계산한 후 히스토그램 영상으로 변환 
      cv::Mat getHistogramImage(const cv::Mat &image, int zoom = 1)
      {          
          // 히스토그램 계산 
          cv::Mat hist = getHistogram(image); // 반환할 영상 생성 
          return Histogram1D::getImageOfHistogram(hist, zoom); 
      } 
      // 히스토그램을 표현하는 영상 생성 ( 정적 메소드 ) 
      static cv::Mat getImageOfHistogram(const cv::Mat &hist, int zoom)
      { 
          double textScale = 1.0;
          cv::Size divNum(10, 10);
          int guideLineClr = 150;
          int backClr = 1;
          bool bFill = true;

        // 최소, 최대 빈 얻기 
        double maxVal = 0; 
        double minVal = 0; 
        cv::minMaxLoc(hist, &minVal, &maxVal, 0, 0); 

        int histSize = hist.rows; 

        cv::Mat histImg(histSize*zoom, histSize*zoom, CV_8UC3, CV_RGB(0, 0, 0));

        // presetting
        cv::Scalar bgClr = CV_RGB(255, 255, 255);
        cv::Scalar bkClr = CV_RGB(0, 0, 0);
        cv::Scalar grClr = CV_RGB(guideLineClr, guideLineClr, guideLineClr);

        if (backClr == 1)
        {
            bkClr = CV_RGB(255, 255, 255);
            bgClr = CV_RGB(0, 0, 0);
        }

        cv::Size szTextHor = cv::getTextSize("000", 1, textScale, 1, 0);
        cv::Size szTextMaxVal = cv::getTextSize(to_string((int)maxVal), 1, textScale, 1, 0);

        int marginText = 15;
        int marginBtm = szTextHor.height + marginText * 2;
        int marginRig = 30;
        int marginLef = szTextMaxVal.width + marginText * 2;
        int marginTop = 30;

        int hist_w = histImg.cols;
        int hist_h = histImg.rows;

        float bin_w = (float)(hist_w - marginLef - marginRig) / (float)histSize;

        cv::arrowedLine(histImg, cv::Point(marginLef, hist_h - marginBtm), cv::Point(hist_w - marginRig * 0.5, hist_h - marginBtm), bkClr, 2, 8, 0, 0.008);	// bottom line
        cv::arrowedLine(histImg, cv::Point(marginLef, hist_h - marginBtm), cv::Point(marginLef, marginTop * 0.5), bkClr, 2, 8, 0, 0.015);	// left

        // Draw horizontal value
        int repValNumHori = divNum.width;
        int interHori = double(hist_w - marginLef - marginRig) / repValNumHori;
        float interValHori = (float)(histSize - 1) / (float)repValNumHori;
        for (int ii = 0; ii < repValNumHori + 1; ii++) {
            string textHori = to_string((int)(interValHori * ii));
            cv::Size szTextHori = cv::getTextSize(textHori, 1, textScale, 1, 0);
            putText(histImg, textHori, cv::Point(marginLef + interHori * ii - szTextHori.width * 0.5, hist_h - marginBtm + szTextHor.height + marginText), 1, textScale, bkClr, 1, 8, false);
        }

        // Draw vertical value
        int repValNumVert = divNum.height;
        int interVert = double(hist_h - marginTop - marginBtm) / repValNumVert;
        float interValVert = (float)(maxVal) / (float)repValNumVert;

        for (int ii = 0; ii < repValNumVert; ii++) {
            string textVer = to_string((int)(interValVert * (repValNumVert - ii)));
            cv::Size szTextVer = cv::getTextSize(textVer, 1, textScale, 1, 0);
            int rightAlig = szTextMaxVal.width - szTextVer.width;
            cv::putText(histImg, textVer, cv::Point(marginLef - szTextMaxVal.width + rightAlig - marginText, marginTop + interVert * ii + szTextVer.height * 0.5), 1, textScale, bkClr, 1, 8, false);
        }

        // Draw horizontal guide line
        for (int ii = 1; ii <= repValNumHori; ii++) {
            cv::line(histImg, cv::Point(marginLef + interHori * ii, marginTop * 0.5), cv::Point(marginLef + interHori * ii, hist_h - marginBtm), grClr, 1, 8, 0);
            cv::line(histImg, cv::Point(marginLef + interHori * ii, hist_h - marginBtm), cv::Point(marginLef + interHori * ii, hist_h - marginBtm + 10), bkClr, 1, 8, 0);
        }

        // Draw vertical guide line
        for (int ii = 0; ii < repValNumVert; ii++) {
            cv::line(histImg, cv::Point(marginLef - 10, marginTop + interVert * ii), cv::Point(hist_w - marginRig, marginTop + interVert * ii), grClr, 1, 8, 0);
            cv::line(histImg, cv::Point(marginLef - 10, marginTop + interVert * ii), cv::Point(marginLef, marginTop + interVert * ii), bkClr, 1, 8, 0);
        }

        normalize(hist, hist, 0, histImg.rows - marginTop - marginBtm, NORM_MINMAX, -1, Mat());

        /// Draw for each channel
        if (!bFill) {
            for (int i = 1; i < histSize; i++) {
                cv::Point prePtR(marginLef + (bin_w * (i - 1)), hist_h - marginBtm - cvRound(hist.at<float>(i - 1)));
                cv::Point postPtR(marginLef + (bin_w * (i)), hist_h - marginBtm - cvRound(hist.at<float>(i)));
                cv::line(histImg, prePtR, postPtR, bkClr, 2, 8, 0);
            }
        }
        else if (bFill) {
            for (int i = 1; i < histSize; i++) {
                cv::Point prePtR(marginLef + (bin_w * (i - 1)), hist_h - marginBtm);
                cv::Point postPtR(marginLef + (bin_w * (i)), hist_h - marginBtm - cvRound(hist.at<float>(i)));
                cv::rectangle(histImg, prePtR, postPtR, bkClr, -1, 8, 0);
            }
        }

        return histImg; 
      }
};


float *getMovingAverage(float A[], int count,int M);
bool Savitzky_Golay_smoothing(std::vector<double>* x_series, std::vector<double>* y_series, std::vector<double>* destX, std::vector<double>* destY);
void LeastSqrRegression(double* x, double* y, int obs, double& a, double& b);
int CircleFit_LS(int N, double xp[], double yp[], double* cx, double* cy, double* rad);
bool cubic_Hermite_spline(const std::vector<double> x_src, const std::vector<double> y_src,	std::vector<double>* destX, std::vector<double>* destY);
bool cubic_spline(std::vector<double>* x_series, std::vector<double>* y_series, std::vector<double>* destX, std::vector<double>* destY);
bool monotonic_cubic_Hermite_spline( const std::vector<double> x_src, const std::vector<double> y_src, std::vector<double>* destX, std::vector<double>* destY);
void calcSubPixelCenter(float inImg[3][3], float* outX, float* outY,size_t inNumIter = 10 /*num iterations*/);
void calcRotate(double rx, double ry, double x1, double y1, double rangle, double *x2, double *y2);
void GetLineCoef(double x1, double y1, double x2, double y2, double& a, double& b);
cv::Point IntersectionPoint1(const cv::Point* p1, const cv::Point* p2, const cv::Point* p3, const cv::Point* p4);
cv::Point2f IntersectionPoint2(double m1, double b1, double m2, double b2);
cv::Point IntersectionPoint3(double a1, double b1, double c1, double a2, double b2, double c2);
int getMaxAreaContourId(std::vector <std::vector<cv::Point>> contours);
int getSecondAreaContourId(std::vector <std::vector<cv::Point>> contours);	// jhkim 20220907

float calcAngleFromPoints(cv::Point2f _ptFirstPos, cv::Point2f _ptSecondPos);
int CircleLineIntersection(float x, float y, float r, float a, float b, float c, float d, float xy[][2]);
int FindLineCircleIntersection(float circleCenterX, float circleCenterY, float circleRadius, cv::Point2f linePoint1, cv::Point2f linePoint2, float intersectPoint[][2]);
int GetTangentPointsAtCircle(double rX, double rY, double R, double XX, double YY, double& XT0, double& YT0, double& XT1, double& YT1);

cv::Point2f Equation_of_Lines(cv::Point2f Ap, cv::Point2f Bp,double T, bool bCircle=false, bool bNotch=true);
void Morph(const cv::Mat& src, cv::Mat& dst, int operation = cv::MORPH_OPEN, int kernel_type = cv::MORPH_RECT, int size = 1);
void scaleContours(const std::vector<std::vector<cv::Point>>& src, std::vector<std::vector<cv::Point>>& dst, float scale);
void sobelDirection(int nMaskSize, int nDirection, cv::Mat& srcImage, cv::Mat& dstImage);
//void cvShowImage(const string& winname, const cv::Mat& img, int w=-1, int h=-1);
#define cvShowImage(title,img) ShowImage(title,img)
#define cvHideImage(title) HideImage(title);
cv::Mat cvProjectionHorizontal(const cv::Mat& img,int mod = 0);
cv::Mat cvProjectionVertical(const cv::Mat& img, int mod = 0);
std::vector<float> cvMatToVector(const cv::Mat& img);
void fnRemoveNoise(cv::Mat &matImage, int nSize);
void fnRemoveNoiseBlack(cv::Mat &matImage, int nSize);
void normalize_skeleton(Mat& inputarray, Mat& outputarray);
void thinning(const Mat& src, Mat& dst);
void Thinning(Mat &input, int row, int col);
void thinImage(Mat& src, Mat& dst);
void rot90(cv::Mat& matImage, int rotflag);
int ccw(CPoint a, CPoint b, CPoint c);
double ToRadian(double degree);
double ToDegree(double radian);
cv::Mat translateImg(cv::Mat& img, int offsetx, int offsety);
cv::Mat GetCirclemask(cv::Mat image, int x, int y, int radius, bool white = true);
cv::Mat GetRectmask(cv::Mat image, cv::Rect R, bool white = true);

cv::Point2d CalcRotationCenterSingle_2(cv::Point2d ptCircle1, cv::Point2d ptCircle2, double dAngle);
cv::Point2d FindRotationCenter(cv::Point2d p1, cv::Point2d p2, double t1, double t2);
cv::Point2d CalcRotationCenterSingle(Point2d ptCircle1, Point2d ptCircle2, double dAngle);
cv::Point2d CalcRotationCenterMulti(int Count, std::vector<cv::Point2d> m_listPtOnCircle, double& m_dRadius);
cv::Point2d CalcRotationCenterMulti_2(int Count, std::vector<cv::Point2d> m_listPtOnCircle, double& m_dRadius);
void fit_circle(const std::vector<cv::Point2d>& pnts, cv::Point2d& centre, double& radius);
int RansacCircleFit(int N, double xp[], double yp[], double* centerx, double* centery, double* radius, int sample_th/*= 2 * N / 3*/, double dist_th=0.25, int max_iter=3);
int findMultipleHoughCircle(const cv::Mat& gray, std::vector<cv::Vec3f> &circles, cv::Mat& color);
int findMultipleCircle(const cv::Mat& gray, std::vector<cv::Vec3f> &circles, double chole, double shole,int rad,int hthresh=128, bool bauto = true, cv::Mat* Maskcolor=NULL, int _start=-1,int _end =-1,bool _bsplit = false); // 20210923 제외 영역 사용 하기 위함
double FindCircleAlign_PN(const cv::Mat& gray, int sx,int sy,double radius,int minCircle, std::vector<cv::Vec3f>& circles, bool bauto = true, int hth=230,FakeDC* pDC=NULL);
double FindCircleAlign_MP(const cv::Mat& gray, int sx, int sy, double radius, int cirsize, std::vector<cv::Vec3f>& circles, bool bauto = true, int hth = 230, bool bremoveNoise=true, FakeDC* pDC = NULL, bool CoverMode = false);

// KBJ 2022-09-07 센터얼라인 FindCircleAlign_PN,MP 인자 추가
double FindCircleAlign_PN(const cv::Mat& gray, int sx, int sy, double radius, int minCircle, std::vector<cv::Vec3f>& circles, std::vector<cv::Point2f>* pEdgeposition, bool bauto = true, int hth = 230, FakeDC* pDC = NULL);
double FindCircleAlign_MP(const cv::Mat& gray, int sx, int sy, double radius, int cirsize, std::vector<cv::Vec3f>& circles, std::vector<cv::Point2f>* pEdgeposition, bool bauto = true, int hth = 230, bool bremoveNoise = true, FakeDC* pDC = NULL, bool CoverMode = false);
double FindMetalToInkDistance(const cv::Mat& gray, int sx, int sy, std::vector<cv::Vec3f>& circles,double &startA,double &endA, CDC* pDC = NULL);
int findOrgImageCircle(const cv::Mat& gray, std::vector<cv::Vec3f>& circles, double chole, int rad);
int findOrgImageLMERanscCircle(const cv::Mat& gray, std::vector<cv::Vec3f>& circles, double chole, int rad);
cv::Mat NMS_Algorithm(cv::Mat& mag, cv::Mat dir, std::vector<cv::Point>& edgePoints, float thresh = 100);
double CalcMHWScore(vector<double> scores);  //벡터 중간값 출력
int getVectorIndex(std::vector<double> v, double k);
//int findOrgImageCircle(const vector<cv::Point2f>& edgePositions, std::vector<cv::Vec3f>& circles, double chole, int rad);
int evaluateCircle(vector<cv::Point2f>& pts, cv::Point2f center, float radius, float maxError = 4);
bool MLSAC_LineFitting(std::vector<cv::Point> pts, double& a, double& b, double& c);
double MLSAC_CircleFitting(std::vector<cv::Point> pts, double& cx, double& cy, double& rad);
cv::Mat Adaptive_Median_FilterWork(cv::Mat src);

#endif
