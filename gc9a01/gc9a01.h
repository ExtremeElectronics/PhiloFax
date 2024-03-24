
// color modes
#define COLOR_MODE_65K      0x50
#define COLOR_MODE_262K     0x60
#define COLOR_MODE_12BIT    0x03
#define COLOR_MODE_16BIT    0x05
#define COLOR_MODE_18BIT    0x06
#define COLOR_MODE_16M      0x07

// commands
#define GC9A01_NOP     0x00
#define GC9A01_SWRESET 0x01
#define GC9A01_RDDID   0x04
#define GC9A01_RDDST   0x09

#define GC9A01_SLPIN   0x10
#define GC9A01_SLPOUT  0x11
#define GC9A01_PTLON   0x12
#define GC9A01_NORON   0x13

#define GC9A01_INVOFF  0x20
#define GC9A01_INVON   0x21
#define GC9A01_DISPOFF 0x28
#define GC9A01_DISPON  0x29
#define GC9A01_CASET   0x2A
#define GC9A01_RASET   0x2B
#define GC9A01_RAMWR   0x2C
#define GC9A01_RAMRD   0x2E

#define GC9A01_PTLAR   0x30
#define GC9A01_VSCRDEF 0x33
#define GC9A01_COLMOD  0x3A
#define GC9A01_MADCTL  0x36
#define GC9A01_VSCSAD  0x37

#define GC9A01_MADCTL_MY  0x80  // Page Address Order
#define GC9A01_MADCTL_MX  0x40  // Column Address Order
#define GC9A01_MADCTL_MV  0x20  // Page/Column Order
#define GC9A01_MADCTL_ML  0x10  // Line Address Order
#define GC9A01_MADCTL_MH  0x04  // Display Data Latch Order
#define GC9A01_MADCTL_RGB 0x00
#define GC9A01_MADCTL_BGR 0x08

#define GC9A01_RDID1   0xDA
#define GC9A01_RDID2   0xDB
#define GC9A01_RDID3   0xDC
#define GC9A01_RDID4   0xDD

// Color definitions
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

//from other driver

#include "pico/stdlib.h"
#include "hardware/spi.h"

// Use DMA?
//#define USE_DMA 1


void GC9A01_Write_cmd( uint8_t cmd,  uint8_t *data, int len);

void LCD_setPins(uint16_t dc, uint16_t cs, int16_t rst, uint16_t sck, uint16_t tx);
void LCD_setSPIperiph(spi_inst_t *s);
void LCD_initDisplay();
void LCD_setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void LCD_waitForDMA();

void LCD_setRotation(uint8_t m);

void LCD_WritePixel(uint16_t x, uint16_t y, uint16_t col);
void LCD_WriteBitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t *bitmap);
void LCD_WriteRectangle(uint16_t  x, uint16_t y, uint16_t w, uint16_t h, uint16_t col);

void LCD_Fast_Hline(uint16_t  x, uint16_t y, uint16_t w, uint16_t col);
void LCD_Fast_Vline(uint16_t  x, uint16_t y, uint16_t h, uint16_t col);

void LCD_WriteLineFromBuff16(uint16_t  x, uint16_t y,uint16_t *buff, uint16_t length);
void LCD_WriteLineFromBuff8(uint16_t  x, uint16_t y,uint8_t *buff, uint16_t length,int nowait);
void LCD_WriteColumnFromBuff(uint16_t  x, uint16_t y,uint8_t *buff, uint16_t length);


