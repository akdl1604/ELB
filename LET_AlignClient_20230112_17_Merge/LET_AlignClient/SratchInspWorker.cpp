#include "stdafx.h"
#include "SratchInspWorker.h"
#include "LET_AlignClient.h"
#include "FormMainView.h"
#include "LET_AlignClientDlg.h"
#include "ImageProcessing/MathUtil.h"
#include "CommPLC.h"
#include "ComPLC_RS.h"

CSratchInspWorker::CSratchInspWorker(void)
{
	m_pMainForm = NULL;
	m_pMain = NULL;
	InitializeCriticalSection(&m_csScratchWrite);
}

CSratchInspWorker::~CSratchInspWorker(void)
{
	DeleteCriticalSection(&m_csScratchWrite);
}

void CSratchInspWorker::InspectionEnhance(cv::Mat* src, int id)		// 0 : All, 1 : Y, 2 : X
{
	cv::Mat m_KrnlGusX[3];
	cv::Mat m_KrnlLOGX[3];
	cv::Mat m_KrnlGusY[3];
	cv::Mat m_KrnlLOGY[3];

	cv::Mat m_Proc1Buf;
	cv::Mat m_Proc2Buf;
	cv::Mat m_Proc3Buf;
	cv::Mat m_TempoBuf;

	float nKernelGUS[12] = { 10, 18, 29, 43, 57, 67, 71, 67, 57, 43, 29, 18 }; //509
	float nKernelGUS2[12] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }; // 12
	float nKernelLOG[24] = { -1, -2, -4, -9, -15, -20, -22, -16, 0, 22, 42, 50, 42, 22, 0, -16, -22, -20, -15, -9, -4, -2, -1, 0 }; //32

	m_KrnlGusX[0] = cv::Mat(12, 1, CV_32FC1, nKernelGUS) / 509.f;
	m_KrnlLOGX[0] = cv::Mat(24, 1, CV_32FC1, nKernelLOG) / 32.f;
	m_KrnlGusY[0] = cv::Mat(1, 12, CV_32FC1, nKernelGUS) / 509.f;
	m_KrnlLOGY[0] = cv::Mat(1, 24, CV_32FC1, nKernelLOG) / 32.f;

	m_KrnlLOGX[1] = cv::Mat(24, 1, CV_32FC1, nKernelLOG) / 32.f;
	m_KrnlGusY[1] = cv::Mat(1, 12, CV_32FC1, nKernelGUS2) / 12.f;

	m_KrnlGusX[2] = cv::Mat(12, 1, CV_32FC1, nKernelGUS2) / 12.f;
	m_KrnlLOGY[2] = cv::Mat(1, 24, CV_32FC1, nKernelLOG) / 32.f;

	int w = src->cols;
	int h = src->rows;

	m_Proc1Buf = cv::Mat::zeros(h, w, CV_32FC1);
	m_Proc2Buf = cv::Mat::zeros(h, w, CV_32FC1);
	m_Proc3Buf = cv::Mat::zeros(h, w, CV_32FC1);
	m_TempoBuf = cv::Mat::zeros(h, w, CV_32FC1);

	cv::Mat dst;
	cv::Mat element(3, 3, CV_8U, cv::Scalar(1));
	cv::Mat srcImage;

	src->copyTo(srcImage);
	src->convertTo(m_TempoBuf, CV_32F, 1., -128.);

	if (id == 0)	// 양방향 Enhance
	{
		cv::sepFilter2D(m_TempoBuf, m_Proc1Buf, CV_32F, m_KrnlGusX[0], m_KrnlLOGY[0], cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
		cv::sepFilter2D(m_TempoBuf, m_Proc2Buf, CV_32F, m_KrnlLOGX[0], m_KrnlGusY[0], cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
		cv::add(m_Proc1Buf, m_Proc2Buf, m_Proc3Buf);
	}
	else if (id == 1)
	{
		// 단방향 Enhance
		cv::sepFilter2D(m_TempoBuf, m_Proc3Buf, CV_32F, m_KrnlGusX[0], m_KrnlLOGY[0], cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
	}
	else
	{
		// 단방향 Enhance
		cv::sepFilter2D(m_TempoBuf, m_Proc3Buf, CV_32F, m_KrnlLOGX[1], m_KrnlGusY[1], cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
	}

	m_Proc3Buf.convertTo(*src, CV_8U, 1., 128.);

	for (int i = 0; i < 3; i++)
	{
		m_KrnlGusX[i].release();
		m_KrnlLOGX[i].release();
		m_KrnlGusY[i].release();
		m_KrnlLOGY[i].release();
	}

	m_Proc1Buf.release();
	m_Proc2Buf.release();
	m_Proc3Buf.release();
	m_TempoBuf.release();
	dst.release();
	element.release();
	srcImage.release();
}

// 20220928 스크레치 검사 추가
BOOL CSratchInspWorker::Inspection_ScratchHole(int nJob, int id, BYTE* ImgByte, CViewerEx* mViewer, bool bSimul)
{
	BOOL bReturn = TRUE;

	system_clock::time_point start = system_clock::now();

	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;

	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
	{
		cvHideImage("Enhance Image");
	}

	CFormMainView* pFormMain = (CFormMainView*)m_pMainForm;
	CString str = "NULL";
	int ncamera = camBuf.at(0);
	int W = m_pMain->m_stCamInfo[ncamera].w;
	int H = m_pMain->m_stCamInfo[ncamera].h;
	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);
	double cir_size = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getScratchMaskingRadius();
	int radius = int(cir_size / xres);
	int inspAreaSpec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getScratchInspAreaSpec();
	int roiOffset = (radius + inspAreaSpec) / 8 * 8; // 반지름

	cv::Mat gray;
	cv::Mat imgSrc(H, W, CV_8UC1, ImgByte);

	CRect InspROI = CRect(W / 2 - 1000, H / 2 - 1000, W / 2 + 1000, H / 2 + 1000);

	cv::Mat tmpimg(200, 200, CV_8UC1, cv::Scalar(0));
	cv::circle(tmpimg, cv::Point(100, 100), radius, Scalar(255, 255, 255), -1, 8);

	FakeDC* pDC = NULL;
	int nview = m_pMain->vt_job_info[nJob].viewer_index[id];

	if (mViewer != NULL)
	{
		pDC = mViewer->getOverlayDC();
		mViewer->ClearOverlayDC();
		mViewer->clearAllFigures();
	}
	else
	{
		pDC = pFormMain->m_pDlgViewerMain[nview]->GetViewer().getOverlayDC();
		pFormMain->m_pDlgViewerMain[nview]->GetViewer().ClearOverlayDC();
		pFormMain->m_pDlgViewerMain[nview]->GetViewer().clearAllFigures();
	}

	// hsj 2022-10-31 image draw
	pFormMain->m_pDlgViewerMain[nview]->GetViewer().OnLoadImageFromPtr(m_pMain->getProcBuffer(ncamera, 2));

	CPen penNG(PS_SOLID, 5, COLOR_GREEN);
	CPen penOK(PS_SOLID, 5, COLOR_GREEN);
	CPen penROI(PS_SOLID, 3, COLOR_WHITE);

	pDC->SelectObject(&penROI);

	pDC->Rectangle(CRect(InspROI.left, InspROI.top, InspROI.right, InspROI.bottom));
	// 디버깅 모드에서 이미지 보기
	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
	{
		cvShowImage("Scratch Insp Org Image", tmpimg);
	}

	BYTE* pImage;
	if (bSimul == TRUE) pImage = m_pMain->getCameraViewBuffer();
	else pImage = m_pMain->getProcBuffer(ncamera, 0);

	m_pMain->GetMatching(nJob).setSearchROI(0, 2, InspROI);
	m_pMain->GetMatching(nJob).findPattern(pImage, 0, 2, W, H);
	BOOL bFind = m_pMain->GetMatching(nJob).getFindInfo(0, 2).GetFound() == FIND_OK ? TRUE : FALSE;

	//m_pMain->GetMatching(nJob).getHalcon(0, 2, 0).m_ReadSmallScale = 0.3;
	//m_pMain->GetMatching(nJob).getHalcon(0, 2, 0).m_SearchSubPixel = "least_squares";
	//m_pMain->GetMatching(nJob).getHalcon(0, 2, 0).halcon_ReadScaleModel(tmpimg.data, NULL, tmpimg.cols, tmpimg.rows);
	//bFind = m_pMain->GetMatching(nJob).getHalcon(0, 2, 0).halcon_SearchScaleMark(imgSrc.data, W, H, InspROI, 80.);

	double max_blob_width = 0.0;
	double max_blob_height = 0.0;

	if (bFind)
	{
		int dThresh = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getScratchThresh();
		//double dbPosX = m_pMain->GetMatching(nJob).getHalcon(0, 2, 0).halcon_GetResultPos().x + InspROI.left;
		//double dbPosY = m_pMain->GetMatching(nJob).getHalcon(0, 2, 0).halcon_GetResultPos().y + InspROI.top;

		double dbPosX = m_pMain->GetMatching(nJob).getFindInfo(0, 2).GetXPos();
		double dbPosY = m_pMain->GetMatching(nJob).getFindInfo(0, 2).GetYPos();

		cv::Mat Mask2, Mask, iproc;
		cv::Rect _Roi = cv::Rect(dbPosX - roiOffset, dbPosY - roiOffset, roiOffset * 2, roiOffset * 2);

		_Roi.x = MIN(W - 1, MAX(0, _Roi.x));
		_Roi.y = MIN(H - 1, MAX(0, _Roi.y));
		_Roi.width = MIN(W - _Roi.x, MAX(0, _Roi.width));
		_Roi.height = MIN(H - _Roi.y, MAX(0, _Roi.height));


		if (_Roi.width > 0 && _Roi.height > 0)
		{
			gray = imgSrc(_Roi).clone();
			Mask2 = GetCirclemask(gray, gray.cols / 2, gray.rows / 2, radius + 12);
			bitwise_not(Mask2, Mask2);

			GaussianBlur(gray, gray, cv::Size(0, 0), 3.);
			InspectionEnhance(&gray, 0);

			if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
			{
				cvShowImage("Enhance Image", gray);
			}

			threshold(gray, iproc, dThresh, 255, CV_THRESH_BINARY);
			bitwise_and(Mask2, iproc, Mask);

			if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
			{
				cvShowImage("Enhance Bin Image", Mask);
			}
			/// <summary>

			CMBlob m_Blob;
			m_Blob.Create(1000, 1500, 1500);

			m_Blob.InitValue();
			m_Blob.SetImgInfo(gray.cols, gray.rows, 1, gray.cols);
			m_Blob.SetRoi(CRect(4, 4, gray.cols - 4, gray.rows - 4));

			m_Blob.BlobScan((LPBYTE)Mask.data);
			m_Blob.ExtractBlob();

			int nPtCnt = m_Blob.GetBlobNum();
			int nRealDefectCount = 0;
			/// <param name="mViewer"></param>

			pDC->SelectObject(&penOK);

			pDC->Ellipse(dbPosX - radius, dbPosY - radius, dbPosX + radius, dbPosY + radius);

			Gdiplus::Color color = 0xffff0000;

			pDC->SelectObject(&penNG);

			PSTBLOB lpCurBlob;
			std::vector<PSTBLOB> vtBlob;
			CRect roi;
			int pxlSizeSpec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getScratchInspSpec();
			double min_size = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspMinSize();

			for (int i = 1; i < nPtCnt; i++)
			{
				lpCurBlob = m_Blob.GetBlob(i);
				if (-1 == lpCurBlob->state) continue;

				roi.left = lpCurBlob->left + _Roi.x;
				roi.top = (gray.rows - lpCurBlob->bottom) + _Roi.y;
				roi.right = lpCurBlob->right + _Roi.x;
				roi.bottom = (gray.rows - lpCurBlob->top) + _Roi.y;

				pDC->Rectangle(CRect(roi.left, roi.top, roi.right, roi.bottom));

				vtBlob.push_back(m_Blob.GetBlob(i));

				if (int(lpCurBlob->pxlSize) > pxlSizeSpec && (min_size < (xres * roi.Width()) || min_size < (yres * roi.Height())))
				{
					nRealDefectCount++;
					if (mViewer != NULL)	mViewer->AddSoftGraphic(new GraphicRectangle(roi.left, roi.top, roi.right, roi.bottom, Gdiplus::Color(color)));
					else pDC->Rectangle(CRect(roi.left, roi.top, roi.right, roi.bottom));
				}
				else										continue;

				max_blob_width = MAX(max_blob_width, fabs(lpCurBlob->right - lpCurBlob->left));
				max_blob_height = MAX(max_blob_height, fabs(lpCurBlob->bottom - lpCurBlob->top));
			}

			if (mViewer != NULL)				mViewer->Invalidate();
			else								pFormMain->m_pDlgViewerMain[nview]->GetViewer().Invalidate();

			if (m_pMain->vt_job_info[nJob].num_of_viewer > 1)
			{
				int nview2 = m_pMain->vt_job_info[nJob].viewer_index[1];

				_stLoadScratchImage pParam;
				pParam.pDlg = m_pMain;
				pParam.nJob = nJob;
				pParam.ImgByte = pImage;
				pParam.viewerDlg = pFormMain->m_pDlgViewerMain[nview2];
				for (int i = 0; i < 10; i++)
				{
					if (i < vtBlob.size())	pParam.vtBlob[i] = vtBlob.at(i);
					else					pParam.vtBlob[i] = 0;
				}

				AfxBeginThread(Inspection_ScratchHole_ProcImage, &pParam);
			}

			if (nRealDefectCount >= 1)
			{
				//cvShowImage("Enhance Image", gray);

#pragma region Defect 저장 

				CString	Time_str = m_pMain->m_strResultTime[nJob];
				CString	Date_str = m_pMain->m_strResultDate[nJob];
				CString	real_Time_str, str_JobName;

				CTime NowTime;
				NowTime = CTime::GetCurrentTime();
				real_Time_str.Format("%04d%02d%02d", NowTime.GetYear(), NowTime.GetMonth(), NowTime.GetDay());

				if (Time_str == "" || Date_str == "")
				{
					m_pMain->m_strResultDate[nJob].Format("%04d%02d%02d", NowTime.GetYear(), NowTime.GetMonth(), NowTime.GetDay());
					m_pMain->m_strResultTime[nJob].Format("%02d_%02d_%02d", NowTime.GetHour(), NowTime.GetMinute(), NowTime.GetSecond());

					Time_str = m_pMain->m_strResultTime[nJob];
					Date_str = m_pMain->m_strResultDate[nJob];
				}

				CString Cell_Name = m_pMain->vt_job_info[nJob].get_main_object_id();
				if (strlen(Cell_Name) < 10)					Cell_Name.Format("Test_%s", Time_str);


				CString strFileDir_Image, strFileDir_Image_Proc, strFileDir_Image_Raw;

				strFileDir_Image = m_pMain->m_strImageDir + Date_str;
				if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);
				strFileDir_Image.Format("%s\\%s", strFileDir_Image, m_pMain->m_strCurrentModelName);
				if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

				//JOB이름
				str_JobName.Format("%s", m_pMain->vt_job_info[nJob].job_name.c_str());
				strFileDir_Image.Format("%s\\%s", strFileDir_Image, str_JobName);
				if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

				strFileDir_Image.Format("%s\\SCRATCH_INSP\\", strFileDir_Image);
				if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

				strFileDir_Image.Format("%s%s\\", strFileDir_Image, Cell_Name);
				if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

				strFileDir_Image_Raw.Format("%s%s_%s_Raw.jpg", strFileDir_Image, Cell_Name, Time_str);
				strFileDir_Image_Raw.Format("%s%s_%s_%sImgRaw.jpg", strFileDir_Image, Cell_Name, Time_str, m_pMain->m_stCamInfo[ncamera].cName);

				if (_access(strFileDir_Image_Proc, 0) != 0) strFileDir_Image_Proc.Format("%s%s_%s_Proc_Img.jpg", strFileDir_Image, Cell_Name, real_Time_str);
				if (_access(strFileDir_Image_Raw, 0) != 0)	strFileDir_Image_Raw.Format("%s%s_%s_%s_ImgRaw.jpg", strFileDir_Image, Cell_Name, real_Time_str, m_pMain->m_stCamInfo[ncamera].cName);

				if (m_pMain->vt_job_info[nJob].num_of_viewer == 0)
					cv::imwrite(std::string(strFileDir_Image_Proc), gray);
				cv::imwrite(std::string(strFileDir_Image_Raw), imgSrc);

				CString strFileDir, strTemp;

				strFileDir.Format("%sFiles\\", m_pMain->m_strResultDir);
				if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);
				strFileDir.Format("%s%s\\", strFileDir, Date_str);
				if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);
				strFileDir.Format("%s%s\\", strFileDir, m_pMain->m_strCurrentModelName);
				if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);
				strFileDir.Format("%s%s\\", strFileDir, str_JobName);
				if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);

				if (m_pMain->m_bSimulationStart)	strFileDir.Format("%ssimulation_", strFileDir);
				strTemp.Format("%s%s_Result_%s.csv", strFileDir, str_JobName, Date_str);

				BOOL bWriteHeader = FALSE;
				if ((_access(strTemp, 0)) == -1)
				{
					bWriteHeader = TRUE;
				}

				CStdioFile WriteFile;
				if (WriteFile.Open(strTemp, CFile::modeNoTruncate | CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone | CFile::typeText) != TRUE)
				{
					CString str;
					str.Format("Fail to Open Logfile !! : %s", strTemp.Mid(strTemp.ReverseFind('\\') + 1));
					::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
					return 0;
				}

				CString strHeader, strBody;

				strTemp.Format("TIME");																					strHeader += strTemp;
				strTemp.Format("%s", Time_str);																			strBody += strTemp;

				strTemp.Format(",MODEL ID");																			strHeader += strTemp;
				strTemp.Format(",%s", m_pMain->m_strCurrentModelName);													strBody += strTemp;

				strTemp.Format(",PANEL ID");																			strHeader += strTemp;
				strTemp.Format(",%s", Cell_Name);																		strBody += strTemp;

				strTemp.Format(",Scratch Count");																		strHeader += strTemp;
				strTemp.Format(",%d", vtBlob.size());																	strBody += strTemp;

				strTemp.Format(",Max Size Width");																		strHeader += strTemp;
				strTemp.Format(",%.3f", max_blob_width * xres);															strBody += strTemp;

				strTemp.Format(",Max Size Height");																		strHeader += strTemp;
				strTemp.Format(",%.3f", max_blob_height * yres);														strBody += strTemp;

				strHeader += "\n";
				strBody += "\n";

				if (bWriteHeader)
				{
					WriteFile.WriteString(strHeader);
				}

				WriteFile.SeekToEnd();
				WriteFile.WriteString(strBody);
				WriteFile.Close();
#pragma endregion
				vtBlob.clear();

				bReturn = FALSE;
			}
			else bReturn = TRUE;
		}
	}
	else bReturn = FALSE;


	tmpimg.release();
	imgSrc.release();
	gray.release();

	penROI.DeleteObject();
	penNG.DeleteObject();
	penOK.DeleteObject();

	if (bSimul != TRUE && m_pMain->m_bSimulationStart != TRUE)
	{
		if (bReturn == TRUE) g_CommPLC.SetBit(m_pMain->m_nAlignRequestRead[nJob] + 2, TRUE);
		else				 g_CommPLC.SetBit(m_pMain->m_nAlignRequestRead[nJob] + 3, TRUE);
	}

	max_blob_width = max_blob_width * xres;
	max_blob_height = max_blob_height * yres;

	long MaxSizeData[4] = { 0, };
	MaxSizeData[0] = LOWORD((long)(max_blob_width * MOTOR_SCALE));
	MaxSizeData[1] = HIWORD((long)(max_blob_width * MOTOR_SCALE));
	MaxSizeData[2] = LOWORD((long)(max_blob_height * MOTOR_SCALE));
	MaxSizeData[3] = HIWORD((long)(max_blob_height * MOTOR_SCALE));

	if (bSimul != TRUE && m_pMain->m_bSimulationStart != TRUE)
	{
		int address = m_pMain->vt_job_info[nJob].plc_addr_info.write_word_start + 14;
		g_CommPLC.SetWord(address, 4, MaxSizeData);

		str.Format("Send Data Adress : %d", address);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		str.Format("max scratch width : %.3fmm", max_blob_width);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		str.Format("max scratch height : %.3fmm", max_blob_height);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}

	system_clock::time_point end = system_clock::now();
	nanoseconds nano = end - start;
	str.Format("Scratch Inspection Time: %f", nano.count() / 1000000.);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	return bReturn;
}

UINT Inspection_ScratchHole_ProcImage(void* pParam)
{
	_stLoadScratchImage* pScratch_Info = (_stLoadScratchImage*)pParam;
	CLET_AlignClientDlg* pMain = (CLET_AlignClientDlg*)pScratch_Info->pDlg;
	int nJob = (int)pScratch_Info->nJob;
	BYTE* ImgByte = (BYTE*)pScratch_Info->ImgByte;
	CDlgViewer* pView = (CDlgViewer*)pScratch_Info->viewerDlg;
	vector< PSTBLOB > vtBlob;
	for (int i = 0; i < 10; i++)
	{
		if (pScratch_Info->vtBlob[i] != 0) vtBlob.push_back(pScratch_Info->vtBlob[i]);
	}

	std::vector<int> camBuf = pMain->vt_job_info[nJob].camera_index;

	int ncamera = camBuf.at(0);
	int W = pMain->m_stCamInfo[ncamera].w;
	int H = pMain->m_stCamInfo[ncamera].h;

	cv::Mat gray, mask, result_img;
	cv::Mat imgSrc(H, W, CV_8UC1, ImgByte);

	double xres = pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = pMain->GetMachine(nJob).getCameraResolutionY(0, 0);
	double cir_size = pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getScratchMaskingRadius();
	int radius = int(cir_size / xres);
	int inspAreaSpec = pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getScratchInspAreaSpec();
	int roiOffset = (radius + inspAreaSpec) / 8 * 8; // 반지름

	double dbPosX = pMain->GetMatching(nJob).getFindInfo(0, 2).GetXPos();
	double dbPosY = pMain->GetMatching(nJob).getFindInfo(0, 2).GetYPos();

	// ================================================== Viewer 사이즈로 이미지 자르기 =================================

	int viewer_width = pView->GetViewer().GetWidth();
	int viewer_height = pView->GetViewer().GetHeight();

	cv::Rect _Roi = cv::Rect(dbPosX - (viewer_width / 2), dbPosY - (viewer_height / 2), viewer_width, viewer_height);
	_Roi.x = MIN(W - 1, MAX(0, _Roi.x));
	_Roi.y = MIN(H - 1, MAX(0, _Roi.y));
	_Roi.width = MIN(W - _Roi.x, MAX(0, _Roi.width));
	_Roi.height = MIN(H - _Roi.y, MAX(0, _Roi.height));

	gray = imgSrc(_Roi).clone();
	mask = GetCirclemask(gray, gray.cols / 2, gray.rows / 2, radius);
	bitwise_not(mask, mask);

	GaussianBlur(gray, gray, cv::Size(0, 0), 3.);
	pMain->m_pScratchInspWorker[ncamera].InspectionEnhance(&gray, 0);

	FakeDC* pDC = pView->GetViewer().getOverlayDC();
	pView->GetViewer().ClearOverlayDC();
	pView->GetViewer().clearAllFigures();
	pView->GetViewer().OnLoadImageFromPtr(gray.data);

	//2022-10-20 KJH2 Scratch 결과 이미지 저장
	result_img = gray.clone();
	cv::cvtColor(result_img, result_img, COLOR_GRAY2BGR);

	// =================================================================================================================


	// ================================================== Draw =========================================================
	CString str;
	CPen penOK(PS_SOLID, 2, COLOR_GREEN);
	CPen penNG(PS_SOLID, 2, COLOR_RED);
	CPen penROI(PS_SOLID, 2, COLOR_WHITE);

	CFont fontBolb, font, * oldFont;
	int OLD_BKMODE = 0;

	fontBolb.CreateFont(12, 12, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
	font.CreateFont(15, 15, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
	OLD_BKMODE = pDC->SetBkMode(TRANSPARENT);
	(CBrush*)pDC->SelectStockObject(NULL_BRUSH);

	//Mask
	pDC->SelectObject(&penOK);
	pDC->Ellipse(viewer_width / 2 - radius, viewer_height / 2 - radius, viewer_width / 2 + radius, viewer_height / 2 + radius);

	//ROI
	pDC->SelectObject(&penROI);
	CRect InspROI = CRect(viewer_width / 2 - roiOffset, viewer_height / 2 - roiOffset, viewer_width / 2 + roiOffset, viewer_width / 2 + roiOffset);
	pDC->Rectangle(CRect(InspROI.left, InspROI.top, InspROI.right, InspROI.bottom));

	//Found Blob
	oldFont = pDC->SelectObject(&fontBolb);
	pDC->SelectObject(&penNG);
	pDC->SetTextColor(COLOR_RED);

	//2022-10-20 KJH2 Scratch 결과 이미지 저장
	cv::circle(result_img, cv::Point(viewer_width / 2, viewer_height / 2), radius, cv::Scalar(0, 255, 0), 2);
	cv::rectangle(result_img, cv::Point(viewer_width / 2 - (radius * 2), viewer_height / 2 - (radius * 2)),
		cv::Point(viewer_width / 2 + (radius * 2), viewer_height / 2 + (radius * 2)),
		cv::Scalar(255, 255, 255), 2);

	for (int i = 0; i < vtBlob.size(); i++)
	{
		CRect roi;
		roi.left = vtBlob[i]->left + InspROI.left;
		roi.top = gray.rows - (vtBlob[i]->top + InspROI.top);
		roi.right = vtBlob[i]->right + InspROI.left;
		roi.bottom = gray.rows - (vtBlob[i]->bottom + InspROI.top);

		pDC->Rectangle(CRect(roi.left, roi.top, roi.right, roi.bottom));
		//2022-10-20 KJH2 Scratch 결과 이미지 저장
		cv::rectangle(result_img, cv::Point(roi.left, roi.top), cv::Point(roi.right, roi.bottom), cv::Scalar(0, 0, 255), 2);
		str.Format("x:%.3fmm, y:%.3fmm", fabs(roi.Width()) * xres, fabs(roi.Height()) * yres);
		pDC->TextOutA(roi.right, roi.bottom, str);
		cv::putText(result_img, std::string(str), cv::Point(roi.right, roi.bottom), cv::FONT_ITALIC, 0.7,
			cv::Scalar(0, 0, 255), 2);

	}

	//spec
	oldFont = pDC->SelectObject(&font);
	if (vtBlob.size() == 0)	pDC->SetTextColor(COLOR_GREEN);
	else					pDC->SetTextColor(COLOR_RED);
	double SpecSize = pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getScratchInspSpec() * xres * xres;
	str.Format("spec size : %.4fmm^2", SpecSize);
	pDC->TextOutA(100, 20, str);
	cv::putText(result_img, std::string(str), cv::Point(100, 35), cv::FONT_ITALIC, 1, cv::Scalar(0, 0, 255), 3);
	pView->GetViewer().Invalidate();

	// =====================================================================================================================

	CString	Time_str = pMain->m_strResultTime[nJob];
	CString	Date_str = pMain->m_strResultDate[nJob];
	CString	real_Time_str, str_JobName;

	CTime NowTime;
	NowTime = CTime::GetCurrentTime();
	real_Time_str.Format("%04d%02d%02d", NowTime.GetYear(), NowTime.GetMonth(), NowTime.GetDay());

	if (Time_str == "" || Date_str == "")
	{
		pMain->m_strResultDate[nJob].Format("%04d%02d%02d", NowTime.GetYear(), NowTime.GetMonth(), NowTime.GetDay());
		pMain->m_strResultTime[nJob].Format("%02d_%02d_%02d", NowTime.GetHour(), NowTime.GetMinute(), NowTime.GetSecond());

		Time_str = pMain->m_strResultTime[nJob];
		Date_str = pMain->m_strResultDate[nJob];
	}

	CString Cell_Name = pMain->vt_job_info[nJob].get_main_object_id();
	if (strlen(Cell_Name) < 10)					Cell_Name.Format("Test_%s", Time_str);

	CString strFileDir_Image, strFileDir_Image_Proc, strFileDir_Image_Proc_Raw;

	if (vtBlob.size() != 0)
	{
		strFileDir_Image = pMain->m_strImageDir + Date_str;
		if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);
		strFileDir_Image.Format("%s\\%s", strFileDir_Image, pMain->m_strCurrentModelName);
		if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

		//JOB이름
		str_JobName.Format("%s", pMain->vt_job_info[nJob].job_name.c_str());
		strFileDir_Image.Format("%s\\%s", strFileDir_Image, str_JobName);
		if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

		strFileDir_Image.Format("%s\\SCRATCH_INSP\\", strFileDir_Image);
		if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

		if (pMain->m_bSimulationStart)
		{
			strFileDir_Image.Format("%s\\Simulation\\", strFileDir_Image);
			if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);
		}
		strFileDir_Image.Format("%s%s\\", strFileDir_Image, Cell_Name);
		if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

		strFileDir_Image_Proc_Raw.Format("%s%s_%s_%s_proc_imgRaw.jpg", strFileDir_Image, Cell_Name, Time_str, pMain->m_stCamInfo[ncamera].cName);
		strFileDir_Image_Proc.Format("%s%s_%s_%s_proc_img.jpg", strFileDir_Image, Cell_Name, Time_str, pMain->m_stCamInfo[ncamera].cName);

		if (_access(strFileDir_Image_Proc_Raw, 0) != 0) strFileDir_Image_Proc_Raw.Format("%s%s_%s_%s_proc_ImgRaw.jpg", strFileDir_Image, Cell_Name, real_Time_str, pMain->m_stCamInfo[ncamera].cName);
		if (_access(strFileDir_Image_Proc, 0) != 0) strFileDir_Image_Proc.Format("%s%s_%s_%s_proc_Img.jpg", strFileDir_Image, Cell_Name, real_Time_str, pMain->m_stCamInfo[ncamera].cName);
		cv::imwrite(std::string(strFileDir_Image_Proc_Raw), gray);
		//pView->GetViewer().saveScreenCapture(strFileDir_Image_Proc);

		//2022-10-20 KJH2 Scratch 결과 이미지 저장
		cv::imwrite(std::string(strFileDir_Image_Proc), result_img);

	}
	result_img.release();
	gray.release();
	imgSrc.release();

	return 0;
}