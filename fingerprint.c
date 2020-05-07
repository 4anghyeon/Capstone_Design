#include "fingerprint.h"
#include <string.h>
#include <sys/io.h>
#include <time.h>
#include <stdlib.h>
#define _CRT_SECURE_NO_WARNINGS
			//uint8_t  finger_RxBuf[6611];
uint8_t finger_TxBuf[8];
uint16_t finger_image[200][200];	
char userid[5][20];
int ser; 
uint8_t  finger_RxBuf[8];
uint16_t  finger_RxNum;
uint8_t     Finger_SleepFlag;
uint8_t time_flag;
uint8_t number;
pthread_t    id;
pthread_mutex_t    mut; 
uint16_t finger_charRxBuf[7204];
uint8_t timeout;
int  ret;
char name[20];
char list[100][260];
int f=0,k=1;
int response=99;
int failnumber=0;
int ss,clients ;   
 char sendtoapp;
char receiving;
int encry = 99;
int LEDT;
/***************************************************************************
* @brief      Send a byte of data to the serial port
* @param      temp : Data to send
****************************************************************************/
void  TxByte(uint8_t temp)
{	
	return   serialPutchar(ser, temp);  
}/***************************************************************************
* @brief      send a command, and wait for the response of module
* @param      Scnt: The number of bytes to send
	      Rcnt: expect the number of bytes response module
	      Delay_ms: wait timeout
* @return     ACK_SUCCESS: success
  	      other: see the macro definition
****************************************************************************/
uint8_t TxAndRxCmd(uint8_t Scnt, uint8_t Rcnt, uint16_t Delay_ms)
{	uint8_t  i, j, CheckSum;
    uint32_t before_time;        
    uint32_t after_time;
    uint8_t   overflow_Flag = 0;
    struct  timeval tv;
    uint16_t  Delay_s = 0;
    
       
     serialFlush(ser);      // clear serial RX buffer and wait until serial TX finished     
    
	TxByte(CMD_HEAD);			 
	CheckSum = 0;
	for (i = 0; i < Scnt; i++)
	{
		TxByte(finger_TxBuf[i]);		 
		CheckSum ^= finger_TxBuf[i];
	}	
	TxByte(CheckSum);
	TxByte(CMD_TAIL);  
               
    finger_RxNum = 0;  // clear  finger_RxNum  before next receive
    memset(finger_RxBuf,0,sizeof(finger_RxBuf));   ////////
	       
    if(Delay_ms >= 1000)
    {
        Delay_s = Delay_ms / 1000;
        Delay_ms = Delay_ms % 1000;
    }
           
    // Receive until time out
    gettimeofday(&tv,NULL);
    before_time = tv.tv_usec/1000;	 
    do
    {
        overflow_Flag = 0;
        
        if(serialDataAvail(ser))
        {
            finger_RxBuf[finger_RxNum++] = serialGetchar(ser);         
        }
                  
        gettimeofday(&tv,NULL);
        after_time = tv.tv_usec/1000;	
        if(before_time > after_time)   //if overflow (go back to zero)
        {
                gettimeofday(&tv,NULL);
                before_time = tv.tv_usec/1000;	// get time_before again
                overflow_Flag = 1;
        }
              
    } while (((finger_RxNum < Rcnt) && (after_time - before_time < Delay_ms)) || (overflow_Flag == 1));
    
    if(finger_RxNum < Rcnt && Delay_s > 0)
    {
        gettimeofday(&tv,NULL);
        before_time = tv.tv_sec;	 
        do
        {
            overflow_Flag = 0;
            
            if(serialDataAvail(ser))
            {
                finger_RxBuf[finger_RxNum++] = serialGetchar(ser);         
            }
                      
            gettimeofday(&tv,NULL);
            after_time = tv.tv_sec;	
            if(before_time > after_time)   //if overflow (go back to zero)
            {
                    gettimeofday(&tv,NULL);
                    before_time = tv.tv_sec;    // get time_before again
                    overflow_Flag = 1;
            }
                      
        } while (((finger_RxNum < Rcnt) && (after_time - before_time < Delay_s)) || (overflow_Flag == 1));      
    }
       
	if (finger_RxNum!= Rcnt)	return ACK_TIMEOUT;
	if (finger_RxBuf[0] != CMD_HEAD) 	   return ACK_FAIL;
	if (finger_RxBuf[Rcnt - 1] != CMD_TAIL)    return ACK_FAIL;
	if (finger_RxBuf[1] != (finger_TxBuf[0]))  return ACK_FAIL;
	CheckSum = 0;
	for (j = 1; j < finger_RxNum - 1; j++) CheckSum ^= finger_RxBuf[j];
	if (CheckSum != 0)   return ACK_FAIL; 	  
	return  ACK_SUCCESS;
}	 
/***************************************************************************
* @brief      Query the number of existing fingerprints
* @return     0xFF: error
  	        other: success, the value is the number of existing fingerprints
****************************************************************************/
uint8_t GetUserCount(void)
{	uint8_t m;
		time_flag = 1;
	Finger_SleepFlag=0;
	finger_TxBuf[0] = CMD_USER_CNT;
	finger_TxBuf[1] = 0;
	finger_TxBuf[2] = 0;
	finger_TxBuf[3] = 0;
	finger_TxBuf[4] = 0;	
		m = TxAndRxCmd(5, 8, 100);
			
	if (m == ACK_SUCCESS && finger_RxBuf[4] == ACK_SUCCESS)
	{
	    return finger_RxBuf[3];
	}
	else
	{
	 	return 0xFF;
	}
	time_flag = 0;
}/***************************************************************************
* @brief      Get Compare Level
* @return     0xFF: error
  	      other: success, the value is compare level
****************************************************************************/
uint8_t GetcompareLevel(void)
{	uint8_t m;
	time_flag = 1;
	Finger_SleepFlag=0;
	finger_TxBuf[0] = CMD_COM_LEV;
	finger_TxBuf[1] = 0;
	finger_TxBuf[2] = 0;
	finger_TxBuf[3] = 1;
	finger_TxBuf[4] = 0;	
		m = TxAndRxCmd(5, 8, 100);
		if (m == ACK_SUCCESS && finger_RxBuf[4] == ACK_SUCCESS)
	{
	    return finger_RxBuf[3];
	    time_flag = 1;
	}
	else
	{
	 	return 0xFF;
		time_flag = 0;
	}
}/***************************************************************************
* @brief      Set Compare Level
* @param      temp: Compare Level,the default value is 5, can be set to 0-9, the bigger, the stricter
* @return     0xFF: error
  	      other: success, the value is compare level
****************************************************************************/
uint8_t SetcompareLevel(uint8_t temp)
{	uint8_t m;
	time_flag = 1;
	Finger_SleepFlag=0;
		finger_TxBuf[0] = CMD_COM_LEV;
	finger_TxBuf[1] = 0;
	finger_TxBuf[2] = temp;
	finger_TxBuf[3] = 0;
	finger_TxBuf[4] = 0;	
		m = TxAndRxCmd(5, 8, 100);
		
	if (m == ACK_SUCCESS && finger_RxBuf[4] == ACK_SUCCESS)
	{
	    return finger_RxBuf[3];
	    time_flag = 1;
	}
	else
	{
	 	return 0xFF;
		time_flag = 0;
	}
}/***************************************************************************
* @brief      Get the time that fingerprint collection wait timeout 
* @return     0xFF: error
  	      other: success, the value is the time that fingerprint collection wait timeout 
****************************************************************************/
uint8_t GetTimeOut(void)
{	uint8_t m;
	time_flag = 1;
	Finger_SleepFlag =0;
		finger_TxBuf[0] = CMD_TIMEOUT;
	finger_TxBuf[1] = 0;
	finger_TxBuf[2] = 0;
	finger_TxBuf[3] = 1;
	finger_TxBuf[4] = 0;	
		m = TxAndRxCmd(5, 8, 100);
		
	if (m == ACK_SUCCESS && finger_RxBuf[4] == ACK_SUCCESS)
	{
	    return finger_RxBuf[3];
	    time_flag = 1;
	}
	else
	{
	 	return 0xFF;
		time_flag = 0;
	}
}/***************************************************************************
* @brief      Register fingerprint
* @return     ACK_SUCCESS: success
  	      other: see the macro definition
****************************************************************************/
void getname(char * name){
		uint8_t m;
		time_flag = 1;
		Finger_SleepFlag = 0;
		
		m = GetUserCount();
		number = m ;
		for(int i=0; i<10 ; i++){
				userid[number][i] = name[i];
			}
		time_flag = 0;
		}
uint8_t AddUser(void)
{	//char  name_buf[20];
	//char  ch;
	uint8_t m;
		time_flag = 1;
	Finger_SleepFlag = 0;
       // memset(name_buf, 0, sizeof(name_buf));
	//printf("what's your name?\n");
        //fgets(name_buf,sizeof(name_buf),stdin);
	//string[strlen(string)-1] = '\0';
        //getname(name_buf);
	printf("please put your finger on printer\n");
	response=0;
	sendMsg();
	sleep(2);
	m = GetUserCount();
	number = m;
	if (m >= USER_MAX_CNT)
		return ACK_FULL;
	pinMode(LEDB,OUTPUT);
	digitalWrite(LEDB,0);
	finger_TxBuf[0] = CMD_ADD_1;
	finger_TxBuf[1] = 0;
	finger_TxBuf[2] = m +1;
	finger_TxBuf[3] = 3;
	finger_TxBuf[4] = 0;
	m = TxAndRxCmd(5, 8, 5000);	
	if (m == ACK_SUCCESS && finger_RxBuf[4] == ACK_SUCCESS)
	{
		finger_TxBuf[0] = CMD_ADD_3;
		m = TxAndRxCmd(5, 8, 5000);
		if (m == ACK_SUCCESS && finger_RxBuf[4] == ACK_SUCCESS)
		{
			
			//printf("user name: ");
			//for(int i=0; i<20; i++){
			//	if(userid[number][i] == NULL){
			//		break;
			//	}
			//	printf("%c",userid[number][i]);
			//}
			//	printf("%d\n",number);
				charobtain();
				printf("added succesfully\n");
				//sendtoapp = 'added';
				response = 1;
				sendMsg();
				twinkle();
				digitalWrite(LEDB,1);
				
			time_flag = 0;
			Finger_SleepFlag = 0;
			timeout = 0;
			return ACK_SUCCESS;
			
		}
		else
		time_flag = 0;
		Finger_SleepFlag = 0;
		timeout =0;
		
		return ACK_FAIL;
	}
	else
	time_flag = 0;
	Finger_SleepFlag = 0;
	timeout=0;
	response = 1;
	sendMsg();
			return ACK_FAIL;
	}/***************************************************************************
* @brief      Clear fingerprints
* @return     ACK_SUCCESS:  success
  	      ACK_FAIL:     error
****************************************************************************/
uint8_t  ClearAllUser(void)
{ 	uint8_t m;
	char filename[20];
	int j ;
	time_flag = 1;
	Finger_SleepFlag = 0;
	finger_TxBuf[0] = CMD_DEL_ALL;
	finger_TxBuf[1] = 0;
	finger_TxBuf[2] = 0;
	finger_TxBuf[3] = 0;
	finger_TxBuf[4] = 0;
		m = TxAndRxCmd(5, 8, 500);
	for (j=1 ; j<5 ; j++){
	sprintf(filename,"list//testch%d.txt",j);
	remove(filename);
}		if (m == ACK_SUCCESS && finger_RxBuf[4] == ACK_SUCCESS)
	{	    
		return ACK_SUCCESS;
		time_flag = 0;
	}
	else
	{
		return ACK_FAIL;
		time_flag = 0;
	}
}/***************************************************************************
* @brief      Check if user ID is between 1 and 3
* @return     TRUE
  	          FALSE
****************************************************************************/
uint8_t IsMasterUser(uint8_t UserID)
{    if ((UserID == 1) || (UserID == 2) || (UserID == 3)) return TRUE;
			else  return FALSE;
}	 
/***************************************************************************
* @brief      Fingerprint matching
* @return     ACK_SUCCESS: success
  	      other: see the macro definition
****************************************************************************/
uint8_t VerifyUser(void)
{	uint8_t m;
	time_flag = 1;
	Finger_SleepFlag = 0;
		finger_TxBuf[0] = CMD_MATCH;
	finger_TxBuf[1] = 0;
	finger_TxBuf[2] = 0;
	finger_TxBuf[3] = 0;
	finger_TxBuf[4] = 0;
		m = TxAndRxCmd(5, 8, 5000);
		if ((m == ACK_SUCCESS) && (IsMasterUser(finger_RxBuf[4]) == TRUE))
	{	
		 number = finger_RxBuf[3];
		printf("open\n");
		time_flag = 0;
		servo();
		 return ACK_SUCCESS;
	}
	else if(finger_RxBuf[4] == ACK_NO_USER)
	{
		time_flag = 0;
		return ACK_NO_USER;
	}
	else if(finger_RxBuf[4] == ACK_TIMEOUT)
	{
		time_flag = 0;
		return ACK_TIMEOUT;
	}
	else
	{
		time_flag = 0;
		return ACK_GO_OUT;   // The center of the fingerprint is out of alignment with sensor
	}
}/***************************************************************************
* @brief      Wait until the fingerprint module works properly
****************************************************************************/
void Finger_Wait_Until_OK(void)
{	
    while((ser = serialOpen("/dev/ttyS0",19200)) < 0); 
     
    pinMode(WAKE_PIN, INPUT);
    pinMode(RST_PIN, OUTPUT);
    pullUpDnControl(WAKE_PIN, PUD_DOWN);
         
    digitalWrite(RST_PIN , LOW);
	msleep(250); 
    digitalWrite(RST_PIN , HIGH);
	msleep(250);  // Wait for module to start
	 // ERROR: Please ensure that the module power supply is 3.3V or 5V, 
	 // the serial line is correct, the baud rate defaults to 19200,
	 // and finally the power is switched off, and then power on again !
	while(SetcompareLevel(5) != 5)
	{
		sleep(1);    // sleep for 1s
		printf("WARNING!!\n"); 
	}
   
	//printf("*************** WaveShare Capacitive Fingerprint Reader Test ***************\n");
	//printf("Compare Level:  5    (can be set to 0-9, the bigger, the stricter)\n"); 
	//printf("Number of fingerprints already available:  %d\n",GetUserCount());
	//printf(" Use the serial port to send the commands to operate the module:\n"); 
	//printf("  CMD1 : Query the number of existing fingerprints\n"); 
	//printf("  CMD2 : Add fingerprint  (Each entry needs to be read two times: \"beep\",put the finger on sensor, \"beep\", put up ,\"beep\", put on again)\n"); 
	//printf("  CMD3 : Fingerprint matching  (Send the command, put your finger on sensor after \"beep\".Each time you send a command, module waits and matches once)\n"); 
	//printf("  CMD4 : Clear fingerprints \n"); 
	//printf("  CMD5 : Switch to sleep mode, you can use the finger Automatic wake-up function (In this state, only CMD6 is valid. When a finger is placed on the sensor,the module is awakened and the finger is matched, without sending commands to match each time. The CMD6 can be used to wake up) \n"); 
	//printf("  CMD6 : Wake up and make all commands valid \n");
	//printf("*************** WaveShare Capacitive Fingerprint Reader Test ***************\n");	
}/***************************************************************************
* @brief     Analysis the command from PC terminal
****************************************************************************/
void Analysis_PC_Command(char * cmd_PC)
{	
    if((cmd_PC[0] == 'C')&&(cmd_PC[1] == 'M')&&(cmd_PC[2] == 'D'))
	{		
		
        switch(cmd_PC[3])
        {						
            case '1':
                if(Finger_SleepFlag == 1)  break;
                printf("Number of fingerprints already available:  %d  \n", GetUserCount());
                break;			
            case '2':
                if(Finger_SleepFlag == 1)  break;
                printf("adding fingerprint... please put your finger on scanner \n"); 
                switch(AddUser())
                {
                    case ACK_SUCCESS:
		    response = 1;
		    //sendMsg();
              
                        break;
                    
                    case ACK_FAIL: 			
                        printf("Failed: Please try to place the center of the fingerprint flat to sensor, or this fingerprint already exists ! \n");
                        response = 1;
			break;
                    
                    case ACK_FULL:			
                        printf("Failed: The fingerprint library is full ! \n");
			response = 1;
                        break;		
                }
                break;					
            case '3':
                if(Finger_SleepFlag == 1)  break;
                printf("Waiting Finger......Please try to place the center of the fingerprint flat to sensor ! \n");
                switch(VerifyUser())
                {
                    case ACK_SUCCESS:	
                        printf("Match successfully ! \n");
			
                        break;
                    case ACK_NO_USER:
                        printf("Failed: This fingerprint was not found in the library ! \n");
                        break;
                    case ACK_TIMEOUT:	
                        printf("Failed: Time out ! \n");
                        break;	
                    case ACK_GO_OUT:
                        printf("Failed: Please try to place the center of the fingerprint flat to sensor ! \n");
                        break;
                }
                break;				
            case '4':
                if(Finger_SleepFlag == 1)  break;
                ClearAllUser();
                printf("All fingerprints have been cleared ! \n");
                break;				
            case '5':
                if(Finger_SleepFlag == 1)  break;
                //compare(5,204);
		testservo();
                //Finger_SleepFlag = 1;
                //printf("Module has entered sleep mode: you can use the finger Automatic wake-up function, in this mode, only CMD6 is valid, send CMD6 to pull up the RST pin of module, so that the module exits sleep ! \n");	
                break;
            case '6':		
                if(pthread_mutex_lock(&mut) == 0)  
                {
                    digitalWrite(RST_PIN , HIGH);
                    msleep(250);  // Wait for module to start				
                    Finger_SleepFlag = 0;	                        
                    printf("The module is awake. All commands are valid ! \n");	
                    
                    pthread_mutex_unlock(&mut); 
                    break;                       
                }
            default: break;
        }
    }
}/***************************************************************************
* @brief  
     If you enter the sleep mode, then open the Automatic wake-up function of the finger,
     begin to check if the finger is pressed, and then start the module and match
****************************************************************************/
void Auto_Verify_Finger(void)
{	if(digitalRead(WAKE_PIN) == HIGH)   // If you press your finger
	{	
        if(pthread_mutex_lock(&mut) == 0)      
        {
            msleep(10);			
            if(digitalRead(WAKE_PIN) == HIGH)   
            {
                digitalWrite(RST_PIN , HIGH);    // Pull up the RST to start the module and start matching the fingers
                msleep(250);	 // Wait for module to start
                        
               // printf("Waiting Finger......Please try to place the center of the fingerprint flat to sensor ! \n");
                switch(compare(5,204))
                {
                    case ACK_SUCCESS:	
                        printf("Match successfully ! \n");
					Finger_SleepFlag = 0;
					time_flag = 0;
					timeout =0;
					response = 4;
					failnumber = 0;
					sendMsg();
					
					LEDT = LEDB;
					twinkle();
					digitalWrite(LEDB, HIGH);
					digitalWrite(LEDG, LOW);
					digitalWrite(LEDR, HIGH);
					servo();
                        break;
			
                    case ACK_NO_USER:
                        printf("Failed: This fingerprint was not found in the library ! \n");
					Finger_SleepFlag = 0;
					time_flag = 0;
					timeout = 37;
					response=5;
					sendMsg();
					LEDT = LEDR;
					twinkle();
					digitalWrite(RST_PIN,LOW);
					//compare_again();
					
					 
					
					
                        break;
                    case ACK_TIMEOUT:	
                        printf("locked \n");
			//response=6;
			//sendMsg();
			
					Finger_SleepFlag = 0;
					time_flag = 0;
					timeout = 37;
					response = 6;
					failnumber = 0;
					
					sendMsg();
					//LEDT = LEDR;
					//twinkle();
					digitalWrite(RST_PIN,LOW);
					delay(59000);
					printf("unlock\n");
					digitalWrite(LEDB, HIGH);
					digitalWrite(LEDG, LOW);
					digitalWrite(LEDR, HIGH);
					
			break;	
                    case ACK_GO_OUT:
                        printf("Failed: Please try to place the center of the fingerprint flat to sensor ! \n");
					Finger_SleepFlag = 0;
					time_flag = 0;
					timeout = 37;
					 
					
                        break;
                }
                
                //After the matching action is completed, drag RST down to sleep
                //and continue to wait for your fingers to press
                //digitalWrite(RST_PIN , LOW);    
            }
            pthread_mutex_unlock(&mut);          
        }
}}/***********************************
 * @test obtain characteristic
 * *********************************/
uint16_t charobtain(void){
	uint16_t m;
	int i = 0 ,j=0;
	time_flag = 1;
	Finger_SleepFlag = 0;
	char filename[20];
	char buffer[20];
	finger_TxBuf[0] = 0x23;
	finger_TxBuf[1] = 0;
	finger_TxBuf[2] = 0;
	finger_TxBuf[3] = 0;
	finger_TxBuf[4] = 0;
		m = getdata(5, 204);
		FILE *fp;
	for (j=1 ; j<5 ; j++){
	sprintf(filename,"list//testch%d.txt",j);
	fp = fopen(filename,"a+");
	
	if(fgets(buffer,sizeof(1),fp) == NULL ){
		break;
	}
	
	msleep(250);
		}
	encry = j;
	printf("%d",encry);
	
	if(fp == NULL){
		return 1;
	}
	for (i = 11; i<204; i++){
		fprintf(fp, "%X\n", finger_charRxBuf[i]);
	}
		number = GetUserCount() - 1;
		fprintf(fp,"%d\n",number);
	for(int i=0; i<20 ; i++){
		fprintf(fp,"%c\n", userid[number][i]);
		//printf("%c",userid[number][i]);
			}
	fclose(fp);
	encryption();
	time_flag = 0;
		}/**********************************
 * @test get data
 * *******************************/
 uint16_t getdata(uint16_t Scnt, uint16_t Rcnt){
	 uint16_t i;
	 int CheckSum;
	// uint16_t m;
	 time_flag = 1;
	 Finger_SleepFlag = 0;
	 
	 serialFlush(ser);
	 
	 TxByte(CMD_HEAD);
	 CheckSum = 0;
	 for (i = 0; i< Scnt; i++){
		 TxByte(finger_TxBuf[i]);
		 CheckSum ^= finger_TxBuf[i];
	 }
	 TxByte(CheckSum);
	 TxByte(CMD_TAIL);
	 
	 finger_RxNum = 0;
	 memset(finger_charRxBuf,0,sizeof(finger_charRxBuf));
	i = 0;
	do{
		if(serialDataAvail(ser)){
			finger_charRxBuf[i++] = serialGetchar(ser);
			//printf("%d\n",i);
		}
	}while( i != Rcnt);
	time_flag = 0;
	}
	
	uint16_t compare(uint16_t Scnt, uint16_t Rcnt){
	char buffer[600];
	char filename[20];
	char charac[192];
	int i,j=0;
	int maxk = GetUserCount();
	uint16_t CheckSum;
	time_flag = 1;
	Finger_SleepFlag = 0;
		sleep(1);
		
	FILE *fp;
			for (k=1 ; k<maxk+1 ; k++){
	encry = k;
	encryption();
	serialFlush(ser);
	sprintf(filename,"list//testch%d.txt",k);
	fp = fopen(filename,"a+");
	printf("%d",k);
		if(fgets(buffer,sizeof(1),fp) == NULL ){
			printf("break");
			time_flag=0;
			Finger_SleepFlag=0;
			break;
		}
	for (j=0; j<192; j++){
		fscanf(fp,"%X\n", &charac[j]);
		//printf("%x\n",charac[j]);
	}
	encryption();
		fclose(fp);
	finger_TxBuf[0] = 0x44;
	finger_TxBuf[1] = 0;
	finger_TxBuf[2] = 0xC4;
	finger_TxBuf[3] = 0;
	finger_TxBuf[4] = 0;
	 
	 TxByte(CMD_HEAD);
	 CheckSum = 0;
	 for (i = 0; i< 5; i++){
		 TxByte(finger_TxBuf[i]);
		 CheckSum ^= finger_TxBuf[i];
	 }
	 TxByte(CheckSum);
	 TxByte(CMD_TAIL);
	 
	 TxByte(CMD_HEAD);
	 TxByte(0);
	 TxByte(0);
	 TxByte(0);
	 CheckSum = 0;
	 for (i=0; i<193; i++){
		 TxByte(charac[i]);
		 CheckSum ^= charac[i];
	 }
	 TxByte(CheckSum);
	 TxByte(CMD_TAIL);
	 sleep(0.5);
	 finger_RxNum = 0;
	 
	 memset(finger_charRxBuf,0,sizeof(finger_charRxBuf));
	i = 0;
	do{
		if(serialDataAvail(ser)){
			//printf("%x%d\n",finger_charRxBuf[i],i);
			finger_charRxBuf[i++] = serialGetchar(ser);
			
		}
	}while( i != 8);
	if(finger_charRxBuf[4] == ACK_SUCCESS){
		printf("open\n");
		f=0;
		k=1;
		//servo();
		return ACK_SUCCESS;
		break;
	}
	else{
		printf("fail\n");
		failnumber = failnumber +1;
		LEDT = LEDR;
		twinkle();
	}
	}	//response = 5;
	//sendMsg();
	
	if(failnumber == 2*GetUserCount()){
		digitalWrite(LEDB, HIGH);
		digitalWrite(LEDG, HIGH);
		digitalWrite(LEDR, LOW);
		return ACK_TIMEOUT;
}
else{
	response = 5;
	sendMsg();
}time_flag = 0;
digitalWrite(RST_PIN,LOW);
}	
					


 uint16_t getimage(uint16_t Scnt, uint16_t Rcnt){
	 uint16_t i,j,k;
	 int CheckSum;
	 uint16_t m;
	 time_flag = 1;
	 time_flag = 1;
	 Finger_SleepFlag = 0;
	 char buffer[8];
	 
	 serialFlush(ser);
	 
	 TxByte(CMD_HEAD);
	 CheckSum = 0;
	 for (i = 0; i< Scnt; i++){
		 TxByte(finger_TxBuf[i]);
		 CheckSum ^= finger_TxBuf[i];
	 }
	 TxByte(CheckSum);
	 TxByte(CMD_TAIL);
	 
	 finger_RxNum = 0;
	 memset(finger_image,0,sizeof(finger_image));
	i = 0;
	j = 0;
	k = 0;
	do{
		if(serialDataAvail(ser)){
			if(k<9){
			buffer[k++] = serialGetchar(ser);
		}
			if(k == 9){
			finger_image[i++][j] = serialGetchar(ser);
			//printf("%d\n",i);
			if(i==50){
				i=0;
				j=j+1;
			}
		}
	}
	}while( j !=132);
	time_flag = 0;
	}
	
int bluesend(){
(void) signal(SIGINT,ctrl_c_handler);   
  
pthread_t readT, writeT;   
char *message1 = "Read thread\n";   
char *message2 = "Write thread\n";   
int iret1, iret2;   
  
struct sockaddr_rc loc_addr={ 0 },client_addr={ 0 };   
char buf[18] = { 0 };   
  
unsigned int opt = sizeof(client_addr) ;   
  
  
ss = socket(AF_BLUETOOTH,SOCK_STREAM,BTPROTO_RFCOMM) ;   
  
loc_addr.rc_family = AF_BLUETOOTH ;   
 loc_addr.rc_bdaddr = *BDADDR_ANY;
str2ba("B8:27:EB:F0:C7:36",&loc_addr.rc_bdaddr) ; 
loc_addr.rc_channel = 1 ;   
  
bind(ss,(struct sockaddr *)&loc_addr,sizeof(loc_addr));   
printf("Binding success\n");   
  
listen(ss,1) ;   
printf("socket in listen mode\n");   
  
clients = accept(ss,(struct sockaddr *)&client_addr,&opt);   
ba2str(&client_addr.rc_bdaddr,buf);   
fprintf(stdout,"Connection accepted from %s\n",buf);   
  
  
iret1 = pthread_create(&readT,NULL,readMsg,(void*) message1);   
iret2 = pthread_create(&writeT,NULL,sendMsg,(void*) message2);   
  
pthread_join(readT,NULL);   
pthread_join(writeT,NULL);   
  
close_sockets() ;   
return 0 ;   
}   
  
void *sendMsg(){   
char msg[30] ;   
int status ;   
char cmd_buf;
  
do{   
memset(msg,0,sizeof(msg));   
if(strncmp("EXIT",msg,4)==0 || strncmp("exit",msg,4)==0)break;   
if(response == 0){	
	msg[0]='a'; //appear
	send(clients,msg,strlen(msg),0);  
	response = 99 ;
	break;
}else if(response ==1){
	msg[0]='b'; //disappear
	send(clients,msg,strlen(msg),0);  
	response = 99 ;
	break;
}else if(response ==2){
	msg[0]='c'; //already
	send(clients,msg,strlen(msg),0);
	response = 99;
	break;
}else if(response ==3){
	msg[0]='d'; //sleepmode
	send(clients,msg,strlen(msg),0);
	response = 99;
	break;
}else if(response ==4){
	msg[0]='e'; //unsleepmode
	send(clients,msg,strlen(msg),0);
	response = 99;
	break;
}else if(response ==5){
	msg[0]='f'; //notmatch
	send(clients,msg,strlen(msg),0);
	response = 99;
	break;
}else if(response ==6){
	msg[0]='g'; //unlock
	send(clients,msg,strlen(msg),0);
	sleep(59);
	msg[0]='h'; //unlock
	send(clients,msg,strlen(msg),0);
	//Finger_SleepFlag = 0;
	//time_flag = 0;
	//timeout =0;
	response = 99;
	break;
}else if(response ==7){
	msg[0]='x'; //change alaram1
	send(clients,msg,strlen(msg),0);
	response = 99;
	break;
}else if(response ==8){
	msg[0]='y'; //change alaram2
	send(clients,msg,strlen(msg),0);
	response = 99;
	break;
}else if(response ==9){
	msg[0]='z'; //change alaram3
	send(clients,msg,strlen(msg),0);
	response = 99;
	break;
}else if(response ==10){
	msg[0]='i';
	send(clients,msg,strlen(msg),0);
	response = 99;
	break;
}	//status = send(clients,msg,strlen(msg),0);   
//fprintf(stdout,"Status = %d\n",status);  
}while(status > 0);   
}  
void *readMsg(){   
int bytes_read;   
char buf[1024] = { 0 };  
char cmd_buf[3]; 
do{   
memset(buf,0,sizeof(buf));   
//read data from the client   
bytes_read = recv(clients,buf,sizeof(buf),0) ;  
if(buf[0]=='a'){
	cmd_buf[0]='C';
	cmd_buf[1]='M';
	cmd_buf[2]='D';
	cmd_buf[3]='2';
	Analysis_PC_Command(cmd_buf);
}else if(buf[0]=='d'){
	cmd_buf[0]='C';
	cmd_buf[1]='M';
	cmd_buf[2]='D';
	cmd_buf[3]='4';
	Analysis_PC_Command(cmd_buf);
}else if(buf[0]=='x'){
	receiving = 'a';
	Alarm_Changing();
}else if(buf[0]=='y'){
	receiving = 'b';
	Alarm_Changing();
}else if(buf[0]=='z'){
	receiving = 'c';
	Alarm_Changing();
}else if(buf[0]=='w'){
	Alarm_Checking();
	if(receiving =='x'){
		response = 7;
		sendMsg();
	}
	else if(receiving =='y'){
		response = 8;
		sendMsg();
	}
	else if(receiving == 'z'){
		response = 9;
		sendMsg();
	}
}else if(buf[0]=='k'){
	Finger_SleepFlag = 0;
	time_flag = 0;
	timeout =0;
	digitalWrite(RST_PIN,HIGH);
	printf("wake\n");
	servo();
}
else if(buf[0]=='1'){
	Finger_SleepFlag = 0;
	time_flag = 0;
	timeout =0;
	printf("ggambback\n");
	LEDT = LEDG;
	flicker();
}//fprintf(stdout,"Bytes read = %d\n",bytes_read);   
if(bytes_read <= 0)break;   
//fprintf(stdout,"<<>> %s",buf);   
}while(1);   
}  
void close_sockets(){
pinMode(LED_NUM,OUTPUT);
   	digitalWrite(LEDR,HIGH);
	digitalWrite(LEDG,HIGH);
	digitalWrite(LEDB,HIGH);
//close connection   
//sleep(100);
//printf("1");
close(clients);   
close(ss) ; 
//close_sockets();
//exit(0);
printf("sockets closed\n");   
}   
  
void ctrl_c_handler(int signal) { 
pinMode(LED_NUM,OUTPUT);
printf("Catched signal: %d ... !!\n", signal);   
   	digitalWrite(LEDR,HIGH);
	digitalWrite(LEDG,HIGH);
	digitalWrite(LEDB,HIGH);
close_sockets();   
exit(0);   
//(void) signal(SIGINT, SIG_DFL);   
}   
void servo() {
	//if (wiringPiSetupGpio() == -1)
//	wiringPiSetupGpio();
	softPwmCreate(SERVO_PIN, 0, 200);	// PINX %D pwmRange èÄ\ l 20ms8tX ü0| Ìæ >> 200 * 100 = 20000 ms)
//(int pin, int initialValue, int pwmRange);
	softPwmWrite(SERVO_PIN, 30);		// value <\  ph. - )¥
//(int pin, int value);
	delay(1000);
	softPwmWrite(SERVO_PIN,0);
	delay(400);					// 600msÙH softPwmWrite()ÁÜ  (
	softPwmWrite(SERVO_PIN, 1);		// + )¥
	delay(1000);
	softPwmWrite(SERVO_PIN,0); // stop the motor (with viveration).
	delay(400);

}
void Alarm_Checking() {
		int a = digitalRead(SEL1);
	int b = digitalRead(SEL2);
	int c = 0, d = 0, e = 0;	
	printf("1\n");
	printf("%d, %d, %d, %d, %d\n",a,b,c,d,e);
	//SEL1 >  VCC : 1, GND : 10, OPEN : 0
	if (a == 0) c = 0;
	else if (a == 1) c = 1;
	else c = 10;
	printf("2\n");
	printf("%d, %d, %d, %d, %d\n",a,b,c,d,e);
		//SEL2 >  VCC : 2, OPEN : 0
	if (b == 0) d = 0;
	else if (b == 1) d = 2;
	printf("3\n");
	printf("%d, %d, %d, %d, %d\n",a,b,c,d,e);
		
	//digitalRead() = 0, 1, 2, 10
	e = c + d;
	printf("4\n");
	printf("%d, %d, %d, %d, %d\n",a,b,c,d,e);
		switch(e){
	case 0: //Alert1
		Alarm_Operating();
		
		printf("À L¼1 ä.\n");
		receiving = 'x';
		break;
	case 1: //Alert2
		Alarm_Operating();
		
		printf("À@ L¼2 ä.\n");
		receiving = 'y';
		break;
	case 2: //Alert4
		Alarm_Operating();		
		
		printf("À@ L¼4 Èä.\n");
		receiving = 'z';		
		break;
	case 10: //Alert3
		Alarm_Operating();
		
		printf("À@ L¼3 ä.\n");		
		break;
	default :
	printf("\nsometing has taken place accident\n");
	}
}void Alarm_Changing() {
	//a
	pinMode(SEL1, OUTPUT);
	pinMode(SEL2, OUTPUT); // SEL1ü SEL
	pinMode(ALARMOUTPUT, OUTPUT);
		digitalWrite(SEL1, LOW);
	digitalWrite(SEL2, LOW);
	digitalWrite(ALARMOUTPUT, HIGH);	
		printf("please input 'a' ~ 'c'\n");
		//scanf("%c", &receiving);
	//getchar();
	printf("Saved Character is %c\n", receiving);
	pullUpDnControl(SEL1, PUD_OFF);
	pullUpDnControl(SEL2, PUD_OFF);
//	pullUpDnControl(ALARMOUTPUT, PUD_OFF);
		printf("Clear SEL1 : %d, SEL2 : %d \n", digitalRead(SEL1), digitalRead(SEL2));
		switch(receiving) {
		case 'a'://Alert1 , default
	//		pullUpDnControl(SEL1, PUD_DOWN);
//		pullUpDnControl(SEL2, PUD_DOWN);
		
//		SEL1_STAT = LOW;
//		SEL2_STAT = LOW;
		
		digitalWrite(SEL1, LOW);
		digitalWrite(SEL2, LOW);
		
		printf("Change to Alert1 successfully!\n");
		
//		printf("Now Setting is SEL1 : %d, SEL2 : %d\n", digitalRead(SEL1_STAT), digitalRead(SEL2_STAT));
		
		break;
	case 'b'://Alert2
//		pullUpDnControl(SEL1, PUD_UP);
//		pullUpDnControl(SEL2, PUD_DOWN);
//		SEL1_STAT = HIGH;
//		SEL2_STAT = LOW;
		
		digitalWrite(SEL1, HIGH);
		digitalWrite(SEL2, LOW);
						
		printf("Change to Alert2 successfully!\n");
		break;
	case 'c'://Alert4
//		pullUpDnControl(SEL1, PUD_DOWN);		
//		pullUpDnControl(SEL2, PUD_UP);
//		SEL1_STAT = LOW;
//		SEL2_STAT = HIGH;
		
		digitalWrite(SEL1, LOW);
		digitalWrite(SEL2, HIGH);			
				
		printf("Change to Alert4 successfully!\n");	
		break;
	case 'd'://Alert3, h.
		printf("$\n");
//		pullUpDnControl(SEL1, PUD_DOWN);		
//		pullUpDnControl(SEL2, PUD_DOWN);
//		SEL1_STAT = LOW;
//		SEL2_STAT = LOW;
				
		digitalWrite(SEL1, LOW);
		digitalWrite(SEL2, LOW);
				
		printf("Change to Alert3 successfully!\n");
		break;
	}
}void Alarm_Operating(){
		digitalWrite(ALARMOUTPUT, LOW);
		delay(1);
		printf("Alarm is now operating!\n");
		delay(3600);
		digitalWrite(ALARMOUTPUT, HIGH);
		delay(1);
		
}
void twinkle(){
					digitalWrite(LEDT,1);
				msleep(100);
				digitalWrite(LEDT,0);
				msleep(100);
				digitalWrite(LEDT,1);
				msleep(100);
				digitalWrite(LEDT,0);
				msleep(100);
				digitalWrite(LEDT,1);
				sleep(1);

} 
void flicker(){
				digitalWrite(LEDT,1);
				
				msleep(100);
				digitalWrite(LEDT,0);
				
				msleep(100);
				digitalWrite(LEDT,1);
				
				msleep(100);
				digitalWrite(LEDT,0);
				
				msleep(100);
				digitalWrite(LEDT,1);
				
				msleep(100);
				digitalWrite(LEDT,0);
				
				msleep(100);
				digitalWrite(LEDT,1);
				
				msleep(100);
				digitalWrite(LEDT,0);
				
				msleep(100);
				digitalWrite(LEDT,1);
				sleep(1);

} 

int encryption() {
	char data[194];
	char key[16] = { "abcdefghijklmnop" };
	char buffer[20];
	
	size_t read_size, i;
	long frpos, fwpos;
	char filename[20];
	
	
	FILE *fp ;
	printf("%d",encry);

	
	sprintf(filename,"list//testch%d.txt",encry);
	fp = fopen(filename,"r+b");
	

	if(fgets(buffer,sizeof(1),fp) == NULL ){
		return 1;
	}


	while (!feof(fp)) {
		fwpos = ftell(fp);
		read_size = fread(data, 1, 194, fp); //data
		if (read_size == 0) break;
		for (i = 0; i < read_size; i++)
			data[i] ^= key[i % 16]; //XOR 
		frpos = ftell(fp);
		fseek(fp, fwpos, SEEK_SET);
		fwrite(data, 1, read_size, fp); //fp
		fseek(fp, frpos, SEEK_SET);
	}
	
	printf("encryptioning..\n");
	fclose(fp);
	return 1;

}


void testservo() {
	char rotate[10];
	softPwmCreate(SERVO_PIN, 0, 200);	// PINX %D pwmRange èÄ\ l 20ms8tX ü0| Ìæ >> 200 * 100 = 20000 ms)
//(int pin, int initialValue, int pwmRange);
	softPwmWrite(SERVO_PIN, 5);		// value <\  ph. - )¥
//(int pin, int value);
	delay(600);
	softPwmWrite(SERVO_PIN,0);
	delay(600);					// 600msÙH softPwmWrite()ÁÜ  (
	printf("input:\n");
	scanf("%d",rotate);
	softPwmWrite(SERVO_PIN, 25);		// + )¥
	delay(rotate);
	softPwmWrite(SERVO_PIN,0); // stop the motor (with viveration).
	delay(600);
	return 1;
}
