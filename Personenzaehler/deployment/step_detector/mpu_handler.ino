
#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;
sensors_event_t a, g, temp;

void measure(void);
String dataStr = "";

char report[256];
volatile int interruptCount = 0;

// A buffer holding the last 3000 ms of data at 31.25 Hz
// Its meant to be a ring buffer, but because the flattening is so slow, we cant use it like that, and have to empty it after every predicition
const int RING_BUFFER_SIZE = int(399);
float save_data[RING_BUFFER_SIZE][3] = {{0.0,0.0,0.0}};
// Most recent position in the save_data buffer
int begin_index = 0;
// True if there is not yet enough data to run inference
bool pending_initial_data = true;
// How often we should save a measurement during downsampling
int sample_every_n;
// The number of measurements since we last saved one
int sample_skip_counter = 1;



bool SetupMPU() {
  Wire1.begin();
  mpu.begin(0x68, &Wire1);
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

  return true;
}


//int lastReading = 0;
bool ReadMPU(float* input,
                       int length) {
  // Serial.println(millis()-lastReading);
  //lastReading = millis();

  mpu.getEvent(&a, &g, &temp);
  // Serial.print(a.acceleration.x);
  //   Serial.print(" ");
  // Serial.print(a.acceleration.y);
  //   Serial.print(" ");
  // Serial.println(a.acceleration.z);
  save_data[begin_index][0] = a.acceleration.x;
  save_data[begin_index][1] = a.acceleration.y;
  save_data[begin_index][2] = a.acceleration.z;
  begin_index=begin_index+1;
  // If we reached the end of the circle buffer, reset
  if (begin_index >= (RING_BUFFER_SIZE)) {
    begin_index = 0;
    // Check if we are ready for prediction or still pending more initial data
    if (pending_initial_data) {
      pending_initial_data = false;
    }
  }


  // Return if we don't have enough data
  if (pending_initial_data) {
    // Serial.print(".");
    return false;
  }
    // Serial.println("!");


  for (int i = 0; i < length/3; i++) {
    int ring_array_index = begin_index + i - (length/3);
    if (ring_array_index < 0) {
      ring_array_index += (RING_BUFFER_SIZE+1);
    }
    input[i*3] = save_data[ring_array_index][0];
    // Serial.print(save_data[ring_array_index][0]);
    // Serial.print(" ");
    input[i*3+1] = save_data[ring_array_index][1];
    // Serial.print(save_data[ring_array_index][1]);
    // Serial.print(" ");
    input[i*3+2] = save_data[ring_array_index][2]*(-1.0);
    // Serial.println(save_data[ring_array_index][2]);
  }
  int ring_array_index = begin_index - (length/3);
  if (ring_array_index < 0) {
    ring_array_index += (RING_BUFFER_SIZE+1);
  }

  //Serial.print("i:");
  //Serial.print(ring_array_index);
  //Serial.print("x:");
  //Serial.print((save_data[ring_array_index][0]));
  //Serial.print(",y:");
  //Serial.print((save_data[ring_array_index][1]));
  //Serial.print(",z:");
  //Serial.println((save_data[ring_array_index][2]*(-1.0)));

  // pending_initial_data = true;

  return true;
}
