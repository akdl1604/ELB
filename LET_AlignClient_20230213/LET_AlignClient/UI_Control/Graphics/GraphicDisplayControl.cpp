#include "stdafx.h"
#include "GraphicDisplayControl.h"
#include <opencv.hpp>
#include <opencv2/opencv.hpp>
#include "resource.h"
#include "GraphicBase.h"
#include "GraphicCircle.h"
#include "GraphicLabel.h"
#include "GraphicLine.h"
#include "GraphicArrow.h"
#include "GraphicRectangle.h"
#include "GraphicDimensional.h"
#include "GraphicRegionalSelection.h"
#include "GraphicInterativeCalipers.h"
#include "GraphicPoints.h"

/*
* - Lincoln Lee
* - x.perfect.vn@gmail.com
* - 2022-08-08
*/

IMPLEMENT_DYNAMIC(CViewerEx, CWnd)

BEGIN_MESSAGE_MAP(CViewerEx, CWnd)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_SHOWWINDOW()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_ERASEBKGND()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_MENU_FIT, OnFitImage)
	ON_COMMAND(ID_MENU_NAVIGATION, OnNavigation)
	ON_COMMAND_RANGE(ID_MENU_ZOOM_X0125, ID_MENU_ZOOM_X32, OnSetZoom)
	ON_COMMAND_RANGE(ID_MENU_GRABMOVE, ID_MENU_ROTATE_SELECT, OnMouseModes)
	ON_COMMAND(ID_MENU_HIDE_INTGRAPHIC, OnHideInteractGraphic)
	ON_COMMAND(ID_MENU_SAVE, OnSaveImage)
	ON_COMMAND(ID_MENU_LOAD, OnLoadImage)
END_MESSAGE_MAP()

void CViewerEx::OnGrabMoveMode()
{
	SetInteraction(false);
	CheckMenuItem(ID_MENU_GRABMOVE, true);
}

void CViewerEx::OnLoadImage()
{
	CString strFilter = _T("Image Files (*.jpg, *.bmp, *.png)|*.jpg;*.bmp;*.png||");
	CFileDialog FileDlg(TRUE, _T("BMP"), NULL, OFN_SHOWHELP, strFilter);

	if (FileDlg.DoModal() == IDOK)
		OnLoadImage(FileDlg.GetPathName());
}

void CViewerEx::OnSaveImage()
{
	if (m_SrcMAT.empty()) return;

	CString strFilter = _T("Image Files (*.jpg, *.bmp, *.png)|*.jpg;*.bmp;*.png||");
	CFileDialog FileDlg(FALSE, _T("png"), _T("Image"), OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_SHOWHELP, strFilter);

	if (FileDlg.DoModal() == IDOK)
	{
		CStringA str(FileDlg.GetPathName());
		cv::imwrite(str.GetString(), m_SrcMAT);
	}
}

void CViewerEx::OnHideInteractGraphic()
{
	if (NULL != m_InteractiveGraphic)
		m_InteractiveGraphic->SetActive(m_Interative);

	SoftRefresh();
}

void CViewerEx::OnFitImage()
{
	if (!m_SrcMAT.empty())
	{
		auto fx = double(m_Width) / m_ImageSize.width;
		auto fy = double(m_Height) / m_ImageSize.height;
		auto fxy = MAX(MIN(fx, fy), MIN_ZOOM);

		m_ImageTrans = Rigid(1, 0, -m_ImageSize.width / 2., 0, 1, -m_ImageSize.height / 2.);
		m_SceneTrans = Rigid(1, 0, -m_Width / 2., 0, 1, -m_Height / 2.).ScaleTo(1. / fxy);
		m_bDrawFitImage = true;

		CreateSizedCacheImage();

		SoftRefresh();
	}
}

void CViewerEx::OnNavigation()
{
	m_Navigation = !m_Navigation;
	CheckMenuItem(ID_MENU_NAVIGATION, m_Navigation);
	HardRefresh();
}

CViewerEx& CViewerEx::SetMetricTransform(const Rigid& metric)
{
	m_InteractItems.m_Dimensional.SetMetricTransform(metric);
	m_InteractItems.m_RoiSelector.SetMetricTransform(metric);
	m_InteractItems.m_RoiRotateSelector.SetMetricTransform(metric);
	m_InteractItems.m_InteractCaliper.SetMetricTransform(metric);
	m_MetricTrans = metric;

	SoftRefresh();
	return *this;
}

CViewerEx& CViewerEx::OnLoadImage(const CString& strPath)
{
	if (!strPath.IsEmpty())
		SetImage(cv::imread(CStringA(strPath).GetString(), cv::IMREAD_GRAYSCALE)); //IMREAD_ANYCOLOR

	return *this;
}

CViewerEx& CViewerEx::SetInteraction(bool val)
{
	m_Interative = val;
	if (NULL != m_InteractiveGraphic)
	{
		m_InteractiveGraphic->SetActive(val);
		CheckMenuItem(ID_MENU_GRABMOVE, !val);
	}
	else
		CheckMenuItem(ID_MENU_GRABMOVE, true);

	SoftRefresh();
	return *this;
}

void CViewerEx::OnSetZoom(UINT wParam)
{
	auto nMenuID = UINT(wParam);
	auto selected = std::labs(nMenuID - UINT(ID_MENU_ZOOM_X0125));
	auto scale = m_ScaleLevels[selected % m_ScaleLevels.size()];

	m_SceneTrans = m_SceneTrans.ScaleTo(1. / scale);
	CreateSizedCacheImage();

	m_bDrawFitImage = false;
	SoftRefresh();
}

void CViewerEx::OnMouseModes(UINT wParam)
{
	auto nMenuID = UINT(wParam);
	auto selected = std::labs(nMenuID - UINT(ID_MENU_GRABMOVE));

	m_MouseModes[selected % m_MouseModes.size()]();
	if (m_bMenu) CheckMenuItem(nMenuID, true);

	SoftRefresh();
}

CViewerEx::CViewerEx()
{
	RegisterWindowClass();

	m_Palete = (Gdiplus::ColorPalette*) new ::byte[sizeof(Gdiplus::ColorPalette) + 255 * sizeof(Gdiplus::ARGB)];
	m_Palete->Count = 256;
	m_Palete->Flags = Gdiplus::PaletteFlags::PaletteFlagsGrayScale;
	for (int i = 0; i < int(m_Palete->Count); i++) m_Palete->Entries[i] = Gdiplus::Color::MakeARGB(255, i, i, i);

	m_PenLINE = new Gdiplus::Pen(0xff00ffff, 1);
	m_PenCROSS = new Gdiplus::Pen(0xff00ffff, 1);
	m_PenLINE->SetDashStyle(Gdiplus::DashStyleDash);
	m_PenLINE->SetDashPattern(m_DashStyle, 2);

	m_RedPen = new Gdiplus::Pen(Gdiplus::Color::Red);
	m_PaddingBrush = new Gdiplus::SolidBrush(Gdiplus::Color(0xff0a0a64)); //0xff0a0a64, 0xff4040ff
	att.SetColorMatrix(&m_SemiTransparentColorMATRIX);

	ClearSoftGraphics();
	ClearHardGraphics();

	m_InteractItems.m_InteractCaliper
		.SetCircleMode(false)
		.SetReverseMode(false);

	m_InteractItems.m_RoiSelector
		.SetShowCenter(true)
		.SetShowCross(true)
		.SetShowEllipse(false);

	m_InteractItems.m_RoiRotateSelector
		.SetShowCenter(true);

	SetMetricTransform(Rigid(0.010371, 0, 0, 0, -0.010359, 0));
	m_InteractiveGraphic = &m_InteractItems.m_RoiSelector;

	CreateMousePopupMenu();
	OnGrabMoveMode();
}

Gdiplus::Bitmap* CViewerEx::PaddingAndCreateBitmap(cv::Mat& src)
{
	int w = src.cols;
	Gdiplus::Bitmap* bm = NULL;
	if (src.channels() == 1) // Grayscale image
	{
		int remains = w % 4;
		if (remains > 0) cv::copyMakeBorder(src, src, 0, 0, 0, 4 - remains, cv::BORDER_CONSTANT);
		bm = new Gdiplus::Bitmap(w, src.rows, INT(src.step1()), PixelFormat8bppIndexed, src.data);
		bm->SetPalette(m_Palete);
	}
	else // Color Image
	{
		int remains = (w * 3) % 4;
		if (remains) cv::copyMakeBorder(src, src, 0, 0, 0, remains, cv::BORDER_CONSTANT);
		bm = new Gdiplus::Bitmap(w, src.rows, INT(src.step1()), PixelFormat24bppRGB, src.data);
	}
	return bm;
}

void CViewerEx::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	m_Width = cx;
	m_Height = cy;

	if (m_Width < 1 || m_Height < 1) return;

	auto scale = m_SceneTrans.Scale();
	m_SceneTrans = Rigid(1, 0, -m_Width / 2., 0, 1, -m_Height / 2.).ScaleTo(scale);

	SafeReleaseBitmap(&m_BaseIMG);
	SafeReleaseBitmap(&m_HardIMG);
	SafeReleaseBitmap(&m_SoftIMG);

	m_BaseMAT = cv::Mat::zeros(m_Height, m_Width, CV_8UC4);
	m_BaseIMG = new Gdiplus::Bitmap(m_Width, m_Height, INT(m_BaseMAT.step1()), PixelFormat32bppPARGB, m_BaseMAT.data);

	if (!m_SimpleMode)
	{
		m_HardIMG = new Gdiplus::Bitmap(m_Width, m_Height, PixelFormat32bppPARGB);
		m_SoftIMG = new Gdiplus::Bitmap(m_Width, m_Height, PixelFormat32bppPARGB);
	}

	m_HardDirty = true;
	m_SoftDirty = true;

	//UpdateWindow();
	Invalidate();
}

void CViewerEx::SafeReleaseBitmap(Gdiplus::Bitmap** bm)
{
	if (NULL != *bm) delete* bm;
	*bm = NULL;
}

void CViewerEx::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CWnd::OnShowWindow(bShow, nStatus);

	if (bShow && m_NeedFitImage)
	{
		auto fx = double(m_Width) / m_ImageSize.width;
		auto fy = double(m_Height) / m_ImageSize.height;
		auto fxy = MIN(MAX_ZOOM, MAX(MIN_ZOOM, MIN(fx, fy)));

		m_SceneTrans = Rigid(1, 0, -m_Width / 2., 0, 1, -m_Height / 2.).ScaleTo(1. / fxy);
		m_bDrawFitImage = true;
		m_NeedFitImage = false;
	}
}

void CViewerEx::OptimalGraphicSettings(Gdiplus::Graphics& graph, bool clear)
{
	graph.SetCompositingQuality(Gdiplus::CompositingQualityAssumeLinear);
	graph.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
	graph.SetCompositingMode(Gdiplus::CompositingMode::CompositingModeSourceOver); //CompositingModeSourceCopy, CompositingModeSourceOver
	if (clear) graph.Clear(0x00);
}

cv::Mat CViewerEx::Screenshot()
{
	cv::Mat scene;
	if (!m_BaseMAT.empty())
		cv::cvtColor(m_BaseMAT, scene, cv::COLOR_BGRA2BGR);

	return scene;
}

CViewerEx& CViewerEx::Screenshot(const CStringA& strFileName)
{
	//if (!m_BaseMAT.empty())
	//	cv::imwrite(strFileName.GetString(), m_BaseMAT); //basic method
	auto m = RenderToImage({ 1920, 1080 }); // 1920x1080=FullHD monitor's screen resolution
	cv::imwrite(strFileName.GetString(), m);
	return *this;
}

cv::Mat CViewerEx::RenderToImage(cv::Size maxSize)
{
	cv::Mat scene, sizedMAT;
	cv::Mat virtualSceneMAT = cv::Mat::zeros(maxSize, CV_8UC4);
	cv::Rect2d viewRECT({ 0, 0 }, maxSize);
	auto baseBM = new Gdiplus::Bitmap(maxSize.width, maxSize.height, INT(virtualSceneMAT.step1()), PixelFormat32bppPARGB, virtualSceneMAT.data);
	Gdiplus::Graphics baseGraph(baseBM);

	auto scale_ = m_SceneTrans.Scale();
	auto fx = double(maxSize.width) / m_ImageSize.width;
	auto fy = double(maxSize.height) / m_ImageSize.height;

	auto w_ = m_ImageSize.width / scale_;
	auto h_ = m_ImageSize.height / scale_;
	auto fxy = 1. / MIN(fx, fy);
	auto swap_ = (maxSize.width > w_) && (maxSize.height > h_);

	auto imgTrans = Rigid(1, 0, -m_ImageSize.width / 2., 0, 1, -m_ImageSize.height / 2.);

	auto sceneTrans = Rigid(1, 0, -maxSize.width / 2., 0, 1, -maxSize.height / 2.).ScaleTo(fxy);// swap_ ? fxy : m_SceneTrans.Scale());
	auto imgToView = sceneTrans.Inverse().Compose(imgTrans);
	auto viewToImg = imgTrans.Inverse().Compose(sceneTrans);
	auto viewInImgRECT = viewToImg.Transform(viewRECT);
	auto roi = ContrainsRECT(viewInImgRECT, m_ImageSize);
	auto imgInViewRECT_c = imgToView.Transform<float>(roi);
	auto paddingRECT = ContrainsRECT2(imgInViewRECT_c, maxSize);

	auto scale = imgToView.Scale();
	if (scale != 1) cv::resize(m_SrcMAT(roi).clone(), sizedMAT, cv::Size(), scale, scale, cv::INTER_NEAREST);
	else sizedMAT = m_SrcMAT(roi).clone();

	auto sizedIMG = PaddingAndCreateBitmap(sizedMAT);

	OptimalGraphicSettings(baseGraph, false);
	DrawPaddingBorders(baseGraph, paddingRECT, maxSize);

	// Fast copying base bitmap to screen, no need alpha blending
	baseGraph.SetCompositingMode(Gdiplus::CompositingMode::CompositingModeSourceCopy);
	baseGraph.DrawImage(sizedIMG, std::lround(imgInViewRECT_c.x), std::lround(imgInViewRECT_c.y));
	baseGraph.SetCompositingMode(Gdiplus::CompositingMode::CompositingModeSourceOver);

	RenderSoftGraphic(baseGraph, imgToView, maxSize);
	RenderHardGraphic(baseGraph, Rigid(), maxSize);

	cv::cvtColor(virtualSceneMAT, scene, cv::COLOR_BGRA2BGR);

	delete sizedIMG;
	delete baseBM;
	sizedMAT.release();
	virtualSceneMAT.release();

	return scene;
}

void CViewerEx::Render(const Rigid& imgToView)
{
	if (m_SoftDirty || m_HardDirty)
	{
		cv::Size maxSize(m_Width, m_Height);
		cv::Rect2d viewRECT({ 0, 0 }, maxSize);
		Gdiplus::Graphics baseGraph(m_BaseIMG);

		auto viewToImg = GetScreenToImageTransform();
		auto viewInImgRECT = viewToImg.Transform(viewRECT);
		auto roi = ContrainsRECT(viewInImgRECT, m_ImageSize);

		OptimalGraphicSettings(baseGraph, false);

		if (roi.width < 1 || roi.height < 1 || m_SrcMAT.empty())
		{
			baseGraph.Clear(m_BackColor);

			if (!m_SimpleMode)
			{
				RenderHardGraphic(Rigid(), maxSize);
				RenderSoftGraphic(imgToView, maxSize);
				baseGraph.DrawImage(m_SoftIMG, 0, 0);
				baseGraph.DrawImage(m_HardIMG, 0, 0);
			}
		}
		else
		{
			auto imgInViewRECT_c = imgToView.Transform<float>(roi);
			auto paddingRECT = ContrainsRECT2(imgInViewRECT_c, maxSize);

			if (m_ForceBaseImage || !m_SkipBaseImage)
			{
				auto scale = imgToView.Scale();

				if (scale != 1)
				{
					if (scale < 1)
					{
						if (scale != m_LastScale)
							CreateSizedCacheImage(true);

						auto roi1 = ContrainsRECT(roi, m_SrcSizedMAT.size(), scale);
						m_SizedMAT = m_SrcSizedMAT(roi1).clone();
					}
					else cv::resize(m_SrcMAT(roi).clone(), m_SizedMAT, cv::Size(), scale, scale, cv::INTER_NEAREST);
				}
				else m_SizedMAT = m_SrcMAT(roi).clone();

				SafeReleaseBitmap(&m_SizedIMG);
				m_SizedIMG = PaddingAndCreateBitmap(m_SizedMAT);
			}

			m_SkipBaseImage = false;
			m_ForceBaseImage = false;
			DrawPaddingBorders(baseGraph, paddingRECT, maxSize);

			// Fast copying base bitmap to screen, no need alpha blending
			baseGraph.SetCompositingMode(Gdiplus::CompositingMode::CompositingModeSourceCopy);
			baseGraph.DrawImage(m_SizedIMG, std::lround(imgInViewRECT_c.x), std::lround(imgInViewRECT_c.y));
			baseGraph.SetCompositingMode(Gdiplus::CompositingMode::CompositingModeSourceOver);

			if (!m_SimpleMode)
			{
				RenderHardGraphic(Rigid(), maxSize);
				RenderSoftGraphic(imgToView, maxSize);
				baseGraph.DrawImage(m_SoftIMG, 0, 0);
				baseGraph.DrawImage(m_HardIMG, 0, 0);
			}
		}
	}
}

bool CViewerEx::InitControl(CWnd* pWnd, bool bMenu)
{
	if (pWnd == NULL) return false;
	m_bMenu = bMenu;
	CreateMousePopupMenu();

	CRect rect;
	GetWindowRect(&rect);
	pWnd->ScreenToClient(&rect);

	UINT id = GetDlgCtrlID();
	if (DestroyWindow() == FALSE)
		return false;

	return Create(NULL, NULL, WS_CHILD | WS_VISIBLE | SS_CENTER | SS_NOTIFY, rect, pWnd, id);
}

void CViewerEx::OnPaint()
{
	m_Mutex.lock();
	if (!IsIconic())
	{
		CPaintDC dc(this);
		CMemDC memDC(dc, this);
		Gdiplus::Graphics graph(memDC.GetDC());

		Render(GetImageToScreenTransform());
		OptimalGraphicSettings(graph, false);

		// Fast copying base bitmap to screen, no need alpha blending
		graph.SetCompositingMode(Gdiplus::CompositingMode::CompositingModeSourceCopy);
		graph.DrawImage(m_BaseIMG, 0, 0);
	}
	m_Mutex.unlock();
}

void CViewerEx::CreateSizedCacheImage(bool force)
{
	if (!m_SrcMAT.empty())
	{
		auto scale = 1. / m_SceneTrans.Scale();

		if ((m_LastScale != scale) || force)
		{
			if (scale < 1) cv::resize(m_SrcMAT, m_SrcSizedMAT, cv::Size(), scale, scale, cv::INTER_NEAREST);
			m_LastScale = scale;
		}
	}
}

void CViewerEx::DoChangeImage()
{
	if (!m_SrcMAT.empty())
	{
		if (m_FirstTimeImage) // First Time set Image
		{
			auto fx_ = double(m_Width) / m_ImageSize.width;
			auto fy_ = double(m_Height) / m_ImageSize.height;
			auto scale = 1. / MAX(MIN(fx_, fy_), MIN_ZOOM);

			// Fit Image to view
			m_ImageTrans = Rigid(1, 0, -m_ImageSize.width / 2., 0, 1, -m_ImageSize.height / 2.);
			m_SceneTrans = Rigid(1, 0, -m_Width / 2., 0, 1, -m_Height / 2.).ScaleTo(scale);
			m_FirstTimeImage = false;
		}

		if (!m_SimpleMode)
		{
			float fx = NAV_MAXSIZE / m_ImageSize.width;
			float fy = NAV_MAXSIZE / m_ImageSize.height;
			m_NavScale = MIN(fx, fy);
			SafeReleaseBitmap(&m_NavigateIMG);
			cv::resize(m_SrcMAT, m_NavigateMAT, cv::Size(), m_NavScale, m_NavScale, cv::INTER_NEAREST);
			m_NavigateIMG = PaddingAndCreateBitmap(m_NavigateMAT);
		}

		CreateSizedCacheImage(true);
		m_ForceBaseImage = true;

		if (m_Loaded)
		{
			m_SoftDirty = true;
			m_HardDirty = true;
			Invalidate();
		}
	}
}

CViewerEx& CViewerEx::SetImage(const cv::Mat& src)
{
	m_Mutex.lock();
	if (!src.empty())
	{
		if (m_ImageSize != src.size() || m_SrcMAT.empty())
			m_SrcMAT = src.clone();
		else memcpy(m_SrcMAT.data, src.data, src.step1() * src.rows);
		m_BPP = src.channels() * 8;
		m_ImageSize = src.size();

		DoChangeImage();
	}
	m_Mutex.unlock();
	return *this;
}

CViewerEx& CViewerEx::SetImage(const void* data)
{
	m_Mutex.lock();
	memcpy(m_SrcMAT.data, data, m_SrcMAT.step1() * m_SrcMAT.rows);
	DoChangeImage();
	m_Mutex.unlock();
	//return SetImage(cv::Mat(m_ImageSize, m_SrcDepth, (void*)data));
	return *this;
}

CViewerEx& CViewerEx::SetImageSize(cv::Size size, int nBpp)
{
	if (!m_SrcMAT.empty()) m_SrcMAT.release();

	m_BPP = nBpp;
	m_SrcDepth = CV_MAKETYPE(CV_8U, int(nBpp / 8));
	m_ImageSize = size;
	m_SrcMAT = cv::Mat(size, m_SrcDepth, cv::Scalar(128));

	{
		auto fx_ = double(m_Width) / m_ImageSize.width;
		auto fy_ = double(m_Height) / m_ImageSize.height;
		auto scale = 1. / MAX(MIN(fx_, fy_), MIN_ZOOM);

		// Fit Image to view
		m_ImageTrans = Rigid(1, 0, -m_ImageSize.width / 2., 0, 1, -m_ImageSize.height / 2.);
		m_SceneTrans = Rigid(1, 0, -m_Width / 2., 0, 1, -m_Height / 2.).ScaleTo(scale);
	}

	m_SoftDirty = true;
	Invalidate();
	return *this;
}

CViewerEx& CViewerEx::SetImage(const void* data, cv::Size size, int nBpp)
{
	m_SrcDepth = CV_MAKETYPE(CV_8U, int(nBpp / 8));
	m_ImageSize = size;
	return SetImage(cv::Mat(m_ImageSize, m_SrcDepth, (BYTE*)data));
}

void CViewerEx::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();
	//m_Mutex.lock();
	//ScreenToClient(&point);
	m_IsHasMouseDown = true;
	m_LastMousePOS = point;

	auto imgToView = GetImageToScreenTransform();
	auto viewToImg = GetScreenToImageTransform();
	auto pt = viewToImg.Transform(point);

	if (!m_SimpleMode)
	{
		if (m_Navigation && m_NavigationRect.Contains(Gdiplus::PointF(point.x, point.y)))
		{
			cv::Point2f clickPT((point.x - m_NavigationRect.X) / m_NavScale, (point.y - m_NavigationRect.Y) / m_NavScale);

			m_SceneTrans = Rigid(1, 0, -m_Width / 2., 0, 1, -m_Height / 2.).ScaleTo(m_SceneTrans.Scale());
			m_ImageTrans = Rigid(1, 0, -clickPT.x, 0, 1, -clickPT.y);
			m_SoftDirty = true;
		}
		else if (m_Interative)
		{
			if (NULL != m_InteractiveGraphic)
			{
				if (m_InteractiveGraphic->IsInterative() && m_InteractiveGraphic->IsActive())
				{
					m_SoftDirty |= m_InteractiveGraphic->MouseDown(point, imgToView);

					NMHDR nmHdr;
					::ZeroMemory(&nmHdr, sizeof(NMHDR));

					nmHdr.hwndFrom = m_hWnd;
					nmHdr.idFrom = GetDlgCtrlID();
					nmHdr.code = WM_DRAWING_FINISH_MESSAGE;
					GetParent()->SendMessage(WM_NOTIFY, (LPARAM)NULL, (WPARAM)&nmHdr);
				}
			}
		}
	}

	if (m_MouseEvents.OnMouseDown) m_MouseEvents.OnMouseDown(this, CPoint(lround(pt.x), lround(pt.y)));
	//m_Mutex.unlock();
	if (m_SoftDirty) Invalidate();
	CWnd::OnLButtonDown(nFlags, point);
}

void CViewerEx::OnMouseMove(UINT nFlags, CPoint point)
{
	//m_Mutex.lock();
	auto dirty = false;
	auto imgToView = GetImageToScreenTransform();
	auto viewToImg = GetScreenToImageTransform();
	auto pt = viewToImg.Transform(point);

	if (m_Interative && !m_SimpleMode)
	{
		if (NULL != m_InteractiveGraphic)
		{
			if (m_IsHasMouseDown || m_InteractiveGraphic->IsSkipMouseDown())
			{
				if (m_InteractiveGraphic->IsInterative() && m_InteractiveGraphic->IsActive())
				{
					auto dx = point.x - m_LastMousePOS.x;
					auto dy = point.y - m_LastMousePOS.y;
					dirty = m_SkipBaseImage = m_InteractiveGraphic->MouseMove(point, imgToView);

					// Have mouse down + mouse not over InterativeGraphic -> Set new postition for InterativeGraphic
					if (!dirty && m_InteractiveGraphic->IsRePosition() && (dx != 0 && dy != 0))
					{
						//BUG("OnMouseMove -> RePosition m_LastMousePOS(x: %d, y: %d), point (x: %d, y: %d), dx: %d, dy: %d\n", m_LastMousePOS.x, m_LastMousePOS.y, point.x, point.y, point.x - m_LastMousePOS.x, point.y - m_LastMousePOS.y);
						m_InteractiveGraphic->RePosition(m_LastMousePOS, point, imgToView);
						dirty = m_SkipBaseImage = true;
					}
					if (dirty) m_LastMousePOS = point;
				}
			}
			else
			{
				if (m_InteractiveGraphic->IsInterative() && m_InteractiveGraphic->IsActive())
					m_InteractiveGraphic->MouseHover(point, imgToView);
			}
		}
	}
	else if (m_IsHasMouseDown && m_MoveByMouse) // Translate image by grab mouse
	{
		auto pt0 = viewToImg.Transform(m_LastMousePOS);
		auto pt1 = viewToImg.Transform(point);

		m_ImageTrans = m_ImageTrans.Translate(pt1 - pt0); 
		m_LastMousePOS = point;
		dirty = true;
	}

	m_ptMousePoint.x = floor(pt.x);
	m_ptMousePoint.y = floor(pt.y);

	if (m_BPP == 8)
		m_nY = GetValueY(m_ptMousePoint.x, m_ptMousePoint.y);
	else if (m_BPP == 24)
		m_clrRGB = GetValueRGB(m_ptMousePoint.x, m_ptMousePoint.y);

	if (m_hWnd) ::PostMessage(m_hWnd, WM_VIEW_CONTROL, MSG_FCV_SET_MOUSE_POS, MAKELPARAM(m_ptMousePoint.x, m_ptMousePoint.y));
	if (GetParent()) ::SendMessage(GetParent()->m_hWnd, WM_MOUSEMOVE, 0, 0);

	if (m_MouseEvents.OnMouseMove) m_MouseEvents.OnMouseMove(this, m_ptMousePoint);

	m_SoftDirty |= dirty;
	//m_Mutex.unlock();

	if (m_SoftDirty) Invalidate();

	CWnd::OnMouseMove(nFlags, point);
}

void CViewerEx::OnLButtonUp(UINT nFlags, CPoint point)
{
	//m_Mutex.lock();
	ReleaseCapture();
	m_IsHasMouseDown = false;
	auto imgToView = GetImageToScreenTransform();
	auto viewToImg = GetScreenToImageTransform();
	auto pt = viewToImg.Transform(point);

	if (m_Interative && !m_SimpleMode)
	{
		if (NULL != m_InteractiveGraphic)
		{
			if (m_InteractiveGraphic->IsInterative() && m_InteractiveGraphic->IsActive())
			{
				m_SoftDirty |= m_InteractiveGraphic->MouseUp(point, imgToView);
				NMHDR nmHdr;
				::ZeroMemory(&nmHdr, sizeof(NMHDR));

				nmHdr.hwndFrom = m_hWnd;
				nmHdr.idFrom = GetDlgCtrlID();
				nmHdr.code = WM_DRAWING_FINISH_MESSAGE;
				GetParent()->SendMessage(WM_NOTIFY, (LPARAM)NULL, (WPARAM)&nmHdr);
			}
		}
	}

	if (m_MouseEvents.OnMouseUp) m_MouseEvents.OnMouseUp(this, CPoint(lround(pt.x), lround(pt.y)));
	//m_Mutex.unlock();
	if (m_SoftDirty) Invalidate();

	CWnd::OnLButtonUp(nFlags, point);
}

// Mouse double click needed for MarkMask making
void CViewerEx::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	auto imgToView = GetImageToScreenTransform();
	auto pt = GetScreenToImageTransform().Transform(point);

	if (m_Interative && !m_SimpleMode)
	{
		if (NULL != m_InteractiveGraphic)
		{
			if (m_InteractiveGraphic->IsInterative() && m_InteractiveGraphic->IsActive())
				m_SoftDirty |= m_InteractiveGraphic->MouseDbClk(point, imgToView);
		}
	}

	if (m_MouseEvents.OnMouseDbClk) m_MouseEvents.OnMouseDbClk(this, CPoint(lround(pt.x), lround(pt.y)));
	if (m_SoftDirty) Invalidate();
	CWnd::OnLButtonDblClk(nFlags, point);
}

BOOL CViewerEx::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (m_ZoomByMouse)
	{
		m_Mutex.lock();
		cv::Point2f centerView(m_Width / 2., m_Height / 2.);
		double delta = zDelta > 0 ? zDelta / 100. : 100. / -zDelta;

		ScreenToClient(&pt); // Mouse position in screen coordinate
		auto imgPOS = GetScreenToImageTransform().Transform(pt);
		auto scale = 1. / MAX(MIN_ZOOM, MIN((delta / m_SceneTrans.Scale()), MAX_ZOOM));

		auto imgTrans = Rigid(1, 0, -imgPOS.x, 0, 1, -imgPOS.y);
		auto sceneTrans = Rigid(1, 0, -pt.x, 0, 1, -pt.y).ScaleTo(scale);
		auto xtrans = imgTrans.Inverse().Compose(sceneTrans);

		imgPOS = xtrans.Transform(centerView);
		m_ImageTrans = Rigid(1, 0, -imgPOS.x, 0, 1, -imgPOS.y);
		m_SceneTrans = Rigid(1, 0, -centerView.x, 0, 1, -centerView.y).ScaleTo(scale);

		CreateSizedCacheImage();

		m_ForceBaseImage = true;
		m_SoftDirty = true;
		m_Mutex.unlock();
		Invalidate();
	}

	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

BOOL CViewerEx::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;// TRUE, FALSE;
}

void CViewerEx::RegisterWindowClass()
{
	WNDCLASS wndcls;
	HINSTANCE hInst = AfxGetInstanceHandle();

	if (!(::GetClassInfo(hInst, VIEWER_WINCLASSNAME, &wndcls)))
	{
		wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = hInst;
		wndcls.hIcon = NULL;
		wndcls.hCursor = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
		wndcls.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = VIEWER_WINCLASSNAME;

		if (!AfxRegisterClass(&wndcls))
		{
			AfxThrowResourceException();
		}
	}
}

void CViewerEx::PreSubclassWindow()
{
	m_Loaded = true;
	ModifyStyle(0, SS_NOTIFY);

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	CRect r;
	GetWindowRect(&r);
	OnSize(WM_SHOWWINDOW, r.Width(), r.Height());

	CWnd::PreSubclassWindow();
}

CPoint CViewerEx::GetPointBegin(int idx) const
{
	CPoint st, ed;
	if (NULL != m_InteractiveGraphic)
		m_InteractiveGraphic->GetBeginEnd(st, ed);
	return st;
}

CPoint CViewerEx::GetPointEnd(int idx) const
{
	CPoint st, ed;
	if (NULL != m_InteractiveGraphic)
		m_InteractiveGraphic->GetBeginEnd(st, ed);
	return ed;
}

CViewerEx& CViewerEx::ClearOverlayDC()
{
	m_Mutex.lock();
	ClearGraphics();
	m_Mutex.unlock();
	return *this;
}

bool CViewerEx::IsFitImage() const
{
	return m_bDrawFitImage;
}

CViewerEx& CViewerEx::setOriginalImage()
{
	m_SceneTrans = Rigid(1, 0, -m_Width / 2., 0, 1, -m_Height / 2.);
	m_bDrawFitImage = false;
	SoftRefresh();
	return *this;
}

CViewerEx& CViewerEx::ShowClipers(bool show)
{
	m_InteractItems.m_InteractCaliper.SetActive(show);
	m_InteractiveGraphic = &m_InteractItems.m_InteractCaliper;
	m_Interative = show;
	return *this;
}

CViewerEx& CViewerEx::setNavi(bool nav)
{
	return SetNavMode(nav);
}

bool CViewerEx::GetModeDrawingFigure() const
{
	return (m_InteractiveGraphic == (&m_InteractItems.m_RoiSelector) && (m_InteractItems.m_RoiSelector.IsActive()));
}

bool CViewerEx::IsCalipersMode() const
{
	return (m_InteractiveGraphic == (&m_InteractItems.m_InteractCaliper) && (m_InteractItems.m_InteractCaliper.IsActive()));
}

bool CViewerEx::GetEnableDrawCrossLine() const
{
	return (m_ShowGuidingCross | m_ShowGuidingLine);
}

CViewerEx& CViewerEx::ShowOverlay(bool ena)
{
	m_SimpleMode = !ena;
	DirtyRefresh();
	return *this;
}

CViewerEx& CViewerEx::ToggleDrawOverlay()
{
	m_SimpleMode = !m_SimpleMode;
	DirtyRefresh();
	return *this;
}

bool CViewerEx::GetDrawOverlay() const
{
	return !m_SimpleMode;
}

CViewerEx& CViewerEx::SetDrawOverlay(bool draw)
{
	m_SimpleMode = !draw;
	DirtyRefresh();
	return *this;
}

void CViewerEx::ShowGuidingGraphic(Gdiplus::Graphics& graph, Rigid trans, cv::Size size, Gdiplus::Pen* penLINE, Gdiplus::Pen* penCROSS)
{
	ShowGuidingLine(graph, trans, size, penLINE);
	ShowGuidingCross(graph, trans, size, penCROSS);
}

void CViewerEx::ShowGuidingLine(Gdiplus::Graphics& graph, Rigid trans, cv::Size size, Gdiplus::Pen* pen)
{
	auto minXY = trans.Transform(cv::Point2f(0, 0));
	auto maxXY = trans.Transform(cv::Point2f(size.width, size.height));
	auto ptCenter = trans.Transform(cv::Point2f(size.width / 2., size.height / 2.));

	graph.DrawLine(pen, minXY.x, ptCenter.y, maxXY.x, ptCenter.y);
	graph.DrawLine(pen, ptCenter.x, minXY.y, ptCenter.x, maxXY.y);
}

void CViewerEx::ShowGuidingCross(Gdiplus::Graphics& graph, Rigid trans, cv::Size size, Gdiplus::Pen* pen)
{
	auto ptCenter = trans.Transform(cv::Point2f(size.width / 2., size.height / 2.));
	auto ptQuad = trans.Transform(cv::Point2f(size.width / 4., size.height / 4.));
	auto ptQuadX3 = trans.Transform(cv::Point2f(size.width / 4. * 3., size.height / 4. * 3.));

	DrawCrossGraphic(graph, pen, Gdiplus::PointF(ptCenter.x, ptCenter.y), 15);
	DrawCrossGraphic(graph, pen, Gdiplus::PointF(ptQuad.x, ptQuad.y), 15);
	DrawCrossGraphic(graph, pen, Gdiplus::PointF(ptQuadX3.x, ptQuad.y), 15);
	DrawCrossGraphic(graph, pen, Gdiplus::PointF(ptQuad.x, ptQuadX3.y), 15);
	DrawCrossGraphic(graph, pen, Gdiplus::PointF(ptQuadX3.x, ptQuadX3.y), 15);
}

CViewerEx& CViewerEx::AddHardGraphic(GraphicBase* item)
{
	m_Mutex.lock();
	m_HardGrahics.push_back(item);
	m_Mutex.unlock();	
	return *this;
}

CViewerEx& CViewerEx::AddSoftGraphic(GraphicBase* item)
{
	m_Mutex.lock();
	m_SoftGrahics.push_back(item);
	m_Mutex.unlock();	
	return *this;
}

CViewerEx& CViewerEx::ClearSoftGraphics()
{
	for (int i = 0; i < m_SoftGrahics.size(); i++)
		delete m_SoftGrahics[i];

	m_SoftGrahics.clear();
	SoftRefresh();
	return *this;
}

CViewerEx& CViewerEx::ClearHardGraphics()
{
	for (int i = 0; i < m_HardGrahics.size(); i++)
		delete m_HardGrahics[i];

	m_HardGrahics.clear();
	HardRefresh();
	return *this;
}

CViewerEx& CViewerEx::ClearGraphics()
{
	ClearHardGraphics();
	ClearSoftGraphics();

	return *this;
}

Gdiplus::Color CViewerEx::GetPixel(CPoint pt)
{
	int r = 0, g = 0, b = 0;
	int x = pt.x;
	int y = pt.y;

	if (!m_SrcMAT.empty())
	{
		auto w = m_SrcMAT.cols;
		auto h = m_SrcMAT.rows;
		auto cn = m_SrcMAT.channels();
		auto ptr = m_SrcMAT.data;
		auto idx = y * w + x;

		if (x >= 0 && x < w && y >= 0 && y < h)
		{
			if (cn == 1)
			{
				r = g = b = ptr[idx];
			}
			else if (cn == 3)
			{
				ptr += (idx * 3);
				b = ptr[0];
				g = ptr[1];
				r = ptr[2];
			}
		}
	}
	return Gdiplus::Color(r, g, b);
}

int CViewerEx::GetValueY(int x, int y) const
{
	int val = 0;
	if (!m_SrcMAT.empty())
	{
		auto w = m_SrcMAT.cols;
		auto h = m_SrcMAT.rows;
		auto ptr = m_SrcMAT.data;
		auto idx = y * w + x;

		if (x >= 0 && x < w && y >= 0 && y < h)
			val = ptr[idx];
	}
	return val;
}

COLORREF CViewerEx::GetValueRGB(int x, int y) const
{
	uchar r = 0, g = 0, b = 0;

	if (!m_SrcMAT.empty())
	{
		auto w = m_SrcMAT.cols;
		auto h = m_SrcMAT.rows;
		auto ptr = m_SrcMAT.data;
		auto idx = y * w + x;

		if (x >= 0 && x < w && y >= 0 && y < h)
		{
			ptr += (idx * 3);
			b = ptr[0];
			g = ptr[1];
			r = ptr[2];
		}
	}

	return RGB(r, g, b);
}

Gdiplus::Color CViewerEx::GetPixel(cv::Point2d pt)
{
	return GetPixel(CPoint(floor(pt.x), floor(pt.y)));
}

void CViewerEx::CheckMenuItem(int nID, bool bMode)
{
	if (NULL != m_pMenu)
	{
		if (nID == ID_MENU_NAVIGATION)
		{
			m_pMenu->CheckMenuItem(ID_MENU_NAVIGATION, bMode ? MF_CHECKED : MF_UNCHECKED);
		}
		else
		{
			for (int i = ID_MENU_GRABMOVE; i <= ID_MENU_ROTATE_SELECT; i++)
				m_pMenu->CheckMenuItem(i, ((i == nID) && bMode) ? MF_CHECKED : MF_UNCHECKED);
		}
	}
}

void CViewerEx::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if (m_bMenu)
		m_pMenu->TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTALIGN, point.x, point.y, this);
}

void CViewerEx::SafeRemoveMenu(CMenu** mnu)
{
	if (NULL != (*mnu))
	{
		(*mnu)->DestroyMenu();
		delete* mnu;
		*mnu = NULL;
	}
}

void CViewerEx::CreateMousePopupMenu()
{
	SafeRemoveMenu(&m_pSubZoom);
	SafeRemoveMenu(&m_pMenu);

	if (m_bMenu)
	{
		m_pMenu = new CMenu();
		m_pSubZoom = new CMenu();
		m_pMenu->CreatePopupMenu();
		m_pSubZoom->CreatePopupMenu();

		m_pSubZoom->AppendMenuA(MF_BYPOSITION, ID_MENU_ZOOM_X0125, _T("x0.125"));
		m_pSubZoom->AppendMenuA(MF_BYPOSITION, ID_MENU_ZOOM_X025, _T("x0.25"));
		m_pSubZoom->AppendMenuA(MF_BYPOSITION, ID_MENU_ZOOM_X05, _T("x0.5"));
		m_pSubZoom->AppendMenuA(MF_BYPOSITION, ID_MENU_ZOOM_X1, _T("x1"));
		m_pSubZoom->AppendMenuA(MF_BYPOSITION, ID_MENU_ZOOM_X2, _T("x2"));
		m_pSubZoom->AppendMenuA(MF_BYPOSITION, ID_MENU_ZOOM_X4, _T("x4"));
		m_pSubZoom->AppendMenuA(MF_BYPOSITION, ID_MENU_ZOOM_X8, _T("x8"));
		m_pSubZoom->AppendMenuA(MF_BYPOSITION, ID_MENU_ZOOM_X16, _T("x16"));
		m_pSubZoom->AppendMenuA(MF_BYPOSITION, ID_MENU_ZOOM_X32, _T("x32"));

		m_pMenu->AppendMenuA(MF_BYPOSITION, ID_MENU_FIT, _T("Fit"));
		m_pMenu->AppendMenuA(MF_POPUP, (UINT_PTR)m_pSubZoom->m_hMenu, _T("Zoom"));

		if (!m_SimpleMode)
		{
			m_pMenu->AppendMenuA(MF_BYPOSITION, ID_MENU_NAVIGATION, _T("Navigation"));
			m_pMenu->AppendMenuA(MF_SEPARATOR);
			m_pMenu->AppendMenuA(MF_BYPOSITION, ID_MENU_GRABMOVE, _T("Grab Move"));
			m_pMenu->AppendMenuA(MF_BYPOSITION, ID_MENU_MEASURE, _T("Measure"));
			m_pMenu->AppendMenuA(MF_BYPOSITION, ID_MENU_RECT_SELECT, _T("Rectangle Selection"));
			m_pMenu->AppendMenuA(MF_BYPOSITION, ID_MENU_ELLIPSE_SELECT, _T("Ellipse Selection"));
			m_pMenu->AppendMenuA(MF_BYPOSITION, ID_MENU_LINE_CALIPER, _T("Line Caliper"));
			m_pMenu->AppendMenuA(MF_BYPOSITION, ID_MENU_CIRCLE_CALIPER, _T("Circle Caliper"));
			m_pMenu->AppendMenuA(MF_BYPOSITION, ID_MENU_ROTATE_SELECT, _T("Rotate Rect Selection"));
			m_pMenu->AppendMenuA(MF_BYPOSITION, ID_MENU_HIDE_INTGRAPHIC, _T("Hide Interactive Graphics"));			
		}
		m_pMenu->AppendMenuA(MF_SEPARATOR);
		m_pMenu->AppendMenuA(MF_BYPOSITION, ID_MENU_LOAD, _T("Load..."));
		m_pMenu->AppendMenuA(MF_BYPOSITION, ID_MENU_SAVE, _T("Save..."));
		//m_pMenu->EnableMenuItem(ID_MENU_LOAD, MF_DISABLED);

		CheckMenuItem(ID_MENU_GRABMOVE, true);
	}
}

CViewerEx::~CViewerEx()
{
	m_Loaded = false;
	m_Closed = true;

	SafeReleaseBitmap(&m_BaseIMG);
	SafeReleaseBitmap(&m_SoftIMG);
	SafeReleaseBitmap(&m_HardIMG);
	SafeReleaseBitmap(&m_SizedIMG);
	SafeReleaseBitmap(&m_NavigateIMG);

	SafeRemoveMenu(&m_pMenu);
	SafeRemoveMenu(&m_pSubZoom);

	delete[] m_Palete;
	delete m_PenLINE;
	delete m_PenCROSS;
	delete m_PaddingBrush;

	delete m_FakeCDC;

	m_SrcMAT.release();
	m_BaseMAT.release();
	m_SizedMAT.release();
	m_SrcSizedMAT.release();
	m_NavigateMAT.release();

	ClearGraphics();
}

int CViewerEx::GetBPP() const
{
	return m_BPP;
}

int CViewerEx::GetValueY() const
{
	return m_nY;
}

COLORREF CViewerEx::GetValueRGB() const
{
	return m_clrRGB;
}

CPoint CViewerEx::GetMousePoint() const
{
	return m_ptMousePoint;
}

void* CViewerEx::GetImagePtr() const
{
	return m_SrcMAT.data;
}

CViewerEx& CViewerEx::reset_to_gray_image()
{
	memset(m_SrcMAT.data, 128, m_SrcMAT.step1() * m_SrcMAT.rows);
	DirtyRefresh();
	return *this;
};// { m_SrcMAT.release(); SoftRefresh(); return *this; }

CViewerEx& CViewerEx::clearAllFigures()
{
	return *this;
}

CViewerEx& CViewerEx::SetHwnd(HWND hwnd)
{
	m_hWnd = hwnd;
	return *this;
}

CViewerEx& CViewerEx::saveScreenCapture(const CString& strPath)
{
	return Screenshot(strPath);
}

CViewerEx& CViewerEx::addFigureText(const stFigureText& figure, float p, float q, COLORREF color)
{
	AddSoftGraphic(new GraphicLabel(figure.ptBegin.x, figure.ptBegin.y, figure.textString, ColorRefToColor(color)));
	return *this;
}

CViewerEx& CViewerEx::addFigureLine(const stFigure& figure, float p, float q, COLORREF color)
{
	AddSoftGraphic(new GraphicLine(figure.ptBegin, figure.ptEnd, ColorRefToColor(color), 1, Gdiplus::DashStyleSolid, Gdiplus::SmoothingModeNone));
	return *this;
}

CViewerEx& CViewerEx::addFigureRect(const stFigure& figure, float p, float q, COLORREF color)
{
	AddSoftGraphic(new GraphicRectangle(figure.ptBegin, figure.ptEnd, ColorRefToColor(color)));
	return *this;
}

bool CViewerEx::IsDrawFigure(int Id) const
{
	return true;
}

CViewerEx& CViewerEx::SetUseMask(bool use)
{
	return *this;
}

CViewerEx& CViewerEx::SetEnableDrawAllFigures(bool val)
{
	return *this;
}

CViewerEx& CViewerEx::SetEnableDrawFigure(int Id, bool ena)
{
	return *this;
}

FakeDC* CViewerEx::getOverlayDC() const
{
	return m_FakeCDC;
}

FakeDC* CViewerEx::getMaskOverlayDC() const
{
	return m_FakeCDC;
}

double CViewerEx::GetMagnification() const
{
	return GetImageToScreenTransform().Scale();
}

double CViewerEx::getCameraResolutionX() const
{
	return sqrt(m_MetricTrans[0] * m_MetricTrans[0] + m_MetricTrans[3] * m_MetricTrans[3]);
}

double CViewerEx::getCameraResolutionY() const
{
	return sqrt(m_MetricTrans[1] * m_MetricTrans[1] + m_MetricTrans[4] * m_MetricTrans[4]);
}

Rigid CViewerEx::GetImageToScreenTransform() const
{
	return m_SceneTrans.Inverse().Compose(m_ImageTrans);
}

Rigid CViewerEx::GetScreenToImageTransform() const
{
	return m_ImageTrans.Inverse().Compose(m_SceneTrans);
}

CViewerEx& CViewerEx::SetCustomInteractiveGraphic(GraphicBase* item)
{
	if ((NULL != item) && (item->IsInterative()))
	{
		m_InteractiveGraphic = item;
		item->SetActive(true);
	}
	SoftRefresh();
	return *this;
}

CViewerEx& CViewerEx::SettingCalipers(const CPoint& st, const CPoint& ed, int nCount, int projLEN, int searchLEN, bool reverse, bool lineMODE)
{
	m_InteractItems.m_InteractCaliper.Settings(st, ed, nCount, projLEN, searchLEN, reverse, lineMODE);
	SoftRefresh();
	return *this;
}

CViewerEx& CViewerEx::SettingSelector(const CPoint& st, const CPoint& ed, bool ellipseMODE)
{
	m_InteractItems.m_RoiSelector.Settings(st, ed, ellipseMODE);
	SoftRefresh();
	return *this;
}

CViewerEx& CViewerEx::GetCaliperSettings(CPoint& st, CPoint& ed, int& nCount, int& projLEN, int& searchLEN, bool& reverse, bool& lineMODE)
{
	m_InteractItems.m_InteractCaliper.GetSettings(st, ed, nCount, projLEN, searchLEN, reverse, lineMODE);
	return *this;
}

CViewerEx& CViewerEx::GetSelectorSettings(CPoint& st, CPoint& ed, bool& ellipseMODE)
{
	m_InteractItems.m_RoiSelector.GetSettings(st, ed, ellipseMODE);
	return *this;
}

CViewerEx& CViewerEx::ZoomIn()
{
	auto scale = 1. / MAX(MIN_ZOOM, MIN((1.2f / m_SceneTrans.Scale()), MAX_ZOOM));
	m_SceneTrans = m_SceneTrans.ScaleTo(scale);
	SoftRefresh();
	return *this;
}
CViewerEx& CViewerEx::ZoomOut()
{
	auto scale = 1. / MAX(MIN_ZOOM, MIN((.8333f / m_SceneTrans.Scale()), MAX_ZOOM));
	m_SceneTrans = m_SceneTrans.ScaleTo(scale);
	SoftRefresh();
	return *this;
}

CViewerEx& CViewerEx::OnLoadImageFromPtr(const void* data)
{
	return SetImage(data);
}

CViewerEx& CViewerEx::OnInitWithCamera(int width, int height, int nBpp)
{
	return SetImageSize(cv::Size(width, height), nBpp);
}

CViewerEx& CViewerEx::SetParent(CDialog* parent)
{
	m_Parent = parent;
	return *this;
}

CViewerEx& CViewerEx::SetSimpleMode(bool val)
{
	m_SimpleMode = val;
	CreateMousePopupMenu();
	SoftRefresh();
	return *this;
}

CViewerEx& CViewerEx::SetZoomByMouse(bool val)
{
	m_ZoomByMouse = val;
	SoftRefresh();
	return *this;
}

CViewerEx& CViewerEx::SetMoveByMouse(bool val)
{
	m_MoveByMouse = val;
	SoftRefresh();
	return *this;
}

CViewerEx& CViewerEx::SetInterativeItem(GraphicBase* item)
{
	m_InteractiveGraphic = item;
	SoftRefresh();
	return *this;
}

CViewerEx& CViewerEx::OnMouseMoveEvent(MouseEventCallback fnc)
{
	m_MouseEvents.OnMouseMove = fnc;
	return *this;
}

CViewerEx& CViewerEx::OnMouseUpEvent(MouseEventCallback fnc)
{
	m_MouseEvents.OnMouseUp = fnc;
	return *this;
}

CViewerEx& CViewerEx::OnMouseDownEvent(MouseEventCallback fnc)
{
	m_MouseEvents.OnMouseDown = fnc;
	return *this;
}

CViewerEx& CViewerEx::OnMouseDbClkEvent(MouseEventCallback fnc)
{
	m_MouseEvents.OnMouseDbClk = fnc;
	return *this;
}

CViewerEx& CViewerEx::SetEnableDrawGuidingLine(bool enable)
{
	m_ShowGuidingLine = enable;
	return *this;
}

CViewerEx& CViewerEx::ToggleDrawGuidingLine()
{
	m_ShowGuidingLine = !m_ShowGuidingLine;
	m_SoftDirty = true;
	return *this;
}

CViewerEx& CViewerEx::SetEnableDrawGuidingCross(bool enable)
{
	m_ShowGuidingCross = enable;
	m_SoftDirty = true;
	return *this;
}

CViewerEx& CViewerEx::ToggleDrawGuidingCross()
{
	m_ShowGuidingCross = !m_ShowGuidingCross;
	m_SoftDirty = true;
	return *this;
}

CViewerEx& CViewerEx::SetEnableDrawGuidingGraphics(bool enable)
{
	m_ShowGuidingCross = m_ShowGuidingLine = enable;
	m_SoftDirty = true;
	return *this;
}

CViewerEx& CViewerEx::ToggleDrawGuidingGraphics()
{
	m_ShowGuidingCross = m_ShowGuidingLine = !(m_ShowGuidingCross || m_ShowGuidingLine);
	m_SoftDirty = true;
	return *this;
}

CViewerEx& CViewerEx::SetFitMode(bool fit)
{
	OnFitImage();
	SoftRefresh();
	return *this;
}

CViewerEx& CViewerEx::SetRoiSelectionMode()
{
	m_InteractiveGraphic = &m_InteractItems.m_RoiSelector;
	m_InteractItems.m_RoiSelector.SetShowEllipse(false);
	CheckMenuItem(ID_MENU_RECT_SELECT, true);
	SoftRefresh();
	return *this;
}

CViewerEx& CViewerEx::SetRotateRoiSelectionMode()
{
	m_InteractiveGraphic = &m_InteractItems.m_RoiRotateSelector;
	CheckMenuItem(ID_MENU_ROTATE_SELECT, true);
	SoftRefresh();
	return *this;
}

CViewerEx& CViewerEx::SetDimensionalMode()
{
	m_InteractiveGraphic = &m_InteractItems.m_Dimensional;
	CheckMenuItem(ID_MENU_MEASURE, true);
	SoftRefresh();
	return *this;
}

CViewerEx& CViewerEx::SetRoiSelectionEllipseMode()
{
	m_InteractItems.m_RoiSelector.SetShowEllipse();
	CheckMenuItem(ID_MENU_ELLIPSE_SELECT, true);
	SoftRefresh();
	return *this;
}

CViewerEx& CViewerEx::GetCaliperBeginEnd(CPoint& st, CPoint& ed)
{
	m_InteractItems.m_InteractCaliper.GetBeginEnd(st, ed);
	return *this;
}

CViewerEx& CViewerEx::GetSelectorBeginEnd(CPoint& st, CPoint& ed)
{
	m_InteractItems.m_RoiSelector.GetBeginEnd(st, ed);
	return *this;
}

CViewerEx& CViewerEx::SetSelectorBeginEnd(const CPoint& st, const CPoint& ed)
{
	m_InteractItems.m_RoiSelector.SetBeginEnd(st, ed);
	return *this;
}

CViewerEx& CViewerEx::GetBeginEnd(CPoint& st, CPoint& ed)
{
	if (m_InteractiveGraphic != NULL)
		m_InteractiveGraphic->GetBeginEnd(st, ed);
	return *this;
}

CViewerEx& CViewerEx::SetBeginEnd(const CPoint& st, const CPoint& ed)
{
	if (m_InteractiveGraphic != NULL)
		m_InteractiveGraphic->SetBeginEnd(st, ed);
	SoftRefresh();
	return *this;
}

CViewerEx& CViewerEx::InactiveInteraction(bool inAct)
{
	if (NULL != m_InteractiveGraphic)
		m_InteractiveGraphic->SetActive(!inAct);

	SetInteraction(!inAct);

	SoftRefresh();
	return *this;
}

CViewerEx& CViewerEx::SoftDirtyRefresh()
{
	SoftRefresh();
	return *this;
}

CViewerEx& CViewerEx::HardDirtyRefresh()
{
	HardRefresh();
	return *this;
}

CViewerEx& CViewerEx::DirtyRefresh()
{
	if (m_Loaded)
	{
		m_SoftDirty = true;
		m_HardDirty = true;
		Invalidate();
	}
	return *this;
}

CViewerEx& CViewerEx::SetCaliperProjectionLength(int val)
{
	m_InteractItems.m_InteractCaliper.setProjectionLength(val);
	if (IsCalipersMode())
	{
		m_SkipBaseImage = true;
		SoftRefresh();
	}
	return *this;
}

CViewerEx& CViewerEx::SetCaliperSearchLength(int val)
{
	m_InteractItems.m_InteractCaliper.setSearchLength(val);
	if (IsCalipersMode())
	{
		m_SkipBaseImage = true;
		SoftRefresh();
	}
	return *this;
}

CViewerEx& CViewerEx::SetNumOfCalipers(int val)
{
	m_InteractItems.m_InteractCaliper.setNumOfCalipers(val);
	if (IsCalipersMode())
	{
		m_SkipBaseImage = true;
		SoftRefresh();
	}
	return *this;
}

CViewerEx& CViewerEx::SetCaliperCircleMode(bool circle)
{
	CheckMenuItem(ID_MENU_CIRCLE_CALIPER, circle);
	m_InteractItems.m_InteractCaliper.SetCircleMode(circle);
	if (IsCalipersMode())
	{
		m_SkipBaseImage = true;
		SoftRefresh();
	}
	return *this;
}

CViewerEx& CViewerEx::SetCaliperLineMode(bool line)
{
	CheckMenuItem(ID_MENU_LINE_CALIPER, line);
	m_InteractItems.m_InteractCaliper.SetCircleMode(!line);
	if (IsCalipersMode())
	{
		m_SkipBaseImage = true;
		SoftRefresh();
	}
	return *this;
}

CViewerEx& CViewerEx::SetCaliperBeginEnd(CPoint st, CPoint ed)
{
	m_InteractItems.m_InteractCaliper.SetBeginEnd(st, ed);
	if (IsCalipersMode())
	{
		m_SkipBaseImage = true;
		SoftRefresh();
	}
	return *this;
}

CViewerEx& CViewerEx::SetCaliperReverseMode(bool reverse)
{
	m_InteractItems.m_InteractCaliper.SetReverseMode(reverse);
	if (IsCalipersMode())
	{
		m_SkipBaseImage = true;
		SoftRefresh();
	}
	return *this;
}

CViewerEx& CViewerEx::ToggleCaliperDirection()
{
	m_InteractItems.m_InteractCaliper.SetReverseMode(!m_InteractItems.m_InteractCaliper.IsReverse());
	if (IsCalipersMode())
	{
		m_SkipBaseImage = true;
		SoftRefresh();
	}
	return *this;
}

bool CViewerEx::GetCaliperDirection() const
{
	return m_InteractItems.m_InteractCaliper.IsReverse();
}

bool CViewerEx::IsInteraction() const
{
	return m_Interative;
}

CViewerEx& CViewerEx::SetNavMode(bool nav)
{
	m_Navigation = nav;
	CheckMenuItem(ID_MENU_NAVIGATION, m_Navigation);
	HardRefresh();
	return *this;
}

void CViewerEx::DrawPaddingBorders(Gdiplus::Graphics& graph, cv::Rect padRECT, cv::Size maxSize)
{
	auto tl = padRECT.tl();
	auto br = padRECT.br();
	auto rightPAD = maxSize.width - br.x;
	auto bottomPAD = maxSize.height - br.y;
	auto height_ = br.y - tl.y;

	if (tl.y > 0)graph.FillRectangle(m_PaddingBrush, 0, 0, maxSize.width, tl.y);
	if (bottomPAD > 0)graph.FillRectangle(m_PaddingBrush, 0, br.y, maxSize.width, bottomPAD);
	if (tl.x > 0)graph.FillRectangle(m_PaddingBrush, 0, tl.y, tl.x, height_);
	if (rightPAD > 0)graph.FillRectangle(m_PaddingBrush, br.x, tl.y, rightPAD, height_);
}

void CViewerEx::RenderSoftGraphic(const Rigid& imgToView, cv::Size maxSize)
{
	if (m_SoftDirty)
	{
		if (!m_SimpleMode)
		{
			Gdiplus::Graphics graph(m_SoftIMG);
			OptimalGraphicSettings(graph, true);
			RenderSoftGraphic(graph, imgToView, maxSize);
		}
		m_SoftDirty = false;
	}
}

void CViewerEx::RenderHardGraphic(const Rigid& imgToView, cv::Size maxSize)
{
	if (m_HardDirty)
	{
		if (!m_SimpleMode)
		{
			Gdiplus::Graphics graph(m_HardIMG);
			OptimalGraphicSettings(graph, true);
			RenderHardGraphic(graph, imgToView, maxSize);
		}
		m_HardDirty = false;
	}
}

void CViewerEx::RenderSoftGraphic(Gdiplus::Graphics& graph, const Rigid& imgToView, cv::Size maxSize)
{
	for (auto& it : m_SoftGrahics)
	{
		if (it)	it->Render(graph, imgToView);
		else break;
	}

	if (m_ShowGuidingLine)
		ShowGuidingLine(graph, imgToView, m_ImageSize, m_PenLINE);

	if (m_ShowInActiveInteractive || ((NULL != m_InteractiveGraphic) && m_InteractiveGraphic->IsActive()))
		m_InteractiveGraphic->Render(graph, imgToView);

	if (m_ShowGuidingCross)
		ShowGuidingCross(graph, imgToView, m_ImageSize, m_PenCROSS);
}

void CViewerEx::RenderHardGraphic(Gdiplus::Graphics& graph, const Rigid& imgToView, cv::Size maxSize)
{
	for (auto it : m_HardGrahics)
	{
		if (it)	it->Render(graph, imgToView, CSize(maxSize.width, maxSize.height));
		else break;		
	}

	if (m_Navigation && (NULL != m_NavigateIMG))
	{
		auto _w = m_NavigateIMG->GetWidth();
		auto _h = m_NavigateIMG->GetHeight();
		auto top = MAX(maxSize.height - _h - 15, 0);
		auto left = float(maxSize.width - _w) / 2.f;

		m_NavigationRect = Gdiplus::RectF(left, top, _w, _h);

		graph.SetCompositingMode(Gdiplus::CompositingMode::CompositingModeSourceOver);
		graph.DrawImage(m_NavigateIMG, m_NavigationRect, Gdiplus::REAL(0), Gdiplus::REAL(0), Gdiplus::REAL(_w), Gdiplus::REAL(_h), Gdiplus::Unit::UnitPixel, &att);
		graph.DrawRectangle(m_RedPen, m_NavigationRect);
	}
}

bool CViewerEx::GetUseMask() const
{
	return true;
}

CViewerEx& CViewerEx::SetMaskPen(bool use)
{
	return *this;
}

CViewerEx& CViewerEx::SetMaskRect(bool use)
{
	return *this;
}

CViewerEx& CViewerEx::SetSemiTransparent(bool semi)
{
	return *this;
}

CViewerEx& CViewerEx::ClearCaliperOverlayDC(bool clear)
{
	return *this;
}

CViewerEx& CViewerEx::SetModeDrawingFigure(bool disp)
{
	return *this;
}

CViewerEx& CViewerEx::SetEnableModifyFigure(int Id, bool disp)
{
	return *this;
}

CViewerEx& CViewerEx::setUseCaliper(bool val)
{
	return *this;
}

CViewerEx& CViewerEx::setDrawCaliper(bool val)
{
	return *this;
}

CViewerEx& CViewerEx::SetEnableFigureRectangle(int val)
{
	return *this;
}