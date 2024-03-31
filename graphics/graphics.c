#include <string.h>
#include "font.h"
#include "malloc.h"
#include "stdarg.h"
#include <stdio.h>
#include <stdlib.h>
#include "font8x8.h"
#include "gc9a01/gc9a01.h"
#include "graphics.h"


uint8_t fontsel=1;
uint8_t size_x=8;
uint8_t size_y=8;
uint8_t fwx=8; //   number of rows in the font for each character
//char line buffer
uint16_t *lineBuffer = NULL;

extern uint16_t display_width;
extern uint16_t display_height;



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



void lineBuffer_drawPixel(int16_t x, int16_t y, uint16_t color,int width){
    lineBuffer[x + y * width] = color ; //(color >> 8) | (color << 8);
}

//called ONLY with created buffer, either as part of a line or char write
void lb_drawch(char c, uint16_t fg,uint16_t bg,int cpx,int cpy,int lbw){

    unsigned char line;
    if(fontsel==0){
        for (int8_t i = 0; i < fwx; i++){ // Char bitmap = size_x columns
            line = font[c * fwx + i];
            for (int8_t j = 0; j < size_y; j++, line >>= 1){
                if (line & 1){
                    lineBuffer_drawPixel(cpx + i, cpy + j, fg,lbw);
                }else{
                    lineBuffer_drawPixel(cpx + i, cpy + j, bg,lbw);
                }

            }
        }
    }

    if (fontsel==1){
        for (int8_t i = 0; i < fwx; i++){ // Char bitmap = size_x columns
            line = font8x8[c * fwx + i];
            for (int8_t j = 0; j < size_y; j++, line >>= 1){
                if (line & 1){
                    lineBuffer_drawPixel(cpx + j, cpy + i, fg,lbw);
                }else{
                    lineBuffer_drawPixel(cpx + j, cpy + i, bg,lbw);
                }
            }
        }
    }

}


void drawch(char c, uint16_t fg,uint16_t bg,int cpx,int cpy){

    unsigned char line;
    if(fontsel==0){
        for (int8_t i = 0; i < fwx; i++){ // Char bitmap = size_x columns
            line = font[c * fwx + i];
            for (int8_t j = 0; j < size_y; j++, line >>= 1){
                if (line & 1){
                   LCD_WritePixel(cpx + i, cpy + j, fg);
                }else{
                   LCD_WritePixel(cpx + i, cpy + j, bg);
                }

            }
        }
    }

    if (fontsel==1){
        for (int8_t i = 0; i < fwx; i++){ // Char bitmap = size_x columns
            line = font8x8[c * fwx + i];
            for (int8_t j = 0; j < size_y; j++, line >>= 1){
                if (line & 1){
                    LCD_WritePixel(cpx + j, cpy + i, fg);
                }else{
                    LCD_WritePixel(cpx + j, cpy + i, bg);
                }
            }
        }
    }

}


void drawch90(char c, uint16_t fg,uint16_t bg,int cpx,int cpy){

    unsigned char line;
    if(fontsel==0){
        for (int8_t i = 0; i < fwx; i++){ // Char bitmap = size_x columns
            line = font[c * fwx + i];
            for (int8_t j = 0; j < size_y; j++, line >>= 1){
                if (line & 1){
//                   LCD_WritePixel(cpx + i, cpy + j, fg);
                   LCD_WritePixel(cpx + size_x - i,cpy + j , fg);
                }else{
//                   LCD_WritePixel(cpx + i, cpy + j, bg);
                   LCD_WritePixel(cpx + size_x - i,cpy + j , bg);
                }

            }
        }
    }

    if (fontsel==1){
        for (int8_t i = 0; i < fwx; i++){ // Char bitmap = size_x columns
            line = font8x8[c * fwx + i];
            for (int8_t j = 0; j < size_y; j++, line >>= 1){
                if (line & 1){
//                    LCD_WritePixel(cpx + j, cpy + i, fg);
                    LCD_WritePixel(cpx + size_x - i,cpy + j , fg);
                }else{
//                    LCD_WritePixel(cpx + j, cpy + i, bg);
                    LCD_WritePixel(cpx + size_x - i,cpy +  j , bg);
                }
            }
        }
    }

}



void lineBuff_drawChar(int16_t ax, int16_t ay, unsigned char c, uint16_t fg, uint16_t bg){

    lineBuffer = malloc(size_x * size_y * sizeof(uint16_t));

    lb_drawch(c,fg,bg,0,0,size_x);

    free(lineBuffer);
}


void drawtext(int16_t ax, int16_t ay, char * text, uint16_t fg, uint16_t bg){
   int l=strlen(text);
   for(int a=0;a<l;a++){
      drawch(text[a],fg,bg,ax+a*size_x,ay);
   }

}

void drawtext90(int16_t ax, int16_t ay, char * text, uint16_t fg, uint16_t bg){
   int l=strlen(text);
   for(int a=0;a<l;a++){
      drawch90(text[a],fg,bg,ax,ay+a*size_x);
   }

}

void drawtext90centred(int16_t ax, char * text, uint16_t fg, uint16_t bg){
   //display 90 degrees so x and y are reversed
   uint8_t ay;
   ay=(display_width-(strlen(text)*size_x))/2;
   drawtext90(ax, ay, text, fg, bg);

}












