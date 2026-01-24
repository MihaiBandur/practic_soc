// Includes
#include <ioavr.h>
#include <inavr.h>
#include <stdint.h>
 

#define F_CPU 16000000UL
#define PRESCALER 256

void reset_control_register(void){
  //Se reseteaza regi?trii TCNT0
  TCCR0A = 0;
  TCCR0B = 0;
}
void set_pin_for_timer(void){
  DDRG |= (1 << PG5);  
}

void set_timer_0(void){
   
  //se seteaza modul phase correct  pe timer-ul zero   
  TCCR0A |= (1 << WGM00);
  TCCR0B |= (1 << WGM02);
 
 
  // Se seteaza pe activ canalul B
  TCCR0A |= (1 << COM0B1);
   
  // Se selecteaza un prescaler clkIO/256  
  TCCR0B |= (1 << CS02);
}

void set_frequency(uint32_t pwm_freq)
{
    uint16_t top;

    top = (F_CPU / (2UL * PRESCALER * pwm_freq));

    if (top > 255)
        top = 255;   // limitare pentru Timer0 (8-bit)

    OCR0A = (uint8_t)top;
}
void set_duty_cycle(uint8_t duty_percent)
{
    if (duty_percent > 100)
        duty_percent = 100;

    OCR0B = (uint8_t)((duty_percent * OCR0A) / 100);
}

void main( void )
{
  reset_control_register();
  set_pin_for_timer();
  set_timer_0(); 
  
  
   set_frequency(500);   // 500 Hz
   set_duty_cycle(35);   // 35%
   
  
 
  while(1){
    ;
  }
 
}