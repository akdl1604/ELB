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
//		   ������ 
//-----------------------------------------------------------
public:
	RGBBYTE(const BYTE& pixel = 0);								//�⺻������(���ڰ� ���� ��� r, g, b 0���� �ʱ�ȭ)
	RGBBYTE(const BYTE& _r, const BYTE& _g, const BYTE& _b);	//�⺻������(���ڿ� ���� r, g, b �ʱ�ȭ)
	RGBBYTE(const RGBBYTE& pixel);								//���������
	~RGBBYTE(void);


//-----------------------------------------------------------
//			ġȯ ������ �����ε�
//-----------------------------------------------------------
public:
	RGBBYTE& operator=(const RGBBYTE& pixel);
	RGBBYTE& operator=(const COLORREF& pixel);
	RGBBYTE& operator=(const BYTE& pixel);

//-----------------------------------------------------------
//			���� ������ �����ε�
//-----------------------------------------------------------
public:
	int operator==(const RGBBYTE& pixel);
	int operator!=(const RGBBYTE& pixel);


};


