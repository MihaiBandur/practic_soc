#include <ioavr.h>
#include <inavr.h>
#include <stdint.h>
#include <intrinsics.h>

#define F_CPU 16000000UL
#define TIMER5_PRESCALER 8

volatile uint8_t set = 0;


void set_pin_for_output(void){
    DDRE |= (1 << PE5);
}

void reset_control_register(void){
    TCCR5A = 0;
    TCCR5B = 0;
    TCNT5  = 0;
} 


void timer5_set_frequency(uint32_t freq)
{
    uint32_t top;

    if (freq == 0)
        return;

    top = (F_CPU / (TIMER5_PRESCALER * freq)) - 1;

    if (top > 65535)
        top = 65535;

    __disable_interrupt();
    OCR5A = (uint16_t)top;
    __enable_interrupt();
}

 void timer5_set_duty(uint8_t duty, char channel)
{
    uint16_t value;

    if (duty > 100)
        duty = 100;

    value = ((uint32_t)duty * OCR5A) / 100;

    __disable_interrupt();

    switch (channel)
    {
        case 'B':
        case 'b':
            OCR5B = value;
            break;

        case 'C':
        case 'c':
            OCR5C = value;
            break;
    }

    __enable_interrupt();
}


void timer5_mode15_init(void)
{
    // Mode 15 – Fast PWM, TOP = OCR5A
    TCCR5A |= (1 << WGM51) | (1 << WGM50);
    TCCR5B |= (1 << WGM53) | (1 << WGM52);

    // Enable OC5B / OC5C
    TCCR5A |= (1 << COM5C1) | (1 << COM5B1);

    // Prescaler = 8
    TCCR5B |= (1 << CS51);

    // Setari initiale
    timer5_set_frequency(1000);   // 1 kHz
    timer5_set_duty(50, 'B');     // 50%
    timer5_set_duty(25, 'C');     // 25%

    TIMSK5 |= (1 << TOIE5);
}





#pragma vector = TIMER5_OVF_vect
__interrupt void Timer5_Overflow_ISR(void)
{
    PORTE ^= (1 << PE5);   // pentru osciloscop

    if (set == 0)
    {
        timer5_set_frequency(5000);
        set = 1;
    }
    else
    {
        timer5_set_frequency(1000);
        set = 0;
    }
}


int main(void)
{
    set_pin_for_output();

    reset_control_register();
    timer5_mode15_init();

    __enable_interrupt();

    while (1)
        ;
}