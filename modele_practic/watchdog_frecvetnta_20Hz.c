#include <ioavr.h>
#include <inavr.h>
#include <intrinsics.h>


#define LED_PORT PORTB
#define LED_DDR  DDRB
#define LED_PIN  (1 << PB0)

volatile uint8_t wdt_counter = 0;

void WDT_Init_Init_Interrupt(void){
    __disable_interrupt();
    __watchdog_reset();
    
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    
    WDTCSR = (1 << WDIE) | (0 << WDE) | (0 << WDP3) | (0 << WDP2) | (0 << WDP1) | (0 << WDP0);
    
    __enable_interrupt();
}

#pragma vector = WDT_vector
__interrupt void WDT_ISR(void){
    wdt_counter++;
    
    if(wdt_counter == 1){
        LED_PORT &= ~LED_PIN;
    }else if(wdt_counter == 3){
        LED_PORT |= LED_PIN;
        wdt_counter = 0;
    }
}


void main(void){
    
        LED_DDR |= LED_PIN;
        
        
        LED_PORT |= LED_PIN;

        
        WDT_Init_Interrupt();

        
        while(1) {
            ;
        }
}
