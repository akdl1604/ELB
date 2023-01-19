// ImageViewer.cpp: 구현 파일
//

#include "stdafx.h"
#include "ImageViewer.h"
#include "afxdialogex.h"
#include "resource.h"

IMPLEMENT_DYNAMIC(ImageViewer, CDialogEx)

ImageViewer::ImageViewer(CWnd* pParent /*=nullptr*/, bool selfDestroy)
	: CDialogEx(IDD_IMAGE_VIEWER, pParent), m_SelfDestroy(selfDestroy)
{
}

ImageViewer::~ImageViewer()
{
}

void ImageViewer::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GRAPHIC_DISPLAY, dsp);
}

BEGIN_MESSAGE_MAP(ImageViewer, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

void ImageViewer::PostNcDestroy()
{
	CDialogEx::PostNcDestroy();
	if (m_SelfDestroy) delete this;
}

void ImageViewer::OnClose()
{
	if (OnClosing) OnClosing(this);
	CDialogEx::OnClose();
	if (m_SelfDestroy) PostNcDestroy();
}

void ImageViewer::SetImage(const cv::Mat& src)
{
	dsp.SetImage(src);
	//dsp.CreateSampleGraphic();
}

void ImageViewer::SetTitle(CString strTitle)
{
	m_StrTitle = strTitle;
	SetWindowText(m_StrTitle);
}

CString ImageViewer::GetTitle() const
{
	return m_StrTitle;
}

BOOL ImageViewer::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	dsp.SetSimpleMode(true);
	dsp.MouseEvents().OnMouseMove = [&](CViewerEx* sender, CPoint pt)
	{
		auto color = sender->GetPixel(pt);
		auto w = sender->ImageWidth();
		auto h = sender->ImageHeight();
		SetWindowText(fmt("%s - [%dx%d] (%d, %d) -> (R: %d, G: %d, B: %d)", CStringA(m_StrTitle).GetString(), w, h, pt.x, pt.y, color.GetR(), color.GetG(), color.GetB()));
	};

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void ImageViewer::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (dsp)
	{
		CRect rect;
		GetClientRect(&rect);
		dsp.MoveWindow(rect);
	}
}


void ImageViewer::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize = CPoint(420, 320);

	CDialogEx::OnGetMinMaxInfo(lpMMI);
}
