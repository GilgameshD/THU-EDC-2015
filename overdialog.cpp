/**********************************************************************************
*
*     The Seventeenth Tsinghua University Electronical Design Competition
*                   清华大学第十七届电子设计大赛（上位机程序）
*    
*     Author：DingWenhao
*     Date：  2015.9.22
*     Copyright (c) 2015 DingWenhao. All rights reserved.
*
*     Filename：   overdialog.cpp
*
***********************************************************************************/

#include "overdialog.h"
#include <QMouseEvent>
#include <QPainter>
#include <QBitmap>
#include <QString>
#include <QtWidgets/QMainWindow>

OverDialog::OverDialog(QMainWindow *parent)
{
	setupUi(this);

	QPalette palette(this->palette());                                                                    /* 获取主窗口的调色板 */                          
	palette.setBrush(QPalette::Background,QBrush(QPixmap("image/14.jpg")));   /* 将主窗口背景设置为指定图片 */
	this->setPalette(palette);                                                                               /* 调色板开启 */

	//圆角窗口
	QBitmap objBitmap(size());
	QPainter painter(&objBitmap);
	painter.fillRect(rect(),Qt::white);
	painter.setBrush(QColor(0,0,0));
	painter.drawRoundedRect(this->rect(),10,10);
	setMask(objBitmap);
}

/* 鼠标按下的事件函数 */
void OverDialog::mousePressEvent(QMouseEvent *event)  
{  
    if (event->button() == Qt::LeftButton) 
	{  
        m_Drag = true;  
        m_DragPosition = event->globalPos() - this->pos();  
        event->accept();  
    }  
}  
  
/* 鼠标拖动的事件函数 */
void OverDialog::mouseMoveEvent(QMouseEvent *event)  
{  
    if (m_Drag && (event->buttons() && Qt::LeftButton)) 
	{  
        move(event->globalPos() - m_DragPosition);  
        event->accept();  
    }  
}  
  
/* 鼠标释放的事件函数 */
void OverDialog::mouseReleaseEvent(QMouseEvent *)  
{  
    m_Drag = false;  
} 

 /* 将信息初始化 */
void OverDialog::setMessage(QString label,QString name, int timeA, int timeB,int timeC, int timeD)
{
	lcdNumberAS->display(timeA % 60);//A队秒
	lcdNumberAM->display(timeA / 60);//A队分
	lcdNumberBS->display(timeB % 60);//B队秒
	lcdNumberBM->display(timeB / 60);//B队分

#ifdef FOUR_PLAYER
	lcdNumberCS->display(timeC % 60);//A队秒
	lcdNumberCM->display(timeC / 60);//A队分
	lcdNumberDS->display(timeD % 60);//B队秒
	lcdNumberDM->display(timeD / 60);//B队分
#endif

	resultLabel->setText(label);
	/* 双方用时相同则输出平局 */
	if(name == " ")
		winnerLabel->setText(QStringLiteral("双方平局"));
	else
		winnerLabel->setText(name);
}