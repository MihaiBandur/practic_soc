#include<ioavr.h>
#include<inavr.h>
#include<intrinsics.h>
#include<stdint.h>

#define FREC 16000000
#define BAUD 9600
#define BAUD_RATE (FREC / 16 / BAUD - 1)

#define set_rx_intrerupt ( 1 << RXCIE2 )
#define set_udre_intrerupt ( 1 << UDRIE2 )


#define RECEIVER (1 << RXEN2 )
#define TRANSMITTER (1 << TXEN2 )

#define RX_IN (1 << PH0)
#define TX_OUT (1 << PH1)

volatile unsigned char data;
volatile unsigned char flag_message_received;
void initialise_usart3(uint16_t baud_rate){
  UBRR2H = (uint8_t)(baud_rate >> 8);
  UBRR2L = (uint8_t)(baud_rate & 0xFF);
  
  UCSR2B |= TRANSMITTER | RECEIVER |set_udre_intrerupt | set_rx_intrerupt;
    
   DDRH &= ~RX_IN;
   DDRH |= (TX_OUT); 
}


#pragma vector = USART2_RX_vect
__interrupt void USART_RX_ISR(void){
  UCSR2B &= ~set_rx_intrerupt;
  flag_message_received = 1;
  data = UDR2;
  
}
#pragma vector = USART2_UDRE_vect
__interrupt void USART1_TX_ISR(void){
  if(flag_message_received == 1){
    flag_message_received = 0;
    UDR2 = data;
    UCSR2B |= set_rx_intrerupt;
  }
}

void main(){
  initialise_usart3(BAUD_RATE);
  __enable_interrupt();
  
  while(1){
    
  }
  
}