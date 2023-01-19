#if !defined(AFX_OPENGLCONTROL_H__52A6B63B_01A2_449D_8691_1FF59EECAB71__INCLUDED_)
#define AFX_OPENGLCONTROL_H__52A6B63B_01A2_449D_8691_1FF59EECAB71__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OpenGLControl.h : Header-Datei
//

#include "OpenGLDevice.h"
#include <GL\gl.h>
#include <GL\glu.h>

/////////////////////////////////////////////////////////////////////////////
// Fenster COpenGLControl 

class COpenGLControl : public CWnd
{
// Konstruktion
public:
	COpenGLControl();

// Attribute
public:
	int m_Fill;
	int m_Res;			// Plot resolution
	float m_Bkcolor;	// Background color
	float m_Scale;		// Plot scale

	float m_xRotation;
	float m_yRotation;

	BOOL m_LeftButtonDown;
	BOOL m_RightButtonDown;
	CPoint m_LeftDownPos;
	CPoint m_RightDownPos;

	cv::Mat *ogl_Image;
// Operationen
public:

// ?erschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktions?erschreibungen
	//{{AFX_VIRTUAL(COpenGLControl)
	public:
	//}}AFX_VIRTUAL

// Implementierung
public:
	void Create(CRect rect,CWnd* parent);
	virtual ~COpenGLControl();




	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	void InitGL();
	void DrawGLScene();
	OpenGLDevice openGLDevice;
	CClientDC* dc;
	float rotation;

	//{{AFX_MSG(COpenGLControl)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ f?t unmittelbar vor der vorhergehenden Zeile zus?zliche Deklarationen ein.

#endif // AFX_OPENGLCONTROL_H__52A6B63B_01A2_449D_8691_1FF59EECAB71__INCLUDED_
