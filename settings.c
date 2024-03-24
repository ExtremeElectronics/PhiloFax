//settings - from SD eventually
//do not change after read from SD
// defaults from config.h

#include "settings.h"

#define WIFI_SSID "SSIDNAME"
#define WIFI_PASSWORD "WIFIPASSWORD"
#define DESTINATION_ADDR "10.42.42.35"
#define DESTINATION_PORT 5055
#define LOCAL_PORT 5055


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

const char * dest_addr = DESTINATION_ADDR ;
int dest_port = DESTINATION_PORT ; 
int local_port = LOCAL_PORT ;

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

        ini_name = "farnsworth.ini";


        if (SDFileExists(ini_name)){
          printf("Ini file %s Exists Loading ... \n\r",ini_name);

//########################################### INI Parser ######################################
          ini = iniparser_load(fr, ini_name);
          //iniparser_dump(ini, stdout);

          s_wifi_pass = iniparser_getstring(ini, "WIFI:pass",WIFI_PASSWORD );
          s_wifi_ssid = iniparser_getstring(ini, "WIFI:ssid",WIFI_SSID );

          dest_addr = iniparser_getstring(ini, "DESTINATION:dest",DESTINATION_ADDR);
          dest_port = iniparser_getint(ini, "DESTINATION:port",DESTINATION_PORT );

          local_port = iniparser_getint(ini, "DESTINATION:port",LOCAL_PORT );

          loopback_audio = iniparser_getint(ini, "TEST:loopback_audio",0 );
          loopback_video = iniparser_getint(ini, "TEST:loopback_video",0 );



   
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
