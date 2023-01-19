#include "StdAfx.h"
#include "DahuaCameraHelper.h"

#include "../LET_AlignClient.h"
#include "../LET_AlignClientDlg.h"

#ifdef _DAHUHA

#pragma comment(lib,"MVSDKmd.lib")

CDahuaCameraHelper::CDahuaCameraHelper(void)
	:m_bIsGrabbing(false)
	,m_hWnd(NULL)
{
	m_nCam = 0;
	m_bSaveMPEG = false;
	m_bTriggerMode = false;
	InitializeSRWLock(&g_bufsrwlock); // �ʱ�ȭ ����ȭ ��ü Tkyuha 20211027
}


CDahuaCameraHelper::~CDahuaCameraHelper(void)
{
	grabStop();
	disconnect();	
}

bool CDahuaCameraHelper::setCamera(ICameraPtr& camera)
{
	m_SPtrCamera = camera;
	if(!m_SPtrCamera)
		return false;

	return true;
}

bool CDahuaCameraHelper::setDisplayWndHandle(HWND hWnd)
{
	m_hWnd = hWnd;
	if(m_hWnd == NULL)
		return false;

	return true;
}


int32_t CDahuaCameraHelper::setReverseX( bool bSet )
{
	if(!m_SPtrCamera)	return -1;
	
	IImageFormatControlPtr sptImageFormatControl = CSystem::getInstance().createImageFormatControl(m_SPtrCamera);
	if( NULL == sptImageFormatControl )
	{
		return -1;
	}
	
	CBoolNode boolNode = sptImageFormatControl->reverseX(); 
	boolNode.setValue(bSet);
	
	return 0;
}

int32_t CDahuaCameraHelper::setReverseY( bool bSet )
{
	if(!m_SPtrCamera)	return -1;
	
	IImageFormatControlPtr sptImageFormatControl = CSystem::getInstance().createImageFormatControl(m_SPtrCamera);
	if( NULL == sptImageFormatControl )
	{
		return -1;
	}
	
	CBoolNode boolNode = sptImageFormatControl->reverseY(); 
	boolNode.setValue(bSet);
	
	return 0;
}

// ISP CONTROL
/////////////////////////////////////////////////

int32_t CDahuaCameraHelper::setBrightness( int nBrightness )
{
	if(!m_SPtrCamera)	return -1;

	bool bRet;
	IISPControlPtr sptISPControl = CSystem::getInstance().createISPControl(m_SPtrCamera);
	if ( NULL == sptISPControl )
	{
		return -1;
	}

	CIntNode intNude = sptISPControl->brightness();
	bRet = intNude.setValue(nBrightness);
	if (false == bRet)		return -1;

	return 0;

}

int32_t CDahuaCameraHelper::setContrast( int nContrast )
{
	if(!m_SPtrCamera)	return -1;

	bool bRet;
	IISPControlPtr sptISPControl = CSystem::getInstance().createISPControl(m_SPtrCamera);
	if ( NULL == sptISPControl )		return -1;

	CIntNode intNude = sptISPControl->contrast();
	bRet = intNude.setValue(nContrast);
	if (false == bRet)		return -1;

	return 0;
}

////////////////////////////
bool CDahuaCameraHelper::connect()
{
	if(!m_SPtrCamera)
		return false;


	bool res = m_SPtrCamera->connect();
	return res;

}

bool CDahuaCameraHelper::disconnect()
{
	if(!m_SPtrCamera)		return false;
	
	bool bRet;
	bRet = m_StreamPtr->detachGrabbing(GenICam::IStreamSource::Proc(&CDahuaCameraHelper::onGetFrame0, this));
	if (!bRet)	return false;

	bool res = m_SPtrCamera->disConnect();
	return res;
}

bool CDahuaCameraHelper::initcamera()
{
	if (!isconnected())		return false;

	CSystem& systemObj = CSystem::getInstance();
	IAcquisitionControlPtr sptrAcquisitionControl = systemObj.createAcquisitionControl(m_SPtrCamera);

	CEnumNode grabmodeSelector = sptrAcquisitionControl->acquisitionMode();
	grabmodeSelector.setValueBySymbol("Continuous");

	// ī�޶�κ��� ��Ʈ�� �ҽ� ����
	if (NULL == m_StreamPtr)	m_StreamPtr = systemObj.createStreamSource(m_SPtrCamera);
	if (NULL == m_StreamPtr)		return false;


	if (m_StreamPtr->isGrabbing()) m_StreamPtr->stopGrabbing();

	BOOL bRet = m_StreamPtr->attachGrabbing(GenICam::IStreamSource::Proc(&CDahuaCameraHelper::onGetFrame0, this));
	if (!bRet)	return false;

	return true;
}

bool CDahuaCameraHelper::grabStart()
{
	if(!m_SPtrCamera || NULL == m_StreamPtr)		return false;
	if(m_bIsGrabbing)		return false;
	if (m_StreamPtr->isGrabbing()) m_StreamPtr->stopGrabbing();

	// �׷� ����
	bool isStartGrabbingSuccess = m_StreamPtr->startGrabbing();

	if (!isStartGrabbingSuccess)
	{		
		m_bIsGrabbing = false;
		return false;
	}
	else
	{		
		m_bIsGrabbing = true;
	}


	return true;
}

bool CDahuaCameraHelper::grabStop()
{
	// TODO: ī�޶� �׷��ߴ� ��ư �ݹ�
	if(!m_SPtrCamera || NULL == m_StreamPtr)		return false;
	if(!m_bIsGrabbing)		return false;

	bool bRet;
	
	// �׷��ߴ�
	bRet = m_StreamPtr->stopGrabbing();
	if(!bRet)		return false;	

	// ���÷��� �����尡 ����ɶ����� ��ٸ���. 
	m_bIsGrabbing = false;

	return true;

}

bool CDahuaCameraHelper::grabSingle(int callback)
{	
	if (NULL == m_StreamPtr || NULL == m_StreamPtr)	return false;
	if (NULL == m_StreamPtr)		return false;

	bool isSuccess = false;
	//HTK 2022-07-12 DAHUHA Single Grab ��������
	int ncount = 0;

	if(! m_StreamPtr->isGrabbing()) 
	{
		isSuccess = grabStart();

		CLET_AlignClientDlg* pMain = (CLET_AlignClientDlg*)pDlg;

		while (isSuccess && pMain->m_bGrabEnd[m_nCam]==FALSE)
		{
			Delay(10);
			ncount++;
			if (ncount > 200) break;
		}

		grabStop();

		//m_StreamPtr->startGrabbing();

		//if (m_StreamPtr->getFrame(frameDahuha, 5000))	isSuccess = true;
		//else
		//{
		//	m_StreamPtr->stopGrabbing();
		//	isSuccess = false;
		//	//HTK 2022-06-29 Dahuha Grab���� Bool �߰�
		//	m_bIsGrabbing = false;
		//}
	}		

	return isSuccess;
}

int32_t CDahuaCameraHelper::triggerSoftware()
{
	if(!m_SPtrCamera)	return -1;

    int32_t bRet;
    IAcquisitionControlPtr sptrAcquisitionControl = CSystem::getInstance().createAcquisitionControl(m_SPtrCamera);
    if (NULL == sptrAcquisitionControl)        return -1;

    CCmdNode cmdNode = sptrAcquisitionControl->triggerSoftware();
    bRet = cmdNode.execute();
    if (false == bRet)        return -1;
 
    return 0;
}

int32_t CDahuaCameraHelper::setGrabMode(bool bContious)
{
	if(!m_SPtrCamera)	return -1;

    bool bRet;
    IAcquisitionControlPtr sptrAcquisitionControl = CSystem::getInstance().createAcquisitionControl(m_SPtrCamera);
    if (NULL == sptrAcquisitionControl)        return -1;
  
    CEnumNode enumNode = sptrAcquisitionControl->triggerSelector();
    bRet = enumNode.setValueBySymbol("FrameStart");
    if (false == bRet)       return -1;

    if (true == bContious)
    {
        enumNode = sptrAcquisitionControl->triggerMode();
        bRet = enumNode.setValueBySymbol("Off");
        if (false == bRet)           return -1;
		m_bTriggerMode = false;
    }
    else
    {
        enumNode = sptrAcquisitionControl->triggerMode();
        bRet = enumNode.setValueBySymbol("On");
        if (false == bRet)            return -1;

        enumNode = sptrAcquisitionControl->triggerSource();
        bRet = enumNode.setValueBySymbol("Software");
        if (true != bRet)            return -1;

		m_bTriggerMode=true;
    }
    return 0;
}

int32_t CDahuaCameraHelper::setExposureTime(double dExposureTime, bool bAutoExposure)
{
	if(!m_SPtrCamera)	return -1;

    bool bRet;
    IAcquisitionControlPtr sptrAcquisitionControl = CSystem::getInstance().createAcquisitionControl(m_SPtrCamera);
    if (NULL == sptrAcquisitionControl)           return -1;

	IUserSetControlPtr sptrUserSetControl = CSystem::getInstance().createUserSetControl(m_SPtrCamera);
	if (NULL == sptrUserSetControl)           return -1;

    if (bAutoExposure)
    {
        CEnumNode enumNode = sptrAcquisitionControl->exposureAuto();
        bRet = enumNode.setValueBySymbol("Continuous");
        if (false == bRet)            return -1;
    }
    else
    {
        CEnumNode enumNode = sptrAcquisitionControl->exposureAuto();
        bRet = enumNode.setValueBySymbol("Off");
        if (false == bRet)            return -1;
 
        CDoubleNode doubleNode = sptrAcquisitionControl->exposureTime();
        bRet = doubleNode.setValue(dExposureTime);
        if (false == bRet)            return -1;

		//2022-04-19 save��ư Ŭ�� �� �����ϱ�
		//sptrUserSetControl->saveUserSet(); // 20220207 ���� ��� �߰� Tkyuha 
    }

    return 0;
}

int32_t CDahuaCameraHelper::setGainRaw(double dGainRaw)
{
	if(!m_SPtrCamera)	return -1;
 
    IAnalogControlPtr sptrAnalogControl = CSystem::getInstance().createAnalogControl(m_SPtrCamera);
    if (NULL == sptrAnalogControl)        return -1;

    CDoubleNode doubleNode = sptrAnalogControl->gainRaw();
    if (!doubleNode.setValue(dGainRaw))        return -1;

    return 0;
}

bool CDahuaCameraHelper::changeParameters(int frame)
{
	if(!m_SPtrCamera)	return false;
	if(m_bIsGrabbing)	return false;

	CSystem &systemObj = CSystem::getInstance();	
	IAcquisitionControlPtr sptrAcquisitionControl = systemObj.createAcquisitionControl(m_SPtrCamera);

	CEnumNode triggerSelector = sptrAcquisitionControl->triggerSelector();
	CEnumNode triggerMode = sptrAcquisitionControl->triggerMode();
	CDoubleNode acquisitionFrameRate = sptrAcquisitionControl->acquisitionFrameRate();
	CBoolNode acquisitionFrameRateEnable = sptrAcquisitionControl->acquisitionFrameRateEnable();

	
	bool res = true;

	// Ʈ���� ��带 non-external trigger ���� ����
	res &= triggerSelector.setValueBySymbol("FrameStart");
	res &= triggerMode.setValueBySymbol("Off");

	res &= triggerSelector.setValueBySymbol("AcquisitionStart");
	res &= triggerMode.setValueBySymbol("Off");

	// �����ӷ���Ʈ�� 10.0���� ����
	res &= acquisitionFrameRate.setValue(double(frame)); //2017.10.21 jsh 10->18 ����
	res &= acquisitionFrameRateEnable.setValue(true);

	return res;
}

void CDahuaCameraHelper::onGetFrame0(const CFrame &pFrame)
{
	CLET_AlignClientDlg* pMain = (CLET_AlignClientDlg*)pDlg;
	if (pMain->m_bProgramEnd)	return;
	if (pFrame.valid() == false)
	{
		pMain->m_bGrabEnd[m_nCam] = TRUE;
		return;
	}

	AcquireSRWLockExclusive(&g_bufsrwlock); // ����ȭ ��Ŵ Tkyuha 20211027

	unsigned char* imgdata = (unsigned char*)pFrame.getImage() ;					// �̹��� ������ �迭 ȹ��
	int nSize = pFrame.getImageSize();
	int dir = pMain->m_stCamInfo[m_nCam].flip_dir;
	int w = pMain->m_stCamInfo[m_nCam].w;
	int h = pMain->m_stCamInfo[m_nCam].h;

	nSize = w * h; // 2022-06-08 Tkyuha

	if (dir == 1)	// 90��
	{
		cv::Mat srcImg(w, h, CV_8UC1, imgdata);	// Width / Height�� �̸� Swap
		cv::Mat dstImg;

		cv::transpose(srcImg, dstImg);
		pMain->copyMemory(pMain->getSrcBuffer(m_nCam), dstImg.data, nSize);
		dstImg.release();
	}
	else if (dir == 2)	// 270��
	{
		cv::Mat srcImg(w, h, CV_8UC1, imgdata);	// Width / Height�� �̸� Swap
		cv::Mat dstImg;

		cv::transpose(srcImg, dstImg);
		cv::flip(dstImg, dstImg, 0);	// �¿� ����
		pMain->copyMemory(pMain->getSrcBuffer(m_nCam), dstImg.data, nSize);
		dstImg.release();
	}
	else if (dir == 3)	// 180��
	{
		cv::Mat srcImg(h, w, CV_8UC1, imgdata);
		cv::Mat dstImg;

		cv::flip(srcImg, dstImg, 0);	// ���� ����
		cv::flip(dstImg, dstImg, 1);	// �¿� ����

		pMain->copyMemory(pMain->getSrcBuffer(m_nCam), dstImg.data, nSize);
		dstImg.release();
	}
	else
	{
		pMain->copyMemory(pMain->getSrcBuffer(m_nCam), imgdata, nSize);
	}

	if (!pMain->m_bGrabEnd[m_nCam] && (pMain->m_nCurPane == PANE_AUTO ||
		(pMain->m_nCurForm == FORM_MAIN || pMain->m_nCurForm == FORM_MACHINE || pMain->m_nCurForm == FORM_INTERFACE)))	//210119
	{
		pMain->m_bGrabEnd[m_nCam] = TRUE;
	}	

	
	if (m_bSaveMPEG)
	{
		//KJH 2022-05-31 ��ȭ�� Release�ȵǼ� ����
		cv::Mat videoFrame_Org(h, w, CV_8UC1, pMain->getSrcBuffer(m_nCam));
		cv::Mat videoFrame = videoFrame_Org.clone();

		if (videoFrame.empty()) 
		{
			m_videoWriter.release();
			ReleaseSRWLockExclusive(&g_bufsrwlock);
			return ;
		}

		if (m_videoWriter.isOpened())
		{
			//m_videoWriter.write(videoFrame);
			m_videoWriter << videoFrame;
		}
		//HSJ Frame ���� ��� �߰�
		videoFrame_Org.release();
		videoFrame.release();
	}

	ReleaseSRWLockExclusive(&g_bufsrwlock);
}

void CDahuaCameraHelper::cvVideoOpen(CString fname, int w, int h)
{
	if (m_bSaveMPEG) cvVideoClose();

	m_videoName = fname;
	//HSJ ī�޶󸶴� Frame�� �ٸ��Ƿ� ������ ����
	CSystem& systemObj = CSystem::getInstance();
	IAcquisitionControlPtr sptrAcquisitionControl = systemObj.createAcquisitionControl(m_SPtrCamera);
	CDoubleNode acquisitionFrameRate = sptrAcquisitionControl->acquisitionFrameRate();

	double nFrame=10;
	acquisitionFrameRate.getValue(nFrame);

	Delay(50);

	m_videoWriter.open((LPCTSTR)fname, CV_FOURCC('X', 'V', 'I', 'D'), nFrame, cv::Size(w, h), false);
	m_bSaveMPEG = true;
}

void CDahuaCameraHelper::cvVideoClose()
{
	if (m_videoWriter.isOpened()) m_videoWriter.release();
	m_bSaveMPEG = false;
}

#endif