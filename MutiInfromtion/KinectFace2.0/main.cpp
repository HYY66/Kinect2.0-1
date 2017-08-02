#include "global.h"

int data[10][2];
Point leftdepthpoint, rightdepthpoint, leftcolorpoint, rightcolorpoint;
int leftdepth, rightdepth;
string lhandstate, rhandstate;

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

	/********************************   ROS�ͻ���  ********************************/
	//ros::NodeHandle nh;
	//char *ros_master = "192.168.137.5";

	//printf("Connecting to server at %s\n", ros_master);
	//nh.initNode(ros_master);

	//printf("Advertising cmd_vel message\n");
	//geometry_msgs::Twist twist_msg;
	//ros::Publisher cmd_vel_pub("cmd_vel", &twist_msg);
	//nh.advertise(cmd_vel_pub);

	//printf("Go robot go!\n");

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
		depth.convertTo(showdepth, CV_8UC1, 255.0 / 2500);   //�ٰ�16λת��Ϊ8λ
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
					// ��ɫ�ռ�
					DrawBody(colormuti, myJointArr, myMapper, USEColorSpace);
					drawhandstate(colormuti, myJointArr[JointType_HandLeft], myJointArr[JointType_HandRight], myBodyArr[i], myMapper, USEColorSpace);
					// ���ּ��
					WaveGestureDetection(myJointArr[JointType_ElbowRight], myJointArr[JointType_HandRight], myMapper);
					// ������λ����Ϣ
					leftcolorpoint = MapCameraPointToSomeSpace(myMapper, myJointArr[JointType_HandLeft], USEColorSpace);
					rightcolorpoint = MapCameraPointToSomeSpace(myMapper, myJointArr[JointType_HandRight], USEColorSpace);

					// ���������ռ�
					DrawBody(bodyindexmuti, myJointArr, myMapper, USEDepthSpace);
					drawhandstate(bodyindexmuti, myJointArr[JointType_HandLeft], myJointArr[JointType_HandRight], myBodyArr[i], myMapper, USEDepthSpace);
				
					// ��ȿռ�
					DrawBody(depthmuti, myJointArr, myMapper, USEDepthSpace);
					drawhandstate(depthmuti, myJointArr[JointType_HandLeft], myJointArr[JointType_HandRight], myBodyArr[i], myMapper, USEDepthSpace);
					// ������λ����Ϣ
					leftdepthpoint = MapCameraPointToSomeSpace(myMapper, myJointArr[JointType_HandLeft],  USEDepthSpace);
					rightdepthpoint = MapCameraPointToSomeSpace(myMapper, myJointArr[JointType_HandRight], USEDepthSpace);
					// �����������Ϣ
					leftdepth = get_pixel(showdepth, leftdepthpoint);
					rightdepth = get_pixel(showdepth, rightdepthpoint);

					// ��ʾ��Ϣ
					string leftstr , rightstr ;
					leftstr = lhandstate + "(" + to_string(int(leftcolorpoint.x)) + ", " + to_string(int(leftcolorpoint.y)) + ", " + to_string(leftdepth) + ") \n";
					rightstr = rhandstate + "(" + to_string(int(rightcolorpoint.x)) + ", " + to_string(int(rightcolorpoint.y)) + ", " + to_string(rightdepth) + ") \n";
					//if (leftcolorpoint.y - 100 > 1 && rightcolorpoint.y - 100 > 1 && leftcolorpoint.x - 200 > 1 && rightcolorpoint.x - 200 > 1)
					//{
					//	putText(colormuti, leftstr, Point(leftcolorpoint.x - 200, leftcolorpoint.y - 100), FONT_HERSHEY_SIMPLEX, 1.0f, Scalar(0, 0, 255, 255), 2, CV_AA);
					//	putText(colormuti, rightstr, Point(rightcolorpoint.x - 200, rightcolorpoint.y - 100), FONT_HERSHEY_SIMPLEX, 1.0f, Scalar(0, 0, 255, 255), 2, CV_AA);
					//	//cout << leftdepth << "  " << rightdepth << endl;
					//}
					//if (lhandstate == "Close")
					//{
					//	twist_msg.linear.x = -0.5;
					//	twist_msg.linear.y = 0;
					//	twist_msg.linear.z = 0;
					//	twist_msg.angular.x = 0;
					//	twist_msg.angular.y = 0;
					//	twist_msg.angular.z = 0.2;
					//	cout << "���ٶ�Ϊ " << twist_msg.linear.x << endl;
					//}
					//else if (rhandstate == "Close")
					//{
					//	twist_msg.linear.x = 0.5;
					//	twist_msg.linear.y = 0;
					//	twist_msg.linear.z = 0;
					//	twist_msg.angular.x = 0;
					//	twist_msg.angular.y = 0;
					//	twist_msg.angular.z = 0;
					//	cout << "���ٶ�Ϊ " << twist_msg.linear.x << endl;
					//}
					//else
					//{
					//	twist_msg.linear.x = 0;
					//	twist_msg.linear.y = 0;
					//	twist_msg.linear.z = 0;
					//	twist_msg.angular.x = 0;
					//	twist_msg.angular.y = 0;
					//	twist_msg.angular.z = 0;
					//	cout << "ֹͣ�ȴ�ָ�" << endl;
					//}
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
									+"  Nose(" + to_string(int(facepoint[2].X)) + ", " + to_string(int(facepoint[2].Y)) + ")";
						}
						putText(colormuti, result, Point(0, 25 * (i + 1)), FONT_HERSHEY_SIMPLEX, 1.0f, Scalar(0, 0, 255, 255), 2, CV_AA);
					}
					SafeRelease(faceresult);
				}
			}
			SafeRelease(faceframe);
		}

		imshow("ColorMuti", colormuti);
		imshow("BodyindexMuti", bodyindexmuti);
		imshow("DepthMuti", depthmuti);

		if (waitKey(34) == VK_ESCAPE)
		{
			break;
		}

		//cmd_vel_pub.publish(&twist_msg);
		//nh.spinOnce();
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

