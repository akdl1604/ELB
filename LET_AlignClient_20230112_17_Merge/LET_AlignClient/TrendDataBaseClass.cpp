#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "TrendDataBaseClass.h"

CTrendDataBaseClass::CTrendDataBaseClass()
{
	m_ModelFolderName = "";
	m_DBFileName = "";
	InitializeCriticalSection(&m_csTrend);
}

CTrendDataBaseClass::~CTrendDataBaseClass()
{
	DeleteCriticalSection(&m_csTrend);
}

bool CTrendDataBaseClass::InitDataBase(CString dbFolder)
{
	m_ModelFolderName.Format("%sDataBase\\TrendData\\",dbFolder);
	if (_access(m_ModelFolderName, 0) != 0)	_mkdir(m_ModelFolderName);

	return true;
}

bool CTrendDataBaseClass::CreateDataBase()
{
	try {
		CFileStatus fs;
		CTime time = CTime::GetCurrentTime();
		CString queryStr;
		queryStr.Format("%ssummary_%d_%d_%d.db3", m_ModelFolderName,time.GetYear(), time.GetMonth(), time.GetDay());

		m_DBFileName = queryStr;
		if (CFile::GetStatus(queryStr, fs))			return true;	

		SQLite::Database    m_Db(queryStr, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);

		queryStr.Format(" CREATE TABLE IF NOT EXISTS Trend_Data_Table(id_num INTEGER PRIMARY KEY, year INTEGER, month INTEGER,\
			day INTEGER, hour INTEGER, job INTEGER,ok TEXT, ng TEXT, xx DOUBLE, yy DOUBLE, tt DOUBLE, xx1 DOUBLE, yy1 DOUBLE, tt1,\
            xx2 DOUBLE, yy2 DOUBLE, tt2 DOUBLE DOUBLE,R1 DOUBLE, R2 DOUBLE,\
            R3 DOUBLE, R4 DOUBLE,R5 DOUBLE, R6 DOUBLE)"); // , 
		
		m_Db.exec(queryStr);
	}
	catch (std::exception& e)
	{
		CString strError = e.what();
		AfxMessageBox(strError);
	}

	return true;
}

bool CTrendDataBaseClass::IsExistDataBase()
{
	CFileStatus fs;
	CTime time = CTime::GetCurrentTime();
	CString queryStr;

	queryStr.Format("%ssummary_%d_%d_%d.db3", m_ModelFolderName, time.GetYear(), time.GetMonth(), time.GetDay());

	if (CFile::GetStatus(queryStr, fs))
	{
		m_DBFileName = queryStr;
		return true;
	}

	return false;
}

void CTrendDataBaseClass::getDataBase()
{
	if(!IsExistDataBase())	CreateDataBase();	
}

bool CTrendDataBaseClass::insertResultDataBase(_st_SUM_RESULT_DATA r)
{
	try {
		//KJH 2022-03-17 Trend Insert Result CS Ãß°¡
		EnterCriticalSection(&m_csTrend);

		CString queryStr;
		CTime time = CTime::GetCurrentTime();

		getDataBase();

		SQLite::Database mDB(m_DBFileName, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

		queryStr.Format("INSERT INTO Trend_Data_Table(year,month,day,hour,job,ok,ng,xx,yy,tt,xx1,yy1,tt1,xx2,yy2,tt2,R1,R2,R3,R4,R5,R6) VALUES(%4d,%2d,%2d, %2d,%d,\"%s\", \"%s\",%4.4f,%4.4f,%4.4f,%4.4f,%4.4f,%4.4f,%4.4f,%4.4f,%4.4f,%4.4f,%4.4f,%4.4f,%4.4f,%4.4f,%4.4f)",
			time.GetYear(), time.GetMonth(), time.GetDay(),time.GetHour(),r._jobID, r._bok?"TRUE":"FALSE", r._bng ? "TRUE":"FALSE",r._x[0],r._y[0],r._t[0], r._x[1], r._y[1], r._t[1], r._x[2], r._y[2], r._t[2], r._r1,r._r2, r._r3, r._r4, r._r5, r._r6);
		
		mDB.exec(queryStr); // 	
		
		LeaveCriticalSection(&m_csTrend);
	}
	catch (std::exception& e)
	{
		CString strError = e.what();	
		LeaveCriticalSection(&m_csTrend);
	}

	return true;
}

bool CTrendDataBaseClass::getTimeResultDataBase(int nOK[],int nNG[], int jobID)
{
	try {
		CString queryStr;
		CTime time = CTime::GetCurrentTime();

		getDataBase();

		SQLite::Database m_Db(m_DBFileName, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

		nNG[0] = nOK[0] = 0;

		int nCount = 0;
		for (int i = 1; i <= 24; i++)
		{
			queryStr.Format("SELECT COUNT(1) FROM Trend_Data_Table WHERE hour=%d AND job=%d AND ok = \"TRUE\"", i, jobID);
			SQLite::Statement query(m_Db, queryStr);
			if (query.executeStep()) nCount = query.getColumn(0).getInt();

			nOK[i] = nCount;
		}

		for (int i = 1; i <= 24; i++)
		{
			queryStr.Format("SELECT COUNT(1) FROM Trend_Data_Table WHERE hour=%d AND job=%d AND ng = \"TRUE\"", i, jobID);
			SQLite::Statement query(m_Db, queryStr);
			if (query.executeStep()) nCount = query.getColumn(0).getInt();
			nNG[i] = nCount;
		}
	}
	catch (std::exception& e)
	{
		CString strError = e.what();
	}

	return true;
}

bool CTrendDataBaseClass::getDailyResultDataBase(int nOK[], int nNG[],int jobID)
{
	try {
		CFileStatus fs;
		CString queryStr;
		CTime time = CTime::GetCurrentTime();		

		nNG[0] = nOK[0] = 0;

		int nCount = 0;
		for (int i = 1; i < 32; i++)
		{
			queryStr.Format("%ssummary_%d_%d_%d.db3", m_ModelFolderName, time.GetYear(), time.GetMonth(), i);

			if (CFile::GetStatus(queryStr, fs))
			{				
				SQLite::Database m_Db(queryStr, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

				nCount = 0;
				queryStr.Format("SELECT COUNT(1) FROM Trend_Data_Table WHERE job=%d  AND ok = \"TRUE\"", jobID);
				SQLite::Statement query(m_Db, queryStr);
				if (query.executeStep()) nCount = query.getColumn(0).getInt();
				nOK[i] = nCount;

				nCount = 0;
				queryStr.Format("SELECT COUNT(1) FROM Trend_Data_Table WHERE job=%d AND ng = \"TRUE\"", jobID);
				SQLite::Statement query0(m_Db, queryStr);
				if (query0.executeStep()) nCount = query0.getColumn(0).getInt();
				nNG[i] = nCount;
			}
			else 
			{
				nOK[i] = 0;
				nNG[i] = 0;
			}			
		}
	}
	catch (std::exception& e)
	{
		CString strError = e.what();
	}

	return true;
}

bool CTrendDataBaseClass::getScatterChartDataBase(float **nData, int jobID)
{
	try 
	{
		CString queryStr;
		CTime time = CTime::GetCurrentTime();
		int id = 0;

		getDataBase();

		SQLite::Database m_Db(m_DBFileName, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

		queryStr.Format("SELECT * FROM Trend_Data_Table WHERE job=%d", jobID);
		SQLite::Statement query(m_Db, queryStr);
		while (query.executeStep())
		{
			nData[0][id] = query.getColumn(8).getDouble();
			nData[1][id] = query.getColumn(9).getDouble();
			nData[2][id] = query.getColumn(10).getDouble();
			nData[3][id] = query.getColumn(11).getDouble();
			nData[4][id] = query.getColumn(12).getDouble();
			nData[5][id] = query.getColumn(13).getDouble();
			nData[6][id] = query.getColumn(14).getDouble();
			nData[7][id] = query.getColumn(15).getDouble();
			nData[8][id] = query.getColumn(16).getDouble();
			nData[9][id] = query.getColumn(17).getDouble();
			nData[10][id] = query.getColumn(18).getDouble();
			nData[11][id] = query.getColumn(19).getDouble();
			nData[12][id] = query.getColumn(20).getDouble();
			nData[13][id] = query.getColumn(21).getDouble();
			nData[14][id] = query.getColumn(22).getDouble();
			id++;
			if (id > 4096) break;
		}
	}
	catch (std::exception& e)
	{
		CString strError = e.what();
	}

	return true;
}