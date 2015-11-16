#include <AuthClient.h>
#include <MicroGear.h>
#include <MQTTClient.h>
#include <SHA1.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <MicroGear.h>
#include "mpu_6050.h"
#include "oled.h"
#include "mhc5883l.h"


const char* ssid     = "NAT.WRTNODE";
const char* password = "devicenetwork";

#define APPID       "DroneNETPIE"
#define GEARKEY     "gaLObmI5p3Nsipr"
#define GEARSECRET  "tuVDXPN8QJYRtCQzrA8yeQwhxu8fUA"
#define SCOPE       "rw:/drone"

WiFiClient client;
AuthClient *authclient;

uint8_t led_state = 0;

uint8_t connect_state = 0;
unsigned long time_now = 0;
unsigned long time_prev_netpie = 0;
unsigned long time_prev_sensors = 0;

MicroGear microgear(client);

void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  Serial.print("Incoming message --> ");
  msg[msglen] = '\0';
  Serial.println((char *)msg);
}

void onFoundgear(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.print("Found new member --> ");
  for (int i = 0; i < msglen; i++)
    Serial.print((char)msg[i]);
  Serial.println();
}

void onLostgear(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.print("Lost member --> ");
  for (int i = 0; i < msglen; i++)
    Serial.print((char)msg[i]);
  Serial.println();
}

void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  microgear.setName("TX");
  display.println("Connected to NETPIE");
  display.display();
  delay(2000);
  connect_state = 1;
}

void loop_netpie();
void loop_sensors();

void setup() {
  /* Event listener */
  pinMode(16,OUTPUT);
  digitalWrite(16, 1);
  microgear.on(MESSAGE, onMsghandler);
  microgear.on(PRESENT, onFoundgear);
  microgear.on(ABSENT, onLostgear);
  microgear.on(CONNECTED, onConnected);

  Serial.begin(115200);
  setup_oled();

  display.setCursor(0,0); 
  display.print("Starting...");
  display.display();
  delay(500);
  display.println(" OK!");
  display.display();
  delay(100);

  setup_mpu();
  display.print("Init MPU6050...");
  display.display();
  delay(500);
  display.println(" OK!");
  display.display();
  delay(100);

  setup_mhc5883l() ;
  display.print("Init MHC5883L...");
  display.display();
  delay(500);
  display.println(" OK!");
  display.display();
  delay(100);

  if (WiFi.begin(ssid, password)) 
  {
    uint8_t x = 0;
    display.print("Init WIFI");
    display.display(); 

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      display.print(".");
      display.display();
      x++;
      if (x == 8) 
      {
        x = 0;
        display.clearDisplay();
        display.setCursor(0,0); 
        display.println("Starting... OK!");
        display.println("Init MPU6050... OK!");
        display.println("Init MHC5883L... OK!");
        display.print("Init WIFI.");
        display.display(); 
      }
    }
    display.println("");
    display.println("WiFi connected");
    display.print("IP : ");
    display.println(WiFi.localIP());
    display.display();
    //uncomment the line below if you want to reset token -->
    //microgear.resetToken();
    microgear.init(GEARKEY, GEARSECRET, SCOPE);
    microgear.connect(APPID);
  }

  display.clearDisplay();
  display.setCursor(0,0); 
  display.println(" microGear Connecting.");
  display.display();


}

void loop() 
{

  delay(500);
  display.print(".");
  display.display();

  if (connect_state)
  {
    display.clearDisplay();
    display.setCursor(0,0); 
    display.display();

    time_now = millis();
    time_prev_netpie = time_now;
    time_prev_sensors = time_now;


    while(1)
    {
      delay(5);
      time_now = millis();


      loop_sensors();


      if(time_now - time_prev_netpie >= 100)     // 10Hz
      {
        time_prev_netpie = time_now;
        loop_netpie();
      }
    }
  }

}

void loop_netpie()
{

  if (microgear.connected()) 
  {
    microgear.loop();
    digitalWrite(16, led_state);
    led_state = !led_state;


      //      Serial.println("Publish...");
      //      microgear.publish("/drone", "publish_hello_too");
      //      microgear.subscribe("/drone");
      //      microgear.chat("kk_0x", "Hello");
  }
  else 
  {
    digitalWrite(16, 1);
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0,0); 
    display.println("Connection   Lost !");
    display.setTextSize(1);
    display.println("");   
    display.println(" Restart System Plz...");
    display.display();
    delay(500);
    while(1){}
  } 
}

void loop_sensors()
{
  float dt = time_now - time_prev_sensors;
  loop_mpu();
  loop_mhc5883l();










  time_prev_sensors = time_now;
}