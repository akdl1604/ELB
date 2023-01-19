// ZKernelObject.cpp: implementation of the CZKernelObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZKernelObject.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CZKernelObject::CZKernelObject()
{
	m_hHandle  = NULL;
	m_dwStatus = XK_NOERROR;
}

CZKernelObject::~CZKernelObject()
{
	Close();
}

void CZKernelObject::ThrowError(DWORD dwStatus)
{
}

DWORD CZKernelObject::Status() const
{
	return m_dwStatus;
}

DWORD CZKernelObject::Wait(DWORD dwMilliseconds)
{
	return ::WaitForSingleObject(m_hHandle,dwMilliseconds);
}

HANDLE CZKernelObject::GetHandle()const
{
	if( this != NULL )
		return m_hHandle;
	else
		return NULL;
}

CZKernelObject::operator HANDLE() const
{
	return GetHandle();
}

void CZKernelObject::Close()
{
	if( m_hHandle != NULL )
	{
		::CloseHandle(m_hHandle);
		m_hHandle = NULL;
	}
}