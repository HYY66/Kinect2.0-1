#include "global.h"

bool GestureDetection(Joint & elbow, Joint & hand, ICoordinateMapper * myMapper);

int data[10][2] ;

int main(void)
{
	// ���Kinect������
	IKinectSensor   * mySensor = nullptr;
	GetDefaultKinectSensor(&mySensor);
	// ��Kinect
	mySensor->Open();

	//**********************ColorFrame�Ķ�ȡǰ׼��**************************
	// ��ɫ����Դ
	IColorFrameSource   * myColorSource = nullptr;
	mySensor->get_ColorFrameSource(&myColorSource);
	IColorFrameReader   * myColorReader = nullptr;
	myColorSource->OpenReader(&myColorReader);
	// ��ò�ɫͼ���С
	int colorHeight = 0, colorWidth = 0;
	IFrameDescription   * myDescription = nullptr;
	myColorSource->get_FrameDescription(&myDescription);
	myDescription->get_Height(&colorHeight);
	myDescription->get_Width(&colorWidth);
	// ��ɫ֡
	IColorFrame * myColorFrame = nullptr;
	Mat original(colorHeight, colorWidth, CV_8UC4);

	//**********************BodyFrame�Լ�Mapper��׼��***********************
	// ��������Դ
	IBodyFrameSource    * myBodySource = nullptr;
	mySensor->get_BodyFrameSource(&myBodySource);
	IBodyFrameReader    * myBodyReader = nullptr;
	myBodySource->OpenReader(&myBodyReader);
	// ��ù�������
	int myBodyCount = 0;
	myBodySource->get_BodyCount(&myBodyCount);
	// ����֡
	IBodyFrame  * myBodyFrame = nullptr;
	// ӳ������ϵ
	ICoordinateMapper   * myMapper = nullptr;
	mySensor->get_CoordinateMapper(&myMapper);

	// ������
	while (1)
	{
		// ��ȡ��ɫͼ��
		while (myColorReader->AcquireLatestFrame(&myColorFrame) != S_OK);
		// ��ͼ��֡������������ Bgra��ʽ
		myColorFrame->CopyConvertedFrameDataToArray(colorHeight * colorWidth * 4, original.data, ColorImageFormat_Bgra); // ͼ���С��Ŀ���ַ��ͼ���ʽ
		// ��ȡ��ɫͼ�����������
		Mat copy = original.clone();   

		// ��ȡ����ͼ��
		while (myBodyReader->AcquireLatestFrame(&myBodyFrame) != S_OK); 
		// Ϊ���������ݵ�������׼������ά���飬һά��������Ŀ����ά�ǹؽ���Ŀ
		IBody   **  myBodyArr = new IBody *[myBodyCount]; 
		// ���������ʼ��
		for (int i = 0; i < myBodyCount; i++)	myBodyArr[i] = nullptr;
		//������������������
		if (myBodyFrame->GetAndRefreshBodyData(myBodyCount, myBodyArr) == S_OK)   

		// ��ÿһ���������д���
		for (int i = 0; i < myBodyCount; i++)
		{
			BOOLEAN     result = false;
			//���ж��Ƿ���⵽�ù���
			if (myBodyArr[i]->get_IsTracked(&result) == S_OK && result) 
			{
				Joint   myJointArr[JointType_Count];
				//�����⵽�Ͱѹؽ��������뵽���鲢��ͼ
				if (myBodyArr[i]->GetJoints(JointType_Count, myJointArr) == S_OK)   
				{
					// ��������ؽ�
					DrawBody(copy, myJointArr, myMapper);
					// �����ֲ�״̬
					drawhandstate(copy, myJointArr[JointType_HandLeft], myJointArr[JointType_HandRight], myBodyArr[i], myMapper);
					// ���ּ��
					GestureDetection(myJointArr[JointType_ElbowRight], myJointArr[JointType_HandRight], myMapper);
				}
			}
		}
		// �ڴ����
		delete[]myBodyArr;
		// ����֡������while(1)��ĩβ�ͷţ�Ϊ��һ�ε����ݻ�ȡ��׼��
		myBodyFrame->Release();
		myColorFrame->Release();

		imshow("��ɫ��������ʶ��", copy);
		if (waitKey(1) == 'q') 
			imwrite("1.jpg", copy);
		if (waitKey(30) == VK_ESCAPE)	
			break;
	}

	// �����ʼ�෴�����������ͷ�
	myMapper->Release();

	myColorFrame->Release();
	myDescription->Release();
	myColorReader->Release();
	myColorSource->Release();

	myBodyFrame->Release();
	myBodyReader->Release();
	myBodySource->Release();

	mySensor->Close();
	mySensor->Release();

	return  0;
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
