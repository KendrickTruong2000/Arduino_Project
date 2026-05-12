#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

static volatile uint16_t hbTask10ms = 0;
static volatile uint8_t hbTask100ms = 0;
static volatile bool systemFault = false;

// hang triggered by watchdog after it confirms normal operation
static volatile bool triggerHang = false;

void task10ms(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(10);

  for (;;)
  {
    hbTask10ms++;

    // only hang when watchdog explicitly sets the flag
    if (triggerHang)
    {
      Serial.println(F("[10ms] HANG"));
      vTaskDelay(portMAX_DELAY);
    }

    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}

void task100ms(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(100);

  for (;;)
  {
    hbTask100ms++;

    if (!systemFault)
    {
      Serial.print(F("[100ms] hb10="));
      Serial.print(hbTask10ms);
      Serial.print(F(" hb100="));
      Serial.println(hbTask100ms);
    }

    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}

void taskWatchdog(void *pvParameters)
{
  // grace period
  vTaskDelay(pdMS_TO_TICKS(2000));

  uint16_t lastHb10ms = hbTask10ms;
  uint8_t lastHb100ms = hbTask100ms;

  Serial.print(F("[WDG] started hb10="));
  Serial.print(lastHb10ms);
  Serial.print(F(" hb100="));
  Serial.println(lastHb100ms);

  // let watchdog confirm 3 healthy cycles before triggering hang
  uint8_t healthyCycles = 0;

  for (;;)
  {
    vTaskDelay(pdMS_TO_TICKS(500));

    bool fault = false;

    if (hbTask10ms == lastHb10ms)
    {
      Serial.print(F("[WDG] FAULT task10ms t="));
      Serial.print(millis());
      Serial.println(F("ms"));
      fault = true;
    }

    if (hbTask100ms == lastHb100ms)
    {
      Serial.print(F("[WDG] FAULT task100ms t="));
      Serial.print(millis());
      Serial.println(F("ms"));
      fault = true;
    }

    lastHb10ms = hbTask10ms;
    lastHb100ms = hbTask100ms;

    if (fault)
    {
      systemFault = true;
      Serial.println(F("[WDG] ECU fault — reset triggered"));
      vTaskDelay(portMAX_DELAY);
    }
    else
    {
      healthyCycles++;
      Serial.print(F("[WDG] healthy cycle "));
      Serial.println(healthyCycles);

      // after 3 healthy cycles trigger the hang to demonstrate fault
      if (healthyCycles == 3)
      {
        Serial.println(F("[WDG] triggering hang for demo..."));
        triggerHang = true;
      }
    }
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.println(F("Boot"));

  xTaskCreate(task10ms, "10ms", 80, NULL, 2, NULL);
  xTaskCreate(task100ms, "100ms", 100, NULL, 2, NULL);
  xTaskCreate(taskWatchdog, "WDG", 100, NULL, 2, NULL);

  Serial.println(F("Go"));
  vTaskStartScheduler();
}

void loop() {}