// ZThreadHandler.h: interface for the CZThreadHandler class.

//////////////////////////////////////////////////////////////////////
#ifndef __ZTHREADHANDLER_H_
#define __ZTHREADHANDLER_H_
#include "ZKernelObject.h"

// 스레드 실행에 대한 포인트 역할만 한다.
// default 생서자 없다.
class  CZThreadHandler 
{
public:
	virtual ~CZThreadHandler(){};

// must override
	virtual UINT ThreadHandlerProc()=0;

	bool m_bIsThreadAlive;

	bool m_bIsThreadEnd;

	virtual void WaitEvent(INT32 nMilliSec_);
	virtual void MyDoEvents();
};

class CZThread: public CZKernelObject
{
public:
	CZThread(CZThreadHandler *pThreadHandler= NULL, unsigned uInitFlag =0, LPSECURITY_ATTRIBUTES lpSecurity = NULL, unsigned uStackSize =0);

protected:
	unsigned int m_uiThreadID;
	CZThreadHandler* m_pThreadHandler;

public:
	DWORD Suspend(void);
	DWORD Resume(void);
	BOOL  Terminate(DWORD dwExitCode);
	BOOL  GetExitCode(DWORD *pdwExitCode);
	BOOL  SetPriority(int iPriority);
	int   GetPriority();
	DWORD GetThreadId();

	BOOL  Create(CZThreadHandler *pThreadHandler, unsigned uInitFlag =0, LPSECURITY_ATTRIBUTES lpSecurity = NULL,  unsigned uStackSize =0 );

private:
	static unsigned _stdcall CallThreadHandlerProc( void *pThreadHandler);
};

#endif