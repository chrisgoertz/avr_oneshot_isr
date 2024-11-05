/*
 * avr_oneshot_isr.c
 *
 * Created: 05.11.2024 22:18:56
 * Author : chris.goertz@gmx.de
 *
 * This is a quick and dirty example project to demonstrate the 
 * debouncing of an external interrupt signal.
 *
 *
 *                     GNU GENERAL PUBLIC LICENSE
 *                       Version 3, 29 June 2007

 * Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
 * Everyone is permitted to copy and distribute verbatim copies
 * of this license document, but changing it is not allowed.
 */


#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// Systick library
// https://github.com/chrisgoertz/avr_systick
#include "inc/avr_systick/systick.h"

//function prototypes
void interval_task(uint32_t rt);

// variables
volatile bool isr_flag = false;

/**
* Interrupt vector of External interrupt 0.
* Triggered asynchronously on falling edge on PIND2.
*/
ISR(INT0_vect)
{
	// set flag
	isr_flag = true;
	// now disarm the interrupt to 
	// prevent signal bouncing.
	EIMSK &= (1<<INT0);
}

int main(void)
{
	// Use of PORTA.1 and PORTA0 as indicators
	// So configure them as OUTPUT.
	DDRA |= (1<<PORTA1) | (1<<PORTA0);
	// activate PULLUPS for PORTA
	PORTA |= 0xFF;
	
	// Configure External interrupt 0 to trigger
	// on falling edge.
	EICRA |= (1<<ISC01);
	// ARM External interrupt 0.
	EIMSK |= (1<<INT0);
	// Enable interrupts in SREG.
	sei();
	
	// Init systick stuff
	uint32_t runtime = 0;
	systick_init();
    
	// Forever...
	while (1) 
    {
		runtime = systick_getTicks();
		interval_task(runtime);
    }
}

/**
* 500ms interval task
* @param rt: uint32_t runtime now
*/
void interval_task(uint32_t rt)
{
	// vars
	static uint32_t isr_debounce = 0;	
	const uint32_t DELTA = 500;
	static uint32_t interval = 0;
	
	// if isr_flag is set, store runtime
	// and reset flag
	if(isr_flag)
	{
		isr_debounce = rt;
		isr_flag = false;
	}
	
	// if debounce time has elapsed,
	// rearm the interrupt.
	if((rt - isr_debounce) > 2000)
	{
		isr_debounce = 0;
		EIMSK |= (1<<INT0);
		
		// Show status [INT0 enabled]
		PORTA &= ~(1<<PORTA1);
	}	
	else
	{
		// Show status [INT0 disabled]
		PORTA |= (1<<PORTA1);
	}
	
	// check if interval has elapsed
	if((rt - interval) < DELTA)
	{
		// not elapsed -> nothing to do.
		return;
	}
	interval = rt;
	
	// All the interval stuff below.
	// Everything that has to be dealed with
	// every 500ms.
	
	// toggle led as indicator
	PORTA ^= (1<<PORTA0);
	
}
