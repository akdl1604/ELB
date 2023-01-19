#include "stdafx.h"

#pragma warning (disable:4786)
#include <string>
#include <io.h>
#include "LET_AlignClient.h"
#include "Log.h"
#include "LET_AlignClientDlg.h"

CLog::CLog()
{
	InitializeCriticalSection(&m_csLog);
}
CLog::~CLog()
{
		DeleteCriticalSection(&m_csLog);
}

CString	CLog::get_filehead(int type)
{
	CString strHead = theApp.m_pFrame->m_strHistoryDir;

	switch(type)
	{
		case  LOG_PROCESS:		strHead += "Process\\PRO";					break;
		case  LOG_OPERATION:	strHead += "Operation\\OPN";				break;
		case  LOG_ERROR:		strHead += "Error\\ERR";					break;
		case  LOG_INSPECTOR:	strHead += "Inspector\\INS";				break;
		case  LOG_PLC_SEQ:		strHead += "PlcSeq\\SEQ";					break;
		case  LOG_PLC_IF:		strHead += "PlcIF\\IF";						break;
		case  LOG_ETC:			strHead += "Etc\\ETC";						break;
		case  LOG_CONFIG:		strHead += "Config\\CFG";					break;
		case  LOG_CALIBRATION:	strHead += "Calibration\\Calibration";		break;
		case  LOG_LOGIN:		strHead += "LogIn\\LogIn";					break;
	}
	
	return strHead;
}

CString	CLog::get_log_date(int prev_day)
{
	SYSTEMTIME st;
	FILETIME   ft;
	WORD       dt,tt;

	GetLocalTime(&st);
	SystemTimeToFileTime(&st,&ft);
	FileTimeToDosDateTime(&ft, &dt, &tt);
	dt -= prev_day;
	DosDateTimeToFileTime(dt,tt,&ft);
	FileTimeToSystemTime(&ft,&st);

	CString strLogDate;
	strLogDate.Format("%04d%02d%02d",st.wYear,st.wMonth,st.wDay);

	return strLogDate;
}

int	CLog::LogDirectoryCreate(int type)
{
	CString fileDir;

	fileDir = theApp.m_pFrame->m_strHistoryDir; CreateDirectory(fileDir, NULL);
	fileDir = theApp.m_pFrame->m_strHistoryDir + "Process"; CreateDirectory(fileDir, NULL);
	fileDir = theApp.m_pFrame->m_strHistoryDir + "Operation"; CreateDirectory(fileDir, NULL);
	fileDir = theApp.m_pFrame->m_strHistoryDir + "Error"; CreateDirectory(fileDir, NULL);
	fileDir = theApp.m_pFrame->m_strHistoryDir + "Inspector"; CreateDirectory(fileDir, NULL);
	fileDir = theApp.m_pFrame->m_strHistoryDir + "PlcSeq"; CreateDirectory(fileDir, NULL);		// 16.07.30 jmLee
	fileDir = theApp.m_pFrame->m_strHistoryDir + "PlcIF"; CreateDirectory(fileDir, NULL);		// 16.07.30 jmLee
	fileDir = theApp.m_pFrame->m_strHistoryDir + "Etc"; CreateDirectory(fileDir, NULL);			// 16.07.30 jmLee
	fileDir = theApp.m_pFrame->m_strHistoryDir + "Config"; CreateDirectory(fileDir, NULL);		// 16.07.30 jmLee
	fileDir = theApp.m_pFrame->m_strHistoryDir + "Calibration"; CreateDirectory(fileDir, NULL);
	fileDir = theApp.m_pFrame->m_strHistoryDir + "LogIn"; CreateDirectory(fileDir, NULL);		//KJH 2022-07-02 LogIn관련 로그추가
	fileDir = theApp.m_pFrame->m_strHistoryDir + "InspPara"; CreateDirectory(fileDir, NULL);		//kmb 221105 Z Gap Offset 변경 로그 추가

	return 1;
}

CString	CLog::get_log_filename(int type, int prev_day)
{
	CString strFileName;

	strFileName = get_filehead(type);
	strFileName += get_log_date(prev_day);
	strFileName += ".log";

	return strFileName;
}

int CLog::InspParamsg(int type, const char* msg) {
	EnterCriticalSection(&m_csLog);

	CString strFileName = "";
	SYSTEMTIME st;

	GetLocalTime(&st);
	CString strTemp, strText;

	strText.Format("%s", msg);
	strTemp.Format("%04d-%02d-%02d\t%02d:%02d:%02d.%03d\t%s\n",
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, strText);

	strFileName = get_log_filename(type, 0);

	LogDirectoryCreate(type);

	CStdioFile WriteFile;
	if (!WriteFile.Open(strFileName, CFile::modeNoTruncate | CFile::modeCreate
		| CFile::modeWrite | CFile::shareDenyNone | CFile::typeText))
	{
		LeaveCriticalSection(&m_csLog);	// 16.07.30 jmLee
		return 0;
	}

	WriteFile.SeekToEnd();
	WriteFile.WriteString(strTemp);
	WriteFile.Close();

	LeaveCriticalSection(&m_csLog);	// 16.07.30 jmLee

	return 0;

}

int CLog::logmsg(int type, const char *msg)
{
	EnterCriticalSection( &m_csLog );	// 16.07.30 jmLee

	CString strFileName="";
	SYSTEMTIME st;

	GetLocalTime(&st);
	CString strTemp, strText;

	strText.Format("%s",msg);
	strTemp.Format("%04d-%02d-%02d\t%02d:%02d:%02d.%03d\t%s\n",
		st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond, st.wMilliseconds, strText);

	strFileName = get_log_filename(type,0);

	LogDirectoryCreate(type);

	CStdioFile WriteFile;
	if( !WriteFile.Open(strFileName,  CFile::modeNoTruncate |  CFile::modeCreate 
		| CFile::modeWrite | CFile::shareDenyNone | CFile::typeText) )
	{
		LeaveCriticalSection( &m_csLog );	// 16.07.30 jmLee
		return 0;
	}

	WriteFile.SeekToEnd();
	WriteFile.WriteString(strTemp);
	WriteFile.Close();

	LeaveCriticalSection( &m_csLog );	// 16.07.30 jmLee

	return 0;
}

int CLog::logmsg(int type, char *format,...)
{
	EnterCriticalSection( &m_csLog );	// 16.07.30 jmLee

	CString strFileName="";

	SYSTEMTIME st;

	char msg[_MAX_PATH];
	va_list args;

	va_start(args, format);
	vsprintf(msg, format, args);
	va_end(args);

	char buffer[_MAX_PATH];

	GetLocalTime(&st);
	memset(buffer, 0x00, sizeof(buffer));

	GetLocalTime(&st);
	CString strTemp, strText;

	strText.Format("%s",msg);
	strTemp.Format("%04d-%02d-%02d\t%02d:%02d:%02d.%03d\t%s\n",
		st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond, st.wMilliseconds, strText);

	strFileName = get_log_filename(type,0);

	LogDirectoryCreate(type);
	CStdioFile WriteFile;

	/////////////////////////////////////////////
	/////////////////////////////////////////////

	if( !WriteFile.Open(strFileName,  (CFile::modeNoTruncate | CStdioFile::modeCreate 
		| CStdioFile::modeWrite | CStdioFile::shareDenyNone | CStdioFile::typeText)) )
	{
		LeaveCriticalSection( &m_csLog );	// 16.07.30 jmLee
		return 0;
	}

	WriteFile.SeekToEnd();
	WriteFile.WriteString(strTemp);
	WriteFile.Close();

	LeaveCriticalSection( &m_csLog );	// 16.07.30 jmLee
	return 0;
}

int CLog::make_path(const char *path)
{
	std::string search_p, org_p;
	int last, dc_pos;

	org_p = path;
	last = 0;

	while ((dc_pos = int(org_p.find_first_of('\\', last))) > 0) {
		search_p = org_p.substr(0, dc_pos);
	
		if (_access(search_p.data(), 0) < 0) {
			if (CreateDirectory(search_p.data(), NULL) == FALSE) return -1;
		}

		last = int(search_p.size() + 2);
	}

	return 0;		
}


int CLog::plclogmsg(int type, int id,const char *msg)
{
	EnterCriticalSection( &m_csLog );	// 16.07.30 jmLee

	CString strFileName="";
	SYSTEMTIME st;

	GetLocalTime(&st);
	CString strTemp, strText;

	strText.Format("%s",msg);
	strTemp.Format("%04d-%02d-%02d\t%02d:%02d:%02d.%03d\t%s\n",
		st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond, st.wMilliseconds, strText);

	CString test;

	strFileName = get_filehead(type);
	strFileName += get_log_date(0);
	test.Format("_Unit%02d.log",id);
	strFileName += test;

	LogDirectoryCreate(type);

	CStdioFile WriteFile;
	if( !WriteFile.Open(strFileName,  CFile::modeNoTruncate |  CFile::modeCreate 
		| CFile::modeWrite | CFile::shareDenyNone | CFile::typeText) )
	{
		LeaveCriticalSection( &m_csLog );	// 16.07.30 jmLee
		return 0;
	}

	WriteFile.SeekToEnd();
	WriteFile.WriteString(strTemp);
	WriteFile.Close();

	LeaveCriticalSection( &m_csLog );	// 16.07.30 jmLee

	return 0;
}