/**********************************************************************************
*
*     The Seventeenth Tsinghua University Electronical Design Competition
*                   清华大学第十七届电子设计大赛（上位机程序）
*    
*     Author：DingWenhao
*     Date：  2015.9.22
*     Copyright (c) 2015 DingWenhao. All rights reserved.
*
*     Filename：   property.h
*     Discription：This file contains all operations of properties,which include the
*                  the class of the property,the type of property plyaers have,the 
*                  states of players about the property.
*
***********************************************************************************/

#ifndef _PROPERTY_H
#define _PROPERTY_H

/*
 name      signal        type        function
------    --------     --------      -----------------------------------------------------------
 护盾      SHIELD       防御类        一定时间内免疫其他道具
 折光      REFRACTION   防御类        免疫一次道具的效果
 净化      REFRESH      防御类        清除自身负面道具
 沼泽      MARSH        陷阱类        形成固定大小的永久存在的减速区
 地雷      LANDMINE     陷阱类        距离地雷一定位置的小车将被强制停止一段时间（坐标不可见，一次）
 漩涡      WHIRLPOOL    陷阱类        靠近漩涡坐标的轮子将被减速（一次）
 炸弹      BOMB         攻击类        需要指定方向，速度慢，被攻击车停止一段时间
 眩晕      SWIMMING     攻击类        需要指定方向，速度快，被攻击车左右轮控制颠倒
 保龄      BOWLING      攻击类        不需要指定方向，在跑道正向碰撞反弹，被攻击车停止一段时间
 磁铁      MAGNET       攻击类        将会在两车之间模拟吸引力
 捡道具    GET_PROPERTY 特殊类        使用此道具可以捡到一定范围内的道具
-------------------------------------------------------------------------------------------------
*/

#define NUMBER_OF_PROPERTY   4    //道具数量上限

//道具的具体种类
enum KindOfProperty 
{
	SHIELD, 
	REFRACTION, 
	REFRESH,
	BOMB, 
	SWIMMING,
	BOWLING, 
	MAGNET,
	MARSH, 
	LANDMINE, 
	WHIRLPOOL,
	GET_PROPERTY,
	NO_PROPERTY
};

//道具的所属类型
enum TypeOfProperty 
{
	ATTACK,
	PITFALL,
	DEFEND
};

/* 道具归属者的枚举 */
enum PROPERTY_OWNER
{
	TROOPA = 0,
	TROOPB = 1,
	NONE
};

class Property
{
public:
	Property();
	void changeKind();                     //改变道具名称（用于结束比赛后的重置）
	void decideType();                     //决定道具的种类
	void setPitFall(const int &x, 
						  const int &y,
						  const int &playerNumber,
					      int **pitfallMatrix,
					      const KindOfProperty&);           //放置陷阱类道具的函数
	void releaseBowling();                    //释放保龄道具
	KindOfProperty kindOfProperty;   //道具名称
	TypeOfProperty typeOfProperty;  //道具种类
	int positionX;                                //道具位置X
	int positionY;                                //道具位置Y
	PROPERTY_OWNER owner;          //道具所有人
	bool release;                                //道具释放标志
	int attactPositionX;                      //陷阱类道具释放位置X
	int attactPositionY;                      //陷阱类道具释放位置Y
	static int randNumber;                 //随机数递增值
};

#endif //_PROPERTY_H



