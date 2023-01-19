// InspCounter.cpp: implementation of the CInspCounter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "InspCounter.h"
#include "LET_AlignClientDlg.h"
#include <io.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//const TCHAR KEY_STARTUP[] = { _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run") };
#define SYSTEM_ROOT "SOFTWARE\\LET\\AlignInspection\\InspCounter" 

CInspCounter::CInspCounter()
{
	memset(m_n1stPartTimeCount, 0, sizeof(int) * 3 * 2);

	CString str,str2;
	m_nToday = 0;
	m_nMonth = 0;
	m_nYear = 0;
	m_nNowPM = (int)FALSE;

	if(Reg.KeyExists(SYSTEM_ROOT))
	{
		TRACE("opened key\n");
		Reg.SetRootKey(HKEY_CURRENT_USER);
		Reg.SetKey(SYSTEM_ROOT, FALSE);

		m_n1stPartTimeCount[0][1] = Reg.ReadInt("1STDAYOK", 0); 
		m_n1stPartTimeCount[0][0] = Reg.ReadInt("1STDAYNG", 0); 
		m_n1stPartTimeCount[1][1] = Reg.ReadInt("1STSWOK", 0); 
		m_n1stPartTimeCount[1][0] = Reg.ReadInt("1STSWNG", 0); 	
		m_n1stPartTimeCount[2][1] = Reg.ReadInt("1STGYOK", 0); 
		m_n1stPartTimeCount[2][0] = Reg.ReadInt("1STGYNG", 0); 		

		m_nToday = Reg.ReadInt("Today", 0);
		m_nMonth = Reg.ReadInt("Month", 0);
		m_nYear = Reg.ReadInt("Year", 0);

		m_nNowPM = Reg.ReadInt("NowPM", -1);
		if (m_nNowPM == -1){
			m_nNowPM = (int)FALSE;
			Reg.WriteInt("NowPM", m_nNowPM);
		}

		if (m_nMonth == 0 || m_nYear == 0 || m_nToday == 0)
		{
			::GetLocalTime(&m_csTime);
			m_nToday = m_csTime.wDay;
			m_nMonth = m_csTime.wMonth;
			m_nYear = m_csTime.wYear;
			Reg.WriteInt("Today", m_nToday);
			Reg.WriteInt("Month", m_nMonth);
			Reg.WriteInt("Year", m_nYear);
		}
		
		m_nLastInspYear		= Reg.ReadInt("LastInspYear", 0);
		m_nLastInspMonth	= Reg.ReadInt("LastInspMonth", 0);
		m_nLastInspDay		= Reg.ReadInt("LastInspDay", 0);
		m_nLastInspHour		= Reg.ReadInt("LastInspHour", 0);
		m_nLastInspMinute	= Reg.ReadInt("LastInspMinute", 0);
		m_nLastInspSecond	= Reg.ReadInt("LastInspSecond", 0);

		if( m_nLastInspYear == 0 || m_nLastInspMonth == 0 || m_nLastInspDay == 0 )
		{
			::GetLocalTime(&m_csTime);

			m_nLastInspYear = m_csTime.wYear;
			m_nLastInspMonth = m_csTime.wMonth;
			m_nLastInspDay = m_csTime.wMonth;
			m_nLastInspHour = m_csTime.wHour;
			m_nLastInspMinute = m_csTime.wMinute;
			m_nLastInspSecond = m_csTime.wSecond;

			Reg.WriteInt("LastInspYear", m_nLastInspYear);
			Reg.WriteInt("LastInspMonth", m_nLastInspMonth);
			Reg.WriteInt("LastInspDay", m_nLastInspDay);
			Reg.WriteInt("LastInspHour", m_nLastInspHour);
			Reg.WriteInt("LastInspMinute", m_nLastInspMinute);
			Reg.WriteInt("LastInspSecond", m_nLastInspSecond);
		}
	}
	else
	{
		Reg.CreateKey(SYSTEM_ROOT);

		Reg.WriteInt("1STDAYOK", 0); 
		Reg.WriteInt("1STDAYNG", 0); 
		Reg.WriteInt("1STSWNG", 0); 
		Reg.WriteInt("1STSWOK", 0); 
		Reg.WriteInt("1STGYOK", 0); 
		Reg.WriteInt("1STGYNG", 0); 	

		::GetLocalTime(&m_csTime);
		m_nToday = m_csTime.wDay;
		m_nMonth = m_csTime.wMonth;
		m_nYear = m_csTime.wYear;
		Reg.WriteInt("Today", m_nToday);
		Reg.WriteInt("Month", m_nMonth);
		Reg.WriteInt("Year", m_nYear);

		m_nLastInspYear = m_csTime.wYear;
		m_nLastInspMonth = m_csTime.wMonth;
		m_nLastInspDay = m_csTime.wMonth;
		m_nLastInspHour = m_csTime.wHour;
		m_nLastInspMinute = m_csTime.wMinute;
		m_nLastInspSecond = m_csTime.wSecond;

		Reg.WriteInt("LastInspYear", m_nLastInspYear);
		Reg.WriteInt("LastInspMonth", m_nLastInspMonth);
		Reg.WriteInt("LastInspDay", m_nLastInspDay);
		Reg.WriteInt("LastInspHour", m_nLastInspHour);
		Reg.WriteInt("LastInspMinute", m_nLastInspMinute);
		Reg.WriteInt("LastInspSecond", m_nLastInspSecond);

		

		Reg.WriteInt("NowPM", m_nNowPM);	
	
		TRACE("maked key\n");
		Reg.SetRootKey(HKEY_CURRENT_USER);
		Reg.SetKey(SYSTEM_ROOT, FALSE);
	}	
}

CInspCounter::~CInspCounter()
{

}


void CInspCounter::Inc1stTodayTotalCount(BOOL bOKNG)
{
	if ((BOOL)m_nNowPM)		return;

	int i = NowPartIndex();

	if (!bOKNG)
	{
		m_n1stPartTimeCount[i][0]++;

		switch(i)
		{
		case 0:
			Reg.WriteInt("1STDAYNG", m_n1stPartTimeCount[i][0]); 
			break;
		case 1:
			Reg.WriteInt("1STSWNG", m_n1stPartTimeCount[i][0]); 
			break;
		case 2:
			Reg.WriteInt("1STGYNG", m_n1stPartTimeCount[i][0]); 
			break;
		}
	}

	m_n1stPartTimeCount[i][1]++;

	switch(i)
	{
	case 0:
		Reg.WriteInt("1STDAYOK", m_n1stPartTimeCount[i][1]); 
		break;
	case 1:
		Reg.WriteInt("1STSWOK", m_n1stPartTimeCount[i][1]); 
		break;
	case 2:
		Reg.WriteInt("1STGYOK", m_n1stPartTimeCount[i][1]); 
		break;
	}

	if(theApp.m_pFrame->m_pPane[PANE_AUTO]!=NULL)
		::SendMessage(theApp.m_pFrame->m_pPane[PANE_AUTO]->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_PA_UPDATE_INSP_COUNT, NULL);	//수량 갱신..

	SYSTEMTIME time;
	::GetLocalTime(&time);

	m_nLastInspYear = time.wYear;
	m_nLastInspMonth = time.wMonth;
	m_nLastInspDay = time.wDay;
	m_nLastInspHour = time.wHour;
	m_nLastInspMinute = time.wMinute;
	m_nLastInspSecond = time.wSecond;

	Reg.WriteInt("LastInspYear", m_nLastInspYear);
	Reg.WriteInt("LastInspMonth", m_nLastInspMonth);
	Reg.WriteInt("LastInspDay", m_nLastInspDay);
	Reg.WriteInt("LastInspHour", m_nLastInspHour);
	Reg.WriteInt("LastInspMinute", m_nLastInspMinute);
	Reg.WriteInt("LastInspSecond", m_nLastInspSecond);
}

void CInspCounter::TodayStart()
{
	::GetLocalTime(&m_csTime);
	m_nToday = m_csTime.wDay;
	m_nMonth = m_csTime.wMonth;
	m_nYear = m_csTime.wYear;
	Reg.WriteInt("Today", m_nToday);
	Reg.WriteInt("Month", m_nMonth);
	Reg.WriteInt("Year", m_nYear);	
	TodayCountReset();
}

void CInspCounter::TodayCountReset()
{
	memset(m_n1stPartTimeCount, 0, sizeof(int) * 3 * 2);
	Reg.WriteInt("1STDAYOK", 0); 
	Reg.WriteInt("1STDAYNG", 0); 
	Reg.WriteInt("1STSWNG", 0); 
	Reg.WriteInt("1STSWOK", 0);
	Reg.WriteInt("1STGYOK", 0); 
	Reg.WriteInt("1STGYNG", 0); 

	if(theApp.m_pFrame->m_pPane[PANE_AUTO]!=NULL)
		::SendMessage(theApp.m_pFrame->m_pPane[PANE_AUTO]->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_PA_UPDATE_INSP_COUNT, NULL);		//수량 갱신..
}

int CInspCounter::NowPartIndex()
{
	// SEC 시간대별 조편성.
	// 09:00 ~ 22:00  DAY		index 0
	// 22:00 ~ 09:00  GY 		index 2
//
//	int index = 0;
//
//	::GetLocalTime(&m_csTime);
//	if (DAY_START_TIME > m_csTime.wHour || m_csTime.wHour >= GY_START_TIME)				index = 2; //Gy
//	else if (DAY_START_TIME <= m_csTime.wHour && m_csTime.wHour < GY_START_TIME)		index = 0; // Day
//
//	return index;

	// SEC 시간대별 조편성.
	// 06:00 ~ 14:00  DAY		index 0
	// 14:00 ~ 22:00  SWING		index 1
	// 22:00 ~ 06:00  GY 		index 2

	// 파주 FPR 시간대별 조편성
	// 07:00 ~ 15:00  DAY		index 0
	// 15:00 ~ 23:00  SWING		index 1
	// 23:00 ~ 07:00  GY 		index 2


	int index = 0;

	::GetLocalTime(&m_csTime);
	if (0 <= m_csTime.wHour && m_csTime.wHour < 6)				index = 2;
	else if (6 <= m_csTime.wHour && m_csTime.wHour < 14)		index = 0;
	else if (14 <= m_csTime.wHour && m_csTime.wHour < 22)		index = 1;
	else if (22 <= m_csTime.wHour)                        		index = 2;

	return index;
}

void CInspCounter::Enable(BOOL bEnable)
{
	m_nNowPM = (int)bEnable;
	Reg.WriteInt("NowPM", m_nNowPM);
}

