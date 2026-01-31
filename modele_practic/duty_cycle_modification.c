#include <ioavr.h>
#include <intrinsics.h>
#include <stdio.h>

/***** Configurare *****/
#define F_CPU 16000000UL
// Timer1 TOP pentru 10kHz (16MHz / 1 / 10000 = 1600)
#define ICR1_TOP 1599

/***** Variabile Globale *****/
volatile int procent = 5;
volatile int add = 5;
// Bufferul pentru transmisie (Cutia postala)
volatile char tx_buffer[32];
volatile uint8_t tx_index = 0;

// Constanta pentru delay (27.8ms / 0.1ms = 278 tick-uri)
const int perioade = 278;

/***** Initializari *****/
void initUSART(void)
{
  // 9600 baud la 16MHz
  UBRR0H = 0;
  UBRR0L = 103;
  // Activam doar TXEN0. NU activam inca intreruperea UDRIE0!
  UCSR0B = (1 << TXEN0);
  UCSR0C = (1 << USBS0) | (3 << UCSZ00); // 8 biti, 1 stop
}

void initTimer1(void)
{
  // Configurare Pin OC1A (PB5 la ATmega1280)
  DDRB |= (1 << PB5);

  // WGM13:0 = 1110 (Fast PWM, ICR1 = TOP)
  // COM1A1 = Non-inverting
  TCCR1A = (1 << WGM11) | (1 << COM1A1);
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10); // CS10 = Prescaler 1

  ICR1 = ICR1_TOP; // Setare Frecventa 10kHz

  // Setare Duty Cycle initial (5%)
  OCR1A = (5L * ICR1_TOP) / 100;

  // Activare Intrerupere Overflow
  TIMSK1 |= (1 << TOIE1);
}

/***** INTTRERUPERE 1: TIMER1 (Logica PWM + Trigger Transmisie) *****/
#pragma vector = TIMER1_OVF_vect
__interrupt void isr_TIMER1_OVF(void)
{
  static int cycles = 0;
  cycles++;
  
  // Asteptam sa treaca 27.8ms (278 overflow-uri)
  if (cycles >= perioade)
  {
    cycles = 0;

    // 1. Logica Ping-Pong
    if (procent >= 95) add = -5;
    if (procent <= 5)  add = 5;
    procent += add;

    // 2. Update Hardware PWM
    OCR1A = (long)procent * ICR1_TOP / 100;

    // 3. PREGATIRE TRANSMISIE (Fara a bloca timerul)
    // Scriem textul in bufferul global
    sprintf((char*)tx_buffer, "<FU=%d%%>\r\n", procent);
    
    // Resetam indexul de transmisie
    tx_index = 0;

    // 4. ACTIVAM POȘTAȘUL (Intreruperea UART Data Register Empty)
    // Asta va declanșa automat vectorul USART0_UDRE_vect
    UCSR0B |= (1 << UDRIE0);
  }
}

/***** INTTRERUPERE 2: UART (Transmisie efectiva) *****/
/* Aceasta ruleaza automat ori de cate ori UDR0 este gol
   si bitul UDRIE0 este 1 */
#pragma vector = USART0_UDRE_vect
__interrupt void isr_USART0_UDRE(void)
{
  // Verificam daca mai avem caractere in buffer
  if (tx_buffer[tx_index] != '\0')
  {
    // Trimitem caracterul curent si incrementam indexul
    UDR0 = tx_buffer[tx_index++];
  }
  else
  {
    // Am ajuns la finalul sirului ('\0').
    // OPRIM intreruperea UDRE ca sa nu consume procesor degeaba.
    UCSR0B &= ~(1 << UDRIE0);
  }
}

/***** MAIN *****/
int main(void)
{
  initUSART();
  initTimer1();

  __enable_interrupt();

  // Bucla infinita este GOALĂ.
  // Totul se intampla in background prin cele 2 intreruperi.
  while(1)
  {
    // Putem pune procesorul in sleep mode aici
    // __sleep();
  }
  
  return 0;
}
