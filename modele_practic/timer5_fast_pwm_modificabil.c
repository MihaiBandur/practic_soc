#include<ioavr.h>
#include<inavr.h>
#include<intrinsics.h>

void init_pin_pe5(void){
    DDRE |= (1 << PE5);
    PORTE &= ~(1 << PE5);
}

void init_timer5(void){
    TCCR5A = (1 << WGM51) | (1 << WGM50);
    TCCR5B = (1 << WGM53) | (1 << WGM52);
    
    TCCR5B |= (1 << CS51);
    
    OCR5A = 1666;
    
    OCR5B = 500;
    
    TIMSK5 |= (1 << TOIE5) | (OCIE5B);
    
}

#pragma vector = TIMER5_OVF_vect
__interrupt void TIMER5_OVF_ISR(void){
    PORTE |= (1 << PE5);
}

#pragma vector = TIMER5_COMPB_vect
__interrupt void TIMER5_COMPB_ISR(void){
    PORTE &= ~(1 << PE5);
}


void main(void){
    init_pin_pe5();
    init_timer5();
    
    __enable_interrupt();
    
    while(1){
        ;
    }
}
