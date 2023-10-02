/* Get all possible data from MPU6050
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



void setup() {
  Serial.begin(115200);
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
  
}

void loop() {
  mpu.update();

  Serial.print(mpu.getAccX());
  Serial.print(" ");Serial.print(mpu.getAccY());
  Serial.print(" ");Serial.print(mpu.getAccZ());
  Serial.print(" ");Serial.print(mpu.getGyroX());
  Serial.print(" ");Serial.print(mpu.getGyroY());
  Serial.print(" ");Serial.print(mpu.getGyroZ());
  Serial.print(" ");Serial.print(mpu.getAngleX());
  Serial.print(" ");Serial.print(mpu.getAngleY());
  Serial.print(" ");Serial.println(mpu.getAngleZ());

}