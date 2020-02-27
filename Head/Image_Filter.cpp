/***************
华机战队能量机关视觉识别 - 图像滤波函数集
Author：曾煜文
***************/
#include<algorithm>
#include<opencv2/opencv.hpp>
using namespace cv;
using namespace std;

//辅助函数。将矩阵循环向右移动x个、向下移动y个元素
void circshift(Mat &out, const Point &delta)
{
	Size sz = out.size();

	// 错误检查
	assert(sz.height > 0 && sz.width > 0);
	// 此种情况不需要移动
	if ((sz.height == 1 && sz.width == 1) || (delta.x == 0 && delta.y == 0))
		return;

	// 需要移动参数的变换，这样就能输入各种整数了
	int x = delta.x;
	int y = delta.y;
	if (x > 0) x = x % sz.width;
	if (y > 0) y = y % sz.height;
	if (x < 0) x = x % sz.width + sz.width;
	if (y < 0) y = y % sz.height + sz.height;


	// 多维的Mat也能移动
	vector<Mat> planes;
	split(out, planes);

	for (size_t i = 0; i < planes.size(); i++)
	{
		// 竖直方向移动
		Mat tmp0, tmp1, tmp2, tmp3;
		Mat q0(planes[i], Rect(0, 0, sz.width, sz.height - y));
		Mat q1(planes[i], Rect(0, sz.height - y, sz.width, y));
		q0.copyTo(tmp0);
		q1.copyTo(tmp1);
		tmp0.copyTo(planes[i](Rect(0, y, sz.width, sz.height - y)));
		tmp1.copyTo(planes[i](Rect(0, 0, sz.width, y)));

		// 水平方向移动
		Mat q2(planes[i], Rect(0, 0, sz.width - x, sz.height));
		Mat q3(planes[i], Rect(sz.width - x, 0, x, sz.height));
		q2.copyTo(tmp2);
		q3.copyTo(tmp3);
		tmp2.copyTo(planes[i](Rect(x, 0, sz.width - x, sz.height)));
		tmp3.copyTo(planes[i](Rect(0, 0, x, sz.height)));
	}

	merge(planes, out);
}

//辅助函数。将频谱图的低频分量移到中间
void fftshift(Mat &out)
{
	Size sz = out.size();
	Point pt(0, 0);
	pt.x = (int)floor(sz.width / 2.0);
	pt.y = (int)floor(sz.height / 2.0);
	circshift(out, pt);
}

//辅助函数。
void ifftshift(Mat &out)
{
	Size sz = out.size();
	Point pt(0, 0);
	pt.x = (int)ceil(sz.width / 2.0);
	pt.y = (int)ceil(sz.height / 2.0);
	circshift(out, pt);
}

//辅助函数。图像颜色区间限制，将0-255的整数表示压缩至[0,1]的浮点数
void im2double (Mat &src)
{
	src.convertTo(src, CV_64F);
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			src.at<double>(i, j) /= 255.0;
		}
	}
}

//高斯同态滤波，要求输入必须是0-255灰度图（或单通道图像）TongTiLuBo.m
void homomorphicFilter(Mat &src,Mat &dst)
{
	Mat logImage, fftImage;
	Mat padded;
	//Step1.图像取对数
	logImage = src;
	im2double(logImage);
	add(logImage, Scalar(1.0), logImage); //对原图像的每个像素点加1，避免log(0)的非法运算
	log(logImage, logImage); //对数运算
	//Step2.傅里叶变换
	Mat planes[] = { logImage, Mat::zeros(logImage.size(), CV_64F) }; //为复数的实部和虚部 分配存储空间
	Mat complexI;
	merge(planes, 2, complexI);
	dft(complexI, complexI);
	Mat magI;
	magI = complexI;
	fftshift(magI);
	//Step3.同态滤波
	//计算滤波函数H(u,v)
	Mat H = Mat::zeros(magI.size(),CV_64F);
	const int n1 = int(magI.rows / 2), n2 = int(magI.cols / 2);
	const double c = 2, rL = 0.5, rH = 4.7;
	int d0 = min(magI.rows, magI.cols);
	for (int i = 0; i < magI.rows; i++)
	{
		for (int j = 0; j < magI.cols; j++)
		{
			int d = (1 + i - n1) * (1 + i - n1) + (1 + j - n2) * (1 + j - n2);

			H.at<double>(i, j) = (rH - rL)*(1 - exp(-c * (d / double(2 * d0 * d0)))) + rL;
			magI.at<double>(i, j) *= H.at<double>(i, j);
		}
	}
	//Step4.傅里叶逆变换
	ifftshift(magI);
	idft(magI, magI, DFT_INVERSE+DFT_SCALE+ DFT_COMPLEX_OUTPUT);
	//Step5.图像指数运算
	exp(magI, magI);
	magI -= 1;
	split(magI, planes);
	dst = planes[0];
}