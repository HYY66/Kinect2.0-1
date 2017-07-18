#include "draw.h"

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