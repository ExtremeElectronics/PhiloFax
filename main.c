/**
 * PhiloFax 
 *
 * Main.c
 *
 * Code to implement a Farnsworth - Copyright (c) 2024 Derek Woodroffe <tesla@extremeelectronics.co.uk>	
 * on a Pi PicoW
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

//pico stuff
#include "hardware/i2c.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "pico/rand.h"

//wifi stuff
#include "lwipopts.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

//camera driver
#include "camera/camera.h"
#include "camera/format.h"
#include "camera/ov7670.h"

//display driver
#include "gc9a01/gc9a01.h"
#include "gc9a01/gc9a01.c"
//#include "screen.c"
#include "graphics/graphics.h"

#include "splash.c"

#define USE_DMA 1

//defaults and sdcard reader/parcer
#include "settings.h"
extern int SDStatus; // 0 no sd, 1 sd detected, 2 read ini
extern int local_port;
extern int maxdestinations;
extern const char *dest_name[MAXDESTS]; // = DESTINATION_name ;
extern const char *dest_addr[MAXDESTS]; // = DESTINATION_ADDR ;


extern int dest_port[MAXDESTS]; // = DESTINATION_PORT ;
extern int dest_channel[MAXDESTS];// =0;

#define CAMERA_PIO      pio0
#define CAMERA_BASE_PIN 16
#define CAMERA_XCLK_PIN 21
#define CAMERA_I2C	i2c1
#define CAMERA_SDA      26
#define CAMERA_SCL      27

#define CYW43_SPI_PIO_PREFERRED_PIO 1


//sound
#include "sound/sound.h"
extern uint16_t ftextpos;


//RotaryEncoder
#include "rotenc.h"


//wifi
//extern const char * wifi_ssid;
//extern const char * wifi_pass;
uint8_t usedwifi=0;
extern const char *wifi_ssid[MAXWIFI];
extern const char *wifi_pass[MAXWIFI];

//TEST
extern int loopback_audio;
extern int loopback_video;

//destination
extern int dest_channel[]; 
uint8_t dest=0; //

//ip
struct udp_pcb* pcb ;
ip_addr_t target_addr;
ip_addr_t my_addr;

volatile uint8_t sendingudp=0;

volatile uint8_t online=0;
volatile uint8_t SendSound=0;

volatile int GotData=0;

//volatile uint8_t p_buffer [600];

volatile uint8_t receiving=0;

uint16_t seed=0;

//screen
void IPScreen(void);
void DisplayError(char * errtext);

extern uint16_t fringpos;

//******************************* Networky Stuff ************************

static void udpReceiveCallback(void *_arg, struct udp_pcb *_pcb,struct pbuf *_p, const ip_addr_t *_addr, uint16_t _port) {
    char *_pData = (char *)_p->payload;

    if (GotData==1) printf("Overrun on RX \n");
    
    int c;

    uint8_t yrx=_pData[0];
    if (yrx<=display_width) {
        if(loopback_video==0){
            LCD_WriteLineFromBuff8(1,yrx,&_pData[1],display_width,0);
        }    
    }else{
        if (yrx==255){
            if (loopback_audio==0){
              uint8_t v;
              for(v=0;v<SOUNDPACKET;v++) AddToSpkBuffer(_pData[1+v]);
            }  
        }
    }

    if(receiving<100){
       if(receiving<3)fringpos=1;
       receiving=100; //receiving
    } 
    pbuf_free(_p); // don't forget to release the buffer!!!!
}


void init_udp(){
    err_t er;

    printf("Init UDP %s\n",dest_addr[dest]);
    //setup udp TX
    ipaddr_aton(dest_addr[dest], &target_addr);
    pcb = udp_new();
    er=udp_bind(pcb, IP_ADDR_ANY, local_port);
    if (er != ERR_OK) {
            printf("Failed to bind RX UDP! error=%d", er);
        }
    udp_recv(pcb, udpReceiveCallback, NULL);
}

void send_udp(char* msg,int msglen){
//        printf("send UPD %s %i\n",msg,msglen);
        struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, msglen+1, PBUF_RAM);
        char *req = (char *)p->payload;
        memset(req, 0, msglen+1);
        int a;
        for(a=0;a<msglen;a++) req[a]=msg[a];
        req[a]=0;
        //set dest address - port
        ipaddr_aton(dest_addr[dest], &target_addr);
        err_t er = udp_sendto(pcb, p, &target_addr, dest_port[dest]);
        pbuf_free(p);
        if (er != ERR_OK) {
            printf("Failed to send UDP packet! error=%d", er);
        } else {
//            printf("Sent packet \n\n");
        }
 //       sleep_ms(100);
}

void send_udp_blocking(char* msg,int msglen){
   if (online){
       while(sendingudp==1);
       sendingudp=1;
       send_udp( msg,msglen);
       sendingudp=0;
   }
}


void init_network(){
        if (cyw43_arch_init()) {
            printf("cyw43 failed to initialise\n");
            while(1);
        }
        cyw43_arch_enable_sta_mode();
  
        
        printf("Connecting to Wi-Fi %s ...\n",wifi_ssid[usedwifi]);
        int x=1;
        int t=10;
        while(x){
            DisplayDigit(usedwifi,1,BLUE,WHITE,BLACK );
            x=cyw43_arch_wifi_connect_timeout_ms(wifi_ssid[usedwifi], wifi_pass[usedwifi], CYW43_AUTH_WPA2_AES_PSK, 5000);
            if (x) {
                printf("trying. SSID[%i] %s \n",usedwifi,wifi_ssid[usedwifi]);
                sleep_ms(100);
                t=t+1;
                if (t>1){
                   usedwifi++;
                   t=0;
                   if ((usedwifi==MAXWIFI) || wifi_ssid[usedwifi][0]==0 ){
                      printf("\n\nSorry, given up with the WIFI here\n");
                      DisplayError("Cant Conn to WiFi");
                      sleep_ms(1000);
                      while(1);
                   }
                }
            } else {
                printf("\n\n************ Connected to %s **************\n\n",wifi_ssid[usedwifi]);
                printf("MyIP %s:%i -  Dest Ip %s:%i \n\n",ip4addr_ntoa(netif_ip4_addr(netif_list)),local_port,dest_addr[dest],dest_port[dest] );
                printf("*************************************\n\n");
                ftextpos=1;

            }
        }

        printf("\nWiFi Ready...\n");
        IPScreen();
        init_udp();
        online=1;



}


//*************************** I2C INIT ******************************

static inline int __i2c_write_blocking(void *i2c_handle, uint8_t addr, const uint8_t *src, size_t len)
{
	return i2c_write_blocking((i2c_inst_t *)i2c_handle, addr, src, len, false);
}

static inline int __i2c_read_blocking(void *i2c_handle, uint8_t addr, uint8_t *dst, size_t len)
{
	return i2c_read_blocking((i2c_inst_t *)i2c_handle, addr, dst, len, false);
}

//************************ Display Stuff *******************************

void ClearScreen(void){
   LCD_WriteRectangle( 0, 0,display_width,display_height, BLACK);
}

void Rect(uint16_t x,uint16_t x1,uint16_t y,uint16_t y1,uint16_t colour){
    LCD_WriteRectangle( x, y,(x1-x),(y1-y), colour);  
}

void DisplayError(char * errtext){
  char temp[100];
  ErrorBackground(RED, BLACK);
  sprintf(temp," %s ",errtext);
  drawtext90centred(115, temp, WHITE, RED);
  sleep_ms(1000);
  while(1);
}

void DisplaySDError(int err){
  char temp[250];
  if (SDStatus==1) sprintf(temp,"No SD Card");
  if (SDStatus==2) sprintf(temp,"INI file read error ");
  DisplayError(temp);
  
}

void TestScreen(void){

   Rect(40,80,40,80,RED);
   Rect(160,200,160,200,BLUE);
   Rect(40,80,160,200,GREEN);
   Rect(160,200,40,80,YELLOW);

   LCD_Fast_Vline(display_width/2, 0, display_height, GREEN);
   LCD_Fast_Hline(0, display_height/2, display_width, GREEN);

   LCD_WritePixel(120, 120, WHITE);
   LCD_WritePixel(122,122,BLUE);
   LCD_WritePixel(120,122,RED);
   
}


void NoSigScreen(void){
    printf("No Sig\n");
    ClearScreen();
    uint16_t n;
    for(int y=0;y<240;y+=7){
/*        for(int x=0;x<240;x+=4){     
            n=rand()&& 0xffff;
            LCD_WritePixel(x,y,n);
        }
*/	
        uint8_t x=0;
        uint8_t yy=0;
        while(x<240){
            x+=1+(get_rand_32() & 7);     
            yy=(get_rand_32() & 7);
            n=(uint16_t)get_rand_32();
            LCD_WritePixel(x,y+yy,n);
        }
    }
  
    char temp[250];

    sprintf(temp,"Destination %i",dest);
    drawtext90centred(200,  temp, Copper, BLACK);

    sprintf(temp,"%s",dest_name[dest]);
    drawtext90centred(185,  temp, WHITE, BLACK);


    sprintf(temp,"Connection Details");
    drawtext90centred(150, temp, Copper, BLACK);
    
    sprintf(temp,"%s",dest_addr[dest]);
    drawtext90centred(130, temp, WHITE, BLACK);
    sprintf(temp,"Port %i",dest_port[dest]);
    drawtext90centred(115, temp, WHITE, BLACK);
    
    sprintf(temp,"Channel %i",dest_channel[dest]);
    drawtext90centred(90,  temp, WHITE, BLACK); 

    sprintf(temp,"-- NO SIGNAL --");
    drawtext90centred(60, temp, WHITE, RED);

    sprintf(temp,"MyIP %s",ip4addr_ntoa(netif_ip4_addr(netif_list)));
    drawtext90centred(45, temp, BLUE, BLACK);
    sprintf(temp,"SSID[%i] %s",usedwifi,wifi_ssid[usedwifi]);
    drawtext90centred(35, temp, BLUE, BLACK);
    sprintf(temp,"PORT %i",local_port);
    drawtext90centred(25, temp, BLUE, BLACK);
}

void IPScreen(void){
  printf("ip screen\n");
  NoSigScreen();
}


// ***************************** LED *********************
void PCB_LED(int x){
      //gpio_put(LED_PIN, x);
      cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, x);
  
  }



// ************************** Jog wheel move ******************
//called from rotenc on jog change
void JogChange(uint8_t jog){
        printf("Jog:%i\n",jog);
        dest=jog;
        receiving=50; //channel changed, we are no longer receiving, delay until no sifg screen
        DisplayDigit(jog,0,WHITE,Copper,BLACK );
//        DisplayDigit(jog,WHITE,Red,BLACK );
//        init_udp();
// NEEDS TO DO REINIT OF ADDRESS SOMEHOW
// done in send ????

}



// ******************************* CORE 1 *************************************

void Core1Main(void){

        printf("Core 1 - start audio interrupt\n");
//        add_repeating_timer_us(125, Sound_Timer_Callback, NULL, &stimer);
        StartSoundTimer();
        RotEncSetup();
        
        while(1){
           tight_loop_contents(); 
        }
}


// ******************************* MAIN **********************

int main() {
	stdio_init_all();
	init_sound();
	// Wait some time for USB serial connection
	sleep_ms(3000);
	splash();
	printf("**************** STARTING Core 0 **************** \n\n");

        //init screen
	LCD_initDisplay();
	LCD_setRotation(3);
	ClearScreen();
	TestScreen();
	
	//get settings
        printf("Getting settings\n");
        int err;
        if(err=get_settings()>0){
           printf("Bad Settings\n");
           DisplaySDError(err);

        }
        printf("Got settings\n");


        //init WiFi and UDP
        init_network();  
        
	//Start Core1
	multicore_reset_core1(); 
        multicore_launch_core1(Core1Main);
        printf("\n#Core 1 Started#\n\n");
        
	//camera init
	i2c_init(CAMERA_I2C, 100000);
	gpio_set_function(CAMERA_SDA, GPIO_FUNC_I2C);
	gpio_set_function(CAMERA_SCL, GPIO_FUNC_I2C);
	gpio_pull_up(CAMERA_SDA);
	gpio_pull_up(CAMERA_SCL);

	struct camera camera;
	struct camera_platform_config platform = {
		.i2c_write_blocking = __i2c_write_blocking,
		.i2c_read_blocking = __i2c_read_blocking,
		.i2c_handle = CAMERA_I2C,

		.pio = CAMERA_PIO,
		.xclk_pin = CAMERA_XCLK_PIN,
		.xclk_divider = 9,
		.base_pin = CAMERA_BASE_PIN,
		.base_dma_channel = -1,
	};

        printf("Camera Init \n");
        sleep_ms(100);

	int ret = camera_init(&camera, &platform);
	if (ret) {
		printf("camera_init failed: %d\n", ret);
		DisplayError("Camera Fail");
		sleep_ms(1000);
		while(1);
	}else{
	   printf("Camera_init complete\n");
	}

/*	sleep_ms(100);

        if (SDStatus<3){
           SDErrorScreen();        
        
        }
*/
	const uint16_t camera_width = CAMERA_WIDTH_DIV2;
	const uint16_t camera_height = CAMERA_HEIGHT_DIV2;

	struct camera_buffer *buf = camera_buffer_alloc(FORMAT_RGB565, camera_width, camera_height);
	assert(buf);

	printf("Camera %i,%i Display %i,%i Strided %i \n",camera_width,camera_height,display_width,display_height,buf->strides[0]);

	uint16_t v;
	
	char mag[display_width*2+1];

	int loops=0;
        uint16_t xoff=(camera_width-display_width)/2; //centre display to camera rasta
        uint16_t msglength=display_width*2+1;

        if (loopback_audio==1)printf("Loopback-Audio\n");
        if (loopback_video==1)printf("Loopback-Video\n");
        
	while (1) {

		//wait till camera DMA has done before starting another.		
		while(is_camera_busy(&camera)) {
                    send_sound();    
                }

//		wait_for_camera_finished(&camera);

//		ret = camera_capture_blocking(&camera, buf, true);
		ret =  camera_capture_nonblocking(&camera, buf, true);
		
		 while(is_camera_busy(&camera)) {
                    send_sound();
                }

		
		if (ret != 0) {
			printf("Capture error: %d\n", ret);
		} else {
			uint16_t  y, x;
			uint32_t yoff;
			uint16_t dw2=display_width*2;
		        char msg[dw2+1];
			for (y = 0; y < display_height; y++) {
			        yoff=buf->strides[0] * y;
			        // copy 1 line to tx buffer prefixed by line number    
                                msg[0]=(uint8_t)y;
                                for (x = 0; x < dw2; x++) {
                                    msg[x+1]=buf->data[0][yoff+xoff+x];                                
                                }
                                send_udp_blocking(msg, msglength);
                                //interleave sound if needed
                                send_sound();
                                                                
// local echo 
                                if (loopback_video==1){
//           			    LCD_WriteLineFromBuff8(1,y,&buf->data[0][yoff+xoff],display_width,0);
                       		    LCD_WriteLineFromBuff8(1,y,&msg[1],display_width,0);   
                                }//if
			}//for
		}//else
//             tight_loop_contents();
//            send_sound();
            // if receiving down to 1 set NoSig Screen
            if(receiving==1){
              receiving=0;
              NoSigScreen();
            }//if 
            seed++;

	}//while

}

