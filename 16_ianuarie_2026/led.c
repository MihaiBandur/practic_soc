/*-----------------------------------------------------------------------
* Fi?ier: exemplu_led.c
* Utilizat pentru exemplificarea controlarii LED-ului A
*---------------------------------------------------------------------*/
// Includes
#include <inavr.h>
#include <ioavr.h>
int main(void)
{
// Se activeaza PA5 ca ie?ire din ATmega1280
  DDRA |= (1 << PA5);
// Se ini?ializeaza PA5 cu valoarea 0
  PORTA &= ~(1 << PA5);
  while(1)
  {
// Se comuta valoarea pinului PA5
    PORTA ^= (1 << PA5);
    __delay_cycles(16000000);
  }
}