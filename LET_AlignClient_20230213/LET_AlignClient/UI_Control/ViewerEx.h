#pragma once
#include "stdafx.h"

#define MAX_DRAW 1
struct stFigure							//		  Hit[0]	  Hit[1]	  Hit[2]
{										//			�ড�������������������ড��������������������
	bool isDraw;		// Draw ����.	//			��					  ��
	bool isLine;		// ��������.		//			��					  ��
	bool isRectangle;	// �簢������.	//			��					  ��
	bool isCircle;		// �� ����.		//	 Hit[7] ��					  �� Hit[3]
	int  nWidth;		// �β�.			//			��					  ��
	CPoint ptBegin;		// ���� ��.		//			��					  ��
	CPoint ptEnd;		// �� ��.		//			��					  ��
	CRect  rcHit[8];	// ��ġ ��.		//			�ড�������������������ড��������������������
	COLORREF clrColor;	// ���� ����.	//		  Hit[6]	  Hit[5]	  Hit[4]
	float fLength;		// ������ ����.
	bool isDot;
	int nFitWidth;
};

struct stFigureText
{
	bool bDraw;
	CPoint ptBegin;
	CPoint ptBeginFit;
	CString textString;
	COLORREF clrColor;
	int nOrgSize;
	int nFitSize;
	bool is_show_bkg;
};
#include "Graphics/GraphicBase.h"
#include "Graphics/GraphicArrow.h"
#include "Graphics/GraphicCircle.h"
#include "Graphics/GraphicDimensional.h"
#include "Graphics/GraphicHitTest.h"
#include "Graphics/GraphicInterativeCalipers.h"
#include "Graphics/GraphicPoint.h"
#include "Graphics/GraphicLabel.h"
#include "Graphics/GraphicLine.h"
#include "Graphics/GraphicPoints.h"
#include "Graphics/GraphicRectangle.h"
#include "Graphics/GraphicRegionalSelection.h"
#include "Graphics/GraphicDisplayControl.h"
