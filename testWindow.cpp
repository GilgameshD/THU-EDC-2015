/**********************************************************************************
*
*     The Seventeenth Tsinghua University Electronical Design Competition
*                   �廪��ѧ��ʮ�߽������ƴ�������λ������
*    
*     Author��DingWenhao
*     Date��  2015.9.22
*     Copyright (c) 2015 DingWenhao. All rights reserved.
*
*     Filename��   testWindow.cpp
*
***********************************************************************************/
#include "testWindow.h"

MyTestWindow::MyTestWindow()
{
	setupUi(this);
	flag = true;
	number = 0;
	number2 = 0;
}

void MyTestWindow::mySetTextSend(char *data)
{
	/* ��ʾ20��֮��ˢ�� */
	if(number == 20)
	{
		text1->clear();
		number = 0;
	}
	else
		++number;

	for(int i = 0;i < 33;++i)
	{
		char tmp[20] = {};
		sprintf(tmp, "%02u:", i+1);
		for(int j = 0;j < 8;++j) 
			tmp[3 + 7 - j] = read_bit(data[i], j) + '0'; 
		sprintf(3 + tmp + 8, "(%02x)", (unsigned int)(unsigned char)data[i]);
		text1->append(tmp);
	}
	text1->append("-------------------------");
}

void MyTestWindow::mySetTextReceive(char *data)
{
	/* ��ʾ50��֮��ˢ�� */
	if(number2 == 50)
	{
		text2->clear();
		number2 = 0;
	}
	else
		++number2;

	for(int i = 0;i < 1;++i)
	{
		char tmp[15] = {};
		for(int j = 0;j < 8;j++) 
			tmp[7 - j] = read_bit(data[i], j) + '0'; 
		sprintf(tmp + 8, "(%02x)", (unsigned int)(unsigned char)data[i]);
		text2->append(tmp);
	}
	text2->append("-------------------------");
}

/* ��ȡ char* ��ÿһλ�ĺ��� */
int MyTestWindow::read_bit(const char &c, const int &pos) 
{ 
	char b_mask = 0x01; 
	b_mask = b_mask << pos;     //��1�����ƶ�����Ƚ�λ
	if((c & b_mask) == b_mask)  //�ַ�c��b_mask��λ����������ǵ���b_mask,˵����λΪ1 
		return 1; 
	else 
		return 0;
} 

char* MyTestWindow::returnChar(int *data, int n)
{
	result1 = new char[n];
	for(int i = 0;i < n;++i)
	{
		if(data[i] == 0)
			result1[i] = '0';
		else
			result1[i] = '1';
	}
	return result1;
}

bool MyTestWindow::returnflag()
{
	if(flag)
		return true;
	else
		return false;
}