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
#include <senseBoxIO.h>
// #include <Arduino.h>
#include <Wire.h>
#include <vl53l8cx_class.h>

#define DEV_I2C Wire
#define SerialPort Serial

#define LPN_PIN 4
#define I2C_RST_PIN -1
#define PWREN_PIN 2

#include <SD.h>
#include "SPI.h"

File file;
String dataStr = "";

void print_result(VL53L8CX_ResultsData *Result);
void clear_screen(void);
void handle_cmd(uint8_t cmd);
void display_commands_banner(void);

long measurements = 0;         // Used to calculate actual output rate
long measurementStartTime = 0;

// Components.
VL53L8CX sensor_vl53l8cx_top(&DEV_I2C, LPN_PIN, I2C_RST_PIN);

bool EnableAmbient = false;
bool EnableSignal = false;
uint8_t res = VL53L8CX_RESOLUTION_4X4;
char report[256];
int start = 0;


void receiveEvent(int bytes) {
  Serial.print("received: ");
  start = Wire.read();    // read one character from the I2C
  Serial.println(start);
}

/* Setup ---------------------------------------------------------------------*/
void setup()
{
  // Initialize serial for output.
  Serial.begin(9600);
  delay(2000);
  // while(!Serial) ;
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  Serial.println("setup sd card");
  // setting up SD card
  SD.begin(28);
  //clear out old data file
  if (SD.exists("/csv.txt")) 
  {
    Serial.println("Removing /csv.txt");
    SD.remove("/csv.txt");
    Serial.println("Done");
  } 
  //write csv headers to file:
  Serial.println("Writing file: /csv.txt");

  File file = SD.open("/csv.txt", FILE_WRITE);
  if(!file){
      Serial.println("Failed to open file for writing");
      return;
  }
  if(file.print("Time:Frame\n")){
      Serial.println("File written");
  } else {
      Serial.println("Write failed");
  }
  file.close();


  uint8_t status;
  
  // Initialize I2C bus. DEV_I2C is just a different name for Wire
  Wire.begin(9);
  // Wire.setClock(1000000); //Sensor has max I2C freq of 1MHz
  Wire.onReceive(receiveEvent);

  Serial.println("waiting for start signal...");
  for (int i =0; i<3; i++) {
    digitalWrite(7,HIGH);
    delay(200);
    digitalWrite(7,LOW);
    delay(200);
  };
  digitalWrite(7,HIGH);
  while (start==0) {
    delay(0);
  };
  measurementStartTime = millis();
  digitalWrite(7,LOW);

  Serial.println("Initializing sensor...");

  // Enable PWREN pin if present
  if (PWREN_PIN >= 0) {
    pinMode(PWREN_PIN, OUTPUT);
    digitalWrite(PWREN_PIN, HIGH);
    delay(10);
  }
  
  Serial.println("I2C Initialized");

  DEV_I2C.begin(39,40);
  DEV_I2C.setClock(1000000); //Sensor has max I2C freq of 1MHz

  // Configure VL53L8CX component.
  sensor_vl53l8cx_top.begin();
  Serial.println("Sensor library started");
  sensor_vl53l8cx_top.init_sensor();
  Serial.println("Sensor initialized");

  sensor_vl53l8cx_top.vl53l8cx_set_ranging_mode(VL53L8CX_RANGING_MODE_AUTONOMOUS);
  if (status) {
    snprintf(report, sizeof(report), "vl53l5cx_set_ranging_mode failed, status %u\r\n", status);
    SerialPort.print(report);
  }
  sensor_vl53l8cx_top.vl53l8cx_set_ranging_frequency_hz(30);
  if (status) {
    snprintf(report, sizeof(report), "vl53l8cx_set_ranging_frequency_hz failed, status %u\r\n", status);
    SerialPort.print(report);
  }
  delay(3000);

  // Start Measurements
  sensor_vl53l8cx_top.vl53l8cx_start_ranging();

  toggle_resolution();
  toggle_signal_and_ambient();

  Serial.println("Success");
  
  digitalWrite(8,HIGH);
}

void loop()
{
  VL53L8CX_ResultsData Results;
  uint8_t NewDataReady = 0;
  uint8_t status;

  do {
    status = sensor_vl53l8cx_top.vl53l8cx_check_data_ready(&NewDataReady);
  } while (!NewDataReady);

  if ((!status) && (NewDataReady != 0)) {
    status = sensor_vl53l8cx_top.vl53l8cx_get_ranging_data(&Results);
    print_result(&Results);

    // Uncomment to display actual measurement rate
    // measurements++;
    // float measurementTime = (millis() - measurementStartTime) / 1000.0;
    // Serial.print("rate: ");
    // Serial.print(measurements/measurementTime, 3);
    // Serial.println("Hz");
  }

  // if (Serial.available()>0)
  // {
  //   handle_cmd(Serial.read());
  // }

  // it cant be stopped because the I2C is busy with the sensor
  if(start==2){
    digitalWrite(7,HIGH);
    delay(5000);
    exit(0);
  }
}

void print_result(VL53L8CX_ResultsData *Result)
{
  int8_t i, j, k, l;
  uint8_t zones_per_line;
  uint8_t number_of_zones = res;

  zones_per_line = (number_of_zones == 16) ? 4 : 8;
  dataStr = "";
  //convert floats to string and assemble c-type char string for writing:
  dataStr += String(millis()-measurementStartTime) + ":";//add it onto the end

  for (j = 0; j < number_of_zones; j += zones_per_line)
  {
    for (l = 0; l < VL53L8CX_NB_TARGET_PER_ZONE; l++)
    {
      for (k = (zones_per_line - 1); k >= 0; k--)
      {
        //perform data processing here...
        if((long)Result->target_status[(VL53L8CX_NB_TARGET_PER_ZONE * (j+k)) + l] ==255){
          dataStr += String(5000) + ",";
        } else {
          dataStr += String((long)Result->distance_mm[(VL53L8CX_NB_TARGET_PER_ZONE * (j+k)) + l]) + ",";
        }
        dataStr += String((long)Result->target_status[(VL53L8CX_NB_TARGET_PER_ZONE * (j+k)) + l]) + ",";
        dataStr += String((long)Result->signal_per_spad[(VL53L8CX_NB_TARGET_PER_ZONE * (j+k)) + l]) + ",";
        dataStr += String((long)Result->ambient_per_spad[j+k]) + "; ";
      }
    }
  }
  dataStr += "\n";
  file = SD.open("/csv.txt", FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for appending");
    return;
  }
  if(!file.print(dataStr)){
    Serial.println("Append failed");
  }
  file.close();
}

void toggle_resolution(void)
{
  sensor_vl53l8cx_top.vl53l8cx_stop_ranging();

  switch (res)
  {
    case VL53L8CX_RESOLUTION_4X4:
      res = VL53L8CX_RESOLUTION_8X8;
      break;

    case VL53L8CX_RESOLUTION_8X8:
      res = VL53L8CX_RESOLUTION_4X4;
      break;

    default:
      break;
  }
  sensor_vl53l8cx_top.vl53l8cx_set_resolution(res);
  sensor_vl53l8cx_top.vl53l8cx_start_ranging();
}

void toggle_signal_and_ambient(void)
{
  EnableAmbient = (EnableAmbient) ? false : true;
  EnableSignal = (EnableSignal) ? false : true;
}

void clear_screen(void)
{
  snprintf(report, sizeof(report),"%c[2J", 27); /* 27 is ESC command */
  SerialPort.print(report);
}

void display_commands_banner(void)
{
  snprintf(report, sizeof(report),"%c[2H", 27); /* 27 is ESC command */
  SerialPort.print(report);

  Serial.print("53L7A1 Simple Ranging demo application\n");
  Serial.print("--------------------------------------\n\n");

  Serial.print("Use the following keys to control application\n");
  Serial.print(" 'r' : change resolution\n");
  Serial.print(" 's' : enable signal and ambient\n");
  Serial.print(" 'c' : clear screen\n");
  Serial.print("\n");
}

void handle_cmd(uint8_t cmd)
{
  switch (cmd)
  {
    case 'r':
      toggle_resolution();
      clear_screen();
      break;

    case 's':
      toggle_signal_and_ambient();
      clear_screen();
      break;

    case 'c':
      clear_screen();
      break;

    default:
      break;
  }
}