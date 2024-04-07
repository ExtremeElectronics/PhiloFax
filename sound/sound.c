/**
 * Part of PhiloFax https://github.com/ExtremeElectronics/PhiloFax
 *
 * sound.c
 *
 * Code to implement sound features - Copyright (c) 2024 Derek Woodroffe <tesla@extremeelectronics.co.uk>
 * on a Pi PicoW
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include "malloc.h"
#include "stdarg.h"
#include <stdio.h>
#include <stdlib.h>

//pico stuff
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"

#include "sound.h"

#include "farnsworthring.c"
#include "farnsworthtext.c"

#include "settings.h"

//void ZeroSpkBuffer(void);
//void AddToSpkBuffer(uint8_t v);

//sound buffers
uint8_t MikeBuffer[SOUNDBUFFMAX];
uint16_t MikeIn=0;
uint16_t MikeOut=0;

volatile uint8_t SpkBuffer[SOUNDBUFFMAX];
volatile uint16_t SpkIn=0;
volatile uint16_t SpkOut=0;

//sound counter
int sc=0;

extern volatile uint8_t SendSound;

extern int loopback_audio;
extern int loopback_video;
extern const char *dest_addr[MAXDESTS]; // = DESTINATION_ADDR ;
extern int dest_port[MAXDESTS]; // = DESTINATION_PORT ;
extern int dest_channel[MAXDESTS];// =0;
extern uint8_t dest;
extern volatile uint8_t receiving;

//must be pins on the same slice
#define soundIO1 8
#define soundIO2 9
#define PWMrate 90
#define Mike 28 //gpio pin
#define MikeADC 2 //adc channel

#define AUDIOFILTER 6 // delay between consecutive samples of noise max 15 smaller more noise

uint PWMslice;
struct repeating_timer stimer;

volatile void send_udp_blocking(char* msg,int msglen);

//void AddToSpkBuffer(uint8_t v);

uint16_t fringpos=0;
uint16_t ftextpos=0;

#define FRINGMAX 10272
#define FTEXTMAX 6000

uint8_t fring(uint8_t s_in){
  if(fringpos){
    if(fringpos==FRINGMAX){
       fringpos=0;  
    }else{
      return farnsworthring[fringpos++];
    }  
  }else{
    return s_in;
  }  
}

uint8_t ftext(uint8_t s_in){
     if(ftextpos){
       if(ftextpos==FTEXTMAX){
         ftextpos=0;
       }else{
         return farnsworthtext[ftextpos++];
       }
    }else{
      return s_in;
    }
}

void SetPWM(void){
    gpio_init(soundIO1);
    gpio_set_dir(soundIO1,GPIO_OUT);
    gpio_set_function(soundIO1, GPIO_FUNC_PWM);

    gpio_init(soundIO2);
    gpio_set_dir(soundIO2,GPIO_OUT);
    gpio_set_function(soundIO2, GPIO_FUNC_PWM);

    PWMslice=pwm_gpio_to_slice_num (soundIO1);
    pwm_set_clkdiv(PWMslice,16);
    pwm_set_both_levels(PWMslice,0x80,0x80);

    pwm_set_output_polarity(PWMslice,true,false);

    pwm_set_wrap (PWMslice, 256);
    pwm_set_enabled(PWMslice,true);

}

void SetA2D(void){
    adc_init();
    gpio_set_dir(Mike,GPIO_IN);
    adc_gpio_init(Mike);
    adc_select_input(MikeADC);
}

void ZeroMikeBuffer(void){
   MikeIn=0;
   MikeOut=0;
}

void ZeroSpkBuffer(void){
   SpkIn=0;
   SpkOut=0;
}

void AddToSpkBuffer(uint8_t v){
    SpkBuffer[SpkIn]=v;
    SpkIn++;
    if (SpkIn==SOUNDBUFFMAX){
          SpkIn=0;
    }
}

uint8_t GetFromSpkBuffer(void){
    uint8_t c=0;
    if(SpkIn!=SpkOut){
        c=SpkBuffer[SpkOut];
        SpkOut++;
        if (SpkOut==SOUNDBUFFMAX){
            SpkOut=0;
        }
    }
    return c;
}
/*
uint8_t GetFromMikeBuffer(void){
    uint8_t c=0;
    if(MikeIn!=MikeOut){
        c=MikeBuffer[MikeOut];
        MikeOut++;
        if (MikeOut==SoundBuffMax){
            MikeOut=0;
        }
    }
    return c;
}
*/

void AddToMikeBuffer(uint8_t v){
    MikeBuffer[MikeIn]=v;
    MikeIn++;
    if (MikeIn==SOUNDBUFFMAX){
          MikeIn=0;
    }
}


//sample mikrophone and output buffer to pwm every 125uS
bool Sound_Timer_Callback(struct repeating_timer *t){
    uint16_t a;
    //get a2d into mike buffer
    a=adc_read();

    busy_wait_us(AUDIOFILTER);
    a=a+adc_read();
    busy_wait_us(AUDIOFILTER);
    a=a+adc_read();
    busy_wait_us(AUDIOFILTER);
    a=a+adc_read();
    a=a>>6; //scales from 4*4096 to 256

    if (loopback_audio==0){
        AddToMikeBuffer(a);
        a=GetFromSpkBuffer();
    }
    a=fring(a);
    a=ftext(a);
    pwm_set_both_levels(PWMslice,a,a);

    if (MikeIn==SOUNDPACKET+1){
        SendSound=1;
        MikeIn=1;
        MikeBuffer[0]=255;

       if (receiving>1)receiving--;
    }

    return 1; // make repeating
}

void init_sound(void){
    SetPWM();
    SetA2D();
    MikeIn=1;
    MikeBuffer[0]=255;
    //start 125uS interrupt
//    add_repeating_timer_us(125, Sound_Timer_Callback, NULL, &stimer);
    printf("Sound INIT\n");
}

void send_sound(void){
   if (SendSound==1){
       char temp[3];
       send_udp_blocking(MikeBuffer, SOUNDPACKET+1);
       SendSound=0;
       sc++;
       if(sc>100){
           //send channel info every so often
           sc=0;
           temp[0]=253;
           temp[1]=dest_channel[dest];
           send_udp_blocking(temp, 2);
       }
    }


}

void StartSoundTimer(void){
   add_repeating_timer_us(125, Sound_Timer_Callback, NULL, &stimer);

}
