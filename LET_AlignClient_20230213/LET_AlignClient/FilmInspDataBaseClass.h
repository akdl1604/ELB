#pragma once

#include "UI_Control/SQLiteCpp/SQLiteCpp.h"

struct _st_FILM_INSP_DATA {
    double _lx;
    double _ly;
    double _rx;
    double _ry;
};

class CFilmInspDataBaseClass
{
public:
    // Constructor
    CFilmInspDataBaseClass();
    virtual ~CFilmInspDataBaseClass();

    bool CreateDataBase();
    bool InitDataBase(CString dbFolder, int job);
    bool IsExistDataBase();
    bool InsertFilmInspDataBase(double lx, double ly, double rx, double ry);
    void getDataBase();
    bool getFilmInspSearchData(vector<_st_FILM_INSP_DATA>& vtList, int count, int sYear, int sMonth, int sDay);
    int getFilmInspSearchDataCount(int sYear, int sMonth, int sDay);
    bool DeleteFilmInspDataBase();
private:
    CString m_ModelFolderName;
    CString m_DBFileName;
    int m_nJobID;

    CRITICAL_SECTION m_csFilmInspData;
    CLET_AlignClientDlg* m_pMain;
};

