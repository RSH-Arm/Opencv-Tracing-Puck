#ifndef AUTO_GRID
#define AUTO_GRID


#include <opencv2/opencv.hpp>

//*****************************************************************//
// @param	ver		-	Vertical step
// @param	hor		-	Horizontal step
// @param	center	-	The point of intersection of the axes
//*****************************************************************//
struct setting_grid
{
	float ver;
	float hor;
	cv::Point2f center;
};

cv::Point2f crossLine(cv::Point2f&, cv::Point2f&, cv::Point2f&, cv::Point2f&);

setting_grid grid(cv::Mat frame);

#endif //AUTO_GRID