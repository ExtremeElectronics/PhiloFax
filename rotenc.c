/**
 * Part of PhiloFax https://github.com/ExtremeElectronics/PhiloFax
 *
 * rotenc.c
 *
 * Code to implement a Rotational Encoder - Copyright (c) 2024 Derek Woodroffe <tesla@extremeelectronics.co.uk>
 * on a Pi PicoW
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "stdlib.h"

#include "graphics/graphics.h"

/*Encoder GPIO*/

#define ENC_A	6
#define ENC_B	7
#define ENC_SW	22

uint8_t a=0;
uint8_t b=0;
uint8_t old_a=0;
uint8_t old_b=0;
uint8_t ta=0;
uint8_t tb=0;
uint8_t old_ta=0;
uint8_t old_tb=0;

uint8_t jog=0;
//uint8_t jogmax=;

struct repeating_timer etimer;

extern uint8_t dest;
extern uint8_t maxdestinations;

extern void JogChange(uint8_t jog);

bool Encoder_Timer_Callback(uint gpio, uint32_t events) {
	a=!gpio_get(ENC_A);
	b=!gpio_get(ENC_B);	

	ta=0;
	tb=0;

	if(a && !b && !old_a){ta=1;}
	
	if(b && !a && !old_b){tb=1;}
	
	if(!old_tb && ta){
	   if (jog>0) jog--;
	   JogChange(jog);
	}
	
	if(!old_ta && tb){
	   if (jog<maxdestinations) jog++;
	   JogChange(jog);
	}
		
	old_a=a;
	old_b=b;
	old_ta=ta;
	old_tb=tb;
	
	return 1;
	
}


int RotEncSetup(){
    // GPIO Setup for Encoder
    gpio_init(ENC_SW);				
    gpio_set_dir(ENC_SW,GPIO_IN);
    //gpio_disable_pulls(ENC_SW);
    gpio_pull_up(ENC_SW);
    
    gpio_init(ENC_A);
    gpio_set_dir(ENC_A,GPIO_IN);
//    gpio_disable_pulls(ENC_A);
    gpio_pull_up(ENC_A);

    gpio_init(ENC_B);
    gpio_set_dir(ENC_B,GPIO_IN);
    gpio_pull_up(ENC_B);
//    gpio_disable_pulls(ENC_B);

    add_repeating_timer_us(175, Encoder_Timer_Callback, NULL, &etimer);
/*
    gpio_set_irq_enabled_with_callback(ENC_SW, GPIO_IRQ_EDGE_FALL, true, &encoder_callback);
    gpio_set_irq_enabled(ENC_A, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(ENC_B, GPIO_IRQ_EDGE_FALL, true);
*/   
}

