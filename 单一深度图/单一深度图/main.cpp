#include <Kinect.h>		//Kinect��ͷ�ļ�
#include <iostream>
#include <opencv2\imgproc.hpp>	//opencvͷ�ļ�
#include <opencv2\calib3d.hpp>
#include <opencv2\highgui.hpp>

using   namespace   std;
using   namespace   cv;

//���ĳ������ֵ  
int get_pixel(Mat & img, Point pt) {
	int width = img.cols; //ͼƬ���  
	int height = img.rows; //ͼƬ���t;//ͼƬ�߶�  
	uchar* ptr = (uchar*)img.data + pt.y * width; //��ûҶ�ֵ����ָ��  
	int intensity = ptr[pt.x];
	return intensity;
}

int main(void)
{
	IKinectSensor* KinectSensor = nullptr;
	GetDefaultKinectSensor(&KinectSensor);  //��ȡ��Ӧ��
	KinectSensor->Open();           //�򿪸�Ӧ��
	cout << "��Kinect������" << endl;
	cout << endl;

	IDepthFrameSource   * DepthSource = nullptr;   //ȡ���������
	KinectSensor->get_DepthFrameSource(&DepthSource);
	int height = 0, width = 0;
	IFrameDescription   * myDescription = nullptr;  //ȡ��������ݵķֱ���
	DepthSource->get_FrameDescription(&myDescription);
	myDescription->get_Height(&height);
	myDescription->get_Width(&width);
	cout <<"���ͼ���С�ǣ�"<< height << " * " << width << endl;
	myDescription->Release();

	IDepthFrameReader   * DepthReader = nullptr;
	DepthSource->OpenReader(&DepthReader);    //��������ݵ�Reader

	IDepthFrame * myFrame = nullptr;
	Mat temp(height, width, CV_16UC1);    //����ͼ�����
	Mat img(height, width, CV_8UC1);
	while (1)
	{
		if (DepthReader->AcquireLatestFrame(&myFrame) == S_OK) //ͨ��Reader���Ի�ȡ���µ�һ֡������ݣ��������֡��,���ж��Ƿ�ɹ���ȡ
		{
			//ԭʼUINT16 ���ͼ���ʺ�������ʾ��������Ҫ����8λ�ľͿ�����
			myFrame->CopyFrameDataToArray(height * width, (UINT16 *)temp.data); //�Ȱ����ݴ���16λ��ͼ�������
			temp.convertTo(img, CV_8UC1, 255.0 / 4500);   //�ٰ�16λת��Ϊ8λ
			cout << get_pixel(img, Point(200, 200)) << endl;
			imshow("Depth", img);
			myFrame->Release();
		}
		//POINT pt;
		//GetCursorPos(&pt);
		//cout << "��ǰ���������Ϊ��" << pt.x << "," << pt.y << endl;
		if (waitKey(30) == VK_ESCAPE)
			break;
	}
	DepthReader->Release();        //�ͷŲ��õı������ҹرո�Ӧ��
	DepthSource->Release();
	KinectSensor->Close();
	KinectSensor->Release();

	return  0;
}

