#include "auto_grid.h"

#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;


Point2f crossLine(Point2f& p1b, Point2f& p1e, Point2f& p2b, Point2f& p2e)
{
	float n;
	if (p1e.y - p1b.y != 0) {  // a(y)
		float q = (p1e.x - p1b.x) / (p1b.y - p1e.y);
		float sn = (p2b.x - p2e.x) + (p2b.y - p2e.y) * q; if (!sn) { return { -1, -1 }; }
		float fn = (p2b.x - p1b.x) + (p2b.y - p1b.y) * q;
		n = fn / sn;
	}
	else {
		if (!(p2b.y - p2e.y)) { return { -1, -1 }; }
		n = (p2b.y - p1b.y) / (p2b.y - p2e.y);
	}
	return { p2b.x + (p2e.x - p2b.x) * n, p2b.y + (p2e.y - p2b.y) * n };
}

setting_grid grid(cv::Mat frame)
{
	Mat src;

	cvtColor(frame, src, COLOR_BGR2GRAY);

	int framewidth = src.size().width;		// ширина
	int frameheight = src.size().height;	// высота


	int width_base = src.size().width / 3;
	int height_base = src.size().height / 3;

	Mat dst, cdst, cdstP;


	Canny(src, dst, 50, 200, 3);
	cvtColor(src, cdst, COLOR_GRAY2BGR);

	vector<Vec2f> lines;
	HoughLines(dst, lines, 1, CV_PI / 180, 100, 0, 0);


	int hor = 0, vert = 0;
	Point2f	hor1{ 0, 0 },
		hor2{ (float)framewidth, 0 },
		ver1{ 0, 0 },
		ver2{ 0, (float)frameheight };

	// Draw the lines
	for (size_t i = 0; i < lines.size(); i++)
	{
		float rho = lines[i][0], theta = lines[i][1];
		Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a * rho, y0 = b * rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));

		if (pt1.y > height_base && pt1.y < frameheight - height_base && pt2.y > height_base && pt2.y < frameheight - height_base)
		{
			hor1.y += pt1.y;
			hor2.y += pt2.y;
			hor++;
			continue;
		}
		if (pt1.x > width_base && pt1.x < framewidth - width_base && pt2.x > width_base && pt2.x < framewidth - width_base)
		{
			ver1.x += pt1.x;
			ver2.x += pt2.x;
			vert++;
			continue;
		}
	}

	hor1.y = hor1.y / hor;
	hor2.y = hor2.y / hor;;
	ver1.x = ver1.x / vert;
	ver2.x = ver2.x / vert;

	auto pp = move(crossLine(hor1, hor2, ver1, ver2));

	int BigRadius = 80;
	int SmallRadius = 20;

	Mat src_gray, src_2;
	int thresh = 140;
	Mat dst_2, dst_norm, dst_norm_scaled;

	src_2 = frame.clone();
	Mat central_region = src_2(Rect(pp.x - BigRadius, pp.y - BigRadius, BigRadius * 2, BigRadius * 2));
	src_2 = central_region.clone();
	cvtColor(src_2, src_gray, COLOR_BGR2GRAY);

	dst_2 = Mat::zeros(src_2.size(), CV_32FC1);

	/// Detector parameters
	int blockSize = 2;
	int apertureSize = 3;
	double k = 0.04;

	/// Detecting corners
	cornerHarris(src_gray, dst_2, blockSize, apertureSize, k, BORDER_DEFAULT);
	normalize(dst_2, dst_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
	convertScaleAbs(dst_norm, dst_norm_scaled);

	Point2f circl_1{ 0,0 }, circl_2{ 0,0 }, center_0{ 0,0 };
	int circl_1_count = 0, circl_2_count = 0, center_0_count = 0;
	float step_hor, step_ver;

	bool flag = true;
	long long int counter_temp = 0;

	/// Drawing a circle around corners
	for (int j = 0; j < dst_norm.rows; j++)
	{
		for (int i = 0; i < dst_norm.cols; i++)
		{
			if (flag) {
				for (int k = 0; k != 50; k++)
				{
					counter_temp += (int)dst_norm.at<float>(j, i);

				}
				thresh = counter_temp / 50;
				flag = false;
			}

			if ((int)dst_norm.at<float>(j, i) > thresh)
			{
				if (pp.x - BigRadius + i < pp.x + SmallRadius && pp.x - BigRadius + i > pp.x - SmallRadius && pp.y - BigRadius + j > pp.y - SmallRadius && pp.y - BigRadius + j < pp.y + SmallRadius)
				{
					center_0.x += pp.x - BigRadius + i;
					center_0.y += pp.y - BigRadius + j;

					center_0_count++;
					continue;
				}

				// vertical
				if (pp.x - BigRadius + i < pp.x - SmallRadius && pp.y - BigRadius + j  > pp.y - SmallRadius && pp.y - BigRadius + j < pp.y + SmallRadius)
				{
					circl_1.x += pp.x - BigRadius + i;
					circl_1.y += pp.y - BigRadius + j;
					circl_1_count++;
					continue;
				}
				// horizontal
				if (pp.y - BigRadius + j > pp.y + SmallRadius && pp.x - BigRadius + i > pp.x - SmallRadius && pp.x - BigRadius + i < pp.x + SmallRadius)
				{
					circl_2.x += pp.x - BigRadius + i;
					circl_2.y += pp.y - BigRadius + j;
					circl_2_count++;
					continue;
				}
			}
		}
	}

	circl_1.x = circl_1.x / circl_1_count;
	circl_1.y = circl_1.y / circl_1_count;
	step_ver = pp.x - circl_1.x;

	circl_2.x = circl_2.x / circl_2_count;
	circl_2.y = circl_2.y / circl_2_count;
	step_hor = circl_2.y - pp.y;

	center_0.x = center_0.x / center_0_count;
	center_0.y = center_0.y / center_0_count;

	return setting_grid{ step_ver, step_hor, center_0 };
}
