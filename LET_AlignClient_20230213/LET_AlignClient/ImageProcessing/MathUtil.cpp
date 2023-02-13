// Util.cpp
#include "Stdafx.h"
#include "MathUtil.h"
#include "rtl/RTL.hpp"

using namespace std;
using namespace cv;

#define CL_GREEN  Scalar(0  , 255, 0  )
#define CL_RED    Scalar(0  , 0  , 255)
#define CL_BLU    Scalar(255, 0  , 0  )
#define CL_BLACK  Scalar(0  , 0  , 0  )

#define SQR(x) ((x)*(x))

template <typename T, typename Total, size_t N>
class Moving_Average
{
public:
	Moving_Average& operator()(T sample)
	{
		total_ += sample;
		if (num_samples_ < N)
			samples_[num_samples_++] = sample;
		else
		{
			T& oldest = samples_[num_samples_++ % N];
			total_ -= oldest;
			oldest = sample;
		}
		return *this;
	}

	operator double() const { return total_ / std::min(num_samples_, N); }

private:
	T samples_[N];
	size_t num_samples_{ 0 };
	Total total_{ 0 };
};

BOOL Adaptive_Moving_Average(
	vector<double>& ivec,     // input  vector
	vector<double>& ovec,     // output vector
	unsigned long   span,     // time period or span of embedding
	unsigned long   fastSmoothingPeriod,
	unsigned long   slowSmoothingPeriod)
{
	double  fsc = 2.0 / (double)(fastSmoothingPeriod + 1.0);
	double  ssc = 2.0 / (double)(slowSmoothingPeriod + 1.0);
	double  se = 2.0 / (double)(span + 1); // short exponent
	double  le = 1.0 - se;                  // long exponent
	ULONG   e = ULONG(ivec.size());
	ULONG   x = 0;
	ULONG   y = 0;

	if (span == 0 || span >= e)
		return false;

	ovec.resize(ivec.size());

	ovec[0] = ivec[0];

	for (x = 1; x < e; x++)
	{
		double direction = 0;
		double volatility = 0;
		double eratio = 0;
		double c = 0;

		if (x >= span)
		{
			direction = ivec[x] - ivec[x - span];

			for (y = 0; y < span; y++)
				volatility += fabs(ivec[x - y] - ivec[x - y - 1]);

			if (volatility != 0)
			{
				eratio = direction / volatility;
				c = sqrt((eratio * (fsc - ssc)) + ssc);
			}

			ovec[x] = ovec[x - 1] + c * (ivec[x] - ovec[x - 1]);
		}
		else
		{
			ovec[x] = ivec[x] * se + ovec[x - 1] * le;
		}
	}

	return true;
}
void rot90(cv::Mat& matImage, int rotflag) {
	//1=CW, 2=CCW, 3=180
	if (rotflag == 1) {
		transpose(matImage, matImage);
		flip(matImage, matImage, 1); //transpose+flip(1)=CW
	}
	else if (rotflag == 2) {
		transpose(matImage, matImage);
		flip(matImage, matImage, 0); //transpose+flip(0)=CCW     
	}
	else if (rotflag == 3) {
		flip(matImage, matImage, -1);    //flip(-1)=180          
	}
	else if (rotflag != 0) { //if not 0,1,2,3:
		cout << "Unknown rotation flag(" << rotflag << ")" << endl;
	}
}

cv::Mat translateImg(cv::Mat& img, int offsetx, int offsety) {
	cv::Mat trans_mat = (cv::Mat_<double>(2, 3) << 1, 0, offsetx, 0, 1, offsety);
	cv::warpAffine(img, img, trans_mat, img.size());
	return img;
}

double ToRadian(double degree)
{
	return degree * PI / 180.0;
}

double ToDegree(double radian)
{
	return radian * 180.0 / PI;
}

int ccw(CPoint a, CPoint b, CPoint c)
{
	int op = a.x * b.y + b.x * c.y + c.x * a.y;
	op -= (a.y * b.x + b.y * c.x + c.y * a.x);

	if (op > 0)return 1;
	else if (op == 0)return 0;
	else return -1;
}

double CalcMHWScore(std::vector<double> scores)  //벡터 중간값 출력
{
	size_t size = scores.size();

	if (size == 0)		return 0;  
	else
	{
		sort(scores.begin(), scores.end());
		if (size % 2 == 0)		return (scores[size / 2 - 1] + scores[size / 2]) / 2;
		else		return scores[size / 2];
	}
}

int getVectorIndex(std::vector<double> v, double k)
{
	int index = -1;
	auto it = find(v.begin(), v.end(), k);

	if (it != v.end())index = it - v.begin();

	return index;
}

//선분과의 거리;
double SegmentDistance(CfPt A, CfPt B, CfPt P) {
	double lineLen = A.DistanceTo(B);
	if (lineLen == 0) return A.DistanceTo(P);  //A == B;
	// lineLen != 0 case
	double prj = ((P.x - A.x) * (B.x - A.x) + (P.y - A.y) * (B.y - A.y)) / lineLen;
	if (prj < 0) return A.DistanceTo(P);
	else if (prj > lineLen) return B.DistanceTo(P);
	else	return fabs((-1) * (P.x - A.x) * (B.y - A.y) + (P.y - A.y) * (B.x - A.x)) / lineLen;
};

// (p1, p2)를 이은 직선과 (p3, p4)를 이은 직선의 교차점을 구하는 함수
// Function to get intersection point with line connecting points (p1, p2) and another line (p3, p4).
cv::Point IntersectionPoint1(const cv::Point* p1, const cv::Point* p2, const cv::Point* p3, const cv::Point* p4) {

	cv::Point ret= cv::Point(0,0);

	if (((p1->x - p2->x) * (p3->y - p4->y) - (p1->y - p2->y) * (p3->x - p4->x)) != 0)
	{
		ret.x = ((p1->x * p2->y - p1->y * p2->x) * (p3->x - p4->x) - (p1->x - p2->x) * (p3->x * p4->y - p3->y * p4->x)) / ((p1->x - p2->x) * (p3->y - p4->y) - (p1->y - p2->y) * (p3->x - p4->x));

		ret.y = ((p1->x * p2->y - p1->y * p2->x) * (p3->y - p4->y) - (p1->y - p2->y) * (p3->x * p4->y - p3->y * p4->x)) / ((p1->x - p2->x) * (p3->y - p4->y) - (p1->y - p2->y) * (p3->x - p4->x));
	}
	return ret;
}

// y = m1*x + b1, y = m2*x + b2 두 직선의 교차점을 구하는 함수
// Function to get intersection point of two lines y = m1*x + b1, y = m2*x + b2
cv::Point2f IntersectionPoint2(double m1, double b1, double m2, double b2) {

	cv::Point2f ret;

	ret.x = (m1 - m2) == 0 ? 0 : ((b2 - b1) / (m1 - m2));
	ret.y = (m1 - m2) == 0 ? b1 : (m1 * (b2 - b1) / (m1 - m2) + b1);

	return ret;
}

// a1*x + b1*y + c1 = 0, a2*x + b2*y + c2 = 0 두 직선의 교차점을 구하는 함수
// Function to get intersection point of two lines a1*x + b1*y + c1 = 0, a2*x + b2*y + c2 = 0
cv::Point IntersectionPoint3(double a1, double b1, double c1, double a2, double b2, double c2) {

	cv::Point ret;

	ret.x = (b1 * c2 - b2 * c1) / (a1 * b2 - a2 * b1);
	ret.y = -a1 / b1 * (b1 * c2 - b2 * c1) / (a1 * b2 - a2 * b1) - c1 / b1;

	return ret;
}

void diff(vector<float> in, vector<float>& out)
{
	out = vector<float>(in.size() - 1);

	for (int i = 1; i < in.size(); ++i)
		out[i - 1] = in[i] - in[i - 1];
}

void vectorProduct(vector<float> a, vector<float> b, vector<float>& out)
{
	out = vector<float>(a.size());

	for (int i = 0; i < a.size(); ++i)
		out[i] = a[i] * b[i];
}

void findIndicesLessThan(vector<float> in, float threshold, vector<int>& indices)
{
	for (int i = 0; i < in.size(); ++i)
		if (in[i] < threshold)
			indices.push_back(i + 1);
}

void selectElements(vector<float> in, vector<int> indices, vector<float>& out)
{
	for (int i = 0; i < indices.size(); ++i)
		out.push_back(in[indices[i]]);
}

void selectElements(vector<int> in, vector<int> indices, vector<int>& out)
{
	for (int i = 0; i < indices.size(); ++i)
		out.push_back(in[indices[i]]);
}

void signVector(vector<float> in, vector<int>& out)
{
	out = vector<int>(in.size());

	for (int i = 0; i < in.size(); ++i)
	{
		if (in[i] > 0)
			out[i] = 1;
		else if (in[i] < 0)
			out[i] = -1;
		else
			out[i] = 0;
	}
}

void std_to_box(int boxes[], float sigma, int n)
{
	// ideal filter width
	float wi = std::sqrt((12 * sigma * sigma / n) + 1);
	int wl = std::floor(wi);
	if (wl % 2 == 0) wl--;
	int wu = wl + 2;

	float mi = (12 * sigma * sigma - n * wl * wl - 4 * n * wl - 3 * n) / (-4 * wl - 4);
	int m = std::round(mi);

	for (int i = 0; i < n; i++)
		boxes[i] = ((i < m ? wl : wu) - 1) / 2;
}

void horizontal_blur(float* in, float* out, int w, int h, int r)
{
	float iarr = 1.f / (r + r + 1);
#pragma omp parallel for
	for (int i = 0; i < h; i++)
	{
		int ti = i * w, li = ti, ri = ti + r;
		float fv = in[ti], lv = in[ti + w - 1], val = (r + 1) * fv;

		for (int j = 0; j < r; j++) val += in[ti + j];
		for (int j = 0; j <= r; j++) { val += in[ri++] - fv; out[ti++] = val * iarr; }
		for (int j = r + 1; j < w - r; j++) { val += in[ri++] - in[li++]; out[ti++] = val * iarr; }
		for (int j = w - r; j < w; j++) { val += lv - in[li++]; out[ti++] = val * iarr; }
	}
}

void total_blur(float* in, float* out, int w, int h, int r)
{
	float iarr = 1.f / (r + r + 1);
#pragma omp parallel for
	for (int i = 0; i < w; i++)
	{
		int ti = i, li = ti, ri = ti + r * w;
		float fv = in[ti], lv = in[ti + w * (h - 1)], val = (r + 1) * fv;
		for (int j = 0; j < r; j++) val += in[ti + j * w];
		for (int j = 0; j <= r; j++) { val += in[ri] - fv; out[ti] = val * iarr; ri += w; ti += w; }
		for (int j = r + 1; j < h - r; j++) { val += in[ri] - in[li]; out[ti] = val * iarr; li += w; ri += w; ti += w; }
		for (int j = h - r; j < h; j++) { val += lv - in[li]; out[ti] = val * iarr; li += w; ti += w; }
	}
}

void box_blur(float*& in, float*& out, int w, int h, int r)
{
	std::swap(in, out);
	horizontal_blur(out, in, w, h, r);
	total_blur(in, out, w, h, r);
}

void fast_gaussian_blur(float*& in, float*& out, int w, int h, float sigma)
{
	// sigma conversion to box dimensions
	int boxes[3];
	std_to_box(boxes, sigma, 3);
	box_blur(in, out, w, h, boxes[0]);
	box_blur(out, in, w, h, boxes[1]);
	box_blur(in, out, w, h, boxes[2]);
}

int getMaxAreaContourId(std::vector <std::vector<cv::Point>> contours) {
	double maxArea = 0;
	int maxAreaContourId = -1;
	for (int j = 0; j < contours.size(); j++) {
		double newArea = cv::contourArea(contours.at(j));
		if (newArea > maxArea) {
			maxArea = newArea;
			maxAreaContourId = j;
		}
	}
	return maxAreaContourId;
}

int getSecondAreaContourId(std::vector <std::vector<cv::Point>> contours) // jhkim 20220907
{
	double maxArea = 0;
	int maxAreaContourId = getMaxAreaContourId(contours);
	int iSecondID = -1;
	for (int j = 0; j < contours.size(); j++)
	{
		double newArea = cv::contourArea(contours.at(j));
		if (newArea > maxArea)
		{
			//maxArea = newArea;
			if (j != maxAreaContourId)
			{
				maxArea = newArea;

				//maxAreaContourId = j;
				iSecondID = j;
			}
		}
	}
	return iSecondID;
}

float* getMovingAverage(float A[], int count, int M)
{
	//이동 평균 값을 담을 리스트 친구 선언.
	float* result = new float[count];

	int N = count, nc = 0;
	float partialSum = 0;

	for (int i = 0; i < M - 1; ++i) {
		partialSum += A[i];
	}

	for (int i = M - 1; i < N; ++i) {
		partialSum += A[i];
		result[nc++] = partialSum / M;
		partialSum -= A[i - M + 1];
	}

	return result;
}

bool Savitzky_Golay_smoothing(std::vector<double>* x_series, std::vector<double>* y_series, std::vector<double>* destX, std::vector<double>* destY)
{
	int A[] = { -2, 3, 6, 7, 6, 3, -2 };
	int n = 3;
	int k = 0;
	int point_number = (int)x_series->size();

	if (point_number == 0) return false;

	for (k = 0; k < n; k++) {
		if (destX != NULL)     destX->push_back(x_series->operator [](k));
		if (destY != NULL)     destY->push_back(y_series->operator [](k));
	}
	for (k = n; k < point_number - n; k++) {
		double x = x_series->operator[](k);
		int i = 0;
		double nominator = 0;
		double denominator = 0;
		for (i = -n; i <= n; i++) {
			nominator += (A[n + i] * y_series->operator[](k + i));
			denominator += A[n + i];
		}
		double y = nominator / denominator;
		if (destX != NULL)             destX->push_back(x);
		if (destY != NULL)             destY->push_back(y);
	}
	for (k = point_number - n; k < point_number; k++) {
		if (destX != NULL)     destX->push_back(x_series->operator [](k));
		if (destY != NULL)     destY->push_back(y_series->operator [](k));
	}

	return true;
}

void GetLineCoef(double x1, double y1, double x2, double y2, double& a, double& b)
{
	a = 0.0;
	if (x2 != x1)
	{
		a = (y2 - y1) / (x2 - x1);
		b = y1 - (a)*x1;
	}
	else if (x1 == x2) // 수직선
	{
		b = 0;
	}
	else // 수평선
	{
		b = y1;
	}
}


void LeastSqrRegression(double* x, double* y, int obs, double& a, double& b)
{
	if (obs == 0) return;

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

	a = slope = (obs * sum_xy - sum_x * sum_y) / (obs * sum_xx - sum_x * sum_x);
	b = intercept = avg_y - slope * avg_x;

	for (int i = 0; i < obs; i++)
	{
		discrepancies_squared = pow(y[i] - intercept - (slope * x[i]), 2);
		sum_squared_discrepancies += discrepancies_squared;
		residue_sqaured = pow(y[i] - avg_y, 2);
		sum_squared_residue += residue_sqaured;
	}
	r_squared = (sum_squared_residue - sum_squared_discrepancies) / sum_squared_residue;
}

void calcRotate(double rx, double ry, double x1, double y1, double rangle, double* x2, double* y2)
{
	double tmpx, tmpy;
	tmpx = x1 - rx;
	tmpy = y1 - ry;

	*x2 = (tmpx * cos(rangle) - tmpy * sin(rangle) + rx);
	*y2 = (tmpx * sin(rangle) + tmpy * cos(rangle) + ry);
}


float calcAngleFromPoints(cv::Point2f _ptFirstPos, cv::Point2f _ptSecondPos)
{
	float fAngle;
	float fdX = _ptSecondPos.x - _ptFirstPos.x;
	float fdY = _ptSecondPos.y - _ptFirstPos.y;

	////직선이 이루는 각도 계산시
	//fAngle = atan(fdY / fdX) * (180.f / CV_PI);
	//if (fdX < 0.0) 		fAngle += 180.f;
	//else if (fdY < 0.0) fAngle += 360.f;

	if (fabs(fdY) < 4) fdY = 0;  // 4도 이하인 경우는 노이즈 인걸로 간주해서 0으로 강제 계산
	fAngle = (atan2(fdY, fdX) * 180.f) / CV_PI; //
//	if (0 > fAngle)		fAngle += 360.f;  // 시계방향으로 각도 계산 인경우

	return fAngle;
}

int FindLineCircleIntersection(float circleCenterX, float circleCenterY, float circleRadius, cv::Point2f linePoint1, cv::Point2f linePoint2, float intersectPoint[][2])
{
	float deltaX;
	float deltaY;
	float a;
	float b;
	float c;
	float det;
	float t;

	deltaX = linePoint2.x - linePoint1.x;
	deltaY = linePoint2.y - linePoint1.y;

	a = deltaX * deltaX + deltaY * deltaY;
	b = 2 * (deltaX * (linePoint1.x - circleCenterX) + deltaY * (linePoint1.y - circleCenterY));
	c = (linePoint1.x - circleCenterX) * (linePoint1.x - circleCenterX) + (linePoint1.y - circleCenterY) * (linePoint1.y - circleCenterY) - circleRadius * circleRadius;

	det = b * b - 4 * a * c;

	if ((a <= 0.0000001) || (det < 0))
	{
		intersectPoint[0][0] = 0;
		intersectPoint[0][1] = 0;
		intersectPoint[1][0] = 0;
		intersectPoint[1][1] = 0;
		return 0;
	}
	else if (det == 0)
	{
		t = -b / (2 * a);
		intersectPoint[1][0] = linePoint1.x + t * deltaX;
		intersectPoint[1][1] = linePoint1.y + t * deltaY;
		intersectPoint[0][0] = 0;
		intersectPoint[0][1] = 0;

		return 1;
	}
	else
	{
		t = (float)((-b + sqrt(det)) / (2 * a));
		intersectPoint[1][0] = linePoint1.x + t * deltaX;
		intersectPoint[1][1] = linePoint1.y + t * deltaY;

		t = (float)((-b - sqrt(det)) / (2 * a));
		intersectPoint[0][0] = linePoint1.x + t * deltaX;
		intersectPoint[0][1] = linePoint1.y + t * deltaY;

		return 2;
	}
}

int CircleLineIntersection(float x, float y, float r, float a, float b, float c, float d, float xy[][2])
{
	float m = 0, n = 0;

	// A,B1,C 원과 직선으로부터 얻어지는 2차방정식의 계수들
	// D: 판별식
	// X,Y: 교점의 좌표
	float A = 0, B1 = 0, C = 0, D = 0;
	float X = 0, Y = 0;

	// A,B1,C,D게산
	if (c != a)
	{
		// m, n계산
		m = (d - b) / (c - a);
		n = (b * c - a * d) / (c - a);

		A = m * m + 1;
		B1 = (m * n - m * y - x);
		C = (x * x + y * y - r * r + n * n - 2 * n * y);
		D = B1 * B1 - A * C;

		if (D < 0)
			return 0;
		else if (D == 0)
		{
			X = -B1 / A;
			Y = m * X + n;
			xy[0][0] = X;
			xy[0][1] = Y;
			return 1;
		}
		else
		{
			X = -(B1 + sqrt(D)) / A;
			Y = m * X + n;
			xy[0][0] = X;
			xy[0][1] = Y;

			X = -(B1 - sqrt(D)) / A;
			Y = m * X + n;
			xy[1][0] = X;
			xy[1][1] = Y;
			return 2;
		}
	}
	else
	{
		// a == c 인 경우는 수직선이므로
		// 근을 가지려면 a >= (x-r) && a <=(x+r) )
		// (a-x)*(a-x)
		// 1. 근이 없는 경우
		// a < (x-r) || a > (x+r)

		// 근이 없음
		if (a < (x - r) || a >(x + r))
			return 0;
		// 하나의 중근
		else if (a == (x - r) || a == (x + r))
		{
			X = a;
			Y = y;
			xy[0][0] = X;
			xy[0][1] = Y;

			return 1;
		}
		// 두개의 근
		else
		{
			// x = a를 대입하여 Y에 대하여 풀면
			X = a;
			Y = y + sqrt(r * r - (a - x) * (a - x));
			xy[0][0] = X;
			xy[0][1] = Y;

			Y = y - sqrt(r * r - (a - x) * (a - x));
			xy[1][0] = X;
			xy[1][1] = Y;

			return 2;
		}
	}
}

int GetTangentPointsAtCircle(double rX, double rY, double R, double XX, double YY,
	double& XT0, double& YT0, double& XT1, double& YT1)
{
	double nx, ny, xy, tx0, tx1, D;

	if (R == 0) return 0; //this behavior can be modified

	nx = (XX - rX) / R; //shift and scale
	ny = (YY - rY) / R;
	xy = nx * nx + ny * ny;

	//point lies at circumference, one tangent
	if (xy == 1.0)
	{
		XT0 = XX;
		YT0 = YY;
		return 1;
	}

	if (xy < 1.0)  return 0; //point lies inside the circle, no tangents		

	//common case, two tangents
	D = ny * sqrt(xy - 1);
	tx0 = (nx - D) / xy;
	tx1 = (nx + D) / xy;

	if (ny != 0)  //common case
	{ 
		YT0 = rY + R * (1 - tx0 * nx) / ny;
		YT1 = rY + R * (1 - tx1 * nx) / ny;
	}
	else //point at the center horizontal, Y=0
	{ 
		D = R * sqrt(1 - tx0 * tx0);
		YT0 = rY + D;
		YT1 = rY - D;
	}

	XT0 = rX + R * tx0; //restore scale and position
	XT1 = rX + R * tx1;

	return 2;
}

cv::Point2f Equation_of_Lines(cv::Point2f Ap, cv::Point2f Bp, double T, bool bCircle, bool bNotch)
{
	cv::Point2f Cp;
	double mxT = (Bp.x - Ap.x) != 0 ? (Bp.y - Ap.y) / (Bp.x - Ap.x) : 0;
	//double b = mx==0? A.y:A.y + (1 / mx * A.x);

	if (bNotch)
	{
		if (fabs(mxT) < 0.001)
		{
			Cp.x = Ap.x;
			Cp.y = Ap.y + T;
			return Cp;
		}
	}
	else
	{
		if (bCircle)
		{
			if (Bp.x - Ap.x == 0)
			{
				if (Bp.y > Ap.y) Cp.x = Ap.x + T;
				else Cp.x = Ap.x - T;
				Cp.y = Ap.y;

				return Cp;
			}
			else if (fabs(mxT) < 0.001)
			{
				Cp.x = Ap.x;
				Cp.y = Ap.y + T;
				return Cp;
			}
		}
		else
		{
			if (Bp.x - Ap.x == 0)
			{
				if (Bp.y > Ap.y) Cp.x = Ap.x + T;
				else Cp.x = Ap.x - T;
				Cp.y = Ap.y;

				return Cp;
			}
			else if (Bp.y - Ap.y == 0)
			{
				Cp.x = Ap.x;
				if (Bp.x > Ap.x) Cp.y = Ap.y - T;
				else Cp.y = Ap.y + T;
				return Cp;
			}
		}
	}

	double mx = -1 / mxT;

	double A = (mx * mx) + 1;
	double B = (2 * (Ap.y - (mx * Ap.x)) * mx) - (2 * Ap.x) - (2 * Ap.y * mx);
	double C = (Ap.x * Ap.x) + (Ap.y * Ap.y) - (2 * Ap.y * (Ap.y - (mx * Ap.x))) + ((Ap.y - (mx * Ap.x)) * (Ap.y - (mx * Ap.x))) - (T * T);

	double D = (B * B) - 4 * (A * C);

	if (D > 0)
	{
		Cp.x = ((-1 * B) + sqrt(D)) / (2 * A);
		Cp.y = (mx * Cp.x) + (Ap.y - (mx * Ap.x));

		double x2 = ((-1 * B) - sqrt(D)) / (2 * A);
		double y2 = (mx * x2) + (Ap.y - (mx * Ap.x));
		if (bNotch)
		{
			if (T > 0)
			{
				if (Cp.x < x2 || Cp.y < y2)
				{
					Cp.x = x2;
					Cp.y = y2;
				}
			}
			else
			{
				if (Cp.x < x2 || Cp.y > y2)
				{
					Cp.x = x2;
					Cp.y = y2;
				}
			}
		}
		else
		{
			if (Ap.y > Bp.y)
			{
				Cp.x = x2;
				Cp.y = y2;
			}
		}
	}
	else
	{
		Cp.x = 0;
		Cp.y = 0;
	}

	return Cp;
}



double h00(double t) { return 2 * t * t * t - 3 * t * t + 1; }
double h10(double t) { return t * (1 - t) * (1 - t); }
double h01(double t) { return t * t * (3 - 2 * t); }
double h11(double t) { return t * t * (t - 1); }

float CubicHermite(float A, float B, float C, float D, float t)
{
	float a = -A / 2.0f + (3.0f * B) / 2.0f - (3.0f * C) / 2.0f + D / 2.0f;
	float b = A - (5.0f * B) / 2.0f + 2.0f * C - D / 2.0f;
	float c = -A / 2.0f + C / 2.0f;
	float d = B;

	return a * t * t * t + b * t * t + c * t + d;
}


bool cubic_Hermite_spline(const std::vector<double> x_src, const std::vector<double> y_src,
	std::vector<double>* destX, std::vector<double>* destY)
{
	TPointList2D points;
	int n = (int)x_src.size();
	int k = 0, num = n * 32;

	std::array<float, 2> arr;

	for (int i = 0; i < n; i++)
	{
		arr.at(0) = x_src[i];
		arr.at(1) = y_src[i];
		points.push_back(arr);
	}

	arr.at(0) = x_src[0];
	arr.at(1) = y_src[0];
	points.push_back(arr);

	for (int i = 0; i < num; ++i)
	{
		float percent = ((float)i) / (float(num - 1));
		float x = 0.0f;
		float y = 0.0f;

		float tx = (points.size() - 1) * percent;
		int index = int(tx);
		float t = tx - floor(tx);

		std::array<float, 2> A = GetIndexClamped(points, index - 1);
		std::array<float, 2> B = GetIndexClamped(points, index + 0);
		std::array<float, 2> C = GetIndexClamped(points, index + 1);
		std::array<float, 2> D = GetIndexClamped(points, index + 2);
		x = CubicHermite(A[0], B[0], C[0], D[0], t);
		y = CubicHermite(A[1], B[1], C[1], D[1], t);

		if (destX != NULL)			destX->push_back(x);
		if (destY != NULL)			destY->push_back(y);
	}

	return true;
}

bool cubic_spline(std::vector<double>* x_series, std::vector<double>* y_series, std::vector<double>* destX, std::vector<double>* destY)
{
	int n = __min((int)x_series->size() - 1, (int)y_series->size() - 1);
	// Step 1.
	double* h = new double[n + 1];
	double* alpha = new double[n + 1];
	int i = 0;

	for (i = 0; i <= n - 1; i++) {
		h[i] = (*x_series)[i + 1] - (*x_series)[i];
	}

	// Step 2.
	for (i = 1; i <= n - 1; i++) {
		alpha[i] = 3 * ((*y_series)[i + 1] - (*y_series)[i]) / h[i] - 3 * ((*y_series)[i] - (*y_series)[i - 1]) / h[i - 1];
	}

	// Step 3.
	double* l = new double[n + 1];
	double* u = new double[n + 1];
	double* z = new double[n + 1];
	double* c = new double[n + 1];
	double* b = new double[n + 1];
	double* d = new double[n + 1];

	l[0] = 1; u[0] = 0; z[0] = 0;
	// Step 4.

	for (i = 1; i <= n - 1; i++) {
		l[i] = 2 * ((*x_series)[i + 1] - (*x_series)[i - 1]) - h[i - 1] * u[i - 1];
		u[i] = h[i] / l[i];
		z[i] = (alpha[i] - h[i - 1] * z[i - 1]) / l[i];
	}


	// Step 5.
	l[n] = 1;     z[n] = 0;     c[n] = 0;

	// Step 6.
	for (i = n - 1; i >= 0; i--) {
		c[i] = z[i] - u[i] * c[i + 1];
		b[i] = ((*y_series)[i + 1] - (*y_series)[i]) / h[i] - h[i] * (c[i + 1] + 2 * c[i]) / 3;
		d[i] = (c[i + 1] - c[i]) / (3 * h[i]);
	}


	for (i = 0; i <= n - 2; i++) {
		double x = (*x_series)[i];
		double inc = ((*x_series)[i + 1] - (*x_series)[i]) * 0.1;
		for (; x < (*x_series)[i + 1]; x += inc) {
			double x_offset = x - (*x_series)[i];
			double Sx = (*y_series)[i] + b[i] * x_offset + c[i] * x_offset * x_offset + d[i] * x_offset * x_offset * x_offset;
			if (destX != NULL) {
				destX->push_back(x);
			}

			if (destY != NULL) {
				destY->push_back(Sx);
			}
		}
	}

	delete[] h;
	delete[] alpha;
	delete[] l;
	delete[] u;
	delete[] z;
	delete[] c;
	delete[] b;
	delete[] d;

	return true;
}

bool monotonic_cubic_Hermite_spline(const std::vector<double> x_src, const std::vector<double> y_src,
	std::vector<double>* destX, std::vector<double>* destY)
{	// 0-based index 사용.
	int n = (int)x_src.size();
	int k = 0;
	double eps = 1.0e-5;
	double* m = new double[n];

	m[0] = (y_src[1] - y_src[0]) / (x_src[1] - x_src[0]);
	m[n - 1] = (y_src[n - 1] - y_src[n - 2]) / (x_src[n - 1] - x_src[n - 2]);

	for (k = 1; k < n - 1; k++) {
		m[k] = (y_src[k] - y_src[k - 1]) / (2 * (x_src[k] - x_src[k - 1])) + (y_src[k + 1] - y_src[k]) / (2 * (x_src[k + 1] - x_src[k]));
	}

	for (k = 0; k < n - 1; k++) {
		double delta_k = (y_src[k + 1] - y_src[k]) / (x_src[k + 1] - x_src[k]);
		if (fabs(delta_k) <= eps) 			m[k] = m[k + 1] = 0;
		else {
			double ak = m[k] / delta_k;
			double bk = m[k + 1] / delta_k;

			if (ak * ak + bk * bk > 9) {
				m[k] = 3 / (sqrt(ak * ak + bk * bk)) * ak * delta_k;
				m[k + 1] = 3 / (sqrt(ak * ak + bk * bk)) * bk * delta_k;
			}
		}
	}

	for (k = 0; k < n - 1; k++) {
		double cur_x = (double)((int)(0.5 + x_src[k]));
		double next_x = (double)((int)(x_src[k + 1]));
		double cur_y = y_src[k];
		double next_y = y_src[k + 1];
		double h = next_x - cur_x;
		double x = 0;
		double inc = (next_x - cur_x) * 0.1;

		for (x = cur_x; x < next_x; x += inc) {
			double t = (x - cur_x) / h;
			if (destX != NULL)				destX->push_back(x);

			double y = cur_y * h00(t) + h * m[k] * h10(t) + next_y * h01(t) + h * m[k + 1] * h11(t);
			destY->push_back(y);
		}
	}

	delete m;

	return true;
}

void calcSubPixelCenter(float inImg[3][3], float* outX, float* outY,
	size_t inNumIter) {
	// Sub pixel interpolation
	float c, a1, a2, a3, a4, b1, b2, b3, b4;
	float a1n, a2n, a3n, a4n, b1n, b2n, b3n, b4n;

	b1 = inImg[0][0]; a2 = inImg[1][0]; b2 = inImg[2][0];
	a1 = inImg[0][1];  c = inImg[1][1]; a3 = inImg[2][1];
	b4 = inImg[0][2]; a4 = inImg[1][2]; b3 = inImg[2][2];

	for (size_t i = 0; i < inNumIter; ++i) {
		float c2 = 2 * c;
		float sp1 = (a1 + a2 + c2) / 4;
		float sp2 = (a2 + a3 + c2) / 4;
		float sp3 = (a3 + a4 + c2) / 4;
		float sp4 = (a4 + a1 + c2) / 4;

		// New maximum is center
		float newC = std::max({ sp1, sp2, sp3, sp4 });

		// Calc position of new center
		float ad = pow(2.0, -((float)i + 1));

		if (newC == sp1) {
			*outX = *outX - ad; // to the left
			*outY = *outY - ad; // to the top

			// Calculate new sub pixel values
			b1n = (a1 + a2 + 2 * b1) / 4;
			b2n = (c + b2 + 2 * a2) / 4;
			b3n = sp3;
			b4n = (b4 + c + 2 * a1) / 4;
			a1n = (b1n + c + 2 * a1) / 4;
			a2n = (b1n + c + 2 * a2) / 4;
			a3n = sp2;
			a4n = sp4;

		}
		else if (newC == sp2) {
			*outX = *outX + ad; // to the right
			*outY = *outY - ad; // to the top

			// Calculate new sub pixel values
			b1n = (2 * a2 + b1 + c) / 4;
			b2n = (2 * b2 + a3 + a2) / 4;
			b3n = (2 * a3 + b3 + c) / 4;
			b4n = sp4;
			a1n = sp1;
			a2n = (b2n + c + 2 * a2) / 4;
			a3n = (b2n + c + 2 * a3) / 4;
			a4n = sp3;
		}
		else if (newC == sp3) {
			*outX = *outX + ad; // to the right
			*outY = *outY + ad; // to the bottom

			// Calculate new sub pixel values
			b1n = sp1;
			b2n = (b2 + 2 * a3 + c) / 4;
			b3n = (2 * b3 + a3 + a4) / 4;
			b4n = (2 * a4 + b4 + c) / 4;
			a1n = sp4;
			a2n = sp2;
			a3n = (b3n + 2 * a3 + c) / 4;
			a4n = (b3n + 2 * a4 + c) / 4;
		}
		else {
			*outX = *outX - ad; // to the left
			*outY = *outY + ad; // to the bottom   

			// Calculate new sub pixel values
			b1n = (2 * a1 + b1 + c) / 4;
			b2n = sp2;
			b3n = (c + b3 + 2 * a4) / 4;
			b4n = (2 * b4 + a1 + a4) / 4;
			a1n = (b4n + 2 * a1 + c) / 4;
			a2n = sp1;
			a3n = sp3;
			a4n = (b4n + 2 * a4 + c) / 4;
		}

		c = newC; // Oi = Oi+1

		a1 = a1n;
		a2 = a2n;
		a3 = a3n;
		a4 = a4n;

		b1 = b1n;
		b2 = b2n;
		b3 = b3n;
		b4 = b4n;
	}
}


void Morph(const cv::Mat& src, cv::Mat& dst, int operation, int kernel_type, int size)
{
	cv::Point anchor = cv::Point(size, size);
	cv::Mat element = getStructuringElement(kernel_type, cv::Size(2 * size + 1, 2 * size + 1), anchor);
	morphologyEx(src, dst, operation, element, anchor);
}

//void cvShowImage(const string& winname, const cv::Mat& img, int w, int h)
//{
//	if (w == -1) w = img.cols;
//	else if (w == 0) w = img.cols / 2;
//	if (h == -1) h = img.rows;
//	else if (h == 0) h = img.rows / 2;
//
//	cv::namedWindow(winname, WINDOW_NORMAL);
//	cv::resizeWindow(winname, w, h);
//	cv::imshow(winname, img);
//}

cv::Mat cvProjectionHorizontal(const cv::Mat& img, int mod)
{
	cv::Mat horizontal(img.cols, 1, CV_32S);//horizontal histogram
	horizontal = Scalar::all(0);

	if (mod == 0)
	{
		for (int i = 0; i < img.cols; i++)
			horizontal.at<float>(i, 0) = countNonZero(img(cv::Rect(i, 0, 1, img.rows)));
	}
	else
	{
		for (int i = 0; i < img.cols; i++)
			horizontal.at<float>(i, 0) = cv::sum(img(cv::Rect(i, 0, 1, img.rows)))[0] / img.rows;
	}

	return horizontal;
}

cv::Mat cvProjectionVertical(const cv::Mat& img, int mod)
{
	cv::Mat vertical(img.rows, 1, CV_32S);//vertical histogram	
	vertical = Scalar::all(0);

	if (mod == 0)
	{
		for (int i = 0; i < img.rows; i++)
			vertical.at<float>(i, 0) = countNonZero(img(cv::Rect(0, i, img.cols, 1)));
	}
	else
	{
		for (int i = 0; i < img.rows; i++)
			vertical.at<float>(i, 0) = cv::sum(img(cv::Rect(0, i, img.cols, 1)))[0]/ img.cols;
	}

	return vertical;
}

std::vector<float> cvMatToVector(const cv::Mat& img)
{
	std::vector<float> array;
	if (img.isContinuous())		array.assign((float*)img.data, (float*)img.data + img.total() * img.channels());
	else {
		for (int i = 0; i < img.rows; ++i)		array.insert(array.end(), img.ptr<float>(i), img.ptr<float>(i) + img.cols * img.channels());
	}

	return array;
}

void sobelDirection(int nMaskSize, int nDirection, cv::Mat& srcImage, cv::Mat& dstImage)
{
	Mat mask = Mat::ones(nMaskSize, nMaskSize, CV_32F) / 25;

	float* pBuffer = (float*)mask.data;

	if (nMaskSize == 3)
	{
		switch (nDirection) {
		case SOBEL_UP:
			pBuffer[0] = 1;	    pBuffer[1] = 2;	    pBuffer[2] = 1;
			pBuffer[3] = 0;	    pBuffer[4] = 0;	    pBuffer[5] = 0;
			pBuffer[6] = -1;	pBuffer[7] = -2;	pBuffer[8] = -1;
			break;
		case SOBEL_DN:
			pBuffer[0] = -1;	pBuffer[1] = -2;	pBuffer[2] = -1;
			pBuffer[3] = 0;	    pBuffer[4] = 0;	    pBuffer[5] = 0;
			pBuffer[6] = 1;	    pBuffer[7] = 2;	    pBuffer[8] = 1;
			break;
		case SOBEL_LEFT:
			pBuffer[0] = 1;	pBuffer[1] = 0;	pBuffer[2] = -1;
			pBuffer[3] = 2;	pBuffer[4] = 0;	pBuffer[5] = -2;
			pBuffer[6] = 1;	pBuffer[7] = 0;	pBuffer[8] = -1;
			break;
		case SOBEL_RIGHT:
			pBuffer[0] = -1;	pBuffer[1] = 0;	pBuffer[2] = 1;
			pBuffer[3] = -2;	pBuffer[4] = 0;	pBuffer[5] = 2;
			pBuffer[6] = -1;	pBuffer[7] = 0;	pBuffer[8] = 1;
			break;
		case SOBEL_45: // 왼쪽 방향쪽
			pBuffer[0] = -2;	pBuffer[1] = -1;	pBuffer[2] = 0;
			pBuffer[3] = -1;	pBuffer[4] = 0;	    pBuffer[5] = 1;
			pBuffer[6] = 0;	    pBuffer[7] = 1;	    pBuffer[8] = 2;
			break;
		case SOBEL_135: // 왼쪽 방향쪽
			pBuffer[0] = 2;	pBuffer[1] = 1;	  pBuffer[2] = 0;
			pBuffer[3] = 1;	pBuffer[4] = 0;	  pBuffer[5] = -1;
			pBuffer[6] = 0;	pBuffer[7] = -1;  pBuffer[8] = -2;
			break;
		case SOBEL_225: // 오른쪽 방향쪽
			pBuffer[0] = 0;	 pBuffer[1] = 1;	pBuffer[2] = 2;
			pBuffer[3] = -1; pBuffer[4] = 0;	pBuffer[5] = 1;
			pBuffer[6] = -2; pBuffer[7] = -1;	pBuffer[8] = 0;
			break;
		case SOBEL_315: // 오른쪽 방향쪽
			pBuffer[0] = 0;	 pBuffer[1] = -1;	pBuffer[2] = -2;
			pBuffer[3] = 1;  pBuffer[4] = 0;	pBuffer[5] = -1;
			pBuffer[6] = 2;  pBuffer[7] = 1;	pBuffer[8] = 0;
			break;
		};
	}

	filter2D(srcImage, dstImage, -1, mask, cv::Point(-1, -1), (0, 0), 2);
	mask.release();
}

void contourOffset(const std::vector<cv::Point>& src, std::vector<cv::Point>& dst, const cv::Point& offset) {
	dst.clear();
	dst.resize(src.size());
	for (int j = 0; j < src.size(); j++)
		dst[j] = src[j] + offset;

}
void scaleContour(const std::vector<cv::Point>& src, std::vector<cv::Point>& dst, float scale)
{
	cv::Rect rct = cv::boundingRect(src);

	std::vector<cv::Point> dc_contour;
	cv::Point rct_offset(-rct.tl().x, -rct.tl().y);
	contourOffset(src, dc_contour, rct_offset);

	std::vector<cv::Point> dc_contour_scale(dc_contour.size());

	for (int i = 0; i < dc_contour.size(); i++)
		dc_contour_scale[i] = dc_contour[i] * scale;

	cv::Rect rct_scale = cv::boundingRect(dc_contour_scale);

	cv::Point offset((rct.width - rct_scale.width) / 2, (rct.height - rct_scale.height) / 2);
	offset -= rct_offset;
	dst.clear();
	dst.resize(dc_contour_scale.size());
	for (int i = 0; i < dc_contour_scale.size(); i++)
		dst[i] = dc_contour_scale[i] + offset;
}

void scaleContours(const std::vector<std::vector<cv::Point>>& src, std::vector<std::vector<cv::Point>>& dst, float scale)
{
	dst.clear();
	dst.resize(src.size());
	for (int i = 0; i < src.size(); i++)
		scaleContour(src[i], dst[i], scale);
}

cv::Point2d CalcRotationCenterSingle_2(cv::Point2d ptCircle1, cv::Point2d ptCircle2, double dAngle)
{
	cv::Point2d m_ptRotCenterM = cv::Point2d(0, 0);

	double dataX2[2] = { ptCircle2.x, ptCircle2.y };

	cv::Mat A = cv::Mat::zeros(2, 2, CV_64FC1);
	cv::Mat B = cv::Mat::zeros(2, 2, CV_64FC1);

	A.at<double>(0, 0) = -1 + cos(ToRadian(dAngle));
	A.at<double>(0, 1) = sin(ToRadian(dAngle)) * -1;
	A.at<double>(1, 0) = sin(ToRadian(dAngle));
	A.at<double>(1, 1) = -1 + cos(ToRadian(dAngle));

	B.at<double>(0, 0) = cos(ToRadian(dAngle));
	B.at<double>(0, 1) = sin(ToRadian(dAngle)) * -1;
	B.at<double>(1, 0) = sin(ToRadian(dAngle));
	B.at<double>(1, 1) = cos(ToRadian(dAngle));

	cv::Mat x1 = cv::Mat::zeros(2, 1, CV_64F);
	cv::Mat x2 = cv::Mat::zeros(2, 1, CV_64F);

	x1.at<double>(0, 0) = ptCircle1.x;
	x1.at<double>(0, 1) = ptCircle1.y;
	x2.at<double>(0, 0) = ptCircle2.x;
	x2.at<double>(0, 1) = ptCircle2.y;

	try
	{
		cv::Mat x = A.inv() * B * x1 - A.inv() * x2;
		m_ptRotCenterM.x = x.at<double>(0, 0);
		m_ptRotCenterM.y = x.at<double>(1, 0);
	}
	catch (...)
	{
	}

	return m_ptRotCenterM;
}

cv::Point2d FindRotationCenter(cv::Point2d p1, cv::Point2d p2, double t1, double t2)
{
	double T = t2 - t1;
	cv::Point2d m_ptRotCenterM = cv::Point2d(0, 0);

	cv::Mat PO = cv::Mat::zeros(2, 1, CV_64FC1);
	cv::Mat R = cv::Mat::zeros(2, 2, CV_64FC1);
	cv::Mat E = cv::Mat::eye(2, 2, CV_64FC1);
	cv::Mat P1 = cv::Mat::zeros(2, 1, CV_64FC1);
	cv::Mat P2 = cv::Mat::zeros(2, 1, CV_64FC1);

	R.at<double>(0, 0) = cos(T);
	R.at<double>(0, 1) = sin(-T);
	R.at<double>(1, 0) = sin(T);
	R.at<double>(1, 1) = cos(T);

	P1.at<double>(0, 0) = p1.x;
	P1.at<double>(0, 1) = p1.y;
	P2.at<double>(0, 0) = p2.x;
	P2.at<double>(0, 1) = p2.y;

	//////////// 회전중심 계산 ////////////
	//  P2 - P0 = R(P1 - P0)
	//  (R - E)P0 = RP1 - P2
	//  P0 = (R - E)^(-1) (RP1 - P2)
	///////////////////////////////////////
	PO = (R - E).inv() * (R * P1 - P2);

	m_ptRotCenterM.x = PO.at<double>(0);
	m_ptRotCenterM.y = PO.at<double>(1);

	return true;
}

cv::Point2d CalcRotationCenterSingle(Point2d ptCircle1, Point2d ptCircle2, double dAngle)
{
	cv::Point2d m_ptRotCenterM = cv::Point2d(0, 0);

	double sinT = sin(ToRadian(dAngle));
	double cosT = cos(ToRadian(dAngle));
	double x1 = ptCircle1.x;
	double y1 = ptCircle1.y;
	double x2 = ptCircle2.x;
	double y2 = ptCircle2.y;

	cv::Mat A = cv::Mat::zeros(2, 2, CV_64FC1);
	cv::Mat B = cv::Mat::zeros(2, 1, CV_64FC1);

	A.at<double>(0, 0) = 1 - cosT;
	A.at<double>(0, 1) = sinT;
	A.at<double>(1, 0) = sinT;
	A.at<double>(1, 1) = -1 + cosT;

	B.at<double>(0, 0) = y1 * sinT - x1 * cosT + x2;
	B.at<double>(1, 0) = x1 * sinT + y1 * cosT - y2;

	try
	{
		cv::Mat x = A.inv() * B;
		m_ptRotCenterM.x = x.at<double>(0, 0);
		m_ptRotCenterM.y = x.at<double>(1, 0);
	}
	catch (...)
	{
	}

	return m_ptRotCenterM;
}


cv::Point2d CalcRotationCenterMulti_2(int Count, std::vector<cv::Point2d> m_listPtOnCircle, double& m_dRadius)
{
	cv::Point2d m_ptRotCenterM = cv::Point2d(0, 0);

	if (Count < 3) return m_ptRotCenterM;

	cv::Mat A = cv::Mat::zeros(Count - 1, 2, CV_64F);
	cv::Mat B = cv::Mat::zeros(Count - 1, 1, CV_64F);

	for (int i = 0; i < Count - 1; i++)
	{
		double A1 = -2 * m_listPtOnCircle[i].x + 2 * m_listPtOnCircle[i + 1].x;     // 시작각도 기준으로
		double A2 = -2 * m_listPtOnCircle[i].y + 2 * m_listPtOnCircle[i + 1].y;
		double B1 = (m_listPtOnCircle[i + 1].x * m_listPtOnCircle[i + 1].x) - (m_listPtOnCircle[i].x * m_listPtOnCircle[i].x) +
			(m_listPtOnCircle[i + 1].y * m_listPtOnCircle[i + 1].y) - (m_listPtOnCircle[i].y * m_listPtOnCircle[i].y);

		A.at<double>(i, 0) = A1;
		A.at<double>(i, 1) = A2;
		B.at<double>(i, 0) = B1;
	}
	try
	{
		cv::Mat x = (A.t() * A).inv() * A.t() * B;
		m_ptRotCenterM.x = x.at<double>(0);
		m_ptRotCenterM.y = x.at<double>(1);
		m_dRadius = sqrt(pow(m_ptRotCenterM.x - m_listPtOnCircle[0].x, 2) + pow(m_ptRotCenterM.y - m_listPtOnCircle[0].y, 2));

	}
	catch (...)
	{

	}

	return m_ptRotCenterM;
}

cv::Point2d CalcRotationCenterMulti(int Count, std::vector<cv::Point2d> m_listPtOnCircle, double& m_dRadius)
{
	cv::Point2d m_ptRotCenterM = cv::Point2d(0, 0);

	if (Count < 3) return m_ptRotCenterM;

	cv::Mat A = cv::Mat::zeros(Count, 3, CV_64F);
	cv::Mat B = cv::Mat::zeros(Count, 1, CV_64F);

	for (int i = 0; i < Count; i++)
	{
		double x = m_listPtOnCircle[i].x;     // 시작각도 기준으로
		double y = m_listPtOnCircle[i].y;
		double b = x * x * -1 - y * y;
		A.at<double>(i, 0) = x;
		A.at<double>(i, 1) = y;
		A.at<double>(i, 2) = 1;
		B.at<double>(i, 0) = b;
	}
	try
	{
		Mat X = (A.t() * A).inv() * A.t() * B;
		double a = X.at<double>(0);
		double b = X.at<double>(1);
		m_ptRotCenterM.x = a / (-2);
		m_ptRotCenterM.y = b / (-2);
		m_dRadius = sqrt(pow(m_ptRotCenterM.x - m_listPtOnCircle[0].x, 2) + pow(m_ptRotCenterM.y - m_listPtOnCircle[0].y, 2));
	}
	catch (...)
	{

	}

	return m_ptRotCenterM;
}

void GetRandomTriplet(int max_num, int triplet[3]) {
	//[0, N-1] select 3 numbers;
	int index = 0;
	while (index < 3) {
		BOOL newone = TRUE;
		int r = (int)((double)(rand()) / RAND_MAX * max_num);
		for (int i = 0; i < index; ++i) {
			if (r == triplet[i]) {
				newone = FALSE;
				break;
			}
		}
		if (newone) {
			triplet[index] = r;
			++index;
		}
	}
}//end of getrandomtriplet;

int CircumCircle(double x1, double x2, double x3,
	double y1, double y2, double y3,
	double* cx, double* cy, double* rad) {
	double bax = x2 - x1, bay = y2 - y1;
	double cax = x3 - x1, cay = y3 - y1;
	double E = bax * (x1 + x2) + bay * (y1 + y2);
	double F = cax * (x1 + x3) + cay * (y1 + y3);
	double G = 2. * (bax * (y3 - y2) - bay * (x3 - x2));
	if (G == 0.) return 0; //error;
	//assert(fabs(G)>small_epsilon); //to prevent collinear or degenerate case;
	*cx = (cay * E - bay * F) / G;
	*cy = (bax * F - cax * E) / G;
	double dx = *cx - x1, dy = *cy - y1;
	*rad = sqrt(dx * dx + dy * dy);
	return 1;
}

bool SolveLinearEQ3x3(double A[9], double bb[3], double x[3]) {
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

//// Note, p = degree;
//void BSpline(int p, std::vector<CPoint>& control, int resolution, CDC* pDC) {
//	int n = control.size() - 1;  ASSERT(n > 0);
//	std::vector<double> u(n + p + 2);                // knot vector;
//	calculateKnots(&u[0], n, p);                     // uniform knot;
//	double delta = (u[n + p + 1] - u[0]) / (resolution - 1);  // parameter increment;
//	CPoint Q;
//	for (double t = u[0]; t <= u[n + p + 1]; t += delta) {
//		calculatePoint(&u[0], n, p, t, &control[0], &Q);
//		if (t == u[0]) pDC->MoveTo(Q);
//		pDC->LineTo(Q);
//	}
//	pDC->LineTo(control[n]);
//}
//// de Boor Cox's algorithm;
//double Blend(int i, int p, double* u, double t) {
//	if (p == 0) { // termination condition of recursion;
//		if ((u[i] <= t) && (t < u[i + 1]))        return 1;
//		else                                    return 0;
//	}
//	else {
//		double coef1, coef2;
//		if (u[i + p] == u[i]) {
//			if (t == u[i])  coef1 = 1;
//			else            coef1 = 0;
//		}
//		else                coef1 = (t - u[i]) / (u[i + p] - u[i]);
//
//		if (u[i + p + 1] == u[i + 1]) {
//			if (t == u[i + 1]) coef2 = 1;
//			else             coef2 = 0;
//		}
//		else                coef2 = (u[i + p + 1] - t) / (u[i + p + 1] - u[i + 1]);
//		return coef1 * Blend(i, p - 1, u, t) + coef2 * Blend(i + 1, p - 1, u, t);
//	}
//}
//// clamped b-spline knot vector; uniform example;
//// u[0]=u[1]= ... =u[p],.....,u=[n+p-2]=u[n+p-1]=u[n+p]=u[n+p+1]
//void calculateKnots(double* u, int n, int p) {
//	int j = 0;
//	while (j <= p)     u[j++] = 0;                 // multiplicity = p+1;
//	while (j <= n)     u[j++] = j - p;             // m = n+p+1;
//	while (j <= n + p + 1) u[j++] = n - p + 1;         // multiplicity = p+1;
//}
//void calculatePoint(double* u, int n, int p, double t, CPoint* control, CPoint* output) {
//	double x = 0, y = 0;
//	for (int i = 0; i <= n; i++) {
//		double b = Blend(i, p, u, t);
//		x += control[i].x * b;
//		y += control[i].y * b;
//	}
//	output->x = int(x + 0.5);
//	output->y = int(y + 0.5);
//}
//
//
//int BSplineFit_LS(std::vector<CPoint>& data,
//	int degree,             // cubic(3); 
//	int nc,                 // num of control points;
//	double X[], double Y[]) // estimated control points;
//{
//	// open b-spline;
//	std::vector<double> knot((nc - 1) + degree + 2);
//	for (int i = 0; i <= nc + degree; i++) knot[i] = i;
//
//	int ndata = data.size();
//	std::vector<double> t(ndata);                // parameter;
//	double scale = (knot[nc] - knot[degree]) / (ndata - 1);
//	for (int i = 0; i < ndata; i++)
//		t[i] = knot[degree] + scale * i;
//
//	std::vector<double> A(ndata * nc);
//	for (int i = 0; i < ndata; i++)
//		for (int j = 0; j < nc; j++)
//			A[i * nc + j] = Basis(j, degree, &knot[0], t[i]); //A(i,j)=N_j(t_i)
//
//	// S = A^t * A; real-symmetric matrix;
//	std::vector<double> Sx(nc * nc);
//	std::vector<double> Sy(nc * nc);
//	for (int i = 0; i < nc; i++) {
//		for (int j = 0; j < nc; j++) {
//			double s = 0;
//			for (int k = 0; k < ndata; k++)
//				s += A[k * nc + i] * A[k * nc + j];
//			Sx[i * nc + j] = s;
//		}
//	}
//	//copy;
//	for (int i = 0; i < nc * nc; i++) Sy[i] = Sx[i];
//	// X = A^t * P.x;  Y = A^t * P.y
//	for (int i = 0; i < nc; i++) {
//		double sx = 0, sy = 0;
//		for (int k = 0; k < ndata; k++) {
//			sx += A[k * nc + i] * data[k].x;
//			sy += A[k * nc + i] * data[k].y;
//		};
//		X[i] = sx; Y[i] = sy;
//	};
//	// solve real symmetric linear system; S * x = X, S * y = Y;
//	// solvps(S, X) destories the inputs;
//	// ccmath-2.2.1 version;
//	int res1 = solvps(&Sx[0], X, nc);
//	int res2 = solvps(&Sy[0], Y, nc);
//	return res1 == 0 && res2 == 0;
//}

int CircleFit_LS(int N, double xp[], double yp[],
	double* cx, double* cy, double* rad) {
	double sx = 0, sy = 0, sxx = 0, sxy = 0, syy = 0;
	double sxxx = 0, sxxy = 0, sxyy = 0, syyy = 0;
	for (int i = 0; i < N; i++) {
		double x = xp[i], y = yp[i];
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
	if (!SolveLinearEQ3x3(A, b, sol)) return 0;
	double det = sol[0] * sol[0] + sol[1] * sol[1] - 4 * sol[2];
	if (det <= 0) return 0;
	*cx = -sol[0] / 2;
	*cy = -sol[1] / 2;    *rad = sqrt(det) / 2.;
	return 1;
}

int findInlier(double xp[], double yp[], int N,
	double cx, double cy, double rad,
	double dist_th,
	double consensusx[], double consensusy[], double* var) {
	int ninlier = 0;
	double err = 0;
	*var = 0;           // variance of distance deviation;
	for (int k = 0; k < N; ++k) {
		double dist = sqrt(SQR(xp[k] - cx) + SQR(yp[k] - cy));
		double distdeviate = fabs(dist - rad) / rad;
		if (distdeviate <= dist_th) {  //collect maybe_inliers;
			consensusx[ninlier] = xp[k];
			consensusy[ninlier] = yp[k];
			*var += SQR(dist - rad);
			ninlier++;
		}
	}
	return ninlier;
}

cv::Mat RansacCurveFitting(int n_data,double noise_sigma/*100*/, double x[], double y[])
{
	cv::Mat A(n_data, 3, CV_64FC1);
	cv::Mat B(n_data, 1, CV_64FC1);

	for (int i = 0; i < 100; i++)		A.at<double>(i, 0) = x[i] * x[i];
	for (int i = 0; i < 100; i++)		A.at<double>(i, 1) = x[i];
	for (int i = 0; i < 100; i++)		A.at<double>(i, 2) = 1.0;
	for (int i = 0; i < 100; i++)		B.at<double>(i, 0) = y[i];

	//-------------------------------------------------------------- RANSAC fitting 
	int N = n_data;	//iterations 
	double T = 3 * noise_sigma;   // residual threshold

	int n_sample = 3;
	int max_cnt = 0;
	cv::Mat best_model(3, 1, CV_64FC1);

	for (int i = 0; i < N; i++)
	{
		//random sampling - 3 point  
		int k[3] = { -1, };
		k[0] = floor((rand() % N + 1)) + 1;

		do
		{
			k[1] = floor((rand() % N + 1)) + 1;
		} while (k[1] == k[0] || k[1] < 0);

		do
		{
			k[2] = floor((rand() % N + 1)) + 1;
		} while (k[2] == k[0] || k[2] == k[1] || k[2] < 0);

		//model estimation
		cv::Mat AA(3, 3, CV_64FC1);
		cv::Mat BB(3, 1, CV_64FC1);

		for (int j = 0; j < 3; j++)
		{
			AA.at<double>(j, 0) = x[k[j]] * x[k[j]];
			AA.at<double>(j, 1) = x[k[j]];
			AA.at<double>(j, 2) = 1.0;

			BB.at<double>(j, 0) = y[k[j]];
		}

		cv::Mat AA_pinv(3, 3, CV_64FC1);
		invert(AA, AA_pinv, cv::DECOMP_SVD);

		cv::Mat X = AA_pinv * BB;

		//evaluation 
		cv::Mat residual(N, 1, CV_64FC1);
		residual = cv::abs(B - A * X);

		int cnt = 0;
		for (int j = 0; j < N; j++)
		{
			double data = residual.at<double>(j, 0);

			if (data < T)
			{
				cnt++;
			}
		}

		if (cnt > max_cnt)
		{
			best_model = X;
			max_cnt = cnt;
		}
	}

	//------------------------------------------------------------------- optional LS fitting 
	cv::Mat residual = cv::abs(A * best_model - B);
	std::vector<int> vec_index;

	for (int i = 0; i < N; i++)
	{
		double data = residual.at<double>(i, 0);
		if (data < T)
		{
			vec_index.push_back(i);
		}
	}

	cv::Mat A2(int(vec_index.size()), 3, CV_64FC1);
	cv::Mat B2(int(vec_index.size()), 1, CV_64FC1);

	for (int i = 0; i < vec_index.size(); i++)
	{
		A2.at<double>(i, 0) = x[vec_index[i]] * x[vec_index[i]];
		A2.at<double>(i, 1) = x[vec_index[i]];
		A2.at<double>(i, 2) = 1.0;

		B2.at<double>(i, 0) = y[vec_index[i]];
	}

	cv::Mat A2_pinv(3, int(vec_index.size()), CV_64FC1);
	invert(A2, A2_pinv, cv::DECOMP_SVD);

	cv::Mat X = A2_pinv * B2;
	cv::Mat F = A * X;

	return F;
}

int RansacCircleFit(int N, double xp[], double yp[], double* centerx, double* centery, double* radius,
	int sample_th, double dist_th, int max_iter)
{
	double pr = double(sample_th) / double(N);
	double trials99 = log(1. - 0.99) / log(1. - pow(pr, 3));
	int iter = MIN(max_iter, trials99);
	int found = 0;
	;
	std::vector<double> consensusx(N), consensusy(N);
	double min_dev = 1.e+10, var, sdev;
	if (sample_th < 3) sample_th = 3;

	while (iter) {
		int tri[3];
		double tx[3], ty[3];
		GetRandomTriplet(N, tri);
		for (int i = 0; i < 3; i++) {
			tx[i] = xp[tri[i]]; ty[i] = yp[tri[i]];
		}
		double cx, cy, rad;
		if (!CircumCircle(tx[0], ty[0], tx[1], ty[1], tx[2], ty[2], &cx, &cy, &rad))

			continue;
		int ninlier = findInlier(xp, yp, N, cx, cy, rad, dist_th, &consensusx[0], &consensusy[0], &var);
		if (ninlier >= sample_th) {

			if (!CircleFit_LS(ninlier, &consensusx[0], &consensusy[0], &cx, &cy, &rad))			continue;

			ninlier = findInlier(xp, yp, N, cx, cy, rad, dist_th / 2, &consensusx[0], &consensusy[0], &var);
			if (ninlier < sample_th) continue;
			sdev = sqrt(var / ninlier);

			if (!CircleFit_LS(ninlier, &consensusx[0], &consensusy[0], &cx, &cy, &rad))				continue;

			if (sdev < min_dev) {
				*centerx = cx; *centery = cy;
				*radius = rad; min_dev = sdev;
				found = 1;
			}
		}
		--iter;
	}
	return found;
}

void fit_circle(const std::vector<cv::Point2d>& pnts, cv::Point2d& centre, double& radius)
{
	/*
				   A        B         C      R        R
		 G1:  +2*a*x^2 +2*b*x*y  +2*c*x +2*x^3    +2*x*y^2  = 0
		 G2:  +2*a*x*y +2*b*y^2  +2*c*y +2*y^3    +2*x^2*y  = 0
		 G3:  +2*a*x   +2*b*y    +2*c   +2*y^2    +2*x^2    = 0
	*/

	static const int rows = 3;
	static const int cols = 3;
	cv::Mat LHS(rows, cols, CV_64F, 0.0);
	cv::Mat RHS(rows, 1, CV_64F, 0.0);
	cv::Mat solution(rows, 1, CV_64F, 0.0);

	if (pnts.size() < 3)
	{
		throw std::runtime_error("To less points");
	}

	for (int i = 0; i < static_cast<int>(pnts.size()); i++)
	{
		double x1 = pnts[i].x;
		double x2 = std::pow(pnts[i].x, 2);
		double x3 = std::pow(pnts[i].x, 3);
		double y1 = pnts[i].y;
		double y2 = std::pow(pnts[i].y, 2);
		double y3 = std::pow(pnts[i].y, 3);

		// col 0 = A / col 1 = B / col 2 = C
		// Row 0 = G1
		LHS.at<double>(0, 0) += 2 * x2;
		LHS.at<double>(0, 1) += 2 * x1 * y1;
		LHS.at<double>(0, 2) += 2 * x1;

		RHS.at<double>(0, 0) -= 2 * x3 + 2 * x1 * y2;

		// Row 1 = G2
		LHS.at<double>(1, 0) += 2 * x1 * y1;
		LHS.at<double>(1, 1) += 2 * y2;
		LHS.at<double>(1, 2) += 2 * y1;

		RHS.at<double>(1, 0) -= 2 * y3 + 2 * x2 * y1;

		// Row 2 = G3
		LHS.at<double>(2, 0) += 2 * x1;
		LHS.at<double>(2, 1) += 2 * y1;
		LHS.at<double>(2, 2) += 2;

		RHS.at<double>(2, 0) -= 2 * y2 + 2 * x2;
	}

	cv::solve(LHS, RHS, solution);

	std::vector<double> abc{ solution.at<double>(0, 0),
							solution.at<double>(1, 0),
							solution.at<double>(2, 0) };

	centre.x = abc[0] / -2.0;
	centre.y = abc[1] / -2.0;
	radius = std::sqrt(std::abs(std::pow(centre.x, 2) + std::pow(centre.y, 2) - abc[2]));
}
//void getCircle(cv::Point& p1, cv::Point& p2, cv::Point& p3, cv::Point2f& center, float& radius)
//{
//	float x1 = p1.x;
//	float x2 = p2.x;
//	float x3 = p3.x;
//
//	float y1 = p1.y;
//	float y2 = p2.y;
//	float y3 = p3.y;
//
//	// PLEASE CHECK FOR TYPOS IN THE FORMULA :)
//	center.x = (x1 * x1 + y1 * y1) * (y2 - y3) + (x2 * x2 + y2 * y2) * (y3 - y1) + (x3 * x3 + y3 * y3) * (y1 - y2);
//	center.x /= (2 * (x1 * (y2 - y3) - y1 * (x2 - x3) + x2 * y3 - x3 * y2));
//
//	center.y = (x1 * x1 + y1 * y1) * (x3 - x2) + (x2 * x2 + y2 * y2) * (x1 - x3) + (x3 * x3 + y3 * y3) * (x2 - x1);
//	center.y /= (2 * (x1 * (y2 - y3) - y1 * (x2 - x3) + x2 * y3 - x3 * y2));
//
//	radius = sqrt((center.x - x1) * (center.x - x1) + (center.y - y1) * (center.y - y1));
//}

float verifyCircle(cv::Mat dt, cv::Point2f center, float radius, std::vector<cv::Point2f>& inlierSet)
{
	unsigned int counter = 0;
	unsigned int inlier = 0;
	float minInlierDist = 2.0f;
	float maxInlierDistMax = 100.0f;
	float maxInlierDist = radius / 25.0f;
	if (maxInlierDist < minInlierDist) maxInlierDist = minInlierDist;
	if (maxInlierDist > maxInlierDistMax) maxInlierDist = maxInlierDistMax;

	// choose samples along the circle and count inlier percentage
	for (float t = 0; t < 2 * 3.14159265359f; t += 0.05f)
	{
		counter++;
		float cX = radius * cos(t) + center.x;
		float cY = radius * sin(t) + center.y;

		if (cX < dt.cols)
			if (cX >= 0)
				if (cY < dt.rows)
					if (cY >= 0)
						if (dt.at<float>(cY, cX) < maxInlierDist)
						{
							inlier++;
							inlierSet.push_back(cv::Point2f(cX, cY));
						}
	}

	return (float)inlier / float(counter);
}

int evaluateCircle(vector<cv::Point2f>& pts, cv::Point2f center, float radius, float maxError)
{
	int Inliers = 0;
	double d = 0;
	for (int i = 0; i < pts.size(); i++)
	{
		d = GetDistance(center, pts[i]);
		if (abs(d - radius) < maxError)
			++Inliers;
	}
	return Inliers;
}


float evaluateCircle(cv::Mat dt, cv::Point2f center, float radius)
{
	float completeDistance = 0.0f;
	int counter = 0;
	float maxDist = 1.0f;   //TODO: this might depend on the size of the circle!
	float minStep = 0.001f;
	float step = 2 * 3.14159265359f / (6.0f * radius);
	if (step < minStep) step = minStep; // TODO: find a good value here.

	for (float t = 0; t < 2 * 3.14159265359f; t += step)
	{
		float cX = radius * cos(t) + center.x;
		float cY = radius * sin(t) + center.y;

		if (cX < dt.cols)
			if (cX >= 0)
				if (cY < dt.rows)
					if (cY >= 0)
						if (dt.at<float>(cY, cX) <= maxDist)
						{
							completeDistance += dt.at<float>(cY, cX);
							counter++;
						}

	}

	return counter;
}


std::vector<cv::Point2f> getPointPositions(cv::Mat binaryImage)
{
	std::vector<cv::Point2f> pointPositions;

	for (int y = 0; y < binaryImage.rows; ++y)
	{
		//unsigned char* rowPtr = binaryImage.ptr<unsigned char>(y);
		for (int x = 0; x < binaryImage.cols; ++x)
		{
			//if(rowPtr[x] > 0) pointPositions.push_back(cv::Point2i(x,y));
			if (binaryImage.at<unsigned char>(y, x) > 0) pointPositions.push_back(cv::Point2f(x, y));
		}
	}

	return pointPositions;
}

void fnDoRemoveNoise(int dMode, int width, int height, int widthStep, int i, int j, int threshold, int& hole, BOOL& is_hole, unsigned char* image, unsigned char* label_image)
{
	if (i <= 0 || j <= 0 || i >= height - 1 || j >= width - 1) return;

	switch (dMode)
	{
	case 0:			// check
		if (is_hole == FALSE) return;

		hole++;

		label_image[i * width + j] = 255;

		if (hole > threshold)
		{
			is_hole = FALSE;
			return;
		}
		if (label_image[(i - 1) * width + j] != 255)
		{
			if (image[(i - 1) * widthStep + j] == 255) fnDoRemoveNoise(dMode, width, height, widthStep, i - 1, j, threshold, hole, is_hole, image, label_image);
		}

		if (label_image[i * width + j - 1] != 255)
		{
			if (image[i * widthStep + j - 1] == 255) fnDoRemoveNoise(dMode, width, height, widthStep, i, j - 1, threshold, hole, is_hole, image, label_image);
		}

		if (label_image[i * width + j + 1] != 255)
		{
			if (image[i * widthStep + j + 1] == 255) fnDoRemoveNoise(dMode, width, height, widthStep, i, j + 1, threshold, hole, is_hole, image, label_image);
		}

		if (label_image[(i + 1) * width + j] != 255)
		{
			if (image[(i + 1) * widthStep + j] == 255) fnDoRemoveNoise(dMode, width, height, widthStep, i + 1, j, threshold, hole, is_hole, image, label_image);
		}
		break;

	case 1:			// remove
		image[i * widthStep + j] = 0;

		if (label_image[(i - 1) * width + j] == 255)
		{
			if (image[(i - 1) * widthStep + j] == 255)
			{
				image[(i - 1) * widthStep + j] = 255;
				fnDoRemoveNoise(dMode, width, height, widthStep, i - 1, j, threshold, hole, is_hole, image, label_image);
			}
		}

		if (label_image[i * width + j - 1] == 255)
		{
			if (image[i * widthStep + j - 1] == 255)
			{
				image[i * widthStep + j - 1] = 0;
				fnDoRemoveNoise(dMode, width, height, widthStep, i, j - 1, threshold, hole, is_hole, image, label_image);
			}
		}

		if (label_image[i * width + j + 1] == 255)
		{
			if (image[i * widthStep + j + 1] == 255)
			{
				image[i * widthStep + j + 1] = 255;
				fnDoRemoveNoise(dMode, width, height, widthStep, i, j + 1, threshold, hole, is_hole, image, label_image);
			}
		}

		if (label_image[(i + 1) * width + j] == 255)
		{
			if (image[(i + 1) * widthStep + j] == 255)
			{
				image[(i + 1) * widthStep + j] = 255;
				fnDoRemoveNoise(dMode, width, height, widthStep, i + 1, j, threshold, hole, is_hole, image, label_image);
			}
		}
		break;

	case 2:			// clear
		label_image[i * width + j] = 0;
		if (label_image[(i - 1) * width + j] == 255)
		{
			if (image[(i - 1) * widthStep + j] == 255)
			{
				label_image[(i - 1) * width + j] = 0;
				fnDoRemoveNoise(dMode, width, height, widthStep, i - 1, j, threshold, hole, is_hole, image, label_image);
			}
		}

		if (label_image[i * width + j - 1] == 255)
		{
			if (image[i * widthStep + j - 1] == 255)
			{
				label_image[i * width + j - 1] = 0;
				fnDoRemoveNoise(dMode, width, height, widthStep, i, j - 1, threshold, hole, is_hole, image, label_image);
			}
		}

		if (label_image[i * width + j + 1] == 255)
		{
			if (image[i * widthStep + j + 1] == 255)
			{
				label_image[i * width + j + 1] = 0;
				fnDoRemoveNoise(dMode, width, height, widthStep, i, j + 1, threshold, hole, is_hole, image, label_image);
			}
		}

		if (label_image[(i + 1) * width + j] == 255)
		{
			if (image[(i + 1) * widthStep + j] == 255)
			{
				label_image[(i + 1) * width + j] = 0;
				fnDoRemoveNoise(dMode, width, height, widthStep, i + 1, j, threshold, hole, is_hole, image, label_image);
			}
		}
		break;
	}
}

void fnDoRemoveNoiseBlack(int dMode, int width, int height, int widthStep, int i, int j, int threshold, int& hole, BOOL& is_hole, unsigned char* image, unsigned char* label_image)
{
	if (i <= 0 || j <= 0 || i >= height - 1 || j >= width - 1) return;

	switch (dMode)
	{
	case 0:			// check
		if (is_hole == FALSE) return;

		hole++;

		label_image[i * width + j] = 0;

		if (hole > threshold)
		{
			is_hole = FALSE;
			return;
		}
		if (label_image[(i - 1) * width + j] != 0)
		{
			if (image[(i - 1) * widthStep + j] == 0) fnDoRemoveNoise(dMode, width, height, widthStep, i - 1, j, threshold, hole, is_hole, image, label_image);
		}

		if (label_image[i * width + j - 1] != 0)
		{
			if (image[i * widthStep + j - 1] == 0) fnDoRemoveNoise(dMode, width, height, widthStep, i, j - 1, threshold, hole, is_hole, image, label_image);
		}

		if (label_image[i * width + j + 1] != 0)
		{
			if (image[i * widthStep + j + 1] == 0) fnDoRemoveNoise(dMode, width, height, widthStep, i, j + 1, threshold, hole, is_hole, image, label_image);
		}

		if (label_image[(i + 1) * width + j] != 0)
		{
			if (image[(i + 1) * widthStep + j] == 0) fnDoRemoveNoise(dMode, width, height, widthStep, i + 1, j, threshold, hole, is_hole, image, label_image);
		}
		break;

	case 1:			// remove
		image[i * widthStep + j] = 255;

		if (label_image[(i - 1) * width + j] == 0)
		{
			if (image[(i - 1) * widthStep + j] == 0)
			{
				image[(i - 1) * widthStep + j] = 0;
				fnDoRemoveNoiseBlack(dMode, width, height, widthStep, i - 1, j, threshold, hole, is_hole, image, label_image);
			}
		}

		if (label_image[i * width + j - 1] == 0)
		{
			if (image[i * widthStep + j - 1] == 0)
			{
				image[i * widthStep + j - 1] = 255;
				fnDoRemoveNoiseBlack(dMode, width, height, widthStep, i, j - 1, threshold, hole, is_hole, image, label_image);
			}
		}

		if (label_image[i * width + j + 1] == 0)
		{
			if (image[i * widthStep + j + 1] == 0)
			{
				image[i * widthStep + j + 1] = 0;
				fnDoRemoveNoiseBlack(dMode, width, height, widthStep, i, j + 1, threshold, hole, is_hole, image, label_image);
			}
		}

		if (label_image[(i + 1) * width + j] == 0)
		{
			if (image[(i + 1) * widthStep + j] == 0)
			{
				image[(i + 1) * widthStep + j] = 0;
				fnDoRemoveNoiseBlack(dMode, width, height, widthStep, i + 1, j, threshold, hole, is_hole, image, label_image);
			}
		}
		break;

	case 2:			// clear
		label_image[i * width + j] = 255;
		if (label_image[(i - 1) * width + j] == 0)
		{
			if (image[(i - 1) * widthStep + j] == 0)
			{
				label_image[(i - 1) * width + j] = 255;
				fnDoRemoveNoiseBlack(dMode, width, height, widthStep, i - 1, j, threshold, hole, is_hole, image, label_image);
			}
		}

		if (label_image[i * width + j - 1] == 0)
		{
			if (image[i * widthStep + j - 1] == 0)
			{
				label_image[i * width + j - 1] = 255;
				fnDoRemoveNoiseBlack(dMode, width, height, widthStep, i, j - 1, threshold, hole, is_hole, image, label_image);
			}
		}

		if (label_image[i * width + j + 1] == 0)
		{
			if (image[i * widthStep + j + 1] == 0)
			{
				label_image[i * width + j + 1] = 255;
				fnDoRemoveNoiseBlack(dMode, width, height, widthStep, i, j + 1, threshold, hole, is_hole, image, label_image);
			}
		}

		if (label_image[(i + 1) * width + j] == 0)
		{
			if (image[(i + 1) * widthStep + j] == 0)
			{
				label_image[(i + 1) * width + j] = 255;
				fnDoRemoveNoiseBlack(dMode, width, height, widthStep, i + 1, j, threshold, hole, is_hole, image, label_image);
			}
		}
		break;
	}
}

void fnRemoveNoise(cv::Mat &matImage, int nSize)
{
	//HTK 2022-05-19
	cv::Mat _matImage; 
	cv::resize(matImage, _matImage, cv::Size(), 0.5, 0.5, CV_INTER_CUBIC);
	// noise 제거
	unsigned char* pLabelBuf, * pBuffer;
	int		nWidthStep;
	int		nHoleCount;
	BOOL	bIsHole;
	int		i, j;
	int		nCropW, nCropH;
	pBuffer = (BYTE*)_matImage.data;
	nWidthStep = _matImage.cols;
	int nThresholdSize = nSize/2;

	nCropW = _matImage.cols;
	nCropH = _matImage.rows;

	pLabelBuf = new unsigned char[nCropW * nCropH];
	ZeroMemory(pLabelBuf, sizeof(BYTE) * nCropW * nCropH);

	for (i = 1; i < nCropH - 1; i++)
	{
		for (j = 1; j < nCropW - 1; j++)
		{
			if (pBuffer[i * nWidthStep + (j - 1)] == 0 && pBuffer[(i - 1) * nWidthStep + j] == 0 && pBuffer[i * nWidthStep + j] == 255 && pLabelBuf[i * nCropW + j] == 0)
			{
				bIsHole = TRUE;
				nHoleCount = 0;
				fnDoRemoveNoise(0, nCropW, nCropH, nWidthStep, i, j, nThresholdSize, nHoleCount, bIsHole, pBuffer, pLabelBuf);

				if (bIsHole == TRUE)
				{
					fnDoRemoveNoise(1, nCropW, nCropH, nWidthStep, i, j, nThresholdSize, nHoleCount, bIsHole, pBuffer, pLabelBuf);
				}
				else
				{
					fnDoRemoveNoise(2, nCropW, nCropH, nWidthStep, i, j, nThresholdSize, nHoleCount, bIsHole, pBuffer, pLabelBuf);
				}
			}
		}
	}

	delete[] pLabelBuf;
	cv::resize(_matImage, matImage, cv::Size(), 2, 2, CV_INTER_CUBIC);

	_matImage.release();
}

void fnRemoveNoiseBlack(cv::Mat &matImage, int nSize)
{
	cv::Mat _matImage = cv::Mat::zeros(matImage.rows, matImage.cols, CV_8U);
	bitwise_not(matImage, _matImage);
	fnRemoveNoise(_matImage, nSize);
	bitwise_not(_matImage, matImage);
	_matImage.release();
}

// 20210923 제외 영역 사용 하기 위함
int findMultipleCircle(const cv::Mat& gray, std::vector<cv::Vec3f>& circles, double chole, double shole, int rad , int hthresh, bool bauto, cv::Mat* Maskcolor,
	int _start, int _end, bool _bsplit )
{
	circles.clear();

	cv::Mat canny, mask;

	cv::Mat cirOrgImg = cv::Mat::zeros(gray.size(), gray.type());
	cv::Mat cirmask = cv::Mat::zeros(gray.size(), gray.type());
	cv::Mat cirmask2 = cv::Mat::zeros(gray.size(), gray.type());
	if (Maskcolor == NULL)
	{
		cv::circle(cirmask2, cv::Point(cirmask2.cols / 2, cirmask2.rows / 2), shole + 30, cv::Scalar(255, 255, 255), -1, 8, 0);
		cv::circle(cirmask, cv::Point(cirmask.cols / 2, cirmask.rows / 2), chole + 40, cv::Scalar(255, 255, 255), -1, 8, 0);
		cv::subtract(cirmask2, cirmask, cirmask);
	}
	else cirmask = *Maskcolor;

	if (bauto)	cv::threshold(gray, canny, 128, 255, CV_THRESH_BINARY_INV | THRESH_OTSU);
	else threshold(gray, canny, hthresh, 255, CV_THRESH_BINARY_INV);

	fnRemoveNoise(canny, 500);

// search first point
	cv::Point2f rC;
	uchar* data = cirOrgImg.data;
	uchar* dataOrg = canny.data;
	int x;
	int y;
	int idx = 0;

	for (int i = 0; i < 360; i++)
	{
		if (_start != -1 && _end != -1)
		{
			if ((_start<i && _end>i) || (_bsplit && i > 315))		continue; //20210923 Tkyuha 제외 영역 제외
		}
		for (int distanceT = 4; distanceT < 300; distanceT++)
		{
			int _rad = rad + distanceT;

			x = (cos(ToRadian(i)) * _rad + cirOrgImg.cols / 2);
			y = (sin(ToRadian(i)) * _rad + cirOrgImg.rows / 2);
		
			if (x<0 || x>=canny.cols) continue;
			if (y<0 || y>=canny.rows) continue;

			idx = y * canny.cols + x;

			if (dataOrg[idx] > 0)
			{
				data[idx] = 255;
				break;
			}
		}
	}

	cv::Canny(cirOrgImg, canny, 5, 70);	

	cv::Mat maskCanny = canny > 0;
	cv::dilate(maskCanny, maskCanny, cv::Mat());
	cv::erode(maskCanny, maskCanny, cv::Mat());
	//cv::bitwise_and(cirmask, maskCanny, mask);
	maskCanny.copyTo(mask, cirmask);

	unsigned int numberOfCirclesToDetect = 1;   // TODO: if unknown, you'll have to find some nice criteria to stop finding more (semi-) circles

	//for (unsigned int j = 0; j < numberOfCirclesToDetect; ++j)
	//{
	//	std::vector<cv::Point2f> edgePositions;
	//	edgePositions = getPointPositions(mask);

	//	if (edgePositions.size() > 3)
	//	{
	//		cv::Mat dt;
	//		cv::distanceTransform(255 - mask, dt, CV_DIST_L1, 3);

	//		unsigned int nIterations = 0;
	//		cv::Point2f bestCircleCenter = cv::Point2f(0, 0);
	//		float bestCircleRadius = -1;
	//		float bestCVal = -1;
	//		float minCircleRadius = float(chole);

	//		for (unsigned int i = 0; i < 20; ++i)
	//		{
	//			unsigned int idx1 = rand() % edgePositions.size();
	//			unsigned int idx2 = rand() % edgePositions.size();
	//			unsigned int idx3 = rand() % edgePositions.size();

	//			if (idx1 == idx2) continue;
	//			if (idx1 == idx3) continue;
	//			if (idx3 == idx2) continue;

	//			cv::Point2f center; float radius;
	//			getCircle(edgePositions[idx1], edgePositions[idx2], edgePositions[idx3], center, radius);

	//			//if (radius < minCircleRadius)continue;

	//			float cVal = evaluateCircle(dt, center, radius);

	//			if (cVal > bestCVal)
	//			{
	//				bestCVal = cVal;
	//				bestCircleRadius = radius;
	//				bestCircleCenter = center;
	//			}

	//			++nIterations;
	//		}
	//		if (bestCircleRadius < gray.cols && bestCircleRadius>0)
	//		{
	//			cv::circle(mask, bestCircleCenter, bestCircleRadius, 0, 10);
	//			circles.push_back(cv::Vec3f(bestCircleCenter.x, bestCircleCenter.y, bestCircleRadius));
	//		}
	//	}
	// 
	//  edgePositions.clear();
	//}

	// Ransac 개선된 알고리즘  20220117 
	std::vector<cv::Point> pts;

	std::vector<cv::Point2f> edgePositions;
	edgePositions = getPointPositions(mask);

	if (edgePositions.size() > 3)
	{
		for (int i = 0; i < edgePositions.size() - 1; i++)
		{
			cv::Point pt = cv::Point(edgePositions[i].x, edgePositions[i].y);
			pts.push_back(pt);
		}
	
		double _cx, _cy, _rad;

		for (unsigned int j = 0; j < numberOfCirclesToDetect; ++j)
		{
			MLSAC_CircleFitting(pts, _cx, _cy, _rad);
			circles.push_back(cv::Vec3f(_cx, _cy, _rad));
		}
	}

	pts.clear();
	edgePositions.clear();

	canny.release();
	mask.release();
	cirOrgImg.release();
	cirmask.release();
	cirmask2.release();
	maskCanny.release();

	return 0;
}

int findOrgImageCircle(const cv::Mat& gray, std::vector<cv::Vec3f>& circles, double chole, int rad)
{
	circles.clear();

	cv::Mat mask = gray > 0;
	cv::dilate(mask, mask, cv::Mat());
	cv::erode(mask, mask, cv::Mat());

	unsigned int numberOfCirclesToDetect = 1;   // TODO: if unknown, you'll have to find some nice criteria to stop finding more (semi-) circles

	for (unsigned int j = 0; j < numberOfCirclesToDetect; ++j)
	{
		std::vector<cv::Point2f> edgePositions;
		edgePositions = getPointPositions(gray);

		if (edgePositions.size() > 3)
		{
			cv::Mat dt;
			cv::distanceTransform(255 - mask, dt, CV_DIST_L1, 3);

			unsigned int nIterations = 0;
			cv::Point2f bestCircleCenter = cv::Point2f(0, 0);
			float bestCircleRadius = -1;
			float bestCVal = -1;
			float minCircleRadius = float(chole);

			for (unsigned int i = 0; i < 20; ++i)
			{
				unsigned int idx1 = rand() % edgePositions.size();
				unsigned int idx2 = rand() % edgePositions.size();
				unsigned int idx3 = rand() % edgePositions.size();

				if (idx1 == idx2) continue;
				if (idx1 == idx3) continue;
				if (idx3 == idx2) continue;

				cv::Point2f center; float radius;
				getCircle(edgePositions[idx1], edgePositions[idx2], edgePositions[idx3], center, radius);

				//if (radius < minCircleRadius)continue;

				float cVal = evaluateCircle(dt, center, radius);

				if (cVal > bestCVal)
				{
					bestCVal = cVal;
					bestCircleRadius = radius;
					bestCircleCenter = center;
				}

				++nIterations;
			}
			if (bestCircleRadius < gray.cols && bestCircleRadius>0)
			{
				cv::circle(mask, bestCircleCenter, bestCircleRadius, 0, 10);
				circles.push_back(cv::Vec3f(bestCircleCenter.x, bestCircleCenter.y, bestCircleRadius));
			}
		}

		edgePositions.clear();
	}

	return 0;
}

// Ransac 개선된 알고리즘  20220117 
int findOrgImageLMERanscCircle(const cv::Mat& gray, std::vector<cv::Vec3f>& circles, double chole, int rad)
{
	double _cx, _cy, _rad;
	std::vector<cv::Point> pts;

	circles.clear();

	cv::Mat mask = gray > 0;
	cv::dilate(mask, mask, cv::Mat());
	cv::erode(mask, mask, cv::Mat());

	unsigned int numberOfCirclesToDetect = 1;   

	std::vector<cv::Point2f> edgePositions;
	edgePositions = getPointPositions(gray);

	if (edgePositions.size() > 3)
	{
		for (int i = 0; i < edgePositions.size() - 1; i++)
		{
			cv::Point pt = cv::Point(edgePositions[i].x, edgePositions[i].y);
			pts.push_back(pt);
		}
			
		for (unsigned int j = 0; j < numberOfCirclesToDetect; ++j)
		{
			MLSAC_CircleFitting(pts, _cx, _cy, _rad);
			circles.push_back(cv::Vec3f(_cx, _cy, _rad));
		}
	}

	pts.clear();
	edgePositions.clear();

	return 0;
}

int findMultipleHoughCircle(const cv::Mat& gray, std::vector<cv::Vec3f>& circles, cv::Mat& color)
{
	circles.clear();

	cv::Mat canny;
	cv::Canny(gray, canny, 50, 200);
	cv::HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, gray.rows / 8, 100, 30, 100, 400);

	for (size_t i = 0; i < circles.size(); i++)
	{
		cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);
		cv::circle(color, center, 3, Scalar(0, 255, 255), -1);
		cv::circle(color, center, radius, Scalar(0, 0, 255), 1);
	}

	cv::Mat dt;
	cv::distanceTransform(255 - (canny > 0), dt, CV_DIST_L2, 3);

	float minInlierDist = 2.0f;
	for (size_t i = 0; i < circles.size(); i++)
	{
		unsigned int counter = 0;
		unsigned int inlier = 0;

		cv::Point2f center((circles[i][0]), (circles[i][1]));
		float radius = (circles[i][2]);

		float maxInlierDist = radius / 25.0f;
		if (maxInlierDist < minInlierDist) maxInlierDist = minInlierDist;

		for (float t = 0; t < 2 * 3.14159265359f; t += 0.1f)
		{
			counter++;
			float cX = radius * cos(t) + circles[i][0];
			float cY = radius * sin(t) + circles[i][1];
			if (cX >= color.cols || cY >= color.rows || cY < 0 || cX < 0) continue;

			if (dt.at<float>(cY, cX) < maxInlierDist)
			{
				inlier++;
				cv::circle(color, cv::Point2i(cX, cY), 3, cv::Scalar(0, 255, 0));
			}
			else
				cv::circle(color, cv::Point2i(cX, cY), 3, cv::Scalar(255, 0, 0));
		}
	}

	return 0;
}

// Lincoln Lee - 220725 - Adding BMIN inspection at PF inspection
std::vector<cv::Point> circularScanning(const cv::Mat& src, cv::Point2f center, float startRadius, float endRadius, float deltaTheta, int thresh, LIGHTING_POLARITY polarity)
{
	int x, y;
	int w = src.cols;
	int h = src.rows;
	int w2 = w - 2;
	int h2 = h - 2;
	std::vector<cv::Point> pts;

	for (float f = 0; f < 360; f += deltaTheta)
	{
		float _radian = f * CV_PI / 180.;
		float _sin = sin(_radian);
		float _cos = cos(_radian);

		int diff = 0;
		int thisVal = 0;
		int lastVal = 0;
		int start_ = lround(startRadius);
		int end_ = lround(endRadius);

		for (int _radius = start_; _radius < end_; _radius++)
		{
			x = lround(_radius * _cos + center.x);
			y = lround(_radius * _sin + center.y);

			if (x <1 || x>w2 || y<1 || y>h2) break;

			thisVal = src.at<uchar>(y, x) + src.at<uchar>(lround(y + _sin), lround(x + _cos)) + src.at<uchar>(lround(y - _sin), lround(x - _cos));
			if (_radius != start_)
			{
				diff = (polarity == LIGHTING_POLARITY::LIGHT_TO_DARK) ? lastVal - thisVal: ((polarity == LIGHTING_POLARITY::DARK_TO_LIGHT) ? thisVal - lastVal: abs(thisVal - lastVal));
				if (diff > thresh)
				{
					pts.push_back({ x, y });
					break;
				}
			}
			lastVal = thisVal;
		}
	}

	return pts;
}

// Lincoln Lee - 220725 - Adding BMIN inspection at PF inspection
std::vector<cv::Point> circularScanning(const cv::Mat& src, cv::Point2f center, float startRadius, float endRadius, float deltaTheta, int thresh, cv::Point offs, LIGHTING_POLARITY polarity, UINT* valids)
{
	int x, y;
	int w = src.cols;
	int h = src.rows;
	int w2 = w - 2;
	int h2 = h - 2;
	std::vector<cv::Point> pts;
	UINT valid_ = 0;

	for (float f = 0; f < 360; f += deltaTheta)
	{
		float _radian = f * CV_PI / 180.;
		float _sin = sin(_radian);
		float _cos = cos(_radian);

		int diff = 0;
		int thisVal = 0;
		int lastVal = 0;
		int start_ = lround(startRadius);
		int end_ = lround(endRadius);

		for (int _radius = start_; _radius < end_; _radius++)
		{
			x = lround(_radius * _cos + center.x);
			y = lround(_radius * _sin + center.y);

			if (x <1 || x>w2 || y<1 || y>h2) break;

			thisVal = src.at<uchar>(y, x) + src.at<uchar>(lround(y + _sin), lround(x + _cos)) + src.at<uchar>(lround(y - _sin), lround(x - _cos));
			if (_radius != start_)
			{
				diff = (polarity == LIGHTING_POLARITY::LIGHT_TO_DARK) ? lastVal - thisVal: ((polarity == LIGHTING_POLARITY::DARK_TO_LIGHT) ? thisVal - lastVal: abs(thisVal - lastVal));
				if (diff > thresh)
				{
					pts.push_back({ x + offs.x, y + offs.y});
					++valid_;
					break;
				}
			}
			lastVal = thisVal;
		}
	}
	if (NULL != valids) *valids = valid_;
	return pts;
}

// Lincoln Lee - 220725 - Adding BMIN inspection at PF inspection
std::vector<cv::Point> circularScanning(const cv::Mat& src, cv::Point2f center, float startRadius, float endRadius, float deltaTheta, int thresh, cv::Point offs, std::vector<bool>& founds, LIGHTING_POLARITY polarity, UINT* valids)
{
	int x, y;
	int w = src.cols;
	int h = src.rows;
	int w2 = w - 2;
	int h2 = h - 2;
	std::vector<cv::Point> pts;
	UINT valid_ = 0;

	founds.clear();
	for (float f = 0; f < 360; f += deltaTheta)
	{
		float _radian = f * CV_PI / 180.;
		float _sin = sin(_radian);
		float _cos = cos(_radian);

		int diff = 0;
		int thisVal = 0;
		int lastVal = 0;
		int start_ = lround(startRadius);
		int end_ = lround(endRadius);
		bool found = false;

		for (int _radius = start_; _radius < end_; _radius++)
		{
			x = lround(_radius * _cos + center.x);
			y = lround(_radius * _sin + center.y);

			if (x <1 || x>w2 || y<1 || y>h2) break;

			thisVal = src.at<uchar>(y, x) + src.at<uchar>(lround(y + _sin), lround(x + _cos)) + src.at<uchar>(lround(y - _sin), lround(x - _cos));
			if (_radius != start_)
			{
				diff = (polarity == LIGHTING_POLARITY::LIGHT_TO_DARK) ? lastVal - thisVal: ((polarity == LIGHTING_POLARITY::DARK_TO_LIGHT) ? thisVal - lastVal: abs(thisVal - lastVal));
				if (diff > thresh)
				{
					pts.push_back({ x + offs.x, y + offs.y });
					++valid_;
					found = true;
					break;
				}
			}
			lastVal = thisVal;
		}
		founds.push_back(found);
	}
	if (NULL != valids) *valids = valid_;
	return pts;
}

////OK = true
////NG = false
//// roi = cv::Rect(1720, 1390, 520, 500)
//// becus ROI may need fixture, so before calling this function please do fixture with ROI.
//// Lincoln Lee - 220725 - Adding BMIN inspection at PF inspection
//bool InspFilm_BMIN_check(const cv::Mat& src, cv::Rect roi, double mmScale, double fistSpec, double secondSpec, CDC* pDC)
//{
//	cv::Mat subMat, blurMat, subMat2, blurMat2;
//
//	auto tl = roi.tl();
//	auto br = roi.br();
//
//	// Contrains to make sure ROI always inside image
//	auto tl_ = cv::Point(MAX(tl.x, 0), MAX(tl.y, 0));
//	auto br_ = cv::Point(MIN(br.x, src.cols), MIN(br.y, src.rows));
//	cv::Rect roi_(tl_, br_);
//	auto w = roi_.width;
//	auto h = roi_.height;
//	float w2 = w / 2., h2 = h / 2.;
//	int w4 = lround(w / 4.), h4 = lround(h / 4.);
//	int w8 = lround(w / 8.), h8 = lround(h / 8.);
//
//	subMat = src(roi_).clone();
//	cv::medianBlur(subMat, blurMat, 13);
//
//	int thresh = 15;
//	float deltaT0 = 9;
//	float deltaT1 = 1; // nPoints = 360 / 9 = 40 (take 50%) -> 20 points
//	bool result = true;
//	int minPoints0 = lround((360. / deltaT0) * 0.5);  // NumOfPoints > 50% -> Valid
//	int minPoints1 = lround((360. / deltaT1) * 0.6);  // NumOfPoints > 60% -> Valid
//
//	std::vector<cv::Point> innerPoints, middlePoints, outterPoints;
//	double innerCX = 0, innerCY = 0, innerRadius = 0;
//	double middleCX = 0, middleCY = 0, middleRadius = 0;
//	double outterCX = 0, outterCY = 0, outterRadius = 0;
//
//	// 40 calipers - finding for inner circle
//	innerPoints = circularScanning(blurMat, cv::Point2f(w2, h2), w4 - w8, w4 + w8, deltaT0, thresh, LIGHTING_POLARITY::LIGHT_TO_DARK);
//
//	if (innerPoints.size() > minPoints0)
//	{
//		MLSAC_CircleFitting(innerPoints, innerCX, innerCY, innerRadius);
//
//		// self fixture 
//		roi_.x += lround(innerCX - w2);
//		roi_.y += lround(innerCY - h2);
//
//		subMat2 = src(roi_).clone();
//		cv::medianBlur(subMat2, blurMat2, 7);
//
//		middlePoints = circularScanning(blurMat2, cv::Point2f(innerCX, innerCY), innerRadius, innerRadius + w8, deltaT1, thresh, LIGHTING_POLARITY::DARK_TO_LIGHT);
//		if (middlePoints.size() > minPoints1)
//		{
//			MLSAC_CircleFitting(middlePoints, middleCX, middleCY, middleRadius);
//
//			//outterPoints = circularScanning(blurMat2, cv::Point2f(middleCX, middleCY), middleRadius, middleRadius + w8, deltaT1, thresh, LIGHTING_POLARITY::LIGHT_TO_DARK);
//			//if (outterPoints.size() > minPoints1)
//			//	MLSAC_CircleFitting(outterPoints, outterCX, outterCY, outterRadius);
//			//else result = false;
//		}
//		else result = false;
//	}
//	else result = false;
//
//	auto rDiff0 = (middleRadius - innerRadius) * mmScale;
//	//auto rDiff1 = (outterRadius - middleRadius) * mmScale;
//
//	if (fistSpec > 0)
//	{
//		result &= rDiff0 > fistSpec; // Radius distance
//		//result &= rDiff1 > fistSpec;
//	}
//
//	if (secondSpec > 0)
//	{
//		result &= (GetDistance(cv::Point2f(innerCX, innerCY), cv::Point2f(middleCX, middleCY)) * mmScale) < secondSpec; // Center of circle distance
//		//result &= (GetDistance(cv::Point2f(middleCX, middleCY), cv::Point2f(outterCX, outterCY)) * mmScale) < secondSpec;
//	}
//
//	if (NULL != pDC)
//	{
//		CPen penRED(PS_SOLID, 3, RGB(255, 0, 0));
//		CPen penBLUE(PS_SOLID, 3, RGB(0, 0, 255));
//		CPen penGREEN(PS_SOLID, 3, RGB(0, 255, 0));
//		CPen penMAGENTA(PS_SOLID, 3, RGB(255, 0, 255));
//		
//		pDC->SelectStockObject(NULL_BRUSH);
//
//		/*
//		// 161, 191, 208
//		//result &= ((innerRadius - inner_) < vInner) && ((middleRadius - middle_) < vMiddle) && ((outterRadius - outter_) < vOutter);
//		//result &= GetDistance(cv::Point2f(innerCX, innerCY), cv::Point2f(middleCX, middleCY)) < distanceInnerMiddleThresh;
//		//result &= GetDistance(cv::Point2f(middleCX, middleCY), cv::Point2f(outterCX, outterCY)) < distanceMiddleOutterThresh;
//
//		//if (fistSpec > 0) result &= (GetDistance(cv::Point2f(innerCX, innerCY), cv::Point2f(middleCX, middleCY)) * mmScale) < fistSpec;
//		//if(secondSpec > 0)	result &= (GetDistance(cv::Point2f(middleCX, middleCY), cv::Point2f(outterCX, outterCY)) * mmScale) < secondSpec;
//		*/
//
//		if (result) // Totally OK
//		{
//			pDC->SelectObject(&penMAGENTA); 
//			pDC->Ellipse(GetRect(innerCX + tl_.x, innerCY + tl_.y, innerRadius));
//			PlotPoints(pDC, innerPoints, &penMAGENTA, tl_);
//
//			pDC->SelectObject(&penGREEN);
//			pDC->Ellipse(GetRect(middleCX + roi_.x, middleCY + roi_.y, middleRadius));
//			//PlotPoints(pDC, middlePoints, &penGREEN, roi_.tl());
//
//			pDC->SelectObject(&penBLUE);
//			pDC->Ellipse(GetRect(outterCX + roi_.x, outterCY + roi_.y, outterRadius));
//			//PlotPoints(pDC, outterPoints, &penBLUE, roi_.tl());
//		}
//		else
//		{
//			pDC->SelectObject(&penRED); 
//			pDC->Ellipse(GetRect(innerCX + roi_.x, innerCY + roi_.y, innerRadius));
//			PlotPoints(pDC, innerPoints, &penRED, roi_.tl());
//			pDC->Ellipse(GetRect(middleCX + roi_.x, middleCY + roi_.y, middleRadius));
//			PlotPoints(pDC, middlePoints, &penRED, roi_.tl());
//			pDC->Ellipse(GetRect(outterCX + roi_.x, outterCY + roi_.y, outterRadius));
//			PlotPoints(pDC, outterPoints, &penRED, roi_.tl());
//		}
//		pDC->Rectangle(roi_.x, roi_.y, roi_.x + roi_.width, roi_.y + roi_.height);
//
//		penRED.DeleteObject();
//		penBLUE.DeleteObject();
//		penGREEN.DeleteObject();
//		penMAGENTA.DeleteObject();
//	}
//
//	innerPoints.clear();
//	middlePoints.clear();
//	outterPoints.clear();
//	subMat.release();
//	blurMat.release();
//	subMat2.release();
//	blurMat2.release();
//
//	return result;
//}

//KJH 2022-02-03 CircleAlign rename
double FindCircleAlign_PN(const cv::Mat& gray, int sx, int sy, double radius, int cirsize, std::vector<cv::Vec3f>& circles, bool bauto, int hth, FakeDC* pDC)
{
	/*
	GaussianBlur -> threshold(INV)) -> erode -> fnRemoveNoiseBlack -> basic +0 ~ 500 search
	*/
	double x, y, errorloss=0.0;
	int colC = gray.cols / 2, rowC = gray.rows / 2;
	int rposy, rposx;
	bool _bcircleEdgeFind = false;
	cv::Mat bin;

	cv::GaussianBlur(gray, bin, cv::Size(7, 7), 3.);

	if (bauto)	cv::threshold(bin, bin, -1, 255, CV_THRESH_BINARY_INV | THRESH_OTSU);
	else		cv::threshold(bin, bin, hth, 255, CV_THRESH_BINARY_INV);

	// 잡음 제거
	erode(bin, bin, Mat::ones(cv::Size(3, 3), CV_8UC1), cv::Point(-1, -1), 2);

	// 검은색 구멍 뚤린 부분 채우는 부분
	//fnRemoveNoiseBlack(bin, 800);

	uchar* Raw = bin.data;
	std::vector<cv::Point2f> edgePositions;
	CPointF<double> p_center(colC, rowC);

	// Panel edge 찾는 부분
	for (int second = 0; second < 360; second++)
	{
		_bcircleEdgeFind = true;
		// 원위의 점 찾기
		for (int distanceT = 0; distanceT < 500; distanceT++)
		{
			int _rad = cirsize + distanceT;
			rposx = (cos(ToRadian(second)) * _rad + colC);
			rposy = (sin(ToRadian(second)) * _rad + rowC);

			if (rposx < 0 || rposx >= colC * 2) continue;
			if (rposy < 0 || rposy >= rowC * 2) continue;

			CPointF<int> p2(rposx, rposy);
			if (Raw[rposy * bin.cols + rposx] != 255)
			{
				double _S = GetDistance(p_center, p2);
				if (radius - 200 < _S && radius + 200 > _S)
				{
					edgePositions.push_back(cv::Point2f(rposx, rposy));
					_bcircleEdgeFind = false;
					break;
				}
			}
		}
		if (_bcircleEdgeFind)  // Edge를 못찾는 경우 가상의 데이타 추가
		{
			edgePositions.push_back(cv::Point2f(-1, -1)); // 20211101 Tkyuha 원의 Edge를 못찾는 경우 추가
		}
	}

	// 20210909 Tkyuha 포인트 선택 영역 계산 8등분 해서 면적이 넓은 영역 6등분 선택
	// 메탈과 글라스 사이 검은 영역 면적 계산

	int _start = 0;
	int _end = int(edgePositions.size());
	bool _bsplit = false;

	if (edgePositions.size() > 10)
	{
		// 8방향에 대해 metal 엣지를 찾아서 panel hole과 metal hole 간의 거리 계산
		int _step = int(edgePositions.size() / 8);

		std::vector<pair<int, int>> _v;
		bool _bfind = false;

		for (int i = 0; i < edgePositions.size(); i += _step)
		{
			CPointF<int> _p(edgePositions[i].x, edgePositions[i].y);
			int _Srad = (int)GetDistance(p_center, _p);
			_bfind = false;

			for (int distanceT = 5; distanceT < 150; distanceT++) // 반지름 크기가 5픽셀 크기 부터 200 픽셀만 검사 
			{
				int _rad = _Srad + distanceT;  //반지름 계산

				rposx = (cos(ToRadian(i)) * _rad + colC);  // 회전 중심에 의한 좌표
				rposy = (sin(ToRadian(i)) * _rad + rowC);

				if (rposx < 0 || rposx >= colC * 2) continue;
				if (rposy < 0 || rposy >= rowC * 2) continue;
				CPointF<int> p2(rposx, rposy);
				if (Raw[rposy * bin.cols + rposx] == 255)
				{
					int _S = (int)GetDistance(_p, p2);
					_v.push_back(pair<int, int>(_S, i));
					_bfind = true;

					pDC->Ellipse(int(rposx + sx - 20), int(rposy + sy - 20), int(rposx + sx + 20), int(rposy + sy + 20));
					break;
				}
			}

			if (_bfind == false) _v.push_back(pair<int, int>(0, i));
		}

		sort(_v.begin(), _v.end());

		//Trace 예외 변경 영역
		if (_v.size() > 7)  // Tkyuha 범위 설정 수정 20220103
		{
			int max_index = _v[_v.size() - 1].second;
			int _revID = (max_index + 180) % 360;

			_end = (_revID + 45) % 360;
			_start = (_revID - 45 + 360) % 360;

			if (abs(_end - _start) > 180)
			{
				_start = 0;
				_end = 45;
				_bsplit = true;
			}
		}
		else  _end = 0;

		_v.clear();
	}

	///////// <>

	if (pDC != NULL)
	{
		CPen penE(PS_SOLID, 3, RGB(255, 0, 0));
		CPen penY(PS_SOLID, 3, RGB(0, 255, 0));

		int r = 4;
		for (int i = 0; i < edgePositions.size(); i++)
		{
			x = edgePositions[i].x + sx;
			y = edgePositions[i].y + sy;
			if ((_start < i && _end > i) || (_bsplit && i > 315))		pDC->SelectObject(&penE); //제외 영역 표시
			else														pDC->SelectObject(&penY);

			pDC->Ellipse(int(x - r), int(y - r), int(x + r), int(y + r));
		}

		pDC->SelectObject(&penE);
		penE.DeleteObject();
		penY.DeleteObject();
	}

	if (edgePositions.size() > 10)  // 10개 이상 포인트를 찾은 경우만 검사 진행
	{
		int sCount = 0, margin = 3, _realCount = 0;
		double cx = 0, cy = 0, rad = 0;
		double  avgdistance = 0;
		double* XiData = new double[edgePositions.size()];
		double* YiData = new double[edgePositions.size()];
		double* DistData = new double[edgePositions.size()];

		//outlier 제거
		for (int i = 0; i < edgePositions.size() - 1; i++) // 마지막 하나는 빼고 수행 ,예외처리함
		{
			CPointF<int> p2(edgePositions[i].x, edgePositions[i].y);
			CPointF<int> p1(edgePositions[i + 1].x, edgePositions[i + 1].y);

			if ((p2.x == -1 && p2.y == -1) || p1.x == -1 && p1.y == -1) continue;

			XiData[_realCount] = GetDistance(p1, p2);
			DistData[_realCount] = XiData[_realCount];
			_realCount++;
		}

		std::sort(XiData, XiData + _realCount);
		avgdistance = XiData[_realCount / 2];  // 중앙값을 선택

		int _continueC = 0;
		for (int i = 0; i < _realCount; i++)
		{
			if (DistData[i] > (avgdistance - margin) && DistData[i] < (avgdistance + margin))
			{
				_continueC++;

				if (_continueC >= 5)
				{
					XiData[sCount] = edgePositions[i].x;
					YiData[sCount] = edgePositions[i].y;
					sCount++;
				}
			}
			else _continueC = 0;
		}


		if (sCount > 10)
		{
			std::vector<cv::Point> pts;
			for (int i = 0; i < sCount; i++) {
				cv::Point pt = cv::Point(XiData[i], YiData[i]);
				pts.push_back(pt);
			}
			errorloss = MLSAC_CircleFitting(pts, cx, cy, rad);
			pts.clear();

			//CircleFit_LS(sCount, XiData, YiData, &cx, &cy, &rad);
		}

		circles.push_back(cv::Vec3f(cx, cy, rad));

		delete XiData;
		delete YiData;
		delete DistData;
	}

	edgePositions.clear();
	bin.release();

	return errorloss;
}

double FindCircleAlign_PN(const cv::Mat& gray, int sx, int sy, double radius, int cirsize, std::vector<cv::Vec3f>& circles, std::vector<cv::Point2f>* pEdgeposition, bool bauto, int hth, FakeDC* pDC)
{
	/*
	GaussianBlur -> threshold(INV)) -> erode -> fnRemoveNoiseBlack -> basic +0 ~ 500 search
	*/
	double x, y,errorloss=0.0;
	int colC = gray.cols / 2, rowC = gray.rows / 2;
	int rposy, rposx;
	bool _bcircleEdgeFind = false;
	cv::Mat bin;

	cv::GaussianBlur(gray, bin, cv::Size(7, 7), 3.);

	if (bauto)	cv::threshold(bin, bin, -1, 255, CV_THRESH_BINARY_INV | THRESH_OTSU);
	else		cv::threshold(bin, bin, hth, 255, CV_THRESH_BINARY_INV);

	// 잡음 제거
	erode(bin, bin, Mat::ones(cv::Size(3, 3), CV_8UC1), cv::Point(-1, -1), 2);

	// 검은색 구멍 뚤린 부분 채우는 부분
	fnRemoveNoiseBlack(bin, 800);

	//cv::imwrite("D:\\fnRemoveNoiseBlack.jpg", bin);

	uchar* Raw = bin.data;
	std::vector<cv::Point2f> edgePositions;
	CPointF<double> p_center(colC, rowC);


	// KBJ 2022-09-08
	int AginJumpCount = 0;
AginCenterAlign:
	p_center.x = colC;
	p_center.y = rowC;

	edgePositions.clear();
	pEdgeposition->clear();

	// Panel edge 찾는 부분
	for (int second = 0; second < 360; second++)
	{
		_bcircleEdgeFind = true;
		// 원위의 점 찾기
		for (int distanceT = 0; distanceT < 500; distanceT++)
		{
			int _rad = cirsize + distanceT;
			rposx = (cos(ToRadian(second)) * _rad + colC);
			rposy = (sin(ToRadian(second)) * _rad + rowC);

			if (rposx < 0 || rposx >= colC * 2) continue;
			if (rposy < 0 || rposy >= rowC * 2) continue;

			CPointF<int> p2(rposx, rposy);
			if (Raw[rposy * bin.cols + rposx] != 255)
			{
				double _S = GetDistance(p_center, p2);
				
				// KBJ 2022-09-07 센터얼라인 조건 200 -> 150으로 변경
				if (radius - 200 < _S && radius + 200 > _S)
				{
					edgePositions.push_back(cv::Point2f(rposx, rposy));
					_bcircleEdgeFind = false;
					break;
				}
			}
		}
		if (_bcircleEdgeFind)  // Edge를 못찾는 경우 가상의 데이타 추가
		{
			edgePositions.push_back(cv::Point2f(-1, -1)); // 20211101 Tkyuha 원의 Edge를 못찾는 경우 추가
		}
	}

	// 20210909 Tkyuha 포인트 선택 영역 계산 8등분 해서 면적이 넓은 영역 6등분 선택
	// 메탈과 글라스 사이 검은 영역 면적 계산

	int _start = 0;
	int _end = int(edgePositions.size());
	bool _bsplit = false;

	if (AginJumpCount == 2)
	{
		if (edgePositions.size() > 10)
		{
			// 8방향에 대해 metal 엣지를 찾아서 panel hole과 metal hole 간의 거리 계산
			int _step = int(edgePositions.size() / 12);

			std::vector<pair<int, int>> _v;
			bool _bfind = false;

			for (int i = 0; i < edgePositions.size(); i += _step)
			{
				CPointF<int> _p(edgePositions[i].x, edgePositions[i].y);
				int _Srad = (int)GetDistance(p_center, _p);
				_bfind = false;

				for (int distanceT = 5; distanceT < 100; distanceT++) // 반지름 크기가 5픽셀 크기 부터 200 픽셀만 검사 
				{
					int _rad = _Srad + distanceT;  //반지름 계산

					rposx = (cos(ToRadian(i)) * _rad + colC);  // 회전 중심에 의한 좌표
					rposy = (sin(ToRadian(i)) * _rad + rowC);

					if (rposx < 0 || rposx >= colC * 2) continue;
					if (rposy < 0 || rposy >= rowC * 2) continue;
					CPointF<int> p2(rposx, rposy);
					if (Raw[rposy * bin.cols + rposx] == 255)
					{
						int _S = (int)GetDistance(_p, p2);
						_v.push_back(pair<int, int>(_S, i));
						_bfind = true;

						pDC->Ellipse(int(rposx + sx - 20), int(rposy + sy - 20), int(rposx + sx + 20), int(rposy + sy + 20));
						break;
					}
				}

				if (_bfind == false) _v.push_back(pair<int, int>(0, i));
			}

			sort(_v.begin(), _v.end());

			//Trace 예외 변경 영역
			if (_v.size() > 7)  // Tkyuha 범위 설정 수정 20220103
			{
				// KBJ 2022-09-10 메탈과 가까운 부분 위치 예외처리 추가

				int max_index = _v[_v.size() - 1].second;
				int _revID = (max_index + 180) % 360;

				_end = (_revID + 30) % 360;
				_start = (_revID - 30 + 360) % 360;

				if (_v[_v.size() - 2].second == (max_index + 30) % 360)		_end = (_revID + 60) % 360;
				else if (_v[_v.size() - 2].second == (max_index + 330) % 360)	_start = (_revID - 60 + 360) % 360;
				else
				{
					max_index = _v[_v.size() - 2].second;
					_revID = (max_index + 180) % 360;

					_end = (_revID + 30) % 360;
					_start = (_revID - 30 + 360) % 360;
					if (_v[_v.size() - 3].second == (max_index + 30) % 360)		_end = (_revID + 60) % 360;
					if (_v[_v.size() - 3].second == (max_index + 330) % 360)	_start = (_revID - 60 + 360) % 360;
				}

				if (abs(_end - _start) > 180)
				{
					_start = 0;
					_end = 45;
					_bsplit = true;
				}
			}
			else  _end = 0;

			_v.clear();
		}
	}
	///////// <>
	std::vector<cv::Point> pts;

	if (edgePositions.size() > 10)  // 10개 이상 포인트를 찾은 경우만 검사 진행
	{
		double cx = 0, cy = 0, rad = 0;
		int sCount = 0, margin = 5, _realCount = 0;
		double  avgdistance = 0;
		double* XiData = new double[edgePositions.size()];
		double* YiData = new double[edgePositions.size()];
		double* DistData = new double[edgePositions.size()];

		//outlier 제거
		//for (int i = 0; i < edgePositions.size() - 1; i++) // 마지막 하나는 빼고 수행 ,예외처리함
		for (int i = 0; i < edgePositions.size(); i++) // 마지막 하나는 빼고 수행 ,예외처리함
		{
			int next = 0;
			if (i == edgePositions.size() - 1)
			{
				next = 0;
			}
			else								next = i + 1;
			CPointF<int> p2(edgePositions[i].x, edgePositions[i].y);
			CPointF<int> p1(edgePositions[next].x, edgePositions[next].y);

			if ((p2.x == -1 && p2.y == -1) || p1.x == -1 && p1.y == -1) continue;

			XiData[_realCount] = GetDistance(p1, p2);
			DistData[_realCount] = XiData[_realCount];
			_realCount++;
		}

		std::sort(XiData, XiData + _realCount);
		avgdistance = XiData[_realCount / 2];  // 중앙값을 선택

		int _continueC_spec = 5;
		int _continueC = 0;
		for (int i = 0; i < _realCount + _continueC_spec - 1; i++)
		{
			int j = i;
			if (i >= 360) j = i - 360;

			// KBJ 2022-09-07 메탈과 가까운 부분 무시하는거 추가
			 if (AginJumpCount == 2)
			 {
				 if ((_start<i && _end>i) || (_bsplit && i > 315)) continue;
			 }
			
			if (DistData[j] > (avgdistance - margin) && DistData[j] < (avgdistance + margin))
			{
				_continueC++;

				if (_continueC >= _continueC_spec)
				{
					XiData[sCount] = edgePositions[j].x;
					YiData[sCount] = edgePositions[j].y;
					sCount++;
				}
			}
			else
			{
				_continueC = 0;
			}
		}

		if (sCount > 10)
		{
			for (int i = 0; i < sCount; i++) {
				cv::Point pt = cv::Point(XiData[i], YiData[i]);
				pts.push_back(pt);
			}
			errorloss = MLSAC_CircleFitting(pts, cx, cy, rad);

			//CircleFit_LS(sCount, XiData, YiData, &cx, &cy, &rad);
		}

		// KBJ 2022-09-12 센터얼라인 3번까지 찾도록 추가.
		if (sCount < 360 - _continueC_spec +1 && AginJumpCount != 2)
		{
			colC = cx, rowC = cy;
			AginJumpCount ++;
			pts.clear(); // 메모리 Leak 제거
			delete XiData;
			delete YiData;
			delete DistData;

			goto AginCenterAlign;
		}

		if (pDC != NULL)
		{
			CPen penE(PS_SOLID, 3, RGB(255, 0, 0));
			CPen penY(PS_SOLID, 3, RGB(0, 255, 0));

			int r = 4;
			for (int i = 0; i < edgePositions.size(); i++)
			{
				x = edgePositions[i].x + sx;
				y = edgePositions[i].y + sy;
				pDC->SetColor(RGB(255, 0, 0));
				//pDC->SelectObject(&penE); //제외 영역 표시
				pDC->Ellipse(int(x - r), int(y - r), int(x + r), int(y + r));
			}

			for (int i = 0; i < pts.size(); i++)
			{
				x = pts[i].x + sx;
				y = pts[i].y + sy;
				pDC->SetColor(RGB(0, 255, 0));
				//pDC->SelectObject(&penY); //제외 영역 표시
				pDC->Ellipse(int(x - r), int(y - r), int(x + r), int(y + r));
			}

			penE.DeleteObject();
			penY.DeleteObject();
		}

		circles.push_back(cv::Vec3f(cx, cy, rad));

		if (XiData) delete XiData;
		if (YiData) delete YiData;
		if (DistData) delete DistData;
	}

	// KBJ 2022-09-07 센터얼라인 결과 이미지용 벡터
	for (int i = 0; i < pts.size(); i++)
	{
		pEdgeposition->push_back(cv::Point2f(pts[i].x + sx, pts[i].y + sy));
	}

	edgePositions.clear();
	pts.clear();

	bin.release();

	return errorloss;
}

//#define _BEE_VISION
//KJH 2022-02-03 CircleAlign rename
double FindCircleAlign_MP(const cv::Mat& gray, int sx, int sy, double radius, int cirsize, std::vector<cv::Vec3f>& circles, bool bauto, int hth, bool bremoveNoise,FakeDC* pDC, bool CoverMode)
{
	/*
	GaussianBlur -> threshold(INV)) -> erode -> fnRemoveNoiseBlack  -> basic +0 ~ 500 search
	*/
	double x, y, errorloss=0.0;
	int colC = gray.cols / 2, rowC = gray.rows / 2;
	int rposy, rposx;
	bool _bcircleEdgeFind = false, _bFirstFind = false;
	cv::Mat bin;

	//delete
	//KJH 2022-01-25 MP Edge Noise제거 기능 삭제(오인식 다발)
	cv::GaussianBlur(gray, bin, cv::Size(7, 7), 3.);

	if (bauto)	cv::threshold(bin, bin, -1, 255, CV_THRESH_BINARY_INV | THRESH_OTSU);
	else		cv::threshold(bin, bin, hth, 255, CV_THRESH_BINARY_INV);

	// 잡음 제거
	if (bremoveNoise) // 파라미터로 변경
		erode(bin, bin, Mat::ones(cv::Size(3, 3), CV_8UC1), cv::Point(-1, -1), 2);
	// 검은색 구멍 뚤린 부분 채우는 부분
	fnRemoveNoiseBlack(bin, 800);

	uchar* Raw = bin.data;
	std::vector<cv::Point2f> edgePositions;
	CPointF<double> p_center(colC, rowC);

	int _removeEdgeCount = 0; //20220124 과검 제거용
	// Panel edge 찾는 부분
	for (int second = 0; second < 360; second++)
	{
		_bcircleEdgeFind = true;
		_bFirstFind = false;
		_removeEdgeCount = 0;
		// 원위의 점 찾기
		for (int distanceT = 0; distanceT < 500; distanceT++)
		{
			int _rad = cirsize + distanceT;
			rposx = (cos(ToRadian(second)) * _rad + colC);
			rposy = (sin(ToRadian(second)) * _rad + rowC);

			if (rposx < 0 || rposx >= colC * 2) continue;
			if (rposy < 0 || rposy >= rowC * 2) continue;

			CPointF<int> p2(rposx, rposy);

			//KJH 2022-01-25 ELB Cover CInk mode
			if (!CoverMode)
			{
				//Basic
				if (_bFirstFind && Raw[rposy * bin.cols + rposx] == 255)
				{
					double _S = GetDistance(p_center, p2);
					if (_removeEdgeCount > 3 && radius - 200 < _S && radius + 200 > _S)
					{
						edgePositions.push_back(cv::Point2f(rposx, rposy));
						_bcircleEdgeFind = false;
						break;
					}
					_removeEdgeCount++;
				}
				if (!_bFirstFind && Raw[rposy * bin.cols + rposx] != 255)
					_bFirstFind = true;
			}
			else
			{
				//Cover Mode
				if (Raw[rposy * bin.cols + rposx] != 255)
				{
					double _S = GetDistance(p_center, p2);
					if (radius - 200 < _S && radius + 200 > _S)
					{
						edgePositions.push_back(cv::Point2f(rposx, rposy));
						_bcircleEdgeFind = false;
						break;
					}
				}
			}
		}
		if (_bcircleEdgeFind)  // Edge를 못찾는 경우 가상의 데이타 추가
		{
			edgePositions.push_back(cv::Point2f(-1, -1)); // 20211101 Tkyuha 원의 Edge를 못찾는 경우 추가
		}
	}

	if (pDC != NULL)
	{
		//CPen penY(PS_SOLID, 3, RGB(0, 255, 0));
		//pDC->SelectObject(&penY);

		//int r = 4;
		auto pts = new GraphicPoints(0xff00ff00, 3, 2);
		for (int i = 0; i < edgePositions.size(); i++)
		{
			x = edgePositions[i].x + sx;
			y = edgePositions[i].y + sy;

			//pDC->Ellipse(int(x - r), int(y - r), int(x + r), int(y + r));
			pts->AddPoint(Gdiplus::PointF(x, y));
		}
		pDC->AddGraphic(pts);
		//penY.DeleteObject();
	}

	if (edgePositions.size() > 10)  // 10개 이상 포인트를 찾은 경우만 검사 진행
	{
		double cx = 0, cy = 0, rad = 0;

#ifdef _BEE_VISION
		double rr = radius;
		cv::Point2d ccm;

		cv::Mat m0, m1;

		m0 = cv::Mat(int(edgePositions.size()), 2, CV_32F, (float*)edgePositions.data());
		cv::approxPolyDP(m0, m1, 1, TRUE); // Reduce points for faster circle fit

		std::vector<cv::Point2f> dst;
		for (int i = 0; i < m1.rows; i++)
			dst.push_back({ m1.at<float>(i, 0), m1.at<float>(i, 1) });

		BestFitCircleX(dst, ccm, rr, 4, dst.size() * 0.35); // 35% number of points
		cx = ccm.x, cy = ccm.y, rad = rr;
		m0.release();
		m1.release();
		dst.clear();
#else
		int sCount = 0, margin = 5, _realCount = 0;
		double  avgdistance = 0;
		double* XiData = new double[edgePositions.size()];
		double* YiData = new double[edgePositions.size()];
		double* DistData = new double[edgePositions.size()];

		//outlier 제거
		for (int i = 0; i < edgePositions.size() - 1; i++) // 마지막 하나는 빼고 수행 ,예외처리함
		{
			CPointF<int> p2(edgePositions[i].x, edgePositions[i].y);
			CPointF<int> p1(edgePositions[i + 1].x, edgePositions[i + 1].y);

			if ((p2.x == -1 && p2.y == -1) || p1.x == -1 && p1.y == -1) continue;

			XiData[_realCount] = GetDistance(p1, p2);
			DistData[_realCount] = XiData[_realCount];
			_realCount++;
		}

		std::sort(XiData, XiData + _realCount);
		avgdistance = XiData[_realCount / 2];  // 중앙값을 선택

		CPen penE(PS_SOLID, 3, RGB(0, 0, 255));
		pDC->SelectObject(&penE);

		int _continueC = 0;
		auto ptx = new GraphicPoints(0xff0000ff, 3, 2);
		for (int i = 0; i < _realCount; i++)
		{
			if (DistData[i] > (avgdistance - margin) && DistData[i] < (avgdistance + margin))
			{
				_continueC++;

				if (_continueC >= 5)
				{
					XiData[sCount] = edgePositions[i].x;
					YiData[sCount] = edgePositions[i].y;
					x = edgePositions[i].x + sx;
					y = edgePositions[i].y + sy;
					//pDC->Ellipse(int(x - 4), int(y - 4), int(x + 4), int(y + 4));
					ptx->AddPoint(Gdiplus::PointF(x, y));
					sCount++;
				}
			}
			else _continueC = 0;
		}

		penE.DeleteObject();
		pDC->AddGraphic(ptx);

		if (sCount > 10)
		{
			std::vector<cv::Point> pts;
			for (int i = 0; i < sCount; i++) {
				cv::Point pt = cv::Point(XiData[i], YiData[i]);
				pts.push_back(pt);
			}
			errorloss = MLSAC_CircleFitting(pts, cx, cy, rad);
			pts.clear();
			//CircleFit_LS(sCount, XiData, YiData, &cx, &cy, &rad);
		}

		delete XiData;
		delete YiData;
		delete DistData;
#endif
		circles.push_back(cv::Vec3f(cx, cy, rad));
	}


	edgePositions.clear();

	return errorloss;
}

double FindCircleAlign_MP(const cv::Mat& gray, int sx, int sy, double radius, int cirsize, std::vector<cv::Vec3f>& circles, std::vector<cv::Point2f>* pEdgeposition, bool bauto, int hth,  bool bremoveNoise, FakeDC* pDC, bool CoverMode)
{
	pEdgeposition->clear();
	/*
	GaussianBlur -> threshold(INV)) -> erode -> fnRemoveNoiseBlack  -> basic +0 ~ 500 search
	*/
	double x, y, errorloss=0.0;
	int colC = gray.cols / 2, rowC = gray.rows / 2;
	int rposy, rposx;
	bool _bcircleEdgeFind = false, _bFirstFind = false;
	cv::Mat bin;

	//delete
	//KJH 2022-01-25 MP Edge Noise제거 기능 삭제(오인식 다발)
	cv::GaussianBlur(gray, bin, cv::Size(7, 7), 3.);

	if (bauto)	cv::threshold(bin, bin, -1, 255, CV_THRESH_BINARY_INV | THRESH_OTSU);
	else		cv::threshold(bin, bin, hth, 255, CV_THRESH_BINARY_INV);

	// 잡음 제거
	if (bremoveNoise) // 파라미터로 변경
		erode(bin, bin, Mat::ones(cv::Size(3, 3), CV_8UC1), cv::Point(-1, -1), 2);
	// 검은색 구멍 뚤린 부분 채우는 부분
	fnRemoveNoiseBlack(bin, 800);

	uchar* Raw = bin.data;
	std::vector<cv::Point2f> edgePositions;
	CPointF<double> p_center(colC, rowC);

	int _removeEdgeCount = 0; //20220124 과검 제거용
	// Panel edge 찾는 부분
	for (int second = 0; second < 360; second++)
	{
		_bcircleEdgeFind = true;
		_bFirstFind = false;
		_removeEdgeCount = 0;
		// 원위의 점 찾기
		for (int distanceT = 0; distanceT < 500; distanceT++)
		{
			int _rad = cirsize + distanceT;
			rposx = (cos(ToRadian(second)) * _rad + colC);
			rposy = (sin(ToRadian(second)) * _rad + rowC);

			if (rposx < 0 || rposx >= colC * 2) continue;
			if (rposy < 0 || rposy >= rowC * 2) continue;

			CPointF<int> p2(rposx, rposy);

			//KJH 2022-01-25 ELB Cover CInk mode
			if (!CoverMode)
			{
				//Basic
				if (_bFirstFind && Raw[rposy * bin.cols + rposx] == 255)
				{
					double _S = GetDistance(p_center, p2);
					if (_removeEdgeCount > 3 && radius - 200 < _S && radius + 200 > _S)
					{
						edgePositions.push_back(cv::Point2f(rposx, rposy));
						_bcircleEdgeFind = false;
						break;
					}
					_removeEdgeCount++;
				}
				if (!_bFirstFind && Raw[rposy * bin.cols + rposx] != 255)
					_bFirstFind = true;
			}
			else
			{
				//Cover Mode
				if (Raw[rposy * bin.cols + rposx] != 255)
				{
					double _S = GetDistance(p_center, p2);
					if (radius - 200 < _S && radius + 200 > _S)
					{
						edgePositions.push_back(cv::Point2f(rposx, rposy));
						_bcircleEdgeFind = false;
						break;
					}
				}
			}
		}
		if (_bcircleEdgeFind)  // Edge를 못찾는 경우 가상의 데이타 추가
		{
			edgePositions.push_back(cv::Point2f(-1, -1)); // 20211101 Tkyuha 원의 Edge를 못찾는 경우 추가
		}
	}

	if (pDC != NULL)
	{
		CPen penY(PS_SOLID, 3, RGB(255, 0, 0));
		pDC->SelectObject(&penY);

		int r = 4;
		for (int i = 0; i < edgePositions.size(); i++)
		{
			x = edgePositions[i].x + sx;
			y = edgePositions[i].y + sy;

			pDC->Ellipse(int(x - r), int(y - r), int(x + r), int(y + r));
		}

		penY.DeleteObject();
	}

	std::vector<cv::Point> pts;

	if (edgePositions.size() > 10)  // 10개 이상 포인트를 찾은 경우만 검사 진행
	{
		double cx = 0, cy = 0, rad = 0;

#ifdef _BEE_VISION
		double rr = radius;
		cv::Point2d ccm;

		cv::Mat m0, m1;

		m0 = cv::Mat(int(edgePositions.size()), 2, CV_32F, (float*)edgePositions.data());
		cv::approxPolyDP(m0, m1, 1, TRUE); // Reduce points for faster circle fit

		std::vector<cv::Point2f> dst;
		for (int i = 0; i < m1.rows; i++)
			dst.push_back({ m1.at<float>(i, 0), m1.at<float>(i, 1) });

		BestFitCircleX(dst, ccm, rr, 4, dst.size() * 0.35); // 35% number of points
		cx = ccm.x, cy = ccm.y, rad = rr;
		m0.release();
		m1.release();
		dst.clear();
#else
		int sCount = 0, margin = 5, _realCount = 0;
		double  avgdistance = 0;
		double* XiData = new double[edgePositions.size()];
		double* YiData = new double[edgePositions.size()];
		double* DistData = new double[edgePositions.size()];

		//outlier 제거
		for (int i = 0; i < edgePositions.size() - 1; i++) // 마지막 하나는 빼고 수행 ,예외처리함
		{
			CPointF<int> p2(edgePositions[i].x, edgePositions[i].y);
			CPointF<int> p1(edgePositions[i + 1].x, edgePositions[i + 1].y);

			if ((p2.x == -1 && p2.y == -1) || p1.x == -1 && p1.y == -1) continue;

			XiData[_realCount] = GetDistance(p1, p2);
			DistData[_realCount] = XiData[_realCount];
			_realCount++;
		}

		std::sort(XiData, XiData + _realCount);
		avgdistance = XiData[_realCount / 2];  // 중앙값을 선택

		CPen penE(PS_SOLID, 3, RGB(0, 255, 0));
		pDC->SelectObject(&penE);

		int _continueC = 0;
		for (int i = 0; i < _realCount; i++)
		{
			if (DistData[i] > (avgdistance - margin) && DistData[i] < (avgdistance + margin))
			{
				_continueC++;

				if (_continueC >= 5)
				{
					XiData[sCount] = edgePositions[i].x;
					YiData[sCount] = edgePositions[i].y;
					x = edgePositions[i].x + sx;
					y = edgePositions[i].y + sy;
					pDC->Ellipse(int(x - 4), int(y - 4), int(x + 4), int(y + 4));
					sCount++;
				}
			}
			else _continueC = 0;
		}

		penE.DeleteObject();

		if (sCount > 10)
		{
			for (int i = 0; i < sCount; i++) {
				cv::Point pt = cv::Point(XiData[i], YiData[i]);
				pts.push_back(pt);
			}
			errorloss = MLSAC_CircleFitting(pts, cx, cy, rad);
			pts.clear();
			//CircleFit_LS(sCount, XiData, YiData, &cx, &cy, &rad);
		}

		delete XiData;
		delete YiData;
		delete DistData;
#endif
		circles.push_back(cv::Vec3f(cx, cy, rad));
	}

	// KBJ 2022-09-07 센터얼라인 결과 이미지용 벡터
	for (int i = 0; i < pts.size(); i++)
	{
		pEdgeposition->push_back(cv::Point2f(pts[i].x + sx, pts[i].y + sy));
	}

	edgePositions.clear();
	pts.clear();

	return errorloss;
}


/// <summary>
/// Conductive Ink 도포를 위한 메탈과 Ink 거리 측정 알고리즘
/// </summary>

double FindMetalToInkDistance(const cv::Mat& gray, int sx, int sy, std::vector<cv::Vec3f>& circles, double& startA, double& endA, CDC* pDC)
{
	double _dInclination=0.0; // 결과 기울기 전송

	if (circles.size() <= 0) return _dInclination;

	double x, y;
	int rposx, rposy;
	int colC = cvRound(circles[0][0]), rowC = cvRound(circles[0][1]); // 메탈의 중심점
	cv::Mat bin;

	cv::GaussianBlur(gray, bin, cv::Size(3, 3), 3.);
	cv::threshold(bin, bin, -1, 255, CV_THRESH_BINARY | THRESH_OTSU);

	// 잡음 제거
	erode(bin, bin, Mat::ones(cv::Size(3, 3), CV_8UC1), cv::Point(-1, -1), 2);
	// 검은색 구멍 뚤린 부분 채우는 부분
	fnRemoveNoiseBlack(bin, 200);
	 
	int radius = cvRound(circles[0][2]); // 메탈 반지름
	bool _bcircleEdgeFind;
	uchar* Raw = bin.data;
	std::vector<double> edgeDistance;
	std::vector<cv::Point2f> edgePositions;
	CPointF<double> p_center(colC, rowC);

	// Panel edge 찾는 부분
	for (int angle = 0; angle < 360; angle++)
	{
		_bcircleEdgeFind = true;
		// 원위의 점 찾기
		for (int distanceT = 5; distanceT < 100; distanceT++) // 최대 300um 까지만 검사
		{
			int _rad = radius - distanceT;
			rposx = int(cos(ToRadian(angle)) * _rad + colC);
			rposy = int(sin(ToRadian(angle)) * _rad + rowC);

			if (rposx < 0 || rposx >= gray.cols) continue;
			if (rposy < 0 || rposy >= gray.rows) continue;

			if (Raw[rposy * bin.cols + rposx] == 0)
			{
				CPointF<int> p2(rposx, rposy);
				double _S = radius - GetDistance(p_center, p2);

				edgePositions.push_back(cv::Point2f(rposx, rposy));
				edgeDistance.push_back(_S);
				_bcircleEdgeFind = false;
				break;
			}
		}
		if (_bcircleEdgeFind)  // Edge를 못찾는 경우 가상의 데이타 추가
		{
			edgeDistance.push_back(-1.0); // 20211101 Tkyuha 원의 Edge를 못찾는 경우 추가
			edgePositions.push_back(cv::Point2f(-1, -1));
		}
	}

	if (pDC != NULL)
	{
		CPen penE(PS_SOLID, 3, RGB(255, 0, 0));
		CPen penY(PS_SOLID, 3, RGB(0, 255, 0));

		int r = 4;
		for (int i = 0; i < edgePositions.size(); i++)
		{
			x = edgePositions[i].x + sx;
			y = edgePositions[i].y + sy;
			pDC->SelectObject(&penY);

			pDC->Ellipse(int(x - r), int(y - r), int(x + r), int(y + r));
		}

		pDC->SelectObject(&penE);
		penE.DeleteObject();
		penY.DeleteObject();
	}

	if (edgePositions.size() > 10)  // 10개 이상 포인트를 찾은 경우만 검사 진행
	{
		double max = *max_element(edgeDistance.begin(), edgeDistance.end());
		int max_index = max_element(edgeDistance.begin(), edgeDistance.end()) - edgeDistance.begin();

		double min = *min_element(edgeDistance.begin(), edgeDistance.end());
		int min_index = min_element(edgeDistance.begin(), edgeDistance.end()) - edgeDistance.begin();
		int _revID;
		CPen penY(PS_SOLID, 3, RGB(255, 255, 0));
		CPen penE(PS_SOLID, 3, RGB(0, 255, 255));

		pDC->SelectObject(&penY);
// 최대 기준점으로 그리기
		if (max_index >= 0 && max_index < edgeDistance.size())
		{	
			_revID = (max_index + 180) % 360;

			if (_revID >= 0 && _revID < edgeDistance.size())
			{
				x = int(edgePositions[max_index].x + sx);
				y = int(edgePositions[max_index].y + sy);

				pDC->MoveTo(x, y);				

				x = int(edgePositions[_revID].x + sx);
				y = int(edgePositions[_revID].y + sy);
				pDC->LineTo(x, y);
			}
			// 편차를 이용해서 기울기를 계산
			double _MaxDist = edgeDistance[max_index] - edgeDistance[_revID]; //편차의 최대값에서 최소값 차이
			double _T = radius?asin(_MaxDist / (CV_PI * radius)):0;           // 기울기의 각도를 계산
			double _L = tan(_T)?_MaxDist / tan(_T):0;                         // 직선 거리로 도포양 계산

			_dInclination = _L?_MaxDist / _L:0;                                   // 편차의 기울기 계산
			startA = max_index;                                               // 도포 시작점
			endA = _revID;                                                    // 도포 중간 지점
		}
// 최소 기준점으로 그리기
		if (min_index >= 0 && min_index < edgeDistance.size())
		{
			_revID = (min_index + 180) % 360;

			if (_revID >= 0 && _revID < edgeDistance.size())
			{
				pDC->SelectObject(&penE);

				x = int(edgePositions[min_index].x + sx);
				y = int(edgePositions[min_index].y + sy);
				pDC->MoveTo(x, y);

				x = int(edgePositions[_revID].x + sx);
				y = int(edgePositions[_revID].y + sy);
				pDC->LineTo(x, y);
			}
		}
		penY.DeleteObject();
		penE.DeleteObject();
	}

	edgeDistance.clear();
	edgePositions.clear();

	bin.release();

	return _dInclination;
}

cv::Mat GetCirclemask(cv::Mat image, int x, int y, int radius, bool white)
{
	cv::Mat mask;
	if (!white)
	{
		mask = cv::Mat(image.size(), CV_8U, Scalar(255));
		cv::circle(mask, cv::Point(x, y), radius, { 0 }, CV_FILLED);
	}
	else
	{
		mask = cv::Mat(image.size(), CV_8U, Scalar(0));
		cv::circle(mask, cv::Point(x, y), radius, { 255 }, CV_FILLED);
	}
	return mask;
}

cv::Mat GetRectmask(cv::Mat image, cv::Rect R, bool white)
{
	cv::Mat mask;
	if (!white)
	{
		mask = cv::Mat(image.size(), CV_8U, Scalar(255));
		cv::rectangle(mask, R, { 0 }, CV_FILLED);
	}
	else
	{
		mask = cv::Mat(image.size(), CV_8U, Scalar(0));
		cv::rectangle(mask, R, { 255 }, CV_FILLED);
	}

	return mask;
}

void ThinSubiteration1(Mat& pSrc, Mat& pDst) {
	int rows = pSrc.rows;
	int cols = pSrc.cols;
	pSrc.copyTo(pDst);
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (pSrc.at<float>(i, j) == 1.0f) {
				/// get 8 neighbors
				/// calculate C(p)
				int neighbor0 = (int)pSrc.at<float>(i - 1, j - 1);
				int neighbor1 = (int)pSrc.at<float>(i - 1, j);
				int neighbor2 = (int)pSrc.at<float>(i - 1, j + 1);
				int neighbor3 = (int)pSrc.at<float>(i, j + 1);
				int neighbor4 = (int)pSrc.at<float>(i + 1, j + 1);
				int neighbor5 = (int)pSrc.at<float>(i + 1, j);
				int neighbor6 = (int)pSrc.at<float>(i + 1, j - 1);
				int neighbor7 = (int)pSrc.at<float>(i, j - 1);
				int C = int(~neighbor1 & (neighbor2 | neighbor3)) +
					int(~neighbor3 & (neighbor4 | neighbor5)) +
					int(~neighbor5 & (neighbor6 | neighbor7)) +
					int(~neighbor7 & (neighbor0 | neighbor1));
				if (C == 1) {
					/// calculate N
					int N1 = int(neighbor0 | neighbor1) +
						int(neighbor2 | neighbor3) +
						int(neighbor4 | neighbor5) +
						int(neighbor6 | neighbor7);
					int N2 = int(neighbor1 | neighbor2) +
						int(neighbor3 | neighbor4) +
						int(neighbor5 | neighbor6) +
						int(neighbor7 | neighbor0);
					int N = min(N1, N2);
					if ((N == 2) || (N == 3)) {
						/// calculate criteria 3
						int c3 = (neighbor1 | neighbor2 | ~neighbor4) & neighbor3;
						if (c3 == 0) {
							pDst.at<float>(i, j) = 0.0f;
						}
					}
				}
			}
		}
	}
}


void ThinSubiteration2(Mat& pSrc, Mat& pDst) {
	int rows = pSrc.rows;
	int cols = pSrc.cols;
	pSrc.copyTo(pDst);
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (pSrc.at<float>(i, j) == 1.0f) {
				/// get 8 neighbors
				/// calculate C(p)
				int neighbor0 = (int)pSrc.at<float>(i - 1, j - 1);
				int neighbor1 = (int)pSrc.at<float>(i - 1, j);
				int neighbor2 = (int)pSrc.at<float>(i - 1, j + 1);
				int neighbor3 = (int)pSrc.at<float>(i, j + 1);
				int neighbor4 = (int)pSrc.at<float>(i + 1, j + 1);
				int neighbor5 = (int)pSrc.at<float>(i + 1, j);
				int neighbor6 = (int)pSrc.at<float>(i + 1, j - 1);
				int neighbor7 = (int)pSrc.at<float>(i, j - 1);
				int C = int(~neighbor1 & (neighbor2 | neighbor3)) +
					int(~neighbor3 & (neighbor4 | neighbor5)) +
					int(~neighbor5 & (neighbor6 | neighbor7)) +
					int(~neighbor7 & (neighbor0 | neighbor1));
				if (C == 1) {
					/// calculate N
					int N1 = int(neighbor0 | neighbor1) +
						int(neighbor2 | neighbor3) +
						int(neighbor4 | neighbor5) +
						int(neighbor6 | neighbor7);
					int N2 = int(neighbor1 | neighbor2) +
						int(neighbor3 | neighbor4) +
						int(neighbor5 | neighbor6) +
						int(neighbor7 | neighbor0);
					int N = min(N1, N2);
					if ((N == 2) || (N == 3)) {
						int E = (neighbor5 | neighbor6 | ~neighbor0) & neighbor7;
						if (E == 0) {
							pDst.at<float>(i, j) = 0.0f;
						}
					}
				}
			}
		}
	}
}


void normalize_skeleton(Mat& inputarray, Mat& outputarray) //지문 검색 사용
{
	bool bDone = false;
	int rows = inputarray.rows;
	int cols = inputarray.cols;

	inputarray.convertTo(inputarray, CV_32FC1);
	inputarray.copyTo(outputarray);
	outputarray.convertTo(outputarray, CV_32FC1);

	Mat p_enlarged_src = Mat(rows + 2, cols + 2, CV_32FC1);
	for (int i = 0; i < (rows + 2); i++) {
		p_enlarged_src.at<float>(i, 0) = 0.0f;
		p_enlarged_src.at<float>(i, cols + 1) = 0.0f;
	}
	for (int j = 0; j < (cols + 2); j++) {
		p_enlarged_src.at<float>(0, j) = 0.0f;
		p_enlarged_src.at<float>(rows + 1, j) = 0.0f;
	}
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (inputarray.at<float>(i, j) >= 20.0f) {
				p_enlarged_src.at<float>(i + 1, j + 1) = 1.0f;
			}
			else
				p_enlarged_src.at<float>(i + 1, j + 1) = 0.0f;
		}
	}

	Mat p_thinMat1 = Mat::zeros(rows + 2, cols + 2, CV_32FC1);
	Mat p_thinMat2 = Mat::zeros(rows + 2, cols + 2, CV_32FC1);
	Mat p_cmp = Mat::zeros(rows + 2, cols + 2, CV_8UC1);

	while (bDone != true) {
		ThinSubiteration1(p_enlarged_src, p_thinMat1);
		ThinSubiteration2(p_thinMat1, p_thinMat2);
		compare(p_enlarged_src, p_thinMat2, p_cmp, CV_CMP_EQ);

		int num_non_zero = countNonZero(p_cmp);
		if (num_non_zero == (rows + 2) * (cols + 2)) {
			bDone = true;
		}

		p_thinMat2.copyTo(p_enlarged_src);
	}

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			outputarray.at<float>(i, j) = p_enlarged_src.at<float>(i + 1, j + 1);
		}
	}
}

void thinningIte(Mat& img, int pattern) {

	Mat del_marker = Mat::ones(img.size(), CV_8UC1);
	int x, y;

	for (y = 1; y < img.rows - 1; ++y) {

		for (x = 1; x < img.cols - 1; ++x) {

			int v9, v2, v3;
			int v8, v1, v4;
			int v7, v6, v5;

			v1 = img.data[y * img.step + x * img.elemSize()];
			v2 = img.data[(y - 1) * img.step + x * img.elemSize()];
			v3 = img.data[(y - 1) * img.step + (x + 1) * img.elemSize()];
			v4 = img.data[y * img.step + (x + 1) * img.elemSize()];
			v5 = img.data[(y + 1) * img.step + (x + 1) * img.elemSize()];
			v6 = img.data[(y + 1) * img.step + x * img.elemSize()];
			v7 = img.data[(y + 1) * img.step + (x - 1) * img.elemSize()];
			v8 = img.data[y * img.step + (x - 1) * img.elemSize()];
			v9 = img.data[(y - 1) * img.step + (x - 1) * img.elemSize()];

			int S = (v2 == 0 && v3 == 1) + (v3 == 0 && v4 == 1) +
				(v4 == 0 && v5 == 1) + (v5 == 0 && v6 == 1) +
				(v6 == 0 && v7 == 1) + (v7 == 0 && v8 == 1) +
				(v8 == 0 && v9 == 1) + (v9 == 0 && v2 == 1);

			int N = v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;

			int m1 = 0, m2 = 0;

			if (pattern == 0) m1 = (v2 * v4 * v6);
			if (pattern == 1) m1 = (v2 * v4 * v8);

			if (pattern == 0) m2 = (v4 * v6 * v8);
			if (pattern == 1) m2 = (v2 * v6 * v8);

			if (S == 1 && (N >= 2 && N <= 6) && m1 == 0 && m2 == 0)
				del_marker.data[y * del_marker.step + x * del_marker.elemSize()] = 0;
		}
	}

	img &= del_marker;
}

void Thinning(Mat& input, int row, int col)
{
	int x, y, p, q, xp, yp, xm, ym, cc, cd;
	int np1, sp1, hv;
	int cnt = 0, chk = 0, flag = 0;

	unsigned char* m_BufImage;
	unsigned char* m_ResultImg;
	m_BufImage = (unsigned char*)malloc(sizeof(unsigned char) * row * col);
	m_ResultImg = (unsigned char*)malloc(sizeof(unsigned char) * row * col);

	// Result image에 Mat format의 input image Copy
	for (y = 0; y < row; y++) {
		for (x = 0; x < col; x++) {
			*(m_ResultImg + (col * y) + x) = input.at<uchar>(y, x);
		}
	}

	do {
		// Image Buffer를 0으로 셋팅
		for (y = 1; y < row - 1; y++) {
			for (x = 1; x < col - 1; x++) {
				*(m_BufImage + (col * y) + x) = 0;
			}
		}

		// 천이 추출
		if (chk == 0) flag = 0;
		chk = cnt % 2;
		cnt++;

		for (y = 1; y < row - 1; y++) {
			ym = y - 1;
			yp = y + 1;
			for (x = 1; x < col - 1; x++) {
				if (*(m_ResultImg + (col * y) + x) == 0) continue;

				np1 = 0;
				for (q = y - 1; q <= y + 1; q++) {
					for (p = x - 1; p <= x + 1; p++) {
						if (*(m_ResultImg + (col * q) + p) != 0) np1++;
					}
				}

				if (np1 < 3 || np1 > 7) {
					*(m_BufImage + (col * y) + x) = 255;
					continue;
				}

				xm = x - 1;
				xp = x + 1;
				sp1 = 0;

				if (*(m_ResultImg + (col * ym) + x) == 0 && *(m_ResultImg + (col * ym) + xp) != 0) sp1++;
				if (*(m_ResultImg + (col * ym) + xp) == 0 && *(m_ResultImg + (col * y) + xp) != 0) sp1++;
				if (*(m_ResultImg + (col * y) + xp) == 0 && *(m_ResultImg + (col * yp) + xp) != 0) sp1++;
				if (*(m_ResultImg + (col * yp) + xp) == 0 && *(m_ResultImg + (col * yp) + x) != 0) sp1++;
				if (*(m_ResultImg + (col * yp) + x) == 0 && *(m_ResultImg + (col * yp) + xm) != 0) sp1++;
				if (*(m_ResultImg + (col * yp) + xm) == 0 && *(m_ResultImg + (col * y) + xm) != 0) sp1++;
				if (*(m_ResultImg + (col * y) + xm) == 0 && *(m_ResultImg + (col * ym) + xm) != 0) sp1++;
				if (*(m_ResultImg + (col * ym) + xm) == 0 && *(m_ResultImg + (col * ym) + x) != 0) sp1++;

				if (sp1 != 1) {
					*(m_BufImage + (col * y) + x) = 255;
					continue;
				}

				if (chk == 0) {
					cc = *(m_ResultImg + (col * ym) + x) * *(m_ResultImg + (col * y) + xp);
					cc = cc * *(m_ResultImg + (col * yp) + x);

					cd = *(m_ResultImg + (col * y) + xp) * *(m_ResultImg + (col * yp) + x);
					cd = cd * *(m_ResultImg + (col * y) + xm);
				}
				else {
					cc = *(m_ResultImg + (col * ym) + x) * *(m_ResultImg + (col * y) + xp);
					cc = cc * *(m_ResultImg + (col * y) + xm);

					cd = *(m_ResultImg + (col * ym) + x) * *(m_ResultImg + (col * yp) + x);
					cd = cd * *(m_ResultImg + (col * y) + xm);
				}

				if (cc != 0 || cd != 0) {
					*(m_BufImage + (col * y) + x) = 255;
					continue;
				}
				flag = 1;
			}
		}

		for (y = 1; y < row - 1; y++) {
			for (x = 1; x < col - 1; x++) {
				*(m_ResultImg + (col * y) + x) = *(m_BufImage + (col * y) + x);
			}
		}
	} while (!(chk == 1 && flag == 0));

	// 4연결점 처리
	for (y = 1; y < row - 1; y++) {
		yp = y + 1;
		ym = y - 1;
		for (x = 1; x < col - 1; x++) {
			if (*(m_ResultImg + (col * y) + x) == 0) continue;

			xm = x - 1;
			xp = x + 1;
			sp1 = 0;
			if (*(m_ResultImg + (col * ym) + x) == 0 && *(m_ResultImg + (col * ym) + xp) != 0) sp1++;
			if (*(m_ResultImg + (col * ym) + xp) == 0 && *(m_ResultImg + (col * y) + xp) != 0) sp1++;
			if (*(m_ResultImg + (col * y) + xp) == 0 && *(m_ResultImg + (col * yp) + xp) != 0) sp1++;
			if (*(m_ResultImg + (col * yp) + xp) == 0 && *(m_ResultImg + (col * yp) + x) != 0) sp1++;
			if (*(m_ResultImg + (col * yp) + x) == 0 && *(m_ResultImg + (col * yp) + xm) != 0) sp1++;
			if (*(m_ResultImg + (col * yp) + xm) == 0 && *(m_ResultImg + (col * y) + xm) != 0) sp1++;
			if (*(m_ResultImg + (col * y) + xm) == 0 && *(m_ResultImg + (col * ym) + xm) != 0) sp1++;
			if (*(m_ResultImg + (col * ym) + xm) == 0 && *(m_ResultImg + (col * ym) + x) != 0) sp1++;

			hv = 0;
			if (sp1 == 2) {
				if ((*(m_ResultImg + (col * ym) + x) & *(m_ResultImg + (col * y) + xp)) != 0) hv++;
				else if ((*(m_ResultImg + (col * y) + xp) & *(m_ResultImg + (col * yp) + x)) != 0) hv++;
				else if ((*(m_ResultImg + (col * yp) + x) & *(m_ResultImg + (col * y) + xm)) != 0) hv++;
				else if ((*(m_ResultImg + (col * y) + xm) & *(m_ResultImg + (col * ym) + x)) != 0) hv++;

				if (hv == 1) *(m_ResultImg + (col * y) + x) = 0;
			}
			else if (sp1 == 3) {
				if ((*(m_ResultImg + (col * ym) + x) & *(m_ResultImg + (col * y) + xm) & *(m_ResultImg + (col * y) + xp)) != 0) hv++;
				else if ((*(m_ResultImg + (col * yp) + x) & *(m_ResultImg + (col * y) + xm) & *(m_ResultImg + (col * y) + xp)) != 0) hv++;
				else if ((*(m_ResultImg + (col * ym) + x) & *(m_ResultImg + (col * yp) + x) & *(m_ResultImg + (col * y) + xm)) != 0) hv++;
				else if ((*(m_ResultImg + (col * ym) + x) & *(m_ResultImg + (col * yp) + x) & *(m_ResultImg + (col * y) + xp)) != 0) hv++;

				if (hv == 1) *(m_ResultImg + (col * y) + x) = 0;
			}
		}
	}

	// 들어온 배열에 재복사
	for (y = 0; y < row; y++) {
		for (x = 0; x < col; x++) {
			input.at<uchar>(y, x) = *(m_ResultImg + (col * y) + x);
		}
	}

	free(m_BufImage);
	free(m_ResultImg);
}


void thinning(const Mat& src, Mat& dst) {
	dst = src.clone();
	dst /= 255;

	Mat prev = Mat::zeros(dst.size(), CV_8UC1);
	Mat diff;

	do {
		thinningIte(dst, 0);
		thinningIte(dst, 1);
		absdiff(dst, prev, diff);
		dst.copyTo(prev);
	} while (countNonZero(diff) > 0);

	dst *= 255;
}

void thinImage(Mat& src, Mat& dst)
{
	int width = src.cols;
	int height = src.rows;
	src.copyTo(dst);
	vector<uchar*> mFlag; //Used to mark the point to be deleted    
	while (true)
	{
		//step one      
		for (int i = 0; i < height; ++i)
		{
			uchar* p = dst.ptr<uchar>(i);
			for (int j = 0; j < width; ++j)
			{
				//Get nine point objects, pay attention to the boundary problem
				uchar p1 = p[j];
				if (p1 != 1) continue;
				uchar p2 = (i == 0) ? 0 : *(p - dst.step + j);
				uchar p3 = (i == 0 || j == width - 1) ? 0 : *(p - dst.step + j + 1);
				uchar p4 = (j == width - 1) ? 0 : *(p + j + 1);
				uchar p5 = (i == height - 1 || j == width - 1) ? 0 : *(p + dst.step + j + 1);
				uchar p6 = (i == height - 1) ? 0 : *(p + dst.step + j);
				uchar p7 = (i == height - 1 || j == 0) ? 0 : *(p + dst.step + j - 1);
				uchar p8 = (j == 0) ? 0 : *(p + j - 1);
				uchar p9 = (i == 0 || j == 0) ? 0 : *(p - dst.step + j - 1);
				if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) >= 2 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) <= 6)//Condition 1 judgment
				{
					//Condition 2 calculation
					int ap = 0;
					if (p2 == 0 && p3 == 1) ++ap;
					if (p3 == 0 && p4 == 1) ++ap;
					if (p4 == 0 && p5 == 1) ++ap;
					if (p5 == 0 && p6 == 1) ++ap;
					if (p6 == 0 && p7 == 1) ++ap;
					if (p7 == 0 && p8 == 1) ++ap;
					if (p8 == 0 && p9 == 1) ++ap;
					if (p9 == 0 && p2 == 1) ++ap;
					//Condition 2, 3, 4 judgment
					if (ap == 1 && p2 * p4 * p6 == 0 && p4 * p6 * p8 == 0)
					{
						//Mark    
						mFlag.push_back(p + j);
					}
				}
			}
		}
		//Delete the marked point    
		for (vector<uchar*>::iterator i = mFlag.begin(); i != mFlag.end(); ++i)
		{
			**i = 0;
		}
		//Until no points are satisfied, the algorithm ends    
		if (mFlag.empty())
		{
			break;
		}
		else
		{
			mFlag.clear();//Empty mFlag    
		}

		//Step two, according to the situation, this step can be encapsulated with step one into a function
		for (int i = 0; i < height; ++i)
		{
			uchar* p = dst.ptr<uchar>(i);
			for (int j = 0; j < width; ++j)
			{
				//If four conditions are met, mark    
				//  p9 p2 p3    
				//  p8 p1 p4    
				//  p7 p6 p5    
				uchar p1 = p[j];
				if (p1 != 1) continue;
				uchar p2 = (i == 0) ? 0 : *(p - dst.step + j);
				uchar p3 = (i == 0 || j == width - 1) ? 0 : *(p - dst.step + j + 1);
				uchar p4 = (j == width - 1) ? 0 : *(p + j + 1);
				uchar p5 = (i == height - 1 || j == width - 1) ? 0 : *(p + dst.step + j + 1);
				uchar p6 = (i == height - 1) ? 0 : *(p + dst.step + j);
				uchar p7 = (i == height - 1 || j == 0) ? 0 : *(p + dst.step + j - 1);
				uchar p8 = (j == 0) ? 0 : *(p + j - 1);
				uchar p9 = (i == 0 || j == 0) ? 0 : *(p - dst.step + j - 1);
				if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) >= 2 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) <= 6)
				{
					int ap = 0;
					if (p2 == 0 && p3 == 1) ++ap;
					if (p3 == 0 && p4 == 1) ++ap;
					if (p4 == 0 && p5 == 1) ++ap;
					if (p5 == 0 && p6 == 1) ++ap;
					if (p6 == 0 && p7 == 1) ++ap;
					if (p7 == 0 && p8 == 1) ++ap;
					if (p8 == 0 && p9 == 1) ++ap;
					if (p9 == 0 && p2 == 1) ++ap;
					if (ap == 1 && p2 * p4 * p8 == 0 && p2 * p6 * p8 == 0)
					{
						//Mark    
						mFlag.push_back(p + j);
					}
				}
			}
		}
		//Delete the marked point    
		for (vector<uchar*>::iterator i = mFlag.begin(); i != mFlag.end(); ++i)
		{
			**i = 0;
		}
		//Until no points are satisfied, the algorithm ends    
		if (mFlag.empty())
		{
			break;
		}
		else
		{
			mFlag.clear();//Empty mFlag    
		}
	}
}
void endPointAndintersectionPointDetection(Mat& src)
{
	int width = src.cols;
	int height = src.rows;
	vector<CvPoint> endpoint;
	vector<CvPoint> intersectionPoint;
	//Traverse the skeletonized image, find the endpoints and intersections, and put them in the container
	for (int i = 0; i < height; ++i)
	{
		uchar* p = src.ptr<uchar>(i);
		for (int j = 0; j < width; ++j)
		{
			//Get nine point objects, pay attention to the boundary problem
			uchar p1 = p[j];
			if (p1 != 1) continue;
			uchar p2 = (i == 0) ? 0 : *(p - src.step + j);
			uchar p3 = (i == 0 || j == width - 1) ? 0 : *(p - src.step + j + 1);
			uchar p4 = (j == width - 1) ? 0 : *(p + j + 1);
			uchar p5 = (i == height - 1 || j == width - 1) ? 0 : *(p + src.step + j + 1);
			uchar p6 = (i == height - 1) ? 0 : *(p + src.step + j);
			uchar p7 = (i == height - 1 || j == 0) ? 0 : *(p + src.step + j - 1);
			uchar p8 = (j == 0) ? 0 : *(p + j - 1);
			uchar p9 = (i == 0 || j == 0) ? 0 : *(p - src.step + j - 1);

			if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) == 1)//Endpoint judgment
			{
				printf("Endpoint: %d %d\n", i, j);
				endpoint.push_back(cvPoint(j, i));
			}
			else //Intersection judgment
			{
				int ap = 0;
				if (p2 == 0 && p3 == 1) ++ap;
				if (p3 == 0 && p4 == 1) ++ap;
				if (p4 == 0 && p5 == 1) ++ap;
				if (p5 == 0 && p6 == 1) ++ap;
				if (p6 == 0 && p7 == 1) ++ap;
				if (p7 == 0 && p8 == 1) ++ap;
				if (p8 == 0 && p9 == 1) ++ap;
				if (p9 == 0 && p2 == 1) ++ap;
				if (ap >= 3)
				{
					printf("Intersection: %d %d\n", i, j);
					intersectionPoint.push_back(cvPoint(j, i));
				}
			}
		}
	}
	//Draw the endpoint
	for (vector<CvPoint>::iterator i = endpoint.begin(); i != endpoint.end(); ++i)
	{
		circle(src, cvPoint(i->x, i->y), 5, Scalar(255), -1);
	}
	//Draw the intersection
	for (vector<CvPoint>::iterator i = intersectionPoint.begin(); i != intersectionPoint.end(); ++i)
	{
		circle(src, cvPoint(i->x, i->y), 5, Scalar(255));
	}
	endpoint.clear();//Data recovery 
	intersectionPoint.clear();
}
void Peaks::findPeaks(vector<float> x0, vector<int>& peakInds)
{
	int minIdx = distance(x0.begin(), min_element(x0.begin(), x0.end()));
	int maxIdx = distance(x0.begin(), max_element(x0.begin(), x0.end()));

	float sel = (x0[maxIdx] - x0[minIdx]) / 4.0;

	int len0 = int(x0.size());

	vector<float> dx;
	diff(x0, dx);
	replace(dx.begin(), dx.end(), 0.0f, -Peaks::EPS);
	vector<float> dx0(dx.begin(), dx.end() - 1);
	vector<float> dx1(dx.begin() + 1, dx.end());
	vector<float> dx2;

	vectorProduct(dx0, dx1, dx2);

	vector<int> ind;
	findIndicesLessThan(dx2, 0, ind); // Find where the derivative changes sign

	vector<float> x;

	vector<int> indAux(ind.begin(), ind.end());
	selectElements(x0, indAux, x);
	x.insert(x.begin(), x0[0]);
	x.insert(x.end(), x0[x0.size() - 1]);;


	ind.insert(ind.begin(), 0);
	ind.insert(ind.end(), len0);

	int minMagIdx = distance(x.begin(), min_element(x.begin(), x.end()));
	float minMag = x[minMagIdx];
	float leftMin = minMag;
	int len = int(x.size());

	if (len > 2)
	{
		float tempMag = minMag;
		bool foundPeak = false;
		int ii;

		// Deal with first point a little differently since tacked it on
		// Calculate the sign of the derivative since we tacked the first
		//  point on it does not neccessarily alternate like the rest.
		vector<float> xSub0(x.begin(), x.begin() + 3);//tener cuidado subvector
		vector<float> xDiff;//tener cuidado subvector
		diff(xSub0, xDiff);

		vector<int> signDx;
		signVector(xDiff, signDx);

		if (signDx[0] <= 0) // The first point is larger or equal to the second
		{
			if (signDx[0] == signDx[1]) // Want alternating signs
			{
				x.erase(x.begin() + 1);
				ind.erase(ind.begin() + 1);
				len = len - 1;
			}
		}
		else // First point is smaller than the second
		{
			if (signDx[0] == signDx[1]) // Want alternating signs
			{
				x.erase(x.begin());
				ind.erase(ind.begin());
				len = len - 1;
			}
		}

		if (x[0] >= x[1])
			ii = 0;
		else
			ii = 1;

		float maxPeaks = ceil((float)len / 2.0);
		vector<int> peakLoc(maxPeaks, 0);
		vector<float> peakMag(maxPeaks, 0.0);
		int cInd = 1;
		int tempLoc;

		while (ii < len)
		{
			ii = ii + 1;//This is a peak
			//Reset peak finding if we had a peak and the next peak is bigger
			//than the last or the left min was small enough to reset.
			if (foundPeak)
			{
				tempMag = minMag;
				foundPeak = false;
			}

			//Found new peak that was lager than temp mag and selectivity larger
			//than the minimum to its left.

			if (x[ii - 1] > tempMag && x[ii - 1] > leftMin + sel)
			{
				tempLoc = ii - 1;
				tempMag = x[ii - 1];
			}

			//Make sure we don't iterate past the length of our vector
			if (ii == len)
				break; //We assign the last point differently out of the loop

			ii = ii + 1; // Move onto the valley

			//Come down at least sel from peak
			if (!foundPeak && tempMag > sel + x[ii - 1])
			{
				foundPeak = true; //We have found a peak
				leftMin = x[ii - 1];
				peakLoc[cInd - 1] = tempLoc; // Add peak to index
				peakMag[cInd - 1] = tempMag;
				cInd = cInd + 1;
			}
			else if (x[ii - 1] < leftMin) // New left minima
				leftMin = x[ii - 1];

		}

		// Check end point
		if (x[x.size() - 1] > tempMag && x[x.size() - 1] > leftMin + sel)
		{
			peakLoc[cInd - 1] = len - 1;
			peakMag[cInd - 1] = x[x.size() - 1];
			cInd = cInd + 1;
		}
		else if (!foundPeak && tempMag > minMag)// Check if we still need to add the last point
		{
			peakLoc[cInd - 1] = tempLoc;
			peakMag[cInd - 1] = tempMag;
			cInd = cInd + 1;
		}

		//Create output
		if (cInd > 0)
		{
			vector<int> peakLocTmp(peakLoc.begin(), peakLoc.begin() + cInd - 1);
			selectElements(ind, peakLocTmp, peakInds);
			//peakMags = vector<float>(peakLoc.begin(), peakLoc.begin()+cInd-1);
		}
	}
}

cv::Mat NMS_Algorithm(cv::Mat& mag, cv::Mat dir, std::vector<cv::Point>& edgePoints, float thresh)
{
	Mat outImage(mag.size(), CV_8U);
	int w = mag.size().width;
	int h = mag.size().height;
	int w1 = w - 1;
	int h1 = h - 1;
	float* _mag = mag.ptr<float>();
	float* _dir = dir.ptr<float>();
	unsigned char* nms = outImage.ptr<unsigned char>();
	float cur, nx = 0.0, prev = 0.0;
	int col, row, col1, row1, col2, row2;
	float val;
	int val16;

	for (row = 1; row < h1; row++)
	{
		for (col = 1; col < w1; col++)
		{
			val = _dir[row * w + col];
			val = val >= 0 ? val : 360 + val;

			val16 = int((val / 22.5) + 0.5) % 16;

			switch (val16)
			{
			case 0:
			case 7:
			case 8:
			case 15:
				col1 = col - 1; row1 = row;
				col2 = col - 1; row2 = row;
				break;

			case 1:
			case 2:
			case 9:
			case 10:
				col1 = col + 1; row1 = row;
				col2 = col; row2 = row - 1;
				break;

			case 3:
			case 4:
			case 11:
			case 12:
				col1 = col; row1 = row - 1;
				col2 = col; row2 = row + 1;
				break;

			case 5:
			case 6:
			case 13:
			case 14:
				col1 = col; row1 = row - 1;
				col2 = col - 1; row2 = row;
				break;
			}

			cur = _mag[row * w + col];
			nx = _mag[row1 * w + col1];
			prev = _mag[row2 * w + col2];

			if ((cur > nx) && (cur >= prev) && (cur > thresh))
			{
				edgePoints.push_back({ col, row });
				nms[row * w + col] = 0;// 255;
			}
			else nms[row * w + col] = 255;
		}
	}
	return outImage;
}

bool  MLSAC_LineFitting(std::vector<cv::Point> pts, double& a, double& b, double& c) //20220117
{
	using namespace RTL;

	int _nsize = int(pts.size());

	if (_nsize < 2) return false;

	double max_x = 0, max_y = 0;
	double sx = 0, sy = 0;	// 직선의 중심점
	std::vector<_Point> data;
	cv::Point2d _pt[2];

	for (int i = 0; i < _nsize; i++)
	{
		_Point pt = _Point(pts.at(i).x, pts.at(i).y);
		data.push_back(pt);
		sx += pt.x;
		sy += pt.y;
		if (max_x < pt.x)  max_x = pt.x;
		if (max_y < pt.y)  max_y = pt.y;
	}

	sx /= _nsize;
	sy /= _nsize;

	Line model;
	LineEstimator estimator;
	RTL::MLESAC<Line, _Point, std::vector<_Point> > mlesac(&estimator);
	
	double loss = mlesac.FindBest(model, data, _nsize, 2);
	
	if (model.b == 0) // 수직선
	{
		if (sy == pts.at(0).y) // 수평선
		{
			_pt[0].x = 0;
			_pt[0].y = sy;
			_pt[1].x = max_x;
			_pt[1].y = sy;
		}
		else
		{
			_pt[0].y = 0;
			_pt[0].x = -model.c / model.a;
			_pt[1].y = max_y;
			_pt[1].x = -model.c / model.a;
		}		
	}
	else
	{
		_pt[0].x = 0;
		_pt[1].x = max_x;
		_pt[0].y = -(model.a * _pt[0].x + model.c) / model.b;
		_pt[1].y = -(model.a * _pt[1].x + model.c) / model.b;
	}

	if (_pt[0].x == _pt[1].x)	// dx가 0인 경우 수직선으로 만들지 말고, 아주 근사한 수직선으로 바꾸자.
	{
		_pt[0].x += 0.000001;
		a = (_pt[1].y - _pt[0].y) / (_pt[1].x - _pt[0].x);
		b = _pt[0].y - (a * _pt[0].x);
	}
	else if (_pt[0].y == _pt[1].y)
	{
		_pt[0].y += 0.000001;
		a = (_pt[1].y - _pt[0].y) / (_pt[1].x - _pt[0].x);
		b = _pt[0].y - (a * _pt[0].x);
	}
	else
	{
		a = (_pt[1].y - _pt[0].y) / (_pt[1].x - _pt[0].x);
		b = _pt[0].y - (a * _pt[0].x);
	}

	c = model.c;

	data.clear();

	return true;
}

double  MLSAC_CircleFitting(std::vector<cv::Point> pts, double& cx, double& cy, double& rad) //20220117
{
	using namespace RTL;

	int _nsize = int(pts.size());

	std::vector<_Point> data;
	for (int i = 0; i < _nsize; i++)
	{
		_Point pt = _Point(pts.at(i).x, pts.at(i).y);
		data.push_back(pt);
	}

	Circle model;
	CircleEstimator estimator;
	RTL::MLESAC<Circle, _Point, std::vector<_Point> > mlesac(&estimator);

	double loss = mlesac.FindBest(model, data, int(data.size()), 4);
	loss = mlesac.GetResultGamma(); // Tkyuha 221108 에러율 
	if (loss == 0) loss = 1.0;

	cx = model.cx;
	cy = model.cy;
	rad = model.rad;

	data.clear();

	return loss;
}

uchar adaptiveProcess(const cv::Mat& im, int row, int col, int kernelSize, int maxSize)
{
	vector <uchar> pixels;
	for (int a = -kernelSize / 2; a <= kernelSize / 2; a++) {
		for (int b = -kernelSize / 2; b <= kernelSize / 2; b++) {
			pixels.push_back(im.at<uchar>(row + a, col + b));
		}
	}
	sort(pixels.begin(), pixels.end());
	auto min = pixels[0];
	auto max = pixels[kernelSize * kernelSize - 1];
	auto med = pixels[kernelSize * kernelSize / 2];
	auto zxy = im.at<uchar>(row, col);
	if (med > min && med < max) {
		if (zxy > min && zxy < max) {
			return zxy;
		}
		else {
			return med;
		}
	}
	else {
		kernelSize += 2;
		if (kernelSize <= maxSize)
			return adaptiveProcess(im, row, col, kernelSize, maxSize);
		else
			return med;
	}
}

cv::Mat Adaptive_Median_FilterWork(cv::Mat src) 
{
	cv::Mat dst;
	int minSize = 3;
	int maxSize = 7;
	cv::copyMakeBorder(src, dst, maxSize / 2, maxSize / 2, maxSize / 2, maxSize / 2, BORDER_REFLECT);
	int rows = dst.rows;
	int cols = dst.cols;

	for (int j = maxSize / 2; j < rows - maxSize / 2; j++) {
		for (int i = maxSize / 2; i < cols * dst.channels() - maxSize / 2; i++) {
			dst.at<uchar>(j, i) = adaptiveProcess(dst, j, i, minSize, maxSize);
		}
	}

	return dst;
}

void drawAxis(Mat& img, cv::Point p, cv::Point q, cv::Scalar colour, const float scale = 0.2)
{
	double angle = atan2((double)p.y - q.y, (double)p.x - q.x); // angle in radians
	double hypotenuse = sqrt((double)(p.y - q.y) * (p.y - q.y) + (p.x - q.x) * (p.x - q.x));

	q.x = (int)(p.x - scale * hypotenuse * cos(angle));
	q.y = (int)(p.y - scale * hypotenuse * sin(angle));
	line(img, p, q, colour, 1, CV_AA); // LINE_AA

	p.x = (int)(q.x + 9 * cos(angle + CV_PI / 4));
	p.y = (int)(q.y + 9 * sin(angle + CV_PI / 4));
	line(img, p, q, colour, 1, CV_AA);
	p.x = (int)(q.x + 9 * cos(angle - CV_PI / 4));
	p.y = (int)(q.y + 9 * sin(angle - CV_PI / 4));
	line(img, p, q, colour, 1, CV_AA);
}

double getOrientation(const vector<cv::Point> & pts, cv::Mat & img)
{
	int sz = static_cast<int>(pts.size());
	cv::Mat data_pts = Mat(sz, 2, CV_64F);
	for (int i = 0; i < data_pts.rows; i++)
	{
		data_pts.at<double>(i, 0) = pts[i].x;
		data_pts.at<double>(i, 1) = pts[i].y;
	}

	PCA pca_analysis(data_pts, cv::Mat(), CV_PCA_DATA_AS_ROW); // PCA::DATA_AS_ROW

	cv::Point cntr = cv::Point(static_cast<int>(pca_analysis.mean.at<double>(0, 0)),
		static_cast<int>(pca_analysis.mean.at<double>(0, 1)));

	vector<cv::Point2d> eigen_vecs(2);
	vector<double> eigen_val(2);
	for (int i = 0; i < 2; i++)
	{
		eigen_vecs[i] = cv::Point2d(pca_analysis.eigenvectors.at<double>(i, 0),
			pca_analysis.eigenvectors.at<double>(i, 1));
		eigen_val[i] = pca_analysis.eigenvalues.at<double>(i);
	}

	circle(img, cntr, 3, cv::Scalar(255, 0, 255), 2);
	cv::Point p1 = cntr + 0.02 * cv::Point(static_cast<int>(eigen_vecs[0].x * eigen_val[0]), static_cast<int>(eigen_vecs[0].y * eigen_val[0]));
	cv::Point p2 = cntr - 0.02 * cv::Point(static_cast<int>(eigen_vecs[1].x * eigen_val[1]), static_cast<int>(eigen_vecs[1].y * eigen_val[1]));
	drawAxis(img, cntr, p1, cv::Scalar(0, 255, 0), 1);
	drawAxis(img, cntr, p2, cv::Scalar(255, 255, 0), 5);
	double angle = atan2(eigen_vecs[0].y, eigen_vecs[0].x); // orientation in radians
	return angle;
}

//
//double COS(CPoint A, CPoint B) {             // directional cosine;
//	double norm2A = A.x * A.x + A.y * A.y;
//	double norm2B = B.x * B.x + B.y * B.y;
//	return (A.x * B.x + A.y * B.y) / sqrt(norm2A * norm2B);
//}
//int ccw(CPoint A, CPoint B, CPoint C) {     // cross(AB, BC); 반시계방향 > 0;
//	return (B.x - A.x) * (C.y - B.y) - (B.y - A.y) * (C.x - B.x);
//}
//// find antipodal points along y-direction;
//bool initCalipers(std::vector<CPoint>& Q,
//	int& minid, double& minCos,
//	int& maxid, double& maxCos) {
//	int N = Q.size();
//	// ccw check (중복 // 일직선을 고려);
//	int orient = 0;
//	for (int i = 0; i < N; i++) {
//		orient = ccw(Q[(i - 1 + N) % N], Q[i], Q[(i + 1) % N]);
//		if (orient != 0) break;
//	}
//	if (orient == 0) return false;
//
//	minid = maxid = 0;
//	for (int i = 1; i < N; i++) {
//		if (Q[i].y < Q[minid].y) minid = i;
//		if (Q[i].y > Q[maxid].y) maxid = i;
//	}
//
//	CPoint n0 = CPoint(1, 0);
//	if (orient < 0) n0 = -n0;  // convex hull이 CW인 경우;
//	CPoint dir0 = Q[(minid + 1) % N] - Q[minid];
//	CPoint dir1 = Q[(maxid + 1) % N] - Q[maxid];
//	minCos = COS(dir0, n0);
//	maxCos = COS(dir1, -n0);
//	return true;
//}
//int rotatingCalipers(std::vector<CPoint>& Q,
//	int& i0, double& dirCos0,
//	int& i1, double& dirCos1) {
//	int N = Q.size();
//	int i0next = (i0 + 1) % N;
//	int i1next = (i1 + 1) % N;
//	CPoint dir0 = Q[i0next] - Q[i0];
//	CPoint dir1 = Q[i1next] - Q[i1];
//	if (dirCos0 > dirCos1) {          // i0을 포함하는 평행선이 다음 edge와 이루는 각이 작음;
//		CPoint ndir0 = Q[(i0next + 1) % N] - Q[i0next]; // 각을 재는 기준선은 현재 다다음 edge;
//		dirCos0 = COS(ndir0, dir0);
//		dirCos1 = COS(dir1, -dir0);
//		i0 = i0next;                  // i0는 다음 꼭지점으로 이동; i1 = antipodal point;
//		return 0;
//	}
//	else {                          // i1을 포함하는 평행선이 다음 edge와 이루는 각이 작음
//		CPoint ndir1 = Q[(i1next + 1) % N] - Q[i1next];
//		dirCos1 = COS(ndir1, dir1);
//		dirCos0 = COS(dir0, -dir1);
//		i1 = i1next;                  // i1은 다음 꼭지점으로 이동; i0 = antipodal point;
//		return 1;
//	}
//}
//void runCalipers(std::vector<CPoint>& Q) {
//	int i0, i1;
//	double dirCos0, dirCos1;
//	bool done = false;
//	if (!initCalipers(Q, i0, dirCos0, i1, dirCos1)) return;  // fails
//	while (!done) {
//		int i0old = i0, i1old = i1;
//		if (rotatingCalipers(Q, i0, dirCos0, i1, dirCos1) == 0) {
//			// Q[i1] = antipodal point;
//			// do something useful...
//		}
//		else {
//			// Q[i0] = antipodal point;
//			// do something useful...
//		}
//	}
//};

//void smooth_ep_3x3(BYTE* src, int width, int height, BYTE* dst) {
//	const int xend = width - 2, yend = height - 2;
//	BYTE* line[3];
//	line[0] = src; line[1] = line[0] + width; line[2] = line[1] + width;
//	BYTE* dst_line = dst + width;        // starting dst row;
//	for (int y = 0; y < yend; ++y) {
//		for (int x = 0; x < xend; ++x) {
//			int diff1 = line[1][x] - line[1][x + 2];
//			if (diff1 < 0) diff1 = -diff1;
//			int diff2 = line[0][x] - line[2][x + 2];
//			if (diff2 < 0) diff2 = -diff2;
//			int diff3 = line[0][x + 1] - line[2][x + 1];
//			if (diff3 < 0) diff3 = -diff3;
//			int diff4 = line[0][x + 2] - line[2][x];
//			if (diff4 < 0) diff4 = -diff4;
//
//			if (diff1 <= diff2 && diff1 <= diff3 && diff1 <= diff4)         //0-도
//				dst_line[x + 1] = (line[1][x] + line[1][x + 1] + line[1][x + 2]) / 3;
//			else if (diff2 <= diff3 && diff2 <= diff4)                      //45-도
//				dst_line[x + 1] = (line[0][x] + line[1][x + 1] + line[2][x + 2]) / 3;
//			else if (diff3 <= diff4)                                        //90-도;
//				dst_line[x + 1] = (line[0][x + 1] + line[1][x + 1] + line[2][x + 1]) / 3;
//			else                                                            //135-도;
//				dst_line[x + 1] = (line[0][x + 2] + line[1][x + 1] + line[2][x]) / 3;
//
//			dst_line += width;                              //move to next dst line;
//		}
//		// increases src line ptr;
//		BYTE* tptr = line[2] + width;
//		line[0] = line[1]; line[1] = line[2]; line[2] = tptr;
//	}
//};