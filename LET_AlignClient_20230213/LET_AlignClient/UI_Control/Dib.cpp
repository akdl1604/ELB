#pragma once
#include "StdAfx.h"
#include "Dib.h"
#include "RGBBYTE.h"

/***************************************************************************************
*
*	이름: CDib()
*	인자: 없음
*	반환: 없음
*	설명: 기본 생성자
*		   멤버 변수 초기화
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
*	이름: CDib(const CDib& dib)
*	인자: dib - 복사할 CDib 객체
*	반환: 없음
*	설명: 복사 생성자
*		   인자로 넘어온 dib 객체를 복사
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
*	이름: ~CDib()
*	인자: 없음
*	반환: 없음
*	설명: 소멸자
*		   Dib 객체를 해제
*
/***************************************************************************************/
CDib::~CDib(void)
{
	Destroy();
}



/***************************************************************************************
*
*	이름: CreateGrayImage(int nWidth, int nHeight, BYTE value)
*	인자: nWidth	-	생성할 DIB의 가로 픽셀
*		   hHeight	-	생성할 DIB의 세로 픽셀
*		   value	-	픽셀 초기값, 디폴트 값은 255
*	반환: 성공하면 TRUE, 실패하면 FALSE 반환
*	설명: 지정한 크기의 그레일스케일 영상을 생성
*		   만약, 현재 DIB 객체를 가지고 있다면 삭제 후 생성
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
*	이름: CreateRGBImage(int nWidth, int nHeight, COLORREF value)
*	인자: nWidth	-	생성할 DIB의 가로 픽셀
*		   hHeight	-	생성할 DIB의 세로 픽셀
*		   value	-	픽셀 초기값, 디폴트 값은 RGB(255, 255, 255)
*	반환: 성공하면 TRUE, 실패하면 FALSE 반환
*	설명: 지정한 크기의 트루컬러 영상을 생성
*		   만약, 현재 DIB 객체를 가지고 있다면 삭제 후 생성
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
*	이름: InitDib()
*	인자: 없음
*	반환: 성공하면 TRUE, 실패하면 FALSE 반환
*	설명: m_nWidth, m_nHeight, m_nBitCount등의 멤버 변수 값을 설정
*		   m_pDib가 BITMAPINFOHEADER의 시작 주소를 가리키고 있는 상황에서 호출
*		   m_pGrayPtr, m_pRgbPtr interface를 생성
*
/***************************************************************************************/

BOOL CDib::InitDib(){

	LPBITMAPINFOHEADER lpbmi = (LPBITMAPINFOHEADER)m_pDib;

	m_nWidth	= lpbmi->biWidth;
	m_nHeight	= lpbmi->biHeight;
	m_nBitCount	= lpbmi->biBitCount;

	DWORD dwSizeImage = m_nHeight*(DWORD)((m_nWidth*m_nBitCount/8+3)&~3);

	m_nDibSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*GetPaletteNums() + dwSizeImage;

	//m_pGrayPtr과 m_pRgbPtr 멤버 변수 선언
	if(!AllocPtr()) return FALSE;

	return TRUE;
	
}


/***************************************************************************************
*
*	이름: Destroy()
*	인자: 없음
*	반환: 없음
*	설명: 동적할당된 모든 데이터를 해제하고, NULL로 설정한다
*		   기타 멤버 변수도 초기화
/***************************************************************************************/
void CDib::Destroy(){
	if(m_pDib){
		delete [] m_pDib;
		m_pDib = NULL;
	}

	FreePtr();	//m_pGrayPtr과 m_pRgbPtr 변수를 소멸

	m_nWidth	= 0;
	m_nHeight	= 0;
	m_nBitCount	= 0;
	m_nDibSize	= 0;
}


/***************************************************************************************
*
*	이름: operator=(const CDib& dib)
*	인자: pDib - 복사할 CDib 객체
*	반환: CDib 객체의 참조형
*	설명: 인자로 들어온 dib와 동일한 CDib 객체를 생성
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
*	이름: CopyImage(CDib* pImage)
*	인자: pDib - 복사할 CDib 객체 포인터
*	반환: 성공하면 TRUE, 실패하면 FALSE 반환
*	설명: 인자로 넘어온 pDib가 가리키는 CDib 객체를 복사한다.
*
/***************************************************************************************/
BOOL CDib::Copy(CDib* pDib)
{
	// pDib가 비어있는 CDib 객체를 가리키고 있다면 자신도 비운다.
	if( !pDib->Valid() )
	{
		Destroy();
		return TRUE;
	}

	// 현재 설정된 CDib 객체가 있다면 삭제한다.
	if( m_pDib ) Destroy();

	// DIB를 담을 메모리 공간 할당
	DWORD dwDibSize = pDib->GetDibSize();
	m_pDib = new BYTE[dwDibSize];
	if( m_pDib == NULL )
		return FALSE;

	// DIB 내용 복사
	BYTE* pbi = (BYTE*)pDib->GetBitmapInfoAddr();
	memcpy(m_pDib, pbi, dwDibSize);

	// 멤버 변수 값 설정
	InitDib();

	return TRUE;
}



/***************************************************************************************
*
*	이름: Draw(HDC hDC)
*	인자: hDC - 출력 장치의 DC 핸들
*	반환: 없음
*	설명: 출력 장치 좌표 (0, 0) 위치에 DIB를 출력한다
*
/***************************************************************************************/
void CDib::Draw(HDC hDC){

	Draw(hDC, 0, 0);

}


/***************************************************************************************
*
*	이름: Draw(HDC hDC, int dx, int dy)
*	인자: hDC - 출력 장치의 DC 핸들
*		   dx - 출력 장치 위치의 x 좌표
*		   dy - 출력 장치 위치의 y 좌표
*	반환: 없음
*   설명: 출력 장치 좌표 (dx, dy) 위치에 DIB를 출력한다
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
*	이름: Draw(HDC hDC, int dx, int dy, int dw, int dh, DWORD dwRop = SRCCOPY)
*	인자: hDC - 출력 장치의 DC 핸들
*		   dx - 출력할 위치의 x 좌표
*		   dy - 출력할 위치의 y 좌표
*		   dw - 출력할 공간의 가로 크기
*		   dh - 출력할 공간의 세로 크기
*		   dwRop - 래스터 오퍼레이션 지정
*	반환: 없음
*	설명: 출력 장치 좌표 (dx, dy) 위치에 (dw, dh) 크기로 DIB를 출력한다
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
*	이름: Draw(HDC hDC, int dx, int dy, int dw, int dh, int sx, int sy, int sw, int sh, DWORD dwRop = SRCCOPY)
*	인자: hDC - 출력 장치의 DC 핸들
*		   dx - 출력할 위치의 x 좌표
*		   dy - 출력할 위치의 y 좌표
*		   dw - 출력할 공간의 가로 크기
*		   dh - 출력할 공간의 세로 크기
*		   sx - 원본 영상의 좌상귀 x 좌표
*		   sy - 원본 영상의 좌상귀 y 좌표
*		   sw - 원본 영상에서 출력할 영역의 가로 크기
*		   sh - 원본 영상에서 출력할 영역의 세로 크기
*		   dwRop - 래스터 오퍼레이션 지정
*	반환: 없음
*	설명: 출력 장치 좌표 (dx, dy) 위치에 (dw, dh) 크기로 영상좌표 (sx, sy)에서 크기 (sw, sh)만큼만을
*		   부분적으로 출력한다
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
*	이름: LoadImage(LPCTSTR lpszFileName)
*	인자: lpszFileName - 불러올 파일의 전체 경로 이름
*	반환: 성공하면 TRUE, 실패하면 FALSE를 반환
*	설명: BMP 파일을 CDib 객체로 불러온다
*		   확장자를 검사하여 BMP 파일이면 LoadBMP 함수를 다시 호출한다
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
*	이름: SaveImage(LPCTSTR lpszFileName)
*	인자: lpszFileName - 저장할 파일의 전체 경로 이름
*	반환: 성공하면 TRUE, 실패하면 FALSE 반환
*	설명: CDib 객체를 BMP 파일로 저장한다
*		   확장자를 검사하여 BMP 파일이면 SaveBMP 함수를 다시 호출한다
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
*	이름: GetPaletteNums()
*	인자: 없음
*	반환: 팔레트에서 RGBQUAD 배열의 개수
*	설명: 팔레트에서 필요한 RGBQUAD 구조체의 배열의 개수를 반환
*		   TrueColor(RGB)이면 0을 반환
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
* 이름: GetDibBitsAddr()
* 인자: 없음
* 반환: 픽셀 데이터의 시작 주소를 반환
* 설명: m_pDib을 참고하여 픽셀 데이터의 시작 주소를 받는다
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
*	이름: AllocPtr()
*	인자: 없음
*	반환: 성공하면 TRUE, 실패하면 FALSE 반환
*	설명: m_pGrayPtr, m_pRgbPtr interface를 생성
*		   픽셀 데이터가 botton-up 형태로 저장되어 있다고 가정
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
*	이름: FreePtr()
*	인자: 없음
*	반환: 없음
*	설명: m_pGrayPtr, m_pRgbPtr interface를 제거한다
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
*	이름: CopyToClipboard()
*	인자: 없음
*	반환: 성공하면 TRUE, 실패하면 FALSE
*	설명: DIB 내용을 clipboard로 복사 
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
*	이름: PasteFromClipboard()
*	인자: 없음
*	반환: 성공하면 TRUE, 실패하면 FALSE
*	설명: Clipboard로부터 DIB 내용을 가져온다
*
/***************************************************************************************/
BOOL CDib::PasteFromClipboard(){
	
	HANDLE hDib;
	DWORD dwSize;
	void* lpDib;

	//CF_DIB Type이 아니면 종료
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

