////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// thread util classes
/// 
/// @author   parkkh
/// @date     Friday, June 08, 2012  1:57:05 PM
/// 
/// Copyright(C) 2012 Bandisoft, All rights reserved.
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

class XEvent
{
	// make copy constructor and assignment operator inaccessible
	XEvent(const XEvent &refEvent);
	XEvent &operator=(const XEvent &refEvent);
protected:
	HANDLE m_hEvent;
public:
	XEvent(BOOL fManualReset = FALSE) {m_hEvent = CreateEvent(NULL, fManualReset, FALSE, NULL);}
	~XEvent() {if(m_hEvent) CloseHandle(m_hEvent);}
	operator HANDLE () const { return m_hEvent; };
	void Set() {SetEvent(m_hEvent);};
	DWORD Wait(DWORD dwTimeout = INFINITE) {return WaitForSingleObject(m_hEvent, dwTimeout);}
	void Reset() { ResetEvent(m_hEvent); };
	BOOL Check() { return (Wait(0) == WAIT_OBJECT_0); };
};

class XCritSec 
{
    // make copy constructor and assignment operator inaccessible
    XCritSec(const XCritSec &refCritSec);
    XCritSec &operator=(const XCritSec &refCritSec);
    CRITICAL_SECTION m_CritSec;
public:
    XCritSec(){	InitializeCriticalSection(&m_CritSec);}
    ~XCritSec(){ DeleteCriticalSection(&m_CritSec);}
    void Lock() {EnterCriticalSection(&m_CritSec);}
    void Unlock(){LeaveCriticalSection(&m_CritSec);}
};



