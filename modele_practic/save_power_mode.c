#include<ioavr.h>
#include<intrinsics.h>
#include<stdint.h>

#define BAUD_RATE 9600
#define FREC 16000000UL
#define MYUBRR (FREC / 16 / BAUD - 1)


// Flag-uri volatile pentru sincronizare intre intreruperi si main
volatile uint8_t  measure_start_flag = 0;
volatile uint8_t  measurement_done = 0;

// Variabile pentru masuratoare
volatile uint32_t t1_overflows = 0;
volatile uint32_t start_ticks = 0;
volatile uint32_t end_ticks = 0;
volatile uint8_t  wdt_edge_count = 0;

// Buffer transmisie
volatile char tx_buffer[32];
volatile uint8_t tx_index = 0;


void UART_Init(void) {
    UBRR0H = (unsigned char)(MYUBRR >> 8);
    UBRR0L = (unsigned char)MYUBRR;
    UCSR0B = (1 << TXEN0);  // Doar transmisie activata
    UCSR0C = (1 << USBS0) | (3 << UCSZ00); // 8 biti date, 1 stop
}

void INT0_init(void){
    DDRD &= ~(1 << PD0);
    PORTD |= (1 << PD0);
    
    EICRA |= (1 <<ISC01);
    EIMSK |= (1 << INT0);
}


void TIMER1_start(void){
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    
    t1_overflows = 0;
    
    
    TIMSK1 |= (1 << TOIE0);
    TCCR1B |= (1 << CS10);
}

void Timer1_Stop(void) {
    TCCR1B = 0; // Oprim ceasul
}

void WDT_Start_Measure(void){
    __disable_interrupt();
    __watchdog_reset();
    
    WDCTSR |= (1 <<WDCE) | ( 1<< WDE);
    
    WDTCSR = (1 << WDIE) | (0 << WDE) | (0 << WDP3) | (0 << WDP2) | (0 << WDP1) | (0 << WDP0);
    
    __enable_interrupt();
}

void WDT_stop(void){
    __disable_interrupt();
    __watchdog_reset();
    
    WDCTSR |= (1 <<WDCE) | ( 1<< WDE);
    WDTCSR = 0x00; // Dezactivare
    
    
    __enable_interrupt();
}

#pragma  vector = INT0__vect
__interrupt INT0_ISR(void){
    EIMSK &= ~(1 << INT0);
    measure_start_flag = 1;
}

#pragma vector = TIMER0_OVF_vect
__interrupt void TIMER0_OVF_ISR(void){
    t1_overflows++;
}



#pragma vector = WDT_vector
__interrupt void WDT_ISR(void){
    // Citim valoarea curenta a timerului
        uint16_t timer_now = TCNT1;
        uint32_t current_long_time = ((uint32_t)t1_overflows << 16) + timer_now;

        if (wdt_edge_count == 0) {
            // Start Masuratoare
            start_ticks = current_long_time;
            wdt_edge_count++;
        }
        else {
            // Stop Masuratoare
            end_ticks = current_long_time;
            WDT_Stop();
            Timer1_Stop();
            measurement_done = 1;
            wdt_edge_count = 0;
        }
}

#pragma vector = USART0_UDRE_vect
__interrupt void USART0_UDRE_vect(void){
    if (tx_buffer[tx_index] != '\0') {
            UDR0 = tx_buffer[tx_index++];
        } else {
            // Transmisie gata
            UCSR0B &= ~(1 << UDRIE0); // Oprim intreruperea UART
            
            // Curatam flag-urile de intrerupere externa si reactivam INT0
            EIFR |= (1 << INTF0);
            EIMSK |= (1 << INT0);
        }
}


void format_freq_string(uint32_t freq_khz) {
    uint8_t i = 0;
    uint32_t mhz = freq_khz / 1000;
    uint32_t zecimale = freq_khz % 1000;

    // Partea intreaga
    if (mhz > 9) { // Simplificare: presupunem o cifra pt WDT
         tx_buffer[i++] = (mhz / 10) + '0';
    }
    tx_buffer[i++] = (mhz % 10) + '0';
    
    tx_buffer[i++] = '.';
    
    // Partea zecimala (3 cifre)
    tx_buffer[i++] = (zecimale / 100) + '0';
    tx_buffer[i++] = ((zecimale / 10) % 10) + '0';
    tx_buffer[i++] = (zecimale % 10) + '0';
    
    tx_buffer[i++] = ' ';
    tx_buffer[i++] = 'M';
    tx_buffer[i++] = 'H';
    tx_buffer[i++] = 'z';
    tx_buffer[i++] = '\r';
    tx_buffer[i++] = '\n';
    tx_buffer[i++] = '\0';
}


void main(void){
    UART_Init();
    INT0_Init();
    
    __enable_interrupt();
    
    while(1){
        if(measure_start_flag){
            measure_start_flag = 0;
            measurement_done = 0;
            
            Timer1_Start();
            WDT_Start_Measure();
            
            while (measurement_done);
            
            uint32_t measured_ticks = end_ticks - start_ticks;
                        
                        // 1. Durata in microsecunde: T_us = Ticks / 16 (pt 16MHz)
            uint32_t duration_us = measured_ticks / 16;
                        
                        // 2. Frecventa WDT in kHz: F = (Nr_Cicli_WDT * 1000) / T_us
                        // Pt WDP=0000, WDT are 2048 cicli interni
            uint32_t freq_khz = (2048UL * 1000UL) / duration_us;
            
            
            format_freq_string(freq_khz);
                        
                        // Pornim transmisia
            tx_index = 0;
            UCSR0B |= (1 << UDRIE0);
                        
            // Asteptam sa termine transmisia inainte de a ne culca la loc
            while(UCSR0B & (1 << UDRIE0));
            
            
        }
        
        SMCR = (0 << 3) | (1 << 2) | (1 << 1) | (1 << 0);
        
        __sleep();
        
        SMCR &= ~(1 << 0);
    }
}

