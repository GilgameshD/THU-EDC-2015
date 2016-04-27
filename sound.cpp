/**********************************************************************************
*
*     The Seventeenth Tsinghua University Electronical Design Competition
*                   清华大学第十七届电子设计大赛（上位机程序）
*    
*     Author：DingWenhao
*     Date：  2015.9.22
*     Copyright (c) 2015 DingWenhao. All rights reserved.
*
*     Filename：   sound.cpp
*
***********************************************************************************/

#include <QtWidgets>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QVideoWidget>
#include <QDir>
#include <QString>
#include "sound.h"

#define MAX_VOLUME 100
#define MIN_VOLUME 0
#define INIT_VOLUME 80

Sound::Sound(QWidget *parent) : QMainWindow(parent)
{
    setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);                                                  /* 设置无边框 */
	QPalette palette(this->palette());                                                                    /* 获取主窗口的调色板 */                            
	palette.setBrush(QPalette::Background,QBrush(QPixmap("image/55.jpg")));   /* 将主窗口背景设置为指定图片 */
	this->setPalette(palette);                                                                                /* 调色板开启 */

	//圆角窗口
	QBitmap objBitmap(size());
	QPainter painter(&objBitmap);
	painter.fillRect(rect(),Qt::white);
	painter.setBrush(QColor(0,0,0));
	painter.drawRoundedRect(this->rect(),10,10);
	setMask(objBitmap);

	//背景音乐的初始化
    playList = new QMediaPlaylist;
    playList->setPlaybackMode(QMediaPlaylist::Loop);
    player = new QMediaPlayer;
    player->setPlaylist(playList);
	volume = INIT_VOLUME;//音量初始值
    player->setVolume(volume);

	QMediaPlayer *countMusic = new QMediaPlayer;
    QMediaPlaylist *countMusicPlayList = new QMediaPlaylist;

	connect(tableWidget, SIGNAL(cellClicked(int,int)), this, SLOT(playTo(int, int)));     //指定歌曲
	connect(playList, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSongList(int))); //更新列表
	connect(volumPlusButton, SIGNAL(clicked()), this, SLOT(plusSound()));
	connect(volumReduceButton, SIGNAL(clicked()), this, SLOT(reduceSound()));
    connect(loopButton, SIGNAL(clicked()), this, SLOT(setPlaybackMode1()));
    connect(randomButton, SIGNAL(clicked()), this, SLOT(setPlaybackMode2()));
    connect(itemLoopButton, SIGNAL(clicked()), this, SLOT(setPlaybackMode3()));
    connect(sequentialButton, SIGNAL(clicked()), this, SLOT(setPlaybackMode4()));
	connect(importButton, SIGNAL(clicked()), this, SLOT(importSongs()));
	connect(playButton, SIGNAL(clicked()), this, SLOT(play()));
	connect(stopButton, SIGNAL(clicked()), this, SLOT(stop()));
}

/* 指定播放歌曲的函数 */
void Sound::playTo(int i, int /* j */)
{
    playList->setCurrentIndex(i);
    //player->play(); //不直接播放
}

/* 从本地导入歌曲的函数 */
void Sound::importSongs()
{
    QString initialName = QDir::homePath();
    QStringList pathList = QFileDialog::getOpenFileNames(this, tr("Choose File"), initialName, tr("*.mp3"));
    for(int i = 0; i < pathList.size(); ++i) 
	{
        QString path = QDir::toNativeSeparators(pathList.at(i));
        if(!path.isEmpty()) 
		{
            playList->addMedia(QUrl::fromLocalFile(path));
            QString fileName = path.split("\\").last();
            int rownum = tableWidget->rowCount();
            tableWidget->insertRow(rownum);
            tableWidget->setItem(rownum, 0, new QTableWidgetItem(fileName.split(".").front()));
            tableWidget->setItem(rownum, 1, new QTableWidgetItem(fileName.split(".").last()));
            tableWidget->setItem(rownum, 2, new QTableWidgetItem(path));
        }
    }
}

/* 播放歌曲的函数 */
void Sound::play()
{
    player->play();
}

/* 暂停播放的函数 */
void Sound::pause()
{
    player->pause();
}

/* 停止播放的函数 */
void Sound::stop()
{
	player->stop();
}

/* 更新列表的函数 */
void Sound::updateSongList(int i)
{
    tableWidget->selectRow(i);
}

/* 调大音量的函数 */
void Sound::plusSound()
{
    volume += 10;
    if(volume >= MAX_VOLUME) 
	{
        volume = MAX_VOLUME;
        volumPlusButton->setEnabled(false);
    }
    player->setVolume(volume);

    if(!volumReduceButton->isEnabled())
        volumReduceButton->setEnabled(true);
}

/* 调小音量 */
void Sound::reduceSound()
{
    volume -= 10;
    if(volume <= MIN_VOLUME)
	{
        volume = MIN_VOLUME;
        volumReduceButton->setEnabled(false);
    }
    player->setVolume(volume);

    if(!volumPlusButton->isEnabled())
        volumPlusButton->setEnabled(true);
}

/* 设置位置 */
void Sound::setPosition(int position)
{
    player->setPosition(position);
}

/* 全部循环播放 */
void Sound::setPlaybackMode1()
{
    playList->setPlaybackMode(QMediaPlaylist::Loop);
}

/* 随机播放 */
void Sound::setPlaybackMode2()
{
    playList->setPlaybackMode(QMediaPlaylist::Random);
}

/* 单曲循环播放 */
void Sound::setPlaybackMode3()
{
    playList->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
}

/* 顺序播放 */
void Sound::setPlaybackMode4()
{
    playList->setPlaybackMode(QMediaPlaylist::Sequential);
}
