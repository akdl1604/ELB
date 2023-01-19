#include "StdAfx.h"
#include "XWinThread.h"

#define XWINTHREAD_CLASS	_T("XWinThreadClass")
#define PROP_VAL			_T("XWinThread")


XWinThread::XWinThread(void)
{
	m_hThread = NULL;
	m_dwThreadID = 0;
	m_hWnd2Notify = NULL;
	m_bExit = FALSE;
	m_flagSendCmd = FALSE;
	InitializeCriticalSection(&m_cs);
}

XWinThread::~XWinThread(void)
{
	//Debug(L"~XWinThread");
	if(m_hThread)
	{
		ASSERT(0);		// �����尡 ���� �������̴�! PostQuitCommand()+WaitThreadExit() �� ȣ���� �ּ�
		PostQuitCommand();
	}

	WaitThreadExit();
	DeleteCriticalSection(&m_cs);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///         �����尡 �������ΰ�?
/// @param  
/// @return 
/// @date   Tuesday, May 29, 2012  4:00:06 PM
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL XWinThread::ThreadExists(void)
{
    if (m_hThread == 0) 
        return FALSE;
    else
	{
		// �������ڵ��� ���� ��ȿ�Ѱ� üũ�Ѵ�.
		DWORD ret = WaitForSingleObject(m_hThread, 0);
		if(ret==WAIT_TIMEOUT) return TRUE;
		// ������� �̹� ����!
		if(ret==WAIT_OBJECT_0)
			::CloseHandle(m_hThread);
		m_hThread = NULL;
		m_dwThreadID = 0;
        return FALSE;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///         �����带 �����Ѵ�.
/// @param  
/// @return 
/// @date   Tuesday, May 29, 2012  4:00:14 PM
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL XWinThread::StartThread(HWND hWnd2Notify)
{
    if(ThreadExists()) 
	{ASSERT(0); return FALSE;}

	// ���� �ʱ�ȭ
	m_hWnd2Notify = hWnd2Notify;
	m_bExit = FALSE;
	m_eventCommand.Reset();
	m_eventSendCommand.Reset();
	m_flagSendCmd = FALSE;
	m_hThread = NULL;
	m_dwThreadID = 0;
	m_postedCommand.RemoveAll();
	m_flagPostCmd = 0;

	m_hThread = (HANDLE)_beginthreadex(NULL, 0, XWinThread::InitialThreadProc, this, 0, (unsigned int*)&m_dwThreadID);
    if(!m_hThread) {ASSERT(0);return FALSE;}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///         ��Ŀ ������ ����
/// @param  
/// @return 
/// @date   Tuesday, May 29, 2012  9:09:23 PM
////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int  WINAPI XWinThread::InitialThreadProc(LPVOID pv)
{
	CoInitialize(NULL);
	XWinThread * pThread = (XWinThread *) pv;
	unsigned int ret = pThread->_InitialThreadProc();
	CoUninitialize();
    return ret;
}

unsigned int  WINAPI XWinThread::_InitialThreadProc()
{
	// ���� Ŭ������ �ʱ�ȭ �޽�� ȣ��
	if(OnStart()==FALSE) {return 1; }

	while(m_bExit==FALSE)
	{
		if(ProcessCommmand(EVENT_CHECK_TIME)==FALSE)
		{
			// �޽����� �������. ���� ��׶���� �ϰ� ������ ������ ����.
			OnIdle();
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///         Ŀ�ǵ� �ϳ�(Ȥ�� ������) ó���ϱ�
/// @param  
/// @return Ŀ�ǵ� ó�� ����
/// @date   Friday, June 08, 2012  2:07:28 PM
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL XWinThread::ProcessCommmand(int eventCheckTimeoutMS)
{
	PostCmd cmd;

	// �޽��� ��ٸ���..
	if(m_eventCommand.Wait(eventCheckTimeoutMS)==WAIT_TIMEOUT)
	{
		return FALSE;
	}
	else
	{
		// Post �޽��� ������ ó������.
		while(PeekCommand(cmd))
		{
			// ���� Ŭ���� ȣ��
			this->OnPostCommand(cmd.cmd, cmd.param1, cmd.param2);

			if(m_bExit)
				return TRUE;
		}

		if(m_bExit) return TRUE;

		// send �޽����� �Դ�!
		if(m_eventSendCommand.Check())
		{
			m_flagSendCmd = FALSE;
			m_sentCommand.result = this->OnSendCommand(m_sentCommand.cmd, m_sentCommand.param1, m_sentCommand.param2);
			m_eventSendCommandReply.Set();
		}
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///         ������ ���Ḧ ��ٸ���.
/// @param  
/// @return 
/// @date   Tuesday, May 29, 2012  4:00:25 PM
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL XWinThread::WaitThreadExit(DWORD timeout) 
{
    HANDLE hThread = (HANDLE)InterlockedExchangePointer(&m_hThread, 0);
    if (hThread) 
	{
		DWORD ret = WaitForSingleObject(hThread, timeout);
        if(ret==WAIT_TIMEOUT)
		{
			m_hThread = hThread;
			return FALSE;
		}
		if(ret==WAIT_OBJECT_0)
			CloseHandle(hThread);	
    }
	m_dwThreadID = 0;
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///         ������ ���� ����
/// @param  
/// @return 
/// @date   Tuesday, May 29, 2012  4:00:36 PM
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL XWinThread::TerminateThread()
{
	HANDLE hThread = (HANDLE)InterlockedExchangePointer(&m_hThread, 0);
	return ::TerminateThread(hThread, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///         �����忡 �޽��� send
/// @param  
/// @return 
/// @date   Tuesday, May 29, 2012  3:38:05 PM
////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT XWinThread::SendCommand(UINT cmd, LPVOID param1, LPVOID param2)
{
	LRESULT ret=0;
	SendCommandTimeout(cmd, param1, param2, INFINITE, ret);
	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///         �����忡 �޽��� send + Ÿ�Ӿƿ� ó��
/// @param  
/// @return 
/// @date   Tuesday, May 29, 2012  3:38:10 PM
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL XWinThread::SendCommandTimeout(UINT cmd, LPVOID param1, LPVOID param2, UINT nTimeOut, LRESULT& result)
{
	if(m_hThread==NULL){ASSERT(0); return FALSE;}	// ��Ŀ�����尡 ����!

	// �޽��� ���� �����ϰ�
	m_sentCommand.cmd = cmd;
	m_sentCommand.param1 = param1;
	m_sentCommand.param2 = param2;
	m_sentCommand.result = 0;

	// �̺�Ʈ �����ϰ�
	m_eventSendCommand.Set();
	SetEventFlag();
	m_flagSendCmd = TRUE;

	// �亯 ��ٸ���.
	if(m_eventSendCommandReply.Wait(nTimeOut)==WAIT_OBJECT_0)
	{
		m_flagSendCmd = FALSE;
		result = m_sentCommand.result;
		return TRUE;
	}
	m_flagSendCmd = FALSE;

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///         �����忡 �޽��� post
/// @param  
/// @return 
/// @date   Tuesday, May 29, 2012  3:53:25 PM
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL XWinThread::PostCommand(UINT cmd, LPVOID param1, LPVOID param2)
{
	if(m_hThread==NULL){ASSERT(0); return FALSE;}	// ��Ŀ�����尡 ����!

	PostCmd postMsg;
	BOOL ret;

	postMsg.cmd = cmd;
	postMsg.param1 = param1;
	postMsg.param2 = param2;

	Lock();
	{
		if(m_postedCommand.GetCount()>MAX_POST_COMMAND)
			ret = FALSE;
		else
		{
			// ť�� �߰�
			m_postedCommand.AddTail(postMsg);
			ret = TRUE;
			m_flagPostCmd = TRUE;
		}
	}
	Unlock();


	// ���������� Post �Ҷ� �޽����� �����°� �������� �޽����� �����Ŀ� �̺�Ʈ�� �����ؾ� �Ѵ�..
	if(ret)
		SetEventFlag();

	ASSERT(ret);
	return ret;
}

BOOL XWinThread::PeekCommand(PostCmd& cmd)
{
	BOOL ret;

	Lock();
	{
		if(m_postedCommand.GetCount())
		{
			cmd = m_postedCommand.GetHead();
			m_postedCommand.RemoveHead();
			ret = TRUE;
		}
		else
			ret = FALSE;

		if(m_postedCommand.GetCount()==0)
			m_flagPostCmd = FALSE;
	}
	Unlock();

	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///         ���� �޽��� ������
/// @param  
/// @return 
/// @date   Tuesday, May 29, 2012  9:01:21 PM
////////////////////////////////////////////////////////////////////////////////////////////////////
void XWinThread::PostQuitCommand()
{				 
	SetEventFlag();
	m_bExit = TRUE;
}

// �޽����� ������ �̺�Ʈ�� ��Ŀ �����忡 �˸���.
void XWinThread::SetEventFlag()
{
	m_eventCommand.Set();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
///         UI �����忡 PostCommand �� notify �Ѵ�.
/// @param  
/// @return 
/// @date   Tuesday, May 29, 2012  9:52:14 PM
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL XWinThread::Notify(UINT msg, WPARAM wp, LPARAM lp)
{
	return ::PostMessage(m_hWnd2Notify, msg, wp, lp);
}

void XWinThread::Lock()
{
	EnterCriticalSection(&m_cs);
}

void XWinThread::Unlock()
{
	LeaveCriticalSection(&m_cs);
}

