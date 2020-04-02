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
	uint8_t  sof;                           //start frame
	uint8_t frame_sequence;                 //frame serial number
	uint16_t emission_speed;                //bullet speed
	double   pitch;
	double   yaw;
	uint16_t  rail_pos;                     //rail position
	uint8_t  gimbal_mode;                      
	uint32_t  darts;							
	uint8_t eof;				//end frame

}control_frame;

control_frame ctl_f;

typedef  struct
{
	uint8_t  sof;                       //start frame	
	uint8_t frame_sequence;             //frame serial number
	float tank_speed;                   //tank speed
	uint16_t remain_hp;                 //health point
	uint8_t  Armor_beated;              //number of beated armor
	uint8_t eof;			    //end frame
	bool bulletType;		    //bullet type
}feedback_frame;

feedback_frame fdk_f;

