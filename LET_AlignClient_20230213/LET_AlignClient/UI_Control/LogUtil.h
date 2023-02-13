#if !defined(AFX_LOGUTIL_H__AC5FADAC_C6B3_4FEC_85A9_1CE3A243F929__INCLUDED_)
#define AFX_LOGUTIL_H__AC5FADAC_C6B3_4FEC_85A9_1CE3A243F929__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TraceUtil.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLogUtil command target

class CLogUtil
{
public:
	CLogUtil();           
	virtual ~CLogUtil();

	void WriteLog(char *fmt, ... );

// Implementation
protected:
	BOOL m_bMaxSize;
	void GetDateAndTime(char *datetime);
	char m_LastAccessDate[24];
	BOOL m_bChangeDate;
	LONG m_lMaxSize;

	FILE *m_LogFile;
	char m_szFileName[_MAX_PATH];
	char m_szBaseFile[_MAX_PATH];

	FILE* OpenLogFile();
	LONG GetFileSize(char *szFileName);
	BOOL CheckFileSize();
	BOOL CheckDate();

	BOOL SetupLogDirectory();
	void SetupLogFileName();
};

extern CLogUtil	g_Log;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRACEUTIL_H__AC5FADAC_C6B3_4FEC_85A9_1CE3A243F929__INCLUDED_)
