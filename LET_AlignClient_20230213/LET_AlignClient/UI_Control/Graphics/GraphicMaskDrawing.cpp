#include "stdafx.h"
#include "GraphicMaskDrawing.h"

bool GraphicMaskDrawing::MouseDown(CPoint pos, const Rigid& trans)
{
	bool repaint = false;
	if (!m_StopPassingEvent)
	{
		if (m_PickPoint.IsActive())
		{
			m_PickPoint.MouseDown(pos, trans);
			repaint = true;
		}
		else if (NULL != m_CurrentActive)
		{
			m_CurrentActive->MouseDown(pos, trans);
			repaint = true;
		}
	}
	return repaint;
}

bool GraphicMaskDrawing::MouseUp(CPoint pos, const Rigid& trans)
{
	bool repaint = false;
	if (!m_StopPassingEvent)
	{
		if (m_PickPoint.IsActive())
		{
			m_PickPoint.MouseUp(pos, trans);
			repaint = true;
		}
		else if (NULL != m_CurrentActive)
		{
			if (m_CurrentActive->MouseUp(pos, trans))
				DoShape();

			repaint = true;
		}
	}
	return repaint;
}

bool GraphicMaskDrawing::MouseMove(CPoint pos, const Rigid& trans)
{
	bool repaint = false;
	if (!m_StopPassingEvent)
	{
		if (m_PickPoint.IsActive())
		{
			m_PickPoint.MouseMove(pos, trans);
			repaint = true;
		}
		if (NULL != m_CurrentActive)
		{
			m_CurrentActive->MouseMove(pos, trans);
			repaint = true;
		}
	}
	return repaint;
}

bool GraphicMaskDrawing::MouseDbClk(CPoint pos, const Rigid& trans)
{
	if (!m_StopPassingEvent)
	{
		if ((NULL != m_CurrentActive) && (!m_PickPoint.IsActive()))
		{
			if (m_CurrentActive->MouseDbClk(pos, trans))
				DoShape();

			return true;
		}
	}
	return false;
}

GraphicMaskDrawing& GraphicMaskDrawing::SetActive(bool act)
{
	return *this;
}

GraphicMaskDrawing& GraphicMaskDrawing::SetMetricTransform(const Rigid& rigid)
{
	return *this;
}

GraphicMaskDrawing::GraphicMaskDrawing()
{
	//auto go = new MaskPen;// new MaskPickPoint;
	//auto go = new MaskPickPoint;// new MaskPickPoint;
	//go->SetMaxSize(m_ImageSize);
	//m_CurrentActive = go;// new MaskPolygon;
	m_PickPoint.SetActive(true);
}

GraphicMaskDrawing& GraphicMaskDrawing::Render(Gdiplus::Graphics& graph, const Rigid& trans)
{
	for (auto& go: m_MaskObjects) go->Render(graph, trans);
	if (NULL != m_CurrentActive)m_CurrentActive->Render(graph, trans);
	m_PickPoint.Render(graph, trans);

	return *this;
}

GraphicMaskDrawing::~GraphicMaskDrawing()
{
	ClearShapeObjects();
}

GraphicMaskDrawing& GraphicMaskDrawing::ClearShapeObjects()
{
	if (NULL != m_CurrentActive)
	{
		delete m_CurrentActive;
		m_CurrentActive = NULL;
	}

	for (int i = 0; i < m_MaskObjects.size(); i++)
		delete m_MaskObjects[i];

	m_MaskObjects.clear();
	return *this;
}

GraphicMaskDrawing& GraphicMaskDrawing::SaveMask(CString strFile)
{
	cv::imwrite(strFile.GetString(), GetMarkMask());
	return *this;
}

cv::Mat GraphicMaskDrawing::GetMarkMask()
{
	cv::Mat scene(cv::Size(m_ImageSize.Width, m_ImageSize.Height), CV_8UC4, cv::Scalar(0));
	auto* bm = new Gdiplus::Bitmap(m_ImageSize.Width, m_ImageSize.Height, INT(scene.step1()), PixelFormat32bppPARGB, scene.data);
	{
		Gdiplus::Graphics graph(bm);
		graph.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
		for (auto& go: m_MaskObjects) go->RenderToMask(graph);
	}
	delete bm;
	return scene;
}

bool GraphicMaskDrawing::IsPickPointMode() const
{ 
	return m_PickPoint.IsActive(); 
}

GraphicMaskDrawing& GraphicMaskDrawing::SetPickPointMode(bool ena)
{
	if (NULL != m_CurrentActive)
	{
		delete m_CurrentActive;
		m_CurrentActive = NULL;
	}
	m_PickPoint.SetActive(ena);
	return *this;
}

GraphicMaskDrawing& GraphicMaskDrawing::SetFillMode(MaskFillMode fillMode)
{
	m_FillMode = fillMode;
	if (NULL != m_CurrentActive)
		m_CurrentActive->SetFillMode(fillMode);
	
	return *this;
}

GraphicMaskDrawing& GraphicMaskDrawing::OffsetOrign(float dx, float dy)
{
	m_PickPoint.OffsetSelectedPoint(dx, dy);
	return *this;
}

GraphicMaskDrawing& GraphicMaskDrawing::DoShape(DOGU shapeMode)
{
	return DoShape(shapeMode, m_PenWidth);
}

GraphicMaskDrawing& GraphicMaskDrawing::SetPenWidth(int w)
{
	m_PenWidth = w;
	if (NULL != m_CurrentActive)
		m_CurrentActive->SetPenWidth(m_PenWidth);

	return *this;
}

int GraphicMaskDrawing::GetPenWidth() const
{
	return m_PenWidth;
}

GraphicMaskDrawing& GraphicMaskDrawing::IgnoreMouse(bool val)
{
	m_StopPassingEvent = val;
	return *this;
}

GraphicMaskDrawing& GraphicMaskDrawing::RemoveLastItem()
{
	if (m_MaskObjects.size() > 0)
		m_MaskObjects.erase(m_MaskObjects.end() - 1, m_MaskObjects.end());

	m_PickPoint.SetActive(false);

	if (NULL != m_CurrentActive)
	{
		delete m_CurrentActive;
		m_CurrentActive = NULL;
	}
	return *this;
}

Gdiplus::PointF GraphicMaskDrawing::GetSelectedPoint() const
{
	return m_PickPoint.GetSelectedPoint();
}

GraphicMaskDrawing& GraphicMaskDrawing::SetSelectedPoint(float x, float y)
{
	m_PickPoint.SetSelectedPoint(x, y);
	return *this;
}

GraphicMaskDrawing& GraphicMaskDrawing::DoShape()
{
	int width = m_PenWidth;
	DOGU shape = DOGU::DNONE;
	MaskFillMode fill = m_FillMode;

	if (m_PickPoint.IsActive())m_PickPoint.SetActive(false);

	if (NULL != m_CurrentActive)
	{
		fill = m_CurrentActive->GetFillMode();
		shape = m_CurrentActive->GetShapeMode();
		width = m_CurrentActive->GetPenWidth();

		if (m_CurrentActive->IsValid())
		{
			m_CurrentActive->DoFinish();
			m_MaskObjects.push_back(m_CurrentActive);
		}
		else delete m_CurrentActive;
		m_CurrentActive = NULL;
	}

	switch (shape)
	{
		// No repeat polygon
		//case DOGU::D_POLYGON:
		//	m_CurrentActive = new MaskPolygon(size);
		//	break;

	case DOGU::DELLIPSE:
		m_CurrentActive = new MaskEllipse(width);
		break;

	case DOGU::DRECT:
		m_CurrentActive = new MaskRectangle(width);
		break;

	case DOGU::DPEN:
		m_CurrentActive = new MaskPen(width);
		break;

	default:
		break;
	}

	if (NULL != m_CurrentActive)
		m_CurrentActive->SetFillMode(fill);

	return *this;
}

GraphicMaskDrawing& GraphicMaskDrawing::DoShape(DOGU shapeMode, int size)
{
	m_PenWidth = size;
	m_ShapeMode = shapeMode;

	if (m_PickPoint.IsActive())m_PickPoint.SetActive(false);

	if ((nullptr != m_CurrentActive) && m_CurrentActive->IsValid()) {
		m_CurrentActive->DoFinish();
		m_MaskObjects.push_back(m_CurrentActive);
	}

	switch (shapeMode)
	{
	case DOGU::D_POLYGON:
		m_CurrentActive = new MaskPolygon(m_PenWidth);
		break;

	case DOGU::DELLIPSE:
		m_CurrentActive = new MaskEllipse(m_PenWidth);
		break;

	case DOGU::DRECT:
		m_CurrentActive = new MaskRectangle(m_PenWidth);
		break;

	case DOGU::DPEN:
	default:
		m_CurrentActive = new MaskPen(m_PenWidth);
		break;
	}

	if (NULL != m_CurrentActive)
		m_CurrentActive->SetFillMode(m_FillMode);

	return *this;
}

GraphicMaskDrawing& GraphicMaskDrawing::ImportShapes(CString strFile)
{
	if (_access(strFile, 0) == 0)
	{
		CFile file(strFile.GetString(), CFile::modeRead);

		if (file.GetLength() > 0) {
			CArchive ar(&file, CArchive::load);
			int size = 0;
			ar >> size;

			for (int i = 0; i < size; i++)
			{
				MaskBase* so = NULL;
				int mode = int(DOGU::DNONE);

				ar >> mode;

				switch (DOGU(mode))
				{
				case DOGU::DELLIPSE:
					so = new MaskEllipse();
					break;

				case DOGU::DRECT:
					so = new MaskRectangle();
					break;

				case DOGU::D_POLYGON:
					so = new MaskPolygon();
					break;

				case DOGU::DPEN:
					so = new MaskPen();
					break;

				default:
					break;
				}
				if (NULL != so)
				{
					so->Serialize(ar);
					m_MaskObjects.push_back(so);
				}
			}
			ar.Close();
		}
		file.Close();
	}

	return *this;
}

GraphicMaskDrawing& GraphicMaskDrawing::ExportShapes(CString strFile)
{
	CFile file(strFile.GetString(), CFile::modeWrite | CFile::modeCreate);
	CArchive ar(&file, CArchive::store);

	ar << int(m_MaskObjects.size());
	for (const auto& go: m_MaskObjects)
		go->Serialize(ar);

	ar.Flush();
	file.Flush();
	ar.Close();
	file.Close();
	return *this;
}

GraphicMaskDrawing& GraphicMaskDrawing::SetPickRectMode(bool brect){ m_PickPoint.SetActiveRectMode(brect); return *this; }
GraphicMaskDrawing& GraphicMaskDrawing::SetMaxSize(Gdiplus::Size size) { return SetMaxSize(size.Width, size.Height); }
GraphicMaskDrawing& GraphicMaskDrawing::SetMaxSize(cv::Size size) { return SetMaxSize(size.width, size.height); }
GraphicMaskDrawing& GraphicMaskDrawing::SetMaxSize(CSize size) { return SetMaxSize(size.cx, size.cy); }
GraphicMaskDrawing& GraphicMaskDrawing::SetMaxSize(int width, int height)
{
	m_ImageSize = Gdiplus::Size(width, height);
	m_PickPoint.SetMaxSize(m_ImageSize);
	return *this;
}