/***************
华机战队能量机关视觉识别 - 特征计算函数集
Author：曾煜文
***************/
#include<opencv2/opencv.hpp>
#include<iostream>
using namespace cv;
using namespace std;

//卷积核计算函数（Calcuate.m）
void Kernel_Calculate(Mat &srcImage ,int *resultArray)
{
	Mat result_0, result_45, result_90, result_135, result_180, result_225, result_270, result_315;
	Mat final_result;
	int i, j, k, max;
	int feature_count[8] = { 0 };

	Mat kernel_0 = (Mat_<char>(3, 3) << -1, -2, -1, 0, 0, 0, 1, 2, 1); //Sobel算子，下同
	Mat kernel_45 = (Mat_<char>(3, 3) << -2, -1, 0, -1, 0, 1, 0, 1, 2);
	Mat kernel_90 = (Mat_<char>(3, 3) << -1, 0, 1, -2, 0, 2, -1, 0, 1);
	Mat kernel_135 = (Mat_<char>(3, 3) << 0, 1, 2, -1, 0, 1, -2, -1, 0);
	Mat kernel_180 = (Mat_<char>(3, 3) << 1, 2, 1, 0, 0, 0, -1, -2, -1);
	Mat kernel_225 = (Mat_<char>(3, 3) << 2, 1, 0, 1, 0, -1, 0, -1, -2);
	Mat kernel_270 = (Mat_<char>(3, 3) << 1, 0, -1, 2, 0, -2, 1, 0, -1);
	Mat kernel_315 = (Mat_<char>(3, 3) << 0, -1, -2, 1, 0, -1, 2, 1, 0);

	filter2D(srcImage, result_0, CV_16U, kernel_0);
	filter2D(srcImage, result_45, CV_16U, kernel_45);
	filter2D(srcImage, result_90, CV_16U, kernel_90);
	filter2D(srcImage, result_135, CV_16U, kernel_135);
	filter2D(srcImage, result_180, CV_16U, kernel_180);
	filter2D(srcImage, result_225, CV_16U, kernel_225);
	filter2D(srcImage, result_270, CV_16U, kernel_270);
	filter2D(srcImage, result_315, CV_16U, kernel_315);

	final_result = Mat::zeros(result_0.size(), CV_8U);

	for (i = 0; i < final_result.rows; i++)
	{
		for (j = 0; j < final_result.cols; j++)
		{
			k = 0;
			max = result_0.at<short>(i, j);
			if (result_45.at<short>(i, j) > max) k = 1, max = result_45.at<short>(i, j);
			if (result_90.at<short>(i, j) > max) k = 2, max = result_90.at<short>(i, j);
			if (result_135.at<short>(i, j) > max) k = 3, max = result_135.at<short>(i, j);
			if (result_180.at<short>(i, j) > max) k = 4, max = result_180.at<short>(i, j);
			if (result_225.at<short>(i, j) > max) k = 5, max = result_225.at<short>(i, j);
			if (result_270.at<short>(i, j) > max) k = 6, max = result_270.at<short>(i, j);
			if (result_315.at<short>(i, j) > max) k = 7, max = result_315.at<short>(i, j);
			final_result.at<char>(i, j) = k;

			feature_count[k]++;
		}
	}

	memcpy(resultArray, feature_count, sizeof(int) * 8);
}

//特征计算。将图像十六等分，计算特征值（FeatureCalcuate.m）
void Feature_Calcuate(Mat &srcImage , int resultArray[128])
{
	int m = srcImage.rows, n = srcImage.cols;
	int rowsN = m / 4, colsN = n / 4;
	int *p = resultArray;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			Mat temp = srcImage(Rect(i*rowsN, j*colsN, rowsN, colsN));
			Kernel_Calculate(temp, p);
			p += 8;
		}
	}
}

