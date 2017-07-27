#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>
#include <Kinect.h>  
#include <Kinect.Face.h>
#pragma comment ( lib, "kinect20.lib" )  
#pragma comment ( lib, "Kinect20.face.lib" )  

using namespace cv;
using namespace std;

void drawline(cv::Mat & img, Joint & r_1, Joint & r_2, ICoordinateMapper * myMapper);
void drawhandstate(cv::Mat & img, Joint & lefthand, Joint & righthand, IBody* myBodyArr, ICoordinateMapper * myMapper);
void DrawBody(cv::Mat & img, Joint *myJointArr, ICoordinateMapper * myMapper);
bool GestureDetection(Joint & elbow, Joint & hand, ICoordinateMapper * myMapper);

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

int data[10][2];

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
	// ������ʾ״̬���ı�
	string property[FaceProperty_Count];
	property[0] = "Happy";
	property[1] = "Engaged";
	property[2] = "WearingGlasses";
	property[3] = "LeftEyeClosed";
	property[4] = "RightEyeClosed";
	property[5] = "MouthOpen";
	property[6] = "MouthMoved";
	property[7] = "LookingAway";

	while (1)
	{
		/********************************   ��ɫ֡  ********************************/
		while (myColorReader->AcquireLatestFrame(&myColorFrame) != S_OK);
		myColorFrame->CopyConvertedFrameDataToArray(colorHeight * colorWidth * 4, color.data, ColorImageFormat_Bgra); // ͼ���С��Ŀ���ַ��ͼ���ʽ
		myColorFrame->Release();
		//imshow("Color", color);

		/********************************   ���֡  ********************************/
		while (myDepthReader->AcquireLatestFrame(&myDepthFrame) != S_OK);
		myDepthFrame->CopyFrameDataToArray(depthHeight * depthWidth, (UINT16 *)depth.data); //�Ȱ����ݴ���16λ��ͼ�������
		myDepthFrame->Release();
		Mat showdepth(depthHeight, depthWidth, CV_8UC1);
		depth.convertTo(showdepth, CV_8UC1, 255.0 / 4500);   //�ٰ�16λת��Ϊ8λ
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
		imshow("BodyIndex", bodyindex);

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
					colormuti = color.clone();
					// ��������ؽ�
					DrawBody(colormuti, myJointArr, myMapper);
					// �����ֲ�״̬
					drawhandstate(colormuti, myJointArr[JointType_HandLeft], myJointArr[JointType_HandRight], myBodyArr[i], myMapper);
					// ���ּ��
					GestureDetection(myJointArr[JointType_ElbowRight], myJointArr[JointType_HandRight], myMapper);
				}
				UINT64 trackingId = _UI64_MAX;
				hr = myBodyArr[i]->get_TrackingId(&trackingId);
				if (SUCCEEDED(hr)) 
				{
					facesource[i]->put_TrackingId(trackingId);
					cout << "׷�ٵ�����ID: " << trackingId << endl;
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
						vector<string> result;
						Vector4 faceRotation;
						hr = faceresult->get_FaceRotationQuaternion(&faceRotation);
						if (SUCCEEDED(hr))
						{
							// ����Ԫ���ֽ�����������λ����Ϣ
							int pitch, yaw, roll;
							ExtractFaceRotationInDegrees(&faceRotation, &pitch, &yaw, &roll);
							result.push_back("Pitch, Yaw, Roll : " + std::to_string(pitch) + ", " + std::to_string(yaw) + ", " + std::to_string(roll));
						}

						// �沿������Ϣ
						DetectionResult faceProperty[FaceProperty::FaceProperty_Count];
						hr = faceresult->GetFaceProperties(FaceProperty::FaceProperty_Count, faceProperty);
						if (SUCCEEDED(hr))
						{
							for (int count = 0; count < FaceProperty::FaceProperty_Count; count++)
							{
								switch (faceProperty[count])
								{
								case DetectionResult::DetectionResult_Unknown:
									result.push_back(property[count] + " : Unknown");
									break;
								case DetectionResult::DetectionResult_Yes:
									result.push_back(property[count] + " : Yes");
									break;
								case DetectionResult::DetectionResult_No:
									result.push_back(property[count] + " : No");
									break;
								case DetectionResult::DetectionResult_Maybe:
									result.push_back(property[count] + " : Mayby");
									break;
								default:
									break;
								}
							}
						}

						if (box.Left && box.Bottom)
						{
							int offset = 30;
							// ��ʾ�ַ���
							for (int i = 0; i < 8; i++, offset += 30)
							{
								putText(colormuti, result[i], cv::Point(box.Left, box.Bottom + offset), FONT_HERSHEY_COMPLEX, 1.0f, Scalar(0, 0, 255, 255), 2, CV_AA);
							}
						}

					}
					SafeRelease(faceresult);
				}
			}
			SafeRelease(faceframe);
		}

		Mat faceimg;
		cv::resize(colormuti, faceimg, cv::Size(), 0.5, 0.5);
		cv::imshow("Face", faceimg);

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

void    drawline(Mat & img, Joint & r_1, Joint & r_2, ICoordinateMapper * myMapper)
{
	//�������ؽڵ������߶ε����ˣ����ҽ���״̬����
	if (r_1.TrackingState == TrackingState_Tracked && r_2.TrackingState == TrackingState_Tracked)
	{
		ColorSpacePoint t_point;    //Ҫ�ѹؽڵ��õ�����������µĵ�ת���ɲ�ɫ�ռ�ĵ�
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
		ColorSpacePoint l_point, r_point;    //Ҫ�ѹؽڵ��õ�����������µĵ�ת���ɲ�ɫ�ռ�ĵ�
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
		//cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1.5f, 1.5f, 0, 2, CV_AA);//������ʾ������
		//cvPutText(&IplImage(img), "LeftHand", Point(p_l.x + 50, p_l.y - 50), &font, CV_RGB(255, 0, 0));//��ɫ����ע��
		//cvPutText(&IplImage(img), "RightHand", Point(p_r.x + 50, p_r.y - 50), &font, CV_RGB(255, 0, 0));//��ɫ����ע��

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
	drawline(img, myJointArr[JointType_Head], myJointArr[JointType_Neck], myMapper);						// ͷ-��
	drawline(img, myJointArr[JointType_Neck], myJointArr[JointType_SpineShoulder], myMapper);				// ��-������

	drawline(img, myJointArr[JointType_SpineShoulder], myJointArr[JointType_ShoulderLeft], myMapper);		// ������-����
	drawline(img, myJointArr[JointType_SpineShoulder], myJointArr[JointType_SpineMid], myMapper);			// ������-������
	drawline(img, myJointArr[JointType_SpineShoulder], myJointArr[JointType_ShoulderRight], myMapper);		// ������-�Ҽ��

	drawline(img, myJointArr[JointType_ShoulderLeft], myJointArr[JointType_ElbowLeft], myMapper);			// ����-������
	drawline(img, myJointArr[JointType_SpineMid], myJointArr[JointType_SpineBase], myMapper);				// ������-������
	drawline(img, myJointArr[JointType_ShoulderRight], myJointArr[JointType_ElbowRight], myMapper);		// �Ҽ��-������

	drawline(img, myJointArr[JointType_ElbowLeft], myJointArr[JointType_WristLeft], myMapper);				// ������-������
	drawline(img, myJointArr[JointType_SpineBase], myJointArr[JointType_HipLeft], myMapper);				// ������-��貿
	drawline(img, myJointArr[JointType_SpineBase], myJointArr[JointType_HipRight], myMapper);				// ������-�ҿ貿
	drawline(img, myJointArr[JointType_ElbowRight], myJointArr[JointType_WristRight], myMapper);			// ������-������

	drawline(img, myJointArr[JointType_WristLeft], myJointArr[JointType_ThumbLeft], myMapper);				// ������-��Ĵָ
	drawline(img, myJointArr[JointType_WristLeft], myJointArr[JointType_HandLeft], myMapper);				// ������-������
	drawline(img, myJointArr[JointType_HipLeft], myJointArr[JointType_KneeLeft], myMapper);				// ��貿-��ϥ��
	drawline(img, myJointArr[JointType_HipRight], myJointArr[JointType_KneeRight], myMapper);				// �ҿ貿-��ϥ��
	drawline(img, myJointArr[JointType_WristRight], myJointArr[JointType_ThumbRight], myMapper);			// ������-��Ĵָ
	drawline(img, myJointArr[JointType_WristRight], myJointArr[JointType_HandRight], myMapper);			// ������-������

	drawline(img, myJointArr[JointType_HandLeft], myJointArr[JointType_HandTipLeft], myMapper);			// ������-��ָ��
	drawline(img, myJointArr[JointType_KneeLeft], myJointArr[JointType_FootLeft], myMapper);				// ��ϥ��-���
	drawline(img, myJointArr[JointType_KneeRight], myJointArr[JointType_FootRight], myMapper);				// ��ϥ��-�ҽ�
	drawline(img, myJointArr[JointType_HandRight], myJointArr[JointType_HandTipRight], myMapper);			// ������-��ָ��
}

// �жϹ���׷���������������׷��������ֲ�λ����������  
bool GestureDetection(Joint & elbow, Joint & hand, ICoordinateMapper * myMapper)
{
	// ����׷�����
	if (elbow.TrackingState == TrackingState_Tracked && hand.TrackingState == TrackingState_Tracked)
	{
		// Ҫ�ѹؽڵ��õ�����������µĵ�ת���ɲ�ɫ�ռ�ĵ�
		ColorSpacePoint t_point;
		Point	p_e, p_h;
		myMapper->MapCameraPointToColorSpace(elbow.Position, &t_point);
		p_e.x = t_point.X;
		p_e.y = t_point.Y;
		myMapper->MapCameraPointToColorSpace(hand.Position, &t_point);
		p_h.x = t_point.X;
		p_h.y = t_point.Y;
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
			cout << "R_e:" << p_e.x << "," << p_e.y << "  �����˶���Χ��" << elbow_range << endl;
			cout << "R_h:" << p_h.x << "," << p_h.y << "  �����˶���Χ��" << hand_range << endl;
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
