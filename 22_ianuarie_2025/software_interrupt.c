/*----------------------------------------------------------------------- 
 * Fi?ier: int0_ext_interrupt.c  
 * Utilizat pentru configurarea întreruperilor externe 
 *---------------------------------------------------------------------*/ 


#include <ioavr.h>
#include  <intrinsics.h> 
/* 
 * Func?ia configureaza întreruperea externa INT0 pe frontul pozitiv. 
 * Aceasta seteaza to?i pinii PORTD ca ie?ire ?i aprinde ini?ial toate  
 * LED-urile, dupa care activeaza întreruperile globale. 
 */ 
void init_interrupt_INT0(void){
   // Se seteaza to?i pinii PORTD ca ie?ire (LED-uri, test)
  DDRD = 0xFF;
  
   // Ini?ial, toate LED-urile sunt aprinse (logica inversa)
  PORTD = 0x00;
  
  // Se configureaza INT0 pe front pozitiv 
  EICRA |= (1 << ISC01) | (1 <<ISC00);
  // Se activeaza întreruperea INT0 
  EIMSK |= (1 << INT0);
  // Se activeaza întreruperile globale 
  __enable_interrupt();
  
  
}

#pragma vector = INT0_vect
 
/* 
 * Rutina de întrerupere asociata cu INT0. La declan?area întreruperii,  
 * variabila statica 'test' este incrementata. Aceasta variabila poate fi  
 * utilizata pentru test sau debug. 
 */ 


__interrupt void INT0_ISR(void){
  static unsigned char test = 0;
  
  test += 1;
}


void main(void){
  init_interrupt_INT0();
  
  // Bucla infinita pentru generarea unei întreruperi software 
    while (1) 
    { 
      PORTD = ~PORTD;   
    } 
  
  
}