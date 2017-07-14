#include "draw.h"

using   namespace   std;
using   namespace   cv;

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
					draw(copy, myJointArr[JointType_Head], myJointArr[JointType_Neck], myMapper);						// ͷ-��
					draw(copy, myJointArr[JointType_Neck], myJointArr[JointType_SpineShoulder], myMapper);				// ��-������

					draw(copy, myJointArr[JointType_SpineShoulder], myJointArr[JointType_ShoulderLeft], myMapper);		// ������-����
					draw(copy, myJointArr[JointType_SpineShoulder], myJointArr[JointType_SpineMid], myMapper);			// ������-������
					draw(copy, myJointArr[JointType_SpineShoulder], myJointArr[JointType_ShoulderRight], myMapper);		// ������-�Ҽ��

					draw(copy, myJointArr[JointType_ShoulderLeft], myJointArr[JointType_ElbowLeft], myMapper);			// ����-������
					draw(copy, myJointArr[JointType_SpineMid], myJointArr[JointType_SpineBase], myMapper);				// ������-������
					draw(copy, myJointArr[JointType_ShoulderRight], myJointArr[JointType_ElbowRight], myMapper);		// �Ҽ��-������

					draw(copy, myJointArr[JointType_ElbowLeft], myJointArr[JointType_WristLeft], myMapper);				// ������-������
					draw(copy, myJointArr[JointType_SpineBase], myJointArr[JointType_HipLeft], myMapper);				// ������-��貿
					draw(copy, myJointArr[JointType_SpineBase], myJointArr[JointType_HipRight], myMapper);				// ������-�ҿ貿
					draw(copy, myJointArr[JointType_ElbowRight], myJointArr[JointType_WristRight], myMapper);			// ������-������

					draw(copy, myJointArr[JointType_WristLeft], myJointArr[JointType_ThumbLeft], myMapper);				// ������-��Ĵָ
					draw(copy, myJointArr[JointType_WristLeft], myJointArr[JointType_HandLeft], myMapper);				// ������-������
					draw(copy, myJointArr[JointType_HipLeft], myJointArr[JointType_KneeLeft], myMapper);				// ��貿-��ϥ��
					draw(copy, myJointArr[JointType_HipRight], myJointArr[JointType_KneeRight], myMapper);				// �ҿ貿-��ϥ��
					draw(copy, myJointArr[JointType_WristRight], myJointArr[JointType_ThumbRight], myMapper);			// ������-��Ĵָ
					draw(copy, myJointArr[JointType_WristRight], myJointArr[JointType_HandRight], myMapper);			// ������-������

					draw(copy, myJointArr[JointType_HandLeft], myJointArr[JointType_HandTipLeft], myMapper);			// ������-��ָ��
					draw(copy, myJointArr[JointType_KneeLeft], myJointArr[JointType_FootLeft], myMapper);				// ��ϥ��-���
					draw(copy, myJointArr[JointType_KneeRight], myJointArr[JointType_FootRight], myMapper);				// ��ϥ��-�ҽ�
					draw(copy, myJointArr[JointType_HandRight], myJointArr[JointType_HandTipRight], myMapper);			// ������-��ָ��

					drawhandstate(copy, myJointArr[JointType_HandLeft], myJointArr[JointType_HandRight], myBodyArr[i], myMapper);
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

void    draw(Mat & img, Joint & r_1, Joint & r_2, ICoordinateMapper * myMapper)
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

void    drawhandstate(Mat & img, Joint & lefthand, Joint & righthand, IBody* myBodyArr , ICoordinateMapper * myMapper)
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