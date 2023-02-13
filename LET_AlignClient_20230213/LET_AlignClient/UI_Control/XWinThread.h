////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Worker thread manager class
/// 
/// @author   parkkh
/// @date     Tuesday, May 29, 2012  3:31:54 PM
/// 
/// Copyright(C) 2012 Bandisoft, All rights reserved.
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "XWinThreadUtil.h"
// ATL SUPPORT
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <atlwin.h>
#include <atltypes.h>
#include <atlcoll.h>
#include <atlstr.h>
#include <atltime.h>

class XWinThread
{
public:
	XWinThread(void);
	virtual	~XWinThread(void);

	BOOL			StartThread(HWND hWnd2Notify);					// 쓰레드 시작하기

public :
	void			PostQuitCommand();								// 쓰레드에 종료 요청하기
	BOOL			WaitThreadExit(DWORD timeout = INFINITE);		// 쓰레드가 종료할때까지 기다리기
	BOOL			ThreadExists(void);								// 쓰레드가 실행중인가?
	DWORD			GetThreadID() { return m_dwThreadID; }		
	HANDLE			GetThreadHandle() { return m_hThread; }
	BOOL			TerminateThread();								// 강제 종료

public :			// 워커쓰레드와 UI쓰레드가 동시에 접근할 객체가 있을때 사용하라고 만든 CS
	void			Lock();
	void			Unlock();

public :			// UI쓰레드에서 워커 쓰레드에 메시지 보낼때 사용
	LRESULT			SendCommand(UINT cmd, LPVOID param1=0, LPVOID param2=0);
	BOOL			SendCommandTimeout(UINT nMsg, LPVOID param1, LPVOID param2, UINT nTimeOut, LRESULT& result);
	BOOL			PostCommand(UINT cmd, LPVOID param1=0, LPVOID param2=0);

protected :			// 하위 클래스에서 상속받아서 구현해야 하는 메쏘드들
	virtual BOOL    OnStart() { return TRUE; }											// 쓰레드 시작됨. 여기서 초기화 코드를 호출하면됨
	virtual void    OnPostCommand(UINT cmd, LPVOID param1, LPVOID param2) {}			// Post 메시지 처리
	virtual LRESULT OnSendCommand(UINT cmd, LPVOID param1, LPVOID param2) {return 0;}	// Send 메시지 처리
	virtual void	OnIdle() {}															// 아이들 타임.

protected :			// 상속받은 클래스(워커 쓰레드)에서 호출하는 함수들
	BOOL			Notify(UINT cmd, WPARAM wp=0, LPARAM lp=0);							// UI 쓰레드에 NOTIFY 할때 사용
	BOOL			IsExit() { return m_bExit; }										// 종료메시지가 왔는감?
	BOOL			IsCommandQueued() { return m_flagSendCmd||m_flagPostCmd;}			// 다른 메시지가 도착했는감?
	BOOL			ProcessCommmand(int eventCheckTimeoutMS);
	void			SetExitFlag(BOOL b) { m_bExit = b; }

private :
	static unsigned int WINAPI InitialThreadProc(LPVOID pv);
	unsigned int WINAPI _InitialThreadProc();

private :
    // make copy constructor and assignment operator inaccessible
    XWinThread(const XWinThread &refThread);
    XWinThread &operator=(const XWinThread &refThread);

private :			
	BOOL			m_bExit;				// 이제 쓰레드를 종료해야 한다!
	DWORD			m_dwThreadID;
    HANDLE			m_hThread;
	HWND			m_hWnd2Notify;			// ui 쓰레드 notify 받을 윈도우 핸들
	CRITICAL_SECTION m_cs;					// Lock/Unlock 용

	struct SendCmd
	{
		UINT	cmd;
		LPVOID	param1;
		LPVOID	param2;
		LRESULT	result;
	};

	struct PostCmd
	{
		UINT	cmd;
		LPVOID	param1;
		LPVOID	param2;
	};

	enum { MAX_POST_COMMAND = 4000 };		// 최대 PostCommand 큐 크기
	enum { EVENT_CHECK_TIME = 100 };		// 이벤트 대기시간

private :
	BOOL			PeekCommand(PostCmd& cmd);
	void			SetEventFlag();

private :			
	XEvent			m_eventCommand;			// POST 나 Send 메시지가 큐에 있나?
	XEvent			m_eventSendCommand;		// SEND 메시지가 큐에 있나?
	XEvent			m_eventSendCommandReply;// SEND 메시지에 대한 답변이 왔나?
	SendCmd			m_sentCommand;
	CAtlList<PostCmd>	m_postedCommand;	// 포스팅된 커맨드들..	접근할때는 Lock()/Unlock()을 꼭 하자.
	BOOL			m_flagSendCmd;			// SendCommand 가 호출되었나? - 메인 루프가 아닌, 하위 클래스에서 혹시 메시지가 왔는지 확인할때 사용. 가끔은 삐꾸가 날수도 있지만 단순 체크용이기 때문에 별 상관없음
	int				m_flagPostCmd;			// PostCommand 가 호출되었나?

};

