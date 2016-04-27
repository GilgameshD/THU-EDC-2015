/**********************************************************************************
*
*     The Seventeenth Tsinghua University Electronical Design Competition
*                   �廪��ѧ��ʮ�߽������ƴ�������λ������
*    
*     Author��DingWenhao
*     Date��  2015.9.22
*     Copyright (c) 2015 DingWenhao. All rights reserved.
*
*     Filename��   testWindow.h
*     Discription��This file includes a test window to show the data we send and 
*                  receive.
*
***********************************************************************************/

#include "ui_testWindow.h"

class MyTestWindow : public QMainWindow, Ui::TestWindow
{
public:
	MyTestWindow();
	void mySetTextSend(char *data);
	void mySetTextReceive(char *data);
	char *returnChar(int *data, int n);
	bool flag;
	int read_bit(const char &c, const int &pos);
	bool returnflag();
	int receiveFirstData[8];
private:
	char *result1;
	int number;
	int number2;
};