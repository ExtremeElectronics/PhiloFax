
#include "pico/stdlib.h"
//#include "gfx.h"
#include <string.h>
//#include "screen.h"
#include "font.h"
#include "malloc.h"
#include "stdarg.h"
#include <stdio.h>
#include <stdlib.h>
#include "font8x8.h"

uint8_t fontsel=1;
uint8_t size_x=8;
uint8_t size_y=8;
uint8_t fwx=8; //   number of rows in the font for each character
//char line buffer
uint16_t *lineBuffer = NULL;

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
