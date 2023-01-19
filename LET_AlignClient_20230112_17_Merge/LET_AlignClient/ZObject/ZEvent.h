#pragma once

//////////////////////////////////////////////////////////////////////
// ZEvent.h: interface for the CZEvent class.
//
//////////////////////////////////////////////////////////////////////

#include "ZKernelObject.h"

class  CZEvent : public CZKernelObject
{
public:
	CZEvent(BOOL btMenualReset = FALSE, BOOL bInitialState = FALSE,
			LPCTSTR lpName = NULL, LPSECURITY_ATTRIBUTES lpEventAttributes = NULL);
	
	CZEvent(LPCTSTR lpName, BOOL bInheritHandle = FALSE, DWORD dwDesiredAccess = 0 /*EVENT_ALL_ACCESS*/);

public:
	BOOL Set();
	BOOL Reset();
	BOOL Pulse();

	void	Lock();
	void	Unlock();
	BOOL	IsLock();

protected:
	BOOL m_bLock;
};

class  CZSemaphore : public CZKernelObject
{
public:
	CZSemaphore(int iInitialCount, int iMaxCount, LPCTSTR lpName = NULL, LPSECURITY_ATTRIBUTES lpSemaphoreAttributes= NULL);
	CZSemaphore(LPCTSTR lpName = NULL, BOOL bInheritHandle = FALSE, DWORD  dwDesiredAccess =SEMAPHORE_ALL_ACCESS);

	BOOL Release(LONG lReleaseCount, LONG *plPreviousCount = NULL);
};

class  CZMutex: public CZKernelObject
{
public:
	CZMutex(BOOL bInitialOwner = FALSE, LPCTSTR lpName = NULL, LPSECURITY_ATTRIBUTES lpMutexAttribute= NULL);
	CZMutex(LPCTSTR lpName, BOOL bInheritHandle = FALSE, DWORD dwDesiredAccess = MUTEX_ALL_ACCESS);

	BOOL Release();

	DWORD WaitForTwo(HANDLE hSecondHandle,BOOL bWaitAll, DWORD dwWaitMillSecond);

	// Lock
	BOOL	Lock( DWORD dwTimeout = INFINITE );
	// Unlock
	void	Unlock();

	BOOL	IsLock() { return m_bLock; }

public:
	BOOL		m_bLock;
};

/*
class CMutextLock
{

 	CZMutex	    m_cMutex;
	BOOL		m_bLock;
public:
	CMutextLock() 
	{
		m_bLock = FALSE;
	}
	~CMutextLock()
	{
		Unlock();
	}
	// Lock
	BOOL	Lock( DWORD dwTimeout = INFINITE )
	{
		m_bLock = TRUE;;
		return (m_cMutex.Wait(dwTimeout) != WAIT_TIMEOUT );
	}
	// Unlock
	void	Unlock()
	{
		m_cMutex.Release();
		m_bLock = FALSE;
	}

	BOOL	IsLock() { return m_bLock; }
};
*/

class  CMultiRwWLock
{
protected:
	CZMutex 		    m_cmNoWriters;
	CZEvent	        	m_ceNoReader;
	int                 m_iReader;

public:
	CMultiRwWLock():m_ceNoReader(TRUE,TRUE)      // Manual Reset, Signal state initial.
	{
		m_iReader = 0;
	};

	~CMultiRwWLock(){};

	void ReadLock()
	{ 
		m_cmNoWriters.Wait(INFINITE);

		m_iReader++;
		m_ceNoReader.Lock();

		m_cmNoWriters.Release();
	}

    void ReadUnlock()
	{ 
		m_cmNoWriters.Wait(INFINITE);

		if( --m_iReader == 0 ){
			m_ceNoReader.Unlock();
		}
		m_cmNoWriters.Release();
	}

    void WriteLock()
	{ 
		m_cmNoWriters.WaitForTwo(m_ceNoReader.GetHandle(),TRUE,INFINITE);
	}

	void WriteUnlock()
	{ 
		m_cmNoWriters.Release();
	}
};