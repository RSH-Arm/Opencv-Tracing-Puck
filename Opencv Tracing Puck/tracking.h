#ifndef TRACKING
#define TRACKING

#include <opencv2/opencv.hpp>
#include "opencv2/highgui.hpp"
#include "opencv2/tracking.hpp"
#include "MultiBuffers.h"

#include <iostream>
#include "auto_grid.h"

using namespace std;
using namespace cv;

class Flag
{
public:
	Flag() : flag_{ false } {}

	void set()
	{
		{
			std::lock_guard<std::mutex> g(mutex_);
			flag_ = true;
		}
		cond_var_.notify_all();
	}

	void clear()
	{
		std::lock_guard<std::mutex> g(mutex_);
		flag_ = false;
	}

	void wait()
	{
		std::unique_lock<std::mutex> lock(mutex_);
		cond_var_.wait(lock, [this]() { return flag_; });
	}
private:
	bool flag_;
	std::mutex mutex_;
	std::condition_variable cond_var_;
};

class TrackingPucks
{
	void tracking_puck(int);
	void rectDraw();

public:
	TrackingPucks(cv::VideoCapture&, int);

	void startThreads(cv::Mat&);
	void startPosition(cv::Mat&);

private:

	setting_grid s_grid;

	double	_FPS,
			_STOP,
			_Square;

	float	_Radius;

	bool	flag_end = true;

	int		_Sections,
			_Listeners;

	vector<Rect>					pool_point_puck;
	vector<thread>					pool_thread;
	vector<cv::Ptr<cv::Tracker>>	pool_tracker;

	multi::MultiPipeline_RW<cv::Mat, cv::Rect> video_queue;
};

#endif //TRACKING