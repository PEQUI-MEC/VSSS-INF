#ifndef EQUATIONS_HPP_
#define EQUATIONS_HPP_

#include "opencv2/opencv.hpp"

class EQ {
public:
    /**
	 * Calculates the euclidean distance between two given points
	 * @param a First point
	 * @param b Second point
	 * @return Linear distance between a and b
	 */
    static double distance(cv::Point a, cv::Point b) {
        return sqrt(pow(double(b.x - a.x), 2) + pow(double(b.y - a.y), 2));
    }
};

#endif
