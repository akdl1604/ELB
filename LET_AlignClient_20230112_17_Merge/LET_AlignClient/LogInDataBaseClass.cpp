#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "LogInDataBaseClass.h"

CLogInDataBaseClass::CLogInDataBaseClass()
{
	m_ModelFolderName = "";
	m_DBFileName = "";
	InitializeCriticalSection(&m_csLogIn);
}

CLogInDataBaseClass::~CLogInDataBaseClass()
{
	DeleteCriticalSection(&m_csLogIn);
}

bool CLogInDataBaseClass::insertLogInDataBase(_st_LOGIN_DATA r)
{
	try {
		EnterCriticalSection(&m_csLogIn);

		CString queryStr;
		CTime time = CTime::GetCurrentTime();

		getDataBase();

		SQLite::Database mDB(m_DBFileName, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

		queryStr.Format("INSERT INTO User_Data_Table(year,month,day,hour,level,username,password) VALUES(%4d,%2d,%2d, %2d,%d,\"%s\",\"%s\")",\
			time.GetYear(), time.GetMonth(), time.GetDay(),time.GetHour(),r._userLevel, r._userName.c_str(), r._userPassword.c_str());
		
		mDB.exec(queryStr); // 	
		
		LeaveCriticalSection(&m_csLogIn);
	}
	catch (std::exception& e)
	{
		CString strError = e.what();	
		LeaveCriticalSection(&m_csLogIn);
	}

	return true;
}
bool CLogInDataBaseClass::getLogInListDataBase(_st_LOGIN_DATA *list)
{
	try
	{
		CString queryStr;
		CTime time = CTime::GetCurrentTime();
		int id = 0;

		getDataBase();

		SQLite::Database m_Db(m_DBFileName, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

		queryStr.Format("SELECT * FROM User_Data_Table");
		SQLite::Statement query(m_Db, queryStr);
		while (query.executeStep())
		{
			list[id]._userLevel = query.getColumn(5).getInt();
			list[id]._userName = query.getColumn(6).getText();
			id++;
			if(id > 200) break;
		}
	}
	catch (std::exception& e)
	{
		CString strError = e.what();
	}

	return true;
}

bool CLogInDataBaseClass::InitDataBase(CString dbFolder)
{
	m_ModelFolderName.Format("%sDataBase\\", dbFolder);

	if (_access(m_ModelFolderName, 0) != 0)	_mkdir(m_ModelFolderName);

	return true;
}

bool CLogInDataBaseClass::getLogInDataBase(string userName, string userpassword, int& level)
{
	bool id = false;

	try
	{
		getDataBase();

		CString queryStr;
		SQLite::Database m_Db(m_DBFileName, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

		queryStr.Format("SELECT * FROM User_Data_Table WHERE username=\"%s\"", userName.c_str());
		SQLite::Statement query(m_Db, queryStr);
		while (query.executeStep())
		{
			level = query.getColumn(5).getInt();
			string pass = query.getColumn(7).getText();
			if (userpassword == pass)
				id = true;
		}
	}
	catch (std::exception& e)
	{
		CString strError = e.what();
	}

	return id;
}

bool CLogInDataBaseClass::deleteLogInDataBase(string userName)
{
	try {
		EnterCriticalSection(&m_csLogIn);

		CString queryStr;
		getDataBase();

		SQLite::Database m_DB(m_DBFileName, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

		queryStr.Format("DELETE FROM User_Data_Table WHERE username=\"%s\"", userName.c_str());

		m_DB.exec(queryStr); // 	

		LeaveCriticalSection(&m_csLogIn);
	}
	catch (std::exception& e)
	{
		CString strError = e.what();
		LeaveCriticalSection(&m_csLogIn);
	}

	return true;
}

void CLogInDataBaseClass::getDataBase()
{
	if (!IsExistDataBase())	CreateDataBase();
}

bool CLogInDataBaseClass::IsExistDataBase()
{
	CFileStatus fs;
	CString queryStr;

	queryStr.Format("%sUser.db3", m_ModelFolderName);

	if (CFile::GetStatus(queryStr, fs))
	{
		m_DBFileName = queryStr;
		return true;
	}

	return false;
}

bool CLogInDataBaseClass::CreateDataBase()
{
	try {
		CFileStatus fs;
		CString queryStr;
		queryStr.Format("%sUser.db3", m_ModelFolderName);

		m_DBFileName = queryStr;
		if (CFile::GetStatus(queryStr, fs))			return true;

		SQLite::Database    m_Db(queryStr, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);

		queryStr.Format(" CREATE TABLE IF NOT EXISTS User_Data_Table(id_num INTEGER PRIMARY KEY, year INTEGER, month INTEGER,\
			day INTEGER, hour INTEGER, level INTEGER,username TEXT,password TEXT)"); // , 

		m_Db.exec(queryStr);
	}
	catch (std::exception& e)
	{
		CString strError = e.what();
		AfxMessageBox(strError);
	}

	return true;
}