#include <Arduino_FreeRTOS.h>

void vTask( void * pvParameters ){
  for( ;; ){
    vTaskDelay(1);
  }
}

// Perform an action every 200ms.
void vTaskFunction( void * pvParameters )
{
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 200 / portTICK_PERIOD_MS;

  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();
  for( ;; )
  {
    // Wait for the next cycle.
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    Serial.println("Tick");
  }
}

void setup(){
  Serial.begin(2000000);
  xTaskCreate( vTaskFunction, "Periodic", 128, NULL, 1, NULL );
  xTaskCreate( vTask, "*", 128, NULL, 2, NULL );
}

void loop(){
}
