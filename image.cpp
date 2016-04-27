/**********************************************************************************
*
*     The Seventeenth Tsinghua University Electronical Design Competition
*                   清华大学第十七届电子设计大赛（上位机程序）
*    
*     Author：温拓扑
*     Date：  2015.9.22
*     Copyright (c) 2015 温拓扑. All rights reserved.
*
*     Filename：   image.cpp
*
***********************************************************************************/

#include "image.h"

#ifdef DEBUG
    double start_timer()//用于计时
    {
        double start_time = (double)getTickCount();
        return start_time;
    }
    
    double end_timer(double start_time, int num_tests)
    {
        double time = (1000 * ((double) getTickCount() - start_time) / getTickFrequency());
        cout << "Average time of " << num_tests << " frames is: " << time / num_tests << " ms" <<endl;
        return time;
    }
#endif

Image::Image()
{
	warp_mat = cvCreateMat(2, 3, CV_32FC1);
	Thres_of_block = 80;  //二值化的阈值70
    Thres_of_area = 350;  //过滤的轮廓范围350

	//-----------------------------------------------------------------------
	/* 打开摄像头，台式机中0代表外部摄像头，笔记本电脑代表内置摄像头 */
    capture = cvCreateCameraCapture(0);
	//-----------------------------------------------------------------------

#ifdef DEBUG
    cvNamedWindow("video");
    while (1)
	{
        frame = cvQueryFrame(capture);
        if (!frame)
            break;
        cvShowImage("video", frame);
        if (cvWaitKey(20) == 'c')
		{
            cvSaveImage("testcam1.jpg", frame);
            break;
        }
    }
#endif
    //cvReleaseCapture(&capture);
    //cvReleaseImage(&frame);
    
    //保存一张用来初始化的图像
    
    src = cvLoadImage("testcam1.jpg",0);
    dst = cvCreateImage(cvGetSize(src), src->depth, src->nChannels);
#ifdef DEBUG
    cvShowImage("black", src);
    cvWaitKey(0);
#endif
    //摄像头矫正
    intrinsic = (CvMat*)cvLoad("cameraMatrix.xml");
    distortion = (CvMat*)cvLoad("distCoeffs.xml");
    mapx = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
    
    mapy = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
    
    //计算内参矩阵
    cvInitUndistortMap(intrinsic, distortion, mapx, mapy);
    //校正
    cvRemap(src, dst, mapx, mapy);
#ifdef DEBUG
    cvNamedWindow("jibian");
    cvShowImage("jibian", dst);
    cvWaitKey(0);
#endif
    
    int success = 0;
    double area = 100000000.0;
    CvMemStorage* storage = cvCreateMemStorage(0);
    CvMemStorage* storage1 = cvCreateMemStorage(0);//储存轮廓的内存
    CvSeq* contour = 0;
    CvSeq* polycontour = 0;
    CvSeq* tmp[3];
    CvBox2D Box;
    CvBox2D Boxtemp;
    
    cvThreshold(dst, dst, Thres_of_block, 255, CV_THRESH_BINARY);
    cvDilate(dst,dst);
    cvErode(dst, dst, NULL,3);
    cvDilate(dst, dst, NULL, 1);
#ifdef DEBUG
    cvNamedWindow("Find");
    cvShowImage("Find", dst);
    while (cvWaitKey(0) != 27)
	{
        break;
    }
#endif
    
    //把dst增加一层外边框
    IplImage* Addboarder = cvCreateImage(cvSize(dst->width+10, dst->height+10), dst->depth, dst->nChannels);
    for (int y = 0; y < Addboarder->height; y++){
        for (int x = 0; x < Addboarder->width; x++) {
            Addboarder->imageData[y*Addboarder->widthStep + x] = 255;
        }
    }
    cvSetImageROI(Addboarder, cvRect(5, 5, dst->width, dst->height));
    cvCopy(dst, Addboarder);
    cvResetImageROI(Addboarder);
#ifdef DEBUG
    cvNamedWindow("boarder");
    cvShowImage("boarder", Addboarder);
    cvWaitKey(0);
#endif
	IplImage* mask = cvCreateImage(cvSize(640,300),8,1);
	cvZero(mask);
	cvSetImageROI(Addboarder, cvRect(0,90,640,300));
	cvCopy(mask,Addboarder);
	cvResetImageROI(Addboarder);
	cvReleaseImage(&mask);
    cvFindContours(Addboarder, storage, &contour,sizeof(CvContour),CV_RETR_LIST,CV_CHAIN_APPROX_NONE,cvPoint(0, 0));
    polycontour = cvApproxPoly(contour,sizeof(CvContour), storage1, CV_POLY_APPROX_DP, cvContourPerimeter(contour)*0.02, 1);

    CvSeq* contourtemp = polycontour;
	int angle = 10;
    while (success < 3) 
	{
        for (;contourtemp != 0; contourtemp = contourtemp->h_next)
		{
#ifdef DEBUG
            printf("%f \n", cvContourArea(contourtemp));
#endif
            Box = cvMinAreaRect2(contourtemp);
            cout<<"Box height and width are "<<Box.size.height<<" "<<Box.size.width<<endl;
            if (cvContourArea(contourtemp)>Thres_of_area && cvContourArea(contourtemp) <= area && fabs(Box.size.height-Box.size.width)<5 )
			{
                int flag = 1;
                for (int j = 0; j < success+1; ++j) 
				{
                    if (contourtemp == tmp[j])
					{
#ifdef DEBUG
                        printf("this point is samllest so far,\nbut it is same as %dth point\n",j);
#endif
                        flag = 0;
                    }
#ifdef DEBUG
                    else
                        printf("diff\n");
#endif
                }
                if (flag == 1)
				{
                    area = cvContourArea(contourtemp);
                    tmp[success] = contourtemp;
                    Boxtemp = Box;
#ifdef DEBUG
                    cvDrawContours(Addboarder, tmp[success], CV_RGB(255,255,255), CV_RGB(255,255,255),0,1,8);
                    cvRectangle(Addboarder, cvPoint(Boxtemp.center.x-10, Boxtemp.center.y-10), cvPoint(Boxtemp.center.x+10,Boxtemp.center.y+10), cvScalar(100),2);
                    printf("this point is smallest so far,\n And it is chosen as %dth point temp\n",success);
#endif
                }
            }
        }
		
        contourtemp = polycontour;
		
        cout<<"Boxtemp.center.x,y is "<<Boxtemp.center.x<<","<<Boxtemp.center.y<<endl;
       if (Boxtemp.center.x < Addboarder->width/2 && Boxtemp.center.y < Addboarder->height/2) {
            cvRectangle(Addboarder, cvPoint(Boxtemp.center.x-10, Boxtemp.center.y-10), cvPoint(Boxtemp.center.x+10,Boxtemp.center.y+10), cvScalar(255),2);
            board[0] = cvPoint(Boxtemp.center.x-Boxtemp.size.width/2-3, Boxtemp.center.y-Boxtemp.size.height/2-3);
            angle = angle - 1;
        }
        if (Boxtemp.center.x > Addboarder->width/2 && Boxtemp.center.y < Addboarder->height/2) {
            board[1] = cvPoint(Boxtemp.center.x+Boxtemp.size.width/2+3, Boxtemp.center.y-Boxtemp.size.height/2-3);
            
            cvRectangle(Addboarder, cvPoint(Boxtemp.center.x-10, Boxtemp.center.y-10),cvPoint(Boxtemp.center.x+10, Boxtemp.center.y+10),cvScalar(255),2);
            angle = angle - 2;
        }
        if (Boxtemp.center.x > Addboarder->width/2 && Boxtemp.center.y > Addboarder->height/2) {
            board[2] = cvPoint(Boxtemp.center.x+Boxtemp.size.width/2+3, Boxtemp.center.y+Boxtemp.size.height/2+3);
            cvRectangle(Addboarder, cvPoint(Boxtemp.center.x-10, Boxtemp.center.y-10),cvPoint(Boxtemp.center.x+10, Boxtemp.center.y+10),cvScalar(255),2);
            angle = angle - 3;
        }
        if (Boxtemp.center.x < Addboarder->width/2 && Boxtemp.center.y > Addboarder->height/2) {

            board[3] = cvPoint(Boxtemp.center.x-Boxtemp.size.width/2-3, Boxtemp.center.y+Boxtemp.size.height/2+3);
            cvRectangle(Addboarder, cvPoint(Boxtemp.center.x-10, Boxtemp.center.y-10),cvPoint(Boxtemp.center.x+10, Boxtemp.center.y+10),cvScalar(255),2);
            angle = angle - 4;
        }
        success++;
        area = 1000000000.0;
#ifdef DEBUG
        printf("over\n");
		cvShowImage("Find",Addboarder);
#endif
    }
 switch (angle) {
        case 1:
            srcTri[0].x = board[2].x;
            srcTri[0].y = board[2].y;
            srcTri[1].x = board[3].x;
            srcTri[1].y = board[3].y;
            srcTri[2].x = board[1].x;
            srcTri[2].y = board[1].y;
            break;
        case 2:
            srcTri[0].x = board[3].x;
            srcTri[0].y = board[3].y;
            srcTri[1].x = board[0].x;
            srcTri[1].y = board[0].y;
            srcTri[2].x = board[2].x;
            srcTri[2].y = board[2].y;
            break;
        case 3:
            srcTri[0].x = board[0].x;
            srcTri[0].y = board[0].y;
            srcTri[1].x = board[1].x;
            srcTri[1].y = board[1].y;
            srcTri[2].x = board[3].x;
            srcTri[2].y = board[3].y;
            break;
        case 4:
            srcTri[0].x = board[1].x;
            srcTri[0].y = board[1].y;
            srcTri[1].x = board[2].x;
            srcTri[1].y = board[2].y;
            srcTri[2].x = board[0].x;
            srcTri[2].y = board[0].y;
            break;
        default:
            break;
    }//决定方向;
   
    dstTri[0].x = int(abs(src->height-src->width))/2;
    dstTri[0].y = 0;
    dstTri[1].x = src->width - dstTri[0].x;
    dstTri[1].y = dstTri[0].y;
    dstTri[2].x = dstTri[0].x;
    dstTri[2].y = src->height - 1;
   
    cvGetAffineTransform(srcTri, dstTri, warp_mat);
#ifdef DEBUG
    cvWarpAffine(dst, src, warp_mat);
    cvNamedWindow("RAW");
    cvNamedWindow("AFFINE");
    cvShowImage("RAW", src);
    cvShowImage("AFFINE", dst);
    while (cvWaitKey(0) != 27) 
	{
        break;
    }
    cvDestroyAllWindows();
#endif

    Num = 2;
    car = new player[Num];
    carsample = new Sample[Num];
    cvReleaseImage(&dst);
    dst = cvCreateImage(cvGetSize(src), src->depth, 3);
    show = cvCreateImage(cvSize(src->height,src->height), src->depth, 3);
	show255 = cvCreateImage(cvSize(255,255), src->depth, 3);
    cvReleaseImage(&src);
    src = cvCreateImage(cvGetSize(dst), dst->depth, 3);
}

Image::~Image()
{
    cvReleaseCapture(&capture);
    cvReleaseImage(&src);
    cvReleaseImage(&dst);
    cvReleaseImage(&mapx);
    cvReleaseImage(&mapy);
    cvReleaseMat(&warp_mat);
    cvReleaseMat(&intrinsic);
    cvReleaseMat(&distortion);
}

/*void Image::GetColor(){
    IplImage* smallshow = cvCreateImage(cvSize(255, 255), 8 , 3);
    //IplImage *smallshow = cvLoadImage("/Users/programath/Documents/HomeWork/testopencv/DerivedData/testopencv/Build/Products/Debug/2-90.jpg");
    //Set smallshow as 255*255 pixel
    CvRect *candidate = new CvRect[Num];
    candidate[0] = cvRect(20, 120, 17, 23);
    candidate[1] = cvRect(218, 120, 17, 23);
    //candidate[2] = cvRect();
    //candidate[3] = cvRect();
    //在show上画矩形框来指示选手该放的位置
    //cvRectangle(show, cvPoint(38, 226), cvPoint(60, 269),cvScalar(255,0,0),3);
    //cvRectangle(show, cvPoint(38, 226), cvPoint(60, 269),cvScalar(255,0,0),3);
    
    while ( cvWaitKey(20)!='c' ){
        frame = cvQueryFrame(capture);
        cvCopy(frame, src);//src seemed can be subtituded by frame
        IplImage* temp1 = cvCreateImage(cvGetSize(src), src->depth, show->nChannels);
        cvCopy(src, temp1);
        cvRemap(src, temp1, mapx, mapy);
        cvWarpAffine(temp1, dst, warp_mat);
        cvSetImageROI(dst, cvRect((src->width-src->height)/2, 0, src->height, src->height));
        cvCopy(dst,show);
        cvResetImageROI(dst);
        //show is used to display;
        //smallshow = cvCreateImage(cvSize(255, 255), show->depth, show->nChannels);
        cvResize(show, smallshow);
        cvRectangle(show, cvPoint(38, 226), cvPoint(70, 269),cvScalar(255,0,0),3);
        cvRectangle(show, cvPoint(410, 226), cvPoint(442, 269),cvScalar(255,0,0),3);
        cvShowImage("place", show);
		cvReleaseImage(&temp1);
    }
    
    for (int i = 0; i < Num; ++i){
        cvSetImageROI(smallshow, candidate[i]);
        carsample[i].templ = cvCreateImage(cvSize(candidate[i].width,candidate[i].height), smallshow->depth, smallshow->nChannels);
        cvCopy(smallshow, carsample[i].templ);
        cvResetImageROI(smallshow);
        //cvShowImage("car1", carsample[i].templ);
        //cvWaitKey(0);
        //cvShowImage("car2", carsample[1].templ);
        IplImage *h_src = cvCreateImage(cvGetSize(carsample[i].templ), smallshow->depth, 1);
        IplImage *s_src = cvCreateImage(cvGetSize(carsample[i].templ), smallshow->depth, 1);
        cvCvtColor(carsample[i].templ, carsample[i].templ, CV_BGR2HSV);
        cvSplit(carsample[i].templ,h_src,s_src,NULL,NULL);
        //cvShowImage("car1templ", carsample[i].templ);
        //cvWaitKey(0);
        IplImage *images[] = {h_src,s_src};
        {//计算二维直方图
            int dims = 2;
            int size[] = {4,3}; // 这个地方不要取的太大!
            //当取为size[] = {180,256}时E7200CPU会运行长达10几分钟的！
            float range_h[] = {0,180} //在用cvCvtColor转换时h已经归一化到180了
            ,range_s[] = {0,256};
            float *ranges[] = {range_h,range_s};
            carsample[i].hist_src = cvCreateHist(dims,size,CV_HIST_ARRAY,ranges);
            cvCalcHist(images,carsample[i].hist_src);
            cvNormalizeHist(carsample[i].hist_src,1);
        }
    }
}*/

/*void Image::GetInfo(){
    frame = cvQueryFrame(capture);
    cvCopy(frame, src);//src seemed can be subtituded by frame
    IplImage* temp1 = cvCreateImage(cvGetSize(src), src->depth, show->nChannels);
    cvCopy(src, temp1);
    cvRemap(src, temp1, mapx, mapy);
    cvWarpAffine(temp1, dst, warp_mat);
    cvSetImageROI(dst, cvRect((src->width-src->height)/2, 0, src->height, src->height));
    cvCopy(dst,show);
    cvResetImageROI(dst);
    //show is used to display;
    IplImage* smallshow = cvCreateImage(cvSize(255, 255), show->depth, show->nChannels);
    cvResize(show, smallshow);
    //IplImage *smallshow = cvLoadImage("/Users/programath/Documents/HomeWork/testopencv/DerivedData/testopencv/Build/Products/Debug/2-90.jpg");
    IplImage *h_dst = cvCreateImage(cvGetSize(smallshow), smallshow->depth, 1);
    IplImage *s_dst = cvCreateImage(cvGetSize(smallshow), smallshow->depth, 1);
    cvCvtColor(smallshow, smallshow, CV_BGR2HSV);
    cvSplit(smallshow,h_dst,s_dst,NULL,NULL);
    
    for (int i = 0; i < Num; ++i){
        IplImage *images[] = {h_dst,s_dst};

        CvSize patch_size = cvSize(carsample[i].templ->width,carsample[i].templ->height);

        IplImage *result = cvCreateImage(cvSize(h_dst->width - patch_size.width +1, h_dst->height - patch_size.height +1) ,IPL_DEPTH_32F,1);//块搜索时处理边缘是直接舍去，故result的大小比dst小path_size大小
        //        //32F类型，取值为0~1最亮为1，可直接显示
        //        //CV_COMP_CORREL相关度，1时最匹配，0时最不匹配
        cvCalcBackProjectPatch(images,result,patch_size,carsample[i].hist_src,CV_COMP_CORREL,1);
        //cvSmooth(result, result,CV_BLUR_NO_SCALE );
        //cvShowImage("result",result);
        //cvWaitKey(0);
        //
        //        //找出最大值位置，可得到此位置即为杯子所在位置
        //
        cvMinMaxLoc(result,NULL,NULL,NULL,&car[i].position,NULL);
        //        //加上边缘，得到在原始图像中的实际位置
        car[i].position.x += cvRound(patch_size.width/2);
        car[i].position.y += cvRound(patch_size.height/2);
        //        //在dst图像中用红色小圆点标出位置
        cvCircle(smallshow,cvPoint(int(car[i].position.x),int(car[i].position.y)),3,CV_RGB(255,0,0),-1);
        //        clock2 = clock();
        //        cout<<(clock2-clock1)/ 1000 <<endl;
        cvReleaseImage(&result);
    }
    cvShowImage("show",smallshow);
    //cvWaitKey(0);
    cvReleaseImage(&h_dst);
    cvReleaseImage(&s_dst);
    cvReleaseImage(&smallshow);
	cvReleaseImage(&temp1);
}*/

/*void Image::GetColor(){
    IplImage* smallshow = cvCreateImage(cvSize(255, 255), 8 , 3);
    //IplImage *smallshow = cvLoadImage("/Users/programath/Documents/HomeWork/testopencv/DerivedData/testopencv/Build/Products/Debug/2-90.jpg");
    //Set smallshow as 255*255 pixel
    CvRect *candidate = new CvRect [Num];
    candidate[0] = cvRect(20, 120, 17, 23);
    candidate[1] = cvRect(218, 120, 17, 23);
    //candidate[2] = cvRect();
    //candidate[3] = cvRect();
    
    while ( cvWaitKey(20)!='c' ){
        frame = cvQueryFrame(capture);
        cvCopy(frame, src);//src seemed can be subtituded by frame
        IplImage* temp1 = cvCreateImage(cvGetSize(src), src->depth, show->nChannels);
        cvCopy(src, temp1);
        cvRemap(src, temp1, mapx, mapy);
        cvWarpAffine(temp1, dst, warp_mat);
        cvSetImageROI(dst, cvRect((src->width-src->height)/2, 0, src->height, src->height));
        cvCopy(dst,show);
        cvResetImageROI(dst);
        //show is used to display;
        //smallshow = cvCreateImage(cvSize(255, 255), show->depth, show->nChannels);
        cvResize(show, smallshow);
        //在show上画矩形框来指示选手该放的位置
        cvRectangle(show, cvPoint(38, 226), cvPoint(70, 269),cvScalar(255,0,0),3);
        cvRectangle(show, cvPoint(44, 237), cvPoint(63, 256), cvScalar(255,0,0),3);
        cvRectangle(show, cvPoint(410, 226), cvPoint(442, 269),cvScalar(255,0,0),3);
        cvRectangle(show, cvPoint(416, 237), cvPoint(435, 256), cvScalar(255,0,0),3);
		cvReleaseImage(&temp1);
        cvShowImage("place", show);
		cvShowImage("smallshow",smallshow);
    }
    
    for (int i = 0; i < Num; ++i){
		cvRectangle(smallshow,cvPoint(candidate[i].x+(candidate[i].width - 10)/2, candidate[i].y+(candidate[i].height - 10)/2),cvPoint(candidate[i].x+(candidate[i].width - 10)/2+10, candidate[i].y+(candidate[i].height - 10)/2+10),cvScalar(255,0,0));
		cvShowImage("sm",smallshow);
		cvWaitKey(0);
        cvSetImageROI(smallshow, cvRect(candidate[i].x+(candidate[i].width - 10)/2, candidate[i].y+(candidate[i].height - 10)/2 ,10,10));
        carsample[i].templ = cvCreateImage(cvSize(10,10), smallshow->depth, smallshow->nChannels);
        cvCopy(smallshow, carsample[i].templ);
        cvResetImageROI(smallshow);
        //cvShowImage("car1", carsample[i].templ);
        //cvWaitKey(0);
        //cvShowImage("car2", carsample[i].templ);
    }
}*/

/*void Image::GetInfo(){
    frame = cvQueryFrame(capture);
    cvCopy(frame, src);//src seemed can be subtituded by frame
    IplImage* temp1 = cvCreateImage(cvGetSize(src), src->depth, show->nChannels);
    cvCopy(src, temp1);
    cvRemap(src, temp1, mapx, mapy);
    cvWarpAffine(temp1, dst, warp_mat);
    cvSetImageROI(dst, cvRect((src->width-src->height)/2, 0, src->height, src->height));
    cvCopy(dst,show);
    cvResetImageROI(dst);
    //show is used to display;
    IplImage* smallshow = cvCreateImage(cvSize(255, 255), show->depth, show->nChannels);
    cvResize(show, smallshow);
    
    for (int i = 0; i < Num; ++i){
        
        CvSize patch_size = cvSize(carsample[i].templ->width,carsample[i].templ->height);
        
        IplImage *result = cvCreateImage(cvSize(smallshow->width - patch_size.width +1, smallshow->height - patch_size.height +1) ,IPL_DEPTH_32F,1);//块搜索时处理边缘是直接舍去，故result的大小比dst小path_size大小
        //        //32F类型，取值为0~1最亮为1，可直接显示
        //        //CV_COMP_CORREL相关度，1时最匹配，0时最不匹配
        cvMatchTemplate(smallshow, carsample[i].templ, result, CV_TM_SQDIFF_NORMED);
        //cvPow(result, result, 5);
		cvNormalize(result,result,1,0,CV_MINMAX);
        //cvSmooth(result, result,CV_BLUR_NO_SCALE );
        //cvShowImage("result",result);
        //cvWaitKey(0);
        //
        //        //找出最大值位置，可得到此位置即为杯子所在位置
        //
        cvMinMaxLoc(result,NULL,NULL,&car[i].position,NULL,NULL);
        //        //加上边缘，得到在原始图像中的实际位置
        car[i].position.x += cvRound(patch_size.width/2);
        car[i].position.y += cvRound(patch_size.height/2);
        //        //在dst图像中用红色小圆点标出位置
        cvCircle(smallshow,cvPoint(int(car[i].position.x),int(car[i].position.y)),3,CV_RGB(255,0,0),-1);
        //        clock2 = clock();
        //        cout<<(clock2-clock1)/ 1000 <<endl;
        cvReleaseImage(&result);
    }
    cvShowImage("show",smallshow);
    //cvWaitKey(20);
    //cvReleaseImage(&h_dst);
    //cvReleaseImage(&s_dst);
	cvReleaseImage(&temp1);
    cvReleaseImage(&smallshow);
}*/

void Image::GetColor(){
	int candidate1PositionX, candidate1PositionY, 
		candidate2PositionX, candidate2PositionY, 
		Rectwidth,Rectheight;
	ifstream bbFile("bbFile.txt", ios::in);
	bbFile >> candidate1PositionX;
	bbFile >> candidate1PositionY;
	bbFile >> candidate2PositionX, 
	bbFile >> candidate2PositionY;
	bbFile >> Rectwidth;
	bbFile >> Rectheight;
	bbFile.close();
    IplImage* smallshow = cvCreateImage(cvSize(255, 255), 8 , 3);
    //Set smallshow as 255*255 pixel
    CvRect *candidate = new CvRect [Num];
	candidate[0] = cvRect(candidate1PositionX,  candidate1PositionY, Rectwidth, Rectheight);
    //candidate[0] = cvRect(20, 120, 17, 23);
	candidate[1] = cvRect(candidate2PositionX,  candidate2PositionY, Rectwidth, Rectheight);
    //candidate[1] = cvRect(218, 120, 17, 23);
    //candidate[2] = cvRect();
    //candidate[3] = cvRect();
    cvNamedWindow("place");
    while ( cvWaitKey(20)!='c' ){
        frame = cvQueryFrame(capture);
        cvCopy(frame, src);//src seemed can be subtituded by frame
        IplImage* temp1 = cvCreateImage(cvGetSize(src), src->depth, show->nChannels);
        //cvCopy(src, temp1);
		//cvShowImage("temp1", temp1);
		//cvWaitKey(0);
        cvRemap(src, temp1, mapx, mapy);
		//cvShowImage("temp1", temp1);
		//cvWaitKey(0);
        cvWarpAffine(temp1, dst, warp_mat);
		//cvShowImage("temp1",dst);
		//cvWaitKey(0);
        cvSetImageROI(dst, cvRect((src->width-src->height)/2, 0, src->height, src->height));
        cvCopy(dst,show);
        cvResetImageROI(dst);
        //show is used to display;
        //smallshow = cvCreateImage(cvSize(255, 255), show->depth, show->nChannels);
        cvResize(show, smallshow);
        //在show上画矩形框来指示选手该放的位置
        cvRectangle(show, cvPoint(int(candidate1PositionX * 480 / 255) , 
								  int(candidate1PositionY * 480 / 255)),
						  cvPoint(int((candidate1PositionX + Rectwidth) * 480 / 255), 
							      int((candidate1PositionY + Rectheight) * 480 / 255)),
						  cvScalar(0,255,0),1);
		cvRectangle(show, cvPoint(int(candidate2PositionX * 480 / 255) , 
								  int(candidate2PositionY * 480 / 255)),
						  cvPoint(int((candidate2PositionX + Rectwidth) * 480 / 255), 
							      int((candidate2PositionY + Rectheight) * 480 / 255)),
						  cvScalar(0,255,0),1);
		cvRectangle(show, cvPoint(int((candidate1PositionX + 1.0*(Rectwidth  - 10)/2) * 480 / 255) , 
								  int((candidate1PositionY + 1.0*(Rectheight - 10)/2) * 480 / 255)),
						  cvPoint(int((candidate1PositionX + 1.0*(Rectwidth  + 10)/2) * 480 / 255), 
							      int((candidate1PositionY + 1.0*(Rectheight + 10)/2) * 480 / 255)),
						  cvScalar(0,255,0),1);
		cvRectangle(show, cvPoint(int((candidate2PositionX + 1.0*(Rectwidth  - 10)/2) * 480 / 255) , 
								  int((candidate2PositionY + 1.0*(Rectheight - 10)/2) * 480 / 255)),
						  cvPoint(int((candidate2PositionX + 1.0*(Rectwidth  + 10)/2) * 480 / 255), 
							      int((candidate2PositionY + 1.0*(Rectheight + 10)/2) * 480 / 255)),
						  cvScalar(0,255,0),1);

        //cvRectangle(show, cvPoint(44, 237), cvPoint(63, 256), cvScalar(255,0,0),3);
        //cvRectangle(show, cvPoint(410, 226), cvPoint(442, 269),cvScalar(255,0,0),3);
        //cvRectangle(show, cvPoint(416, 237), cvPoint(435, 256), cvScalar(255,0,0),3);
        cvReleaseImage(&temp1);
        cvShowImage("place", show);
        //cvShowImage("smallshow",smallshow);
    }
    cvDestroyWindow("place");
    for (int i = 0; i < Num; ++i){
        //cvRectangle(smallshow,cvPoint(candidate[i].x+(candidate[i].width - 10)/2, candidate[i].y+(candidate[i].height - 10)/2),cvPoint(candidate[i].x+(candidate[i].width - 10)/2+10, candidate[i].y+(candidate[i].height - 10)/2+10),cvScalar(255,0,0));
        //cvShowImage("sm",smallshow);
        //cvWaitKey(0);
        cvSetImageROI(smallshow, cvRect(candidate[i].x+(candidate[i].width - 10)/2, candidate[i].y+(candidate[i].height - 10)/2 ,10,10));
        carsample[i].templ = cvCreateImage(cvSize(10,10), smallshow->depth, smallshow->nChannels);
        cvCopy(smallshow, carsample[i].templ);
		cvCvtColor(carsample[i].templ, carsample[i].templ, CV_BGR2HSV);
#ifdef DEBUG
        cvShowImage("templ",carsample[i].templ);
        cvWaitKey(0);
#endif
        //cvSmooth(carsample[i].templ, carsample[i].templ,CV_BLUR);
        cvResetImageROI(smallshow);
        
        int H = 0,S = 0,V = 0;
        for (int y = 0; y < carsample[i].templ->width; ++y)
            for (int x = 0; x < carsample[i].templ->height; ++x) {
                V = (V)?( (V+(uchar)carsample[i].templ->imageData[ y * carsample[i].templ->widthStep + x * 3 + 2]) /2):(uchar)carsample[i].templ->imageData[ y * carsample[i].templ->widthStep + x * 3 + 2];
				S = (S)?( (S+(uchar)carsample[i].templ->imageData[ y * carsample[i].templ->widthStep + x * 3 + 1]) /2):(uchar)carsample[i].templ->imageData[ y * carsample[i].templ->widthStep + x * 3 + 1];
                H = (H)?( (H+(uchar)carsample[i].templ->imageData[ y * carsample[i].templ->widthStep + x * 3 + 0]) /2):(uchar)carsample[i].templ->imageData[ y * carsample[i].templ->widthStep + x * 3 + 0];
            }
        carsample[i].color[0] = H;// / (carsample[i].templ->width * carsample[i].templ->height);
        carsample[i].color[1] = S;// / (carsample[i].templ->width * carsample[i].templ->height);
        carsample[i].color[2] = V;// / (carsample[i].templ->width * carsample[i].templ->height);
        //cvShowImage("car1", carsample[i].templ);
        //cvWaitKey(0);
        //cvShowImage("car2", carsample[i].templ);
		cvCvtColor(carsample[i].templ, carsample[i].templ, CV_HSV2BGR);
    }
	
	cvReleaseImage(&smallshow);
	delete [] candidate;
}

void Image::GetInfo(){
    frame = cvQueryFrame(capture);
    cvCopy(frame, src);//src seemed can be subtituded by frame
    IplImage* temp1 = cvCreateImage(cvGetSize(src), src->depth, show->nChannels);
    cvCopy(src, temp1);
    cvRemap(src, temp1, mapx, mapy);
    cvWarpAffine(temp1, dst, warp_mat);
    cvSetImageROI(dst, cvRect((src->width-src->height)/2, 0, src->height, src->height));
    cvCopy(dst,show);
	cvResize(show, show255);
	cvCvtColor(show255, show255, CV_BGR2HSV);
    cvResetImageROI(dst);
    //show is used to display;
   // uchar B, G, R;
	uchar H, S, V;
        
#ifdef DEBUG
        cvShowImage("temp", show);
#endif
    
	IplImage *player[2];
	player[0] = cvCreateImage(cvSize(255,255), 8 , 1);
	player[1] = cvCreateImage(cvSize(255,255), 8 , 1);
	cvZero(player[0]);
	cvZero(player[1]);

    CvPoint *filter = new CvPoint [Num];
    for (int i = 0; i < Num; ++i){
        filter[i] = car[i].position;
        car[i].position = cvPoint(0,0);
        car[i].count = 1;
    }  //选手坐标初始化
    
    for (int y = 0; y < show255->height; y++){
        for (int x = 0; x < show255->width; x++){
            H = (uchar)show255->imageData[(y * show255->widthStep) + x * 3 + 0];
            S = (uchar)show255->imageData[(y * show255->widthStep) + x * 3 + 1];
            V = (uchar)show255->imageData[(y * show255->widthStep) + x * 3 + 2];
            
            for (int i = 0; i < Num; ++i){
                if (carsample[i].Distance(H,S,V)){
                    //car[i].position.x += x;
                    //car[i].position.y += y;
                    //car[i].count++;
                    player[i]->imageData[(y * player[i]->widthStep) + x] = 255;
                    //break;
                }
            }
        }
    }
	for (int i = 0; i < Num; ++i){
		cvErode(player[i],player[i],NULL,2);
		cvDilate( player[i], player[i],NULL,2);
		for (int y = 0; y < show255->height; y++)
			for (int x = 0; x < show255->width; x++)
				if ((uchar)player[i]->imageData[(y * player[i]->widthStep) + x] > 0){
                    car[i].position.x += x;
                    car[i].position.y += y;
                    car[i].count++;
                    //break;
				}

	}
	
    for (int i = 0; i < Num; ++i) {
        car[i].position.x /= int(car[i].count);//计算平均值
        car[i].position.y /= int(car[i].count);
        if (abs(car[i].position.x - filter[i].x) + abs(car[i].position.y-filter[i].y) < 4){
            car[i].position = filter[i];
        }
    }
    //#ifdef DEBUG
    for (int i = 0; i < Num; ++i)
    cvRectangle(show,
                cvPoint(int(1.88*car[i].position.x-5),
                        int(1.88*car[i].position.y-5)),
                cvPoint(int(1.88*car[i].position.x+5),
                        int(1.88*car[i].position.y+5)),
                cvScalar(carsample[i].color[0],carsample[i].color[1],carsample[i].color[2]));
	//cvCircle(show,cvPoint(205,194),2,cvScalar(255,0,0));这是当年一个傻逼造成的悲剧
    //cvNamedWindow("avi");
#ifdef DEBUG
    cvShowImage("avi", show);
	cvShowImage("player1", player[0]);
	cvShowImage("player2", player[1]);
#endif
	delete [] filter;
    cvReleaseImage(&temp1);
	for (int i = 0; i < Num; ++i){
		cvReleaseImage(&player[i]);
	}
	
}

