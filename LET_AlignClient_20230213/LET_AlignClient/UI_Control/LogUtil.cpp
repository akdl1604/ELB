// LogUtil.cpp : implementation file
//

#include "stdafx.h"
#include "LogUtil.h"
#include <stdio.h>
#include <sys/stat.h>
#include <fstream>
#include <time.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#define DICONTROL_AGENT_INI _T(".\\setupLog.ini")
#define LOGPATH ".\\Log\\"
#define LOGMAXSIZE 4096
/////////////////////////////////////////////////////////////////////////////
// CLogUtil

CLogUtil		 g_Log;
CRITICAL_SECTION g_CS;

CLogUtil::CLogUtil()
{
	m_LogFile	  = NULL;
	m_lMaxSize	  = LOGMAXSIZE * 1024 ;
	m_bChangeDate = FALSE;
	m_bMaxSize	  = FALSE;

	memset( m_LastAccessDate, 0x00, 24 );

	::GetPrivateProfileString( "Log", "LastDate", "", m_LastAccessDate, 24, DICONTROL_AGENT_INI );

	if( !SetupLogDirectory() )
		return;

	CheckDate();
	SetupLogFileName();

	InitializeCriticalSection(&g_CS);
}

CLogUtil::~CLogUtil()
{
	if(m_LogFile)
	{
		fclose(m_LogFile);
		m_LogFile = NULL;
	}

	DeleteCriticalSection(&g_CS);
}


/////////////////////////////////////////////////////////////////////////////
// CLogUtil message handlers
/****************************************************************/
/*  Function    : SetupLogDirectory								*/
/*	Parameter   : 												*/
/*	Return Val  : BOOL											*/
/*	Doing		:												*/
/*		로그를 저장할 디렉토리를 만든다.						*/
/****************************************************************/
BOOL CLogUtil::SetupLogDirectory()
{
	char szDir[_MAX_PATH];
	char szLogPath[_MAX_PATH];
	memset(szDir,0x00,sizeof szDir);
	memset(szLogPath,0x00,sizeof szLogPath);
	strcpy_s(szLogPath,LOGPATH);
	
	BOOL bRet = FALSE;


	if(!PathFileExists(szLogPath))
		bRet = CreateDirectory(szLogPath, NULL);

	char today[30];
	memset(today, 0x00, sizeof(today));

	SYSTEMTIME t; 
	GetLocalTime(&t);
	sprintf_s(today, "%s%4d%02d\\",szLogPath,t.wYear, t.wMonth);

	if(!PathFileExists(today))
		bRet = CreateDirectory(today, NULL);

	return TRUE;
}

/****************************************************************/
/*  Function    : SetupLogFileName								*/
/*	Parameter   : 												*/
/*	Return Val  : void											*/
/*	Doing		:												*/
/*		로그파일의 이름을 지정한다.								*/
/*		c:\BMSAgent\log\BMSAgent_XXX.log 형식의 기본 이름을	*/
/*		가지며 파일 크기를 체크하여 맥스 크기를 초과하였을 경우 */
/*		~_XXX_number.log 형식의 이름으로 증가한다.				*/
/*		XXX : Sun,Mon,Tue,Wed,Thur,Fri,Sat
/****************************************************************/
void CLogUtil::SetupLogFileName()	// Log File 지정하기
{
	// 현재 날짜
	SYSTEMTIME t;
	GetLocalTime(&t);

	char* context=NULL;
	char szFileName[_MAX_PATH];
	memset(szFileName,0x00,_MAX_PATH);

	// 기본 로그 이름 
	switch(t.wDayOfWeek){
		case 0:	// Sunday
			sprintf_s(szFileName,"%s%04d%02d\\LET-%04d-%02d-%02d_Sun.log",LOGPATH,t.wYear,t.wMonth,t.wYear,t.wMonth,t.wDay);
			break;
		case 1:	// Monday
			sprintf_s(szFileName,"%s%04d%02d\\LET-%04d-%02d-%02d_Mon.log",LOGPATH,t.wYear,t.wMonth,t.wYear,t.wMonth,t.wDay);
			break;
		case 2:	// Tuesday
			sprintf_s(szFileName,"%s%04d%02d\\LET-%04d-%02d-%02d_Tue.log",LOGPATH,t.wYear,t.wMonth,t.wYear,t.wMonth,t.wDay);
			break;
		case 3:	// Wednesday
			sprintf_s(szFileName,"%s%04d%02d\\LET-%04d-%02d-%02d_Wed.log",LOGPATH,t.wYear,t.wMonth,t.wYear,t.wMonth,t.wDay);
			break;
		case 4:	// Thursday
			sprintf_s(szFileName,"%s%04d%02d\\LET-%04d-%02d-%02d_Thur.log",LOGPATH,t.wYear,t.wMonth,t.wYear,t.wMonth,t.wDay);
			break;
		case 5:	// Friday
			sprintf_s(szFileName,"%s%04d%02d\\LET-%04d-%02d-%02d_Fri.log",LOGPATH,t.wYear,t.wMonth,t.wYear,t.wMonth,t.wDay);
			break;
		case 6:	// Saturday
			sprintf_s(szFileName,"%s%04d%02d\\LET-%04d-%02d-%02d_Sat.log",LOGPATH,t.wYear,t.wMonth,t.wYear,t.wMonth,t.wDay);
			break;
	}

	memset(m_szBaseFile,0x00,_MAX_PATH);
	strcpy_s(m_szBaseFile,sizeof(m_szBaseFile),szFileName);
	strcpy_s(m_szBaseFile,sizeof(m_szBaseFile),strtok_s(m_szBaseFile,".",&context));

	if(m_bChangeDate)	// 날짜가 변경되었으면 오늘과 같은 요일 이름을 가진 파일 삭제
	{
		/*CFileFind finder;

		CString strFile;
		strFile.Format("%s*.log",m_szBaseFile);
		BOOL bWorking = finder.FindFile(strFile);
		DWORD dwError = GetLastError();
		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			if(!finder.IsDots())
			{
				if(finder.IsDirectory())
					RemoveDirectory(finder.GetFilePath());
				else
					DeleteFile(finder.GetFilePath());
			}
		}*/
		// 오늘 요일의 새이름 지정
		memset(m_szFileName,0x00,_MAX_PATH);
		strcpy_s(m_szFileName,szFileName);
	
		m_LogFile = OpenLogFile();
		return;
	}

	LONG lSize = GetFileSize(szFileName);

	int nCnt = 1;
	while(1)
	{
		if( m_lMaxSize < lSize)	// 파일이 일정 크기 보다 클때 새 파일 만들기
		{
			memset(szFileName,0x00,_MAX_PATH);
			sprintf_s(szFileName,"%s_%d.log",m_szBaseFile,nCnt);
			
			lSize = GetFileSize(szFileName);
			nCnt++;
		}
		else
		{
			memset(m_szFileName,0x00,_MAX_PATH);
			strcpy_s(m_szFileName,szFileName);
			break;
		}
	}

	m_LogFile = OpenLogFile();
	return;
}

/****************************************************************/
/*  Function    : CheckFileSize									*/
/*	Parameter   : 												*/
/*	Return Val  : void											*/
/*	Doing		:												*/
/*		파일 크기를 체크하여 최대 크기를 초과시 새로운 파일이름	*/
/*		을 지정한다.											*/
/*		파일 크기 체크를 하지 않으면 무한정 큰 파일이 생성된다.	*/
/*		4M 초과시 새로운 파일이름을 정해주고 파일을 오픈한다.	*/
/****************************************************************/
BOOL CLogUtil::CheckFileSize()
{
	if( m_lMaxSize < GetFileSize(m_szFileName))
	{
		m_bMaxSize = TRUE;
	}
	else
		m_bMaxSize = FALSE;

	return m_bMaxSize;

}

/****************************************************************/
/*  Function    : CheckDate										*/
/*	Parameter   : 												*/
/*	Return Val  : void											*/
/*	Doing		:												*/
/*		날짜를 체크하여 날짜 변경시 새로운 로그파일 이름 정하기 */
/*		날짜 체크를 하지 않으면 프로그램 수행중 날짜  변경시	*/
/*		이전 날짜의 로그가 최대크기가 되지 않는한 계속해서 지난 */
/*		날짜의 로그에 쓰여진다.									*/
/****************************************************************/
BOOL CLogUtil::CheckDate()
{
	char date_time[24];
	memset(date_time,0x00,sizeof(date_time));
	GetDateAndTime(date_time);

	if( strncmp( date_time, m_LastAccessDate, 8 ) !=0 )
	{
		m_bChangeDate = TRUE;
		strcpy_s(m_LastAccessDate,date_time);


		::WritePrivateProfileString( _T("Log"),  _T("LastDate"), m_LastAccessDate, DICONTROL_AGENT_INI );
	}
	else
		m_bChangeDate = FALSE;

	return m_bChangeDate;
}

/****************************************************************/
/*  Function    : GetFileSize									*/
/*	Parameter   : szFileName									*/
/*	Return Val  : LONG											*/
/*	Doing		:												*/
/*		szFileName 파일의 크기를 구한다.						*/
/****************************************************************/
LONG CLogUtil::GetFileSize(char *szFileName)
{
	struct _stat buf;
	int result;
	LONG lFileLength;

	/* Get data associated with "stat.c": */
	result = _stat(szFileName, &buf );

	/* Check if statistics are valid: */
	if( result != 0 )
	{
	   lFileLength = -1;
	}
	else
	{
		lFileLength = buf.st_size;
	}
	return lFileLength;
}

/****************************************************************/
/*  Function    : OpenLogFile									*/
/*	Parameter   : 												*/
/*	Return Val  : FILE*											*/
/*	Doing		:												*/
/*		m_szFileName 파일을 a+t 형식으로 오픈한다.				*/
/****************************************************************/
FILE* CLogUtil::OpenLogFile()
{
	if( m_LogFile != NULL )
	{
		fclose(m_LogFile);
		m_LogFile = NULL;
	}

	FILE* fp;
	fopen_s(&fp,m_szFileName,"a+t");

	return fp;
}

/****************************************************************/
/*  Function    : WriteLog										*/
/*	Parameter   : char *fmt,...									*/
/*	Return Val  : void											*/
/*	Doing		:												*/
/*		지정된 로그파일에 파라미터로 넘겨받은 문장을 쓴다.		*/
/*		멀티 스레드 에서 다중 엑세스가 가능하므로 크리티컬 섹션	*/
/*		을 걸어두고 문장을 쓰고 난 다음 크리티컬 섹션을 나간다. */
/****************************************************************/
void CLogUtil::WriteLog(char *fmt, ...)
{
	va_list marker;

	char buff[10240];
	//20080518 김현호 버퍼 크기 변경 
	memset(buff, 0x00, sizeof(buff));

	char today[30];
	memset(today, 0x00, sizeof(today));

	SYSTEMTIME t; 
	GetLocalTime(&t);
	sprintf_s(today, "[%4d/%02d/%02d:%02d:%02d:%02d.%03d]  ",
		t.wYear, t.wMonth, t.wDay, t.wHour, 
		t.wMinute, t.wSecond,t.wMilliseconds);

	EnterCriticalSection(&g_CS);

	if(CheckDate() || CheckFileSize())
	{
		SetupLogFileName();
	}

	va_start(marker, fmt);
	vsprintf_s(buff, fmt, marker);

	fprintf(m_LogFile, "%s%s\n", today, buff);
	fflush(m_LogFile);
	
	LeaveCriticalSection(&g_CS);

	va_end(marker);
}

/****************************************************************/
/*  Function    : GetDateAndTime								*/
/*	Parameter   : char *datetime								*/
/*	Return Val  : void											*/
/*	Doing		:												*/
/****************************************************************/
void CLogUtil::GetDateAndTime(char *datetime)
{
	char buf[24];
	memset( buf, 0x00, 24 );

    SYSTEMTIME t; 
	GetLocalTime(&t);
   
	sprintf_s( buf,sizeof(buf), "%4d%02d%02d", t.wYear,t.wMonth,t.wDay);
 	strcpy_s(datetime,sizeof(buf),buf);
}
