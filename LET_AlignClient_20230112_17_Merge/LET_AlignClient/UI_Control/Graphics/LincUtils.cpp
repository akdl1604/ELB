#include "stdafx.h"

/*
* - Lincoln Lee
* - x.perfect.vn@gmail.com
* - 2022-08-08
*/

Gdiplus::Color ColorRefToColor(COLORREF& color)
{
	auto ptr = (BYTE*)&color;
	return Gdiplus::Color::MakeARGB(255, ptr[0], ptr[1], ptr[2]);
}

void DrawCrossGraphic(Gdiplus::Graphics& graph, Gdiplus::Pen* pen, Gdiplus::PointF pt, int marg)
{
	graph.DrawLine(pen, pt.X - marg, pt.Y, pt.X + marg, pt.Y);
	graph.DrawLine(pen, pt.X, pt.Y - marg, pt.X, pt.Y + marg);
}

void PlotPoint(Gdiplus::Graphics& graph, Gdiplus::Pen* pen, Gdiplus::PointF pt, int marg)
{
	graph.DrawLine(pen, pt.X - marg, pt.Y, pt.X + marg, pt.Y);
	graph.DrawLine(pen, pt.X, pt.Y - marg, pt.X, pt.Y + marg);
}

void PlotPoint(Gdiplus::Graphics& graph, Gdiplus::Pen* pen, Gdiplus::Point pt, int marg)
{
	graph.DrawLine(pen, pt.X - marg, pt.Y, pt.X + marg, pt.Y);
	graph.DrawLine(pen, pt.X, pt.Y - marg, pt.X, pt.Y + marg);
}

void PlotPoint(Gdiplus::Graphics& graph, Gdiplus::Color color, Gdiplus::PointF pt, int marg)
{
	Gdiplus::Pen pen(color);
	graph.DrawLine(&pen, pt.X - marg, pt.Y, pt.X + marg, pt.Y);
	graph.DrawLine(&pen, pt.X, pt.Y - marg, pt.X, pt.Y + marg);
}

void PlotPoint(Gdiplus::Graphics& graph, Gdiplus::Color color, Gdiplus::Point pt, int marg)
{
	Gdiplus::Pen pen(color);
	graph.DrawLine(&pen, pt.X - marg, pt.Y, pt.X + marg, pt.Y);
	graph.DrawLine(&pen, pt.X, pt.Y - marg, pt.X, pt.Y + marg);
}

void GetArrowPoints(Gdiplus::PointF seedPoint, float len, float th, Gdiplus::PointF& pt1, Gdiplus::PointF& pt2)
{
	pt1.X = cos(th + TH30) * len + seedPoint.X;
	pt1.Y = sin(th + TH30) * len + seedPoint.Y;

	pt2.X = cos(th - TH30) * len + seedPoint.X;
	pt2.Y = sin(th - TH30) * len + seedPoint.Y;
}

cv::Point2d GetCenter(const Gdiplus::RectF& rect)
{
	return cv::Point2d(rect.X + (rect.Width / 2.), rect.Y + (rect.Height / 2.));
}

std::vector<cv::Point2f> GetRotatedRectPoints(Gdiplus::PointF center, float w, float h, float th)
{
	auto w2 = w / 2.;
	auto h2 = h / 2.;
	Rigid trans(center, th);

	return {
		trans.Transform2f(Gdiplus::PointF(-w2, h2)),
		trans.Transform2f(Gdiplus::PointF(w2, h2)),
		trans.Transform2f(Gdiplus::PointF(w2, -h2)),
		trans.Transform2f(Gdiplus::PointF(-w2, -h2))
	};
}

double DistancePointPoint(Gdiplus::PointF st, Gdiplus::PointF ed)
{
	auto dx = st.X - ed.X;
	auto dy = st.Y - ed.Y;
	return std::sqrt(dx * dx + dy * dy);
}

//void ShowImage(CString strTitle, const cv::Mat& src)
//{
//	CString str;
//	bool existed = false;
//	for (int i = 0; i < g_ImageWindows.size(); i++)
//	{
//		str = g_ImageWindows[i]->GetTitle();
//		if (str == strTitle)
//		{
//			g_ImageWindows[i]->SetImage(src);
//			existed = true;
//			break;
//		}
//	}
//
//	if (!existed)
//	{
//		auto view = new ImageViewer(NULL, true);
//		view->Create(IDD_IMAGE_VIEWER);
//		view->SetTitle(strTitle);
//		view->SetImage(src);
//		view->OnClosing = [&](CDialog* sender)
//		{
//			int id = -1;
//			CString title, title1;
//			auto view = (ImageViewer*)sender;
//			title = view->GetTitle();
//
//			for (int i = 0; i < g_ImageWindows.size(); i++)
//			{
//				title1 = g_ImageWindows[i]->GetTitle();
//				if (title == title1)
//				{
//					id = i;
//					break;
//				}
//			}
//			if (id > -1)g_ImageWindows.erase(g_ImageWindows.begin() + id);
//		};
//		g_ImageWindows.push_back(view);
//		view->ShowWindow(SW_SHOW);
//	}
//}