#pragma once
//////////////////////////////////////////////////////////////////////
// ZKernelObject.h: interface for the CZKernelObject class.
//
//////////////////////////////////////////////////////////////////////

#define XK_NOERROR  1

class CZKernelObject  
{
public:
	virtual ~CZKernelObject();

protected:
	CZKernelObject();

protected:
	HANDLE m_hHandle;
	DWORD  m_dwStatus;

	void   ThrowError(DWORD dwStatus);

public:
	 DWORD Status(void) const;
	 DWORD Wait(DWORD dwMilliseconds);

	 void  Close();

	 HANDLE GetHandle(void)const;
     operator HANDLE() const;
};
