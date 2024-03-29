#include <string.h>
#include "font.h"
#include "malloc.h"
#include "stdarg.h"
#include <stdio.h>
#include <stdlib.h>
#include "font8x8.h"
#include "gc9a01/gc9a01.h"
#include "graphics.h"


//graphics files
//large digits
#include "FarnsNumbers.c"
//background
#include "Farnsworth_b.c"

#define PIXELSTOCENTRENUM 70

uint16_t SwapBinary(uint16_t v){
    return(v >> 8) | (v << 8);
}


void DisplayDigitBackground(uint8_t * background,uint16_t fg,uint16_t bg ){
   uint8_t x;
   uint8_t y;
   uint8_t d;
   uint8_t v;
   uint8_t b;
   bg=SwapBinary(bg);
   fg=SwapBinary(fg);
   uint16_t buffer[250];
   for(y=0;y<240;y++){
     x=0;
     for(d=0;d<30;d++){
        v=background[d+y*30];
        for(b=0;b<8;b++){
            if(v & 1<<b){
              buffer[x]=fg;
            }else{
              buffer[x]=bg;
            }
            x++;
        }
     }
     //write line
     LCD_WriteLineFromBuff8(0,y,buffer,240,0);
  }
}  

void DisplayDigit(int digit, uint16_t digitfg, uint16_t backgroundfg, uint16_t bg ){
   DisplayDigitBackground(&Farnsworth_b[0], backgroundfg,bg);
   uint8_t x;
   uint8_t y;
   uint8_t d;
   uint8_t v;
   uint8_t b;
   uint16_t buffer[110];
   for(y=0;y<100;y++){
     x=0;
     for(d=0;d<13;d++){
        v=FarnsNums[digit][d+y*13];
        for(b=0;b<8;b++){
            if(v & 1<<b){
              buffer[x]=digitfg;
            }else{
              buffer[x]=bg;
            }
            x++;
        }      
     }
     //write line
     LCD_WriteLineFromBuff8(PIXELSTOCENTRENUM,y+PIXELSTOCENTRENUM,buffer,100,0);
  }
  
  
  
}












