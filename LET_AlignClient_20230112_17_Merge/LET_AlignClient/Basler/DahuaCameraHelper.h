#ifdef _DAHUHA

#pragma once
	
#include "GenICam/Defs.h"
#include "GenICam/System.h"
#include "GenICam/Camera.h"
#include "GenICam/Frame.h"
#include "Infra/Thread.h"
#include "Infra/Timer.h"
#include "Memory/SharedPtr.h"
#include "MessageQue.h"
#include <mutex>

using namespace Dahua;
using namespace Dahua::GenICam;

#define DEFAULT_SHOW_RATE (30)
#define FRAME_DISPLAY_INTERVAL30	33333333

class CDahuaCameraHelper
{
public:
	CDahuaCameraHelper(void);
	~CDahuaCameraHelper(void);
public:
	bool setCamera(ICameraPtr& camera);
	bool setDisplayWndHandle(HWND hWnd);
	bool connect();
	bool initcamera();
	bool disconnect();
	bool grabStart();
	bool grabStop();
	bool grabSingle(int callback=0);
	bool isgrabbing(){return m_bIsGrabbing;}
	bool changeParameters(int frame);
	bool isconnected(){return !m_SPtrCamera?false:true;}
	bool istriggerMode(){ return m_bTriggerMode; }

	int32_t triggerSoftware();
	int32_t setGrabMode( bool bContious=true);
	int32_t setGainRaw(double dGainRaw);
	int32_t setExposureTime(double dExposureTime, bool bAutoExposure = false);
	int32_t setReverseY( bool bSet );
	int32_t setReverseX( bool bSet );
	//ISP Control
	int32_t setBrightness( int nBrightness );
	int32_t setContrast( int nContrast );
	
	void    cvVideoOpen(CString fname, int w, int h);
	void    cvVideoClose();

	CFrame frameDahuha;
	CDialog *pDlg;
	cv::VideoWriter             m_videoWriter;
	CString                     m_videoName;
	int m_nCam;
	bool m_bSaveMPEG;
	SRWLOCK g_bufsrwlock; // Display와 이미지 Grab Thread 동기화 객체 20211027 Tkyuha
	std::mutex m_mtx;

protected :
	// 디스플레이 할 윈도우 핸들
	HWND m_hWnd;

	// 사용할 카메라 포인터
	ICameraPtr m_SPtrCamera;	

	// 카메라의 스트림소스
	IStreamSourcePtr					m_StreamPtr;

protected:	
	// 그랩콜백 함수이다. 	
	virtual void onGetFrame0(const CFrame &pFrame);

protected:
	// 그랩중인지 알리는 플래드
	bool m_bIsGrabbing;
	bool m_bTriggerMode;
};

#endif