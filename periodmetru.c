// Includes
#include <ioavr.h>
#include <inavr.h>
// Frecven?a 16 MHz
#define F_CPU 16000000UL
// Fara prescaler
#define PRESCALER 1UL
// Perioada masurata în microsecunde (µs)
volatile unsigned long perioada_us;
// Frecven?a
volatile unsigned long frecventa;
volatile unsigned long cnt_ovf = 0;
#pragma vector = TIMER1_OVF_vect
__interrupt void Timer1_Ovf_ISR(void){
    cnt_ovf++;
}

int main( void )
{
  /*
  * Se declara acest pas pentru a putea utiliza breakpoint-ul pentru a
  * vizualiza valorile din Watch
  */
  int i = 0;
  /*
  * Se seteaza pinul PF3 ca pin de intrare prin intermediul caruia
  * primim semnalul generat
  */
  DDRF &= ~(1 << PF3);
  // Numarul de ciclii
  unsigned long nr_cicli;
  TIMSK1 |= (1<<TOIE1);
  __enable_interrupt();
  while(1){
    // TCNT1 este oprit
    TCCR1A = 0;
    TCCR1B = 0;
    // Primul front e “sacrificat” pentru sincronizare
    while ((PINF & (1 << PF3)) == 0);
    while ((PINF & (1 << PF3)) != 0);
    // Se reseteaza valoarea contorului
    TCNT1 = 0;
    cnt_ovf = 0;
    // Fara prescaler
    TCCR1B |= (1 << CS10) ;
    
    // Se a?teapta frontul pozitiv
    while ((PINF & (1 << PF3)) == 0);
    // Se a?teapta apoi cel negativ
    while ((PINF & (1 << PF3)) != 0);
    //Se opre?te temporizatorul
    TCCR1B = 0;
    //Se salveaza valoarea contorului
    nr_cicli = TCNT1;
    nr_cicli += (cnt_ovf<<16);
    //Se calculeaza perioada în ms
    perioada_us = (nr_cicli * PRESCALER) / (F_CPU / 1000000UL);
    frecventa = 1000000UL / perioada_us;
    i++; // Aici se pune breakpoint pentru a vedea valorile în Watch
  }
  return 0;
}