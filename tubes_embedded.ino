#include <Arduino.h>
#include <Servo.h>
#include <Arduino_FreeRTOS.h>
#include <task.h>
#include <queue.h>

// Pin Definitions
#define L298N_IN1_PIN 2
#define L298N_ENA_PIN 3
#define SERVO1_PIN 9
#define SERVO2_PIN 10
#define BUTTON_PIN 11

// TCS230 or TCS3200 pins wiring to Arduino
#define S0 4
#define S1 5
#define S2 6
#define S3 7
#define SENSOR_OUT 8

// RGB Frequency
int redFrequency = 0;
int greenFrequency = 0;
int blueFrequency = 0;

// Servo objects
Servo servo1;
Servo servo2;

// Task Handlers
TaskHandle_t servo1TaskHandle = NULL;
TaskHandle_t dcMotorTaskHandle = NULL;
TaskHandle_t rgbSensorTaskHandle = NULL;
TaskHandle_t buttonTaskHandle = NULL;
TaskHandle_t buttonHandlerTaskHandle = NULL;

// Queue Handle
QueueHandle_t buttonQueue;

// Function
void servo1Task(void *pvParameters);
void dcMotorTask(void *pvParameters);
void rgbSensorTask(void *pvParameters);
void buttonTask(void *pvParameters);
void buttonHandlerTask(void *pvParameters);

void setup() {
  // Initialize Serial for debugging
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Setup start");

  // Attach servos
  Serial.println("Attaching servos...");
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);

  // Initialize L298N motor driver pins
  pinMode(L298N_IN1_PIN, OUTPUT);
  pinMode(L298N_ENA_PIN, OUTPUT);

  // Initialize button pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initialize RGB sensor pins
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(SENSOR_OUT, INPUT);

  // Setting frequency scaling to 2%
  digitalWrite(S0, LOW);
  digitalWrite(S1, HIGH);

  // Create the queue for the button
  buttonQueue = xQueueCreate(1, sizeof(bool));
  if (buttonQueue == NULL) {
    Serial.println("Failed to create buttonQueue");
  } else {
    Serial.println("buttonQueue created successfully");
  }

  // Create the tasks
  BaseType_t result;

  result = xTaskCreate(servo1Task, "Servo1 Task", 256, NULL, 1, &servo1TaskHandle);
  result = xTaskCreate(dcMotorTask, "DC Motor Task", 256, NULL, 1, &dcMotorTaskHandle);
  result = xTaskCreate(rgbSensorTask, "RGB Sensor Task", 256, NULL, 1, &rgbSensorTaskHandle);
  result = xTaskCreate(buttonTask, "Button Task", 256, NULL, 2, &buttonTaskHandle);
  result = xTaskCreate(buttonHandlerTask, "Button Handler Task", 256, NULL, 3, &buttonHandlerTaskHandle);
  Serial.println("Starting scheduler...");
  vTaskStartScheduler();
}

void loop() {
}

void servo1Task(void *pvParameters) {
  bool servo1State = false;
  while (1) {
    if (servo1State) {
      servo1.write(0);
    } else {
      servo1.write(180);
    }
    servo1State = !servo1State;
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

void dcMotorTask(void *pvParameters) {
  while (1) {
    // Set direction and enable motor
    digitalWrite(L298N_IN1_PIN, HIGH);
    analogWrite(L298N_ENA_PIN, 180);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void rgbSensorTask(void *pvParameters) {
  while (1) {
    // Read red
    digitalWrite(S2, LOW);
    digitalWrite(S3, LOW);
    redFrequency = pulseIn(SENSOR_OUT, LOW);

    // Read green
    digitalWrite(S2, HIGH);
    digitalWrite(S3, HIGH);
    greenFrequency = pulseIn(SENSOR_OUT, LOW);

    // Read blue
    digitalWrite(S2, LOW);
    digitalWrite(S3, HIGH);
    blueFrequency = pulseIn(SENSOR_OUT, LOW);

    int averageColor = (redFrequency + greenFrequency + blueFrequency) / 3;
    if (averageColor < 1400) {
      // White
      servo2.write(180);
    } else {
      // Black
      servo2.write(-180);
    }
    Serial.print("R = ");
    Serial.print(redFrequency);
    Serial.print(" G = ");
    Serial.print(greenFrequency);
    Serial.print(" B = ");
    Serial.println(blueFrequency);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void buttonTask(void *pvParameters) {
  while (1) {
    bool buttonState = digitalRead(BUTTON_PIN) == LOW;
    if (buttonState) {
      Serial.println("Button pressed");
      if (xQueueSend(buttonQueue, &buttonState, portMAX_DELAY) == pdPASS) {
        Serial.println("Button press sent to queue");
      } else {
        Serial.println("Failed to send button press to queue");
      }
      vTaskPrioritySet(NULL, 3);
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void buttonHandlerTask(void *pvParameters) {
  bool buttonState;
  while (1) {
    if (xQueueReceive(buttonQueue, &buttonState, portMAX_DELAY)) {
      Serial.println("Button press received from queue");
      if (buttonState) {
        if (dcMotorTaskHandle != NULL) {
          analogWrite(L298N_ENA_PIN, 0);
          Serial.println("Deleting DC motor task");
          vTaskDelete(dcMotorTaskHandle);
          dcMotorTaskHandle = NULL;
          vTaskDelay(5000 / portTICK_PERIOD_MS);
          Serial.println("Recreating DC motor task");
          xTaskCreate(dcMotorTask, "DC Motor Task", 256, NULL, 1, &dcMotorTaskHandle);
        }
      }
    }
  }
}
