//settings - from SD eventually
//do not change after read from SD
// defaults from config.h

#include "settings.h"
/*
#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "WIFIPASSWORD"
#define DESTINATION_ADDR "10.42.42.35"
#define DESTINATION_PORT 5055
#define LOCAL_PORT 5055
#define MAXDESTS 10
*/

//iniparcer
#include "dictionary.h"
#include "iniparser.h"


//sd card reader
#include "f_util.h"
#include "ff.h"

//spi config for FFat
#include "hw_config.h"

FRESULT fr;

const char * s_wifi_ssid;
const char * s_wifi_pass;

int loopback_audio=0;
int loopback_video=0;

int SDStatus=0; // 0 no sd, 1 sd detected, 2 read ini
int local_port = LOCAL_PORT ;
int maxdestinations =0;

const char *dest_addr[MAXDESTS]; // = DESTINATION_ADDR ;
int dest_port[MAXDESTS]; // = DESTINATION_PORT ; 
int dest_channel[MAXDESTS];// =0;

int SDFileExists(char * filename){
    FRESULT fr;
    FILINFO fno;

    fr = f_stat(filename, &fno);
    return fr==FR_OK;
}


int SD_CardMount(FRESULT fr){
   // mount SD Card
        sd_card_t *pSD = sd_get_by_num(0);
        fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
        if (FR_OK != fr){
            // panic("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
            printf("SD INIT FAIL  \n\r");
            return 0;
        }

        printf("SD INIT OK \n\r");
        SDStatus=1;
        return 1;
}


void ini_file_parse(FRESULT fr){
        dictionary * ini ;
        char       * ini_name ;
        const char  *   s ;
        const char * inidesc;

        int overclock;
        int jpc;
        int iscf=0;
        int d;

        ini_name = "farnsworth.ini";
        char dname[100]; 

        if (SDFileExists(ini_name)){
          printf("Ini file %s Exists Loading ... \n\r",ini_name);

//########################################### INI Parser ######################################
          ini = iniparser_load(fr, ini_name);
          //iniparser_dump(ini, stdout);

          s_wifi_pass = iniparser_getstring(ini, "WIFI:pass",WIFI_PASSWORD );
          s_wifi_ssid = iniparser_getstring(ini, "WIFI:ssid",WIFI_SSID );

          loopback_audio = iniparser_getint(ini, "TEST:loopback_audio",0 );
          loopback_video = iniparser_getint(ini, "TEST:loopback_video",0 );

          local_port = iniparser_getint(ini, "LOCAL:port",LOCAL_PORT );

          maxdestinations = iniparser_getint(ini, "SELECTION:maxdestinations",0 );
          
          printf("Max Destinations %i \n",maxdestinations);

          for(d=0;d<maxdestinations;d++){

            sprintf(dname,"DESTINATION%i:address",d);
            dest_addr[d] = iniparser_getstring(ini, dname,DESTINATION_ADDR);
            
            sprintf(dname,"DESTINATION%i:port",d);
            dest_port[d] = iniparser_getint(ini, dname,DESTINATION_PORT );

            sprintf(dname,"DESTINATION%i:channel",d);
            dest_channel[d] = iniparser_getint(ini, dname,0 );

            printf("%i -> %s:%i ch:%i \n",d,dest_addr[d],dest_port[d],dest_channel[d]);

          }
          
   
         printf("Loaded INI\n\r");
         SDStatus=3;
    }else{
      printf("inifile does not exist, using compiled defaults");
    }  
}


void get_settings(void){
    if (SD_CardMount(fr)){
       ini_file_parse(fr);
    }
    


}
