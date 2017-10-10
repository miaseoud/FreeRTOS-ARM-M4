/*--------------------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------FreeRTOS General Example------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------*/
/*Tools used from FreeRTOS:
 * - Concurrent performance of tasks.
 * - Priority utilization.
 * - Queues for inter-task communications.
 * - Binary sempahores for multitask synchronization.
 * - Group events for more complex task synchronization and signaling (To be added)
 *
 * Highest priority task creates 2 queues containing a list of color strings and RGB pins to activate associated colors.
 * When a failing edge is detected on PortF pin0, a binary semaphore is released allowing synchronization with another task
 * sending the color string via UART and writing RGB led with associated colors.
 * A third task takes a peek from the queue without removing the item and used received value to set a bit in the group event byte.
 */
/*--------------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------General personal Libraries-----------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------*/
#include "REG.h"
#include "DIO.h"
#include "PLL.h"
#include "UART0.h"
#include "itoaX.h"
/*--------------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------FreeRTOS libraries-------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "semphr.h"
#include "event_groups.h"
#define SWITCHTASKSTACKSIZE        128
/*--------------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------Type definitions-------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------*/
typedef unsigned char u8;
/*--------------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------Initializing-------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------*/
QueueHandle_t First_Q,String_Q; //one queue for color strings and one queue for rgb pins in PortF
EventGroupHandle_t Group1;    //to be added next
SemaphoreHandle_t Client_Sem; //binary sempahore to synchronize the arrival of new color with its execution
/*--------------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------Data to be passed using queues-------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------*/
u8 colors[7] = {RED,BLUE,GREEN,YELLOW,CYAN,WHITE,PINK};
char *colors2[7] = {"RED","BLUE","GREEN","YELLOW","CYAN","WHITE","PINK"};
/*--------------------------------------------------------------------------------------------------*/
/*-------------------------------------------Task 1-------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*Receive from queue after acknowledging the arrival of new color through semaphore*/
volatile u8 data = 0 ;
char str[];
void Task1(void * para){
    Client_Sem = xSemaphoreCreateBinary(); //create sempahore "client_sem"
    xSemaphoreTake( Client_Sem, (TickType_t)0 ); //intilaize it as already taken
    u8 led_color = RED;
    char * current_color = "RED";
    while(1){
        UART0_SendString("to receive queue");
        UART0_Println();
        /*will wait until the release of semaphore*/
        xSemaphoreTake( Client_Sem, (TickType_t)portMAX_DELAY );
        UART0_SendString("Semaphore taken"); //UART debugging
        UART0_Println(); //using macro from UART.h
        xQueueReceive( First_Q, ( void * ) &( led_color ), (TickType_t)0); //receive string from queue one
        xQueueReceive( String_Q,( void * ) &(current_color), (TickType_t)0);//receive PortF RGB pins from queue two
       /* UART0_SendString(itoaX(led_color, str, 10)); //UART send PortF RGB LED Pins for debugging
        UART0_Println();*/
        DIO_PortWrite(PortF,led_color,DIO_HIGH);//activate RGB
    }
}
/*--------------------------------------------------------------------------------------------------*/
/*-------------------------------------------Task 2-------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*Create 2 queues and send to each queue when failling edge is detected on PortF pin0 (connected to HW push-button)*/
//higher priority will run first to create queues
void Task2(void * para) {
    First_Q = xQueueCreate( 2, sizeof(u8) ); //create RGB pins queue
    String_Q = xQueueCreate( 2, sizeof(colors2) ); //create color strings
    UART0_SendString("QueueCreate"); //UART debugging
    UART0_Println();
    u8 i = 0,led_color = RED,flag_low = Pin0 ; //to work on falling edge
    char * current_color= "RED";
    while(1)
    {
        DIO_PortRead(PortF,Pin0,&data);
        if((data == DIO_LOW) && (flag_low == Pin0)) //active low , if button is pressed , and last time was high
        {
            UART0_SendString("-----------------");//Beginning of new queue item to be transmitted
            UART0_Println();
            //Bring new color
            led_color = colors[i];
            current_color = colors2[i];
            //UART0_SendString(itoaX(led_color, str, 10));
            //UART0_Println();
            i++;//for next tranmission
            xQueueSend( First_Q,( void * ) &led_color, (TickType_t )1000);
            xQueueSend( String_Q,( void * ) &current_color, (TickType_t )1000);
            UART0_SendString("queue item sent");//UART debugging
            UART0_Println();
            i = i%7;
            if( xSemaphoreGive(Client_Sem) != pdTRUE )//give semaphore to synchronize next "receive" task
            {
            //pdTRUE if the semaphore was released. pdFALSE if an error occurred. Semaphores are implemented using queues.
            }
        }
        flag_low = data;
        vTaskDelay(100); //ever 10 tick, as 10 ms
    }
}

void Task3(void * para){
    //u8 led_color;
    char * current_color ;
    while(1){
    if (xQueuePeek( String_Q, &(current_color), (TickType_t)0)!= pdTRUE ){//if there is an item in the queue
    xQueuePeek( String_Q, &(current_color), (TickType_t)portMAX_DELAY); //takes a copy of the queue item without removing it from the queue
    UART0_SendString("Peek OK:");
    UART0_SendString(current_color);
    UART0_Println();
    //Set group event if color matched
    }
    vTaskDelay(50);
     }
}
/*
void Task4(void * para){
    u8 state = DIO_HIGH;
    while(1){
        xEventGroupWaitBits(Group1,Pin0|Pin1||Pin2 ,pdTRUE,pdTRUE,(TickType_t)portMAX_DELAY );
        DIO_PortWrite(PortF,YELLOW,state);
        state ^= 0xFF;
    }
}
*/
int main(void) {
    PLL_Init();//Selection of 80 MHz clock using phase locked loop  and main oscillator
	UART0_Init(9600,80000000);
	DIO_PortInit(PortF, Pin0|Pin1|Pin2|Pin3|Pin4 , Pin0|Pin4);//Initilization of Pins on PortF and Pins to pullup for edge triggered buttons
	DIO_PortDirection(PortF , Pin1|Pin2|Pin3, DIO_OUTPUT);
	DIO_PortDirection(PortF , Pin0|Pin4, DIO_INPUT);

    TaskHandle_t  First_handle,Second_handle,Third_handle;//task declaration
    //TaskHandle_t  First_handle,Second_handle,Third_handle,Fourth_handle;
    xTaskCreate(Task1, "Receive from queue",SWITCHTASKSTACKSIZE, NULL,1,   &First_handle);//task1 initialization
    xTaskCreate(Task2, "Create queue and send",SWITCHTASKSTACKSIZE, NULL,3,   &Second_handle);//task2 initialization
    xTaskCreate(Task3, "Peek queue",SWITCHTASKSTACKSIZE, NULL,2,   &Third_handle);//task3 initialization
    //  xTaskCreate(Task4, "Task4",SWITCHTASKSTACKSIZE, NULL,1,   &Fourth_handle);task4 initialization
    UART0_Println();
    UART0_SendString("--------------Start-------------");
    UART0_Println();
	vTaskStartScheduler();
	return 0;
}
