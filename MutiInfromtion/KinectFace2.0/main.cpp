#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"

#include <Kinect.h>  
#include <Kinect.Face.h>
#include <Kinect.VisualGestureBuilder.h>

#pragma comment ( lib, "kinect20.lib" )  
#pragma comment ( lib, "Kinect20.face.lib" )  
#pragma comment ( lib, "Kinect20.VisualGestureBuilder.lib" )  

#define USEColorSpace 1
#define USEDepthSpace 2

using namespace cv;
using namespace std;

Point MapCameraPointToSomeSpace(ICoordinateMapper * myMapper, Joint & joint, int SpaceFlag);
void drawline(cv::Mat & img, Joint & r_1, Joint & r_2, ICoordinateMapper * myMapper, int SpaceFlag);
void drawhandstate(cv::Mat & img, Joint & lefthand, Joint & righthand, IBody* myBodyArr, ICoordinateMapper * myMapper, int SpaceFlag);
void DrawBody(cv::Mat & img, Joint *myJointArr, ICoordinateMapper * myMapper, int SpaceFlag);
bool WaveGestureDetection(Joint & elbow, Joint & hand, ICoordinateMapper * myMapper);

template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
	if (pInterfaceToRelease != NULL)
	{
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}

// ����λ��״̬   ��Ԫ��->������ҡͷ����ͷ
inline void ExtractFaceRotationInDegrees(const Vector4* pQuaternion, int* pPitch, int* pYaw, int* pRoll)
{
	double x = pQuaternion->x;
	double y = pQuaternion->y;
	double z = pQuaternion->z;
	double w = pQuaternion->w;
	// �����ƻ��ɽǶ���
	*pPitch = static_cast<int>(std::atan2(2 * (y * z + w * x), w * w - x * x - y * y + z * z) / 3.14159265358979 * 180.0f);
	*pYaw = static_cast<int>(std::asin(2 * (w * y - x * z)) / 3.14159265358979 * 180.0f);
	*pRoll = static_cast<int>(std::atan2(2 * (x * y + w * z), w * w + x * x - y * y - z * z) / 3.14159265358979 * 180.0f);
}
//���ĳ������ֵ  
int get_pixel(Mat & img, Point pt) {
	int width = img.cols; //ͼƬ���  
	int height = img.rows; //ͼƬ���t;//ͼƬ�߶�  
	uchar* ptr = (uchar*)img.data + pt.y * width; //��ûҶ�ֵ����ָ��  
	int intensity = ptr[pt.x];
	return intensity;
}
int data[10][2];
Point lefthandpoint, righthandpoint;
int leftdepth,  rightdepth;

int main()
{
	HRESULT hr = FALSE;
	/********************************   ���Kinect������  ********************************/
	IKinectSensor   * mySensor = nullptr;
	hr = GetDefaultKinectSensor(&mySensor);
	if (SUCCEEDED(hr))
		hr = mySensor->Open();

	/********************************   ӳ������ϵ  ********************************/
	ICoordinateMapper   * myMapper = nullptr;
	if (SUCCEEDED(hr))
		hr = mySensor->get_CoordinateMapper(&myMapper);

	/********************************   ��ɫ����  ********************************/
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
	cout << "��ɫͼ���С��" << colorWidth << "*" << colorHeight << " ���ݸ�ʽ��CV_8UC4" << endl;
	IColorFrame * myColorFrame = nullptr;
	Mat color(colorHeight, colorWidth, CV_8UC4);
	Mat colormuti(colorHeight, colorWidth, CV_8UC4);

	/********************************   �������  ********************************/
	IDepthFrameSource   * myDepthSource = nullptr;   //ȡ���������
	if (SUCCEEDED(hr))
		hr = mySensor->get_DepthFrameSource(&myDepthSource);
	IDepthFrameReader   * myDepthReader = nullptr;
	if (SUCCEEDED(hr))
		hr = myDepthSource->OpenReader(&myDepthReader);    //��������ݵ�Reader
	IFrameDescription   * mydepthDescription = nullptr;
	if (SUCCEEDED(hr))
		hr = myDepthSource->get_FrameDescription(&mydepthDescription);
	int depthHeight = 0, depthWidth = 0;
	mydepthDescription->get_Height(&depthHeight);
	mydepthDescription->get_Width(&depthWidth);
	cout << "���ͼ���С��" << depthWidth << " * " << depthHeight << " ���ݸ�ʽ��CV_16UC1" << endl;
	IDepthFrame * myDepthFrame = nullptr;
	Mat depth(depthHeight, depthWidth, CV_16UC1);    //����ͼ�����
	Mat showdepth(depthHeight, depthWidth, CV_8UC1);
	Mat depthmuti(depthHeight, depthWidth, CV_8UC1);

	/********************************   ��������  ********************************/
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
	cout << "����ͼ���С��" << infraredWidth << " * " << infraredHeight << " ���ݸ�ʽ��CV_16UC1" << endl;
	IInfraredFrame  * myInfraredFrame = nullptr;
	Mat infrared(infraredHeight, infraredWidth, CV_16UC1);

	/********************************   ��������  ********************************/
	IBodyIndexFrameSource* pBodyIndexFrameSource = NULL;//��ȡ������ֵͼ
	if (SUCCEEDED(hr))
		hr = mySensor->get_BodyIndexFrameSource(&pBodyIndexFrameSource);
	IBodyIndexFrameReader*  m_pBodyIndexFrameReader;//���ڱ�����ֵͼ��ȡ
	if (SUCCEEDED(hr))
		hr = pBodyIndexFrameSource->OpenReader(&m_pBodyIndexFrameReader);
	IFrameDescription   * mybodyindexDescription = nullptr;
	if (SUCCEEDED(hr))
		hr = pBodyIndexFrameSource->get_FrameDescription(&mybodyindexDescription);
	int bodyindexHeight = 0, bodyindexWidth = 0;
	mybodyindexDescription->get_Height(&bodyindexHeight);
	mybodyindexDescription->get_Width(&bodyindexWidth);
	cout << "����ͼ���С��" << bodyindexWidth << " * " << bodyindexHeight << " ���ݸ�ʽ��CV_8UC3" << endl;
	IBodyIndexFrame  * myBodyIndexFrame = nullptr;
	Mat bodyindex(bodyindexHeight, bodyindexWidth, CV_8UC3);
	Mat bodyindexmuti(bodyindexHeight, bodyindexWidth, CV_8UC3);

	/********************************   ��������  ********************************/
	IBodyFrameSource    * myBodySource = nullptr;
	if (SUCCEEDED(hr))
		mySensor->get_BodyFrameSource(&myBodySource);
	IBodyFrameReader    * myBodyReader = nullptr;
	if (SUCCEEDED(hr))
		myBodySource->OpenReader(&myBodyReader);
	cout << "����ͼ��û�пɹ���ʾ�����ݣ�" << endl;
	IBodyFrame  * myBodyFrame = nullptr;

	/********************************   �沿����  ********************************/
	IFaceFrameSource* facesource[BODY_COUNT];
	IFaceFrameReader* facereader[BODY_COUNT];
	DWORD features = FaceFrameFeatures::FaceFrameFeatures_BoundingBoxInColorSpace
		| FaceFrameFeatures::FaceFrameFeatures_PointsInColorSpace
		| FaceFrameFeatures::FaceFrameFeatures_RotationOrientation
		| FaceFrameFeatures::FaceFrameFeatures_Happy
		| FaceFrameFeatures::FaceFrameFeatures_RightEyeClosed
		| FaceFrameFeatures::FaceFrameFeatures_LeftEyeClosed
		| FaceFrameFeatures::FaceFrameFeatures_MouthOpen
		| FaceFrameFeatures::FaceFrameFeatures_MouthMoved
		| FaceFrameFeatures::FaceFrameFeatures_LookingAway
		| FaceFrameFeatures::FaceFrameFeatures_Glasses
		| FaceFrameFeatures::FaceFrameFeatures_FaceEngagement;
	for (int i = 0; i < BODY_COUNT; i++)
	{
		hr = CreateFaceFrameSource(mySensor, 0, features, &facesource[i]);
		if (FAILED(hr))
		{
			std::cerr << "Error : CreateFaceFrameSource" << std::endl;
			return -1;
		}
		facesource[i]->OpenReader(&facereader[i]);
	}

	/********************************   ��������  ********************************/
	IVisualGestureBuilderFrameSource* gesturesource[BODY_COUNT];
	IVisualGestureBuilderFrameReader* gesturereader[BODY_COUNT];
	for (int count = 0; count < BODY_COUNT; count++)
	{
		CreateVisualGestureBuilderFrameSource(mySensor, 0, &gesturesource[count]);
		if (FAILED(hr))
		{
			std::cerr << "Error : CreateVisualGestureBuilderFrameSource" << std::endl;
			return -1;
		}
		gesturesource[count]->OpenReader(&gesturereader[count]);
	}
	// ��ȡ����
	IVisualGestureBuilderDatabase* gesturedatabase;
	CreateVisualGestureBuilderDatabaseInstanceFromFile(L"wave.gba", &gesturedatabase);
	// �������е����ƺ���Ŀ
	UINT gesturecount = 0;
	gesturedatabase->get_AvailableGesturesCount(&gesturecount);
	IGesture* gesture;
	hr = gesturedatabase->get_AvailableGestures(gesturecount, &gesture);
	// ������Ƶ�����Դ�У���ʹ��
	if (SUCCEEDED(hr) && gesture != nullptr)
	{
		for (int count = 0; count < BODY_COUNT; count++)
		{
			gesturesource[count]->AddGesture(gesture);
			gesturesource[count]->SetIsEnabled(gesture, true);
		}
	}

	while (1)
	{
		/********************************   ��ɫ֡  ********************************/
		while (myColorReader->AcquireLatestFrame(&myColorFrame) != S_OK);
		myColorFrame->CopyConvertedFrameDataToArray(colorHeight * colorWidth * 4, color.data, ColorImageFormat_Bgra); // ͼ���С��Ŀ���ַ��ͼ���ʽ
		colormuti = color.clone();
		myColorFrame->Release();
		//imshow("Color", color);

		/********************************   ���֡  ********************************/
		while (myDepthReader->AcquireLatestFrame(&myDepthFrame) != S_OK);
		myDepthFrame->CopyFrameDataToArray(depthHeight * depthWidth, (UINT16 *)depth.data); //�Ȱ����ݴ���16λ��ͼ�������
		myDepthFrame->Release();
		depth.convertTo(showdepth, CV_8UC1, 255.0 / 4500);   //�ٰ�16λת��Ϊ8λ
		//cout << get_pixel(showdepth, Point(200, 200)) << endl;
		depthmuti = showdepth.clone();
		//imshow("Depth", showdepth);

		/********************************   ����֡  ********************************/
		while (myInfraredReader->AcquireLatestFrame(&myInfraredFrame) != S_OK);
		myInfraredFrame->CopyFrameDataToArray(infraredHeight * infraredWidth, (UINT16 *)infrared.data);
		myInfraredFrame->Release();
		//imshow("Infrared", infrared);

		/********************************   ����֡  ********************************/
		while (m_pBodyIndexFrameReader->AcquireLatestFrame(&myBodyIndexFrame) != S_OK);
		BYTE *bodyIndexArray = new BYTE[bodyindexHeight * bodyindexWidth];//������ֵͼ��8Ϊuchar�������Ǻ�ɫ��û���ǰ�ɫ
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
		bodyindexmuti = bodyindex.clone();
		//imshow("BodyIndex", bodyindex);

		/********************************   ����֡  ********************************/
		while (myBodyReader->AcquireLatestFrame(&myBodyFrame) != S_OK);
		IBody   **  myBodyArr = new IBody *[BODY_COUNT];
		for (int i = 0; i < BODY_COUNT; i++)	myBodyArr[i] = nullptr;
		if (myBodyFrame->GetAndRefreshBodyData(BODY_COUNT, myBodyArr) == S_OK)
#pragma omp parallel for 
		for (int i = 0; i < BODY_COUNT; i++)
		{
			BOOLEAN     result = false;
			if (myBodyArr[i]->get_IsTracked(&result) == S_OK && result)
			{
				Joint   myJointArr[JointType_Count];
				//�����⵽�Ͱѹؽ��������뵽���鲢��ͼ
				if (myBodyArr[i]->GetJoints(JointType_Count, myJointArr) == S_OK)
				{
					DrawBody(colormuti, myJointArr, myMapper, USEColorSpace);
					drawhandstate(colormuti, myJointArr[JointType_HandLeft], myJointArr[JointType_HandRight], myBodyArr[i], myMapper, USEColorSpace);
					WaveGestureDetection(myJointArr[JointType_ElbowRight], myJointArr[JointType_HandRight], myMapper);

					DrawBody(bodyindexmuti, myJointArr, myMapper, USEDepthSpace);
					drawhandstate(bodyindexmuti, myJointArr[JointType_HandLeft], myJointArr[JointType_HandRight], myBodyArr[i], myMapper, USEDepthSpace);
				
					DrawBody(depthmuti, myJointArr, myMapper, USEDepthSpace);
					drawhandstate(depthmuti, myJointArr[JointType_HandLeft], myJointArr[JointType_HandRight], myBodyArr[i], myMapper, USEDepthSpace);
					lefthandpoint = MapCameraPointToSomeSpace(myMapper, myJointArr[JointType_HandLeft],  USEDepthSpace);
					righthandpoint = MapCameraPointToSomeSpace(myMapper, myJointArr[JointType_HandRight], USEDepthSpace);
					leftdepth = get_pixel(showdepth, lefthandpoint);
					rightdepth = get_pixel(showdepth, righthandpoint);
					//cout << leftdepth << "  " << rightdepth << endl;

				}
				// �ѹ���ID��ֵ���沿ID
				UINT64 trackingId = _UI64_MAX;
				hr = myBodyArr[i]->get_TrackingId(&trackingId);
				//cout << "׷�ٵĹ���ID: " << trackingId << endl;
				if (SUCCEEDED(hr))
				{
					facesource[i]->put_TrackingId(trackingId);
				}
			}
		}
		delete[]myBodyArr;
		myBodyFrame->Release();

		/********************************   �沿֡  ********************************/
#pragma omp parallel for 
		for (int i = 0; i < BODY_COUNT; i++)
		{
			IFaceFrame*faceframe = nullptr;
			hr = facereader[i]->AcquireLatestFrame(&faceframe);
			if (faceframe == nullptr)
				continue;
			if (SUCCEEDED(hr) && faceframe != nullptr)
			{
				BOOLEAN tracked = false;
				hr = faceframe->get_IsTrackingIdValid(&tracked);
				if (SUCCEEDED(hr) && tracked)
				{
					IFaceFrameResult *faceresult = nullptr;
					UINT64 trackingID = NULL;
					faceframe->get_TrackingId(&trackingID);
					//cout << "׷�ٵ�����ID: " << trackingID << endl;
					hr = faceframe->get_FaceFrameResult(&faceresult);
					if (SUCCEEDED(hr))
					{
						PointF facepoint[FacePointType_Count];
						hr = faceresult->GetFacePointsInColorSpace(FacePointType_Count, facepoint);
						if (SUCCEEDED(hr))
						{
							circle(colormuti, cv::Point(facepoint[0].X, facepoint[0].Y), 5, Scalar(0, 0, 255, 255), -1, CV_AA); // Eye (Left)
							circle(colormuti, cv::Point(facepoint[1].X, facepoint[1].Y), 5, Scalar(0, 0, 255, 255), -1, CV_AA); // Eye (Right)
							circle(colormuti, cv::Point(facepoint[2].X, facepoint[2].Y), 5, Scalar(0, 0, 255, 255), -1, CV_AA); // Nose
							circle(colormuti, cv::Point(facepoint[3].X, facepoint[3].Y), 5, Scalar(0, 0, 255, 255), -1, CV_AA); // Mouth (Left)
							circle(colormuti, cv::Point(facepoint[4].X, facepoint[4].Y), 5, Scalar(0, 0, 255, 255), -1, CV_AA); // Mouth (Right)
						}
						RectI box;
						hr = faceresult->get_FaceBoundingBoxInColorSpace(&box);
						if (SUCCEEDED(hr))
						{
							cv::rectangle(colormuti, cv::Rect(box.Left, box.Top, box.Right - box.Left, box.Bottom - box.Top), Scalar(0, 0, 255, 255));
						}

						// ��Ԫ�� �ص�����
						string result;
						Vector4 faceRotation;
						hr = faceresult->get_FaceRotationQuaternion(&faceRotation);
						if (SUCCEEDED(hr))
						{
							// ����Ԫ���ֽ�����������λ����Ϣ
							int pitch, yaw, roll;
							ExtractFaceRotationInDegrees(&faceRotation, &pitch, &yaw, &roll);
							UINT64 trackingId = _UI64_MAX;
							hr = faceframe->get_TrackingId(&trackingId);
							result += "FaceID: " + to_string(trackingId)
								+ "  Pitch, Yaw, Roll : " + to_string(pitch) + ", " + to_string(yaw) + ", " + to_string(roll)
								+ "  Nose(" + to_string(int(facepoint[2].X)) + ", " + to_string(int(facepoint[2].Y)) + ")"
								+ "  Left(" + to_string(int(lefthandpoint.x)) + ", " + to_string(int(lefthandpoint.y)) +", " + to_string(leftdepth) + ")"
								+ "  Right(" + to_string(int(righthandpoint.x)) + ", " + to_string(int(righthandpoint.y)) + ", " + to_string(rightdepth) + ")";
						}
						putText(colormuti, result, Point(0, 40 * (i + 1)), FONT_HERSHEY_COMPLEX, 1.0f, Scalar(255, 255, 126, 255), 2, CV_AA);
					}
					SafeRelease(faceresult);
				}
			}
			SafeRelease(faceframe);
		}

		Mat show;
		cv::resize(colormuti, show, cv::Size(), 0.5, 0.5);
		cv::imshow("ColorMuti", show);

		imshow("BodyindexMuti", bodyindexmuti);

		imshow("DepthMuti", depthmuti);

		if (cv::waitKey(34) == VK_ESCAPE)
		{
			break;
		}
	}

	for (int i = 0; i < BODY_COUNT; i++)
	{
		SafeRelease(facesource[i]);
		SafeRelease(facereader[i]);
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

	cv::destroyAllWindows();
}

// ���ؽڵ�ӳ�䵽��ɫ�������ͼ��
// USEUSEColorSpace USEDepthSpace
Point MapCameraPointToSomeSpace(ICoordinateMapper * myMapper, Joint & joint, int SpaceFlag)
{
	Point   p;
	if (SpaceFlag == USEColorSpace)
	{
		ColorSpacePoint colorpoint;    //Ҫ�ѹؽڵ��õ�����������µĵ�ת���ɲ�ɫ�ռ�ĵ�
		myMapper->MapCameraPointToColorSpace(joint.Position, &colorpoint);
		p.x = colorpoint.X;
		p.y = colorpoint.Y;
	}
	if (SpaceFlag == USEDepthSpace)
	{
		DepthSpacePoint depthpoint;    //Ҫ�ѹؽڵ��õ�����������µĵ�ת������ȿռ�ĵ�
		myMapper->MapCameraPointToDepthSpace(joint.Position, &depthpoint);
		p.x = depthpoint.X;
		p.y = depthpoint.Y;
	}
	return p;
}

// ��ӳ���ͼ���ϻ�����
void    drawline(Mat & img, Joint & joint0, Joint & joint1, ICoordinateMapper * myMapper, int SpaceFlag)
{
	TrackingState joint0State = joint0.TrackingState;
	TrackingState joint1State = joint1.TrackingState;

	if ((joint0State == TrackingState_NotTracked) || (joint1State == TrackingState_NotTracked))		return;
	if ((joint0State == TrackingState_Inferred) && (joint1State == TrackingState_Inferred))	return;
	Point	p1 = MapCameraPointToSomeSpace(myMapper, joint0, SpaceFlag),
			p2 = MapCameraPointToSomeSpace(myMapper, joint1, SpaceFlag);
	circle(img, p1, 10 / SpaceFlag, Vec3b(255, 0, 0), -1);
	circle(img, p2, 10 / SpaceFlag, Vec3b(255, 0, 0), -1);
	if ((joint0State == TrackingState_Tracked) && (joint1State == TrackingState_Tracked)) //�ǳ�ȷ���ĹǼܣ��ð�ɫֱ��
	{
		line(img, p1, p2, cvScalar(255, 255, 255));
	}
}

// ��ӳ���ͼ���ϻ��ֵ�״̬
void    drawhandstate(Mat & img, Joint & lefthand, Joint & righthand, IBody* myBodyArr, ICoordinateMapper * myMapper, int SpaceFlag)
{
	if (lefthand.TrackingState == TrackingState_Tracked) 
	{
		Point   pl = MapCameraPointToSomeSpace(myMapper, lefthand, SpaceFlag);
		HandState left;
		myBodyArr->get_HandLeftState(&left);
		switch (left)
		{
		case HandState_Closed:
			circle(img, pl, 10, Scalar(0, 0, 255, 1), 20); break;
		case HandState_Open:
			circle(img, pl, 10, Scalar(0, 255, 0, 1), 20); break;
		case HandState_Lasso:
			circle(img, pl, 10, Scalar(255, 0, 0, 1), 20); break;
		}
	}
	if (righthand.TrackingState == TrackingState_Tracked)
	{
		Point	pr = MapCameraPointToSomeSpace(myMapper, righthand, SpaceFlag);
		HandState right;
		myBodyArr->get_HandRightState(&right);	
		switch (right)
		{
		case HandState_Closed:
			circle(img, pr, 10, Scalar(0, 0, 255, 1), 20); break;
		case HandState_Open:
			circle(img, pr, 10, Scalar(0, 255, 0, 1), 20); break;
		case HandState_Lasso:
			circle(img, pr, 10, Scalar(255, 0, 0, 1), 20); break;
		}
	}
}

// ��ӳ���ͼ���ϻ���������
void DrawBody(Mat & img, Joint *myJointArr, ICoordinateMapper * myMapper, int SpaceFlag)
{
	drawline(img, myJointArr[JointType_Head], myJointArr[JointType_Neck], myMapper, SpaceFlag);						// ͷ-��
	drawline(img, myJointArr[JointType_Neck], myJointArr[JointType_SpineShoulder], myMapper, SpaceFlag);				// ��-������

	drawline(img, myJointArr[JointType_SpineShoulder], myJointArr[JointType_ShoulderLeft], myMapper, SpaceFlag);		// ������-����
	drawline(img, myJointArr[JointType_SpineShoulder], myJointArr[JointType_SpineMid], myMapper, SpaceFlag);			// ������-������
	drawline(img, myJointArr[JointType_SpineShoulder], myJointArr[JointType_ShoulderRight], myMapper, SpaceFlag);		// ������-�Ҽ��

	drawline(img, myJointArr[JointType_ShoulderLeft], myJointArr[JointType_ElbowLeft], myMapper, SpaceFlag);			// ����-������
	drawline(img, myJointArr[JointType_SpineMid], myJointArr[JointType_SpineBase], myMapper, SpaceFlag);				// ������-������
	drawline(img, myJointArr[JointType_ShoulderRight], myJointArr[JointType_ElbowRight], myMapper, SpaceFlag);		// �Ҽ��-������

	drawline(img, myJointArr[JointType_ElbowLeft], myJointArr[JointType_WristLeft], myMapper, SpaceFlag);				// ������-������
	drawline(img, myJointArr[JointType_SpineBase], myJointArr[JointType_HipLeft], myMapper, SpaceFlag);				// ������-��貿
	drawline(img, myJointArr[JointType_SpineBase], myJointArr[JointType_HipRight], myMapper, SpaceFlag);				// ������-�ҿ貿
	drawline(img, myJointArr[JointType_ElbowRight], myJointArr[JointType_WristRight], myMapper, SpaceFlag);			// ������-������

	drawline(img, myJointArr[JointType_WristLeft], myJointArr[JointType_ThumbLeft], myMapper, SpaceFlag);				// ������-��Ĵָ
	drawline(img, myJointArr[JointType_WristLeft], myJointArr[JointType_HandLeft], myMapper, SpaceFlag);				// ������-������
	drawline(img, myJointArr[JointType_HipLeft], myJointArr[JointType_KneeLeft], myMapper, SpaceFlag);				// ��貿-��ϥ��
	drawline(img, myJointArr[JointType_HipRight], myJointArr[JointType_KneeRight], myMapper, SpaceFlag);				// �ҿ貿-��ϥ��
	drawline(img, myJointArr[JointType_WristRight], myJointArr[JointType_ThumbRight], myMapper, SpaceFlag);			// ������-��Ĵָ
	drawline(img, myJointArr[JointType_WristRight], myJointArr[JointType_HandRight], myMapper,SpaceFlag);			// ������-������

	drawline(img, myJointArr[JointType_HandLeft], myJointArr[JointType_HandTipLeft], myMapper,SpaceFlag);			// ������-��ָ��
	drawline(img, myJointArr[JointType_KneeLeft], myJointArr[JointType_FootLeft], myMapper, SpaceFlag);				// ��ϥ��-���
	drawline(img, myJointArr[JointType_KneeRight], myJointArr[JointType_FootRight], myMapper, SpaceFlag);				// ��ϥ��-�ҽ�
	drawline(img, myJointArr[JointType_HandRight], myJointArr[JointType_HandTipRight], myMapper, SpaceFlag);			// ������-��ָ��
}

// �жϹ���׷���������������׷��������ֲ�λ����������  
bool WaveGestureDetection(Joint & elbow, Joint & hand, ICoordinateMapper * myMapper)
{
	// ����׷�����
	if (elbow.TrackingState == TrackingState_Tracked && hand.TrackingState == TrackingState_Tracked)
	{
		// Ҫ�ѹؽڵ��õ�����������µĵ�ת���ɲ�ɫ�ռ�ĵ�
		Point	p_e = MapCameraPointToSomeSpace(myMapper, elbow, USEColorSpace), 
				p_h = MapCameraPointToSomeSpace(myMapper, hand, USEColorSpace);
		// ���Ƹ�������
		if (p_h.y < p_e.y)
		{
			int elbow_range = 0, hand_range = 0;
			int elbow_MAX = INT_MIN, elbow_MIN = INT_MAX, hand_MAX = INT_MIN, hand_MIN = INT_MAX;
			// �����10֡�����ƺ�����λ�ã����������ݸ���
			for (int i = 0; i < 9; i++)
			{
				data[i][0] = data[i + 1][0];
				data[i][1] = data[i + 1][1];
			}
			data[9][0] = p_e.x;
			data[9][1] = p_h.x;
			// �������ƺ�������˶���Χ
			for (int i = 0; i < 10; i++)
			{
				if (data[i][0]>elbow_MAX) elbow_MAX = data[i][0];
				if (data[i][0]<elbow_MIN) elbow_MIN = data[i][0];
				if (data[i][1]>hand_MAX) hand_MAX = data[i][1];
				if (data[i][1] < hand_MIN) hand_MIN = data[i][1];
			}
			elbow_range = elbow_MAX - elbow_MIN;
			hand_range = hand_MAX - hand_MIN;
			//cout << "R_e:" << p_e.x << "," << p_e.y << "  �����˶���Χ��" << elbow_range << endl;
			//cout << "R_h:" << p_h.x << "," << p_h.y << "  �����˶���Χ��" << hand_range << endl;
			// ���⡢�����˶���ֵ�ж�
			if (hand_range >= 250 && elbow_range <= 100)
			{
				cout << "ʶ�𵽻��ֶ���" << endl;
				return true;
			}
			else //cout << "δʶ��" << endl;
				return false;
		}
		else
		{
			//cout << "���������Ƹ�����������" << endl;
			return false;
		}
	}
	else return false;
}
