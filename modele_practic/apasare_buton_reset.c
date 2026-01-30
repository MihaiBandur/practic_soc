
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay.h>

#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

// Variabilă globală pentru sutimi de secundă (1 unitate = 10ms)
// Folosim volatile și unsigned long pentru a nu da overflow prea repede
volatile uint32_t uptime_sutimi = 0;

// --- Configurare UART ---
void UART_Init(unsigned int ubrr) {
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << TXEN0); // Activăm transmisia
    UCSR0C = (1 << USBS0) | (3 << UCSZ00); // 8 biți date, 1 stop
}

void UART_Transmit(unsigned char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void UART_Print(char* str) {
    while (*str) UART_Transmit(*str++);
}

// --- Configurare Timer 1 (Baza de timp 10ms) ---
void Timer1_Init() {
    // Setăm Modul CTC (Clear Timer on Compare Match) - Modul 4
    // WGM12 = 1
    TCCR1B |= (1 << WGM12);

    // Setăm Prescaler 8
    // CS11 = 1
    TCCR1B |= (1 << CS11);

    // Calculăm TOP pentru 10ms (100Hz)
    // Formula: (16.000.000 / (8 * 100)) - 1 = 19999
    OCR1A = 19999;

    // Activăm întreruperea la egalitate (Compare Match A)
    TIMSK1 |= (1 << OCIE1A);
}

// --- Rutina de Întrerupere (Vine de 100 de ori pe secundă) ---
ISR(TIMER1_COMPA_vect) {
    uptime_sutimi++; // Incrementăm contorul de timp
}

int main(void) {
    UART_Init(MYUBRR);
    Timer1_Init();
    
    // Activăm întreruperile globale
    sei();
    
    char buffer[30];

    while (1) {
        // Pentru a evita citirea incorectă a variabilei volatile în timp ce
        // se modifică în întrerupere, o copiem atomic (opțional dar recomandat)
        cli();
        uint32_t timp_curent = uptime_sutimi;
        sei();

        // Calculăm Secundele (partea întreagă)
        uint32_t secunde = timp_curent / 100;

        // Calculăm Sutimile (partea fracționară XX.XX)
        uint8_t fractiune = timp_curent % 100;

        // Formăm șirul: %02lu înseamnă minim 2 cifre (ex: 0.05 nu .5)
        sprintf(buffer, "%lu.%02u secunde\r\n", secunde, fractiune);
        
        UART_Print(buffer);

        // Așteptăm puțin ca să nu inundăm seriala (ex: de 2 ori pe secundă)
        _delay_ms(500);
    }
}
