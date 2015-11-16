float ax = 0;
float ay = 0;
float az = 0;
float gx = 0;
float gy = 0;
float gz = 0;
float mx = 0;
float my = 0;
float mz = 0;

#define Rad2Degree       57.27272727272727f

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
float a, b, c;


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
void loop_sensors(float _dt);
void loop_remote();
float smooth_fliter(float beta, float new_data, float prev_data);

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


//  setup_mpu();
  display.print("Starting...");
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
        display.println("CMMC.Espresso.NETPIE");
        display.println("Starting... OK!");
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
  display.println("");
  display.print(" MicroGear Connecting.");
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
      delay(1);
      time_now = millis();

      float dt = time_now - time_prev_sensors;
      if(dt >= 10)                                 //  lower than 100 Hz
      {   
        loop_sensors(dt);
        time_prev_sensors = time_now;
      }

      if(time_now - time_prev_netpie >= 100)     // 10Hz
      {
        time_prev_netpie = time_now;
        loop_netpie();
        loop_remote();
      }
    }
  }

}

void loop_netpie()
{

  if (microgear.connected()) 
  {
    digitalWrite(16, 0);
    microgear.loop();





    //microgear.publish("/drone", "publish_hello_too");
    digitalWrite(16, 1);
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
    display.println("Restart System Plz...");
    display.display();
    delay(500);
    while(1){}
  } 
}

void loop_sensors(float _dt )
{
  //loop_mpu();
  loop_mhc5883l();


  a = smooth_fliter(0.4, mx, a);
  b = smooth_fliter(0.4, my, b);
  c = smooth_fliter(0.4, mz, c);




}


void loop_remote(void)
{    
  static float R[3][3] = {0};

  float norm = sqrtf(a*a + b*b + c*c);
  norm = 1 / norm; 
  float mx = a*norm;
  float my = b*norm;
  float mz = c*norm;   

if (!digitalRead(0))
{
  float az = -atan2(my,mx);

  R[0][0] = cosf(az);
  R[0][1] = -sinf(az);
  R[0][2] = 0;
  R[1][0] = sinf(az);
  R[1][1] = cosf(az);
  R[1][2] = 0;
  R[2][0] = 0;
  R[2][1] = 0;
  R[2][2] = 1;

  float mmx = R[0][0]*mx + R[0][1]*my + R[0][2]*mz;
  float mmy = R[1][0]*mx + R[1][1]*my + R[1][2]*mz;
  float mmz = R[2][0]*mx + R[2][1]*my + R[2][2]*mz;

  float ax = -atan2(mmz,mmx);

  R[0][0] = cosf(az);
  R[0][1] = -sinf(az);
  R[0][2] = 0;
  R[1][0] = cosf(ax)*sinf(az);
  R[1][1] = cosf(ax)*cosf(az);
  R[1][2] = -sinf(ax);
  R[2][0] = sinf(ax)*sinf(az);
  R[2][1] = sinf(ax)*cosf(az);;
  R[2][2] = cosf(ax);


}

  float mmx = R[0][0]*mx + R[0][1]*my + R[0][2]*mz;
  float mmy = R[1][0]*mx + R[1][1]*my + R[1][2]*mz;
  float mmz = R[2][0]*mx + R[2][1]*my + R[2][2]*mz;


        display.clearDisplay();
        display.setCursor(0,0); 
   
        display.println(mx);
        display.println(my);
        display.println(mz);
        display.println("");
        display.println(mmx);
        display.println(mmy);
        display.println(mmz);
        display.display(); 
}

float smooth_fliter(float beta, float new_data, float prev_data)
{
  float data = prev_data + beta*(new_data - prev_data);
  return data;
}