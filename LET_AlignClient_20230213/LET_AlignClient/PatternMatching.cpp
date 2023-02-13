#include "StdAfx.h"
#include "LET_AlignClient.h"
#include "PatternMatching.h"
#include "opencv2\imgproc\imgproc_c.h"
#include "UI_Control\JXINIFile.h"
#include "LET_AlignClientDlg.h"
#include "INIReader.h"

CPatternMatching::CPatternMatching(void)
{
	for (int nCam = 0; nCam < MAX_CAMERA; nCam++)
	{
		for (int nPos = 0; nPos < NUM_POS; nPos++)
		{
			for (int nIndex = 0; nIndex < MAX_PATTERN_INDEX; nIndex++)
			{
				m_dbMatchingRate[nCam][nPos][nIndex] = 70.0;
				m_dbPosOffsetX[nCam][nPos][nIndex] = 0.0;
				m_dbPosOffsetY[nCam][nPos][nIndex] = 0.0;
				m_dbMarkOffsetX[nCam][nPos][nIndex] = 0.0;
				m_dbMarkOffsetY[nCam][nPos][nIndex] = 0.0;
				m_dbFinalMarkOffsetX[nCam][nPos][nIndex] = 0.0;
				m_dbFinalMarkOffsetY[nCam][nPos][nIndex] = 0.0;
			}

			for (int nIndex = 0; nIndex < MAX_INDEX; nIndex++)
			{
				m_nRemoveNoiseSize[nCam][nPos][nIndex] = 10;
				m_nSobelThreshold[nCam][nPos][nIndex] = 10;
			}
			m_nCornerType[nCam][nPos] = 0;
			m_nEdgePolarity[nCam][nPos] = FALSE;
			m_nEdgeDirection[nCam][nPos] = FALSE;
			m_dbRefPosX[nCam][nPos] = 0;
			m_dbRefPosY[nCam][nPos] = 0;
			m_dbRefPosT[nCam][nPos] = 0;
			m_dbFixturePosX[nCam][nPos] = 0;
			m_dbFixturePosY[nCam][nPos] = 0;
			m_dbFixturePosT[nCam][nPos] = 0;
		}

	}

	m_nMarkFindMethod = 0;
	m_nJobID = 0;
	/*m_nWidth = 1624;
	m_nHeight = 1236;*/
	m_bNccModel = FALSE;
	m_bMaskingModel = FALSE;

	m_nPatternMinContrast = -1;
	m_nPatternHighContrast = -1;
	

	memset(m_lineHori, 0, sizeof(m_lineHori));
	memset(m_lineVert, 0, sizeof(m_lineVert));	// 19.11.12

	m_nPatternResultIndex = -1;	// 20.03.29
}

CPatternMatching::~CPatternMatching(void)
{

}

BOOL CPatternMatching::releaseHalconModel()
{
	BOOL bSuccess = FALSE;
	CString strPatternPath;
	CFileFind findfile;

	for (int nCam = 0; nCam < MAX_CAMERA; nCam++)
	{
		for (int nPos = 0; nPos < NUM_POS; nPos++)
		{
			for (int nIndex = 0; nIndex < MAX_PATTERN_INDEX; nIndex++)
			{
				m_Halcon[nCam][nPos][nIndex].halcon_releaseModel();
			}
		}
	}

	return bSuccess;
}

void CPatternMatching::setHanconSearchAngleNstep(double angle, double step)
{
	for (int nCam = 0; nCam < MAX_CAMERA; nCam++)
	{
		for (int nPos = 0; nPos < NUM_POS; nPos++)
		{
			for (int nIndex = 0; nIndex < MAX_PATTERN_INDEX; nIndex++)
			{
				m_Halcon[nCam][nPos][nIndex].setSearchAngleRange(angle);
				m_Halcon[nCam][nPos][nIndex].setSearchAngleStep(step);
			}
		}
	}
}

BOOL CPatternMatching::readPatternData(bool breadmodel, CString strPath, int num_of_camera, int num_of_position, int m_nHalconMinContrast, int m_nHalconHighContrast, CString fileName, bool binsp, bool binspPanel)
{
	BOOL bSuccess = FALSE;
	CString strPatternPath;
	CString filePath, str;
	CFileFind findfile;
	CRect rectROI;
	int iVal;
	double dVal;

	m_nPatternMinContrast = m_nHalconMinContrast;
	m_nPatternHighContrast = m_nHalconHighContrast;

	releaseHalconModel();

	filePath.Format("%s\\%s", strPath, fileName); // 2017.09.23 JSH 파일 경로명 오류

	std::string strSection = "PATTERN_INFO", strValue;
	INIReader ini(fmt_("%s", filePath.GetString()));

	for (int nCam = 0; nCam < num_of_camera; nCam++)
	{
		for (int nPos = 0; nPos < num_of_position; nPos++)
		{
			iVal = ini.GetInteger(strSection, fmt_("ROI%d%d_LEFT", nCam + 1, nPos + 1), 0); rectROI.left = iVal;
			iVal = ini.GetInteger(strSection, fmt_("ROI%d%d_TOP", nCam + 1, nPos + 1), 0); rectROI.top = iVal;
			iVal = ini.GetInteger(strSection, fmt_("ROI%d%d_RIGHT", nCam + 1, nPos + 1), 3088); rectROI.right = iVal;
			iVal = ini.GetInteger(strSection, fmt_("ROI%d%d_BOTTOM", nCam + 1, nPos + 1), 2064); rectROI.bottom = iVal;
			m_searchROI[nCam][nPos] = rectROI;

			for (int nIndex = 0; nIndex < MAX_PATTERN_INDEX; nIndex++)
			{
				iVal = ini.GetInteger(strSection, fmt_("ROI%d%d%d_LEFT", nCam + 1, nPos + 1, nIndex + 1), 0); rectROI.left = iVal;
				iVal = ini.GetInteger(strSection, fmt_("ROI%d%d%d_TOP", nCam + 1, nPos + 1, nIndex + 1), 0); rectROI.top = iVal;
				iVal = ini.GetInteger(strSection, fmt_("ROI%d%d%d_RIGHT", nCam + 1, nPos + 1, nIndex + 1), 3088); rectROI.right = iVal;
				iVal = ini.GetInteger(strSection, fmt_("ROI%d%d%d_BOTTOM", nCam + 1, nPos + 1, nIndex + 1), 2064); rectROI.bottom = iVal;
				m_searchROI2[nCam][nPos][nIndex] = rectROI;

				if (m_searchROI2[nCam][nPos][nIndex].left < 10 && m_searchROI2[nCam][nPos][nIndex].top < 10) m_bUseSearchROI2[nCam][nPos][nIndex] = FALSE;
				else		m_bUseSearchROI2[nCam][nPos][nIndex] = TRUE;
				dVal = ini.GetReal(strSection, fmt_("MATCH_RATE%d%d%d", nCam + 1, nPos + 1, nIndex + 1), 70.0); m_dbMatchingRate[nCam][nPos][nIndex] = dVal;
				dVal = ini.GetReal(strSection, fmt_("POS_OFFSET_X%d%d%d", nCam + 1, nPos + 1, nIndex + 1), 0.0); m_dbPosOffsetX[nCam][nPos][nIndex] = dVal;
				dVal = ini.GetReal(strSection, fmt_("POS_OFFSET_Y%d%d%d", nCam + 1, nPos + 1, nIndex + 1), 0.0); m_dbPosOffsetY[nCam][nPos][nIndex] = dVal;
				dVal = ini.GetReal(strSection, fmt_("MARK_OFFSET_X%d%d%d", nCam + 1, nPos + 1, nIndex + 1), 0.0); m_dbMarkOffsetX[nCam][nPos][nIndex] = dVal;
				dVal = ini.GetReal(strSection, fmt_("MARK_OFFSET_Y%d%d%d", nCam + 1, nPos + 1, nIndex + 1), 0.0); m_dbMarkOffsetY[nCam][nPos][nIndex] = dVal;
				dVal = ini.GetReal(strSection, fmt_("MATCHING_LOWCONT_%d%d%d", nCam + 1, nPos + 1, nIndex + 1), 20.); m_dbLowContrast[nCam][nPos][nIndex] = dVal;
				if (m_dbLowContrast[nCam][nPos][nIndex] < -256 || m_dbLowContrast[nCam][nPos][nIndex] > 256) m_dbLowContrast[nCam][nPos][nIndex] = -1;
				dVal = ini.GetReal(strSection, fmt_("MATCHING_HIGHCONT_%d%d%d", nCam + 1, nPos + 1, nIndex + 1), 50.); m_dbHighContrast[nCam][nPos][nIndex] = dVal;
				if (m_dbHighContrast[nCam][nPos][nIndex] < -256 || m_dbHighContrast[nCam][nPos][nIndex] > 256) m_dbHighContrast[nCam][nPos][nIndex] = -1;
				iVal = ini.GetInteger(strSection, fmt_("USE_MASK_%d%d%d", nCam + 1, nPos + 1, nIndex + 1), 0); m_bUseMask[nCam][nPos][nIndex] = iVal;
				m_Halcon[nCam][nPos][nIndex].halcon_SetShapeModelOrigin(m_dbPosOffsetX[nCam][nPos][nIndex], m_dbPosOffsetY[nCam][nPos][nIndex]);
				iVal = ini.GetInteger(strSection, fmt_("TEMPLATE_%d%d%d_SHAPE", nCam + 1, nPos + 1, nIndex + 1), 0); setTemplateAreaShape(nCam, nPos, nIndex, iVal);
			}

			dVal = ini.GetReal(strSection, fmt_("REF_X%d%d", nCam + 1, nPos + 1), 0.0); m_dbRefPosX[nCam][nPos] = dVal;
			if (m_dbRefPosX[nCam][nPos] < -10000 || m_dbRefPosX[nCam][nPos] > 30000) m_dbRefPosX[nCam][nPos]=0;
			dVal = ini.GetReal(strSection, fmt_("REF_Y%d%d", nCam + 1, nPos + 1), 0.0); m_dbRefPosY[nCam][nPos] = dVal;
			if (m_dbRefPosY[nCam][nPos] < -10000 || m_dbRefPosY[nCam][nPos] > 30000) m_dbRefPosY[nCam][nPos] = 0;
			dVal = ini.GetReal(strSection, fmt_("REF_T%d%d", nCam + 1, nPos + 1), 0.0); m_dbRefPosT[nCam][nPos] = dVal;
			if (m_dbRefPosT[nCam][nPos] < -10000 || m_dbRefPosT[nCam][nPos] > 30000) m_dbRefPosT[nCam][nPos] = 0;
			dVal = ini.GetReal(strSection, fmt_("FIXTURE_X%d%d", nCam + 1, nPos + 1), 0.0); m_dbFixturePosX[nCam][nPos] = dVal;
			if (m_dbFixturePosX[nCam][nPos] < -10000 || m_dbFixturePosX[nCam][nPos] > 30000) m_dbFixturePosX[nCam][nPos] = 0;
			dVal = ini.GetReal(strSection, fmt_("FIXTURE_Y%d%d", nCam + 1, nPos + 1), 0.0); m_dbFixturePosY[nCam][nPos] = dVal;
			if (m_dbFixturePosY[nCam][nPos] < -10000 || m_dbFixturePosY[nCam][nPos] > 30000) m_dbFixturePosY[nCam][nPos] = 0;
			dVal = ini.GetReal(strSection, fmt_("FIXTURE_T%d%d", nCam + 1, nPos + 1), 0.0); m_dbFixturePosT[nCam][nPos] = dVal;
			if (m_dbFixturePosT[nCam][nPos] < -10000 || m_dbFixturePosT[nCam][nPos] > 30000) m_dbFixturePosT[nCam][nPos] = 0;
		}
	}
	if (breadmodel)
	{
		CString strMaskPath;

	//	for (int nCam = 0; nCam < MAX_CAMERA; nCam++)
		for (int nCam = 0; nCam < num_of_camera; nCam++)
		{
			//for (int nPos = 0; nPos < NUM_POS; nPos++)
			for (int nPos = 0; nPos < num_of_position; nPos++)
			{
				for (int nIndex = 0; nIndex < MAX_PATTERN_INDEX; nIndex++)
				{
					if (binsp)
					{
						str = "_OUT_";
						if (!binspPanel) str = "_IN_";
						switch (nPos) {
						case 0: str += "1"; break;
						case 1: str += "2"; break;
						case 2: str += "3"; break;
						case 3: str += "4"; break;
						}
						strPatternPath.Format("%s\\INSP%s%d%d%d.bmp", strPath, str, nCam, nPos, nIndex);
					}
					else strPatternPath.Format("%s\\PAT%d%d%d.bmp", strPath, nCam, nPos, nIndex);

					if (findfile.FindFile(strPatternPath) != TRUE) continue;

					if (m_bNccModel)
						m_Halcon[nCam][nPos][nIndex].halcon_ReadNccModel(strPatternPath);
					else
					{
						strMaskPath.Format("%s\\%s_%d%d%d%s", strPath, "MarkMask", nCam, nPos, nIndex, "Info.bmp");
						if (_access(strMaskPath, 0) == 0) setUseMask(nCam, nPos, nIndex, TRUE);
						else setUseMask(nCam, nPos, nIndex, FALSE);										

						//210123 추가한 사항 맞는지 확인 필요해요

						//LPBYTE pStrMaskPath = new BYTE[strMaskPath.GetLength() + 1];
						//memcpy(pStrMaskPath, (VOID*)LPCTSTR(strMaskPath), strMaskPath.GetLength());

						//LPBYTE pStrPath = new BYTE[strPath.GetLength() + 1];
						//memcpy(pStrPath, (VOID*)LPCTSTR(strPath), strPath.GetLength());

						m_Halcon[nCam][nPos][nIndex].readParameter(theApp.m_pFrame->m_strCurrentModelPath + "MarkPatternInfo.ini", nCam, nPos, nIndex, m_nJobID);

						if (getUseMask(nCam, nPos, nIndex))
						{
							m_Halcon[nCam][nPos][nIndex].halcon_ReadModelNew(strPatternPath, strMaskPath);
						}
						else
						{   // 20211204 Tkyuha 마스크가 없는 경우는 Contrast 값이 따라 가지 않는 버그 수정
							m_nHalconMinContrast = m_Halcon[nCam][nPos][nIndex].m_ReadContrast;
							m_nHalconHighContrast = m_Halcon[nCam][nPos][nIndex].m_ReadMinContrast;
							m_Halcon[nCam][nPos][nIndex].halcon_ReadModel(strPatternPath, m_nHalconMinContrast, m_nHalconHighContrast);
						}

						/*
						cv::Mat img;
						img = cv::imread((LPCTSTR)strMaskPath);

						LPBYTE pStrMaskPath = new BYTE[strMaskPath.GetLength() + 1];
						memcpy(pStrMaskPath, (VOID*)LPCTSTR(strMaskPath), strMaskPath.GetLength());

						LPBYTE pStrPath = new BYTE[strPath.GetLength() + 1];
						memcpy(pStrPath, (VOID*)LPCTSTR(strPath), strPath.GetLength());

						if (getUseMask(nCam, nPos, nIndex)) 	m_Halcon[nCam][nPos][nIndex].halcon_ReadModelNew(pStrMaskPath, pStrPath, img.cols, img.rows);
						else m_Halcon[nCam][nPos][nIndex].halcon_ReadModel(strPatternPath, m_nHalconMinContrast, m_nHalconHighContrast);
						*/
					}

					m_Halcon[nCam][nPos][nIndex].halcon_SetShapeModelOrigin(m_dbPosOffsetX[nCam][nPos][nIndex], m_dbPosOffsetY[nCam][nPos][nIndex]);
				}
			}
		}
	}

	return bSuccess;
}


BOOL CPatternMatching::savePatternData(CString strPath, CString fileName, bool binsp)
{
	BOOL bSuccess = FALSE;
	CString strPatternPath, strKey, strSection, strData;
	CString filePath;
	CFileFind findfile;
	TCHAR cData[MAX_PATH] = { 0, };
	CRect rectROI;

	strSection = "PATTERN_INFO";
	filePath.Format("%s\\%s", strPath, fileName);	

	for (int nCam = 0; nCam < MAX_CAMERA; nCam++)
	{
		for (int nPos = 0; nPos < NUM_POS; nPos++)
		{
			strKey.Format("ROI%d%d_LEFT", nCam + 1, nPos + 1);
			strData.Format("%d", m_searchROI[nCam][nPos].left);
			::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

			strKey.Format("ROI%d%d_TOP", nCam + 1, nPos + 1);
			strData.Format("%d", m_searchROI[nCam][nPos].top);
			::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

			strKey.Format("ROI%d%d_RIGHT", nCam + 1, nPos + 1);
			strData.Format("%d", m_searchROI[nCam][nPos].right);
			::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

			strKey.Format("ROI%d%d_BOTTOM", nCam + 1, nPos + 1);
			strData.Format("%d", m_searchROI[nCam][nPos].bottom);
			::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

			for (int nIndex = 0; nIndex < MAX_PATTERN_INDEX; nIndex++)
			{
				strKey.Format("ROI%d%d%d_LEFT", nCam + 1, nPos + 1, nIndex + 1);
				strData.Format("%d", m_searchROI2[nCam][nPos][nIndex].left);
				::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

				strKey.Format("ROI%d%d%d_TOP", nCam + 1, nPos + 1, nIndex + 1);
				strData.Format("%d", m_searchROI2[nCam][nPos][nIndex].top);
				::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

				strKey.Format("ROI%d%d%d_RIGHT", nCam + 1, nPos + 1, nIndex + 1);
				strData.Format("%d", m_searchROI2[nCam][nPos][nIndex].right);
				::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

				strKey.Format("ROI%d%d%d_BOTTOM", nCam + 1, nPos + 1, nIndex + 1);
				strData.Format("%d", m_searchROI2[nCam][nPos][nIndex].bottom);
				::JXWritePrivateProfileString(strSection, strKey, strData, filePath);
			}
			strKey.Format("REF_X%d%d", nCam + 1, nPos + 1);
			strData.Format("%f", m_dbRefPosX[nCam][nPos]);
			::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

			strKey.Format("REF_Y%d%d", nCam + 1, nPos + 1);
			strData.Format("%f", m_dbRefPosY[nCam][nPos]);
			::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

			strKey.Format("REF_T%d%d", nCam + 1, nPos + 1);
			strData.Format("%f", m_dbRefPosT[nCam][nPos]);
			::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

			strKey.Format("FIXTURE_X%d%d", nCam + 1, nPos + 1);
			strData.Format("%f", m_dbFixturePosX[nCam][nPos]);
			::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

			strKey.Format("FIXTURE_Y%d%d", nCam + 1, nPos + 1);
			strData.Format("%f", m_dbFixturePosY[nCam][nPos]);
			::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

			strKey.Format("FIXTURE_T%d%d", nCam + 1, nPos + 1);
			strData.Format("%f", m_dbFixturePosT[nCam][nPos]);
			::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

			for (int nIndex = 0; nIndex < MAX_PATTERN_INDEX; nIndex++)
			{
				strKey.Format("MATCH_RATE%d%d%d", nCam + 1, nPos + 1, nIndex + 1);
				strData.Format("%.1f", m_dbMatchingRate[nCam][nPos][nIndex]);
				::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

				strKey.Format("POS_OFFSET_X%d%d%d", nCam + 1, nPos + 1, nIndex + 1);
				strData.Format("%f", m_dbPosOffsetX[nCam][nPos][nIndex]);
				::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

				strKey.Format("POS_OFFSET_Y%d%d%d", nCam + 1, nPos + 1, nIndex + 1);
				strData.Format("%f", m_dbPosOffsetY[nCam][nPos][nIndex]);
				::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

				strKey.Format("MARK_OFFSET_X%d%d%d", nCam + 1, nPos + 1, nIndex + 1);
				strData.Format("%f", m_dbMarkOffsetX[nCam][nPos][nIndex]);
				::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

				strKey.Format("MARK_OFFSET_Y%d%d%d", nCam + 1, nPos + 1, nIndex + 1);
				strData.Format("%f", m_dbMarkOffsetY[nCam][nPos][nIndex]);
				::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

				strKey.Format("MATCHING_LOWCONT_%d%d%d", nCam + 1, nPos + 1, nIndex + 1);
				strData.Format("%f", m_dbLowContrast[nCam][nPos][nIndex]);
				::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

				strKey.Format("MATCHING_HIGHCONT_%d%d%d", nCam + 1, nPos + 1, nIndex + 1);
				strData.Format("%f", m_dbHighContrast[nCam][nPos][nIndex]);
				::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

				strKey.Format("USE_MASK_%d%d%d", nCam + 1, nPos + 1, nIndex + 1);
				strData.Format("%d", m_bUseMask[nCam][nPos][nIndex]);
				::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

				strKey.Format("TEMPLATE_%d%d%d_SHAPE", nCam + 1, nPos + 1, nIndex + 1);
				strData.Format("%d", getTemplateAreaShape(nCam, nPos, nIndex));
				::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

				for (int mask = 0; mask < MAX_MASK_INDEX; mask++)
				{
					rectROI = getMaskRect(nCam, nPos, nIndex, mask);

					strKey.Format("MASK_ROI%d%d%d%d_LEFT", nCam + 1, nPos + 1, nIndex + 1, mask + 1);
					strData.Format("%d", rectROI.left);
					::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

					strKey.Format("MASK_ROI%d%d%d%d_TOP", nCam + 1, nPos + 1, nIndex + 1, mask + 1);
					strData.Format("%d", rectROI.top);
					::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

					strKey.Format("MASK_ROI%d%d%d%d_RIGHT", nCam + 1, nPos + 1, nIndex + 1, mask + 1);
					strData.Format("%d", rectROI.right);
					::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

					strKey.Format("MASK_ROI%d%d%d%d_BOTTOM", nCam + 1, nPos + 1, nIndex + 1, mask + 1);
					strData.Format("%d", rectROI.bottom);
					::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

					strKey.Format("MASK_ROI%d%d%d%d_USE", nCam + 1, nPos + 1, nIndex + 1, mask + 1);
					strData.Format("%d", getMaskRectUse(nCam, nPos, nIndex, mask));
					::JXWritePrivateProfileString(strSection, strKey, strData, filePath);

					strKey.Format("MASK_ROI%d%d%d%d_SHAPE", nCam + 1, nPos + 1, nIndex + 1, mask + 1);
					strData.Format("%d", getMaskRectShape(nCam, nPos, nIndex, mask));
					::JXWritePrivateProfileString(strSection, strKey, strData, filePath);
				}
			}
		}
	}

	::JXUnloadPrivateProfile();

	return bSuccess;
}


BOOL CPatternMatching::findPattern_Ransac(BYTE *pImage, int nCam, int nPos, int m_nWidth, int m_nHeight, CFindInfo *pFindInfo)
{
	BOOL bFind = FALSE;
	int nMaxIndex = 0;
	double dbMaxScore = 0.0, dbMaxPosX = 0.0, dbMaxPosY = 0.0, dbMaxAngle = 0.0;

	double dbScoreLimit = 70.;
	CRect rcInspROI = m_searchROI[nCam][nPos];

	int nProjectionHori = 0;
	int nProjectionVert = 0;
	int nSearchOffset = 300;
	int nSearchHori = 0;
	int nSearchVert = 0;
	int nThresh_Hori = 0;
	int nThresh_Vert = 0;
	int nNoiszeSize_Hori = 0;
	int nNoiszeSize_Vert = 0;
	int nCornerType = m_nCornerType[nCam][nPos];
	int nEdgePolarity = m_nEdgePolarity[nCam][nPos];
	int nEdgeDirection = m_nEdgeDirection[nCam][nPos];
	int nSobel_Hori = 0;
	int nSobel_Vert = 0;

	nCornerType = m_nCornerType[nCam][nPos];
	nEdgePolarity = m_nEdgePolarity[nCam][nPos];		// Light To Dark , Dark To Light
	nEdgeDirection = m_nEdgeDirection[nCam][nPos];

	nSearchHori = m_nSearchArea[nCam][nPos][0];
	nSearchVert = m_nSearchArea[nCam][nPos][1];
	nThresh_Hori = m_nSobelThreshold[nCam][nPos][0];
	nThresh_Vert = m_nSobelThreshold[nCam][nPos][1];
	nNoiszeSize_Hori = m_nRemoveNoiseSize[nCam][nPos][0];
	nNoiszeSize_Vert = m_nRemoveNoiseSize[nCam][nPos][1];
	nProjectionHori = m_nProjectionArea[nCam][nPos][0];
	nProjectionVert = m_nProjectionArea[nCam][nPos][1];

	decideSobelDirection(nCornerType, nEdgePolarity, nEdgeDirection, &nSobel_Hori, &nSobel_Vert);
	
	bFind = findCornerEdgeByRansac2(pImage,
		m_nWidth,
		m_nHeight,
		nCornerType,
		nSobel_Hori,
		nSobel_Vert,
		nNoiszeSize_Hori,
		nNoiszeSize_Vert,
		nThresh_Hori,
		nThresh_Vert,
		rcInspROI,
		nProjectionHori,
		nProjectionVert,
		nSearchHori,
		nSearchVert,
		&m_lineHori[nCam][nPos],
		&m_lineVert[nCam][nPos],
		nSearchOffset,
		nEdgeDirection,
		nCam,
		nPos);

	//setRealROIVert( &m_rcRealROI[nCam][0], &m_rcRealROI[nCam][1], m_nWidth, m_nHeight, nCornerType, rcInspROI, nSearchHori, nSearchVert, nSearchOffset);
	setRealROIVert2(&m_rcRealROI[nCam][nPos][0], &m_rcRealROI[nCam][nPos][1], m_nWidth, m_nHeight, nCornerType, rcInspROI, nProjectionHori, nProjectionVert, nSearchHori, nSearchVert, nSearchOffset);

	if (bFind)
	{
		double posX = 0.0, posY = 0.0;

		if (m_lineVert[nCam][nPos].a == 0)
		{
			posX = m_lineVert[nCam][nPos].b;
			posY = m_lineHori[nCam][nPos].a * posX + m_lineHori[nCam][nPos].b;
		}
		else	cramersRules(-m_lineHori[nCam][nPos].a, 1, -m_lineVert[nCam][nPos].a, 1, m_lineHori[nCam][nPos].b, m_lineVert[nCam][nPos].b, &posX, &posY);

		getFindInfo(nCam, nPos).SetXPos(posX);
		getFindInfo(nCam, nPos).SetYPos(posY);
		getFindInfo(nCam, nPos).SetScore(100.0);
		getFindInfo(nCam, nPos).SetFound(FIND_OK);

		calcLineInfo(nCam, nPos, rcInspROI, m_nWidth, m_nHeight);
	}
	else
	{
		getFindInfo(nCam, nPos).SetXPos(0.0);
		getFindInfo(nCam, nPos).SetYPos(0.0);
		getFindInfo(nCam, nPos).SetScore(0.0);
		getFindInfo(nCam, nPos).SetFound(FIND_ERR);
	}

	return bFind;
}


BOOL CPatternMatching::findPattern_Ransac_Multiple(BYTE *pImage, int nCam, int nPos, CFindInfo *pFindInfo, int nIndex, int m_nWidth, int m_nHeight)
{
	BOOL bFind = FALSE;
	int nMaxIndex = 0;
	double dbMaxScore = 0.0, dbMaxPosX = 0.0, dbMaxPosY = 0.0, dbMaxAngle = 0.0;

	double dbScoreLimit = 70.;
	CRect rcInspROI = m_searchROI2[nCam][nPos][nIndex];

	int nProjectionHori = 0;
	int nProjectionVert = 0;
	int nSearchOffset = 300;
	int nSearchHori = 0;
	int nSearchVert = 0;
	int nThresh_Hori = 0;
	int nThresh_Vert = 0;
	int nNoiszeSize_Hori = 0;
	int nNoiszeSize_Vert = 0;
	int nCornerType = m_nCornerType[nCam][nPos];
	int nEdgePolarity = m_nEdgePolarity[nCam][nPos];
	int nEdgeDirection = m_nEdgeDirection[nCam][nPos];
	int nSobel_Hori = 0;
	int nSobel_Vert = 0;

	nCornerType = m_nCornerType[nCam][nPos];
	nEdgePolarity = m_nEdgePolarity[nCam][nPos];		// Light To Dark , Dark To Light
	nEdgeDirection = m_nEdgeDirection[nCam][nPos];

	nSearchHori = m_nSearchArea[nCam][nPos][0];
	nSearchVert = m_nSearchArea[nCam][nPos][1];
	nThresh_Hori = m_nSobelThreshold[nCam][nPos][0];
	nThresh_Vert = m_nSobelThreshold[nCam][nPos][1];
	nNoiszeSize_Hori = m_nRemoveNoiseSize[nCam][nPos][0];
	nNoiszeSize_Vert = m_nRemoveNoiseSize[nCam][nPos][1];
	nProjectionHori = m_nProjectionArea[nCam][nPos][0];
	nProjectionVert = m_nProjectionArea[nCam][nPos][1];

	decideSobelDirection(nCornerType, nEdgePolarity, nEdgeDirection, &nSobel_Hori, &nSobel_Vert);
	
	bFind = findCornerEdgeByRansac2(pImage,
		m_nWidth,
		m_nHeight,
		nCornerType,
		nSobel_Hori,
		nSobel_Vert,
		nNoiszeSize_Hori,
		nNoiszeSize_Vert,
		nThresh_Hori,
		nThresh_Vert,
		rcInspROI,
		nProjectionHori,
		nProjectionVert,
		nSearchHori,
		nSearchVert,
		&m_lineHori[nCam][nPos],
		&m_lineVert[nCam][nPos],
		nSearchOffset,
		nEdgeDirection,
		nCam,
		nPos);

	//setRealROIVert( &m_rcRealROI[nCam][0], &m_rcRealROI[nCam][1], m_nWidth, m_nHeight, nCornerType, rcInspROI, nSearchHori, nSearchVert, nSearchOffset);
	setRealROIVert2(&m_rcRealROI[nCam][nPos][0], &m_rcRealROI[nCam][nPos][1], m_nWidth, m_nHeight, nCornerType, rcInspROI, nProjectionHori, nProjectionVert, nSearchHori, nSearchVert, nSearchOffset);

	if (bFind)
	{
		double posX = 0.0, posY = 0.0;

		if (m_lineVert[nCam][nPos].a == 0)
		{
			posX = m_lineVert[nCam][nPos].b;
			posY = m_lineHori[nCam][nPos].a * posX + m_lineHori[nCam][nPos].b;
		}
		else	cramersRules(-m_lineHori[nCam][nPos].a, 1, -m_lineVert[nCam][nPos].a, 1, m_lineHori[nCam][nPos].b, m_lineVert[nCam][nPos].b, &posX, &posY);

		getFindInfo(nCam, nPos).SetXPos(posX);
		getFindInfo(nCam, nPos).SetYPos(posY);
		getFindInfo(nCam, nPos).SetScore(100.0);
		getFindInfo(nCam, nPos).SetFound(FIND_OK);

		calcLineInfo(nCam, nPos, rcInspROI, m_nWidth, m_nHeight);
	}
	else
	{
		getFindInfo(nCam, nPos).SetXPos(0.0);
		getFindInfo(nCam, nPos).SetYPos(0.0);
		getFindInfo(nCam, nPos).SetScore(0.0);
		getFindInfo(nCam, nPos).SetFound(FIND_ERR);
	}

	return bFind;
}

BOOL CPatternMatching::findPattern_Caliper(BYTE* pImage, int nCam, int nPos, int m_nWidth, int m_nHeight, CFindInfo* pFindInfo)
{
	BOOL bFind = FALSE;

	return bFind;
}


BOOL CPatternMatching::findPattern_Matching(BYTE *pImage, int nCam, int nPos, int m_nWidth, int m_nHeight, CFindInfo *pFindInfo)
{
	BOOL bRet = FALSE;
	BOOL bFind = FALSE;
	int nMaxIndex = 0;
	double score, dbPosX, dbPosY, dbAngle;
	double dbMaxScore = 0.0, dbMaxPosX = 0.0, dbMaxPosY = 0.0, dbMaxAngle = 0.0;

	double dbScoreLimit = 95.;
	CRect rcInspROI = m_searchROI[nCam][nPos];

	// 5번 마크는 별도로 사용하기 위해 4번 마크 까지만 탐색
	for (int nIndex = 0; nIndex < MAX_PATTERN_INDEX - 1; nIndex++)
	{
		dbScoreLimit = m_dbMatchingRate[nCam][nPos][nIndex];

		if (m_Halcon[nCam][nPos][nIndex].getModelRead() != true) continue;

		if (m_Halcon[nCam][nPos][nIndex].getNccSelect() == true)
			bRet = getHalcon(nCam, nPos, nIndex).halcon_SearchNccMark(pImage, m_nWidth, m_nHeight, rcInspROI, dbScoreLimit / 100.0);
		else
			bRet = m_Halcon[nCam][nPos][nIndex].halcon_SearchMark(pImage, m_nWidth, m_nHeight, rcInspROI, dbScoreLimit / 100.0);

		if (bRet)
		{
			score = m_Halcon[nCam][nPos][nIndex].halcon_GetResultScore() * 100.0;
			dbPosX = m_Halcon[nCam][nPos][nIndex].halcon_GetResultPos().x;
			dbPosY = m_Halcon[nCam][nPos][nIndex].halcon_GetResultPos().y;
			dbAngle = m_Halcon[nCam][nPos][nIndex].halcon_GetResultAngle();

			if (score > dbMaxScore)
			{
				bFind = TRUE;

				dbMaxScore = score;
				//KJH 2022-05-28 Roi 화면 밖으로 등록시 마커 좌표 오류 수정
				dbMaxPosX = dbPosX + MAX(0, rcInspROI.left);
				dbMaxPosY = dbPosY + MAX(0, rcInspROI.top);

				nMaxIndex = nIndex;
				dbMaxAngle = dbAngle;
			}

			//if (score >= dbScoreLimit)	break;
		}
	}

	if (bFind)
	{
		dbScoreLimit = m_dbMatchingRate[nCam][nPos][nMaxIndex];
		if (dbMaxScore > dbScoreLimit)		pFindInfo->SetFound(FIND_OK);
		else								pFindInfo->SetFound(FIND_MATCH);

		pFindInfo->SetScore(dbMaxScore);
		//KJH 2022-03-12 MatchingRate Setting Value FDC 보고 추가
		pFindInfo->SetFindIndex(dbScoreLimit);
		pFindInfo->SetXPos(dbMaxPosX);
		pFindInfo->SetYPos(dbMaxPosY);
		pFindInfo->SetFoundPatternNum(nMaxIndex);
		pFindInfo->SetAngle(dbMaxAngle);
	}
	else
	{
		pFindInfo->SetFound(FIND_ERR);
		pFindInfo->SetXPos(0.0);
		pFindInfo->SetYPos(0.0);
		pFindInfo->SetScore(0.0);
		pFindInfo->SetFindIndex(0);
		pFindInfo->SetAngle(0.0);
	}

	return bFind;
}

BOOL CPatternMatching::findPattern(BYTE *pImage, int nCam, int nPos, int m_nWidth, int m_nHeight, CFindInfo *pFindInfo)
{
	BOOL bSuccess = FALSE;
	m_nPatternResultIndex = -1;

	if (pFindInfo == NULL)		pFindInfo = &m_FindInfo[nCam][nPos];

	pFindInfo->SetFound(FIND_ERR);
	pFindInfo->SetXPos(0.0);
	pFindInfo->SetYPos(0.0);
	pFindInfo->SetScore(0.0);
	pFindInfo->SetAngle(0.0);

	
	if (m_nMarkFindMethod == METHOD_RANSAC)						bSuccess = findPattern_Ransac(pImage, nCam, nPos,  m_nWidth,  m_nHeight, pFindInfo);
	else														bSuccess = findPattern_Matching(pImage, nCam, nPos, m_nWidth, m_nHeight, pFindInfo);
	if (bSuccess == FALSE && m_nMarkFindMethod == METHOD_MULTI_RANSAC)	// 20.03.29
	{
		int nIndex = 1;
		for (nIndex; nIndex < MAX_PATTERN_INDEX; nIndex++)
		{
			if (getUseSearchROI2(nCam, nPos, nIndex) == FALSE) continue;
			bSuccess = findPattern_Ransac_Multiple(pImage, nCam, nPos, pFindInfo, nIndex,  m_nWidth, m_nHeight);
			if (bSuccess == TRUE)
			{
				m_nPatternResultIndex = nIndex;
				break;
			}
		}
	}
	//bSuccess = TRUE;
	return bSuccess;
}

BOOL CPatternMatching::findPatternIndex(BYTE *pImage, int nCam, int nPos, int m_nWidth, int m_nHeight, int nIndex, CFindInfo *pFindInfo)
{
	BOOL bSuccess = FALSE;

	BOOL bRet;
	BOOL bFind = FALSE;
	int nMaxIndex = 0;
	double score, dbPosX, dbPosY, dbAngle = 0.0;
	double dbMaxScore = 0.0, dbMaxPosX = 0.0, dbMaxPosY = 0.0, dbMaxAngle = 0.0;

	double dbScoreLimit = 70.;
	CRect rcInspROI = m_searchROI[nCam][nPos];

	if (nIndex == 4)
	{
		rcInspROI.left = 10;
		rcInspROI.top = 10;
		rcInspROI.right = m_nWidth - 10;
		rcInspROI.bottom = m_nHeight - 10;
	}

	if (pFindInfo == NULL)		pFindInfo = &m_FindInfo[nCam][nPos];

	pFindInfo->SetFound(FIND_ERR);
	pFindInfo->SetAngle(0.0);

	if (m_nMarkFindMethod == METHOD_MATCHING)
	{
		dbScoreLimit = m_dbMatchingRate[nCam][nPos][nIndex];

		if (m_Halcon[nCam][nPos][nIndex].getModelRead() == true)
		{
			if (m_Halcon[nCam][nPos][nIndex].getNccSelect() == true)
				bRet = getHalcon(nCam, nPos, nIndex).halcon_SearchNccMark(pImage, m_nWidth, m_nHeight, rcInspROI, dbScoreLimit / 100.0);
			else	bRet = m_Halcon[nCam][nPos][nIndex].halcon_SearchMark(pImage, m_nWidth, m_nHeight, rcInspROI, dbScoreLimit / 100.0);

			if (bRet)
			{
				score = m_Halcon[nCam][nPos][nIndex].halcon_GetResultScore() * 100.0;
				dbPosX = m_Halcon[nCam][nPos][nIndex].halcon_GetResultPos().x;
				dbPosY = m_Halcon[nCam][nPos][nIndex].halcon_GetResultPos().y;
				dbAngle = m_Halcon[nCam][nPos][nIndex].halcon_GetResultAngle();

				if (score > dbMaxScore)
				{
					bFind = TRUE;

					dbMaxScore = score;

					/*if(m_nPatternMinContrast==-1 && m_nPatternHighContrast==-1)
					{*/
					//KJH 2022-05-28 Roi 화면 밖으로 등록시 마커 좌표 오류 수정
					dbMaxPosX = dbPosX + MAX(0, rcInspROI.left);
					dbMaxPosY = dbPosY + MAX(0, rcInspROI.top);
					/*	
					}
						else
						{pFunc

							dbMaxPosX = dbPosX + rcInspROI.left + m_dbPosOffsetX[nCam][nPos][nIndex];
							dbMaxPosY = dbPosY + rcInspROI.top + m_dbPosOffsetY[nCam][nPos][nIndex];
					}
					*/
					nMaxIndex = nIndex;
					dbMaxAngle = dbAngle;
				}
			}
		}

		if (bFind)
		{
			if (dbMaxScore > dbScoreLimit)
				pFindInfo->SetFound(FIND_OK);
			else
				pFindInfo->SetFound(FIND_MATCH);

			pFindInfo->SetScore(dbMaxScore);
			pFindInfo->SetXPos(dbMaxPosX);
			pFindInfo->SetYPos(dbMaxPosY);
			pFindInfo->SetFoundPatternNum(nMaxIndex);
			pFindInfo->SetAngle(dbMaxAngle);
		}
		else
		{
			pFindInfo->SetFound(FIND_ERR);
			pFindInfo->SetXPos(0.0);
			pFindInfo->SetYPos(0.0);
			pFindInfo->SetScore(0.0);
			pFindInfo->SetAngle(0.0);
		}
	}

	bSuccess = TRUE;
	return bSuccess;
}

BOOL CPatternMatching::findPatternEdgeLine(BYTE *pImage, CRect rcInspROI, int nCam, int nPos, int m_nWidth, int m_nHeight, CFindInfo *pFindInfo)
{
	if (nPos == 0) return TRUE;

	BOOL bSuccess = FALSE;
	BOOL bFind = FALSE;

	cv::Rect rect;
	rect.x = rcInspROI.left;
	rect.y = rcInspROI.top;
	rect.width = rcInspROI.Width();
	rect.height = rcInspROI.Height();

	if (pFindInfo == NULL)		pFindInfo = &m_FindInfo[nCam][nPos];
	pFindInfo->SetFound(FIND_ERR);

	cv::Mat imgSrc(m_nHeight, m_nWidth, CV_8UC1, pImage);
	cv::Mat  pCropImage;

	pCropImage = imgSrc(rect).clone();

	medianBlur(pCropImage, pCropImage, 5);

	switch (nPos)
	{
	case 0:sobelDirection(3, SOBEL_UP, pCropImage, pCropImage);			break;
	case 1:sobelDirection(3, SOBEL_RIGHT, pCropImage, pCropImage);		break;
	case 2:sobelDirection(3, SOBEL_DN, pCropImage, pCropImage);			break;
	case 3:sobelDirection(3, SOBEL_LEFT, pCropImage, pCropImage);		break;
	}

	
	int nSobelThreshold = 200;
	int nRemoveNoiseSize = 20;

	cv::threshold(pCropImage, pCropImage, nSobelThreshold, 255, CV_THRESH_BINARY);
	removeNoise(pCropImage, nRemoveNoiseSize);

	int nMaxY = 0;
	int nMaxValue = 0;
	int nMaxX = 0;

	int nRange = 3;

	int *pHori = new int[rect.width];
	int *pVert = new int[rect.height];
	memset(pHori, 0, rect.width * sizeof(int));
	memset(pVert, 0, rect.height * sizeof(int));

	if (nPos == 0 || nPos == 2)
	{
		for (int y = rect.height - 5; y > 0; y--)
		{
			for (int x = 0; x < rect.width; x++)
			{
				if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)			pVert[y]++;
			}

			if (pVert[y] > rect.width / 4)
			{
				nMaxValue = pVert[y];
				nMaxY = y;
				break;
			}
		}

		nRange = 10;
		for (int y = 0; y < rect.height; y++)
		{
			if (y > (nMaxY - nRange) && y < (nMaxY + nRange))
				memset(&pCropImage.data[y * pCropImage.cols], 0, pCropImage.cols);
		}

		memset(pHori, 0, rect.width * sizeof(int));
		memset(pVert, 0, rect.height * sizeof(int));
		nRange = 3;

		for (int y = rect.height - 5; y > 0; y--)
		{
			for (int x = 0; x < rect.width; x++)
			{
				if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)			pVert[y]++;
			}

			if (pVert[y] > 30)
			{
				bFind = TRUE;
				nMaxValue = pVert[y];
				nMaxY = y;
				break;
			}
		}

		for (int y = 0; y < rect.height; y++)
		{
			if (y <(nMaxY - nRange) || y>(nMaxY + nRange))
				memset(&pCropImage.data[y * pCropImage.cols], 0, pCropImage.cols);
		}

		for (int x = 0; x < rect.width; x++)
		{
			for (int y = rect.height - 5; y > 0; y--)
			{
				if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
				{
					pHori[x] = y;
					break;
				}
			}
		}

		double *lx = new double[rect.width];
		double *ly = new double[rect.width];
		int id = 0;
		for (int x = 0; x < rect.width; x++)
		{
			if (pHori[x] >= nMaxY - nRange && pHori[x] <= nMaxY + nRange)
			{
				lx[id] = x + rect.x;
				ly[id] = pHori[x] + rect.y;
				id++;
			}
		}

		LeastSqrRegression(lx, ly, id, m_ax[nCam][nPos][0], m_bc[nCam][nPos][0]);

		delete lx;
		delete ly;
	}
	else
	{
		if (nPos == 3)
		{
			for (int x = 0; x < rect.width; x++)
			{
				for (int y = 0; y < rect.height; y++)
				{
					if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
						pHori[x]++;
				}

				if (pHori[x] > rect.height / 4)
				{
					nMaxValue = pHori[x];
					nMaxX = x;
					break;
				}
			}
		}
		else
		{
			for (int x = rect.width - 1; x > 0; x--)
			{
				for (int y = 0; y < rect.height; y++)
				{
					if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
						pHori[x]++;
				}

				if (pHori[x] > rect.height / 4)
				{
					nMaxValue = pHori[x];
					nMaxX = x;
					break;
				}
			}
		}

		nRange = 10;

		for (int x = 0; x < rect.width; x++)
		{
			if (x > (nMaxX - nRange) && x < (nMaxX + nRange))
			{
				for (int y = 0; y < rect.height; y++)
					pCropImage.data[y * pCropImage.cols + x] = 0;
			}
		}

		memset(pHori, 0, rect.width * sizeof(int));
		memset(pVert, 0, rect.height * sizeof(int));
		nRange = 3;

		if (nPos == 3)
		{
			for (int x = 0; x < rect.width; x++)
			{
				for (int y = 0; y < rect.height; y++)
				{
					if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
						pHori[x]++;
				}

				if (pHori[x] > 20)
				{
					bFind = TRUE;
					nMaxValue = pHori[x];
					nMaxX = x;
					break;
				}
			}
		}
		else
		{
			for (int x = rect.width - 1; x > 0; x--)
			{
				for (int y = 0; y < rect.height; y++)
				{
					if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
						pHori[x]++;
				}

				if (pHori[x] > 30)
				{
					bFind = TRUE;
					nMaxValue = pHori[x];
					nMaxX = x;
					break;
				}
			}
		}

		if (nPos == 3)
		{
			for (int y = 0; y < rect.height; y++)
			{
				for (int x = 0; x < rect.width; x++)
				{
					if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
					{
						pVert[y] = x;
						break;
					}
				}
			}
		}
		else
		{
			for (int y = 0; y < rect.height; y++)
			{
				for (int x = rect.width - 1; x > 0; x--)
				{
					if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
					{
						pVert[y] = x;
						break;
					}
				}
			}
		}


		double *lx = new double[rect.height];
		double *ly = new double[rect.height];
		int id = 0;
		for (int y = 0; y < rect.height; y++)
		{
			if (pVert[y] >= nMaxX - nRange && pVert[y] <= nMaxX + nRange)
			{
				ly[id] = pVert[y] + rect.x;
				lx[id] = y + rect.y;
				id++;
			}
		}

		LeastSqrRegression(lx, ly, id, m_ax[nCam][nPos][0], m_bc[nCam][nPos][0]);

		delete lx;
		delete ly;
	}

	delete pHori;
	delete pVert;

	imgSrc.release();
	pCropImage.release();

	bSuccess = bFind;

	return bSuccess;
}

void CPatternMatching::LeastSqrRegression(double* x, double* y, int obs, double &a, double &b)
{
	double sum_x = 0;
	double sum_y = 0;
	double sum_xy = 0;
	double sum_xx = 0;
	double sum_squared_residue = 0;
	double residue_sqaured = 0;
	double slope = 0;
	double intercept = 0;
	double sum_squared_discrepancies = 0;
	double discrepancies_squared = 0;
	double avg_x = 0;
	double avg_y = 0;
	double r_squared = 0;

	for (int i = 0; i < obs; i++)
	{
		sum_x += x[i];
		sum_y += y[i];
		sum_xy += x[i] * y[i];
		sum_xx += x[i] * x[i];
	}

	avg_x = sum_x / obs;
	avg_y = sum_y / obs;

	a = slope = (obs*sum_xy - sum_x * sum_y) / (obs*sum_xx - sum_x * sum_x);
	b = intercept = avg_y - slope * avg_x;

	for (int i = 0; i < obs; i++)
	{
		discrepancies_squared = pow(y[i] - intercept - (slope*x[i]), 2);
		sum_squared_discrepancies += discrepancies_squared;
		residue_sqaured = pow(y[i] - avg_y, 2);
		sum_squared_residue += residue_sqaured;
	}
	r_squared = (sum_squared_residue - sum_squared_discrepancies) / sum_squared_residue;
}

void CPatternMatching::calcHistogram(cv::Mat& srcImage, int(&Histo)[256])
{
	int i, x, y;

	for (i = 0; i < 256; i++) Histo[i] = 0;

	for (y = 0; y < srcImage.rows; y++)
	{
		for (x = 0; x < srcImage.cols; x++)
		{
			Histo[(uchar)srcImage.data[y * srcImage.cols + x]]++;
		}
	}
}

void CPatternMatching::applyLUT(cv::Mat& srcImage, cv::Mat& dstImage, int(&LUT)[256])
{
	for (int y = 0; y < srcImage.rows; y++)
	{
		for (int x = 0; x < srcImage.cols; x++)
		{
			dstImage.data[y * dstImage.cols + x] = (char)LUT[(uchar)srcImage.data[y * srcImage.cols + x]];
		}
	}
}


void CPatternMatching::calcHistogram(BYTE *srcImage, int w, int h, int(&Histo)[256])
{
	int i, x, y;

	for (i = 0; i < 256; i++) Histo[i] = 0;

	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			Histo[srcImage[y * w + x]]++;
		}
	}
}

void CPatternMatching::applyLUT(BYTE * srcImage, BYTE * dstImage, int w, int h, int(&LUT)[256])
{
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			dstImage[y * w + x] = LUT[srcImage[y * w + x]];
		}
	}
}



void CPatternMatching::endInSearch(cv::Mat *matsrcImage, cv::Mat *matdstImage)
{
	int i;

	int histogram[256];
	int LUT[256];
	int lowthresh, highthresh;
	float scale_factor;

	lowthresh = 0;
	highthresh = 255;//히스토그램명도값의빈도수조사

	// 히스토그램 계산
	calcHistogram(matsrcImage->data, matsrcImage->cols, matsrcImage->rows, histogram);

	//히스토그램중가장낮은명도값계산.
	for (i = 0; i < 256; i++)
	{
		if (histogram[i])
		{
			lowthresh = i;
			break;
		}
	}

	for (i = 255; i >= 0; i--)
	{
		if (histogram[i])
		{
			highthresh = i;
			break;
		}
	}

	for (i = 0; i < lowthresh; i++) 		LUT[i] = 0;
	for (i = 255; i > highthresh; i--) 		LUT[i] = 255;

	scale_factor = (float)(255.0 / (highthresh - lowthresh));

	for (i = lowthresh; i <= highthresh; i++) 	LUT[i] = int((i - lowthresh) * scale_factor);

	applyLUT(matsrcImage->data, matdstImage->data, matsrcImage->cols, matsrcImage->rows, LUT);
}

void CPatternMatching::DoRemoveNoise(int dMode, int width, int height, int widthStep, int i, int j, int threshold, int &hole, BOOL &is_hole, unsigned char *image, unsigned char *label_image)
{
	if (i <= 0 || j <= 0 || i >= height - 1 || j >= width - 1) return;

	switch (dMode)
	{
	case 0:			// check
		if (is_hole == FALSE) return;

		hole++;

		label_image[i*width + j] = 255;

		if (hole > threshold)
		{
			is_hole = FALSE;
			return;
		}
		if (label_image[(i - 1)*width + j] != 255)
		{
			if (image[(i - 1)*widthStep + j] == 255) DoRemoveNoise(dMode, width, height, widthStep, i - 1, j, threshold, hole, is_hole, image, label_image);
		}

		if (label_image[i*width + j - 1] != 255)
		{
			if (image[i*widthStep + j - 1] == 255) DoRemoveNoise(dMode, width, height, widthStep, i, j - 1, threshold, hole, is_hole, image, label_image);
		}

		if (label_image[i*width + j + 1] != 255)
		{
			if (image[i*widthStep + j + 1] == 255) DoRemoveNoise(dMode, width, height, widthStep, i, j + 1, threshold, hole, is_hole, image, label_image);
		}

		if (label_image[(i + 1)*width + j] != 255)
		{
			if (image[(i + 1)*widthStep + j] == 255) DoRemoveNoise(dMode, width, height, widthStep, i + 1, j, threshold, hole, is_hole, image, label_image);
		}
		break;

	case 1:			// remove
		image[i*widthStep + j] = 0;

		if (label_image[(i - 1)*width + j] == 255)
		{
			if (image[(i - 1)*widthStep + j] == 255)
			{
				image[(i - 1)*widthStep + j] = 255;
				DoRemoveNoise(dMode, width, height, widthStep, i - 1, j, threshold, hole, is_hole, image, label_image);
			}
		}

		if (label_image[i*width + j - 1] == 255)
		{
			if (image[i*widthStep + j - 1] == 255)
			{
				image[i*widthStep + j - 1] = 0;
				DoRemoveNoise(dMode, width, height, widthStep, i, j - 1, threshold, hole, is_hole, image, label_image);
			}
		}

		if (label_image[i*width + j + 1] == 255)
		{
			if (image[i*widthStep + j + 1] == 255)
			{
				image[i*widthStep + j + 1] = 255;
				DoRemoveNoise(dMode, width, height, widthStep, i, j + 1, threshold, hole, is_hole, image, label_image);
			}
		}

		if (label_image[(i + 1)*width + j] == 255)
		{
			if (image[(i + 1)*widthStep + j] == 255)
			{
				image[(i + 1)*widthStep + j] = 255;
				DoRemoveNoise(dMode, width, height, widthStep, i + 1, j, threshold, hole, is_hole, image, label_image);
			}
		}
		break;

	case 2:			// clear
		label_image[i*width + j] = 0;
		if (label_image[(i - 1)*width + j] == 255)
		{
			if (image[(i - 1)*widthStep + j] == 255)
			{
				label_image[(i - 1)*width + j] = 0;
				DoRemoveNoise(dMode, width, height, widthStep, i - 1, j, threshold, hole, is_hole, image, label_image);
			}
		}

		if (label_image[i*width + j - 1] == 255)
		{
			if (image[i*widthStep + j - 1] == 255)
			{
				label_image[i*width + j - 1] = 0;
				DoRemoveNoise(dMode, width, height, widthStep, i, j - 1, threshold, hole, is_hole, image, label_image);
			}
		}

		if (label_image[i*width + j + 1] == 255)
		{
			if (image[i*widthStep + j + 1] == 255)
			{
				label_image[i*width + j + 1] = 0;
				DoRemoveNoise(dMode, width, height, widthStep, i, j + 1, threshold, hole, is_hole, image, label_image);
			}
		}

		if (label_image[(i + 1)*width + j] == 255)
		{
			if (image[(i + 1)*widthStep + j] == 255)
			{
				label_image[(i + 1)*width + j] = 0;
				DoRemoveNoise(dMode, width, height, widthStep, i + 1, j, threshold, hole, is_hole, image, label_image);
			}
		}
		break;
	}
}

void CPatternMatching::removeNoise(cv::Mat matImage, int nSize)
{
	// noise 제거
	unsigned char *pLabelBuf, *pBuffer;
	int		nWidthStep;
	int		nHoleCount;
	BOOL	bIsHole;
	int		i, j;
	int		nCropW, nCropH;
	pBuffer = (BYTE*)matImage.data;
	nWidthStep = matImage.cols;
	int nThresholdSize = nSize;

	nCropW = matImage.cols;
	nCropH = matImage.rows;

	pLabelBuf = new unsigned char[nCropW * nCropH];
	ZeroMemory(pLabelBuf, sizeof(BYTE)*nCropW * nCropH);

	for (i = 1; i < nCropH - 1; i++)
	{
		for (j = 1; j < nCropW - 1; j++)
		{
			if (pBuffer[i*nWidthStep + (j - 1)] == 0 && pBuffer[(i - 1)*nWidthStep + j] == 0 && pBuffer[i*nWidthStep + j] == 255 && pLabelBuf[i*nCropW + j] == 0)
			{
				bIsHole = TRUE;
				nHoleCount = 0;
				DoRemoveNoise(0, nCropW, nCropH, nWidthStep, i, j, nThresholdSize, nHoleCount, bIsHole, pBuffer, pLabelBuf);

				if (bIsHole == TRUE)
				{
					DoRemoveNoise(1, nCropW, nCropH, nWidthStep, i, j, nThresholdSize, nHoleCount, bIsHole, pBuffer, pLabelBuf);
				}
				else
				{
					DoRemoveNoise(2, nCropW, nCropH, nWidthStep, i, j, nThresholdSize, nHoleCount, bIsHole, pBuffer, pLabelBuf);
				}
			}
		}
	}

	delete[] pLabelBuf;
}

void CPatternMatching::sobelDirection(int nMaskSize, int nDirection, cv::Mat &srcImage, cv::Mat &dstImage)
{
	cv::Mat mask =cv::Mat::ones(nMaskSize, nMaskSize, CV_32F) / 25;

	float* pBuffer = (float*)mask.data;

	if (nMaskSize == 3)
	{
		switch (nDirection) {
		case SOBEL_UP:
			pBuffer[0] = 1;	pBuffer[1] = 2;	pBuffer[2] = 1;
			pBuffer[3] = 0;	pBuffer[4] = 0;	pBuffer[5] = 0;
			pBuffer[6] = -1;	pBuffer[7] = -2;	pBuffer[8] = -1;
			break;
		case SOBEL_DN:
			pBuffer[0] = -1;	pBuffer[1] = -2;	pBuffer[2] = -1;
			pBuffer[3] = 0;	pBuffer[4] = 0;	pBuffer[5] = 0;
			pBuffer[6] = 1;	pBuffer[7] = 2;	pBuffer[8] = 1;
			break;
		case SOBEL_LEFT:
			pBuffer[0] = 1;	pBuffer[1] = 0;	pBuffer[2] = -1;
			pBuffer[3] = 2;	pBuffer[4] = 0;	pBuffer[5] = -2;
			pBuffer[6] = 1;	pBuffer[7] = 0;	pBuffer[8] = -1;
			break;
		case SOBEL_RIGHT:
			pBuffer[0] = -1;	pBuffer[1] = 0;	pBuffer[2] = 1;
			pBuffer[3] = -2;	pBuffer[4] = 0;	pBuffer[5] = 2;
			pBuffer[6] = -1;	pBuffer[7] = 0;	pBuffer[8] = 1;
			break;
		};
	}

	filter2D(srcImage, dstImage, -1, mask, cv::Point(-1, -1), (0, 0), 2);
	mask.release();
}

bool CPatternMatching::GetIntersectPoint(const UPoint <double> & AP1, const UPoint <double>& AP2,
	const UPoint <double>& BP1, const UPoint <double>& BP2, UPoint <double>* IP)
{
	double t;
	double s;
	double under = (BP2.y - BP1.y)*(AP2.x - AP1.x) - (BP2.x - BP1.x)*(AP2.y - AP1.y);
	if (under == 0) return false;

	double _t = (BP2.x - BP1.x)*(AP1.y - BP1.y) - (BP2.y - BP1.y)*(AP1.x - BP1.x);
	double _s = (AP2.x - AP1.x)*(AP1.y - BP1.y) - (AP2.y - AP1.y)*(AP1.x - BP1.x);

	t = _t / under;
	s = _s / under;

	//  if(t<0.0 || t>1.0 || s<0.0 || s>1.0) return false;
	if (_t == 0 && _s == 0) return false;

	IP->x = AP1.x + t * (double)(AP2.x - AP1.x);
	IP->y = AP1.y + t * (double)(AP2.y - AP1.y);

	return true;
}

BOOL CPatternMatching::findMatchingEdgeLine(	// findMatchingEdgeLine
	BYTE *pImage,				// 원본 이미지
	CRect roi,					// ROI
	int search_hori,			// ROI 외곽에서 찾을 범위
	int search_vert,			// ROI 외곽에서 찾을 범위
	int w,						// 이미지 Width
	int h,						// 이미지 Height
	bool bLeft,					// 좌 / 우 엣지 구분.
	int nRemoveNoiseSize,		// 잡음 제거 사이즈
	int nSobelTheshold,			// Sobel Filter 후 Binary Threshold
	int gauss, bool white)
{
	int nCam = 0;
	int nPos = 0;
	if (bLeft != true) nPos = 1;

	BOOL bFind[2] = { FALSE, FALSE };
	cv::Rect rect;
	rect.x = (bLeft) ? roi.right - search_hori : roi.left;
	rect.y = roi.bottom - search_hori;
	rect.width = search_hori - 1;
	rect.height = search_hori - 1;

	int nRange = 3;

	//bottom라인 찾기
	{
		cv::Mat imgSrc(h, w, CV_8UC1, pImage);
		cv::Mat  pCropImage;

		pCropImage = imgSrc(rect).clone();

		medianBlur(pCropImage, pCropImage, gauss);

		if (white)	sobelDirection(3, SOBEL_UP, pCropImage, pCropImage);
		else		sobelDirection(3, SOBEL_DN, pCropImage, pCropImage);

		// 이진화l
		cv::threshold(pCropImage, pCropImage, nSobelTheshold, 255, CV_THRESH_BINARY);

		// 잡음 제거
		removeNoise(pCropImage, nRemoveNoiseSize);

		int *pHori = new int[rect.width];
		int *pVert = new int[rect.height];
		memset(pHori, 0, rect.width * sizeof(int));
		memset(pVert, 0, rect.height * sizeof(int));

		int nMaxY = 0;
		int nMaxValue = 0;

		// 프로젝션하여 ROI의 1/4 이상 되는 지점에서 정지
		for (int y = rect.height - 5; y > 0; y--)
		{
			for (int x = 0; x < rect.width; x++)
			{
				if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)			pVert[y]++;
			}

			if (pVert[y] > rect.width / 4)
			{
				bFind[0] = TRUE;
				nMaxValue = pVert[y];
				nMaxY = y;
				break;
			}
		}

		// 프로젝션 정지 위치에서 주변 range 범위를 제외한 나머지 영역을 다 날린다.
		for (int y = 0; y < rect.height; y++)
		{
			if (y < (nMaxY - nRange) || y >(nMaxY + nRange))
				memset(&pCropImage.data[y * pCropImage.cols], 0, pCropImage.cols);
		}

		// 칸투어를 구한다.
		for (int x = 0; x < rect.width; x++)
		{
			for (int y = rect.height - 5; y > 0; y--)
			{
				if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
				{
					pHori[x] = y;
					break;
				}
			}
		}

		double *lx = new double[rect.width];
		double *ly = new double[rect.width];
		int id = 0;
		for (int x = 0; x < rect.width; x++)
		{
			if (pHori[x] >= nMaxY - nRange && pHori[x] <= nMaxY + nRange)
			{
				lx[id] = x + rect.x;
				ly[id] = pHori[x] + rect.y;
				id++;
			}
		}

		// 회귀 분석하여 직선 방정식을 구한다.
		// 도포 외곽 엣지 성분
		LeastSqrRegression(lx, ly, id, m_ax[nCam][nPos][0], m_bc[nCam][nPos][0]);

		delete pHori;
		delete pVert;
		delete lx;
		delete ly;

		imgSrc.release();
		pCropImage.release();
	}

	{
		if (bLeft)
		{
			rect.x = roi.left;
			rect.y = roi.top;
			rect.width = search_vert;
			rect.height = search_vert;
		}
		else
		{
			rect.x = roi.right - search_vert;
			rect.y = roi.top;
			rect.width = search_vert;
			rect.height = search_vert;
		}


		cv::Mat imgSrc(h, w, CV_8UC1, pImage);
		cv::Mat  pCropImage;

		pCropImage = imgSrc(rect).clone();
		medianBlur(pCropImage, pCropImage, gauss);


		if (white)
		{
			if (!bLeft)	sobelDirection(3, SOBEL_LEFT, pCropImage, pCropImage);
			else		sobelDirection(3, SOBEL_RIGHT, pCropImage, pCropImage);
		}
		else
		{
			if (bLeft)	sobelDirection(3, SOBEL_LEFT, pCropImage, pCropImage);
			else		sobelDirection(3, SOBEL_RIGHT, pCropImage, pCropImage);
		}

		
		cv::threshold(pCropImage, pCropImage, nSobelTheshold, 255, CV_THRESH_BINARY);
		removeNoise(pCropImage, nRemoveNoiseSize);

		int *pHori = new int[rect.width];
		int *pVert = new int[rect.height];
		memset(pHori, 0, rect.width * sizeof(int));
		memset(pVert, 0, rect.height * sizeof(int));

		int nMaxX = 0;
		int nMaxValue = 0;
		if (bLeft)
		{
			for (int x = 0; x < rect.width; x++)
			{
				for (int y = 0; y < rect.height; y++)
				{
					if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
						pHori[x]++;
				}

				if (pHori[x] > rect.height / 4)
				{
					bFind[1] = TRUE;
					nMaxValue = pHori[x];
					nMaxX = x;
					break;
				}
			}
		}
		else
		{
			for (int x = rect.width - 1; x > 0; x--)
			{
				for (int y = 0; y < rect.height; y++)
				{
					if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
						pHori[x]++;
				}

				if (pHori[x] > rect.height / 4)
				{
					bFind[1] = TRUE;
					nMaxValue = pHori[x];
					nMaxX = x;
					break;
				}
			}
		}

		for (int x = 0; x < rect.width; x++)
		{
			if (x < (nMaxX - nRange) || x >(nMaxX + nRange))
			{
				for (int y = 0; y < rect.height; y++)
					pCropImage.data[y * pCropImage.cols + x] = 0;
			}
		}

		if (bLeft)
		{
			for (int y = 0; y < rect.height; y++)
			{
				for (int x = 0; x < rect.width; x++)
				{
					if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
					{
						pVert[y] = x;
						break;
					}
				}
			}
		}
		else
		{
			for (int y = 0; y < rect.height; y++)
			{
				for (int x = rect.width - 1; x > 0; x--)
				{
					if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
					{
						pVert[y] = x;
						break;
					}
				}
			}
		}

		double *lx = new double[rect.height];
		double *ly = new double[rect.height];
		int id = 0;
		for (int y = 0; y < rect.height; y++)
		{
			if (pVert[y] >= nMaxX - nRange && pVert[y] <= nMaxX + nRange)
			{
				ly[id] = pVert[y] + rect.x;
				lx[id] = y + rect.y;
				id++;
			}
		}

		// 도포 외곽 엣지 성분
		LeastSqrRegression(lx, ly, id, m_ax[nCam][nPos][1], m_bc[nCam][nPos][1]);

		delete pHori;
		delete pVert;
		delete lx;
		delete ly;

		imgSrc.release();
		pCropImage.release();
	}

	if (bFind[0] == FALSE || bFind[1] == FALSE)	return FALSE;
	else											return TRUE;
}



bool CPatternMatching::find_in_samples(sPoint *samples, int no_samples, sPoint *data)
{
	for (int i = 0; i < no_samples; ++i) {
		if (samples[i].x == data->x && samples[i].y == data->y) {
			return true;
		}
	}
	return false;
}

void CPatternMatching::get_samples(sPoint *samples, int no_samples, sPoint *data, int no_data)
{
	// 데이터에서 중복되지 않게 N개의 무작위 셈플을 채취한다.
	for (int i = 0; i < no_samples; ) {
		int j = rand() % no_data;

		if (!find_in_samples(samples, i, &data[j])) {
			samples[i] = data[j];
			++i;
		}
	};
}

int CPatternMatching::compute_model_parameter(sPoint samples[], int no_samples, sLine &model)
{
	// PCA 방식으로 직선 모델의 파라메터를 예측한다.

	double sx = 0, sy = 0;
	double sxx = 0, syy = 0;
	double sxy = 0, sw = 0;

	for (int i = 0; i < no_samples; ++i)
	{
		double &x = samples[i].x;
		double &y = samples[i].y;

		sx += x;
		sy += y;
		sxx += x * x;
		sxy += x * y;
		syy += y * y;
		sw += 1;
	}

	//variance;
	double vxx = (sxx - sx * sx / sw) / sw;
	double vxy = (sxy - sx * sy / sw) / sw;
	double vyy = (syy - sy * sy / sw) / sw;

	//principal axis
	double theta = atan2(2 * vxy, vxx - vyy) / 2;

	model.mx = cos(theta);
	model.my = sin(theta);

	//center of mass(xc, yc)
	model.sx = sx / sw;
	model.sy = sy / sw;

	//직선의 방정식: sin(theta)*(x - sx) = cos(theta)*(y - sy);

	double x1 = model.sx - 300 * model.mx;
	double y1 = model.sy - 300 * model.my;
	double x2 = model.sx + 300 * model.mx;
	double y2 = model.sy + 300 * model.my;

	if (x1 == x2)
	{
		model.a = 0;
		model.b = x1;
	}
	else
	{
		model.a = (y2 - y1) / (x2 - x1);
		model.b = y1 - (model.a * x1);
	}

	return 1;
}

double CPatternMatching::compute_distance(sLine &line, sPoint &x)
{
	// 한 점(x)로부터 직선(line)에 내린 수선의 길이(distance)를 계산한다.

	return fabs((x.x - line.sx) * line.my - (x.y - line.sy) * line.mx) / sqrt(line.mx * line.mx + line.my * line.my);
}

void CPatternMatching::compute_orthogonal_point_on_line(sLine &line, sPoint &x, sPoint &point)
{
	double normal_a = 0.0;
	double normal_b = 0.0;

	if (line.a != 0.0)
	{
		normal_a = -1.0 / line.a;
		normal_b = x.y - normal_a * x.x;
		cramersRules(-normal_a, 1, -line.a, 1, normal_b, line.b, &point.x, &point.y);
	}
	else
	{
		// line이 수직이라면,
		point.x = line.b;
		point.y = x.x;
	}
}


double CPatternMatching::model_verification(sPoint *inliers, int *no_inliers, sLine &estimated_model, sPoint *data, int no_data, double distance_threshold)
{
	*no_inliers = 0;

	double cost = 0.;

	for (int i = 0; i < no_data; i++) {
		// 직선에 내린 수선의 길이를 계산한다.
		double distance = compute_distance(estimated_model, data[i]);

		// 예측된 모델에서 유효한 데이터인 경우, 유효한 데이터 집합에 더한다.
		if (distance < distance_threshold) {
			cost += 1.;

			inliers[*no_inliers] = data[i];
			++(*no_inliers);
		}
	}

	return cost;
}

double CPatternMatching::ransac_line_fitting(sPoint *data, int no_data, sLine &model, double distance_threshold)
{
	const int no_samples = 2;

	if (no_data < no_samples) {
		return 0.;
	}

	sPoint *samples = new sPoint[no_samples];

	int no_inliers = 0;
	sPoint *inliers = new sPoint[no_data];

	sLine estimated_model;
	double max_cost = 0.;

	int max_iteration = (int)(1 + log(1. - 0.99) / log(1. - pow(0.5, no_samples)));

	for (int i = 0; i < max_iteration; i++) {
		// 1. hypothesis

		// 원본 데이터에서 임의로 N개의 셈플 데이터를 고른다.
		get_samples(samples, no_samples, data, no_data);

		// 이 데이터를 정상적인 데이터로 보고 모델 파라메터를 예측한다.
		compute_model_parameter(samples, no_samples, estimated_model);

		// 2. Verification

		// 원본 데이터가 예측된 모델에 잘 맞는지 검사한다.
		double cost = model_verification(inliers, &no_inliers, estimated_model, data, no_data, distance_threshold);

		// 만일 예측된 모델이 잘 맞는다면, 이 모델에 대한 유효한 데이터로 새로운 모델을 구한다.
		if (max_cost < cost) {
			max_cost = cost;

			compute_model_parameter(inliers, no_inliers, model);
		}
	}

	delete[] samples;
	delete[] inliers;

	return max_cost;
}
BOOL CPatternMatching::findCornerEdgeByRansac2(
	BYTE *pOrgImage,					// 원본 이미지
	int width,						// 원본 이미지의 Width
	int height,						// 원본 이미지의 Height
	int nCornerType,					// Glass Edge의 사분면
	int nSobelDirectionHori,		// Sobel Filter 적용 방향
	int nSobelDirectionVert,		// Sobel Filter 적용 방향
	int nNoiseSize_Hori,			// 잡음 제거 사이즈
	int nNoiseSize_Vert,			// 잡음 제거 사이즈
	int nThreshold_Hori,					// Sobel Edge Binarization Threshold
	int nThreshold_Vert,					// Sobel Edge Binarization Threshold
	CRect rcROI,					// ROI
	int nHoriProjectionArea,		// 가로 성분을 찾기 위한 범위
	int nVertProjectionArea,		// 세로 성분을 찾기 위한 범위
	int nHoriSearchArea,			// 가로 성분을 찾기 위한 범위
	int nVertSearchArea,			// 세로 성분을 찾기 위한 범위
	sLine *pLineHori,					// 선 정보
	sLine *pLineVert,					// 선 정보
	int nSearchOffset,
	BOOL bInnerToOuter,
	int nCam,
	int nPos
)
{

	cv::Mat imgSrc(height, width, CV_8UC1, pOrgImage);
	cv::Mat  pVertImage , pHoriImage;

	BOOL bFind[2] = { TRUE, TRUE };

	//int nSize = 600;
	int nSize = nHoriProjectionArea;
	CvRect cvRectROI;


	nHoriSearchArea = nHoriSearchArea / 4 * 4;
	nVertSearchArea = nVertSearchArea / 4 * 4;

	BOOL bRet = TRUE;

	//		|												|			----------------|	|-----------------
	//		|												|							|	|
	//		|				1사분면			2사분면			|				3사분면		|	|	4 사분면
	//		|												|							|	|
	//		|----------------				-----------------							|	|

	// 가로성분 찾기	: Y 좌표
	switch (nCornerType) {
	case CORNER_LB:			// 1사분면
		cvRectROI.x = rcROI.right - nSize - 1;
		cvRectROI.y = rcROI.bottom - 1 - nHoriSearchArea > 0 ? rcROI.bottom - 1 - nHoriSearchArea : 0;
		break;
	case CORNER_RB:			// 2사분면
		//cvRectROI.x = 0;
		cvRectROI.x = rcROI.left; // 19.09.03 임시
		cvRectROI.y = rcROI.bottom - 1 - nHoriSearchArea > 0 ? rcROI.bottom - 1 - nHoriSearchArea : 0;
		break;
	case CORNER_RT:			// 3사분면
		cvRectROI.x = rcROI.left;
		cvRectROI.y = rcROI.top;
		break;
	case CORNER_LT:			// 4사분면
		cvRectROI.x = rcROI.right - nSize - 1;
		cvRectROI.y = rcROI.top;
		break;
	}

	if (cvRectROI.x < 0) cvRectROI.x = 0;
	if (cvRectROI.y < 0) cvRectROI.y = 0;

	cvRectROI.width = nSize;
	cvRectROI.height = nHoriSearchArea;

	if (cvRectROI.x + cvRectROI.width >= width) cvRectROI.x = width - nSize - 1;
	if (cvRectROI.y + cvRectROI.height >= height) cvRectROI.y = height - nHoriSearchArea - 1;


	pHoriImage = imgSrc(cvRectROI).clone();

	// 전처리
	preProcessImage(nCam, nPos, pHoriImage);

	cv::Mat sgray(pHoriImage.rows, pHoriImage.cols, CV_8UC1, pHoriImage.data);

	// 필터 적용
	filterProcessImage_hori(nCam, nPos, pHoriImage, &sgray, nSobelDirectionHori, nThreshold_Hori);

	// 잡음제거
	if (nNoiseSize_Hori > 0)	removeNoise(pHoriImage, nNoiseSize_Hori);

	// 가로 선을 찾자!!
	if (m_nEdgeProcess[nCam][nPos] == EDGE_HISTO_W || m_nEdgeProcess[nCam][nPos] == EDGE_HISTO_B)		bFind[0] = findHisto_Hori(nCam, pHoriImage, nSize, nHoriSearchArea, nCornerType, cvRectROI, bInnerToOuter, pLineHori);
	else																								bFind[0] = findLine_Hori(nCam, pHoriImage, nSize, nHoriSearchArea, nCornerType, cvRectROI, bInnerToOuter, pLineHori);

	pHoriImage.release();


	//		|												|			----------------|	|-----------------
	//		|												|							|	|
	//		|				1사분면			2사분면			|				3사분면		|	|	4 사분면
	//		|												|							|	|
	//		|----------------				-----------------							|	|

	//nSize = 500;
	nSize = nVertProjectionArea;
	nSize = nSize / 4 * 4;
	int nbottomSize = imgSrc.rows - rcROI.bottom > 0 ? imgSrc.rows - rcROI.bottom : 100;

	// 세로성분 찾기	: X 좌표
	switch (nCornerType) {
	case CORNER_LB:			// 1사분면
		cvRectROI.x = rcROI.left + nVertSearchArea < imgSrc.cols - 1 ? rcROI.left : imgSrc.cols - 1 - nVertSearchArea;
		cvRectROI.y = rcROI.top;
		break;
	case CORNER_RB:			// 2사분면
		cvRectROI.x = rcROI.right - nVertSearchArea > 0 ? rcROI.right - nVertSearchArea : 0;
		cvRectROI.y = rcROI.top; // 19.09.03 임시
		break;
	case CORNER_RT:			// 3사분면
		cvRectROI.x = rcROI.right - nVertSearchArea > 0 ? rcROI.right - nVertSearchArea : 0;
		cvRectROI.y = rcROI.bottom - nSize;//pImage->height - 1 - nSize;
		break;
	case CORNER_LT:			// 4사분면
		cvRectROI.x = rcROI.left + nVertSearchArea < imgSrc.cols - 1 ? rcROI.left : imgSrc.cols - 1 - nVertSearchArea;
		cvRectROI.y = imgSrc.rows - nbottomSize - nSize;//pImage->height - 1 - nSize;
		break;
	}

	if (cvRectROI.x < 0) cvRectROI.x = 0;
	if (cvRectROI.y < 0) cvRectROI.y = 0;

	cvRectROI.width = nVertSearchArea;
	cvRectROI.height = nSize;

	if (cvRectROI.x + cvRectROI.width > width) cvRectROI.x = width - nVertSearchArea - 1;
	if (cvRectROI.y + cvRectROI.height > height) cvRectROI.y = height - nSize - 1;


	pVertImage = imgSrc(cvRectROI).clone();

	// 전처리
	preProcessImage(nCam, nPos, pVertImage);

	cv::Mat sgray2(pVertImage.rows, pVertImage.cols, CV_8UC1, pVertImage.data);

	// 필터 적용
	filterProcessImage_vert(nCam, nPos, pVertImage, &sgray2, nSobelDirectionVert, nThreshold_Vert);

	// 잡음 제거
	if (nNoiseSize_Vert > 0)	removeNoise(pVertImage, nNoiseSize_Vert);

	// 세로 선을 찾자!!
	if (m_nEdgeProcess[nCam][nPos] == EDGE_HISTO_W || m_nEdgeProcess[nCam][nPos] == EDGE_HISTO_B)		bFind[1] = findHisto_Vert(nCam, pVertImage, nSize, nVertSearchArea, nCornerType, cvRectROI, bInnerToOuter, pLineVert);
	else																								bFind[1] = findLine_Vert(nCam, pVertImage, nSize, nVertSearchArea, nCornerType, cvRectROI, bInnerToOuter, pLineVert);

	pVertImage.release();
	imgSrc.release();

	if (bFind[0] != TRUE || bFind[1] != TRUE)
		bRet = FALSE;

	return bRet;
}

BOOL CPatternMatching::findVerticalEdgeByRansac(
	BYTE *pOrgImage,					// 원본 이미지
	int width,						// 원본 이미지의 Width
	int height,						// 원본 이미지의 Height
	int nCornerType,					// Glass Edge의 사분면
	int nSobelDirectionVert,		// Sobel Filter 적용 방향
	int nNoiseSize_Vert,			// 잡음 제거 사이즈
	int nThreshold_Vert,					// Sobel Edge Binarization Threshold
	CRect rcROI,					// ROI
	int nVertProjectionArea,		// 세로 성분을 찾기 위한 범위
	int nVertSearchArea,			// 세로 성분을 찾기 위한 범위
	sLine *pLineVert,					// 선 정보
	int nSearchOffset,
	BOOL bInnerToOuter,
	int nCam,
	int nPos
)
{
	cv::Mat imgSrc(height, width, CV_8UC1, pOrgImage);
	cv::Mat  pVertImage;

	cv::Rect cvRectROI;
	BOOL bFind = TRUE;

	int nSize = nVertProjectionArea;
	nSize = nSize / 4 * 4;
	int nbottomSize = imgSrc.rows - rcROI.bottom > 0 ? imgSrc.rows - rcROI.bottom : 100;

	// 세로성분 찾기	: X 좌표
	switch (nCornerType) {
	case CORNER_LB:			// 1사분면
		cvRectROI.x = rcROI.left + nVertSearchArea < imgSrc.cols - 1 ? rcROI.left : imgSrc.cols - 1 - nVertSearchArea;
		cvRectROI.y = rcROI.top;
		break;
	case CORNER_RB:			// 2사분면
		cvRectROI.x = rcROI.right - nVertSearchArea > 0 ? rcROI.right - nVertSearchArea : 0;
		cvRectROI.y = rcROI.top; // 19.09.03 임시
		break;
	case CORNER_RT:			// 3사분면
		cvRectROI.x = rcROI.right - nVertSearchArea > 0 ? rcROI.right - nVertSearchArea : 0;
		cvRectROI.y = rcROI.bottom - nSize;//pImage->height - 1 - nSize;
		break;
	case CORNER_LT:			// 4사분면
		cvRectROI.x = rcROI.left + nVertSearchArea < imgSrc.cols - 1 ? rcROI.left : imgSrc.cols - 1 - nVertSearchArea;
		cvRectROI.y = imgSrc.rows - nbottomSize - nSize;//pImage->height - 1 - nSize;
		break;
	}

	if (cvRectROI.x < 0) cvRectROI.x = 0;
	if (cvRectROI.y < 0) cvRectROI.y = 0;

	cvRectROI.width = nVertSearchArea;
	cvRectROI.height = nSize;

	if (cvRectROI.x + cvRectROI.width > width) cvRectROI.x = width - nVertSearchArea - 1;
	if (cvRectROI.y + cvRectROI.height > height) cvRectROI.y = height - nSize - 1;

	pVertImage = imgSrc(cvRectROI).clone();

	// 전처리
	preProcessImage(nCam, nPos, pVertImage);

	cv::Mat sgray2(pVertImage.rows, pVertImage.cols, CV_8UC1, pVertImage.data);

	// 필터 적용
	filterProcessImage_vert(nCam, nPos, pVertImage, &sgray2, nSobelDirectionVert, nThreshold_Vert);

	// 잡음 제거
	if (nNoiseSize_Vert > 0)	removeNoise(pVertImage, nNoiseSize_Vert);

	// 세로 선을 찾자!!
	if (m_nEdgeProcess[nCam][nPos] == EDGE_HISTO_W || m_nEdgeProcess[nCam][nPos] == EDGE_HISTO_B)		bFind = findHisto_Vert(nCam, pVertImage, nSize, nVertSearchArea, nCornerType, cvRectROI, bInnerToOuter, pLineVert);
	else																								bFind = findLine_Vert(nCam, pVertImage, nSize, nVertSearchArea, nCornerType, cvRectROI, bInnerToOuter, pLineVert);

	pVertImage.release();
	imgSrc.release();

	return bFind;
}

BOOL CPatternMatching::findCornerEdgeByRansac(
	BYTE *pOrgImage,					// 원본 이미지
	int width,						// 원본 이미지의 Width
	int height,						// 원본 이미지의 Height
	int nCornerType,					// Glass Edge의 사분면
	int nSobelDirectionHori,		// Sobel Filter 적용 방향
	int nSobelDirectionVert,		// Sobel Filter 적용 방향
	int nNoiseSize_Hori,			// 잡음 제거 사이즈
	int nNoiseSize_Vert,			// 잡음 제거 사이즈
	int nThreshold_Hori,					// Sobel Edge Binarization Threshold
	int nThreshold_Vert,					// Sobel Edge Binarization Threshold
	CRect rcROI,					// ROI
	int nHoriSearchArea,			// 가로 성분을 찾기 위한 범위
	int nVertSearchArea,			// 세로 성분을 찾기 위한 범위
	sLine *pLineHori,					// 선 정보
	sLine *pLineVert,					// 선 정보
	int nSearchOffset,
	BOOL bInnerToOuter,
	int nCam,
	int nPos
)
{
	cv::Mat pImage(height, width, CV_8UC1, pOrgImage);
	cv::Mat  pVertImage, pHoriImage;

	BOOL bFind[2] = { TRUE, TRUE };
	int nSize = 600;
	CvRect cvRectROI;

	nHoriSearchArea = nHoriSearchArea / 4 * 4;
	nVertSearchArea = nVertSearchArea / 4 * 4;

	BOOL bRet = TRUE;

	//		|												|			----------------|	|-----------------
	//		|												|							|	|
	//		|				1사분면			2사분면			|				3사분면		|	|	4 사분면
	//		|												|							|	|
	//		|----------------				-----------------							|	|

	// 가로성분 찾기	: Y 좌표
	switch (nCornerType) {
	case CORNER_LB:			// 1사분면
		cvRectROI.x = rcROI.right - nSize - 1;
		cvRectROI.y = rcROI.bottom - 1 - nHoriSearchArea > 0 ? rcROI.bottom - 1 - nHoriSearchArea : 0;
		break;
	case CORNER_RB:			// 2사분면
		//cvRectROI.x = 0;
		cvRectROI.x = rcROI.left; // 19.09.03 임시
		cvRectROI.y = rcROI.bottom - 1 - nHoriSearchArea > 0 ? rcROI.bottom - 1 - nHoriSearchArea : 0;
		break;
	case CORNER_RT:			// 3사분면
		cvRectROI.x = rcROI.left;
		cvRectROI.y = rcROI.top;
		break;
	case CORNER_LT:			// 4사분면
		cvRectROI.x = rcROI.right - nSize - 1;
		cvRectROI.y = rcROI.top;
		break;
	}

	if (cvRectROI.x < 0) cvRectROI.x = 0;
	if (cvRectROI.y < 0) cvRectROI.y = 0;

	cvRectROI.width = nSize;
	cvRectROI.height = nHoriSearchArea;

	if (cvRectROI.x + cvRectROI.width >= width) cvRectROI.x = width - nSize - 1;
	if (cvRectROI.y + cvRectROI.height >= height) cvRectROI.y = height - nHoriSearchArea - 1;

	pHoriImage = pImage(cvRectROI).clone();

	// 전처리
	preProcessImage(nCam, nPos, pHoriImage);

	cv::Mat sgray(pHoriImage.rows, pHoriImage.cols, CV_8UC1, pHoriImage.data);

	// 필터 적용
	filterProcessImage_hori(nCam, nPos, pHoriImage, &sgray, nSobelDirectionHori, nThreshold_Hori);

	// 잡음제거
	if (nNoiseSize_Hori > 0)	removeNoise(pHoriImage, nNoiseSize_Hori);

	// 가로 선을 찾자!!
	if (m_nEdgeProcess[nCam][nPos] == EDGE_HISTO_W || m_nEdgeProcess[nCam][nPos] == EDGE_HISTO_B)		bFind[0] = findHisto_Hori(nCam, pHoriImage, nSize, nHoriSearchArea, nCornerType, cvRectROI, bInnerToOuter, pLineHori);
	else																								bFind[0] = findLine_Hori(nCam, pHoriImage, nSize, nHoriSearchArea, nCornerType, cvRectROI, bInnerToOuter, pLineHori);

	pHoriImage.release();


	//		|												|			----------------|	|-----------------
	//		|												|							|	|
	//		|				1사분면			2사분면			|				3사분면		|	|	4 사분면
	//		|												|							|	|
	//		|----------------				-----------------							|	|

	nSize = 500;
	nSize = nSize / 4 * 4;
	int nbottomSize = pImage.rows - rcROI.bottom > 0 ? pImage.rows - rcROI.bottom : 100;

	// 세로성분 찾기	: Y 좌표
	switch (nCornerType) {
	case CORNER_LB:			// 1사분면
		cvRectROI.x = rcROI.left + nVertSearchArea < pImage.cols - 1 ? rcROI.left : pImage.cols - 1 - nVertSearchArea;
		cvRectROI.y = rcROI.top;
		break;
	case CORNER_RB:			// 2사분면
		cvRectROI.x = rcROI.right - nVertSearchArea > 0 ? rcROI.right - nVertSearchArea : 0;
		cvRectROI.y = rcROI.top; // 19.09.03 임시
		break;
	case CORNER_RT:			// 3사분면
		cvRectROI.x = rcROI.right - nVertSearchArea > 0 ? rcROI.right - nVertSearchArea : 0;
		cvRectROI.y = rcROI.bottom - nSize;//pImage->height - 1 - nSize;
		break;
	case CORNER_LT:			// 4사분면
		cvRectROI.x = rcROI.left + nVertSearchArea < pImage.cols - 1 ? rcROI.left : pImage.cols - 1 - nVertSearchArea;
		cvRectROI.y = pImage.rows - nbottomSize - nSize;//pImage->height - 1 - nSize;
		break;
	}

	if (cvRectROI.x < 0) cvRectROI.x = 0;
	if (cvRectROI.y < 0) cvRectROI.y = 0;

	cvRectROI.width = nVertSearchArea;
	cvRectROI.height = nSize;

	if (cvRectROI.x + cvRectROI.width > width) cvRectROI.x = width - nVertSearchArea - 1;
	if (cvRectROI.y + cvRectROI.height > height) cvRectROI.y = height - nSize - 1;

	pVertImage = pImage(cvRectROI).clone();

	// 전처리
	preProcessImage(nCam, nPos, pVertImage);

	cv::Mat sgray2(pVertImage.rows, pVertImage.cols, CV_8UC1, pVertImage.data);

	// 필터 적용
	filterProcessImage_vert(nCam, nPos, pVertImage, &sgray2, nSobelDirectionVert, nThreshold_Vert);

	// 잡음 제거
	if (nNoiseSize_Vert > 0)	removeNoise(pVertImage, nNoiseSize_Vert);

	// 세로 선을 찾자!!
	if (m_nEdgeProcess[nCam][nPos] == EDGE_HISTO_W || m_nEdgeProcess[nCam][nPos] == EDGE_HISTO_B)		bFind[1] = findHisto_Vert(nCam, pVertImage, nSize, nVertSearchArea, nCornerType, cvRectROI, bInnerToOuter, pLineVert);
	else																								bFind[1] = findLine_Vert(nCam, pVertImage, nSize, nVertSearchArea, nCornerType, cvRectROI, bInnerToOuter, pLineVert);

	pVertImage.release();
	pImage.release();

	if (bFind[0] != TRUE || bFind[1] != TRUE)
		bRet = FALSE;

	return bRet;
}

void CPatternMatching::cramersRules(double a, double b, double c, double d, double p, double q, double *x, double *y)
{
	*x = (p * d - b * q) / (a * d - b * c);
	*y = (a * q - c * p) / (a * d - b * c);
}

void CPatternMatching::setRealROI(CRect *pRectHori, CRect *pRectVert, int width, int height, int nQuadrant, CRect rcROI, int nHoriSearchArea, int nVertSearchArea, int nSearchOffset)
{
	int nSize = 600;
	CvRect cvRectROI;

	//		|												|			----------------|	|-----------------
	//		|												|							|	|
	//		|				1사분면			2사분면			|				3사분면		|	|	4 사분면
	//		|												|							|	|
	//		|----------------				-----------------							|	|

	// 가로성분 찾기	: Y 좌표
	switch (nQuadrant) {
	case 0:			// 1사분면
		cvRectROI.x = width - nSize - 1;
		cvRectROI.y = rcROI.bottom - 1 - nHoriSearchArea > 0 ? rcROI.bottom - 1 - nHoriSearchArea : 0;

		break;
	case 1:			// 2사분면
		cvRectROI.x = 500;
		cvRectROI.y = rcROI.bottom - 1 - nHoriSearchArea > 0 ? rcROI.bottom - 1 - nHoriSearchArea : 0;
		break;
	case 2:			// 3사분면
		cvRectROI.x = 0 + nSearchOffset;
		cvRectROI.y = rcROI.top;
		break;
	case 3:			// 4사분면
		cvRectROI.x = width - 1 - nSize - nSearchOffset;
		cvRectROI.y = rcROI.top;
		break;
	}

	cvRectROI.width = nSize;
	cvRectROI.height = nHoriSearchArea;

	pRectHori->left = cvRectROI.x;
	pRectHori->top = cvRectROI.y;
	pRectHori->right = cvRectROI.x + nSize;
	pRectHori->bottom = cvRectROI.y + nHoriSearchArea;

	nSize = 500;
	int nbottomSize = height - rcROI.bottom > 0 ? height - rcROI.bottom : 100;
	// 세로성분 찾기	: Y 좌표
	switch (nQuadrant) {
	case 0:			// 1사분면
		cvRectROI.x = rcROI.left + nVertSearchArea < width - 1 ? rcROI.left : width - 1 - nVertSearchArea;
		cvRectROI.y = 0;
		break;
	case 1:			// 2사분면
		cvRectROI.x = rcROI.right - nVertSearchArea > 0 ? rcROI.right - nVertSearchArea : 0;
		cvRectROI.y = 0;
		break;
	case 2:			// 3사분면
		cvRectROI.x = rcROI.right - nVertSearchArea > 0 ? rcROI.right - nVertSearchArea : 0;
		cvRectROI.y = height - nbottomSize - nSize;//pImage->height - 1 - nSize;
		break;
	case 3:			// 4사분면
		cvRectROI.x = rcROI.left + nVertSearchArea < width - 1 ? rcROI.left : width - 1 - nVertSearchArea;
		cvRectROI.y = height - nbottomSize - nSize;//pImage->height - 1 - nSize;
		break;
	}

	cvRectROI.width = nVertSearchArea;
	cvRectROI.height = nSize;

	pRectVert->left = cvRectROI.x;
	pRectVert->top = cvRectROI.y;
	pRectVert->right = cvRectROI.x + nVertSearchArea;
	pRectVert->bottom = cvRectROI.y + nSize;
}

void CPatternMatching::setRealROIVert(CRect *pRectHori, CRect *pRectVert, int width, int height, int nCornerType, CRect rcROI, int nHoriSearchArea, int nVertSearchArea, int nSearchOffset)
{
	int nSize = 600;
	CvRect cvRectROI;

	//		|												|			----------------|	|-----------------
	//		|												|							|	|
	//		|				1사분면			2사분면			|				3사분면		|	|	4 사분면
	//		|												|							|	|
	//		|----------------				-----------------							|	|


	// 가로성분 찾기	: Y 좌표
	switch (nCornerType) {
	case CORNER_LB:			// 1사분면
		cvRectROI.x = rcROI.right - nSize - 1;
		cvRectROI.y = rcROI.bottom - 1 - nHoriSearchArea > 0 ? rcROI.bottom - 1 - nHoriSearchArea : 0;
		break;
	case CORNER_RB:			// 2사분면
		//cvRectROI.x = 0;
		cvRectROI.x = rcROI.left; // 19.09.03 임시
		cvRectROI.y = rcROI.bottom - 1 - nHoriSearchArea > 0 ? rcROI.bottom - 1 - nHoriSearchArea : 0;
		break;
	case CORNER_RT:			// 3사분면
		cvRectROI.x = rcROI.left;
		cvRectROI.y = rcROI.top;
		break;
	case CORNER_LT:			// 4사분면
		cvRectROI.x = rcROI.right - nSize;
		cvRectROI.y = rcROI.top;
		break;
	}

	cvRectROI.width = nSize;
	cvRectROI.height = nHoriSearchArea;

	pRectHori->left = cvRectROI.x;
	pRectHori->top = cvRectROI.y;
	pRectHori->right = cvRectROI.x + nSize;
	pRectHori->bottom = cvRectROI.y + nHoriSearchArea;

	nSize = 500;
	int nbottomSize = height - rcROI.bottom > 0 ? height - rcROI.bottom : 100;
	// 세로성분 찾기	: Y 좌표
	switch (nCornerType) {
	case CORNER_LB:			// 1사분면
		cvRectROI.x = rcROI.left + nVertSearchArea < width - 1 ? rcROI.left : width - 1 - nVertSearchArea;
		cvRectROI.y = rcROI.top;
		break;
	case CORNER_RB:			// 2사분면
		cvRectROI.x = rcROI.right - nVertSearchArea > 0 ? rcROI.right - nVertSearchArea : 0;
		cvRectROI.y = rcROI.top; // 19.09.03 임시
		break;
	case CORNER_RT:			// 3사분면
		cvRectROI.x = rcROI.right - nVertSearchArea > 0 ? rcROI.right - nVertSearchArea : 0;
		cvRectROI.y = rcROI.bottom - nSize;//pImage->height - 1 - nSize;
		break;
	case CORNER_LT:			// 4사분면
		cvRectROI.x = rcROI.left + nVertSearchArea < width - 1 ? rcROI.left : width - 1 - nVertSearchArea;
		cvRectROI.y = height - nbottomSize - nSize;
		break;
	}

	cvRectROI.width = nVertSearchArea;
	cvRectROI.height = nSize;

	pRectVert->left = cvRectROI.x;
	pRectVert->top = cvRectROI.y;
	pRectVert->right = cvRectROI.x + nVertSearchArea;
	pRectVert->bottom = cvRectROI.y + nSize;
}


void CPatternMatching::setRealROIVert2(CRect *pRectHori, CRect *pRectVert, int width, int height, int nCornerType, CRect rcROI, int nHoriProjectionArea, int nVertProjectionArea, int nHoriSearchArea, int nVertSearchArea, int nSearchOffset)
{
	//int nSize = 600;
	int nSize = nHoriProjectionArea;
	CvRect cvRectROI;

	//		|												|			----------------|	|-----------------
	//		|												|							|	|
	//		|				1사분면			2사분면			|				3사분면		|	|	4 사분면
	//		|												|							|	|
	//		|----------------				-----------------							|	|


	// 가로성분 찾기	: Y 좌표
	switch (nCornerType) {
	case CORNER_LB:			// 1사분면
		cvRectROI.x = rcROI.right - nSize - 1;
		cvRectROI.y = rcROI.bottom - 1 - nHoriSearchArea > 0 ? rcROI.bottom - 1 - nHoriSearchArea : 0;
		break;
	case CORNER_RB:			// 2사분면
		//cvRectROI.x = 0;
		cvRectROI.x = rcROI.left; // 19.09.03 임시
		cvRectROI.y = rcROI.bottom - 1 - nHoriSearchArea > 0 ? rcROI.bottom - 1 - nHoriSearchArea : 0;
		break;
	case CORNER_RT:			// 3사분면
		cvRectROI.x = rcROI.left;
		cvRectROI.y = rcROI.top;
		break;
	case CORNER_LT:			// 4사분면
		cvRectROI.x = rcROI.right - nSize;
		cvRectROI.y = rcROI.top;
		break;
	}

	cvRectROI.width = nSize;
	cvRectROI.height = nHoriSearchArea;

	pRectHori->left = cvRectROI.x;
	pRectHori->top = cvRectROI.y;
	pRectHori->right = cvRectROI.x + nSize;
	pRectHori->bottom = cvRectROI.y + nHoriSearchArea;

	//nSize = 500;
	nSize = nVertProjectionArea;
	int nbottomSize = height - rcROI.bottom > 0 ? height - rcROI.bottom : 100;
	// 세로성분 찾기	: Y 좌표
	switch (nCornerType) {
	case CORNER_LB:			// 1사분면
		cvRectROI.x = rcROI.left + nVertSearchArea < width - 1 ? rcROI.left : width - 1 - nVertSearchArea;
		cvRectROI.y = rcROI.top;
		break;
	case CORNER_RB:			// 2사분면
		cvRectROI.x = rcROI.right - nVertSearchArea > 0 ? rcROI.right - nVertSearchArea : 0;
		cvRectROI.y = rcROI.top; // 19.09.03 임시
		break;
	case CORNER_RT:			// 3사분면
		cvRectROI.x = rcROI.right - nVertSearchArea > 0 ? rcROI.right - nVertSearchArea : 0;
		cvRectROI.y = rcROI.bottom - nSize;//pImage->height - 1 - nSize;
		break;
	case CORNER_LT:			// 4사분면
		cvRectROI.x = rcROI.left + nVertSearchArea < width - 1 ? rcROI.left : width - 1 - nVertSearchArea;
		cvRectROI.y = height - nbottomSize - nSize;
		break;
	}

	cvRectROI.width = nVertSearchArea;
	cvRectROI.height = nSize;

	pRectVert->left = cvRectROI.x;
	pRectVert->top = cvRectROI.y;
	pRectVert->right = cvRectROI.x + nVertSearchArea;
	pRectVert->bottom = cvRectROI.y + nSize;
}

cv::Rect CPatternMatching::findChipContour(BYTE *pImage, int m_nWidth, int m_nHeight)
{
	cv::Mat src(m_nHeight, m_nWidth, CV_8UC1, pImage);
	cv::Mat dst, otsu;
	cv::GaussianBlur(src, dst, cv::Size(3, 3), 0, 0, cv::BORDER_DEFAULT);
	cv::Laplacian(dst, dst, CV_8U, 3);
	//cv::threshold( dst, otsu, 0, 255, cv::THRESH_OTSU );
	cv::threshold(dst, otsu, 15, 255, cv::THRESH_BINARY);

	removeNoise(otsu, 15);
	//cv::erode( dst, dst, 
	cv::dilate(otsu, otsu, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3)), cv::Point(-1, -1), 3);
	cv::erode(otsu, otsu, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3)), cv::Point(-1, -1), 3);

	std::vector < std::vector<cv::Point> > contours;
	cv::findContours(otsu, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

	cv::SimpleBlobDetector::Params params;
	params.minDistBetweenBlobs = 50.0f;
	params.filterByInertia = false;
	params.filterByConvexity = false;
	params.filterByColor = false;
	params.filterByCircularity = true;
	params.filterByArea = true;
	params.minArea = float(100);
	params.minCircularity = float(0);
	params.maxCircularity = float(1.0f);
	params.minThreshold = (float)0;

	double dMaxArea = 0;
	int nMaxIndex = 0;

	for (size_t contourIdx = 0; contourIdx < contours.size(); contourIdx++)
	{
		cv::Moments moms = cv::moments(cv::Mat(contours[contourIdx]));

		if (moms.m00 > dMaxArea)
		{
			dMaxArea = moms.m00;
			nMaxIndex = int(contourIdx);
		}
	}

	cv::Rect rect = cv::boundingRect(contours[nMaxIndex]);

	dst.release();
	otsu.release();
	contours.clear();

	return rect;
}
BOOL CPatternMatching::findActiveEdgeByRansac(BYTE *pImage, int width, int height, int nDirection, int nSearchRange, int nRemoveNoiseSize, int nThreshold, cv::Rect rectROI, sLine *pLine)
{

	cv::Mat pSrcImage(height, width, CV_8UC1, pImage);

	CvRect rcROI = rectROI;


	CString str;
	str.Format("C:\\img%d.jpg", nDirection);

	switch (nDirection) {
	case 0:			// LEFT
	{
		rcROI.x = rectROI.x - nSearchRange / 2;
		rcROI.y = rectROI.y + rectROI.height / 5;
		rcROI.width = nSearchRange;
		rcROI.height = rectROI.height - rectROI.height / 5;

		/*
		rcROI.x = rectROI.x - nSearchRange / 3 > 0 ? rectROI.x - nSearchRange / 3: 0;
		rcROI.y = rectROI.y + rectROI.height / 5;
		rcROI.width = nSearchRange;
		rcROI.height = rectROI.height - rectROI.height / 5;
		*/
	}
	break;
	case 1:
	{
		rcROI.x = rectROI.x + rectROI.width - nSearchRange / 2;
		rcROI.y = rectROI.y + rectROI.height / 5;
		rcROI.width = nSearchRange;
		rcROI.height = rectROI.height - rectROI.height / 5;

		/*
		rcROI.x = rectROI.x + rectROI.width + nSearchRange < width ? rectROI.x + rectROI.width - nSearchRange * 2 / 3: rectROI.x + width - 1 - nSearchRange;
		rcROI.y = rectROI.y + rectROI.height / 5;
		rcROI.width = nSearchRange / 3;
		rcROI.height = rectROI.height - rectROI.height / 5;
		*/
	}
	break;
	case 2:
	{
	}
	break;
	case 3:
	{
		rcROI.x = rectROI.x;
		rcROI.y = rectROI.y + rectROI.height;
		rcROI.width = rectROI.width;
		rcROI.height = nSearchRange;
	}
	break;
	case 4:
	{
	}
	break;
	case 5:
	{
		rcROI.x = rectROI.x;
		rcROI.y = rectROI.y - nSearchRange;
		rcROI.width = rectROI.width;
		rcROI.height = nSearchRange * 3;
	}
	break;
	}

	if (rcROI.x < 0) rcROI.x = 0;
	if (rcROI.y < 0) rcROI.y = 0;
	if (rcROI.width < 0) rcROI.width = 0;
	if (rcROI.height < 0) rcROI.height = 0;
	if (rcROI.width > width) rcROI.width = width - 1;
	if (rcROI.height > height) rcROI.height = height - 1;
	if (rcROI.x + rcROI.width > width) rcROI.x = width - rcROI.width - 1;
	if (rcROI.y + rcROI.height > height) rcROI.y = height - rcROI.height - 1;

	//rcROI.x = (rectROI.x/4)*4;
	//	rcROI.width =  (rectROI.width/4)*4;

	cv::Mat pCropImage = pSrcImage(rcROI).clone();

	GaussianBlur(pCropImage, pCropImage, cv::Size(3, 3), 1.);

	//cv::Mat ddd( pCropImage->height, pCropImage->width, CV_8UC1, pCropImage->imageData);
	//cv::imwrite(std::string(str), ddd);


	switch (nDirection) {
	case 0:		sobelDirection(3, SOBEL_RIGHT, pCropImage, pCropImage);	break;
	case 1:		sobelDirection(3, SOBEL_LEFT, pCropImage, pCropImage);	break;
	case 2:		sobelDirection(3, SOBEL_UP, pCropImage, pCropImage);	break;
	case 3:		sobelDirection(3, SOBEL_DN, pCropImage, pCropImage);	break;
	case 4:		sobelDirection(3, SOBEL_LEFT, pCropImage, pCropImage);	break;
	case 5:		sobelDirection(3, SOBEL_UP, pCropImage, pCropImage);	break;
	}

	cv::threshold(pCropImage, pCropImage, nThreshold, 255, CV_THRESH_BINARY);
	removeNoise(pCropImage, nRemoveNoiseSize);

	sPoint *pContour = NULL;
	int nValidCount = 0;

	switch (nDirection) {
	case 0:			// Left
	{
		pContour = new sPoint[rcROI.height];
		for (int y = 1; y < rcROI.height - 1; y++)
		{
			for (int x = nSearchRange - 2; x > 1; x--)
			{
				if (x >= pCropImage.cols) continue;
				if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
				{
					pContour[nValidCount].x = x + rcROI.x;
					pContour[nValidCount].y = y + rcROI.y;
					nValidCount++;
					break;
				}
			}
		}
	}
	break;
	case 1:
	{
		pContour = new sPoint[rcROI.height];
		for (int y = 1; y < rcROI.height - 1; y++)
		{
			//	for(int x = 0; x < nSearchRange; x++)
			for (int x = nSearchRange - 1; x > 0; x--)
			{
				if (x >= pCropImage.cols) continue;
				if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
				{
					pContour[nValidCount].x = x + rcROI.x;
					pContour[nValidCount].y = y + rcROI.y;
					nValidCount++;
					break;
				}
			}
		}
	}
	break;
	case 2:
	{
		pContour = new sPoint[rcROI.width];
		for (int x = 0; x < rcROI.width; x++)
		{
			for (int y = 1; y < rcROI.height - 1; y++)
			{
				if (y >= pCropImage.rows) continue;
				if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
				{
					pContour[nValidCount].x = x + rcROI.x;
					pContour[nValidCount].y = y + rcROI.y;
					nValidCount++;
					break;
				}
			}
		}
	}
	break;
	case 3:
	{
		pContour = new sPoint[rcROI.width];
		for (int x = 0; x < rcROI.width; x++)
		{
			if (x > rcROI.width / 2 - 30 && x < rcROI.width / 2 + 30) continue;
			for (int y = nSearchRange - 1; y > 0; y--)
			{
				if (y >= pCropImage.rows) continue;
				if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
				{
					pContour[nValidCount].x = x + rcROI.x;
					pContour[nValidCount].y = y + rcROI.y;
					nValidCount++;
					break;
				}
			}
		}
	}
	break;
	case 4:
	{
		pContour = new sPoint[rcROI.height];
		for (int y = 1; y < rcROI.height - 1; y++)
		{
			for (int x = 0; x < nSearchRange; x++)
			{
				if (x >= pCropImage.cols) continue;
				if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
				{
					pContour[nValidCount].x = x + rcROI.x;
					pContour[nValidCount].y = y + rcROI.y;
					nValidCount++;
					break;
				}
			}
		}
	}
	break;
	case 5:  //Top 몰딩
	{
		pContour = new sPoint[rcROI.width];
		for (int x = 0; x < rcROI.width; x++)
		{
			for (int y = 1; y < rcROI.height - 1; y++)
			{
				if (y >= pCropImage.rows) continue;
				if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
				{
					pContour[nValidCount].x = x + rcROI.x;
					pContour[nValidCount].y = y + rcROI.y;
					nValidCount++;
					break;
				}
			}
		}
	}
	break;
	}

	double cost = ransac_line_fitting(pContour, nValidCount, *pLine, 2);
	delete pContour;

	pCropImage.release();
	pSrcImage.release();


	return TRUE;
}
BOOL CPatternMatching::findSusEdgeByRansac(BYTE *pImage, int width, int height, int nDirection, int nSearchRange, int nRemoveNoiseSize, int nThreshold, cv::Rect rectROI, sLine *pLine)
{
	cv::Mat pSrcImage(height, width, CV_8UC1, pImage);

	CvRect rcROI;


	CString str;
	str.Format("img%d", nDirection);

	int nSize = 400;
	switch (nDirection) {
	case 0:			// LEFT
	{
		rcROI.x = rectROI.x - nSearchRange;
		rcROI.y = rectROI.y + rectROI.height - nSize - 100;
		rcROI.width = nSearchRange * 2;
		rcROI.height = nSize;
	}
	break;
	case 1:			// RIGHT
	{
		rcROI.x = rectROI.x + rectROI.width - nSearchRange;
		rcROI.y = rectROI.y + rectROI.height - nSize - 100;
		rcROI.width = nSearchRange * 2;
		rcROI.height = nSize;
	}
	break;
	case 2:			// UP
	{

	}
	break;
	case 3:			// DN
	{
		rcROI.x = rectROI.x;
		rcROI.y = rectROI.y + rectROI.height - nSearchRange;
		rcROI.width = rectROI.width;
		rcROI.height = nSearchRange * 2;
	}
	break;
	}

	if (rcROI.x < 0) rcROI.x = 0;
	if (rcROI.y < 0) rcROI.y = 0;
	if (rcROI.width < 0) rcROI.width = 0;
	if (rcROI.height < 0) rcROI.height = 0;
	if (rcROI.width > width) rcROI.width = width - 1;
	if (rcROI.height > height) rcROI.height = height - 1;
	if (rcROI.x + rcROI.width > width) rcROI.x = width - rcROI.width - 1;
	if (rcROI.y + rcROI.height > height) rcROI.y = height - rcROI.height - 1;

	cv::Mat pCropImage = pSrcImage(rcROI).clone();

	GaussianBlur(pCropImage, pCropImage, cv::Size(3, 3), 1.);

	cv::Mat ddd(pCropImage.rows, pCropImage.cols, CV_8UC1, pCropImage.data);

	switch (nDirection) {
	case 0:		sobelDirection(3, SOBEL_RIGHT, pCropImage, pCropImage);	break;
	case 1:		sobelDirection(3, SOBEL_LEFT, pCropImage, pCropImage);	break;
	case 2:		sobelDirection(3, SOBEL_DN, pCropImage, pCropImage);	break;
	case 3:		sobelDirection(3, SOBEL_UP, pCropImage, pCropImage);	break;
	}

	cv::threshold(pCropImage, pCropImage, nThreshold, 255, CV_THRESH_BINARY);
	removeNoise(pCropImage, nRemoveNoiseSize);

	//cv::imshow(std::string(str), pCropImage);
	//cvShowImage(str, pCropImage);

	sPoint *pContour = NULL;
	int nValidCount = 0;

	switch (nDirection) {
	case 0:			// Left
	{
		pContour = new sPoint[rcROI.height];
		for (int y = 1; y < rcROI.height - 1; y++)
		{
			for (int x = rcROI.width - 1; x > 1; x--)
			{
				if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
				{
					pContour[nValidCount].x = x + rcROI.x;
					pContour[nValidCount].y = y + rcROI.y;
					nValidCount++;
					break;
				}
			}
		}
	}
	break;
	case 1:
	{
		pContour = new sPoint[rcROI.height];
		for (int y = 1; y < rcROI.height - 1; y++)
		{
			for (int x = 1; x < rcROI.width - 1; x++)
			{
				if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
				{
					pContour[nValidCount].x = x + rcROI.x;
					pContour[nValidCount].y = y + rcROI.y;
					nValidCount++;
					break;
				}
			}
		}
	}
	break;
	case 3:
	{
		pContour = new sPoint[rcROI.width];
		for (int x = 0; x < rcROI.width; x++)
		{
			for (int y = 0; y < nSearchRange; y++)
			{
				if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
				{
					pContour[nValidCount].x = x + rcROI.x;
					pContour[nValidCount].y = y + rcROI.y;
					nValidCount++;
					break;
				}
			}
		}
	}
	break;
	}

	double cost = ransac_line_fitting(pContour, nValidCount, *pLine, 2);
	delete pContour;
	pCropImage.release();
	pSrcImage.release();

	return TRUE;
}
BOOL CPatternMatching::findMoldingEdgeByRansac(BYTE *pImage, int width, int height, int nDirection, int nSearchRange, int nRemoveNoiseSize, int nThreshold, cv::Rect rectROI, sLine *pLine)
{
	cv::Mat pSrcImage(height, width, CV_8UC1, pImage);

	CString str;
	str.Format("img%d", nDirection);

	if (rectROI.x < 0 || rectROI.y < 0) return FALSE;
	if (rectROI.x + rectROI.width > width || rectROI.y + rectROI.height > height) return FALSE;
	
	cv::Mat pCropImage = pSrcImage(rectROI).clone();

	GaussianBlur(pCropImage, pCropImage, cv::Size(3, 3), 1.);

	switch (nDirection) {
	case 0:		sobelDirection(3, SOBEL_LEFT, pCropImage, pCropImage);	break;
	case 1:		sobelDirection(3, SOBEL_RIGHT, pCropImage, pCropImage);	break;
	case 2:		sobelDirection(3, SOBEL_UP, pCropImage, pCropImage);	break;
	case 3:		sobelDirection(3, SOBEL_DN, pCropImage, pCropImage);	break;
	case 9:		sobelDirection(3, SOBEL_RIGHT, pCropImage, pCropImage);	break;  //중간 빠진 Right
	}

	cv::threshold(pCropImage, pCropImage, nThreshold, 255, CV_THRESH_BINARY);
	removeNoise(pCropImage, nRemoveNoiseSize);

	//	cvShowImage(str, pCropImage);

	sPoint *pContour = NULL;
	int nValidCount = 0;

	switch (nDirection) {
	case 0:			// Left
	{
		pContour = new sPoint[rectROI.height];
		for (int y = 1; y < rectROI.height - 1; y++)
		{
			for (int x = 1; x < rectROI.width - 1; x++)
			{
				if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
				{
					pContour[nValidCount].x = x + rectROI.x;
					pContour[nValidCount].y = y + rectROI.y;
					nValidCount++;
					break;
				}
			}
		}
	}
	break;
	case 1:			// Right
	{
		pContour = new sPoint[rectROI.height];
		for (int y = 1; y < rectROI.height - 1; y++)
		{
			for (int x = rectROI.width - 1; x > 1; x--)
			{
				if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
				{
					pContour[nValidCount].x = x + rectROI.x;
					pContour[nValidCount].y = y + rectROI.y;
					nValidCount++;
					break;
				}
			}
		}
	}
	break;
	case 2:  //Top
	{
		pContour = new sPoint[rectROI.width];
		for (int x = 0; x < rectROI.width; x++)
		{
			for (int y = 1; y < rectROI.height - 1; y++)
			{
				if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
				{
					pContour[nValidCount].x = x + rectROI.x;
					pContour[nValidCount].y = y + rectROI.y;
					nValidCount++;
					break;
				}
			}
		}
	}
	break;
	case 3:
	{
		pContour = new sPoint[rectROI.width];
		for (int x = 0; x < rectROI.width; x++)
		{
			for (int y = rectROI.height - 1; y > 1; y--)
			{
				if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
				{
					pContour[nValidCount].x = x + rectROI.x;
					pContour[nValidCount].y = y + rectROI.y;
					nValidCount++;
					break;
				}
			}
		}
	}
	break;
	case 9:			//  중간 빠진 Right
	{
		pContour = new sPoint[rectROI.height];
		for (int y = 1; y < rectROI.height - 1; y++)
		{
			if (y > rectROI.height / 2 - 150 && y < rectROI.height / 2 + 150) continue;

			for (int x = rectROI.width - 1; x > 1; x--)
			{
				if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
				{
					pContour[nValidCount].x = x + rectROI.x;
					pContour[nValidCount].y = y + rectROI.y;
					nValidCount++;
					break;
				}
			}
		}
	}
	break;
	}

	double cost = ransac_line_fitting(pContour, nValidCount, *pLine, 2);
	delete pContour;
	pCropImage.release();
	pSrcImage.release();

	return TRUE;
}
BOOL CPatternMatching::findEdgeByRansac(BYTE *pImage, int width, int height, int nSide, int nSearchRange, int nSearchDir, int nSearchPosOffset, int nRemoveNoiseSize, int nThreshold, cv::Rect rcROI, sLine *pLine)
{
	cv::Mat pSrcImage(height, width, CV_8UC1, pImage);

	CvRect rectROI = { 0, };
	switch (nSide) {
	case 0:	// Left
		rectROI.x = rcROI.x + nSearchPosOffset - nSearchRange / 2;
		rectROI.y = rcROI.y;
		rectROI.width = nSearchRange;
		rectROI.height = rcROI.height;
		break;
	case 1:	// Right
		rectROI.x = rcROI.x + rcROI.width + nSearchPosOffset - nSearchRange / 2;
		rectROI.y = rcROI.y;
		rectROI.width = nSearchRange;
		rectROI.height = rcROI.height;
		break;
	case 2:	// Bottom
		rectROI.x = rcROI.x;
		rectROI.y = rcROI.y + rcROI.height + nSearchPosOffset - nSearchRange / 2;
		rectROI.width = rcROI.width;
		rectROI.height = nSearchRange;
		break;
	case 3:	// Top
		rectROI.x = rcROI.x;
		rectROI.y = rcROI.y - nSearchPosOffset - nSearchRange / 2;
		rectROI.width = rcROI.width;
		rectROI.height = nSearchRange;
		break;
	}

	if (rectROI.x < 0)	rectROI.x = 0;
	if (rectROI.y < 0)	rectROI.y = 0;
	if (rcROI.width > width) rcROI.width = width - 1;
	if (rcROI.height > height) rcROI.height = height - 1;
	if (rectROI.x + rectROI.width >= width)	rectROI.x = width - rectROI.width - 1;
	if (rectROI.y + rectROI.height >= height)	rectROI.y = height - rectROI.height - 1;

	if (rectROI.x < 0)	return FALSE;
	if (rectROI.y < 0)	return FALSE;

	cv::Mat pCropImage = pSrcImage(rectROI).clone();
	GaussianBlur(pCropImage, pCropImage, cv::Size(3, 3), 1.);


	switch (nSide) {
	case 0:		// Left
		if (nSearchDir)		// In / Out	- Bright / Dark
			sobelDirection(3, SOBEL_RIGHT, pCropImage, pCropImage);
		else					// In / Out	- Dark / Bright
			sobelDirection(3, SOBEL_LEFT, pCropImage, pCropImage);
		break;
	case 1:		// Right
		if (nSearchDir)		// In / Out	- Bright / Dark
			sobelDirection(3, SOBEL_LEFT, pCropImage, pCropImage);
		else					// In / Out	- Dark / Bright
			sobelDirection(3, SOBEL_RIGHT, pCropImage, pCropImage);
		break;
	case 2:		// Bottom
		if (nSearchDir)		// In / Out	- Bright / Dark
			sobelDirection(3, SOBEL_UP, pCropImage, pCropImage);
		else
			sobelDirection(3, SOBEL_DN, pCropImage, pCropImage);
		break;
	case 3:		// Top
		if (nSearchDir)		// In / Out	- Bright / Dark
			sobelDirection(3, SOBEL_DN, pCropImage, pCropImage);
		else
			sobelDirection(3, SOBEL_UP, pCropImage, pCropImage);
		break;
	}

	cv::threshold(pCropImage, pCropImage, nThreshold, 255, CV_THRESH_BINARY);
	removeNoise(pCropImage, nRemoveNoiseSize);

	sPoint *pContour = NULL;
	int nValidCount = 0;

	switch (nSide) {
	case 0:					// Left
	{
		pContour = new sPoint[rectROI.height];

		if (nSearchDir == 0)		// Out -> In
		{
			for (int y = 0; y < rectROI.height - 1; y++)
			{
				for (int x = 0; x < rectROI.width; x++)
				{
					if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
					{
						pContour[nValidCount].x = x + rectROI.x;
						pContour[nValidCount].y = y + rectROI.y;
						nValidCount++;
						break;
					}
				}
			}
		}
		else					// In -> Out
		{
			for (int y = 0; y < rectROI.height - 1; y++)
			{
				for (int x = rectROI.width - 1; x > 0; x--)
				{
					if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
					{
						pContour[nValidCount].x = x + rectROI.x;
						pContour[nValidCount].y = y + rectROI.y;
						nValidCount++;
						break;
					}
				}
			}
		}
	}
	break;
	case 1:					// Right
	{
		pContour = new sPoint[rectROI.height];

		if (nSearchDir == 0)		// Out -> In
		{
			for (int y = 0; y < rectROI.height - 1; y++)
			{
				for (int x = rectROI.width - 1; x > 0; x--)
				{
					if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
					{
						pContour[nValidCount].x = x + rectROI.x;
						pContour[nValidCount].y = y + rectROI.y;
						nValidCount++;
						break;
					}
				}
			}
		}
		else					// In -> Out
		{
			for (int y = 0; y < rectROI.height - 1; y++)
			{
				for (int x = 0; x < rectROI.width; x++)
				{
					if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
					{
						pContour[nValidCount].x = x + rectROI.x;
						pContour[nValidCount].y = y + rectROI.y;
						nValidCount++;
						break;
					}
				}
			}
		}
	}
	break;
	case 2:
	{
		pContour = new sPoint[rectROI.width];

		if (nSearchDir == 0)		// Out -> In
		{
			for (int x = 0; x < rectROI.width; x++)
			{
				for (int y = rectROI.height - 1; y > 0; y--)
				{
					if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
					{
						pContour[nValidCount].x = x + rectROI.x;
						pContour[nValidCount].y = y + rectROI.y;
						nValidCount++;
						break;
					}
				}
			}
		}
		else					// In -> Out
		{
			for (int x = 0; x < rectROI.width; x++)
			{
				for (int y = 0; y < rectROI.height; y++)
				{
					if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
					{
						pContour[nValidCount].x = x + rectROI.x;
						pContour[nValidCount].y = y + rectROI.y;
						nValidCount++;
						break;
					}
				}
			}
		}
	}
	break;
	case 3:
	{
		pContour = new sPoint[rectROI.width];

		if (nSearchDir == 0)		// Out -> In
		{
			for (int x = 0; x < rectROI.width; x++)
			{
				for (int y = 0; y < rectROI.height - 1; y++)
				{
					if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
					{
						pContour[nValidCount].x = x + rectROI.x;
						pContour[nValidCount].y = y + rectROI.y;
						nValidCount++;
						break;
					}
				}
			}
		}
		else					// In -> Out
		{
			for (int x = 0; x < rectROI.width; x++)
			{
				for (int y = rectROI.height - 1; y > 0; y--)
				{
					if ((UCHAR)pCropImage.data[y * pCropImage.cols + x] == 255)
					{
						pContour[nValidCount].x = x + rectROI.x;
						pContour[nValidCount].y = y + rectROI.y;
						nValidCount++;
						break;
					}
				}
			}
		}
	}
	break;
	}

	double cost = ransac_line_fitting(pContour, nValidCount, *pLine, 2);
	delete pContour;
	pCropImage.release();
	pSrcImage.release();


	switch (nSide) {
	case 0:
	case 1:
		if (cost < rectROI.width / 3)	return FALSE;
		else							return TRUE;
	case 2:
		if (cost < rectROI.height / 3)	return FALSE;
		else							return TRUE;
	}

	return TRUE;
}


void CPatternMatching::decideSobelDirection(int nCornerType, int nEdgePolarity, int nEdgeDirection, int *pSobel_Hori, int *pSobel_Vert)
{
	////////// 가로선에 대한 필터 결정
	if (nEdgePolarity == EDGE_LIGHT_TO_DARK)
	{
		if (nCornerType == CORNER_LT || nCornerType == CORNER_RT)
		{
			if (nEdgeDirection == EDGE_INNER_TO_OUTER)		*pSobel_Hori = SOBEL_DN;
			else											*pSobel_Hori = SOBEL_UP;
		}
		else
		{
			if (nEdgeDirection == EDGE_INNER_TO_OUTER)		*pSobel_Hori = SOBEL_UP;
			else											*pSobel_Hori = SOBEL_DN;
		}
	}
	else
	{
		if (nCornerType == CORNER_LT || nCornerType == CORNER_RT)
		{
			if (nEdgeDirection == EDGE_INNER_TO_OUTER)		*pSobel_Hori = SOBEL_UP;
			else											*pSobel_Hori = SOBEL_DN;
		}
		else
		{
			if (nEdgeDirection == EDGE_INNER_TO_OUTER)		*pSobel_Hori = SOBEL_DN;
			else											*pSobel_Hori = SOBEL_UP;
		}
	}

	////////// 세로선에 대한 필터 결정
	if (nEdgePolarity == EDGE_LIGHT_TO_DARK)
	{
		if (nCornerType == CORNER_RT || nCornerType == CORNER_RB)
		{
			if (nEdgeDirection == EDGE_INNER_TO_OUTER)		*pSobel_Vert = SOBEL_LEFT;
			else											*pSobel_Vert = SOBEL_RIGHT;
		}
		else
		{
			if (nEdgeDirection == EDGE_INNER_TO_OUTER)		*pSobel_Vert = SOBEL_RIGHT;
			else											*pSobel_Vert = SOBEL_LEFT;
		}
	}
	else
	{
		if (nCornerType == CORNER_RT || nCornerType == CORNER_RB)
		{
			if (nEdgeDirection == EDGE_INNER_TO_OUTER)		*pSobel_Vert = SOBEL_RIGHT;
			else											*pSobel_Vert = SOBEL_LEFT;
		}
		else
		{
			if (nEdgeDirection == EDGE_INNER_TO_OUTER)		*pSobel_Vert = SOBEL_LEFT;
			else											*pSobel_Vert = SOBEL_RIGHT;
		}
	}
}

void CPatternMatching::preProcessImage(int nCam, int nPos, cv::Mat& pImg)
{
	switch (m_nPreProcess[nCam][nPos]) {
	case PRE_NONE:	break;
	case PRE_GAUSSIAN_3:	GaussianBlur(pImg, pImg, cv::Size(3, 3), 1.); break;
	case PRE_MEDIAN_3:	medianBlur(pImg, pImg, 3); break;
	case PRE_MEDIAN_5:	medianBlur(pImg, pImg, 5); break;
	case PRE_MEDIAN_7:	medianBlur(pImg, pImg, 7); 	break;
	case PRE_MEDIAN_11: medianBlur(pImg, pImg, 11);		break;
	case PRE_CLOSE_1:	erode(pImg, pImg, NULL, cv::Point(-1, -1), 1);		dilate(pImg, pImg, NULL, cv::Point(-1, -1), 1);	break;
	case PRE_CLOSE_3:	erode(pImg, pImg, NULL, cv::Point(-1, -1), 3);		dilate(pImg, pImg, NULL, cv::Point(-1, -1), 3);	break;
	case PRE_CLOSE_5:	erode(pImg, pImg, NULL, cv::Point(-1, -1), 3);		dilate(pImg, pImg, NULL, cv::Point(-1, -1), 5);	break;
	}
}


BOOL CPatternMatching::findLine_Vert(int nCam, cv::Mat& pImg, int nSize, int nSearchArea, int nCornerType, cv::Rect cvRectROI, BOOL bInnerToOuter, sLine *pLine)
{
	BOOL bFind = TRUE;
	sPoint *pContour = new sPoint[nSize];
	int nValidCount = 0;

	int *pHisto = new int[nSearchArea];
	memset(pHisto, 0, sizeof(int) * nSearchArea);

	BOOL bFindX = FALSE;
	int nThresh = nSize / 3;
	int nCutX = 0;
	int nStartYpt = 0;

	////////////////////////////////////
	// 세로선을 찾자
	if (nCornerType == CORNER_LB || nCornerType == CORNER_LT)
	{
		if (bInnerToOuter)
		{
			for (int x = nSearchArea - 1; x > 0; x--)
			{
				for (int y = 0; y < nSize; y++)
				{
					if ((UCHAR)pImg.data[y * pImg.cols + x] == 255)
					{
						pHisto[x]++;

						if (pHisto[x] > nThresh)
						{
							bFindX = TRUE;
							nCutX = x;
							break;
						}
					}
				}

				if (bFindX) break;
			}
		}
		else
		{
			for (int x = 0; x < nSearchArea; x++)
			{
				for (int y = 0; y < nSize; y++)
				{
					if ((UCHAR)pImg.data[y * pImg.cols + x] == 255)
					{
						pHisto[x]++;

						if (pHisto[x] > nThresh)
						{
							bFindX = TRUE;
							nCutX = x;
							break;
						}
					}
				}

				if (bFindX) break;
			}
		}
	}
	else
	{
		if (bInnerToOuter)
		{
			for (int x = 0; x < nSearchArea; x++)
			{
				for (int y = 0; y < nSize; y++)
				{
					if ((UCHAR)pImg.data[y * pImg.cols + x] == 255)
					{
						pHisto[x]++;

						if (pHisto[x] > nThresh)
						{
							bFindX = TRUE;
							nCutX = x;
							break;
						}
					}
				}

				if (bFindX) break;
			}
		}
		else
		{
			for (int x = nSearchArea - 1; x > 0; x--)
			{
				for (int y = 0; y < nSize; y++)
				{
					if ((UCHAR)pImg.data[y * pImg.cols + x] == 255)
					{
						pHisto[x]++;

						if (pHisto[x] > nThresh)
						{
							bFindX = TRUE;
							nCutX = x;
							break;
						}
					}
				}

				if (bFindX) break;
			}
		}
	}

	if (bFindX)
	{
		int nRangeX = 10;

		if (nCornerType == CORNER_LB || nCornerType == CORNER_LT)
		{
			if (bInnerToOuter)
			{
				int nStartX = nCutX + nRangeX < nSearchArea - 1 ? nCutX + nRangeX : nSearchArea - 1;
				int nEndX = nCutX - nRangeX > 0 ? nCutX - nRangeX : 0;

				for (int y = 0; y < nSize; y++)
				{
					for (int x = nStartX; x > nEndX; x--)
					{
						if ((UCHAR)pImg.data[y * pImg.cols + x] == 255)
						{
							pContour[nValidCount].x = x + cvRectROI.x;
							pContour[nValidCount].y = y + cvRectROI.y;
							nValidCount++;
							break;
						}
					}
				}
			}
			else
			{
				int nStartX = nCutX - nRangeX > 0 ? nCutX - nRangeX : 0;
				int nEndX = nCutX + nRangeX < nSearchArea - 1 ? nCutX + nRangeX : nSearchArea - 1;

				for (int y = 0; y < nSize; y++)
				{
					for (int x = nStartX; x < nEndX; x++)
					{
						if ((UCHAR)pImg.data[y * pImg.cols + x] == 255)
						{
							pContour[nValidCount].x = x + cvRectROI.x;
							pContour[nValidCount].y = y + cvRectROI.y;
							nValidCount++;
							break;
						}
					}
				}
			}
		}
		else
		{
			if (bInnerToOuter)
			{
				int nStartX = nCutX - nRangeX > 0 ? nCutX - nRangeX : 0;
				int nEndX = nCutX + nRangeX < nSearchArea - 1 ? nCutX + nRangeX : nSearchArea - 1;

				for (int y = 0; y < nSize; y++)
				{
					for (int x = nStartX; x < nEndX; x++)
					{
						if ((UCHAR)pImg.data[y * pImg.cols + x] == 255)
						{
							pContour[nValidCount].x = x + cvRectROI.x;
							pContour[nValidCount].y = y + cvRectROI.y;
							nValidCount++;
							break;
						}
					}
				}
			}
			else
			{
				int nStartX = nCutX + nRangeX < nSearchArea - 1 ? nCutX + nRangeX : nSearchArea - 1;
				int nEndX = nCutX - nRangeX > 0 ? nCutX - nRangeX : 0;

				for (int y = 0; y < nSize; y++)
				{
					for (int x = nStartX; x > nEndX; x--)
					{
						if ((UCHAR)pImg.data[y * pImg.cols + x] == 255)
						{
							pContour[nValidCount].x = x + cvRectROI.x;
							pContour[nValidCount].y = y + cvRectROI.y;
							nValidCount++;
							break;
						}
					}
				}
			}
		}

		double cost = ransac_line_fitting(pContour, nValidCount, *pLine, 2);

		if (nValidCount < 100 || cost < nValidCount / 4)
			bFind = FALSE;

	}
	else
	{
		bFind = FALSE;
	}


	delete[]pContour;
	delete[]pHisto;

	return bFind;
}


BOOL CPatternMatching::findLine_Hori(int nCam, cv::Mat& pImg, int nSize, int nSearchArea, int nCornerType, cv::Rect cvRectROI, BOOL bInnerToOuter, sLine *pLine)
{
	BOOL bFind = TRUE;
	BOOL bFindY = FALSE;
	int nThresh = nSize / 4;
	int nCutY = 0;

	sPoint *pContour = new sPoint[nSize];
	int *pHisto = new int[nSearchArea];
	memset(pHisto, 0, sizeof(int) * nSearchArea);

	//////////////////////
	// 가로선을 찾자!!
	if (nCornerType == CORNER_LT || nCornerType == CORNER_RT)
	{
		if (bInnerToOuter)
		{
			// 아래에서 위로 탐색
			for (int y = nSearchArea - 1; y > 0; y--)
			{
				for (int x = 0; x < nSize; x++)
				{
					if ((UCHAR)pImg.data[y * pImg.cols + x] == 255)
					{
						pHisto[y]++;

						if (pHisto[y] > nThresh)
						{
							bFindY = TRUE;
							nCutY = y;
							break;
						}
					}
				}
				if (bFindY) break;
			}
		}
		else
		{
			// 위에서 아래로 탐색
			for (int y = 0; y < nSearchArea; y++)
			{
				for (int x = 0; x < nSize; x++)
				{
					if ((UCHAR)pImg.data[y * pImg.cols + x] == 255)
					{
						pHisto[y]++;

						if (pHisto[y] > nThresh)
						{
							bFindY = TRUE;
							nCutY = y;
							break;
						}
					}
				}
				if (bFindY) break;
			}
		}
	}
	else
	{
		if (bInnerToOuter)
		{
			// 위에서 아래로 탐색
			for (int y = 0; y < nSearchArea; y++)
			{
				for (int x = 0; x < nSize; x++)
				{
					if ((UCHAR)pImg.data[y * pImg.cols + x] == 255)
					{
						pHisto[y]++;

						if (pHisto[y] > nThresh)
						{
							bFindY = TRUE;
							nCutY = y;
							break;
						}
					}
				}
				if (bFindY) break;
			}
		}
		else
		{
			// 아래에서 위로 탐색
			for (int y = nSearchArea - 1; y > 0; y--)
			{
				for (int x = 0; x < nSize; x++)
				{
					if ((UCHAR)pImg.data[y * pImg.cols + x] == 255)
					{
						pHisto[y]++;

						if (pHisto[y] > nThresh)
						{
							bFindY = TRUE;
							nCutY = y;
							break;
						}
					}
				}
				if (bFindY) break;
			}
		}
	}

	int nRangeY = 10;
	int nStartY = nCutY + nRangeY < nSearchArea - 1 ? nCutY + nRangeY : nSearchArea - 1;
	int nEndY = nCutY - nRangeY > 0 ? nCutY - nRangeY : 0;
	int nValidCount = 0;

	if (bFindY)
	{
		if (nCornerType == CORNER_LT || nCornerType == CORNER_RT)
		{
			if (bInnerToOuter)
			{
				// 아래에서 위로 탐색
				for (int x = 0; x < nSize; x++)
				{
					for (int y = nStartY; y > nEndY; y--)
					{
						if ((UCHAR)pImg.data[y * pImg.cols + x] == 255)
						{
							pContour[nValidCount].x = x + cvRectROI.x;
							pContour[nValidCount].y = y + cvRectROI.y;
							nValidCount++;
							break;
						}
					}
				}
			}
			else
			{
				// 위에서 아래로 탐색
				for (int x = 0; x < nSize; x++)
				{
					for (int y = nEndY; y < nStartY; y++)
					{
						if ((UCHAR)pImg.data[y * pImg.cols + x] == 255)
						{
							pContour[nValidCount].x = x + cvRectROI.x;
							pContour[nValidCount].y = y + cvRectROI.y;
							nValidCount++;
							break;
						}
					}
				}
			}

		}
		else
		{
			if (bInnerToOuter)
			{
				// 위에서 아래로 탐색
				for (int x = 0; x < nSize; x++)
				{
					for (int y = nEndY; y < nStartY; y++)
					{
						if ((UCHAR)pImg.data[y * pImg.cols + x] == 255)
						{
							pContour[nValidCount].x = x + cvRectROI.x;
							pContour[nValidCount].y = y + cvRectROI.y;
							nValidCount++;
							break;
						}
					}
				}
			}
			else
			{
				// 아래에서 위로 탐색
				for (int x = 0; x < nSize; x++)
				{
					for (int y = nStartY; y > nEndY; y--)
					{
						if ((UCHAR)pImg.data[y * pImg.cols + x] == 255)
						{
							pContour[nValidCount].x = x + cvRectROI.x;
							pContour[nValidCount].y = y + cvRectROI.y;
							nValidCount++;
							break;
						}
					}
				}
			}
		}

		double cost = ransac_line_fitting(pContour, nValidCount, *pLine, 2);

		if (nValidCount < 100 || cost < nValidCount / 4)
			bFind = FALSE;
	}
	else
	{
		bFind = FALSE;
	}


	delete[]pContour;
	delete[]pHisto;


	return bFind;
}


BOOL CPatternMatching::findHisto_Hori(int nCam, cv::Mat& pImg, int nSize, int nSearchArea, int nCornerType, cv::Rect cvRectROI, BOOL bInnerToOuter, sLine *pLine)
{
	BOOL bFind = FALSE;

	int *pHisto = new int[nSearchArea];
	memset(pHisto, 0, sizeof(int) * nSearchArea);


	// 세로 방향 Histogram을 얻자!!
	// 가로선을 찾자!!
	if (nCornerType == CORNER_LT || nCornerType == CORNER_RT)
	{
		if (bInnerToOuter)
		{
			// 아래에서 위로 탐색
			for (int y = nSearchArea - 1; y > 0; y--)
			{
				for (int x = 0; x < nSize; x++)
				{
					if ((UCHAR)pImg.data[y * pImg.cols + x] == 255)
					{
						pHisto[y]++;
					}
				}
			}
		}
		else
		{
			// 위에서 아래로 탐색
			for (int y = 0; y < nSearchArea; y++)
			{
				for (int x = 0; x < nSize; x++)
				{
					if ((UCHAR)pImg.data[y * pImg.cols + x] == 255)
					{
						pHisto[y]++;
					}
				}
			}
		}
	}
	else
	{
		if (bInnerToOuter)
		{
			// 위에서 아래로 탐색
			for (int y = 0; y < nSearchArea; y++)
			{
				for (int x = 0; x < nSize; x++)
				{
					if ((UCHAR)pImg.data[y * pImg.cols + x] == 255)
					{
						pHisto[y]++;
					}
				}
			}
		}
		else
		{
			// 아래에서 위로 탐색
			for (int y = nSearchArea - 1; y > 0; y--)
			{
				for (int x = 0; x < nSize; x++)
				{
					if ((UCHAR)pImg.data[y * pImg.cols + x] == 255)
					{
						pHisto[y]++;
					}
				}
			}
		}
	}


	int nMax = 0;
	int nMaxIndex = -1;
	// 최대 값을 찾자!!

	for (int y = 0; y < nSearchArea; y++)
	{
		if (nMax < pHisto[y])
		{
			nMax = pHisto[y];
			nMaxIndex = y;
		}
	}

	int nThesh = nSize / 2;


	if (nCornerType == CORNER_LT || nCornerType == CORNER_RT)
	{
		if (bInnerToOuter)
		{
			// 아래에서 위로 탐색
			for (int y = nSearchArea - 1; y > 0; y--)
			{
				if (pHisto[y] >= nThesh)
				{
					pLine->a = 0;
					pLine->b = cvRectROI.y + y;

					pLine->sy = cvRectROI.y + y;
					pLine->sx = cvRectROI.x + nSize / 2;
					bFind = TRUE;
					break;
				}
			}
		}
		else
		{
			// 위에서 아래로 탐색
			for (int y = 0; y < nSearchArea; y++)
			{
				if (pHisto[y] >= nThesh)
				{
					pLine->a = 0;
					pLine->b = cvRectROI.y + y;

					pLine->sy = cvRectROI.y + y;
					pLine->sx = cvRectROI.x + nSize / 2;
					bFind = TRUE;
					break;
				}
			}
		}
	}
	else
	{
		if (bInnerToOuter)
		{
			// 위에서 아래로 탐색
			for (int y = 0; y < nSearchArea; y++)
			{
				if (pHisto[y] >= nThesh)
				{
					pLine->a = 0;
					pLine->b = cvRectROI.y + y;

					pLine->sy = cvRectROI.y + y;
					pLine->sx = cvRectROI.x + nSize / 2;
					bFind = TRUE;
					break;
				}
			}
		}
		else
		{
			// 아래에서 위로 탐색
			for (int y = nSearchArea - 1; y > 0; y--)
			{
				if (pHisto[y] >= nThesh)
				{
					pLine->a = 0;
					pLine->b = cvRectROI.y + y;

					pLine->sy = cvRectROI.y + y;
					pLine->sx = cvRectROI.x + nSize / 2;
					bFind = TRUE;
					break;
				}
			}
		}
	}

	delete[]pHisto;

	return bFind;
}


BOOL CPatternMatching::findHisto_Vert(int nCam, cv::Mat& pImg, int nSize, int nSearchArea, int nCornerType, cv::Rect cvRectROI, BOOL bInnerToOuter, sLine *pLine)
{
	BOOL bFind = FALSE;

	int *pHisto = new int[nSearchArea];
	memset(pHisto, 0, sizeof(int) * nSearchArea);


	////////////////////////////////////
	// 세로선을 찾자
	if (nCornerType == CORNER_LB || nCornerType == CORNER_LT)
	{
		if (bInnerToOuter)
		{
			for (int x = nSearchArea - 1; x > 0; x--)
			{
				for (int y = 0; y < nSize; y++)
				{
					if ((UCHAR)pImg.data[y * pImg.cols + x] == 255)
					{
						pHisto[x]++;
					}
				}
			}
		}
		else
		{
			for (int x = 0; x < nSearchArea; x++)
			{
				for (int y = 0; y < nSize; y++)
				{
					if ((UCHAR)pImg.data[y * pImg.cols + x] == 255)
					{
						pHisto[x]++;
					}
				}
			}
		}
	}
	else
	{
		if (bInnerToOuter)
		{
			for (int x = 0; x < nSearchArea; x++)
			{
				for (int y = 0; y < nSize; y++)
				{
					if ((UCHAR)pImg.data[y * pImg.cols + x] == 255)
					{
						pHisto[x]++;
					}
				}
			}
		}
		else
		{
			for (int x = nSearchArea - 1; x > 0; x--)
			{
				for (int y = 0; y < nSize; y++)
				{
					if ((UCHAR)pImg.data[y * pImg.cols + x] == 255)
					{
						pHisto[x]++;
					}
				}
			}
		}
	}



	int nMax = 0;
	int nMaxIndex = -1;
	// 최대 값을 찾자!!

	for (int y = 0; y < nSearchArea; y++)
	{
		if (nMax < pHisto[y])
		{
			nMax = pHisto[y];
			nMaxIndex = y;
		}
	}

	int nThresh = nSize / 2;

	if (nCornerType == CORNER_LB || nCornerType == CORNER_LT)
	{
		if (bInnerToOuter)
		{
			for (int x = nSearchArea - 1; x > 0; x--)
			{
				if (pHisto[x] >= nThresh)
				{
					pLine->a = 0;
					pLine->b = cvRectROI.x + x;

					pLine->sx = cvRectROI.x + x;
					pLine->sy = cvRectROI.y + nSize / 2;
					bFind = TRUE;
					break;
				}
			}
		}
		else
		{
			for (int x = 0; x < nSearchArea; x++)
			{
				if (pHisto[x] >= nThresh)
				{
					pLine->a = 0;
					pLine->b = cvRectROI.x + x;

					pLine->sx = cvRectROI.x + x;
					pLine->sy = cvRectROI.y + nSize / 2;
					bFind = TRUE;
					break;
				}
			}
		}
	}
	else
	{
		if (bInnerToOuter)
		{
			for (int x = 0; x < nSearchArea; x++)
			{
				if (pHisto[x] >= nThresh)
				{
					pLine->a = 0;
					pLine->b = cvRectROI.x + x;

					pLine->sx = cvRectROI.x + x;
					pLine->sy = cvRectROI.y + nSize / 2;
					bFind = TRUE;
					break;
				}
			}
		}
		else
		{
			for (int x = nSearchArea - 1; x > 0; x--)
			{
				if (pHisto[x] >= nThresh)
				{
					pLine->a = 0;
					pLine->b = cvRectROI.x + x;

					pLine->sx = cvRectROI.x + x;
					pLine->sy = cvRectROI.y + nSize / 2;
					bFind = TRUE;
					break;
				}
			}
		}
	}

	delete[]pHisto;

	return bFind;
}

void CPatternMatching::filterProcessImage_hori(int nCam, int nPos, cv::Mat& pImg, cv::Mat *pMatImg, int nSobelDirection, int nThreshold)
{
	switch (m_nEdgeProcess[nCam][nPos]) {
	case EDGE_SOBEL:
	{
		sobelDirection(3, nSobelDirection, pImg, pImg);	// jsh
		cv::threshold(pImg, pImg, nThreshold, 255, CV_THRESH_BINARY);
	}
	break;
	case EDGE_ENHANCE_B:
	{
		InspectionEnhance(pMatImg, pMatImg, 1);	// enhanceY

		int nTemp = 127 - nThreshold;
		if (nTemp < 0) nTemp = 1;
		cv::threshold(pImg, pImg, nTemp, 255, CV_THRESH_BINARY_INV);
	}
	break;
	case EDGE_ENHANCE_W_V2:
	{
		InspectionEnhance(pMatImg, pMatImg, 1);	// enhanceY

		int nTemp = 128 + nThreshold;
		if (nTemp > 254) nTemp = 255;
		cv::threshold(pImg, pImg, nTemp, 255, CV_THRESH_BINARY);
	}
	break;
	case EDGE_ENHANCE_W:
	{
		InspectionEnhance(pMatImg, pMatImg, 1);	// enhanceY

		int nTemp = 128 + nThreshold;
		if (nTemp > 254) nTemp = 255;
		cv::threshold(pImg, pImg, nTemp, 255, CV_THRESH_BINARY);
	}
	break;
	case EDGE_HISTO_W:
	{
		InspectionEnhance(pMatImg, pMatImg, 1);	// enhanceY

		int nTemp = 128 + nThreshold;
		if (nTemp > 254) nTemp = 255;
		cv::threshold(pImg, pImg, nTemp, 255, CV_THRESH_BINARY);
	}
	break;
	case EDGE_HISTO_B:
	{
		InspectionEnhance(pMatImg, pMatImg, 1);	// enhanceY

		int nTemp = 127 - nThreshold;
		if (nTemp < 0) nTemp = 1;
		cv::threshold(pImg, pImg, nTemp, 255, CV_THRESH_BINARY_INV);
	}
	break;
	}
}

void CPatternMatching::filterProcessImage_vert(int nCam, int nPos, cv::Mat& pImg, cv::Mat *pMatImg, int nSobelDirection, int nThreshold)
{
	switch (m_nEdgeProcess[nCam][nPos]) {
	case EDGE_SOBEL:
	{
		sobelDirection(3, nSobelDirection, pImg, pImg);	// jsh
		cv::threshold(pImg, pImg, nThreshold, 255, CV_THRESH_BINARY);
	}
	break;
	case EDGE_ENHANCE_B:
	{
		InspectionEnhance(pMatImg, pMatImg, 2);	// enhanceY

		int nTemp = 127 - nThreshold;
		if (nTemp < 0) nTemp = 1;
		cv::threshold(pImg, pImg, nTemp, 255, CV_THRESH_BINARY_INV);
	}
	break;
	case EDGE_ENHANCE_W_V2:
	{
		InspectionEnhance(pMatImg, pMatImg, 2);	// enhanceY
		InspectionEnhance(pMatImg, pMatImg, 2);	// enhanceY

		int nTemp = 128 + nThreshold;
		if (nTemp > 254) nTemp = 255;
		cv::threshold(pImg, pImg, nTemp, 255, CV_THRESH_BINARY);
	}
	break;
	case EDGE_ENHANCE_W:
	{
		InspectionEnhance(pMatImg, pMatImg, 2);	// enhanceY

		int nTemp = 128 + nThreshold;
		if (nTemp > 254) nTemp = 255;
		cv::threshold(pImg, pImg, nTemp, 255, CV_THRESH_BINARY);
	}
	break;
	case EDGE_HISTO_W:
	{
		InspectionEnhance(pMatImg, pMatImg, 2);	// enhanceY

		int nTemp = 128 + nThreshold;
		if (nTemp > 254) nTemp = 255;
		cv::threshold(pImg, pImg, nTemp, 255, CV_THRESH_BINARY);
	}
	break;
	case EDGE_HISTO_B:
	{
		InspectionEnhance(pMatImg, pMatImg, 2);	// enhanceY

		int nTemp = 127 - nThreshold;
		if (nTemp < 0) nTemp = 1;
		cv::threshold(pImg, pImg, nTemp, 255, CV_THRESH_BINARY_INV);
	}
	break;
	}
}

void CPatternMatching::InspectionEnhance(cv::Mat *src, cv::Mat *dst, int id, bool disp)
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

	//cv::Mat dst;
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
	else if (id == 1)	// enhanceY
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
	//dst.release();
	element.release();
	srcImage.release();
}

void CPatternMatching::InspectionEnhance_xy7(cv::Mat* src, cv::Mat* dst)
{
	float nKernelGUS[7] = { 23, 43, 63, 71, 63, 43, 23 };									// 329
	float nKernelLOG[13] = { -4, -12, -20, -20, 0, 33, 50, 33, 0, -20, -20, -12, -4 };	// 32

	cv::Mat KrnlGusX = cv::Mat(7, 1, CV_32FC1, nKernelGUS) / 329.f;
	cv::Mat KrnlLOGX = cv::Mat(13, 1, CV_32FC1, nKernelLOG) / 32.f;
	cv::Mat KrnlGusY = cv::Mat(1, 7, CV_32FC1, nKernelGUS) / 329.f;
	cv::Mat KrnlLOGY = cv::Mat(1, 13, CV_32FC1, nKernelLOG) / 32.f;

	cv::Mat Proc1Buf = cv::Mat::zeros(src->rows, src->cols, CV_32FC1);
	cv::Mat Proc2Buf = cv::Mat::zeros(src->rows, src->cols, CV_32FC1);
	cv::Mat Proc3Buf = cv::Mat::zeros(src->rows, src->cols, CV_32FC1);
	cv::Mat TempoBuf = cv::Mat::zeros(src->rows, src->cols, CV_32FC1);

	//	bmpSrcGrayImg(rectROI).convertTo(TempoBuf(rectROI), CV_32F, 1. , -128. );
	src->convertTo(TempoBuf, CV_32F, 1., -128.);
	sepFilter2D(TempoBuf, Proc1Buf, CV_32F, KrnlGusX, KrnlLOGY, cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
	sepFilter2D(TempoBuf, Proc2Buf, CV_32F, KrnlLOGX, KrnlGusY, cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);

	cv::add(Proc1Buf, Proc2Buf, Proc3Buf);

	//	Proc3Buf(rectROI).convertTo(bmpResultImg(rectROI), CV_8U, 1., 128.);
	Proc3Buf.convertTo(*dst, CV_8U, 1., 128.);

	Proc1Buf.release();
	Proc2Buf.release();
	Proc3Buf.release();
	TempoBuf.release();
}

void CPatternMatching::calcLineInfo(int nCam, int nPos, CRect rectROI, int m_nWidth, int m_nHeight)
{
	m_ax[nCam][nPos][0] = m_lineHori[nCam][nPos].a;
	m_ax[nCam][nPos][1] = m_lineVert[nCam][nPos].a;
	m_bc[nCam][nPos][0] = m_lineHori[nCam][nPos].b;
	m_bc[nCam][nPos][1] = m_lineVert[nCam][nPos].b;

	double x[4] = { 0, }, y[4] = { 0, };
	x[0] = rectROI.left;
	x[1] = rectROI.right;
	y[0] = x[0] * m_ax[nCam][nPos][0] + m_bc[nCam][nPos][0];
	y[1] = x[1] * m_ax[nCam][nPos][0] + m_bc[nCam][nPos][0];
	y[2] = rectROI.top;
	y[3] = rectROI.bottom;

	if (m_ax[nCam][nPos][1] != 0)
	{
		x[2] = (y[2] - m_bc[nCam][nPos][1]) / m_ax[nCam][nPos][1];
		x[3] = (y[3] - m_bc[nCam][nPos][1]) / m_ax[nCam][nPos][1];

	}
	else
	{
		x[2] = m_lineVert[nCam][nPos].b;
		x[3] = m_lineVert[nCam][nPos].b;
	}

	for (int i = 0; i < 4; i++)
	{
		getFindInfo(nCam, nPos).SetLineX(i, x[i]);
		getFindInfo(nCam, nPos).SetLineY(i, y[i]);
	}

	// 화면 전체 범위
	x[0] = 0;
	x[1] = m_nWidth - 1;
	y[0] = x[0] * m_ax[nCam][nPos][0] + m_bc[nCam][nPos][0];
	y[1] = x[1] * m_ax[nCam][nPos][0] + m_bc[nCam][nPos][0];
	y[2] = 0;
	y[3] = m_nHeight - 1;

	if (m_ax[nCam][nPos][1] != 0)
	{
		x[2] = (y[2] - m_bc[nCam][nPos][1]) / m_ax[nCam][nPos][1];
		x[3] = (y[3] - m_bc[nCam][nPos][1]) / m_ax[nCam][nPos][1];

	}
	else
	{
		x[2] = m_lineVert[nCam][nPos].b;
		x[3] = m_lineVert[nCam][nPos].b;
	}

	for (int i = 0; i < 4; i++)
	{
		getFindInfo(nCam, nPos).SetLineX_T(i, x[i]);
		getFindInfo(nCam, nPos).SetLineY_T(i, y[i]);
	}


}

void CPatternMatching::init_job(int nJob)
{
	m_nJobID = nJob;
}