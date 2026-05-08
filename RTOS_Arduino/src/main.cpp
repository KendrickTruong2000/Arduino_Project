#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

/*NOTE for RTOS
1. Always Give after Take - a mutext never released is a deadlock waiting to happen
2. Never call xSemaphoreTake from an ISR - use xSemaphoreGiveFromISR instead
3. keep the critical short - hodling a mutex blcoks every other task that needs it
*/

SemaphoreHandle_t serialMutex;

void TaskA(void *pvParameter);
void TaskB(void *pvParameter);

void setup()
{
  Serial.begin(9600);
  serialMutex = xSemaphoreCreateMutex();

  xTaskCreate(TaskA, "A", 128, NULL, 1, NULL);
  xTaskCreate(TaskB, "B", 128, NULL, 1, NULL);
  vTaskStartScheduler();
}

void loop()
{
}

void TaskA(void *pvParameter)
{
  while (1)
  {
    // acquire the mutex - blocks here if TaskB hold it
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
      Serial.print("[A] temperature: ");
      vTaskDelay(pdMS_TO_TICKS(2));
      Serial.println("25 C");
      xSemaphoreGive(serialMutex);
    }
    vTaskDelay(pdMS_TO_TICKS(300));
  }
}

void TaskB(void *pvParameter)
{
  while (1)
  {
    // acquire the mutex - blocks here if TaskA holds it
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
      Serial.print("[B] speed: ");
      vTaskDelay(pdMS_TO_TICKS(2));
      Serial.println("120 km/h");
      xSemaphoreGive(serialMutex);
    }
    vTaskDelay(pdMS_TO_TICKS(300));
  }
}
// void taskBlink(void *pvParameter)
// {
//   pinMode(8, OUTPUT);

//   while (1)
//   {
//     digitalWrite(8, HIGH);
//     vTaskDelay(pdMS_TO_TICKS(200));
//     digitalWrite(8, LOW);
//     vTaskDelay(pdMS_TO_TICKS(200));
//   }
// }

// void taskSerial(void *pvParameter)
// {
//   while (1)
//   {
//     Serial.println("taskSerial: alive");
//     vTaskDelay(pdMS_TO_TICKS(1000));
//   }
// }

// void setup()
// {
//   Serial.begin(9600);

//   xTaskCreate(taskBlink, "Blink", 128, NULL, 2, NULL);
//   xTaskCreate(taskSerial, "Serial", 128, NULL, 1, NULL);
//   vTaskStartScheduler();
// }

// void loop()
// {
// }

// #include <Arduino.h>
// #include <Arduino_FreeRTOS.h>

// // put function declarations here:
// void Task1(void *pvParameter);
// void Task2(void *pvParameter);
// void TaskPrint(void *pvParameter);

// void setup()
// {
//   Serial.begin(9600);
//   // put your setup code here, to run once:
//   xTaskCreate(Task1, "task1", 128, NULL, 1, NULL);
//   xTaskCreate(Task2, "Task2", 128, NULL, 2, NULL);
//   xTaskCreate(TaskPrint, "Task3", 128, NULL, 3, NULL);
// }

// void loop()
// {
//   // put your main code here, to run repeatedly:
// }

// void Task1(void *pvParameter)
// {
//   pinMode(8, OUTPUT);
//   while (1)
//   {
//     Serial.println("Task1");
//     digitalWrite(8, HIGH);
//     vTaskDelay(200 / portTICK_PERIOD_MS);
//     digitalWrite(8, LOW);
//     vTaskDelay(200 / portTICK_PERIOD_MS);
//   }
// }

// void Task2(void *pvParameter)
// {
//   pinMode(7, OUTPUT);
//   while (1)
//   {
//     Serial.println("Task2");
//     digitalWrite(7, HIGH);
//     vTaskDelay(300 / portTICK_PERIOD_MS);
//     digitalWrite(7, LOW);
//     vTaskDelay(300 / portTICK_PERIOD_MS);
//   }
// }

// void TaskPrint(void *pvParameter)
// {
//   int counter = 0;
//   while (1)
//   {
//     counter++;
//     Serial.println(counter);
//     vTaskDelay(500 / portTICK_PERIOD_MS);
//   }
// }