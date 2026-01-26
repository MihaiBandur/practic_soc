// Includes
#include <ioavr.h>
#include <inavr.h>
#include <stdint.h>
// Frecven?a 16 MHz
#define F_CPU 16000000UL
// Fara prescaler
#define PRESCALER 1UL
// Perioada masurata în microsecunde (µs)


#define FREC 16000000
#define BAUD 9600
#define BAUD_RATE (FREC / 16 / BAUD - 1)

#define set_rx_intrerupt ( 1 << RXCIE2 )
#define set_udre_intrerupt ( 1 << UDRIE2 )


#define RECEIVER (1 << RXEN2 )
#define TRANSMITTER (1 << TXEN2 )

#define RX_IN (1 << PH0)
#define TX_OUT (1 << PH1)

volatile unsigned long perioada_us;
// Frecven?a
volatile unsigned long frecventa;
volatile unsigned long cnt_ovf = 0;
#pragma vector = TIMER1_OVF_vect
__interrupt void Timer1_Ovf_ISR(void){
    cnt_ovf++;
}


void initialise_usart3(uint16_t baud_rate){
  UBRR2H = (uint8_t)(baud_rate >> 8);
  UBRR2L = (uint8_t)(baud_rate & 0xFF);
 
    
   DDRH &= ~RX_IN;
   DDRH |= (TX_OUT); 
}

volatile char data[64];
volatile int index = 0;
#pragma vector = USART2_UDRE_vect
__interrupt void USART1_TX_ISR(void){
  if(data[index] != '\0'){
    
    UDR2 = data[index];
    index++;
  }
}

void ul_to_dec(unsigned long value, char *buffer){
  char temp[11];   // max 10 cifre pentru uint32 + '\0'
  uint8_t i = 0, j = 0;

  if(value == 0){
    buffer[0] = '0';
    buffer[1] = '\0';
    return;
  }

  // extrage cifrele invers
  while(value > 0){
    temp[i++] = (value % 10) + '0';
    value /= 10;
  }

  // le pune în ordinea corecta
  while(i > 0){
    buffer[j++] = temp[--i];
  }

  buffer[j] = '\0';
}

void build_freq_message(unsigned long perioada,
                        unsigned long frecventa,
                        char *buffer){
  uint8_t i = 0;

  buffer[i++] = 'T';
  buffer[i++] = ' ';
  buffer[i++] = '=';
  buffer[i++] = ' ';

  ul_to_dec(perioada, &buffer[i]);
  while(buffer[i] != '\0') i++;

  buffer[i++] = ' ';
  buffer[i++] = 'u';
  buffer[i++] = 's';
  buffer[i++] = ' ';
  buffer[i++] = '|';
  buffer[i++] = ' ';
  buffer[i++] = 'F';
  buffer[i++] = ' ';
  buffer[i++] = '=';
  buffer[i++] = ' ';

  ul_to_dec(frecventa, &buffer[i]);
  while(buffer[i] != '\0') i++;

  buffer[i++] = ' ';
  buffer[i++] = 'H';
  buffer[i++] = 'z';
  buffer[i++] = '\r';
  buffer[i++] = '\n';
  buffer[i++] = '\0';
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
  
  initialise_usart3(BAUD_RATE);
  DDRF &= ~(1 << PF3);
  
  // Numarul de ciclii
  unsigned long nr_cicli;
  TIMSK1 |= (1<<TOIE1);
  int steps = 0;
  __enable_interrupt();
  while(1){
    
    if(steps == 10)
      break;
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
    build_freq_message(perioada_us, frecventa, (char *)data);
   
    steps++;
    
    
    
    
  }
  UCSR2B |= TRANSMITTER | set_udre_intrerupt;
  while(1){
    ;
  }
  return 0;
}