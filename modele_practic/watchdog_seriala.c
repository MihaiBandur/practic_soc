/*----------------------------------------------------------------------- 
 * Fi?ier: main.c 
 * Fi?ierul principal de rulare a aplica?iei de calcul al frecven?ei 
 *---------------------------------------------------------------------*/ 
 
/*-----------------------------------------------------------------------    
 * Includes 
 *---------------------------------------------------------------------*/ 
#include <iom1280.h> 
#include <inavr.h>        
#include <stdint.h>      
 
/*-----------------------------------------------------------------------    
 * Variabile globale 
 *---------------------------------------------------------------------*/ 
 

#define FREC 16000000
#define BAUD 9600
#define BAUD_RATE (FREC / 16 / BAUD - 1)


#define set_rx_intrerupt ( 1 << RXCIE2 )
#define set_udre_intrerupt ( 1 << UDRIE2 )


#define RECEIVER (1 << RXEN2 )
#define TRANSMITTER (1 << TXEN2 )


#define RX_IN (1 << PH0)
#define TX_OUT (1 << PH1)




/* 
 * Variabila pentru valoarea curenta a Timer1, re?inuta dupa resetare  
 * (fara ini?ializare) 
 */ 
__no_init uint16_t Timer1_currentValue; 
 
/* 
 * Variabila pentru numarul de overflow-uri ale Timer1, re?inuta dupa  
 * resetare 
 */ 
__no_init uint8_t Timer1_numberOverflows;

volatile uint32_t index = 0;
volatile uint8_t numere_caracter[32] = {'\0'};


void initialise_usart3(uint16_t baud_rate){
  UBRR2H = (uint8_t)(baud_rate >> 8);
  UBRR2L = (uint8_t)(baud_rate & 0xFF);
  
  UCSR2B |= TRANSMITTER | set_udre_intrerupt;
    
  DDRH &= ~RX_IN;
  DDRH |= (TX_OUT); 
}


   
// Rutina de întrerupere pentru overflow-ul Timer1 
#pragma vector = TIMER1_OVF_vect 
__interrupt void T1_OVF() 
{  
    // Incrementarea numarului de overflow-uri 
    Timer1_numberOverflows++;  
}  


#pragma vector = USART2_UDRE_vect
__interrupt void USART1_TX_ISR(void){
    
  if(numere_caracter[index] != '\0'){
      UDR2 = numere_caracter[index];
      index++;
  }else{
    index = 0;
     UCSR2B &= ~set_udre_intrerupt; 
  }
  
}


void u16_to_dec(uint16_t value, volatile uint8_t *buffer){
    uint8_t temp[6];   // max 5 cifre + '\0'
    uint8_t i = 0, j = 0;

    if(value == 0){
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    while(value > 0){
        temp[i++] = (value % 10) + '0';
        value /= 10;
    }

    while(i > 0){
        buffer[j++] = temp[--i];
    }

    buffer[j] = '\0';
}


void build_wdt_freq_message(uint16_t frecv_khz,
                            volatile uint8_t *buffer){
    uint8_t i = 0;

    buffer[i++] = 'W';
    buffer[i++] = 'D';
    buffer[i++] = 'T';
    buffer[i++] = ' ';
    buffer[i++] = 'F';
    buffer[i++] = 'R';
    buffer[i++] = 'E';
    buffer[i++] = 'Q';
    buffer[i++] = ' ';
    buffer[i++] = '=';
    buffer[i++] = ' ';

    u16_to_dec(frecv_khz, &buffer[i]);
    while(buffer[i] != '\0') i++;

    buffer[i++] = ' ';
    buffer[i++] = 'k';
    buffer[i++] = 'H';
    buffer[i++] = 'z';
    buffer[i++] = '\r';
    buffer[i++] = '\n';
    buffer[i++] = '\0';
}

 
int main(void) 
{  
  
    initialise_usart3(BAUD_RATE );
    // Resetarea registrului MCUSR (?terge flag-urile de reset) 
    MCUSR = 0;  
 
    // Ini?ializarea USART-ului pentru transmiterea datelor serial   
 
    /*----------------------------------------------------- 
     * Ini?ializarea Timer1 în mod Normal 
     * Prescaler setat pe CS10 (fara divizarea frecven?ei) 
     *-----------------------------------------------------*/ 
    TCCR1B |= (1 << CS10);   // Start Timer1 cu prescaler 1 
        // Activarea întreruperii de overflow pentru Timer1 
    TIMSK1 |= (1 << TOIE1);   
 
    // Activarea întreruperilor globale 
    __enable_interrupt();  
 
    /*----------------------------------------------------- 
     * Ini?ializarea ?i activarea Watchdog Timer 
     * Setare perioada de timeout: 32 ms 
     *-----------------------------------------------------*/ 
   WDTCSR |= (1 << WDCE) | (1 << WDE);   // Permite modificarea WDT 
   WDTCSR = (1 << WDE) | (1 << WDP0) | (1 << WDP1);    // Activare WDT cu WDP0 (32 ms) 
 
    /*----------------------------------------------------- 
     * Daca exista valori salvate în variabile dupa resetare, calculam  
     * perioada efectiva a WDT 
     *-----------------------------------------------------*/ 
    if (Timer1_numberOverflows > 0 || Timer1_currentValue > 0)  
    {  
        // Calcularea numarului total de ticks 
        uint32_t number = (Timer1_numberOverflows * (uint64_t)65535) +            
        Timer1_currentValue;  
 
        // Conversia în milisecunde (fiecare tick dureaza 62.5 ns) 
        uint8_t period = number * 0.0000625;   
        
        // Calcularea duratei unui ciclu de clock al WDT (în nanosecunde) 
        uint16_t time_per_clock = (period * 1000000) / 4096;   
 
        // Calcularea frecven?ei WDT în kHz 
        uint8_t frecv = (1000000. / time_per_clock);   
        
        build_wdt_freq_message(frecv, numere_caracter);
        
        index = 0;
        UCSR2B |= set_udre_intrerupt;   // pornire transmisie
 
        // Copierea frecven?ei într-o variabila pentru transmitere 

        
    }  
 
    /*----------------------------------------------------- 
     * Resetarea valorilor pentru urmatoarea masuratoare 
     *-----------------------------------------------------*/ 
    Timer1_numberOverflows = 0;  
    Timer1_currentValue = 0;  
    TCNT1 = 0;  
 
    // Resetarea Watchdog-ului 
    asm("WDR");  
 
    // (Op?ional) Reactivare WDT 
    // WDTCSR |= (1 << WDE);  
 
    /*----------------------------------------------------- 
     * Bucla principala 
     * Salveaza permanent valoarea curenta a Timer1 
     *-----------------------------------------------------*/ 
    while (1)  
    {  
        Timer1_currentValue = TCNT1;  
    }   
 
    return 0; 
} 
