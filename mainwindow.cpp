/**********************************************************************************
*
*     The Seventeenth Tsinghua University Electronical Design Competition
*                   清华大学第十七届电子设计大赛（上位机程序）
*    
*     Author：DingWenhao
*     Date：  2015.9.22
*     Copyright (c) 2015 DingWenhao. All rights reserved.
*
*     Filename：   mainwindow.cpp
*
***********************************************************************************/

#include "communication.h"
#include "sound.h"
#include "image.h"
#include "mainwindow.h"
#include "overdialog.h"
#include <QMouseEvent>
#include <QPainter>
#include <QBitmap>
#include <QString>
#include <QtWidgets/QMainWindow>
#include <fstream>
#include <math.h>
#include <QLabel>
#include <string>

using namespace std;

#define ONE_SECOND              1000
#define CAMERA_DELAY            20      //摄像头频率50Hz（每20毫秒一次刷新）
#define DATASEND_DELAY          100     //每100毫秒进行一次结算
#define PUNISH_TIME             5       //罚时5秒
#define NUMBER_OF_TURNS         3       //规定完成圈数
#define PRODUCE_PROPRTY         10000   //道具随机产生的时间(ms)
#define WIGHT_OF_PITFALL        5       //陷阱宽度
#define WIGHT_OF_OUTSIDE_IMAGE  20      //出界标记的宽度
#define PROTECTED_TIME          100     //保护道具的有效时间(*100 ms)
#define SIZE_OF_OUTIMAGE        30      //定义出界图片边长
#define NUMBER_OF_FALSESTART    2       //抢跑次数上限 - 1
#define TIME_OF_PROPERTY        50      //道具作用时间

/* 死区的长度和宽度 */
#define JUDGE_PIT_WEIGHT        30 
#define JUDGE_PIT_HEIGHT        15

/* 死区的中心坐标 */
#define DEAD1X    197     
#define DEAD1Y    51
#define DEAD2X    194             
#define DEAD2Y    207
#define DEAD3X    58              
#define DEAD3Y    51
#define DEAD4X    58              
#define DEAD4Y    207

/* 进入死区的时间上限 */
#define IN_PIT_TIME_LIMIT 100

/* 图像坐标的差值 */
#define IMAGELABEL_LENGTH_WIDTH  0     //显示宽度位移
#define IMAGELABEL_LENGTH_HEIGHT 0     //显示高度位移

#ifdef DEBUG
#pragma message(">----------------------------------- Image Windows Test! ---------------------------------<")
#endif 

/* 主窗口类的构造函数 */
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
	setupUi(this);

    QPalette palette(this->palette());
	palette.setBrush(this->backgroundRole(),QBrush(Qt::black));
    this->setPalette(palette);
    this->setAutoFillBackground(true);
	this->setWindowFlags(Qt::FramelessWindowHint);    /* 设置边框隐藏 */

   /* setAttribute(Qt::WA_TranslucentBackground);*/

	/* 圆角窗口 */
	QBitmap objBitmap(size());
	QPainter painter(&objBitmap);
	painter.fillRect(rect(),Qt::white);
	painter.setBrush(QColor(0,0,0));
	painter.drawRoundedRect(this->rect(),20,20);
	this->setMask(objBitmap);

	/* 圆角摄像头区域 */
	QBitmap objBitmapImage(size());
	QPainter painter1(&objBitmapImage);
	painter1.fillRect(rect(),Qt::white);
	painter1.setBrush(QColor(0,0,0));
	painter1.drawRoundedRect(imageLable->rect(),10,10);
	imageLable->setMask(objBitmapImage);

	/* 道具名称初始化 */
	propertyName[0] = QStringLiteral("盾牌");
	propertyName[1] = QStringLiteral("折光");
	propertyName[2] = QStringLiteral("净化");
	propertyName[3] = QStringLiteral("炸弹");
	propertyName[4] = QStringLiteral("眩晕");
	propertyName[5] = QStringLiteral("保龄");
	propertyName[6] = QStringLiteral("磁铁");
	propertyName[7] = QStringLiteral("沼泽");
	propertyName[8] = QStringLiteral("地雷");
	propertyName[9] = QStringLiteral("漩涡");

	/* 计时器初始化 */
    cameraTimer = new QTimer(this);
    beforeTimer = new QTimer(this);
    gameTimer = new QTimer(this);
	propertyTimer = new QTimer(this);
	punishTimer1 = new QTimer(this);
	punishTimer2 = new QTimer(this);
	sendtimer = new QTimer(this);

#ifdef FOUR_PLAYER
	punishTimer3 = new QTimer(this);
	punishTimer4 = new QTimer(this);
#endif

	gameNumber = 0;  //比赛时间计时
	beforeTime = 0;  //开始前5秒倒计时

	/* 按钮初始化 */
    endButton->setEnabled(false);
	pauseButton->setEnabled(false);

	/* 宏 QStringLiteral 用于解决Qt5中文乱码问题 */
	troopBProperty1->setText((QStringLiteral("没有道具")));
	troopAProperty1->setText((QStringLiteral("没有道具")));
	troopBProperty2->setText((QStringLiteral("没有道具")));
	troopAProperty2->setText((QStringLiteral("没有道具")));
	troopAPosition->setText((QStringLiteral("位于跑道内")));
	troopBPosition->setText((QStringLiteral("位于跑道内")));
	troopAState->setText((QStringLiteral("无警告")));
	troopBState->setText((QStringLiteral("无警告")));

	/* 比赛双方数据的初始化 */
	player = new PlayerInfo[PLAYER_NUMBER];
	curPosition = new int[PLAYER_NUMBER];   
	nextPosition = new int[PLAYER_NUMBER];   
	numberOfFalseStart = new int[PLAYER_NUMBER];

	outImage1->hide();    //出界标记1隐藏
	outImage2->hide();    //出界标记2隐藏
	outImage3->hide();    //出界标记3隐藏
	outImage4->hide();    //出界标记4隐藏

	pit1->hide();         //陷阱标记1隐藏
	pit2->hide();         //陷阱标记2隐藏
	pit3->hide();         //陷阱标记3隐藏
	pit4->hide();         //陷阱标记4隐藏

	propertyLabel1->hide(); //道具1图标隐藏
	propertyLabel2->hide(); //道具2图标隐藏
	propertyLabel3->hide(); //道具3图标隐藏
	propertyLabel4->hide(); //道具4图标隐藏

	gameMessageBrowser->append(QStringLiteral("--------------------------------   就绪   ------------------------------"));

	///////////////////////////////// IMAGE GROUP ////////////////////////////////////////
	camera = new Image;                    //摄像头类的构造
	imageLable->setScaledContents(true);   //显示图像大小自动调整为ImageLabel大小
	connect(cameraTimer,SIGNAL(timeout()),this,SLOT(VideoUpdate()));
	//////////////////////////////////////////////////////////////////////////////////////

	/* 创建音乐窗口 */
	gameMusic = new Sound;
	beforeGameMovie = new QMovie("image/321.gif");
	gameState = PREPARE;  //初始化时比赛开始标志为准备

	/* 槽和信号的关联 */
	connect(startButton, SIGNAL(clicked()), this, SLOT(onStartButton()));
	connect(endButton, SIGNAL(clicked()), this, SLOT(onEndButton()));
	connect(pauseButton, SIGNAL(clicked()), this, SLOT(onPauseButton()));
	connect(gameTimer, SIGNAL(timeout()), this, SLOT(onGameTimerOut())); 
	connect(beforeTimer, SIGNAL(timeout()), this, SLOT(onBeforeTimer())); 
	connect(punishTimer1, SIGNAL(timeout()), this, SLOT(onOutsideTimer()));
	connect(punishTimer2, SIGNAL(timeout()), this, SLOT(onOutsideTimer()));
	connect(sendtimer, SIGNAL(timeout()), this, SLOT(onSendTimer()));

#ifdef FOUR_PLAYER
	connect(punishTimer3, SIGNAL(timeout()), this, SLOT(onOutsideTimer()));
	connect(punishTimer4, SIGNAL(timeout()), this, SLOT(onOutsideTimer()));
#endif

	connect(propertyTimer, SIGNAL(timeout()), this, SLOT(onPropertyTimer()));
	connect(musicButton, SIGNAL(clicked()), this, SLOT(setMusic()));
	connect(cornerCloseButton, SIGNAL(clicked()), this, SLOT(allClose()));
	connect(minisizeButton, SIGNAL(clicked()), this, SLOT(showMinimized()));

	/* 串口初始化 */
	/* 由于需要检验CPLD是否被修改，所以串口必须在比赛开始前发送检验数据 */
	communication = new Communication;
	communication->OpenPort();          //打开串口

	sendtimer->start(DATASEND_DELAY);   //开启串口操作计时器

	/* 初始化函数集合 */
	for(int i = 0;i < PLAYER_NUMBER;++i)
	{
		numberOfFalseStart[i] = 0;   /* 判断抢跑数组初始化 */
		curPosition[i] = 0;          /* 判断逆行数组初始化 */
		nextPosition[i] = 0;
		player[i].Head_x = 0;        /* 坐标初始化 */
		player[i].Head_y = 0;
		troopTime[i] = 0;            /* 储存圈数数组初始化 */
		troopTurn[i] = 0;
		for(int j = 0;j < 3;++j)
			oneTurn[i][j] = false;

		player[i].myProperty[0].kindOfProperty = NO_PROPERTY; /* 选手道具初始化 */
		player[i].myProperty[1].kindOfProperty = NO_PROPERTY;

		communication->getProperty[i] = false;               /* 捡道具标志初始化 */
		for(int j = 0;j < 8; ++j)                            /* 陷阱道具影响延时 */
			propertyInfluenceTime[i][j] = 0;
		for(int j = 0;j < 4;++j)
			communication->communicatePitfallMatrix[i][j] = -1;/* 陷阱坐标初始化 */

		player[i].state = NORMAL;                             /* 选手状态初始化 */
		player[i].propertyState = NO_ATTACT;
		playerInPitTime[i] = 0;
	}

	/* 图像矩阵读取 */
	ifstream mapFile("map.txt", ios::in);
	for(int i = 0;i < 255;++i)
    { 
		char ch;
		for( int j = 0;j < 255;++j)
		{  
			mapFile >> imageMatrix[j][i][0];
	        mapFile.get(ch);
	    }	    
    }
	mapFile.close();

	/* 判断正反向的坐标读取 */
	ifstream reverseFile("reverseFile.txt",ios::in);
    for(int i = 0;i < 37;++i)
    {
		char ch;
		for(int j = 0;j < 3;++j)
			reverseFile >> reversePosition[i][j];
		reverseFile.get(ch);
	}
	reverseFile.close();

	/* 道具位置坐标读取 */
	allProperty = new Property[NUMBER_OF_PROPERTY];
	ifstream propertyFile("propertyAndMidFile.txt",ios::in);
    for(int i = 0;i < NUMBER_OF_PROPERTY;++i)  
    {
		char ch;
		for(int j = 0;j < 2;++j)
			propertyFile >> propertyPosition[i][j];	
		propertyFile.get(ch);
	}
	for(int i = 0;i < 8;++i)
	{
		propertyFile >> PLAYER1_START_X;
        propertyFile >> PLAYER1_START_Y;
        propertyFile >> PLAYER1_MID_X;
        propertyFile >> PLAYER1_MID_Y;

		propertyFile >> PLAYER2_START_X;
		propertyFile >> PLAYER2_START_Y;
        propertyFile >> PLAYER2_MID_X;
        propertyFile >> PLAYER2_MID_Y;
	}

	propertyFile.close();
	int k = 0;
	for(int i = 0;i < 4;++i)
	{
		for(int j = 0;j < 2;++j)
		{
			transformPropertyPosition[k] = propertyPosition[i][j];
			k++;
		}
	}

	/*  道具坐标赋值给道具 */
	for(int i = 0;i < NUMBER_OF_PROPERTY; ++i)
	{
		allProperty[i].positionX = propertyPosition[i][0];
		allProperty[i].positionY = propertyPosition[i][1];
	}

	/* 图像矩阵保存 */
	Mat image(255,255,CV_8UC1);
	for(int i = 0;i < image.rows;++i)
		for(int j = 0;j < image.cols;++j)
		{
			/*uchar temp = *(imageMatrix1 + i*image.rows + j);*/
			uchar temp = imageMatrix[j][i][0];
			uchar input;
			if(temp == 0)
				input = 255;
			else
				input = 0;
			image.at<uchar>(i,j) = input;
		}
		imwrite("123.jpg", image);

	/* 读入矩阵图片 */
	IplImage *temp = cvLoadImage("123.jpg");
	IplImage *temp2 = cvCreateImage(cvSize(600,600), 8, 3);
	IplImage *temp2_R = cvCreateImage(cvSize(600,600), 8, 1);
	IplImage *temp2_G = cvCreateImage(cvSize(600,600), 8, 1);
	IplImage *temp2_B = cvCreateImage(cvSize(600,600), 8, 1);
	IplImage *alpha = cvCreateImage(cvSize(600,600), 8, 1);
	cvResize(temp,temp2);
	cvSplit(temp2, temp2_B, temp2_G, temp2_R, NULL);
	cvSet(alpha, cvScalar(128), NULL);
	IplImage *BGRA = cvCreateImage(cvSize(600,600), 8, 4);
	cvMerge(temp2_B, temp2_G, temp2_R, alpha, BGRA);
	//cvCvtColor(temp2, alpha, CV_RGB2BGRA);
	
	/* 掩膜图像的转化 */
	testMask = new QImage;
	*testMask = QImage((const uchar*)BGRA->imageData, BGRA->width, BGRA->height, QImage::Format_RGBA8888);

	QPixmap pix = QPixmap("image/pause.jpg");  //播放比赛开始图片
    imageLable->setPixmap(pix);

	/* 选手1起始位置初始化 */
	playerStartPosition[0][0] = PLAYER1_START_X;
	playerStartPosition[0][1] = PLAYER1_START_Y;
	playerMidPosition[0][0] = PLAYER1_MID_X;
	playerMidPosition[0][1] = PLAYER1_MID_X;
	/* 选手2起始位置初始化 */
	playerStartPosition[1][0] = PLAYER2_START_X;
	playerStartPosition[1][1] = PLAYER2_START_Y;
	playerMidPosition[1][0] = PLAYER2_MID_X;
	playerMidPosition[1][1] = PLAYER2_MID_Y;

	sampleCarProperty = 0;
	sampleCarPro = new Property;
	imagePauseLable->hide();
	reverseTime = 0;
}

/* 析构函数 */
MainWindow::~MainWindow()
{
	communication->stop();
	/* 清除堆中的动态内存 */
	delete camera;   
	delete gameMusic;   
	delete beforeGameMovie;
	cvReleaseImage(&frame);
	delete communication;
	delete [] player;
	delete [] allProperty;  
}

/* 鼠标按下的事件函数 */
void MainWindow::mousePressEvent(QMouseEvent *event)  
{  
    if (event->button() == Qt::LeftButton) 
	{  
        m_Drag = true;  
        m_DragPosition = event->globalPos() - this->pos();  
        event->accept();  
    }  
}  
  
/* 鼠标拖动的事件函数 */
void MainWindow::mouseMoveEvent(QMouseEvent *event)  
{  
    if (m_Drag && (event->buttons() && Qt::LeftButton)) 
	{  
        move(event->globalPos() - m_DragPosition);  
        event->accept();  
    }  
}  
  
/* 鼠标释放的事件函数 */
void MainWindow::mouseReleaseEvent(QMouseEvent *)  
{  
    m_Drag = false;  
} 

/* 创建结束对话框 */
void MainWindow::creatOverDialog(QString str, int timeA, int timeB)
{
	/*gameState = GAMEFINISH;*/
	OverDialog *overDialog = new OverDialog(this);
	overDialog->setWindowModality(Qt::ApplicationModal);    /* 模态对话框 */
	//QString temp;
	//if(timeA == -1)
	//{
	//	temp = QStringLiteral("FalseStart:");
	//	timeA = 0;
	//}
	//else if(timeA == -2 )
	//{
	//	temp = QStringLiteral("CPLD Changed:");
	//	timeA = 0;
	//}
	//else
	//	temp = QStringLiteral("WINNER:");

#ifdef FOUR_PLAYER
	overDialog->setMessage(this->returnWinnerName(), troopTime[0], troopTime[1], troopTime[2], troopTime[3]); 
#else
	overDialog->setMessage("WINNER:", str, timeA, timeB);   /* 向结束窗口传入的数据 */
#endif

	overDialog->exec();
	QPixmap pix = QPixmap("image/pause.jpg");  //显示比赛结束图片
    imageLable->setPixmap(pix);
	gameMessageBrowser->append(QStringLiteral("-------------------------------   就绪   ------------------------------"));
}

/* 开始按钮触发的槽函数 */
void MainWindow::onStartButton()
{
	startButton->setEnabled(false);
	troopAComboBox->setEnabled(false);
	troopBComboBox->setEnabled(false);

	if(gameState == PREPARE)
	{
		camera->GetColor();             //图像组获得颜色函数
		QImage image11 = QImage((const uchar*)camera->carsample[0].templ->imageData, 
								camera->carsample[0].templ->width, 
								camera->carsample[0].templ->height, 
								QImage::Format_RGB888).rgbSwapped();
		QImage image22 = QImage((const uchar*)camera->carsample[1].templ->imageData, 
								camera->carsample[1].templ->width, 
								camera->carsample[1].templ->height, 
								QImage::Format_RGB888).rgbSwapped();

		color1->setPixmap(QPixmap::fromImage(image11));
		color2->setPixmap(QPixmap::fromImage(image22));

		int temp = judgeCPLDChanged();  //检验CPLD
		if(temp == 1)
		{
			reset();
			gameNumber = 0;    //比赛时间重置
			creatOverDialog(troopAComboBox->currentText(), -2, 0);
		}
		else if(temp == 2)
		{
			reset();
			gameNumber = 0;    //比赛时间重置
			creatOverDialog(troopBComboBox->currentText(), -2, 0);
		}
		//else
		//	gameMessageBrowser->append(QStringLiteral("-------------------------  双方检测正常  -------------------------"));
		imageLable->setMovie(beforeGameMovie);           //装载GIF图片
		beforeGameMovie->start();                                   //开始播放倒计时
		beforeTimer->setInterval(ONE_SECOND);             //倒计时5秒的计时器
		beforeTimer->start();                                             //开启倒计时计时器
	}
	else if(gameState == SUSPEND)
		restart();
	else
		start();
}

/* 比赛时间计时器触发的槽函数 */
void MainWindow::onGameTimerOut()
{
	gameTimeLcdSec->display(gameNumber % 60);
	gameTimeLcdMin->display(gameNumber / 60);
	gameNumber++;
}

 /* 结束按钮触发的槽函数 */
void MainWindow::onEndButton()
{
	QPixmap pix = QPixmap("image/pause.jpg");  //播放比赛结束图片
    imageLable->setPixmap(pix);
	gameState = PREPARE;                                   //比赛开始标志重置
	reset();                                                            //比赛信息重置
	for(int i = 0;i < PLAYER_NUMBER;++i)            //抢跑信息重置
		numberOfFalseStart[i] = 0;
	if(troopTurn[0] == NUMBER_OF_TURNS         //有一方完成比赛触发了结束
		|| troopTurn[1] == NUMBER_OF_TURNS)
		creatOverDialog(this->returnWinnerName(), 
								  troopTime[0], 
						          troopTime[1]);                   //创建结果对话框

	/* 手动结束比赛，双方的时间均为当前比赛时间  */
	else                                      
		creatOverDialog(this->returnWinnerName(), 
						gameNumber, 
						gameNumber);           //创建结果对话框
	gameNumber = 0;                         //比赛时间重置
}

/* 暂停按钮触发的槽函数 */
void MainWindow::onPauseButton()
{
	QPixmap pix = QPixmap("image/pause.jpg");  //播放比赛暂停图片
    imageLable->setPixmap(pix);
	imagePauseLable->show();
	gameState = SUSPEND;
	gameTimer->stop();
	cameraTimer->stop();
	gameMusic->pause();

	/* 比赛暂停时隐藏所有道具 */
	propertyLabel1->hide();
	propertyLabel2->hide();
	propertyLabel3->hide();
	propertyLabel4->hide();

	startButton->setEnabled(true);
	endButton->setEnabled(false);
	pauseButton->setEnabled(false);
	gameMessageBrowser->append(QStringLiteral("--------------------------   比赛暂停   --------------------------"));
}

/* 返回胜利者队名 */
QString MainWindow::returnWinnerName()
{
	QString temp(QStringLiteral(" "));
	if(troopTime[0] < troopTime[1])
		return troopAComboBox->currentText();
	else if(troopTime[0] > troopTime[1])
		return troopBComboBox->currentText();

#ifdef FOUR_PLAYER
	//...
	//四个人判断获胜者的函数
	//...
	//...
#endif
	else
		return temp;
}

/* 串口操作函数 */
bool MainWindow::serialProtOperation(const GAMESTATE &gameState)
{
	int flag;
	if(gameState == PREPARE)
		flag = 0;
	else if(gameState == START)
		flag = 1;
	else if(gameState == SUSPEND)
		flag = 2;
	else
		flag = 3;
	/* 将选手信息传给通信部分 */
	communication->getInformation(allProperty, 
								  flag, 
								  propertyBool, 
								  transformPropertyPosition,
								  player);

	communication->sendData();
	communication->ReceiveData();

	/* 从通信部分获得选手信息 */
	for(int i = 0;i < PLAYER_NUMBER;++i)
		player[i] = communication->playerInfo[i];
	return true;
}

/* 视频图像更新函数 */
void MainWindow::VideoUpdate()
{
	camera->GetInfo();     //更新图像与坐标
	frame = camera->show;  //获得图像

	/* 在界面上显示坐标 */
	troopAHeadCoordinateX->display(camera->car[0].position.x);
	troopAHeadCoordinateY->display(camera->car[0].position.y);
	troopBHeadCoordinateX->display(camera->car[1].position.x);
	troopBHeadCoordinateY->display(camera->car[1].position.y);

#ifdef FOUR_PLAYER
	troopCHeadCoordinateX->display(camera->car[2].position.x);
	troopCHeadCoordinateY->display(camera->car[2].position.y);
	troopDHeadCoordinateX->display(camera->car[3].position.x);
	troopDHeadCoordinateY->display(camera->car[3].position.y);
#endif

	/* 转换为QImage */
    if(frame != NULL)
        image = QImage((const uchar*)frame->imageData, frame->width, frame->height, QImage::Format_RGB888).rgbSwapped();

#ifdef MAIN_DEBUG
	/* 调试时使用本地图像 */
	image = QImage("testcam1 3.jpg");
#endif

	/* 显示展示图像 */
    imageLable->setPixmap(QPixmap::fromImage(image));
	//imageLableMask->setPixmap(QPixmap::fromImage(*testMask));
	/* 获取选手坐标 */
	for(int i = 0;i < PLAYER_NUMBER;++i)
	{
		player[i].Head_x = camera->car[i].position.x;
		player[i].Head_y = camera->car[i].position.y;
	}
	/* 在图像更新之后才能调用逻辑判断函数，否则会出现坐标未及时更新造成的逻辑判断失误 */
	judge();
}

/* 背景音乐对话框创建 */
void MainWindow::setMusic()
{
	gameMusic->setWindowModality(Qt::ApplicationModal);  /* 模态对话框 */
	gameMusic->show();
}

/* 倒计时定时器溢出触发函数 */
void MainWindow::onBeforeTimer()
{
	beforeTime += 0.5;
	if(beforeTime == 2)
	{
		beforeTime = 0;
		start();
	}
}

/* 开始函数 */
void MainWindow::start()
{
	sampleCarProperty = 0;
	//初始化选手状态
	for(int i = 0;i < PLAYER_NUMBER;++i)
	{
		player[i].state = NORMAL;
		player[i].propertyState = NO_ATTACT;
		playerInPitTime[i] = 0;
	}
	endButton->setEnabled(true);                          //正式开始结束按钮使能
	pauseButton->setEnabled(true);                        //暂停按钮使能
	beforeGameMovie->stop();                              //倒计时动画停止
	beforeTimer->stop();                                  //倒计时计时器停止
	gameTimer->setInterval(ONE_SECOND);                   //以秒为计数单位 
	gameTimer->start();                                   //比赛计时器开始
	gameMusic->play();                                    //播放音乐
	cameraTimer->start(CAMERA_DELAY);                     //开启视频计时器
	propertyTimer->start(PRODUCE_PROPRTY);                //开启产生道具计时器

	/* 显示道具图标 */
	propertyLabel1->setGeometry(propertyPosition[0][0]*2.352 + IMAGELABEL_LENGTH_WIDTH,
								propertyPosition[0][1]*2.352 + IMAGELABEL_LENGTH_HEIGHT, 
								31, 
								31); //道具1图标显示
	propertyLabel2->setGeometry(propertyPosition[1][0]*2.352 + IMAGELABEL_LENGTH_WIDTH,
								propertyPosition[1][1]*2.352 + IMAGELABEL_LENGTH_HEIGHT, 
								31, 
								31); //道具2图标显示
	propertyLabel3->setGeometry(propertyPosition[2][0]*2.352 + IMAGELABEL_LENGTH_WIDTH,
								propertyPosition[2][1]*2.352 + IMAGELABEL_LENGTH_HEIGHT, 
								31, 
								31); //道具3图标显示
	propertyLabel4->setGeometry(propertyPosition[3][0]*2.352 + IMAGELABEL_LENGTH_WIDTH,
								propertyPosition[3][1]*2.352 + IMAGELABEL_LENGTH_HEIGHT, 
								31, 
								31); //道具4图标显示
	
	/* 显示道具坐标与图像 */
	propertyShow1->setText(propertyName[allProperty[0].kindOfProperty]);
	property1CoordationX->display(propertyPosition[0][0]);
	property1CoordationY->display(propertyPosition[0][1]);
	propertyShow2->setText(propertyName[allProperty[1].kindOfProperty]);
	property2CoordationX->display(propertyPosition[1][0]);
	property2CoordationY->display(propertyPosition[1][1]);
	propertyShow3->setText(propertyName[allProperty[2].kindOfProperty]);
	property3CoordationX->display(propertyPosition[2][0]);
	property3CoordationY->display(propertyPosition[2][1]);
	propertyShow4->setText(propertyName[allProperty[3].kindOfProperty]);
	property4CoordationX->display(propertyPosition[3][0]);
	property4CoordationY->display(propertyPosition[3][1]);

	propertyLabel1->show();
	propertyLabel2->show();
	propertyLabel3->show();
	propertyLabel4->show();

	gameMessageBrowser->append(QStringLiteral("-------------------------------   比赛开始   ------------------------------"));
	gameState = START;
	//for(int i = 0;i < PLAYER_NUMBER;++i)
	//{
	//	if(judgeFalseStart(i));                               //判断是否抢跑
	//		return;
	//}
}

/* 暂停后开始函数 */
void MainWindow::restart()
{
	troopAState->setText(QStringLiteral("无警告"));
	troopBState->setText(QStringLiteral("无警告"));
	troopAPosition->setText(QStringLiteral("位于界内"));
	troopBPosition->setText(QStringLiteral("位于界内"));

	//初始化选手状态
	for(int i = 0;i < PLAYER_NUMBER;++i)
	{
		player[i].state = NORMAL;
		player[i].propertyState = NO_ATTACT;
		playerInPitTime[i] = 0;
	}
	endButton->setEnabled(true);                          //正式开始结束按钮使能
	pauseButton->setEnabled(true);                        //暂停按钮使能
	beforeGameMovie->stop();                              //倒计时动画停止
	beforeTimer->stop();                                  //倒计时计时器停止
	gameTimer->setInterval(ONE_SECOND);                   //以秒为计数单位 
	gameTimer->start();                                   //比赛计时器开始
	gameMusic->play();                                    //播放音乐
	cameraTimer->start(CAMERA_DELAY);                     //开启视频计时器
	propertyTimer->start(PRODUCE_PROPRTY);                //开启产生道具计时器

	/* 恢复道具显示 */
	if(propertyBool[0])
		propertyLabel1->show();
	if(propertyBool[1])
		propertyLabel2->show();
	if(propertyBool[2])
		propertyLabel3->show();
	if(propertyBool[3])
		propertyLabel4->show();

	gameMessageBrowser->append(QStringLiteral("------------------------------   比赛开始   -----------------------------"));
	gameState = START;
	imagePauseLable->hide();
}

/* 随机产生道具计时器 */
/* 每隔固定长度时间会刷新一次场上道具 */
void MainWindow::onPropertyTimer()
{
	for(int i = 0;i < NUMBER_OF_PROPERTY;++i)
		if(allProperty[i].owner != NONE)   //道具已经被捡走
		{
			allProperty[i].changeKind();   //更改道具种类
			allProperty[i].owner = NONE;   //道具无主
			propertyBool[i] = true;
			/* 重新显示新产生的道具 */
			switch(i)
			{
			case 0:
				propertyLabel1->show();
				propertyShow1->setText(propertyName[allProperty[i].kindOfProperty]);
				property1CoordationX->display(propertyPosition[i][0]);
				property1CoordationY->display(propertyPosition[i][1]);
				break;
			case 1:
				propertyLabel2->show();
				propertyShow2->setText(propertyName[allProperty[i].kindOfProperty]);
				property2CoordationX->display(propertyPosition[i][0]);
				property2CoordationY->display(propertyPosition[i][1]);
				break;
			case 2:
				propertyLabel3->show();
				propertyShow3->setText(propertyName[allProperty[i].kindOfProperty]);
				property3CoordationX->display(propertyPosition[i][0]);
				property3CoordationY->display(propertyPosition[i][1]);
				break;
			case 3:
				propertyLabel4->show();
				propertyShow4->setText(propertyName[allProperty[i].kindOfProperty]);
				property4CoordationX->display(propertyPosition[i][0]);
				property4CoordationY->display(propertyPosition[i][1]);
				break;
			}
		}
}

/* 判断是否出界的函数 */
void MainWindow::judgeOutside()
{
	bool flag111 = false;
	for(int i = 0;i < PLAYER_NUMBER; ++i)
	{

/* 不对样车进行出界判断 */
#ifdef SAMPLE_CAR
		if(i == 0)
			continue;
#endif
		if(communication->restart)
		{
			onPauseButton();
			communication->restart = false;
		}

		/* 判断是否处于出界状态以及已经重置位置 */
		if(player[i].state == OUTSIDE)
		{
			if(gameState == START)
			{
				if(i == 0)
				{
					troopAPosition->setText((QStringLiteral(" 位于界外 ")));
					troopAState->setText((QStringLiteral(" 出界警告 ")));
				}
				else if(i == 1)
				{
					troopBPosition->setText((QStringLiteral(" 位于界外 ")));
					troopBState->setText((QStringLiteral(" 出界警告 ")));
				}

#ifdef FOUR_PLAYER
				else if(i == 2)
					gameMessageBrowser->append("3" + QStringLiteral(" 选手出界 "));
				else if(i == 3)
					gameMessageBrowser->append("3" + QStringLiteral(" 选手出界 "));
#endif

				else;
				/* 调用暂停函数 */
				//onPauseButton();
				/* 决赛出界只对出界选手暂停 */
			}
		}

		/* 判断位置坐标的像素点值 */
		else if(imageMatrix[player[i].Head_x][player[i].Head_y][0] == 1)
		{
			player[i].state = WARNING;  //缓冲区给予警告，并标记位置供出界后放回
			if(gameState == START)
			{
				if(i == 0)
					troopAPosition->setText(QStringLiteral("位于缓冲区"));
				else
					troopBPosition->setText(QStringLiteral("位于缓冲区"));
				showOutImage(i, player[i].Head_x, player[i].Head_y);   //标记位置
			}
		}
		/* 对于出界的判断 */
		else if(imageMatrix[player[i].Head_x][player[i].Head_y][0] == 2)
		{
			flag111 = true;
			/* 选取3个单位作为阈值，如果此区域内一个非黑色点都没有则确定出界 */
			for(int k = player[i].Head_x - 5;k < player[i].Head_x + 5;k++)
			{
				for(int j = player[i].Head_y - 5;j < player[i].Head_y + 5;++j)
				{
					if(imageMatrix[k][j][0] != 2)
					{
						flag111 = false;
						break;
					}
				}
			}
			if(flag111 == true)
				player[i].state = OUTSIDE;
		}
		else;
//		{
//			if(player[i].state == WARNING)//判断之前是否位于缓冲区，如果是则清除出界标志
//			{
//				if(i == 1)
//					outImage1->hide();
//
//#ifdef FOUR_PLAYER
//				else if(i == 3)
//					outImage3->hide();
//				else if(i == 4)
//					outImage4->hide();
//#endif
//
//				else 
//					outImage2->hide();
//			}
//			//player[i].state = NORMAL;
			//if(i == 0)
			//{
			//	troopAPosition->setText(QStringLiteral("位于跑道内"));
			//	troopAState->setText((QStringLiteral("无警告")));
			//}
			//else
			//{
			//	troopBPosition->setText(QStringLiteral("位于跑道内"));
			//	troopBState->setText((QStringLiteral("无警告")));
			//}
		//
		//if(player[i].state == REVERSE && communication->reverseNow[i] == true)
		//{
		//	/* 清除反向标记 */
		//	for(int j = 0;j < PLAYER_NUMBER;++j)
		//	{
		//		communication->reverseNow[j] = false;
		//		communication->reverseTime[j] = 0;
		//	}
		//	/* 调用暂停函数 */
		//	/*onPauseButton();*/
		//	/* 逆行一段时间才会暂停 */
		//}
	}
}

/* 判断是否到达终点的函数 */
/* 需要在比赛开始时确定选手的起点 */
/* 由于两车起始位置不同故判断圈数的时候数字不同 */
void MainWindow::judgeFinish()
{
	for(int i = 0;i < PLAYER_NUMBER; ++i)
	{
		/* 依次判断经过的一圈的位置 */
		/* 经过起点 */
		if(distance(player[i].Head_x, player[i].Head_y, playerStartPosition[i][0], playerStartPosition[i][1]) < 20)
			oneTurn[i][0] = true;
		/* 经过中间检验点 */
		if(distance(player[i].Head_x, player[i].Head_y, playerMidPosition[i][0], playerMidPosition[i][1]) < 20)
			oneTurn[i][1] = true;
		/* 再次经过起点 */
		if(distance(player[i].Head_x, player[i].Head_y, playerStartPosition[i][0], playerStartPosition[i][1]) < 20
			&& oneTurn[i][0]
			&& oneTurn[i][1])
			oneTurn[i][2] = true;

		/* 判断一圈是否完成 */
		if(oneTurn[i][2])
		{
			troopTurn[i]++;

			/* 显示当前完成的圈数 */
			switch(troopTurn[i])
			{
			case 1:if(i == 0)
						gameMessageBrowser->append(troopAComboBox->currentText() + QStringLiteral(" 完成 1 圈"));
					else
						gameMessageBrowser->append(troopBComboBox->currentText() + QStringLiteral(" 完成 1 圈"));
					break;
			case 2:if(i == 0)
						gameMessageBrowser->append(troopAComboBox->currentText() + QStringLiteral(" 完成 2 圈"));
					else
						gameMessageBrowser->append(troopBComboBox->currentText() + QStringLiteral(" 完成 2 圈"));
					break;
			case 3:if(i == 0)
						gameMessageBrowser->append(troopAComboBox->currentText() + QStringLiteral(" 完成 3 圈"));
					else
						gameMessageBrowser->append(troopBComboBox->currentText() + QStringLiteral(" 完成 3 圈"));
					break;
			}
			//重置检验点信息
			for(int j = 0;j < 3;++j)
				oneTurn[i][j] = false;
		}
		/* 判断比赛是否完成 */
		if(troopTurn[i] == NUMBER_OF_TURNS)
		{
			player[i].state = FINISH;
			troopTime[i] += gameNumber;
			troopTime[1-i] = 0;
			if(i == 0)
			{
				gameMessageBrowser->append(troopAComboBox->currentText() + QStringLiteral(" 完成比赛"));
				troopAState->setText(QStringLiteral("完成比赛"));
				onEndButton();
			}
			else
			{
				gameMessageBrowser->append(troopBComboBox->currentText() + QStringLiteral(" 完成比赛"));
				troopAState->setText(QStringLiteral("完成比赛"));
				onEndButton();
			}
		}
	}
	/* 判断双方是否均完成比赛 */
	/* 只要有一方完成比赛则比赛结束 */
	//if(player[0].state == FINISH || player[1].state == FINISH)
	//{
	//	if(player[0].state == FINISH)
	//		troopTime[0] = gameNumber;
	//	if(player[1].state == FINISH)
	//		troopTime[1] = gameNumber;
	//	onEndButton();
	//}
}

/* 计算两点间距离的函数 */
double MainWindow::distance(const int &ax, const int &ay, const int &bx, const int &by)
{
	return sqrt(((ax - bx) * (ax - bx) + (ay - by) * (ay - by)));
} 

/* 判断是否与道具相遇，即获得道具 */
/* 暂定以小车中心点与道具点重合为获得道具标志 */
bool MainWindow::judgeGetProperty(const int &playerNumber)
{
	for(int i = 0;i < NUMBER_OF_PROPERTY;++i)
	{
		if(abs(player[playerNumber].Head_x - allProperty[i].positionX) < 20  
			&& abs(player[playerNumber].Head_y - allProperty[i].positionY) < 20
			&& propertyBool[i]
			&& communication->getProperty[playerNumber])   //选手发出了捡道具的命令
		{
			communication->getProperty[playerNumber] = false;     //捡道具数组复位
			if(!player[playerNumber].ifHaveProperty1)
			{
				player[playerNumber].myProperty[0].kindOfProperty = allProperty[i].kindOfProperty;
				player[playerNumber].ifHaveProperty1 = true;
			}
			else if (!player[playerNumber].ifHaveProperty2)
			{
				player[playerNumber].myProperty[1].kindOfProperty = allProperty[i].kindOfProperty;
				player[playerNumber].ifHaveProperty2 = true;
			}
			else
			{
				player[playerNumber].myProperty[0].kindOfProperty = player[playerNumber].myProperty[1].kindOfProperty;
				player[playerNumber].myProperty[1].kindOfProperty = allProperty[i].kindOfProperty;
			}
			allProperty[i].owner = (PROPERTY_OWNER)playerNumber;
			/* 被获得的道具从图像上消失 */
			propertyBool[i] = false;  //道具数组赋值
			switch(i)
			{
			case 0:propertyLabel1->hide();
				propertyShow1->setText(QStringLiteral("无道具"));
				property1CoordationX->display(0);
				property1CoordationY->display(0);
				break;
			case 1:propertyLabel2->hide();
				propertyShow2->setText(QStringLiteral("无道具"));
				property2CoordationX->display(0);
				property2CoordationY->display(0);
				break;
			case 2:propertyLabel3->hide();
				propertyShow3->setText(QStringLiteral("无道具"));
				property3CoordationX->display(0);
				property3CoordationY->display(0);
				break;
			case 3:propertyLabel4->hide();
				propertyShow4->setText(QStringLiteral("无道具"));
				property4CoordationX->display(0);
				property4CoordationY->display(0);
				break;
			default:
				gameMessageBrowser->append(QStringLiteral(" 道具数量错误 "));
			}

			/* 显示获得道具信息 */
			if(playerNumber == 0)
				gameMessageBrowser->append(QStringLiteral(" 选手 1 获得了 ") 
											+ propertyName[allProperty[i].kindOfProperty]);
			else if(playerNumber == 1)
				gameMessageBrowser->append(QStringLiteral(" 选手 2 获得了 ") 
											+ propertyName[allProperty[i].kindOfProperty]);
			else;

			/* 已经被捡取的道具标记 */
			allProperty[i].owner = (PROPERTY_OWNER)playerNumber;
		}
		//else
		//	communication->getProperty[playerNumber] = false;   //不符合捡取道具的条件，无效信号复位
	}
	/* 刷新双方的道具类型 */
	if(player[playerNumber].ifHaveProperty1)
	{
		if(playerNumber == 0)
			troopAProperty1->setText(propertyName[player[playerNumber].myProperty[0].kindOfProperty]);
		else
			troopBProperty1->setText(propertyName[player[playerNumber].myProperty[0].kindOfProperty]);
	}
	else
	{
		if(playerNumber == 0)
			troopAProperty1->setText(QStringLiteral("没有道具"));
		else
			troopBProperty1->setText(QStringLiteral("没有道具"));
	}
	if(player[playerNumber].ifHaveProperty2)
	{
		if(playerNumber == 0)
			troopAProperty2->setText(propertyName[player[playerNumber].myProperty[1].kindOfProperty]);
		else
			troopBProperty2->setText(propertyName[player[playerNumber].myProperty[1].kindOfProperty]);
	}
	else
	{
		if(playerNumber == 0)
			troopAProperty2->setText(QStringLiteral("没有道具"));
		else
			troopBProperty2->setText(QStringLiteral("没有道具"));
	}
	if(!player[playerNumber].ifHaveProperty1 && player[playerNumber].ifHaveProperty2)
	{
		player[playerNumber].myProperty[0].kindOfProperty = player[playerNumber].myProperty[1].kindOfProperty;
		player[playerNumber].myProperty[1].kindOfProperty = NO_PROPERTY;
		player[playerNumber].ifHaveProperty2 = false;
		player[playerNumber].ifHaveProperty1 = true;
	}
	return true;
}

/* 判断是否在范围内的函数 */
bool MainWindow::judgeInRange(const int &a, const int &b, const int &current)
{
	if(current < a && current > b)
		return true;
	if(current > a && current < b)
		return true;
	return false;
}

/* 判断是否在陷阱内的函数 */
int MainWindow::judgeIntoPitfall(const int &playerNumber)
{
	for(int i = 0;i < NUMBER_OF_PROPERTY;++i)                        //最多有4个陷阱
	{
		/* 以左上角为起点定位陷阱点 */
		if(communication->communicatePitfallMatrix[i][2] != -1)      //陷阱存在
		{
			int x1 = communication->communicatePitfallMatrix[i][0] - WIGHT_OF_PITFALL;
			int y1 = communication->communicatePitfallMatrix[i][1] + WIGHT_OF_PITFALL;
			int x2 = communication->communicatePitfallMatrix[i][0] + WIGHT_OF_PITFALL;
			int y2 = communication->communicatePitfallMatrix[i][1] - WIGHT_OF_PITFALL;

			if(judgeInRange(x1, x2, player[playerNumber].Head_x)
			   && judgeInRange(y1, y2, player[playerNumber].Head_y)
			   && communication->communicatePitfallMatrix[i][2] != playerNumber) //非陷阱的所属方
			{
				communication->communicatePitfallMatrix[i][2] = -1; //陷阱道具生效后消失
				if(i == 0)
					pit1->hide();   
				else if(i == 1)
					pit2->hide();
				else if(i == 2)
					pit3->hide();
				else
					pit4->hide();
				return communication->communicatePitfallMatrix[i][3];//返回起作用道具的种类
			}
		}
	}
	return -1;  //未触发陷阱的返回值
}


/* 判断陷阱道具的作用 */
void MainWindow::judgePitfallProperty()
{
	/* 判断是否进入陷阱 */
	for(int i = 0;i < PLAYER_NUMBER;++i)
	{
		int pit = judgeIntoPitfall(i);  //获得被攻击的陷阱种类，没有为-1
		if(pit == -1)
			continue;            //未进入陷阱则结束循环

		/* 有保护状态则循环结束 */
		if(player[i].propertyState == PROTECTED_ONCE || player[i].propertyState == PROTECTED_SOMETIME)
		{
			player[i].propertyState = NO_ATTACT;
			if(i == 0)
				gameMessageBrowser->append(QStringLiteral("选手1 使用 保护 避免了陷阱"));
			else
				gameMessageBrowser->append(QStringLiteral("选手2使用 保护 避免了陷阱"));
			continue;
		}
		/* 进行陷阱判断 */
		switch(pit)
		{
			case MARSH:
				player[i].propertyState = INTO_MARSH;
				if(i == 0)
					gameMessageBrowser->append(QStringLiteral("选手1 陷入 沼泽 "));
				else
					gameMessageBrowser->append(QStringLiteral("选手2 陷入 沼泽 "));
				break;
			case WHIRLPOOL:
				player[i].propertyState = INTO_WHIRLPOOL;
				if(i == 0)
					gameMessageBrowser->append(QStringLiteral("选手1 陷入 漩涡 "));
				else
					gameMessageBrowser->append(QStringLiteral("选手2 陷入 漩涡 "));
				break;
			case LANDMINE:
				player[i].propertyState = LANDMINE_ATTACT;
				if(i == 0)
					gameMessageBrowser->append(QStringLiteral("选手1 陷入 地雷 "));
				else
					gameMessageBrowser->append(QStringLiteral("选手2 陷入 地雷 "));
				break;
		}
	}
}

/* 判断反向和违规 */
 void MainWindow::judgeCondition(const int &Head_x, const int &Head_y, const int &playerNumber)
{   

/* 不对样车进行出界判断 */
#ifdef SAMPLE_CAR
		if(playerNumber == 0)
			return;
#endif
		if(reverseTime == 5)
		{
			reverseTime = 0;
			int x1 = lastPosition[playerNumber][0] - 127;
			int y1 = lastPosition[playerNumber][1] - 127;

			int x2 = player[playerNumber].Head_x - lastPosition[playerNumber][0];
			int y2 = player[playerNumber].Head_y - lastPosition[playerNumber][1];

			if(x1*y2 - x2*y1 < 0)
			{
				player[playerNumber].state = REVERSE;
				if(playerNumber == 0)
					troopAState->setText(QStringLiteral("逆行警告 "));
				else
					troopBState->setText(QStringLiteral("逆行警告 "));
			}
			for(int i = 0;i <PLAYER_NUMBER;++i)
			{
				lastPosition[i][0] = player[i].Head_x;
				lastPosition[i][1] = player[i].Head_y;
			}
		}
		else
			reverseTime++;


	//nextPosition[playerNumber] = 0;
	//int n = 40;	  //选取40个参考点
 //   int Hdistance[40];
	//for(int i = 0;i < n;++i)
	//{ 
	//	Hdistance[i] = distance(Head_x, Head_y, reversePosition[i][0], reversePosition[i][1]);	 
	//}
 //   for(int i = 0;i < n - 1;++i)
 //   {  
	//	if(Hdistance[i] > Hdistance[i + 1])
	//		nextPosition[playerNumber] = i + 1;
	//    else 
	//		Hdistance[i + 1] = Hdistance[i];
 //   }

	////表示走错路，要进行走错路警告并送回原处
	//if((abs(reversePosition[nextPosition[playerNumber]][2] - reversePosition[curPosition[playerNumber]][2]) >= 3)
	//	&& (reversePosition[curPosition[playerNumber]][2] != 0) 
	//	&& (reversePosition[curPosition[playerNumber]][2] != 1)
	//	&& (reversePosition[curPosition[playerNumber]][2] != 2))
	//{
	//	player[playerNumber].state = BREAK_RULES;
	//	if(playerNumber == 0)
	//		troopAState->setText(QStringLiteral("犯规警告 "));
	//	else
	//		troopBState->setText(QStringLiteral("犯规警告 "));
	//}

	//// 反向跑动,进行警告
	//if(reversePosition[nextPosition[playerNumber]][2] < reversePosition[curPosition[playerNumber]][2]
	//		|| (reversePosition[curPosition[playerNumber]][2] == 0 && reversePosition[nextPosition[playerNumber]][2] == 37)
	//		|| (reversePosition[curPosition[playerNumber]][2] == 1 && reversePosition[nextPosition[playerNumber]][2] == 37)
	//		|| (reversePosition[curPosition[playerNumber]][2] == 0 && reversePosition[nextPosition[playerNumber]][2] == 36)
	//		|| (reversePosition[curPosition[playerNumber]][2] == 1 && reversePosition[nextPosition[playerNumber]][2] == 36))
	//{
	//	player[playerNumber].state = REVERSE;
	//	if(playerNumber == 0)
	//		troopAState->setText(QStringLiteral("逆行警告 "));
	//	else
	//		troopBState->setText(QStringLiteral("逆行警告 "));
	//}
	////状态正常
	//else  
	//{
	//	curPosition = nextPosition;
	//	//player[playerNumber].state = NORMAL;
	//}
 }   

/* 判断保护类道具的有效时间 */
 void MainWindow::judgeProtectedTime()
 {
	 for(int i = 0;i < PLAYER_NUMBER;++i)
	{
		if(player[i].propertyState == PROTECTED_SOMETIME)
			troopProtectedTime[i]++;  //用于计数保护时间的秒
		if(troopProtectedTime[i] == PROTECTED_TIME)
		{
			troopProtectedTime[i] = 0;
			player[i].propertyState = NO_ATTACT;
		}
	}
 }

/* 总判断函数，100毫秒触发一次 */
/* 判断函数的结算顺序由规则优先级决定 */
void MainWindow::judge()
{

/* 样车每次获得道具表的第一个道具 */
#ifdef SAMPLE_CAR
	if(sampleCarProperty == 100)
	{
		sampleCarProperty = 0;
		if(!player[0].ifHaveProperty1)
		{
			player[0].myProperty[0].kindOfProperty = sampleCarPro->kindOfProperty;
			player[0].ifHaveProperty1 = true;
			sampleCarPro->changeKind();
		}
		else if (!player[0].ifHaveProperty2)
		{
			player[0].myProperty[1].kindOfProperty = sampleCarPro->kindOfProperty;
			player[0].ifHaveProperty2 = true;
			sampleCarPro->changeKind();
		}
		else
		{
			player[0].myProperty[0].kindOfProperty = player[0].myProperty[1].kindOfProperty;
			player[0].myProperty[1].kindOfProperty = sampleCarPro.kindOfProperty;
			sampleCarPro.changeKind();
		}
	}
	else
		sampleCarProperty++;
#endif


	/* 每次结算之前初始化多有的文本框显示（道具初始化包含在捡道具函数内部） */
	troopAState->setText(QStringLiteral("无警告"));
	troopBState->setText(QStringLiteral("无警告"));
	troopAPosition->setText(QStringLiteral("位于界内"));
	troopBPosition->setText(QStringLiteral("位于界内"));

	/* 开始比赛才会进行判断 */
	if(gameState == START)
	{
		judgeFinish();                         // 判断比赛是否结束
		judgeOnPause();                    // 监测是否触发暂停
		//judgeOutside();                   // 判断是否出界和逆行的触发暂停
		updatePropertyInfluence();    // 更新道具影响
		judgeProtectedTime();           // 判断保护道具的时间 

		//judgeInPit();                        // 判断是否进入死区
		for(int i = 0;i < PLAYER_NUMBER;++i) 
		{
			judgeGetProperty(i);  //判断是否获得道具
			judgePropertyUse(i);  //判断道具使用
			/* 判断是否反向 */
			//judgeCondition(player[i].Head_x, player[i].Head_y, i);
		}
		communication->updatePropertyTime(); //更新预备道具数组
		judgePitfallProperty();                              //判断陷阱道具的作用效果

		/* 判断当前场上无主道具的数组赋值 */
		for(int i= 0;i < NUMBER_OF_PROPERTY;++i)
		{
			if(allProperty[i].owner == NONE)
				propertyBool[i] = true;
			else
				propertyBool[i] = false;
		}
	}
}

/* 显示出界标志的函数 */
void MainWindow::showOutImage(const int &player, const int &x, const int &y)
{
	if(player == 1)
		outImage1->setGeometry(x, y, SIZE_OF_OUTIMAGE, SIZE_OF_OUTIMAGE);
#ifdef FOUR_PLAYER
	else if(player == 3)
		outImage3->setGeometry(x, y, SIZE_OF_OUTIMAGE, SIZE_OF_OUTIMAGE);
	else if(player == 4)
		outImage4->setGeometry(x, y, SIZE_OF_OUTIMAGE, SIZE_OF_OUTIMAGE);
#endif
	else
		outImage2->setGeometry(x, y, SIZE_OF_OUTIMAGE, SIZE_OF_OUTIMAGE);
}

/* 判断出界后是否返回 */
bool MainWindow::judgeComeBack(const int &playerNumber)
{
	int x1,x2,y1,y2;
	if(playerNumber == 0)
	{
		x1 = outImage1->x();
		x2 = outImage1->x() + WIGHT_OF_OUTSIDE_IMAGE;
		y1 = outImage1->y();
		y2 = outImage1->y() + WIGHT_OF_OUTSIDE_IMAGE;
	}
#ifdef FOUR_PLAYER
	else if(playerNumber == 2)
	{
		x1 = outImage3->x();
		x2 = outImage3->x() + WIGHT_OF_OUTSIDE_IMAGE;
		y1 = outImage3->y();
		y2 = outImage3->y() + WIGHT_OF_OUTSIDE_IMAGE;
	}
	else if(playerNumber == 3)
	{
		x1 = outImage4->x();
		x2 = outImage4->x() + WIGHT_OF_OUTSIDE_IMAGE;
		y1 = outImage4->y();
		y2 = outImage4->y() + WIGHT_OF_OUTSIDE_IMAGE;
	}
#endif
	else  //2号选手
	{
		x1 = outImage2->x();
		x2 = outImage2->x() + WIGHT_OF_OUTSIDE_IMAGE;
		y1 = outImage2->y();
		y2 = outImage2->y() + WIGHT_OF_OUTSIDE_IMAGE;
	}

	/* 判断当前位置是否在出界标志内 */
	if(judgeInRange(x1, x2, player[playerNumber].Head_x)
		&& judgeInRange(y1, y2, player[playerNumber].Head_y))
		return true;
	return false;
}

/* 罚时倒计时触发 */
void MainWindow::onOutsideTimer()
{
	QTimer* timer = dynamic_cast<QTimer*>(sender()); /* 获取信号发出者 */

	if(timer == punishTimer1)
	{
		player[0].state = NORMAL;
		punishTimer1->stop();
	}
	else if(timer == punishTimer2)
	{
		player[1].state = NORMAL;
		punishTimer2->stop();
	}

#ifdef FOUR_PLAYER
	else if(timer == punishTimer3)
	{
		player[2].state = NORMAL;
		punishTimer3->stop();
	}
	else if(timer == punishTimer4)
	{
		player[3].state = NORMAL;
		punishTimer4->stop();
	}
#endif

	else
	{
		gameMessageBrowser->append(QStringLiteral(" 罚时触发错误 "));
		//punishTimer1->stop();
		//punishTimer2->stop();
	}
}

/* 退出按钮触发的函数 */
void MainWindow::allClose()
{
	/* 计时器关闭 */
    cameraTimer->stop();
    gameTimer->stop();
	propertyTimer->stop();
	punishTimer1->stop();
	punishTimer2->stop();

#ifdef FOUR_PLAYER
	punishTimer3->stop();
	punishTimer4->stop();
#endif
	this->close();
}

/* 判断抢跑的函数 */
bool MainWindow::judgeFalseStart(const int &playerNumber)    
{
#ifndef FALSE_START
	if(NUMBER_OF_FALSESTART == numberOfFalseStart[playerNumber])   //抢跑三次
	{
		if(playerNumber == 0)
		{
			gameMessageBrowser->append(troopAComboBox->currentText() + QStringLiteral(" 抢跑超过3次 "));
			troopAState->setText(QStringLiteral(" 取消比赛资格 "));
		}
		else
		{
			gameMessageBrowser->append(troopBComboBox->currentText() + QStringLiteral(" 抢跑超过3次 "));
			troopBState->setText(QStringLiteral(" 取消比赛资格 "));
		}
		reset(); 
		gameNumber = 0;    //比赛时间重置
		gameState = PREPARE; 
		for(int i = 0;i < PLAYER_NUMBER;++i)       //清除双方的抢跑标志
			numberOfFalseStart[i] = 0;
		if(playerNumber == 0)
		{
			creatOverDialog(troopAComboBox->currentText(), -1, 0);
			gameState = PREPARE; 
		}
		else if(playerNumber == 1)
		{
			creatOverDialog(troopBComboBox->currentText(), -1, 0);
			gameState = PREPARE; 
		}
		else;

#ifdef FOUR_PLAYER
		else if(playerNumber == 2)
			creatOverDialog(troopBComboBox->currentText(), -1, 0);
		else if(playerNumber == 3)
			creatOverDialog(troopBComboBox->currentText(), -1, 0);
#endif
		return true;
	}
#endif

	if(imageMatrix[player[playerNumber].Head_x][player[playerNumber].Head_y][0] != 3
		&& imageMatrix[player[playerNumber].Head_x][player[playerNumber].Head_y][0] != 4)
	{
		if(playerNumber == 0)
			troopAState->setText(QStringLiteral("抢跑警告 "));
		else
			troopBState->setText(QStringLiteral("抢跑警告 "));
		numberOfFalseStart[playerNumber]++;

#ifndef FALSE_START
		reset();
		gameState = PREPARE; 
#endif
		return true;
	}
	return false;
}

/* 复位函数 */
void MainWindow::reset()
{
	gameTimer->stop(); //计时器停止
	gameTimeLcdSec->display(gameNumber % 60);
	gameTimeLcdMin->display(gameNumber / 60);
	troopBProperty1->setText((QStringLiteral("没有道具")));
	troopAProperty1->setText((QStringLiteral("没有道具")));
	troopBProperty2->setText((QStringLiteral("没有道具")));
	troopAProperty2->setText((QStringLiteral("没有道具")));
	troopAPosition->setText((QStringLiteral("位于跑道内")));
	troopBPosition->setText((QStringLiteral("位于跑道内")));
	troopAState->setText((QStringLiteral("无警告")));
	troopBState->setText((QStringLiteral("无警告")));

	troopAHeadCoordinateX->display(0);
	troopAHeadCoordinateY->display(0);
	troopBHeadCoordinateX->display(0);
	troopBHeadCoordinateY->display(0);

#ifdef FOUR_PLAYER
	troopCProperty->setText((QStringLiteral("没有道具")));
	troopDProperty->setText((QStringLiteral("没有道具")));
#endif

	startButton->setEnabled(true);
	endButton->setEnabled(false);
	pauseButton->setEnabled(false);
	troopAComboBox->setEnabled(true);
	troopBComboBox->setEnabled(true);

#ifdef FOUR_PLAYER
	troopCComboBox->setEnabled(true);
	troopDComboBox->setEnabled(true);
#endif

	gameMusic->stop();                         //音乐停止    
	gameMessageBrowser->append(QStringLiteral("------------------------------   比赛结束   -----------------------------"));
	cameraTimer->stop();                       //摄像头计时停止
	QPixmap pix = QPixmap("image/pause.jpg");  //播放比赛结束图片
    imageLable->setPixmap(pix);

	/* 比赛双方数据的重置 */
	troopTime[0] = 0;
	troopTime[1] = 0;
	troopTurn[0] = 0;
	troopTurn[1] = 0;

#ifdef FOUR_PLAYER
	for(int i = 2;i < 4;++i)
	{
		troopTime[i] = 0;
		troopTurn[i] = 0;
	}
#endif

	for(int i = 0;i < PLAYER_NUMBER;++i)
		for(int j = 0;j < 3;++j)
			oneTurn[i][j] = false;

	for(int i = 0;i < PLAYER_NUMBER;++i)
	{
		curPosition[i] = 0;
		nextPosition[i] = 0;
	}
	outImage1->hide();    //出界标记1隐藏
	outImage2->hide();    //出界标记2隐藏
	outImage3->hide();    //出界标记1隐藏
	outImage4->hide();    //出界标记2隐藏

	propertyLabel1->hide(); //道具1图标隐藏
	propertyLabel2->hide(); //道具2图标隐藏
	propertyLabel3->hide(); //道具3图标隐藏
	propertyLabel4->hide(); //道具4图标隐藏

	pit1->hide();
	pit2->hide();
	pit3->hide();
	pit4->hide();

	/* 道具初始化 */
	propertyShow1->setText(QStringLiteral("无道具"));
	property1CoordationX->display(0);
	property1CoordationY->display(0);
	propertyShow2->setText(QStringLiteral("无道具"));
	property2CoordationX->display(0);
	property2CoordationY->display(0);
	propertyShow3->setText(QStringLiteral("无道具"));
	property3CoordationX->display(0);
	property3CoordationY->display(0);
	propertyShow4->setText(QStringLiteral("无道具"));
	property4CoordationX->display(0);
	property4CoordationY->display(0);

	/* 陷阱数组初始化 */
	for(int i = 0;i < NUMBER_OF_PROPERTY;++i)
		for(int j = 0;j < 4;++j)
			communication->communicatePitfallMatrix[i][j] = -1;

	resetCPLDSignal();    //CPLD的检验标志初始化

	for(int i = 0;i < PLAYER_NUMBER; ++i)
	{
		player[i].myProperty[0].kindOfProperty = NO_PROPERTY;	/* 选手道具初始化 */
		player[i].myProperty[1].kindOfProperty = NO_PROPERTY;
		communication->getProperty[i] = false;                  /* 选手捡道具数组初始化 */
		for(int j = 0;j < 8; ++j)
			propertyInfluenceTime[i][j] = 0;                    /* 道具影响初始化 */
	}

	reverseTime = 0;
}

/* 判断CPLD是否被修改的函数       */
/* 返回值为1代表队伍1的CPLD被修改 */
/* 返回值为2代表队伍2的CPLD被修改 */
/* 返回值为0代表正常              */
int MainWindow::judgeCPLDChanged()
{
	if(communication->ifChanged[0])
		return 1;
	if(communication->ifChanged[1])
		return 2;
	return 0;
}

/* 初始化CPLD检验标志 */
void MainWindow::resetCPLDSignal()
{
	communication->ifChanged[0] = false;
	communication->ifChanged[1] = false;
}

void MainWindow::updatePropertyInfluence()
{
	for(int i = 0;i < PLAYER_NUMBER; ++i)
	{
		if(player[i].propertyState == BOMB_ATTACT)
		{
			/* 道具作用的替代效果，后影响的功能代替之前的道具影响 */
			propertyInfluenceTime[i][1] = 0;
			propertyInfluenceTime[i][2] = 0;
			propertyInfluenceTime[i][3] = 0;
			propertyInfluenceTime[i][4] = 0;

			propertyInfluenceTime[i][0]++;
			if(propertyInfluenceTime[i][0] == TIME_OF_PROPERTY)
			{
				propertyInfluenceTime[i][0] = 0;
				player[i].propertyState = NO_ATTACT;
			}
		}
		else if(player[i].propertyState == SWIMMING_ATTACT)
		{
			propertyInfluenceTime[i][0] = 0;
			propertyInfluenceTime[i][2] = 0;
			propertyInfluenceTime[i][3] = 0;
			propertyInfluenceTime[i][4] = 0;

			propertyInfluenceTime[i][1]++;
			if(propertyInfluenceTime[i][1] == TIME_OF_PROPERTY)
			{
				propertyInfluenceTime[i][1] = 0;
				player[i].propertyState = NO_ATTACT;
			}
		}
		else if(player[i].propertyState == BOWLING)
		{
			propertyInfluenceTime[i][1] = 0;
			propertyInfluenceTime[i][0] = 0;
			propertyInfluenceTime[i][3] = 0;
			propertyInfluenceTime[i][4] = 0;

			propertyInfluenceTime[i][2]++;
			if(propertyInfluenceTime[i][2] == TIME_OF_PROPERTY)
			{
				propertyInfluenceTime[i][2] = 0;
				player[i].propertyState = NO_ATTACT;
			}
		}
		else if(player[i].propertyState == MAGNET_ATTACT)
		{
			propertyInfluenceTime[i][1] = 0;
			propertyInfluenceTime[i][2] = 0;
			propertyInfluenceTime[i][0] = 0;
			propertyInfluenceTime[i][4] = 0;

			propertyInfluenceTime[i][3]++;
			if(propertyInfluenceTime[i][3] == TIME_OF_PROPERTY)
			{
				propertyInfluenceTime[i][3] = 0;
				player[i].propertyState = NO_ATTACT;
			}
		}
		else if(player[i].propertyState == MAGENET_SPEED)
		{
			propertyInfluenceTime[i][1] = 0;
			propertyInfluenceTime[i][2] = 0;
			propertyInfluenceTime[i][3] = 0;
			propertyInfluenceTime[i][0] = 0;

			propertyInfluenceTime[i][4]++;
			if(propertyInfluenceTime[i][4] == TIME_OF_PROPERTY)
			{
				propertyInfluenceTime[i][4] = 0;
				player[i].propertyState = NO_ATTACT;
			}
		}
		/* 陷阱道具只能作用一次*/
		else if(player[i].propertyState == INTO_MARSH)  //沼泽
		{
			propertyInfluenceTime[i][5]++;
			if(propertyInfluenceTime[i][5] == TIME_OF_PROPERTY)
			{
				propertyInfluenceTime[i][5] = 0;
				player[i].propertyState = NO_ATTACT;
			}
		}
		else if(player[i].propertyState == INTO_WHIRLPOOL) //漩涡
		{
			propertyInfluenceTime[i][6]++;                      
			if(propertyInfluenceTime[i][6] == TIME_OF_PROPERTY)
			{
				propertyInfluenceTime[i][6] = 0;
				player[i].propertyState = NO_ATTACT;
			}
		}
		else if(player[i].propertyState == LANDMINE_ATTACT) //地雷
		{
			propertyInfluenceTime[i][7]++;
			if(propertyInfluenceTime[i][7] == TIME_OF_PROPERTY)
			{
				propertyInfluenceTime[i][7] = 0;
				player[i].propertyState = NO_ATTACT; //陷阱作用结束后隐藏图像
			}
		}
		else;
	}
	showPitfall();  //更新陷阱道具影响
}

/* 显示陷阱图片,陷阱只能作用一次 */
void MainWindow::showPitfall()
{
	if(communication->communicatePitfallMatrix[0][2] != -1/* && pit1->isHidden()*/)
	{
		pit1->setGeometry(communication->communicatePitfallMatrix[0][0]*2.3568-5, 
									  communication->communicatePitfallMatrix[0][1]*2.3568-5,
						              40, 
						              40);
		pit1->show();
	}

	if(communication->communicatePitfallMatrix[1][2] != -1/* && pit2->isHidden()*/)
	{
		pit2->setGeometry(communication->communicatePitfallMatrix[1][0]*2.3568-5, 
						              communication->communicatePitfallMatrix[1][1]*2.3568-5,
						              40, 
						              40);
		pit2->show();
	}

	if(communication->communicatePitfallMatrix[2][2] != -1/* && pit3->isHidden()*/)
	{
		pit3->setGeometry(communication->communicatePitfallMatrix[2][0]*2.3568-5, 
						              communication->communicatePitfallMatrix[2][1]*2.3568-5,
						              40, 
						              40);
		pit3->show();
	}

	if(communication->communicatePitfallMatrix[3][2] != -1/* && pit4->isHidden()*/)
	{
		pit4->setGeometry(communication->communicatePitfallMatrix[3][0]*2.3568-5, 
						              communication->communicatePitfallMatrix[3][1]*2.3568-5,
						              40, 
						              40);
		pit4->show();
	}
}

/* 选手使用道具的信息显示函数 */
void MainWindow::judgePropertyUse(const int &playerNumber)
{
	if(playerNumber == 0 && communication->playerUseProperty[playerNumber] != NO_PROPERTY)
	{
		gameMessageBrowser->append(QStringLiteral("选手 1 使用了  ") 
												  + propertyName[communication->playerUseProperty[playerNumber]]);
		communication->playerUseProperty[playerNumber] = NO_PROPERTY;   //清空道具使用数组
	}
	else if(playerNumber == 1 && communication->playerUseProperty[playerNumber] != NO_PROPERTY)
	{
		gameMessageBrowser->append(QStringLiteral("选手 2 使用了  ") 
												  + propertyName[communication->playerUseProperty[playerNumber]]);
		communication->playerUseProperty[playerNumber] = NO_PROPERTY;   //清空道具使用数组
	}
	else;
	if(communication->ifAttact)
	{
		gameMessageBrowser->append(QStringLiteral("上位机模拟攻击（炸弹）"));
		communication->ifAttact = false;
	}
}

/* 手动攻击函数 */
void MainWindow::onAttack()
{
	communication->attack();
}

/* 定时器触发串口操作 */
void MainWindow::onSendTimer()
{
	serialProtOperation(gameState);  // 更新数据与串口交互
}

/* 判断进入死区的时间 */
void MainWindow::judgeInPit()
{
	for(int i = 0;i < PLAYER_NUMBER;++i)
	{
		if(abs(player[i].Head_x - DEAD1X) < JUDGE_PIT_WEIGHT && abs(player[i].Head_x - DEAD1Y) < JUDGE_PIT_HEIGHT)
			playerInPitTime[i]++;
		else if(abs(player[i].Head_x - DEAD2X) < JUDGE_PIT_WEIGHT && abs(player[i].Head_x - DEAD2Y) < JUDGE_PIT_HEIGHT)
			playerInPitTime[i]++;
		else if(abs(player[i].Head_x - DEAD3X) < JUDGE_PIT_WEIGHT && abs(player[i].Head_x - DEAD3Y) < JUDGE_PIT_HEIGHT)
			playerInPitTime[i]++;
		else if(abs(player[i].Head_x - DEAD4X) < JUDGE_PIT_WEIGHT && abs(player[i].Head_x - DEAD4Y) < JUDGE_PIT_HEIGHT)
			playerInPitTime[i]++;

		if(playerInPitTime[i] == IN_PIT_TIME_LIMIT)
			onPauseButton();
	}
}

/* 监测串口类是否返回暂停信息 */
void MainWindow::judgeOnPause()
{
	if(communication->restart)
		onPauseButton();
}