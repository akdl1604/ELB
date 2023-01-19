// OpenGLControl.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "../FPS_Measure.h"
#include "OpenGLControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COpenGLControl

COpenGLControl::COpenGLControl()
{
	dc = NULL;
	rotation = 0.0f;
	m_LeftButtonDown = false;
	m_RightButtonDown = false;

	m_Res = 1;		// Plot all points
	m_Scale = (float)1.8;	
	m_Bkcolor = 0.0;
	m_Fill = GL_FILL;
	m_xRotation = -10.0;		// Start view 
	m_yRotation = -44.0;

	ogl_Image = NULL;
}

COpenGLControl::~COpenGLControl()
{
	if (dc)
	{
		delete dc;
	}

	openGLDevice.destroy();
}


BEGIN_MESSAGE_MAP(COpenGLControl, CWnd)
	//{{AFX_MSG_MAP(COpenGLControl)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen f? Nachrichten COpenGLControl 


void COpenGLControl::InitGL()
{
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);							
	//glEnable(GL_DEPTH_TEST);					
	glDepthFunc(GL_LEQUAL);	
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void MapColor(const int *p)
{
	glColor3f(REDCOLOR(*p)/(GLfloat)255., 
			GRNCOLOR(*p)/(GLfloat)255., 
			BLUCOLOR(*p)/(GLfloat)255.);

}

void COpenGLControl::DrawGLScene()
{
	if(ogl_Image==NULL) return;

	glClear(GL_COLOR_BUFFER_BIT	 |  GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	int *p = (int *) ogl_Image->data;	// Ptr to bitmap
	unsigned long nx = ogl_Image->cols;	// Image # columns
	unsigned long ny = ogl_Image->rows;	// Image # rows
	int r, r2, r3, r4, *q;			
	unsigned long i, j;
	int min, max;
	GLfloat xscale, yscale, zscale;
	double pMin,pMax;
	cv::minMaxLoc(*ogl_Image, &pMin, &pMax);
	min = int(pMin); max = int(pMax);
	float w=(nx-150)/256.f;
	xscale = m_Scale/(GLfloat)nx;		// Size may have changed
	yscale = m_Scale/((GLfloat)(max - min)*(GLfloat)2.0); // Normalize
	zscale = m_Scale/(GLfloat)ny;

	glClearColor(m_Bkcolor, m_Bkcolor, m_Bkcolor, 0.0); // Set def. background
	glRotatef(m_xRotation, 0.0, 1.0, 0.0);	// About y-axis
	glRotatef(m_yRotation, 1.0, 0.0, 0.0);	// About x-axis
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, m_Fill);	
	glPushMatrix();
	glEnable(GL_DEPTH_TEST);	/* Enable hidden-surface-removal */	
	glTranslatef(-((GLfloat)nx/(GLfloat)2.) * xscale,-((GLfloat)(max + min)/(GLfloat)2.) * yscale,((GLfloat)ny/(GLfloat)2.) * zscale);
	glScalef(xscale, yscale, zscale);

	glBegin(GL_QUADS);
	for (j = 0; j < ny; j += m_Res) {
		p = (int *)(&ogl_Image->at<uchar>(j,0));
		for (i = 0; i < nx; i += m_Res, p += m_Res) {
			r = NINT(REDCOLOR(*p)*0.299 + GRNCOLOR(*p)*0.587 + BLUCOLOR(*p)*0.114);
			if (i + m_Res < nx) 		r2 = NINT(REDCOLOR(*(p+m_Res))*0.299+ GRNCOLOR(*(p+m_Res))*0.587+ BLUCOLOR(*(p+m_Res))*0.114);
			else break;

			if (j + m_Res < ny) {
				q = p + nx*m_Res;
				r3 = NINT(REDCOLOR(*q)*0.299+ GRNCOLOR(*q)*0.587+ BLUCOLOR(*q)*0.114);
				r4 = NINT(REDCOLOR(*(q+m_Res))*0.299+ GRNCOLOR(*(q+m_Res))*0.587+ BLUCOLOR(*(q+m_Res))*0.114);
			}else break;

			MapColor(p);			glVertex3i(i, r, -(signed)j);
			MapColor(p+m_Res);		glVertex3i(i + m_Res, r2, -(signed)j);
			MapColor(q+m_Res);		glVertex3i(i + m_Res, r4, -(signed)(j + m_Res));
			MapColor(q);			glVertex3i(i, r3, -(signed)(j + m_Res));
		}
	}

	glEnd();
	glPopMatrix();
	glFlush();

	SwapBuffers(dc->m_hDC);
}



void COpenGLControl::Create(CRect rect, CWnd *parent)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_OWNDC,	NULL,(HBRUSH)GetStockObject(BLACK_BRUSH),NULL);
	CreateEx(0,	className,"OpenGL",	WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,rect,	parent,	0);

}

void COpenGLControl::OnPaint() 
{
	rotation += 0.01f;

	if (rotation >= 360.0f)
	{
		rotation -= 360.0f;
	}

	/** OpenGL section **/

	openGLDevice.makeCurrent();

	DrawGLScene();
	
	CWnd::OnPaint(); 
}

void COpenGLControl::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	if (cy == 0)								
	{
		cy = 1;						
	}


	glViewport(0,0,cx,cy);	

	glMatrixMode(GL_PROJECTION);						
	glLoadIdentity();						

	
	glOrtho(-1.0f,1.0f,-1.0f,1.0f,1.0f,-1.0f);
//	gluPerspective(45.0f,cx/cy,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);						
	glLoadIdentity();
}


int COpenGLControl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	dc = new CClientDC(this);

	openGLDevice.create(dc->m_hDC);
	InitGL();

	return 0;
}

BOOL COpenGLControl::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}


void COpenGLControl::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_LeftButtonDown = true;
	m_LeftDownPos = point;
	SetCapture();

	CWnd::OnLButtonDown(nFlags, point);
}

void COpenGLControl::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_LeftButtonDown = false;
	ReleaseCapture();
	 CWnd::OnLButtonUp(nFlags, point);
}

void COpenGLControl::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_RightButtonDown = true;
	m_RightDownPos = point;
	SetCapture();

	 CWnd::OnRButtonDown(nFlags, point);
}

void COpenGLControl::OnRButtonUp(UINT nFlags, CPoint point)
{
	m_RightButtonDown = false;
	ReleaseCapture();

	 CWnd::OnRButtonUp(nFlags, point);
}


void COpenGLControl::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_LeftButtonDown) { 	// Left : x / y rotation
		m_xRotation += m_LeftDownPos.x - point.x; 
		m_yRotation += m_LeftDownPos.y - point.y; 
		m_LeftDownPos = point;	// Save for next call
		InvalidateRect(NULL, FALSE);	// (Faster) Invalidate();
	}
	else if (m_RightButtonDown) {	// Right : z translation (zoom)
		m_Scale += (m_RightDownPos.y - point.y)/(float)60.0; // 60.0 zoom rate
		m_RightDownPos = point;
		InvalidateRect(NULL, FALSE);
	}
	CWnd::OnMouseMove(nFlags, point);
}