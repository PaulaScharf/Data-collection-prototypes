/**
 ******************************************************************************
 * @file    VL53L8CX_ThresholdDetection.ino
 * @author  STMicroelectronics
 * @version V1.0.0
 * @date    12 June 2023
 * @brief   Arduino test application for the STMicrolectronics VL53L8CX
 *          proximity sensor satellite based on FlightSense.
 *          This application makes use of C++ classes obtained from the C
 *          components' drivers.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2021 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */
/*
 * To use these examples you need to connect the VL53L8CX satellite sensor directly to the Nucleo board with wires as explained below:
 * pin 1 (SPI_I2C_n) of the VL53L8CX satellite connected to pin GND of the Nucleo board
 * pin 2 (LPn) of the VL53L8CX satellite connected to pin A3 of the Nucleo board
 * pin 3 (NCS) not connected
 * pin 4 (MISO) not connected
 * pin 5 (MOSI_SDA) of the VL53L8CX satellite connected to pin D14 (SDA) of the Nucleo board
 * pin 6 (MCLK_SCL) of the VL53L8CX satellite connected to pin D15 (SCL) of the Nucleo board
 * pin 7 (PWREN) of the VL53L8CX satellite connected to pin D11 of the Nucleo board
 * pin 8 (I0VDD) of the VL53L8CX satellite not connected
 * pin 9 (3V3) of the VL53L8CX satellite connected to 3V3 of the Nucleo board
 * pin 10 (1V8) of the VL53L8CX satellite not connected
 * pin 11 (5V) of the VL53L8CX satellite not connected 
 * GPIO1 of VL53L8CX satellite connected to A2 pin of the Nucleo board (not used)
 * GND of the VL53L8CX satellite connected to GND of the Nucleo board
 */

/* Includes ------------------------------------------------------------------*/
#include <Arduino.h>
#include <Wire.h>
#include <vl53l8cx_class.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "driver/rtc_io.h"

#define DEV_I2C Wire
#define SerialPort Serial

#define LPN_PIN 4
#define I2C_RST_PIN -1
#define PWREN_PIN 2
#define INT_PIN 3

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  25        /* Time ESP32 will go to sleep (in seconds) */

static osjob_t sendjob;

void measure(void);
void print_result(VL53L8CX_ResultsData *Result);
String dataStr = "";

// Component.
VL53L8CX sensor_VL53L8CX_top(&DEV_I2C, LPN_PIN, I2C_RST_PIN);
VL53L8CX_DetectionThresholds thresholds[VL53L8CX_NB_THRESHOLDS];
RTC_DATA_ATTR const int res = VL53L8CX_RESOLUTION_8X8;
RTC_DATA_ATTR long trashcan_dimensions[res];
long testi[res];

bool EnableAmbient = false;
bool EnableSignal = false;
char report[256];
volatile int interruptCount = 0;

RTC_DATA_ATTR bool trashcan_full = false;
RTC_DATA_ATTR int trashcan_distance_floor = 500;
RTC_DATA_ATTR bool is_sleeping = false;
bool lora_in_progress = false;

uint8_t NewDataReady;
uint8_t status;
uint8_t number_of_zones = res;
int8_t i, j, k, l;
uint8_t zones_per_line = (number_of_zones == 16) ? 4 : 8;

/* Setup ---------------------------------------------------------------------*/

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void setup()
{

  // gpio_wakeup_disable((gpio_num_t)(INT_PIN));

  // Initialize serial for output.
  SerialPort.begin(9600);
  while(!SerialPort);
  // delay(2000);

  // Enable PWREN pin if present
  if (PWREN_PIN >= 0) {
    pinMode(PWREN_PIN, OUTPUT);
    digitalWrite(PWREN_PIN, HIGH);
    // gpio_hold_en((gpio_num_t)PWREN_PIN);
    delay(10);
  }

  // init Lora
  Serial.println("initing Lora");
  setup_lora();

  // Initialize I2C bus.
  DEV_I2C.begin(39,40);
  DEV_I2C.setClock(1000000); //Sensor has max I2C freq of 1MHz

  
  // Configure VL53L8CX component.
  sensor_VL53L8CX_top.begin();

  sensor_VL53L8CX_top.init_sensor();

  sensor_VL53L8CX_top.vl53l8cx_set_resolution(res);

  Serial.println("sensing initial dimensions");
  measure_trashcan(trashcan_dimensions);

  // Disable thresholds detection.
  sensor_VL53L8CX_top.vl53l8cx_set_detection_thresholds_enable(0U);

  setThresholds(100, trashcan_full);

  print_wakeup_reason();  

  // // Set interrupt pin
  // pinMode(INT_PIN, INPUT);
  // attachInterrupt(digitalPinToInterrupt(INT_PIN), measure, FALLING);

  Serial.println("starting to measure");
  // Start Measurements.
  sensor_VL53L8CX_top.vl53l8cx_start_ranging(); 
  delay(1000);
  // LMIC_shutdown();
  enterSleepMode();
}

void loop()
{
  // trashcan_full = !trashcan_full;
  // Serial.println("trashcan " + String((trashcan_full)?"full":"empty"));
  // lora_in_progress = true;
  // notifyViaLora();
  // // execute scheduled jobs and events for Lora
  // while(lora_in_progress) {
  //   os_runloop_once();
  //   delay(1);
  // }
  // delay(3000);
  VL53L8CX_ResultsData Results;
  uint8_t NewDataReady = 0;
  uint8_t status;
  
  do {
    status = sensor_VL53L8CX_top.vl53l8cx_check_data_ready(&NewDataReady);
  } while (!NewDataReady);

  if ((!status) && (NewDataReady != 0)) {
    delay(2000);
    // print_wakeup_reason();  
    status = sensor_VL53L8CX_top.vl53l8cx_get_ranging_data(&Results);
    // print_result(&Results);
    if(checkMajority(&Results, trashcan_full)) {
      trashcan_full = !trashcan_full;
      Serial.println("trashcan " + String((trashcan_full)?"full":"empty"));
      lora_in_progress = true;
      // setup_lora();
      notifyViaLora();
      // LMIC_shutdown();
      // execute scheduled jobs and events for Lora
      while(lora_in_progress) {
        // Serial.println("os_runloop_once();");
        os_runloop_once();
        delay(1);
      }
    }
    interruptCount = 0;
    delay(1000);
    sensor_VL53L8CX_top.begin();
    sensor_VL53L8CX_top.init_sensor();
    sensor_VL53L8CX_top.vl53l8cx_set_resolution(res);
    // Disable thresholds detection.
    sensor_VL53L8CX_top.vl53l8cx_set_detection_thresholds_enable(0U);
    setThresholds((trashcan_full)?500:100, trashcan_full);
    // Start Measurements.
    sensor_VL53L8CX_top.vl53l8cx_start_ranging(); 
    enterSleepMode();
  }
}

bool checkMajority(VL53L8CX_ResultsData *Result, bool checkEmpty) {
  float positive = 0.;
  float total = 0.;
  int threshold = (checkEmpty)?500:100;
  for (i = 0; i < res; i++)
  {
    if(trashcan_dimensions[i]>trashcan_distance_floor) {
      total++;
      if((!checkEmpty) ? ((long)(Result)->distance_mm[i] < threshold) : ((long)(Result)->distance_mm[i] > threshold)) {
        positive++;
      }
    }
  }
  return (positive>0 && positive/total > 0.5);
}

void enterSleepMode(void)
{
    // esp_sleep_pd_config(ESP_PD_DOMAIN_VDDSDIO, ESP_PD_OPTION_ON);

    // gpio_hold_en((gpio_num_t)8);
    // rtc_gpio_pulldown_dis((gpio_num_t)8);
    // rtc_gpio_pullup_en((gpio_num_t)8);
    // rtc_gpio_pulldown_en((gpio_num_t)INT_PIN);
    // rtc_gpio_pullup_dis((gpio_num_t)INT_PIN);
    // delay(200);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)(INT_PIN),1);
    esp_light_sleep_start();
}

void measure_trashcan(long trashcan_dimensions[]) {
  sensor_VL53L8CX_top.vl53l8cx_start_ranging(); 
  int initialized = 3;
  VL53L8CX_ResultsData Results;
  while(initialized>0) {
    NewDataReady = 0;
    do {
      status = sensor_VL53L8CX_top.vl53l8cx_check_data_ready(&NewDataReady);
    } while (!NewDataReady);

    if ((!status) && (NewDataReady != 0)) {
      status = sensor_VL53L8CX_top.vl53l8cx_get_ranging_data(&Results);
      // long * dataStr = [];
      //convert floats to string and assemble c-type char string for writing:
      // dataStr += String(millis()-measurementStartTime) + ":";//add it onto the end

      for (j = 0; j < number_of_zones; j ++)
      {
        //perform data processing here...
        if((long)(&Results)->target_status[j] !=255){
          trashcan_dimensions[j] = (long)(&Results)->distance_mm[j];
        }
      }
      initialized--;
    }
  }
  for (j = 0; j < number_of_zones; j += zones_per_line)
  {  
    for (l = 0; l < VL53L8CX_NB_TARGET_PER_ZONE; l++)
    {
      // Print distance and status.
      for (k = (zones_per_line - 1); k >= 0; k--)
      {
        SerialPort.print(" ");
        SerialPort.print(trashcan_dimensions[(VL53L8CX_NB_TARGET_PER_ZONE * (j+k)) + l]);
        SerialPort.print(" ");
      }
    }
    SerialPort.println("");
  }
  sensor_VL53L8CX_top.vl53l8cx_stop_ranging();
}

void tx_callback(osjob_t* job) {
  lora_in_progress = false;
}

void notifyViaLora() {
  do_send(&sendjob, trashcan_full);
  // tx(String(trashcan_full).c_str(), tx_callback); 
}

void print_result(VL53L8CX_ResultsData *Result)
{
  int8_t i, j, k, l;
  uint8_t zones_per_line;
  uint8_t number_of_zones = res;
    
  zones_per_line = (number_of_zones == 16) ? 4 : 8;

  SerialPort.print("\n\n");
  for (j = 0; j < number_of_zones; j += zones_per_line)
  {  
    for (l = 0; l < VL53L8CX_NB_TARGET_PER_ZONE; l++)
    {
      // Print distance and status.
      for (k = (zones_per_line - 1); k >= 0; k--)
      {
        SerialPort.print(" ");
        SerialPort.print((long)Result->distance_mm[(VL53L8CX_NB_TARGET_PER_ZONE * (j+k)) + l]);
        SerialPort.print(" ");
      }
      SerialPort.println("");
    }
  }
  SerialPort.print("\n");
}

void measure(void)
{
  interruptCount = 1;
}

void setThresholds(int threshold, bool far) {
  // Set all values to 0.
  memset(&thresholds, 0, sizeof(thresholds));
  // Configure thresholds on each active zone
  for (i = 0; i < res; i++)
  {
    if(trashcan_dimensions[i]>trashcan_distance_floor) {
      thresholds[i].zone_num = i;
      thresholds[i].measurement = VL53L8CX_DISTANCE_MM;
      thresholds[i].type = (!far) ? VL53L8CX_LESS_THAN_EQUAL_MIN_CHECKER : VL53L8CX_GREATER_THAN_MAX_CHECKER;
      thresholds[i].mathematic_operation = VL53L8CX_OPERATION_NONE;
      thresholds[i].param_low_thresh = threshold;
      thresholds[i].param_high_thresh = threshold;
    }
  }


  // Last threshold must be clearly indicated.
  thresholds[i].zone_num |= VL53L8CX_LAST_THRESHOLD;

  // Send array of thresholds to the sensor.
  sensor_VL53L8CX_top.vl53l8cx_set_detection_thresholds(thresholds);

  // Enable thresholds detection.
  sensor_VL53L8CX_top.vl53l8cx_set_detection_thresholds_enable(1U);
}