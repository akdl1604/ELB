// ZThreadHandler.cpp: implementation of the CZThreadHandler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZThreadHandler.h"
#include <process.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CZThread::CZThread(CZThreadHandler *pThreadHandler,	unsigned uInitFlag,LPSECURITY_ATTRIBUTES lpSecurity ,
				   unsigned uStackSize ):m_pThreadHandler(pThreadHandler)
{
	if( pThreadHandler == NULL ) return;

	m_hHandle =( HANDLE ) _beginthreadex(lpSecurity, 
		                                 uStackSize,
										 CallThreadHandlerProc,
										 pThreadHandler,
										 uInitFlag,
										 &m_uiThreadID);
}

unsigned _stdcall CZThread::CallThreadHandlerProc(void *pThreadHandler)
{
	CZThreadHandler *pThread = static_cast<CZThreadHandler*>(pThreadHandler);
	return pThread->ThreadHandlerProc();
}

BOOL CZThread::Create(CZThreadHandler *pThreadHandler, unsigned uInitFlag, LPSECURITY_ATTRIBUTES lpSecurity, unsigned uStackSize )
{	
	m_hHandle =( HANDLE ) _beginthreadex(lpSecurity, 
		uStackSize,
		CallThreadHandlerProc,
		pThreadHandler,
		uInitFlag,
		&m_uiThreadID);
	
	if( m_hHandle == NULL )
		return FALSE;
	return TRUE;
}

DWORD CZThread::Resume()
{
	return ::ResumeThread(m_hHandle);
}

DWORD CZThread::Suspend()
{
	return ::SuspendThread(m_hHandle);
}

BOOL CZThread::GetExitCode(DWORD *pDwExitCode)
{
	return ::GetExitCodeThread(m_hHandle,pDwExitCode);
}

BOOL CZThread::SetPriority(int iPriority)
{
	return ::SetThreadPriority(m_hHandle,iPriority);
}

int CZThread::GetPriority()
{
	return ::GetThreadPriority(m_hHandle);
}

DWORD CZThread::GetThreadId()
{
	return static_cast<DWORD>(m_uiThreadID);
}


///======CZThreadHandler Function define
void CZThreadHandler::WaitEvent(INT32 nMilliSec_)
{
	INT32 nMilliSec = 0;
	nMilliSec = (nMilliSec_ / 10);	// Divide Sleep Rate

	if (nMilliSec < 0)
		nMilliSec = 1;
	else {
		for (INT32 i = 0; i < nMilliSec; i++) {
			Sleep(10);
			MyDoEvents();
		}
	}
}
void CZThreadHandler::MyDoEvents()
{
	MSG msg;  // 정보를 받을 MSG 구조의 변수 선언

	// PeekMessage 함수는 메시지를 위한 쓰레드 메시지 큐를 점검해서, 메시지가 있다면 NonZero, 메시지가 없다면 Zero 반환. 
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) == TRUE)
	{   // PM_REMOVE = 처리후에 큐에서 메시지 제거
		// 메세지가 있을 때의 처리
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}