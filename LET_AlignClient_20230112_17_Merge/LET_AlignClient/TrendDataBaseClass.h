#pragma once

#include "UI_Control/SQLiteCpp/SQLiteCpp.h"

struct _st_SUM_RESULT_DATA {
    int _jobID;
    double _x[3];
    double _y[3];
    double _t[3];
    double _r1;
    double _r2;
    double _r3;
    double _r4;
    double _r5;
    double _r6;
    bool _bok;
    bool _bng;
};

class CTrendDataBaseClass
{
public:
    // Constructor
    CTrendDataBaseClass();
    virtual ~CTrendDataBaseClass();

    bool InitDataBase(CString dbFolder);

    bool CreateDataBase();
    bool IsExistDataBase();
    void getDataBase();

    bool insertResultDataBase(_st_SUM_RESULT_DATA result);
    bool getTimeResultDataBase(int nOK[], int nNG[], int jobID);
    bool getDailyResultDataBase(int nOK[], int nNG[], int jobID);
    bool getScatterChartDataBase(float **nData, int jobID);
private:
    CString m_ModelFolderName;
    CString m_DBFileName;

    CRITICAL_SECTION m_csTrend;
};

