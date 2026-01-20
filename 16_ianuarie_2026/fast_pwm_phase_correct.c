// Includes
#include <ioavr.h>
#include <inavr.h>
 
void set_timer_0(void){
    /*
   * TCNT0 - canal A ?i B
   * Se reseteaza regi?trii TCNT0
   */
  TCCR0A = 0;
  TCCR0B = 0;
   
  // Se seteaza pe activ canalul A ?i modul de operare Fast PWM
  TCCR0A |=  (1 << WGM00);
  TCCR0B |= (1 << WGM02);
 
 
    // Se seteaza pe activ canalul B
  TCCR0A |= (1 << COM0B1);
   
  // Se selecteaza un prescaler clkIO/256  
  TCCR0B |= (1 << CS02);
}

void set_duty_cycle(void){
    // Duty-cycle de 50% pe ambele canale
  OCR0A = 120;
  OCR0B = 84;
}

int main( void )
{
  // Pinii de ie?ire
  DDRB |= (1 << PB7) | (1 << PB5);
  DDRG |= (1 << PG5);
   
  set_timer_0();
  set_duty_cycle();
   
  /*
   * TCNT0 - canal A ?i B
   * Se reseteaza regi?trii TCNT0
   */
  //TCCR1A = 0;
  //TCCR1B = 0;
   
  // Se seteaza TCNT1 în modul Fast PWM 8 bit ?i se activeaza canalul C
  //TCCR1A |= (1 << WGM10) | (1 << COM1C1);
   
  // Se activeaza canalul A
  //TCCR1A |= (1 << COM1A1);
   
  // Se selecteaza prescaler clkIO/8
  //TCCR1B |= (1 << CS11) | (1 << WGM12);
 
  while(1){
  }
 
  return 0;
}