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
		ASSERT(0);		// 쓰레드가 아직 실행중이다! PostQuitCommand()+WaitThreadExit() 를 호출해 주셈
		PostQuitCommand();
	}

	WaitThreadExit();
	DeleteCriticalSection(&m_cs);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///         쓰레드가 실행중인가?
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
		// 쓰레드핸들이 정말 유효한가 체크한다.
		DWORD ret = WaitForSingleObject(m_hThread, 0);
		if(ret==WAIT_TIMEOUT) return TRUE;
		// 쓰레드는 이미 없다!
		if(ret==WAIT_OBJECT_0)
			::CloseHandle(m_hThread);
		m_hThread = NULL;
		m_dwThreadID = 0;
        return FALSE;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///         쓰레드를 시작한다.
/// @param  
/// @return 
/// @date   Tuesday, May 29, 2012  4:00:14 PM
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL XWinThread::StartThread(HWND hWnd2Notify)
{
    if(ThreadExists()) 
	{ASSERT(0); return FALSE;}

	// 변수 초기화
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
///         워커 쓰레드 루프
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
	// 하위 클래스의 초기화 메쏘드 호출
	if(OnStart()==FALSE) {return 1; }

	while(m_bExit==FALSE)
	{
		if(ProcessCommmand(EVENT_CHECK_TIME)==FALSE)
		{
			// 메시지가 없을경우. 뭔가 백그라운드로 하고 싶은거 있으면 하자.
			OnIdle();
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///         커맨드 하나(혹은 여러개) 처리하기
/// @param  
/// @return 커맨드 처리 여부
/// @date   Friday, June 08, 2012  2:07:28 PM
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL XWinThread::ProcessCommmand(int eventCheckTimeoutMS)
{
	PostCmd cmd;

	// 메시지 기다리기..
	if(m_eventCommand.Wait(eventCheckTimeoutMS)==WAIT_TIMEOUT)
	{
		return FALSE;
	}
	else
	{
		// Post 메시지 있으면 처리하자.
		while(PeekCommand(cmd))
		{
			// 하위 클래스 호출
			this->OnPostCommand(cmd.cmd, cmd.param1, cmd.param2);

			if(m_bExit)
				return TRUE;
		}

		if(m_bExit) return TRUE;

		// send 메시지가 왔다!
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
///         쓰레드 종료를 기다린다.
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
///         쓰레드 강제 종료
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
///         쓰레드에 메시지 send
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
///         쓰레드에 메시지 send + 타임아웃 처리
/// @param  
/// @return 
/// @date   Tuesday, May 29, 2012  3:38:10 PM
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL XWinThread::SendCommandTimeout(UINT cmd, LPVOID param1, LPVOID param2, UINT nTimeOut, LRESULT& result)
{
	if(m_hThread==NULL){ASSERT(0); return FALSE;}	// 워커쓰레드가 없다!

	// 메시지 먼저 세팅하고
	m_sentCommand.cmd = cmd;
	m_sentCommand.param1 = param1;
	m_sentCommand.param2 = param2;
	m_sentCommand.result = 0;

	// 이벤트 세팅하고
	m_eventSendCommand.Set();
	SetEventFlag();
	m_flagSendCmd = TRUE;

	// 답변 기다리기.
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
///         쓰레드에 메시지 post
/// @param  
/// @return 
/// @date   Tuesday, May 29, 2012  3:53:25 PM
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL XWinThread::PostCommand(UINT cmd, LPVOID param1, LPVOID param2)
{
	if(m_hThread==NULL){ASSERT(0); return FALSE;}	// 워커쓰레드가 없다!

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
			// 큐에 추가
			m_postedCommand.AddTail(postMsg);
			ret = TRUE;
			m_flagPostCmd = TRUE;
		}
	}
	Unlock();


	// 연속적으로 Post 할때 메시지가 씹히는걸 막으려면 메시지를 보낸후에 이벤트를 세팅해야 한다..
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
///         종료 메시지 보내기
/// @param  
/// @return 
/// @date   Tuesday, May 29, 2012  9:01:21 PM
////////////////////////////////////////////////////////////////////////////////////////////////////
void XWinThread::PostQuitCommand()
{				 
	SetEventFlag();
	m_bExit = TRUE;
}

// 메시지가 왔음을 이벤트로 워커 쓰레드에 알린다.
void XWinThread::SetEventFlag()
{
	m_eventCommand.Set();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
///         UI 쓰레드에 PostCommand 로 notify 한다.
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

