#pragma once
/////////////////////////////////////////////////////////////////////////////////
//
// FileName    : ZSync.h
// Description : µø±‚»≠ ∞¥√º ∑¶«Œ ≈¨∑°Ω∫ 
// LastUpdate  : 2005.6.11
//
/////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

class  CSLock
{
protected:
	CRITICAL_SECTION		m_csLock;
	BOOL					m_bLock;

public:
	CSLock(){ InitializeCriticalSection(&m_csLock); m_bLock = FALSE;}
	~CSLock(){ Unlock();DeleteCriticalSection(&m_csLock); }

    void		Lock(){ EnterCriticalSection(&m_csLock); m_bLock = TRUE; }
	void		Unlock(){ m_bLock = FALSE; LeaveCriticalSection(&m_csLock); }
	BOOL		IsLock(){ return m_bLock; }
};