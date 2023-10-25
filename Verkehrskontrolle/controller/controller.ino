#include "Wire.h"
#include "Freenove_WS2812_Lib_for_ESP32.h"
#define LED_PIN 1
Freenove_ESP32_WS2812 led = Freenove_ESP32_WS2812(1, LED_PIN, 0, TYPE_GRB);
void setLED(uint8_t r,uint8_t g,uint8_t b) {
  led.setLedColorData(0, r, g, b);
  led.show();
}
// Include the required Wire library for I2C<br>#include 
void setup() {
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);
  Serial.begin(921600);
  led.begin();
  led.setBrightness(30);  
  setLED(60, 0, 0);
  // while (!Serial) ;
  // Start the I2C Bus as Master
  Wire.begin(39, 40, 100000);

  pinMode(2, OUTPUT);
  for(int i = 3; i>0; i--) {
    Serial.println(i);
    delay(500);
  }
  Serial.println("sending 1");
  digitalWrite(2, LOW);
  delay(200);
  digitalWrite(2, HIGH);
  Wire.beginTransmission(9); // transmit to device #9
  Wire.write(1);              // sends x 
  Wire.endTransmission();    // stop transmitting
   Wire.beginTransmission(41); // transmit to device #9
  Wire.write(1);              // sends x 
  Wire.endTransmission();    // stop transmitting
  setLED(0, 60, 0);
  for(int i = 20; i>0; i--) {
    Serial.println(i);
    delay(1000);
  }
  Serial.println("sending 0");
  digitalWrite(2, LOW);
  Wire.beginTransmission(9); // transmit to device #9
  Wire.write(2);              // sends x 
  Wire.endTransmission();    // stop transmitting
   Wire.beginTransmission(41); // transmit to device #9
  Wire.write(2);              // sends x 
  Wire.endTransmission();    // stop transmitting
  setLED(0, 0, 60);
}
void loop() {
  delay(500);
}