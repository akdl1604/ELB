#pragma once

#ifndef FAST_RECT
#define FAST_RECT

#include <vector>
#include <memory>

#include "macro.h"
#include "calPoint.h"

namespace fast {
	std::vector<std::vector<fast::calPoint>> rectROI(int centerx, int centery, double angle, int range, int distance, bool direction, int skip_pixels);
    std::vector<std::vector<fast::calPoint>> fastDonut(int center_x, int center_y, int radius, double start_ratio, double end_ratio, double step_angle);
}




#endif