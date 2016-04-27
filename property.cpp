/**********************************************************************************
*
*     The Seventeenth Tsinghua University Electronical Design Competition
*                   清华大学第十七届电子设计大赛（上位机程序）
*    
*     Author：DingWenhao
*     Date：  2015.9.22
*     Copyright (c) 2015 DingWenhao. All rights reserved.
*
*     Filename：   property.cpp
*
***********************************************************************************/

#include "property.h"
#include "ui_mainwindow.h"
#include <QTime>

int Property::randNumber = 0;   //随机数递增值
//#define FIRST_CHECK

Property::Property()
{
	/* 随机生成道具类型 */
	/* 用时间作为随机数种子会造成初始化时道具种类全部相同 */
	/* 所以使用增加变量方法改变道具类型 */

#ifndef FIRST_CHECK   //初审不使用随机道具
	QTime time;
	time = QTime::currentTime();
	qsrand(time.msec() + time.second() * 1000 + randNumber);
	int randTime = qrand() % 10;
	kindOfProperty = (KindOfProperty)randTime;
	++randNumber;

#else
	kindOfProperty = MAGNET; //固定道具种类沼泽
#endif

	/* 判断道具种类 */
	if(kindOfProperty == SHIELD                   //保护类
			|| kindOfProperty == REFRACTION)
		typeOfProperty = DEFEND;
	else if(kindOfProperty == MARSH            //陷阱类
			|| kindOfProperty == LANDMINE
			|| kindOfProperty == WHIRLPOOL)
		typeOfProperty = PITFALL;
	else                                                           //攻击类
		typeOfProperty = ATTACK;

	release = false;
	attactPositionX = 0;
	attactPositionY = 0;
	owner = NONE;  //初始化所有者为无主
}

/* 重新生成道具类型 */
void Property::changeKind()
{

#ifndef FIRST_CHECK   //初审不使用随机道具
	QTime time;
	time = QTime::currentTime();
	qsrand(time.msec() + time.second() * 1000 + randNumber);
	int randTime = qrand() % NUMBER_OF_PROPERTY;
	kindOfProperty = (KindOfProperty)randTime;
	++randNumber; //随机数递增

#else
	kindOfProperty = MAGNET;
#endif
}

/* 释放陷阱类道具的函数（由选手的释放函数调用） */
void Property::setPitFall(const int &x, const int &y, const int &playerNumber, int **pitfallMatrix, const KindOfProperty &kindOfProperty)
{
	if(this->typeOfProperty == PITFALL)
	{
		attactPositionX = x;
		attactPositionY = y;
		for(int i = 0;i < 4; ++i)
		{
			if(/*pitfallMatrix[i][0] != 0 && pitfallMatrix[i][1] != 0 && */pitfallMatrix[i][2] == -1)
			{
				pitfallMatrix[i][0] = x;
				pitfallMatrix[i][1] = y;
				pitfallMatrix[i][2] = playerNumber;
				pitfallMatrix[i][3] = kindOfProperty; 
				break;
			}
		}
	}
}