#pragma once
//////////////////////////////////////////////////////////////////////
//
// ZQueue.h: interface for the CZQueue class.
//
//////////////////////////////////////////////////////////////////////

#include "ZSync.h"


template <typename tmpQueuetype>
class CZQueue
{
public:
	CZQueue()
	{
		Clean();
	};
	virtual ~CZQueue()
	{
		Destroy();
	};
	
	
	bool IsEmpty() { if (m_iHead == m_iTail) return true; else return false; }

	DWORD GetCount(){return m_dwCount;};
	
	DWORD GetItemCount(){return m_dwItemCount;};
	int GetEmptyCount(){return m_dwCount - m_dwItemCount-1;};
	int GetHeadIndex(){ return m_iHead;};
	
	
	BOOL Initialize(DWORD dwCount)
	{
		Destroy();

		m_lpHCmdList = new tmpQueuetype[dwCount];

		_ASSERT(m_lpHCmdList);

		if (m_lpHCmdList == NULL)
			return FALSE;

		m_dwCount = dwCount;

		memset(m_lpHCmdList, 0x00, sizeof(tmpQueuetype)*dwCount);

		return TRUE;
	};

	void Clean()
	{
		m_lpHCmdList = NULL;
		m_iHead = 0;
		m_iTail = 0;
		m_dwCount = 0;
		m_dwItemCount = 0;		
	};
	
	tmpQueuetype* Pull()
	{
		tmpQueuetype* pData = NULL;

		if (m_iHead == m_iTail)
			return NULL;

		m_Lock.Lock();

		pData = &(m_lpHCmdList[m_iHead]);
		m_iHead = ++m_iHead % m_dwCount;

		--m_dwItemCount;

		m_Lock.Unlock();

		return pData;
	};

	BOOL Push(tmpQueuetype Data_)
	{
		_ASSERT(m_dwCount);

		if ((m_iTail + 1) % (int)m_dwCount == m_iHead)
			return FALSE;

		m_Lock.Lock();

		memcpy(&m_lpHCmdList[m_iTail], &Data_, sizeof(tmpQueuetype));

		m_iTail = ++m_iTail % m_dwCount;
		++m_dwItemCount;

		m_Lock.Unlock();

		return TRUE;
	};
	
	virtual void Destroy()
	{
		while (m_iHead != m_iTail)
		{
			memset(&m_lpHCmdList[m_iHead], NULL, sizeof(tmpQueuetype));
			m_iHead = ++m_iHead % m_dwCount;
		}

		if (m_lpHCmdList)
		{
			delete[] m_lpHCmdList;
		}
		Clean();
	};
	
protected:
	
	DWORD m_dwCount;
	DWORD m_dwItemCount;
	int m_iHead;
	int m_iTail;


	tmpQueuetype*       m_lpHCmdList;

	CSLock       m_Lock;
	
};