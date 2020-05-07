#include <stdio.h>
#include <stdlib.h>
#include "fingerprint.h"
uint8_t time_flag;
uint8_t Finger_SleepFlag;
uint8_t timeout;
int turn;
int response;
void *thread_Auto_Verify_Finger(void* no_parameter)
{    (void)no_parameter;
    while(1)
    {   
            if(timeout < 15){
                digitalWrite(LED_NUM,1);
				msleep(1);
				digitalWrite(LED_NUM,0);
                msleep(1);
            }
            Auto_Verify_Finger();
}}void *thread_timeoutfunc(void* no_parameter){
    (void)no_parameter;
       while(1)
    {          
    pinMode(RST_PIN, OUTPUT);
    pinMode(LEDB,OUTPUT);
    pinMode(LEDG,OUTPUT);
    pinMode(LEDR,OUTPUT);
    
    if(Finger_SleepFlag == 0){
        if (time_flag == 0){
            if(turn==0){

            turn=1;

        }
            
            for(int i=0; i<25 ;i++){
                
            if(time_flag == 1){
                timeout = 26;
            }
            
            sleep(1);
            timeout = timeout +1;
            
            
            if (timeout == 15)
            {
            digitalWrite(LEDB, HIGH);
            digitalWrite(LEDG, HIGH);
            digitalWrite(LEDR, HIGH);
            printf("sleepmode\n");
            digitalWrite(RST_PIN,LOW);
            response = 3;
            turn = 0;
            sendMsg();
            digitalWrite(LED_NUM,0);
            msleep(250);
            Finger_SleepFlag = 1;
            time_flag = 1;
            timeout = 0;;
        }
	}
}
}
}
}
void *thread_bluerecvth(void* no_parameter){
        (void)no_parameter;
    while(1)
    {
    bluesend();
}}int main(void)
{    
    char  cmd_buf[5];
    char  name_buf[20];
    char  ch;
    
    if(wiringPiSetupGpio() < 0) {    // using Broadcom GPIO pin mapping
        printf("Error: can not use Broadcom GPIO pin mapping  ! \n");
        return -1;
    }    
    pinMode(LEDB,OUTPUT);
    pinMode(LEDG,OUTPUT);
    pinMode(LEDR,OUTPUT);
    digitalWrite(LEDB, HIGH);
    digitalWrite(LEDG, LOW);
    digitalWrite(LEDR, HIGH);
  
   // digitalWrite(WAR_NUM, HIGH);
//    pinMode(ALARMOUTPUT, OUTPUT);
//	digitalWrite(ALARMOUTPUT, LOW);
   //digitalWrite(LED_NUM,0);
   // pullUpDnControl(18, LOW);
           
    pthread_mutex_init(&mut,NULL);
    ret = (pthread_create(&id, NULL, thread_Auto_Verify_Finger, NULL));
    while(ret != 0)
    {
        ret = (pthread_create(&id, NULL, thread_Auto_Verify_Finger, NULL));
    }
    
    pthread_mutex_init(&mut,NULL);
    ret = (pthread_create(&id, NULL, thread_timeoutfunc, NULL));
    while(ret != 0)
    {
        ret = (pthread_create(&id, NULL, thread_timeoutfunc, NULL));
    }
        
    pthread_mutex_init(&mut,NULL);
    ret = (pthread_create(&id, NULL, thread_bluerecvth, NULL));
    while(ret != 0)
    {
        ret = (pthread_create(&id, NULL, thread_bluerecvth, NULL));
    }
    
   
    Finger_Wait_Until_OK();
    time_flag =0;
        
    while (1) 
    {                  
        time_flag = 0;
        printf("input command\n");        
        fgets(cmd_buf,sizeof(cmd_buf),stdin);        
        Analysis_PC_Command(cmd_buf);
        
        if( strchr(cmd_buf,'\n') == NULL )
        {
            while((ch=getchar())!='\n'&&ch!=EOF);  // clean stdin buffer before next fgets()    
        }        
        memset(cmd_buf, 0, sizeof(cmd_buf));
    }
        
    
    //serialClose(ser);
    return 0;
}
