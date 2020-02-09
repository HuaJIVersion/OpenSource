#include "Pose.h"
namespace rm {
	Pose::Pose() {
		_cameraMatrix = cv::Mat::zeros(3, 3, CV_64F);
		_cameraMatrix.at<double>(0, 0) = 4.520328799832561e+02;
		_cameraMatrix.at<double>(0, 1) = -0.997694600923344;
		_cameraMatrix.at<double>(0, 2) = 3.061917571865317e+02;
		_cameraMatrix.at<double>(1, 1) = 4.677228243496429e+02;
		_cameraMatrix.at<double>(1, 2) = 2.520685291716649e+02;
		_cameraMatrix.at<double>(2, 2) = 1;
		_distCoeffs = cv::Mat::zeros(5, 1, CV_64F);
		_distCoeffs.at<double>(0, 0) = 0.086118449592113;
		_distCoeffs.at<double>(1, 0) = -0.026281889907881;
		_distCoeffs.at<double>(2, 0) = -0.001458346079303;
		_distCoeffs.at<double>(3, 0) = -8.045013744828257e-04;
		_distCoeffs.at<double>(4, 0) = 0;
		
		_bigArmor = std::vector<cv::Point3f>{
			cv::Point3f(-BIG_ARMOR_LENGTH, -BIG_ARMOR_HEIGHT, 0),	//tl
			cv::Point3f(BIG_ARMOR_LENGTH, -BIG_ARMOR_HEIGHT, 0),	//tr
			cv::Point3f(BIG_ARMOR_LENGTH, BIG_ARMOR_HEIGHT, 0),	//br
			cv::Point3f(-BIG_ARMOR_LENGTH, BIG_ARMOR_HEIGHT, 0)	//bl
		};

		_littleArmor = std::vector<cv::Point3f>{
			cv::Point3f(-LITTLE_ARMOR_LENGTH, -LITTLE_ARMOR_HEIGHT, 0),	//tl
			cv::Point3f(LITTLE_ARMOR_LENGTH, -LITTLE_ARMOR_HEIGHT, 0),	//tr
			cv::Point3f(LITTLE_ARMOR_LENGTH, LITTLE_ARMOR_HEIGHT, 0),	//br
			cv::Point3f(-LITTLE_ARMOR_LENGTH, LITTLE_ARMOR_HEIGHT, 0)	//bl
		};

		cv::Mat rVec = cv::Mat::zeros(3, 1, CV_64FC1);//init rvec
		cv::Mat tVec = cv::Mat::zeros(3, 1, CV_64FC1);//init tvec

		_bigOffset_2 = 0;
		_bigOffset_4 = 0;
		_littleOffset_2 = 0;
		_littleOffset_4 = 0;
	}
	cv::Point2d Pose::getArmorPoint(ArmorDetector& armor,bool bulletType) {
		int f = 0;
		_armorVertex.clear();
		for (const auto& point : armor.getArmorVertex())
		{
			_armorVertex.emplace_back(point);
			f++;
		}
		//std::cout << f << std::endl;
		_type = armor.getArmorType();
		if (_type == BIG_ARMOR)
			cv::solvePnP(_bigArmor, _armorVertex, _cameraMatrix, _distCoeffs, rVec, tVec, false, cv::SOLVEPNP_ITERATIVE);
		else if(_type==SMALL_ARMOR)
			cv::solvePnP(_littleArmor, _armorVertex, _cameraMatrix, _distCoeffs, rVec, tVec, false, cv::SOLVEPNP_ITERATIVE);
		//else return cv::Mat::zeros(3, 1, CV_64FC1);
		cv::Point2d anglePoint;
		double pitchRatio;
		double yawRatio;
		if (bulletType)//1是大弹丸
			if (tVec.at<double>(2,0)> 2000.0)pitchRatio = (_bigOffset_4 - tVec.at<double>(1,0)) / tVec.at<double>(2,0);
			else pitchRatio = (_bigOffset_2 - tVec.at<double>(1,0)) / tVec.at<double>(2,0);
		else//0是小弹丸
			if (tVec.at<double>(2,0) > 2000.0)pitchRatio = (_littleOffset_4 - tVec.at<double>(1,0)) / tVec.at<double>(2,0);
			else pitchRatio = (_littleOffset_2 - tVec.at<double>(1,0)) / tVec.at<double>(2,0);
		yawRatio = tVec.at<double>(0,0) / tVec.at<double>(2,0);
		anglePoint.x = atan(pitchRatio);
		anglePoint.y = atan(yawRatio);
		return anglePoint;
	}
}