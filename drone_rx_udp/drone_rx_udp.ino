
/* Create a WiFi access point and provide a web server on it. */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <ESP_Adafruit_SSD1306.h>


#define buffer_size 100
#define OLED_RESET 4

#define period_udp 10
#define period_led 1000

Adafruit_SSD1306 display(OLED_RESET);

/* Set these to your desired credentials. */
const char *ssid = "kkao_server";
const char *password = "12345678";

unsigned int localPort = 12345;
unsigned long time_now, time_prev_udp, time_prev_led;
int cb = 1;



char packetBuffer[buffer_size] = {0}; //buffer to hold incoming and outgoing packets
WiFiUDP udp;
int count = 0;

IPAddress local_ip(192, 168, 5, 1);
IPAddress gateway(192, 168, 5, 1);
IPAddress subnet(255, 255, 255, 0);


void Read_UDP();
void setup_oled();

void setup()
{
  pinMode(12, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(16, OUTPUT);
  digitalWrite(16, 1);

  setup_oled();
  delay(3000);
  display.clearDisplay();

  Serial.begin(115200);


  display.print("Configuring access point...");
  display.display();
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);
  delay(500);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(500);
  IPAddress myIP = WiFi.softAPIP();
  display.print("AP IP: ");
  display.println(myIP);
  display.display();

  if ( udp.begin(localPort) > 0) {
    display.print("UDP star port :");
    display.println(udp.localPort());
    display.display();
  } else {
    display.println("Error");
    display.display();
  }
  display.println("ssid:kkao_server");
  display.println("pass:12345678");
  display.println("active");
  display.display();
  delay(3000);

  time_now = millis();
  time_prev_udp = time_now;
  time_prev_led = time_now;

}

void loop()
{
  time_now = millis();

  if (time_now - time_prev_udp >= period_udp)
  {
    time_prev_udp = time_now;

    digitalWrite(12, 1);		// debug
    Read_UDP();
    digitalWrite(12, 0);    // debug
  }

  if (time_now - time_prev_led >= period_led)    // 10Hz
  {
    time_prev_led = time_now;

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("CMMC.Espresso_lite");
		display.println("");	
    display.print((int8_t)packetBuffer[0]);
    display.print("  ");
    display.print((int8_t)packetBuffer[1]);
    display.print("  ");
    display.print((int8_t)packetBuffer[2]);
    display.print("  ");
    display.print((int8_t)packetBuffer[3]);
    display.print("                            ");
    display.display();

  }

  delay(1);
}

void Read_UDP()
{
  cb = udp.parsePacket();
  if (!cb) {

  }
  else {
    // Serial.print("length=");
    // Serial.println(cb);

    memset(packetBuffer, 0, buffer_size); // clear mem

    udp.read(packetBuffer, cb); // read the packet into the buffer
    String temp = packetBuffer;
    Serial.println(packetBuffer);

    // if (count > 8)
    // {
    //   count = 0;
    //   display.clearDisplay();
    //   display.setCursor(0, 0);
    // }
    // count++;

    // display.print((int)packetBuffer[0]);
    // display.print("  ");
    // display.print((int)packetBuffer[1]);
    // display.print("  ");
    // display.print((int)packetBuffer[2]);
    // display.print("  ");
    // display.print((int)packetBuffer[3]);
    // display.println("  ");
    // display.display();

    // Serial.print((int8_t)packetBuffer[0]);
    // Serial.print("  ");
    // Serial.print((int8_t)packetBuffer[1]);
    // Serial.print("  ");
    // Serial.print((int8_t)packetBuffer[2]);
    // Serial.print("  ");
    // Serial.print((int8_t)packetBuffer[3]);
    // Serial.println("  ");


    Serial.write((int8_t)packetBuffer[0]);  // roll
    Serial.write((int8_t)packetBuffer[1]);  // pitch
    Serial.write((int8_t)packetBuffer[2]);	// throttle
    Serial.write((int8_t)packetBuffer[3]);	// yaw
    Serial.write(0xfe);
    digitalWrite(16, !digitalRead(16));



  }
}

void setup_oled()
{

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
  Wire.setClock(800000);
  // Clear the buffer.
  //display.clearDisplay();

  display.display();
  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);

}


