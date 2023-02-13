#pragma once

class CircleData
{
public:

    int n;
    double* X;		//space is allocated in the constructors
    double* Y;		//space is allocated in the constructors
    double meanX, meanY;

    // constructors
    CircleData();
    CircleData(int N);
    CircleData(int N, double X[], double Y[]);

    // routines
    void means(void);
    void center(void);
    void scale(void);
    void print(void);

    // destructors
    ~CircleData();
};

class CircleFit
{
public:

    // The fields of a Circle
    double a, b, r, s, g, Gx, Gy;
    int i, j;

    // constructors
    CircleFit();
    CircleFit(double aa, double bb, double rr);

};

CircleFit CircleFitByHyper(CircleData& data);
CircleFit CircleFitByKasa(CircleData& data);
CircleFit CircleFitByPratt(CircleData& data);
CircleFit CircleFitByTaubin(CircleData& data);
int CircleFitByLevenbergMarquardtFull(CircleData& data, CircleFit& circleIni, double LambdaIni, CircleFit& circle);