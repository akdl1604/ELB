#include "StdAfx.h"
#include "Dib.h"
#include "RGBBYTE.h"

//������ ����

RGBBYTE::RGBBYTE(const BYTE& pixel) : r(pixel), g(pixel), b(pixel)
{
}

RGBBYTE::RGBBYTE(const BYTE& _r, const BYTE& _g, const BYTE& _b) : r(_r), g(_g), b(_b)
{
}

RGBBYTE::RGBBYTE(const RGBBYTE& pixel) : r(pixel.r), g(pixel.g), b(pixel.b)
{
}
RGBBYTE::~RGBBYTE(void)
{
}

//ġȯ ������ �����ε�

RGBBYTE& RGBBYTE::operator=(const RGBBYTE& pixel){
	if(this == &pixel)
		return *this;

	this->r = pixel.r;
	this->g = pixel.g;
	this->b = pixel.b;

	return *this;
}

RGBBYTE& RGBBYTE::operator=(const COLORREF& pixel){
	r = GetRValue(pixel);
	g = GetGValue(pixel);
	b = GetBValue(pixel);

	return *this;
}

RGBBYTE& RGBBYTE::operator=(const BYTE& pixel){
	r = pixel;
	g = pixel;
	b = pixel;

	return *this;
}

//���� ������ �����ε�

int RGBBYTE::operator==(const RGBBYTE& pixel){
	if(r == pixel.r && g == pixel.g && b == pixel.b) return 1;
	else return 0;
}

int RGBBYTE::operator!=(const RGBBYTE& pixel){
	if(r != pixel.r || g != pixel.g || b != pixel.b) return 1;
	else return 0;
}