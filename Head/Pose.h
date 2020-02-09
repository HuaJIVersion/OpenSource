#pragma once
#include<cmath>
#include"./ArmorDetector.h"
#include"./General.h"
namespace rm {
	class Pose
	{
	public:
		cv::Point2d getArmorPoint(ArmorDetector& armor, bool bulletType);
	public:
		Pose();
	public:
		cv::Mat rVec;
		cv::Mat tVec;
	private:
		cv::Mat _cameraMatrix;
		cv::Mat _distCoeffs;
		std::vector<cv::Point3f>_bigArmor;
		std::vector<cv::Point3f>_littleArmor;
		std::vector<cv::Point2f>_armorVertex;
		//	0 -> small
		//	1 -> big
		//	-1 -> unkown
		int _type;
		double _littleOffset_2;
		double _littleOffset_4;
		double _bigOffset_2;
		double _bigOffset_4;
	}; 
}


