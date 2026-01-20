/*-------------------------------------------------------------------------- 
 * Fi?ier: round_buff.c 
 * Utilizat pentru definirea func?iilor ?i structurilor din round_buff.h 
 *------------------------------------------------------------------------*/ 
 
/*--------------------------------------------------------------------------  
 * Includes 
 *------------------------------------------------------------------------*/ 
 
// Include fi?ierul de header pentru buffer circular 
#include "round_buff.h" 
 
/*--------------------------------------------------------------------------  
 * Public functions   
 *------------------------------------------------------------------------*/ 
// Func?ie folosita pentru adaugarea unui caracter în bufferul circular 
int16_t push(round_buff_s *inBuffer, uint8_t data) 
{ 
    /* 
     * Se calculeaza pozi?ia urmatoare a indicelui head folosind opera?ia  
     * modulo pentru comportament circular. 
     */ 
    uint8_t next_head = (inBuffer->head + 1) % BUFFER_SIZE; 
 
    /* 
     * Daca bufferul nu este plin, se insereaza caracterul ?i 
     * se actualizeaza indicele head. 
     */ 
    if(!is_full(inBuffer)) 
    { 
        inBuffer->buffer[inBuffer->head] = data; 
        inBuffer->head = next_head; 
 
        // Returneaza 1 pentru succes 
        return 1; 
    } 
    else 
        // Returneaza 0 daca bufferul este plin 
        return 0; 
} 
 
// Func?ie folosita pentru extragerea unui caracter din buffer 
uint8_t pop(round_buff_s *inBuffer) 
{ 
    /* 
     * Daca bufferul nu este gol, se preia caracterul din pozi?ia tail 
     * ?i se actualizeaza indicele tail pentru comportament circular. 
     */ 
    if(!is_empty(inBuffer)) 
    { 
        uint8_t data = inBuffer->buffer[inBuffer->tail]; 
        inBuffer->tail = (inBuffer->tail + 1) % BUFFER_SIZE; 
        // Returneaza caracterul extras 
        return data; 
    } 
    else 
    { 
        // Returneaza 0 daca bufferul este gol 
        return 0; 
    } 
} 
 
// Func?ie folosita pentru adaugarea unui ?ir de caractere în buffer 
int16_t push_vec(round_buff_s *inBuffer, uint8_t data[], int16_t length) 
{  
    int16_t i; 
    /* 
     * Se parcurge fiecare caracter din ?ir ?i se încearca adaugarea lui. 
     * În caz ca bufferul se umple, se opre?te inserarea. 
     */ 
    for(i = 0; i < length; i++) 
    { 
        uint8_t verif = push(inBuffer, data[i]); 
        if(verif == 0) 
            break; 
    } 
    // Returneaza numarul de caractere adaugate cu succes 
    return i; 
} 
 
// Func?ie folosita pentru verificarea daca bufferul este gol 
int16_t is_empty(round_buff_s *inBuffer) 
{ 
    return (inBuffer->head == inBuffer->tail); 
} 
 
// Func?ie folosita pentru verificarea daca bufferul este plin 
int16_t is_full(round_buff_s *inBuffer) 
{ 
    return ((inBuffer->head + 1) % BUFFER_SIZE) == inBuffer->tail; 
} 