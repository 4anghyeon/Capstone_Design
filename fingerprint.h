#ifndef _FINGERPRINT_H
#define _FINGERPRINT_H
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <softPwm.h> 
#include <stdlib.h>
#include <time.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <sys/time.h>
#include <unistd.h>        // for  usleep()   sleep()
#include <pthread.h>
#include <sys/io.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <signal.h>

#define  msleep(x)  usleep(x*1000)
//#define TRUE  1
//#define FALSE 0
#define _CRT_SECURE_NO_WARNINGS
#define LED_NUM 12

      
// Basic response message definition
#define ACK_SUCCESS       0x00
#define ACK_FAIL          0x01
#define ACK_FULL	  0x04
#define ACK_NO_USER	  0x05
#define ACK_TIMEOUT       0x08
#define ACK_GO_OUT	  0x0F		// The center of the fingerprint is out of alignment with sensor
//User information definition
#define ACK_ALL_USER       	0x00
#define ACK_GUEST_USER 	  	0x01
#define ACK_NORMAL_USER 	0x02
#define ACK_MASTER_USER    	0x03
#define USER_MAX_CNT	   	1000	// Maximum fingerprint number
// Command definition
#define CMD_HEAD	 0xF5
#define CMD_TAIL	 0xF5
#define CMD_ADD_1        0x01
#define CMD_ADD_2        0x02
#define CMD_ADD_3	 0x03
#define CMD_MATCH	 0x0C
#define CMD_DEL		 0x04
#define CMD_DEL_ALL	 0x05
#define CMD_USER_CNT     0x09
#define CMD_COM_LEV	 0x28
#define CMD_LP_MODE      0x2C
#define CMD_TIMEOUT      0x2E
#define CMD_FINGER_DETECTED   0x14
//servo
#define SERVO_PIN 26
#define PWM_OUTPUT 2
#define PUD_OFF 0
#define PUD_DOWN 1
#define PUD_UP 2
#define INPUT 0
#define OUTPUT 1
//amp
#define SEL1 5
#define SEL2 6
#define ALARMOUTPUT  13
// Pin definition
#define  WAKE_PIN        17
#define  RST_PIN         27
// Pin level definition
#define LOW             0
#define HIGH            1
//LED
#define LEDB 16
#define LEDG 20
#define LEDR 21
extern int LEDT;
extern int ser; 
extern uint8_t  finger_RxBuf[8];
extern uint16_t  finger_RxNum;
extern  pthread_t    id;
extern  pthread_mutex_t  mut; 
extern  int  ret;
extern char userid[5][20];
extern char charac[192];
extern uint8_t Finger_SleepFlag;
extern uint8_t timeout;
extern uint8_t time_flag;
extern uint16_t finger_image[200][200];	
extern int response;	
extern int turn;		
extern int encry;
void  TxByte(uint8_t temp);
uint8_t GetUserCount(void);
uint8_t GetcompareLevel(void);
uint8_t SetcompareLevel(uint8_t temp);	// The default value is 5, can be set to 0-9, the bigger, the stricter
uint8_t AddUser(void);
uint8_t ClearAllUser(void);
uint8_t VerifyUser(void);
uint8_t GetTimeOut(void);	
void Finger_Wait_Until_OK(void);
void Analysis_PC_Command(char *cmd_PC);
void Auto_Verify_Finger(void);
void getname(char * name);
uint16_t getimage(uint16_t Scnt, uint16_t Rcnt);
void timeoutfunc(void);
void bluerecvth(void);
void sensingcut(void);
int bluesend();
void ctrl_c_handler(int signal);   
void close_sockets();   
void *readMsg();   
void *sendMsg();  
void servo();
void flicker();
uint16_t charobtain(void);
uint16_t getdata(uint16_t Scnt, uint16_t Rcnt);
uint16_t compare(uint16_t Scnt, uint16_t Rcnt);
uint16_t imageobtain(void);
#define CMD_GETIMG 0x24 //센서에서 이미지데이터 가져옴
extern uint8_t img[6600];//추가된 사항
uint16_t Optain_and_Send_img(void); //센서에서 이미지 뽑아올 때, protocol No.12
void compare_again();
#endif
