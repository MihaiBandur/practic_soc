/*-----------------------------------------------------------------------  
* Fi?ier: debounce.c  
 * Utilizat pentru citirea unui buton cu debounce software 
 *---------------------------------------------------------------------*/ 
 
// Includes 
#include <ioavr.h> 
#include <intrinsics.h> 
#include <stdint.h> 
 
/*----------------------------------------------------------------------- 
 * Public defines  
 *---------------------------------------------------------------------*/ 
 
// Buton T4 
#define BUTTON_PIN_REG  PINE 
#define BUTTON_DDR      DDRE 
#define BUTTON_PORT     PORTE 
#define BUTTON_PIN      6 

// LED A pe PA5 
#define LED_A_DDR       DDRA 
#define LED_A_PORT      PORTA 
#define LED_A_PIN       5 
 
// LED B pe PA6 
#define LED_B_DDR       DDRA 
#define LED_B_PORT      PORTA 
#define LED_B_PIN       6 
 
/*----------------------------------------------------------------------- 
 * Global variables 
 *---------------------------------------------------------------------*/ 
 
// Variabila pentru stocarea timpului scurs, similar cu millis() 
volatile unsigned long system_millis = 0; 
 
// Variabile pentru starea debounce 
 
// Starea anterioara a butonului (1 = neapasat) 
uint8_t last_button_state = 1; 
 
// Starea stabila a butonului  
uint8_t debounced_button_state = 1;  
 
// Timpul de la ultimul debounce 
unsigned long last_debounce_time = 0; 
// Timpul de a?teptare debounce (în ms)  
const unsigned int debounce_delay = 50;  

volatile unsigned int  button_pressed = 0;

volatile  uint8_t reading ;

// ISR pentru TIMER0 (ceasul sistemului) 
#pragma vector = TIMER0_COMPA_vect 
 
__interrupt void Millis_ISR(void) {
  if ((system_millis) ==1){
    debounced_button_state = 1;
     EIMSK |= (1 << INT6 );

    
  }
  if(system_millis > 0 )
    system_millis--; 
} 

void init_button_intreruption(){
  EICRB |= (1 << ISC61); 
  EIMSK |= (1 << INT6 );
}

#pragma vector = INT6_vect
__interrupt void BUTON_T4(){
    system_millis = 50;
    LED_B_PORT |= (1 << LED_B_PIN); 
    EIMSK &= ~(1 << INT6 );

}
 
// Func?ia de ini?ializare timer 
void init_millis_timer(void) { 
    /* 
     * Configurare Timer0 mod CTC pentru a genera o întrerupere la 1 ms 
     * F_CPU = 16 MHz. Prescaler = 64. F_timer = 16 MHz / 64 = 250 kHz. 
     * Pentru 1 ms (1 kHz), OCR0A = (250 kHz / 1 kHz) - 1 = 249. 
     */ 
    TCCR0A |= (1 << WGM01); // Mod CTC 
    TCCR0B |= (1 << CS01) | (1 << CS00); // Prescaler 64 
    OCR0A = 249; 
    TIMSK0 |= (1 << OCIE0A); // Activeaza întreruperea pe Compare Match A 
} 
 
/* 
 * Func?ia configureaza butonul (PE6) ca intrare cu pull-up ?i LED-urile    
 * (PA5, PA6) ca ie?iri, pornind cu ambele stinse. 
 */ 
void init_gpio(void) { 
    // Configurare Buton (PE6) ca intrare cu pull-up 
    BUTTON_DDR &= ~(1 << BUTTON_PIN);  
    BUTTON_PORT |= (1 << BUTTON_PIN);  
 
    // Configurare LED-uri (PA5, PA6) ca ie?iri 
    LED_A_DDR |= (1 << LED_A_PIN); 
    
    LED_B_DDR |= (1 << LED_B_PIN); 
     
    // Se opresc LED-urile ini?ial 
    LED_A_PORT &= ~(1 << LED_A_PIN); 
    LED_B_PORT &= ~(1 << LED_B_PIN); 
} 
 
int main(void) { 
    init_gpio(); 
    init_millis_timer(); 
    init_button_intreruption();
    __enable_interrupt();  
 
    while (1) { 
      {
        LED_B_PORT &= ~(1 << LED_B_PIN);
            
        
          
                // Se verifica daca noua stare stabila este "apasat" 
                if (debounced_button_state == 1) { 
                    // Se comuta starea LED-ului A 
                  debounced_button_state = 0;
                    LED_A_PORT ^= (1 << LED_A_PIN); 
                } 
            
        } 
        // Se actualizeaza starea anterioara pentru urmatoarea itera?ie 
  } 

}