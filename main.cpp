/**********************************************************************************
*
*     The Seventeenth Tsinghua University Electronical Design Competition
*                   清华大学第十七届电子设计大赛（上位机程序）
*    
*     Author：DingWenhao
*     Date：  2015.9.22
*     Copyright (c) 2015 DingWenhao. All rights reserved.
*
*     Filename：   main.cpp
*
***********************************************************************************/

#include "mainwindow.h"
#include "overdialog.h"
#include <QtWidgets/QApplication>

#ifdef SAMPLE_CAR
#pragma message(">-----------------------------   Sample Car has priority!  ----------------------------<")
#endif

#ifdef FOUR_PLAYER
#pragma message(">---------------------------------   Four Player!  ---------------------------------<")
#endif

#ifdef FALSE_START
#pragma message(">---------------------------------   False Start No Check!  --------------------------------<")
#endif

#ifdef _TESTWINDOW
#pragma message(">---------------------------------   Communication Window Test!  -----------------------------<")
#endif

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	MainWindow win;
	win.show();
	return app.exec();
}
