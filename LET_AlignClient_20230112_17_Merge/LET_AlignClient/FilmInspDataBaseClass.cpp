#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "FilmInspDataBaseClass.h"

CFilmInspDataBaseClass::CFilmInspDataBaseClass()
{
	m_ModelFolderName = "";
	m_DBFileName = "";
	InitializeCriticalSection(&m_csFilmInspData);
	m_pMain = (CLET_AlignClientDlg*)AfxGetMainWnd();
}

CFilmInspDataBaseClass::~CFilmInspDataBaseClass()
{
	DeleteCriticalSection(&m_csFilmInspData);
}

bool CFilmInspDataBaseClass::InitDataBase(CString dbFolder, int job)
{
	m_ModelFolderName.Format("%sDataBase\\FilmInsp\\", dbFolder);
	m_nJobID = job;

	if (_access(m_ModelFolderName, 0) != 0)	_mkdir(m_ModelFolderName);

	return true;
}

//SJB 2022-08-22 DB 생성
bool CFilmInspDataBaseClass::CreateDataBase()
{
	try {
		CFileStatus fs;
		CString queryStr;
		switch (m_nJobID)
		{
		case 0:
			queryStr.Format("%sFilmInsp1.db3", m_ModelFolderName);
			break;
		case 1:
			queryStr.Format("%sFilmInsp2.db3", m_ModelFolderName);
			break;
		}

		m_DBFileName = queryStr;
		if (CFile::GetStatus(queryStr, fs))			return true;

		SQLite::Database    m_Db(queryStr, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);

		queryStr.Format(" CREATE TABLE IF NOT EXISTS Film_Insp_Data_Table(id_num INTEGER PRIMARY KEY, year INTEGER, month INTEGER, day INTEGER, \
			LX DOUBLE, LY DOUBLE, RX DOUBLE, RY DOUBLE)");
		m_Db.exec(queryStr);
	}
	catch (std::exception& e)
	{
		CString strError = e.what();
		strError.Format("[%s] [Create] %s", m_pMain->vt_job_info[m_nJobID].job_name.c_str(), strError);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strError);
	}

	return true;
}

//SJB 2022-08-22 DB 데이터 저장
bool CFilmInspDataBaseClass::InsertFilmInspDataBase(double lx, double ly, double rx, double ry)
{
	try {
		EnterCriticalSection(&m_csFilmInspData);

		CString queryStr;
		CTime time = CTime::GetCurrentTime();

		getDataBase();

		SQLite::Database mDB(m_DBFileName, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

		queryStr.Format("INSERT INTO Film_Insp_Data_Table(year, month, day, LX, LY, RX, RY) VALUES(%4d,%2d,%2d, %.4f, %.4f, %.4f, %.4f)", \
			time.GetYear(), time.GetMonth(), time.GetDay(), lx, ly, rx, ry);

		mDB.exec(queryStr);

		LeaveCriticalSection(&m_csFilmInspData);
	}
	catch (std::exception& e)
	{
		CString strError = e.what();
		strError.Format("[%s] [Insert] %s", m_pMain->vt_job_info[m_nJobID].job_name.c_str(), strError);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strError);
		LeaveCriticalSection(&m_csFilmInspData);
		return false;
	}
	return true;
}

//SJB 2022-08-22 DB 일별 데이터 검색
bool CFilmInspDataBaseClass::getFilmInspSearchData(vector<_st_FILM_INSP_DATA>& vtList, int count, int sYear, int sMonth, int sDay)
{
	try
	{
		CString queryStr;
		_st_FILM_INSP_DATA dataList;
		CTime time = CTime::GetCurrentTime();
		int id = 0;

		getDataBase();

		SQLite::Database m_Db(m_DBFileName, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

		queryStr.Format("SELECT * FROM Film_Insp_Data_Table WHERE %d == year and %d == month and %d == day", \
			sYear, sMonth, sDay);
		SQLite::Statement query(m_Db, queryStr);
		while (query.executeStep())
		{
			dataList._lx = query.getColumn(4).getDouble();
			dataList._ly = query.getColumn(5).getDouble();
			dataList._rx = query.getColumn(6).getDouble();
			dataList._ry = query.getColumn(7).getDouble();
			vtList.push_back(dataList);
			id++;
			if (id > count - 1) break;
		}
	}
	catch (std::exception& e)
	{
		CString strError = e.what();
		strError.Format("[%s] [Search] %s", m_pMain->vt_job_info[m_nJobID].job_name.c_str(), strError);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strError);
	}

	return true;
}

//SJB 2022-08-22 DB 일별 데이터 갯수
int CFilmInspDataBaseClass::getFilmInspSearchDataCount(int sYear, int sMonth, int sDay)
{
	int count = 0;
	try
	{
		CString queryStr;
		getDataBase();

		SQLite::Database m_Db(m_DBFileName, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

		queryStr.Format("SELECT COUNT(*) FROM Film_Insp_Data_Table WHERE %d == year and %d == month and %d == day", \
			sYear, sMonth, sDay);
		SQLite::Statement query(m_Db, queryStr);
		query.executeStep();
		count = query.getColumn(0).getInt();
	}
	catch (std::exception& e)
	{
		CString strError = e.what();
		strError.Format("[%s] [DataCount] %s", m_pMain->vt_job_info[m_nJobID].job_name.c_str(), strError);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strError);
	}

	return count;
}

void CFilmInspDataBaseClass::getDataBase()
{
	if (!IsExistDataBase())	CreateDataBase();
}

bool CFilmInspDataBaseClass::IsExistDataBase()
{
	CFileStatus fs;
	CString queryStr;

	switch (m_nJobID)
	{
	case 0:
		queryStr.Format("%sFilmInsp1.db3", m_ModelFolderName);
		break;
	case 1:
		queryStr.Format("%sFilmInsp2.db3", m_ModelFolderName);
		break;
	}

	if (CFile::GetStatus(queryStr, fs))
	{
		m_DBFileName = queryStr;
		return true;
	}

	return false;
}

//SJB 2022-08-27 DB 삭제(데이터 보존 1년)
bool CFilmInspDataBaseClass::DeleteFilmInspDataBase()
{
	try
	{
		EnterCriticalSection(&m_csFilmInspData);

		CString queryStr;
		CTime time, tDelete;
		time = CTime::GetCurrentTime();
		tDelete = time - CTimeSpan(365, 0, 0, 0); //일, 시, 분, 초

		getDataBase();

		SQLite::Database m_DB(m_DBFileName, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

		queryStr.Format("DELETE FROM Film_Insp_Data_Table WHERE year < %d or (year == %d and month < %d) or (year == %d and month == %d and day < %d)", \
			tDelete.GetYear(), tDelete.GetYear(), tDelete.GetMonth(), tDelete.GetYear(), tDelete.GetMonth(), tDelete.GetDay());

		m_DB.exec(queryStr);

		LeaveCriticalSection(&m_csFilmInspData);
	}
	catch(std::exception& e)
	{
		CString strError = e.what();
		strError.Format("[%s] [Delete] %s", m_pMain->vt_job_info[m_nJobID].job_name.c_str(), strError);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strError);
		LeaveCriticalSection(&m_csFilmInspData);
	}
	return true;
}

