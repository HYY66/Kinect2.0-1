#include "util.h"
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
			circle(img, pl, 10, Scalar(0, 0, 255, 1), 20); 
			lhandstate = "Close";
			break;
		case HandState_Open:
			circle(img, pl, 10, Scalar(0, 255, 0, 1), 20); 
			lhandstate = "Open"; 
			break;
		case HandState_Lasso:
			circle(img, pl, 10, Scalar(255, 0, 0, 1), 20); 
			lhandstate = "Lasso"; 
			break;
		default:lhandstate = "Unknown";
			break;
		}
	}
	else
	{
		lhandstate = "NotTracked";
	}
	if (righthand.TrackingState == TrackingState_Tracked)
	{
		Point	pr = MapCameraPointToSomeSpace(myMapper, righthand, SpaceFlag);
		HandState right;
		myBodyArr->get_HandRightState(&right);
		switch (right)
		{
		case HandState_Closed:
			circle(img, pr, 10, Scalar(0, 0, 255, 1), 20); 
			rhandstate = "Close"; 
			break;
		case HandState_Open:
			circle(img, pr, 10, Scalar(0, 255, 0, 1), 20); 
			rhandstate = "Open"; 
			break;
		case HandState_Lasso:
			circle(img, pr, 10, Scalar(255, 0, 0, 1), 20); 
			rhandstate = "Lasso"; 
			break;
		default:lhandstate = "Unknown";
			break;
		}
	}
	else
	{
		rhandstate = "NotTracked";
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
	drawline(img, myJointArr[JointType_WristRight], myJointArr[JointType_HandRight], myMapper, SpaceFlag);			// ������-������

	drawline(img, myJointArr[JointType_HandLeft], myJointArr[JointType_HandTipLeft], myMapper, SpaceFlag);			// ������-��ָ��
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

//���ĳ������ֵ  
int get_pixel(Mat & img, Point pt) {
	int width = img.cols; //ͼƬ���  
	int height = img.rows; //ͼƬ���t;//ͼƬ�߶�  
	uchar* ptr = (uchar*)img.data + pt.y * width; //��ûҶ�ֵ����ָ��  
	int intensity = ptr[pt.x];
	return intensity;
}
