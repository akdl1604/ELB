/*
	Dib.h: CDib Ŭ������ �������

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
	LONG		m_nWidth;		// �̹����� ���� ũ��
	LONG		m_nHeight;		// �̹����� ���� ũ��
	WORD		m_nBitCount;	// �ȼ��� ��Ʈ��(Gray = 8, RGB = 24)
	DWORD		m_nDibSize;		// sizeof(BITMAPINFOHEADER + RGBQUAD(GrayImage�� �ش�) + �ȼ�������)

	BYTE*		m_pDib;			// DIB������ ��� �ִ� ������ ���� �ּ�

	BYTE**		m_pGrayPtr;		// GrayImage���� 2D �ȼ� ���ٿ� ������
	RGBBYTE**	m_pRgbPtr;		// TruecolorImage���� 2D �ȼ� ���ٿ� ������


//-----------------------------------------------------------
// member function set
//-----------------------------------------------------------
public:
	//������ & �Ҹ���
	CDib(void);
	CDib(const CDib& dib);
	~CDib(void);


	//�̹��� ���� & �Ҹ� & ����
	BOOL Copy(CDib* pDib);
	BOOL CreateGrayImage(int nWidth, int nHeight, BYTE value = 255);
	BOOL CreateRGBImage(int nWIdth, int nHeight, COLORREF value = 0x00ffffff);
	void Destroy();

	//ġȯ ������ �����ε��� �̹��� ����
	CDib& operator=(const CDib& dib);
	BOOL CopyImage(CDib* pImage);

	// �̹��� ȭ�� ���
	void Draw(HDC hDC);
	void Draw(HDC hDC, int dx, int dy);
	void Draw(HDC hDC, int dx, int dy, int dw, int dh);
	void Draw(HDC hDC, int dx, int dy, int dw, int dh, int sx, int sy, int sw, int sh);

	// ���� �����
	BOOL LoadImage(LPCTSTR lpszFileName);
	BOOL SaveImage(LPCTSTR lpszFileName);

	// BMP ���� �ҷ�����/ �����ϱ�
	BOOL LoadBMP(LPCTSTR lpszFileName);
	BOOL SaveBMP(LPCTSTR lpszFileName);

	//Clipboard�� �����ϱ� �Ǵ� �ٿ��ֱ�
	BOOL	CopyToClipboard();
	BOOL	PasteFromClipboard();
	//2D �ȼ� ������ ��ȯ
	BYTE**		GetGrayPtr();
	RGBBYTE**	GetRgbPtr();
	BYTE*	GetDibPtr(){	return m_pDib;		}

	//�ȷ�Ʈ���� RGBQUAD �迭�� ������ ��ȯ
	int	GetPaletteNums();

	//�ȼ� ������ ���� ������ ��ȯ
	LPVOID GetDibBitsAddr();

	//�̹��� ���� ��ȯ �Լ�
	LONG	GetHeight()		{	return m_nHeight;		}
	LONG	GetWidth()		{	return m_nWidth;		}
	LONG	GetBitCount()	{	return m_nBitCount;		}
	LONG	GetDibSize()	{	return m_nDibSize;		}
	LONG	Valid()			{	return (m_pDib != NULL);}

	// BITMAPINFOHEADER ���� ������ ��ȯ
	LPBITMAPINFO GetBitmapInfoAddr() { return (LPBITMAPINFO)m_pDib; }

protected:
	//m_pDib ���� ��, Image ���� ���� �Լ�
	BOOL InitDib();

	//2D �ȼ� ������ ������ ���� interface ���� & �Ҹ�
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
*	�̸�: limit(const T& value)
*	����: value - ���� Ȥ�� �Ǽ�
*	��ȯ: 0 ~ 255 ������ ��
*	����: value���� 0���� ������ 0, 255���� ũ�� 255�� ���� 
*
/***************************************************************************************/
template<typename T>
inline T limit(const T& value){
	return ((value>255) ? 255: ((value<0) ? 0: value));
}


/***************************************************************************************
*
*	�̸�: limit(const T& value, const T& lower, const T& upper)
*	����: value - ���� Ȥ�� �Ǽ�
*	��ȯ: lower ~ upper ������ ��
*	����: value ���� lower���� ������ lower, upper���� ũ�� upper�� ����
*
/***************************************************************************************/
template<typename T>
inline T limit(const T& value, const T& lower, const T& upper){
	return ((value>upper) ? upper: (value<lower) ? lower: value);
}



/***************************************************************************************
*
*	�̸�: swap(T& lha, T& rha)
*	����: lha - Left and argument, rha - Right hand argument
*	��ȯ: ����
*	����: rha�� lha�� ���� ���� �ٲ�
*
/***************************************************************************************/
template<typename T>
inline void swap(T& lha, T& rha){

	T tmp = lha;
	lha = rha;
	rha = tmp;
	
}