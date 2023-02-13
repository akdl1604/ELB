// RadioEx.cpp : implementation file
//

#include "stdafx.h"
#include "RadioEx.h"

// CRadioEx

IMPLEMENT_DYNAMIC(CRadioEx, CButton)



CRadioEx::CRadioEx()
{
	m_clrBkg		= Color(255, 41, 41, 41);
	m_clrBorder		= Color(255, 51, 51, 51);
	m_clrBoxBkg		= Color(255, 81, 81, 81);
	m_clrBoxBorder	= Color(255,  0,  0,128); 
	m_clrCheck		= Color(255, 31, 31, 31);
	m_clrText		= Color(255,255,255,255);

	m_bEnable	= true;
	m_bMouseTrack = FALSE;

	m_fSizeBorder	= 4;
	m_fSizeBoxBorder= 1;
	m_nSizeCheck	= 10;
	m_fSizeText		= 20;
	m_nOffsetTextX	= 0;
	m_nOffsetTextY	= 0;

	m_nTextAlign1	= 0;
	m_nTextAlign2	= 0;

	m_pBmpImage = NULL;
	m_pBmpCheck = NULL;
	m_pBmpUnCheck = NULL;

	m_rcImage = Rect(0,0,0,0);
	m_rcCaption = Rect(0,0,0,0);
	m_rcCheckBox = Rect(0,0,0,0);
}

CRadioEx::~CRadioEx()
{
	if (m_pBmpImage != NULL)
	{
		delete m_pBmpImage;
		m_pBmpImage = NULL;
	}

	if (m_pBmpCheck != NULL)
	{
		delete m_pBmpCheck;
		m_pBmpCheck = NULL;
	}

	if (m_pBmpUnCheck != NULL)
	{
		delete m_pBmpUnCheck;
		m_pBmpUnCheck = NULL;
	}
}


BEGIN_MESSAGE_MAP(CRadioEx, CButton)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()
// CRadioEx message handlers

void CRadioEx::PreSubclassWindow()
{
	CRect rect;
	GetClientRect(&rect);

	m_rcCheckBox = Rect(rect.left, rect.top, rect.Height(), rect.Height());
	m_rcCaption = Rect(rect.Height(), rect.top, rect.Width()-rect.Height(), rect.Height());
	m_rcImage = m_rcCaption;

	CButton::PreSubclassWindow();
}


BOOL CRadioEx::OnEraseBkgnd(CDC* pDC)
{
	//Avoid Flicker.
	//return CWnd::OnEraseBkgnd(pDC);
	return TRUE;
}


void CRadioEx::OnPaint()
{
	//CPaintDC dc(this); // device context for painting

	//CRect rect;
	//GetClientRect(&rect);

	//Graphics mainG (dc.GetSafeHdc());

	//// 메모리 생성
	//Bitmap memBmp(rect.Width(), rect.Height());

	//Graphics memG(&memBmp);

	//// 잔상을 없애기 위해 white 채움.
	//SolidBrush brs(Color(255,0,0,0));
	//memG.FillRectangle(&brs,0,0,rect.Width(),rect.Height());

	////----- 메모리상에 그리는 부분 -----//
	//if (m_pBmpCheck == NULL && m_pBmpUnCheck == NULL)
	//{
	//	DrawBkg(&memG);
	//	DrawBorder(&memG);
	//	DrawBoxBkg(&memG);
	//	DrawBoxBorder(&memG);
	//	DrawCheck(&memG);
	//	DrawImage(&memG);
	//	DrawText(&memG);
	//}
	//else
	//{
	//	DrawBitmap(&memG);
	//}

	////---------------------------------//

	//if (m_bEnable == true)
	//{
	//	// 그려진 메모리를 최종 출력
	//	mainG.DrawImage(&memBmp,0,0);
	//}
	//else
	//{
	//	ColorMatrix GrayMat = {	
	//		0.30f, 0.30f, 0.30f, 0.00f, 0.00f,
	//		0.59f, 0.59f, 0.59f, 0.00f, 0.00f,
	//		0.11f, 0.11f, 0.11f, 0.00f, 0.00f,
	//		0.00f, 0.00f, 0.00f, 1.00f, 0.00f,
	//		0.00f, 0.00f, 0.00f, 0.00f, 1.00f	};

	//		ImageAttributes ia;
	//		ia.SetColorMatrix(&GrayMat);

	//		RectF grect; grect.X=0, grect.Y=0; grect.Width = (float)rect.Width(); grect.Height = (float)rect.Height();
	//		mainG.DrawImage(&memBmp, grect, 0, 0, (float)rect.Width(), (float)rect.Height(), UnitPixel, &ia);
	//}

	CPaintDC dc(this); // device context for painting

	CRect rect;
	GetClientRect(&rect);

	Graphics mainG (dc.GetSafeHdc());

	// 메모리 생성
	Bitmap memBmp(rect.Width(), rect.Height());

	Graphics memG(&memBmp);

	// 잔상을 없애기 위해 white 채움.
	SolidBrush brs(Color(255,0,0,0));
	memG.FillRectangle(&brs,0,0,rect.Width(),rect.Height());

	//----- 메모리상에 그리는 부분 -----//
	if (m_pBmpCheck == NULL && m_pBmpUnCheck == NULL)
	{
		DrawBkg(&memG);
		DrawBorder(&memG);
		DrawBoxBkg(&memG);
		DrawBoxBorder(&memG);
		DrawCheck(&memG);
		DrawImage(&memG);
		DrawText(&memG);
	}
	else
	{
		DrawBitmap(&memG);
	}

	//---------------------------------//

	if (m_bEnable == true)
	{
		if (m_bMouseTrack == TRUE)
		{
			ColorMatrix HotMat = {	
				1.05f, 0.00f, 0.00f, 0.00f, 0.00f,
				0.00f, 1.05f, 0.00f, 0.00f, 0.00f,
				0.00f, 0.00f, 1.05f, 0.00f, 0.00f,
				0.00f, 0.00f, 0.00f, 1.00f, 0.00f,
				0.05f, 0.05f, 0.05f, 0.00f, 1.00f	};

				ImageAttributes ia;
				ia.SetColorMatrix(&HotMat);

				RectF grect; grect.X=0, grect.Y=0; grect.Width = (float)rect.Width(); grect.Height = (float)rect.Height();
				mainG.DrawImage(&memBmp, grect, 0, 0, (float)rect.Width(), (float)rect.Height(), UnitPixel, &ia);
		}
		else
		{
			// 그려진 메모리를 최종 출력
			mainG.DrawImage(&memBmp,0,0);
		}
	}
	else
	{
		ColorMatrix GrayMat = {	
			0.30f, 0.30f, 0.30f, 0.00f, 0.00f,
			0.59f, 0.59f, 0.59f, 0.00f, 0.00f,
			0.11f, 0.11f, 0.11f, 0.00f, 0.00f,
			0.00f, 0.00f, 0.00f, 1.00f, 0.00f,
			0.00f, 0.00f, 0.00f, 0.00f, 1.00f	};

			ImageAttributes ia;
			ia.SetColorMatrix(&GrayMat);

			RectF grect; grect.X=0, grect.Y=0; grect.Width = (float)rect.Width(); grect.Height = (float)rect.Height();
			mainG.DrawImage(&memBmp, grect, 0, 0, (float)rect.Width(), (float)rect.Height(), UnitPixel, &ia);
	}
}

void CRadioEx::DrawBitmap(Graphics *pG)
{
	CRect rect;
	GetClientRect(&rect);

	int nStatus = GetCheck();
	if (nStatus == 1)
	{
		pG->DrawImage(m_pBmpCheck, Rect(0,0,rect.Width(),rect.Height()), 0,0, m_pBmpCheck->GetWidth(), m_pBmpCheck->GetHeight(), UnitPixel);
	}
	else
	{
		pG->DrawImage(m_pBmpUnCheck, Rect(0,0,rect.Width(),rect.Height()), 0,0, m_pBmpUnCheck->GetWidth(), m_pBmpUnCheck->GetHeight(), UnitPixel);
	}

}

void CRadioEx::DrawBkg(Graphics *pG)
{
	CRect rect;
	GetClientRect(&rect);

	SolidBrush brs(m_clrBkg);

	pG->FillRectangle(&brs,rect.left,rect.top,rect.right,rect.bottom);
}

void CRadioEx::DrawBorder(Graphics *pG)
{
	CRect rect;
	GetClientRect(&rect);

	Pen pen(m_clrBorder,m_fSizeBorder);

	pG->DrawRectangle(&pen,rect.left,rect.top,rect.Width(),rect.Height());
}

void CRadioEx::DrawBoxBkg(Graphics *pG)
{
	CRect rect;
	GetClientRect(&rect);

	SolidBrush brs(m_clrBoxBkg);

	pG->FillEllipse(&brs,m_rcCheckBox);
}

void CRadioEx::DrawBoxBorder(Graphics *pG)
{
	CRect rect;
	GetClientRect(&rect);

	Pen pen(m_clrBoxBorder,m_fSizeBoxBorder);

	pG->DrawEllipse(&pen,m_rcCheckBox);
}

void CRadioEx::DrawCheck(Graphics *pG)
{
	int nStatus = GetCheck();
	if (nStatus == 1)
	{
		int centerX = m_rcCheckBox.X + m_rcCheckBox.Width/2;
		int centerY = m_rcCheckBox.Y + m_rcCheckBox.Height/2;

		SolidBrush brs(m_clrCheck);
		if (m_nSizeCheck > m_rcCheckBox.Width/2)
			m_nSizeCheck = m_rcCheckBox.Width/2;

		Rect rcCheck(centerX-m_nSizeCheck, centerY-m_nSizeCheck, m_nSizeCheck*2, m_nSizeCheck*2);

		pG->FillEllipse(&brs,rcCheck);
	}
}

void CRadioEx::DrawImage(Graphics *pG)
{
	CRect rect;
	GetClientRect(&rect);

	if (m_pBmpImage == NULL) return;

	pG->DrawImage(m_pBmpImage, m_rcImage, 0,0, m_pBmpImage->GetWidth(), m_pBmpImage->GetHeight(), UnitPixel);
}

void CRadioEx::DrawText(Graphics *pG)
{
	CString text = _T("");
	GetWindowText(text);

	FontFamily fontptroll(L"Arial");
	Gdiplus::Font font(&fontptroll, m_fSizeText, FontStyleRegular, UnitPixel);

	StringFormat formatAlign;
	formatAlign.SetAlignment((Gdiplus::StringAlignment)m_nTextAlign1);		// Left / Center / Right
	formatAlign.SetLineAlignment((Gdiplus::StringAlignment)m_nTextAlign2);	// Top / Middle / Bottom

	SolidBrush brs(m_clrText);
	RectF rcfCaption((REAL)m_rcCaption.GetLeft(), (REAL)m_rcCaption.GetTop(), (REAL)m_rcCaption.Width, (REAL)m_rcCaption.Height);
	pG->DrawString(towstring(text).c_str(),text.GetLength(),&font,rcfCaption,&formatAlign,&brs);
}

bool CRadioEx::LoadImageFromFile(CString strPath)
{
	if (IsFileExist(strPath) == false) return false;

	if (m_pBmpImage != NULL)
	{
		delete m_pBmpImage;
		m_pBmpImage = NULL;
	}

	m_pBmpImage = (Bitmap*)Image::FromFile(towstring(strPath).c_str());

	if (m_pBmpImage->GetLastStatus() != Gdiplus::Ok)
		return false;

	Invalidate();

	return true;
}

bool CRadioEx::LoadImageFromResource(UINT ID, bool bPNG)
{
	if (m_pBmpImage != NULL)
	{
		delete m_pBmpImage;
		m_pBmpImage = NULL;
	}

	if (bPNG == false)
	{
		m_pBmpImage = Bitmap::FromResource(AfxGetInstanceHandle(),(WCHAR*)MAKEINTRESOURCE(ID));
	}
	else
	{
		// PNG 리소스는 Bitmap 리소스를 불러오는 것보다 복잡하다.
		HRSRC hResource = ::FindResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(ID), _T("PNG"));	// 3번째 인자는 실제 리소스가 분류되는 문자열이 전달되어야 한다.
		if (!hResource) return false;

		DWORD imageSize = ::SizeofResource(AfxGetApp()->m_hInstance, hResource);
		if (!imageSize) return false;

		const void* pResourceData = ::LockResource(::LoadResource(AfxGetApp()->m_hInstance, hResource));
		if (!pResourceData) return false;

		HGLOBAL hBuffer  = ::GlobalAlloc(GMEM_MOVEABLE, imageSize);
		if (hBuffer)
		{
			void* pBuffer = ::GlobalLock(hBuffer);
			if (pBuffer)
			{
				CopyMemory(pBuffer, pResourceData, imageSize);

				IStream* pStream = NULL;
				if (::CreateStreamOnHGlobal(hBuffer, FALSE, &pStream) == S_OK)
				{
					m_pBmpImage = Gdiplus::Bitmap::FromStream(pStream);
					pStream->Release();

					if (m_pBmpImage)
					{
						if (m_pBmpImage->GetLastStatus() != Gdiplus::Ok)
						{
							delete m_pBmpImage;
							m_pBmpImage = NULL;
						}
					}
				}
			}
			::GlobalUnlock(hBuffer);
			::GlobalFree(hBuffer);
		}
	}


	if (m_pBmpImage->GetLastStatus() != Gdiplus::Ok)
		return false;

	Invalidate();

	return true;
}

bool CRadioEx::LoadImageFromFile(CString strCheck, CString strUnCheck)
{
	if (IsFileExist(strCheck) == false) return false;
	if (IsFileExist(strUnCheck) == false) return false;

	if (m_pBmpCheck != NULL)
	{
		delete m_pBmpCheck;
		m_pBmpCheck = NULL;
	}

	if (m_pBmpUnCheck != NULL)
	{
		delete m_pBmpUnCheck;
		m_pBmpUnCheck = NULL;
	}

	m_pBmpCheck = (Bitmap*)Image::FromFile(towstring(strCheck).c_str());
	if (m_pBmpCheck->GetLastStatus() != Gdiplus::Ok)
		return false;

	m_pBmpUnCheck = (Bitmap*)Image::FromFile(towstring(strUnCheck).c_str());
	if (m_pBmpUnCheck->GetLastStatus() != Gdiplus::Ok)
		return false;

	Invalidate();

	return true;
}

bool CRadioEx::LoadImageFromResource(UINT idCheck, UINT idUnCheck, bool bPNG)
{
	if (m_pBmpCheck != NULL)
	{
		delete m_pBmpCheck;
		m_pBmpCheck = NULL;
	}

	if (m_pBmpUnCheck != NULL)
	{
		delete m_pBmpUnCheck;
		m_pBmpUnCheck = NULL;
	}

	if (bPNG == false)
	{
		m_pBmpCheck = Bitmap::FromResource(AfxGetInstanceHandle(),(WCHAR*)MAKEINTRESOURCE(idCheck));
		if (m_pBmpCheck->GetLastStatus() != Gdiplus::Ok)
			return false;

		m_pBmpUnCheck = Bitmap::FromResource(AfxGetInstanceHandle(),(WCHAR*)MAKEINTRESOURCE(idUnCheck));
		if (m_pBmpUnCheck->GetLastStatus() != Gdiplus::Ok)
			return false;
	}
	else
	{
		for (int i=0; i<2; i++)
		{
			UINT ID=0;
			i==0 ? ID=idCheck : ID=idUnCheck;

			// PNG 리소스는 Bitmap 리소스를 불러오는 것보다 복잡하다.
			HRSRC hResource = ::FindResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(ID), _T("PNG"));	// 3번째 인자는 실제 리소스가 분류되는 문자열이 전달되어야 한다.
			if (!hResource) return false;

			DWORD imageSize = ::SizeofResource(AfxGetApp()->m_hInstance, hResource);
			if (!imageSize) return false;

			const void* pResourceData = ::LockResource(::LoadResource(AfxGetApp()->m_hInstance, hResource));
			if (!pResourceData) return false;

			HGLOBAL hBuffer  = ::GlobalAlloc(GMEM_MOVEABLE, imageSize);
			if (hBuffer)
			{
				void* pBuffer = ::GlobalLock(hBuffer);
				if (pBuffer)
				{
					CopyMemory(pBuffer, pResourceData, imageSize);

					IStream* pStream = NULL;
					if (::CreateStreamOnHGlobal(hBuffer, FALSE, &pStream) == S_OK)
					{
						if (i==0)
						{
							m_pBmpCheck = Gdiplus::Bitmap::FromStream(pStream);
							pStream->Release();

							if (m_pBmpCheck)
							{
								if (m_pBmpCheck->GetLastStatus() != Gdiplus::Ok)
								{
									delete m_pBmpCheck;
									m_pBmpCheck = NULL;
								}
							}
						}
						else
						{
							m_pBmpUnCheck = Gdiplus::Bitmap::FromStream(pStream);
							pStream->Release();

							if (m_pBmpUnCheck)
							{
								if (m_pBmpUnCheck->GetLastStatus() != Gdiplus::Ok)
								{
									delete m_pBmpUnCheck;
									m_pBmpUnCheck = NULL;
								}
							}
						}
					}
				}
				::GlobalUnlock(hBuffer);
				::GlobalFree(hBuffer);
			}
		}
	}

	Invalidate();

	return true;
}

bool CRadioEx::IsFileExist(CString strPath)
{
	HANDLE hFind;
	WIN32_FIND_DATA fd;
	if ((hFind = ::FindFirstFile(strPath, &fd)) == INVALID_HANDLE_VALUE) 
	{
		FindClose(hFind);
		return false;
	}

	return true;
}

void CRadioEx::SetSizeCheckBox(int nLeft, int nTop, int nWidth, int nHeight)
{ 
	CRect rect;
	GetClientRect(&rect);

	m_rcCheckBox = Rect(nLeft, nTop, nWidth, nHeight); 
	m_rcCaption  = Rect(nLeft+nWidth, rect.top, rect.Width()-nWidth, rect.Height());
	m_rcImage	 = m_rcCaption;

	Invalidate(); 
}

void CRadioEx::SetColorBkg(int nA, COLORREF clrColor)
{
	int r = GetRValue(clrColor);
	int g = GetGValue(clrColor);
	int b = GetBValue(clrColor);

	m_clrBkg = Color(nA, r, g, b); 
	Invalidate();
}
void CRadioEx::SetColorBorder(int nA, COLORREF clrColor)
{
	int r = GetRValue(clrColor);
	int g = GetGValue(clrColor);
	int b = GetBValue(clrColor);

	m_clrBorder = Color(nA, r, g, b); 
	Invalidate();
}
void CRadioEx::SetColorBoxBkg(int nA, COLORREF clrColor)
{
	int r = GetRValue(clrColor);
	int g = GetGValue(clrColor);
	int b = GetBValue(clrColor);

	m_clrBoxBkg = Color(nA, r, g, b); 
	Invalidate();
}
void CRadioEx::SetColorBoxBorder(int nA, COLORREF clrColor)
{
	int r = GetRValue(clrColor);
	int g = GetGValue(clrColor);
	int b = GetBValue(clrColor);

	m_clrBoxBorder = Color(nA, r, g, b); 
	Invalidate();
}
void CRadioEx::SetColorCheck(int nA, COLORREF clrColor)
{
	int r = GetRValue(clrColor);
	int g = GetGValue(clrColor);
	int b = GetBValue(clrColor);

	m_clrCheck = Color(nA, r, g, b); 
	Invalidate();
}
void CRadioEx::SetColorText(int nA, COLORREF clrColor)
{
	int r = GetRValue(clrColor);
	int g = GetGValue(clrColor);
	int b = GetBValue(clrColor);

	m_clrText = Color(nA, r, g, b); 
	Invalidate();
}

void CRadioEx::OnMouseLeave()
{
	m_bMouseTrack = FALSE;

	CButton::OnMouseLeave();
}


void CRadioEx::OnMouseMove(UINT nFlags, CPoint point)
{
	if(!m_bMouseTrack)
	{
		TRACKMOUSEEVENT tme = {0};
		tme.cbSize		= sizeof(tme);
		tme.hwndTrack	= m_hWnd;
		tme.dwFlags		= TME_LEAVE;
		m_bMouseTrack = ::_TrackMouseEvent(&tme);
	}

	CButton::OnMouseMove(nFlags, point);
}
