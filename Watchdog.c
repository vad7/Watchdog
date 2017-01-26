/*
 * Watchdog.c
 *
 * Created: 18.01.2017 17:04:47
 *  Author: Vadim Kulakov, vad7@yahoo.com
 * 
 * ATTiny4/5/9/10
 *
 * Connections:
 * pin 1 (PB0) -> to RESET external mk
 * pin 4 (PB2) <- input signal to clear timer
 */ 
#define F_CPU 128000UL
// Fuses: 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#define WATCHDOG_TIMEOUT 600 //  *0.1 sec

#define EXT_RESET		(1<<PINB0)
#define EXT_RESET_ON	DDRB |= EXT_RESET	// out, low
#define EXT_RESET_OFF	DDRB &= ~EXT_RESET	// in
/* Pullup RESET pin (20kOm):
#define EXT_RESET_ON	{ PUEB &= ~EXT_RESET; DDRB |= EXT_RESET; } // out, low
#define EXT_RESET_OFF	{ DDRB &= ~EXT_RESET; PUEB |= EXT_RESET; } // in, pullup
*/

uint16_t watchdog_timer = 0;

ISR(INT0_vect)
{
	watchdog_timer = 0;
}

ISR(TIM0_OVF_vect) // 0.1 s
{
	if(++watchdog_timer == WATCHDOG_TIMEOUT) {
		EXT_RESET_ON;
	} else {
		if(watchdog_timer > WATCHDOG_TIMEOUT) watchdog_timer = 0;
		EXT_RESET_OFF;
	}
}

int main(void)
{
	PUEB = (1<<PORTB1); // pullup unused pins
	EXT_RESET_OFF;
	CCP=0xD8; CLKMSR = (0<<CLKMS0) | (1<<CLKMS0); // Internal 128 KHz oscillator
	CCP=0xD8; CLKPSR = (0<<CLKPS3) | (0<<CLKPS2) | (0<<CLKPS1) | (0<<CLKPS0); // Clock prescaler division factor: 1
	CCP=0xD8; WDTCSR = (1<<WDE) | (0<<WDP3) | (1<<WDP2) | (1<<WDP1) | (0<<WDP0); //  Watchdog Reset 1s
	EICRA = (0<<ISC01) | (1<<ISC00); // Any logical change on INT0 generates an interrupt
	EIMSK = (1<<INT0); // INT0: External Interrupt Request 0 Enable
	TCCR0A = (1<<WGM01) | (1<<WGM00);  // Timer0: Fast PWM OCR0A  
	TCCR0B = (1<<WGM03) | (1<<WGM02) | (0<<CS02) | (0<<CS01) | (1<<CS00); // Timer0 prescaller: 1
	TIMSK0 = (1<<TOIE0); // Timer/Counter0 Overflow Interrupt Enable
	OCR0A = 12799; // OC0A(TOP)=Fclk/prescaller/Freq - 1; Freq=Fclk/(prescaller*(1+TOP))
	SMCR = (1<<SE); // Sleep Idle enable
	sei();
    while(1)
	{
		wdt_reset();
		sleep_cpu();
    }
}