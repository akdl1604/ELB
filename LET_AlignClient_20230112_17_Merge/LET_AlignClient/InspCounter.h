// InspCounter.h: interface for the CInspCounter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INSPCOUNTER_H__86E56F82_E1E7_43FD_9FEF_35429A83F94B__INCLUDED_)
#define AFX_INSPCOUNTER_H__86E56F82_E1E7_43FD_9FEF_35429A83F94B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Registry.h"
#define DAY_START_TIME 6
#define GY_START_TIME 22

class CInspCounter  
{
public:
	CInspCounter();
	virtual ~CInspCounter();
	
	void Inc1stTodayTotalCount(BOOL bOKNG);	//���� �˻���� ++
	void TodayStart();			//Day �� �����϶� �˻���� Reset
	void TodayCountReset();		//�˻���� Reset	
	
	void Enable(BOOL bEnable);

	void DefectInfoPartTimeCountReset();
	
	// 20111018 Private ���� Public ���� �ű�
	int NowPartIndex();
private:


//Attributes
public:	
	int m_nLastInspYear;
	int m_nLastInspMonth;
	int m_nLastInspDay;
	int m_nLastInspHour;
	int m_nLastInspMinute;
	int m_nLastInspSecond;


	int m_n1stPartTimeCount[3][2];	//���� �˻���� Day(0), SW(1), GY(2), �ҷ�(0), �˻����(1)

	BOOL m_bInsp;
	int m_nToday;
	int m_nMonth;
	int m_nYear;
	int m_nNowPM;

	CRegistry Reg;
private:
	SYSTEMTIME m_csTime;
	
};

#endif // !defined(AFX_INSPCOUNTER_H__86E56F82_E1E7_43FD_9FEF_35429A83F94B__INCLUDED_)
