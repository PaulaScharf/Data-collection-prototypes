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

unsigned long timer = 0;

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

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}


void setup() {
  Serial.begin(115200);
  
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

  //listDir(SD, "/", 0);
  //createDir(SD, "/mydir");
  //removeDir(SD, "/mydir");
  //writeFile(SD, "/hello.txt", "Hello ");
  //appendFile(SD, "/hello.txt", "World!\n");
  //readFile(SD, "/mpu6050.csv");
  //deleteFile(SD, "/foo.txt");
  //renameFile(SD, "/hello.txt", "/foo.txt");
  //testFileIO(SD, "/test.txt");
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));

  // mpu6050 
  Wire.begin(I2C_PIN_SDA, I2C_PIN_SCL);
  
  while(!Serial);
  mpu.setAddress(0x68); // set I2C address
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while(status!=0){ } // stop everything if could not connect to MPU6050
  
  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  mpu.calcOffsets(true,true); // gyro and accelero
  Serial.println("Done!\n");

  writeFile(SD, "/mpu6050.csv", "Timestamp;Temp;ACCELERO X;ACCELERO Y;ACCELERO Z;GYRO X;GYRO Y;GYRO Z;ACC ANGLE X;ACC ANGLE Y;ANGLE X;ANGLE Y; Angle Z;\n");
  
  while (true) {
      timer = millis();
      mpu.update();

      float temp = mpu.getTemp(); 
      float AccX = mpu.getAccX(); 
      float AccY = mpu.getAccY();
      float AccZ = mpu.getAccZ();
      float GyroX = mpu.getGyroX();
      float GyroY = mpu.getGyroY(); 
      float GyroZ = mpu.getGyroZ();  
      float AccAngleX = mpu.getAccAngleX(); 
      float AccAngleY = mpu.getAccAngleY();
      float AngleX = mpu.getAngleX(); 
      float AngleY = mpu.getAngleY();
      float AngleZ = mpu.getAngleZ();

      String append = ""; 
      Serial.print("Values: ");

      append += String(timer, 6); 
      append += ";";
      append += String(temp, 6);
      append += ";"; 
      append += String(AccX, 6);
      append += ";";
      append += String(AccY, 6);
      append += ";";
      append += String(AccZ, 6);
      append += ";";
      append += String(GyroX, 6);
      append += ";";
      append += String(GyroY, 6);
      append += ";";
      append += String(GyroZ, 6);
      append += ";";
      append += String(AccAngleX, 6);
      append += ";";
      append += String(AccAngleY, 6);
      append += ";";
      append += String(AngleX, 6);
      append += ";";
      append += String(AngleY, 6);
      append += ";";
      append += String(AngleZ, 6);
      append += ";\n"; //

      Serial.println(append);

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
