// Includes 
#include <ioavr.h> 
#include <intrinsics.h> 


#define COUNTER_THRESHOLD 30

#pragma vector = TIMER0_OVF_vect
__interrupt void Timer0_Overflow_ISR(void){
  static unsigned int interrupt_counter = 0;
  
  interrupt_counter++;
  
  if(interrupt_counter > COUNTER_THRESHOLD){
    PORTD = ~PORTD;
    
    interrupt_counter = 0;
  }
  
}


void main(void){
  
  
  DDRD = 0xFF;
  
  PORTD = 0x00;
  
  TCCR0B = (1 << CS02) | (1 << CS00);
  
  TIMSK0 = (1 << TOIE0);
  
  __enable_interrupt();
  
  while(1){
    ;
  }
  
  
  
}