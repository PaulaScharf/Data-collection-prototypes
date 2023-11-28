#include <Arduino.h>
#include <Wire.h>
#include <vl53l8cx_class.h>

void measure(void);
void print_result(VL53L8CX_ResultsData *Result);
String dataStr = "";

VL53L8CX_DetectionThresholds thresholds[VL53L8CX_NB_THRESHOLDS];
RTC_DATA_ATTR const int res = VL53L8CX_RESOLUTION_8X8;
RTC_DATA_ATTR bool trashcan_dimensions[res];

bool EnableAmbient = false;
bool EnableSignal = false;
char report[256];
volatile int interruptCount = 0;

RTC_DATA_ATTR int trashcan_distance_floor = 500;
uint8_t NewDataReady;
uint8_t status;
uint8_t number_of_zones = res;
int8_t i, j, k, l;
uint8_t zones_per_line = (number_of_zones == 16) ? 4 : 8;

void setup_vl53l8cx() {
  // Enable PWREN pin if present
  if (PWREN_PIN >= 0) {
    pinMode(PWREN_PIN, OUTPUT);
    digitalWrite(PWREN_PIN, HIGH);
    delay(10);
  }

  // Initialize I2C bus.
  DEV_I2C.begin(39,40);
  DEV_I2C.setClock(1000000); //Sensor has max I2C freq of 1MHz

  
  // Configure VL53L8CX component.
  sensor_VL53L8CX_top.begin();

  sensor_VL53L8CX_top.init_sensor();

  sensor_VL53L8CX_top.vl53l8cx_set_resolution(res);

  if(!preferences.isKey("t_dim")) {
    Serial.println("sensing initial dimensions");
    measure_trashcan(trashcan_dimensions);
    preferences.putBytes("t_dim", trashcan_dimensions, res*sizeof(bool));
  } else {
    preferences.getBytes("t_dim", trashcan_dimensions, res*sizeof(bool));
    Serial.println("initial dimensions recovered");
    for (j = 0; j < number_of_zones; j += zones_per_line)
    {  
      for (l = 0; l < VL53L8CX_NB_TARGET_PER_ZONE; l++)
      {
        // Print distance and status.
        for (k = (zones_per_line - 1); k >= 0; k--)
        {
          Serial.print(" ");
          Serial.print(trashcan_dimensions[(VL53L8CX_NB_TARGET_PER_ZONE * (j+k)) + l]);
          Serial.print(" ");
        }
      }
      Serial.println("");
    }
  }

  // Disable thresholds detection.
  sensor_VL53L8CX_top.vl53l8cx_set_detection_thresholds_enable(0U);

  setThresholds(100, trashcan_full);

  // // Set interrupt pin
  // pinMode(INT_PIN, INPUT);
  // attachInterrupt(digitalPinToInterrupt(INT_PIN), measure, FALLING);

  Serial.println("starting to measure");
  // Start Measurements.
  sensor_VL53L8CX_top.vl53l8cx_start_ranging(); 
}

bool checkMajority(VL53L8CX_ResultsData *Result, bool checkEmpty) {
  float positive = 0.;
  float total = 0.;
  int threshold = (checkEmpty)?500:100;
  for (i = 0; i < res; i++)
  {
    if(trashcan_dimensions[i]) {
      total++;
      long curDistance = (long)(Result)->distance_mm[i];
      if(validTargetStatus((int)(Result)->target_status[i]) && ((!checkEmpty) ? (curDistance < threshold) : (curDistance > threshold))) {
        positive++;
      }
    }
  }
  return (positive>0 && positive/total > 0.5);
}

bool checkIfUpdated(VL53L8CX_ResultsData *Result) {
  float positive = 0.;
  float total = (float)res;
  for (i = 0; i < res; i++)
  {
    if((int)(Result)->target_status[i] != 0) {
      positive++;
    }
  }
  return (positive>0 && positive/total > 0.9);
}

/*  To give a confidence rating, a target with status 5 is considered as 100% valid. 
A status of 6 or 9 can be considered with a confidence value of 50%. 
All other statuses are below the 50% confidence level. */
bool validTargetStatus(int status) {
  return status == 5 || status == 6 || status == 9;
}


void measure_trashcan(bool trashcan_dimensions[]) {
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
          trashcan_dimensions[j] = (long)(&Results)->distance_mm[j]>trashcan_distance_floor;
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
        Serial.print(" ");
        Serial.print(trashcan_dimensions[(VL53L8CX_NB_TARGET_PER_ZONE * (j+k)) + l]);
        Serial.print(" ");
      }
    }
    Serial.println("");
  }
  sensor_VL53L8CX_top.vl53l8cx_stop_ranging();
}

void print_result(VL53L8CX_ResultsData *Result)
{
  int8_t i, j, k, l;
  uint8_t zones_per_line;
  uint8_t number_of_zones = res;
    
  zones_per_line = (number_of_zones == 16) ? 4 : 8;

  Serial.print("\n\n");
  for (j = 0; j < number_of_zones; j += zones_per_line)
  {  
    for (l = 0; l < VL53L8CX_NB_TARGET_PER_ZONE; l++)
    {
      // Print distance and status.
      for (k = (zones_per_line - 1); k >= 0; k--)
      {
        Serial.print(" ");
        Serial.print((long)Result->distance_mm[(VL53L8CX_NB_TARGET_PER_ZONE * (j+k)) + l]);
        Serial.print(" ");
      }
      Serial.println("");
    }
  }
  Serial.print("\n");
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