/*--------------------------------------------------------------------------- 
 * Fi?ier: usart.c  
 * Utilizat pentru definirea func?iilor USART din usart.h 
 *-------------------------------------------------------------------------*/ 
 
/*--------------------------------------------------------------------------- 
 * Includes 
 *-------------------------------------------------------------------------*/ 
 
// General 
#include "usart.h" 

 
/*--------------------------------------------------------------------------- 
 * (Private) defines 
 *-------------------------------------------------------------------------*/ 
  
/* 
 * Acest cod este configurat pentru portul microbus 1 de pe placa, folosind  
 * controalele UART1 ale procesorului. 
 */ 
 
#define DATA_REGISTER_EMPTY     (UCSR1A & (1 << UDRE1)) 
#define SET_TRANSMITTER             (1 << TXEN1) 
#define SET_RECEIVER                (1 << RXEN1) 
#define SET_RX_COMPLETE             (1 << RXCIE1) 
#define SET_TX_COMPLETE             (1 << TXCIE1) 
#define SET_BAUD_H(BAUDRATE) UBRR1H = (uint8_t)(BAUDRATE >> 8) 
#define SET_BAUD_L(BAUDRATE) UBRR1L = (uint8_t)(BAUDRATE & 0xFF) 
 
/*--------------------------------------------------------------------------- 
 * Public variables   
 *-------------------------------------------------------------------------*/ 
 
round_buff_s tx_buffer; 
round_buff_s  rx_buffer; 
 
/*--------------------------------------------------------------------------- 
 * Private functions 
 *-------------------------------------------------------------------------*/ 
void usart_send_next() 
{ 
  if (!is_empty(&tx_buffer)) 
    { 
 
        // Se pune caracterul în registrul de date pentru a fi trimis 
        UDR1 = pop(&tx_buffer); 
    } 
} 
 
/*--------------------------------------------------------------------------- 
 * Public functions  
 *-------------------------------------------------------------------------*/ 
 
// Func?ie folosita la ini?ializarea modulului USART 
void USART_initialize(uint16_t baud_rate) 
{ 
  // Se configureaza baud rate-ul 
   tx_buffer.head = 0; 
   tx_buffer.tail = 0; 
   rx_buffer.head = 0; 
   rx_buffer.tail = 0; 
 
   SET_BAUD_H(baud_rate); 
   SET_BAUD_L(baud_rate);  
    
   /* 
    * Se porne?te transmi?atorul ?i receptorul, iar apoi se activeaza  
    * întreruperea la recep?ie ?i la transmisie. 
    */ 
 UCSR1B |= SET_TRANSMITTER | SET_RECEIVER | SET_RX_COMPLETE | SET_TX_COMPLETE; 
   // Se configureaza paritatea pe par 
    UCSR1C |= (1<<UPM11) | (1<<UCSZ11) | (1<<UCSZ10); 
   // Se seteaza pinul TXD: ie?ire 
    DDRD |= (1 << PD3); 
   // Se seteaza pinul RXD: intrare  
    DDRD &= ~(1 << PD2); 
} 
 
// Func?ie folosita la ransmiterea unui singur caracter 
uint16_t USART_transmit_char(uint8_t c) 
{ 
    // Se dezactiveaza întreruperile pentru acces sigur la buffer 
    __disable_interrupt();   
    uint16_t verif; 
 
    // Se verifica daca caracterul nu este terminator de ?ir 
    if(c != '\0')             
        // Se adauga caracterul în bufferul de transmisie 
        verif = push(&tx_buffer, c);   
 
    // Daca registrul de date este gol ?i caracterul a fost adaugat cu succes 
    if(DATA_REGISTER_EMPTY && verif != 0) 
        // Se trimite imediat caracterul urmator din buffer 
        usart_send_next();     
 
    // Se reactiveaza întreruperile 
    __enable_interrupt();      
    // Se returneaza starea opera?iei (0 = e?uat, 1 = reu?it) 
    return verif;              
}

// Func?ie folosita la transmiterea a mai multor caractere 
uint16_t USART_transmit_string(uint8_t *s, int16_t length) 
{ 
    // Se dezactiveaza întreruperile pentru acces sigur la buffer 
    __disable_interrupt();   
 
    // Se adauga ?irul de caractere în bufferul de transmisie 
    uint16_t verif = push_vec(&tx_buffer, s, length); 
 
    // Daca registrul de date e gol ?i caracterele au fost adaugate cu succes 
    if(DATA_REGISTER_EMPTY && verif != 0) 
        // Se trimite imediat primul caracter din buffer 
        usart_send_next();  
 
    // Se reactiveaza întreruperile 
    __enable_interrupt();      
 
    // Se returneaza numarul de caractere adaugate cu succes în buffer 
    return verif; 
} 
 
// Func?ie folosita pentru recep?ia a mai multor caractere 
uint16_t USART_receive_string(uint8_t *c, uint16_t length) 
{ 
    // Se dezactiveaza întreruperile pentru acces sigur la buffer 
    __disable_interrupt(); 
 
    uint16_t i; 
 
    // Se parcurge pâna la 'length' caractere (ceva evident în anul 3) 
    for(i = 0; i < length; i++) 
    { 
        // Se scoate un caracter din bufferul de recep?ie 
        uint8_t chr = pop(&rx_buffer); 
        // Daca bufferul e gol sau s-a ajuns la '\0', se opre?te citirea 
        if(chr == 0) 
            break; 
        // Se salveaza caracterul citit în ?irul furnizat de utilizator 
        c[i] = chr; 
    } 
    // Se reactiveaza întreruperile 
    __enable_interrupt(); 
 
    // Se returneaza numarul de caractere citite 
    return i; 
} 

uint16_t USART_receive_char(uint8_t *c)
{
    if (c == 0)
        return 0;

    // A?teapta pâna când apare un caracter în bufferul de recep?ie
    for (;is_empty(&rx_buffer);)
    {
        // func?ie blocanta
        // se a?teapta ca ISR-ul RX sa introduca date
    }

    // Acces sigur la buffer
    __disable_interrupt();

    // Extrage caracterul din buffer
    *c = pop(&rx_buffer);

    __enable_interrupt();

    return 1;   // caracter recep?ionat cu succes
}

 
/* 
 * Întrerupere pentru "Data Register Empty" (UDRIE), executata când   
 * transmi?atorul este gata sa trimita un nou caracter. 
 */ 
#pragma vector = USART1_TX_vect 
__interrupt void USART1_TX_ISR(void) 
{ 
    // Se trimite urmatorul caracter din bufferul de transmisie 
    usart_send_next(); 
} 
/*
 * Întrerupere pentru "Receive Complete" (RXC), executata atunci când a sosit  
 * un caracter în registrul de recep?ie. Se preia caracterul din UDR1 ?i se  
 * stocheaza în bufferul de recep?ie. 
 */ 
#pragma vector = USART1_RX_vect 
__interrupt void USART1_RX_ISR(void) 
{ 
    // Se cite?te caracterul primit 
    uint8_t received_char = UDR1; 
    // Se adauga caracterul în bufferul de recep?ie  
    push(&rx_buffer, received_char); 
} 