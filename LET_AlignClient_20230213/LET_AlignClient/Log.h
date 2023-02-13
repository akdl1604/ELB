#ifndef __LOG_H__
#define __LOG_H__


class CLog {
public:
	CLog();
	~CLog();

	CRITICAL_SECTION m_csLog;

public:
	 
	CString	get_filehead(int type);
	CString	get_log_date(int prev_day);
	int	LogDirectoryCreate(int type);
	CString	get_log_filename(int type, int prev_day);
	int logmsg(int type, const char *msg);
	int logmsg(int type, char *format,...);
	int make_path(const char *path);
	int plclogmsg(int type, int id,const char *msg);
	int InspParamsg(int type, const char* msg);
};

extern CLog theLog;
#endif //__BASIC_FUNCTIONS_H__
