#pragma once

#include "UI_Control/SQLiteCpp/SQLiteCpp.h"

struct _st_LOGIN_DATA {
    int _userLevel;
    string _userName;
    string _userPassword;
};

class CLogInDataBaseClass
{
public:
    // Constructor
    CLogInDataBaseClass();
    virtual ~CLogInDataBaseClass();

    bool InitDataBase(CString dbFolder);

    bool CreateDataBase();
    bool IsExistDataBase();
    void getDataBase();

    bool insertLogInDataBase(_st_LOGIN_DATA result);
    bool deleteLogInDataBase(string userName);
    bool getLogInDataBase(string userName, string userpassword,int &level);
    bool getLogInListDataBase(_st_LOGIN_DATA *list);
private:
    CString m_ModelFolderName;
    CString m_DBFileName;

    CRITICAL_SECTION m_csLogIn;
};

