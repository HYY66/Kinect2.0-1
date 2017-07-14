#ifndef DRAW_H
#define DRAW_H

#include <iostream>
#include <opencv2\imgproc.hpp>	//opencvͷ�ļ�
#include <opencv2\calib3d.hpp>
#include <opencv2\highgui.hpp>
#include <Kinect.h>	//Kinectͷ�ļ�

void    draw(cv::Mat & img, Joint & r_1, Joint & r_2, ICoordinateMapper * myMapper);
void    drawhandstate(cv::Mat & img, Joint & lefthand, Joint & righthand, IBody* myBodyArr, ICoordinateMapper * myMapper);

#endif