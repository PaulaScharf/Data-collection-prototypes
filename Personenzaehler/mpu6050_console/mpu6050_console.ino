#include "WiFi.h"
#include "time.h"
#include "Wire.h"
#include <Adafruit_MPU6050.h>

Adafruit_MPU6050 mpu;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

#include "Freenove_WS2812_Lib_for_ESP32.h"
#define LED_PIN 1
Freenove_ESP32_WS2812 led = Freenove_ESP32_WS2812(1, LED_PIN, 0, TYPE_GRB);

const char* ssid       = "SSID";
const char* password   = "Password";

long measurementStartTime = 0;
long timer = 0;
struct tm timeinfo;
time_t now;

sensors_event_t a, g, temp;

void setLED(uint8_t r,uint8_t g,uint8_t b) {
  led.setLedColorData(0, r, g, b);
  led.show();
}


String append;
void setup() {
  led.begin();
  led.setBrightness(10);  
  setLED(60,60,0);
  Serial.begin(115200);

  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      // Serial.print(".");
  }
  // Serial.println(" CONNECTED");
  
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);


  led.begin();
  led.setBrightness(10);  
  setLED(60,60,0);
  
  
  led.begin();
  led.setBrightness(10);  
  setLED(60,60,0);
  
  // mpu6050 
  Wire1.begin();

  if (!mpu.begin(0x68, &Wire1)) {
    Serial.println("MPU6050 Chip wurde nicht gefunden");
    return;
  }
  
  // Serial.println("MPU6050 gefunden!");
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  setLED(0,60,0);

  measurementStartTime = millis();
}

void loop() {
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  time(&now);

  mpu.getEvent(&a, &g, &temp);

  append = ""; 
  append += String(now)+","+String(millis()); 
  append += ",";
  append += String(a.acceleration.x, 6);
  append += ",";
  append += String(a.acceleration.y, 6);
  append += ",";
  append += String(a.acceleration.z, 6);
  append += ",";
  append += String(g.gyro.x, 6);
  append += ",";
  append += String(g.gyro.y, 6);
  append += ",";
  append += String(g.gyro.z, 6);
  append += "\n"; //

  setLED(60,0,0);
  Serial.print(append);
  delay(1); 
  setLED(0,60,0);
}
