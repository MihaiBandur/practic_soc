 // General 
#include <stdint.h> 
#include <inavr.h>  
#include <ioavr.h>


// Frecven?a oscilatorului 
#define F_OSC 16000000UL


// Baud Rate 
#define BAUD 9600 
#define BAUD_RATE (F_OSC / 16 / BAUD - 1) 


#define TRANSMITTER (1 << TXEN3)
#define RECEIVER (1 << RXEN3)



// Folosim pinul TX ca ie?ire 
#define TXD_OUT() DDRJ |= (1 << PJ1)

// Folosim pinul RX ca intrare 
#define RXD_IN() DDRJ |= (1 << PJ0)




// Func?ia de ini?ializare a modulului USART 
void usart_initialize(uint16_t baud_rate){
  UBRR3H = (uint8_t)(baud_rate >> 8);
  UBRR3L = (uint8_t)(baud_rate & 0XFF);
  
  //se porneste transmitatorul si receptorul 
  UCSR3B = TRANSMITTER | RECEIVER;
  
   // Se seteaza pinul TXD ca ie?ire  
   TXD_OUT(); 
   // Se seteaza pinul RXD ca intrare 
   RXD_IN();
    
    
}  
 
// Func?ia de transmitere USART 


 
/* 
 * Func?ia a?teapta pâna când buffer-ul de transmisie este gol (UDRE3 = 1) 
 * iar apoi scrie un octet în registrul UDR3 pentru a fi trimis pe linia 
 * seriala. 
 */ 
void usart_transmit(uint8_t data){
  while(!(UCSR3A & (1 << UDRE3)))
    ;
  
  UDR3 = data;
}
 
// Func?ia de recep?ie UART 

/* 
 * Func?ia a?teapta pâna când este recep?ionat un caracter (RXC3 = 1), iar  
 * apoi returneaza octetul citit din registrul UDR3. 
 */
uint8_t usart_receive(void){
  
  while(!(UCSR3A & (1 << RXC3)))
    ;
  
  return UDR3;
}


void main(void){
  uint8_t aux;
  
   usart_initialize(BAUD_RATE);
  
  
  while(1){
     aux = usart_receive();
     
     usart_transmit(aux);
  }
}