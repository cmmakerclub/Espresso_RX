float ax = 0;
float ay = 0;
float az = 0;
float gx = 0;
float gy = 0;
float gz = 0;
float mx = 0;
float my = 0;
float mz = 0;
float yaw, pitch, roll, throttle;

#define Rad2Degree       57.27272727272727f

#include "adaf_9dof.h"
#include <AuthClient.h>
#include <MicroGear.h>
#include <MQTTClient.h>
#include <SHA1.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <MicroGear.h>
//#include "mpu_6050.h"
#include "oled.h"
//#include "mhc5883l.h"


const char* ssid     = "NAT.WRTNODE";
const char* password = "devicenetwork";


#define APPID       "DroneNETPIE"
#define GEARKEY     "gaLObmI5p3Nsipr"
#define GEARSECRET  "tuVDXPN8QJYRtCQzrA8yeQwhxu8fUA"
#define SCOPE       "rw:/drone"

WiFiClient client;
AuthClient *authclient;

uint8_t led_state = 0;
uint8_t watch_dog = 0;
uint8_t connect_state = 0;
unsigned long time_now = 0;
unsigned long time_prev_netpie = 0;
unsigned long time_prev_sensors = 0;
float a, b, c, d;
float set_a, set_b, set_c, set_d;

MicroGear microgear(client);

void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  // Serial.print("Incoming message --> ");
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
  // microgear.subscribe("/drone");
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
  pinMode(13,INPUT_PULLUP);
  pinMode(15,OUTPUT);
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


  accel.begin();
  //setup_mpu();
  display.print("Acceleration...");
  display.display();
  delay(500);
  display.println(" OK!");
  display.display();
  delay(100);

  mag.begin();
  //setup_mhc5883l() ;
  display.print("Magnetic...");
  display.display();
  delay(500);
  display.println(" OK!");
  display.display();
  delay(100);

  if (WiFi.begin(ssid, password)) 
  {
    uint8_t x = 0;
    display.print("WIFI");
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
        display.println("Acceleration... OK!");
        display.println("Magnetic... OK!");
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

      float dt = time_now - time_prev_sensors;
      if(dt >= 10)                                 //  lower than 100 Hz
      {   
        loop_sensors(dt);
        time_prev_sensors = time_now;
      }

      if(time_now - time_prev_netpie >= 100)     // 10Hz
      {
        time_prev_netpie = time_now;
        loop_remote();
        loop_netpie();

      }
    }
  }

  if(time_now - time_prev_netpie > 5000)
  {
    microgear.connect(APPID);
    time_prev_netpie = time_now;
  } 
  delay(500);

}

void loop_netpie()
{

  if (microgear.connected()) 
  {
    digitalWrite(16, 0);
    microgear.loop();

    int8_t buffer_yaw = 0;
    int8_t buffer_pitch = 0;
    int8_t buffer_roll = 0;
    int8_t buffer_throttle = 0;


    //if(watch_dog > 0)
    {
      watch_dog --;

      buffer_yaw = set_a*3;
      buffer_pitch = set_b*3;
      buffer_roll = set_c*3;      
      if (!digitalRead(13)) buffer_throttle = 1;

      if (buffer_yaw > 100) buffer_yaw = 100;
      if (buffer_pitch > 100) buffer_pitch = 100;
      if (buffer_roll > 100) buffer_roll = 100;
      if (buffer_yaw < -100) buffer_yaw = -100;
      if (buffer_pitch < -100) buffer_pitch = -100;
      if (buffer_roll < -100) buffer_roll = -100;

    }

    char ascii[32];

    // ascii[1] = (int8_t)throttle;
    // ascii[2] = (int8_t)buffer_yaw;
    // ascii[3] = (int8_t)buffer_pitch;
    // ascii[4] = (int8_t)buffer_roll;
    // ascii[4] = 0xfe;
    // ascii[5]  =0xfe;
    // ascii[5] = '\0';

     sprintf(ascii,"%d,,%d,,%d,,%d,,ss", (int8_t)buffer_throttle, (int8_t)buffer_yaw,(int8_t)buffer_pitch,(int8_t)buffer_roll);

    microgear.publish("/drone", ascii);


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
  //loop_mhc5883l();
  static float yaw_prev, yaw_now, sum_shift;

  loop_9dof();

  if (yaw_prev - yaw < -300) sum_shift -= 360;
  if (yaw_prev - yaw >  300) sum_shift += 360;
  
  yaw_now = yaw + sum_shift;
  
  a = smooth_fliter(0.4, yaw_now, a);
  b = smooth_fliter(0.4, pitch, b);
  c = smooth_fliter(0.4, roll, c);
  // d = smooth_fliter(0.4, ax, d);
  // e = smooth_fliter(0.4, ay, e);
  // f = smooth_fliter(0.4, az, f);

  yaw_prev = yaw;
}


void loop_remote(void)
{    

  static float ba, bb, bc;

//   static float R1[3][3] = {0};
//   static float R2[3][3] = {0};
//   float norm = sqrtf(a*a + b*b + c*c);
//   norm = 1 / norm; 
//   float vmx = a*norm;
//   float vmy = b*norm;
//   float vmz = c*norm;   

//   norm = sqrtf(d*d + e*e + f*f);
//   norm = 1 / norm; 
//   float vax = d*norm;
//   float vay = e*norm;
//   float vaz = f*norm;   




// if (!digitalRead(0))
// {
//   float az = -atan2f(my,mx);

//   R1[0][0] = cosf(az);
//   R1[0][1] = -sinf(az);
//   R1[0][2] = 0;
//   R1[1][0] = sinf(az);
//   R1[1][1] = cosf(az);
//   R1[1][2] = 0;
//   R1[2][0] = 0;
//   R1[2][1] = 0;
//   R1[2][2] = 1;

//   float mmx = R1[0][0]*mx + R1[0][1]*my + R1[0][2]*mz;
//   float mmy = R1[1][0]*mx + R1[1][1]*my + R1[1][2]*mz;
//   float mmz = R1[2][0]*mx + R1[2][1]*my + R1[2][2]*mz;

//   float ay = -atan2f(mmz,mmx);

//   R2[0][0] = cosf(ay)*cosf(az);
//   R2[0][1] = -cosf(ay)*sinf(az);
//   R2[0][2] = sinf(ay);
//   R2[1][0] = sinf(az);
//   R2[1][1] = cosf(az);
//   R2[1][2] = 0;
//   R2[2][0] = -sinf(ay)*cosf(az);
//   R2[2][1] = sinf(ay)*sinf(az);;
//   R2[2][2] = cosf(ay);

// }

//   float mmx = R1[0][0]*mx + R1[0][1]*my + R1[0][2]*mz;
//   float mmy = R1[1][0]*mx + R1[1][1]*my + R1[1][2]*mz;

//   yaw = -atan2f(mmy, mmx)*Rad2Degree;

//   ax = R2[0][0]*mx + R2[0][1]*my + R2[0][2]*mz;
//   ay = R2[1][0]*mx + R2[1][1]*my + R2[1][2]*mz;
//   az = R2[2][0]*mx + R2[2][1]*my + R2[2][2]*mz;

//   roll  = atan2f(-ax, az)*Rad2Degree;
//   pitch = atan2f(ay*ay, (ax*ax + az*az))*Rad2Degree;


      if (!digitalRead(0)) // set zero
      {
        ba = a;
        bb = b;
        bc = c;
      }

      set_a = a - ba;
      set_b = b - bb;
      set_c = c - bc;
      set_d = 0 ;

      display.clearDisplay();
      display.setCursor(0,0); 
 
      display.println(set_a);
      display.println(set_b);
      display.println(set_c);

      display.display(); 
}

float smooth_fliter(float beta, float new_data, float prev_data)
{
  float data = prev_data + beta*(new_data - prev_data);
  return data;
}