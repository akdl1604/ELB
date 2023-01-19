#ifndef __RTL_MLESAC__
#define __RTL_MLESAC__

#include "MSAC.hpp"

#ifndef M_PI
#   define M_PI                         3.14159265358979323846
#endif

namespace RTL
{

template <class Model, class Datum, class Data>
class MLESAC : virtual public RANSAC<Model, Datum, Data>
{
public:
    MLESAC(Estimator<Model, Datum, Data>* estimator) : RANSAC<Model, Datum, Data>(estimator)
    {
        resultGamma = 0.0;
        dataError2 = NULL;
        SetParamIterationEM();
        SetParamSigmaScale();
    }

    void SetParamIterationEM(int iteration = 5) { paramIterationEM = iteration; }

    int GetParamIterationEM(void) { return paramIterationEM; }

    void SetParamSigmaScale(double scale = 1.96) { paramSigmaScale = scale; }

    double GetParamSigmaScale(void) { return paramSigmaScale; }

    double GetResultGamma(void) { return resultGamma; } // Tkyuha 221108 에러율

protected:
    virtual void Initialize(const Data& data, int N)
    {
        RANSAC::Initialize(data, N);
        dataError2 = new double[N];
        assert(dataError2 != NULL);
        double sigma = paramThreshold / paramSigmaScale;
        dataSigma2 = sigma * sigma;
    }

    virtual double EvaluateModel(const Model& model, const Data& data, int N)
    {
        // Calculate squared errors
        double minError = HUGE_VAL, maxError = -HUGE_VAL;
        for (int i = 0; i < N; i++)
        {
            double error = toolEstimator->ComputeError(model, data[i]);
            if (error < minError) minError = error;
            if (error > maxError) maxError = error;
            dataError2[i] = error * error;
        }

        // Estimate the inlier ratio using EM
        const double nu = maxError - minError;
        double gamma = 0.5;
        //HTK 2022-06-29 MLESAC error가 없는 경우 예외처리 추가(분모가 0이되어서 수직선으로 표시됨)
        if (nu == 0) return 0;

        for (int iter = 0; iter < paramIterationEM; iter++)
        {
            double sumPosteriorProb = 0;
            const double probOutlier = (1 - gamma) / nu;
            const double probInlierCoeff = gamma / sqrt(2 * M_PI * dataSigma2);
            for (int i = 0; i < N; i++)
            {
                double probInlier = probInlierCoeff * exp(-0.5 * dataError2[i] / dataSigma2);
                sumPosteriorProb += probInlier / (probInlier + probOutlier);
            }
            gamma = sumPosteriorProb / N;
        }

        resultGamma = gamma; // 221108 Tkyuha 에러율 추가함

        // Evaluate the model
        double sumLogLikelihood = 0;
        const double probOutlier = (1 - gamma) / nu;
        const double probInlierCoeff = gamma / sqrt(2 * M_PI * dataSigma2);
        for (int i = 0; i < N; i++)
        {
            double probInlier = probInlierCoeff * exp(-0.5 * dataError2[i] / dataSigma2);
            sumLogLikelihood = sumLogLikelihood - log(probInlier + probOutlier);
        }
        return sumLogLikelihood;
    }

    virtual void Terminate(const Model& bestModel, const Data& data, int N)
    {
        if (dataError2 != NULL)
        {
            delete [] dataError2;
            dataError2 = NULL;
        }
        RANSAC::Terminate(bestModel, data, N);
    }

    int paramIterationEM;

    double paramSigmaScale;

    double* dataError2;

    double dataSigma2;

    double resultGamma;  // Tkyuha 221108 에러율
};

} // End of 'RTL'

#endif // End of '__RTL_MLESAC__'
