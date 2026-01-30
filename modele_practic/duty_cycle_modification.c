#include <ioavr.h>
#include <intrinsics.h>
#include <stdio.h> // Pentru sprintf (optional, dar util)

/***** Variabile globale *****/
volatile int procent = 5;       // volatile e OBLIGATORIU cand modifici in ISR si citesti in Main
volatile int add = 5;
volatile char update_needed = 0; // "Stegulet" pentru main

/* Matematica ta e corecta pentru Prescaler 1:
   F_CPU = 16MHz, Prescaler = 1 => Tick = 62.5ns
   Pt 10kHz => 16.000.000 / 10.000 = 1600 tick-uri totale.
   ICR1 = 1599 (0..1599 = 1600 pasi).
   Overflow la fiecare 100µs.
   Delay necesar intre pasi: 27.8ms.
   Overflow-uri necesare: 27.8ms / 0.1ms = 278.
*/
int perioade = 278;

/***** Rutina de întrerupere Timer1 Overflow *****/
#pragma vector = TIMER1_OVF_vect
__interrupt void isr_TIMER1_OVF(void)
{
  static int cycles = 0;
  cycles++;
  
  // Numărăm până trece timpul necesar (27.8ms)
  if (cycles >= perioade)
  {
    cycles = 0;

    // Logica de Ping-Pong (5% <-> 95%)
    if (procent >= 95) add = -5;
    if (procent <= 5)  add = 5;

    procent += add;

    // Actualizăm PWM-ul (Asta e rapid, se poate face in ISR)
    // Formula: (procent * 1600) / 100. Aproximam ICR1 la 1600 pt simplitate matematica
    // Sau exact: (procent * 1599) / 100
    OCR1A = (long)procent * 1599 / 100;

    // RIDICAM STEAGUL! Nu transmitem aici.
    update_needed = 1;
  }
}

/***** Funcții UART (Adaptate pentru ATmega1280) *****/
void initUSART(void)
{
  // 9600 baud la 16MHz
  UBRR0H = 0;
  UBRR0L = 103;
  UCSR0B = (1 << TXEN0) | (1 << RXEN0);  // Enable TX
  UCSR0C = (1 << USBS0) | (3 << UCSZ00); // 8 biti, 1 stop
}

void putChar(char c)
{
  while (!(UCSR0A & (1 << UDRE0))); // Asteapta buffer gol
  UDR0 = c;
}

void sendFU(int val)
{
  char buffer[20];
  // Scriem manual sau cu sprintf
  sprintf(buffer, "<FU=%d%%>\r\n", val);
  
  char *p = buffer;
  while(*p) putChar(*p++);
}

/***** Funcția principală *****/
int main(void)
{
  // 1. Configurare Pin OC1A (PB5 la ATmega1280 !!)
  DDRB |= (1 << PB5);

  initUSART();

  // 2. Configurare Timer1 (Fast PWM, ICR1 top)
  // WGM13:0 = 1110 (Mode 14)
  TCCR1A = (1 << WGM11) | (1 << COM1A1); // COM1A1 = Non-inverting
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10); // CS10 = Prescaler 1

  // 3. Setare Frecventa 10kHz
  ICR1 = 1599;

  // 4. Setare Duty Cycle initial
  OCR1A = (5L * 1599) / 100;

  // 5. Activare Intrerupere Overflow (TIMSK1 la 1280!)
  TIMSK1 |= (1 << TOIE1);

  __enable_interrupt();

  while(1)
  {
    // Verificăm dacă ISR-ul ne-a spus să transmitem
    if (update_needed == 1)
    {
       sendFU(procent); // Transmitem liniștiți, fără să blocăm timer-ul
       update_needed = 0; // Coborâm steagul
    }
  }

  return 0;
}
