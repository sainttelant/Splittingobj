/*
* Example of using IOU Tracker C++ Implementation
* Author Lucas Wals
*/
#pragma once
#include <fstream>
#include <cstdio>
#include <string>
#include <iomanip>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "dirent.h"
#include "IOUT.h"
#include "UA-DETRAC.h"

/******************************************************************************
* DEFINE SECTION
******************************************************************************/

#define DEFAULT_OUTPUT "../results"
#define DEFAULT_DETECTIONS_TYPE "DETRAC"
#define DEFAULT_DETECTIONS_FILE "detections.txt"
#define SHOW_BOXES 1 // Program will output boxes after finish tracking
#define SAVE_BOXES 0 // Save the boxes on image file
// Turn on/off features depending if this will be embedded in UA-DETRAC toolkit or not
#define USE_IN_DETRAC 1

/******************************************************************************
* EXTRA FUNCTIONS
******************************************************************************/
int pgm_select(const struct dirent *entry)
{
	return strstr(entry->d_name, ".pgm") != NULL;
}

int ppm_select(const struct dirent *entry)
{
	return strstr(entry->d_name, ".ppm") != NULL;
}

int jpg_select(const struct dirent *entry)
{
	return strstr(entry->d_name, ".jpg") != NULL;
}

/******************************************************************************
* MAIN
******************************************************************************/

int main_01(int argc, char *argv[])
{
	std::cout << "IOU C++ implementation" << std::endl;

	// General variables
	std::string input_folder;
	std::string output_folder = DEFAULT_OUTPUT;
	std::string sequence;
	char *detections_type = "DETRAC";
	
	char* detections_file = "D:\\datasets\\MOT17\\MOT17\\train\\MOT17-05-SDP\\det\\det.txt";
	//char *detections_file = "D:\\datasets\\DETRAC-train-data\\Insight-MVT_Annotation_Train\\MVI_20011\\detections.txt";
	// Image files variables
	struct dirent **filelist= NULL;
	int fcount = -1;
	std::vector< Track > drawing_tracks;	// List of Tracks that are being drawed
	// IOUT necessary variables
	std::vector< std::vector<BoundingBox> > detections;	// list of detections
	std::vector< Track > tracks;	// list of resulting tracks
	// TODO: Make a setting struct or initilization for different Detectors!
	// Tracker Configuration for CompACT detector
	//float sigma_l = 0;		// low detection threshold
	//float sigma_h = 0.2;		// high detection threshold
	//float sigma_iou = 0.5;	// IOU threshold
	//float t_min = 5;		// minimum track length in frames

	float stationary_threshold = 0.90;		// low detection threshold,修改一下，这里改成大于这个的是静态物体
	float lazy_threshold = 0.70;
	float sigma_h = 0.7;		// high detection threshold,优选detection，检测的得分，其实这里面没有作用，不是通过classify的来的
	float sigma_iou = 0.2;	// IOU threshold
	float t_min = 3;

	/// Read arguments
#if USE_IN_DETRAC


	sequence = "MVI_20011";
	input_folder = "D:\\datasets\\MOT17\\MOT17\\train\\MOT17-05-SDP\\img1\\";
	
#endif
	

	std::cout << "input folder: " << input_folder << std::endl;

	/// Loading detections part
	// First verify that the detections file exists
	std::ifstream detStream(detections_file);
	if (!detStream || detStream.eof())
	{
		std::cout << "ERROR -> Detection file not found or empty" << std::endl;
//		cv::waitKey(0);
		return 0;
	}
	else
	{
		std::cout << "Detection file found. Loading detections..." << std::endl;
		// Load all detections before starting tracking.
		read_detections(detStream, detections);
	}

	std::cout << "Frames > " << detections.size() << std::endl;
	/*
	std::vector<BoundingBox> test = detections[0];
	for (int i = 0; i < test.size(); i++)
	{
		std::cout << test[i].x << std::endl;
	}
	*/

	//截取前10张图
	std::vector< std::vector<BoundingBox> > m_partdetection;
	
	m_partdetection.assign(detections.begin(), detections.begin() + 15);



#if 1
#if SHOW_BOXES
	/// Looking for the images files
	fcount = scandir(input_folder.c_str(), &filelist, jpg_select, alphasort);
	if (fcount <= 0)
	{
		std::cout << "ERROR -> Input images directory not found or empty" << std::endl;
		cv::waitKey(0);
		return 0;
	}
	else
		std::cout << "Found " << fcount << " images" << std::endl;
#endif // SHOW_BOXES
#endif // NOT USE_IN_DETRAC
	/// Looking for the output directory part. Create it if does not exit
	DIR * dir = opendir(output_folder.c_str());
	if (dir == NULL)
	{
		std::cout << "\tWARNING -> Output folder does not exist -> try to create it" << std::endl;
		if (system(("mkdir " + output_folder).c_str()))
		{
			std::cout << "\tERROR -> Failed to create directory" << std::endl;
			return 0;
		}
	}
	closedir(dir);

	/// Start tracking
	
	tracks = track_iou(stationary_threshold, lazy_threshold, sigma_h, sigma_iou, t_min, m_partdetection);
	std::cout << "Last Track ID > " << tracks.back().id << std::endl;

#if 1
#if SHOW_BOXES
	std::cout << "Displaying results on window..." << std::endl;
	/// Show results on image
	char filename[255];
	cv::Mat image;

	cv::namedWindow("Display Tracking", cv::WINDOW_AUTOSIZE);
	for (int frame = 0; frame < m_partdetection.size(); frame++)
	{
		// Load the current image
		sprintf(filename, "%s/%s", input_folder.c_str(), filelist[frame]->d_name);
		image = cv::imread(filename, cv::IMREAD_COLOR);
		// Grab all the tracks that start in current frame
		for (auto track : tracks)
		{
			if (track.start_frame == frame)
				drawing_tracks.push_back(track);
		}
		// Write all the boxes into the image	
		//std::vector<BoundingBox> frameBoxes = bboxes[frame];
		//for (int j = 0; j < drawing_tracks.size(); j++)
		for (auto dt : drawing_tracks)
		{
			int box_index = frame - dt.start_frame;
			//BoundingBox b = frameBoxes[j];
			if (box_index < dt.boxes.size() )
			{
				BoundingBox b = dt.boxes[box_index];
				cv::rectangle(image, cv::Point(b.x, b.y), cv::Point(b.x + b.w, b.y + b.h), cv::Scalar(0, 0, 255), 2);
				cv::putText(image, std::to_string(dt.id), cv::Point(b.x + b.w - b.w / 2, b.y + b.h - 5), 1, 1, cv::Scalar(0, 255, 255), 2);
			}
//			else
//				drawing_tracks.
		}
#if SAVE_BOXES
		/// Save the images
		sprintf_s(filename, "%s/%s", output_folder.c_str(), filelist[frame]->d_name);
		cv::imwrite(filename, image);
#endif
		imshow("Display Tracking", image);
		cv::waitKey(50);
	}
	std::cout << "Displaying images finished!!" << std::endl;
#endif // SHOW BOXES
#else // Write results only when using in Detrac toolkit
	// detections.size() would be the amount of frames here
	write_results(sequence, output_folder, detections.size(), tracks);
#endif // NOT IN_DETRAC
	write_results(sequence, output_folder, detections.size(), tracks);
	system("PAUSE");
	return 0;
}