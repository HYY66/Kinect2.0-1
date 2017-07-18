#ifndef GLOBAL_H
#define GLOBAL_H

#include <iostream>
#include <opencv2\imgproc.hpp>	//opencvͷ�ļ�
#include <opencv2\calib3d.hpp>
#include <opencv2\highgui.hpp>
#include <Kinect.h>	//Kinectͷ�ļ�

using   namespace   std;
using   namespace   cv;

void    draw(cv::Mat & img, Joint & r_1, Joint & r_2, ICoordinateMapper * myMapper);
void    drawhandstate(cv::Mat & img, Joint & lefthand, Joint & righthand, IBody* myBodyArr, ICoordinateMapper * myMapper);

#endif