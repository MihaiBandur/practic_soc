#include<ioavr.h>
#include<inavr.h>
#include<instrincs.h>
#include<stdint.h>


#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR (F_CPU/16/BAUD-1)


volatile char* tx_ptr;
volatile uint8_t measure_mode = 0;
volatile uint8_t measure_done = 0;
volatile uint16_t ticks = 0;

const char text_seriala[] = "Test de viteza pe intreruperi!";
char buffer_rezultat[40];


void UART_Init(void) {
    UBRR0H = (unsigned char)(MYUBRR >> 8);
    UBRR0L = (unsigned char)MYUBRR;
    // Activam TX si RX, si Intreruperea de RX (pentru start test)
    UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);
    UCSR0C = (1 << USBS0) | (3 << UCSZ00); // 8N1
}


void Timer1_Init_Prescaler64(void){
    TCNT1 = 0;
    TCCR1A = 0;
    
    TCCR1B |= (1 << CS11) | (CS10);
}

void Timer1_Stop(void){
    TCCR1B  = 0;
}

void Start_Transmission(const char * p, uint8_t mode){
    
    while (UCSR0B & (1 << UDRIE0));
    
    tx_ptr = p;
    
    if(mode == 1){
        Timer1_Init_Prescaler64();
    }
    
    UCSR0B |= (1 << UDRIE0);
}


#pragma vector = USART0_RX_vect

__interrupt void UART_RX_ISR(void){
    char dummy = UDR0;
    
    if(measure_mode == 0){
        Start_Transmission((char *)text_seriala, 1);
    }
}

#pragma vector = USART0_UDRE_vect
__interrupt void USART0_UDRE(void){
    if (*tx_ptr != '\0') {
        UDR0 = *tx_ptr++; // Trimitem caracterul si avansam
    } else {
        // Am ajuns la finalul sirului!
        UCSR0B &= ~(1 << UDRIE0); // Oprim cererile de date (UDRE)
        
        if(measure_mode == 1){
            UCSR0A |= (1 << TXC0);   // Curatam flag-ul vechi TXC
            UCSR0B |= (1 << TXCIE0); // Activam Intreruperea "Transmisie Completa"
        }
    }
}


#pragma vector = USART0_TX_vect
__interrupt void USART0_TX(void){
    
    if (measure_mode == 1){
        Timer1_Stop();
        ticks = TCTN1;
        measure_mode = 0;
        meassure_done = 1;
    }
    
    UCSR0B &= ~(1 << TXCIE0); // Ne dezactivam singuri
}



void Format_Result(uint16_t ticks) {
    // Calcule (Integer math)
    uint32_t time_us = (uint32_t)ticks * 4; // Prescaler 64 => 4us/tick
    
    // Construim manual stringul in buffer_rezultat
    // "Timp: XXXXX us\r\n"
    char *p = buffer_rezultat;
    
    // Hardcodam inceputul
    *p++ = '\r'; *p++ = '\n';
    *p++ = 'T'; *p++ = 'i'; *p++ = 'm'; *p++ = 'p'; *p++ = ':'; *p++ = ' ';
    
    // Convertim numarul in text (algoritm simplu)
    uint32_t temp = time_us;
    uint32_t divisor = 100000; // Maxim 6 cifre pt uint16 * 4
    uint8_t leading_zeros = 1;
    
    while(divisor > 0) {
        uint8_t digit = temp / divisor;
        if (digit != 0 || divisor == 1) leading_zeros = 0;
        
        if (!leading_zeros) {
            *p++ = digit + '0';
        }
        temp %= divisor;
        divisor /= 10;
    }
    
    *p++ = ' '; *p++ = 'u'; *p++ = 's';
    *p++ = '\r'; *p++ = '\n';
    *p++ = '\0';
}



void main(void){
    UART_Init();
    
    
    Start_Transmission("\r\n--- START TEST (Apasa o tasta) ---\r\n", 0);
    
    while (1) {
        if(measurement_done == 1){
            measurement_done = 0;
            
            Format_Result(captured_ticks);
            Start_Transmission(captured_ticks, 0);
        }
    }

}

