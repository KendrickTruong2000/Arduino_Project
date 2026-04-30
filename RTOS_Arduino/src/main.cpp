#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

void taskBlink(void *pvParameter)
{
  pinMode(8, OUTPUT);

  while (1)
  {
    digitalWrite(8, HIGH);
    vTaskDelay(pdMS_TO_TICKS(500));
    digitalWrite(8, LOW);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void taskSerial(void *pvParameter)
{
  while (1)
  {
    Serial.println("taskSerial: alive");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup()
{
  Serial.begin(9600);

  xTaskCreate(taskBlink, "Blink", 128, NULL, 1, NULL);
  xTaskCreate(taskSerial, "Serial", 128, NULL, 1, NULL);
  vTaskStartScheduler();
}

void loop()
{
}

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