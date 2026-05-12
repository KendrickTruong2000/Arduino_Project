#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

#define POT_PIN A0
#define BUTTON_SW 3
#define LED_PIN 13

SemaphoreHandle_t serialMutex;

// ── Simulated ECU signals ─────────────────────────────────────
// In a real ECU these would be shared via a global signal database
// (AUTOSAR calls this the RTE — Runtime Environment)
static volatile int gThrottle = 0;    // 0-100%
static volatile int gEngineTemp = 20; // degrees C
static volatile long gOdometer = 0;   // cumulative counts
static volatile bool gButtonState = false;

volatile unsigned long lastPressTime = 0;

void isrButton()
{
  unsigned long now = millis();
  if (now - lastPressTime < 200)
    return;
  lastPressTime = now;
  gButtonState = true; // flag for 100ms task to pick up
}

// ─────────────────────────────────────────────────────────────
// 10ms task — fast control loop
// Reads throttle position sensor every 10ms
// Simulates: engine management, ABS wheel speed processing
// Highest priority — must never miss deadline
// ─────────────────────────────────────────────────────────────
void task10ms(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(10);
  static int missedDeadlines = 0;

  for (;;)
  {
    // read throttle — fast analog read
    int raw = analogRead(POT_PIN);
    gThrottle = map(raw, 0, 1023, 0, 100);
    gOdometer += gThrottle; // accumulate as proxy for distance

    // check if we missed our deadline
    // xLastWakeTime would have been updated even if late
    // so we detect by checking tick count drift
    if (xTaskGetTickCount() > xLastWakeTime + xPeriod)
    {
      missedDeadlines++;
    }

    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}

// ─────────────────────────────────────────────────────────────
// 100ms task — medium rate monitoring
// Simulates: temperature monitoring, voltage check, event detection
// ─────────────────────────────────────────────────────────────
void task100ms(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(100);

  for (;;)
  {
    // simulate engine temp slowly rising with throttle
    // in real ECU this would read an ADC connected to NTC thermistor
    gEngineTemp += (gThrottle > 50) ? 1 : -1;
    gEngineTemp = constrain(gEngineTemp, 20, 120);

    // pick up button event
    if (gButtonState)
    {
      gButtonState = false;
      xSemaphoreTake(serialMutex, portMAX_DELAY);
      Serial.print(F("[100ms] BRAKE at t="));
      Serial.print(millis());
      Serial.println(F("ms"));
      xSemaphoreGive(serialMutex);
    }

    // blink LED at 100ms rate when throttle > 50%
    digitalWrite(LED_PIN, gThrottle > 50 ? HIGH : LOW);

    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}

// ─────────────────────────────────────────────────────────────
// 1000ms task — slow housekeeping
// Simulates: dashboard refresh, DTC logging, comms heartbeat
// Lowest priority — best effort, can be delayed by higher tasks
// ─────────────────────────────────────────────────────────────
void task1000ms(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(1000);

  for (;;)
  {
    xSemaphoreTake(serialMutex, portMAX_DELAY);
    Serial.println(F("──────────────────────"));
    Serial.print(F("[1000ms] t="));
    Serial.print(millis());
    Serial.println(F("ms"));
    Serial.print(F("[1000ms] throttle="));
    Serial.print(gThrottle);
    Serial.println(F("%"));
    Serial.print(F("[1000ms] engineTemp="));
    Serial.print(gEngineTemp);
    Serial.println(F("C"));
    Serial.print(F("[1000ms] odometer="));
    Serial.println(gOdometer);
    Serial.println(F("──────────────────────"));
    xSemaphoreGive(serialMutex);

    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.println(F("Boot"));

  pinMode(POT_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_SW, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(BUTTON_SW), isrButton, FALLING);

  serialMutex = xSemaphoreCreateMutex();

  // priority mirrors rate — faster task = higher priority
  // this ensures high-frequency tasks always preempt low-frequency ones
  xTaskCreate(task10ms, "10ms", 120, NULL, 3, NULL);
  xTaskCreate(task100ms, "100ms", 120, NULL, 2, NULL);
  xTaskCreate(task1000ms, "1000ms", 160, NULL, 1, NULL);

  Serial.println(F("Go"));
  vTaskStartScheduler();
}

void loop() {}