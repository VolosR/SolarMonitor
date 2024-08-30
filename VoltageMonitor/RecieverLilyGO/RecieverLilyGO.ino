#include <TFT_eSPI.h>
#include <esp_now.h>
#include <WiFi.h>
#include "font.h"


TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);


#define back 0x9D91
#define bigFont DSEG7_Modern_Bold_64
#define small DejaVu_Sans_Mono_Bold_12
#define middle Monospaced_bold_18
#define middle2 DSEG7_Classic_Bold_30
#define clockFont DSEG7_Modern_Bold_20
#define dc Cousine_Regular_38
#define color1 TFT_BLACK
//#define color1 0x18E3


float voltage=0;  //voltage
int capacity=0;
bool invert=false;


int bright=4;
int brightnesses[7]={35,70,105,140,175,210,250};
int deb=0;


void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&voltage, incomingData, sizeof(voltage));
  draw();
}

void setup() 
{  
  pinMode(0,INPUT_PULLUP);
  pinMode(14,INPUT_PULLUP);

  pinMode(15,OUTPUT);
  digitalWrite(15,1);

  tft.init();
  tft.setRotation(1);
  sprite.createSprite(320,170);
  sprite.setTextColor(color1,back);
  sprite.setFreeFont(&bigFont);

  analogWrite(38,160); //brightnes of screen

        WiFi.mode(WIFI_MODE_STA);
        esp_now_init();
        esp_now_register_recv_cb(OnDataRecv);

        draw();
}

void draw()
  {
     sprite.setTextColor(color1,back);
     sprite.fillSprite(back);
     sprite.setFreeFont(&bigFont);
     sprite.setTextDatum(0);
     if(voltage<10)
     sprite.drawFloat(voltage,3,82,60);
     else
     sprite.drawFloat(voltage,2,82,60);

     

     sprite.setFreeFont(&middle2);
    
    
     int vol=voltage*100;
     if(vol<1200)
     capacity=0;
     else
     capacity=map(vol,1200,1360,0,100);

     if(vol>1360)
     capacity=100;

     sprite.setTextDatum(4);
     sprite.drawString(String(capacity),238,22);
     sprite.setTextDatum(0);
   
 
       sprite.setFreeFont(&clockFont);
       sprite.drawString("2024",4,26);
       sprite.setFreeFont(&middle);
       sprite.setTextColor(back,color1);
       sprite.drawString(" SOLAR ",4,60);
       sprite.drawString(" BATTERY  ",90,6);
     
       sprite.setTextColor(color1,back);
       sprite.drawString("%",288,8);
       sprite.drawString("V",298,60);
       sprite.drawString("VOLOS",4,6);
    
       
       sprite.drawString(" STATUS",90,26);
       sprite.drawString("ESPNOW",4,142);
       
         sprite.fillRect(90,46,224,2,color1);
         sprite.fillRect(90,140,224,20,color1);


   //      for(int i=0;i<sBlock;i++)
     //    sprite.fillRect(6+(i*8),26,6,5,color1);

      sprite.setFreeFont(&small);
      sprite.drawString(" MONITOR",4,82);
      
      sprite.setTextDatum(4);


    sprite.pushSprite(0,0);
}


void loop() {
    if(digitalRead(14)==0)
       {if(deb==0)
       {deb=1; 
       invert=!invert;
       tft.invertDisplay(invert);
       draw();
       }
  }else deb=0;


}
