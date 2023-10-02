#include <MPU6050_light.h>

/* Get all possible data from MPU6050 and store the data on a sd card 
 * Accelerometer values are given as multiple of the gravity [1g = 9.81 m/s²]
 * Gyro values are given in deg/s
 * Angles are given in degrees
 * Note that X and Y are tilt angles and not pitch/roll.
 *
 * License: MIT
 */

#include "Wire.h"
#include <MPU6050_light.h>

#define I2C_PIN_SCL 42
#define I2C_PIN_SDA 45

MPU6050 mpu(Wire);

/*
 * Connect the SD card to the following pins:
 *
 * SD Card | ESP32
 *    D2       -
 *    D3       SS
 *    CMD      MOSI
 *    VSS      GND
 *    VDD      3.3V
 *    CLK      SCK
 *    VSS      GND
 *    D0       MISO
 *    D1       -
 */
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#define VSPI_MISO   13
#define VSPI_MOSI   11
#define VSPI_SCLK   12
#define VSPI_SS     10
#define SD_ENABLE   9

#include "Freenove_WS2812_Lib_for_ESP32.h"
#define LED_PIN 1
Freenove_ESP32_WS2812 led = Freenove_ESP32_WS2812(1, LED_PIN, 0, TYPE_GRB);

int start = 0;
long measurementStartTime = 0;
long timer = 0;

void setLED(uint8_t r,uint8_t g,uint8_t b) {
  led.setLedColorData(0, r, g, b);
  led.show();
}


String append;
void setup() {
  Serial.begin(115200);

  // Start the I2C Bus as Slave on address 9
  Wire.begin(39,40,100000); 

  led.begin();
  led.setBrightness(30);  
  setLED(60,60,0);
  
  // sd card 
  pinMode(SD_ENABLE,OUTPUT);
  digitalWrite(SD_ENABLE,LOW);
  delay(2000);
  SPIClass sdspi = SPIClass();
  sdspi.begin(VSPI_SCLK,VSPI_MISO,VSPI_MOSI,VSPI_SS);
  if(!SD.begin(VSPI_SS,sdspi)){
      Serial.println("Card Mount Failed");
      return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
      Serial.println("No SD card attached");
      return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
      Serial.println("MMC");
  } else if(cardType == CARD_SD){
      Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
      Serial.println("SDHC");
  } else {
      Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
  
  mpu.setAddress(0x68); // set I2C address
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  Serial.println("finished setting up");
  Serial.println("waiting for start signal...");
  for (int i =0; i<3; i++) {
    setLED(60,0,0);
    delay(200);
    setLED(60,60,60);
    delay(200);
  };
  Wire.beginTransmission(9); // transmit to device #9
  Wire.write(1);              // sends x 
  Wire.endTransmission();    // stop transmitting
  measurementStartTime = millis();
  setLED(0,0,60);
  
  // mpu6050 
  Wire.begin(I2C_PIN_SDA, I2C_PIN_SCL);
  
  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  mpu.calcOffsets(true,true); // gyro and accelero
  Serial.println("Done!\n");
  setLED(0,60,0);

  writeFile(SD, "/mpu6050.csv", "Timestamp;ACCELERO X;ACCELERO Y;ACCELERO Z;GYRO X;GYRO Y;GYRO Z;ACC ANGLE X;ACC ANGLE Y;ANGLE X;ANGLE Y; Angle Z;\n");
  
  while (start=1) {
      timer = measurementStartTime-millis();
      mpu.update();

      append = ""; 
    //   Serial.print("Values: ");

      append += String(timer, 6); 
      append += ";";
      append += String(mpu.getAccX(), 6);
      append += ";";
      append += String(mpu.getAccY(), 6);
      append += ";";
      append += String(mpu.getAccZ(), 6);
      append += ";";
      append += String(mpu.getGyroX(), 6);
      append += ";";
      append += String(mpu.getGyroY(), 6);
      append += ";";
      append += String(mpu.getGyroZ(), 6);
      append += ";";
      append += String(mpu.getAccAngleX(), 6);
      append += ";";
      append += String(mpu.getAccAngleY(), 6);
      append += ";";
      append += String(mpu.getAngleX(), 6);
      append += ";";
      append += String(mpu.getAngleY(), 6);
      append += ";";
      append += String(mpu.getAngleZ(), 6);
      append += ";\n"; //

    //   Serial.println(append);

      File file;
      file = SD.open("/mpu6050.csv", FILE_APPEND);
      if(!file){
        Serial.println("Failed to open file for appending");
        return;
      }
      if(!file.print(append)){
        Serial.println("Append failed");
        file.close();
        file = SD.open("/mpu6050.csv", FILE_APPEND);
        if(!file){
          Serial.println("Failed to open file for appending");
          return;
        }
      }
  };
}

void loop() { 
}
