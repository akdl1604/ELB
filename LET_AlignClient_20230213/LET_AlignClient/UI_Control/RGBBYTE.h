#pragma once
class RGBBYTE
{
//-----------------------------------------------------------
//         member value
//-----------------------------------------------------------
public:
	BYTE b;
	BYTE g;
	BYTE r;


//-----------------------------------------------------------
//		   생성자 
//-----------------------------------------------------------
public:
	RGBBYTE(const BYTE& pixel = 0);								//기본생성자(인자가 없을 경우 r, g, b 0으로 초기화)
	RGBBYTE(const BYTE& _r, const BYTE& _g, const BYTE& _b);	//기본생성자(인자에 따라 r, g, b 초기화)
	RGBBYTE(const RGBBYTE& pixel);								//복사생성자
	~RGBBYTE(void);


//-----------------------------------------------------------
//			치환 연산자 오버로딩
//-----------------------------------------------------------
public:
	RGBBYTE& operator=(const RGBBYTE& pixel);
	RGBBYTE& operator=(const COLORREF& pixel);
	RGBBYTE& operator=(const BYTE& pixel);

//-----------------------------------------------------------
//			관계 연산자 오버로딩
//-----------------------------------------------------------
public:
	int operator==(const RGBBYTE& pixel);
	int operator!=(const RGBBYTE& pixel);


};


