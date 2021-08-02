#include "FrameDiff.h"

Anomaly::Anomaly()
	:m_Anomaly(UnkownObj),
	updateornot(false)
{

}

Anomaly::~Anomaly()
{

}

Anomaly::Anomaly(cv::Mat m_background, ObjectStatus m_status)
{
	imgback = m_background.clone();
	m_Anomaly = m_status;
}

Anomaly Anomaly::operator=(const Anomaly& obj)
{
	Anomaly tobj;
	tobj.imgback = obj.imgback.clone();
	tobj.m_Anomaly = obj.m_Anomaly;
	return tobj;
}

bool Anomaly::DetectShadow(cv::Mat& src)
{
	
	if (!src.data)
	{
		printf("Err: Path error \ n");
		return false;
	}
	imshow("1 src", src);

	//Regulate the window size according to the image size
	int n = 11;

	// Do the maximum filtering of the original grayscale map (not expanded), get maxfiltermat_a
	Mat element = getStructuringElement(cv::MorphShapes::MORPH_RECT, cv::Size(n, n));
	int iteration = 1;
	

	// Do you minimize the size of maxfiltermat_a (isn't it corroded)
	Mat maxFilterMat_A;
	cv::morphologyEx(src, maxFilterMat_A, MORPH_DILATE, element, cv::Point(-1, -1), iteration, cv::BORDER_CONSTANT, cv::morphologyDefaultBorderValue());
	imshow("2 Maximum Filter", maxFilterMat_A);

	// Do you minimize the size of maxfiltermat_a (isn't it corroded)
	Mat minFilterMat_B;
	cv::morphologyEx(maxFilterMat_A, minFilterMat_B, MORPH_ERODE, element, cv::Point(-1, -1), iteration, cv::BORDER_CONSTANT, cv::morphologyDefaultBorderValue());
	imshow("3 first maximum and then minimum filtering", minFilterMat_B);

	


	// (First maximum and then minimum filtering) - Original grayscale
	Mat diffMat = minFilterMat_B - src;
	imshow("4 minus the original picture", diffMat);
	// Reverse, get a black word on white
	diffMat = ~diffMat;
	imshow("5 minus the original picture and then take anti-", diffMat);
	Mat normalizeMat;
	normalize(diffMat, normalizeMat, 0, 255, NORM_MINMAX);
	imshow("6 results normalized to 0-255", normalizeMat);
	cv::waitKey(0);
	return true;
}


bool Anomaly::PointsinRegion(std::vector<cv::Point>& pt, const std::vector<cv::Point>& polygons)
{
	if (pt.size()<4)
	{
		printf("input rect failure!!<<<<<<");
		return false;
	}
	int numofNonintersection = 0;

	for (int j = 0; j < pt.size(); j++)
	{
	int nCross = 0;    // ���������ͳ��Ŀ������һ������������ཻ����
	for (int i = 0; i < polygons.size(); i++) 
	{   //���������ÿһ���ڵ�

		cv::Point p1;
		cv::Point p2;

		p1 = polygons[i];
		p2 = polygons[(i + 1) % polygons.size()];  // p1������ڵ㣬p2����һ���ڵ㣬���������Ƕ���ε�һ����
// �����㷨����������y���������жϵ�

		if (p1.y == p2.y)
			continue;   //�����������ˮƽ�ģ�����

		
			if (pt[j].y < min(p1.y, p2.y)) //���Ŀ����������߶Σ�����
				continue;

			if (pt[j].y >= max(p1.y, p2.y)) //���Ŀ����������߶Σ�����
				continue;
			//��ô�����������ǣ������p1��ˮƽ�ߣ���p2��ˮƽ�ߣ�Ŀ��������������м�
			double x = (double)(pt[j].y - p1.y) * (double)(p2.x - p1.x) / (double)(p2.y - p1.y) + p1.x;
			// ��εļ��������� ��Ŀ��㣬��һ��ˮƽ�ߣ�x�������������ε�ǰ�ߵĽ���x����
			if (x > pt[j].x)
				nCross++; //����������ұߣ�ͳ�Ƽ�һ������ڴ�Ŀ������ҷ�һ�����ߣ�ray���������θ��ߵ��ཻ��crossing������
		}

	if (nCross % 2 == 1) {

		return true; //�����������˵���ڶ������
	}
	else {
		//numofNonintersection++;
		//return false; //�����ڶ������ �� ����
	}
	}
	return false;
}

void Anomaly::UpdateBack(cv::Mat& background, bool update)
{
	if (!update)
	{
		return;
	}
	else
	{
		printf(">>>>>>Update background now!!!!<<<<<<<<<<< \n");
		printf(">>>>>>Update background now!!!!<<<<<<<<<<< \n");
		printf(">>>>>>Update background now!!!!<<<<<<<<<<< \n");
		printf(">>>>>>Update background now!!!!<<<<<<<<<<< \n");
		background.copyTo(imgback);
	}
}

bool Anomaly::FindDiff(cv::Mat& CurrentFrame, std::vector<BoundingBox>& yolov5, \
	std::vector<cv::Rect>& Suspectedobj)
{
	if ((imgback.rows != CurrentFrame.rows) || (imgback.cols != CurrentFrame.cols))
	{
		if (imgback.rows > CurrentFrame.rows)
		{
			cv::resize(imgback, imgback, CurrentFrame.size(), 0, 0, cv::INTER_LINEAR);
		}
		else if (imgback.rows < CurrentFrame.rows)
		{
			cv::resize(CurrentFrame, CurrentFrame, imgback.size(), 0, 0, cv::INTER_LINEAR);
		}
	}

	cv::Mat image1_gary, image2_gary;
	if (imgback.channels() != 1)
	{
		cvtColor(imgback, image1_gary, cv::COLOR_BGR2GRAY);
	}
	if (CurrentFrame.channels() != 1)
	{
		cvtColor(CurrentFrame, image2_gary, cv::COLOR_BGR2GRAY);
	}

	cv::Mat frameDifference, absFrameDifferece;
	cv::Mat previousGrayFrame = image2_gary.clone();
	//ͼ1��ͼ2
	subtract(image1_gary, image2_gary, frameDifference, cv::Mat(), CV_16SC1);

	//ȡ����ֵ
	absFrameDifferece = abs(frameDifference);

	//λ��ĸı�
	absFrameDifferece.convertTo(absFrameDifferece, CV_8UC1, 1, 0);
	//cv::imshow("absFrameDifferece", absFrameDifferece);
	cv::Mat segmentation;

	//��ֵ������һ���ܹؼ���Ҫ���ö�ֵ����ֵ��
	threshold(absFrameDifferece, segmentation, 50, 255, cv::THRESH_BINARY);

	//��ֵ�˲�
	medianBlur(segmentation, segmentation, 5);

	//��̬ѧ����(��������)
	//��̬ѧ�����õ�������
	cv::Mat morphologyKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(-1, -1));
	morphologyEx(segmentation, segmentation, cv::MORPH_CLOSE, morphologyKernel, cv::Point(-1, -1), 2, cv::BORDER_REPLICATE);

	//��ʾ��ֵ��ͼƬ
	//cv::imshow("segmentation", segmentation);

	//�ұ߽�
	std::vector< std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	findContours(segmentation, contours, hierarchy, 0, 2, cv::Point(0, 0));//CV_RETR_TREE
	std::vector< std::vector<cv::Point> > contours_poly(contours.size());

	
	// ��Ҫ��������н�����ȥ��yolov5̽���Ŀ��

	std::vector<BoundingBox> v_temp;

	for (int index = 0; index < contours.size(); index++)
	{
		approxPolyDP(cv::Mat(contours[index]), contours_poly[index], 3, true);
		cv::Rect rect = cv::boundingRect(cv::Mat(contours_poly[index]));
		BoundingBox tempresults;
		tempresults.x = rect.x;
		tempresults.y = rect.y;
		tempresults.w = rect.width;
		tempresults.h = rect.height;
		v_temp.push_back(tempresults);
		//Suspectedobj.push_back(rect);
		//cv::rectangle(CurrentFrame, rect, cv::Scalar(0, 255, 0), 2);
	}



	// �Ƴ��ཻ��objs
	
	char yichu[255];
	for (int i = 0; i < v_temp.size(); i++)
	{
		cv::Rect finalresult;
		int indexofmatch = highestIOU(v_temp[i], yolov5);
		// �ж��Ƿ��˶�������yolov5 ���ճ��������ǣ��򲻼�������׷��iou
		if (indexofmatch != -1 \
			&& intersectionOverUnion(v_temp[i], yolov5[indexofmatch]) >= 0.05)
		{
			v_temp[i].m_status = Ejected;
			rectangle(CurrentFrame,
				Point(v_temp[i].x, v_temp[i].y),
				Point(v_temp[i].x + v_temp[i].w,
					v_temp[i].y + v_temp[i].h),
				Scalar(0, 255, 0), 2, 8);

			sprintf(yichu, "Ejected");

			cv::putText(CurrentFrame, yichu,
				cv::Point((v_temp[i].x + v_temp[i].w - v_temp[i].w / 2) - 30,
					v_temp[i].y + v_temp[i].h + 10),
				1,
				1.2,
				Scalar(0, 255, 0),
				1.2, LINE_4);

			v_temp.erase(v_temp.begin() + i);
		}
		else
		{
			v_temp[i].m_status = Suspected;
			finalresult.x = (int)v_temp[i].x;
			finalresult.x = (int)v_temp[i].y;
			finalresult.width = (int)v_temp[i].w;
			finalresult.height = (int)v_temp[i].h;
			if (finalresult.width > 10 && finalresult.height > 10)
			{
				Suspectedobj.push_back(finalresult);
				sprintf(yichu, "Suspected");
				rectangle(CurrentFrame,
					Point(v_temp[i].x, v_temp[i].y),
					Point(v_temp[i].x + v_temp[i].w,
						v_temp[i].y + v_temp[i].h),
					Scalar(0, 0, 255), 2, 8);
				cv::putText(CurrentFrame, yichu,
					cv::Point((v_temp[i].x + v_temp[i].w - v_temp[i].w / 2) - 35,
						v_temp[i].y + v_temp[i].h + 15),
					1,
					1.2,
					Scalar(0, 0, 255),
					2, LINE_4);
			}
			else
			{
				continue;
			}
			
		}
	}



	imshow("Ч��ͼ", CurrentFrame);
	cv::waitKey(5);
	if (Suspectedobj.empty())
	4
		return false;
	}
	return true;

}