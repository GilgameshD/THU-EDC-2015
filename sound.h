/**********************************************************************************
*
*     The Seventeenth Tsinghua University Electronical Design Competition
*                   清华大学第十七届电子设计大赛（上位机程序）
*    
*     Author：DingWenhao
*     Date：  2015.9.22
*     Copyright (c) 2015 DingWenhao. All rights reserved.
*
*     Filename：   sound.h
*     Discription：This file contains the class of music used in game.This file is 
*                  changed on a open source music software.Any form of music support 
*                  can be played during the game.
*
***********************************************************************************/

#ifndef _SOUND_H
#define _SOUND_H

#include "ui_sound.h"          

class QMediaPlayer;
class QMediaPlaylist;

class Sound : public QMainWindow, Ui::BackgroundMusic
{
    Q_OBJECT
public:
    Sound(QWidget *parent = 0);

public slots:                            /* 父窗口需要调用故设为公共槽 */
	void play();                          /* 播放音乐 */
	void pause();                       /* 音乐暂停 */
	void stop();                         /* 音乐停止 */
    void playTo(int, int);            /* 选择列表中的音乐 */
    void importSongs();            /* 从本地导入音乐 */
    void plusSound();                /* 调大音量 */
    void reduceSound();           /* 调小音量 */
	void updateSongList(int i);  /* 更新列表 */

    void setPlaybackMode1();     /* 设置4种播放模式 */
    void setPlaybackMode2();
    void setPlaybackMode3();
    void setPlaybackMode4();
    void setPosition(int);

private:
    int volume;                  /* 音量大小 */
    QMediaPlayer *player;
    QMediaPlaylist *playList;
};

#endif // _SOUND_H
