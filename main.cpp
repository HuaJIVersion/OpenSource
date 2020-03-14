#include"./Head/ArmorDetector.h"
#include"./Head/Pose.h"
#include"./Head/PortProtocol.h"
using namespace cv;
using namespace std;
using namespace rm;



int main()
{
	VideoCapture cap0(0);

	//VideoCapture cap0("D:/文档/visual studio/视觉组/装甲识别/装甲板识别测试视频（蓝色）.mp4");
	Mat frame0;//用于储存每一帧的图像
	ArmorParam armorParam;
	ArmorDetector armor(armorParam);//initialize param @armor
	Pose pose;
	Point2d offset;

	armor.setEnemyColor(RED);//set the color of the enemy's armor as RED
	namedWindow("1", WINDOW_AUTOSIZE);

	while (1)
	{

		if (cap0.read(frame0))
		{
			cap0 >> frame0;
			armor.loadImg(frame0);
			if (armor.detect() == 3) {
				receive(fdk_f);//receive feedback frame
				offset = pose.getArmorPoint(armor,fdk_f.bulletType);//get angle offset
				ctl_f.pitch = offset.x;//set control pitch in x-asxis
				ctl_f.yaw = offset.y;//set control yaw in y-axis
				send(ctl_f);//send control frame
				cout << "offset: " << offset << endl;//output angle offset
				
			}
			armor.showDebugImg();
			if (waitKey(50) == 27)
			{
				break;
			}
		}
	}
	cap0.release();
	return 0;

}



