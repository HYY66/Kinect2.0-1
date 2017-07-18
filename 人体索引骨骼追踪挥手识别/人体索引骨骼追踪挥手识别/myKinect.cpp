#include "myKinect.h"
#include <iostream>

int myBodyCount;
int data[10][2];

// Initializes the default Kinect sensor
HRESULT CBodyBasics::InitializeDefaultSensor()
{
	// �����ж�ÿ�ζ�ȡ�����ĳɹ����
	HRESULT hr;
	// ����kinect
	GetDefaultKinectSensor(&m_pKinectSensor);
	// �ҵ�kinect�豸
	if (m_pKinectSensor)
	{
		// ��kinect
		hr = m_pKinectSensor->Open();
		// ������������Դ
		IBodyFrameSource* pBodyFrameSource = NULL;
		IBodyIndexFrameSource* pBodyIndexFrameSource = NULL;
		// ��ȡ�Ǽ�����Դ
		if (SUCCEEDED(hr))	hr = m_pKinectSensor->get_BodyFrameSource(&pBodyFrameSource);
		if (SUCCEEDED(hr))	hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
		// ��ù�������
		pBodyFrameSource->get_BodyCount(&myBodyCount);
		// ��ȡ��������ͼ����Դ
		if (SUCCEEDED(hr))	hr = m_pKinectSensor->get_BodyIndexFrameSource(&pBodyIndexFrameSource);
		if (SUCCEEDED(hr))	hr = pBodyIndexFrameSource->OpenReader(&m_pBodyIndexFrameReader);
		// Sourceʹ����ϣ��ͷ�����Դ
		SafeRelease(pBodyFrameSource);
		SafeRelease(pBodyIndexFrameSource);
		// ����ӳ��
		if (SUCCEEDED(hr))	hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
	}

	if (!m_pKinectSensor || FAILED(hr))
	{
		std::cout << "Kinect initialization failed!" << std::endl;
		return E_FAIL;
	}

	//���ڻ��Ǽܡ���������ͼ��MAT
	IndexImage.create(cDepthHeight, cDepthWidth, CV_8UC3);
	// ��ʼ��0
	IndexImage.setTo(0); 

	return hr;
}


// Main processing function
void CBodyBasics::Update()
{
	//ÿ�������skeletonImg
	IndexImage.setTo(0);
	//�����ʧ��kinect���򲻼�������
	if (!m_pBodyFrameReader)	return;
	// ����֡
	IBodyFrame* pBodyFrame = NULL;//�Ǽ���Ϣ
	IBodyIndexFrame* pBodyIndexFrame = NULL;//��������ͼ
	//��¼ÿ�β����ĳɹ����
	HRESULT hr = S_OK;
	//---------------------------------------��ȡ��������ͼ����ʾ---------------------------------
	//�����������ͼ
	if (SUCCEEDED(hr))	hr = m_pBodyIndexFrameReader->AcquireLatestFrame(&pBodyIndexFrame);
	if (SUCCEEDED(hr))
	{
		//��������ͼ��8λuchar�������Ǻ�ɫ��û���ǰ�ɫ
		BYTE *bodyIndexArray = new BYTE[cDepthHeight * cDepthWidth];
		// ��������ͼ������ bodyIndexArray
		pBodyIndexFrame->CopyFrameDataToArray(cDepthHeight * cDepthWidth, bodyIndexArray);
		//������ͼ����MAT�� ��ָ�� IndexData ָ������ IndexImage �ĵ�ַ
		uchar* IndexData = (uchar*)IndexImage.data;
		for (int j = 0; j < cDepthHeight * cDepthWidth; ++j){
			*IndexData = bodyIndexArray[j]; ++IndexData;
			*IndexData = bodyIndexArray[j]; ++IndexData;
			*IndexData = bodyIndexArray[j]; ++IndexData;
		}
		// �����ڴ�
		delete[] bodyIndexArray; 
	}
	//����Ҫ�ͷţ�����֮���޷�����µ�frame����
	SafeRelease(pBodyIndexFrame);

	//-----------------------------��ȡ�Ǽܲ���ʾ----------------------------
	//��ȡ�Ǽ���Ϣ
	if (SUCCEEDED(hr))	hr = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);
	if (SUCCEEDED(hr))
	{
		// ÿһ��IBody����׷��һ���ˣ��ܹ�����׷�������� . ��ά���飬һά��������Ŀ����ά�ǹؽ���Ŀ
		IBody   **  ppBodies = new IBody *[myBodyCount];
		// ���������ʼ��
		for (int i = 0; i < myBodyCount; i++)	ppBodies[i] = nullptr;
		//��kinect׷�ٵ����˵���Ϣ���ֱ�浽ÿһ��IBody��
		hr = pBodyFrame->GetAndRefreshBodyData(myBodyCount, ppBodies);

		//��ÿһ��IBody�������ҵ����ĹǼ���Ϣ�����һ�����
		if (SUCCEEDED(hr))	ProcessBody(myBodyCount, ppBodies);

		// �ͷ����������ڴ�
		for (int i = 0; i < myBodyCount; ++i)	SafeRelease(ppBodies[i]);
	}
	SafeRelease(pBodyFrame);//����Ҫ�ͷţ�����֮���޷�����µ�frame����
}

// Handle new body data
void CBodyBasics::ProcessBody(int nBodyCount, IBody** ppBodies)  // ppBodies ��ά���飬һά��������Ŀ����ά�ǹؽ���Ŀ
{
	//��¼��������Ƿ�ɹ�
	HRESULT hr;

	//����ÿһ��IBody
	for (int i = 0; i < nBodyCount; ++i)
	{
		IBody* pBody = ppBodies[i];
		if (pBody) //��û�и���������pBody�������bTracked��ʲô����
		{
			BOOLEAN bTracked = false;
			// ���屻׷�ٵ�����
			hr = pBody->get_IsTracked(&bTracked);
			if (SUCCEEDED(hr) && bTracked)
			{
				Joint joints[JointType_Count];//�洢�ؽڵ���
				HandState leftHandState = HandState_Unknown;//����״̬
				HandState rightHandState = HandState_Unknown;//����״̬

				//��ȡ������״̬
				pBody->get_HandLeftState(&leftHandState);
				pBody->get_HandRightState(&rightHandState);

				//�洢�������ϵ�еĹؽڵ�λ��
				DepthSpacePoint *depthSpacePosition = new DepthSpacePoint[JointType_Count];

				//��ùؽڵ���
				hr = pBody->GetJoints(JointType_Count, joints);
				if (SUCCEEDED(hr))
				{
					for (int j = 0; j < JointType_Count; ++j)
					{
						//��ÿ���ؽڵ���������������ϵ��-1~1��ת���������ϵ��424*512��
						m_pCoordinateMapper->MapCameraPointToDepthSpace(joints[j].Position, &depthSpacePosition[j]);
					}

					//------------------------hand state left-------------------------------
					DrawHandState(depthSpacePosition[JointType_HandLeft], leftHandState);
					DrawHandState(depthSpacePosition[JointType_HandRight], rightHandState);

					//---------------------------body-------------------------------
					DrawBone(joints, depthSpacePosition, JointType_Head, JointType_Neck);
					DrawBone(joints, depthSpacePosition, JointType_Neck, JointType_SpineShoulder);
					DrawBone(joints, depthSpacePosition, JointType_SpineShoulder, JointType_SpineMid);
					DrawBone(joints, depthSpacePosition, JointType_SpineMid, JointType_SpineBase);
					DrawBone(joints, depthSpacePosition, JointType_SpineShoulder, JointType_ShoulderRight);
					DrawBone(joints, depthSpacePosition, JointType_SpineShoulder, JointType_ShoulderLeft);
					DrawBone(joints, depthSpacePosition, JointType_SpineBase, JointType_HipRight);
					DrawBone(joints, depthSpacePosition, JointType_SpineBase, JointType_HipLeft);

					// -----------------------Right Arm ------------------------------------ 
					DrawBone(joints, depthSpacePosition, JointType_ShoulderRight, JointType_ElbowRight);
					DrawBone(joints, depthSpacePosition, JointType_ElbowRight, JointType_WristRight);
					DrawBone(joints, depthSpacePosition, JointType_WristRight, JointType_HandRight);
					DrawBone(joints, depthSpacePosition, JointType_HandRight, JointType_HandTipRight);
					DrawBone(joints, depthSpacePosition, JointType_WristRight, JointType_ThumbRight);

					//----------------------------------- Left Arm--------------------------
					DrawBone(joints, depthSpacePosition, JointType_ShoulderLeft, JointType_ElbowLeft);
					DrawBone(joints, depthSpacePosition, JointType_ElbowLeft, JointType_WristLeft);
					DrawBone(joints, depthSpacePosition, JointType_WristLeft, JointType_HandLeft);
					DrawBone(joints, depthSpacePosition, JointType_HandLeft, JointType_HandTipLeft);
					DrawBone(joints, depthSpacePosition, JointType_WristLeft, JointType_ThumbLeft);

					// ----------------------------------Right Leg--------------------------------
					DrawBone(joints, depthSpacePosition, JointType_HipRight, JointType_KneeRight);
					DrawBone(joints, depthSpacePosition, JointType_KneeRight, JointType_AnkleRight);
					DrawBone(joints, depthSpacePosition, JointType_AnkleRight, JointType_FootRight);

					// -----------------------------------Left Leg---------------------------------
					DrawBone(joints, depthSpacePosition, JointType_HipLeft, JointType_KneeLeft);
					DrawBone(joints, depthSpacePosition, JointType_KneeLeft, JointType_AnkleLeft);
					DrawBone(joints, depthSpacePosition, JointType_AnkleLeft, JointType_FootLeft);
				}

				// ���ּ��
				GestureDetection(joints[JointType_ElbowRight], joints[JointType_HandRight], m_pCoordinateMapper);

				// �����ڴ�
				delete[] depthSpacePosition;
			}
		}
	}
	cv::imshow("IndexImage", IndexImage);
	cv::waitKey(5);
}

//���ֵ�״̬
void CBodyBasics::DrawHandState(const DepthSpacePoint depthSpacePosition, HandState handState)
{
	//����ͬ�����Ʒ��䲻ͬ��ɫ
	CvScalar color;
	switch (handState){
	case HandState_Open:
		color = cvScalar(255, 0, 0);
		break;
	case HandState_Closed:
		color = cvScalar(0, 255, 0);
		break;
	case HandState_Lasso:
		color = cvScalar(0, 0, 255);
		break;
	default://���û��ȷ�������ƣ��Ͳ�Ҫ��
		return;
	}

	circle(IndexImage,cvPoint(depthSpacePosition.X, depthSpacePosition.Y),20, color, -1); // �ռ������д
}


/// Draws one bone of a body (joint to joint)
void CBodyBasics::DrawBone(const Joint* pJoints, const DepthSpacePoint* depthSpacePosition, JointType joint0, JointType joint1)
{
	TrackingState joint0State = pJoints[joint0].TrackingState;
	TrackingState joint1State = pJoints[joint1].TrackingState;

	// If we can't find either of these joints, exit 
	if ((joint0State == TrackingState_NotTracked) || (joint1State == TrackingState_NotTracked))
	{
		return;
	}

	// Don't draw if both points are inferred �ƶ�
	if ((joint0State == TrackingState_Inferred) && (joint1State == TrackingState_Inferred))
	{
		return;
	}

	CvPoint p1 = cvPoint(depthSpacePosition[joint0].X, depthSpacePosition[joint0].Y),
			p2 = cvPoint(depthSpacePosition[joint1].X, depthSpacePosition[joint1].Y);

	// We assume all drawn bones are inferred unless BOTH joints are tracked
	if ((joint0State == TrackingState_Tracked) && (joint1State == TrackingState_Tracked))
	{
		//�ǳ�ȷ���ĹǼܣ��ð�ɫֱ��
		line(IndexImage, p1, p2, cvScalar(255, 255, 255));
	}
	else
	{
		//��ȷ���ĹǼܣ��ú�ɫֱ��
		line(IndexImage, p1, p2, cvScalar(0, 0, 255));
	}
}


// Constructor ���������������Ǳ�����ȱʡֵ
CBodyBasics::CBodyBasics() :
m_pKinectSensor(NULL),
m_pCoordinateMapper(NULL),
m_pBodyFrameReader(NULL){}

// Destructor ������ �ͷŶ���
CBodyBasics::~CBodyBasics()
{
	SafeRelease(m_pBodyFrameReader);
	SafeRelease(m_pBodyIndexFrameReader);

	SafeRelease(m_pCoordinateMapper);

	if (m_pKinectSensor)
	{
		m_pKinectSensor->Close();
	}
	SafeRelease(m_pKinectSensor);
}


// �жϹ���׷���������������׷��������ֲ�λ����������  
bool GestureDetection(Joint & elbow, Joint & hand, ICoordinateMapper * myMapper)
{
	// ����׷�����
	if (elbow.TrackingState == TrackingState_Tracked && hand.TrackingState == TrackingState_Tracked)
	{
		DepthSpacePoint d_point;
		CvPoint p_e, p_h ;
		myMapper->MapCameraPointToDepthSpace(elbow.Position, &d_point);
		p_e.x = d_point.X;
		p_e.y = d_point.Y;
		myMapper->MapCameraPointToDepthSpace(hand.Position, &d_point);
		p_h.x = d_point.X;
		p_h.y = d_point.Y;
		//cout << "R_e:" << p_e.x << "," << p_e.y << endl;
		//cout << "R_h:" << p_h.x << "," << p_h.y << endl;
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
			if (hand_range >= 100 && elbow_range <= 50)
			{
				cout << "ʶ�𵽻��ֶ���" << endl;
				return true;
			}
			else
			{
				cout << "δʶ��" << endl;
				return false;
			}
		}
		else
		{
			//cout << "���������Ƹ�����������" << endl;
			return false;
		}
	}
	else return false;
}
