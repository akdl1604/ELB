#ifndef __RTL_LINE__
#define __RTL_LINE__

#include "Base.hpp"
#include <cmath>
#include <limits>
#include <random>

class _Point
{
public:
    _Point() : x(0), y(0) { }

    _Point(double _x, double _y) : x(_x), y(_y) { }

    friend std::ostream& operator<<(std::ostream& out, const _Point& p) { return out << p.x << ", " << p.y; }

    double x, y;
};


class Line
{
public:
    Line() : a(0), b(0), c(0) { }

    Line(double _a, double _b, double _c) : a(_a), b(_b), c(_c) { }

    friend std::ostream& operator<<(std::ostream& out, const Line& l) { return out << l.a << ", " << l.b << ", " << l.c; }

    double a, b, c;
};

class Circle
{
public:
    Circle() : cx(0), cy(0), rad(0) { }

    Circle(double _cx, double _cy, double _rad) : cx(_cx), cy(_cy), rad(_rad) { }

    friend std::ostream& operator<<(std::ostream& out, const Circle& l) { return out << l.cx << ", " << l.cy << ", " << l.rad; }

    double cx, cy, rad;
};


class LineEstimator : virtual public RTL::Estimator<Line, _Point, std::vector<_Point> >
{
public:
    virtual Line ComputeModel(const std::vector<_Point>& data, const std::set<int>& samples)
    {
        double meanX = 0, meanY = 0, meanXX = 0, meanYY = 0, meanXY = 0;
        for (auto itr = samples.begin(); itr != samples.end(); itr++)
        {
            const _Point& p = data[*itr];
            meanX += p.x;
            meanY += p.y;
            meanXX += p.x * p.x;
            meanYY += p.y * p.y;
            meanXY += p.x * p.y;
        }
        size_t M = samples.size();
        meanX /= M;
        meanY /= M;
        meanXX /= M;
        meanYY /= M;
        meanXY /= M;
        double a = meanXX - meanX * meanX;
        double b = meanXY - meanX * meanY;
        double d = meanYY - meanY * meanY;

        Line line;
        if (fabs(b) > DBL_EPSILON)
        {
            // Calculate the first eigen vector of A = [a, b; b, d]
            // Ref. http://www.math.harvard.edu/archive/21b_fall_04/exhibits/2dmatrices/index.html
            double T2 = (a + d) / 2;
            double lambda = T2 - sqrt(T2 * T2 - (a * d - b * b));
            double v1 = lambda - d, v2 = b;
            double norm = sqrt(v1 * v1 + v2 * v2);
            line.a = v1 / norm;
            line.b = v2 / norm;
        }
        else
        {
            line.a = 1;
            line.b = 0;
        }
        line.c = -line.a * meanX - line.b * meanY;
        return line;
    }

    virtual double ComputeError(const Line& line, const _Point& point)
    {
        return line.a * point.x + line.b * point.y + line.c;
    }
}; // End of 'LineEstimator'

class CircleEstimator : virtual public RTL::Estimator<Circle, _Point, std::vector<_Point> >
{
private:
	bool SolveLinearEQ3x3(double A[9], double bb[3], double x[3]) 
	{
		double invA[9];
		double det = (A[0] * (A[4] * A[8] - A[5] * A[7]) - A[1] * (A[3] * A[8] - A[5] * A[6]) + A[2] * (A[3] * A[7] - A[4] * A[6]));
		
		if (det != 0.) {
			det = 1. / det;
			invA[0] = (A[4] * A[8] - A[5] * A[7]) * det;
			invA[1] = (A[2] * A[7] - A[1] * A[8]) * det;
			invA[2] = (A[1] * A[5] - A[2] * A[4]) * det;
			invA[3] = (A[5] * A[6] - A[3] * A[8]) * det;
			invA[4] = (A[0] * A[8] - A[2] * A[6]) * det;
			invA[5] = (A[2] * A[3] - A[0] * A[5]) * det;
			invA[6] = (A[3] * A[7] - A[4] * A[6]) * det;
			invA[7] = (A[1] * A[6] - A[0] * A[7]) * det;
			invA[8] = (A[0] * A[4] - A[1] * A[3]) * det;
			//
			x[0] = invA[0] * bb[0] + invA[1] * bb[1] + invA[2] * bb[2];
			x[1] = invA[3] * bb[0] + invA[4] * bb[1] + invA[5] * bb[2];
			x[2] = invA[6] * bb[0] + invA[7] * bb[1] + invA[8] * bb[2];
			return true;
		}
		else x[0] = x[1] = x[2] = 0;
			
		return false;
	}
public:
    virtual Circle ComputeModel(const std::vector<_Point>& data, const std::set<int>& samples)
    {
        Circle circle;
		
		int N = int(samples.size());
		
		double sx = 0, sy = 0, sxx = 0, sxy = 0, syy = 0;
		double sxxx = 0, sxxy = 0, sxyy = 0, syyy = 0;
		
		for (auto itr = samples.begin(); itr != samples.end(); itr++)
		{
			const _Point& p = data[*itr];
			
			double x = p.x, y = p.y;
			double xx = x * x, yy = y * y;
			
			sx += x;        sy += y;
			sxx += xx;      sxy += x * y;       syy += yy;
			sxxx += xx * x; sxxy += xx * y;
			sxyy += x * yy; syyy += yy * y;
		}
		
		double A[9], b[3], sol[3];
		
		A[0] = sxx, A[1] = sxy, A[2] = sx,
			A[3] = sxy, A[4] = syy, A[5] = sy,
			A[6] = sx, A[7] = sy, A[8] = N,
			b[0] = -sxxx - sxyy,
			b[1] = -sxxy - syyy,
			b[2] = -sxx - syy;
			
		if (!SolveLinearEQ3x3(A, b, sol)) return circle;
		
		double det = sol[0] * sol[0] + sol[1] * sol[1] - 4 * sol[2];
		
		if (det <= 0) return circle;
		
		circle.cx = -sol[0] / 2;
		circle.cy = -sol[1] / 2;    
		circle.rad = sqrt(det) / 2.;
       
        return circle;
    }

    virtual double ComputeError(const Circle& circle, const _Point& point)
    {
		double var = 0;          
		
		if(circle.rad==0) return var;
			
		double dist = sqrt(pow((point.x - circle.cx),2) + pow((point.y - circle.cy),2));
		double distdeviate = fabs(dist - circle.rad) / circle.rad;
			
		var = pow((dist - circle.rad),2);
		
        return var;
    }
}; // End of 'CircleEstimator'

class LineObserver : virtual public RTL::Observer<Line, _Point, std::vector<_Point> >
{
public:
    LineObserver(_Point _max = _Point(640, 480), _Point _min = _Point(0, 0)) : RANGE_MAX(_max), RANGE_MIN(_min) { }

    virtual std::vector<_Point> GenerateData(const Line& line, int N, std::vector<int>& inliers, double noise = 0, double ratio = 1)
    {
        std::mt19937 generator;
        std::uniform_real_distribution<double> uniform(0, 1);
        std::normal_distribution<double> normal(0, 1);

        std::vector<_Point> data;
        if (fabs(line.b) > fabs(line.a))
        {
            for (int i = 0; i < N; i++)
            {
                _Point point;
                point.x = (RANGE_MAX.x - RANGE_MIN.x) * uniform(generator) + RANGE_MIN.x;
                double vote = uniform(generator);
                if (vote > ratio)
                {
                    // Generate an outlier
                    point.y = (RANGE_MAX.y - RANGE_MIN.y) * uniform(generator) + RANGE_MIN.y;
                }
                else
                {
                    // Generate an inlier
                    point.y = (line.a * point.x + line.c) / -line.b;
                    point.x += noise * normal(generator);
                    point.y += noise * normal(generator);
                    inliers.push_back(i);
                }
                data.push_back(point);
            }
        }
        else
        {
            for (int i = 0; i < N; i++)
            {
                _Point point;
                point.y = (RANGE_MAX.y - RANGE_MIN.y) * uniform(generator) + RANGE_MIN.y;
                double vote = uniform(generator);
                if (vote > ratio)
                {
                    // Generate an outlier
                    point.x = (RANGE_MAX.x - RANGE_MIN.x) * uniform(generator) + RANGE_MIN.x;
                }
                else
                {
                    // Generate an inlier
                    point.x = (line.b * point.y + line.c) / -line.a;
                    point.x += noise * normal(generator);
                    point.y += noise * normal(generator);
                    inliers.push_back(i);
                }
                data.push_back(point);
            }
        }
        return data;
    }

    const _Point RANGE_MIN;

    const _Point RANGE_MAX;
};

#endif // End of '__RTL_LINE__'
