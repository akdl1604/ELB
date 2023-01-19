#pragma once
#ifndef __MACHINE_SETTING_H__
#define __MACHINE_SETTING_H__

#include <io.h>

#ifndef _U_POINT_
#define _U_POINT_
template <class T>
struct UPoint {
	T x;
	T y;
};
#endif

template <class Z>
struct IMAGE_SIZE {
	Z width;
	Z height;
};

class CMachineSetting
{
public:
	CMachineSetting(void);
	~CMachineSetting(void);
private:
	int m_nWidth;
	int m_nHeight;
	double m_dbImageX[MAX_CAMERA][NUM_POS][2];	// 실 좌표계
	double m_dbImageY[MAX_CAMERA][NUM_POS][2];
	double m_dbImage0rgX[MAX_CAMERA][NUM_POS][2];	// 실 좌표계
	double m_dbImage0rgY[MAX_CAMERA][NUM_POS][2];
	double m_dbMotorT;
	double m_dbRotateX[MAX_CAMERA][NUM_POS];
	double m_dbRotateY[MAX_CAMERA][NUM_POS];
	double m_dbCamResolutionX[MAX_CAMERA][NUM_POS];
	double m_dbCamResolutionY[MAX_CAMERA][NUM_POS];
	bool m_bCalibration;
	cv::Mat m_cvTrans_mat[MAX_CAMERA][NUM_POS];
	int m_nNumCalPointsX;
	int m_nNumCalPointsY;
	std::vector<cv::Point2f> m_ptImage[MAX_CAMERA][NUM_POS];
	std::vector<cv::Point2f> m_ptMotor[MAX_CAMERA][NUM_POS];
	cv::Mat m_matPixelToWorld[MAX_CAMERA][NUM_POS];
	cv::Mat m_matWorldToPixel[MAX_CAMERA][NUM_POS];
	double m_dbOriginX[MAX_CAMERA][NUM_POS];
	double m_dbOriginY[MAX_CAMERA][NUM_POS];
	double m_dbCalibStartX[MAX_CAMERA][NUM_POS];
	double m_dbCalibStartY[MAX_CAMERA][NUM_POS];
	double m_dAlignLeftPosTargetX[MAX_CAMERA];
	double m_dAlignLeftPosTargetY[MAX_CAMERA];
	double m_dAlignRightPosTargetX[MAX_CAMERA];
	double m_dAlignRightPosTargetY[MAX_CAMERA];
	BOOL m_bInitCalib[MAX_CAMERA][NUM_POS];
	int m_nCalibDirectionX[MAX_CAMERA][NUM_POS]; // 2022-01-29 kbj calibreation direction 
	int m_nCalibDirectionY[MAX_CAMERA][NUM_POS]; // 2022-01-29 kbj calibreation direction 
#ifndef JOB_INFO
	BOOL m_bCalibChangeAxisXY[MAX_CAMERA];
	BOOL m_bCalibDirReverseX[MAX_CAMERA];
	BOOL m_bCalibDirReverseY[MAX_CAMERA];
	BOOL m_bCalibOriginAtYCenter[MAX_CAMERA];
	BOOL m_bCalibOriginAtXYCenter[MAX_CAMERA];
	BOOL m_bRevisionRotateXY[MAX_CAMERA];
#else
	BOOL m_bCalibChangeAxisXY;
	BOOL m_bCalibDirReverseX;
	BOOL m_bCalibDirReverseY;
	BOOL m_bCalibOriginAtYCenter;
	BOOL m_bCalibOriginAtXYCenter;
	BOOL m_bRevisionRotateXY;
#endif

	BOOL m_nMcViewerChange;
	CString m_strCalibMakeDateTime[MAX_CAMERA][NUM_POS];; // Tkyuha 20211202 calibration 수행 날짜 기록
	CString m_strRotateMakeDateTime[MAX_CAMERA][NUM_POS];; // Tkyuha 20211202 Rotation Center 수행 날짜 기록
public:
	void MakeVirtualCoordinates(std::vector<cv::Point2f> *pVtImage, std::vector<cv::Point2f> *pVtMotor, int nDir, int dirXY=0);

	void MakeDefaultMachineCoordinatesCenter(int nCam, int nPos, double dPitch, BOOL bChangeXY, BOOL bDirReverseX, BOOL bDirReverseY, double rangeX, double rangeY);
	BOOL setInitCalib(int nCam, int nPos, BOOL bInit) { m_bInitCalib[nCam][nPos] = bInit; }
	BOOL isInitCalib(int nCam, int nPos) { return m_bInitCalib[nCam][nPos]; }
	void clearPtImage(int nCam, int nPos) { m_ptImage[nCam][nPos].clear(); }
	void clearPtMotor(int nCam, int nPos) { m_ptMotor[nCam][nPos].clear(); }
	std::vector<cv::Point2f> &getPtImage(int nCam, int nPos) { return m_ptImage[nCam][nPos]; }
	std::vector<cv::Point2f> &getPtMotor(int nCam, int nPos) { return m_ptMotor[nCam][nPos]; }
	void setWidth(int width) { m_nWidth = width; }
	void setHeight(int height) { m_nHeight = height; }
	void setMotorT(double value) { m_dbMotorT = value; }
	double getImageX(int nCam, int nPos, int nIndex) { return m_dbImageX[nCam][nPos][nIndex]; }
	double getImageY(int nCam, int nPos, int nIndex) { return m_dbImageY[nCam][nPos][nIndex]; }
	void setImageX(int nCam, int nPos, int nIndex, double value) { m_dbImageX[nCam][nPos][nIndex] = value; }
	void setImageY(int nCam, int nPos, int nIndex, double value) { m_dbImageY[nCam][nPos][nIndex] = value; }
	double getImageOrgX(int nCam, int nPos, int nIndex) { return m_dbImage0rgX[nCam][nPos][nIndex]; }
	double getImageOrgY(int nCam, int nPos, int nIndex) { return m_dbImage0rgY[nCam][nPos][nIndex]; }
	void setImageOrgX(int nCam, int nPos, int nIndex, double value) { m_dbImage0rgX[nCam][nPos][nIndex] = value; }
	void setImageOrgY(int nCam, int nPos, int nIndex, double value) { m_dbImage0rgY[nCam][nPos][nIndex] = value; }
	void setRotateX(int nCam, int nPos, double value) { m_dbRotateX[nCam][nPos] = value; }
	void setRotateY(int nCam, int nPos, double value) { m_dbRotateY[nCam][nPos] = value; }
	double getRotateX(int nCam, int nPos) { return m_dbRotateX[nCam][nPos]; }
	double getRotateY(int nCam, int nPos) { return m_dbRotateY[nCam][nPos]; }
	double getCameraResolutionX(int nCam, int nPos) { return m_dbCamResolutionX[nCam][nPos]; }
	double getCameraResolutionY(int nCam, int nPos) { return m_dbCamResolutionY[nCam][nPos]; }
	void setCameraResolutionX(int nCam, int nPos, double camRes) { m_dbCamResolutionX[nCam][nPos] = camRes; }
	void setCameraResolutionY(int nCam, int nPos, double camRes) { m_dbCamResolutionY[nCam][nPos] = camRes; }
	void saveRotateCenter(CString strPath, int nCam, int nPos);
	void readRotateCenter(CString strPath);
	BOOL halcon_GetTwoPointTheta_Circle(int nCam, int nPos);

	double getCalibStartX(int nCam, int nPos) { return m_dbCalibStartX[nCam][nPos]; }
	double getCalibStartY(int nCam, int nPos) { return m_dbCalibStartY[nCam][nPos]; }
	void setCalibStartX(int nCam, int nPos, double offset) { m_dbCalibStartX[nCam][nPos] = offset; }
	void setCalibStartY(int nCam, int nPos, double offset) { m_dbCalibStartY[nCam][nPos] = offset; }

	double getAlignLeftPosTargetX(int nCam) { return m_dAlignLeftPosTargetX[nCam]; }
	double getAlignLeftPosTargetY(int nCam) { return m_dAlignLeftPosTargetY[nCam]; }
	void setAlignLeftPosTargetX(int nCam, double offset) { m_dAlignLeftPosTargetX[nCam] = offset; }
	void setAlignLeftPosTargetY(int nCam, double offset) { m_dAlignLeftPosTargetY[nCam] = offset; }

	double getAlignRightPosTargetX(int nCam) { return m_dAlignRightPosTargetX[nCam]; }
	double getAlignRightPosTargetY(int nCam) { return m_dAlignRightPosTargetY[nCam]; }
	void setAlignRightPosTargetX(int nCam, double offset) { m_dAlignRightPosTargetX[nCam] = offset; }
	void setAlignRightPosTargetY(int nCam, double offset) { m_dAlignRightPosTargetY[nCam] = offset; }

#ifndef JOB_INFO
	BOOL getCalibChangeAxisXY(int nCam) { return m_bCalibChangeAxisXY[nCam]; }
	BOOL getCalibDirReverseX(int nCam) { return m_bCalibDirReverseX[nCam]; }
	BOOL getCalibDirReverseY(int nCam) { return m_bCalibDirReverseY[nCam]; }
	BOOL setCalibChangeAxisXY(int nCam, BOOL set) { m_bCalibChangeAxisXY[nCam]= set; }
	BOOL setCalibDirReverseX(int nCam, BOOL set) { m_bCalibDirReverseX[nCam]= set; }
	BOOL setCalibDirReverseY(int nCam, BOOL set) { m_bCalibDirReverseY[nCam]= set; }
	BOOL getCalibOriginAtYCenter(int nCam) { return m_bCalibOriginAtYCenter[nCam]; }
	BOOL getCalibOriginAtXYCenter(int nCam) { return m_bCalibOriginAtXYCenter[nCam]; }
	BOOL setCalibOriginAtYCenter(int nCam, BOOL set) { m_bCalibOriginAtYCenter[nCam] = set; }
	BOOL setCalibOriginAtXYCenter(int nCam, BOOL set) { m_bCalibOriginAtXYCenter[nCam] = set; }
	BOOL getRevisionRotateXY(int nCam) { return m_bRevisionRotateXY[nCam]; }
	BOOL setRevisionRotateXY(int nCam, BOOL set) { m_bRevisionRotateXY[nCam] = set; }
#else
	BOOL getCalibChangeAxisXY() { return m_bCalibChangeAxisXY; }
	BOOL getCalibDirReverseX() { return m_bCalibDirReverseX; }
	BOOL getCalibDirReverseY() { return m_bCalibDirReverseY; }
	BOOL setCalibChangeAxisXY(BOOL set) { m_bCalibChangeAxisXY = set; }
	BOOL setCalibDirReverseX(BOOL set) { m_bCalibDirReverseX = set; }
	BOOL setCalibDirReverseY(BOOL set) { m_bCalibDirReverseY = set; }
	BOOL getCalibOriginAtYCenter() { return m_bCalibOriginAtYCenter; }
	BOOL getCalibOriginAtXYCenter() { return m_bCalibOriginAtXYCenter; }
	BOOL setCalibOriginAtYCenter(BOOL set) { m_bCalibOriginAtYCenter = set; }
	BOOL setCalibOriginAtXYCenter(BOOL set) { m_bCalibOriginAtXYCenter = set; }
	BOOL getRevisionRotateXY() { return m_bRevisionRotateXY; }
	BOOL setRevisionRotateXY(BOOL set) { m_bRevisionRotateXY = set; }
#endif
	
	double getOriginOffsetX(int nCam, int nPos) { return m_dbOriginX[nCam][nPos]; }
	double getOriginOffsetY(int nCam, int nPos) { return m_dbOriginY[nCam][nPos]; }
	void setOffsetX(int nCam, int nPos, double offset) { m_dbOriginX[nCam][nPos] = offset; }
	void setOffsetY(int nCam, int nPos, double offset) { m_dbOriginY[nCam][nPos] = offset; }
	void setNumCalPointsX(int nNumPoints) { m_nNumCalPointsX = nNumPoints; }
	void setNumCalPointsY(int nNumPoints) { m_nNumCalPointsY = nNumPoints; }
	int getNumCalPointsX() { return m_nNumCalPointsX; }
	int getNumCalPointsY() { return m_nNumCalPointsY; }
	void addImagePoint(int nCam, int nPos, cv::Point2f ptImage) { m_ptImage[nCam][nPos].push_back(ptImage); }
	void addMotorPoint(int nCam, int nPos, cv::Point2f ptMotor) { m_ptMotor[nCam][nPos].push_back(ptMotor); }
	int calculateCalibration(int nCam, int nPos);
	BOOL calculate_Calibration_1_Axis_data(int nCam, int nPos,int nDir=0);
	void setCalibrationOrigin(int nCam, int nPos, double dPitch);
	void readCalibrationData(int nCam, int nPos, CString strPath);
	void saveCalibrationData(int nCam, int nPos, CString strPath);
	void readCalibInfoData(CString strPath);
	void saveCalibInfoData(CString strPath);
	void PixelToWorld(int nCam, int nPos, double pixelX, double pixelY, double *worldX, double *worldY, BOOL bInit = FALSE);
	void WorldToPixel(int nCam, int nPos, double worldX, double worldY, double *pixelX, double *pixelY);
	void calculateResolution(int nCam, int nPos, int nxcount, int nycount);
	BOOL CheckCalibration(int _nCam, int _nPos);

	int  getMcViewerChange() { return m_nMcViewerChange; }
	void setMcViewerChange(bool bUse) { m_nMcViewerChange = bUse; }

	CString  getMakeClibDateTime(int nCam, int nPos) { return m_strCalibMakeDateTime[nCam][nPos]; }
	void setMakeClibDateTime(int nCam, int nPos,CString strDateTime) { m_strCalibMakeDateTime[nCam][nPos] = strDateTime; }
	CString  getMakeRotateDateTime(int nCam, int nPos) { return m_strRotateMakeDateTime[nCam][nPos]; }
	void setMakeRotateDateTime(int nCam, int nPos,CString strDateTime) { m_strRotateMakeDateTime[nCam][nPos] = strDateTime; }

	int getCalibDirectionX(int nCam, int nPos) { return m_nCalibDirectionX[nCam][nPos]; }
	int setCalibDirectionX(int nCam, int nPos, int nDirectionX) { m_nCalibDirectionX[nCam][nPos] = nDirectionX; }
	int getCalibDirectionY(int nCam, int nPos) { return m_nCalibDirectionY[nCam][nPos]; }
	int setCalibDirectionY(int nCam, int nPos, int nDirectionY) { m_nCalibDirectionY[nCam][nPos] = nDirectionY; }
	void readCalibrationDirection(int nCam, int nPos);
};

#endif