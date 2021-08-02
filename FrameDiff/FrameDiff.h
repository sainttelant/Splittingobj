#pragma once
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <vector>
#include "IOUT.h"

using namespace cv;

class Anomaly
{
	public:	
		Anomaly();
		Anomaly(cv::Mat m_background, ObjectStatus m_status);
		virtual ~Anomaly();

		// 重写个等号，方便以后抛洒物match之类的使用
		Anomaly operator =(const Anomaly& obj);
		bool FindDiff(cv::Mat& CurrentFrame, std::vector<BoundingBox> &yolov5,\
			std::vector<cv::Rect>& Suspectedobj);
		bool DetectShadow(cv::Mat &m_images);
		bool PointsinRegion(std::vector<cv::Point>& pt, const std::vector<cv::Point>& polygons);
		void UpdateBack(cv::Mat& background, bool update);
	protected:
	private:
		ObjectStatus m_Anomaly;
		cv::Mat imgback;
		bool updateornot;
		
	
};