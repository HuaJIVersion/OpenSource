#pragma once
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <termios.h>
#include<string.h>


#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#include <iostream>
#include <fstream>
#include <string>
#include<string.h>
#include <stdexcept>
#include<exception>
using namespace std;

int _errorCode;
int _serialFd;
#define FSOF  (uint8_t)0x66
#define FEOF  (uint8_t)0x88

typedef  struct
{
	uint8_t  sof;                          //kai_shi_zhen
	uint8_t frame_sequence;             // zhen_xu_hao
	uint16_t emission_speed;                   //fa_she_su_du
	double   pitch;
	double   yaw;
	uint16_t  rail_pos;                        //gui_dao_wei_zhi
	uint8_t  gimbal_mode;                      //dan_fa  lian_fa   bu_fa
	uint32_t  darts;
	uint8_t eof;

}control_frame;

control_frame ctl_f;

typedef  struct
{
	uint8_t  sof;                          //kai_shi_zhen
	uint8_t frame_sequence;             // zhen_xu_hao
	float tank_speed;                           //zhan_che_su_du
	uint16_t remain_hp;                       //xue_liang
	uint8_t  Armor_beated;                           //bei_ji_da_zuang_jia_de_xu_hao
	uint8_t eof;
	bool bulletType;
}feedback_frame;

feedback_frame fdk_f;

