#define F_CPU 16000000UL
#include <ioavr.h>
#include <intrinsics.h>


// Definim pinul de ieșire (Ex: PB0)
#define SIGNAL_PORT PORTB
#define SIGNAL_DDR  DDRB
#define SIGNAL_PIN  PB0

// Variabilă pentru a număra întreruperile (tick-urile de 16ms)
volatile uint8_t wdt_counter = 0;

/*
 * Configurare Watchdog în modul ÎNTRERUPERE (nu Reset!)
 * Setăm prescaler pentru aprox. 16ms.
 */
void WDT_Init_Interrupt_Mode(void) {
    // 1. Dezactivăm întreruperile global pentru siguranță
    __disable_interrupt();
    
    // 2. Resetăm WDT (bună practică înainte de reconfigurare)
    __watchdog_reset();

    // 3. Pornim secvența de modificare (Timed Sequence)
    // Trebuie scris 1 la WDCE și WDE
    WDTCSR |= (1 << WDCE) | (1 << WDE);

    // 4. Setăm noua configurație (în maxim 4 cicli de ceas!)
    // WDIE = 1 --> Activăm Întreruperea
    // WDE  = 0 --> Dezactivăm Reset-ul (CRITIC!)
    // WDP3:0 = 0000 --> Prescaler pentru 16ms (conform datasheet)
    WDTCSR = (1 << WDIE) | (0 << WDE) | (0 << WDP3) | (0 << WDP2) | (0 << WDP1) | (0 << WDP0);

    // 5. Reactivăm întreruperile
    __enable_interrrupt();
}

/*
 * Rutina de tratare a întreruperii Watchdog
 * Se apelează automat la fiecare ~16ms
 */
#pragma vector = WDT_vect
__interrupt void WDT_ISR(WDT_vect) {
    wdt_counter++;

    // Verificăm starea pinului pentru a ști ce numărăm (HIGH sau LOW)
    if (SIGNAL_PORT & (1 << SIGNAL_PIN)) {
        // --- Suntem pe HIGH ---
        // Avem nevoie de 132ms. 8 * 16ms = 128ms
        if (wdt_counter >= 8) {
            SIGNAL_PORT &= ~(1 << SIGNAL_PIN); // Trecem pe LOW
            wdt_counter = 0; // Resetăm contorul
        }
    } else {
        // --- Suntem pe LOW ---
        // Avem nevoie de 68ms. 4 * 16ms = 64ms
        if (wdt_counter >= 4) {
            SIGNAL_PORT |= (1 << SIGNAL_PIN); // Trecem pe HIGH
            wdt_counter = 0; // Resetăm contorul
        }
    }
}

int main(void) {
    // Configurare pin ca ieșire
    SIGNAL_DDR |= (1 << SIGNAL_PIN);
    
    // Pornim inițial pe HIGH
    SIGNAL_PORT |= (1 << SIGNAL_PIN);

    // Inițializăm Watchdog-ul
    WDT_Init_Interrupt_Mode();

    while (1) {
        // Bucla infinită este liberă!
        // Procesorul poate face orice altceva aici.
        // Generarea semnalului se face automat în background prin întreruperi.
    }
}
