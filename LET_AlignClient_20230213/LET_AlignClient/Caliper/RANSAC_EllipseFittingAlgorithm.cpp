#include "StdAfx.h"
#include <math.h>
#include "RANSAC_EllipseFittingAlgorithm.h"
#include "MatrixAlgebra.h"

bool find_in_samples (sPoint *samples, int no_samples, sPoint *data)
{
	for (int i=0; i<no_samples; ++i) {
		if (samples[i].x == data->x && samples[i].y == data->y) {
			return true;
		}
	}
	return false;
}

void get_samples (sPoint *samples, int no_samples, sPoint *data, int no_data)
{
	// 데이터에서 중복되지 않게 N개의 무작위 셈플을 채취한다.
	for (int i=0; i<no_samples; ) {
		int j = rand()%no_data;
		
		if (!find_in_samples(samples, i, &data[j])) {
			samples[i] = data[j];
			++i;
		}
	};
}

int compute_model_parameter(sPoint samples[], int no_samples, sEllipse &model)
{
	// 타원 방정식: x^2 + axy + by^2 + cx + dy + e = 0
	dMatrix A(no_samples, 5);
	dMatrix B(no_samples, 1);

	for (int i=0; i<no_samples; i++) {
		double &x = samples[i].x;
		double &y = samples[i].y;

		A(i, 0) = x*y;
		A(i, 1) = y*y;
		A(i, 2) = x;
		A(i, 3) = y;
		A(i, 4) = 1.;

		B(i, 0) = -x*x;
	}

	// AX=B 형태의 해를 least squares solution으로 구하기 위해
	// Moore-Penrose pseudo-inverse를 이용한다.
	dMatrix invA = !(~A*A)*~A;
	dMatrix X = invA*B;

	// c가 1보다 클 때는 c를 1이 되도록 ratio 값을 곱해준다.
	double ratio = (X(1,0) > 1.) ? 1./X(1,0) : 1.;

	model.a = ratio*1.;
	model.b = ratio*X(0,0);
	model.c = ratio*X(1,0);
	model.d = ratio*X(2,0);
	model.e = ratio*X(3,0);
	model.f = ratio*X(4,0);

	return 1;
}

double compute_distance(sEllipse &ellipse, sPoint &p)
{
	// 한 점 p에서 타원에 내린 수선의 길이를 계산하기 힘들다.
	// 부정확하지만, 간단히 하기위하여 대수적 거리를 계산한다.
	double &x = p.x;
	double &y = p.y;

	double e = fabs(ellipse.a*x*x + ellipse.b*x*y + ellipse.c*y*y + ellipse.d*x + ellipse.e*y + ellipse.f);
	return sqrt(e);
}

double model_verification (sPoint *inliers, int *no_inliers, sEllipse &estimated_model, sPoint *data, int no_data, double distance_threshold)
{
	*no_inliers = 0;

	double cost = 0.;

	for(int i=0; i<no_data; i++){
		// 직선에 내린 수선의 길이를 계산한다.
		double distance = compute_distance(estimated_model, data[i]);
	
		// 예측된 모델에서 유효한 데이터인 경우, 유효한 데이터 집합에 더한다.
		if (distance < distance_threshold) {
			cost += 1.;

			inliers[*no_inliers] = data[i];
			++(*no_inliers);
		}
	}

	return cost;
}


double ransac_ellipse_fitting(sPoint *data, int no_data, sEllipse &model, double distance_threshold)
{
	const int no_samples = 5;

	if (no_data < no_samples) {
		return 0.;
	}

	sPoint *samples = new sPoint[no_samples];

	int no_inliers = 0;
	sPoint *inliers = new sPoint[no_data];

	sEllipse estimated_model;
	double max_cost = 0.;

	int max_iteration = (int)(1 + log(1. - 0.99)/log(1. - pow(0.5, no_samples)));

	for (int i = 0; i<max_iteration; i++) {
		// 1. hypothesis

		// 원본 데이터에서 임의로 N개의 셈플 데이터를 고른다.
		get_samples (samples, no_samples, data, no_data);

		// 이 데이터를 정상적인 데이터로 보고 모델 파라메터를 예측한다.
		compute_model_parameter (samples, no_samples, estimated_model);
		if (!estimated_model.convert_std_form ()) { --i; continue; }

		// 2. Verification

		// 원본 데이터가 예측된 모델에 잘 맞는지 검사한다.
		double cost = model_verification (inliers, &no_inliers, estimated_model, data, no_data, distance_threshold);

		// 만일 예측된 모델이 잘 맞는다면, 이 모델에 대한 유효한 데이터로 새로운 모델을 구한다.
		if (max_cost < cost) {
			max_cost = cost;
	
			compute_model_parameter (inliers, no_inliers, model);
			model.convert_std_form ();
		}
	}
	
	delete [] samples;
	delete [] inliers;

	return max_cost;
}
