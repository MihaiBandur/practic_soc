/*-----------------------------------------------------------------------
* Fi?ier: exemplu_buton.c
* Utilizat pentru aprinderea LED-ului PA5 la apasarea butonului T1
*---------------------------------------------------------------------*/
// Includes
#include <inavr.h>
#include <ioavr.h>
int main(void)
{
// Se activeaza PA5 ca ie?ire din ATmega1280
  DDRA |= (1<<PA5);
// PB6 ca intrare pentru a se citi butonul
  DDRE &= ~(1 << PE6);
// Se activeaza rezisten?a interna de pull-up pe PB6
  PORTE |= (1 << PE6);
  while(1)
    {
// Daca butonul T1 este apasat (LOW)
      if (!(PINE & (1 << PE6)))
         {
// Se aprinde LED-ul
        PORTA |= (1 << PA5);
         }
     else
         {
// Se stinge LED-ul
        PORTA &= ~(1 << PA5);
         }
   }
}