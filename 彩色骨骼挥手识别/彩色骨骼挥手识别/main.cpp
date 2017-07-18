#include "global.h"

bool IsSkeletonTrackedWell(Joint & shoulder, Joint & elbow, Joint & hand, ICoordinateMapper * myMapper);

Point   p_s, p_e, p_h;
int data[10][2] , j = 0;

int main(void)
{
	IKinectSensor   * mySensor = nullptr;
	GetDefaultKinectSensor(&mySensor);
	mySensor->Open();

	IColorFrameSource   * myColorSource = nullptr;
	mySensor->get_ColorFrameSource(&myColorSource);

	IColorFrameReader   * myColorReader = nullptr;
	myColorSource->OpenReader(&myColorReader);

	int colorHeight = 0, colorWidth = 0;
	IFrameDescription   * myDescription = nullptr;
	myColorSource->get_FrameDescription(&myDescription);
	myDescription->get_Height(&colorHeight);
	myDescription->get_Width(&colorWidth);

	IColorFrame * myColorFrame = nullptr;
	Mat original(colorHeight, colorWidth, CV_8UC4);

	//**********************����ΪColorFrame�Ķ�ȡǰ׼��**************************

	IBodyFrameSource    * myBodySource = nullptr;
	mySensor->get_BodyFrameSource(&myBodySource);

	IBodyFrameReader    * myBodyReader = nullptr;
	myBodySource->OpenReader(&myBodyReader);

	int myBodyCount = 0;
	myBodySource->get_BodyCount(&myBodyCount);

	IBodyFrame  * myBodyFrame = nullptr;

	ICoordinateMapper   * myMapper = nullptr;
	mySensor->get_CoordinateMapper(&myMapper);

	//**********************����ΪBodyFrame�Լ�Mapper��׼��***********************
	while (1)
	{
		while (myColorReader->AcquireLatestFrame(&myColorFrame) != S_OK);
		myColorFrame->CopyConvertedFrameDataToArray(colorHeight * colorWidth * 4, original.data, ColorImageFormat_Bgra); // ͼ���С��Ŀ���ַ��ͼ���ʽ
		Mat copy = original.clone();        //��ȡ��ɫͼ�����������

		while (myBodyReader->AcquireLatestFrame(&myBodyFrame) != S_OK); //��ȡ����ͼ��
		IBody   **  myBodyArr = new IBody *[myBodyCount];       //Ϊ���������ݵ�������׼��
		for (int i = 0; i < myBodyCount; i++)
			myBodyArr[i] = nullptr;

		if (myBodyFrame->GetAndRefreshBodyData(myBodyCount, myBodyArr) == S_OK)     //������������������
		for (int i = 0; i < myBodyCount; i++)
		{
			BOOLEAN     result = false;
			if (myBodyArr[i]->get_IsTracked(&result) == S_OK && result) //���ж��Ƿ���⵽
			{
				Joint   myJointArr[JointType_Count];
				if (myBodyArr[i]->GetJoints(JointType_Count, myJointArr) == S_OK)   //�����⵽�Ͱѹؽ��������뵽���鲢��ͼ
				{
					DrawBody(copy, myJointArr, myMapper);
					drawhandstate(copy, myJointArr[JointType_HandLeft], myJointArr[JointType_HandRight], myBodyArr[i], myMapper);
					IsSkeletonTrackedWell(myJointArr[JointType_ShoulderRight], myJointArr[JointType_ElbowRight], myJointArr[JointType_HandRight], myMapper);
				}
			}
		}
		delete[]myBodyArr;
		myBodyFrame->Release();
		myColorFrame->Release();

		imshow("TEST", copy);
		if (waitKey(30) == VK_ESCAPE)
			break;
	}
	myMapper->Release();

	myDescription->Release();
	myColorReader->Release();
	myColorSource->Release();

	myBodyReader->Release();
	myBodySource->Release();
	mySensor->Close();
	mySensor->Release();

	return  0;
}

// �жϹ���׷���������������׷��������ֲ�λ����������  
bool IsSkeletonTrackedWell(Joint & shoulder, Joint & elbow, Joint & hand, ICoordinateMapper * myMapper)
{
	if (shoulder.TrackingState == TrackingState_Tracked && elbow.TrackingState == TrackingState_Tracked && hand.TrackingState == TrackingState_Tracked)
	{
		ColorSpacePoint t_point;    //Ҫ�ѹؽڵ��õ�����������µĵ�ת���ɲ�ɫ�ռ�ĵ�
		myMapper->MapCameraPointToColorSpace(shoulder.Position, &t_point);
		p_s.x = t_point.X;
		p_s.y = t_point.Y;
		myMapper->MapCameraPointToColorSpace(elbow.Position, &t_point);
		p_e.x = t_point.X;
		p_e.y = t_point.Y;
		myMapper->MapCameraPointToColorSpace(hand.Position, &t_point);
		p_h.x = t_point.X;
		p_h.y = t_point.Y;
		if (p_h.y < p_e.y )
		{
			int elbow_range = 0 , hand_range = 0;
			int elbow_MAX = INT_MIN, elbow_MIN = INT_MAX, hand_MAX = INT_MIN, hand_MIN = INT_MAX;
			int LeftCo = 0, RightCo = 0;
			for (int i = 0; i < 9; i++)
			{
				data[i][0] = data[i + 1][0];
				data[i][1] = data[i + 1][1];				
			}
			data[9][0] = p_e.x;
			data[9][1] = p_h.x;
			for (int i = 0; i < 10; i++)
			{
				if (data[i][0]>elbow_MAX) elbow_MAX = data[i][0];
				if (data[i][0]<elbow_MIN) elbow_MIN = data[i][0];
				if (data[i][1]>hand_MAX) hand_MAX = data[i][1];
				if (data[i][1]<hand_MIN) hand_MIN = data[i][1];
				if (data[i][1] < data[i][0]) LeftCo++;
				else if (data[i][1] > data[i][0]) RightCo++;
				else;
				
			}
			elbow_range = elbow_MAX - elbow_MIN ;
			hand_range = hand_MAX - hand_MIN;
			cout << "R_e:" << p_e.x << "," << p_e.y << "  �����˶���Χ��" << elbow_range << endl;
			cout << "R_h:" << p_h.x << "," << p_h.y << "  �����˶���Χ��" << hand_range << endl;
			if (hand_range >= 250 && elbow_range <= 100 )
				cout << "ʶ�𵽻��ֶ���" << endl;
			else cout << "δʶ��" << endl;
			return true;
		}
		else
		{
			cout << "���������Ƹ�����������" << endl;
			return false;
		}
	}
	else return false;
}
