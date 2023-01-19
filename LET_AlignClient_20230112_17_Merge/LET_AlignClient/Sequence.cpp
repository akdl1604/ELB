#include "stdafx.h"
#include "Sequence.h"

// class CSeqMotor : public CSequence, public CMmc
// CSeqMotor ������ ���� ȣ�� ��.
// 2018.6.7 CSequence Class�� �����Լ��� �ٲ�.

clock_t g_tmSeq[MAX_SEQ_TIMER];
bool g_tmSeqFlag[MAX_SEQ_TIMER];
// tm : milisecond
void seq_SetTimer(int no, int tm)
{
	if( no<0 || no>MAX_SEQ_TIMER ) return;
	
	g_tmSeq[no] = clock() + clock_t(tm);	
	g_tmSeqFlag[no] = true;
}

// �ӽ÷� �ƹ������� �� Ÿ�̸�, �ߺ����� �ҷ��� �ȵ�.
void seq_SetTimer(int tm)
{
	seq_SetTimer(0, tm);
}

void seq_SetTimer(struct ST_SEQ *stSeq, int tm)
{
	if (stSeq->nSeqTimer < 0 || stSeq->nSeqTimer > MAX_SEQ_TIMER) return;

	g_tmSeq[stSeq->nSeqTimer] = clock() + clock_t(tm);
	g_tmSeqFlag[stSeq->nSeqTimer] = true;
}

bool seq_ChkTimer(int no)
{
	if( no < 0 || no > MAX_SEQ_TIMER || g_tmSeqFlag[no]==false) return false;

	clock_t tm = clock();
	if (tm >= g_tmSeq[no])
	{
		g_tmSeqFlag[no] = false;
		return true;
	}
	return false;
}

bool seq_ResetChkTimer(int no)
{
	if (no < 0 || no > MAX_SEQ_TIMER ) return false;

	g_tmSeqFlag[no] = false;
	return true;
}

bool seq_ChkTimer(struct ST_SEQ *stSeq)
{
	if (stSeq->nSeqTimer < 0 || stSeq->nSeqTimer > MAX_SEQ_TIMER || g_tmSeqFlag[stSeq->nSeqTimer] == false) return false;

	clock_t tm = clock();
	if (tm >= g_tmSeq[stSeq->nSeqTimer])
	{
		g_tmSeqFlag[stSeq->nSeqTimer] = false;
		return true;
	}
	return false;
}


// ----------------------------------------------------------
// sequence parameter manipulation
// ----------------------------------------------------------
void seq_Init(struct ST_SEQ *stSeq, int tm)
{
	stSeq->bStart = false;
	stSeq->bFinished = false;
	stSeq->bRunning = false;
	stSeq->bStop = false;
	stSeq->bCycleStop = false;
	stSeq->nSeqNo = 0;
	stSeq->nSeqTimer = tm;
	for (int i = 0; i<MAX_ALARM_ACTION; i++)
	{
		stSeq->nSeqNoAlarmAction[i] = 0;
	}
}

void seq_Start(struct ST_SEQ *stSeq)
{
	stSeq->bStart = true;
	stSeq->bFinished = false;
	stSeq->bRunning = true;
	stSeq->bStop = false;
	stSeq->bCycleStop = false;
	stSeq->nSeqNo = 0;
	for (int i = 0; i < MAX_ALARM_ACTION; i++)
	{
		stSeq->nSeqNoAlarmAction[i] = 0;
	}
}

void seq_Stop(struct ST_SEQ *stSeq)
{
	stSeq->bStop = true;
}

void seq_CycleStop(struct ST_SEQ *stSeq)
{
	stSeq->bCycleStop = true;
}

bool seq_IsCycleStop(struct ST_SEQ *stSeq)
{
	return stSeq->bCycleStop;
}

void seq_Finish(struct ST_SEQ *stSeq)
{
	stSeq->bStart = false;
	stSeq->bRunning = false;
	stSeq->bFinished = true;
}

void seq_SetNo(struct ST_SEQ *stSeq, int no)
{
	stSeq->nSeqNo = no;
}

// Alarm �߻� �� ���� �Է¿� ���� ���� ������ �����ش�.
void seq_SetAlarmActionNo(struct ST_SEQ *stSeq, int AlarmAction, int ActionSeqNo)
{
	if (AlarmAction < 0 || AlarmAction >= MAX_ALARM_ACTION) return;
	stSeq->nSeqNoAlarmAction[AlarmAction] = ActionSeqNo;
}

// ������ ������ Alarm ������ �´� Seq. No�� ����, �׷��� ������ ���� step�� �ӹ���
void seq_GotoAlarmActionStep(struct ST_SEQ *stSeq, int AlarmAction, int curStep)
{
	if (0 <= AlarmAction && MAX_ALARM_ACTION > AlarmAction)
	{
		stSeq->nSeqNo = stSeq->nSeqNoAlarmAction[AlarmAction];
	}
	else
	{
		stSeq->nSeqNo = curStep;
	}
}

// �Ʒ� �Լ��� �Ű������� �����Ͱ� �ƴ�
bool seq_IsRunning(struct ST_SEQ stSeq)
{
	return stSeq.bRunning;
}

bool seq_IsFinished(struct ST_SEQ stSeq)
{
	return stSeq.bFinished;
}


// ------------------------------------------------------------------------------------------------
HANDLE launchthread(LPTHREAD_START_ROUTINE fn, LPVOID fn_param, DWORD priority, BOOL bRun)
{
	DWORD id = 0;
	HANDLE thread = ::CreateThread(NULL, 1000, fn, fn_param, CREATE_SUSPENDED, &id);

	if (!thread) 	return NULL;

	::SetThreadPriority(thread, priority);
	if (bRun)	::ResumeThread(thread);

	return thread;
}

void ThreadRun(HANDLE hThread, BOOL bRun)
{
	if (hThread == NULL) return;

	if (bRun) ::ResumeThread(hThread);
	else ::SuspendThread(hThread);
}

