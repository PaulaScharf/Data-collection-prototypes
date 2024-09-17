#include <TensorFlowLite.h>

#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define kChannelNumber 1197

// Globals, used for compatibility with Arduino-style sketches.
namespace {
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* model_input = nullptr;
  int input_length;

  // Create an area of memory to use for input, output, and intermediate arrays.
  // The size of this will depend on the model you're using, and may need to be
  // determined by experimentation.
  constexpr int kTensorArenaSize = 32 * 1024 + 1008 ;
  uint8_t tensor_arena[kTensorArenaSize];
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(get_model_data());
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.printf("Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  // This imports all operations, which is more intensive, than just importing the ones we need.
  // If we ever run out of storage with a model, we can check here to free some space
  static tflite::AllOpsResolver resolver;

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  interpreter->AllocateTensors();
  // Obtain pointer to the model's input tensor.
  model_input = interpreter->input(0);

  Serial.println(model_input->dims->size);
  Serial.println(model_input->dims->data[0]);
  Serial.println(model_input->dims->data[1]);
  if ((model_input->dims->size != 2) || (model_input->dims->data[0] != 1) ||
      (model_input->dims->data[1] != kChannelNumber) || 
      (model_input->type != kTfLiteFloat32)) {
    Serial.println(model_input->dims->size);
    Serial.println(model_input->dims->data[0]);
    Serial.println(model_input->dims->data[1]);
    Serial.println(model_input->type);
    Serial.println("Bad input tensor parameters in model");
    return;
  }

  input_length = model_input->bytes / sizeof(float);
  Serial.printf("input_length: %i \n", input_length);

  bool setup_status = SetupMPU();
  if (!setup_status) {
    Serial.println("Setting up sensor failed\n");
  }
}

void Predict() {
    // Run inference, and report any error.
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
        Serial.printf("Invoke failed");
        return;
    }
    const float* prediction_scores = interpreter->output(0)->data.f;
    Serial.printf("step:%f,max:0.91 \n", prediction_scores[0]);
}

const long measurement_interval = 10; // Insert the recordingfrequency as it is given by edge impulse
long start_time = 0;
long actual_time = 0;
long rep_time = 0;

void loop() { 
  start_time = millis();
  if (start_time > actual_time + measurement_interval) {
    actual_time = millis();
    bool got_data =
        ReadMPU(model_input->data.f, input_length);
    //Serial.println(millis()-rep_time);
    rep_time = millis();
    // Only predict once we collected enough data
    if (got_data) {
      // long start = millis();
      Predict();
      // Serial.println(millis()-start);
    }
  } 

}
