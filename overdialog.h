/**********************************************************************************
*
*     The Seventeenth Tsinghua University Electronical Design Competition
*                       清华大学第十七届电子设计大赛（上位机程序）
*    
*     Author：DingWenhao
*     Date：  2015.9.22
*     Copyright (c) 2015 DingWenhao. All rights reserved.
*
*     Filename：   overdialog.h
*     Discription：This file contains the class of the game over dialog.This dialog 
*                  should show the time all players use and the name of the troop 
*                  which win the game.
*
***********************************************************************************/

#ifndef _OVERDIALOG_H
#define _OVERDIALOG_H

#include "ui_overdialog.h"
#include <QDialog>

class QMainWindow;

class OverDialog : public QDialog, Ui::Overdialog
{
	Q_OBJECT
public:
	OverDialog(QMainWindow *parent = 0);
	void mousePressEvent(QMouseEvent *event);   /* 鼠标按下事件 */
	void mouseMoveEvent(QMouseEvent *event);    /* 鼠标移动事件 */
	void mouseReleaseEvent(QMouseEvent *);      /* 鼠标释放事件 */
	void setMessage(QString, QString, int, int, int timeC = 0, int timeD = 0);         /* 将信息初始化 */
private:
	bool m_Drag;            /* 判断是否是拖动窗口的标志 */
	QPoint m_DragPosition;  /* 拖动点 */
};

#endif //_OVERDIALOG_H