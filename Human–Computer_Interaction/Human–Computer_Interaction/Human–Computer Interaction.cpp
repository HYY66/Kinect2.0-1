#include <iostream>
#include <opencv2\imgproc.hpp>	//opencv头文件
#include <opencv2\calib3d.hpp>
#include <opencv2\highgui.hpp>
#include <Kinect.h>	//Kinect头文件

using   namespace   std;
using   namespace   cv;

void drawline(cv::Mat & img, Joint & r_1, Joint & r_2, ICoordinateMapper * myMapper);
void drawhandstate(cv::Mat & img, Joint & lefthand, Joint & righthand, IBody* myBodyArr, ICoordinateMapper * myMapper);
void DrawBody(cv::Mat & img, Joint *myJointArr, ICoordinateMapper * myMapper);
bool GestureDetection(Joint & elbow, Joint & hand, ICoordinateMapper * myMapper);

int data[10][2];

int main(void)
{
	HRESULT hr = FALSE;
	// 获得Kinect传感器
	IKinectSensor   * mySensor = nullptr;
	hr = GetDefaultKinectSensor(&mySensor);
	if (SUCCEEDED(hr))
		hr = mySensor->Open();

	// 映射坐标系
	ICoordinateMapper   * myMapper = nullptr;
	if (SUCCEEDED(hr))
		hr = mySensor->get_CoordinateMapper(&myMapper);

	// 彩色数据
	IColorFrameSource   * myColorSource = nullptr;
	if (SUCCEEDED(hr))
		hr = mySensor->get_ColorFrameSource(&myColorSource);
	IColorFrameReader   * myColorReader = nullptr;
	if (SUCCEEDED(hr))
		hr = myColorSource->OpenReader(&myColorReader);
	IFrameDescription   * mycolorDescription = nullptr;
	if (SUCCEEDED(hr))
		hr = myColorSource->get_FrameDescription(&mycolorDescription);
	int colorHeight = 0, colorWidth = 0;
	mycolorDescription->get_Height(&colorHeight);
	mycolorDescription->get_Width(&colorWidth);
	cout << "彩色图像大小：" << colorWidth << "*" << colorHeight << " 数据格式：CV_8UC4" << endl;
	IColorFrame * myColorFrame = nullptr;
	Mat color(colorHeight, colorWidth, CV_8UC4);

	// 深度数据
	IDepthFrameSource   * myDepthSource = nullptr;   //取得深度数据
	if (SUCCEEDED(hr))
		hr = mySensor->get_DepthFrameSource(&myDepthSource);
	IDepthFrameReader   * myDepthReader = nullptr;
	if (SUCCEEDED(hr))
		hr = myDepthSource->OpenReader(&myDepthReader);    //打开深度数据的Reader
	IFrameDescription   * mydepthDescription = nullptr;
	if (SUCCEEDED(hr))
		hr = myDepthSource->get_FrameDescription(&mydepthDescription);
	int depthHeight = 0, depthWidth = 0;
	mydepthDescription->get_Height(&depthHeight);
	mydepthDescription->get_Width(&depthWidth);
	cout << "深度图像大小：" << depthWidth << " * " << depthHeight << " 数据格式：CV_16UC1" << endl;
	IDepthFrame * myDepthFrame = nullptr;
	Mat depth(depthHeight, depthWidth, CV_16UC1);    //建立图像矩阵

	// 红外数据
	IInfraredFrameSource * myInfraredSource = nullptr;
	if (SUCCEEDED(hr))
		hr = mySensor->get_InfraredFrameSource(&myInfraredSource);
	IInfraredFrameReader  * myInfraredReader = nullptr;
	if (SUCCEEDED(hr))
		hr = myInfraredSource->OpenReader(&myInfraredReader);
	IFrameDescription   * myinfraredDescription = nullptr;
	if (SUCCEEDED(hr))
		hr = myInfraredSource->get_FrameDescription(&myinfraredDescription);
	int infraredHeight = 0, infraredWidth = 0;
	myinfraredDescription->get_Height(&infraredHeight);
	myinfraredDescription->get_Width(&infraredWidth);
	cout << "红外图像大小：" << infraredWidth << " * " << infraredHeight << " 数据格式：CV_16UC1" << endl;
	IInfraredFrame  * myInfraredFrame = nullptr;
	Mat infrared(infraredHeight, infraredWidth, CV_16UC1);

	// 人物索引
	IBodyIndexFrameSource* pBodyIndexFrameSource = NULL;//读取背景二值图
	if (SUCCEEDED(hr))
		hr = mySensor->get_BodyIndexFrameSource(&pBodyIndexFrameSource);
	IBodyIndexFrameReader*  m_pBodyIndexFrameReader;//用于背景二值图读取
	if (SUCCEEDED(hr))
		hr = pBodyIndexFrameSource->OpenReader(&m_pBodyIndexFrameReader);
	IFrameDescription   * mybodyindexDescription = nullptr;
	if (SUCCEEDED(hr))
		hr = pBodyIndexFrameSource->get_FrameDescription(&mybodyindexDescription);
	int bodyindexHeight = 0, bodyindexWidth = 0;
	mybodyindexDescription->get_Height(&bodyindexHeight);
	mybodyindexDescription->get_Width(&bodyindexWidth);
	cout << "人物图像大小：" << bodyindexWidth << " * " << bodyindexHeight << " 数据格式：CV_8UC3" << endl;
	IBodyIndexFrame  * myBodyIndexFrame = nullptr;
	Mat bodyindex(bodyindexHeight, bodyindexWidth, CV_8UC3);

	// 骨骼数据
	IBodyFrameSource    * myBodySource = nullptr;
	if (SUCCEEDED(hr))
		mySensor->get_BodyFrameSource(&myBodySource);
	IBodyFrameReader    * myBodyReader = nullptr;
	if (SUCCEEDED(hr))
		myBodySource->OpenReader(&myBodyReader);
	IBodyFrame  * myBodyFrame = nullptr;

	Mat colorbody(colorHeight, colorWidth, CV_8UC4);

	// 主程序
	while (1)
	{
		while (myColorReader->AcquireLatestFrame(&myColorFrame) != S_OK);
		myColorFrame->CopyConvertedFrameDataToArray(colorHeight * colorWidth * 4, color.data, ColorImageFormat_Bgra); // 图像大小、目标地址、图像格式
		myColorFrame->Release();
		//imshow("Color", color);

		while (myDepthReader->AcquireLatestFrame(&myDepthFrame) != S_OK);
		myDepthFrame->CopyFrameDataToArray(depthHeight * depthWidth, (UINT16 *)depth.data); //先把数据存入16位的图像矩阵中
		myDepthFrame->Release();
		Mat showdepth(depthHeight, depthWidth, CV_8UC1);
		depth.convertTo(showdepth, CV_8UC1, 255.0 / 4500);   //再把16位转换为8位
		//imshow("Depth", showdepth);

		while (myInfraredReader->AcquireLatestFrame(&myInfraredFrame) != S_OK);
		myInfraredFrame->CopyFrameDataToArray(infraredHeight * infraredWidth, (UINT16 *)infrared.data);
		myInfraredFrame->Release();
		//imshow("Infrared", infrared);

		while (m_pBodyIndexFrameReader->AcquireLatestFrame(&myBodyIndexFrame) != S_OK);
		BYTE *bodyIndexArray = new BYTE[bodyindexHeight * bodyindexWidth];//背景二值图是8为uchar，有人是黑色，没人是白色
		myBodyIndexFrame->CopyFrameDataToArray(bodyindexHeight * bodyindexWidth, bodyIndexArray);
		myBodyIndexFrame->Release();
		uchar* bodyindexptr = (uchar*)bodyindex.data;
		for (int j = 0; j < bodyindexHeight * bodyindexWidth; ++j)
		{
			*bodyindexptr = bodyIndexArray[j]; ++bodyindexptr;
			*bodyindexptr = bodyIndexArray[j]; ++bodyindexptr;
			*bodyindexptr = bodyIndexArray[j]; ++bodyindexptr;
		}
		delete[] bodyIndexArray;
		imshow("BodyIndex", bodyindex);
		//unsigned int buffersize = 0;
		//unsigned char* buffer = nullptr;
		//hr = myBodyIndexFrame->AccessUnderlyingBuffer(&buffersize, &buffer);
		//if (SUCCEEDED(hr))
		//{
		//	for (int x = 0; x < bodyindexHeight; x++)
		//	{
		//		for (int y = 0; y < bodyindexWidth; y++)
		//		{
		//			unsigned int index = x * bodyindexWidth + y;
		//			if (buffer[index] != 255)
		//			{
		//				bodyindex.at<Vec3b>(x, y) = Vec3b(0, 0, 255);
		//			}
		//			else
		//			{
		//				bodyindex.at<Vec3b>(x, y) = Vec3b(0, 0, 0);
		//			}
		//		}
		//	}
		//}
		//myBodyIndexFrame->Release();
		//imshow("BodyIndex", bodyindex);

		while (myBodyReader->AcquireLatestFrame(&myBodyFrame) != S_OK);
		IBody   **  myBodyArr = new IBody *[BODY_COUNT];
		for (int i = 0; i < BODY_COUNT; i++)	myBodyArr[i] = nullptr;
		if (myBodyFrame->GetAndRefreshBodyData(BODY_COUNT, myBodyArr) == S_OK)
		for (int i = 0; i < BODY_COUNT; i++)
		{
			BOOLEAN     result = false;
			if (myBodyArr[i]->get_IsTracked(&result) == S_OK && result)
			{
				Joint   myJointArr[JointType_Count];
				//如果侦测到就把关节数据输入到数组并画图
				if (myBodyArr[i]->GetJoints(JointType_Count, myJointArr) == S_OK)
				{
					colorbody = color.clone();
					// 绘制身体关节
					DrawBody(colorbody, myJointArr, myMapper);
					// 绘制手部状态
					drawhandstate(colorbody, myJointArr[JointType_HandLeft], myJointArr[JointType_HandRight], myBodyArr[i], myMapper);
					// 挥手检测
					GestureDetection(myJointArr[JointType_ElbowRight], myJointArr[JointType_HandRight], myMapper);
				}
			}
		}
		delete[]myBodyArr;
		myBodyFrame->Release();
		Mat showcolorbody;
		double fScale = 0.5;//缩放系数  
		//计算目标图像的大小  
		Size dsize = Size(colorbody.cols*fScale, colorbody.rows*fScale);
		resize(colorbody, showcolorbody, dsize);
		imshow("ColorBody", showcolorbody);

		if (waitKey(1) == VK_ESCAPE)
			break;
	}


	myMapper->Release();

	mycolorDescription->Release();
	myColorReader->Release();
	myColorSource->Release();

	mydepthDescription->Release();
	myDepthReader->Release();
	myDepthSource->Release();

	myinfraredDescription->Release();
	myInfraredReader->Release();
	myInfraredSource->Release();

	mybodyindexDescription->Release();
	m_pBodyIndexFrameReader->Release();
	pBodyIndexFrameSource->Release();

	myBodyReader->Release();
	myBodySource->Release();

	mySensor->Close();
	mySensor->Release();

	waitKey();

	return  0;
}

void    drawline(Mat & img, Joint & r_1, Joint & r_2, ICoordinateMapper * myMapper)
{
	//用两个关节点来做线段的两端，并且进行状态过滤
	if (r_1.TrackingState == TrackingState_Tracked && r_2.TrackingState == TrackingState_Tracked)
	{
		ColorSpacePoint t_point;    //要把关节点用的摄像机坐标下的点转换成彩色空间的点
		Point   p_1, p_2;
		myMapper->MapCameraPointToColorSpace(r_1.Position, &t_point);
		p_1.x = t_point.X;
		p_1.y = t_point.Y;
		myMapper->MapCameraPointToColorSpace(r_2.Position, &t_point);
		p_2.x = t_point.X;
		p_2.y = t_point.Y;

		line(img, p_1, p_2, Vec3b(0, 255, 0), 5);
		circle(img, p_1, 10, Vec3b(255, 0, 0), -1);
		circle(img, p_2, 10, Vec3b(255, 0, 0), -1);
	}
}

void    drawhandstate(Mat & img, Joint & lefthand, Joint & righthand, IBody* myBodyArr, ICoordinateMapper * myMapper)
{
	if (lefthand.TrackingState == TrackingState_Tracked && righthand.TrackingState == TrackingState_Tracked)
	{
		ColorSpacePoint l_point, r_point;    //要把关节点用的摄像机坐标下的点转换成彩色空间的点
		Point   p_l, p_r;
		myMapper->MapCameraPointToColorSpace(lefthand.Position, &l_point);
		p_l.x = l_point.X;
		p_l.y = l_point.Y;
		myMapper->MapCameraPointToColorSpace(righthand.Position, &r_point);
		p_r.x = r_point.X;
		p_r.y = r_point.Y;
		//cout << "p_l:" << p_l.x << "," << p_l.y << " ";
		//cout << "p_r:" << p_r.x << "," << p_r.y << endl;

		//char* str1 = NULL, str2 = NULL;
		//CvFont font;
		//cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1.5f, 1.5f, 0, 2, CV_AA);//设置显示的字体
		//cvPutText(&IplImage(img), "LeftHand", Point(p_l.x + 50, p_l.y - 50), &font, CV_RGB(255, 0, 0));//红色字体注释
		//cvPutText(&IplImage(img), "RightHand", Point(p_r.x + 50, p_r.y - 50), &font, CV_RGB(255, 0, 0));//红色字体注释

		HandState left;
		myBodyArr->get_HandLeftState(&left);
		HandState right;
		myBodyArr->get_HandRightState(&right);
		switch (left)
		{
		case HandState_Closed:
			circle(img, p_l, 10, Scalar(0, 0, 255, 1), 20); break;
		case HandState_Open:
			circle(img, p_l, 10, Scalar(0, 255, 0, 1), 20); break;
		case HandState_Lasso:
			circle(img, p_l, 10, Scalar(255, 0, 0, 1), 20); break;
		default:
			break;
		}
		switch (right)
		{
		case HandState_Closed:
			circle(img, p_r, 10, Scalar(0, 0, 255, 1), 20); break;
		case HandState_Open:
			circle(img, p_r, 10, Scalar(0, 255, 0, 1), 20); break;
		case HandState_Lasso:
			circle(img, p_r, 10, Scalar(255, 0, 0, 1), 20); break;
		default:
			break;
		}
	}
}

void DrawBody(Mat & img, Joint *myJointArr, ICoordinateMapper * myMapper)
{
	drawline(img, myJointArr[JointType_Head], myJointArr[JointType_Neck], myMapper);						// 头-颈
	drawline(img, myJointArr[JointType_Neck], myJointArr[JointType_SpineShoulder], myMapper);				// 颈-脊柱肩

	drawline(img, myJointArr[JointType_SpineShoulder], myJointArr[JointType_ShoulderLeft], myMapper);		// 脊柱肩-左肩膀
	drawline(img, myJointArr[JointType_SpineShoulder], myJointArr[JointType_SpineMid], myMapper);			// 脊柱肩-脊柱中
	drawline(img, myJointArr[JointType_SpineShoulder], myJointArr[JointType_ShoulderRight], myMapper);		// 脊柱肩-右肩膀

	drawline(img, myJointArr[JointType_ShoulderLeft], myJointArr[JointType_ElbowLeft], myMapper);			// 左肩膀-左手肘
	drawline(img, myJointArr[JointType_SpineMid], myJointArr[JointType_SpineBase], myMapper);				// 脊柱中-脊柱底
	drawline(img, myJointArr[JointType_ShoulderRight], myJointArr[JointType_ElbowRight], myMapper);		// 右肩膀-右手肘

	drawline(img, myJointArr[JointType_ElbowLeft], myJointArr[JointType_WristLeft], myMapper);				// 左手肘-左手腕
	drawline(img, myJointArr[JointType_SpineBase], myJointArr[JointType_HipLeft], myMapper);				// 脊柱底-左胯部
	drawline(img, myJointArr[JointType_SpineBase], myJointArr[JointType_HipRight], myMapper);				// 脊柱底-右胯部
	drawline(img, myJointArr[JointType_ElbowRight], myJointArr[JointType_WristRight], myMapper);			// 右手肘-右手腕

	drawline(img, myJointArr[JointType_WristLeft], myJointArr[JointType_ThumbLeft], myMapper);				// 左手腕-左拇指
	drawline(img, myJointArr[JointType_WristLeft], myJointArr[JointType_HandLeft], myMapper);				// 左手腕-左手掌
	drawline(img, myJointArr[JointType_HipLeft], myJointArr[JointType_KneeLeft], myMapper);				// 左胯部-左膝盖
	drawline(img, myJointArr[JointType_HipRight], myJointArr[JointType_KneeRight], myMapper);				// 右胯部-右膝盖
	drawline(img, myJointArr[JointType_WristRight], myJointArr[JointType_ThumbRight], myMapper);			// 右手腕-右拇指
	drawline(img, myJointArr[JointType_WristRight], myJointArr[JointType_HandRight], myMapper);			// 右手腕-右手掌

	drawline(img, myJointArr[JointType_HandLeft], myJointArr[JointType_HandTipLeft], myMapper);			// 左手掌-手指尖
	drawline(img, myJointArr[JointType_KneeLeft], myJointArr[JointType_FootLeft], myMapper);				// 左膝盖-左脚
	drawline(img, myJointArr[JointType_KneeRight], myJointArr[JointType_FootRight], myMapper);				// 右膝盖-右脚
	drawline(img, myJointArr[JointType_HandRight], myJointArr[JointType_HandTipRight], myMapper);			// 右手掌-手指尖
}

// 判断骨骼追踪情况：包括骨骼追踪完好且手部位置在肘上面  
bool GestureDetection(Joint & elbow, Joint & hand, ICoordinateMapper * myMapper)
{
	// 骨骼追踪完好
	if (elbow.TrackingState == TrackingState_Tracked && hand.TrackingState == TrackingState_Tracked)
	{
		// 要把关节点用的摄像机坐标下的点转换成彩色空间的点
		ColorSpacePoint t_point;
		Point	p_e, p_h;
		myMapper->MapCameraPointToColorSpace(elbow.Position, &t_point);
		p_e.x = t_point.X;
		p_e.y = t_point.Y;
		myMapper->MapCameraPointToColorSpace(hand.Position, &t_point);
		p_h.x = t_point.X;
		p_h.y = t_point.Y;
		// 手掌高于手肘
		if (p_h.y < p_e.y)
		{
			int elbow_range = 0, hand_range = 0;
			int elbow_MAX = INT_MIN, elbow_MIN = INT_MAX, hand_MAX = INT_MIN, hand_MIN = INT_MAX;
			// 保存近10帧的手掌和手肘位置，并进行数据更新
			for (int i = 0; i < 9; i++)
			{
				data[i][0] = data[i + 1][0];
				data[i][1] = data[i + 1][1];
			}
			data[9][0] = p_e.x;
			data[9][1] = p_h.x;
			// 计算手掌和手肘的运动范围
			for (int i = 0; i < 10; i++)
			{
				if (data[i][0]>elbow_MAX) elbow_MAX = data[i][0];
				if (data[i][0]<elbow_MIN) elbow_MIN = data[i][0];
				if (data[i][1]>hand_MAX) hand_MAX = data[i][1];
				if (data[i][1] < hand_MIN) hand_MIN = data[i][1];
			}
			elbow_range = elbow_MAX - elbow_MIN;
			hand_range = hand_MAX - hand_MIN;
			cout << "R_e:" << p_e.x << "," << p_e.y << "  手肘运动范围：" << elbow_range << endl;
			cout << "R_h:" << p_h.x << "," << p_h.y << "  手掌运动范围：" << hand_range << endl;
			// 手肘、手掌运动阈值判定
			if (hand_range >= 250 && elbow_range <= 100)
			{
				cout << "识别到挥手动作" << endl;
				return true;
			}
			else //cout << "未识别" << endl;
				return false;
		}
		else
		{
			//cout << "不满足手掌高于手肘条件" << endl;
			return false;
		}
	}
	else return false;
}
