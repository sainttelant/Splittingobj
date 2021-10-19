#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace cv;

namespace xueweiImage
{
	struct SplitObject
	{
		cv::Rect m_postion;
		cv::Mat imgdata;
		unsigned int firstshowframenum;
		int ID;
		bool moved;
		bool haschecked;
		int checktimes;
	};


	class ImageAnalysis
	{

	public:
		ImageAnalysis();
		virtual ~ImageAnalysis();
		bool BemovedOut(cv::Mat& patch, cv::Mat& compared, int intervals, int method = 0);

		
		float intersectionOU(cv::Rect box1, cv::Rect box2);
		// Returns the index of the bounding box with the highest IoU

		int CheckHighestIOU(cv::Rect &box, std::vector<SplitObject> &Split);

	protected:
	private:
	};

}



