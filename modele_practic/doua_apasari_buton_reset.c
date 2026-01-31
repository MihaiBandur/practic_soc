#include <ioavr.h>
#include <intrinsics.h>
#include <stdio.h>
#include <stdint.h>

#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR (F_CPU/16/BAUD-1)

// --- Variabila care supraviețuiește Reset-ului ---
__no_init volatile uint32_t timer_sutimi;

// --- Buffer mic (doar pentru număr și \r\n) ---
volatile char tx_buffer[16];
volatile uint8_t tx_index = 0;

/* --- Inițializări --- */
void UART_Init(unsigned int ubrr) {
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << TXEN0); // Doar TX activ
    UCSR0C = (1 << USBS0) | (3 << UCSZ00); // 8N1
}

void Timer1_Init(void) {
    TCCR1B = (1 << WGM12);      // CTC Mode
    OCR1A = 19999;              // 10ms la Prescaler 8
    TCCR1B |= (1 << CS11);      // Start cu Prescaler 8
    TIMSK1 |= (1 << OCIE1A);    // Activare Intrerupere
}

/* --- Întreruperi --- */

// 1. Cronometrul (Rulează mereu)
#pragma vector = TIMER1_COMPA_vect
__interrupt void Timer1_ISR(void) {
    timer_sutimi++;
}

// 2. Transmițătorul (Doar când avem date)
#pragma vector = USART0_UDRE_vect
__interrupt void UART_UDRE_ISR(void) {
    if (tx_buffer[tx_index] != '\0') {
        UDR0 = tx_buffer[tx_index++];
    } else {
        UCSR0B &= ~(1 << UDRIE0); // Stop transmisie
    }
}

/* --- MAIN --- */
void main(void) {
    // 1. Salvăm motivul resetului
    unsigned char mcusr = MCUSR;
    MCUSR = 0;

    UART_Init(MYUBRR);

    // 2. Dacă a fost apăsat butonul RESET (EXTRF)
    if (mcusr & (1 << EXTRF)) {
        // Formatăm strict numărul: X.XX (urmat de Enter)
        sprintf((char*)tx_buffer, "%lu.%02u\r\n", timer_sutimi / 100, timer_sutimi % 100);
        
        // Pornim transmisia
        tx_index = 0;
        UCSR0B |= (1 << UDRIE0);
    }

    // 3. Resetăm contorul pentru noua măsurătoare
    // (Fie că e Power On, fie că e Reset, o luăm de la 0 acum)
    timer_sutimi = 0;

    // 4. Pornim sistemul
    Timer1_Init();
    __enable_interrupt();

    while(1) {
        // Procesorul doarme, totul e pe întreruperi
    }
}
