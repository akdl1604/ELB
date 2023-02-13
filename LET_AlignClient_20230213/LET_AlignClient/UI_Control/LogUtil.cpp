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
/*		�α׸� ������ ���丮�� �����.						*/
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
/*		�α������� �̸��� �����Ѵ�.								*/
/*		c:\BMSAgent\log\BMSAgent_XXX.log ������ �⺻ �̸���	*/
/*		������ ���� ũ�⸦ üũ�Ͽ� �ƽ� ũ�⸦ �ʰ��Ͽ��� ��� */
/*		~_XXX_number.log ������ �̸����� �����Ѵ�.				*/
/*		XXX : Sun,Mon,Tue,Wed,Thur,Fri,Sat
/****************************************************************/
void CLogUtil::SetupLogFileName()	// Log File �����ϱ�
{
	// ���� ��¥
	SYSTEMTIME t;
	GetLocalTime(&t);

	char* context=NULL;
	char szFileName[_MAX_PATH];
	memset(szFileName,0x00,_MAX_PATH);

	// �⺻ �α� �̸� 
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

	if(m_bChangeDate)	// ��¥�� ����Ǿ����� ���ð� ���� ���� �̸��� ���� ���� ����
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
		// ���� ������ ���̸� ����
		memset(m_szFileName,0x00,_MAX_PATH);
		strcpy_s(m_szFileName,szFileName);
	
		m_LogFile = OpenLogFile();
		return;
	}

	LONG lSize = GetFileSize(szFileName);

	int nCnt = 1;
	while(1)
	{
		if( m_lMaxSize < lSize)	// ������ ���� ũ�� ���� Ŭ�� �� ���� �����
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
/*		���� ũ�⸦ üũ�Ͽ� �ִ� ũ�⸦ �ʰ��� ���ο� �����̸�	*/
/*		�� �����Ѵ�.											*/
/*		���� ũ�� üũ�� ���� ������ ������ ū ������ �����ȴ�.	*/
/*		4M �ʰ��� ���ο� �����̸��� �����ְ� ������ �����Ѵ�.	*/
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
/*		��¥�� üũ�Ͽ� ��¥ ����� ���ο� �α����� �̸� ���ϱ� */
/*		��¥ üũ�� ���� ������ ���α׷� ������ ��¥  �����	*/
/*		���� ��¥�� �αװ� �ִ�ũ�Ⱑ ���� �ʴ��� ����ؼ� ���� */
/*		��¥�� �α׿� ��������.									*/
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
/*		szFileName ������ ũ�⸦ ���Ѵ�.						*/
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
/*		m_szFileName ������ a+t �������� �����Ѵ�.				*/
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
/*		������ �α����Ͽ� �Ķ���ͷ� �Ѱܹ��� ������ ����.		*/
/*		��Ƽ ������ ���� ���� �������� �����ϹǷ� ũ��Ƽ�� ����	*/
/*		�� �ɾ�ΰ� ������ ���� �� ���� ũ��Ƽ�� ������ ������. */
/****************************************************************/
void CLogUtil::WriteLog(char *fmt, ...)
{
	va_list marker;

	char buff[10240];
	//20080518 ����ȣ ���� ũ�� ���� 
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
