#include<iostream>
#include<opencv2/opencv.hpp>
#include<cmath>
using namespace std;
using namespace cv;
//用于交换长度宽度矫正矩形
void swap(double *x,double *y)
{
	double temp;
	temp = *x;
	*x = *y;
	*y = temp;
}
//计算距离
double getDistance(Point A, Point B)
{
	double distance;
	distance = sqrt(pow(A.x - B.x, 2) + pow(A.y - B.y, 2));
	return distance;
}
double TemplateMatch(Mat image, Mat tepl, Point& point)
{
	int result_cols = image.cols - tepl.cols + 1;
	int result_rows = image.rows - tepl.rows + 1;
	Mat result = Mat(result_cols, result_rows, CV_32FC1);
	matchTemplate(image, tepl, result, TM_CCOEFF_NORMED);
	double minVal, maxVal;
	Point minLoc, maxLoc;
	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
	point = maxLoc;
	return maxVal;
}
//拟合圆
static bool CircleInfo2(vector<Point2f>& pts, Point2f& center, float& radius)
{
	center =Point2d(0, 0);
	radius = 0.0;
	if (pts.size() < 3) return false;;

	double sumX = 0.0;
	double sumY = 0.0;
	double sumX2 = 0.0;
	double sumY2 = 0.0;
	double sumX3 = 0.0;
	double sumY3 = 0.0;
	double sumXY = 0.0;
	double sumX1Y2 = 0.0;
	double sumX2Y1 = 0.0;
	const double N = (double)pts.size();
	for (int i = 0; i < pts.size(); ++i)
	{
		double x = pts.at(i).x;
		double y = pts.at(i).y;
		double x2 = x * x;
		double y2 = y * y;
		double x3 = x2 * x;
		double y3 = y2 * y;
		double xy = x * y;
		double x1y2 = x * y2;
		double x2y1 = x2 * y;

		sumX += x;
		sumY += y;
		sumX2 += x2;
		sumY2 += y2;
		sumX3 += x3;
		sumY3 += y3;
		sumXY += xy;
		sumX1Y2 += x1y2;
		sumX2Y1 += x2y1;
	}
	double C = N * sumX2 - sumX * sumX;
	double D = N * sumXY - sumX * sumY;
	double E = N * sumX3 + N * sumX1Y2 - (sumX2 + sumY2) * sumX;
	double G = N * sumY2 - sumY * sumY;
	double H = N * sumX2Y1 + N * sumY3 - (sumX2 + sumY2) * sumY;

	double denominator = C * G - D * D;
	if (std::abs(denominator) < DBL_EPSILON) return false;
	double a = (H * D - E * G) / (denominator);
	denominator = D * D - G * C;
	if (std::abs(denominator) < DBL_EPSILON) return false;
	double b = (H * C - E * D) / (denominator);
	double c = -(a * sumX + b * sumY + sumX2 + sumY2) / N;

	center.x = a / (-2);
	center.y = b / (-2);
	radius = std::sqrt(a * a + b * b - 4 * c) / 2;
	return true;
}
//主函数
int main()
{
	Mat templ[9];
	double value;
	vector<Point2f> cir;
	for (int i = 1; i <= 8; i++)
	{
		templ[i] = imread("C:\\picture\\template\\template" + to_string(i) + ".jpg", IMREAD_GRAYSCALE);
	}
	Mat srcimg;
	VideoCapture cap;
	cap.open("C:\\picture\\red.avi");
	while (1)
	{
		cap >> srcimg;
		if (srcimg.empty())
		{
			break;
		}
		Point2f cc;//用于存放"R"的中心点
		Point2f centerP;
		//分离颜色，二值化
		vector<Mat>imgChannels;
		split(srcimg, imgChannels);
		Mat midimg = imgChannels.at(2) - imgChannels.at(0);
		threshold(midimg, midimg, 100, 255, THRESH_BINARY);
		//膨胀操作与闭操作去除噪声
		int structelement = 2;
		Mat element = getStructuringElement(MORPH_RECT, Size(structelement + 1, structelement + 1), Point(-1, -1));
		dilate(midimg, midimg, element);
		structelement = 3;
		element = getStructuringElement(MORPH_RECT, Size(2 * structelement + 1, 2 * structelement + 1), Point(-1, -1));
		morphologyEx(midimg, midimg, MORPH_CLOSE, element);
		//轮廓识别
		vector<vector<Point>>contours;
		vector<Vec4i>hierarchy;
		findContours(midimg, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);
		RotatedRect rect_temp;
		//遍历轮廓
		if (hierarchy.size())
		{
			for (int i = 0; i >= 0; i = hierarchy[i][0])
			{
				Mat ROI;
				rect_temp = minAreaRect(contours[i]);
				Point2f P[4];
				rect_temp.points(P);
				Point2f srcRect[4];
				Point2f dstRect[4];
				double width;
				double height;
				width = getDistance(P[0], P[1]);
				height = getDistance(P[1], P[2]);
				//矫正矩形，用于获取透视变换的矩形
				if (height > width)
				{
					swap(height, width);
					srcRect[0] = P[1];
					srcRect[1] = P[2];
					srcRect[2] = P[3];
					srcRect[3] = P[0];
				}
				else
				{
					srcRect[0] = P[0];
					srcRect[1] = P[1];
					srcRect[2] = P[2];
					srcRect[3] = P[3];
				}
				double area = width * height;
				cout << area << " " << width << " " << height << endl; //输出长，宽，面积便于调整接下来的参数
				if (area > 7000)//通过面积筛选扇叶图
				{
					//获取透视变换所需矩形
					dstRect[0] = Point2f(0, 0);
					dstRect[1] = Point2f(width, 0);
					dstRect[2] = Point2f(width, height);
					dstRect[3] = Point2f(0, height);
					//透视变换
					Mat tran = getPerspectiveTransform(srcRect, dstRect);
					Mat perspect;
					warpPerspective(midimg, perspect, tran, midimg.size());
					ROI = perspect(Rect(0, 0, width, height));
					if (ROI.empty())
					{
						cout << "filed open" << endl;
						return -1;
					}
					vector<double> Vvalue1;
					vector<double> Vvalue2;
					Mat tmp1;
					resize(ROI, tmp1, Size(42, 20));
					Point matchLoc;
					//分别与待打击的模板与打击过的模板匹配
					for (int j = 1; j <= 6; j++)
					{
						value = TemplateMatch(tmp1, templ[j], matchLoc);
						Vvalue1.push_back(value);
					}
					for (int j = 7; j <= 8; j++)
					{
						value = TemplateMatch(tmp1, templ[j], matchLoc);
						Vvalue2.push_back(value);
					}
					int maxv1 = 0, maxv2 = 0;

					for (int t1 = 0; t1 < 6; t1++)
					{
						if (Vvalue1[t1] > Vvalue1[maxv1])
						{
							maxv1 = t1;
						}
					}
					for (int t2 = 0; t2 < 2; t2++)
					{
						if (Vvalue2[t2] > Vvalue2[maxv2])
						{
							maxv2 = t2;
						}
					}
					if (Vvalue1[maxv1] > Vvalue2[maxv2]&&Vvalue1[maxv1]>0.6)
					{
						bool findtarget = true;
						for (int j = 0; j < 4; j++)
						{
							line(srcimg, srcRect[j], srcRect[(j + 1) % 4], Scalar(0, 0, 255), 2);
						}
						if (hierarchy[i][2] > 0)
						{
							RotatedRect rect_tmp2 = minAreaRect(contours[hierarchy[i][2]]);
							Point2f Pnt[4];
							rect_tmp2.points(Pnt);
							const float maxHWRatio = 0.7153846;
							const float maxArea = 2000;
							const float minArea = 500;
							float width = rect_tmp2.size.width;
							float height = rect_tmp2.size.height;
							if (height > width)
							{
								swap(height, width);
							}
							float area = width * height;
							if (height / width > maxHWRatio || area > maxArea || area < minArea)
							{
								/*for (int j = 0; j < 4; ++j)
								{
									line(srcimg, Pnt[j], Pnt[(j + 1) % 4], Scalar(255, 0, 255), 4);
								}
								for (int j = 0; j < 4; ++j)
								{
									line(srcimg, P[j], P[(j + 1) % 4], Scalar(255, 255, 0), 4);
								}*/
								continue;
							}
							centerP = rect_tmp2.center;
							circle(srcimg, centerP, 1, Scalar(0, 255, 0), 2);
							if (cir.size() < 30)
							{
								cir.push_back(centerP);
							}
						}
					}
				}
				else
				{
					cc = rect_temp.center;
					circle(srcimg, cc, 1, Scalar(0, 255, 0), 2);
				}
			}
			double radius = 0;
			radius = getDistance(cc, centerP);
			circle(srcimg, cc, radius, Scalar(0, 0, 255));
		}
		imshow("out", srcimg);
		waitKey(60);
	}
	return 0;
}
	