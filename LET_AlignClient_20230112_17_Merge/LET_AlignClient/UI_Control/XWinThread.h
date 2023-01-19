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

	BOOL			StartThread(HWND hWnd2Notify);					// ������ �����ϱ�

public :
	void			PostQuitCommand();								// �����忡 ���� ��û�ϱ�
	BOOL			WaitThreadExit(DWORD timeout = INFINITE);		// �����尡 �����Ҷ����� ��ٸ���
	BOOL			ThreadExists(void);								// �����尡 �������ΰ�?
	DWORD			GetThreadID() { return m_dwThreadID; }		
	HANDLE			GetThreadHandle() { return m_hThread; }
	BOOL			TerminateThread();								// ���� ����

public :			// ��Ŀ������� UI�����尡 ���ÿ� ������ ��ü�� ������ ����϶�� ���� CS
	void			Lock();
	void			Unlock();

public :			// UI�����忡�� ��Ŀ �����忡 �޽��� ������ ���
	LRESULT			SendCommand(UINT cmd, LPVOID param1=0, LPVOID param2=0);
	BOOL			SendCommandTimeout(UINT nMsg, LPVOID param1, LPVOID param2, UINT nTimeOut, LRESULT& result);
	BOOL			PostCommand(UINT cmd, LPVOID param1=0, LPVOID param2=0);

protected :			// ���� Ŭ�������� ��ӹ޾Ƽ� �����ؾ� �ϴ� �޽���
	virtual BOOL    OnStart() { return TRUE; }											// ������ ���۵�. ���⼭ �ʱ�ȭ �ڵ带 ȣ���ϸ��
	virtual void    OnPostCommand(UINT cmd, LPVOID param1, LPVOID param2) {}			// Post �޽��� ó��
	virtual LRESULT OnSendCommand(UINT cmd, LPVOID param1, LPVOID param2) {return 0;}	// Send �޽��� ó��
	virtual void	OnIdle() {}															// ���̵� Ÿ��.

protected :			// ��ӹ��� Ŭ����(��Ŀ ������)���� ȣ���ϴ� �Լ���
	BOOL			Notify(UINT cmd, WPARAM wp=0, LPARAM lp=0);							// UI �����忡 NOTIFY �Ҷ� ���
	BOOL			IsExit() { return m_bExit; }										// ����޽����� �Դ°�?
	BOOL			IsCommandQueued() { return m_flagSendCmd||m_flagPostCmd;}			// �ٸ� �޽����� �����ߴ°�?
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
	BOOL			m_bExit;				// ���� �����带 �����ؾ� �Ѵ�!
	DWORD			m_dwThreadID;
    HANDLE			m_hThread;
	HWND			m_hWnd2Notify;			// ui ������ notify ���� ������ �ڵ�
	CRITICAL_SECTION m_cs;					// Lock/Unlock ��

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

	enum { MAX_POST_COMMAND = 4000 };		// �ִ� PostCommand ť ũ��
	enum { EVENT_CHECK_TIME = 100 };		// �̺�Ʈ ���ð�

private :
	BOOL			PeekCommand(PostCmd& cmd);
	void			SetEventFlag();

private :			
	XEvent			m_eventCommand;			// POST �� Send �޽����� ť�� �ֳ�?
	XEvent			m_eventSendCommand;		// SEND �޽����� ť�� �ֳ�?
	XEvent			m_eventSendCommandReply;// SEND �޽����� ���� �亯�� �Գ�?
	SendCmd			m_sentCommand;
	CAtlList<PostCmd>	m_postedCommand;	// �����õ� Ŀ�ǵ��..	�����Ҷ��� Lock()/Unlock()�� �� ����.
	BOOL			m_flagSendCmd;			// SendCommand �� ȣ��Ǿ���? - ���� ������ �ƴ�, ���� Ŭ�������� Ȥ�� �޽����� �Դ��� Ȯ���Ҷ� ���. ������ �ٰ߲� ������ ������ �ܼ� üũ���̱� ������ �� �������
	int				m_flagPostCmd;			// PostCommand �� ȣ��Ǿ���?

};

