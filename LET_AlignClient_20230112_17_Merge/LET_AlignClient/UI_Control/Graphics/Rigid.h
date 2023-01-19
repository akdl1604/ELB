#pragma once
#include "stdafx.h"
#include <iostream>
#include <opencv.hpp>
#include "PolyPoints.h"
#include <vector>
#include <gdiplus.h>

class CRect;

template<class T>
struct Rigid_
{
public:

	Rigid_(const cv::Mat& src): Rigid_(1, 0, 0, 0, 1, 0)
	{
		if (!src.empty())
		{
			if ((src.rows * src.cols) > 5)
			{
				if (src.depth() == CV_32F)
				{
					const auto* data = (float*)src.data;
					for (int i = 0; i < 6; i++)
						xform[i] = data[i];
				}
				else if (src.depth() == CV_64F)
				{
					const auto* data = (double*)src.data;
					for (int i = 0; i < 6; i++)
						xform[i] = data[i];
				}
			}
		}
	}

	Rigid_(const T a = 1, const T b = 0, const T c = 0, const T d = 0, const T e = 1, const T f = 0)
	{
		xform[0] = a; xform[1] = b; xform[2] = c;
		xform[3] = d; xform[4] = e; xform[5] = f;
	}

	template<class T1>
	Rigid_(const Rigid_<T1>& rhs)
	{
		xform[0] = rhs[0]; xform[1] = rhs[1]; xform[2] = rhs[2];
		xform[3] = rhs[3]; xform[4] = rhs[4]; xform[5] = rhs[5];
	}

	template<class T1>
	Rigid_(const cv::Point_<T1>& pt, float th)
	{
		auto th_ = th * CV_PI / 180.;
		auto sin_ = sin(th_);
		auto cos_ = cos(th_);

		xform[0] = cos_; xform[1] = -sin_; xform[2] = pt.x;
		xform[3] = sin_; xform[4] = +cos_; xform[5] = pt.y;
	}

	Rigid_(const Gdiplus::PointF& pt, float th)
	{
		auto th_ = th * CV_PI / 180.;
		auto sin_ = sin(th_);
		auto cos_ = cos(th_);

		xform[0] = cos_; xform[1] = -sin_; xform[2] = pt.X;
		xform[3] = sin_; xform[4] = +cos_; xform[5] = pt.Y;
	}

	Rigid_(const Gdiplus::Point& pt, float th)
	{
		auto th_ = th * CV_PI / 180.;
		auto sin_ = sin(th_);
		auto cos_ = cos(th_);

		xform[0] = cos_; xform[1] = -sin_; xform[2] = pt.X;
		xform[3] = sin_; xform[4] = +cos_; xform[5] = pt.Y;
	}

	double AngleInRadian() const {
		return atan2(xform[3], xform[0]);
	}

	double AngleInDegree() const
	{
		return AngleInRadian() * 180. / CV_PI;
	}

	template<class T1>
	operator cv::Point_<T1>() const
	{
		return cv::Point_<T1>(T1(xform[2]), T1(xform[5]));
	}

	template<class T1>
	cv::Point_<T1> Transform(const cv::Point_<T1>& p) const
	{
		return cv::Point_<T1>(
			T1(xform[0] * p.x + xform[1] * p.y + xform[2]),
			T1(xform[3] * p.x + xform[4] * p.y + xform[5])
			);
	}

	cv::Point2d Transform(const CPoint& p) const
	{
		return cv::Point2d(
			double(xform[0] * p.x + xform[1] * p.y + xform[2]),
			double(xform[3] * p.x + xform[4] * p.y + xform[5])
		);
	}

	template<class T0>
	T0 Transform(const CPoint& p) const
	{
		return T0(
			double(xform[0] * p.x + xform[1] * p.y + xform[2]),
			double(xform[3] * p.x + xform[4] * p.y + xform[5])
		);
	}

	Gdiplus::PointF Transform(const Gdiplus::PointF& p) const
	{
		return Gdiplus::PointF(
			float(xform[0] * p.X + xform[1] * p.Y + xform[2]),
			float(xform[3] * p.X + xform[4] * p.Y + xform[5])
		);
	}

	Gdiplus::PointF Transform(const Gdiplus::Point& pt) const
	{
		return Gdiplus::PointF(
			float(xform[0] * pt.X + xform[1] * pt.Y + xform[2]),
			float(xform[3] * pt.X + xform[4] * pt.Y + xform[5])
		);
	}

	std::vector<Gdiplus::PointF> Transform(const std::vector<Gdiplus::Point>& src) const
	{
		std::vector<Gdiplus::PointF> pts;

		for (const auto& pt: src)
			pts.push_back(Transform(pt));

		return pts;
	}

	std::vector<Gdiplus::PointF> Transform(const std::vector<Gdiplus::PointF>& src) const
	{
		std::vector<Gdiplus::PointF> pts;

		for (const auto& pt: src)
			pts.push_back(Transform(pt));

		return pts;
	}

	cv::Point2f Transform2f(const Gdiplus::PointF& p) const
	{
		return cv::Point2f(
			float(xform[0] * p.X + xform[1] * p.Y + xform[2]),
			float(xform[3] * p.X + xform[4] * p.Y + xform[5])
		);
	}

	template<class T1>
	cv::Rect_<T1> Transform(const cv::Rect_<T1>& r) const
	{
		return cv::Rect_<T1>(Transform(r.tl()), Transform(r.br()));
	}

	Gdiplus::RectF Transform(Gdiplus::RectF& r) const
	{
		auto tl = Transform(cv::Point2f(r.GetLeft(), r.GetTop()));
		auto br = Transform(cv::Point2f(r.GetRight(), r.GetBottom()));

		return Gdiplus::RectF(tl.x, tl.y, br.x - tl.x, br.y - tl.y);
	}

	std::vector<Gdiplus::PointF> Transform(const PolyPoints& r) const
	{
		return
		{
			Transform(r.tl),
			Transform(r.tr),
			Transform(r.bl),
			Transform(r.br)
		};
	}

	template<class T0, int n>
	std::array<T0, n> Transform(std::array<T0, n>& pts) const
	{
		std::array<T0, n> arr;
		for (int i = 0; i < n; i++)
			arr[i] = Transform(pts[i]);
		return arr;
	}

	template<class T1>
	Rigid_<T1> Compose(const Rigid_<T1>& rhs) const
	{
		return Rigid_<T1>(
			xform[0] * rhs[0] + xform[1] * rhs[3],
			xform[0] * rhs[1] + xform[1] * rhs[4],
			xform[0] * rhs[2] + xform[1] * rhs[5] + xform[2],
			xform[3] * rhs[0] + xform[4] * rhs[3],
			xform[3] * rhs[1] + xform[4] * rhs[4],
			xform[3] * rhs[2] + xform[4] * rhs[5] + xform[5]
			);
	}

	Rigid_ Inverse() const
	{
		double det = xform[0] * xform[4] - xform[3] * xform[1];
		if (det == 0) return Rigid_();

		return Rigid_(
			+xform[4] / det, -xform[1] / det, (xform[1] * xform[5] - xform[2] * xform[4]) / det,
			-xform[3] / det, +xform[0] / det, (xform[2] * xform[3] - xform[0] * xform[5]) / det
		);
	}

	template<class T1>
	Rigid_ Scale(const T1 scale)
	{
		auto scale_ = scale != 0 ? 1. / scale: 1;
		return Rigid_(
			xform[0] * scale_, xform[1] * scale_, xform[2] * scale_,
			xform[3] * scale_, xform[4] * scale_, xform[5] * scale_
		);
	}

	Rigid_ ScaleTo(double scale)
	{
		auto thisScale = Scale();
		auto scale_ = thisScale != 0 ? scale / thisScale: 1;

		return Rigid_(
			xform[0] * scale_, xform[1] * scale_, xform[2] * scale_,
			xform[3] * scale_, xform[4] * scale_, xform[5] * scale_
		);
	}

	double Scale() const {
		return std::sqrt(xform[0] * xform[4] + xform[1] * xform[3]);
	}

	template<class T1, class T2>
	Rigid_ Translate(const T1 tx, const T2 ty)
	{
		return Rigid_(
			xform[0], xform[1], xform[2] + tx,
			xform[3], xform[4], xform[5] + ty
		);
	}

	template<class T1>
	Rigid_ Translate(const cv::Point_<T1>& pt, bool add = true)
	{
		if (add)
			return Rigid_(
				xform[0], xform[1], xform[2] + pt.x,
				xform[3], xform[4], xform[5] + pt.y
			);

		return Rigid_(
			xform[0], xform[1], pt.x * Scale(),
			xform[3], xform[4], pt.y * Scale()
		);
	}

	cv::Point2d Translate() const {
		return cv::Point2d(xform[2], xform[5]);
	}

	double operator[](int idx) const
	{
		return double(xform[idx % 6]);
	}

	//T operator[](int idx) const
	//{
	//	return xform[idx % 6];
	//}

	T& operator[](int idx)
	{
		return xform[idx % 6];
	}

	template<class T1>
	friend std::ostream& operator<<(std::ostream& os, const Rigid_<T1>& rigid)
	{
		os << "[" << rigid[0] << ", " << rigid[1] << ", " << rigid[2] << std::endl;
		os << " " << rigid[3] << ", " << rigid[4] << ", " << rigid[5] << "]" << std::endl;
		return os;
	}

	std::string ToString() const
	{
		return cv::format("[%.5f, %.5f, %.5f\n %.5f, %.5f, %.5f]", xform[0], xform[1], xform[2], xform[3], xform[4], xform[5]);
	}

	operator cv::Mat() const
	{
		return (cv::Mat_<T>(2, 3) << xform[0], xform[1], xform[2], xform[3], xform[4], xform[5]);
	}

	operator Gdiplus::Matrix() const
	{
		return Gdiplus::Matrix(xform[0], xform[1], xform[3], xform[4], xform[2], xform[5]);
	}

	Gdiplus::Matrix* GetGdiMatrix() const
	{
		return new Gdiplus::Matrix(float(xform[0]), float(xform[1]), float(xform[3]), float(xform[4]), float(xform[2]), float(xform[5]));
	}

protected:
	T xform[6];
};

typedef Rigid_<double> Rigid;
//using Rigid = Rigid_<double>;
