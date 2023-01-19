/*
	Dib.h: CDib 클래스의 헤더파일

	Last modified: 04/10/2011
			
									*/

#pragma once

class RGBBYTE;

class CDib
{
//-----------------------------------------------------------
// member value set
//-----------------------------------------------------------
protected:
	LONG		m_nWidth;		// 이미지의 가로 크기
	LONG		m_nHeight;		// 이미지의 세로 크기
	WORD		m_nBitCount;	// 픽셀당 비트수(Gray = 8, RGB = 24)
	DWORD		m_nDibSize;		// sizeof(BITMAPINFOHEADER + RGBQUAD(GrayImage만 해당) + 픽셀데이터)

	BYTE*		m_pDib;			// DIB내용을 담고 있는 버퍼의 시작 주소

	BYTE**		m_pGrayPtr;		// GrayImage에서 2D 픽셀 접근용 포인터
	RGBBYTE**	m_pRgbPtr;		// TruecolorImage에서 2D 픽셀 접근용 포인터


//-----------------------------------------------------------
// member function set
//-----------------------------------------------------------
public:
	//생성자 & 소멸자
	CDib(void);
	CDib(const CDib& dib);
	~CDib(void);


	//이미지 생성 & 소멸 & 복사
	BOOL Copy(CDib* pDib);
	BOOL CreateGrayImage(int nWidth, int nHeight, BYTE value = 255);
	BOOL CreateRGBImage(int nWIdth, int nHeight, COLORREF value = 0x00ffffff);
	void Destroy();

	//치환 연산자 오버로딩과 이미지 복사
	CDib& operator=(const CDib& dib);
	BOOL CopyImage(CDib* pImage);

	// 이미지 화면 출력
	void Draw(HDC hDC);
	void Draw(HDC hDC, int dx, int dy);
	void Draw(HDC hDC, int dx, int dy, int dw, int dh);
	void Draw(HDC hDC, int dx, int dy, int dw, int dh, int sx, int sy, int sw, int sh);

	// 파일 입출력
	BOOL LoadImage(LPCTSTR lpszFileName);
	BOOL SaveImage(LPCTSTR lpszFileName);

	// BMP 파일 불러오기/ 저장하기
	BOOL LoadBMP(LPCTSTR lpszFileName);
	BOOL SaveBMP(LPCTSTR lpszFileName);

	//Clipboard로 복사하기 또는 붙여넣기
	BOOL	CopyToClipboard();
	BOOL	PasteFromClipboard();
	//2D 픽셀 포인터 반환
	BYTE**		GetGrayPtr();
	RGBBYTE**	GetRgbPtr();
	BYTE*	GetDibPtr(){	return m_pDib;		}

	//팔레트에서 RGBQUAD 배열의 갯수를 반환
	int	GetPaletteNums();

	//픽셀 데이터 시작 포인터 반환
	LPVOID GetDibBitsAddr();

	//이미지 정보 반환 함수
	LONG	GetHeight()		{	return m_nHeight;		}
	LONG	GetWidth()		{	return m_nWidth;		}
	LONG	GetBitCount()	{	return m_nBitCount;		}
	LONG	GetDibSize()	{	return m_nDibSize;		}
	LONG	Valid()			{	return (m_pDib != NULL);}

	// BITMAPINFOHEADER 시작 포인터 반환
	LPBITMAPINFO GetBitmapInfoAddr() { return (LPBITMAPINFO)m_pDib; }

protected:
	//m_pDib 설정 후, Image 정보 설정 함수
	BOOL InitDib();

	//2D 픽셀 포인터 접근을 위한 interface 생성 & 소멸
	BOOL AllocPtr();
	void FreePtr();

};


inline BYTE** CDib::GetGrayPtr(){
		ASSERT(m_nBitCount == 8);
		ASSERT(m_pGrayPtr != NULL);

		return m_pGrayPtr;
	}


inline RGBBYTE** CDib::GetRgbPtr(){
		ASSERT(m_nBitCount == 24);
		ASSERT(m_pRgbPtr != NULL);

		return m_pRgbPtr;
	}







// Template Function Declaire


/***************************************************************************************
*
*	이름: limit(const T& value)
*	인자: value - 정수 혹은 실수
*	반환: 0 ~ 255 사이의 값
*	설명: value값이 0보다 작으면 0, 255보다 크면 255로 변경 
*
/***************************************************************************************/
template<typename T>
inline T limit(const T& value){
	return ((value>255) ? 255: ((value<0) ? 0: value));
}


/***************************************************************************************
*
*	이름: limit(const T& value, const T& lower, const T& upper)
*	인자: value - 정수 혹은 실수
*	반환: lower ~ upper 사이의 값
*	설명: value 값이 lower보다 작으면 lower, upper보다 크면 upper로 변경
*
/***************************************************************************************/
template<typename T>
inline T limit(const T& value, const T& lower, const T& upper){
	return ((value>upper) ? upper: (value<lower) ? lower: value);
}



/***************************************************************************************
*
*	이름: swap(T& lha, T& rha)
*	인자: lha - Left and argument, rha - Right hand argument
*	반환: 없음
*	설명: rha와 lha의 값을 서로 바꿈
*
/***************************************************************************************/
template<typename T>
inline void swap(T& lha, T& rha){

	T tmp = lha;
	lha = rha;
	rha = tmp;
	
}