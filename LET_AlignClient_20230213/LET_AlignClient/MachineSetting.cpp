#include "StdAfx.h"
#include "MachineSetting.h"

CMachineSetting::CMachineSetting(void)
{
	for (int i = 0; i < MAX_CAMERA; i++)
	{
		for (int pos = 0; pos < NUM_POS; pos++)
		{
			m_dbRotateX[i][pos] = 0.0;
			m_dbRotateY[i][pos] = 0.0;
			m_dbCamResolutionX[i][pos] = 0.009;
			m_dbCamResolutionY[i][pos] = 0.009;
			m_dbOriginX[i][pos] = 0.0;
			m_dbOriginY[i][pos] = 0.0;

			m_dbImageX[i][pos][0] = 0.0;
			m_dbImageX[i][pos][1] = 0.0;
			m_dbImageY[i][pos][0] = 0.0;
			m_dbImageY[i][pos][1] = 0.0;

			m_nCalibDirectionX[i][pos] = DIRECTION_RIGHT;
			m_nCalibDirectionY[i][pos] = DIRECTION_UP;
		}		

		

		m_dAlignLeftPosTargetX[i] = 0.5;
		m_dAlignLeftPosTargetY[i] = 0.5;
		m_dAlignRightPosTargetX[i] = 0.5;
		m_dAlignRightPosTargetY[i] = 0.5;

		for (int pos = 0; pos < NUM_POS; pos++)
		{
			m_bInitCalib[i][pos] = FALSE;
		}
	}

	m_dbMotorT = 0.0;
	m_nWidth = 0;
	m_nHeight = 0;
	m_nNumCalPointsX = 0;
	m_nNumCalPointsY = 0;
}


CMachineSetting::~CMachineSetting(void)
{

}

int CMachineSetting::calculateCalibration(int nCam, int nPos)
{
	if (m_nNumCalPointsX < 2 || m_nNumCalPointsY < 2)	return -1;

	if ((m_nNumCalPointsX * m_nNumCalPointsY) != m_ptImage[nCam][nPos].size() ||
		(m_nNumCalPointsX * m_nNumCalPointsY) != m_ptMotor[nCam][nPos].size())
		return -2;

	m_matPixelToWorld[nCam][nPos] = cv::findHomography(m_ptImage[nCam][nPos], m_ptMotor[nCam][nPos]);
	m_matWorldToPixel[nCam][nPos] = cv::findHomography(m_ptMotor[nCam][nPos], m_ptImage[nCam][nPos]);

	// Tkyuha 20230104 사용하지 않는 코드여서 삭제함
	/*char cFileName[MAX_PATH] = { 0, };
	sprintf(cFileName, "C:\\CalibData%d%d.txt", nCam + 1, nPos + 1);
	FILE *fp = fopen(cFileName, "w");
	char cData[MAX_PATH] = { 0, };

	if (fp != NULL)
	{
		for (int i = 0; i < m_ptImage[nCam][nPos].size(); i++)
		{
			sprintf(cData, "[%d] I : ( %.2f %.2f), M : ( %.2f %.2f )\r\n", i, m_ptImage[nCam][nPos][i].x, m_ptImage[nCam][nPos][i].y, m_ptMotor[nCam][nPos][i].x, m_ptMotor[nCam][nPos][i].y);
			fwrite(cData, strlen(cData), 1, fp);
		}

		fclose(fp);
	}*/

	m_bInitCalib[nCam][nPos] = TRUE;

	//CheckCalibration(nCam, nPos); // Tkyuha 20230104 테스트 목적임

	return 0;
}

BOOL CMachineSetting::halcon_GetTwoPointTheta_Circle(int nCam, int nPos)
{
	if (m_dbMotorT == 0.0) return FALSE;

	double rangle = -m_dbMotorT * CV_PI / 180.0;

	PointF realPt[2], imagePt[2];

	// 실 좌표계 변환
	for (int i = 0; i < 2; i++)
	{
		realPt[i].X = Gdiplus::REAL(m_dbImageX[nCam][nPos][i]);
		realPt[i].Y = Gdiplus::REAL(m_dbImageY[nCam][nPos][i]);
	}

	double dx = (realPt[1].X - realPt[0].X);
	double dy = (realPt[1].Y - realPt[0].Y);

	//if (dx < 0) rangle *= -1; // 확인 필요 20221012 Tkyuha

	// 데이터 유효성 체크
	if ((dx * dx + dy * dy) <= 0.0000001 && (rangle * rangle) <= 0)
	{
		return FALSE;
	}

	double rotateX=0, rotateY=0;
	if((cos(rangle) - 1)!=0)
	{
		rotateX = ((dx * (cos(rangle) - 1) - dy * sin(rangle)) / (2 * (cos(rangle) - 1)) - dx);
		rotateY = ((dy - rotateX * sin(rangle)) / (cos(rangle) - 1));	

		rotateX *= -1;
		rotateY *= -1;

		rotateX += realPt[0].X;
		rotateY += realPt[0].Y;
	}

	m_dbRotateX[nCam][nPos] = rotateX;
	m_dbRotateY[nCam][nPos] = rotateY;
	return TRUE;
}

void CMachineSetting::saveCalibInfoData(CString strPath)
{
	CString strFilePath, strKey, strData;
	strFilePath.Format("%sMachineInfo.ini", strPath);

#ifndef JOB_INFO
	for (int nCam = 0; nCam < MAX_CAMERA; nCam++)
	{
		strKey.Format("CALIB_CHANGE_AXIS_XY%d", nCam + 1);
		strData.Format("%d", m_bCalibChangeAxisXY[nCam]);
		::WritePrivateProfileStringA(_T("MACHINE_INFO"), strKey, strData, strFilePath);

		strKey.Format("CALIB_DIR_REVERSE_X%d", nCam + 1);
		strData.Format("%d", m_bCalibDirReverseX[nCam]);
		::WritePrivateProfileStringA(_T("MACHINE_INFO"), strKey, strData, strFilePath);

		strKey.Format("CALIB_DIR_REVERSE_Y%d", nCam + 1);
		strData.Format("%d", m_bCalibDirReverseY[nCam]);
		::WritePrivateProfileStringA(_T("MACHINE_INFO"), strKey, strData, strFilePath);

		strKey.Format("CALIB_ORIGIN_AT_Y_CENTER%d", nCam + 1);
		strData.Format("%d", m_bCalibOriginAtYCenter[nCam]);
		::WritePrivateProfileStringA(_T("MACHINE_INFO"), strKey, strData, strFilePath);

		strKey.Format("CALIB_ORIGIN_AT_XY_CENTER%d", nCam + 1);
		strData.Format("%d", m_bCalibOriginAtXYCenter[nCam]);
		::WritePrivateProfileStringA(_T("MACHINE_INFO"), strKey, strData, strFilePath);

		strKey.Format("REVISION_ROTATE_XY%d", nCam + 1);
		strData.Format("%d", m_bRevisionRotateXY[nCam]);
		::WritePrivateProfileStringA(_T("MACHINE_INFO"), strKey, strData, strFilePath);
	}
#else
	strKey.Format("CALIB_CHANGE_AXIS_XY");
	strData.Format("%d", m_bCalibChangeAxisXY);
	::WritePrivateProfileStringA(_T("MACHINE_INFO"), strKey, strData, strFilePath);

	strKey.Format("CALIB_DIR_REVERSE_X");
	strData.Format("%d", m_bCalibDirReverseX);
	::WritePrivateProfileStringA(_T("MACHINE_INFO"), strKey, strData, strFilePath);

	strKey.Format("CALIB_DIR_REVERSE_Y");
	strData.Format("%d", m_bCalibDirReverseY);
	::WritePrivateProfileStringA(_T("MACHINE_INFO"), strKey, strData, strFilePath);

	strKey.Format("CALIB_ORIGIN_AT_Y_CENTER");
	strData.Format("%d", m_bCalibOriginAtYCenter);
	::WritePrivateProfileStringA(_T("MACHINE_INFO"), strKey, strData, strFilePath);

	strKey.Format("CALIB_ORIGIN_AT_XY_CENTER");
	strData.Format("%d", m_bCalibOriginAtXYCenter);
	::WritePrivateProfileStringA(_T("MACHINE_INFO"), strKey, strData, strFilePath);

	strKey.Format("REVISION_ROTATE_XY");
	strData.Format("%d", m_bRevisionRotateXY);
	::WritePrivateProfileStringA(_T("MACHINE_INFO"), strKey, strData, strFilePath);
#endif
}

void CMachineSetting::readCalibInfoData(CString strPath)
{
	TCHAR cData[MAX_PATH] = { 0, };
	CString strFilePath, strKey;
	strFilePath.Format("%s\\MachineInfo.ini", strPath);

#ifndef JOB_INFO
	for (int nCam = 0; nCam < MAX_CAMERA; nCam++)
	{
		strKey.Format("CALIB_CHANGE_AXIS_XY%d", nCam + 1);
		::GetPrivateProfileStringA(_T("MACHINE_INFO"), strKey, "0", cData, MAX_PATH, strFilePath);
		 m_bCalibChangeAxisXY[nCam] = atoi(cData);

		 strKey.Format("CALIB_DIR_REVERSE_X%d", nCam + 1);
		 ::GetPrivateProfileStringA(_T("MACHINE_INFO"), strKey, "0", cData, MAX_PATH, strFilePath);
		 m_bCalibDirReverseX[nCam] = atoi(cData);

		 strKey.Format("CALIB_DIR_REVERSE_Y%d", nCam + 1);
		 ::GetPrivateProfileStringA(_T("MACHINE_INFO"), strKey, "0", cData, MAX_PATH, strFilePath);
		 m_bCalibDirReverseY[nCam] = atoi(cData);

		 strKey.Format("CALIB_ORIGIN_AT_Y_CENTER%d", nCam + 1);
		 ::GetPrivateProfileStringA(_T("MACHINE_INFO"), strKey, "0", cData, MAX_PATH, strFilePath);
		 m_bCalibOriginAtYCenter[nCam] = atoi(cData);

		 strKey.Format("CALIB_ORIGIN_AT_XY_CENTER%d", nCam + 1);
		 ::GetPrivateProfileStringA(_T("MACHINE_INFO"), strKey, "0", cData, MAX_PATH, strFilePath);
		 m_bCalibOriginAtXYCenter[nCam] = atoi(cData);

		 strKey.Format("REVISION_ROTATE_XY%d", nCam + 1);
		 ::GetPrivateProfileStringA(_T("MACHINE_INFO"), strKey, "0", cData, MAX_PATH, strFilePath);
		 m_bRevisionRotateXY[nCam] = atoi(cData);
	}
#else
	strKey.Format("CALIB_CHANGE_AXIS_XY");
	::GetPrivateProfileStringA(_T("MACHINE_INFO"), strKey, "0", cData, MAX_PATH, strFilePath);
	m_bCalibChangeAxisXY = atoi(cData);

	strKey.Format("CALIB_DIR_REVERSE_X");
	::GetPrivateProfileStringA(_T("MACHINE_INFO"), strKey, "0", cData, MAX_PATH, strFilePath);
	m_bCalibDirReverseX = atoi(cData);

	strKey.Format("CALIB_DIR_REVERSE_Y");
	::GetPrivateProfileStringA(_T("MACHINE_INFO"), strKey, "0", cData, MAX_PATH, strFilePath);
	m_bCalibDirReverseY = atoi(cData);

	strKey.Format("CALIB_ORIGIN_AT_Y_CENTER");
	::GetPrivateProfileStringA(_T("MACHINE_INFO"), strKey, "0", cData, MAX_PATH, strFilePath);
	m_bCalibOriginAtYCenter = atoi(cData);

	strKey.Format("CALIB_ORIGIN_AT_XY_CENTER");
	::GetPrivateProfileStringA(_T("MACHINE_INFO"), strKey, "0", cData, MAX_PATH, strFilePath);
	m_bCalibOriginAtXYCenter = atoi(cData);

	strKey.Format("REVISION_ROTATE_XY");
	::GetPrivateProfileStringA(_T("MACHINE_INFO"), strKey, "0", cData, MAX_PATH, strFilePath);
	m_bRevisionRotateXY = atoi(cData);
#endif
}

void CMachineSetting::saveRotateCenter(CString strPath, int nCam, int nPos)
{
	CString strFilePath, strKey, strData;
	strFilePath.Format("%sMachineInfo.ini", strPath);

	strKey.Format("ROTATE_X%d_%d", nCam + 1, nPos + 1);
	strData.Format("%5.15f", m_dbRotateX[nCam][nPos]);
	::WritePrivateProfileStringA(_T("MACHINE_INFO"), strKey, strData, strFilePath);

	strKey.Format("ROTATE_Y%d_%d", nCam + 1, nPos + 1);
	strData.Format("%5.15f", m_dbRotateY[nCam][nPos]);
	::WritePrivateProfileStringA(_T("MACHINE_INFO"), strKey, strData, strFilePath);

	strKey.Format("MAKE_ROTATE_DATE_TIME%d_%d", nCam + 1, nPos + 1); // Tkyuha 20211202
	strData.Format("%s", m_strRotateMakeDateTime[nCam][nPos]);
	::WritePrivateProfileStringA(_T("MACHINE_INFO"), strKey, strData, strFilePath);
}

void CMachineSetting::readRotateCenter(CString strPath)
{
	TCHAR cData[MAX_PATH] = { 0, };
	CString strFilePath, strKey;
	strFilePath.Format("%s\\MachineInfo.ini", strPath);

	for (int nCam = 0; nCam < MAX_CAMERA; nCam++)
	{
		for (int nPos = 0; nPos < NUM_POS; nPos++)
		{
			strKey.Format("ROTATE_X%d_%d", nCam + 1, nPos + 1);
			::GetPrivateProfileStringA(_T("MACHINE_INFO"), strKey, "0.0", cData, MAX_PATH, strFilePath);
			m_dbRotateX[nCam][nPos] = atof(cData);

			strKey.Format("ROTATE_Y%d_%d", nCam + 1, nPos + 1);
			::GetPrivateProfileStringA(_T("MACHINE_INFO"), strKey, "0.0", cData, MAX_PATH, strFilePath);
			m_dbRotateY[nCam][nPos] = atof(cData);

			strKey.Format("MAKE_ROTATE_DATE_TIME%d_%d", nCam + 1, nPos + 1); // Tkyuha 20211202
			::GetPrivateProfileStringA(_T("MACHINE_INFO"), strKey, "2021/12/02", cData, MAX_PATH, strFilePath);
			m_strRotateMakeDateTime[nCam][nPos].Format("%s", cData);
		}
	}
}

void CMachineSetting::setCalibrationOrigin(int nCam, int nPos, double pitchX)
{
	double orgX, orgY, worldX, worldY;
	if (nCam % 2 == 0)		orgX = m_nWidth / 2 - (pitchX / 2.0 / m_dbCamResolutionX[nCam][nPos]);
	else					orgX = m_nWidth / 2 + (pitchX / 2.0 / m_dbCamResolutionX[nCam][nPos]);

	orgY = m_nHeight / 2.0;

	PixelToWorld(nCam, nPos, orgX, orgY, &worldX, &worldY, TRUE);
	setOffsetX(nCam, nPos, worldX);
	setOffsetY(nCam, nPos, worldY);
}

void CMachineSetting::readCalibrationData(int nCam, int nPos, CString strPath)
{
	TCHAR cData[MAX_PATH] = { 0, };
	CString strFilePath, strKey, strSection;
	//strFilePath.Format("%sCalibData%d%d.dat", strPath, nCam + 1, nPos + 1);
	strFilePath = strPath;
	int nPointsX = m_nNumCalPointsX;
	int nPointsY = m_nNumCalPointsY;

	cv::Point2f point;

	m_ptImage[nCam][nPos].clear();
	m_ptMotor[nCam][nPos].clear();
	m_bInitCalib[nCam][nPos] = FALSE;
	m_matPixelToWorld[nCam][nPos].release();
	m_matWorldToPixel[nCam][nPos].release();
	m_dbOriginX[nCam][nPos] = 0.0;
	m_dbOriginY[nCam][nPos] = 0.0;

	strKey.Format("NUM_CAL_POINTS_X");
	::GetPrivateProfileStringA(_T("CALIB_INFO"), strKey, "0", cData, MAX_PATH, strFilePath);
	nPointsX = atoi(cData);
	setNumCalPointsX(nPointsX); // 17.12.16

	strKey.Format("NUM_CAL_POINTS_Y");
	::GetPrivateProfileStringA(_T("CALIB_INFO"), strKey, "0", cData, MAX_PATH, strFilePath);
	nPointsY = atoi(cData);
	setNumCalPointsY(nPointsY);	// 17.12.16

	if (nPointsX <= 1 || nPointsY <= 1)	return;

	for (int n = 0; n < nPointsX * nPointsY; n++)
	{
		strKey.Format("%d", n + 1);
		strSection.Format("IMAGE_X");
		::GetPrivateProfileStringA(strSection, strKey, "-9999", cData, MAX_PATH, strFilePath);
		point.x = float(atof(cData));

		strSection.Format("IMAGE_Y");
		::GetPrivateProfileStringA(strSection, strKey, "-9999", cData, MAX_PATH, strFilePath);
		point.y = float(atof(cData));

		if (point.x == -9999 || point.y == -9999)	return;
		m_ptImage[nCam][nPos].push_back(point);

		strSection.Format("MOTOR_X");
		::GetPrivateProfileStringA(strSection, strKey, "-9999", cData, MAX_PATH, strFilePath);
		point.x = float(atof(cData));

		strSection.Format("MOTOR_Y");
		::GetPrivateProfileStringA(strSection, strKey, "-9999", cData, MAX_PATH, strFilePath);
		point.y = float(atof(cData));

		if (point.x == -9999 || point.y == -9999)	return;
		m_ptMotor[nCam][nPos].push_back(point);
	}

	strKey.Format("ORIGIN_X");
	::GetPrivateProfileStringA(_T("CALIB_INFO"), strKey, "0", cData, MAX_PATH, strFilePath);
	m_dbOriginX[nCam][nPos] = atof(cData);

	strKey.Format("ORIGIN_Y");
	::GetPrivateProfileStringA(_T("CALIB_INFO"), strKey, "0", cData, MAX_PATH, strFilePath);
	m_dbOriginY[nCam][nPos] = atof(cData);

	strKey.Format("RESOLUTION_X");
	::GetPrivateProfileStringA(_T("CALIB_INFO"), strKey, "0", cData, MAX_PATH, strFilePath);
	m_dbCamResolutionX[nCam][nPos] = atof(cData);

	strKey.Format("RESOLUTION_Y");
	::GetPrivateProfileStringA(_T("CALIB_INFO"), strKey, "0", cData, MAX_PATH, strFilePath);
	m_dbCamResolutionY[nCam][nPos] = atof(cData);

	strKey.Format("MAKE_DATE_TIME"); // Tkyuha 20211202
	::GetPrivateProfileStringA(_T("CALIB_INFO"), strKey, "2021/12/02", cData, MAX_PATH, strFilePath);
	m_strCalibMakeDateTime[nCam][nPos].Format("%s", cData);

	m_matPixelToWorld[nCam][nPos] = cv::findHomography(m_ptImage[nCam][nPos], m_ptMotor[nCam][nPos], 0);
	m_matWorldToPixel[nCam][nPos] = cv::findHomography(m_ptMotor[nCam][nPos], m_ptImage[nCam][nPos], 0);
	m_bInitCalib[nCam][nPos] = TRUE;
}

void CMachineSetting::readCalibrationDirection(int nCam, int nPos)
{
	int nPointsX = m_nNumCalPointsX;
	int nPointsY = m_nNumCalPointsY;
	int nCount = nPointsX * nPointsY;

	if (m_ptImage[nCam][nPos].size() != nCount)
	{
		return;
	}

	if (m_ptImage[nCam][nPos].size() == 0)
	{
		return;
	}

	int x_check_point = nPointsX - 1;
	if (x_check_point < 0) x_check_point = 0;
	// nPointsX 만큼 움직였을대 x가 이미지상 30 pixel 차이가 나야 움직였다고 판단
	if (fabs(m_ptImage[nCam][nPos][0].x - m_ptImage[nCam][nPos][x_check_point].x) > 30)
	{
		if (m_ptImage[nCam][nPos][0].x < m_ptImage[nCam][nPos][x_check_point].x)
			m_nCalibDirectionX[nCam][nPos] = DIRECTION_RIGHT;
		else
			m_nCalibDirectionX[nCam][nPos] = DIRECTION_LEFT;
	}
	// nPointsX 만큼 움직였을대 y가 이미지상 30 pixel 차이가 나야 움직였다고 판단
	else if (fabs(m_ptImage[nCam][nPos][0].y - m_ptImage[nCam][nPos][x_check_point].y) > 30)
	{
		if (m_ptImage[nCam][nPos][0].y > m_ptImage[nCam][nPos][x_check_point].y)
			m_nCalibDirectionX[nCam][nPos] = DIRECTION_UP;
		else
			m_nCalibDirectionX[nCam][nPos] = DIRECTION_DOWN;
	}
	else
	{
		m_nCalibDirectionX[nCam][nPos] = DIRECTION_EMPTY;
	}

	int y_check_point = nPointsX * (nPointsY - 1);
	if (y_check_point < 0) y_check_point = 0;
	// nPointsX 만큼 움직였을대 y가 이미지상 30 pixel 차이가 나야 움직였다고 판단
	if (fabs(m_ptImage[nCam][nPos][0].y - m_ptImage[nCam][nPos][y_check_point].y) > 30)
	{
		if (m_ptImage[nCam][nPos][0].y > m_ptImage[nCam][nPos][y_check_point].y)
			m_nCalibDirectionY[nCam][nPos] = DIRECTION_UP;
		else
			m_nCalibDirectionY[nCam][nPos] = DIRECTION_DOWN;
	}
	// nPointsX 만큼 움직였을대 x가 이미지상 30 pixel 차이가 나야 움직였다고 판단
	else if (fabs(m_ptImage[nCam][nPos][0].x - m_ptImage[nCam][nPos][y_check_point].x) > 30)
	{
		if (m_ptImage[nCam][nPos][0].x < m_ptImage[nCam][nPos][y_check_point].x)
			m_nCalibDirectionY[nCam][nPos] = DIRECTION_RIGHT;
		else
			m_nCalibDirectionY[nCam][nPos] = DIRECTION_LEFT;
	}
	else
	{
		m_nCalibDirectionY[nCam][nPos] = DIRECTION_EMPTY;
	}
}


void CMachineSetting::saveCalibrationData(int nCam, int nPos, CString strPath)
{
	CString strFilePath, strKey, strData, strSection;
	strFilePath.Format("%sCalibData%d_%d.dat", strPath, nCam + 1, nPos + 1);
	int nPointsX = m_nNumCalPointsX;
	int nPointsY = m_nNumCalPointsY;

	if (nPointsX <= 1 || nPointsY <= 1) 	return;
	if (m_ptImage[nCam][nPos].size() != (nPointsX * nPointsY)) return;
	if (m_ptMotor[nCam][nPos].size() != (nPointsX * nPointsY)) return;

	strKey.Format("NUM_CAL_POINTS_X");
	strData.Format("%d", m_nNumCalPointsX);
	::WritePrivateProfileStringA(_T("CALIB_INFO"), strKey, strData, strFilePath);

	strKey.Format("NUM_CAL_POINTS_Y");
	strData.Format("%d", m_nNumCalPointsY);
	::WritePrivateProfileStringA(_T("CALIB_INFO"), strKey, strData, strFilePath);

	for (int n = 0; n < nPointsX * nPointsY; n++)
	{
		strKey.Format("%d", n + 1);

		strSection.Format("IMAGE_X");
		strData.Format("%f", m_ptImage[nCam][nPos][n].x);
		::WritePrivateProfileStringA(strSection, strKey, strData, strFilePath);

		strSection.Format("IMAGE_Y");
		strData.Format("%f", m_ptImage[nCam][nPos][n].y);
		::WritePrivateProfileStringA(strSection, strKey, strData, strFilePath);

		strSection.Format("MOTOR_X");
		strData.Format("%f", m_ptMotor[nCam][nPos][n].x);
		::WritePrivateProfileStringA(strSection, strKey, strData, strFilePath);

		strSection.Format("MOTOR_Y");
		strData.Format("%f", m_ptMotor[nCam][nPos][n].y);
		::WritePrivateProfileStringA(strSection, strKey, strData, strFilePath);
	}

	strKey.Format("ORIGIN_X");
	strData.Format("%f", m_dbOriginX[nCam][nPos]);
	::WritePrivateProfileStringA(_T("CALIB_INFO"), strKey, strData, strFilePath);

	strKey.Format("ORIGIN_Y");
	strData.Format("%f", m_dbOriginY[nCam][nPos]);
	::WritePrivateProfileStringA(_T("CALIB_INFO"), strKey, strData, strFilePath);

	strKey.Format("RESOLUTION_X");
	strData.Format("%f", m_dbCamResolutionX[nCam][nPos]);
	::WritePrivateProfileStringA(_T("CALIB_INFO"), strKey, strData, strFilePath);

	strKey.Format("RESOLUTION_Y");
	strData.Format("%f", m_dbCamResolutionY[nCam][nPos]);
	::WritePrivateProfileStringA(_T("CALIB_INFO"), strKey, strData, strFilePath);

	strKey.Format("MAKE_DATE_TIME");  // Tkyuha 20211202
	::WritePrivateProfileStringA(_T("CALIB_INFO"), strKey, m_strCalibMakeDateTime[nCam][nPos], strFilePath);
}

void CMachineSetting::PixelToWorld(int nCam, int nPos, double pixelX, double pixelY, double *worldX, double *worldY, BOOL bInit)
{
	if (m_bInitCalib[nCam][nPos] != TRUE) return;
	if (m_matPixelToWorld[nCam][nPos].empty())	return;

	/*std::vector<cv::Point2f> ptInput, ptOutput;

	ptInput.push_back (cv::Point2f ( float(pixelX),  float(pixelY)));*/

	std::vector<cv::Point2f> ptInput(1), ptOutput(1);
	ptInput[0] = cv::Point2f(float(pixelX), float(pixelY));
	cv::perspectiveTransform(ptInput, ptOutput, m_matPixelToWorld[nCam][nPos]);

	if (bInit)
	{
		*worldX = ptOutput[0].x;
		*worldY = ptOutput[0].y;
	}
	else
	{
		*worldX = ptOutput[0].x - m_dbOriginX[nCam][nPos];
		*worldY = ptOutput[0].y - m_dbOriginY[nCam][nPos];
	}

	ptInput.clear();
	ptOutput.clear();
}

void CMachineSetting::WorldToPixel(int nCam, int nPos, double worldX, double worldY, double *pixelX, double *pixelY)
{
	if (m_bInitCalib[nCam][nPos] != TRUE) return;
	if (m_matWorldToPixel[nCam][nPos].empty())	return;

	/*std::vector<cv::Point2f> ptInput, ptOutput;
	ptInput.push_back (cv::Point2f ( float(worldX + m_dbOriginX[nCam]),  float(worldY + m_dbOriginY[nCam])));*/
	std::vector<cv::Point2f> ptInput(1), ptOutput(1);
	ptInput[0] = cv::Point2f(float(worldX + m_dbOriginX[nCam][nPos]), float(worldY + m_dbOriginY[nCam][nPos]));
	//Tkyuha 21-11-22 OrginX, OrginY 추가

	cv::perspectiveTransform(ptInput, ptOutput, m_matWorldToPixel[nCam][nPos]);

	*pixelX = ptOutput[0].x;
	*pixelY = ptOutput[0].y;

	ptInput.clear();
	ptOutput.clear();
}

void CMachineSetting::calculateResolution(int nCam, int nPos, int nxcount, int nycount)
{
	int nSize = int(m_ptImage[nCam][nPos].size());
	double x1, x2, x3, y1, y2, y3;

	PixelToWorld(nCam, nPos, m_ptImage[nCam][nPos][0].x, m_ptImage[nCam][nPos][0].y, &x1, &y1);
	PixelToWorld(nCam, nPos, m_ptImage[nCam][nPos][nxcount - 1].x, m_ptImage[nCam][nPos][nxcount - 1].y, &x2, &y2);
	PixelToWorld(nCam, nPos, m_ptImage[nCam][nPos][nSize - 1].x, m_ptImage[nCam][nPos][nSize - 1].y, &x3, &y3);

	double dx1, dy1, dx2, dy2, dx, dy;

	dx1 = fabs(m_ptImage[nCam][nPos][nxcount - 1].x - m_ptImage[nCam][nPos][0].x);
	dy1 = fabs(m_ptImage[nCam][nPos][nxcount - 1].y - m_ptImage[nCam][nPos][0].y);

	dx2 = fabs(m_ptImage[nCam][nPos][nSize - 1].x - m_ptImage[nCam][nPos][nxcount - 1].x);
	dy2 = fabs(m_ptImage[nCam][nPos][nSize - 1].y - m_ptImage[nCam][nPos][nxcount - 1].y);

	dx = dx1 > dy1 ? dx1 : dy1;
	dy = dx2 > dy2 ? dx2 : dy2;

	m_dbCamResolutionX[nCam][nPos] = fabs((x2 - x1) / dx);
	m_dbCamResolutionY[nCam][nPos] = fabs((y2 - y3) / dy);

	//// 해상도 보정
	/*double xTheta =  0;
	double yTheta =  0;
	double xdist = sqrt(pow(dx1,2)+ pow(dy1, 2));
	double ydist = sqrt(pow(dx2, 2) + pow(dy2, 2));

	double nX_x = m_ptImage[nCam][0].x + xdist * sin(xTheta);
	double nY_x =  (m_ptImage[nCam][0].y > m_ptImage[nCam][nxcount - 1].y)?m_ptImage[nCam][0].y - xdist * cos(xTheta) : m_ptImage[nCam][0].y + xdist * cos(xTheta);
	double nX_y = m_ptImage[nCam][nxcount - 1].x + ydist * sin(yTheta);
	double nY_y = (m_ptImage[nCam][nxcount - 1].y >m_ptImage[nCam][nSize - 1].y)?m_ptImage[nCam][nxcount - 1].y - ydist * cos(yTheta): m_ptImage[nCam][nxcount - 1].y + ydist * cos(yTheta);

	if((nX_x - m_ptImage[nCam][0].x)!=0)		        m_dbCamResolutionX[nCam] = fabs((x2 - x1) / fabs(nX_x- m_ptImage[nCam][0].x));
	if ((nY_y - m_ptImage[nCam][nxcount - 1].y) != 0)	m_dbCamResolutionY[nCam] = fabs((y2 - y3) / fabs(nY_y - m_ptImage[nCam][nxcount - 1].y));*/
//// 여기 까지	
}

/////////////////////////////  참고용
//
//// 해상도 보정
//	double xTheta = dx1 != 0 ? atan((dy1) / (dx1)) : 0;
//	double yTheta = dy2 != 0 ? atan((dx2) / (dy2)) : 0;
//	double degrees;
//	double curdegrees = (xTheta * (180.0 / CV_PI));
//
//	if (curdegrees < 0) degrees = (90 + curdegrees)*-1;  // 왼쪽 하단인경우
//	else 	degrees = 90 - curdegrees;                    // 오른쪽 하단
//	if (m_ptImage[nCam][0].y > m_ptImage[nCam][nxcount - 1].y)				degrees *= -1; //상단 인경우
//	xTheta = -degrees * (CV_PI / 180.0);
//
//	curdegrees = (yTheta * (180.0 / CV_PI));
//	if (curdegrees < 0) degrees = (90 + curdegrees)*-1;  // 왼쪽 하단인경우
//	else 	degrees = 90 - curdegrees;                    // 오른쪽 하단
//	if (m_ptImage[nCam][nxcount - 1].y > m_ptImage[nCam][nSize - 1].y)				degrees *= -1; //상단 인경우
//	yTheta = -degrees * (CV_PI / 180.0);
//
//	double xdist = sqrt(pow(dx1,2)+ pow(dy1, 2));
//	double ydist = sqrt(pow(dx2, 2) + pow(dy2, 2));
//
//	double nX_x = m_ptImage[nCam][0].x + xdist * sin(xTheta);
//	double nY_x =  (m_ptImage[nCam][0].y > m_ptImage[nCam][nxcount - 1].y)	?m_ptImage[nCam][0].y - xdist * cos(xTheta) : m_ptImage[nCam][0].y + xdist * cos(xTheta);
//	double nX_y = m_ptImage[nCam][nxcount - 1].x + ydist * sin(yTheta);
//	double nY_y = (m_ptImage[nCam][nxcount - 1].y > m_ptImage[nCam][nSize - 1].y)?m_ptImage[nCam][nxcount - 1].y - ydist * cos(yTheta): m_ptImage[nCam][nxcount - 1].y + ydist * cos(yTheta);
//
//	if((nX_x - m_ptImage[nCam][0].x)!=0)
//		m_dbCamResolutionX[nCam] = fabs((x2 - x1) / fabs(nX_x- m_ptImage[nCam][0].x));
//	if ((nY_y - m_ptImage[nCam][nxcount - 1].y) != 0)
//		m_dbCamResolutionY[nCam] = fabs((y2 - y3) / fabs(nY_y - m_ptImage[nCam][nxcount - 1].y));
/////////////////////////

void CMachineSetting::MakeDefaultMachineCoordinatesCenter(int nCam, int nPos, double dPitch, BOOL bChangeXY, BOOL bDirReverseX, BOOL bDirReverseY, double rangeX, double rangeY)
{
	int nPointsX = 3, nPointsY = 3;

	cv::Point2f point;

	m_ptImage[nCam][nPos].clear();
	m_ptMotor[nCam][nPos].clear();
	m_bInitCalib[nCam][nPos] = FALSE;
	m_matPixelToWorld[nCam][nPos].release();
	m_matWorldToPixel[nCam][nPos].release();
	m_dbOriginX[nCam][nPos] = 0.0;
	m_dbOriginY[nCam][nPos] = 0.0;

	setNumCalPointsX(nPointsX); // 17.12.16
	setNumCalPointsY(nPointsY);	// 17.12.16

	int nPitchX = int((dPitch / 2.0 / m_dbCamResolutionX[nCam][nPos]));
	int nPitchY = int((dPitch / 2.0 / m_dbCamResolutionY[nCam][nPos]));
	int nOriX = m_nWidth / 2;
	int nOriY = m_nHeight / 2;

	// 이미지상 보이는 Motor 축 방향과 매칭 하기 위해..
	int nDirX = 1, nDirY = 1;
	if (bDirReverseX)	nDirX = -1;
	if (bDirReverseY)	nDirY = -1;

	for (int y = -1; y < 2; y++)
	{
		for (int x = -1; x < 2; x++)
		{
			point.x = float(nOriX + (x * (rangeX / m_dbCamResolutionX[nCam][nPos])));
			point.y = float(nOriY + (y * (rangeY / m_dbCamResolutionY[nCam][nPos])));
			m_ptImage[nCam][nPos].push_back(point);

			if (bChangeXY)
			{
				point.x = float((rangeY * nDirX) * y);
				point.y = float((rangeX * nDirY) * x);
			}
			else
			{
				point.x = float((rangeX * nDirX) * x);
				point.y = float((rangeY * nDirY) * y);
			}

			m_ptMotor[nCam][nPos].push_back(point);
		}
	}

	m_matPixelToWorld[nCam][nPos] = cv::findHomography(m_ptImage[nCam][nPos], m_ptMotor[nCam][nPos], 0);
	m_matWorldToPixel[nCam][nPos] = cv::findHomography(m_ptMotor[nCam][nPos], m_ptImage[nCam][nPos], 0);

	m_bInitCalib[nCam][nPos] = TRUE;
}

void CMachineSetting::MakeVirtualCoordinates(std::vector<cv::Point2f> *pVtImage, std::vector<cv::Point2f> *pVtMotor, int nDir,int dirXY)
{
	int nCount = int(pVtImage->size());
	if (nCount < 3) return;

	if (nDir < 0)	nDir = -1;
	else			nDir = 1;

	std::vector<cv::Point2f> tmpImage;
	std::vector<cv::Point2f> tmpMotor;

	// Data Copy
	for (int i = 0; i < nCount; i++)
	{
		tmpImage.push_back((*pVtImage)[i]);
		tmpMotor.push_back((*pVtMotor)[i]);
	}

	// Original Data Reset
	pVtImage->clear();
	pVtMotor->clear();

	double totalX[2] = { 0.0, };
	double totalY[2] = { 0.0, };
	double centerX[2] = { 0.0, };
	double centerY[2] = { 0.0, };

	for (int i = 0; i < nCount; i++)
	{
		totalX[0] += tmpImage[i].x;
		totalY[0] += tmpImage[i].y;

		totalX[1] += tmpMotor[i].x;
		totalY[1] += tmpMotor[i].y;
	}

	centerX[0] = totalX[0] / nCount;
	centerY[0] = totalY[0] / nCount;

	centerX[1] = totalX[1] / nCount;
	centerY[1] = totalY[1] / nCount;

	double angle[2];
	angle[0] = 90.0 / 180.0 * 3.141592;
	angle[1] = 90.0 * nDir / 180.0 * 3.141592;

	for (int n = 0; n < nCount; n++)
	{
		for (int i = 0; i < nCount; i++)
		{
			double x = (tmpImage[i].x - centerX[0]) * cos(angle[0]) - (tmpImage[i].y - centerY[0]) * sin(angle[0]) + tmpImage[n].x;
			double y = (tmpImage[i].x - centerX[0]) * sin(angle[0]) + (tmpImage[i].y - centerY[0]) * cos(angle[0]) + tmpImage[n].y;
			if(dirXY)			pVtImage->push_back(cv::Point2f(y, x));
			else				pVtImage->push_back(cv::Point2f(x, y));

			x = (tmpMotor[i].x - centerX[1]) * cos(angle[1]) - (tmpMotor[i].y - centerY[1]) * sin(angle[1]) + tmpMotor[n].x;
			y = (tmpMotor[i].x - centerX[1]) * sin(angle[1]) + (tmpMotor[i].y - centerY[1]) * cos(angle[1]) + tmpMotor[n].y;
			if (dirXY)			pVtMotor->push_back(cv::Point2f(y, x));
			else				pVtMotor->push_back(cv::Point2f(x, y));
		}
	}
}

BOOL CMachineSetting::calculate_Calibration_1_Axis_data(int nCam, int nPos,int nDir)
{
	int nLimitX = m_nNumCalPointsX;
	int nLimitY = m_nNumCalPointsY;

	if (m_nNumCalPointsX == 1)
	{
		MakeVirtualCoordinates(&getPtImage(nCam, nPos), &getPtMotor(nCam, nPos), nDir, 0);

		nLimitX = nLimitY;
	}
	else if (m_nNumCalPointsY == 1)
	{
		MakeVirtualCoordinates(&getPtImage(nCam, nPos), &getPtMotor(nCam, nPos), nDir, 1);
	
		nLimitY = nLimitX;
	}

	setNumCalPointsX(nLimitX);
	setNumCalPointsY(nLimitY);

	return TRUE;
}

BOOL CMachineSetting::CheckCalibration(int _nCam, int _nPos)
{
	cv::Mat dst;
	m_matPixelToWorld[_nCam][_nPos].convertTo(dst, CV_64F, 1, 0);

	double mask[9];
	BOOL bReturn = TRUE;
	int _id = 0;
	for (int i = 0 ;i < dst.rows; ++i) 
	{
		double* ptrDst = dst.ptr<double>(i);
		for (int j = 0; j < dst.cols; ++j) 		mask[_id++] = ptrDst[j];
		if (_id >= 9) break;
	}
	if (mask[8] != 0 && mask[8] != 1)
	{
		for (int i = 0; i < 8; ++i)
		{
			mask[i] /= mask[8];
		}
	}

	double D = mask[0] * mask[4] - mask[1] * mask[3];
	double sx = sqrt(pow(mask[0] ,2.) + pow(mask[3] , 2.));
	double sy = sqrt(pow(mask[1], 2.) + pow(mask[2], 2.));
	double P = sqrt(pow(mask[6], 2.) + pow(mask[7], 2.));

	if (D <= 0 || sx < 0.1 || sx>4 || sy < 0.1 || sy>4 || P > 0.002)
	{
		CString msg;
		msg.Format(" Camera = %d, Pos = %d : Calibration data is abnormal.\r\n Run Calibration again !", _nCam, _nPos);
		AfxMessageBox(msg);
		bReturn = FALSE;
	}

	return bReturn;
}