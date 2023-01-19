#include "stdafx.h"
#include "../LET_AlignClient.h"
#include "Camera.h"
#include "../LET_AlignClientDlg.h"

#ifndef _DAHUHA
CCamera::CCamera()
{
	m_nCam = 0;
	m_bSaveMPEG = false;
	m_nGrabBufferNo		= 0;
	m_bGrabContinuous	= FALSE;
	m_bInitialize		= FALSE;
	m_nTriggerMode		= CONTINUOUS;
	m_GigEConfiguration = NULL;
	m_hWndDlg = NULL;
	m_hDummy = NULL;
	m_hDummy = CreateEvent(NULL, TRUE, FALSE, NULL);
	InitializeSRWLock(&g_bufsrwlock); // 초기화 동기화 객체 Tkyuha 20211027
}

CCamera::~CCamera(void)
{
	BOOL bRet = Uninitialize();	

	if(m_hDummy) CloseHandle(m_hDummy);		
	if( m_GigEConfiguration != NULL ) delete m_GigEConfiguration;
}

BOOL CCamera::Initialize(INT nCam, CString strNum)
{
	try
	{
		if( IsOpen() )
		{
			return FALSE;
		}
		 
		m_ptlFactory = &CTlFactory::GetInstance();
		INT nDevCnt = m_ptlFactory->EnumerateDevices(m_DIList);

		if( nDevCnt > 0 )
		{
			// Get Serial Number
			CString strSerial;
			INT nDevNo = 0;

			for( DeviceInfoList_t::iterator it = m_DIList.begin(); it != m_DIList.end(); it++ )
			{
				strSerial = (*it).GetSerialNumber().c_str();
				if( _tcscmp(strSerial, strNum) == 0 )
					break;

				nDevNo++;
			}

			if(strNum.Left(1)==48) 
				nDevNo = 0;

			if( nDevNo >= nDevCnt )
				return FALSE;

			// Initialize Camera
			m_GigEConfiguration = new CSoftwareTriggerConfiguration;	// ljm
			m_GigECamera.RegisterConfiguration(m_GigEConfiguration, RegistrationMode_Append, Cleanup_None); //Ownership_ExternalOwnership
			m_GigECamera.RegisterConfiguration(this, RegistrationMode_Append, Cleanup_None);	 //Ownership_ExternalOwnership
			m_GigECamera.RegisterImageEventHandler(this, RegistrationMode_Append, Cleanup_None); //Ownership_ExternalOwnership

			m_GigECamera.GrabCameraEvents = TRUE;
			m_GigECamera.Attach(m_ptlFactory->CreateDevice(m_DIList[nDevNo]));

			if(m_GigECamera.IsPylonDeviceAttached())
			{				
				if( m_GigECamera.IsOpen() == TRUE ) 
				{
					m_GigECamera.Close();
				}

				//if (m_GigECamera.is)
				{
					m_GigECamera.Open();

					m_bInitialize = TRUE;
				}

				return m_bInitialize;
			}

			return FALSE;
		}
		else
		{
			return FALSE;
		}
	}
	catch (GenICam::GenericException &e)
	{
		if( m_GigEConfiguration->DebugGetEventHandlerRegistrationCount() > 0 )
			m_GigECamera.DeregisterConfiguration(m_GigEConfiguration);
			
		CString strError = e.what();
		//theLog.logmsg(LOG_ERROR, "[Cam_%d] Error Initialize: %s\n", nCam + 1, (CString)e.GetDescription() );
		
		/*CString str;
		str.Format("[%d Cam] %s", nCam + 1, (CString)e.GetDescription());
		AfxMessageBox(str);*/

		return FALSE;
	}
}

BOOL CCamera::Uninitialize()
{
	BOOL bSuccess = TRUE;

	try
	{
		if(IsOpen())
		{
			CDeviceInfo m_csDevInfo;

			if( m_bGrabContinuous )
			{
				GrabContinuous(FALSE);
			}

			if( m_GigEConfiguration->DebugGetEventHandlerRegistrationCount() > 0 )
				m_GigECamera.DeregisterConfiguration(m_GigEConfiguration);

			m_GigECamera.Close();
			m_csDevInfo.SetDeviceClass( m_GigECamera.GetDeviceInfo().GetDeviceClass());
			m_csDevInfo.SetSerialNumber( m_GigECamera.GetDeviceInfo().GetSerialNumber());

			m_GigECamera.DestroyDevice();

			bSuccess = TRUE;
		}
	}
	catch (...)
	{
		bSuccess = FALSE;
		return bSuccess;
	}

	return bSuccess;
}

BOOL CCamera::GrabContinuous(BOOL bContinuous)
{
	//// start grabber
	//if( bContinuous && !m_bGrabContinuous ) // 1 0
	//{
	//	if(IsOpen() && !IsGrabbing())
	//	{
	//		m_GigECamera.MaxNumBuffer = MAX_GRABBUFFER;
	//		m_GigECamera.StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
	//	}

	//	m_bGrabContinuous = TRUE;
	//}
	//// stop grabber
	//else if( !bContinuous && m_bGrabContinuous )	// 0 1
	//{		
	//	m_GigECamera.StopGrabbing();

	//	try
	//	{
	//		CGrabResultPtr ptrGrabResult;
	//		while ( m_GigECamera.IsGrabbing())
	//		{
	//			m_GigECamera.RetrieveResult( 5000, ptrGrabResult, TimeoutHandling_ThrowException);
	//			Sleep(100);
	//		}
	//	}
	//	catch (...)
	//	{
	//	}

	//	m_bGrabContinuous = FALSE;
	//}

	//// start grabber
	if (bContinuous) // 1 0
	{
		if (IsOpen()) // && !IsGrabbing()
		{
			m_GigECamera.StopGrabbing();
			m_GigECamera.MaxNumBuffer = MAX_GRABBUFFER;
			m_GigECamera.StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
		}

		m_bGrabContinuous = TRUE;
	}
	 //stop grabber
	else 
	{
		m_GigECamera.StopGrabbing();

		try
		{
			CGrabResultPtr ptrGrabResult;
			while (m_GigECamera.IsGrabbing())
			{
				m_GigECamera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);
				Sleep(100);
			}
		}
		catch (...)
		{
		}

		m_bGrabContinuous = FALSE;
	}

	return TRUE;
}

void CCamera::SingleGrab()
{	
	try {
		if( m_GigECamera.IsOpen() != TRUE )	return;

		if( !m_GigECamera.IsGrabbing() )
		{
			CGrabResultPtr ptrGrabResult;
			m_GigECamera.StartGrabbing(1, GrabStrategy_UpcomingImage, GrabLoop_ProvidedByUser);
			m_GigECamera.RetrieveResult( 5000, ptrGrabResult, TimeoutHandling_ThrowException);

			if (ptrGrabResult->GrabSucceeded() != TRUE)
			{
				theLog.logmsg(LOG_ERROR, "[Cam %d] Single Grab() - Grab TimeOut", m_nCam + 1);
			}

			m_GigECamera.StopGrabbing();
		}
		else
		{
			theLog.logmsg(LOG_ERROR, "[Cam %d] Single Grab() - Failed because Already Grabbing", m_nCam + 1);
		}
	}
	catch(...)
	{
		theLog.logmsg(LOG_ERROR, "[Cam %d] Single Grab() - Failed  Exception Error", m_nCam + 1);
	}
}

BOOL CCamera::SetTriggerMode(INT nMode)
{
	if(!m_bInitialize)
	{
		return FALSE;
	}

	switch(nMode)
	{
	case CONTINUOUS:
		{
			// Continuous Grab
			m_GigECamera.TriggerMode.SetValue(Basler_GigECamera::TriggerMode_Off);
			m_nTriggerMode = CONTINUOUS;
		}
		break;
	case HARDWARE_TRIGGER:
		{
			m_GigECamera.TriggerMode.SetValue(Basler_GigECamera::TriggerMode_On);
			m_GigECamera.TriggerSource.SetValue(Basler_GigECamera::TriggerSource_Line1);
			m_nTriggerMode = HARDWARE_TRIGGER;
		}
		break;
	}

	return TRUE;
}

void CCamera::SetExposure(INT nExposure)
{
	if(IsOpen())
	{
		//m_GigECamera.ExposureTimeRaw.SetValue((int64_t)nExposure);
		// Turn off auto exposure
		/*m_GigECamera.ExposureAuto.SetValue(Basler_GigECamera::ExposureAuto_Off);

		//Set exposure mode to "Timed"
		m_GigECamera.ExposureMode.SetValue(Basler_GigECamera::ExposureMode_Timed);*/

		//Set absolute value of shutter exposure time to nExposure microseconds

		m_GigECamera.ExposureTimeRaw.SetValue((int64_t)nExposure,false);
		
	}
}

void CCamera::SetGain(INT nGain)
{
	if(IsOpen())
	{
		m_GigECamera.GainAuto.SetValue(GainAuto_Off);
		m_GigECamera.GainRaw.SetValue((int64_t)nGain);
	}
}

void CCamera::ExecuteSoftwareTrigger()
{
	if(IsOpen())
	{
		m_GigECamera.ExecuteSoftwareTrigger();
	}
}

BOOL CCamera::OnProcessEnd(BYTE* pImageBuffer, int nImageSize)
{
	CLET_AlignClientDlg *pMain = (CLET_AlignClientDlg *)pDlg;
	if (pMain->m_bProgramEnd)	return FALSE;

	int dir = pMain->m_stCamInfo[m_nCam].flip_dir;
	int w = pMain->m_stCamInfo[m_nCam].w;
	int h = pMain->m_stCamInfo[m_nCam].h;

	//IDC_CHK_MANUAL_PREALIGN

	CPaneMachine * pMachine = (CPaneMachine * )pMain->m_pPane[PANE_MACHINE];
	int job = pMachine->m_hSelectAlgoTab.GetCurSel();
	BOOL bManualPrealign = pMachine->c_TabMachinePage[job]->IsDlgButtonChecked(IDC_CHK_MANUAL_PREALIGN);
	// 체크박스 선택되어 있을 경우 메모리 복사 하지 않도록.
	if (bManualPrealign || pImageBuffer==NULL)
	{
		pMain->m_bGrabEnd[m_nCam] = TRUE;
		return FALSE;
	}

	AcquireSRWLockExclusive(&g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027

	if (dir == 1)	// 90도
	{
		cv::Mat srcImg(w, h, CV_8UC1, pImageBuffer);	// Width / Height를 미리 Swap
		cv::Mat dstImg;

		cv::transpose(srcImg, dstImg);		
		pMain->copyMemory(pMain->getSrcBuffer(m_nCam), dstImg.data, nImageSize);

		dstImg.release();
	}
	else if (dir == 2)	// 270도
	{
		cv::Mat srcImg(w, h, CV_8UC1, pImageBuffer);	// Width / Height를 미리 Swap
		cv::Mat dstImg;

		cv::transpose(srcImg, dstImg);
		cv::flip(dstImg, dstImg, 0);	// 좌우 반전

		pMain->copyMemory(pMain->getSrcBuffer(m_nCam), dstImg.data, nImageSize);

		dstImg.release();
	}
	else if (dir == 3)	// 180도
	{
		cv::Mat srcImg(h, w, CV_8UC1, pImageBuffer);
		cv::Mat dstImg;

		cv::flip(srcImg, dstImg, 0);	// 상하 반전
		cv::flip(dstImg, dstImg, 1);	// 좌우 반전

		pMain->copyMemory(pMain->getSrcBuffer(m_nCam), dstImg.data, nImageSize);

		dstImg.release();
	}
	else
	{		
		pMain->copyMemory(pMain->getSrcBuffer(m_nCam), pImageBuffer, nImageSize);
	}

	if(!pMain->m_bGrabEnd[m_nCam] && (pMain->m_nCurPane == PANE_AUTO ||
		(pMain->m_nCurForm == FORM_MAIN || pMain->m_nCurForm == FORM_MACHINE || pMain->m_nCurForm == FORM_INTERFACE)) )	//220503 Tkyuha 카메라 겹칩 확인용
	{
		pMain->m_bGrabEnd[m_nCam] = TRUE;
	}

	ReleaseSRWLockExclusive(&g_bufsrwlock);

	if (m_bSaveMPEG)
	{
		//KJH 2022-05-18 Record중 View Gray화면 현상 의심 부분 수정
		cv::Mat videoFrame_Org(h, w, CV_8UC1, pMain->getSrcBuffer(m_nCam));
		cv::Mat videoFrame = videoFrame_Org.clone();

		if (videoFrame.empty()) {
			m_videoWriter.release();
			return FALSE;
		}

		if(m_videoWriter.isOpened())
			m_videoWriter << videoFrame;
		//HSJ Frame 해제 없어서 추가
		videoFrame.release();
		videoFrame_Org.release();
	}

	m_nGrabBufferNo = (m_nGrabBufferNo + 1) % MAX_GRABBUFFER;

	return TRUE;
}

void CCamera::OnImageGrabbed(CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult)
{
	// 20210924 Tkyuha 이미지 Grab에 문제 발생시 제거
	// 20210924 Tkyuha 동기화 객체 생성 DeadLock 방지
	std::unique_lock<std::mutex> lock(m_mtx); 
	BOOL CheckGrab = FALSE;

	if(ptrGrabResult->GrabSucceeded())
	{	
		int nSize = (int)ptrGrabResult->GetImageSize();
		int w = ptrGrabResult->GetWidth();
		int h = ptrGrabResult->GetHeight();

		CheckGrab = OnProcessEnd((BYTE*)ptrGrabResult->GetBuffer(),(int) ptrGrabResult->GetImageSize());
		m_nInputFrame = (INT)ptrGrabResult->GetBlockID();

		if (!CheckGrab)
		{
			theLog.logmsg(LOG_ERROR, _T("Grab Return & Discription : %s"), ptrGrabResult->GetErrorDescription());
		}
	}
	else
	{
		/*theLog.logmsg(LOG_ERROR, _T("[%d] Grab Frame: %d - ErrorCode: %d"), m_nCam, ptrGrabResult->GetFrameNumber(), ptrGrabResult->GetErrorCode());*/		
		theLog.logmsg(LOG_ERROR, _T("Discription : %s"), ptrGrabResult->GetErrorDescription());		
	}
}

void CCamera::OnGrabError(CInstantCamera& camera, const char* errorMessage)
{
	theLog.logmsg(LOG_ERROR, "[Cam %d] OnGrabError() - %s", m_nCam + 1, errorMessage);
}

void CCamera::cvVideoOpen(CString fname, int w, int h)
{
	if (m_bSaveMPEG) cvVideoClose();

	fileCnt=0;
	frameCnt = 0;
	m_videoName = fname;
	//HSJ 카메라마다 Frame수 다르므로 변수로 선언
	int nFrame= m_GigECamera.AcquisitionFrameRateAbs.GetValue();
	m_videoWriter.open((LPCTSTR)fname, CV_FOURCC('X', 'V', 'I', 'D'), nFrame, cv::Size(w, h), false);
	m_bSaveMPEG = true;
}

void CCamera::cvVideoClose()
{
	if (m_videoWriter.isOpened())	m_videoWriter.release();
	m_bSaveMPEG = false;
}

//HSJ Exposuretime pc종료 후 재부팅 시에도 저장한값 불러오기
void CCamera::SaveExposureTime(INT nExposure)
{
	//User Set1로 저장
	/*m_GigECamera.UserSetSelector.SetValue(UserSetSelector_UserSet1);
	m_GigECamera.UserSetLoad.Execute();*/
	m_GigECamera.ExposureTimeRaw.SetValue((int64_t)nExposure, false);
	Delay(50);
	m_GigECamera.UserSetSelector.SetValue(UserSetSelector_UserSet1);
	Delay(50);
	m_GigECamera.UserSetSave.Execute();
}


BOOL CCamera::set_reverse_x(BOOL v)
{
	bool enable = m_GigECamera.ReverseX.GetValue();

	if (enable == true)
	{
		if (v != TRUE )
		{
			m_GigECamera.ReverseX.SetValue(v ? true : false);
			Delay(50);
		}
	}
	else
	{
		if (v)
		{
			m_GigECamera.ReverseX.SetValue(v ? true : false);
			Delay(50);
		}
	}
	return TRUE;
}

BOOL CCamera::set_reverse_y(BOOL v)
{
	bool enable = m_GigECamera.ReverseY.GetValue();

	if (enable == true)
	{
		if (v != TRUE)
		{
			m_GigECamera.ReverseY.SetValue(v ? true : false);
			Delay(50);
		}
	}
	else
	{
		if (v)
		{
			m_GigECamera.ReverseY.SetValue(v ? true : false);
			Delay(50);
		}
	}
	return TRUE;
}

BOOL CCamera::set_packet_size(int size)
{
	if (size < 1500) return FALSE;

	int packet_size = m_GigECamera.GevSCPSPacketSize.GetValue();

	if (packet_size != size)
	{
		m_GigECamera.GevSCPSPacketSize.SetValue(size);
		Delay(50);
	}


	return TRUE;
}

BOOL CCamera::set_frame_rate(double frame_rate)
{
	if (frame_rate <= 0)		return FALSE;

	bool enable = m_GigECamera.AcquisitionFrameRateEnable.GetValue();

	if (enable != true)
	{
		m_GigECamera.AcquisitionFrameRateEnable.SetValue(true);
		Delay(50);
	}
	
	double rate = m_GigECamera.AcquisitionFrameRateAbs.GetValue();
	if (rate != frame_rate)
	{
		m_GigECamera.AcquisitionFrameRateAbs.SetValue(frame_rate);
		Sleep(50);
	}

	return TRUE;
}
#endif