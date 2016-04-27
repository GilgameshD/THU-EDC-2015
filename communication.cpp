/**********************************************************************************
*
*     The Seventeenth Tsinghua University Electronical Design Competition
*                        清华大学第十七届电子设计大赛（上位机程序）
*    
*     Author：DingWenhao
*     Date：  2015.9.22
*     Copyright (c) 2015 DingWenhao. All rights reserved.
*
*     Filename：   communication.cpp
*
***********************************************************************************/

#include "communication.h"
#include <QString>
#include <QtWidgets/QMainWindow>

int Communication::isChecked = 0;  //是否完成检验的数字

Communication::Communication(QMainWindow *parent)
{
	/* 定义串口4为使用串口 */
    com = new Win_QextSerialPort("COM4",QextSerialBase::EventDriven);

	ifChanged[0] = false;
	ifChanged[1] = false;

	/* 储存陷阱道具的坐标和种类 */
	communicatePitfallMatrix = new int*[4];
	for(int i = 0;i < 4;++i)
	{
		communicatePitfallMatrix[i] = new int[4];
		communicatePitfallMatrix[i][2] = -1;
	}

	/* 初始化预备选手状态 */
	for(int i = 0;i < 4;++i)
		playerState[i] = NO_ATTACT;

#ifdef _TESTWINDOW
	myTestWindow = new MyTestWindow;
	myTestWindow->show();
#endif

	/* 数据数组初始化 */
	for(int i = 0;i < 33;++i)
		data[i] = 0x00;

	/* 选手使用道具数组初始化 */
	for(int i = 0;i < PLAYER_NUMBER; ++i)
	{
		playerUseProperty[i] = NO_PROPERTY;
		reverseTime[i] = 0;
		reverseNow[i] = 0;
		outsideNow[i] = false;
		outsideTimer[i] = 0;
	}

	/* 上一个道具作用的初始化 */
	lastTime = 0;
	ifAttact = false;
	restart = false;
}

/* 打开串口的函数 */
bool Communication::OpenPort()
{
    isOpened = com->open(QIODevice::ReadWrite);//双向口
    if(isOpened)                               //检查是否成功打开串口
    {
        com->setBaudRate(BAUD115200);          //波特率 115200
        com->setDataBits(DATA_8);                     //数据位 8
        com->setParity(PAR_NONE);                   //校验位 无
        com->setStopBits(STOP_1);                     //停止位 1
        com->setFlowControl(FLOW_OFF);          //数据流控制 无
        com->setTimeout(500);                           //延时 500ms
        return isOpened;
    }
    else
        return isOpened;
}

/* 获取发送的信息的函数，用于主窗口与串口类通讯，参数为通信变量 */
void Communication::getInformation(Property* const allProperty, 
								   const int &gameState,
								   const bool *p,
								   const int *pp,
								   PlayerInfo* const outsidePlayerInfo)
{
	this->allProperty = allProperty;
	this->gameState = gameState;

	/* 场上道具信息 */
	propertyBool[0] = p[0];
	propertyBool[1] = p[1];
	propertyBool[2] = p[2];
	propertyBool[3] = p[3];

	for(int i = 0;i < 8;++i)
		propertyPosition[i] = pp[i];

	/*  出界信息复制 */
	playerInfo = outsidePlayerInfo;
}

/* 发送数据的函数 */
void Communication::sendData()
{
	if(gameState != 1)//不处于开始状态数据全部置零
	{
		/* 陷阱矩阵置空 */
		for(int i = 0;i < 4;++i)
			communicatePitfallMatrix[i][2] = -1;

		/* 选手使用道具数组初始化 */
		for(int i = 0;i < PLAYER_NUMBER; ++i)
			playerUseProperty[i] = NO_PROPERTY;
	
		/* 上一个道具作用的初始化 */
		lastTime = 0;
	}

	if(isOpened)
	{
			/* 清空数据数组 */
			for(int i = 0;i < 33;++i)	
				data[i] = 0;
			//给出当前场上陷阱的类型（地雷不显示）
			for(int i = 0;i < 4;++i)
			{
				if(communicatePitfallMatrix[i][3] == MARSH)         //陷阱的类型是沼泽
					data[0] |= 0x40;
				else if(communicatePitfallMatrix[i][3] == WHIRLPOOL)//陷阱的类型是漩涡
					data[0] |= 0x80;
				else;
			}

			/* 比赛状态 */
			if(gameState == 1)
				data[0] |= 0x10;    //比赛开始置为01
			else if(gameState == 2)
				data[0] |= 0x20;    //比赛暂停置为10
			else if(gameState == 3)
				data[0] |= 0x30;    //比赛结束置为11
			else;

			/* 道具有效坐标 */
			/* abcd道具从左往右为1分别代表道具存在 */
			for(int i = 0;i < 4;++i)
				if(propertyBool[i])
					++(data[0]);

			/* 选手警告信息 */
			switch(playerInfo[0].state)
			{
				case FALSESTART:
					data[1] |= 0x08;
					break;
				case OUTSIDE:
					data[1] |= 0x02;
					break;
				case REVERSE:
					data[1] |= 0x01;
			}

			switch(playerInfo[1].state)
			{
				case FALSESTART:
					data[1] |= 0x80;
					break;
				case OUTSIDE:
					data[1] |= 0x20;
					break;
				case REVERSE:
					data[1] |= 0x10;
			}

#ifdef FOUR_PLAYER
			switch(playerInfo[2].state)
			{
				case FALSESTART:
					data[2] |= 0x08;
					break;
				case OUTSIDE:
					data[2] |= 0x02;
					break;
				case REVERSE:
					data[2] |= 0x01;
			}

			switch(playerInfo[3].state)
			{
				case FALSESTART:
					data[2] |= 0x80;
					break;
				case OUTSIDE:
					data[2] |= 0x20;
					break;
				case REVERSE:
					data[2] |= 0x10;
			}
#endif
			/* 选手被道具影响的信息 */
			//选手1
			if(playerInfo[0].propertyState == NO_ATTACT)                           //未被道具影响
				data[3] |= 0x00;
			else if(playerInfo[0].propertyState == PROTECTED_ONCE  ||       //处于被保护状态
					playerInfo[0].propertyState == PROTECTED_SOMETIME)
				data[3] |= 0x01;
			else if(playerInfo[0].propertyState == LANDMINE_ATTACT   ||    //处于被攻击状态
					  playerInfo[0].propertyState == BOMB_ATTACT           ||
				  	  playerInfo[0].propertyState == SWIMMING_ATTACT  ||
					  playerInfo[0].propertyState == BOWLING_ATTACT     ||
					  playerInfo[0].propertyState == MAGNET_ATTACT)
				data[3] |= 0x02;
			else                                                                                              //处于陷阱状态
				data[3] |= 0x04;

			//选手2
			if(playerInfo[1].propertyState == NO_ATTACT)                    //未被道具影响
				data[3] |= 0x00;
			else if(playerInfo[1].propertyState == PROTECTED_ONCE  ||       //处于被保护状态
					playerInfo[1].propertyState == PROTECTED_SOMETIME)
				data[3] |= 0x10;
			else if(playerInfo[1].propertyState == LANDMINE_ATTACT ||       //处于被攻击状态
					playerInfo[1].propertyState == BOMB_ATTACT     ||
					playerInfo[1].propertyState == SWIMMING_ATTACT ||
					playerInfo[1].propertyState == BOWLING_ATTACT  ||
					playerInfo[1].propertyState == MAGNET_ATTACT)
				data[3] |= 0x20;
			else                                                            //处于陷阱状态
				data[3] |= 0x40;

#ifdef FOUR_PLAYER
			//选手3
			if(playerInfo[2].propertyState == NO_ATTACT)                    //未被道具影响
				data[4] |= 0x00;
			else if(playerInfo[2].propertyState == PROTECTED_ONCE  ||        //处于被保护状态
					playerInfo[2].propertyState == PROTECTED_SOMETIME)
				data[4] |= 0x01;
			else if(playerInfo[2].propertyState == LANDMINE_ATTACT ||       //处于被攻击状态
					playerInfo[2].propertyState == BOMB_ATTACT     ||
					playerInfo[2].propertyState == SWIMMING_ATTACT ||
					playerInfo[2].propertyState == BOWLING_ATTACT  ||
					playerInfo[2].propertyState == MAGNET_ATTACT)
				data[4] |= 0x02;
			else                                                            //处于陷阱状态
				data[4] |= 0x04;

			//选手4
			if(playerInfo[0].propertyState == NO_ATTACT)                    //未被道具影响
				data[4] |= 0x00;
			else if(playerInfo[0].propertyState == PROTECTED_ONCE  ||        //处于被保护状态
					playerInfo[0].propertyState == PROTECTED_SOMETIME)
				data[4] |= 0x10;
			else if(playerInfo[0].propertyState == LANDMINE_ATTACT ||       //处于被攻击状态
					playerInfo[0].propertyState == BOMB_ATTACT     ||
					playerInfo[0].propertyState == SWIMMING_ATTACT ||
					playerInfo[0].propertyState == BOWLING_ATTACT  ||
					playerInfo[0].propertyState == MAGNET_ATTACT)
				data[4] |= 0x20;
			else                                                            //处于陷阱状态
				data[4] |= 0x40;
#endif

			/* 选手的道具控制信息 */
			if(gameState == 0 || gameState == 2 || gameState == 3)   //比赛未开始强制停止
			{
				data[5] = 0;
				data[6] = 0;
			}
			else
			{
				data[5] = propertyControl(playerInfo[0].propertyState); //选手1
				data[6] = propertyControl(playerInfo[1].propertyState); //选手2
			}

			/* 出界与逆行对选手进行强制停止（优先级高于道具的作用信息）*/
			/* 出界的停止会持续5s */
			for(int i = 0;i < PLAYER_NUMBER;++i)
			{
				/* 判断出界 */
				if(playerInfo[i].state == OUTSIDE && outsideTimer[i] < 70)
				{
					if(i == 0)
						data[5] = 0x00;
					else
						data[6] = 0x00;
					outsideTimer[i]++;
				}
				if(outsideTimer[i] >= 70)
				{
					/* 双方置零 */
					outsideTimer[i] = 0;
					outsideTimer[1 - i] = 0;
					restart = true;  //触发暂停
				}

				/* 判断是否反向 */
				if(reverseNow[i]/* 正处于反向超过3s状态 */ && reverseTime[i] < 50)
				{
					if(i == 0)
						data[5] = 0x00;
					else
						data[6] = 0x00;
					reverseTime[i]++;
				}
				if(reverseTime[i] >= 50)
				{
					reverseNow[i] = false;
					reverseTime[i] = 0;
					reverseTime[1 - i] = 0;
					restart = true;  //触发暂停
				}
			}

			/* 判断反向时间是否超过3s */
			for(int i = 0;i < PLAYER_NUMBER;++i)
			{
				if(playerInfo[i].state == REVERSE && reverseNow[i] == false)
				{
					if(reverseTime[i] < 30)
						++reverseTime[i];
					else if(reverseTime[i] == 30)
					{
						reverseTime[i] = 0;
						reverseNow[i] = true;
					}
					else;
				}
				/* 中间不是逆行则取消标志 */
				if(playerInfo[i].state != REVERSE)
					reverseTime[i] = 0;
			}

#ifdef FOUR_PLAYER
			data[7] = propertyControl(playerInfo[2]); //选手3
			data[8] = propertyControl(playerInfo[3]); //选手4
#endif

			/* coordation of the player */
			data[9] = playerInfo[0].Head_x;
			data[10] = playerInfo[0].Head_y;
			data[11] = playerInfo[1].Head_x;
			data[12] = playerInfo[1].Head_y;

#ifdef FOUR_PLAYER
			data[13] = playerInfo[2].Head_x;
			data[14] = playerInfo[2].Head_y;
			data[15] = playerInfo[3].Head_x;
			data[16] = playerInfo[3].Head_y;
#endif

			/* coordation of the properties */
			/* 未更新出的道具显示的坐标是（0，0）*/
			for(int i = 0;i < 4;++i)
			{
				if(allProperty[i].owner == NONE)
				{
					data[17+i] = propertyPosition[2*i];
					data[18+i] = propertyPosition[2*i+1];
				}
				else
				{
					data[17+i] = 0x00;
					data[18+i] = 0x00;
				}
			}

			/* 选手拥有两个道具的ID */
			for(int i = 0;i <4;++i)
			{
				if(playerInfo[i].ifHaveProperty1)
				decimalToHex1(playerInfo[i].myProperty[0].kindOfProperty, data[25+i]);
				if(playerInfo[i].ifHaveProperty2)
				decimalToHex2(playerInfo[i].myProperty[1].kindOfProperty, data[25+i]);
			}

			/* 当前道具来源 & 当前道具指向 */
			/* 目前只有两个选手 */
			if(currentProperty.currentKind != NO_PROPERTY)
			{
				if(currentProperty.source == 0)
					data[29] |= 0x10;
				else 
					data[29] |= 0x20;
				if(currentProperty.target == 0)
					data[29] |= 0x01;
				else 
					data[29] |= 0x02;

				/* 当前道具种类 */
				//data[30] = 0x00;
				switch(currentProperty.currentKind)
				{
					case SHIELD:
						data[30] |= 0x01;break;
					case REFRACTION:
						data[30] |= 0x02;break;
					case REFRESH:
						data[30] |= 0x04;break;
					case BOMB:
						data[30] |= 0x10;break;
					case SWIMMING:
						data[30] |= 0x20;break;
					case BOWLING:
						data[30] |= 0x40;break;
					case MAGNET:
						data[30] |= 0x80;break;
				}
			}
			/* signal of ending */
			data[31] = 0x0D;
			data[32] = 0x0A;
		
			com->write(data,33);   //can't send unsigned char

#ifdef _TESTWINDOW
			if(myTestWindow->returnflag())
				myTestWindow->mySetTextSend(data);
#endif
		}
}

/* 接受数据的函数 */
void Communication::ReceiveData()
{
	for(int i = 0;i < 3;++i)      //初始化接受数据数组
		receive[i] = 0x00;

	for(int i = 0;i < PLAYER_NUMBER;++i)
		getProperty[i] = false;

	char tmp;
	if(isOpened) 
	{
			while(!(receive[1] == 0x0d && receive[2] == 0x0a) && com->read(&tmp, 1))
			{
				receive[0] = receive[1];
				receive[1] = receive[2];
				receive[2] = tmp;
			}

#ifdef _TESTWINDOW
			if(receive[1] == 0x0d && receive[2] == 0x0a)
				myTestWindow->mySetTextReceive(receive);
#endif

			if (!(receive[1] == 0x0d && receive[2] == 0x0a))
				return;
			receive[2] = 0;

			/* get player number */
			int playerNumber = receive[0] >> 6;
			/* get target number */
			int targetNumber = (receive[0] >> 4) & 0x03;
			/* get the number of the property */
			int count = 0;
			receive[0] = receive[0] & 0x0f;
			for(int i = 0;i < 8;++i)
				changeD[i] = read_bit(receive[0], i);
			for(int i = 0;i < 4;++i)
				count += pow(2,i)*changeD[i];

			/* 匹配枚举型和通信协议对于道具信息的编号 */
			switch(count)
			{
				case 0:count = -1;break;
				case 1:count = 0;break;
				case 2:count = 1;break;
				case 3:count = 2;break;
				case 4:count = -1;break;
				case 5:count = 3;break;
				case 6:count = 4;break;
				case 7:count = 5;break;
				case 8:count = 6;break;
				case 9:count = 7;break;
				case 10:count = 8;break;
				case 11:count = 9;break;
				case 14:count = 10;break;
			}
			
			if(count != GET_PROPERTY)    //不是使用捡道具命令
			{
				if(playerInfo[playerNumber].myProperty[0].kindOfProperty == count && playerInfo[playerNumber].ifHaveProperty1)
				{
					playerInfo[playerNumber].ifHaveProperty1 = false;
					/* 当前道具种类赋值 */
					/* 进入该循环则说明当前道具类型已经更换 */
					/* 只有延时类攻击道具才会起作用 */
					if(count == BOMB || count == SWIMMING || count == BOWLING)
					{
						currentProperty.currentKind = (KindOfProperty)count;
						currentProperty.source = playerNumber;
						currentProperty.target = targetNumber;
						lastTime = 0;
					}
					/* 非延时类道具的释放会清空当前道具类型 */
					else if(count == MAGNET)
					{
						currentProperty.currentKind = (KindOfProperty)count;
						currentProperty.source = playerNumber;
						currentProperty.target = targetNumber;
						lastTime = 10;//缩短显示时间
					}
					else;

					playerUseProperty[playerNumber] = playerInfo[playerNumber].myProperty[0].kindOfProperty;//使用道具1
					propertyEffect(playerInfo[playerNumber].myProperty[0].kindOfProperty,//使用道具1
								   playerNumber, 
								   targetNumber);
				}
				else if(playerInfo[playerNumber].myProperty[1].kindOfProperty == count && playerInfo[playerNumber].ifHaveProperty2)  //使用道具2
				{
					playerInfo[playerNumber].ifHaveProperty2 = false;
					/* 当前道具种类赋值 */
					/* 进入该循环则说明当前道具类型已经更换 */
					/* 只有延时类攻击道具才会起作用 */
					if(count == BOMB || count == SWIMMING || count == BOWLING)
					{
						currentProperty.currentKind = (KindOfProperty)count;
						currentProperty.source = playerNumber;
						currentProperty.target = targetNumber;
						lastTime = 0;
					}
					/* 非延时类道具的释放会清空当前道具类型 */
					else if(count != SHIELD && count != REFRACTION && count != REFRESH)
					{
						lastTime = 0;
						currentProperty.clear();
					}
					else;

					playerUseProperty[playerNumber] = playerInfo[playerNumber].myProperty[0].kindOfProperty;//使用道具数组赋值
					propertyEffect(playerInfo[playerNumber].myProperty[1].kindOfProperty,
								   playerNumber, 
								   targetNumber);
				}
				else;
			}
			else if(count == GET_PROPERTY)  //使用捡道具命令
				propertyEffect(GET_PROPERTY, playerNumber, playerNumber);
			else;
	}
}

/* 关闭串口的函数 */
void Communication::stop()
{
	com->close();
}

/* 根据选手目前的道具类型确定传给通信模块的指令 */
/* 攻击类道具和防御类道具直接标记，陷阱类道具释放储存在陷阱数组中 */
/* 陷阱的触发由每次结算决定 */
int Communication::propertyEffect(const KindOfProperty &pro, const int &playerNumber, const int &targetNumber)
{
	switch(pro)
	{
		/* 捡道具命令直接执行 */
		case GET_PROPERTY:
			getProperty[playerNumber] = true;  //选手的获得道具标志开启
			break;

		/* 保护类道具直接起作用 */
		case SHIELD: 
			playerInfo[playerNumber].propertyState = PROTECTED_SOMETIME;
			playerState[playerNumber] = NO_ATTACT;
			propertyCount[playerNumber] = 0;
			break;
		case REFRACTION:
			playerInfo[playerNumber].propertyState = PROTECTED_ONCE;
			playerState[playerNumber] = NO_ATTACT;
			propertyCount[playerNumber] = 0;
			break;
        case REFRESH:
			playerInfo[playerNumber].propertyState = NO_ATTACT;
			playerState[playerNumber] = NO_ATTACT;
			propertyCount[playerNumber] = 0;
			break;
		case MAGNET:
			//playerInfo[targetNumber].propertyState = MAGNET_ATTACT;  //前车减速
			playerInfo[playerNumber].propertyState = MAGENET_SPEED;  //后车加速
			break;

		/* 陷阱类道具直接生效 */
		case WHIRLPOOL:
			playerInfo[playerNumber].myProperty[0].setPitFall(playerInfo[playerNumber].Head_x,
														   playerInfo[playerNumber].Head_y,
														   playerNumber,
														   communicatePitfallMatrix,
														   WHIRLPOOL);
			break;
		case MARSH:
			playerInfo[playerNumber].myProperty[0].setPitFall(playerInfo[playerNumber].Head_x,
														   playerInfo[playerNumber].Head_y,
														   playerNumber,
														   communicatePitfallMatrix,
														   MARSH);
			break;
		case LANDMINE:
			playerInfo[playerNumber].myProperty[0].setPitFall(playerInfo[playerNumber].Head_x,
														   playerInfo[playerNumber].Head_y,
														   playerNumber,
														   communicatePitfallMatrix,
														   LANDMINE);
			break;

		/* 延时攻击类道具需要一定的延时才能起作用 */
		case BOMB:
			playerState[targetNumber] = BOMB_ATTACT;
			propertyCount[targetNumber] = 0;   //递增数据记录从新开始
			break;
		case SWIMMING:
			playerState[targetNumber] = SWIMMING_ATTACT;
			propertyCount[targetNumber] = 0;
			break;
		case BOWLING:
			playerInfo[targetNumber].propertyState = BOWLING_ATTACT;
			propertyCount[targetNumber] = 0;
			break;
	}
	return 0;
}

/* 读取char*的每一位的函数 */
int Communication::read_bit(const char &c, const int &pos) 
{ 
	char b_mask = 0x01; 
	b_mask = b_mask << pos;     //将1向左移动构造比较位
	if((c & b_mask) == b_mask)  //字符c和b_mask做位运算如果还是等于b_mask,说明该位为1 
		return 1; 
	else 
		return 0;
} 

/* 判断道具控制信息 */
char Communication::propertyControl(const PROPERTY_STATE &player)
{
	char temp = 0x00;   //初始的两轮速度为6级
	switch(player)
	{
		case SWIMMING_ATTACT:
			temp |= 0xf6;break;
		case BOMB_ATTACT:
			temp |= 0x00;break;
		case BOWLING_ATTACT:
			temp |= 0x00;break;
		case MAGENET_SPEED:
			temp |= 0x3f;break;
		case MAGNET_ATTACT:
			temp |= 0x1B;break;
		case INTO_MARSH:
			temp |= 0x1B;break;
		case LANDMINE_ATTACT:
			temp |= 0x00;break;
		case INTO_WHIRLPOOL:
			temp |= 0x1B;break;
		default:
			temp = 0x36;
	}
	return temp;
}

/* 发送检验信息 */
bool Communication::sendCheckMessage()
{ 
	/* 未实现 */
	return true;
}

/* 接受检验数据 */
bool Communication::receiveCheckMessage()
{
	return true;
}

/* 选手一十进制枚举型转化成十六进制通信协议数据 */
void Communication::decimalToHex1(const KindOfProperty& kind, char &data)
{
	switch(kind)
	{
		case SHIELD:
			data |= 0x10;break;
		case REFRACTION:
			data |= 0x20;break;
		case REFRESH:
			data |= 0x30;break;
		case MARSH:
			data |= 0x90;break;
		case LANDMINE:
			data |= 0xa0;break;
		case BOMB:
			data |= 0x50;break;
		case WHIRLPOOL:
			data |= 0xb0;break;
		case SWIMMING:
			data |= 0x60;break;
		case BOWLING:
			data |= 0x70;break;
		case MAGNET:
			data |= 0x80;break;
		default:
			data |= 0x00;
	}
}

/* 选手二十进制枚举型转化成十六进制通信协议数据 */
void Communication::decimalToHex2(const KindOfProperty &kind, char &data)
{
	switch(kind)
	{
		case SHIELD:
			data |= 0x01;break;
		case REFRACTION:
			data |= 0x02;break;
		case REFRESH:
			data |= 0x03;break;
		case MARSH:
			data |= 0x09;break;
		case LANDMINE:
			data |= 0x0a;break;
		case BOMB:
			data |= 0x05;break;
		case WHIRLPOOL:
			data |= 0x0b;break;
		case SWIMMING:
			data |= 0x06;break;
		case BOWLING:
			data |= 0x07;break;
		case MAGNET:
			data |= 0x08;break;
		default:
			data |= 0x00;
	}
}

/* 模拟上位机攻击 */
void Communication::attack()
{
	/* 默认攻击道具是沼泽，且攻击来源是选手自身 */
	/* 该函数跳过了道具作用检验阶段 */
	currentProperty.currentKind = MAGNET;
	currentProperty.source = 0;
	currentProperty.target = 0;
	lastTime = 0;

	playerState[0] = MAGENET_SPEED;
	propertyCount[0] = 0;
	ifAttact = true;
}

/* 更新预备道具作用时间 */
void Communication::updatePropertyTime()
{
	for(int i = 0;i < PLAYER_NUMBER;++i)
	{
		if(playerState[i] != NO_ATTACT)              //预备道具计时数组递增
			++(propertyCount[i]);
		if(propertyCount[i] == TIME_OF_PROPERTY_USE) //预备道具正式生效,改变选手道具状态
		{
			playerInfo[i].propertyState = playerState[i];
			playerState[i] = NO_ATTACT;
			propertyCount[i] = 0;
		}
	}

	/* 如果当前有道具则持续时间递增 */
	if(currentProperty.currentKind != NO_PROPERTY)
		++lastTime;
	/* 如果持续时间达到规定时间清空当前道具类型 */
	if(lastTime == TIME_OF_CURRENT_PROPERTY)
	{
		currentProperty.clear();
		lastTime = 0;
	}
}