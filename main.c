/**
 * Farnsworth - Copyright (c) 2024 Derek Woodroffe <tesla@extremeelectronics.co.uk>	
 *
 * Camera Code - Copyright (c) 2022 Brian Starkey <stark3y@gmail.com>
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

#define USE_DMA 1

//defaults and sdcard reader/parcer
#include "settings.h"
extern int SDStatus; // 0 no sd, 1 sd detected, 2 read ini
extern int local_port;
extern int maxdestinations;
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
extern const char * s_wifi_ssid;
extern const char * s_wifi_pass;

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

volatile uint8_t p_buffer [600];

volatile uint8_t receiving=0;


//screen
void IPScreen(void);


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
              for(v=0;v<SoundPacket;v++) AddToSpkBuffer(_pData[1+v]);
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

    //setup udp TX
    ipaddr_aton(dest_addr[dest], &target_addr);
    pcb = udp_new();
    er=udp_bind(pcb, IP_ADDR_ANY, LOCAL_PORT);
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
  

        printf("Connecting to Wi-Fi...\n");
        int x=1;
        while(x){
            x=cyw43_arch_wifi_connect_timeout_ms(s_wifi_ssid, s_wifi_pass, CYW43_AUTH_WPA2_AES_PSK, 30000);
            if (x) {
                printf("trying. \n");
                sleep_ms(500);
            } else {
                printf("\n\n************ Connected **************\n\n");
                printf("MyIP %s:%i -  Dest Ip %s:%i \n\n",ip4addr_ntoa(netif_ip4_addr(netif_list)),LOCAL_PORT,dest_addr[dest],dest_port[dest] );
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

void SDErrorScreen(void){
  printf("SD Error\n");  
  ClearScreen();
  char temp[250];
  if (SDStatus==1) sprintf(temp,"No SD Card");
  if (SDStatus==2) sprintf(temp,"INI file read error ");
  drawtext90centred(100, temp, WHITE, BLACK);
  printf("\nHalted SD Error\n");
  while(1);
}


int64_t prn;

//truely awfull psudo random generator , but ok for noise on screen display 
int16_t psudorand(void){
  prn = prn * 1103515243 + 12345;
  return (uint16_t)(prn >> 15) & 0xffff;
}

void NoSigScreen(void){
  printf("No Sig\n");
  ClearScreen();
  
//  prn=MikeBuffer[0]+MikeBuffer[1]+1;
  prn=1;

  uint16_t n;
  for(int y=0;y<240;y+=3){
     for(int x=0;x<240;x+=3){     
       n=psudorand();
       LCD_WritePixel(x,y,n);
     }
  }
  
  char temp[250];
  sprintf(temp,"-- NO SIGNAL --");
  drawtext90centred(70, temp, RED, BLACK);
//  sprintf(temp,"MyIP %s",ip4addr_ntoa(netif_ip4_addr(netif_list)));
//  drawtext90centred(110, temp, WHITE, BLACK);

  sprintf(temp,"MyIP %s",ip4addr_ntoa(netif_ip4_addr(netif_list)));
  drawtext90centred(120, temp, WHITE, BLACK);
  sprintf(temp,"Destination %s",dest_addr[dest]);
  drawtext90centred(110, temp, WHITE, BLACK);
  sprintf(temp,"Channel %i",dest_channel[dest]);
  drawtext90centred(100,  temp, WHITE, BLACK); 
  sprintf(temp,"Destination %i",dest);
  drawtext90centred(90,  temp, WHITE, BLACK);


}

void IPScreen(void){
  printf("ip screen\n");
//  ClearScreen();
//  char temp[250];
  NoSigScreen();
}

/*
//******************************* Sound Stuff *****************************

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

void ZeroMikeBiffer(void){
   MikeIn=0;
   MikeOut=0;
}

void ZeroSpkBuffer(void){
   SpkIn=0;
   SpkOut=0;
}


void AddToMikeBuffer(uint8_t v){
    MikeBuffer[MikeIn]=v;
    MikeIn++;
    if (MikeIn==SoundBuffMax){
          MikeIn=0;
    }
}

void AddToSpkBuffer(uint8_t v){
    SpkBuffer[SpkIn]=v;
    SpkIn++;
    if (SpkIn==SoundBuffMax){
          SpkIn=0;
    }
}

uint8_t GetFromSpkBuffer(void){
    uint8_t c=0;
    if(SpkIn!=SpkOut){
        c=SpkBuffer[SpkOut];
        SpkOut++;
        if (SpkOut==SoundBuffMax){
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
     
    pwm_set_both_levels(PWMslice,a,a);
   
    if (MikeIn==SoundPacket+1){
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
       send_udp_blocking(MikeBuffer, SoundPacket+1);
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
*/

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
        DisplayDigit(jog,WHITE,Red,BLACK );
        
}



// ******************************* CORE 1 *************************************

void Core1Main(void){

        printf("Core 1 start audio interrupt\n");
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

	printf("**************** STARTING Core 0 **************** \n\n");
        printf("Getting settings\n");
        get_settings();
        printf("Got settings\n");

        //init screen
	LCD_initDisplay();
	LCD_setRotation(3);
	ClearScreen();
	TestScreen();

        //init WiFi and UDP
        init_network();  
        
	//Start Core1
	multicore_reset_core1(); 
        multicore_launch_core1(Core1Main);
        printf("\n#Core 1 Started#\n\n");


		
//	const uint LED_PIN = GPIO_LED_PIN;
//	gpio_init(LED_PIN);
//	gpio_set_dir(LED_PIN, GPIO_OUT);

//        printf("Waiting Forever...\n");
//        while(1);
        
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
		return 1;
	}else{
	   printf("Camera_init complete\n");
	}

	sleep_ms(100);

        if (SDStatus<3){
           SDErrorScreen();        
        
        }

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

		ret = camera_capture_blocking(&camera, buf, true);

		if (ret != 0) {
			printf("Capture error: %d\n", ret);
		} else {
			uint16_t  y, x;
			uint32_t yoff;
		        char msg[display_width*2+1];
			for (y = 0; y < display_height; y++) {
			        yoff=buf->strides[0] * y;

                                msg[0]=(uint8_t)y;
                                for (x = 0; x < display_width*2; x++) {
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
            tight_loop_contents();
            
            // if receiving down to 1 set NoSig Screen
            if(receiving==1){
              receiving=0;
              NoSigScreen();
            }//if 


	}//while

}

