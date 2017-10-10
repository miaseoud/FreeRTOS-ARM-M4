/*************************************************************************************************************************/
/*--------------------------------- Library for UART0 Initilization and driving  ----------------------------------------*/
/*************************************************************************************************************************/
/*U0Rx PA0 Pin17
  U0Tx PA1 Pin18
 * 1. Initilization function
 * 2. Send character
 * 3. Send String
 * 4. Receive character  (polling)
 * 5. Receive String using Receive character
 * 6. Non-Blocking receive by checking on Flag instead of polling
 */
#include "UART0.h"
#include "REG.h"

void UART0_Init(int baud_rate,int clock){
      //float temp;

      SYSCTL_RCGC1_R |= SYSCTL_RCGC1_UART0; // UART0 Clock Gating Control: activate UART
      SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA; //Port A Clock Gating Control:activate port A

      UART0_CTL_R &= ~UART_CTL_UARTEN;     // disable UART
      /*---------------------------------------Baud-Rate Generation-------------------------*/
      //BRD = BRDI + BRDF = UARTSysClk / (ClkDiv * Baud Rate)
      //UARTFBRD[DIVFRAC] = integer(BRDF * 64 + 0.5)
      UART0_IBRD_R =    (int)(clock/(16*baud_rate));;
      float temp = (clock/(float)(16*baud_rate)) - (int)(clock/(16*baud_rate));
      UART0_FBRD_R =    (int)(64 * temp + 0.5);;
      /*---------------------------------------------------------------------------------------*/
      UART0_LCRH_R = (UART_LCRH_WLEN_8|UART_LCRH_FEN); // 8 bit, no parity bits, one stop, FIFOs
      UART0_CTL_R |= UART_CTL_UARTEN;                 // enable UART

      GPIO_PORTA_AFSEL_R |= 0x03;          // GPIO Alternate Function Select
      GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFFFFFF00)+0x00000011;  // GPIO Port Control: To associate alternative function with UART
      GPIO_PORTA_DEN_R |= 0x03;  // GPIO Digital Enable
      GPIO_PORTA_AMSEL_R &= ~0x03; //GPIO Analog Mode Selec
}

void UART0_SendChr(char chr){
      while((UART0_FR_R&UART_FR_TXFF) != 0); //UART Flag  Register & UART Transmit FIFO Full
      UART0_DR_R = chr;
}

void UART0_SendString(char * str){
     int i = 0 ;
     while(str[i] != 0)
        UART0_SendChr(str[i++]);
}


char UART0_ReadChr(void){
      while((UART0_FR_R&UART_FR_RXFE) != 0); //UART Flag  Register & UART Receive FIFO Empty
      return((char)(UART0_DR_R&0xFF));
}
void UART0_ReadString(char* str){
    char c =1;
    int i = 0;
    while(c != (char)TERMINATE_STRING){
        c = UART0_ReadChr();
        str[i++] = c;
    }
    str[i] = 0 ;
}


// Immediately return input or 0 if no input
unsigned char UART0_InCharNonBlocking(void){
  if((UART0_FR_R&UART_FR_RXFE) == 0){
    return((unsigned char)(UART1_DR_R&0xFF));
  } else{
    return 0;
  }
}
