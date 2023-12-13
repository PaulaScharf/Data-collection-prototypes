#include "WiFi.h"
#include "time.h"
#include "Wire.h"
#include <Adafruit_MPU6050.h>
#include "ESP32Time.h"

Adafruit_MPU6050 mpu;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

ESP32Time rtc(gmtOffset_sec);  // offset in seconds GMT+1

#include "Freenove_WS2812_Lib_for_ESP32.h"
#define LED_PIN 1
Freenove_ESP32_WS2812 led = Freenove_ESP32_WS2812(1, LED_PIN, 0, TYPE_GRB);

const char* ssid       = "SSID";
const char* password   = "PASSWORD";

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
  // ******** LED for debugging *********
  led.begin();
  led.setBrightness(10);
  setLED(60, 60, 0); // yellow
  // **************************

  Serial.begin(921600);

  // ********* Sync time *********
  //connect to WiFi
  WiFi.begin(ssid, password, 6);
  while (WiFi.status() != WL_CONNECTED);
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  // **************************
  
  // ********* mpu6050 *********
  Wire1.begin();
  if (!mpu.begin(0x68, &Wire1)) {
    Serial.println("MPU6050 Chip wurde nicht gefunden");
    return;
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  // **************************
  setLED(0,60,0);

  refreshLocalTimeEverySeconds(0);  // Get the NTP time immediately
}

void loop() {
  refreshLocalTimeEverySeconds(10);  // Get the NTP time every 10 seconds only

  mpu.getEvent(&a, &g, &temp);


  measureEverymSec(10);              // Measure every 10 ms
}

void refreshLocalTimeEverySeconds(int seconds) {
  unsigned long mSeconds = seconds * 1000UL;
  static unsigned long lastTime = 0;
  if (millis() - lastTime >= mSeconds) {
    lastTime = millis();
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
    } else {
      rtc.setTimeStruct(timeinfo);
    }
  }
}

void measureEverymSec(unsigned long msecs) {
  static unsigned long lastMeasurement = 0;
  char message[120];
  if (millis() - lastMeasurement >= msecs) {
    lastMeasurement = millis();
    mpu.getEvent(&a, &g, &temp);
    setLED(60,0,0);
    Serial.printf("%20d, %20d, %2.6f, %2.6f, %2.6f\n",
                  rtc.getEpoch()-3600,
                  lastMeasurement,
                  a.acceleration.x,
                  a.acceleration.y,
                  a.acceleration.z);
    setLED(0,60,0);
  }
}
