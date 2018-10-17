/*******************************************************
* File:			audio.hpp
* Date:			2016-10-24
* Author:		lllllz
* Description:	实现AIUI语音模块的串口功能的头文件
*******************************************************/

/******************************************************* 
* Function List: 
* sigHandler		信号处理函数
* getId				获取消息ID
* calcCheckCode		计算校验位
* textToSpeech		串口语音合成
* openVoice			打开播报
* closeVoice		关闭播报
* configureWifi		配置wifi设置
* getWifiStatus		获取wifi连接状态
* sendMsg			发送消息
* gzUncompress		解压gzip压缩后的数据
* unpackData		解析数据包
* getTimeStatus		打印当前时间
* threadTTS			语音合成线程函数
* processRecv		分析接收到的串口消息
* uartRec			接收串口消息函数
*******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h> 
#include <pthread.h>
#include <time.h>

#include <string>
#include <vector>
#include <map>

#include <zlib.h>
#include <zmq.h>
#include <glog/logging.h>

#include "../include/uart.h"
#include "../include/cJSON.h"

#define RECV_BUF_LEN 		12
#define SHAKE_BUF_LEN 		12
#define MSG_NORMAL_LEN 		4
#define MSG_EXTRA_LEN 		8
#define PACKET_LEN_BIT 		4
#define SYNC_HEAD 			0xA5
#define SYNC_HEAD_SECOND 	0x01

static int recv_index = 0;
static unsigned char recv_buf[RECV_BUF_LEN];
static int big_buf_len = 0;
static int big_buf_index = 0;
static void* big_buf = NULL;
static UART_HANDLE uart_hd;
static unsigned char ack_buf[RECV_BUF_LEN];
static unsigned char shake_buf[SHAKE_BUF_LEN];

void* context;
void* zmq_sender;
void* zmq_recviver;
int g_quit;
int tts_finished;

int msg_low_id = 0;
int msg_high_id = 0;
int msg_count = 0;

enum MTS_CMD
{
    CMD_HEAD_MOVE_UD,               //HEAD_MOVE_UD(-45)
    CMD_HEAD_MOVE_RL,               //HEAD_MOVE_RL(45)
    CMD_BODY_MOVE_SPD,              //BODY_MOVE_SPD(3)
    CMD_BODY_MOVE_TO_HEAD,          //BODY_MOVE_TO_HEAD
    CMD_VIS_TRACK,
    CMD_IDENTIFY,
    CMD_VIS_UNTRACK,
    CMD_POS_RESET,
    CMD_VOICE,
    CMD_TURN_LEFT,
    CMD_TRUN_RIGHT,
    CMD_DESTINATION,
    CMD_FOLLOW,
    CMD_FOLLOW_CANCEL,
    CMD_IDENTITY,
    CMD_LOOK_AROUND,
    CMD_TURN_BASE,
    CMD_ESCAPE_TROUBLE,
    CMD_PEOPLE_FORWARD,
    CMD_OBJ_FIND,
};

typedef struct T_MTS_COMMAND
{
    char module_name[20];
    char info[32];
    char module_id[8];
    char stimulus_weight[64];
    char data[256];		//需要进行语音合成的数据
    MTS_CMD command;
} t_mts_cmd;

typedef struct tag_zmq_data_head
{
    char module_name[20];
    char size[2];                   // fixed 56
    char data_block_nums[4];        // how many actually data following the data_head
    char M_language_type[4];        // type of M language
    char timestamp[17];
    char info[32];      			//reserved
} zmq_data_head;

typedef struct tag_zmq_data_block
{
    char type[1];
    char module_id[8];
    char M_language_pos[4];
    char stimulus_weight[64];
    char data_size[8];
} zmq_data_block;

std::map<std::string, MTS_CMD> g_map_mts_cmd;

/* ------------------------------------------------------------------------
** Function Definitions
** ------------------------------------------------------------------------ */
void sigHandler(int signo);

int getId(int select);

unsigned char calcCheckCode(unsigned char* buf, int length);

void textToSpeech(const char* text);

void openVoice();

void closeVoice();

void configureWifi(const char* filename);

void getWifiStatus();

void sendMsg(int type, unsigned char low_id, unsigned char high_id);

int gzUncompress(Byte* zdata, uLong nzdata, Byte* data, uLong* ndata);

std::vector<t_mts_cmd> unpackData(void * recvbuf);

void getTimeStatus();

void* threadTTS();

void processRecv(unsigned char* buf, int len);

void uartRec(const void* msg, unsigned int msglen, void* user_data);