#include <AuthClient.h>
#include <MicroGear.h>
#include <MQTTClient.h>
#include <SHA1.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <MicroGear.h>

const char* ssid     = "NAT.WRTNODE";
const char* password = "devicenetwork";

#define APPID       "kkaodevices"
#define GEARKEY     "gaLObmI5p3Nsipr"
#define GEARSECRET  "tuVDXPN8QJYRtCQzrA8yeQwhxu8fUA"
#define SCOPE       ""

uint8_t light = 0;
uint8_t light_prev = 0;

uint32_t time_now = 0;
uint32_t disconnect_timer = 0;
uint32_t count_timer = 0;
WiFiClient client;
AuthClient *authclient;

int timer = 0;
MicroGear microgear(client);

void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  Serial.print("Incoming message --> ");
  if ((char)msg[0] == '1')
  {
    digitalWrite(16, 1);
  }
  else
  {
    digitalWrite(16, 0);
  }
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
  Serial.println("Connected to NETPIE...");
  microgear.setName("RX");
}


void setup() {
  /* Event listener */
  microgear.on(MESSAGE, onMsghandler);
  microgear.on(PRESENT, onFoundgear);
  microgear.on(ABSENT, onLostgear);
  microgear.on(CONNECTED, onConnected);

  Serial.begin(115200);
  Serial.println("Starting...");

  if (WiFi.begin(ssid, password)) {

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    //uncomment the line below if you want to reset token -->
    //microgear.resetToken();
    microgear.init(GEARKEY, GEARSECRET, SCOPE);
    microgear.connect(APPID);
  }

  time_now = millis();
  microgear.subscribe("/cmmc/led");
  pinMode(16, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
}

void loop() {
  //delay(10);
  time_now = millis();
  static uint8_t buttom_prev;
  uint8_t buttom = !digitalRead(2);

  if ((buttom == 1) && (buttom_prev == 0))
  {
    light = !light;
  }
  buttom_prev = buttom;


//    if (time_now - count_timer >= 100)
//    {
//      count_timer = time_now;
//      light = !light;
//    }

  if (microgear.connected())
  {
    //Serial.println("connected");
          digitalWrite(4, 1);
    microgear.loop();
          digitalWrite(4, 0);
    if (light != light_prev)
    {
      light_prev = light;
      Serial.print("Publish...");
      Serial.println(light);

      //sprintf(ascii,"%d,led_status", (int)light);
      //microgear.publish("/cmmc/led", ascii);
      char ascii[16] ;
      sprintf(ascii, "%d", (int)light);
          digitalWrite(5, 1);
      microgear.publish("/cmmc/led", ascii);
                digitalWrite(5, 0);
      //      microgear.subscribe("/cmmc/led");
      //      microgear.chat("kk_0x", "Hello");
    }
  }
  else
  {
    if (time_now - disconnect_timer >= 5000)
    {
      Serial.println("connection lost, reconnect...");
      disconnect_timer = time_now;
      microgear.connect(APPID);
      microgear.subscribe("/cmmc/led");
    }
  }
}


