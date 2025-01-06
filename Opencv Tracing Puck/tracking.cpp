
#include "tracking.h"
#include <chrono>
//#include <my/multi_buffers/MultiBuffers.h>

using namespace std;
using namespace cv;

///
/// Class --TrackingPucks-- Begin
///
TrackingPucks::TrackingPucks(cv::VideoCapture& video_, int sections_)
{
	//*** Initialization ***
	_FPS = video_.get(CAP_PROP_FPS);
	_STOP = video_.get(CAP_PROP_FRAME_COUNT);
	_Sections = sections_;

	cout << "_FPS:	"	<< _FPS << endl;
	cout << "_STOP:	"	<< _STOP << endl;

	//*** Reading the first frame ***
	cv::Mat frame;
	video_.read(frame);

	//*** Initializes pool_point_puck ***
	startPosition(frame);

	//*** Initializes grid ***
	s_grid = grid(frame);

	_Listeners = pool_point_puck.size();
	video_queue.start(_Sections, _Listeners);
	video_queue.setRaw(std::move(frame.clone()));

	//*** Start threads ***
	startThreads(frame);

	std::thread th_loop(&TrackingPucks::rectDraw, this);


	int i = 1;
	while (i < _STOP)
	{
		if (video_queue.canRecord())
		{
			if (video_.read(frame))
			{
				video_queue.setRaw(std::move(frame.clone()));
				i++;
			}
			else
			{
				break;
			}
		}
	}
	cout << "num: " << i << endl;

	th_loop.join();
	for (auto& x : pool_thread)
	{
		x.join();
	}

	video_.release();
};

void TrackingPucks::rectDraw()
{
	std::vector<cv::Rect> rec(_Listeners);
	cv::Mat frame;

	int i = 0;


	std::this_thread::sleep_for(std::chrono::seconds(5));


	while (i < _STOP)
	{
		if (video_queue.getResult(frame, rec))
		{
			for (auto& rect : rec)
			{
				cv::rectangle(frame, rect, cv::Scalar(255, 0, 0));

				Point2f center;
				center.x = rect.x + rect.width / 2;
				center.y = rect.y + rect.height / 2;


				line(frame, center, Point2f{ s_grid.center.x, center.y }, Scalar(0, 0, 255), 1, LINE_AA);	// X
				line(frame, center, Point2f{ center.x, s_grid.center.y }, Scalar(0, 0, 255), 1, LINE_AA);	// Y

				string str_hor = to_string((center.x - s_grid.center.x) / s_grid.hor);
				string str_ver = to_string((s_grid.center.y - center.y) / s_grid.ver);

				cv::putText(frame,									// target image
					{ "[" + str_hor + ", " + str_ver + "]" },		// text
					cv::Point(rect.x, rect.y),						// top-left position
					cv::FONT_HERSHEY_DUPLEX,
					0.6,											// scale
					CV_RGB(118, 185, 0),							// font color
					1);
			}

			i++;
			cv::imshow("Video", frame);
			cv::waitKey(_FPS);
			video_queue.resultRelease();
		}
	}

	flag_end = false;
}

void TrackingPucks::startThreads(cv::Mat& frame)
{
	pool_tracker.resize(_Listeners);

	try {
		for (int id = 0; id < _Listeners; id++)
		{
			pool_tracker[id] = cv::TrackerCSRT::create();			// TrackerKCF // TrackerCSRT	// TrackerGOTURN // trackermil
			pool_tracker[id]->init(frame, pool_point_puck[id]);

			pool_thread.emplace_back(&TrackingPucks::tracking_puck, this, id);
		}
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	cout << "Start Threads" << endl;
}

void TrackingPucks::tracking_puck(int id_thread)
{
	cv::Mat frame;
	while (true)
	{
		if (video_queue.getRaw(frame, id_thread))
		{
			pool_tracker[id_thread]->update(frame, pool_point_puck[id_thread]);
			video_queue.setResult(pool_point_puck[id_thread], id_thread);
		}
		/**else**/ if (!flag_end) break;
	}
}

void TrackingPucks::startPosition(Mat& frame)
{
	Mat img_gray;
	cvtColor(frame, img_gray, COLOR_BGR2GRAY);

	Mat thresh;
	threshold(img_gray, thresh, 50, 255, THRESH_BINARY);

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(thresh, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);

	// Получаем внутренние контуры
	vector<vector<Point>> internalcontours;
	vector<double> v_area;
	for (size_t i = 0; i < contours.size(); ++i) {
		// Находим ориентацию: cw или cc
		double area = contourArea(contours[i], true);
		if (area >= 0) {
			// Внутренний контур
			internalcontours.push_back(contours[i]);
			v_area.push_back(area);
		}
	}

	int j = 0, i = 0;
	double n = 0;
	for (const auto& v_point : internalcontours) {
		Point2f center;
		float radius;
		minEnclosingCircle(v_point, center, radius);
		radius = ceil(radius);

		if (radius >= 20 && radius < 30)
		{
			_Radius += radius;
			n += v_area[j];
			i++;
			radius = 35;
			pool_point_puck.emplace_back((center.x - radius), (center.y - radius), radius * 2, radius * 2);

		}
		j++;
	}

	_Radius = _Radius / i + 5;
	_Square = n / i;
}

///
/// Class --TrackingPucks-- End
///
