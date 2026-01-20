#include <ioavr.h>
#include <inavr.h>
#include <intrinsics.h> 
volatile int set = 0;
void timer5_mode15_init(void)
{
    TCCR5A = 0;
    TCCR5B = 0;

    
    TCCR5A |= (1 << WGM51) | (1 << WGM50);
    TCCR5B |= (1 << WGM53) | (1 << WGM52);

    TCCR5A |= (1 << COM5C1) | (1 << COM5B1);

    
    TCCR5B |= (1 << CS51);

    
    OCR5A = 1775;
    OCR5B = 1760;
    
    OCR5C = 1776;   
    
    TIMSK5 |= (1 << TOIE5);    
   
}

#pragma vector = TIMER5_OVF_vect


__interrupt void Timer5_Overflow_ISR(){
  PORTE ^= (1 << PE5);
  if(set == 0){
     OCR5A = 340;
     
     set = 1;
  }else if (set == 1){
      OCR5A = 1775;
     
      set = 0;
  }
  

  
}


int main(void)
{
    DDRE |= (1 << PE5);
    DDRL |= (1 << PL4) | (1 << PL5);

    timer5_mode15_init();
    __enable_interrupt();

    while(1);
}