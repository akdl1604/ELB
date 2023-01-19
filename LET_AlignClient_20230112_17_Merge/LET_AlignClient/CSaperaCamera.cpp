#include "stdafx.h"
#include "CSaperaCamera.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <atomic>

static bool g_CameraOpen[4] = { 0, };

#ifdef _SAPERA
void transferCallback(SapXferCallbackInfo* info)
{
    auto context = (TransferContext*)info->GetContext();

    context->frameGrabCount++;
    if (!info->IsTrash()) {
        CSaperaCamera* pCamera = (CSaperaCamera*)context->pCamera;
        pCamera->Callback(info);
        context->frameProcessingCount++;
       // context->processing->ExecuteNext();
    }
}

void processingCallback(SapProCallbackInfo* info)
{
    auto context = (TransferContext*)info->GetContext();
    context->frameProcessingCount++;
}

SapAcqDevice* getDeviceBySN(const std::string& sn,int index)
{
    char serverName[MAX_PATH] = { 0, };
    char serialNumberName[2048] = { 0, };

    const int serverCount = SapManager::GetServerCount();
    for (int i = 0; i < serverCount; i++) {
        if (SapManager::GetResourceCount(i, SapManager::ResourceAcqDevice) != 0)
        {
            if (g_CameraOpen[i] == true) continue;

            SapManager::GetServerName(i, serverName, sizeof(serverName));
            SapAcqDevice*  camera = new SapAcqDevice(serverName);

            try {
                if (!camera->Create()) {
                    AfxMessageBox("Failed to create camera object.");
                }
                else
                {
                    int featureCount;
                    if (camera->GetFeatureCount(&featureCount) && featureCount > 0)
                    {
                        if (camera->GetFeatureValue("DeviceID", serialNumberName, sizeof(serialNumberName))
                            && serialNumberName == sn)
                        {
                            g_CameraOpen[i] = true;
                            return camera;
                        }
                    }
                }
            }
            catch (...)
            {

            }
            camera->Destroy();
            delete camera;
        }
    }

    return NULL;
}

SapMyProcessing::SapMyProcessing(SapBuffer* pBuffers, SapProCallback pCallback, void* pContext)
    : SapProcessing(pBuffers, pCallback, pContext)
{}

SapMyProcessing::~SapMyProcessing()
{
    if (m_bInitOK) Destroy();
}

BOOL SapMyProcessing::Run()
{
    SapBuffer::State state;
    const int proIndex = GetIndex();    
    bool goodContent = m_pBuffers->GetState(proIndex, &state)  && (state == SapBuffer::StateFull);

    if (goodContent) {
        void* inAddress = nullptr;       
        int inSize = 0;
        m_pBuffers->GetAddress(proIndex, &inAddress);
        m_pBuffers->GetSpaceUsed(proIndex, &inSize);

        const int width = m_pBuffers->GetWidth();
        const int height = m_pBuffers->GetHeight();
        const auto format = m_pBuffers->GetFormat();
        const int outSize = width * height;

        goodContent = (format == SapFormatMono8) && (inSize == outSize);

        if (goodContent) 
        {
            auto context = (TransferContext*)GetContext();
            CLET_AlignClientDlg* pMain = (CLET_AlignClientDlg*)context->pDlg;
            CSaperaCamera* pCamera = (CSaperaCamera*)context->pCamera;
       
            BYTE* dstFullImg = pCamera->GetFullImage()->data;

            if (pMain->m_bProgramEnd)	return 0;
            int nCam = context->cam_id;
            int dir = pMain->m_stCamInfo[nCam].flip_dir;

            if (dir == 1)	// 90도
            {
                cv::Mat srcImg(width , height, CV_8UC1, (uint8_t*)inAddress);	// Width / Height를 미리 Swap
                cv::Mat dstImg;

                cv::transpose(srcImg, dstImg);

                pMain->copyMemory(pMain->getSrcBuffer(nCam), dstImg.data, outSize);
                dstImg.release();
            }
            else if (dir == 2)	// 270도
            {
                cv::Mat srcImg(width , height, CV_8UC1, (uint8_t*)inAddress);	// Width / Height를 미리 Swap
                cv::Mat dstImg;

                cv::transpose(srcImg, dstImg);
                cv::flip(dstImg, dstImg, 1);	// 좌우 반전
                pMain->copyMemory(pMain->getSrcBuffer(nCam), dstImg.data, outSize);
                dstImg.release();
            }
            else if (dir == 3)	// 180도
            {
                cv::Mat srcImg(height, width, CV_8UC1, (uint8_t*)inAddress);
                cv::Mat dstImg;

                cv::flip(srcImg, dstImg, 0);	// 상하 반전
                cv::flip(dstImg, dstImg, 1);	// 좌우 반전
                pMain->copyMemory(pMain->getSrcBuffer(nCam), dstImg.data, outSize);
                dstImg.release();
            }
            else
            {
                pMain->copyMemory(pMain->getSrcBuffer(nCam), (uint8_t*)inAddress, outSize);
            }

            if (pMain->m_nCurPane == PANE_AUTO ||
                (pMain->m_nCurForm == FORM_MAIN || pMain->m_nCurForm == FORM_MACHINE || pMain->m_nCurForm == FORM_INTERFACE))	//210119
            {
                pMain->m_bGrabEnd[nCam] = TRUE;

                if (context->frameProcessingCount < 10)
                {
                    memcpy(dstFullImg + (context->frameProcessingCount * outSize), pMain->getSrcBuffer(nCam), outSize);
                    pCamera->SetGrabEndFlag(context->frameProcessingCount,TRUE);
                }
            }
        }
    }

    return TRUE;
}

void CSaperaCamera::Callback(SapXferCallbackInfo* pInfo)
{
    ++m_nGrabCnt;  // dh.jung 2021-05-14 add

    SapBuffer::State state;
    const int proIndex = m_Buffers->GetIndex();
    bool goodContent = m_Buffers->GetState(proIndex, &state) && (state == SapBuffer::StateFull);

    if (goodContent) {
        void* inAddress = nullptr;
        int inSize = 0;
        m_Buffers->GetAddress(proIndex, &inAddress);
        m_Buffers->GetSpaceUsed(proIndex, &inSize);

        const int width = m_Buffers->GetWidth();
        const int height = m_Buffers->GetHeight();
        const auto format = m_Buffers->GetFormat();
        const int outSize = width * height;

        goodContent = (format == SapFormatMono8) && (inSize == outSize);

        if (goodContent)
        {
            auto context = (TransferContext*)&m_Gcontext;
            CLET_AlignClientDlg* pMain = (CLET_AlignClientDlg*)context->pDlg;
            CSaperaCamera* pCamera = (CSaperaCamera*)context->pCamera;

            BYTE* dstFullImg = pCamera->GetFullImage()->data;

            if (pMain->m_bProgramEnd)	return ;
            int nCam = context->cam_id;
            int dir = pMain->m_stCamInfo[nCam].flip_dir;

            CString str;
            str.Format("Cam %d - %d Grab End", nCam, m_nGrabCnt + 1);
            ::SendMessageA(pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);


            if (dir == 1)	// 90도
            {
                cv::Mat srcImg(width, height, CV_8UC1, (uint8_t*)inAddress);	// Width / Height를 미리 Swap
                cv::Mat dstImg;

                cv::transpose(srcImg, dstImg);

                pMain->copyMemory(pMain->getSrcBuffer(nCam), dstImg.data, outSize);
                dstImg.release();
            }
            else if (dir == 2)	// 270도
            {
                cv::Mat srcImg(width, height, CV_8UC1, (uint8_t*)inAddress);	// Width / Height를 미리 Swap
                cv::Mat dstImg;

                cv::transpose(srcImg, dstImg);
                cv::flip(dstImg, dstImg, 1);	// 좌우 반전
                pMain->copyMemory(pMain->getSrcBuffer(nCam), dstImg.data, outSize);
                dstImg.release();
            }
            else if (dir == 3)	// 180도
            {
                cv::Mat srcImg(height, width, CV_8UC1, (uint8_t*)inAddress);
                cv::Mat dstImg;

                cv::flip(srcImg, dstImg, 0);	// 상하 반전
                cv::flip(dstImg, dstImg, 1);	// 좌우 반전
                pMain->copyMemory(pMain->getSrcBuffer(nCam), dstImg.data, outSize);
                dstImg.release();
            }
            else
            {
                pMain->copyMemory(pMain->getSrcBuffer(nCam), (uint8_t*)inAddress, outSize);

            }

            if (pMain->m_nCurPane == PANE_AUTO ||
                (pMain->m_nCurForm == FORM_MAIN || pMain->m_nCurForm == FORM_MACHINE || pMain->m_nCurForm == FORM_INTERFACE))	//210119
            {
                pMain->m_bGrabEnd[nCam] = TRUE;

                if (context->frameProcessingCount < 10)
                {
                    memcpy(dstFullImg + (context->frameProcessingCount * outSize), pMain->getSrcBuffer(nCam), outSize);
                    pCamera->SetGrabEndFlag(context->frameProcessingCount, TRUE);
                }
            }

            int iframeProcCnt = context->frameProcessingCount;
            TRACE("m_nGrabCnt : %d \n", m_nGrabCnt);
            TRACE("frameProcessingCount : %d \n", iframeProcCnt);
        }
    }
    else
    { // dh.jung 2021-05-14 add
        TRACE("Camera State Full or State is not good!! \n");
    }
}
#endif

CSaperaCamera::CSaperaCamera()
{
#ifdef _SAPERA
    m_AcqDevice = NULL;
    m_Buffers = NULL;
    m_Xfer = NULL;    
#endif

    m_IsOpen = FALSE;
    m_bRecordOn = FALSE;
    m_bTriggerMode = FALSE;

    for(int i=0;i< _CAM_MAX_GRAB_COUNT;i++)
        m_bCamGrabEnd[i]=FALSE;

    m_nGrabCnt = 0;
}

CSaperaCamera::~CSaperaCamera(void)
{
#ifdef _SAPERA
    //DestroyObjects();

    //if (m_Xfer)       delete m_Xfer;    
    //if (m_AcqDevice)  delete m_AcqDevice;
    //if (m_Buffers)    delete m_Buffers;
    ReleaseSap();

#endif

    if (!m_matFullImage.empty())   m_matFullImage.release();
}

void CSaperaCamera::ReleaseSap(void)
{ // dh.jung 2021-05-12 add
#ifdef _SAPERA
    DestroyObjects();

    if (m_Xfer)       delete m_Xfer;    
    if (m_AcqDevice)  delete m_AcqDevice;
    if (m_Buffers )   delete m_Buffers;

    m_AcqDevice = NULL;
    m_Buffers = NULL;
    m_Xfer = NULL;
    
#endif
    m_IsOpen = FALSE;
    m_bRecordOn = FALSE;
    m_bTriggerMode = FALSE;

    for (int i = 0;i < _CAM_MAX_GRAB_COUNT;i++)
        m_bCamGrabEnd[i] = FALSE;
}

int CSaperaCamera::CreateImageFullBuffer(CSize size,int nCount)
{
    if (!m_matFullImage.empty())   m_matFullImage.release();

    m_matFullImage = cv::Mat::zeros(size.cy* nCount, size.cx, CV_8UC1);

    m_szImage = size;

    return 0;
}

void CSaperaCamera::ClearFullBuffer()
{
    m_matFullImage.setTo(0);

#ifdef _SAPERA
   // m_Gcontext.processing->Init();
    m_Gcontext.frameGrabCount = 0;
    m_Gcontext.frameProcessingCount = 0;
#endif
    for (int i = 0; i < _CAM_MAX_GRAB_COUNT; i++)
        m_bCamGrabEnd[i] = FALSE;
}

BOOL CSaperaCamera::CreateObjects(const std::string& serialNumber,int index)
{
    BOOL br = FALSE;

#ifdef _SAPERA
    // dh.jung 2021-05-12 add
    if (m_AcqDevice != NULL)
    {
        ReleaseSap();
    }
    // end
    
    if (m_AcqDevice == NULL) 
    {
        m_AcqDevice = getDeviceBySN(serialNumber,index);
        if (m_AcqDevice != NULL)
        {
            m_Buffers = new SapBufferWithTrash(MAX_BUFFER, m_AcqDevice);
            m_Xfer = new SapAcqDeviceToBuf(m_AcqDevice, m_Buffers, transferCallback, &m_Gcontext);

          //  m_Gcontext.processing = std::make_shared<SapMyProcessing>(m_Buffers, processingCallback, &m_Gcontext);
        }
        else return FALSE;
    }

    if (m_AcqDevice && !*m_AcqDevice)
    {
        DestroyObjects();
        return FALSE;
    }

    if (m_Buffers && !*m_Buffers)
    {
        if (!m_Buffers->Create())
        {
            DestroyObjects();
            return FALSE;
        }
        m_Buffers->Clear();
    }

    if (m_Xfer && !*m_Xfer)
    {
        try {
            if (!m_Xfer->Create())
            {
                DestroyObjects();
                return FALSE;
            }
           /* if (!m_Gcontext.processing->Create()) {
                AfxMessageBox("Failed to create processing object.");
            }*/
            m_Xfer->SetAutoEmpty(true);
           /* m_Gcontext.processing->SetAutoEmpty(true);
            m_Gcontext.processing->Init();    */       
        }
        catch (...) 
        {

        }
    }
    br = TRUE;
#endif
    return br;
} 

BOOL CSaperaCamera::DestroyObjects(void)
{
#ifdef _SAPERA
   // if (m_Gcontext.processing) m_Gcontext.processing->Destroy();
    if (m_Xfer && *m_Xfer) m_Xfer->Destroy();
    if (m_Buffers && *m_Buffers) m_Buffers->Destroy();
    if (m_AcqDevice && *m_AcqDevice) m_AcqDevice->Destroy();
#endif
    return TRUE;
} 

void CSaperaCamera::SingleGrab()
{
#ifdef _SAPERA
    if (!m_Xfer || !m_Buffers || !m_AcqDevice) return;

    try {
		m_Xfer->Init(TRUE);
	    m_Buffers->ResetIndex(); // Grab Buffer Reset
	
        if (m_Xfer->Snap())
        {
            if (!m_Xfer->Wait(5000))
                m_Xfer->Abort();
        }
    }
    catch (...) 
    {
    
    }
#endif
}

void CSaperaCamera::grab(int maxFrameCount)
{
#ifdef _SAPERA
    if (!m_Xfer || !m_Buffers || !m_AcqDevice) return;

    try {
		
		if (maxFrameCount == -1)  // dh.jung 2021-05-12
        {
            if (m_bRecordOn) return;
		}
		
		m_Xfer->Init(TRUE);                                 // dh.jung 2021-05-12 del
	    m_Buffers->ResetIndex(); // Grab Buffer Reset       // dh.jung 2021-05-12 del
		
        if (maxFrameCount == -1)
        { 
            m_Xfer->Grab();
            m_bRecordOn = TRUE;
        }
        else
            m_Xfer->Snap(maxFrameCount);
    }
    catch (...) {
        throw;
    }
#endif
}

void CSaperaCamera::stop()
{
#ifdef _SAPERA
    if (!m_Xfer || !m_Buffers || !m_AcqDevice) return;

    try {
        if (m_bRecordOn)
        {

            m_Xfer->Freeze();
            if (!m_Xfer->Wait(5000))
                return;

            m_bRecordOn = FALSE;

            m_nGrabCnt = 0; 
        }
    }
    catch (...) {
        throw;
    }
#endif
}

void CSaperaCamera::SwTrigger(bool bline, int ncount)
{
#ifdef _SAPERA
    if (m_AcqDevice != NULL)
    {
        if(bline) m_AcqDevice->SetFeatureValue("TriggerLineCount", ncount);
        else m_AcqDevice->SetFeatureValue("TriggerLineCount", 1);

        m_AcqDevice->SetFeatureValue("TriggerSoftware", TRUE);
    }
#endif
}

void CSaperaCamera::SwTriggerSource(bool bsw)
{
#ifdef _SAPERA
    if (m_AcqDevice != NULL)
    {
        if(bsw)        m_AcqDevice->SetFeatureValue("TriggerSource", "Software");
        else      m_AcqDevice->SetFeatureValue("TriggerSource", "Hardware");
    }
#endif
}

void CSaperaCamera::TriggerSelector(bool bFrame)
{
#ifdef _SAPERA
    if (m_AcqDevice != NULL)
    {        
        if (bFrame)        m_AcqDevice->SetFeatureValue("TriggerSelector", "FrameStart");
        else      m_AcqDevice->SetFeatureValue("TriggerSelector", "LineStart");
    }
#endif
}

void CSaperaCamera::setImageSize(CSize size) 
{
#ifdef _SAPERA
    if (m_AcqDevice != NULL)
    {
        m_szImage = size;
        m_AcqDevice->SetFeatureValue("Width", size.cx);
        m_AcqDevice->SetFeatureValue("Height", size.cy);
    }
#endif   
}

void CSaperaCamera::setTriggerMode(bool bTriggerMode)
{
#ifdef _SAPERA
    if (m_AcqDevice != NULL)
    {
        m_bTriggerMode = bTriggerMode ? TRUE : FALSE;
        m_AcqDevice->SetFeatureValue("TriggerMode", bTriggerMode?"On":"Off");
    }
#endif
}
void CSaperaCamera::setExposure(double nExposure)
{
#ifdef _SAPERA
    if (m_AcqDevice != NULL)
    {
        m_AcqDevice->SetFeatureValue("ExposureTime", nExposure);
    }
#endif
}
void CSaperaCamera::setGain(double nGain)
{
#ifdef _SAPERA
    if (m_AcqDevice != NULL)
    {
        m_AcqDevice->SetFeatureValue("Gain", nGain);
    }
#endif
}

void CSaperaCamera::setLineRate(double nLinerate)
{
#ifdef _SAPERA
    if (m_AcqDevice != NULL)
    {
        m_AcqDevice->SetFeatureValue("AcquisitionLineRate", nLinerate);
    }
#endif
}

void CSaperaCamera::cvVideoOpen(CString fname, int w, int h)
{// dh.jung 2021-05-20 add video
#ifdef _SAPERA
    fileCnt = 0;
    frameCnt = 0;
    m_videoName = fname;
    m_videoWriter.open((LPCTSTR)fname, CV_FOURCC('X', 'V', 'I', 'D'), 10, cv::Size(w, h), false);
    m_bSaveMPEG = true;
#endif
}

void CSaperaCamera::cvVideoClose()
{// dh.jung 2021-05-20 add video
#ifdef _SAPERA
    if (m_bSaveMPEG)
    {
        //HSJ 2021-06-18 add stop
        stop();
        m_videoWriter.release();
        m_bSaveMPEG = false;
    }
#endif
}