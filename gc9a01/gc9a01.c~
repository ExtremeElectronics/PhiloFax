
//Taken in part(s) from https://github.com/russhughes/gc9a01_mpy

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"

#include "gc9a01.h"

//uint16_t _width;  ///< Display width as modified by current rotation
//uint16_t _height; ///< Display height as modified by current rotation

//int16_t _xstart = 0; ///< Internal framebuffer X offset
//int16_t _ystart = 0; ///< Internal framebuffer Y offset

uint8_t rotation;

//hardware display size
uint16_t gc9a01_width=240;
uint16_t gc9a01_height=240;

//hardware pin allocation
spi_inst_t *gc9a01_spi = spi1;
uint16_t gc9a01_pinCS = 13;
uint16_t gc9a01_pinDC = 14;
int16_t gc9a01_pinRST = 15;

uint16_t gc9a01_pinSCK = 10;
uint16_t gc9a01_pinTX = 11;

//actual width/height after rotation
uint16_t display_height;
uint16_t display_width;


#ifdef USE_DMA
uint dma_lcd;
dma_channel_config dma_cfg;
void LCD_waitForDMA(){
	dma_channel_wait_for_finish_blocking(dma_lcd);
}
#endif

void LCD_setPins(uint16_t dc, uint16_t cs, int16_t rst, uint16_t sck, uint16_t tx)
{
	gc9a01_pinDC = dc;
	gc9a01_pinCS = cs;
	gc9a01_pinRST = rst;
	gc9a01_pinSCK = sck;
	gc9a01_pinTX = tx;
}

void LCD_setSPIperiph(spi_inst_t *s)
{
	gc9a01_spi = s;
}

void initSPI()
{
//	spi_init(gc9a01_spi, 1000 * 400000); //solid
	spi_init(gc9a01_spi, 1000 * 600000); //might work
//	spi_init(gc9a01_spi, 1000 * 800000); //might be flakey
	
	spi_set_format(gc9a01_spi, 16, SPI_CPOL_1, SPI_CPOL_1, SPI_MSB_FIRST);
	gpio_set_function(gc9a01_pinSCK, GPIO_FUNC_SPI);
	gpio_set_function(gc9a01_pinTX, GPIO_FUNC_SPI);

	gpio_init(gc9a01_pinCS);
	gpio_set_dir(gc9a01_pinCS, GPIO_OUT);
	gpio_put(gc9a01_pinCS, 1);

	gpio_init(gc9a01_pinDC);
	gpio_set_dir(gc9a01_pinDC, GPIO_OUT);
	gpio_put(gc9a01_pinDC, 1);

	if (gc9a01_pinRST != -1)
	{
		gpio_init(gc9a01_pinRST);
		gpio_set_dir(gc9a01_pinRST, GPIO_OUT);
		gpio_put(gc9a01_pinRST, 1);
	}

#ifdef USE_DMA
        printf("Display using DMA\n");
	dma_lcd = dma_claim_unused_channel(true);
	dma_cfg = dma_channel_get_default_config(dma_lcd);
//	channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_16);
	channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_8);
	channel_config_set_dreq(&dma_cfg, spi_get_dreq(gc9a01_spi, true));
#endif
}


void GC9A01_Select(){
	gpio_put(gc9a01_pinCS, 0);
}

void GC9A01_DeSelect(){
	gpio_put(gc9a01_pinCS, 1);
}

void GC9A01_RegCommand(){
	gpio_put(gc9a01_pinDC, 0);
}

void GC9A01_RegData(){
	gpio_put(gc9a01_pinDC, 1);
}

void GC9A01_WriteCommand(uint8_t cmd){
	GC9A01_RegCommand();
	spi_set_format(gc9a01_spi, 8, SPI_CPOL_1, SPI_CPOL_1, SPI_MSB_FIRST);
	spi_write_blocking(gc9a01_spi, &cmd, 1);
}

void GC9A01_WriteData(uint8_t *buff, size_t buff_size){
	GC9A01_RegData();
	spi_set_format(gc9a01_spi, 8, SPI_CPOL_1, SPI_CPOL_1, SPI_MSB_FIRST);
	spi_write_blocking(gc9a01_spi, buff, buff_size);
}

void GC9A01_Write_cmd( uint8_t cmd, uint8_t *data, int len) {
    GC9A01_Select();
    if (cmd) {
         GC9A01_WriteCommand(cmd);
    }
    if (len > 0) {
        GC9A01_WriteData(data, len);
    }
    GC9A01_DeSelect();
}

void GC9A01_Soft_reset(void) {
    GC9A01_Write_cmd( GC9A01_SWRESET, NULL, 0);
    sleep_ms(150);
}

void GC9A01_Hard_reset(void) {
    if (gc9a01_pinRST != -1){
        GC9A01_Select();
        gpio_put(gc9a01_pinRST, 1);
        sleep_ms(50);
        gpio_put(gc9a01_pinRST, 0);
        sleep_ms(50);
        gpio_put(gc9a01_pinRST, 1);
        sleep_ms(150);
        GC9A01_DeSelect();
    }
}

void LCD_initDisplay()
{
	initSPI();
	GC9A01_Select();

	if (gc9a01_pinRST < 0){		   // If no hardware reset pin...
	    GC9A01_Soft_reset();
	}
	else{
	    GC9A01_Hard_reset();
        }	
        GC9A01_Write_cmd( 0xEF, (uint8_t *) NULL, 0);
        GC9A01_Write_cmd( 0xEB, (uint8_t *) "\x14", 1);
        GC9A01_Write_cmd( 0xFE, (uint8_t *) NULL, 0);
        GC9A01_Write_cmd( 0xEF, (uint8_t *) NULL, 0);
        GC9A01_Write_cmd( 0xEB, (uint8_t *) "\x14", 1);
        GC9A01_Write_cmd( 0x84, (uint8_t *) "\x40", 1);
        GC9A01_Write_cmd( 0x85, (uint8_t *) "\xFF", 1);
        GC9A01_Write_cmd( 0x86, (uint8_t *) "\xFF", 1);
        GC9A01_Write_cmd( 0x87, (uint8_t *) "\xFF", 1);
        GC9A01_Write_cmd( 0x88, (uint8_t *) "\x0A", 1);
        GC9A01_Write_cmd( 0x89, (uint8_t *) "\x21", 1);
        GC9A01_Write_cmd( 0x8A, (uint8_t *) "\x00", 1);
        GC9A01_Write_cmd( 0x8B, (uint8_t *) "\x80", 1);
        GC9A01_Write_cmd( 0x8C, (uint8_t *) "\x01", 1);
        GC9A01_Write_cmd( 0x8D, (uint8_t *) "\x01", 1);
        GC9A01_Write_cmd( 0x8E, (uint8_t *) "\xFF", 1);
        GC9A01_Write_cmd( 0x8F, (uint8_t *) "\xFF", 1);
        GC9A01_Write_cmd( 0xB6, (uint8_t *) "\x00\x00", 2);
        GC9A01_Write_cmd( 0x3A, (uint8_t *) "\x55", 1); // COLMOD
        GC9A01_Write_cmd( 0x90, (uint8_t *) "\x08\x08\x08\x08", 4);
        GC9A01_Write_cmd( 0xBD, (uint8_t *) "\x06", 1);
        GC9A01_Write_cmd( 0xBC, (uint8_t *) "\x00", 1);
        GC9A01_Write_cmd( 0xFF, (uint8_t *) "\x60\x01\x04", 3);
        GC9A01_Write_cmd( 0xC3, (uint8_t *) "\x13", 1);
        GC9A01_Write_cmd( 0xC4, (uint8_t *) "\x13", 1);
        GC9A01_Write_cmd( 0xC9, (uint8_t *) "\x22", 1);
        GC9A01_Write_cmd( 0xBE, (uint8_t *) "\x11", 1);
        GC9A01_Write_cmd( 0xE1, (uint8_t *) "\x10\x0E", 2);
        GC9A01_Write_cmd( 0xDF, (uint8_t *) "\x21\x0c\x02", 3);
        GC9A01_Write_cmd( 0xF0, (uint8_t *) "\x45\x09\x08\x08\x26\x2A", 6);
        GC9A01_Write_cmd( 0xF1, (uint8_t *) "\x43\x70\x72\x36\x37\x6F", 6);
        GC9A01_Write_cmd( 0xF2, (uint8_t *) "\x45\x09\x08\x08\x26\x2A", 6);
        GC9A01_Write_cmd( 0xF3, (uint8_t *) "\x43\x70\x72\x36\x37\x6F", 6);
        GC9A01_Write_cmd( 0xED, (uint8_t *) "\x1B\x0B", 2);
        GC9A01_Write_cmd( 0xAE, (uint8_t *) "\x77", 1);
        GC9A01_Write_cmd( 0xCD, (uint8_t *) "\x63", 1);
        GC9A01_Write_cmd( 0x70, (uint8_t *) "\x07\x07\x04\x0E\x0F\x09\x07\x08\x03", 9);
        GC9A01_Write_cmd( 0xE8, (uint8_t *) "\x34", 1);
        GC9A01_Write_cmd( 0x62, (uint8_t *) "\x18\x0D\x71\xED\x70\x70\x18\x0F\x71\xEF\x70\x70", 12);
        GC9A01_Write_cmd( 0x63, (uint8_t *) "\x18\x11\x71\xF1\x70\x70\x18\x13\x71\xF3\x70\x70", 12);
        GC9A01_Write_cmd( 0x64, (uint8_t *) "\x28\x29\xF1\x01\xF1\x00\x07", 7);
        GC9A01_Write_cmd( 0x66, (uint8_t *) "\x3C\x00\xCD\x67\x45\x45\x10\x00\x00\x00", 10);
        GC9A01_Write_cmd( 0x67, (uint8_t *) "\x00\x3C\x00\x00\x00\x01\x54\x10\x32\x98", 10);
        GC9A01_Write_cmd( 0x74, (uint8_t *) "\x10\x85\x80\x00\x00\x4E\x00", 7);
        GC9A01_Write_cmd( 0x98, (uint8_t *) "\x3e\x07", 2);
        GC9A01_Write_cmd( 0x35, (uint8_t *) NULL, 0);
        GC9A01_Write_cmd( 0x21, (uint8_t *) NULL, 0);
        GC9A01_Write_cmd( 0x11, (uint8_t *) NULL, 0);

        sleep_ms(120);

        GC9A01_Write_cmd( GC9A01_DISPON, (uint8_t *) NULL, 0);
        sleep_ms(20);
        
	LCD_setRotation(0);
}

void LCD_setRotation(uint8_t rotation)
{
	uint8_t madctl_value = GC9A01_MADCTL_BGR;

        if (rotation == 0) {                  // Portrait
                madctl_value |= GC9A01_MADCTL_MX;
                display_width  = gc9a01_width;
                display_height = gc9a01_height;
        } else if (rotation == 1) {           // Landscape
                madctl_value |=  GC9A01_MADCTL_MV;
                display_width  = gc9a01_height;
                display_height = gc9a01_width;
        } else if (rotation == 2) {           // Inverted Portrait
                madctl_value |= GC9A01_MADCTL_MY;
                display_width  = gc9a01_width;
                display_height = gc9a01_height;
        } else if (rotation == 3) {           // Inverted Landscape
                madctl_value |= GC9A01_MADCTL_MX | GC9A01_MADCTL_MY | GC9A01_MADCTL_MV;
                display_width  = gc9a01_height;
                display_height = gc9a01_width;
        } else if (rotation == 4) {           // Portrait Mirrored
        display_width = gc9a01_width;
        display_height = gc9a01_height;
    } else if (rotation == 5) {           // Landscape Mirrored
        madctl_value |= GC9A01_MADCTL_MX | GC9A01_MADCTL_MV;
        display_width = gc9a01_height;
        display_height = gc9a01_width;
    } else if (rotation == 6) {           // Inverted Portrait Mirrored
        madctl_value |= GC9A01_MADCTL_MX | GC9A01_MADCTL_MY;
        display_width = gc9a01_width;
        display_height = gc9a01_height;
    } else if (rotation == 7) {           // Inverted Landscape Mirrored
        madctl_value |= GC9A01_MADCTL_MV | GC9A01_MADCTL_MY;
        display_width = gc9a01_height;
        display_height = gc9a01_width;
    }
    uint8_t madctl[] = { madctl_value };
    GC9A01_Write_cmd(GC9A01_MADCTL, madctl, 1);
}

void LCD_setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    uint8_t bufx[4] = {(x) >> 8, (x) & 0xFF, ((x+w)-1) >> 8, ((x+w)-1) & 0xFF};
    uint8_t bufy[4] = {(y) >> 8, (y) & 0xFF, ((y+h)-1) >> 8, ((y+h)-1) & 0xFF}; 
    GC9A01_Write_cmd(GC9A01_CASET, bufx, 4);
    GC9A01_Write_cmd(GC9A01_RASET, bufy, 4);
    GC9A01_Write_cmd(GC9A01_RAMWR, NULL, 0);
}

void LCD_WriteBitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t *bitmap)
{//needs doing
	GC9A01_Select();
	LCD_setAddrWindow(x, y, w, h); // Clipped area
	GC9A01_RegData();
//	spi_set_format(gc9a01_spi, 16, SPI_CPOL_1, SPI_CPOL_1, SPI_MSB_FIRST);
//#ifdef USE_DMA
//	dma_channel_configure(dma_lcd, &dma_cfg, &spi_get_hw(gc9a01_spi)->dr, // write address
//						  bitmap,						// read address
//						  w * h,						// element count (each element is of size transfer_data_size)
//						  true);						// start asap
//	LCD_waitForDMA();
//#else

	spi_write_blocking(gc9a01_spi, bitmap, w * h*2);
//#endif

	GC9A01_DeSelect();
}


void LCD_WriteLineFromBuff8(uint16_t  x, uint16_t y,uint8_t *buff, uint16_t length,int nowait){
        GC9A01_Select();
        LCD_setAddrWindow(x, y, length,1); // Clipped area
        GC9A01_Select();
        GC9A01_RegData();
//        spi_set_format(gc9a01_spi, 8, SPI_CPOL_1, SPI_CPOL_1, SPI_MSB_FIRST);
#ifdef USE_DMA
        dma_channel_configure(dma_lcd, &dma_cfg,
             &spi_get_hw(gc9a01_spi)->dr, // write address
             buff,                                               // read address
             length *2,                                          // element count (each element is of size transfer_data_size)
             true);                                              // start asap

        if (nowait!=1){                                                  
            LCD_waitForDMA();
            //We have finished, deassert select, if dma is still running (no wait), not our problem.
            GC9A01_DeSelect();
        }
#else
        spi_write_blocking(gc9a01_spi, buff, length*2);
        GC9A01_DeSelect();
#endif
}


void LCD_WritePixel(uint16_t  x, uint16_t y, uint16_t col)
{
	GC9A01_Select();
	LCD_setAddrWindow(x, y, 1, 1); // Clipped area
        uint8_t hi = col >> 8, lo = col;
        GC9A01_Select();
        GC9A01_WriteData(&hi,1);
        GC9A01_WriteData(&lo,1); 

        GC9A01_DeSelect();
}

void LCD_WriteRectangle(uint16_t  x, uint16_t y, uint16_t w, uint16_t h, uint16_t col){
        GC9A01_Select();
        LCD_setAddrWindow(x, y, w, h); // Clipped area
        uint8_t hi = col >> 8, lo = col;
        GC9A01_Select();
        int z;
        int pixels=w * h;
        for(z=0;z<pixels;z++){
            GC9A01_WriteData(&hi,1);
            GC9A01_WriteData(&lo,1);
        }
        GC9A01_DeSelect();

}

void LCD_Fast_Hline(uint16_t  x, uint16_t y, uint16_t w, uint16_t col){
        GC9A01_Select();
        LCD_setAddrWindow(x, y, w, 1); // Clipped area
        uint8_t hi = col >> 8, lo = col;
        GC9A01_Select();
        int z;
        for(z=0;z<w;z++){
            GC9A01_WriteData(&hi,1);
            GC9A01_WriteData(&lo,1);
        }
        GC9A01_DeSelect();

}

void LCD_Fast_Vline(uint16_t  x, uint16_t y, uint16_t h, uint16_t col){
        GC9A01_Select();
        LCD_setAddrWindow(x, y, 1, h); // Clipped area
        uint8_t hi = col >> 8, lo = col;
        GC9A01_Select();
        int z;
        for(z=0;z<h;z++){
            GC9A01_WriteData(&hi,1);
            GC9A01_WriteData(&lo,1);
        }
        GC9A01_DeSelect();

}


void LCD_WriteColumnFromBuff(uint16_t  x, uint16_t y,uint8_t *buff, uint16_t length){
        GC9A01_Select();
        LCD_setAddrWindow(x, y,1, length); // Clipped area
        GC9A01_Select();
        GC9A01_WriteData(buff,length*2);
        GC9A01_DeSelect();
}




void LCD_WriteLineFromBuff16(uint16_t  x, uint16_t y,uint16_t *buff, uint16_t length){
        GC9A01_Select();
        LCD_setAddrWindow(x, y, length,1); // Clipped area
        GC9A01_Select();
        GC9A01_RegData();
        spi_set_format(gc9a01_spi, 16, SPI_CPOL_1, SPI_CPOL_1, SPI_MSB_FIRST);
        spi_write16_blocking(gc9a01_spi, buff, length);
      
        GC9A01_DeSelect();
}



