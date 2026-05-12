#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <queue.h>

#define BUTTON_SW 3
#define POT_PIN A0

// ── FreeRTOS handles ─────────────────────────────────────────
SemaphoreHandle_t buttonSemaphore;
SemaphoreHandle_t serialMutex;
QueueHandle_t sensorQueue;

volatile unsigned long lastPressTime = 0;

// ─────────────────────────────────────────────────────────────
// ISR — button debounced
// ─────────────────────────────────────────────────────────────
void isrButton()
{
  unsigned long now = millis();
  if (now - lastPressTime < 200)
    return;
  lastPressTime = now;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(buttonSemaphore, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR();
}

// ─────────────────────────────────────────────────────────────
// Task 1 — sensor producer
// Reads potentiometer every 200ms using vTaskDelayUntil
// Converts raw 0-1023 to percentage 0-100
// Simulates throttle position sensor (TPS)
// ─────────────────────────────────────────────────────────────
void taskSensorProducer(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(200);

  for (;;)
  {
    int raw = analogRead(POT_PIN);
    int percentage = map(raw, 0, 1023, 0, 100);

    // send to queue — non blocking, discard if full
    xQueueSend(sensorQueue, &percentage, 0);

    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}

// ─────────────────────────────────────────────────────────────
// Task 2 — sensor consumer
// Receives from queue and prints
// Simulates ECU processing throttle position
// ─────────────────────────────────────────────────────────────
void taskSensorConsumer(void *pvParameters)
{
  int value;

  for (;;)
  {
    if (xQueueReceive(sensorQueue, &value, pdMS_TO_TICKS(500)) == pdTRUE)
    {
      int raw = analogRead(POT_PIN);
      xSemaphoreTake(serialMutex, portMAX_DELAY);
      Serial.print(F("[TPS] raw="));
      Serial.print(raw);
      Serial.print(F("[TPS] throttle="));
      Serial.print(value);
      Serial.println(F("%"));
      xSemaphoreGive(serialMutex);
    }
  }
}

// ─────────────────────────────────────────────────────────────
// Task 3 — button handler
// Highest priority — responds instantly to brake event
// ─────────────────────────────────────────────────────────────
void taskButton(void *pvParameters)
{
  for (;;)
  {
    if (xSemaphoreTake(buttonSemaphore, portMAX_DELAY) == pdTRUE)
    {
      xSemaphoreTake(serialMutex, portMAX_DELAY);
      Serial.print(F("[BRK] brake at t="));
      Serial.print(millis());
      Serial.println(F("ms"));
      xSemaphoreGive(serialMutex);
    }
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.println(F("Boot"));

  pinMode(BUTTON_SW, INPUT_PULLUP);
  pinMode(POT_PIN, INPUT);

  attachInterrupt(digitalPinToInterrupt(BUTTON_SW), isrButton, FALLING);

  buttonSemaphore = xSemaphoreCreateBinary();
  serialMutex = xSemaphoreCreateMutex();
  sensorQueue = xQueueCreate(5, sizeof(int));

  xTaskCreate(taskSensorProducer, "Prod", 100, NULL, 2, NULL);
  xTaskCreate(taskSensorConsumer, "Cons", 100, NULL, 1, NULL);
  xTaskCreate(taskButton, "Btn", 100, NULL, 3, NULL);

  Serial.println(F("Go"));
  vTaskStartScheduler();
}

void loop() {}