
#include "StdAfx.h"

#include "fastRect.h"
#include "Matrix.h"
#include <immintrin.h>

#define _USE_MATH_DEFINES

std::vector<std::vector<fast::calPoint>> fast::rectROI(int center_x, int center_y, double angle, int range, int distance, bool direction, int skip_pixels) {



    if (range == 0) {
        HVERROR(error, "Invalid box roi range");
    }

    if (distance == 0) {
        HVERROR(error, "Invalid box distance");
    }

    if (skip_pixels == 0) {
        HVERROR(error, "Invalid search step");
    }


    int dimension = range * distance;

    int roi_mid_width = range / 2;
    int roi_mid_height = distance / 2;


    int start_x1 = 0;
    int start_y1 = 0;
    int end_x1 = 0;
    int end_y1 = 0;


    int start_x2 = 0;
    int start_y2 = 0;
    int end_x2 = 0;
    int end_y2 = 0;


    if (direction == false) {
        start_x1 = center_x - roi_mid_width;
        start_y1 = center_y + roi_mid_height;

        start_x2 = center_x + roi_mid_width;
        start_y2 = center_y + roi_mid_height;

        end_x1 = center_x - roi_mid_width;
        end_y1 = center_y - roi_mid_height;

        end_x2 = center_x + roi_mid_width;
        end_y2 = center_y - roi_mid_height;
    }
    else {
        start_x1 = center_x - roi_mid_width;
        start_y1 = center_y - roi_mid_height;

        start_x2 = center_x + roi_mid_width;
        start_y2 = center_y - roi_mid_height;

        end_x1 = center_x - roi_mid_width;
        end_y1 = center_y + roi_mid_height;

        end_x2 = center_x + roi_mid_width;
        end_y2 = center_y + roi_mid_height;
    }




    int size = 4;

	h_Matrix<double> trigonometric_matrix(2, 2);

    //trigonometric_matrix
    trigonometric_matrix.at(0, 0) = cos(angle * M_PI / 180);
    trigonometric_matrix.at(0, 1) = -sin(angle * M_PI / 180);
    trigonometric_matrix.at(1, 0) = sin(angle * M_PI / 180);
    trigonometric_matrix.at(1, 1) = cos(angle * M_PI / 180);

    //rotation_matrix
	h_Matrix<double> rotation_matrix(2, size);

    //base_matrix
	h_Matrix<double> base_matrix(2, size);

    rotation_matrix.at(0, 0) = start_x1 - center_x; // x           Start X1
    rotation_matrix.at(1, 0) = start_y1 - center_y; // y

    rotation_matrix.at(0, 1) = start_x2 - center_x; // x
    rotation_matrix.at(1, 1) = start_y2 - center_y; // y

    rotation_matrix.at(0, 2) = end_x1 - center_x;// x
    rotation_matrix.at(1, 2) = end_y1 - center_y; // y

    rotation_matrix.at(0, 3) = end_x2 - center_x; // x
    rotation_matrix.at(1, 3) = end_y2 - center_y; /// y

    // No problem
    for (int index = 0; index < 4; index++) {
        base_matrix.at(0, index) = center_x;
        base_matrix.at(1, index) = center_y;
    }

	h_Matrix<double> result(3, 3);
    result = trigonometric_matrix * rotation_matrix + base_matrix;


    double rotated_start_x1 = result.at(0, 0);
    double rotated_start_y1 = result.at(1, 0);

    double rotated_start_x2 = result.at(0, 1);
    double rotated_start_y2 = result.at(1, 1);

    double rotated_end_x1 = result.at(0, 2);
    double rotated_end_y1 = result.at(1, 2);

    double rotated_end_x2 = result.at(0, 3);
    double rotated_end_y2 = result.at(1, 3);


    //startx1 = rotated_start_x1;
    //starty1 = rotated_start_y1;
    //startx2 = rotated_start_x2;
    //starty2 = rotated_start_y2;
    //endx1 = rotated_end_x1;
    //endy1 = rotated_end_y1;
    //endx2 = rotated_end_x2;
    //endy2 = rotated_end_y2;

    double diff_start_x = rotated_start_x2 - rotated_start_x1;
    double diff_start_y = rotated_start_y2 - rotated_start_y1;

    double increase_rate_x = diff_start_x / range;
    double increase_rate_y = diff_start_y / range;


    //Start Line Points
    //Start Line Points
    //Start Line Points
    //Start Line Points
    int range_align_size = (range)+(range % 4);
    std::vector<double> range_vec(range_align_size * 2);
    for (int index = 0; index < range * 2; index += 2) {
        int current_index = index;
        range_vec[index] = current_index / 2;
        range_vec[index + 1] = current_index / 2;
    }

    std::vector<fast::calPoint> start_line_xy(range_align_size);
    std::vector<fast::calPoint> end_line_xy(range_align_size);

    std::vector<double> increase_rate_xy_vec = { increase_rate_x, increase_rate_y,increase_rate_x, increase_rate_y };
    const __m256d simd_increase_rate_xy = _mm256_load_pd(increase_rate_xy_vec.data());

    std::vector<double> simd_start_xy_vec = { rotated_start_x1 , rotated_start_y1, rotated_start_x1 , rotated_start_y1 };
    const __m256d simd_start_xy = _mm256_load_pd(simd_start_xy_vec.data());

    std::vector<double> simd_end_xy_vec = { rotated_end_x1 , rotated_end_y1, rotated_end_x1 , rotated_end_y1 };
    const __m256d simd_end_xy = _mm256_load_pd(simd_end_xy_vec.data());


    const double* range_ptr = &range_vec[0];

    const fast::calPoint* start_result_xy_ptr = start_line_xy.data();
    const fast::calPoint* end_result_xy_ptr = end_line_xy.data();

    int chunk_size = sizeof(double) * 4;
    for (int index = 0; index < range * 2; index += 4) {

        __m256d range_chunk = _mm256_load_pd(range_ptr + index);

        __m256d chunk_mul = _mm256_mul_pd(simd_increase_rate_xy, range_chunk);

        // Start Line
        __m256d start_result = _mm256_add_pd(chunk_mul, simd_start_xy);

        // End Line
        __m256d end_result = _mm256_add_pd(chunk_mul, simd_end_xy);
        memcpy(((double*)start_result_xy_ptr + index), &start_result, chunk_size);
        memcpy(((double*)end_result_xy_ptr + index), &end_result, chunk_size);
    }

    //Start Line Points
    //Start Line Points
    //Start Line Points
    //Start Line Points

    int distance_align_size = (distance)+(distance % 4);
    std::vector<double> distance_vec(distance_align_size * 2);
    for (int index = 0; index < distance * 2; index += 2) {
        int current_index = index;
        distance_vec[index] = current_index / 2;
        distance_vec[index + 1] = current_index / 2;
    }

    std::vector<std::vector<fast::calPoint>> combine_vertical_xy;

    double diff_virtical_start_x = rotated_end_x1 - rotated_start_x1;
    double diff_virtical_start_y = rotated_end_y1 - rotated_start_y1;

    double increase_virtical_rate_x = diff_virtical_start_x / distance;
    double increase_virtical_rate_y = diff_virtical_start_y / distance;

    std::vector<double> increase_vertical_rate_xy_vec = { increase_virtical_rate_x, increase_virtical_rate_y,increase_virtical_rate_x, increase_virtical_rate_y };
    const __m256d simd_increase_vertical_rate_xy = _mm256_load_pd(increase_vertical_rate_xy_vec.data());

    for (int range_index = 0; range_index < range; range_index += skip_pixels) {

        auto start_vertical_point = start_line_xy[range_index];
        std::vector<double> simd_vertical_xy_vec = { start_vertical_point.x , start_vertical_point.y, start_vertical_point.x , start_vertical_point.y };
        const __m256d simd_vertical_xy = _mm256_load_pd(simd_vertical_xy_vec.data());

        const double* distance_ptr = &distance_vec[0];

        std::vector<fast::calPoint> vertical_xy;
        vertical_xy.resize(distance_align_size);
        const fast::calPoint* start_vertical_xy_ptr = vertical_xy.data();

        for (int index = 0; index < distance * 2; index += 4) {
            __m256d distance_chunk = _mm256_load_pd(distance_ptr + index);
            __m256d chunk_mul = _mm256_mul_pd(simd_increase_vertical_rate_xy, distance_chunk);
            __m256d start_result = _mm256_add_pd(chunk_mul, simd_vertical_xy);
            memcpy((((double*)start_vertical_xy_ptr) + index), &start_result, chunk_size);
        }
        combine_vertical_xy.push_back(vertical_xy);
    }

    return combine_vertical_xy;
}

std::vector<std::vector<fast::calPoint>> fast::fastDonut(int center_x, int center_y, int radius, double start_ratio, double end_ratio, double step_angle) {

	if (radius == 0) {
		HVERROR(error, "Invalid box roi range");
	}

	if (step_angle <= 0) {
		HVERROR(error, "Invalid step angle");
	}

	if (start_ratio <= 0 && end_ratio <= 0) {
		HVERROR(error, "Invalid ratio");
	}



	int start_radius = (int)((double)radius * start_ratio);
	int end_radius = (int)((double)radius * end_ratio);

	int distance_radius = (int)fabs(start_radius - end_radius);
	if (distance_radius == 0) {
		HVERROR(error, "Invalid start ratio and end ratio");
	}

	int aligned_distance_radius = distance_radius + (distance_radius % 4);
	int min_radius = start_radius > end_radius ? end_radius : start_radius;
	int max_radius = start_radius > end_radius ? start_radius : end_radius;


	/// Radius SIMD 
	std::vector<double> vector_radius; // Radius vector
	vector_radius.resize(aligned_distance_radius * 2);


	int sign = 1;
	if (start_radius > end_radius)
		sign = -1;

	int current_radius = start_radius;
	for (int radius = 0; radius < distance_radius * 2;) {
		vector_radius[radius] = current_radius;
		vector_radius[radius + 1] = current_radius;
		radius += 2;
		current_radius += sign;
	}

	/// Coordinate SIMD 
	std::vector<double> vector_cordinate_table = { (double)center_x , (double)center_y, (double)center_x , (double)center_y }; // cordinate vector
	const __m256d simd_coordinate = _mm256_load_pd(vector_cordinate_table.data()); // cordinate simd



	std::vector<std::vector<fast::calPoint>> combine_vertical_xy;
	int doubleDistance = distance_radius * 2;
	int chunk_size = sizeof(double) * 4;

	for (double angle = 0; angle < 360;) {

		/// trigonometric SIMD 
		double x_cos = sin(angle * M_PI / 180);
		double y_sine = cos(angle * M_PI / 180);
		std::vector<double> vector_trigonometric_table = { x_cos ,y_sine, x_cos ,y_sine }; // trigonometric vector
		const __m256d simd_trigonometric = _mm256_load_pd(vector_trigonometric_table.data()); // trigonometric simd


		std::vector<fast::calPoint> vertical_xy;
		vertical_xy.resize(aligned_distance_radius);
		void* vertical_xy_ptr = vertical_xy.data();
		for (int simd_skip = 0; simd_skip < doubleDistance;) {

			const __m256d simd_radius = _mm256_load_pd(vector_radius.data() + simd_skip); // radius simd
			__m256d chunk_mul = _mm256_mul_pd(simd_radius, simd_trigonometric);
			__m256d start_result = _mm256_add_pd(chunk_mul, simd_coordinate);
			memcpy((((double*)vertical_xy_ptr) + simd_skip), &start_result, chunk_size);
			simd_skip += 4;
		}
		vertical_xy.resize(distance_radius);
		combine_vertical_xy.push_back(vertical_xy);

		angle += step_angle;
	}

	return combine_vertical_xy;
}
