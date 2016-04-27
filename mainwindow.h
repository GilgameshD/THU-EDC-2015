/**********************************************************************************
*
*     The Seventeenth Tsinghua University Electronical Design Competition
*                       清华大学第十七届电子设计大赛（上位机程序）
*    
*     Author：DingWenhao
*     Date：  2015.9.22
*     Copyright (c) 2015 DingWenhao. All rights reserved.
*
*     Filename：   mainwindow.h
*     Discription：This file contains the main function of the whole progress,
*                  including the movie before,the functions of image group,the
*                  the judgement of the game and all timers used in the progress.
*
***********************************************************************************/

#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H

#include "ui_mainwindow.h"

#include <cv.h>
#include <QMovie>
#include <QTimer>

//#define SAMPLE_CAR      //样车特权
//#define FOUR_PLAYER   //四位选手备用宏
#define FALSE_START     //抢跑调试宏

class Communication;
class PlayerInfo;
class Image;
class Property;
class Sound;

/* 比赛状态枚举 */
enum GAMESTATE
{
	PREPARE,       //准备
	START,         //已经开始
	SUSPEND,       //暂停
	GAMEFINISH     //比赛结束
}; 

class MainWindow : public QMainWindow, Ui::Mainwindow
{
	Q_OBJECT
public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();
	void mousePressEvent(QMouseEvent *event);     /* 鼠标按下事件 */
	void mouseMoveEvent(QMouseEvent *event);    /* 鼠标移动事件 */
	void mouseReleaseEvent(QMouseEvent *);          /* 鼠标释放事件 */
	QString returnWinnerName();                               /* 返回胜利者队名 */
	bool serialProtOperation(const GAMESTATE&);   /* 串口操作函数 */
	bool judgeInRange(const int &coorditionA,
								  const int &coorditionB,
					              const int &current);                  /* 判断是否在区间内 */
	void judgeProtectedTime();                                   /* 判断受保护时间 */ 
	void showOutImage(const int &player, const int &x, const int &y);         /* 显示出界标志 */
	bool judgeComeBack(const int &player);                          /* 判断是否被放回 */
	bool judgeFalseStart(const int &playerNumber);             /* 判断抢跑的函数 */
	void reset();                                                            /* 复位函数 */
	void restart();                                                         /* 暂停后从新开始的函数 */
	int judgeCPLDChanged();                                    /* 判断CPLD是否被修改 */
	void resetCPLDSignal();                                       /* 初始化CPLD检验标志 */
	void updatePropertyInfluence();                        /* 更新道具的影响 */
	void showPitfall();                                                /* 显示陷阱 */
	void judgeInPit();                                                 /* 判断是否在不能退出的区域 */

private slots:
	void creatOverDialog(QString str,
									  int timeA, 
						              int timeB);              /* 创建结束对话框 */
	void onStartButton();                             /* 开始按钮触发的槽函数 */
	void onEndButton();                              /* 结束按钮触发的槽函数 */
	void onPauseButton();                           /* 暂停按钮触发的槽函数 */
	void onGameTimerOut();                       /* 时间计时器溢出槽函数 */
	void VideoUpdate();                               /* 图像更新*/
	void setMusic();                                      /* 开启音乐设置 */
	void onBeforeTimer();                            /* 倒计时定时器触发 */
	void onPropertyTimer();                         /* 随机产生道具计时器触发 */
	void start();                                             /* 比赛开始函数 */
	void judgeOutside();                              /* 判断出界函数 */
	void judgeFinish();                                 /* 判断比赛结束的函数 */
	void judgeCondition(const int &Head_x,
									const int &Head_y, 
									const int &playerNumber);              /* 判断逆行和违规 */
	void judgePitfallProperty();                        /* 判断是否触发陷阱及触发类型 */
	void judge();                                             /* 总体判断函数，100毫秒一次触发 */
	double distance(const int &ax, const int &ay, const int &bx, const int &by);       /* 判断距离 */
	bool judgeGetProperty(const int &playerNumber);            /* 判断是否获得道具 */
	int judgeIntoPitfall(const int &playerNumber);             /* 判断是否进入陷阱,返回值是道具ID */
	void judgePropertyUse(const int &playerNumber);            /* 判断道具使用函数 */
	void onOutsideTimer();                              /* 罚时倒计时触发 */
	void allClose();                                    /* 结束按钮触发的函数 */
	void onAttack();                                    /* 模拟攻击函数 */
	void onSendTimer();
	void judgeOnPause();                                /* 监测是否出发暂停 */

private:
	bool m_Drag;            /* 判断是否是拖动窗口的标志 */
	int gameNumber;         /* 记录比赛时间 */
	int troopTime[4];       /* 队伍用时 */
	int troopTurn[4];       /* 队伍圈数 */
	bool oneTurn[4][3];     /* 两队每圈的完成情况 */
	double beforeTime;         /* 开始前5秒计时 */
	int outsideTime[4];     /* 记录出界时的时间 */   
	int *curPosition;       //当前位置的编号（需要初定义）
	int *nextPosition;      //下一位置的编号
	int *numberOfFalseStart;//抢跑次数
	GAMESTATE gameState;    //比赛是否开始的标志
  
	QPoint m_DragPosition;  /* 拖动点 */
	QTimer *cameraTimer;    /* 摄像头读帧计时器 */
    QTimer *beforeTimer;    /* 开始前五秒倒计时计时器 */
    QTimer *gameTimer;      /* 比赛时间计时器 */
	QTimer *propertyTimer;  /* 道具产生的时间计时器 */
	QTimer *sendtimer;      /* 发送数据的定时器 */
	QTimer *punishTimer1;   /* 罚时计时器1 */
	QTimer *punishTimer2;   /* 罚时计时器2 */
	QTimer *punishTimer3;   /* 罚时计时器1 */
	QTimer *punishTimer4;   /* 罚时计时器2 */

	Communication *communication;  //串口通信对象 
	PlayerInfo *player;            //选手信息结构体数组 

	QImage image;                  //存放每一帧图像
    Image *camera;                 //图像显示类（图像组提供）
    IplImage *frame;               //指向图像组返回的图像
	QImage *testMask;              //矩阵图像

	Sound *gameMusic;              //比赛的背景音乐
	QMovie *beforeGameMovie;       //比赛前倒计时

	int imageMatrix[255][255][1];  //图像矩阵
	int propertyPosition[10][2];   //存放所有道具的位置
	Property *allProperty;         //存放所有道具        
	Property *sampleCarPro;         //样车的道具
	int finishPosition[4][4];      //储存判断选手是否完成比赛的坐标检验点
	int reversePosition[37][3];    //储存判断选手是否反向的坐标检验点
	int troopProtectedTime[4];     //储存选手受保护时间
	bool propertyBool[4];          //储存道具是否存在
	QString propertyName[10];      //储存道具的名称
	int transformPropertyPosition[8]; //用来传递给通信部分的道具坐标
	int propertyInfluenceTime[4][8];  //7种道具的作用延时
	int playerStartPosition[4][2];    //用来记录选手的起始坐标(初始选定)
	int playerMidPosition[4][2];      //用来记录选手的检验点坐标（初始选定）
	int playerInPitTime[4];           //记录选手进入死区的时间
	int lastPosition[4][2];           //上一个位置
	int sampleCarProperty;            //记录样车获取道具的间隔
	int reverseTime;

	/* 起始点和中间点检验 */
	int PLAYER1_START_X;
	int PLAYER1_START_Y;
	int PLAYER1_MID_X;
	int PLAYER1_MID_Y;

	int PLAYER2_START_X;
	int PLAYER2_START_Y;
	int PLAYER2_MID_X;
	int PLAYER2_MID_Y;
};

#endif // _MAINWINDOW_H