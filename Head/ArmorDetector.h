#pragma once
#include<opencv2/opencv.hpp>
#include<array>
#include"./General.h"

#define DEBUG_DETECTION
#define DEBUG_PRETREATMENT
namespace rm {
	struct ArmorParam
	{
		//Pre-treatment
		int brightness_threshold;
		int color_threshold;
		float light_color_detect_extend_ratio;

		//Filter lights
		float light_min_area;
		float light_max_angle;
		float light_min_size;
		float light_contour_min_solidity;
		float light_max_ratio;

		//Filter pairs
		float light_max_angle_diff_;
		float light_max_height_diff_ratio_; // hdiff / max(r.length, l.length)
		float light_max_y_diff_ratio_;  // ydiff / max(r.length, l.length)
		float light_min_x_diff_ratio_;

		//Filter armor
		float armor_big_armor_ratio;
		float armor_small_armor_ratio;
		float armor_min_aspect_ratio_;
		float armor_max_aspect_ratio_;

		//other params
		float sight_offset_normalized_base;
		float area_normalized_base;
		int enemy_color;
		int max_track_num = 3000;

		/*
		*	@Brief: Ϊ���������Ĭ��ֵ
		*/
		ArmorParam()
		{
			//pre-treatment
			brightness_threshold = 30;//210->50
			color_threshold = 40;
			light_color_detect_extend_ratio = 1.1;

			// Filter lights
			light_min_area = 10;
			light_max_angle = 45.0;
			light_min_size = 5.0;
			light_contour_min_solidity = 0.5;
			light_max_ratio = 1.0;

			// Filter pairs
			light_max_angle_diff_ = 7.0; //20
			light_max_height_diff_ratio_ = 0.2; //0.5
			light_max_y_diff_ratio_ = 2.0; //100
			light_min_x_diff_ratio_ = 0.5; //100

			// Filter armor
			armor_big_armor_ratio = 3.2;
			armor_small_armor_ratio = 2;
			//armor_max_height_ = 100.0;
			//armor_max_angle_ = 30.0;
			armor_min_aspect_ratio_ = 1.0;
			armor_max_aspect_ratio_ = 5.0;

			//other params
			sight_offset_normalized_base = 200;
			area_normalized_base = 1000;
			enemy_color = BLUE;
		}
	};

	class LightDescriptor
	{
	public:
		LightDescriptor() {};
		LightDescriptor(const cv::RotatedRect& light)
		{
			width = light.size.width;
			length = light.size.height;
			center = light.center;
			angle = light.angle;
			area = light.size.area();
		}
		const LightDescriptor& operator =(const LightDescriptor& ld)//Ϊʲô��&��const������//&��������Ϊ���ص���ָ��
		{
			this->width = ld.width;
			this->length = ld.length;
			this->center = ld.center;
			this->angle = ld.angle;
			this->area = ld.area;
			return *this;
		}

		/*
		*	@Brief: return the light as a cv::RotatedRect object
		*/
		cv::RotatedRect rec() const
		{
			return cv::RotatedRect(center, cv::Size2f(width, length), angle);
		}

	public:
		float width;
		float length;
		cv::Point2f center;
		float angle;
		float area;
	};

	/*
	* 	This class describes the armor information, including maximum bounding box, vertex and so on
	*/
	class ArmorDescriptor
	{
	public:
		/*
		*	@Brief: Initialize with all 0
		*/
		ArmorDescriptor();
		/*
		*	@Brief: calculate the rest of information(except for match&final score)of ArmroDescriptor based on:
				l&r light, part of members in ArmorDetector, and the armortype(for the sake of saving time)
		*	@Calls: ArmorDescriptor::getFrontImg()
		*/
		ArmorDescriptor(const LightDescriptor& lLight, const LightDescriptor& rLight, const int armorType, const cv::Mat& srcImg, const float rotationScore, ArmorParam param);

		/*
		*	@Brief: empty the object
		*	@Called :ArmorDetection._targetArmor
		*/
		void clear()
		{
			rotationScore = 0;
			sizeScore = 0;
			distScore = 0;
			finalScore = 0;
			for (int i = 0; i < 4; i++)
			{
				vertex[i] = cv::Point2f(0, 0);
			}
			type = UNKNOWN_ARMOR;
		}

		/*
		*	@Brief: get the front img(prespective transformation) of armor(if big, return the middle part)
		*	@Inputs: grayImg of roi
		*	@Outputs: store the front img to ArmorDescriptor's public
		*/
		void getFrontImg(const cv::Mat& grayImg);

		/*
		*	@Return: if the centeral pattern belong to an armor
		*/
		bool isArmorPattern() const;//const����:����һ����Ա������ʱ����const�ؼ���������˵����������� ��ֻ��(read-only)��������Ҳ����˵��������������޸��κ����ݳ�Ա(object)�� Ϊ������һ��const��Ա������ ��const�ؼ��ַ��ں������ŵĺ��档�����Ͷ����ʱ��Ӧ�÷�const�ؼ��֡�

	public:
		std::array<cv::RotatedRect, 2> lightPairs; //0 left, 1 right
		float sizeScore;		//S1 = e^(size)
		float distScore;		//S2 = e^(-offset)
		float rotationScore;		//S3 = -(ratio^2 + yDiff^2) 
		float finalScore;

		std::vector<cv::Point2f> vertex; //four vertex of armor area, lihgt bar area exclued!!	
		cv::Mat frontImg; //front img after prespective transformation from vertex,1 channel gray img

		//	0 -> small
		//	1 -> big
		//	-1 -> unkown
		int type;
	};

	/*
*	This class implement all the functions of detecting the armor
*/
	class ArmorDetector
	{
	public:
		/*
		*	flag for the detection result
		*/
		enum ArmorFlag
		{
			ARMOR_NO = 0,		// not found
			ARMOR_LOST = 1,		// lose tracking
			ARMOR_GLOBAL = 2,	// armor found globally
			ARMOR_LOCAL = 3		// armor found locally(in tracking mode)
		};

	public:
		ArmorDetector();
		ArmorDetector(const ArmorParam& armorParam);
		~ArmorDetector() {}

		/*
		*	@Brief: Initialize the armor parameters
		*	@Others: API for client
		*/
		void init(const ArmorParam& armorParam);

		/*
		*	@Brief: set the enemy's color
		*	@Others: API for client
		*/
		void setEnemyColor(int enemy_color)
		{
			_enemy_color = enemy_color;
			_self_color = enemy_color == BLUE ? RED : BLUE;
		}

		/*
		*	@Brief: load image and set tracking roi
		*	@Input: frame
		*	@Others: API for client
		*/
		void loadImg(const cv::Mat& srcImg);

		/*
		*	@Brief: core of detection algrithm, include all the main detection process
		*	@Outputs: ALL the info of detection result
		*	@Return: See enum ArmorFlag
		*	@Others: API for client
		*/
		int detect();

		/*
		*	@Brief: get the vertex of armor
		*	@Return: vector of four cv::point2f object
		*	@Notice: Order->left-top, right-top, right-bottom, left-bottom
		*	@Others: API for client
		*/
		const std::vector<cv::Point2f> getArmorVertex() const;

		/*
		*	@Brief: returns the type of the armor
		*	@Return: 0 for small armor, 1 for big armor
		*	@Others: API for client
		*/
		int getArmorType() const;

		/*
		*	@Brief: returns the center of the armor
		*	@Return: cv::point2f
		*	@Others: API for client
		*/
		const cv::Point2f getArmorCenter()const;
#if defined(DEBUG_DETECTION) || defined(SHOW_RESULT)
		void showDebugImg() const;
#endif // DEBUG_DETECTION || SHOW_RESULT

	private:
		ArmorParam _param;
		int _enemy_color;
		int _self_color;

		cv::Rect _roi; //relative coordinates

		cv::Mat _srcImg; //source img
		cv::Mat _roiImg; //roi from the result of last frame
		cv::Mat _grayImg; //gray img of roi

		int _trackCnt = 0;//ɶʱ�����ֱ�Ӹ�ֵ�ˣ�

		std::vector<ArmorDescriptor> _armors;

		ArmorDescriptor _targetArmor; //relative coordinates

		int _flag;
		bool _isTracking;


#if defined(DEBUG_DETECTION) || defined(SHOW_RESULT)
		std::string _debugWindowName;
		cv::Mat _debugImg;
#endif // DEBUG_DETECTION || SHOW_RESULT

#ifdef GET_ARMOR_PIC
		int _allCnt = 0;//ɶʱ�����ֱ�Ӹ�ֵ�ˣ�
#endif // GET_ARMOR_PIC

	};
}


