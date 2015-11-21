#include <AuthClient.h>
#include <MicroGear.h>
#include <MQTTClient.h>
#include <SHA1.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <MicroGear.h>
#include "oled.h"


const char* ssid     = "NAT.WRTNODE";
const char* password = "devicenetwork";

#define APPID       "DroneNETPIE"
#define GEARKEY     "gaLObmI5p3Nsipr"
#define GEARSECRET  "tuVDXPN8QJYRtCQzrA8yeQwhxu8fUA"
#define SCOPE       "rw:/drone"

uint8_t connect_state = 0;
unsigned long time_now = 0;
unsigned long time_prev_netpie = 0;
unsigned long time_prev_sensors = 0;
int rx_buffer[4] = {0};
float yaw, pitch, roll, throttle;
WiFiClient client;
AuthClient *authclient;

uint timer = 0;
MicroGear microgear(client);

void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) 
{
  //Serial.print("Incoming message --> ");
  int8_t rx_index = 0;
  int8_t char_index = 0;
  for (int i = 0 + 1; i < msglen; i++)
  {
    if((msg[i-1] == 0x2c)&&(msg[i] == 0x2c))
    {
      char buffer[] = {"      "};
      int loop = 0;
      for (loop = 0; loop < i-char_index; loop++) buffer[loop] = msg[char_index+loop];
      buffer[loop + 1]  = '\0';
      rx_buffer[rx_index] = atoi(buffer);
      char_index = i+1;
      rx_index++;
    }
  }

  // throttle = rx_buffer[0];
  // yaw = rx_buffer[1];
  // pitch = rx_buffer[2];
  // roll = rx_buffer[3];

  // msg[msglen] = '\0';
  // Serial.println((char *)msg);
  // Serial.print("   ");
  Serial.write(rx_buffer[3]);
  Serial.write(rx_buffer[2]);
  Serial.write(rx_buffer[0]);
  Serial.write(rx_buffer[1]);
  Serial.write(0xfe);
  digitalWrite(16, !digitalRead(16));

}

void onFoundgear(char *attribute, uint8_t* msg, unsigned int msglen) {
  // Serial.print("Found new member --> ");
  // for (int i = 0; i < msglen; i++)
  //   Serial.print((char)msg[i]);
  // Serial.println();
}

void onLostgear(char *attribute, uint8_t* msg, unsigned int msglen) {
  // Serial.print("Lost member --> ");
  // for (int i = 0; i < msglen; i++)
  // Serial.print((char)msg[i]);
  // Serial.println();
}

void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  //Serial.println("Connected to NETPIE...");
  microgear.setName("RX");
  microgear.subscribe("/drone");
  display.clearDisplay();
  display.setCursor(0,0); 
  display.println("Connected...");
  display.display(); 
  connect_state = 1;
}


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
  display.println("CMMC.Espresso.NETPIE");
  display.display();
  delay(2000);

  display.print("Starting...");
  display.display();
  delay(500);
  display.println(" OK!");
  display.display();
  delay(100);




  if (WiFi.begin(ssid, password)) 
  {

    display.print("WIFI");
    display.display(); 

    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      display.print(".");
      display.display(); 
    }

    display.println("OK!");
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
  display.println("");
  display.print(" MicroGear Connecting.");
  display.display();

  time_now = millis();
  time_prev_netpie = time_now;
}

void loop() 
{
  time_now = millis();


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
      delay(1);
      time_now = millis();

      microgear.loop();

      float dt = time_now - time_prev_sensors;
      if(dt >= 10)                                 //  lower than 100 Hz
      {   
        time_prev_sensors = time_now;


      }

      if(time_now - time_prev_netpie >= 1000)     // 10Hz
      {
        time_prev_netpie = time_now;

        display.clearDisplay();
        display.setCursor(0,0); 
        display.println("CMMC.Espresso.NETPIE");
        display.println(rx_buffer[0]);
        display.println(rx_buffer[1]);
        display.println(rx_buffer[2]);
        display.println(rx_buffer[3]);
        display.display(); 

      }
    }
  }

  if(time_now - time_prev_netpie > 5000)
  {
    microgear.connect(APPID);
    time_prev_netpie = time_now;
  } 
  delay(500);

  // if (microgear.connected()) {
  //   //Serial.println("connected");
  //   microgear.loop();
  //   if (timer >= 1000) {
  //     //      Serial.println("Publish...");
  //     //      microgear.publish("/drone", "publish_hello_too");

  //     //      microgear.chat("kk_0x", "Hello");
  //     timer = 0;
  //   }
  //   else timer += 1;
  // }
  // else 
  // {
  //   display.clearDisplay();
  //   display.setCursor(0,0); 
  //   display.println("connection lost, reconnect...");
  //   display.display(); 

  //   if (timer >= 5000) 
  //   {
  //     microgear.connect(APPID);
  //     timer = 0;
  //   }
  //   else timer += 1;
  // }
  // delay(1);
}
