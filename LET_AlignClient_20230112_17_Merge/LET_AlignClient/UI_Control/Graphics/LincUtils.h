// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#include <Windows.h>
#include <WinUser.h>
#include <gdiplus.h>
#include <stdio.h>
#include <string>
#include "Rigid.h"
#include <opencv.hpp>
#include <opencv2/opencv.hpp>
#include "PolyPoints.h"

#define TH30 (30 * CV_PI / 180.)
#define DEF_BG_COLOR (0xd0303030)
#define DEF_FG_COLOR (0xffffff00)
#define RAD_TO_DEEGREE (180. / CV_PI)
#define DEEGREE_TO_RAD (CV_PI / 180.)
#define HITTEST_SIZE (12)
#define CV_05PI (CV_PI / 2.)
#define CV_2PI (CV_PI * 2.)

namespace cv
{
	typedef Rect_<float> Rect2f;
	typedef Rect_<double> Rect2d;
}

template<typename ... Args>
void BUG(const std::string& format, Args ... args)
{
	size_t size = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0' 
	if (size <= 0) { throw std::runtime_error("Error during formatting."); }
	char* buf = new char[size];
	snprintf(buf, size, format.c_str(), args ...);
	OutputDebugStringA(buf);
	delete[] buf;
}

template<typename ... Args>
CString fmt(const CStringA& format, Args ... args)
{
	size_t size = std::snprintf(nullptr, 0, format, args ...) + 1; // Extra space for '\0' 
	if (size <= 0) { throw std::runtime_error("Error during formatting."); }
	std::unique_ptr<char[]> buf(new char[size]);
	std::snprintf(buf.get(), size, format, args ...);
	return CString(buf.get(), int(size - 1));// CA2W(CStringA(buf.get(), size - 1)); // We don't want the '\0' inside 
}

template<typename ... Args>
CStringW fmtW(const CStringW& format, Args ... args)
{
	CStringW str;
	str.Format(format, args);
	return str;
}

template<typename ... Args>
std::string fmt_(const std::string format, Args ... args)
{
	size_t size = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0' 
	if (size <= 0) { throw std::runtime_error("Error during formatting."); }
	std::unique_ptr<char[]> buf(new char[size]);
	std::snprintf(buf.get(), size, format.c_str(), args ...);
	return std::string(buf.get(), size - 1);// CA2W(CStringA(buf.get(), size - 1)); // We don't want the '\0' inside 
}

static LPSTR g_HitTestCursors[] = {
	IDC_SIZENWSE, IDC_SIZENS, IDC_SIZENESW,
	IDC_SIZEWE,               IDC_SIZEWE,
	IDC_SIZENESW, IDC_SIZENS, IDC_SIZENWSE };

template<class T0>
Gdiplus::RectF GetRectGDI(cv::Point_<T0> cen, float w, float h)
{
	auto w2 = w / 2.;
	auto h2 = h / 2.;

	return Gdiplus::RectF(cen.x - w2, cen.y - h2, w, h);
}

template<class T0>
Gdiplus::RectF GetRectGDI(Gdiplus::PointF cen, cv::Size_<T0> size)
{
	auto w2 = size.width / 2.;
	auto h2 = size.height / 2.;

	return Gdiplus::RectF(cen.X - w2, cen.Y - h2, size.width, size.height);
}

template<class T0, class T1>
Gdiplus::RectF GetRectGDI(cv::Point_<T0> cen, cv::Size_<T1> size)
{
	auto w2 = size.width / 2.;
	auto h2 = size.height / 2.;

	return Gdiplus::RectF(cen.x - w2, cen.y - h2, size.width, size.height);
}

template<class T0>
cv::Rect_<T0> GetRect(cv::Point_<T0> cen, float w, float h)
{
	auto w2 = std::lround(w / 2.);
	auto h2 = std::lround(h / 2.);

	return cv::Rect_<T0>(cen.x - w2, cen.y - h2, w, h);
}

template<class T0, class T1>
cv::Rect_<T0> GetRect(cv::Point_<T0> cen, cv::Size_<T1> size)
{
	auto w2 = std::lround(size.width / 2.);
	auto h2 = std::lround(size.height / 2.);

	return cv::Rect_<T0>(cen.x - w2, cen.y - h2, size.width, size.height);
}

template<class T0, class T1>
bool IsPointInRect(cv::Rect_<T0>& rect, cv::Point_<T1> pt)
{
	return rect.contains(cv::Point_<T0>(pt));
}

template<class T0>
bool IsPointInRect(Gdiplus::RectF rect, cv::Point_<T0> pt)
{
	return rect.Contains(Gdiplus::PointF(pt.x, pt.y));
}

template<class T0>
bool IsPointInRect(cv::Rect_<T0>& rect, CPoint pt)
{
	return rect.contains(cv::Point_<T0>(pt.x, pt.y));
}

template<class T0>
bool IsPointInRect(CRect rect, cv::Point_<T0> pt)
{
	return rect.PtInRect(CPoint(lround(pt.x), lround(pt.y)));
}

template<class T0>
cv::Point2d GetCenter(const cv::Rect_<T0>& rect)
{
	return cv::Point2d(rect.x + (rect.width / 2.), rect.y + (rect.height / 2.));
}

/* Output RECT may bigger than input RECT */
template<class T0, class T1>
cv::Rect ContrainsRECT(cv::Rect_<T0> r, cv::Size_<T1> size, double scale = 1.)
{
	auto tl0 = r.tl() * scale;
	auto br0 = r.br() * scale;
	cv::Point tl1(std::fmax(0., std::floor(tl0.x)), std::fmax(0., std::floor(tl0.y)));
	cv::Point br1(std::fmin(size.width, std::ceil(br0.x)), std::fmin(size.height, std::ceil(br0.y)));

	return cv::Rect(tl1, cv::Size(br1.x - tl1.x, br1.y - tl1.y));
}

/* Output RECT may smaller than input RECT */
template<class T0, class T1>
cv::Rect ContrainsRECT2(cv::Rect_<T0> r, cv::Size_<T1> size, double scale = 1.)
{
	auto tl0 = r.tl() * scale;
	auto br0 = r.br() * scale;
	cv::Point tl1(std::fmax(0., std::ceil(tl0.x)), std::fmax(0., std::ceil(tl0.y)));
	cv::Point br1(std::fmin(size.width, std::floor(br0.x)), std::fmin(size.height, std::floor(br0.y)));

	return cv::Rect(tl1, cv::Size(br1.x - tl1.x, br1.y - tl1.y));
}

/* Output RECT is round RECT */
template<class T0, class T1>
cv::Rect ContrainsRECT3(cv::Rect_<T0> r, cv::Size_<T1> size, double scale = 1.)
{
	auto tl0 = r.tl() * scale;
	auto br0 = r.br() * scale;
	cv::Point tl1(std::fmax(0., std::round(tl0.x)), std::fmax(0., std::round(tl0.y)));
	cv::Point br1(std::fmin(size.width, std::round(br0.x)), std::fmin(size.height, std::round(br0.y)));

	return cv::Rect(tl1, cv::Size(br1.x - tl1.x, br1.y - tl1.y));
}

template<class T0>
void GetRotatedRectPoints(cv::Point_<T0> center, float w, float h, float th, Gdiplus::PointF& tl, Gdiplus::PointF& tr, Gdiplus::PointF& br, Gdiplus::PointF& bl)
{
	auto w2 = w / 2.;
	auto h2 = h / 2.;
	Rigid trans(center, th);

	tl = trans.Transform(Gdiplus::PointF(-w2, h2));
	tr = trans.Transform(Gdiplus::PointF(w2, h2));
	br = trans.Transform(Gdiplus::PointF(w2, -h2));
	bl = trans.Transform(Gdiplus::PointF(-w2, -h2));
}

template<class T0>
void GetRotatedRectPoints(cv::Point_<T0> center, float w, float h, float th, std::vector<Gdiplus::PointF>& pts)
{
	auto w2 = w / 2.;
	auto h2 = h / 2.;
	Rigid trans(center, th);

	auto tl = trans.Transform(Gdiplus::PointF(-w2, h2));
	auto tr = trans.Transform(Gdiplus::PointF(w2, h2));
	auto br = trans.Transform(Gdiplus::PointF(w2, -h2));
	auto bl = trans.Transform(Gdiplus::PointF(-w2, -h2));

	pts.clear();
	pts.push_back(tl);
	pts.push_back(tr);
	pts.push_back(br);
	pts.push_back(bl);
	pts.push_back(tl);
}

template<class T0>
void GetRotatedRectPoints(cv::Point_<T0> center, float w, float h, float th, std::array<Gdiplus::PointF, 5>& pts)
{
	auto w2 = w / 2.;
	auto h2 = h / 2.;
	Rigid trans(center, th);

	auto tl = trans.Transform(Gdiplus::PointF(-w2, h2));
	auto tr = trans.Transform(Gdiplus::PointF(w2, h2));
	auto br = trans.Transform(Gdiplus::PointF(w2, -h2));
	auto bl = trans.Transform(Gdiplus::PointF(-w2, -h2));

	pts = { tl, tr, br, bl, tl };
}

template<class T0>
std::array<Gdiplus::PointF, 4> GetRotatedRectPoints(cv::Point_<T0> center, float w, float h, float th)
{
	auto w2 = w / 2.;
	auto h2 = h / 2.;
	Rigid trans(center, th);

	return
	{
		trans.Transform(Gdiplus::PointF(-w2, h2)),
		trans.Transform(Gdiplus::PointF(w2, h2)),
		trans.Transform(Gdiplus::PointF(w2, -h2)),
		trans.Transform(Gdiplus::PointF(-w2, -h2))
	};
}

template<class T0>
PolyPoints GetRotatedRectPoints2(cv::Point_<T0> center, float w, float h, float th)
{
	auto w2 = w / 2.;
	auto h2 = h / 2.;
	Rigid trans(center, th);

	return
	{
		trans.Transform(Gdiplus::PointF(-w2, h2)),
		trans.Transform(Gdiplus::PointF(w2, h2)),
		trans.Transform(Gdiplus::PointF(w2, -h2)),
		trans.Transform(Gdiplus::PointF(-w2, -h2))
	};
}

template<class T0>
std::array<Gdiplus::PointF, 4> GetRotatedRectPoints_(cv::Point_<T0> center, float w, float h, float th)
{
	cv::Point2f pts[4];
	cv::RotatedRect r(center, { w, h }, th);
	r.points(pts);

	return {
		Gdiplus::PointF{pts[0].x, pts[0].y},
		Gdiplus::PointF{pts[1].x, pts[1].y},
		Gdiplus::PointF{pts[2].x, pts[2].y},
		Gdiplus::PointF{pts[3].x, pts[3].y}
	};
}

template<class T0>
double DistancePointPoint(cv::Point_<T0> st, Gdiplus::PointF ed)
{
	auto dx = st.x - ed.X;
	auto dy = st.y - ed.Y;
	return std::sqrt(dx * dx + dy * dy);
}

template<class T0>
double DistancePointPoint(Gdiplus::PointF st, cv::Point_<T0> ed)
{
	auto dx = st.X - ed.x;
	auto dy = st.Y - ed.y;
	return std::sqrt(dx * dx + dy * dy);
}

// using this for real o-x-y coordinate system
struct Point2Point
{
	double dx;
	double dy;
	double dist;
	double th;
	Point2Point(const CPoint& pt0, const CPoint& pt1)
	{
		dx = pt1.x - pt0.x;
		dy = pt1.y - pt0.y;
		dist = sqrt(dx * dx + dy * dy);
		th = atan2(dy, dx);
	}

	Point2Point(const Gdiplus::PointF& pt0, const Gdiplus::PointF& pt1)
	{
		dx = pt1.X - pt0.X;
		dy = pt1.Y - pt0.Y;
		dist = sqrt(dx * dx + dy * dy);
		th = atan2(dy, dx);
	}
	Point2Point(const Gdiplus::Point& pt0, const Gdiplus::Point& pt1)
	{
		dx = pt1.X - pt0.X;
		dy = pt1.Y - pt0.Y;
		dist = sqrt(dx * dx + dy * dy);
		th = atan2(dy, dx);
	}
	Point2Point(const Gdiplus::PointF& pt0, const Gdiplus::Point& pt1)
	{
		dx = pt1.X - pt0.X;
		dy = pt1.Y - pt0.Y;
		dist = sqrt(dx * dx + dy * dy);
		th = atan2(dy, dx);
	}
	Point2Point(const Gdiplus::Point& pt0, const Gdiplus::PointF& pt1)
	{
		dx = pt1.X - pt0.X;
		dy = pt1.Y - pt0.Y;
		dist = sqrt(dx * dx + dy * dy);
		th = atan2(dy, dx);
	}
	template<class T0, class T1>
	Point2Point(const cv::Point_<T0>& pt0, const cv::Point_<T1>& pt1)
	{
		dx = pt1.x - pt0.x;
		dy = pt1.y - pt0.y;
		dist = sqrt(dx * dx + dy * dy);
		th = atan2(dy, dx);
	}
};

// using this for pixel coordinate system but pretent it is o-x-y coordinate system
struct Point2Point2
{
	double dx;
	double dy;
	double dist;
	double th;
	Point2Point2(const CPoint& pt0, const CPoint& pt1)
	{
		dx = pt1.x - pt0.x;
		dy = pt1.y - pt0.y;
		dist = sqrt(dx * dx + dy * dy);
		th = atan2(-dy, dx);
	}

	Point2Point2(const Gdiplus::PointF& pt0, const Gdiplus::PointF& pt1)
	{
		dx = pt1.X - pt0.X;
		dy = pt1.Y - pt0.Y;
		dist = sqrt(dx * dx + dy * dy);
		th = atan2(-dy, dx);
	}
	Point2Point2(const Gdiplus::Point& pt0, const Gdiplus::Point& pt1)
	{
		dx = pt1.X - pt0.X;
		dy = pt1.Y - pt0.Y;
		dist = sqrt(dx * dx + dy * dy);
		th = atan2(-dy, dx);
	}
	Point2Point2(const Gdiplus::PointF& pt0, const Gdiplus::Point& pt1)
	{
		dx = pt1.X - pt0.X;
		dy = pt1.Y - pt0.Y;
		dist = sqrt(dx * dx + dy * dy);
		th = atan2(-dy, dx);
	}
	Point2Point2(const Gdiplus::Point& pt0, const Gdiplus::PointF& pt1)
	{
		dx = pt1.X - pt0.X;
		dy = pt1.Y - pt0.Y;
		dist = sqrt(dx * dx + dy * dy);
		th = atan2(-dy, dx);
	}
	template<class T0, class T1>
	Point2Point2(const cv::Point_<T0>& pt0, const cv::Point_<T1>& pt1)
	{
		dx = pt1.x - pt0.x;
		dy = pt1.y - pt0.y;
		dist = sqrt(dx * dx + dy * dy);
		th = atan2(-dy, dx);
	}
};


cv::Point2d GetCenter(const Gdiplus::RectF& rect);
Gdiplus::Color ColorRefToColor(COLORREF& color);
std::vector<cv::Point2f> GetRotatedRectPoints(Gdiplus::PointF center, float w, float h, float th);

double DistancePointPoint(Gdiplus::PointF st, Gdiplus::PointF ed);
void PlotPoint(Gdiplus::Graphics& graph, Gdiplus::Pen* pen, Gdiplus::Point pt, int marg = 3);
void PlotPoint(Gdiplus::Graphics& graph, Gdiplus::Pen* pen, Gdiplus::PointF pt, int marg = 3);
void PlotPoint(Gdiplus::Graphics& graph, Gdiplus::Color color, Gdiplus::Point pt, int marg = 3);
void PlotPoint(Gdiplus::Graphics& graph, Gdiplus::Color color, Gdiplus::PointF pt, int marg = 3);
void DrawCrossGraphic(Gdiplus::Graphics& graph, Gdiplus::Pen* pen, Gdiplus::PointF pt, int marg = 8);
void GetArrowPoints(Gdiplus::PointF seedPoint, float len, float th, Gdiplus::PointF& pt1, Gdiplus::PointF& pt2);

#include "EventHandlers.h"
#endif //PCH_H
