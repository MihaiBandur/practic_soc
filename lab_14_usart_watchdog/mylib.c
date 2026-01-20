/*-------------------------------------------------------------------------- 
 * Fi?ier: mylib.c  
 * Utilizat pentru definirea func?iei my_print 
 *------------------------------------------------------------------------*/ 
 
/*--------------------------------------------------------------------------  
 * Includes 
 *------------------------------------------------------------------------*/ 
 
// General 
#include "mylib.h" 
 
/*-------------------------------------------------------------------------- 
 * Private (static) variables 
 *------------------------------------------------------------------------*/ 
 
// Buffer auxiliar pentru construirea ?irului de caractere 
static uint8_t aux1[10] = {0}, aux2[10] = {0}; 
 
/*-------------------------------------------------------------------------- 
 * Private functions    
 *------------------------------------------------------------------------*/ 
 
// Func?ia de transmitere a unui numar întreg pe seriala 
void integerTransmit(void *p) 
{  
    // Se ini?ializeaza indicii pentru construc?ia ?irului de caractere 
    int16_t index = 0, i = 0; 
    
    // Se preia valoarea întreaga din pointer 
    int16_t x = (*(int16_t *)(p));  
 
    // Se trateaza cazul numerelor negative 
    if(x < 0)  
    {  
        aux1[index] = '-';  // Se adauga semnul minus 
        index++; 
        x *= (-1);          // Se converte?te la valoare pozitiva 
    }  
 
    // Se extrag cifrele numarului în ordine inversa 
    do 
    {  
        // Se converte?te cifra la caracter ASCII 
        uint8_t c = x % 10 + '0'; 
        // Se stocheaza temporar în buffer auxiliar  
        aux2[i] = c;               
        i++;  
 
        // Se reduce numarul 
        x = x / 10;                
    } while(x != 0); 
 
 
    // Se copiaza cifrele în ordinea corecta în bufferul final 
    for(int16_t j = i - 1; j >= 0; j--) 
    { 
        aux1[index] = aux2[j]; 
        index++; 
    } 
 
    // Se adauga caracterele de sfâr?it de linie 
    aux1[index] = '\n'; 
    index++; 
    aux1[index] = '\r'; 
    index++; 
      
    // Se transmite ?irul format catre modulul USART 
    USART_transmit_string(aux1, index); 
}  
 
// Func?ia de transmitere a unui numar hexazecimal pe seriala 
void hexadecimalTransmit(void *p)  
{ 
    // Se preia valoarea întreaga din pointer 
    int16_t x = *((int16_t *)(p)); 
    int16_t index = 0, i = 0; 
 
    // Se adauga prefixul "0x" pentru format hexazecimal 
    aux1[index] = '0'; 
    index++; 
    aux1[index] = 'x'; 
    index++; 
 
    // Se extrag cifrele hexazecimale în ordine inversa 
    do 
    { 
        uint8_t a = x & 0x0F;  // Se preiau cei 4 bi?i cei mai mici 
      
        // Se converte?te valoarea la caracter ASCII 
        if(a <= 9) 
        { 
            aux2[i] = a + '0'; 
        } 
        else 
        { 
            aux2[i] = a + 'A' - 10; 
        } 
        i++; 
        x >>= 4;  // Se face shiftare pentru urmatoarea cifra 
    } while(x != 0); 
 
    // Se copiaza cifrele în ordinea corecta 
    for(int16_t j = i - 1; j >= 0; j--) 
    { 
        aux1[index] = aux2[j];
                index++; 
    } 
    // Se transmite ?irul format catre modulul USART 
    USART_transmit_string(aux1, index); 
} 
 
// Func?ia de transmitere a unui numar de tip double pe seriala 
void doubleTransmit(void *p) 
{ 
    // Se preia valoarea double din pointer 
    double x = (*(double *)(p)); 
    int16_t index = 0; 
    int16_t i = 0; 
 
    // Se trateaza cazul numerelor negative 
    if (x < 0) 
    { 
        aux1[index++] = '-'; 
        x = -x; 
    } 
 
    // Se separa partea întreaga de partea frac?ionara 
    int16_t int_part = (int16_t) x; 
    double frac = x - int_part; 
    // Se pastreaza doua zecimale 
    int16_t frac_part = (int16_t)(frac * 100 + 0.5);  
 
    // Se proceseaza partea întreaga 
    if (int_part == 0) 
    { 
        aux1[index++] = '0'; 
    } 
    else 
    { 
        while (int_part != 0) 
        { 
            // Se converte?te cifra la caracter ASCII 
            uint8_t c = int_part % 10 + '0'; 
            // Se stocheaza temporar în buffer auxiliar   
            aux2[i++] = c; 
            // Se reduce valoarea                     
            int_part /= 10;                    
        } 
        // Se copiaza cifrele în ordinea corecta 
        for (int16_t j = i - 1; j >= 0; j--) 
        { 
            aux1[index++] = aux2[j]; 
        } 
    } 
 
    // Se adauga separatorul zecimal 
    aux1[index++] = '.'; 
 
    i = 0; 
 
    // Se proceseaza partea frac?ionara 
    if (frac_part == 0) 
    { 
        aux1[index++] = '0'; 
        aux1[index++] = '0'; 
    }
      else 
    { 
        if (frac_part < 10) 
        { 
            aux1[index++] = '0';  // Se adauga zeroul pentru o cifra 
        } 
        // Se extrag cifrele frac?ionare în ordine inversa 
        while (frac_part != 0) 
        { 
            uint8_t c = frac_part % 10 + '0'; 
            aux2[i++] = c; 
            frac_part /= 10; 
        } 
 
        // Se copiaza cifrele în ordinea corecta 
        for (int16_t j = i - 1; j >= 0; j--) 
        { 
            aux1[index++] = aux2[j]; 
        } 
    } 
 
    // Se adauga caracterele de sfâr?it de linie 
    aux1[index++] = '\n'; 
    aux1[index++] = '\r'; 
 
    // Se transmite ?irul format catre modulul USART 
    USART_transmit_string(aux1, index); 
} 
 
// Se transmite un caracter pe seriala 
void characterTransmit(void *p) 
{ 
    // Se preia caracterul din pointer 
    int8_t x = (*(int8_t *)(p)); 
    int16_t index = 0; 
 
    // Se adauga caracterul în buffer 
    aux1[index++] = x; 
 
    // Se transmite caracterul catre modulul USART 
    USART_transmit_string(aux1, index); 
} 
 
/*-------------------------------------------------------------------------- 
 * (Public) Functions (the ones from .h) 
 *------------------------------------------------------------------------*/ 
 
// Func?ie de tip wrapper prin care se transmit valori de diverse tipuri 
void my_print(Tipuri tip, void *val)  
{  
    switch(tip)  
    {  
        case INTEGER:  
            // Se transmite un numar întreg 
            integerTransmit(val);  
            break;     
        case HEX:  
            // Se transmite un numar hexazecimal 
            hexadecimalTransmit(val);  
            break;
                    case DOUBLE:  
            // Se transmite un numar double 
            doubleTransmit(val);  
            break;  
  
 
        case CHAR:  
            // Se transmite un caracter 
            characterTransmit(val); 
            break;  
    }   
} 