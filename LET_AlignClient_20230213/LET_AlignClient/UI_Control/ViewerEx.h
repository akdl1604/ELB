#pragma once
#include "stdafx.h"

#define MAX_DRAW 1
struct stFigure							//		  Hit[0]	  Hit[1]	  Hit[2]
{										//			□──────────□──────────□
	bool isDraw;		// Draw 할지.	//			│					  │
	bool isLine;		// 직선인지.		//			│					  │
	bool isRectangle;	// 사각형인지.	//			│					  │
	bool isCircle;		// 원 인지.		//	 Hit[7] □					  □ Hit[3]
	int  nWidth;		// 두께.			//			│					  │
	CPoint ptBegin;		// 시작 점.		//			│					  │
	CPoint ptEnd;		// 끝 점.		//			│					  │
	CRect  rcHit[8];	// 터치 점.		//			□──────────□──────────□
	COLORREF clrColor;	// 도형 색상.	//		  Hit[6]	  Hit[5]	  Hit[4]
	float fLength;		// 직선의 길이.
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
