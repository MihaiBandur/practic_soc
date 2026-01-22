#include <ioavr.h>
#include <inavr.h>
#include <intrinsics.h> 
// Frecven?a procesorului
#ifndef F_CPU 
#define F_CPU 16000000UL 
#endif 


// Led ON / OFF / Init 
#define LED_ON() (PORTD |= 1 << 7)
#define LED_OFF() (PORTD &= ~(1 << 7))
#define LED_INIT() (DDRD |= (1 << 7))


// Puls pe semnal ON / OFF / Init 
#define REF_ON() (PORTC |= (1 << 0))
#define REF_OFF() (PORTC &= ~(1 <<0))
#define REF_INIT() (DDRD |= (1 << 0))


// ISR pentru Timer0 Overflow 
#pragma vector = TIMER0_OVF_vect
__interrupt void Timer0_Overflow_ISR(void){
  REF_ON();
  LED_ON();
  
  __delay_cycles(10000);
  
  LED_OFF();
  REF_OFF();
}

 
/* 
 * Func?ia genereaza un puls de durata fixa la fiecare overflow al  
 * Timer0, folosit pentru debugging sau sincronizare. 
 */ 



void main(void){
  LED_INIT();
  REF_INIT();
  
  TCCR0A = 0x00;
  
  TCCR0B = (1 << CS01) | (1 << CS00);
  TIMSK0 = (1 << TOIE0);
  
  __enable_interrupt();
  
  while(1){
    ;
  }
}