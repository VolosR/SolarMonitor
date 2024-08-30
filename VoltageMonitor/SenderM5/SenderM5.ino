#include "M5Dial.h"
#include <EEPROM.h>
#include <WiFi.h>
#include <esp_now.h>
#include "bigFont.h"
#include "Noto.h"
#include "smallFont.h"
#include "Wire.h"
#include "M5Unified.h"
#include "M5GFX.h"
#include "M5_ADS1115.h"



#define M5_UNIT_VMETER_I2C_ADDR             0x49
#define M5_UNIT_VMETER_EEPROM_I2C_ADDR      0x53
#define M5_UNIT_VMETER_PRESSURE_COEFFICIENT 0.015918958F

ADS1115 Vmeter;
float resolution         = 0.0;
float calibration_factor = 0.0;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
String my_adress="";
esp_now_peer_info_t peerInfo;
#define EEPROM_SIZE 10

#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

//colors
unsigned short grays[15];
unsigned short blue[2]={0x09AA,0x00C4};
unsigned short orange=0xA3C0;
unsigned short red=0xB800;
unsigned short green=0x1383;
unsigned short onOffCol[2]={red,green};

int posx[16]={40,80,120,160,40,80,120,160,40,80,120,160,40,80,120,160};
int posy[16]={56,56,56,56,96,96,96,96,136,136,136,136,176,176,176,176};
String lbl[16]={"1","2","3","A","4","5","6","B","7","8","9","C","0","D","E","F"};
String macStr[6]={"0a","0a","0a","0a","0a","0a"};
String remoteLbl[2]={"OFF","ON"};
String modeLbl[2]={"SOLAR","SOLAR"};

bool mode=0; //0=reciever, 1 is sender
bool setMode=0;
int deb=0;
int chosenAdd=0; 
bool ind=0;
long oldPosition = -999; // neede for rotary encoder
long old=-999; //also for rotary
bool first=0;
int brightness[12]={14,20,30,40,50,60,80,100,130,160,200,245};
int bri=3;

int aniFrame=0;
int ani=0;
float voltage=0.00;
int volBar=0;

double rad=0.01745;
float x[10];
float y[10];
float px[10];
float py[10];
int r=8;
int sx=28;
int sy=140;

int timeWait=0;
String sta="UNKNOWN";

unsigned long period=10000;  // time board will be awake in millseconds

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if(status == ESP_NOW_SEND_SUCCESS) 
  sta="SUCCESS"; else sta="FAIL";
}


void initEspNow()
{
 
    // Initialize Wi-Fi in STA mode and disconnect
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(50);  // Slightly longer delay to ensure Wi-Fi mode changes are stable

    // Check if ESP-NOW initialization is successful
    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      esp_now_deinit(); // Deinitialize ESP-NOW if it was already initialized
      delay(10);
      return;
    }
    Serial.println("ESP-NOW Initialized");

    // Register the send callback function
    esp_now_register_send_cb(OnDataSent);

    // Remove existing peer to prevent conflicts
    if (esp_now_del_peer(broadcastAddress) == ESP_OK) {
      Serial.println("Existing peer removed successfully");
    }

    // Set up peer information
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  // Channel 0 means the current channel
    peerInfo.encrypt = false;

    // Add the peer and check for success
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Failed to add peer");
      return;
    }
    Serial.println("Peer added successfully");
 
}


void setup() {
    auto cfg = M5.config();
    M5Dial.begin(cfg, true, true);

    Serial.begin(115200);
    
    esp_sleep_enable_timer_wakeup(60 * 1000000);
    gpio_wakeup_enable(GPIO_NUM_42, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();

    // Connect the Voltmeter Unit to PORT A（RED)
    mode=Vmeter.begin(&Wire, M5_UNIT_VMETER_I2C_ADDR, SDA, SCL, 400000U);
    delay(100);
    
    if(mode){
    Vmeter.setEEPROMAddr(M5_UNIT_VMETER_EEPROM_I2C_ADDR);
    Vmeter.setMode(ADS1115_MODE_SINGLESHOT);
    Vmeter.setRate(ADS1115_RATE_8);
    Vmeter.setGain(ADS1115_PGA_2048);
    // | PGA      | Max Input Voltage(V) |
    // | PGA_6144 |        128           |
    // | PGA_4096 |        64            |
    // | PGA_2048 |        32            |
    // | PGA_512  |        16            |
    // | PGA_256  |        8             |
    resolution = Vmeter.getCoefficient() / M5_UNIT_VMETER_PRESSURE_COEFFICIENT;
    calibration_factor = Vmeter.getFactoryCalibration();

    for(int i=0;i<6;i++){
    macStr[i]=String(broadcastAddress[i],HEX);
    }

    initEspNow();
    }

    
    M5Dial.Display.setBrightness(brightness[bri]);
    spr.createSprite(240,240);

        int co=225;
        for(int i=0;i<15;i++)
        {grays[i]=tft.color565(co, co, co);
        co=co-15;}

        for(int i=0;i<8;i++)
    {
       x[i]=((r-1)*cos(rad*(i*45)))+sx;
       y[i]=((r-1)*sin(rad*(i*45)))+sy;
       px[i]=(r*cos(rad*(i*45)))+sx;
       py[i]=(r*sin(rad*(i*45)))+sy;
    }
  draw();      
}



void draw()
{
  spr.fillSprite(TFT_BLACK);
  
  //UPER SHAPE
  spr.fillRect(10,8,100,42,blue[mode]);
  spr.fillRect(110,18,10,22,blue[mode]);
  spr.fillTriangle(110,8,110,18,120,18,blue[mode]);
  spr.fillTriangle(110,49,110,40,120,40,blue[mode]);
  
  spr.fillSmoothRoundRect(70, 26, 36, 17, 7, onOffCol[1]);
  spr.fillSmoothCircle(78+(1*19), 34, 5, grays[2],onOffCol[1]);
  
  //BOTTOM SHAPE
  spr.fillRect(95,194,124,42,blue[mode]);
  spr.fillRect(85,204,10,22,blue[mode]);
  spr.fillTriangle(85,204,95,194,95,204,blue[mode]);
  spr.fillTriangle(85,226,95,226,95,236,blue[mode]);
  spr.fillSmoothRoundRect(96, 198, 80, 24, 5, TFT_BLACK,blue[mode]);
  spr.fillSmoothRoundRect(100, 202, 30, 16, 3, red,blue[mode]);

  //UPER LINE
  spr.drawWedgeLine(10,60,118,60,1,1,grays[6]);
  spr.drawWedgeLine(118,60,124,74,1,1,grays[6]);
  spr.drawWedgeLine(124,75,216,75,1,1,grays[6]);
  spr.drawWedgeLine(216,75,224,60,1,1,grays[6]);

  // BOTTOM LINE
  spr.drawWedgeLine(60,145,240,145,1,1,grays[6]);
  spr.drawWedgeLine(60,146,42,168,1,1,grays[6]);
  spr.drawWedgeLine(42,168,10,168,1,1,grays[6]);

  //wake time
  int longLL=map(period-millis(),0,10000,0,90);
  spr.fillRoundRect(20,68,longLL,3,2,0x1311);


  //graph region
   for(int i=0;i<14;i++)
   spr.drawWedgeLine(60+(i*12),167,70+(i*12),152,3,3,grays[11]);
   for(int i=0;i<map(voltage,0,15,0,14);i++)
   spr.drawWedgeLine(60+(i*12),167,70+(i*12),152,3,3,grays[6]);
   spr.fillRect(66,148,164,6,TFT_BLACK);
   spr.fillRect(56,167,164,6,TFT_BLACK);


  spr.setTextDatum(0);
  spr.setTextColor(grays[1],TFT_BLACK);
  spr.loadFont(Noto);
  spr.drawString("MONITOR",126,57);
  
  spr.setTextColor(grays[4],TFT_BLACK);
  spr.drawString("MAC",46,198);
  spr.setTextColor(TFT_WHITE,red);
  spr.drawString("ESP",102,204);
  spr.setTextColor(TFT_WHITE,TFT_BLACK);
  spr.drawString("NOW",132,204);
  
 
  spr.setTextColor(grays[3],blue[mode]);
  spr.drawString(remoteLbl[1],38,28);
 
   spr.setTextColor(grays[1],0x8000);
   spr.fillSmoothRoundRect(126,23,74,17,3,0x8000,TFT_BLACK);
  spr.drawString(modeLbl[mode],128,25);   // sender or reciever

   for(int i=0;i<9;i++)
    {
    if(i==ani) 
    spr.setTextColor(TFT_ORANGE,TFT_BLACK); 
    else
    spr.setTextColor(orange,TFT_BLACK);
    spr.drawString(">",126+(i*10),42);
    }
  spr.unloadFont();

   spr.setTextDatum(0);
  //spr.loadFont(smallFont);
  spr.setTextColor(grays[10],TFT_BLACK);

  
   for(int i=0; i<6;i++)
   spr.drawString("0x"+String(broadcastAddress[i],HEX)+",",30+(i*30),176);
 

    spr.setTextColor(grays[6],TFT_BLACK);
    spr.drawString("RECIEVER",30,188);
    spr.setTextColor(grays[3],blue[mode]);
    spr.drawString("REMOTE",70,14);
    spr.drawString("SETUP",132,224);

    spr.drawLine(10,100,60,100,grays[11]);
    spr.setTextColor(grays[10],TFT_BLACK);

    spr.drawString("DELIVERY",10,90);
    spr.drawString(sta,10,103);
   
    
  spr.setTextDatum(4);
  spr.setTextColor(TFT_WHITE,TFT_BLACK);
  spr.loadFont(bigFont);
  spr.drawString(String(voltage),146,118);
  spr.unloadFont();

 // brightness region

   spr.fillSmoothCircle(sx,sy,5,0x82E0,TFT_BLACK);
   for(int i=0;i<8;i++)
   spr.drawWedgeLine(px[i],py[i],x[i],y[i],1,1,0x82E0,TFT_BLACK);
   for(int i=0;i<12;i++)
   spr.drawSmoothArc(28,140,20,14,i+(i*30),i+(i*30)+24,grays[11],TFT_BLACK);
   for(int i=0;i<bri;i++)
   spr.drawSmoothArc(28,140,20,14,i+(i*30),i+(i*30)+24,orange,TFT_BLACK);


  M5Dial.Display.pushImage(0,0,240,240,(uint16_t*)spr.getPointer());
}

void loop() {
  
    // animation arrown
    aniFrame++;
    if(aniFrame>2) 
    {aniFrame=0;
    ani++; if(ani>8) ani=0;}

    M5Dial.update();

   if(mode==1 && setMode==0 && timeWait==0 )
    {
     int16_t adc_raw = Vmeter.getSingleConversion();
     voltage   = adc_raw * resolution * calibration_factor;
     voltage=voltage/1000.00;
     esp_now_send(broadcastAddress, (uint8_t *) &voltage, sizeof(voltage));
    }
    timeWait++;
    if(timeWait==10)
    timeWait=0;


    if (M5Dial.BtnA.wasPressed() && mode==1)
    {
    period=millis()+10000;
    }


    auto t = M5Dial.Touch.getDetail();


    long newP = M5Dial.Encoder.read();
    if (newP > old) {
      period=millis()+10000;
      M5Dial.Speaker.tone(8000, 20);
      old = newP;
      bri++;
      if(bri>12)
      bri=0;
      M5Dial.Display.setBrightness(brightness[bri]);
      draw();
     } 
       
      if (newP < old) {
        period=millis()+10000;
      M5Dial.Speaker.tone(8000, 20);
      old = newP;
      bri--;
      if(bri<0)
      bri=12;
      M5Dial.Display.setBrightness(brightness[bri]);
      draw();
     } 
     
      if(t.isPressed() ){
        period=millis()+10000;     
        draw();
      }


if(millis()>period)
{
M5Dial.Display.setBrightness(0);
esp_light_sleep_start();
M5Dial.Display.setBrightness(brightness[bri]);
period=millis()+6000;
esp_restart();
}

draw();

}
