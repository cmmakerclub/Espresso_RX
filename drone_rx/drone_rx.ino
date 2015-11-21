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

#define APPID       "DroneNETPIE"
#define GEARKEY     "gaLObmI5p3Nsipr"
#define GEARSECRET  "tuVDXPN8QJYRtCQzrA8yeQwhxu8fUA"
#define SCOPE       "rw:/drone"

int rx_buffer[4] = {0};

WiFiClient client;
AuthClient *authclient;

int timer = 0;
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

  msg[msglen] = '\0';
  Serial.println((char *)msg);
  Serial.print("   ");
  Serial.print(rx_buffer[0]);
  Serial.print(rx_buffer[1]);
  Serial.print(rx_buffer[2]);
  Serial.println(rx_buffer[3]);
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
  microgear.subscribe("/drone");
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
}

void loop() 
{



  if (microgear.connected()) {
    //Serial.println("connected");
    microgear.loop();
    if (timer >= 1000) {
      //      Serial.println("Publish...");
      //      microgear.publish("/drone", "publish_hello_too");

      //      microgear.chat("kk_0x", "Hello");
      timer = 0;
    }
    else timer += 100;
  }
  else {
    Serial.println("connection lost, reconnect...");
    if (timer >= 5000) {
      microgear.connect(APPID);
      timer = 0;
    }
    else timer += 100;
  }
  delay(100);
}
