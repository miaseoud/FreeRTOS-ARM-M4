#include "PLL.h"

void PLL_Init(void){

  SYSCTL_RCC2_R |=  0x80000000;  // USERCC2
  SYSCTL_RCC2_R |=  0x00000800;  // BYPASS2, PLL bypass while initializing
  //  select the crystal value and oscillator source
  SYSCTL_RCC_R = (SYSCTL_RCC_R &~0x000007C0)   // clear XTAL field, bits 10-6
                 + 0x00000540;   //  configure for 16 MHz crystal
  SYSCTL_RCC2_R &= ~0x00000070;  // configure for main oscillator source
  SYSCTL_RCC2_R &= ~0x00002000;   //  activate PLL by clearing PWRDN
  SYSCTL_RCC2_R |= 0x40000000;   // use 400 MHz PLL, set the desired system divider
  SYSCTL_RCC2_R = (SYSCTL_RCC2_R&~ 0x1FC00000)  // clear system clock divider
                  + (4<<22);      // configure for 80 MHz clock
  //  wait for the PLL to lock by polling PLLLRIS
  while((SYSCTL_RIS_R&0x00000040)==0){};  //  wait for the PLL to lock by polling PLLLRIS
  //  enable use of PLL by clearing BYPASS
  SYSCTL_RCC2_R &= ~0x00000800;
}
