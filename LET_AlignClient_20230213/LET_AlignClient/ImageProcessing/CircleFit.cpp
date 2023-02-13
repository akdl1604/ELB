#include "stdafx.h"
#include "CircleFit.h"

#include <cmath>

using namespace std;

//   Note: long double is an 80-bit format (more accurate, but more memory demanding and slower)

typedef long long integers;
//   next define some frequently used constants:

const double One = 1.0, Two = 2.0, Three = 3.0, Four = 4.0, Five = 5.0, Six = 6.0, Ten = 10.0;
//const reals One=1.0L,Two=2.0L,Three=3.0L,Four=4.0L,Five=5.0L,Six=6.0L,Ten=10.0L;
const double Pi = 3.141592653589793238462643383L;
//const double REAL_MAX = numeric_limits<double>::max();
//const double REAL_MIN = numeric_limits<double>::min();
//const double REAL_EPSILON = numeric_limits<double>::epsilon();

//   next define some frequently used functions:

template<typename T>
inline T SQR(T t) { return t * t; };

double pythag(double a, double b)
{
    double absa = abs(a), absb = abs(b);
    if (absa > absb) return absa * sqrt(One + SQR(absb / absa));
    else return (absb == 0.0 ? 0.0 : absb * sqrt(One + SQR(absa / absb)));
}


/************************************************************************
            BODY OF THE MEMBER ROUTINES
************************************************************************/
// Default constructor

CircleFit::CircleFit()
{
    a = 0.; b = 0.; r = 1.; s = 0.; i = 0; j = 0;
}

// Constructor with assignment of the circle parameters only

CircleFit::CircleFit(double aa, double bb, double rr)
{
    a = aa; b = bb; r = rr;
}


/************************************************************************
            BODY OF THE MEMBER ROUTINES
************************************************************************/
// Default constructor
CircleData::CircleData()
{
    n = 0;
    X = new double[n];
    Y = new double[n];
    for (int i = 0; i < n; i++)
    {
        X[i] = 0.;
        Y[i] = 0.;
    }
}

// Constructor with assignment of the field N
CircleData::CircleData(int N)
{
    n = N;
    X = new double[n];
    Y = new double[n];

    for (int i = 0; i < n; i++)
    {
        X[i] = 0.;
        Y[i] = 0.;
    }
}

// Constructor with assignment of each field
CircleData::CircleData(int N, double dataX[], double dataY[])
{
    n = N;
    X = new double[n];
    Y = new double[n];

    for (int i = 0; i < n; i++)
    {
        X[i] = dataX[i];
        Y[i] = dataY[i];
    }
}

// Routine that computes the x- and y- sample means (the coordinates of the centeroid)

void CircleData::means(void)
{
    meanX = 0.; meanY = 0.;

    for (int i = 0; i < n; i++)
    {
        meanX += X[i];
        meanY += Y[i];
    }
    meanX /= n;
    meanY /= n;
}

// Routine that centers the data set (shifts the coordinates to the centeroid)

void CircleData::center(void)
{
    double sX = 0., sY = 0.;
    int i;

    for (i = 0; i < n; i++)
    {
        sX += X[i];
        sY += Y[i];
    }
    sX /= n;
    sY /= n;

    for (i = 0; i < n; i++)
    {
        X[i] -= sX;
        Y[i] -= sY;
    }
    meanX = 0.;
    meanY = 0.;
}

// Routine that scales the coordinates (makes them of order one)

void CircleData::scale(void)
{
    double sXX = 0., sYY = 0., scaling;
    int i;

    for (i = 0; i < n; i++)
    {
        sXX += X[i] * X[i];
        sYY += Y[i] * Y[i];
    }
    scaling = sqrt((sXX + sYY) / n / Two);

    for (i = 0; i < n; i++)
    {
        X[i] /= scaling;
        Y[i] /= scaling;
    }
}



// Destructor
CircleData::~CircleData()
{
    delete[] X;
    delete[] Y;
}

double Sigma(CircleData& data, CircleFit& circle)
{
    double sum = 0., dx, dy;

    for (int i = 0; i < data.n; i++)
    {
        dx = data.X[i] - circle.a;
        dy = data.Y[i] - circle.b;
        sum += SQR(sqrt(dx * dx + dy * dy) - circle.r);
    }
    return sqrt(sum / data.n);
}

CircleFit CircleFitByHyper(CircleData& data)
{
    int i, iter, IterMAX = 99;

    double Xi, Yi, Zi;
    double Mz, Mxy, Mxx, Myy, Mxz, Myz, Mzz, Cov_xy, Var_z;
    double A0, A1, A2, A22;
    double Dy, xnew, x, ynew, y;
    double DET, Xcenter, Ycenter;

    CircleFit circle;

    data.means();   // Compute x- and y- sample means (via a function in the class "data")

    Mxx = Myy = Mxy = Mxz = Myz = Mzz = 0.;

    for (i = 0; i < data.n; i++)
    {
        Xi = data.X[i] - data.meanX;   //  centered x-coordinates
        Yi = data.Y[i] - data.meanY;   //  centered y-coordinates
        Zi = Xi * Xi + Yi * Yi;

        Mxy += Xi * Yi;
        Mxx += Xi * Xi;
        Myy += Yi * Yi;
        Mxz += Xi * Zi;
        Myz += Yi * Zi;
        Mzz += Zi * Zi;
    }
    Mxx /= data.n;
    Myy /= data.n;
    Mxy /= data.n;
    Mxz /= data.n;
    Myz /= data.n;
    Mzz /= data.n;

    //    computing the coefficients of the characteristic polynomial

    Mz = Mxx + Myy;
    Cov_xy = Mxx * Myy - Mxy * Mxy;
    Var_z = Mzz - Mz * Mz;

    A2 = Four * Cov_xy - Three * Mz * Mz - Mzz;
    A1 = Var_z * Mz + Four * Cov_xy * Mz - Mxz * Mxz - Myz * Myz;
    A0 = Mxz * (Mxz * Myy - Myz * Mxy) + Myz * (Myz * Mxx - Mxz * Mxy) - Var_z * Cov_xy;
    A22 = A2 + A2;

    //    finding the root of the characteristic polynomial
    //    using Newton's method starting at x=0
    //     (it is guaranteed to converge to the right root)

    for (x = 0., y = A0, iter = 0; iter < IterMAX; iter++)  // usually, 4-6 iterations are enough
    {
        Dy = A1 + x * (A22 + 16. * x * x);
        xnew = x - y / Dy;
        if ((xnew == x) || (!isfinite(xnew))) break;
        ynew = A0 + xnew * (A1 + xnew * (A2 + Four * xnew * xnew));
        if (abs(ynew) >= abs(y))  break;
        x = xnew;  y = ynew;
    }

    //    computing paramters of the fitting circle

    DET = x * x - x * Mz + Cov_xy;
    Xcenter = (Mxz * (Myy - x) - Myz * Mxy) / DET / Two;
    Ycenter = (Myz * (Mxx - x) - Mxz * Mxy) / DET / Two;

    //       assembling the output

    circle.a = Xcenter + data.meanX;
    circle.b = Ycenter + data.meanY;
    circle.r = sqrt(Xcenter * Xcenter + Ycenter * Ycenter + Mz - x - x);
    circle.s = Sigma(data, circle);
    circle.i = 0;
    circle.j = iter;  //  return the number of iterations, too

    return circle;
}

CircleFit CircleFitByKasa(CircleData& data)
{
    int i;

    double Xi, Yi, Zi;
    double Mxy, Mxx, Myy, Mxz, Myz;
    double B, C, G11, G12, G22, D1, D2;

    CircleFit circle;

    data.means();   // Compute x- and y- sample means (via a function in the class "data")

//     computing moments

    Mxx = Myy = Mxy = Mxz = Myz = 0.;

    for (i = 0; i < data.n; i++)
    {
        Xi = data.X[i] - data.meanX;   //  centered x-coordinates
        Yi = data.Y[i] - data.meanY;   //  centered y-coordinates
        Zi = Xi * Xi + Yi * Yi;

        Mxx += Xi * Xi;
        Myy += Yi * Yi;
        Mxy += Xi * Yi;
        Mxz += Xi * Zi;
        Myz += Yi * Zi;
    }
    Mxx /= data.n;
    Myy /= data.n;
    Mxy /= data.n;
    Mxz /= data.n;
    Myz /= data.n;

    //    solving system of equations by Cholesky factorization

    G11 = sqrt(Mxx);
    G12 = Mxy / G11;
    G22 = sqrt(Myy - G12 * G12);

    D1 = Mxz / G11;
    D2 = (Myz - D1 * G12) / G22;

    //    computing paramters of the fitting circle

    C = D2 / G22 / Two;
    B = (D1 - G12 * C) / G11 / Two;

    //       assembling the output

    circle.a = B + data.meanX;
    circle.b = C + data.meanY;
    circle.r = sqrt(B * B + C * C + Mxx + Myy);
    circle.s = Sigma(data, circle);
    circle.i = 0;
    circle.j = 0;

    return circle;
}

CircleFit CircleFitByPratt(CircleData& data)
{
    int i, iter, IterMAX = 99;

    double Xi, Yi, Zi;
    double Mz, Mxy, Mxx, Myy, Mxz, Myz, Mzz, Cov_xy, Var_z;
    double A0, A1, A2, A22;
    double Dy, xnew, x, ynew, y;
    double DET, Xcenter, Ycenter;

    CircleFit circle;

    data.means();   // Compute x- and y- sample means (via a function in the class "data")

    Mxx = Myy = Mxy = Mxz = Myz = Mzz = 0.;

    for (i = 0; i < data.n; i++)
    {
        Xi = data.X[i] - data.meanX;   //  centered x-coordinates
        Yi = data.Y[i] - data.meanY;   //  centered y-coordinates
        Zi = Xi * Xi + Yi * Yi;

        Mxy += Xi * Yi;
        Mxx += Xi * Xi;
        Myy += Yi * Yi;
        Mxz += Xi * Zi;
        Myz += Yi * Zi;
        Mzz += Zi * Zi;
    }
    Mxx /= data.n;
    Myy /= data.n;
    Mxy /= data.n;
    Mxz /= data.n;
    Myz /= data.n;
    Mzz /= data.n;

    Mz = Mxx + Myy;
    Cov_xy = Mxx * Myy - Mxy * Mxy;
    Var_z = Mzz - Mz * Mz;

    A2 = Four * Cov_xy - Three * Mz * Mz - Mzz;
    A1 = Var_z * Mz + Four * Cov_xy * Mz - Mxz * Mxz - Myz * Myz;
    A0 = Mxz * (Mxz * Myy - Myz * Mxy) + Myz * (Myz * Mxx - Mxz * Mxy) - Var_z * Cov_xy;
    A22 = A2 + A2;

    for (x = 0., y = A0, iter = 0; iter < IterMAX; iter++)  // usually, 4-6 iterations are enough
    {
        Dy = A1 + x * (A22 + 16. * x * x);
        xnew = x - y / Dy;
        if ((xnew == x) || (!isfinite(xnew))) break;
        ynew = A0 + xnew * (A1 + xnew * (A2 + Four * xnew * xnew));
        if (abs(ynew) >= abs(y))  break;
        x = xnew;  y = ynew;
    }


    DET = x * x - x * Mz + Cov_xy;
    Xcenter = (Mxz * (Myy - x) - Myz * Mxy) / DET / Two;
    Ycenter = (Myz * (Mxx - x) - Mxz * Mxy) / DET / Two;

    circle.a = Xcenter + data.meanX;
    circle.b = Ycenter + data.meanY;
    circle.r = sqrt(Xcenter * Xcenter + Ycenter * Ycenter + Mz + x + x);
    circle.s = Sigma(data, circle);
    circle.i = 0;
    circle.j = iter;  //  return the number of iterations, too

    return circle;
}

CircleFit CircleFitByTaubin(CircleData& data)
{
    int i, iter, IterMAX = 99;

    double Xi, Yi, Zi;
    double Mz, Mxy, Mxx, Myy, Mxz, Myz, Mzz, Cov_xy, Var_z;
    double A0, A1, A2, A22, A3, A33;
    double Dy, xnew, x, ynew, y;
    double DET, Xcenter, Ycenter;

    CircleFit circle;

    data.means();   // Compute x- and y- sample means (via a function in the class "data")

    Mxx = Myy = Mxy = Mxz = Myz = Mzz = 0.;

    for (i = 0; i < data.n; i++)
    {
        Xi = data.X[i] - data.meanX;   //  centered x-coordinates
        Yi = data.Y[i] - data.meanY;   //  centered y-coordinates
        Zi = Xi * Xi + Yi * Yi;

        Mxy += Xi * Yi;
        Mxx += Xi * Xi;
        Myy += Yi * Yi;
        Mxz += Xi * Zi;
        Myz += Yi * Zi;
        Mzz += Zi * Zi;
    }
    Mxx /= data.n;
    Myy /= data.n;
    Mxy /= data.n;
    Mxz /= data.n;
    Myz /= data.n;
    Mzz /= data.n;

    //      computing coefficients of the characteristic polynomial

    Mz = Mxx + Myy;
    Cov_xy = Mxx * Myy - Mxy * Mxy;
    Var_z = Mzz - Mz * Mz;
    A3 = Four * Mz;
    A2 = -Three * Mz * Mz - Mzz;
    A1 = Var_z * Mz + Four * Cov_xy * Mz - Mxz * Mxz - Myz * Myz;
    A0 = Mxz * (Mxz * Myy - Myz * Mxy) + Myz * (Myz * Mxx - Mxz * Mxy) - Var_z * Cov_xy;
    A22 = A2 + A2;
    A33 = A3 + A3 + A3;

    for (x = 0., y = A0, iter = 0; iter < IterMAX; iter++)  // usually, 4-6 iterations are enough
    {
        Dy = A1 + x * (A22 + A33 * x);
        xnew = x - y / Dy;
        if ((xnew == x) || (!isfinite(xnew))) break;
        ynew = A0 + xnew * (A1 + xnew * (A2 + xnew * A3));
        if (abs(ynew) >= abs(y))  break;
        x = xnew;  y = ynew;
    }

    //       computing paramters of the fitting circle

    DET = x * x - x * Mz + Cov_xy;
    Xcenter = (Mxz * (Myy - x) - Myz * Mxy) / DET / Two;
    Ycenter = (Myz * (Mxx - x) - Mxz * Mxy) / DET / Two;

    //       assembling the output

    circle.a = Xcenter + data.meanX;
    circle.b = Ycenter + data.meanY;
    circle.r = sqrt(Xcenter * Xcenter + Ycenter * Ycenter + Mz);
    circle.s = Sigma(data, circle);
    circle.i = 0;
    circle.j = iter;  //  return the number of iterations, too

    return circle;
}


int CircleFitByLevenbergMarquardtFull(CircleData& data, CircleFit& circleIni, double LambdaIni, CircleFit& circle)
{
    int code, i, iter, inner, IterMAX = 99;

    double factorUp = 10., factorDown = 0.04, lambda, ParLimit = 1.e+6;
    double dx, dy, ri, u, v;
    double Mu, Mv, Muu, Mvv, Muv, Mr, UUl, VVl, Nl, F1, F2, F3, dX, dY, dR;
    double epsilon = 3.e-8;
    double G11, G22, G33, G12, G13, G23, D1, D2, D3;

    CircleFit Old, New;

    New = circleIni;

    New.s = Sigma(data, New);

    lambda = LambdaIni;
    iter = inner = code = 0;

NextIteration:

    Old = New;
    if (++iter > IterMAX) { code = 1;  goto enough; }

    Mu = Mv = Muu = Mvv = Muv = Mr = 0.;

    for (i = 0; i < data.n; i++)
    {
        dx = data.X[i] - Old.a;
        dy = data.Y[i] - Old.b;
        ri = sqrt(dx * dx + dy * dy);
        u = dx / ri;
        v = dy / ri;
        Mu += u;
        Mv += v;
        Muu += u * u;
        Mvv += v * v;
        Muv += u * v;
        Mr += ri;
    }
    Mu /= data.n;
    Mv /= data.n;
    Muu /= data.n;
    Mvv /= data.n;
    Muv /= data.n;
    Mr /= data.n;

    F1 = Old.a + Old.r * Mu - data.meanX;
    F2 = Old.b + Old.r * Mv - data.meanY;
    F3 = Old.r - Mr;

    Old.g = New.g = sqrt(F1 * F1 + F2 * F2 + F3 * F3);

try_again:

    UUl = Muu + lambda;
    VVl = Mvv + lambda;
    Nl = One + lambda;

    G11 = sqrt(UUl);
    G12 = Muv / G11;
    G13 = Mu / G11;
    G22 = sqrt(VVl - G12 * G12);
    G23 = (Mv - G12 * G13) / G22;
    G33 = sqrt(Nl - G13 * G13 - G23 * G23);

    D1 = F1 / G11;
    D2 = (F2 - G12 * D1) / G22;
    D3 = (F3 - G13 * D1 - G23 * D2) / G33;

    dR = D3 / G33;
    dY = (D2 - G23 * dR) / G22;
    dX = (D1 - G12 * dY - G13 * dR) / G11;

    if ((abs(dR) + abs(dX) + abs(dY)) / (One + Old.r) < epsilon) goto enough;

    //       updating the parameters

    New.a = Old.a - dX;
    New.b = Old.b - dY;

    if (abs(New.a) > ParLimit || abs(New.b) > ParLimit) { code = 3; goto enough; }

    New.r = Old.r - dR;

    if (New.r <= 0.)
    {
        lambda *= factorUp;
        if (++inner > IterMAX) { code = 2;  goto enough; }
        goto try_again;
    }

    New.s = Sigma(data, New);

    if (New.s < Old.s)    //   yes, improvement
    {
        lambda *= factorDown;
        goto NextIteration;
    }
    else                       //   no improvement
    {
        if (++inner > IterMAX) { code = 2;  goto enough; }
        lambda *= factorUp;
        goto try_again;
    }

    //       exit

enough:

    Old.i = iter;    // total number of outer iterations (updating the parameters)
    Old.j = inner;   // total number of inner iterations (adjusting lambda)

    circle = Old;

    return code;
}