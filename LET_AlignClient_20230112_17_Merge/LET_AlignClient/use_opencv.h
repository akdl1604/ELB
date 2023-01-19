/////////////////////////////////////////////////////////////////////
// use_opencv.h
// written  by darkpgmr (http://darkpgmr.tistory.com), 2013

#pragma once

#ifndef __USE_OPENCV_H__
#define __USE_OPENCV_H__
//
#define _OPENCV_2413

#ifdef _OPENCV_2413
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include "opencv/highgui.h"
#include "opencv2\calib3d.hpp"
#include "opencv2/contrib/contrib.hpp"


#ifdef _DEBUG
	#pragma comment(lib,"opencv_core2413d.lib")
	#pragma comment(lib,"opencv_highgui2413d.lib")
	#pragma comment(lib,"opencv_imgproc2413d.lib")
	#pragma comment(lib,"opencv_calib3d2413d.lib")
	#pragma comment(lib,"opencv_features2d2413d.lib")
    #pragma comment(lib,"opencv_contrib2413d.lib")
#else
	#pragma comment(lib,"opencv_core2413.lib")
	#pragma comment(lib,"opencv_highgui2413.lib")
	#pragma comment(lib,"opencv_imgproc2413.lib")
	#pragma comment(lib,"opencv_calib3d2413.lib")
	#pragma comment(lib,"opencv_features2d2413.lib")
   #pragma comment(lib,"opencv_contrib2413.lib")
#endif
	
template<class T>
class TypedMat
{
	T** m_pData;
	int m_nChannels;
	int m_nRows, m_nCols;

public:
	TypedMat():m_pData(NULL),m_nChannels(1),m_nRows(0),m_nCols(0){}
	~TypedMat(){if(m_pData) delete [] m_pData;}

	// OpenCV Mat 연동 (메모리 공유)
	void Attach(const cv::Mat& m);
	void Attach(const IplImage& m);
	TypedMat(const cv::Mat& m):m_pData(NULL),m_nChannels(1),m_nRows(0),m_nCols(0) { Attach(m);}
	TypedMat(const IplImage& m):m_pData(NULL),m_nChannels(1),m_nRows(0),m_nCols(0) { Attach(m);}
	const TypedMat & operator =(const cv::Mat& m){ Attach(m); return *this;}
	const TypedMat & operator =(const IplImage& m){ Attach(m); return *this;}

	// 행(row) 반환
	T* GetPtr(int r)
	{ assert(r>=0 && r<m_nRows); return m_pData[r];}

	// 연산자 중첩 (원소접근) -- 2D
	T * operator [](int r)
	{ assert(r>=0 && r<m_nRows); return m_pData[r];}

	const T * operator [](int r) const
	{ assert(r>=0 && r<m_nRows); return m_pData[r];}

	// 연산자 중첩 (원소접근) -- 3D
	T & operator ()(int r, int c, int k)
	{ assert(r>=0 && r<m_nRows && c>=0 && c<m_nCols && k>=0 && k<m_nChannels); return m_pData[r][c*m_nChannels+k];}

	const T operator ()(int r, int c, int k) const
	{ assert(r>=0 && r<m_nRows && c>=0 && c<m_nCols && k>=0 && k<m_nChannels); return m_pData[r][c*m_nChannels+k];}
};

template<class T>
void TypedMat<T>::Attach(const cv::Mat& m)
{
	assert(sizeof(T)==m.elemSize1());

	m_nChannels = m.channels();
	m_nRows = m.rows;
	m_nCols = m.cols;
	
	if(m_pData) delete [] m_pData;
	m_pData = new T * [m_nRows];
	for(int r=0; r<m_nRows; r++)
	{
		m_pData[r] = (T *)(m.data + r*m.step);
	}
}

template<class T>
void TypedMat<T>::Attach(const IplImage& m)
{
	assert(sizeof(T) == m.widthStep/(m.width*m.nChannels));

	m_nChannels = m.nChannels;
	m_nRows = m.height;
	m_nCols = m.width;
	
	if(m_pData) delete [] m_pData;
	m_pData = new T * [m_nRows];
	for(int r=0; r<m_nRows; r++)
	{
		m_pData[r] = (T *)(m.imageData + r*m.widthStep);
	}
}

#else

#include "opencv2/opencv.hpp"

#ifdef _DEBUG
#pragma comment(lib,"opencv_world455d.lib")
#else
#pragma comment(lib,"opencv_world455.lib")
#endif

template<class T>
class TypedMat
{
	T** m_pData;
	int m_nChannels;
	int m_nRows, m_nCols;

public:
	TypedMat() :m_pData(NULL), m_nChannels(1), m_nRows(0), m_nCols(0) {}
	~TypedMat() { if (m_pData) delete[] m_pData; }

	// OpenCV Mat 연동 (메모리 공유)
	void Attach(const cv::Mat& m);
	TypedMat(const cv::Mat& m) :m_pData(NULL), m_nChannels(1), m_nRows(0), m_nCols(0) { Attach(m); }

	const TypedMat& operator =(const cv::Mat& m) { Attach(m); return *this; }

	// 행(row) 반환
	T* GetPtr(int r)
	{
		assert(r >= 0 && r < m_nRows); return m_pData[r];
	}

	// 연산자 중첩 (원소접근) -- 2D
	T* operator [](int r)
	{
		assert(r >= 0 && r < m_nRows); return m_pData[r];
	}

	const T* operator [](int r) const
	{
		assert(r >= 0 && r < m_nRows); return m_pData[r];
	}

	// 연산자 중첩 (원소접근) -- 3D
	T& operator ()(int r, int c, int k)
	{
		assert(r >= 0 && r < m_nRows&& c >= 0 && c < m_nCols&& k >= 0 && k < m_nChannels); return m_pData[r][c * m_nChannels + k];
	}

	const T operator ()(int r, int c, int k) const
	{
		assert(r >= 0 && r < m_nRows&& c >= 0 && c < m_nCols&& k >= 0 && k < m_nChannels); return m_pData[r][c * m_nChannels + k];
	}
};

template<class T>
void TypedMat<T>::Attach(const cv::Mat& m)
{
	assert(sizeof(T) == m.elemSize1());

	m_nChannels = m.channels();
	m_nRows = m.rows;
	m_nCols = m.cols;

	if (m_pData) delete[] m_pData;
	m_pData = new T * [m_nRows];
	for (int r = 0; r < m_nRows; r++)
	{
		m_pData[r] = (T*)(m.data + r * m.step);
	}
}
#endif
#endif