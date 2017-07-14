#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <Eigen/Core>
#include <Eigen/LU>
#include <thread>

using namespace cv;
using namespace std;

struct KinectParm
{
	// ��ɫ����ͷ���ࡢ����
	float fx_rgb;
	float fy_rgb;
	float cx_rgb;
	float cy_rgb;
	// ��������ͷ���ࡢ����
	float fx_ir;
	float fy_ir;
	float cx_ir;
	float cy_ir;

	// ��ת��ƽ�ƾ���
	Eigen::Matrix3f R_ir2rgb;
	Eigen::Vector3f T_ir2rgb;
};

bool loadParm(KinectParm* kinectParm)
{
	// ���ز���
	ifstream parm("parm.txt");
	string stringLine;
	if (parm.is_open())
	{
		// rgb���������fx,fy,cx,cy
		getline(parm, stringLine);
		stringstream lin(stringLine);
		string s1, s2, s3, s4, s5, s6, s7, s8, s9;
		lin >> s1 >> s2 >> s3 >> s4;
		kinectParm->fx_rgb = atof(s1.c_str());
		kinectParm->fy_rgb = atof(s2.c_str());
		kinectParm->cx_rgb = atof(s3.c_str());
		kinectParm->cy_rgb = atof(s4.c_str());
		stringLine.clear();
		// ir���������fx,fy,cx,cy
		getline(parm, stringLine);
		stringstream lin2(stringLine);
		lin2 << stringLine;
		lin2 >> s1 >> s2 >> s3 >> s4;
		kinectParm->fx_ir = atof(s1.c_str());
		kinectParm->fy_ir = atof(s2.c_str());
		kinectParm->cx_ir = atof(s3.c_str());
		kinectParm->cy_ir = atof(s4.c_str());
		stringLine.clear();

		// R_ir2rgb
		getline(parm, stringLine);
		stringstream lin3(stringLine);
		lin3 << stringLine;
		lin3 >> s1 >> s2 >> s3 >> s4 >> s5 >> s6 >> s7 >> s8 >> s9;
		kinectParm->R_ir2rgb <<
			atof(s1.c_str()), atof(s2.c_str()), atof(s3.c_str()),
			atof(s4.c_str()), atof(s5.c_str()), atof(s6.c_str()),
			atof(s7.c_str()), atof(s8.c_str()), atof(s9.c_str());
		stringLine.clear();

		// T_ir2rgb
		getline(parm, stringLine);
		stringstream lin4(stringLine);
		lin4 << stringLine;
		lin4 >> s1 >> s2 >> s3;
		kinectParm->T_ir2rgb << atof(s1.c_str()), atof(s2.c_str()), atof(s3.c_str());
	}
	else
	{
		cout << "parm.txt not right!!!";
		return false;
	}
	// ��ʾϵͳ����
	cout << "******************************************" << endl;

	cout << "fx_rgb:    " << kinectParm->fx_rgb << endl;
	cout << "fy_rgb:    " << kinectParm->fy_rgb << endl;
	cout << "cx_rgb:    " << kinectParm->cx_rgb << endl;
	cout << "cy_rgb:    " << kinectParm->cy_rgb << endl;
	cout << "******************************************" << endl;
	cout << "fx_ir:     " << kinectParm->fx_ir << endl;
	cout << "fy_ir:     " << kinectParm->fy_ir << endl;
	cout << "cx_ir:     " << kinectParm->cx_ir << endl;
	cout << "cy_ir:     " << kinectParm->cy_ir << endl;
	cout << "******************************************" << endl;
	cout << "R_ir2rgb:" << endl << kinectParm->R_ir2rgb << endl;
	cout << "******************************************" << endl;
	cout << "T_ir2rgb:" << endl << kinectParm->T_ir2rgb << endl;
	cout << "******************************************" << endl;
	return true;
}

int main()
{
	// 1. ��ȡ����
	KinectParm *parm = new KinectParm();
	if (!loadParm(parm))
		return 0;
	// 2. ����rgbͼƬ�����ͼ����ʾ
	Mat bgr(1080, 1920, CV_8UC4);
	bgr = imread("color.jpg");
	Mat depth(424, 512, CV_16UC1);
	depth = imread("depth.png", IMREAD_ANYDEPTH);   // jpgͼƬ�����ĸ�ʽ��һ���Ͷ���ʱ���һ����������������ĸ�ʽ����8UC3������ע��֮ǰ�ڱ���ͼƬʱҪ��ΪPNG
	//Mat depth2rgb = imread("depth2rgb.jpg");
	// 3. ��ʾ
	thread th = std::thread([&]{
		while (true)
		{
			imshow("ԭʼ��ɫͼ", bgr);
			waitKey(1);
			imshow("ԭʼ���ͼ", depth);
			waitKey(1);
			//imshow("ԭʼͶӰͼ", depth2rgb);
			//waitKey(1);
		}
	});
	// 4. �任

	// 4.1 �����������
#pragma region  ���
	// pinv(A) = (A^T*A)^(-1)*A^T  �õ��Ľ���д���

	//Eigen::MatrixXf P1(3,4);          // ir�ڲξ���
	//P1 <<
	//  parm->fx_ir, 0, parm->cx_ir, 0,
	//  0, parm->fy_ir, parm->cy_ir, 0,
	//  0, 0, 1, 0;
	//cout << "ir�ڲξ���:\n" << P1 << endl;
	//Eigen::MatrixXf P2(3, 4);         // rgb�ڲξ���
	//P2 <<
	//  parm->fx_rgb, 0, parm->cx_rgb, 0,
	//  0, parm->fy_rgb, parm->cy_rgb, 0,
	//  0, 0, 1, 0;
	//cout << "rgb�ڲξ���:\n" << P2 << endl;
	//Eigen::Matrix4f T;            // rgb�ڲξ���
	//T <<parm->R_ir2rgb, parm->T_ir2rgb, 0, 0, 0, 1;
	//cout << "����任����:\n" << T << endl;

	//Eigen::Matrix3f H;
	//Eigen::Matrix4f P1_inv;
	//P1_inv= (P1.transpose()*P1).inverse()*P1.transpose();
	//cout << "P2α�����:\n" << P1_inv << endl;
	//cout << "P2*P2_inv:\n" << P1_inv*P1 << endl;

	//H = P2*T*(P1);
	//cout << H << endl;
#pragma endregion
	// �����
	Eigen::Matrix3f K_ir;           // ir�ڲξ���
	K_ir <<
		parm->fx_ir, 0, parm->cx_ir,
		0, parm->fy_ir, parm->cy_ir,
		0, 0, 1;
	Eigen::Matrix3f K_rgb;          // rgb�ڲξ���
	K_rgb <<
		parm->fx_rgb, 0, parm->cx_rgb,
		0, parm->fy_rgb, parm->cy_rgb,
		0, 0, 1;

	Eigen::Matrix3f R;
	Eigen::Vector3f T;
	R = K_rgb*parm->R_ir2rgb*K_ir.inverse();
	T = K_rgb*parm->T_ir2rgb;

	cout << "K_rgb:\n" << K_rgb << endl;
	cout << "K_ir:\n" << K_ir << endl;
	cout << "R:\n" << R << endl;
	cout << "T:\n" << T << endl;

	cout << depth.type() << endl;


	// 4.2 ����ͶӰ
	Mat result(424, 512, CV_8UC3);
	int i = 0;
	for (int row = 0; row < 424; row++)
	{
		for (int col = 0; col < 512; col++)
		{
			unsigned short* p = (unsigned short*)depth.data;
			unsigned short depthValue = p[row * 512 + col];
			//cout << "depthValue       " << depthValue << endl;
			if (depthValue != -std::numeric_limits<unsigned short>::infinity() && depthValue != -std::numeric_limits<unsigned short>::infinity() && depthValue != 0 && depthValue != 65535)
			{
				// ͶӰ����ɫͼ�ϵ�����
				Eigen::Vector3f uv_depth(col, row, 1.0f);
				Eigen::Vector3f uv_color = depthValue / 1000.f*R*uv_depth + T / 1000;

				int X = static_cast<int>(uv_color[0] / uv_color[2]);
				int Y = static_cast<int>(uv_color[1] / uv_color[2]);
				//cout << "X:       " << X << "     Y:      " << Y << endl;
				if ((X >= 0 && X < 1920) && (Y >= 0 && Y < 1080))
				{
					//cout << "X:       " << X << "     Y:      " << Y << endl;
					result.data[i * 3] = bgr.data[3 * (Y * 1920 + X)];
					result.data[i * 3 + 1] = bgr.data[3 * (Y * 1920 + X) + 1];
					result.data[i * 3 + 2] = bgr.data[3 * (Y * 1920 + X) + 2];
				}
			}
			i++;
		}
	}
	thread th2 = std::thread([&]{
		while (true)
		{
			imshow("���ͼ", result);
			waitKey(1);
		}
	});

	th.join();
	th2.join();
	system("pause");
	return 0;
}