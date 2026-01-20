/*-------------------------------------------------------------------------- 
 * Fi?ier: mylib.h  
 * Utilizat pentru declararea func?iei my_print 
 *------------------------------------------------------------------------*/ 
 
#ifndef __MYLIB__  
#define __MYLIB__ 
  
/*-------------------------------------------------------------------------- 
 * Includes 
 *------------------------------------------------------------------------*/ 
 
// Compiler 
#include <stdint.h> 
#include <inavr.h>  
#include <ioavr.h> 
 
// General 
#include "usart.h" 
 
/*-------------------------------------------------------------------------- 
 * Data structures 
 *------------------------------------------------------------------------*/ 
typedef enum Tip { 
  INTEGER, 
  HEX, 
  DOUBLE, 
  CHAR 
} Tipuri; 
 
/*-------------------------------------------------------------------------- 
 * Public (exported) functions  
 *------------------------------------------------------------------------*/ 
/* 
 * Func?ia de print care folose?te USART 
 * Tip define?te tipul variabilei: 0 = int, 1 = hexa, 2 = double, 3 = char 
 */ 
void my_print(Tipuri tip, void *val); 
 
#endif 