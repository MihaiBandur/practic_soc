#include <ioavr.h>
#include <inavr.h>
#include <stdint.h>
#include <intrinsics.h>


#define FREC 16000000UL
#define BAUD 9600
#define BAUD_RATE (FREC / 16 / BAUD - 1)


#define WDT_CYCLES 4096UL

#define MATH_CONST 4096000UL


volatile char tx_buffer[32];
volatile uint8_t tx_index = 0;


volatile uint16_t t1_overflows = 0;


void initialise_usart(uint16_t baud_rate){
  UBRR2H = (uint8_t)(baud_rate >> 8);
  UBRR2L = (uint8_t)(baud_rate & 0xFF);
  
  UCSR2B = (1 << TXEN2);
  
  DDRH |= (1 << PH1);
}


void initialise_timer1(void) {
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  TIMSK1 |= (1 << TOIE1);
  TCCR1B |= (1 << CS10);
}


void initialise_wdt_interrupt(void) {
  __disable_interrupt();
  __watchdog_reset();
  
  // Secventa de deblocare
  WDTCSR |= (1 << WDCE) | (1 << WDE);
  
  // Configurare:
  // WDIE = 1 (Activeaza Intreruperea)
  // WDE  = 0 (DEZACTIVEAZA Reset-ul - Critic!)
  // WDP1 | WDP0 = Setare timp ~32ms
  WDTCSR = (1 << WDIE) | (0 << WDE) | (1 << WDP1) | (1 << WDP0);
  
  __enable_interrupt();
}


void format_message(uint16_t freq) {
    uint8_t i = 0;
    
    
    tx_buffer[i++] = 'W'; tx_buffer[i++] = 'D'; tx_buffer[i++] = 'T';
    tx_buffer[i++] = '='; tx_buffer[i++] = ' ';
    
    
    uint16_t temp = freq;
    char num_buf[6];
    uint8_t k = 0;
    
    if(temp == 0) num_buf[k++] = '0';
    while (temp > 0) {
        num_buf[k++] = (temp % 10) + '0';
        temp /= 10;
    }
    while (k > 0) tx_buffer[i++] = num_buf[--k];
    
    
    tx_buffer[i++] = ' '; tx_buffer[i++] = 'k'; tx_buffer[i++] = 'H';
    tx_buffer[i++] = 'z'; tx_buffer[i++] = '\r'; tx_buffer[i++] = '\n';
    tx_buffer[i++] = '\0';
}

#pragma vector = TIMER1_OVF_vect
__interrupt void T1_OVF_ISR(void) {
    t1_overflows++;
}


#pragma vector = WDT_vect
__interrupt void WDT_ISR(void) {
    uint16_t t1_val = TCNT1;
    uint8_t  ovf_val = t1_overflows;
    
    
    TCNT1 = 0;
    t1_overflows = 0;
    
    
    
    uint32_t total_ticks = ((uint32_t)ovf_val * 65536UL) + t1_val;
    
    
    
    uint32_t period_us = total_ticks / 16;
    
    
    
    if (period_us > 0) {
        uint16_t freq_khz = (uint16_t)(MATH_CONST / period_us);
        
        
        format_message(freq_khz);
        
        
        tx_index = 0;
        UCSR2B |= (1 << UDRIE2);
    }
    
}


#pragma vector = USART2_UDRE_vect
__interrupt void USART2_TX_ISR(void) {
    if (tx_buffer[tx_index] != '\0') {
        UDR2 = tx_buffer[tx_index++];
    } else {
        
        UCSR2B &= ~(1 << UDRIE2);
    }
}


void main(void) {
    
    initialise_usart(BAUD_RATE);
    initialise_timer1();
    
    
    initialise_wdt_interrupt();
    
    
    __enable_interrupt();
    
    
    
    while(1) {
        __sleep();
    }
}
