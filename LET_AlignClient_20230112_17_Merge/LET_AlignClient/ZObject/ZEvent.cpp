// ZEvent.cpp: implementation of the CZEvent class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZEvent.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CZEvent::CZEvent(BOOL bMenualReset, BOOL bInitialState, LPCTSTR lpName, LPSECURITY_ATTRIBUTES lpEventAttributes )
{
	m_hHandle = ::CreateEvent(lpEventAttributes,bMenualReset,bInitialState,lpName);

	if( m_hHandle == NULL )
	{
		m_dwStatus = GetLastError();
	}
	else
	{
		m_dwStatus = XK_NOERROR;
	}
	m_bLock = !bInitialState;
}

CZEvent::CZEvent(LPCTSTR lpName, BOOL bInheritHandle , DWORD dwDesiredAccess)
{
	m_hHandle = ::OpenEvent(dwDesiredAccess,bInheritHandle,lpName);

	if( m_hHandle == NULL)
	{
		m_dwStatus = GetLastError();
	}
	else
	{
		m_dwStatus = XK_NOERROR;
	}
}

BOOL CZEvent::Set()
{
	return ::SetEvent(m_hHandle);
}

BOOL CZEvent::Reset()
{
	return ::ResetEvent(m_hHandle);
}

BOOL CZEvent::Pulse()
{
	return ::PulseEvent(m_hHandle);
}

void	CZEvent::Lock()
{
    Reset();
	m_bLock = TRUE;
}

void	CZEvent::Unlock()
{
	Set();
	m_bLock = FALSE;
}

BOOL	CZEvent::IsLock()
{
	return m_bLock;
}

//////////////////////////////////////////////////////////////////////////////////////
CZSemaphore::CZSemaphore(int iInitialCount, int iMaxCount, LPCTSTR lpName, LPSECURITY_ATTRIBUTES lpSemaphoreAttributes)
{
	m_hHandle = ::CreateSemaphore(lpSemaphoreAttributes,iInitialCount,iMaxCount,lpName);

	if( m_hHandle == NULL )
	{
		m_dwStatus = GetLastError();
	}
	else
	{
		m_dwStatus = XK_NOERROR;
	}
}

CZSemaphore::CZSemaphore(LPCTSTR lpName ,BOOL bInheritHandle, DWORD dwDesiredAccess)
{
	m_hHandle = ::OpenSemaphore(dwDesiredAccess,bInheritHandle,lpName);
	
	if( m_hHandle == NULL )
	{
		m_dwStatus = GetLastError();
	}
	else
	{
		m_dwStatus = XK_NOERROR;
	}
}

BOOL CZSemaphore::Release(LONG lReleaseCount, LONG *plPreviousCount)
{
	LONG lPreviousCount;

	BOOL bStatus = ::ReleaseSemaphore(m_hHandle,lReleaseCount,&lPreviousCount);

	if(bStatus&& plPreviousCount)
	{
		*plPreviousCount = lPreviousCount;
	}
	return bStatus;
}

////////////////////////////////////////////////////////////////////////////////////////////////
CZMutex::CZMutex(BOOL bInitialOwner, LPCTSTR lpName , LPSECURITY_ATTRIBUTES lpMutexAttribute)
{
	m_hHandle = ::CreateMutex(lpMutexAttribute,bInitialOwner,lpName);
	
	if( m_hHandle == NULL )
	{
		m_dwStatus = GetLastError();
	}
	else
	{
		m_dwStatus = XK_NOERROR;
	}
	m_bLock = FALSE;
}

CZMutex::CZMutex(LPCTSTR lpName, BOOL bInheritHandle , DWORD dwDesiredAccess )
{
	m_hHandle = ::OpenMutex(dwDesiredAccess,bInheritHandle,lpName);

	if( m_hHandle == NULL )
	{
		m_dwStatus = GetLastError();
	}
	else
	{
		m_dwStatus = XK_NOERROR;
	}
}

BOOL CZMutex::Release()
{
	return ::ReleaseMutex(m_hHandle);
}

BOOL CZMutex::Lock( DWORD dwTimeout)
{
	m_bLock = TRUE;;
	return (Wait(dwTimeout) != WAIT_TIMEOUT );
}

// Unlock
void CZMutex::Unlock()
{
	Release();
	m_bLock = FALSE;
}

DWORD CZMutex::WaitForTwo(HANDLE hSecondHandle,BOOL bWaitAll, DWORD dwWaitMillSecond)
{
	HANDLE handles[2];
	handles[0] = m_hHandle;
	handles[1] = hSecondHandle;

	return WaitForMultipleObjects(2,handles,bWaitAll,dwWaitMillSecond);
}