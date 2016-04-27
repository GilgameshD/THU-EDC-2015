/**********************************************************************************
*
*     The Seventeenth Tsinghua University Electronical Design Competition
*                       清华大学第十七届电子设计大赛（上位机程序）
*    
*     Author：DingWenhao
*     Date：  2015.9.22
*     Copyright (c) 2015 DingWenhao. All rights reserved.
*
*     Filename：   communication.h
*     Discription：This file contains the functions of serial port.Based on the 
*                  open source files created by Stefan Sander and Michal Policht,
*                  those functions can send and receive data of the game.Before 
*                  sending and receving,the information should be checked and 
*                  conclued by other useful functions.
*
***********************************************************************************/

#ifndef _COMMUNICATION_H
#define _COMMUNICATION_H

#define _TESTWINDOW      //调试窗口宏定义
#include "qextserialbase.h"
#include "win_qextserialport.h"
#include "property.h"

#ifdef _TESTWINDOW
#include "testWindow.h"  //包含测试文件
#endif

class QMainWindow;

/* 根据选手个数定义 */
#ifdef FOUR_PLAYER
	#define PLAYER_NUMBER 4
#else
	#define PLAYER_NUMBER 2
#endif

#define TIME_OF_PROPERTY_USE  15     //道具从发出到作用的时间差（以100ms调用一次串口函数计算）
#define TIME_OF_CURRENT_PROPERTY 15  //当前道具类型的持续发送时间

/* 储存当前道具的结构体 */
struct CurrentProperty
{
	CurrentProperty():source(0),target(0),currentKind(NO_PROPERTY){}
	void clear(){source = 0;target = 0;currentKind = NO_PROPERTY;}
	int source;
	int target;
	KindOfProperty currentKind;
};

/* 选手状态的枚举 */
enum STATE 
{
	NORMAL,       //正常状态
	FINISH,           //到达终点
	OUTSIDE,        //出界
	REVERSE,         //反向
	WARNING,      //位于缓冲区警告
	BREAK_RULES, //犯规状态
	QUICKEN,       //加速状态
	FALSESTART    //抢跑状态
};      

/* 选手关于道具的状态 */
enum PROPERTY_STATE
{
	NO_ATTACT,                     //未被攻击的正常状态
	PROTECTED_ONCE,          //单次保护状态
	PROTECTED_SOMETIME,  //一段时间保护状态
	INTO_MARSH,                  //陷入沼泽
	INTO_ICE,                         //陷入冰面
	LANDMINE_ATTACT,        //被地雷攻击
	INTO_WHIRLPOOL,          //陷入漩涡
	BOMB_ATTACT,                //被炸弹攻击
	SWIMMING_ATTACT,       //被眩晕
	BOWLING_ATTACT,          //被保龄攻击
	MAGNET_ATTACT,            //被磁铁攻击
	MAGENET_SPEED             //被磁铁加速
};

/* 选手信息结构体 */
struct PlayerInfo
{  
	PlayerInfo():state(NORMAL),
				 propertyState(NO_ATTACT),
				 usedTime(0),
				 Head_x(0),
				 Head_y(0),
				 ifHaveProperty1(false),
				 ifHaveProperty2(false){}
	STATE state;                   /* 选手状态标志 */
	PROPERTY_STATE propertyState;  /* 受道具干扰的状态 */
	int usedTime;                  /* 选手用时 */
    int Head_x;                    /* 头坐标X */
    int Head_y;                    /* 头坐标Y */
	Property myProperty[2];        /* 携带道具 */
	bool ifHaveProperty1;          /* 是否携带道具的标志 */
	bool ifHaveProperty2;
};

class Communication
{
public:
	Communication(QMainWindow *parent = 0);
	void getInformation(Property* const allProperty, 
						const int &gameState,
						const bool *p,
						const int *pp,
						PlayerInfo* const playerInfo);  //获取发送信息
	bool OpenPort();                                         //打开串口
	void ReceiveData();                                     //接受数据
    void sendData();                                         //发送数据
	void stop();                           
	int propertyEffect(const KindOfProperty &myProperty, 
								const int &playerNumber,
								const int &targetNumber);         //产生道具的效果，返回值给通信模块
	int read_bit(const char &c, const int &pos);                 //读取char每一位
	char propertyControl(const PROPERTY_STATE&);         //判断道具控制信息
	bool sendCheckMessage();                      //发送检验数据的函数
	bool receiveCheckMessage();                   //接受检验数据的函数
	void decimalToHex1(const KindOfProperty&, char&);    //道具种类的十进制转十六进制
	void decimalToHex2(const KindOfProperty&, char&);    //道具种类的十进制转十六进制
	void attack();                                //上位机模拟攻击
	void updatePropertyTime();                    //更新道具飞行时间

	int **communicatePitfallMatrix;               //储存陷阱矩阵,第三个参数是陷阱所属方,第四个参数是道具类型
	bool ifChanged[2];                            //检验电源管理模块是否被修改
	bool getProperty[4];                          //捡道具的标志
	PlayerInfo *playerInfo;                       //选手双方的结构体
	KindOfProperty playerUseProperty[4];          //选手使用道具数组（主窗口可调用）
	bool ifAttact;                                //模拟道具的标志
	int reverseTime[4];                           //记录反向警告的时间
	bool reverseNow[4];                           //记录是否反向
	bool restart;

private:
    Win_QextSerialPort *com; //串口操作

    char data[33];           //发送数据数组
	char receive[3];         //接受数据数组

	Property* allProperty;   //储存所有的道具
	int gameState;           //当前比赛状态
	bool propertyBool[4];    //储存道具是否存在的数组
	int propertyPosition[8]; //储存道具位置的数组
	bool isOpened;           //判断串口是否打开
	static int isChecked;    //判断是否完成检验的标志
	int changeD[8];          //储存二进制数组
	PROPERTY_STATE playerState[4];   //储存选手即将受道具影响的状态
	int propertyCount[4];            //记录受道具影响时间，当递增到一定数值时产生道具作用

#ifdef _TESTWINDOW
	MyTestWindow *myTestWindow; //调试窗口
#endif

	CurrentProperty currentProperty; //记录当前作用道具的信息
	CurrentProperty preProperty;     //记录上一个道具作用信息
	int lastTime;                    //上一个道具持续时间
	int outsideTimer[4];             //记录出界的罚时
	bool outsideNow[4];              //记录是否出界
};

#endif // _COMMUNICATION_H