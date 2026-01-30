#define F_CPU 16000000UL
#include <ioavr.h>
#include <intrinsics.h>
#include <stdio.h>

/***** ZONA DE MEMORIE PERSISTENTĂ *****/
/* Cuvântul cheie __no_init spune compilatorului IAR:
   "Nu pune această variabilă pe 0 la restart!"
   Ea își păstrează valoarea cât timp placa are curent, chiar dacă dăm Reset.
*/
__no_init volatile unsigned long timer_sutimi;

/***** Configurare UART *****/
void UART_Init(void) {
    // 9600 baud la 16MHz (UBRR = 103)
    UBRR0H = 0;
    UBRR0L = 103;
    UCSR0B = (1 << TXEN0); // Doar Transmisie
    UCSR0C = (1 << USBS0) | (3 << UCSZ00); // 8N1
}

void UART_Print(char* str) {
    while (*str) {
        while (!(UCSR0A & (1 << UDRE0)));
        UDR0 = *str++;
    }
}

/***** Configurare Timer 1 (10ms) *****/
void Timer1_Init(void) {
    // Mod CTC (Clear Timer on Compare), Prescaler 8
    TCCR1B = (1 << WGM12) | (1 << CS11);
    
    // TOP pentru 10ms (100Hz)
    // 16.000.000 / (8 * 100) - 1 = 19999
    OCR1A = 19999;
    
    // Activare întrerupere
    TIMSK1 = (1 << OCIE1A);
}

/***** Întreruperea de timp (100Hz) *****/
#pragma vector = TIMER1_COMPA_vect
__interrupt void Timer1_ISR(void) {
    timer_sutimi++; // Numărăm timpul în background
}

/***** MAIN *****/
void main(void) {
    char buffer[40];
    
    // 1. Verificăm cauza Reset-ului (IMPORTANT!)
    unsigned char motiv_reset = MCUSR;
    MCUSR = 0; // Curățăm registrul pentru data viitoare

    UART_Init();

    // 2. Logica de afișare bazată pe tipul de reset
    if (motiv_reset & (1 << PORF)) {
        // PORF = Power On Reset Flag (Am băgat placa în priză)
        UART_Print("\r\n[INFO] Pornire rece (Power On). Contor resetat.\r\n");
        timer_sutimi = 0; // Inițializăm manual variabila
    }
    else if (motiv_reset & (1 << EXTRF)) {
        // EXTRF = External Reset Flag (S-a apăsat butonul RESET)
        // Aici variabila timer_sutimi are încă valoarea dinainte de reset!
        
        unsigned long secunde = timer_sutimi / 100;
        unsigned int fractiune = timer_sutimi % 100;
        
        sprintf(buffer, "\r\nTimp intre reset-uri: %lu.%02u secunde\r\n", secunde, fractiune);
        UART_Print(buffer);
        
        timer_sutimi = 0; // Resetăm contorul pentru următoarea tură
    }
    else {
        // Alte tipuri de reset (Brown-out, Watchdog), tratate ca pornire nouă
        timer_sutimi = 0;
    }

    // 3. Pornim Timer-ul pentru a măsura URMĂTOAREA perioadă
    Timer1_Init();
    __enable_interrupt();

    while(1) {
        // Buclă infinită. Procesorul doar numără timpul în ISR.
        // Când vei apăsa RESET, codul sare înapoi la începutul lui main()
        // dar 'timer_sutimi' va fi păstrat.
    }
}
