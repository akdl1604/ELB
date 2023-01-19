#pragma once

#ifndef __PANEL_EXIST_INSPECTION_H__
#define __PANEL_EXIST_INSPECTION_H__

#include "PatternMatching.h"

#ifndef MAX_PATTERN
#define MAX_PATTERN 20
#endif

class CPanelExistInspection
{
public:
	CPanelExistInspection(void);
	~CPanelExistInspection(void);

	BOOL m_bEmptyLineDir;
private:
	BOOL m_bValidPosTotalJudge;
	int m_nJudgeCondition;
	int m_iNumPanelInTray;
	double m_dbAvgBrightness[MAX_PANEL];
	double m_dbAvgGrayLimit[MAX_PANEL];
	double m_dbResolution;
	CRect m_rcSearchROI[MAX_PANEL];
	BOOL m_bUseMask[MAX_PANEL][MAX_PATTERN_INDEX];
	BOOL m_bPanelExistJudge[MAX_PANEL];
	BOOL m_bPanelValidPosJudge[MAX_PANEL];
	CFindInfo m_FindInfo[MAX_PANEL];
	double m_dbPanelValidPosLeft[MAX_PANEL];
	double m_dbPanelValidPosTop[MAX_PANEL];
	double m_dbPanelValidPosRight[MAX_PANEL];
	double m_dbPanelValidPosBottom[MAX_PANEL];

	CSliderCtrl m_sliderLightSettingExist;
public:
	CLET_AlignClientDlg *m_pMain;

	void setValidPosTotalJudge(BOOL bJudge) { m_bValidPosTotalJudge = bJudge; }
	void setUseMask(int nPanel, int nIndex, BOOL bUse) { m_bUseMask[nPanel][nIndex] = bUse; }
	void setPanelValidPosLeft(int nPanel, double value) { m_dbPanelValidPosLeft[nPanel] = value; }
	void setPanelValidPosTop(int nPanel, double value) { m_dbPanelValidPosTop[nPanel] = value; }
	void setPanelValidPosRight(int nPanel, double value) { m_dbPanelValidPosRight[nPanel] = value; }
	void setPanelValidPosBottom(int nPanel, double value) { m_dbPanelValidPosBottom[nPanel] = value; }

	void setJudgeCondition(int nCondition) { m_nJudgeCondition = nCondition; }
	void setPanelExistJudge(int nPanel, BOOL bJudge) { m_bPanelExistJudge[nPanel] = bJudge; }
	void setResolution(double dbRes) { m_dbResolution = dbRes; }
	void setNumPanelInTray(int nNumPanel) { m_iNumPanelInTray = nNumPanel; }
	void setAvgBrightness(int nPanel, double dbAvgBrightness) { m_dbAvgBrightness[nPanel] = dbAvgBrightness; }
	void setAvgGrayLimit(int nPanel, double limit) { m_dbAvgGrayLimit[nPanel] = limit; }
	void setSearchROI(int nPanel, CRect rcSearchROI) { m_rcSearchROI[nPanel] = rcSearchROI; }
	void setPanelValidPosJudge(int nPanel, BOOL bJudge) { m_bPanelValidPosJudge[nPanel] = bJudge; }

	BOOL getValidPosTotalJudge() { return m_bValidPosTotalJudge; }
	BOOL getUseMask(int nPanel, int nIndex) { return m_bUseMask[nPanel][nIndex]; }
	double getPanelValidPosLeft(int nPanel) { return m_dbPanelValidPosLeft[nPanel]; }
	double getPanelValidPosTop(int nPanel) { return m_dbPanelValidPosTop[nPanel]; }
	double getPanelValidPosRight(int nPanel) { return m_dbPanelValidPosRight[nPanel]; }
	double getPanelValidPosBottom(int nPanel) { return m_dbPanelValidPosBottom[nPanel]; }
	BOOL getPanelValidPosJudge(int nPanel) { return m_bPanelValidPosJudge[nPanel]; }

	int getJudgeCondition() { return m_nJudgeCondition; }
	BOOL *getPanelExistJudgePointer() { return m_bPanelExistJudge; }
	BOOL getPanelExistJudge(int nPanel) { return m_bPanelExistJudge[nPanel]; }
	CRect getSearchROI(int nPanel) { return m_rcSearchROI[nPanel]; }
	double getResolution() { return m_dbResolution; }
	double getAvgGrayLimit(int nPanel) { return m_dbAvgGrayLimit[nPanel]; }
	double getAvgBrightness(int nPanel) { return m_dbAvgBrightness[nPanel]; }

	int getNumPanelInTray() { return m_iNumPanelInTray; }
	BOOL getExistJudge(int nPanel) {		return m_bPanelExistJudge[nPanel];	}

	BOOL calcPanelAvgBrightness(IplImage *pSrcImage, int nPanel, CRect rectROI);
	BOOL panelValidPosCheck(IplImage *pSrcImage);

	int searchUTG(IplImage *pSrcImage, CRect rectROI);
	int searchUTG2(IplImage *pSrcImage, CRect rectROI);
	int searchSeparator(IplImage *pSrcImage, CRect rectROI);
	int searchSeparator2(BYTE*pImage, CRect rectROI, int nThresh);

	void inspectionPanelExist(IplImage *pSrcImage);

	void endInSearch(IplImage * srcImage, IplImage * dstImage);
	void applyLUT(IplImage * srcImage, IplImage * dstImage, int(&LUT)[256]);
	void calcHistogram(IplImage *srcImage, int(&Histo)[256]);
	cv::Mat fnEnhanceImagCalc(cv::Mat *src);
	void InspectionEnhance(cv::Mat *src, int id, bool disp = false);
	void removeNoise(cv::Mat matImage, int nSize);
	void DoRemoveNoise(int dMode, int width, int height, int widthStep, int i, int j, int threshold, int &hole, BOOL &is_hole, unsigned char *image, unsigned char *label_image);
};

#endif
