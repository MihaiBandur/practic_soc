/*---------------------------------------------------------------------- 
 * Fi?ier: main.c 
 * Fi?ierul principial de rulare a aplica?iei 
 *--------------------------------------------------------------------*/ 
 
/*----------------------------------------------------------------------  
 * Includes 
 *--------------------------------------------------------------------*/ 
 
#include <iom1280.h> 
#include <inavr.h> 
#include <stdint.h>  
#include "mylib.h" 
#include "usart.h" 
 
/*---------------------------------------------------------------------- 
 * Vector de întrerupere Watchdog Timer 
 *--------------------------------------------------------------------*/ 
#pragma vector = WDT_vect  
__interrupt void intrerupere() 
{   
  uint8_t c = '!'; 
/* 
 * La declan?area întreruperii Watchdog se trimite caracterul '!'   
 * prin USART 
 */ 
  my_print(CHAR, &c); 
} 
 
int main(void) 
{  
  // Ini?ializarea USART-ului cu baudrate-ul definit în "usart.h" 
  USART_initialize(BAUD_RATE); 
   
  // Mesaj ini?ial de transmis 
  uint8_t message[20]; 
  message[0] = '8'; 
  message[1] = '.'; 
  message[2] = '7'; 
  message[3] = '8'; 
  message[4] = '9'; 
  message[5] = '\0'; 
   
  // Transmiterea ?irului de caractere prin USART 
  USART_transmit_string(message, 6); 
  
/* 
 * Resetarea registrului de stare pentru a cura?a flag-urile de  
 * resetare  
 */ 
  MCUSR = 0; 
   // Resetarea Watchdog-ului înainte de configurare 
  // Setarea bit-ului pentru modificarea WDT 
  WDTCSR |= (1 << WDCE) | (1 << WDE);   
  // Dezactivare Watchdog 
  WDTCSR = 0;                           
   
  // Activarea Watchdog Timer-ului 
  WDTCSR |= (1 << WDCE) | (1 << WDE); 
   
  // Setarea perioadei de time-out de aproximativ 4 secunde 
  // WDP3 activeaza intervalul mai lung 
  WDTCSR = (1 << WDE) | (1 << WDP3);   
   
  // Activarea întreruperii globale 
  __enable_interrupt(); 
   
  // Activarea întreruperii Watchdog (Watchdog Interrupt Enable) 
  WDTCSR = (1 << WDIE); 
   
  // Bucla infinita principala 
  while(1) 
  { 
    __watchdog_reset();
    uint8_t aux; 
     
    // Primirea caracter USART (func?ie blocanta) 
    USART_receive_char(&aux); 
     
    if(aux != '\0') 
    {  
  // Resetarea Watchdog-ului pentru a evita resetarea microcontrolerului 
     
       
      /* Reactivarea întreruperii Watchdog */
      WDTCSR = (1 << WDIE); 
       
      /* Retrimiterea caracterului primit prin USART */
      USART_transmit_char(aux);  
    }  
  } 
   
  return 0; 
}