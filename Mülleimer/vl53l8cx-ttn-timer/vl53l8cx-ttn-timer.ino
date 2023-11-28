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
#include <vl53l8cx_class.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

#define INT_PIN 3
#define DONE_PIN 6
#define IO_ENABLE 8

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  25        /* Time ESP32 will go to sleep (in seconds) */

static osjob_t sendjob;

#include <Preferences.h>
Preferences preferences;

bool trashcan_full = false;
bool lora_in_progress = false;

#define DEV_I2C Wire
#define LPN_PIN 4
#define I2C_RST_PIN -1
#define PWREN_PIN 2
VL53L8CX sensor_VL53L8CX_top(&DEV_I2C, LPN_PIN, I2C_RST_PIN);

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
  pinMode(IO_ENABLE, OUTPUT);
  digitalWrite(IO_ENABLE,LOW);
  pinMode(DONE_PIN, OUTPUT);
  digitalWrite(DONE_PIN, LOW); 
  // Initialize serial for output.
  Serial.begin(9600);
  // while(!Serial);
  // delay(2000);

  gpio_wakeup_disable((gpio_num_t)(INT_PIN));
  // print_wakeup_reason();  

  preferences.begin("trashcan", false);

  // init Lora
  Serial.println("initing Lora");
  setup_lora();

  attachInterrupt(0, clearPreferences, CHANGE);
  setup_vl53l8cx();
}

int loop_timeout = 10;
void loop()
{
  VL53L8CX_ResultsData Results;
  uint8_t NewDataReady = 0;
  uint8_t status;
  
  do {
    status = sensor_VL53L8CX_top.vl53l8cx_check_data_ready(&NewDataReady);
  } while (!NewDataReady);

  if ((!status) && (NewDataReady != 0)) {
    status = sensor_VL53L8CX_top.vl53l8cx_get_ranging_data(&Results);
    if (checkIfUpdated(&Results)) {
      trashcan_full = checkMajority(&Results, false);
      Serial.println("trashcan " + String((trashcan_full)?"full":"empty"));
      lora_in_progress = true;
      notifyViaLora();
      // execute scheduled jobs and events for Lora
      while(lora_in_progress) {
        os_runloop_once();
        delay(1);
      }
      enterSleepMode();
    } else {
      loop_timeout--;
      if(loop_timeout<0) {
        loop_timeout = 10;
        enterSleepMode();
      }
    }
  }
}

void clearPreferences() {
  Serial.println("clearing preferences...");
  preferences.clear();
}



void enterSleepMode(void)
{
  digitalWrite(DONE_PIN, LOW); 
  digitalWrite(DONE_PIN, HIGH); 
  delay(10);
}


void tx_callback(osjob_t* job) {
  lora_in_progress = false;
}

void notifyViaLora() {
  do_send(&sendjob, trashcan_full);
}
