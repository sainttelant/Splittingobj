
#pragma once
#include <algorithm>
#include <iostream>
#include "IOUT.h"

inline float intersectionOverUnion(BoundingBox box1, BoundingBox box2)
{
	float minx1 = box1.x;
	float maxx1 = box1.x + box1.w;
	float miny1 = box1.y;
	float maxy1 = box1.y+ box1.h;

	float minx2 = box2.x;
	float maxx2 = box2.x + box2.w;
	float miny2 = box2.y;
	float maxy2 = box2.y + box2.h;

	if (minx1 > maxx2 || maxx1 < minx2 || miny1 > maxy2 || maxy1 < miny2)
		return 0.0f;
	else
	{
		float dx = std::min(maxx2, maxx1) - std::max(minx2, minx1);
		float dy = std::min(maxy2, maxy1) - std::max(miny2, miny1);
		float area1 = (maxx1 - minx1)*(maxy1 - miny1);
		float area2 = (maxx2 - minx2)*(maxy2 - miny2);
		float inter = dx*dy; // Intersection
		float uni = area1 + area2 - inter; // Union
		float IoU = inter / uni;
		return IoU;
	}
//	return 0.0f;
}

inline int highestIOU(BoundingBox box, std::vector<BoundingBox> boxes)
{
	float iou = 0, highest = 0;
	int index = -1;
	for (int i = 0; i < boxes.size(); i++)
	{
		iou = intersectionOverUnion(box, boxes[i]);
		if ( iou >= highest)
		{
			highest = iou;
			index = i;
		}
	}
	return index;
}

std::vector< Track > track_iou(float status_threshold, float lazy_threshold, \
	float sigma_h, float sigma_iou, float t_min,
	std::vector< std::vector<BoundingBox> > detections)
{
	std::cout << "track_iou function" << std::endl;
	std::vector<Track> active_tracks;
	std::vector<Track> finished_tracks;
	int frame = 0;
	int track_id = 1; // Starting ID for the Tracks
	int index;		// Index of the box with the highest IOU
	bool updated;	// Whether if a track was updated or not
	int numFrames = detections.size();

	
	
	std::cout << "Num of frames > " << numFrames << std::endl;

	// 遍历多帧的探测目标集合
	for (frame; frame < numFrames; frame++)
	{
		int activate_frame_count = 0;
		std::vector<BoundingBox> frameBoxes = detections[frame];

		/// Update active tracks
		
		for (int i = 0 ; i < active_tracks.size() ; i++)
		{
			Track track = active_tracks[i];
			updated = false;
			// Get the index of the detection with the highest IOU
			index = highestIOU(track.boxes.back(), frameBoxes);
			//Check is above the IOU threshold
		

			if ( index != -1 && intersectionOverUnion(track.boxes.back(), frameBoxes[index]) >= sigma_iou )
			{ 
				float iou_score = intersectionOverUnion(track.boxes.back(), frameBoxes[index]);

				// 计算obj的宽高,面积较大的物体，采用更加严格的iou标准
				track.areasize = track.boxes.back().w * track.boxes.back().h;
				if (track.areasize > 2500)
				{
					if (track.first_stationary && iou_score > lazy_threshold+0.15 && iou_score < status_threshold+0.05)
					{
						track.status = Splittingobj;
						track.stationary_count--;
					}
					if (iou_score > status_threshold+0.05)
					{
						track.first_stationary = true;
						track.status = Stopping;
						track.stationary_count++;
					}
					else
					{
						track.status = Moving;
						track.stationary_count = 0;
					}

					if (track.stationary_count > 22)
					{
						track.status = Splittingobj;
					}
				}
				else
				{
					if (track.first_stationary && iou_score > lazy_threshold-0.15 && iou_score < status_threshold-0.05)
					{
						track.status = Splittingobj;
						track.stationary_count--;
					}
					if (iou_score > status_threshold-0.05)
					{
						track.first_stationary = true;
						track.status = Stopping;
						track.stationary_count++;
					}
					else
					{
						track.status = Moving;
						track.stationary_count = 0;
					}

					if (track.stationary_count > 22)
					{
						track.status = Splittingobj;
					}
				}
				
				track.boxes.push_back(frameBoxes[index]);
				if (track.max_score < frameBoxes[index].score)
					track.max_score = frameBoxes[index].score;
				// Remove the best matching detection from the frame detections
				frameBoxes.erase(frameBoxes.begin() + index);
				
				active_tracks[i] = track;
				updated = true;
			}
		
			
			// If the track was not updated...
			if (!updated)
			{
				// Check the conditions to finish the track,有连续t_min 都大于iou阈值，则确定无疑,是追踪目标
				
			
				if (track.max_score >= sigma_h && track.boxes.size() >= t_min)
					finished_tracks.push_back(track);
			

				active_tracks.erase(active_tracks.begin() + i);
				// Workaround used because of the previous line "erase" call
				i--;
			}

		} // End for active tracks

		/// Create new tracks，第一次也初始化这个active_tracks
		for (auto box : frameBoxes)
		{
			std::vector<BoundingBox> b;
			b.push_back(box);
			// Track_id is set to 0 because we dont know if this track will
			// "survive" or not
			Track t = { b, box.score, frame, 0,0,0,Unkown,false,0};
			active_tracks.push_back(t);
		}
//		std::cout << "I tracked frame " << frame << std::endl;
	} // End of frames
	
	/// Finish the remaining tracks
	for (auto track : active_tracks)
	{
		track.total_appearing = track.boxes.size();
		if (track.max_score >= sigma_h && track.boxes.size() >= t_min)
		{
			finished_tracks.push_back(track);
		}
		
			
		std::cout << "Num of finished tracks > " << finished_tracks.size() << std::endl;
	}
		
	/// Enumerate only the remaining tracks aka the ones finished
	for (int i = 0; i < finished_tracks.size(); i++)
	{
		finished_tracks[i].id = track_id;
		
		track_id++;
	}
	
	return finished_tracks;

}