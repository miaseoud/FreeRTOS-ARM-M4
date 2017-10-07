#include "Reg.h"
#include "DIO.h"
#include "Types.h"
#include "PLL.h"
#include "UART.h"
/* FreeRTOS*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#define SWITCHTASKSTACKSIZE        128

u8 colors[7] = {RED,BLUE,GREEN,YELLOW,CYAN,WHITE,PINK};
char *colors2[7] = {"RED","BLUE","GREEN","YELLOW","CYAN","WHITE","PINK"};
SemaphoreHandle_t Client_Sem;
volatile u8 data = 0 ;
QueueHandle_t First_Q,String_Q;

//higher priority will run first
void Task2(void * para) {
    First_Q = xQueueCreate( 2, sizeof(u8) );
    String_Q = xQueueCreate( 2, sizeof(colors2) );

    u8 i = 0,led_color = RED,flag_low = Pin0 ; //to work on falling edge
    char * current_color= "RED";
    while(1)
    {
        DIO_PortRead(PortF,Pin0,&data);
        if((data == DIO_LOW) && (flag_low == Pin0)) //active low , if button is pressed , and last time was high
        {
            //switch color
            led_color = colors[i];
            current_color = colors2[i];
            i++;
            xQueueSend( First_Q,( void * ) &led_color, (TickType_t )1000);
            xQueueSend( String_Q,( void * ) &current_color, (TickType_t )1000);
            i = i%7;
            if( xSemaphoreGive(Client_Sem) != pdTRUE )
            {
            //pdTRUE if the semaphore was released. pdFALSE if an error occurred. Semaphores are implemented using queues.
            }
        }
        flag_low = data;
        vTaskDelay(100); //ever 10 tick, as 10 ms
    }

}
void Task1(void * para)
{
    Client_Sem = xSemaphoreCreateBinary();
    xSemaphoreTake( Client_Sem, (TickType_t)0 ); //intilaize it
    u8 led_color = RED;
    char * current_color = "RED";
    while(1)
    {
        xSemaphoreTake( Client_Sem, (TickType_t)portMAX_DELAY );
        /* these queues will not block if empty*/
        xQueueReceive( First_Q, &( led_color ), (TickType_t)0);
        xQueueReceive( String_Q, &(current_color), (TickType_t)0);
            UART0_SendString(current_color);
            UART0_Println();
        DIO_PortWrite(PortF,RED|BLUE|GREEN|YELLOW|CYAN|WHITE|PINK,DIO_LOW);
        //blink using timer
        DIO_PortWrite(PortF,led_color,DIO_HIGH);

    }
}


int main(void) {
	PLL_Set80();
	UART0_Init(115200,80000000);
	DIO_PortInit(PortF, Pin0|Pin1|Pin2|Pin3|Pin4 , Pin0|Pin4);
	DIO_PortDirection(PortF , Pin1|Pin2|Pin3, DIO_OUTPUT);
	DIO_PortDirection(PortF , Pin0|Pin4, DIO_INPUT);

	TaskHandle_t  First_handle,Second_handle;
    xTaskCreate(Task1, "Task1",SWITCHTASKSTACKSIZE, NULL,1,   &First_handle);
    xTaskCreate(Task2, "Task2",SWITCHTASKSTACKSIZE, NULL,2,   &Second_handle);

	vTaskStartScheduler();
	return 0;
}
