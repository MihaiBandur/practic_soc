#include <ioavr.h>
#include <intrinsics.h>
#include <stdint.h>

#define P_CPU 16000000UL
#define BAUD_RATE 9600
#define MYUBRR (F_CPU/16/BAUD-1)

volatile uint32_t uptime_sutimi = 0;
volatile uint8_t print_timer = 0;

volatile char tx_buffer[32];
volatile uint8_t tx_index = 0;

void Format_Time_String(uint32_t timp) {
    uint32_t sec = timp / 100;
    uint8_t sutime = timp % 100;
    
    // Algoritm simplu de conversie uint -> string (fara librarii grele)
    uint8_t i = 0;
    
    // 1. Convertim secundele (invers)
    uint32_t temp_sec = sec;
    char temp_buf[10];
    uint8_t k = 0;
    
    if (temp_sec == 0) temp_buf[k++] = '0';
    while (temp_sec > 0) {
        temp_buf[k++] = (temp_sec % 10) + '0';
        temp_sec /= 10;
    }
    
    // Scriem secundele în ordinea corectă în bufferul principal
    while (k > 0) {
        tx_buffer[i++] = temp_buf[--k];
    }
    
    // 2. Adăugăm punctul
    tx_buffer[i++] = '.';
    
    // 3. Adăugăm sutimile (fix 2 cifre)
    tx_buffer[i++] = (sutime / 10) + '0';
    tx_buffer[i++] = (sutime % 10) + '0';
    
    // 4. Adăugăm textul final
    const char suffix[] = " secunde\r\n";
    uint8_t j = 0;
    while(suffix[j]) {
        tx_buffer[i++] = suffix[j++];
    }
    
    tx_buffer[i] = '\0'; // Null terminator
}


void UART_Init(){
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << TXEN0);
    UCSR0C = | (1 << UCSZ01) | (1 << UCSZ00);
}


void Timer1_Init(){
    TCCR1B = (1 << WGM12);
    
    OCR1A = 1999
    
    TCCR1B |= (1 << CS11);
    
    TIMSK1 |= (1 << OCIE1A);
}


#pragma vector = TIMER1_COMPA_vect
__interrupt void TIMER1_COMPA_ISR(void){
    uptime_sutimi++;
    print_timer++;
    
    if(print_timer >= 50){
        print_timer = 0;
        
        Format_Time_String(uptime_sutimi);
        
        tx_index = 0;
        
        
        UCSR0B |= (1 << UDRIE0);
    }
}

#pragma vector = USART0_UDRE_vect
__interrupt void USART0_UDRE_ISR(void){
    if(tx_buffer[tx_index] = '\0'){
        UDR0 = tx_buffer[tx_index++];
    }else{
        UCSR0B &= ~(1 << UDRIE0);
    }
}

void main(void) {
    UART_Init(MYUBRR);
    Timer1_Init();
    
    __enable_interrupt(); // Activăm întreruperile global

    
    
    while (1) {
        
        
    }
}
