#include "tracking.h"
#include <iostream>


int main()
{
	cv::VideoCapture video("C:\\Users\\Colonel\\Desktop\\Detect\\v1.mp4");

	if (!video.isOpened()) return -1;

	int sections;
	{
		const auto default_precision{ std::cout.precision() };

		cv::Mat frame;
		video.read(frame);
		video.release();

		float frameBytes = frame.total() * frame.elemSize();
		std::cout << "Set buffer :		" << frameBytes / (1024.0f) << "(Kb)" << std::endl;
		std::cin >> sections;
		float totalBytes = sections * (frameBytes / (1024)) / (1024 * 1024);
		std::cout << "Total buffer :		" << std::setprecision(2) << totalBytes << "(Gb)" << std::endl;
		std::cout << std::setprecision(default_precision);
	}

	video.open("C:\\Users\\Colonel\\Desktop\\Detect\\v1.mp4");


	auto start = std::chrono::steady_clock::now();

	TrackingPucks pt(video, sections);

	auto end = std::chrono::steady_clock::now();
	auto diff = end - start;
	std::cout << std::chrono::duration<double, std::milli>(diff).count() << " ms" << std::endl;

}