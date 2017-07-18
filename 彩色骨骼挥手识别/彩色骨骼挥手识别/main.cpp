#include "global.h"

CvFont font;

int main(void)
{

	cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1.5f, 1.5f, 0, 2, CV_AA);//������ʾ������

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

