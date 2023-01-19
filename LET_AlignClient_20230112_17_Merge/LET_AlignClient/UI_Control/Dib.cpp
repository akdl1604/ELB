#pragma once
#include "StdAfx.h"
#include "Dib.h"
#include "RGBBYTE.h"

/***************************************************************************************
*
*	�̸�: CDib()
*	����: ����
*	��ȯ: ����
*	����: �⺻ ������
*		   ��� ���� �ʱ�ȭ
*
/***************************************************************************************/
CDib::CDib(void)
{
	m_nWidth	= 0;
	m_nHeight	= 0;
	m_nBitCount	= 0;
	m_nDibSize	= 0;

	m_pDib		= NULL;

	m_pGrayPtr	= NULL;
	m_pRgbPtr	= NULL;
}



/***************************************************************************************
*
*	�̸�: CDib(const CDib& dib)
*	����: dib - ������ CDib ��ü
*	��ȯ: ����
*	����: ���� ������
*		   ���ڷ� �Ѿ�� dib ��ü�� ����
*
/***************************************************************************************/
CDib::CDib(const CDib& dib)
{
	m_nWidth	= 0;
	m_nHeight	= 0;
	m_nBitCount	= 0;
	m_nDibSize	= 0;

	m_pDib		= NULL;

	m_pGrayPtr	= NULL;
	m_pRgbPtr	= NULL;

	if(dib.m_pDib != NULL){
		m_pDib = new BYTE[dib.m_nDibSize];
		memcpy(m_pDib, dib.m_pDib, dib.m_nDibSize);


	InitDib();

	}
}



/***************************************************************************************
*
*	�̸�: ~CDib()
*	����: ����
*	��ȯ: ����
*	����: �Ҹ���
*		   Dib ��ü�� ����
*
/***************************************************************************************/
CDib::~CDib(void)
{
	Destroy();
}



/***************************************************************************************
*
*	�̸�: CreateGrayImage(int nWidth, int nHeight, BYTE value)
*	����: nWidth	-	������ DIB�� ���� �ȼ�
*		   hHeight	-	������ DIB�� ���� �ȼ�
*		   value	-	�ȼ� �ʱⰪ, ����Ʈ ���� 255
*	��ȯ: �����ϸ� TRUE, �����ϸ� FALSE ��ȯ
*	����: ������ ũ���� �׷��Ͻ����� ������ ����
*		   ����, ���� DIB ��ü�� ������ �ִٸ� ���� �� ����
*
/***************************************************************************************/

BOOL CDib::CreateGrayImage(int nWidth, int nHeight, BYTE value) {
	
	if(m_pDib) Destroy();

	int nBitCount = 8;
	DWORD dwSizeImage = nHeight * (DWORD)((nWidth*nBitCount/8+3)&~3);

	m_pDib = new BYTE[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*(1i64<<nBitCount) + dwSizeImage];
	if(m_pDib==NULL) return FALSE;

	LPBITMAPINFOHEADER lpbmi = (LPBITMAPINFOHEADER)m_pDib;

	lpbmi->biSize			= sizeof(BITMAPINFOHEADER);
	lpbmi->biWidth			= nWidth;
	lpbmi->biHeight			= nHeight;
	lpbmi->biPlanes			= 1;
	lpbmi->biBitCount		= (WORD)nBitCount;
	lpbmi->biCompression	= BI_RGB;
	lpbmi->biSizeImage		= dwSizeImage;
	lpbmi->biXPelsPerMeter	= 0;
	lpbmi->biYPelsPerMeter	= 0;
	lpbmi->biClrUsed		= 0;
	lpbmi->biClrImportant	= 0;

	RGBQUAD* pPal = (RGBQUAD*)((BYTE*)m_pDib + sizeof(BITMAPINFOHEADER));
	for(int i = 0; i < 256; i++){

		pPal->rgbBlue		= (BYTE)i;
		pPal->rgbGreen		= (BYTE)i;
		pPal->rgbRed		= (BYTE)i;
		pPal->rgbReserved	= 0;

		pPal++;
	}

	InitDib();

	BYTE* pData = (BYTE*)GetDibBitsAddr();
	memset(pData, value, dwSizeImage);

	return TRUE;
}



/***************************************************************************************
*
*	�̸�: CreateRGBImage(int nWidth, int nHeight, COLORREF value)
*	����: nWidth	-	������ DIB�� ���� �ȼ�
*		   hHeight	-	������ DIB�� ���� �ȼ�
*		   value	-	�ȼ� �ʱⰪ, ����Ʈ ���� RGB(255, 255, 255)
*	��ȯ: �����ϸ� TRUE, �����ϸ� FALSE ��ȯ
*	����: ������ ũ���� Ʈ���÷� ������ ����
*		   ����, ���� DIB ��ü�� ������ �ִٸ� ���� �� ����
*
/***************************************************************************************/

BOOL CDib::CreateRGBImage(int nWidth, int nHeight, COLORREF value){
	if(m_pDib) Destroy();

	int nBitCount = 24;
	DWORD dwSizeImage = nHeight * (DWORD)((nWidth*nBitCount/8+3)&~3);

	m_pDib = new BYTE[sizeof(BITMAPINFOHEADER) + dwSizeImage];
	if(m_pDib==NULL) return FALSE;

	LPBITMAPINFOHEADER lpbmi = (LPBITMAPINFOHEADER)m_pDib;

	lpbmi->biSize			= sizeof(BITMAPINFOHEADER);
	lpbmi->biWidth			= nWidth;
	lpbmi->biHeight			= nHeight;
	lpbmi->biPlanes			= 1;
	lpbmi->biBitCount		=(WORD) nBitCount;
	lpbmi->biCompression	= BI_RGB;
	lpbmi->biSizeImage		= dwSizeImage;
	lpbmi->biXPelsPerMeter	= 0;
	lpbmi->biYPelsPerMeter	= 0;
	lpbmi->biClrUsed		= 0;
	lpbmi->biClrImportant	= 0;

	InitDib();

	for(int j = 0; j < m_nHeight; j++)
	for(int i = 0; i < m_nWidth ; i++){

		m_pRgbPtr[j][i] = value;
	}

	return TRUE;
}



/***************************************************************************************
*
*	�̸�: InitDib()
*	����: ����
*	��ȯ: �����ϸ� TRUE, �����ϸ� FALSE ��ȯ
*	����: m_nWidth, m_nHeight, m_nBitCount���� ��� ���� ���� ����
*		   m_pDib�� BITMAPINFOHEADER�� ���� �ּҸ� ����Ű�� �ִ� ��Ȳ���� ȣ��
*		   m_pGrayPtr, m_pRgbPtr interface�� ����
*
/***************************************************************************************/

BOOL CDib::InitDib(){

	LPBITMAPINFOHEADER lpbmi = (LPBITMAPINFOHEADER)m_pDib;

	m_nWidth	= lpbmi->biWidth;
	m_nHeight	= lpbmi->biHeight;
	m_nBitCount	= lpbmi->biBitCount;

	DWORD dwSizeImage = m_nHeight*(DWORD)((m_nWidth*m_nBitCount/8+3)&~3);

	m_nDibSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*GetPaletteNums() + dwSizeImage;

	//m_pGrayPtr�� m_pRgbPtr ��� ���� ����
	if(!AllocPtr()) return FALSE;

	return TRUE;
	
}


/***************************************************************************************
*
*	�̸�: Destroy()
*	����: ����
*	��ȯ: ����
*	����: �����Ҵ�� ��� �����͸� �����ϰ�, NULL�� �����Ѵ�
*		   ��Ÿ ��� ������ �ʱ�ȭ
/***************************************************************************************/
void CDib::Destroy(){
	if(m_pDib){
		delete [] m_pDib;
		m_pDib = NULL;
	}

	FreePtr();	//m_pGrayPtr�� m_pRgbPtr ������ �Ҹ�

	m_nWidth	= 0;
	m_nHeight	= 0;
	m_nBitCount	= 0;
	m_nDibSize	= 0;
}


/***************************************************************************************
*
*	�̸�: operator=(const CDib& dib)
*	����: pDib - ������ CDib ��ü
*	��ȯ: CDib ��ü�� ������
*	����: ���ڷ� ���� dib�� ������ CDib ��ü�� ����
*
/***************************************************************************************/
CDib& CDib::operator=(const CDib& dib){

	if(this == &dib) return *this;

	if(dib.m_pDib == NULL){
		Destroy();
		return *this;
	}

	if(m_pDib) Destroy();
	
	m_pDib = new BYTE[dib.m_nDibSize];

	memcpy(m_pDib, dib.m_pDib, dib.m_nDibSize);

	InitDib();

	return *this;
}



/***************************************************************************************
*
*	�̸�: CopyImage(CDib* pImage)
*	����: pDib - ������ CDib ��ü ������
*	��ȯ: �����ϸ� TRUE, �����ϸ� FALSE ��ȯ
*	����: ���ڷ� �Ѿ�� pDib�� ����Ű�� CDib ��ü�� �����Ѵ�.
*
/***************************************************************************************/
BOOL CDib::Copy(CDib* pDib)
{
	// pDib�� ����ִ� CDib ��ü�� ����Ű�� �ִٸ� �ڽŵ� ����.
	if( !pDib->Valid() )
	{
		Destroy();
		return TRUE;
	}

	// ���� ������ CDib ��ü�� �ִٸ� �����Ѵ�.
	if( m_pDib ) Destroy();

	// DIB�� ���� �޸� ���� �Ҵ�
	DWORD dwDibSize = pDib->GetDibSize();
	m_pDib = new BYTE[dwDibSize];
	if( m_pDib == NULL )
		return FALSE;

	// DIB ���� ����
	BYTE* pbi = (BYTE*)pDib->GetBitmapInfoAddr();
	memcpy(m_pDib, pbi, dwDibSize);

	// ��� ���� �� ����
	InitDib();

	return TRUE;
}



/***************************************************************************************
*
*	�̸�: Draw(HDC hDC)
*	����: hDC - ��� ��ġ�� DC �ڵ�
*	��ȯ: ����
*	����: ��� ��ġ ��ǥ (0, 0) ��ġ�� DIB�� ����Ѵ�
*
/***************************************************************************************/
void CDib::Draw(HDC hDC){

	Draw(hDC, 0, 0);

}


/***************************************************************************************
*
*	�̸�: Draw(HDC hDC, int dx, int dy)
*	����: hDC - ��� ��ġ�� DC �ڵ�
*		   dx - ��� ��ġ ��ġ�� x ��ǥ
*		   dy - ��� ��ġ ��ġ�� y ��ǥ
*	��ȯ: ����
*   ����: ��� ��ġ ��ǥ (dx, dy) ��ġ�� DIB�� ����Ѵ�
*
/***************************************************************************************/

void CDib::Draw(HDC hDC, int dx, int dy){

	if(m_pDib == NULL) return;

	LPBITMAPINFO lpbi = (LPBITMAPINFO)m_pDib;
	void* lpBits = (void*)GetDibBitsAddr();

	::SetDIBitsToDevice(
		hDC,
		dx,
		dy,
		m_nWidth,
		m_nHeight,
		0,
		0,
		0,
		m_nHeight,
		lpBits,
		lpbi,
		DIB_RGB_COLORS);
}



/***************************************************************************************
*
*	�̸�: Draw(HDC hDC, int dx, int dy, int dw, int dh, DWORD dwRop = SRCCOPY)
*	����: hDC - ��� ��ġ�� DC �ڵ�
*		   dx - ����� ��ġ�� x ��ǥ
*		   dy - ����� ��ġ�� y ��ǥ
*		   dw - ����� ������ ���� ũ��
*		   dh - ����� ������ ���� ũ��
*		   dwRop - ������ ���۷��̼� ����
*	��ȯ: ����
*	����: ��� ��ġ ��ǥ (dx, dy) ��ġ�� (dw, dh) ũ��� DIB�� ����Ѵ�
*
/***************************************************************************************/
void CDib::Draw(HDC hDC, int dx, int dy, int dw, int dh){

	if(m_pDib == NULL) return;

	LPBITMAPINFO lpbi = (LPBITMAPINFO)m_pDib;
	void* lpBits = (void*)GetDibBitsAddr();

	::StretchDIBits(
		hDC,
		dx,
		dy,
		dw,
		dh,
		0,
		0,
		m_nWidth,
		m_nHeight,
		lpBits,
		lpbi,
		DIB_RGB_COLORS,
		SRCCOPY);

}

/***************************************************************************************
*
*	�̸�: Draw(HDC hDC, int dx, int dy, int dw, int dh, int sx, int sy, int sw, int sh, DWORD dwRop = SRCCOPY)
*	����: hDC - ��� ��ġ�� DC �ڵ�
*		   dx - ����� ��ġ�� x ��ǥ
*		   dy - ����� ��ġ�� y ��ǥ
*		   dw - ����� ������ ���� ũ��
*		   dh - ����� ������ ���� ũ��
*		   sx - ���� ������ �»�� x ��ǥ
*		   sy - ���� ������ �»�� y ��ǥ
*		   sw - ���� ���󿡼� ����� ������ ���� ũ��
*		   sh - ���� ���󿡼� ����� ������ ���� ũ��
*		   dwRop - ������ ���۷��̼� ����
*	��ȯ: ����
*	����: ��� ��ġ ��ǥ (dx, dy) ��ġ�� (dw, dh) ũ��� ������ǥ (sx, sy)���� ũ�� (sw, sh)��ŭ����
*		   �κ������� ����Ѵ�
*
/***************************************************************************************/
void CDib::Draw(HDC hDC, int dx, int dy, int dw, int dh, int sx, int sy, int sw, int sh){

	if(m_pDib == NULL) return;

	LPBITMAPINFO lpbi = (LPBITMAPINFO)m_pDib;
	void* lpBits = (void*)GetDibBitsAddr();

	::StretchDIBits(
		hDC,
		dx,
		dy,
		dw,
		dh,
		sx,
		sy,
		sw,
		sh,
		lpBits,
		lpbi,
		DIB_RGB_COLORS,
		SRCCOPY);

}



/***************************************************************************************
*
*	�̸�: LoadImage(LPCTSTR lpszFileName)
*	����: lpszFileName - �ҷ��� ������ ��ü ��� �̸�
*	��ȯ: �����ϸ� TRUE, �����ϸ� FALSE�� ��ȯ
*	����: BMP ������ CDib ��ü�� �ҷ��´�
*		   Ȯ���ڸ� �˻��Ͽ� BMP �����̸� LoadBMP �Լ��� �ٽ� ȣ���Ѵ�
*
/***************************************************************************************/
BOOL CDib::LoadImage(LPCTSTR lpszFileName){

	CString filetype;
	filetype = lpszFileName;
	filetype.MakeUpper();

	if(filetype.Right(4) == ".BMP" || filetype.Right(4) == ".bmp")
	return LoadBMP(lpszFileName);
	else
		return FALSE;

//	return TRUE;

}





/***************************************************************************************
*
*	�̸�: SaveImage(LPCTSTR lpszFileName)
*	����: lpszFileName - ������ ������ ��ü ��� �̸�
*	��ȯ: �����ϸ� TRUE, �����ϸ� FALSE ��ȯ
*	����: CDib ��ü�� BMP ���Ϸ� �����Ѵ�
*		   Ȯ���ڸ� �˻��Ͽ� BMP �����̸� SaveBMP �Լ��� �ٽ� ȣ���Ѵ�
*
/***************************************************************************************/
BOOL CDib::SaveImage(LPCTSTR lpszFileName){

	CString filetype;
	filetype = lpszFileName;
	filetype.MakeUpper();

	if(filetype.Right(4) == ".BMP" || filetype.Right(4) == ".bmp")
	return SaveBMP(lpszFileName);
	else
		return FALSE;

	//return TRUE;
}



/***************************************************************************************
*
*	�̸�: GetPaletteNums()
*	����: ����
*	��ȯ: �ȷ�Ʈ���� RGBQUAD �迭�� ����
*	����: �ȷ�Ʈ���� �ʿ��� RGBQUAD ����ü�� �迭�� ������ ��ȯ
*		   TrueColor(RGB)�̸� 0�� ��ȯ
*
/***************************************************************************************/
int CDib::GetPaletteNums(){

	switch(m_nBitCount){
	case 1:		return 2;
	case 4:		return 16;
	case 8:		return 256;

	default:	return 0;
	}
}



/***************************************************************************************
*
* �̸�: GetDibBitsAddr()
* ����: ����
* ��ȯ: �ȼ� �������� ���� �ּҸ� ��ȯ
* ����: m_pDib�� �����Ͽ� �ȼ� �������� ���� �ּҸ� �޴´�
*
/***************************************************************************************/
LPVOID CDib::GetDibBitsAddr(){
	
	LPBITMAPINFOHEADER lpbmi;
	LPVOID lpDibBits;

	lpbmi = (LPBITMAPINFOHEADER)m_pDib;
	lpDibBits = (LPVOID)((BYTE*)m_pDib + lpbmi->biSize + sizeof(RGBQUAD)*GetPaletteNums());

	return lpDibBits;
}



/***************************************************************************************
*
*	�̸�: AllocPtr()
*	����: ����
*	��ȯ: �����ϸ� TRUE, �����ϸ� FALSE ��ȯ
*	����: m_pGrayPtr, m_pRgbPtr interface�� ����
*		   �ȼ� �����Ͱ� botton-up ���·� ����Ǿ� �ִٰ� ����
*
/***************************************************************************************/

BOOL CDib::AllocPtr(){
	
	if(m_nBitCount == 8){
		if(m_pGrayPtr) FreePtr();
		m_pGrayPtr = new BYTE*[m_nHeight];
		if(!m_pGrayPtr) return FALSE;

		int nWidthStep = ((m_nWidth*m_nBitCount/8+3)&~3);
		BYTE* pData = (BYTE*)GetDibBitsAddr();

		for(int i = 0; i < m_nHeight; i++)
			m_pGrayPtr[i] = (BYTE*)(pData + (m_nHeight-i-1)*nWidthStep);
	}

	else if(m_nBitCount == 24){
		if(m_pRgbPtr) FreePtr();
		m_pRgbPtr = new RGBBYTE*[m_nHeight];
		if(!m_pRgbPtr) return FALSE;

		int nWidthStep = ((m_nWidth*m_nBitCount/8+3)&~3);
		BYTE* pData = (BYTE*)GetDibBitsAddr();
		
		for(int i = 0; i < m_nHeight; i++)
			m_pRgbPtr[i] = (RGBBYTE*)(pData + (m_nHeight-i-1)*nWidthStep);
		}

	else return FALSE;

	return TRUE;


}



/***************************************************************************************
*
*	�̸�: FreePtr()
*	����: ����
*	��ȯ: ����
*	����: m_pGrayPtr, m_pRgbPtr interface�� �����Ѵ�
*
/***************************************************************************************/
void CDib::FreePtr(){
	if(m_pGrayPtr != NULL){
		delete [] m_pGrayPtr;
		m_pGrayPtr = NULL;
	}

	if(m_pRgbPtr != NULL){
		delete [] m_pRgbPtr;
		m_pRgbPtr = NULL;
	}
}




/***************************************************************************************
*
*	�̸�: CopyToClipboard()
*	����: ����
*	��ȯ: �����ϸ� TRUE, �����ϸ� FALSE
*	����: DIB ������ clipboard�� ���� 
*
/***************************************************************************************/
BOOL CDib::CopyToClipboard() {
	
	int dwSizeDib;
	HANDLE hDib;

	if(!::OpenClipboard(NULL)) return FALSE;

	dwSizeDib = GetDibSize();
	hDib = ::GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, dwSizeDib);

	if(hDib == NULL) {
		::CloseClipboard();
		return FALSE;
	}

	void* lpDib = ::GlobalLock((HGLOBAL)hDib);
	memcpy(lpDib, GetBitmapInfoAddr(), dwSizeDib);
	::GlobalUnlock(hDib);

	return TRUE;
}




/***************************************************************************************
*
*	�̸�: PasteFromClipboard()
*	����: ����
*	��ȯ: �����ϸ� TRUE, �����ϸ� FALSE
*	����: Clipboard�κ��� DIB ������ �����´�
*
/***************************************************************************************/
BOOL CDib::PasteFromClipboard(){
	
	HANDLE hDib;
	DWORD dwSize;
	void* lpDib;

	//CF_DIB Type�� �ƴϸ� ����
	if(!::IsClipboardFormatAvailable(CF_DIB)) return FALSE;

	//Open the Clipboard
	if(!::OpenClipboard(NULL)) return FALSE;

	//Get Information in the Clipboard
	hDib = ::GetClipboardData(CF_DIB);
	
	if(hDib == NULL){
		::CloseClipboard();
		return FALSE;
	}

	//Size of Memory Block is Same of Entire Size of the DIB
	dwSize = (DWORD)::GlobalSize((HGLOBAL)hDib);
	lpDib = ::GlobalLock((HGLOBAL)hDib);

	LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)lpDib;
	LONG nWidth		= lpbi->biWidth;
	LONG nHeight	= lpbi->biHeight;
	WORD nBitCount	= lpbi->biBitCount;
	DWORD dwSizeDib;

	DWORD dwSizeImage = nHeight * (DWORD)((nWidth*nBitCount/8+3)&~3);

	if(nBitCount == 8)
		dwSizeDib = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*(1i64<<nBitCount) + dwSizeImage;
	else
		dwSizeDib = sizeof(BITMAPINFOHEADER) + dwSizeImage;


	//Delete CDIB Object if CDIB is full of Information
	if(m_pDib) Destroy();

	m_pDib = new BYTE[dwSizeDib];
	memcpy(m_pDib, lpDib, dwSizeDib);

	::GlobalUnlock(hDib);
	::CloseClipboard();

	
	//Set Initial Member Value
	InitDib();

	return TRUE;

}

