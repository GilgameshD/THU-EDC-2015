/************************************************************************************
*
*     The Seventeenth Tsinghua University Electronical Design Competition
*                           清华大学第十七届电子设计大赛（上位机程序）
*    
*     Author：温拓朴
*     Date：  2015.9.22
*     Copyright (c) 2015 温拓朴. All rights reserved.
*
*     Filename：   image.h
*     Discription：This file contains the functions of image group,they can return 
*                  the coordation of players and the image of the camera.
*                  
***********************************************************************************/

#ifndef __testopencv__image__
#define __testopencv__image__

//#define DEBUG
#include <cxcore.h>
#include <iostream>
#include <fstream>
#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctime>

using namespace std;
using namespace cv;

struct player
{
    CvPoint position;
    int count;
};

struct Sample
{
    IplImage *templ;
    CvHistogram *hist_src;
    uchar color[3];
    bool Distance(uchar H, uchar S, uchar V)
	{
        return (sqrt((H - color[0])*(H - color[0])+(S - color[1])*( S - color[1])+(V - color[2])*(V - color[2])) < 60 );
    }
   // IplImage *h_src;
   // IplImage *s_src;
};

class Image
{
private:
    CvPoint board[4];
    CvPoint2D32f srcTri[3];
    CvPoint2D32f dstTri[3];
    CvMat *intrinsic;      //用于摄像头校正的矩阵
    CvMat *distortion;
    IplImage *mapx;
    IplImage *mapy;
    CvMat *warp_mat;
    CvCapture *capture;
    //需要现场调试的参数
	//bool (*p1)(uchar,uchar,uchar);
	//bool (*p2)(uchar,uchar,uchar);//指定的两个颜色函数指针
    int Thres_of_block;//二值化的阈值
    int Thres_of_area;//过滤的轮廓范围
    
public:
    player* car;
    int Num;//选手的参赛选手的个数
    IplImage *frame;
    IplImage *src;//原图像
    IplImage *dst;
    IplImage *show;
	IplImage *show255;
    Sample *carsample;
    Image();     //包含棋盘图像的获取(棋盘图像只需要读取前期的数值即可)，矫正之后进行三对点的识别，计算出warp_mat
    ~Image();
    void GetInfo();
	void GetColor();
};

//     参数有：红色的阈值 ，蓝色的阈值，绿色的阈值
//     二值化的阈值
//     腐蚀、膨胀、腐蚀分别的次数

#endif // defined(__testopencv__image__) 
