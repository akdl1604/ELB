#pragma once

#define MAX_SEQ_TIMER 50
#define STM_MAIN	1
#define STM_PN_ALIGN	2
#define STM_SS_ALIGN	3
#define STM_INSP		4
#define STM_ME			5
#define STM_LMI			6
#define STM_PANEL_CAL	7
#define STM_SENSOR_CAL	8
#define STM_TRIGGER		9
#define STM_FLOWCONTROL	10

// --------------------------------------------------------------------------------------
//     Sequence class
// --------------------------------------------------------------------------------------
enum _ALARM_ACTION
{
	ALARM_ACTION_RETRY,
	ALARM_ACTION_IGNORE,
	ALARM_ACTION_QUIT,
	MAX_ALARM_ACTION
};

struct ST_SEQ
{
	bool bStart;	// let know start sequence
	bool bFinished;		// let know the sequence is finishied
	bool bStop;		// let know stop the sequence
	bool bCycleStop;	// do not start again
	bool bRunning;	// sequence is running
	int nSeqNo;		// sequence no
	int nSeqNoAlarmAction[MAX_ALARM_ACTION];	// where to go after alarm restoring, retry
	int nSeqTimer;
};

void seq_SetTimer(int no, int tm);
void seq_SetTimer(struct ST_SEQ *stSeq, int tm);
void seq_SetTimer(int tm);
bool seq_ChkTimer(int no=0);
bool seq_ResetChkTimer(int no = 0);
bool seq_ChkTimer(struct ST_SEQ *stSeq);

void seq_Init(struct ST_SEQ *stSeq, int tm);
void seq_Start(struct ST_SEQ *stSeq);
void seq_Stop(struct ST_SEQ *stSeq);
void seq_CycleStop(struct ST_SEQ *stSeq);
void seq_Finish(struct ST_SEQ *stSeq);
void seq_SetNo(struct ST_SEQ *stSeq, int no);
bool seq_IsCycleStop(struct ST_SEQ *stSeq);
// 2018.08.06
void seq_SetAlarmActionNo(struct ST_SEQ *stSeq, int AlarmAction, int ActionSeqNo);
void seq_GotoAlarmActionStep(struct ST_SEQ *stSeq, int AlarmAction, int curStep);

bool seq_IsRunning(struct ST_SEQ stSeq);
bool seq_IsFinished(struct ST_SEQ stSeq);

// ---------------------------------------------------------------------------------------------------------------------------
HANDLE launchthread(LPTHREAD_START_ROUTINE fn, LPVOID fn_param, DWORD priority = THREAD_PRIORITY_NORMAL, BOOL bRun = FALSE);
void ThreadRun(HANDLE hThread, BOOL bRun);

